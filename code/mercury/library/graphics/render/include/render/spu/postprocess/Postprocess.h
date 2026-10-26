#ifndef RENDER_SPU_POSTPROCESS_H
#define RENDER_SPU_POSTPROCESS_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Render
{
namespace Spu
{

struct C_PostprocessInfo
{
	U32		m_SourceAddress;
	U32		m_SourceStride;
	U32		m_SourceWidth;
	U32		m_SourceHeight;
	U32		m_SourceFormat;
	U32		m_DestinationAddress;
	U32		m_DestinationStride;
	U32		m_DestinationWidth;
	U32		m_DestinationHeight;
	U32		m_DestinationFormat;
	U32		m_UserData0;
	U32		m_UserData1;
	U32		m_GateAddress0;
	U32		m_GateAddress1;
	U32		m_GateValue0;
	U32		m_GateValue1;
};

void BindPostprocessSourceTexture(C_PostProcessInfo& info, const Display::C_Texture2D* pTexture);
void BindPostprocessDestinationTexture(C_PostProcessInfo& info, const Display::C_Texture2D* pTexture);

}; // namespace Spu
}; // namespace Render

#endif
