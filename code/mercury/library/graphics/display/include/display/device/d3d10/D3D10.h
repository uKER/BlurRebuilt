#ifndef DISPLAY_D3D10_H
#define DISPLAY_D3D10_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/D3D10/D3D10DeviceTypes.h"

namespace NewDisplay
{

#ifdef GRAPHICS_D3D10
using namespace D3D10; // Make d3d10 the global device.
#endif

}; // namespace NewDisplay

#endif // DISPLAY_D3D10_H
