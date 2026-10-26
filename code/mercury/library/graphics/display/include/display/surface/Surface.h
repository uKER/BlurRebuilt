#ifndef DISPLAY_SURFACE_SURFACE_H
#define DISPLAY_SURFACE_SURFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceDefines.h"

#if defined(DEVICE_PCD3D11)
#include "Display/Device/D3D11/D3D11Resources.h" // [Blur1PCDX11] Native resources.
#elif defined(DEVICE_PS3GCM)
#include "Display/Surface/GCM/GCMSurface.h"
#elif defined(DEVICE_PCGL) || defined(DEVICE_PSGL)
#include "Display/Surface/GL/GLSurface.h"
#else
#pragma message("Unsupported device define when compiling Surface.h")
#endif

#endif // DISPLAY_SURFACE_SURFACE_H
