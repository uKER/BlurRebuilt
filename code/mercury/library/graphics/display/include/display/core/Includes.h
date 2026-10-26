#ifndef DISPLAY_CORE_INCLUDES_H
#define DISPLAY_CORE_INCLUDES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/core.h>

#if defined(PLATFORM_SPU)

#include <cell/gcm_spu.h>
#define GCM_NAMESPACE

#elif defined(PLATFORM_PS3)

#ifdef CFG_DEBUG
//#define CELL_GCM_DEBUG
#endif

#include <cell/gcm.h>
#include <cell/cgb.h>
#undef assert
#include <core/utility/coredebug.h>

#ifdef USE_GCM_HUD
#include <cell/gcm_pm.h>
#endif

#ifdef USE_GCM_REPLAY
#include "GcmReplay.h"
#endif

#include <cell/resc.h>

//#define USE_UNSAFE_GCM
#ifdef USE_UNSAFE_GCM
using namespace cell::Gcm::UnsafeInline;
#else
using namespace cell::Gcm::Inline;
#define GCM_NAMESPACE cell::Gcm::Inline
#endif

#elif defined(_WIN32) || defined(WIN32)

//#include <gl/gl.h>
//#include <gl/glu.h>
//#include <gl/glext.h>
//#include <gl/wglext.h>
//#include <cg/cg.h>
//#include <cg/cggl.h>
//#include <d3d10.h>
//#include <d3dx10.h>

#ifdef BUILD_CONFIG_DEBUG
#define D3D_DEBUG_INFO
#endif
#include <d3d9.h> // [Blur2PC][x64] enum values for the D3D11 backend; D3DX9 is no longer used
#include <dxerr.h>

#endif

#endif // DISPLAY_CORE_INCLUDES_H
