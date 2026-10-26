#include "stdafx.h"
#include "Display/Device/DeviceSetup.h"


namespace Display
{
	
const C_TileSetup g_TileSetups[] =
{
	// 640x360
	{
		640, 360, E_Multisample_None,
	},
	{
		640, 360, E_Multisample_2,
	},
	{
		640, 360, E_Multisample_4,
	},

	// 640x480 resolutions
	{
		640, 480, E_Multisample_None,
	},
	{
		640, 480, E_Multisample_2,
	},
	{
		640, 480, E_Multisample_4,
	},

	// 640x720
	{
		640, 720, E_Multisample_None,
	},
	{
		640, 720, E_Multisample_2,
	},
	{
		640, 720, E_Multisample_4,
	},

	// 1024x576
	{
		1024, 576, E_Multisample_None,
	},
	{
		1024, 576, E_Multisample_2,
	},
	{
		1024, 576, E_Multisample_4,
	},
	{
		1024, 576, E_Multisample_4,
	},

	// 960x640
	{
		960, 640, E_Multisample_None,
	},
	{
		960, 640, E_Multisample_2,
	},
	{
		960, 640, E_Multisample_4,
	},

	// 1280x720
	{
		1280, 720, E_Multisample_None,
	},
	{
		1280, 720, E_Multisample_2,
	},
	{
		1280, 720, E_Multisample_2,
	},
	{
		1280, 720, E_Multisample_2,
	},
	{
		1280, 720, E_Multisample_2,
	},
	{
		1280, 720, E_Multisample_4,
	},
	{
		1280, 720, E_Multisample_4,
	},

	// Invalid (use default setup)
	{
		1280, 720, E_Multisample_None,
	},
};

C_TileSetup LookupTileSetup(const U32 width, const U32 height, const E_Format colourFormat, const E_Format depthStencilFormat, const E_Multisample multisample)
{

	const U32 tileCount = (sizeof(g_TileSetups)/sizeof(C_TileSetup))-1;

	U32 bestMatch = tileCount; 

	for(U32 tileIndex=0; tileIndex<tileCount; tileIndex++)
	{
		const C_TileSetup& tileSetup = g_TileSetups[tileIndex];

		if(tileSetup.m_Width != width)
			continue;
		if(tileSetup.m_Height != height)
			continue;
		if(tileSetup.m_Multisample != multisample)
			continue;

		bestMatch = tileIndex;

		break;
	}

	return g_TileSetups[bestMatch];
}

}; // namespace Display
