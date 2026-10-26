#ifndef DISPLAY_DEVICE_DEVICE_INTERFACE_H
#define DISPLAY_DEVICE_DEVICE_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceTypes.h"
#include "Display/Utils/Matrix.h"
#include "Display/Utils/Vector.h"

namespace Display
{

class I_Device
{
#ifdef DEVICE_INTERFACES
public:

	virtual ~I_Device() {}
#endif
};

}; // namespace Display

#endif // DISPLAY_DEVICE_DEVICE_INTERFACE_H
