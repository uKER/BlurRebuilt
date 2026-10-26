#include "AftkRenderPch.h"
#include <algorithm>

#include "common/TextureManager.h"
#include "common/TextureState.h"
#include "common/Material.h"

#include "scene/MaterialBuilder.h"

#include "Utility.h"

using namespace AftkRender;

class HashKeyOpMaterial
{
public:
	static U32 GetHash(const C_Material* pMaterial)
	{
		return pMaterial->GetHash();
	}

	static bool Equal(const C_Material *pM1, const C_Material *pM2)
	{
		return (pM1->GetHash() == pM2->GetHash());
	}
};


bool MaterialBuilder::Build(
	const Aftk::C_Scene* pScene, 
	Ut::UtVector< Ut::DynamicVector<C_Material> >& materials,
	Ut::UtVector< Ut::DynamicVector<S32> >& materialMap)
{
	assert(pScene);
	if (pScene->GetNumMaterialNodes() == 0)
		return true;

	return Build(pScene->GetRootSceneNodePtr(), materials, materialMap);
}


bool MaterialBuilder::Build(
	const Aftk::C_SceneNode* pSceneNode,
	Ut::UtVector< Ut::DynamicVector<C_Material> >& materials,
	Ut::UtVector< Ut::DynamicVector<S32> >& materialMap)
{
	assert(pSceneNode);
	const Aftk::C_Scene *pScene = pSceneNode->GetScenePtr();
	if (pScene->GetNumMaterialNodes() == 0)
		return true;

	Ut::BitVector materialsInUse;
	materialsInUse.Resize(pScene->GetNumMaterialNodes());
	materialsInUse.ClearBits();
	materialMap.Resize(pScene->GetNumMaterialNodes());
	materials.Reserve(pScene->GetNumMaterialNodes());
	memset(&materialMap[0], -1, sizeof(U32)*pScene->GetNumMaterialNodes());

	//Check if materials are actually referenced
	BuildMaterialUsageArray(pSceneNode, materialsInUse);

	//build materials that are actually referenced
	//should be safe using C_Material pointers as the materials array has been preallocated
	//Ut::OpenHashSet<C_Material*, HashKeyOpMaterial> materialSet;
	Ut::UtMap<U32, U32> materialHashMap;

	for (S32 i=0; i<materialMap.Size(); ++i)
	{
		if (!materialsInUse[i])
			continue;

		const Aftk::C_MaterialNode *pAftkMaterial = &pScene->GetMaterialResourcePtr(i)->GetContent();
		C_Material material;

		if (!Build(material, pAftkMaterial))
		{
			materials.Clear();
			materialMap.Clear();
			return false;
		}

		//Ut::OpenHashSet<C_Material*, HashKeyOpMaterial>::ConstIterator it = materialSet.Find(&material);
		Ut::UtMap<U32, U32>::Iterator it = materialHashMap.Find(material.GetHash());
		if (it == materialHashMap.End())
		{
			materials.PushBack(material);
			materialMap[i] = materials.Size()-1;
			materialHashMap[material.GetHash()] = materials.Size()-1;//materialSet.Insert(&(materials[materials.Size()-1]));
		}
		else
			materialMap[i] = it->GetValue();
	}

	return true;

}


void MaterialBuilder::BuildMaterialUsageArray(
	const Aftk::C_SceneNode *pNode,
	Ut::BitVector& indexedMaterialInUse)
{
	if (pNode->HasType(Aftk::E_ObjType_SceneMeshRemappable))
	{
		const Aftk::C_SceneMeshRemappableNode *pRemappableNode = (Aftk::C_SceneMeshRemappableNode*) pNode;

		for (U16 j=0; j<pRemappableNode->GetNumPolygonMaterialMappings(); ++j)
		{
			const Aftk::C_MaterialResource *pMaterialResource = pRemappableNode->GetPolygonMaterialMappingPtr(j);
			if (pMaterialResource)
				indexedMaterialInUse[pMaterialResource->GetIndex()] = true;
		}
	}

	//recurse into child nodes
	const Aftk::C_SceneNode *pChildNode = pNode->GetChildPtr(0);
	while (pChildNode)
	{
		BuildMaterialUsageArray(pChildNode, indexedMaterialInUse);
		pChildNode = pChildNode->GetNextPtr();
	}
}


bool MaterialBuilder::Build(C_Material& material, const Aftk::C_MaterialNode* pAftkMaterial)
{

	Ut::UtVector< Ut::DynamicVector<C_Material::TextureInfo> >& textureInfoArray = material.GetTextureInfoArray();
	textureInfoArray.Clear();
	textureInfoArray.Reserve(pAftkMaterial->GetNumShaderImageNodes());

	//FIXME: need some way of communicating shader requirements
	// eg diffuse s0, normalmap s1, etc...

	for (U32 iImage = 0; iImage < pAftkMaterial->GetNumShaderImageNodes(); iImage++)
	{
		std::string fileName;
		const Aftk::C_ShaderImageNode* pImage = pAftkMaterial->GetShaderImageNodePtr(iImage);

		pImage->GetFilePlug().GetValue(fileName);
		std::transform(fileName.begin(), fileName.end(), fileName.begin(), tolower);

		if (fileName.length())
		{
			C_Material::TextureInfo info;
			info.m_FileName = fileName.c_str();
			info.m_Type = C_Material::E_TextureType_2D;

			material.AddTextureInfo(info);
		}
	}

	S32 shaderIdOverride = 0;
	for (U32 i=0; i<pAftkMaterial->GetNumShaderNodes(); ++i)
	{
		const Aftk::C_ShaderNode *pShader = pAftkMaterial->GetShaderNodePtr(i);
		std::string shader;
		const Aftk::I_Plug *pPlug = pShader->GetPlugPtr("shader");
		if (pPlug && pPlug->GetValue(shader))
		{
			if (shader == "SpeedTreeLeaf.fx")
			{
				material.SetCullMode(Display::E_CullMode_None);
				material.SetBlendMode(Display::E_BlendMode_SolidAlphaTest);
				shaderIdOverride = CGFX_SPEEDTREE_LEAF;
			}
			else if (shader.find("SpeedTree") != std::string::npos)
			{
				material.SetCullMode(Display::E_CullMode_None);
				material.SetBlendMode(Display::E_BlendMode_SolidAlphaTest);
			}		
		}
	}

	material.SetMaterialTypeId(shaderIdOverride ? shaderIdOverride : pAftkMaterial->GetMaterialTypeId());

	return true;

}