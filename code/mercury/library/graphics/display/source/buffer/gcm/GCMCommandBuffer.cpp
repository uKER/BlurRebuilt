#include "stdafx.h"
#include "Display/Buffer/GCM/GCMCommandBuffer.h"
#include "Display/Buffer/gcm/GCMDeferredCommandBufferManager.h"

namespace Display
{
namespace GCM
{

void C_CommandBuffer::Run()
{
	Display::g_DeferredCommandBufferManager.AddCommandBuffer(this);
	Display::g_DeferredCommandBufferManager.Flush();
}

}; // namespace GCM
}; // namespace Display
