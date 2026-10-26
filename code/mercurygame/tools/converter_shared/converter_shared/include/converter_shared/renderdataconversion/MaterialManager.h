#ifndef __MATERIALMANAGER_H__
#define __MATERIALMANAGER_H__

#include "converter_shared/renderdataconversion/common.h"

namespace RenderDataConversion
{

class EffectManager;
class Effect;
class AnnotationReaderList;

enum E_StandardMaterial
{
	E_StandardMaterial_WorldMatrix = 0,

	E_StandardMaterial_Count
};

struct MtlKey
{
	const Aftk::C_MaterialNode *pNode;	// material to convert
	E_NodeType type;					// material type
	const Aftk::C_SceneMeshNode *pMesh;	// node that the material belongs to (this influences the effect permutation type)
	int iCustomKey;						// custom key - currently used for skinning palette index
};

// contains the converted material data
struct Material
{
	Material() : iMaterialID(-1), sortType(RenderingData::SORT_TYPE_BACK_FRONT), pEffect(NULL), decalOrder(0), iCustomFlags(0) {}

	int iMaterialID;
	Btl::Vector<int> materialIDs; // one material ID per stream
	RenderingData::SortType	sortType;
	RenderingData::BlendState blendState;
	Effect *pEffect;
	U8 decalOrder;
	U32 iCustomFlags;
	Btl::Vector<RenderingData::PrimitiveInfo::TextureInfo> textureInfo;
};

/// manage the creation of RenderingData materials from Aftk materials
class MaterialManager
{
friend class AftkMaterialConverter;
friend class EffectPermutationSelector;

public:
	/// constructor
	MaterialManager(RenderingData::Build::Scene *pSceneExport, TextureExport::TextureManager *pTextureManager, const Util::C_String &sFXBPath);
	/// destructor
	~MaterialManager();

	/// creates a RenderingData::Material from an Aftk Material
	/// returns the GUID for the given Aftk material
	void GetConvertedAftkMaterial(const Aftk::C_SceneMeshNode *pMesh, const Aftk::C_MaterialNode *pMtl, Material *pOutMtl, int iCustomKey, E_NodeType type);

	/// create a RenderingData::Material that resets any parameters set on an instance - returns the RenderingData material index
	int GetInstanceResetMaterialIdx(const Aftk::C_SceneNode *pNode);

	/// create a material given an aftk converter class and a list of annotation readers
	int GetMaterialIndex(AftkMaterialConverter *pMtl, AnnotationReaderList *pList, const char *pszName);

	/// generate a new GUID
	int GetGUID();

	/// returns the index of an empty material
	int CreateEmptyMaterial();

protected:
	// map Aftk Material ID to RenderingData Material ID
	// TODO: some polyflags will need adding into the key of this map e.g. alpha type polyflag
	typedef Btl::Map<MtlKey,Material> MaterialMap;
	MaterialMap m_Map;
	MtlKey m_LastKey;
	Material m_LastMtl;

	Btl::Vector<S32, Btl::VectorParameters_Array<E_StandardMaterial_Count> >	m_StandardMaterials;

	RenderingData::Build::Scene *m_pSceneExport;
	TextureExport::TextureManager *m_pTextureManager;
	EffectManager *m_pEffectManager;
	int m_iGUID;

	// these variables are used to maintain the instance reset material annotation reader list
	int m_nEffectsLoaded;
	AnnotationReaderList *m_pInstanceReset;
};

}

#endif
