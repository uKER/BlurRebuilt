#ifndef DISPLAY2_H
#define DISPLAY2_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display2/Display2Includes.h"
#include "Display2/Display2Logger.h"

#if 1

#if PLATFORM_SPU

#include <Display2/Buffer/GCMBufferSPU.h>

#else

#include "Display2/Device/Device.h"

#endif

#else

#include "Display2/Device/Device.h"
#include "Display2/Utils/Utils.h"

#endif

#endif // DISPLAY2_H
