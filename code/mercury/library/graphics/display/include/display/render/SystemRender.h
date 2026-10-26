#ifndef DISPLAY_RENDER_SYSTEM_RENDER_H
#define DISPLAY_RENDER_SYSTEM_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

class C_SystemRender
{
public:

	C_SystemRender();
	~C_SystemRender();

	void Draw();
};

extern C_SystemRender* g_pSystemRender;
void SystemRender(C_SystemRender* pSystemRender);
C_SystemRender* SystemRender();
const bool SystemRenderExists();

}; // namespace Display

#include "Display/Render/SystemRender.inl"

#endif // DISPLAY_RENDER_SYSTEM_RENDER_H
