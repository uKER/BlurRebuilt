#ifndef DISPLAY_RENDER_HEAP_RENDER_H
#define DISPLAY_RENDER_HEAP_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Memory/MemoryManager2.h"

namespace Display
{

class C_HeapRender
{
public:

	C_HeapRender();
	~C_HeapRender();

	void Draw();
	void Draw(C_MemoryBuffer* pBuffer, const Util::C_String& bufferName);
};

extern C_HeapRender* g_pHeapRender;
void HeapRender(C_HeapRender* pHeapRender);
C_HeapRender* HeapRender();
const bool HeapRenderExists();

}; // namespace Display

#include "Display/Render/HeapRender.inl"

#endif // DISPLAY_RENDER_HEAP_RENDER_H
