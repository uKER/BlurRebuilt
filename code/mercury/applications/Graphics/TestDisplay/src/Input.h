#ifndef INPUT_H
#define INPUT_H

#ifdef _MSC_VER
#pragma once
#endif

#if PLATFORM_XENON
#include "InputXB2.h"
#elif PLATFORM_PS3
#include "InputPS3.h"
#elif PLATFORM_PC
#include "InputPC.h"
#endif

#endif // INPUT_H
