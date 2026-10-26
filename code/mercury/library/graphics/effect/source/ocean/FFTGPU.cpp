#include "stdafx.h"
#include "Misc_Shared/Types/Float16.h"
#include "Effect/Shaders/Constants.h"
#include "Effect/Ocean/FFTGPU.h"
#include "System/Misc/Random.h"

EXTERN_SHADER(psFFTHorizontal);
EXTERN_SHADER(psFFTVertical);
EXTERN_SHADER(psFrequency);
EXTERN_SHADER(psFrequencyDifferential);

namespace Effect
{
namespace Ocean
{
namespace Waves
{

Debug::Int g_debugFFTSeed("Render.Ocean.FFT.Seed", 37890);
Debug::Float g_debugFFTSize("Render.Ocean.FFT.Size", 32.0f, Debug::Range(0.0f, 128.0f));
Debug::Float g_debugFFTGravity("Render.Ocean.FFT.Gravity", 9.81f, Debug::Range(-16.0f, 16.0f));
Debug::Float g_debugFFTPhillipsA("Render.Ocean.FFT.PhillipsA", 0.40f, Debug::Range(0.0f, 10000.0f));
Debug::Vector g_debugFFTWind("Render.Ocean.FFT.Wind", 1.25f, -1.40f, 0.0f);

F32 PhillipsSpectrum(const F32 A, const F32 kX, const F32 kY, const F32 wX, const F32 wY, const F32 g = 9.81f)
{
	const F32 w2 = wX * wX + wY * wY;
	const F32 w = sqrtf(w2);
	const F32 k2 = kX * kX + kY * kY;
	const F32 k4 = k2 * k2;
	const F32 k = sqrtf(k2);
	const F32 L = w2 / g;

	if (abs(k) < 0.00001f)
	{
		return 0.0f;
	}
	else
	{
		const F32 cosine = Maths::Clamp01( (kX * wX + kY * wY) / (w * k) );
		return A * expf(-1.0f / (k2 * L * L)) * cosine / k4;
	}
}

C_FFTGPU::C_FFTGPU() : Display::I_Renderer("FFTGPU")
{
	m_Width = 128;
	m_Height = 128;
}

C_FFTGPU::~C_FFTGPU()
{
	DestroyRenderer();
}

bool C_FFTGPU::OnCreate()
{
	if (!Display::CreatePixelShaderFromFunc("Shaders/psFFTHorizontal", 0, &m_FFTHorizontalPixelShader, REF_SHADER(psFFTHorizontal)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psFFTVertical", 0, &m_FFTVerticalPixelShader, REF_SHADER(psFFTVertical)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psFrequency", 0, &m_FrequencyPixelShader, REF_SHADER(psFrequency)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psFrequencyDifferential", 0, &m_FrequencyDifferentialPixelShader, REF_SHADER(psFrequencyDifferential)))
		return false;

	if (!Display::Device()->CreateTexture2D(m_Width, m_Height, 1, Display::E_Format_Linear, Display::E_Format_G16R16F, &m_FrequencyTexture2D))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetTextureName(&m_FrequencyTexture2D, "_FFTFrequencyTexture");
	#endif

	BuildFrequencies();

	return true;
}

bool C_FFTGPU::OnCreateVolatile()
{
	if (!Display::Device()->CreateRenderTexture2D(m_Width, m_Height, 1, Display::E_Usage_Colour, Display::E_Format_G16R16F, Display::E_Multisample_None, 0, &m_HeightMapRenderTexture2D))
		return false;
	if (!Display::Device()->CreateRenderTexture2D(m_Width, m_Height, 1, Display::E_Usage_Colour, Display::E_Format_G16R16F, Display::E_Multisample_None, 0, &m_GradientMapRenderTexture2D))
		return false;
	if (!Display::Device()->CreateRenderTexture2D(m_Width, m_Height, 0, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_NormalMapRenderTexture2D))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_HeightMapRenderTexture2D, "_FFTGPUHeightMapRT");
	Display2::Device()->SetRenderTexture2DName(&m_GradientMapRenderTexture2D, "_FFTGPUGradientMapRT");
	Display2::Device()->SetRenderTexture2DName(&m_NormalMapRenderTexture2D, "_FFTGPUNormalMapRT");
	#endif

	return true;
}

void C_FFTGPU::OnDestroyVolatile()
{
	Display::Device()->Destroy(&m_GradientMapRenderTexture2D);
	Display::Device()->Destroy(&m_NormalMapRenderTexture2D);
	Display::Device()->Destroy(&m_HeightMapRenderTexture2D);
}

void C_FFTGPU::OnDestroy()
{
	Display::Device()->Destroy(&m_FrequencyTexture2D);

	Display::Device()->Destroy(&m_FrequencyDifferentialPixelShader);
	Display::Device()->Destroy(&m_FrequencyPixelShader);
	Display::Device()->Destroy(&m_FFTHorizontalPixelShader);
	Display::Device()->Destroy(&m_FFTVerticalPixelShader);
}

void C_FFTGPU::Render(const F32 fTime)
{
	PERF_EVENT("RenderFFT");

	// set states
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Wrap);

	// set surface
	Display::Device()->SetColourSurface(&m_HeightMapRenderTexture2D.GetSurface());

	// calculate the frequencies
	Display::Device()->SetTexture(0, &m_FrequencyTexture2D);

	Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS, Display::C_PSConstant(m_Width * g_debugFFTSize, m_Height * g_debugFFTSize, g_debugFFTGravity, fTime), 1);
	Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS_1, Display::C_PSConstant(1.0f / (F32) m_Width, 1.0f / (F32) m_Height, 0.0f, 0.0f), 1);
	Display::Device()->SetPixelShader(&m_FrequencyPixelShader);

