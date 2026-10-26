#ifndef DISPLAY_WINDOW_WINDOW_BASE_H
#define DISPLAY_WINDOW_WINDOW_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceTypes.h"

namespace Display
{


// C_TileSetup

struct C_TileSetup
{
	U32					m_Width;
	U32					m_Height;
	E_Multisample		m_Multisample;
};

C_TileSetup GetTileSetup(const U32 width, const U32 height, const E_Format colourFormat, const E_Format depthStencilFormat, const E_Multisample multisample);

// C_VideoMode

struct C_VideoMode
{
	U32					m_Width;
	U32					m_Height;
	E_Format			m_Format;
	#ifdef PLATFORM_PC
	U32					m_RefreshRate;	// [Blur2PC] retail PC keeps the adapter mode's refresh rate here
	#endif
	bool				m_WideScreen;
	bool				m_HiDef;

	const bool operator<(const C_VideoMode& other) const;
};

}; // namespace Display

#endif // DISPLAY_WINDOW_WINDOW_BASE_H
