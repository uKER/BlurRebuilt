#ifndef DISPLAY2_DEVICE_H
#define DISPLAY2_DEVICE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display2/Display2Defines.h"

#if DEVICE_D3D11_PC
#include "Display2/Device/PCD3D11Device.h" // [Blur1PCDX11] Native device.
#elif DEVICE_GCM
#include "Display2/Device/GCMDevice.h"
#endif

#if DISPLAY2_MEMORY_GUARDS_ENABLE
#define CHECK_FOR_GRAPHICS_MEMORY_STOMPING() BIZ_ASSERT_SPEED(Display2::Device()->CheckMemoryGuards())
#else
#define CHECK_FOR_GRAPHICS_MEMORY_STOMPING()
#endif

namespace Display2
{

bool CreateDeviceManager(C_DeviceManager* pDeviceManager, C_DeviceManagerParams& deviceManagerParams);

void DestroyDeviceManager(C_DeviceManager* pDeviceManager);

void SetupDefaultDeviceParams(C_DeviceManager* pDeviceManager, C_DeviceParams& deviceParams);

bool CreateDevice(C_DeviceManager* pDeviceManager, C_Device* pDevice, C_DeviceParams& deviceParams);

void DestroyDevice(C_DeviceManager* pDeviceManager, C_Device* pDevice);

extern void* g_pDevice;

void Device(C_Device* pDevice);

C_Device* Device();

bool DeviceExists();

} // namespace Display2

#include "Display2/Device/Device.inl"

#endif // DISPLAY2_DEVICE_H