	Display::Quad::DrawFullScreen();

	m_HeightMapRenderTexture2D.Resolve();

	// perform the FFT

	// horizontal
	for (S32 i = 1; i <= m_Width; i *= 2)
	{
		const F32 fTheta = 2.0f * Maths::Pi / (F32) i;

		Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS, Display::C_PSConstant((F32) m_Width, (F32) m_Height, 1.0f / (F32) m_Width, 1.0f / (F32) m_Height), 1);
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS_1, Display::C_PSConstant(m_Width / (F32) i, i / (F32) m_Width, fTheta, 1.0f / g_debugFFTSize), 1);
		Display::Device()->SetPixelShader(&m_FFTHorizontalPixelShader);

		Display::Device()->SetTexture(0, &m_HeightMapRenderTexture2D.GetTexture());

		Display::Quad::DrawFullScreen();

		m_HeightMapRenderTexture2D.Resolve();
	}
	for (S32 i = 1; i <= m_Height; i *= 2)
	{
		const F32 fTheta = 2.0f * Maths::Pi / (F32) i;

		Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS, Display::C_PSConstant((F32) m_Width, (F32) m_Height, 1.0f / (F32) m_Width, 1.0f / (F32) m_Height), 1);
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS_1, Display::C_PSConstant(m_Height / (F32) i, i / (F32) m_Height, fTheta, 1.0f / g_debugFFTSize), 1);
		Display::Device()->SetPixelShader(&m_FFTVerticalPixelShader);

		Display::Device()->SetTexture(0, &m_HeightMapRenderTexture2D.GetTexture());

		Display::Quad::DrawFullScreen();

		m_HeightMapRenderTexture2D.Resolve();
	}
	
	// set surface
	Display::Device()->SetColourSurface(&m_GradientMapRenderTexture2D.GetSurface());

	// calculate the frequencies
	Display::Device()->SetTexture(0, &m_FrequencyTexture2D);

	Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS, Display::C_PSConstant(m_Width * g_debugFFTSize, m_Height * g_debugFFTSize, g_debugFFTGravity, fTime), 1);
	Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS_1, Display::C_PSConstant(1.0f / (F32) m_Width, 1.0f / (F32) m_Height, 0.0f, 0.0f), 1);
	Display::Device()->SetPixelShader(&m_FrequencyDifferentialPixelShader);

	Display::Quad::DrawFullScreen();

	m_GradientMapRenderTexture2D.Resolve();

	// perform the FFT

	// horizontal
	for (S32 i = 1; i <= m_Width; i *= 2)
	{
		const F32 fTheta = 2.0f * Maths::Pi / (F32) i;

		Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS, Display::C_PSConstant((F32) m_Width, (F32) m_Height, 1.0f / (F32) m_Width, 1.0f / (F32) m_Height), 1);
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS_1, Display::C_PSConstant(m_Width / (F32) i, i / (F32) m_Width, fTheta, 1.0f / g_debugFFTSize), 1);
		Display::Device()->SetPixelShader(&m_FFTHorizontalPixelShader);

		Display::Device()->SetTexture(0, &m_GradientMapRenderTexture2D.GetTexture());

		Display::Quad::DrawFullScreen();

		m_GradientMapRenderTexture2D.Resolve();
	}
	for (S32 i = 1; i <= m_Height; i *= 2)
	{
		const F32 fTheta = 2.0f * Maths::Pi / (F32) i;

		Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS, Display::C_PSConstant((F32) m_Width, (F32) m_Height, 1.0f / (F32) m_Width, 1.0f / (F32) m_Height), 1);
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS_1, Display::C_PSConstant(m_Height / (F32) i, i / (F32) m_Height, fTheta, 1.0f / g_debugFFTSize), 1);
		Display::Device()->SetPixelShader(&m_FFTVerticalPixelShader);

		Display::Device()->SetTexture(0, &m_GradientMapRenderTexture2D.GetTexture());

		Display::Quad::DrawFullScreen();

		m_GradientMapRenderTexture2D.Resolve();
	}

	// convert to normal map
	m_TextureUtils.GenerateNormalFromGradient(&m_NormalMapRenderTexture2D, &m_GradientMapRenderTexture2D.GetTexture());

	// build mip maps
	m_TextureUtils.BuildMipMaps(&m_NormalMapRenderTexture2D.GetTexture(), true);

	// reset states
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);

	// restore colour surface
	Display::Device()->SetColourSurface();
}

