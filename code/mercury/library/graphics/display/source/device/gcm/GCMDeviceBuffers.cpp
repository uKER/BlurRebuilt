#include "stdafx.h"
#include "Display/Device/GCM/GCMDeviceBuffers.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

namespace Display
{
namespace GCM
{

// C_BackBuffer

C_BackBuffer::C_BackBuffer()
{
}

C_BackBuffer::~C_BackBuffer()
{
}

void C_BackBuffer::SetWidth(const U32 width)
{
	m_RenderBuffer.m_Width = width;
}

void C_BackBuffer::SetHeight(const U32 height)
{
	m_RenderBuffer.m_Height = height;
}

void C_BackBuffer::SetMultisample(const E_Multisample multisample)
{
	m_RenderBuffer.m_Multisample = GetMultisample2(multisample);
}

void C_BackBuffer::SetColourFormat(const E_Format format)
{
	m_RenderBuffer.SetColourFormat(0, GetTextureFormat2(format));
}

void C_BackBuffer::SetDepthStencilFormat(const E_Format format)
{
	m_RenderBuffer.SetDepthStencilFormat(GetTextureFormat2(format));
}

// C_FrontBuffer

C_FrontBuffer::C_FrontBuffer()
{
}

C_FrontBuffer::~C_FrontBuffer()
{
}

void C_FrontBuffer::SetWidth(const U32 width)
{
	m_RenderBuffer.m_Width  = width;
}

void C_FrontBuffer::SetHeight(const U32 height)
{
	m_RenderBuffer.m_Height = height;
}

void C_FrontBuffer::SetFormat(const E_Format format)
{
	m_RenderBuffer.SetColourFormat(0, GetTextureFormat2(format));
}

// C_RenderTarget

C_RenderTarget::C_RenderTarget() :
	m_pColourSurface(NULL),
	m_pDepthStencilSurface(NULL)
{
}

C_RenderTarget::~C_RenderTarget()
{
}

}; // namespace GCM
}; // namespace Display
