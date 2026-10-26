#ifndef DISPLAY_MANAGER_MANAGER_INTERFACE_H
#define DISPLAY_MANAGER_MANAGER_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/Device.h"

namespace Display
{

//! I_Manager

class I_Manager
{
#ifdef DEVICE_INTERFACES
public:

	virtual ~I_Manager() {}
#endif
};

}; // namespace Display

#endif // DISPLAY_MANAGER_MANAGER_INTERFACE_H