F32 C_FFTGPU::GetWidth()
{
	return g_debugFFTSize;
}

F32 C_FFTGPU::GetHeight()
{
	return g_debugFFTSize;
}

void C_FFTGPU::BuildFrequencies()
{
	static const F32 c_RcpSqrt2 = 0.70710678118654752440084436210485f;

	Util::C_Random rng;
	rng.Seed(g_debugFFTSeed);

	Display::C_LockedRect rect = m_FrequencyTexture2D.Lock(0);

	U8 * pData = (U8 *) rect.pBits;
	for (S32 y = 0; y < m_Height; ++y)
	{
		U16 * pRow = (U16 *) (pData + rect.pitch * y);
		for (S32 x = 0; x < m_Width; ++x)
		{
			S32 xNo = x, yNo = y;
			if (xNo >= m_Width / 2)
				xNo = x - m_Width;
			if (yNo >= m_Height / 2)
				yNo = y - m_Height;

			const F32 kX = Maths::Pi2 * g_debugFFTSize * xNo / (F32) m_Width;
			const F32 kY = Maths::Pi2 * g_debugFFTSize * yNo / (F32) m_Height;

			const F32 fRealRandom = rng.SignedFRand() * c_RcpSqrt2;
			const F32 fImagRandom = rng.SignedFRand() * c_RcpSqrt2;

			const F32 fSqrtPhillips = sqrtf(PhillipsSpectrum(g_debugFFTPhillipsA, kX, kY, g_debugFFTWind.Value().vx, g_debugFFTWind.Value().vy));

			Half::F16 f16Real(fRealRandom * fSqrtPhillips);
			Half::F16 f16Imag(fImagRandom * fSqrtPhillips);

			*pRow++ = f16Imag.m_U16;
			*pRow++ = f16Real.m_U16;
		}
	}

	m_FrequencyTexture2D.Unlock(0);
}

}; // namespace Waves
}; // namespace Ocean
}; // namespace Effect
