#ifndef DISPLAY_DEVICE_DEVICE_SETUP_H
#define DISPLAY_DEVICE_DEVICE_SETUP_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceTypes.h"

namespace Display
{

struct C_TileSetup
{
	U32				m_Width;
	U32				m_Height;
	E_Multisample	m_Multisample;
};

C_TileSetup LookupTileSetup(const U32 width, const U32 height, const E_Format colourFormat, const E_Format depthStencilFormat, const E_Multisample multisample);

}; // Display

#endif // DISPLAY_DEVICE_DEVICE_SETUP_H
