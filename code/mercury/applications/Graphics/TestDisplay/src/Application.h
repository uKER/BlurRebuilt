#ifndef APPLICATION_H
#define APPLICATION_H

#ifdef _MSC_VER
#pragma once
#endif

#if PLATFORM_XENON
#include "ApplicationXB2.h"
#elif PLATFORM_PS3
#include "ApplicationPS3.h"
#elif PLATFORM_PC
#include "ApplicationPC.h"
#endif

#endif // APPLICATION_H
