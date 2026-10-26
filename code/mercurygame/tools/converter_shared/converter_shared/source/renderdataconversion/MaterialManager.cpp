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
#include "converter_shared\helpers\AftkHelpers.h"

namespace RenderDataConversion
{

/// constructor
MaterialManager::MaterialManager(RenderingData::Build::Scene *pSceneExport, TextureExport::TextureManager *pTextureManager, const Util::C_String &sFXBPath)
{
	m_pSceneExport = pSceneExport;
	m_pTextureManager = pTextureManager;
	m_pEffectManager = new EffectManager(pSceneExport, sFXBPath);
	m_iGUID = 0;
	m_nEffectsLoaded= 0;
	m_pInstanceReset = new AnnotationReaderList;
	m_LastKey.type = (E_NodeType)-1;
}

/// destructor
MaterialManager::~MaterialManager()
{
	delete m_pEffectManager;
	delete m_pInstanceReset;
}

// for the material map
bool operator <(const MtlKey &a, const MtlKey &b)
{
	if (a.pNode != b.pNode)
		return a.pNode>b.pNode;
	if (a.pMesh != b.pMesh)
		return (int)a.pMesh>(int)b.pMesh;
	if (a.iCustomKey != b.iCustomKey)
		return a.iCustomKey > b.iCustomKey;
	return a.type > b.type;
}

/// creates a RenderingData::Material from an Aftk::Material
/// returns the GUID for the given Aftk material
void MaterialManager::GetConvertedAftkMaterial(const Aftk::C_SceneMeshNode *pMesh, const Aftk::C_MaterialNode *pMtl, Material *pOutMtl, int iCustomKey, E_NodeType type)
{
	if (m_LastKey.pNode == pMtl &&
		m_LastKey.type == type &&
		m_LastKey.pMesh == pMesh &&
		m_LastKey.iCustomKey == iCustomKey)
	{
		*pOutMtl = m_LastMtl;
		return;
	}

	MtlKey key;
	key.pNode = pMtl;
	key.type = type;
	key.pMesh = pMesh;
	key.iCustomKey = iCustomKey;
	MaterialMap::InsertionReturnType r = m_Map.Insert(key);

	if (r.inserted)
	{
		AftkMaterialConverter converter(this, pMtl, pMesh, iCustomKey, type);
		EffectPermutationSelector eps(this, pMesh, pMtl);
		Util::C_String sPermutatedFXName = eps.GetPermutation(converter.GetFXName());
		converter.SetPermutatedFXName(sPermutatedFXName);
		converter.Convert(&r.iterator->GetValue(), pMesh, true);
	}

	*pOutMtl = r.iterator->GetValue();
	m_LastKey = key;
	m_LastMtl = r.iterator->GetValue();
}

/// returns the index of an empty material
int MaterialManager::CreateEmptyMaterial()
{
	RenderingData::MaterialType materialType;

	// create material type
	int iMaterialType = m_pSceneExport->GetMaterialList().InsertMaterialType(materialType);

	// create a new material from the material type
	RenderingData::Material material;
	m_pSceneExport->GetMaterialList().CreateMaterial(material, iMaterialType);

	return m_pSceneExport->GetMaterialList().InsertMaterial(material);
}

/// create a material given an aftk converter class and a list of annotation readers
int MaterialManager::GetMaterialIndex(AftkMaterialConverter *pMtl, AnnotationReaderList *pList, const char *pszName)
{
	// get material data for list of annotation readers
	RenderingData::MaterialType materialType;
	materialType.SetName(pszName);

	U8 buffer[1024*16]; // TODO: FIXME: make this a dynamically resizable buffer instead of 16K fixed size
	U8 *pData = buffer;
	int iSize = 0;

	pList->GetMaterialData(pMtl, &materialType, &iSize, &pData, pMtl->GetNodeType());

#if 1
	// TODO: [rthomas]	Think of a better way to do this!
	//   (Jan) could this be moved into a plugin?
	if (pMtl && pMtl->GetSceneNode() && pMtl->GetNodeType() == E_NodeType_Instance)
	{
		bool status = false;
		AftkUt::C_NodeFlagParser nodeParser(*pMtl->GetSceneNode(), &status);

		if (status)
		{
			S32 paletteIndex = -1;
			S32 paletteType = -1;

			if (nodeParser.GetParameterValue("PaletteIndex", paletteIndex) && paletteIndex > -1)
			{
				nodeParser.GetParameterValue("PaletteSortId", paletteType);

				const Aftk::C_SceneNode* pSourceNode = AftkHelpers::InstanceGetSourceNode(pMtl->GetSceneNode());

				if (pSourceNode && pSourceNode->HasType(Aftk::E_ObjType_SceneMesh) && static_cast<const Aftk::C_SceneMeshNode*>(pSourceNode)->IsSkinned())
				{
					{
						RenderingData::Build::MaterialList *pMaterialList = &pMtl->GetSceneExport()->GetMaterialList();

						RenderingData::MaterialElement materialElement;
						U8 **ppData = &pData;

						materialElement.type = RenderingData::MATERIAL_ELEMENT_TYPE_DATA_INDEX;
						materialElement.count =	1;
						materialElement.name = "PaletteData";

						{
							RenderingData::MaterialElement dataMaterialElement;

							dataMaterialElement.type = RenderingData::MATERIAL_ELEMENT_TYPE_S16;
							dataMaterialElement.count = 2;
							dataMaterialElement.name = "PaletteData";

							RenderingData::MaterialType dataMaterialType;
							dataMaterialType.SetName("PaletteData");
							dataMaterialType.AddElement(dataMaterialElement);

							S32 iDataMaterialType = pMaterialList->InsertMaterialType(dataMaterialType);

							RenderingData::Material dataMaterial;
							pMaterialList->CreateMaterial(dataMaterial, iDataMaterialType);

							dataMaterial.SetData(0, static_cast<S16>(paletteIndex));
							dataMaterial.SetData(sizeof(S16), static_cast<S16>(paletteType));

							// store the index of this new material in the static material output
							*((int*)*ppData) =  pMaterialList->InsertMaterial(dataMaterial);
							(*ppData) += 4;
						}

						materialType.AddElement(materialElement);
						iSize += RenderingData::materialDataTypeSizes[materialElement.type] * materialElement.count;
					}
				}
			}
		}
	}
#endif

	// create material type
	int iMaterialType = m_pSceneExport->GetMaterialList().InsertMaterialType(materialType);

	// create a new material from the material type
	RenderingData::Material material;
	m_pSceneExport->GetMaterialList().CreateMaterial(material, iMaterialType);

	// set data
	material.SetData(buffer, iSize);

	return m_pSceneExport->GetMaterialList().InsertMaterial(material);
}


/// create a RenderingData::Material that resets any parameters set on an instance - returns the RenderingData material index
int MaterialManager::GetInstanceResetMaterialIdx(const Aftk::C_SceneNode *pNode)
{
	Btl::Map<Util::C_String,Effect*> *pEffectMap = m_pEffectManager->GetEffectMap();

	// update instance reset material annotation readers if necessary
	if (pEffectMap->Size() > m_nEffectsLoaded)
	{
		int iAlreadyChecked = m_nEffectsLoaded;
		m_nEffectsLoaded = pEffectMap->Size();

		for (Btl::Map<Util::C_String,Effect*>::Iterator iter=pEffectMap->Begin(); iter!=pEffectMap->End(); ++iter)
		{
			if (iAlreadyChecked>0)
				iAlreadyChecked--;
			else
				m_pInstanceReset->AddAnnotationReadersFromEffect(iter->GetValue(), E_NodeType_InstanceReset);
		}
	}

	AftkMaterialConverter converter(this, pNode, true);
	return GetMaterialIndex(&converter, m_pInstanceReset, "InstanceReset");
}

/// generate a new GUID
int MaterialManager::GetGUID()
{
	return m_iGUID++;
}

};
