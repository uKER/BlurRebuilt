#ifndef __STDAFX_H__
#define __STDAFX_H__

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#ifdef GCC
#define DONT_USE_SIMD
#pragma warning(disable:4101)
#endif

#define _USE_MATH_DEFINES
#include <string.h>

#include <core/core.h>
#include <Core/Maths.h>
#include <Core/containers/vector.h>
#include <Core/containers/bitvector.h>
#include <Core/containers/ContainerSerialisation.h>
#include <system/debuglog.h>
#include <system/memory.h>

#include <Aftk/Aftk.h>

#include "spatial_hierarchy/common.h"
#include "spatial_hierarchy/StorageMaths.h"

namespace SD{ extern Debug::Logger &g_Log; }

#endif
