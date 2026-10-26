#ifndef DISPLAY_DEVICE_DEVICE_MARKERS_H
#define DISPLAY_DEVICE_DEVICE_MARKERS_H

#ifdef _MSC_VER
#pragma once
#endif

#if PLATFORM_PS3 && !CFG_GOLD
#include "Display/Device/GCM/GCMDeviceMarkers.h"
#define USE_DEVICE_MARKERS 1
#endif

#if USE_DEVICE_MARKERS

#define DEVICE_MARKER(_name) \
	{ \
		static S32 deviceMarkerId = Display::C_DeviceMarkerManager::GetInstance().AddMarker(_name); \
		Display::C_DeviceMarkerManager::GetInstance().SetMarker(deviceMarkerId); \
	}

#else

#define DEVICE_MARKER(_name)

#endif

#endif // DISPLAY_DEVICE_DEVICE_MARKERS_H
