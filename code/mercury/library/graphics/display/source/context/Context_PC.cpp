#include "stdafx.h"
#include "Display/Context/Context_PC.h"
#include "Display/Device/Device.h"
#include "Display/DisplayToDisplay2.h"

namespace Display
{

DEFINE_CONTEXT(Context, C_Context);

void C_SamplerState::SetMaxAnisotropy(S8 x)
{
	m_MaxAnisotropy = x;
}

C_Context::C_Context()
{
}

void C_Context::Initialise()
{
	m_Flags = m_Id;
}

void C_Context::CloneState(const I_Context* pContextInterface)
{
	if(pContextInterface == NULL)
	{
		m_CullMode = Display::Device()->GetCullModeDefault();
		m_Multisample = Display::Device()->GetBackBuffer().GetMultisample();
		m_Context.m_pd3dDevice = Display2::Device()->GetD3DDevice();
	}
	else
	{
		Display::C_Context* pContext = (Display::C_Context*)pContextInterface;

		m_CullMode = pContext->m_CullMode;
		m_Multisample = pContext->m_Multisample;
		m_pColourTexture = pContext->m_pColourTexture;
		m_pDepthStencilTexture = pContext->m_pDepthStencilTexture;
		m_pStencilTexture = pContext->m_pStencilTexture;	// [Blur2PC] T089
		m_Context = pContext->m_Context;
	}
}

void C_Context::InvalidateState()
{
}

void C_Context::BeginCommandBuffer(Display::C_CommandBuffer* pCommandBuffer)
{
}

void C_Context::EndCommandBuffer()
{
}

const C_View* C_Context::GetView() const
{
	return Display::Device()->GetView();
}

} // namespace Display
