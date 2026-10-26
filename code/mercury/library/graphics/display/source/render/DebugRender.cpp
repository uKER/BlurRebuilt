#include "stdafx.h"
#include "Display/Render/DebugRender.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace Display
{
	
Debug::Bool g_debugDisplayShowHeapRender			("Graphics.Display.Render.HeapRender.ShowHeapRender",						false);
Debug::Bool g_debugDisplayShowMemoryRender			("Graphics.Display.Render.MemoryRender.ShowMemoryRender",					false);
Debug::Bool g_debugDisplayShowSystemRender			("Graphics.Display.Render.SystemRender.ShowSystemRender",					false);
Debug::Bool g_debugDisplayShowResourceRender		("Graphics.Display.Render.ResourceRender.ShowResourceRender",				false);
Debug::Bool g_debugDisplayShowTextureRender			("Graphics.Display.Render.TextureRender.ShowTextureRender",					false);
Debug::Bool g_debugDisplayShowTimingRender			("Graphics.Display.Render.TimingRender.ShowTimingRender",					false);
Debug::Bool g_debugDisplayShowCommandBufferRender	("Graphics.Display.Render.CommandBufferRender.ShowCommandBufferRender",		false);

C_DebugRender::C_DebugRender()
{
	m_FontRender.SetRendererType(E_RendererType_Persistent);
	m_BasicRender.SetRendererType(E_RendererType_Persistent);

	assert(g_pDebugRender == NULL);
	g_pDebugRender = this;
}

C_DebugRender::~C_DebugRender()
{
	assert(g_pDebugRender == this);
	g_pDebugRender = NULL;
}

void C_DebugRender::Draw()
{
	if(g_debugDisplayShowHeapRender)
		m_HeapRender.Draw();
	
	if(g_debugDisplayShowMemoryRender)
		m_MemoryRender.Draw();
	
	if(g_debugDisplayShowSystemRender)
		m_SystemRender.Draw();
	
	if(g_debugDisplayShowResourceRender)
		m_ResourceRender.Draw();
	
	if(g_debugDisplayShowTextureRender)
		m_TextureRender.Draw();
	
	if(g_debugDisplayShowTimingRender)
		m_TimingRender.Draw();
	
	if(g_debugDisplayShowCommandBufferRender)
		m_CommandBufferRender.Draw();
	
	m_MenuRender.Draw();
}

C_DebugRender* g_pDebugRender = NULL;

void DebugRender(C_DebugRender* pDebugRender)
{
	g_pDebugRender = pDebugRender;
}

}; // namespace Display
