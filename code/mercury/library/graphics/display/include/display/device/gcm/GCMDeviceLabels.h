#ifndef DISPLAY_DEVICE_GCM_GCM_DEVICE_LABELS_H
#define DISPLAY_DEVICE_GCM_GCM_DEVICE_LABELS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Display2/context/GcmContext.h>

// Labels: 64-255
static const U32 	g_CpuGpuSyncWaitLabel		= Display2::E_Label_CpuGpuSyncWait;		
static const U32 	g_BlockUntilIdleWaitLabel	= Display2::E_Label_BlockUntilIdleWait;
static const U32 	g_BinkTexturesWaitLabel		= Display2::E_Label_BinkTexturesWait;
static const U32	g_CommandFenceLabel			= Display2::E_Label_CommandFence;
static const U32	g_TextureFenceLabel			= Display2::E_Label_TextureFence;
static const U32	g_RasterFenceLabel			= Display2::E_Label_RasterFence;
static const U32	g_LastMarkerIdLabel			= Display2::E_Label_LastMarkerId;
static const U32	g_LastMarkerEaLabel			= Display2::E_Label_LastMarkerEa;

// Reports: 0-2047
static const U32	g_ReportZcullStats0			= 0;
static const U32	g_ReportZcullStats1			= 1;
static const U32	g_ReportGpuTime				= 2;
static const U32 	g_ReportIndexStart			= 16;
static const U32	g_ReportIndexCount			= 1008;
static const U32 	g_TimingIndexStart			= 1024;
static const U32	g_TimingIndexCount			= 1024;

#endif // DISPLAY_DEVICE_GCM_GCM_DEVICE_LABELS_H
