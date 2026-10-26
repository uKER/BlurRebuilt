#include "FrameworkPch.h"
#include "Framework/Debug/MemoryDumpers.h"

namespace Framework
{

#if !CFG_GOLD

#if PLATFORM_PS3
Debug::Bool		g_debugShowHeaps					("Graphics.Display2.Dumps.ShowHeaps",				false);
Debug::Bool		g_debugDumpHeaps					("Graphics.Display2.Dumps.DumpHeaps",				false);
#endif

#if TRACK_DISPLAY2_RESOURCES
Debug::Bool 	g_debugDumpVertexBuffers			("Graphics.Display2.Dumps.DumpVertexBuffers",		false);
Debug::Bool 	g_debugDumpIndexBuffers				("Graphics.Display2.Dumps.DumpIndexBuffers",		false);
Debug::Bool 	g_debugDumpVertexShaders			("Graphics.Display2.Dumps.DumpVertexShaders",		false);
Debug::Bool 	g_debugDumpPixelShaders				("Graphics.Display2.Dumps.DumpPixelShaders",		false);
Debug::Bool 	g_debugDumpSurfaces					("Graphics.Display2.Dumps.DumpSurfaces",			false);
Debug::Bool 	g_debugDumpTextures					("Graphics.Display2.Dumps.DumpTextures",			false);
Debug::Bool 	g_debugDumpRenderTexture2Ds			("Graphics.Display2.Dumps.DumpRenderTexture2Ds",	false);
Debug::Bool 	g_debugDumpRenderTextureCubes		("Graphics.Display2.Dumps.DumpRenderTextureCubes",	false);
Debug::Bool 	g_debugDumpRenderBuffers			("Graphics.Display2.Dumps.DumpRenderBuffers",		false);
Debug::Int		g_debugDumpStart					("Graphics.Display2.Dumps.DumpStart",				0, Debug::Range(0, 999));
Debug::Int		g_debugDumpCount					("Graphics.Display2.Dumps.DumpCount",				-1, Debug::Range(-1, 999));
#endif

#if PLATFORM_PS3
Debug::Bool 	g_debugDumpTileManager				("Graphics.Display2.Dumps.DumpTileManager",			false);
Debug::Bool 	g_debugDumpZcullManager				("Graphics.Display2.Dumps.DumpZcullManager",		false);
#endif

#if TRACK_TILED_MEMORY
Debug::Bool		g_debugDumpTiledMemory				("Graphics.Display2.Dumps.DumpTiledMemory",			false);
#endif

#endif

void DumpGraphicsHeaps()
{
#if !CFG_GOLD
	#if PLATFORM_PS3
	if(g_debugShowHeaps)
	{
		Display2::Device()->ShowMemoryHeaps();
		g_debugShowHeaps = false;
	}

	if(g_debugDumpHeaps)
	{
		Display2::Device()->DumpMemoryHeaps();
		g_debugDumpHeaps = false;
	}
	#endif
	
	#if TRACK_DISPLAY2_RESOURCES
	if(g_debugDumpVertexBuffers)
	{
		Display2::Device()->DumpVertexBuffers(g_debugDumpStart, g_debugDumpCount);
		g_debugDumpVertexBuffers = false;
	}
	
	if(g_debugDumpIndexBuffers)
	{
		Display2::Device()->DumpIndexBuffers(g_debugDumpStart, g_debugDumpCount);
		g_debugDumpIndexBuffers = false;
	}
	
	if(g_debugDumpVertexShaders)
	{
		Display2::Device()->DumpVertexShaders(g_debugDumpStart, g_debugDumpCount);
		g_debugDumpVertexShaders = false;
	}
	
	if(g_debugDumpPixelShaders)
	{
		Display2::Device()->DumpPixelShaders(g_debugDumpStart, g_debugDumpCount);
		g_debugDumpPixelShaders = false;
	}

	if(g_debugDumpSurfaces)
	{
		Display2::Device()->DumpSurfaces(g_debugDumpStart, g_debugDumpCount);
		g_debugDumpSurfaces = false;
	}

	if(g_debugDumpTextures)
	{
		Display2::Device()->DumpTextures(g_debugDumpStart, g_debugDumpCount);
		g_debugDumpTextures = false;
	}

	if(g_debugDumpRenderTexture2Ds)
	{
		Display2::Device()->DumpRenderTexture2Ds(g_debugDumpStart, g_debugDumpCount);
		g_debugDumpRenderTexture2Ds = false;
	}

	if(g_debugDumpRenderTextureCubes)
	{
		Display2::Device()->DumpRenderTextureCubes(g_debugDumpStart, g_debugDumpCount);
		g_debugDumpRenderTextureCubes = false;
	}

	if(g_debugDumpRenderBuffers)
	{
		Display2::Device()->DumpRenderBuffers(g_debugDumpStart, g_debugDumpCount);
		g_debugDumpRenderBuffers = false;
	}
	#endif

	#if PLATFORM_PS3
	if(g_debugDumpTileManager)
	{
		Display2::Device()->GetTileManager().Dump();
		g_debugDumpTileManager = false;
	}

	if(g_debugDumpZcullManager)
	{
		Display2::Device()->GetZcullManager().Dump();
		g_debugDumpZcullManager = false;
	}
	#endif

	#if TRACK_TILED_MEMORY
	if(g_debugDumpTiledMemory)
	{
		Display2::Device()->m_TileInfoManager.DumpTiledMemory();
		g_debugDumpTiledMemory = false;
	}
	#endif
#endif
}

}; // namespace Framework
