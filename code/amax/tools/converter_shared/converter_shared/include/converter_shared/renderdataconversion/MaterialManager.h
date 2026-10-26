#ifndef __MATERIALMANAGER_H__
#define __MATERIALMANAGER_H__

#include "converter_shared/renderdataconversion/common.h"

namespace RenderDataConversion
{

class EffectManager;
class Effect;

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
};

// contains the converted material data
struct Material
{
	int iMaterialID;
	RenderingData::SortType	sortType;
	RenderingData::BlendState blendState;
	Effect *pEffect;
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

	/// creates some standard material types
	void CreateStandardMaterials();

	/// creates a RenderingData::Material from an Aftk Material
	/// returns the GUID for the given Aftk material
	void GetConvertedAftkMaterial(const Aftk::C_SceneMeshNode *pMesh, const Aftk::C_MaterialNode *pMtl, Material *pOutMtl, E_NodeType type);

	S32 GetStandardMaterial(E_StandardMaterial material) const;

	/// generate a new GUID
	int GetGUID();

protected:
	// map Aftk Material ID to RenderingData Material ID
	// TODO: some polyflags will need adding into the key of this map e.g. alpha type polyflag
	typedef Btl::Map<MtlKey,Material> MaterialMap;
	MaterialMap m_Map;

	Btl::Vector<S32, Btl::VectorParameters_Array<E_StandardMaterial_Count> >	m_StandardMaterials;

	RenderingData::Build::Scene *m_pSceneExport;
	TextureExport::TextureManager *m_pTextureManager;
	EffectManager *m_pEffectManager;
	int m_iGUID;
};

}

#endif
