#include "GameDebugPch.h"

#include <game_debug/gameDebug.h>
#include <game_debug/ModelResourceUsageUtils.h>
#include <game_resource/LevelResource.h>
#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>
#include <world_render/RenderModelData.h>
#include <world_render/RenderModelResourceUsage.h>

namespace GameDebug
{

void DumpResourceStats()
{
	LOGINFO( g_Log, "Resource,Diffuse Kb,IPR Kb,Normal Kb,Lightmap Kb,Misc Kb, Total Kb" );
	Resource::C_HandleList handleList;
	Resource::C_Manager::GetInstance()->GetHandleList( handleList, GameResource::C_ModelResource::StaticGetResourceTypeId() );

	ModelResourceUsage::C_TextureTotals	gTextureTotals;
	for(S32 t=0; t<ModelResourceUsage::E_TextureType_Count; t++)
	{
		gTextureTotals[t] = 0;
	}


	const S32 resourceCount = handleList.Size();
	for(S32 i=0; i<resourceCount; i++)
	{
		GameResource::C_ModelResource* pModelResource = (GameResource::C_ModelHandle)handleList[i];
		if(pModelResource)
		{
			WorldRender::RenderModelResourceUsage resourceUsage;
			pModelResource->GetRenderModelData()->AccumulateResourceUsage(resourceUsage);

			ModelResourceUsage::C_TextureTotals	textureTotals;
			for(S32 t=0; t<ModelResourceUsage::E_TextureType_Count; t++)
			{
				textureTotals[t] = 0;
			}

			for (WorldRender::RenderModelTextureInfoList::ConstIterator it = resourceUsage.textureInfoList.Begin(); it != resourceUsage.textureInfoList.End(); ++it)
			{
				const WorldRender::RenderModelTextureInfo& info = it->GetValue();
				ModelResourceUsage::AccumulateTotals(textureTotals, info.textureName, info.uSizeInBytes, info.eFormat);
				ModelResourceUsage::AccumulateTotals(gTextureTotals, info.textureName, info.uSizeInBytes, info.eFormat);
			}


			LOGINFO( g_Log, "%s, %d, %d,%d, %d, %d, %d", pModelResource->GetResourceFilename().Chars(), 
				textureTotals[ModelResourceUsage::E_TextureType_Diffuse] / 1024, 
				textureTotals[ModelResourceUsage::E_TextureType_IPR] / 1024, 
				textureTotals[ModelResourceUsage::E_TextureType_Normal] / 1024, 
				textureTotals[ModelResourceUsage::E_TextureType_Lightmap] / 1024, 
				textureTotals[ModelResourceUsage::E_TextureType_Misc] / 1024,
				( textureTotals[ModelResourceUsage::E_TextureType_Diffuse] + textureTotals[ModelResourceUsage::E_TextureType_IPR] + textureTotals[ModelResourceUsage::E_TextureType_Normal] + textureTotals[ModelResourceUsage::E_TextureType_Lightmap] + textureTotals[ModelResourceUsage::E_TextureType_Misc] ) / 1024 );
		}
	}

	LOGINFO( g_Log, "Total, %d, %d, %d, %d, %d, %d", 
		gTextureTotals[ModelResourceUsage::E_TextureType_Diffuse] / 1024, 
		gTextureTotals[ModelResourceUsage::E_TextureType_IPR] / 1024, 
		gTextureTotals[ModelResourceUsage::E_TextureType_Normal] / 1024, 
		gTextureTotals[ModelResourceUsage::E_TextureType_Lightmap] / 1024, 
		gTextureTotals[ModelResourceUsage::E_TextureType_Misc] / 1024,
		( gTextureTotals[ModelResourceUsage::E_TextureType_Diffuse] + gTextureTotals[ModelResourceUsage::E_TextureType_IPR] + gTextureTotals[ModelResourceUsage::E_TextureType_Normal] + gTextureTotals[ModelResourceUsage::E_TextureType_Lightmap] + gTextureTotals[ModelResourceUsage::E_TextureType_Misc] ) / 1024 );

}

} // namespace GameDebug
