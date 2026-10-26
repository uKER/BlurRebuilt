#include "stdafx.h"
#include "Render/Debug/DebugRender.h"
#include "Render/Debug/ImmediateDebugRender.h"
#include "Render/Debug/DeferredDebugRender.h"
#include "Render/Debug/PrimitiveDebugRender.h"


static void ClearPersistent(void*)
{
	Render::Debug::DebugRenderSys()->ResetPersistent();
}
static Debug::Function g_GamePhysicsAddBox("Graphics.Debug.ClearPersistentDebug", ClearPersistent, NULL);


namespace Render
{
namespace Debug
{

C_DebugRenderSys::C_DebugRenderSys() : m_pImmediateRender(NULL), m_pDeferredRender(NULL), m_pPersistentRender(NULL), m_pPrimitiveRender(NULL)
{
	assert(g_pDebugRenderSys == NULL);
	g_pDebugRenderSys = this;

	m_pImmediateRender = new C_ImmediateDebugRender();
	m_pPrimitiveRender = new C_PrimitiveDebugRender();
	m_pDeferredRender = new C_DeferredDebugRender(m_pImmediateRender, 512);
	m_pPersistentRender = new C_DeferredDebugRender(m_pImmediateRender, 512);
	
}

C_DebugRenderSys::~C_DebugRenderSys()
{
	delete m_pImmediateRender;
	delete m_pDeferredRender;
	delete m_pPersistentRender;
	delete m_pPrimitiveRender;

	assert(g_pDebugRenderSys == this);
	g_pDebugRenderSys = NULL;
}

DbUtil::I_DebugRenderPtr C_DebugRenderSys::Immediate()
{
   return m_pImmediateRender;
}

DbUtil::I_DebugRenderPtr C_DebugRenderSys::Deferred()
{
	return m_pDeferredRender;
}

DbUtil::I_DebugRenderPtr C_DebugRenderSys::Persistent()
{
    return m_pPersistentRender;
}

DbUtil::I_DebugRender* C_DebugRenderSys::Primitive()
{
	return m_pPrimitiveRender;
}

void C_DebugRenderSys::RenderDeferred()
{
	//assert(m_pDeferredRender);
	
	Display::Device()->SetViewShaderConstants();

	m_pDeferredRender->Render();
	//m_pDeferredRender->Reset();
	if (m_pPersistentRender)
    {
        m_pPersistentRender->Render();
    }

	//Knocked out - this is called later in 2 places, so must be drawn twice!!!
	//Render Primitive
	//RenderPrimitive();
}

void C_DebugRenderSys::ResetDeferred()
{
	m_pDeferredRender->Reset();
}

void C_DebugRenderSys::ResetPersistent()
{
	m_pPersistentRender->Reset();
}


void C_DebugRenderSys::RenderPrimitive()
{
	m_pPrimitiveRender->Render();
}

void C_DebugRenderSys::PrimitiveEndFrame()
{
	m_pPrimitiveRender->EndFrame();
}
C_DebugRenderSys* g_pDebugRenderSys = NULL;


} // namespace Render
} // namespace Debug
