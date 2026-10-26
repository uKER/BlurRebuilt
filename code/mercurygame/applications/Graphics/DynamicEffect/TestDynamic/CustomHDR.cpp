#include "StdAfx.h"
#include "CustomHDR.h"

#include <Render/Shaders/Constants.h>

using namespace Display;

Debug::Bool g_debugHDRShaderHDR("Render.PostProcess.HDR.UseShaderHDR", false);
Debug::Bool g_debugHDRBloomGamma("Render.PostProcess.HDR.UseBloomGamma", false);
Debug::Bool g_debugHDRBloomGammaSimple("Render.PostProcess.HDR.UseBloomGammaSimple", false);

#ifdef PLATFORM_PS3
Debug::Bool g_debugHDRShaderHDRU32("Render.PostProcess.HDR.UseShaderHDRU32", true);
#else
Debug::Bool g_debugHDRShaderHDRU32("Render.PostProcess.HDR.UseShaderHDRU32", false);
#endif

C_CustomHDR::C_CustomHDR()
{
	if(g_debugHDRShaderHDRU32)
		m_eType = PostProcess::E_HighDynamicRangeType_ShaderU32;
	else if(g_debugHDRShaderHDR)
		m_eType = PostProcess::E_HighDynamicRangeType_Shader;
	else
		m_eType = PostProcess::E_HighDynamicRangeType_PostProcess;
}

C_CustomHDR::~C_CustomHDR()
{
}

bool C_CustomHDR::Create()
{
/*#ifndef USE_HDR_U32
	if(!Display::CreatePixelShaderFromFunc("TonemapCustom", 0, &m_ToneMapCustom, sizeof(psToneMap), psToneMap))
			return false;
#else
	if(!Display::CreatePixelShaderFromFunc("TonemapCustom", 0, &m_ToneMapCustom, sizeof(psToneMapHDRU32), psToneMapHDRU32))
			return false;
#endif
	SetShader(&m_ToneMapCustom);*/

	return C_HighDynamicRangeBase::Create();
}


void C_CustomHDR::Destroy()
{
	//if(m_ToneMapCustom.IsCreated())
	//	Display::Device()->Destroy(&m_ToneMapCustom);

	C_HighDynamicRangeBase::Destroy();
}

void C_CustomHDR::Display() const
{
//	SetBloomGamma(g_debugHDRBloomGamma, g_debugHDRBloomGammaSimple);

	CalculateLuminance();

	C_HighDynamicRangeBase::Display();
}

