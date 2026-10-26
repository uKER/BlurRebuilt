#ifndef DISPLAY_BUFFER_GCM_GCM_COMMAND_BUFFER_H
#define DISPLAY_BUFFER_GCM_GCM_COMMAND_BUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GCM/GCM.h"
#include "Display/Memory/GCM/GCMRingBuffer.h"
#include "Display2/buffer/GCMBufferSPU.h"

#ifndef USE_GCM_HUD
#define USE_GCM_MEASURE
#endif

namespace Display
{
namespace GCM
{

//! C_CommandBuffer

class C_CommandBuffer : public Display2::C_CommandBuffer
{
public:

	void Run();

public:

	Display::C_MemorySet	m_UsedMemory;
};

}; // namespace GCM
}; // namespace Display

//#include "Display/Buffer/GCM/GCMCommandBuffer.inl"

#endif // DISPLAY_BUFFER_GCM_GCM_COMMAND_BUFFER_H
