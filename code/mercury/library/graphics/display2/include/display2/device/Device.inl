#ifndef DISPLAY2_DEVICE_INL
#define DISPLAY2_DEVICE_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display2
{

INLINE C_Device* Device()
{
	assert(g_pDevice);
	return (C_Device*)g_pDevice;
}

INLINE bool DeviceExists()
{
	return g_pDevice != NULL;
}

} // namespace Display2

#endif // DISPLAY2_DEVICE_INL
