#ifndef RENDERDATACONVERTER_H
#define RENDERDATACONVERTER_H

#include "btl/containers/vector.h"
#include "btl/containers/map.h"
#include "btl/utility/string.h"
#include "renderdataconversion/clustering.h"
#include "renderdataconversion/common.h"
#include "renderingdata/renderingdata.h"

namespace TextureExport
{
	class TextureManager;
}
namespace RenderDataConversion
{
	class MaterialManager;
	class I_PolyInform;
	class Effect;
	struct Material;
	class AftkMeshConverter;
}

namespace RenderingData
{
	namespace Build
	{
		class Scene;
		class Model;
	}
	class BatchIDGenerator;
	class ResourceIndex;
	class PrimitiveInfo;

	typedef Btl::Vector< const Aftk::C_SceneNode* > C_AftkSceneNodeList;

	struct BaseInstanceData
	{
		int iBaseModelIdx;
		RenderDataConversion::Effect *pEffect;
		const Aftk::C_SceneMeshNode *pMesh;
		Btl::Vector<S32> m_iBatchIDs;
	};

	/// renderingdata converter class
	class C_Converter
	{
	public:
		/// constructor
		C_Converter(const Util::C_String sTextureCachePath, const Util::C_String sFXBPath);

		/// destructor
		~C_Converter();

		/// begin the building of a new model
		void BeginModel();
		
		/// add an element to the model that is currently being built
		void AddElement( const Maths::HMat43 &worldMtx, const C_AftkSceneNodeList& aftkSceneNodeList);

		/// finish building the current model
		void EndModel();

		/// add list of Aftk nodes that are to be built into a single static mesh
		void AddStaticNodes( const C_AftkSceneNodeList& aftkSceneNodeList );

		/// this is called once to build the rendering data
		void Process();

		/// set a callback class which passes polyflag information to the renderdata builder
		void SetPolyInform( const RenderDataConversion::I_PolyInform *p );

		// generate a test version of scene by hand
		void TestProcess();

		/// set the name of the effect to use when no specific effect was specified (for example in the HLSL shader)
		static void SetDefaultEffect(const char *psz);
		
		/// set the variation to use for all shaders. this variation will be appended to the name of each .fx file
		static void SetEffectVariation(const char *psz);

		/// write out the rendering data for a particular platform (call Process first)
		bool ExportBinary(Sys::I_File* pBinaryStream, Sys::I_File* pStreamFile, RenderingData::PlatformType platformType);
		bool ExportXml(Sys::I_File* pBinaryStream, RenderingData::PlatformType platformType);

	private:
		void BuildStreamData(RenderingData::Build::Scene& scene);
		const RenderingData::Build::Scene& GetExportScene() const { return *m_pSceneExport; }

		/// construct a RenderingData::PrimitiveInfo from a given Aftk polygon
		bool GetPrimitiveInfo(const RenderDataConversion::AftkMeshConverter *pMesh, int iPoly, RenderDataConversion::E_NodeType nodeType, RenderingData::PrimitiveInfo *pPrimitiveInfo, RenderDataConversion::Material *pConvertedMtl);

		/// returns a material index for the material containing element data for the nth element
		int GenerateElementMaterial(int iElement);

		/// create a new cullnode
		RenderingData::ResourceIndex C_Converter::AddCullNode(const Btl::String &name, RenderingData::Node& parent, S32 blockIndex);

		BatchIDGenerator *m_pBatchIDGenerator;

		RenderingData::Build::Scene* m_pSceneExport;

		RenderingData::ResourceIndex m_iCurrentNode;

		/// this is called once to build the rendering data
		void Process(const C_AftkSceneNodeList &nodes);

		/// perform clustering on the Aftk mesh
		void Process1(const Aftk::C_SceneMeshNode* pMeshNode);
		void Process1(const Aftk::C_SceneNode* pInstanceNode);

		/// build the actual renderdata
		void Process2(const Aftk::C_SceneMeshNode* pNode);
		void Process2(const Aftk::C_SceneNode* pInstanceNode);

		void GetClusteringData(const Aftk::C_SceneMeshNode& node, S32 polygonIndex, S32 baseModelIndex, RenderDataConversion::Clustering::Polygon* clusteringPolygon);
		void GetMaterial(const Aftk::C_Polygon& aftkPolygon, RenderDataConversion::E_NodeType nodeType, RenderDataConversion::Material* convertedMtl);
		void GetClusteringData(const Aftk::C_SceneNode& node, const BaseInstanceData& baseInstanceData, RenderDataConversion::Clustering::Instance* clusteringInstance);

		void CreateInstance(const Aftk::C_SceneNode *pNode, BaseInstanceData &data);
		void CreateModelNodes();
		void CreateClusterNode(RenderingData::ResourceIndex parent, const RenderDataConversion::Clustering::Node& node);

		S32 InsertBaseModel(const Aftk::C_SceneNode* pNode);

		// test functions
		S32							CreateTestEffect(const Btl::String& effectName);
		S32							CreateTestMaterial(const Btl::String& materialName, S32 effectIndex);

		struct ModelData
		{
			ModelData() : modelIndex(0), elementIndex(-1), elementMaterialIndex(-1) {}
			S32				modelIndex;
			S32				elementIndex;
			S32				elementMaterialIndex;
			Maths::HMat43	elementMatrix;
		};

		ModelData					m_ModelData;

		//int							m_iCurrentInstanceId;

		C_AftkSceneNodeList			m_AftkSceneNodes;

		typedef Btl::Map<const Aftk::C_SceneNode*, BaseInstanceData> BaseInstanceMap;
		BaseInstanceMap m_BaseInstanceMap;

		int m_iBlockIndexOffset;

		TextureExport::TextureManager* m_pTextureManager;
		RenderDataConversion::MaterialManager* m_pMtlConverter;
		const RenderDataConversion::I_PolyInform* m_pPolyInform;
		RenderDataConversion::Clustering::I_Scene* m_pClusteringScene;
		Btl::Map<S32, RenderingData::Build::Model*> m_modelPointers;
		Btl::Map<S32, RenderingData::RenderNode*> m_instancePointers;
		Btl::Map<const Aftk::C_SceneNode*, S32> m_baseModels;
	};
}
#endif
