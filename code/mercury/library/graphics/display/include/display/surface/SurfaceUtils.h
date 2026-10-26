#ifndef DISPLAY_SURFACE_SURFACE_UTILS_H
#define DISPLAY_SURFACE_SURFACE_UTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Surface/Surface.h"

namespace Display
{

U32 GetSurfaceSize(const U32 width, const U32 height, const U32 flags, const E_Format format, const E_Multisample multisample);

}; // namespace Display

#endif // DISPLAY_SURFACE_SURFACE_UTILS_H
