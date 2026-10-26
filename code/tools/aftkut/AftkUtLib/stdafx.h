#pragma once

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#include <iostream>
#ifdef COMPILER_VC
#include <tchar.h>
#endif

#include <stdarg.h>
#include <cassert>
#include <math.h>

#include <map>
#include <string>
#include <vector>
#include <deque>
#include <set>

#include <sstream>
#include <fstream>

#include <cctype>
#include <algorithm>

#include <core/core.h>
#include <core/maths.h>
#include <core/containers/vector.h>
#include <core/containers/bitvector.h>
#include <core/utility/stringproc.h>


#include <float.h>

#define PI 3.14159265358979323846f
#define AFTK_EPSILON 1e-6f

//#include <Aftk/AftkMathsNM.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <tools_debug/DebugManager.h>

extern Tu::DebugManager g_aftkDebugManager;
extern Tu::DebugManager g_aftkOutputManager;


#define AFTK_USE_MEMORY_POOL

#include <Aftk/Aftk.h>
#include <Aftk/AftkStringTable.h>

#include <aftkut/aftkut.h>
#include <aftkut/mathsutility.h>