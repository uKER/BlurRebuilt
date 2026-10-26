#include "stdafx.h"
#include "Display/Device/GL/GLDeviceBuffers.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) 
#endif

namespace NewDisplay
{
namespace GL
{

C_BackBuffer::C_BackBuffer() :
	m_Width(0),
	m_Height(0),
	m_Multisample(E_Multisample_None),
	m_ColourFormat(E_Format_Unknown),
	m_DepthStencilFormat(E_Format_Unknown)
{
}

C_BackBuffer::~C_BackBuffer()
{
}

C_FrontBuffer::C_FrontBuffer() :
	m_Texture(0),
	m_Width(0),
	m_Height(0),
	m_Format(E_Format_Unknown),
	m_Multisample(E_Multisample_None)
{
}

C_FrontBuffer::~C_FrontBuffer()
{
}

C_RenderTarget::C_RenderTarget() :
	m_pColourSurface(NULL),
	m_pDepthStencilSurface(NULL)
{
}

C_RenderTarget::~C_RenderTarget()
{
}

}; // namespace GL
}; // namespace NewDisplay
