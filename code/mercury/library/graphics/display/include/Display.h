#ifndef DISPLAY_H
#define DISPLAY_H

#ifdef _MSC_VER
#pragma once
#endif

#if PLATFORM_SPU

#include <System/Perf.h>

#include "Display/Device/DeviceDefines.h"
#include "Display/Device/StencilState.h"

#include "Display/Buffer/GCM/GCMVertexDeclaration.h"
#include "Display/Buffer/GCM/GCMVertexBuffer.h"
#include "Display/Buffer/GCM/GCMIndexBuffer.h"

#include "Display/Shader/GCM/GCMVertexShader.h"
#include "Display/Shader/GCM/GCMPixelShader.h"

#include "Display/Texture/GCM/GCMTexture2D.h"

#include "Display/Utils/Matrix.h"
#include "Display/View/View.h"

#else // PLATFORM_SPU

#include "Display/Core/Includes.h"

#include <System/DebugLog.h>
#include <System/Debug/Debug.h>

#include "Display/Device/DeviceDefines.h"
#include "Display/Utils/DebugLog.h"
#include "Display/Utils/Globals.h"
#include "Display/Utils/Perf.h"
#include "Display/Timing/Timings.h"
#include "Display/Utils/Shader.h"
#include "Display/Memory/Memory.h"
#include "Display/Manager/Manager.h"
#include "Display/Buffer/DynamicVB.h"
#include "Display/Render/Render.h"
#include "Display/Occlusion/OcclusionQuery.h"
#include "Display/Render/DebugRender.h"
#include "Display/Utils/Rect.h"

#endif // PLATFORM_SPU

#endif // DISPLAY_H
