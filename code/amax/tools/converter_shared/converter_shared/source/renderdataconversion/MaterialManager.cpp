#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUt.h>
#include <texture_export/texturemanager.h>
#include <texture_export/textureprocessing.h>
#include <renderingdata/renderingdata.h>
#include "converter_shared\renderdataconversion\MaterialManager.h"
#include "converter_shared\renderdataconversion\EffectManager.h"
#include "converter_shared\renderdataconversion\ConvertAftkMaterial.h"
#include "converter_shared\renderdataconversion\Effect.h"
#include "converter_shared\renderdataconversion\EffectPermutationSelector.h"

namespace RenderDataConversion
{

/// constructor
MaterialManager::MaterialManager(RenderingData::Build::Scene *pSceneExport, TextureExport::TextureManager *pTextureManager, const Util::C_String &sFXBPath)
{
	m_pSceneExport = pSceneExport;
	m_pTextureManager = pTextureManager;
	m_pEffectManager = new EffectManager(pSceneExport, sFXBPath);
	m_iGUID = 0;
}

/// destructor
MaterialManager::~MaterialManager()
{
	delete m_pEffectManager;
}

// for the material map
bool operator <(const MtlKey &a, const MtlKey &b)
{
	if (a.pNode != b.pNode)
		return a.pNode>b.pNode;
	return a.type > b.type;
}

/// creates a RenderingData::Material from an Aftk::Material
/// returns the GUID for the given Aftk material
void MaterialManager::GetConvertedAftkMaterial(const Aftk::C_SceneMeshNode *pMesh, const Aftk::C_MaterialNode *pMtl, Material *pOutMtl, E_NodeType type)
{
	MtlKey key;
	key.pNode = pMtl;
	key.type = type;
	key.pMesh = pMesh;
	MaterialMap::InsertionReturnType r = m_Map.Insert(key);

	if (r.inserted)
	{
		AftkMaterialConverter converter(this, pMtl, type);
		EffectPermutationSelector eps(this, pMesh, pMtl);
		Util::C_String sPermutatedFXName = eps.GetPermutation(converter.GetFXName());
		converter.SetPermutatedFXName(sPermutatedFXName);
		converter.Convert(&r.iterator->GetValue());
	}

	*pOutMtl = r.iterator->GetValue();
}

/// creates some standard materials
void MaterialManager::CreateStandardMaterials()
{
	for(S32 i = 0; i < E_StandardMaterial_Count; ++i)
	{	
		m_StandardMaterials[i] = -1;
	}

	// world matrix material type
	{
		RenderingData::MaterialType materialType;
		materialType.SetName("WorldMatrix");
		materialType.SetProcessType(RenderingData::MaterialType::MATERIAL_PROCESS_TYPE_CONSTANT);
		RenderingData::MaterialElement materialElement;
		materialElement.type = RenderingData::MATERIAL_ELEMENT_TYPE_F32;
		materialElement.count = 16;
		materialElement.name = "g_WorldMatrix";
		materialType.AddElement(materialElement);
		m_StandardMaterials[E_StandardMaterial_WorldMatrix] = m_pSceneExport->GetMaterialList().InsertMaterialType(materialType);
	}
}


S32 MaterialManager::GetStandardMaterial(E_StandardMaterial material) const
{
	return m_StandardMaterials[material];
}

/// generate a new GUID
int MaterialManager::GetGUID()
{
	return m_iGUID++;
}

};
