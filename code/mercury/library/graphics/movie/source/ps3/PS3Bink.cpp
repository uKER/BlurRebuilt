#include "stdafx.h"
#include "Movie/PS3/PS3Bink.h"
#include <Display/Device/Device.h>

EXTERN_SHADER(vsBink);
EXTERN_SHADER(psBink);
EXTERN_SHADER(psBinkNoAlpha);

namespace Movie
{

C_PS3Shaders::C_PS3Shaders()
{
	screen_width	= 0.0f;
	screen_height	= 0.0f;
	vertex_size		= 0;
	vertex_offset	= 0;
	vertex_address	= NULL;
}

C_PS3Shaders::~C_PS3Shaders()
{
}

bool C_PS3Shaders::CreateShaders()
{
	if(!m_vsBink.IsCreated())
	{
		if(!Display::CreateVertexShaderFromFunc("Shaders/vsBink", 0, &m_vsBink, REF_SHADER(vsBink)))
		{
			DestroyShaders();
			return false;
		}

		m_AdjustParam		= m_vsBink.GetParameter(0, "Adjust");
		m_UVRangeParam		= m_vsBink.GetParameter(1, "UVRange");
	}
	
	if(!m_psBink.IsCreated())
	{
		if(!Display::CreatePixelShaderFromFunc("Shaders/psBink", 0, &m_psBink, REF_SHADER(psBink)))
		{
			DestroyShaders();
			return false;
		}

		m_AlphaParam		= m_psBink.GetParameter(-1,	"Alpha");
	}

	if(!m_psBinkNoAlpha.IsCreated())
	{
		if(!Display::CreatePixelShaderFromFunc("Shaders/psBinkNoAlpha", 0, &m_psBinkNoAlpha, REF_SHADER(psBinkNoAlpha)))
		{
			DestroyShaders();
			return false;
		}

		m_AlphaNPAParam		= m_psBinkNoAlpha.GetParameter(-1, "AlphaNPA");
	}

	return true;
}

void C_PS3Shaders::DestroyShaders()
{
	if(m_vsBink.IsCreated())
	{
		Display::Device()->Destroy(&m_vsBink);
	}

	if(m_psBink.IsCreated())
	{
		Display::Device()->Destroy(&m_psBink);
	}

	if(m_psBinkNoAlpha.IsCreated())
	{
		Display::Device()->Destroy(&m_psBinkNoAlpha);
	}
}

void C_PS3Shaders::SetVertexShaderBink()
{
	Display::Device()->SetVertexShader(&m_vsBink);
}

void C_PS3Shaders::SetVertexShaderBink_Adjust(const F32* pAdjust)
{
	Display::Device()->SetVertexShaderParameterF(m_AdjustParam, pAdjust, 1);
}

void C_PS3Shaders::SetVertexShaderBink_UVRange(const F32* pUVRange)
{
	Display::Device()->SetVertexShaderParameterF(m_UVRangeParam, pUVRange, 1);
}

void C_PS3Shaders::SetPixelShaderBink()
{
	Display::Device()->SetPixelShader(&m_psBink);
}

void C_PS3Shaders::SetPixelShaderBink_AlphaLevel(const F32 alphaLevel)
{
	const F32 temp[4] = {alphaLevel, 0.0f, 0.0f, 0.0f};
	Display::Device()->SetPixelShaderParameterF(m_AlphaParam, temp);
}

void C_PS3Shaders::SetPixelShaderBinkNoAlpha()
{
	Display::Device()->SetPixelShader(&m_psBinkNoAlpha);
}

void C_PS3Shaders::SetPixelShaderBinkNoAlpha_AlphaLevel(const F32 alphaLevel)
{
	const F32 temp[4] = {alphaLevel, 0.0f, 0.0f, 0.0f};
	Display::Device()->SetPixelShaderParameterF(m_AlphaNPAParam, temp);
}

bool C_PS3Shaders::AllocVertexMemory(const U32 size, U8** ppAddress)
{
	Display::C_MemoryBuffer* pVideoMemory = Display::Device()->GetVideoMemory();
	return pVideoMemory->AllocateFromStart(128, size, ppAddress);
}

void C_PS3Shaders::FreeVertexMemory(const U32 size, U8* ppAddress)
{
	Display::C_MemoryBuffer* pVideoMemory = Display::Device()->GetVideoMemory();
	pVideoMemory->Free(size, ppAddress);
}

bool C_PS3Shaders::AllocTextureMemory(const U32 size, U8** ppAddress)
{
	Display::C_MemoryBuffer* pSystemMemory = Display::Device()->GetSystemMemory();
	return pSystemMemory->AllocateFromStart(128, size, ppAddress);
}

void C_PS3Shaders::FreeTextureMemory(const U32 size, U8* ppAddress)
{
	Display::C_MemoryBuffer* pSystemMemory = Display::Device()->GetSystemMemory();
	pSystemMemory->Free(size, ppAddress);
}

}; // namespace Movie
