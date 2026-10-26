#include <stdafx.h>

#include <vis_core/ProcessTextureTags.h>
#include <vis_core/TextureTagDefinitions.h>

#include <file_tag_db/FileTagDB.h>
#include <attk/Attk.h>

#include <algorithm>

bool C_TextureTags::Process(const Aftk::C_Scene& scene)
{
	//Aquire a list of the unique texture stored within our scene.
	Ut::UtMap<Util::C_String, bool> textureSet;
	if (!BuildTextureList(textureSet, scene))
		return false;

	return SetTextureTags(textureSet);
}

bool C_TextureTags::IsComplexMaterial(const Aftk::C_ShaderNode& shaderNode, bool& forceAlpha)
{
	forceAlpha = false;
	for (U32 i = 0; i < shaderNode.GetNumPlugs(); i++)
	{
		//If we're using a CgFX shader then we cannot simply look at the alpha channel for 
		//the first texture on the material to determine whether we're transparent or not.
		if (!_stricmp(shaderNode.GetPlugPtr(i)->GetName().c_str(), "shader"))
		{
			std::string str;
			if (shaderNode.GetPlugPtr(i)->GetValue(str))
			{
				std::transform(str.begin(), str.end(), str.begin(), tolower);

				if (str.find("puddle") != std::string::npos)
				{
					forceAlpha = true;
					return false;
				}
			}

			return true;
		}
	}
	
	return false;
}

bool C_TextureTags::IsComplexMaterial(const Aftk::C_MaterialNode& mtl, bool& forceAlpha)
{
	for (U32 i = 0 ; i < mtl.GetNumShaderNodes(); i++)
	{
		const Aftk::C_ShaderNode* pShaderNode = mtl.GetShaderNodePtr(i);
		if (!pShaderNode)
			continue;

		if (IsComplexMaterial(*pShaderNode, forceAlpha))
			return true;

		//Return the minute the alpha-hack is activated.
		if (forceAlpha)
			return false;
	}
	return false;
}

//Here we're going to initialise per-texture flags such as 'transparent to pvs'.
bool C_TextureTags::BuildTextureList(
	Ut::UtMap<Util::C_String, bool>& textureSet,
	const Aftk::C_Scene& scene)
{
	//Loop thru the entire scene.
	for (Aftk::C_SceneNodeConstIterator it(&scene); it; ++it)
	{
		Aftk::C_SceneNode* pSceneNode = it;
		if (pSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappable))
		{
			Aftk::C_SceneMeshRemappableNode* pMesh = 
				(Aftk::C_SceneMeshRemappableNode*)pSceneNode;

			//Loop thru all of the materials on our mesh.
			for (U16 iMaterial = 0; iMaterial < pMesh->GetNumPolygonMaterialMappings(); iMaterial++)
			{
				bool forceAlpha = false;
				const Aftk::C_MaterialResource* pRes = pMesh->GetPolygonMaterialMappingPtr(iMaterial);
				if (!pRes)
					continue;

				const Aftk::C_MaterialNode& mtl = pRes->GetContent();

				//Opt out for opaque PGR4 materials.
				if (IsComplexMaterial(mtl, forceAlpha))
				{
					continue;
				}

				//Loop thru all of the images on our material.
				for (U32 jImage = 0; jImage < mtl.GetNumShaderImageNodes(); jImage++)
				{
					const Aftk::C_ShaderImageNode* pImage = mtl.GetShaderImageNodePtr(jImage);
					if (!pImage)
						continue;

					//Get the filename of our texture.
					std::string tmp;
					if (!pImage->GetFilePlug().GetValue(tmp))
						continue;

					if (tmp.length() == 0)
						continue;

					//Attempt to add to our unique list of textures as lower case.
					textureSet[Util::C_String(tmp.c_str()).Lower()] = forceAlpha;
				}
			}
		}
	}

	return true;
}

bool C_TextureTags::SetTextureTags(const Ut::UtMap<Util::C_String, bool>& textureSet)
{
	const F32 alphaEpsilon = 1e-3f;
	for (Ut::UtMap<Util::C_String, bool>::ConstIterator it = textureSet.Begin(); it != textureSet.End(); ++it)
	{
		const Util::C_String& filePath = it->GetKey();
		Attk::C_Image image;
		if (!image.Read(filePath))
			continue;

		//We don't concern ourselves with volume textures here.
		if (image.GetDepth() > 1)
			continue;

		bool hasAlpha = it->GetValue();
		if (!hasAlpha)
		{
			if (image.GetChannelsPerPixel() > 3)
			{
				for (U32 x = 0; x < image.GetWidth(); x++)
					for (U32 y = 0; y < image.GetHeight(); y++)
					{
						//We may have alpha here!
						Attk::C_PixelR32G32B32A32F pixel = image.GetPixel(x, y);
						if (fabsf(pixel.alpha-1.f) > alphaEpsilon)
						{
							hasAlpha = true;
							goto breakLoop;
						}
					}
			}
		}

breakLoop:

		FileTag::C_Tag* pTag = 
			FileTag::C_TagDBManager::Instance().GetTagPtr(filePath);

		//Add a new tag if none exists.
		if (!pTag)
			pTag = FileTag::C_TagDBManager::Instance().AddTag(filePath);
		assert(pTag);

		//Add "Transparent for PVS" flag.
		FileTag::C_GenericAttribute* pAttr = 
			pTag->GetAttributePtr(TRANSPARENT_FOR_PVS);
		//Add a new flag if none exists.
		if (!pAttr)
		{
			pTag->GetAttributes().PushBack();
			pAttr = &pTag->GetAttributes().Back();
		}

		pAttr->GetName() = TRANSPARENT_FOR_PVS;
		pAttr->SetValue(hasAlpha);
	}

	return true;
}
