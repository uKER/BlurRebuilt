#ifndef RENDER_DEBUGRENDER_H
#define RENDER_DEBUGRENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/Device.h"
#include "System/Debug/I_DebugRender.h"
#include "Core/Threads/Misc.h"

namespace Render
{
namespace Debug
{

class C_ImmediateDebugRender;
class C_DeferredDebugRender;
class C_PrimitiveDebugRender;

class C_DebugRenderSys : public DbUtil::I_DebugRenderSys
{
public:
	C_DebugRenderSys();
	~C_DebugRenderSys();

    DbUtil::I_DebugRender* Immediate();
    DbUtil::I_DebugRender* Deferred();
	DbUtil::I_DebugRender* Primitive();
    DbUtil::I_DebugRender* Persistent();

	void ResetDeferred();
    void RenderDeferred();
	void RenderPrimitive();
	void ResetPersistent();
	void PrimitiveEndFrame();

private:

	C_ImmediateDebugRender* m_pImmediateRender;
	C_DeferredDebugRender* m_pDeferredRender;
    C_DeferredDebugRender* m_pPersistentRender;
	C_PrimitiveDebugRender* m_pPrimitiveRender;
};

extern C_DebugRenderSys* g_pDebugRenderSys;
C_DebugRenderSys* DebugRenderSys();

INLINE C_DebugRenderSys* DebugRenderSys()
{
	assert(g_pDebugRenderSys && "No DebugRenderSys instanciated");
    return g_pDebugRenderSys;
}

INLINE DbUtil::I_DebugRender* Immediate()
{
	//assert((Display::g_MainThreadID == MT::GetThreadId()) && "It must be called from renderthread"); 
	return DebugRenderSys()->Immediate();
}

INLINE DbUtil::I_DebugRender* Primitive()
{
	//assert((Display::g_MainThreadID == MT::GetThreadId()) && "It must be called from renderthread");
	return DebugRenderSys()->Primitive();
}

INLINE DbUtil::I_DebugRender* Deferred()
{
	return DebugRenderSys()->Deferred();
}

INLINE DbUtil::I_DebugRender* Persistent()
{
	return DebugRenderSys()->Persistent();
}

INLINE void PrimitiveEndFrame()
{
	DebugRenderSys()->PrimitiveEndFrame();
}

} // namespace Render
} // namespace Debug

#endif // RENDER_DEBUGRENDER_H
