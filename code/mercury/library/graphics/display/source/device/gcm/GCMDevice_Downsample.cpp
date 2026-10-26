#include "stdafx.h"
#include "Display/Device/GCM/GCMDevice.h"
#include "Display/Shader/PixelShaderUtils.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

EXTERN_SHADER(psDownsampleDefault);
EXTERN_SHADER(psDownsampleQuincunx);
EXTERN_SHADER(psDownsampleQuincunxAlt);
EXTERN_SHADER(psDownsampleQuincunxShifted);

namespace Display
{
namespace GCM
{

bool C_GCMDevice::CreateDownsampleShaders()
{
	if(!CreatePixelShaderFromFunc("Shaders/psDownsampleDefault", 0, &m_psDownsample[E_Downsample_Default], REF_SHADER(psDownsampleDefault)))
		return false;
	if(!CreatePixelShaderFromFunc("Shaders/psDownsampleQuincunx", 0, &m_psDownsample[E_Downsample_Quincunx], REF_SHADER(psDownsampleQuincunx)))
		return false;
	if(!CreatePixelShaderFromFunc("Shaders/psDownsampleQuincunxAlt", 0, &m_psDownsample[E_Downsample_QuincunxAlt], REF_SHADER(psDownsampleQuincunxAlt)))
		return false;
	if(!CreatePixelShaderFromFunc("Shaders/psDownsampleQuincunxShifted", 0, &m_psDownsample[E_Downsample_QuincunxShifted], REF_SHADER(psDownsampleQuincunxShifted)))
		return false;

	return true;
}

void C_GCMDevice::DestroyDownsampleShaders()
{
	DestroyPixelShader(&m_psDownsample[E_Downsample_Default]);
	DestroyPixelShader(&m_psDownsample[E_Downsample_Quincunx]);
	DestroyPixelShader(&m_psDownsample[E_Downsample_QuincunxAlt]);
	DestroyPixelShader(&m_psDownsample[E_Downsample_QuincunxShifted]);
}

void C_GCMDevice::SetupDownsample(const E_Downsample downsample, C_Texture* pSrcTexture)
{
	assert(downsample < E_Downsample_Count);
	SetPixelShader(&m_psDownsample[downsample]);

	assert(pSrcTexture);
	SetTextureCached(0, pSrcTexture);
	
	const C_TextureControl textureControl(
		1,
		0,
		15*256,
		CELL_GCM_TEXTURE_MAX_ANISO_1);
	SetTextureControlCached(0, textureControl);
	
	const C_TextureAddress textureAddress(
		CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
		CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
		CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
		CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL,
		CELL_GCM_TEXTURE_ZFUNC_LESS,
		0);
	SetTextureAddressCached(0, textureAddress);

	switch(downsample)
	{
	case E_Downsample_Quincunx:
		{
			const C_TextureFilter textureFilter(
				0,
				CELL_GCM_TEXTURE_CONVOLUTION_MIN,
				CELL_GCM_TEXTURE_CONVOLUTION_MAG,
				CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
			SetTextureFilterCached(0, textureFilter);
		}
		break;
	case E_Downsample_QuincunxAlt:
		{
			const C_TextureFilter textureFilter(
				0,
				CELL_GCM_TEXTURE_CONVOLUTION_MIN,
				CELL_GCM_TEXTURE_CONVOLUTION_MAG,
				CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX_ALT);
			SetTextureFilterCached(0, textureFilter);
		}
		break;
	case E_Downsample_QuincunxShifted:
		{
			const C_TextureFilter textureFilter0(
				0,
				CELL_GCM_TEXTURE_CONVOLUTION_MIN,
				CELL_GCM_TEXTURE_CONVOLUTION_MAG,
				CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
			SetTextureFilterCached(0, textureFilter0);

			const C_TextureFilter textureFilter1(
				0,
				CELL_GCM_TEXTURE_CONVOLUTION_MIN,
				CELL_GCM_TEXTURE_CONVOLUTION_MAG,
				CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX_ALT);
			SetTextureFilterCached(1, textureFilter1);

			SetTexture(1, pSrcTexture);
			SetTextureControlCached(1, textureControl);
			SetTextureAddressCached(1, textureAddress);
		}
		break;
	default:
		{
			const C_TextureFilter textureFilter(
				0,
				CELL_GCM_TEXTURE_LINEAR,
				CELL_GCM_TEXTURE_LINEAR,
				CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
			SetTextureFilterCached(0, textureFilter);
		}
		break;
	}
}

}; // namespace GCM
}; // namespace Display
