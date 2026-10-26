#ifndef AFTK_STDAFX_H
#define AFTK_STDAFX_H

//This disables the deprecation of the unsecure c library functions with MSVC 2005.
//We may well investigate a better solution to this, but as we're not currently writing
//PC games it's not so relevant.
#define _CRT_SECURE_NO_DEPRECATE 

#pragma once

#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#ifdef WIN32
#include <crtdbg.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#define _USE_MATH_DEFINES

#include <stdarg.h>
#include <math.h>

#include <string>

#include <iostream>
#include <sstream>
#include <fstream>

#include <cctype>
#include <algorithm>

#include <core/core.h>

#if !defined(WIN64) && !defined(GCC)
#define AFTK_XSI
#endif

#ifdef AFTK_XSI
#define __XSI_APPLICATION_
#define XSI_STATIC_LINK
#pragma warning(push)
//Disable warning regarding XSI stomping on the macros defined in MATHS.H. Will fix this.
#pragma warning(disable:4005) 
//Disable warning regarding conversion from 64bit int to 32bit int in XSI's SIBCString.h
#pragma warning(disable:4244)
#include <SemanticLayer.h>
#pragma warning(pop)
#endif

#include <core/maths.h>

#include <core/containers/vector.h>
#include <core/utility/HashOps.h>
#include <core/containers/OpenHashTable.h>
#include <core/containers/SortedBinaryTree.h>

#include <core/allocators/SSSMemoryPool.h>
#include <core/allocators/SSSMultiMemoryPool.h>
#include <core/allocators/SSSMemoryPoolUtInitialiser.h>

#include <core/utility/endian.h>

#include <system/Serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

#include <Aftk/Aftk.h>
#include "./source/aftk/AftkOutputMacros.h"

//#pragma message("TODO: Fix inclusion of old string builder")
#include "../../../common/tools_common/StringBuilder.h"

#ifdef AFTK_MAYA_IO
#include <set>
#include "./source/maya_io/MayaIo.h"
#endif

//FBX Definitions
#ifdef AFTK_FBX
#define K_PLUGIN
#define K_FBXSDK
#define K_NODLL
#endif

#endif
