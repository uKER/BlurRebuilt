#ifndef RENDERDATACONVERTER_H
#define RENDERDATACONVERTER_H

#include "btl/containers/vector.h"
#include "btl/containers/map.h"
#include "btl/utility/string.h"
#include "renderdataconversion/clustering.h"
#include "renderdataconversion/common.h"
#include "renderingdata/renderingdata.h"
#include "renderdataconversion/AnnotationReaderList.h"

namespace Pvs
{
	namespace Build
	{
		class PvsData;
	}
}

namespace Portal
{
	class C_Builder;
}

namespace TextureExport
{
	class TextureManager;
}
namespace RenderDataConversion
{
	class MaterialManager;
	class I_PolyInform;
	class I_NodeInform;
	class I_CubemapInform;
	class I_TextureTagsReader;
	class Effect;
	struct Material;
	class AftkMeshConverter;
}
namespace SkinningProcessor
{
	class I_SkinningProcessor;
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
		BaseInstanceData() : iBaseModelIdx(-1), pEffect(0), pMesh(0), process(0) {}

		int iBaseModelIdx;
		RenderDataConversion::Effect *pEffect;
		const Aftk::C_SceneMeshNode *pMesh;
		RenderDataConversion::AnnotationReaderList instanceMtl;
		S32 process;

