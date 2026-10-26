#ifndef WORLD_RENDER_WORLD_RENDER_H
#define WORLD_RENDER_WORLD_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace RenderingData
{

	namespace Target
	{
		class Scene;
	}
}


namespace WorldRender
{

extern Debug::Logger &g_Log;

void ReadScene( Sys::I_File* pFile, RenderingData::Target::Scene* pScene, bool* pLoadedResources = 0 );

} // namespace WorldRender

#endif // WORLD_RENDER_WORLD_RENDER_H

