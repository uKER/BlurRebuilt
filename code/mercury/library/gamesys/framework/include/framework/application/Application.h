#ifndef FRAMEWORK_APPLICATION_H
#define FRAMEWORK_APPLICATION_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined(PLATFORM_PS3)
#include "Framework/Application/ApplicationPS3.h"
#elif defined(PLATFORM_PC)
#include "Framework/Application/ApplicationPC.h"
#endif

#define C_APPLICATION C_TestApp
#define C_CORESYSTEMS Framework::C_CoreSystems

#endif // FRAMEWORK_APPLICATION_H
