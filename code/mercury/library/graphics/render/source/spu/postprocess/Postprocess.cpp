#include "stdafx.h"
#include "Render/Spu/Postprocess/Postprocess.h"

namespace Render
{
namespace Spu
{

void BindPostprocessSourceTexture(C_PostProcessInfo& info, const Display::C_Texture2D* pTexture)
{
	info.m_SourceAddress		= pTexture->GetAddress();
	info.m_SourceStride			= pTexture->GetPitch();
	info.m_SourceWidth			= pTexture->GetWidth();
	info.m_SourceHeight			= pTexture->GetHeight();
	info.m_SourceFormat			= pTexture->GetFormat();
}

void BindPostprocessDestinationTexture(C_PostProcessInfo& info, const Display::C_Texture2D* pTexture)
{
	info.m_DestinationAddress	= pTexture->GetAddress();
	info.m_DestinationStride	= pTexture->GetPitch();
	info.m_DestinationWidth		= pTexture->GetWidth();
	info.m_DestinationHeight	= pTexture->GetHeight();
	info.m_DestinationFormat	= pTexture->GetFormat();
}

}; // namespace Spu
}; // namespace Render
