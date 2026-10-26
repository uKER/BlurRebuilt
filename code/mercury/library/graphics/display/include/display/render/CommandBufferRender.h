#ifndef DISPLAY_RENDER_COMMAND_BUFFER_RENDER_H
#define DISPLAY_RENDER_COMMAND_BUFFER_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Memory/MemoryManager2.h"

namespace Display
{

class C_CommandBufferRender
{
public:

	C_CommandBufferRender();
	~C_CommandBufferRender();

	void Draw();
};

extern C_CommandBufferRender* g_pCommandBufferRender;
void CommandBufferRender(C_CommandBufferRender* pCommandBufferRender);
C_CommandBufferRender* CommandBufferRender();
const bool CommandBufferRenderExists();

}; // namespace Display

#include "Display/Render/CommandBufferRender.inl"

#endif // DISPLAY_RENDER_COMMAND_BUFFER_RENDER_H
