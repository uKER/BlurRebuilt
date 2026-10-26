#ifndef DISPLAY2_BUFFER_H
#define DISPLAY2_BUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display2/Context/Context.h"

#if DEVICE_GCM
#include "Display2/Buffer/GCMBuffer.h"
#endif

namespace Display2
{
} // namespace Display2

#include "Display2/Buffer/Buffer.inl"

#endif // DISPLAY2_BUFFER_H
