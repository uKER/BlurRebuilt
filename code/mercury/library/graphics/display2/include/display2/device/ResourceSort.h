#ifndef DISPLAY2_RESOURCE_SORT_H
#define DISPLAY2_RESOURCE_SORT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display2/Device/BaseDevice.h"

namespace Display2
{

struct C_ResourceSort
{
	U32												m_Size;
	C_ResourceListIt								m_Iterator;

	bool operator<(const C_ResourceSort& other) const
	{
		return m_Size > other.m_Size;
	}
};

} // namespace Display2

#endif // DISPLAY2_BASE_DEVICE_H
