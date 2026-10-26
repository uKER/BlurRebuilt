#ifndef DISPLAY_BUFFER_COMMAND_BUFFER_H
#define DISPLAY_BUFFER_COMMAND_BUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceDefines.h"

#if defined(DEVICE_PS3GCM)
#include "Display/Buffer/GCM/GCMCommandBuffer.h"
#else
#include "Display/Buffer/D3D9/D3D9CommandBuffer.h"	// dummy
#endif

#endif // DISPLAY_BUFFER_COMMAND_BUFFER_H
