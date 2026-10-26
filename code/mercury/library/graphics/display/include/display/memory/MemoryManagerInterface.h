#ifndef DISPLAY_MEMORY_MEMORY_MANAGER_INTERFACE_H
#define DISPLAY_MEMORY_MEMORY_MANAGER_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

//! I_MemoryBuffer

class I_MemoryBuffer
{
#ifdef DEVICE_INTERFACES
public:

	virtual ~I_MemoryBuffer() {}
#endif
};

}; // namespace Display

#endif // DISPLAY_MEMORY_MEMORY_MANAGER_INTERFACE_H
