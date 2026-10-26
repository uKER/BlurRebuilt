
#include "stdafx.h"

#include <renderingdata/renderingdata.h>
#include <world_render/WorldRender.h>

//#pragma optimize("", off)

namespace WorldRender
{

Debug::Logger &g_Log = Debug::GetLogger("mercury.graphics.world_render");

// Reads data written by C_Converter::ExportBinary().
void ReadScene( Sys::I_File* pFile, RenderingData::Target::Scene* pScene,  bool* pLoadedResources)
{
	Arch::BinaryInputArchive ar(*pFile);
	
	S32 version = -1;
	ar >> version;

	ar >> *pScene;

	S32 nBlocks;

	if (version == 0)
	{
		bool loadBlocks;
		ar >> loadBlocks;
		nBlocks = loadBlocks ? pScene->GetNumResourceBlocks() : 0;
		if (pLoadedResources) *pLoadedResources = loadBlocks;
	}
	else
	{
		ar >> nBlocks;
		if (pLoadedResources) *pLoadedResources = (nBlocks == pScene->GetNumResourceBlocks());
	}

	for (int i = 0; i < nBlocks; i++)
	{
		ar >> pScene->GetResourceBlock(i);
	}
}

} // namespace WorldRender
