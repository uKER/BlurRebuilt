#ifdef _MSC_VER_
#pragma once
#endif

#ifndef ATTK_STDAFX_H
#define ATTK_STDAFX_H

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#include <string>
#include <iostream>

#ifndef COMPILER_GNU
#define WIN32_LEAN_AND_MEAN	
#include <windows.h>
#endif

#include <core/core.h>
#include <core/maths.h>

//#include <core/containers.h>
#include <core/containers/vector.h>

#endif
