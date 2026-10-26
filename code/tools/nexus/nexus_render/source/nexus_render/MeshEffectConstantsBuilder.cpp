#include <nexus_render/stdafx.h>
#include <nexus_render/MeshEffectConstantsBuilder.h>

using namespace NexusRender;

Tu::Status MeshEffectConstantsBuilder::Build(
	EffectConstants** ppEffectConstants, 
	const Aftk::C_MaterialNode* pAftkMaterial)
{
	Tu::Status status;
	*ppEffectConstants = 0;

	const Aftk::C_ShaderImageNode* pColourImage = 0;
	const Aftk::C_ShaderImageNode* pLightmapImage = 0;

	std::string imageType;
	for (U32 iImage = 0; iImage < pAftkMaterial->GetNumShaderImageNodes(); iImage++)
	{
		const Aftk::C_ShaderImageNode* pImage = pAftkMaterial->GetShaderImageNodePtr(iImage);
		pImage->GetImageTypePlug().GetValue(imageType);
		if (!strcmp(imageType.c_str(),"lightmap"))
			pLightmapImage = pImage;
		else if (!pColourImage)
			pColourImage = pImage;
	}

	if (pLightmapImage)
	{
		//Build lightmap effect constants.
		MeshLightmapEffectConstants* pMeshEffectConstants = new MeshLightmapEffectConstants();
		status = pMeshEffectConstants->Build(pAftkMaterial);
		if (!status) return status;
		*ppEffectConstants = pMeshEffectConstants;
	}
	else if (pColourImage)
	{
		//Build textured effect constants.
		MeshTextureEffectConstants* pMeshEffectConstants = new MeshTextureEffectConstants();
		status = pMeshEffectConstants->Build(pAftkMaterial);
		if (!status) return status;
		*ppEffectConstants = pMeshEffectConstants;
	}
	else
	{
		//Build basic effect constants (untextured).
		MeshBasicEffectConstants* pMeshEffectConstants = new MeshBasicEffectConstants();
		status = pMeshEffectConstants->Build(pAftkMaterial);
		if (!status) return status;
		*ppEffectConstants = pMeshEffectConstants;
	}

	return Tu::Status::SUCCESS;
}