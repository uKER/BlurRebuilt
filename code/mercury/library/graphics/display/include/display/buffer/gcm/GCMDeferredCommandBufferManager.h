#ifndef DISPLAY_BUFFER_GCM_GCM_DEFERRED_COMMAND_BUFFER_MANAGER_H
#define DISPLAY_BUFFER_GCM_GCM_DEFERRED_COMMAND_BUFFER_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GCM/GCM.h"
#include "Display/Buffer/GCM/GCMCommandBuffer.h"
#include "Display/Memory/GCM/GCMRingBuffer.h"
#include "Display2/buffer/GCMBufferSPU.h"

#ifndef USE_GCM_HUD
#define USE_GCM_MEASURE
#endif

namespace Display
{

#define MAX_DEFERRED_COMMAND_BUFFERS 128

class C_DeferredCommandBufferManager
{
public:
	/// constructor
	C_DeferredCommandBufferManager();

	/// add a command buffer to be patched into the main command buffer later on
	void AddCommandBuffer(Display::GCM::C_CommandBuffer *p);

	/// patch in any command buffers that are ready
	/// returns true if all commandbuffers were flushed, or false if some commandbuffers haven't finished building yet
	bool Flush();

private:
	U32 m_iRead, m_iWrite;
	C_CommandBuffer *m_Entries[MAX_DEFERRED_COMMAND_BUFFERS];
};

extern C_DeferredCommandBufferManager g_DeferredCommandBufferManager;

}; // namespace Display

#endif // DISPLAY_BUFFER_GCM_GCM_COMMAND_BUFFER_H
