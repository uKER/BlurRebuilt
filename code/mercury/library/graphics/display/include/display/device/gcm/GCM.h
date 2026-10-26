#ifndef DISPLAY_DEVICE_GCM_GCM_H
#define DISPLAY_DEVICE_GCM_GCM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GCM/GCMDeviceTypes.h"

namespace Display
{
namespace GCM
{

extern sys_ppu_thread_t g_MainThreadID;
extern sys_ppu_thread_t g_RenderThreadID;

bool IsMainThread();

void SetMainThread( sys_ppu_thread_t threadID = UINT_MAX );

const U32 GCMGetNextReportIndex1();
const U32 GCMGetNextReportIndex2();

#define GCM_ALIGNMENT_SURFACE		128
#define GCM_ALIGNMENT_TEXTURE2D		128
#define GCM_ALIGNMENT_TEXTURE3D		128
#define GCM_ALIGNMENT_TEXTURECUBE	128
#define GCM_ALIGNMENT_VERTEXBUFFER	16
#define GCM_ALIGNMENT_INDEXBUFFER	16
#define GCM_ALIGNMENT_VERTEXSHADER	64
#define GCM_ALIGNMENT_PIXELSHADER	64

#ifdef CFG_DEBUG
void CheckGCMTexture(const CellGcmTexture* p);
void CheckGCMVertexTexture(const CellGcmTexture* p);
#else
inline void CheckGCMTexture(const CellGcmTexture* p) {}
inline void CheckGCMVertexTexture(const CellGcmTexture* p) {}
#endif

#if !PLATFORM_SPU
INLINE void GCMSetReport(const U32 type, const U32 index)
{
	assert(index >= 0 && index <= 2047);
	cellGcmSetReport(type, index);
}

INLINE void GCMSetTimeStamp(const U32 index)
{
	assert(index >= 0 && index <= 2047);
	cellGcmSetTimeStamp(index);
}
#endif

}; // namespace GCM

#ifdef GRAPHICS_GCM
using namespace GCM; // Make gcm the global device.
#endif

}; // namespace Display

#endif // DISPLAY_DEVICE_GCM_GCM_H