		Btl::Vector<S32> m_iBatchIDs;
	};

	/// renderingdata converter class
	class C_Converter
	{
	public:
		/// constructor
		C_Converter(const Util::C_String sTextureCachePath, const Util::C_String sFXBPath, bool bDeleteOldTextureCache = true);

		/// destructor
		~C_Converter();

		/// begin the building of a new model
		void BeginModel();
		
		/// add an element to the model that is currently being built
		void AddElement( const Maths::HMat43 &worldMtx, const C_AftkSceneNodeList& aftkSceneNodeList, const Aftk::C_SceneNode* pRootNode, bool bSharedTextures);

		/// finish building the current model
		void EndModel();

		// hint to the converter on how it should arrange the data
		void SetStreamed(bool b);

		/// enable (or disable) skinning on this converter
		void EnableSkinning(bool bEnable, U32 maxMatrices, U32 maxVertexBlend, const char *pszVariation,
			const bool rotateY180 = false);

		/// add a skeleton to the skinning processor for explicit mapping on joints to matrix palette indexes
		void AddSkinningSkeletonNode(const Aftk::C_SceneNode* pSceneNode);

		void SetSceneName(const char* name);

		/// add list of Aftk nodes that are to be built into a single static mesh
		void AddStaticNodes( const C_AftkSceneNodeList& aftkSceneNodeList );

		/// this is called once to build the rendering data
		void Process();

		/// set a callback class which passes polyflag information to the renderdata builder
		void SetPolyInform( const RenderDataConversion::I_PolyInform *p );

		/// set a callback class which passes node information to the renderdata builder
		void SetNodeInform( const RenderDataConversion::I_NodeInform *p );

		/// set a callback class which can be used to query what cubemap texture filename is associated with a particular polygon
		void SetCubemapInform( const RenderDataConversion::I_CubemapInform *p );

		/// set LOD fade distance (a number from 0 to 1). default is 0.2, 0 disables LOD fading (only works with models)
		void SetLODFadeDistance( float f );

		// generate a test version of scene by hand
		void TestProcess();

		/// set the name of the effect to use when an invalid effect was specified
		static void SetInvalidEffect(const char *psz);

		/// set the name of the effect to use when no specific effect was specified (for example in the HLSL shader)
		static void SetDefaultEffect(const char *psz);
		
		/// set the variation to use for all shaders. this variation will be appended to the name of each .fx file
		static void SetEffectVariation(const char *psz);

		// set PVS data
		void SetPVSData(const Pvs::Build::PvsData* pvsData);

		/// set the portal data.
		void SetPortalData(Portal::C_Builder* pPortalData);

		/// set the filename of the texture that will be used if the real texture can't be found on disk
		void SetMissingTexture(const char *pszFilename);

		/// write out the rendering data for a particular platform (call Process first)
		bool ExportBinary(Sys::I_File* pBinaryStream, Sys::I_File* pStreamFile, RenderingData::PlatformType platformType);
		bool ExportXml(Sys::I_File* pBinaryStream, RenderingData::PlatformType platformType);

		/// function for external code to query the material on a polygon (used by PVS tool to detect alpha/double sided)
		void GetMaterial(const Aftk::C_Polygon& aftkPolygon, RenderDataConversion::Material* convertedMtl);

		// tell the scene how it should generate lods
		void SetLodGenerationOptions(S32 lodsToGenerate, float reductionFactor, S32 minVertices, bool reduceVertices);

	private:
		void BuildStreamData(RenderingData::Build::Scene& scene, RenderingData::PlatformType platformType);
		const RenderingData::Build::Scene& GetExportScene() const { return *m_pSceneExport; }

		/// construct a RenderingData::PrimitiveInfo from a given Aftk polygon
		bool GetPrimitiveInfo(const RenderDataConversion::AftkMeshConverter *pMesh, int iPoly, RenderDataConversion::E_NodeType nodeType, RenderingData::PrimitiveInfo *pPrimitiveInfo, RenderDataConversion::Material *pConvertedMtl);

		/// returns a material index for the material containing element data for the nth element
		int GenerateElementMaterial(int iElement);

		/// create a new cullnode
		RenderingData::ResourceIndex C_Converter::AddCullNode(const Btl::String &name, RenderingData::Node& parent, S32 blockIndex, S32 groupIndex);

		/// returns true if we're exporting a model, false if we're exporting a scene
		bool ExportingModel();

		/// copy the UV/texture info into the texture data
		void SetTextureInfo(RenderingData::Build::Scene &sceneExport, RenderDataConversion::AftkMeshConverter &mesh, RenderingData::PrimitiveInfo &primitiveInfo, int poly, const Aftk::C_SceneMeshNode *pNode);

		void GetInstanceBounds(const Aftk::C_SceneNode *pNode, const BaseInstanceData& data, RenderingData::Bounds& bounds);

		BatchIDGenerator *m_pBatchIDGenerator;

		RenderingData::Build::Scene* m_pSceneExport;

		RenderingData::ResourceIndex m_iCurrentNode;

		/// this is called once to build the rendering data
		void Process(const C_AftkSceneNodeList &nodes);

		/// perform clustering on the Aftk mesh
		void Process1(const Aftk::C_SceneMeshNode* pMeshNode, bool bDummyRun);
		void Process1(const Aftk::C_SceneNode* pInstanceNode, bool bDummyRun);

		/// build the actual renderdata
		void Process2(const Aftk::C_SceneMeshNode* pNode, bool addInstance = true);
		void Process2(const Aftk::C_SceneNode* pInstanceNode);

		void GetClusteringData(const Aftk::C_SceneMeshNode& node, S32 polygonIndex, S32 baseModelIndex, RenderDataConversion::Clustering::Polygon* clusteringPolygon, bool lodFade);
		void GetMaterial(const Aftk::C_Polygon& aftkPolygon, RenderDataConversion::E_NodeType nodeType, RenderDataConversion::Material* convertedMtl, int iCustomKey);
		void GetClusteringData(const Aftk::C_SceneNode& node, const BaseInstanceData& baseInstanceData, RenderDataConversion::Clustering::Instance* clusteringInstance);

		void CreateInstance(const Aftk::C_SceneNode *pNode, BaseInstanceData &data);
		void CreateModelNodes();
		void CreateClusterNode(RenderingData::ResourceIndex parent, const RenderDataConversion::Clustering::SceneNode& node);

		S32 InsertBaseModel(const Aftk::C_SceneNode* pNode);

		// test functions
		S32							CreateTestEffect(const Btl::String& effectName);
		S32							CreateTestMaterial(const Btl::String& materialName, S32 effectIndex);

		struct ModelData
		{
			ModelData() : modelIndex(0), elementIndex(-1), elementMaterialIndex(-1), elementRootNode(NULL) {}
			S32							modelIndex;
			S32							elementIndex;
			S32							elementMaterialIndex;
			Maths::HMat43				elementMatrix;
			const Aftk::C_SceneNode*	elementRootNode;
		};

		ModelData					m_ModelData;

		//int							m_iCurrentInstanceId;

		C_AftkSceneNodeList			m_AftkSceneNodes;

		typedef Btl::Map<const Aftk::C_SceneNode*, BaseInstanceData> BaseInstanceMap;
		BaseInstanceMap m_BaseInstanceMap;

		int m_iBlockIndexOffset;
		int m_iSkinningMaxMatrices;
		int m_iSkinningMaxVertexBlend;
		bool m_bSkinningRotateY180;
		bool m_bSkinning;
		float m_fLODFadeDistance;
		float m_fInvLODFadeDistanceSquared;
		bool m_streamed;

		TextureExport::TextureManager* m_pTextureManager;
		RenderDataConversion::MaterialManager* m_pMtlManager;
		const RenderDataConversion::I_PolyInform* m_pPolyInform;
		const RenderDataConversion::I_NodeInform* m_pNodeInform;
		const RenderDataConversion::I_CubemapInform* m_pCubemapInform;
		RenderDataConversion::Clustering::I_Scene* m_pClusteringScene;
		SkinningProcessor::I_SkinningProcessor* m_pSkinningProcessor;
		Btl::Map<S32, RenderingData::Build::Model*> m_modelPointers;
		Btl::Map<S32, RenderingData::RenderNode*> m_instancePointers;
		const Pvs::Build::PvsData* m_pvsData;
		Portal::C_Builder* m_pPortalBuilder;
		Btl::Vector<const Aftk::C_SceneNode*> m_SkeletonNodes;
	};
}
#endif
