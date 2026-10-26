#ifndef DISPLAY_RENDER_DEBUG_RENDER_H
#define DISPLAY_RENDER_DEBUG_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Render/FontRender.h"
#include "Display/Render/BasicRender.h"
#include "Display/Render/HeapRender.h"
#include "Display/Render/MemoryRender.h"
#include "Display/Render/SystemRender.h"
#include "Display/Render/ResourceRender.h"
#include "Display/Render/TextureRender.h"
#include "Display/Render/TimingRender.h"
#include "Display/Render/MenuRender.h"
#include "Display/Render/CommandBufferRender.h"

namespace Display
{

class C_DebugRender
{
public:

	C_DebugRender();
	~C_DebugRender();

	void Draw();

public:

	C_FontRender				m_FontRender;
	C_BasicRender				m_BasicRender;
	C_HeapRender				m_HeapRender;
	C_MemoryRender				m_MemoryRender;
	C_SystemRender				m_SystemRender;
	C_ResourceRender			m_ResourceRender;
	C_TextureRender				m_TextureRender;
	C_TimingRender				m_TimingRender;
	C_MenuRender				m_MenuRender;
	C_CommandBufferRender		m_CommandBufferRender;
};

extern C_DebugRender* g_pDebugRender;
void DebugRender(C_DebugRender* pDebugRender);
C_DebugRender* DebugRender();
bool DebugRenderExists();

}; // namespace Display

#include "Display/Render/DebugRender.inl"

#endif // DISPLAY_RENDER_DEBUG_RENDER_H
