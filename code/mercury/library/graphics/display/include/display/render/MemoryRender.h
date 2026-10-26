#ifndef DISPLAY_RENDER_MEMORY_RENDER_H
#define DISPLAY_RENDER_MEMORY_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Display2/Memory/Memory.h>

namespace Display
{

class C_MemoryRender
{
public:

	C_MemoryRender();
	~C_MemoryRender();
	
	void DrawManager(const Display2::C_MemoryManager* pMemoryManager, const F32 x, F32& y, const F32 size, const Util::C_String& name);

	void Draw(const F32 offset = 0.f);
};

extern C_MemoryRender* g_pMemoryRender;
void MemoryRender(C_MemoryRender* pMemoryRender);
C_MemoryRender* MemoryRender();
const bool MemoryRenderExists();

}; // namespace Display

#include "Display/Render/MemoryRender.inl"

#endif // DISPLAY_RENDER_MEMORY_RENDER_H
