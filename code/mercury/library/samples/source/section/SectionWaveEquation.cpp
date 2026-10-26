#include "StdAfx.h"
#include "Samples/Section/SectionWaveEquation.h"
#include "Render/Render/GridRender.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize("", off)
#endif

namespace Samples
{

F32 RippleFunction(const F32 x)
{
	const F32 fAlpha = 0.60f;
	const F32 x2 = x * x;
	const F32 x3 = x * x2;
	const F32 x4 = x2 * x2;
	const F32 x5 = x2 * x3;
	const F32 x6 = x3 * x3;
	const F32 x7 = x3 * x4;
	return (1.0f / 6.0f) * x6 - 0.25f * (1 + fAlpha * fAlpha) * x4 + 0.5f * fAlpha * fAlpha * x2 - 1.0f;
}

C_SectionWaveEquation::C_SectionWaveEquation(const Util::C_String& name) : C_Section(name)
{
}

C_SectionWaveEquation::~C_SectionWaveEquation()
{
}

bool C_SectionWaveEquation::OnCreate()
{
	if (!Display::Device()->CreateTexture2D(32, 32, 1, Display::E_Format_Linear, Display::E_Format_R32F, &m_SplashTexture))
		return false;

	Display::C_LockedRect lockedRect = m_SplashTexture.Lock(0);

	F32* pRow = (F32*) lockedRect.pBits;
	for (U32 y = 0; y < 32; ++y)
	{
		for (U32 x = 0; x < 32; ++x)
		{
			const F32 fWidth = (x - 15.5f) / 15.5f;
			const F32 fHeight = (y - 15.5f) / 15.5f;
			const F32 fLengthSquared = fWidth * fWidth + fHeight * fHeight;
			pRow[x] = Maths::Clamp(1.0f - fLengthSquared, 0.0f, 1.0f);
		}
		pRow += lockedRect.pitch / 4;
	}

	m_SplashTexture.Unlock(0);

	return true;
}

bool C_SectionWaveEquation::OnCreateVolatile()
{
	if (!m_WaveSurface.CreateVolatile())
		return false;

	m_WaveSolver.ClearSurface(&m_WaveSurface);

	return true;
}

void C_SectionWaveEquation::OnDestroyVolatile()
{
	m_WaveSurface.DestroyVolatile();
}

void C_SectionWaveEquation::OnDestroy()
{
	Display::Device()->Destroy(&m_SplashTexture);
}

void C_SectionWaveEquation::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	if (input.IsButtonDn(E_Button_A))
	{
		const F32 fX = m_SplashRand.FRand(0.10f, 0.90f);
		const F32 fY = m_SplashRand.FRand(0.10f, 0.90f);
		const F32 fSize = 0.05f;
		const Display::C_Float4 vTransformU(1.0f / fSize, 0.0f, -fX / fSize, 0.0f);
		const Display::C_Float4 vTransformV(0.0f, 1.0f / fSize, -fY / fSize, 0.0f);
		m_WaveSolver.ModifySurface(&m_WaveSurface, &m_SplashTexture, vTransformU, vTransformV);
	}

	m_WaveSolver.SolveSurface(&m_WaveSurface, timer.GetFrameTime());
}

void C_SectionWaveEquation::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionWaveEquation");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_SkyDome.Render();

	Render::DrawGrid();

	m_TextureRender.DisplayRed<Display::C_Texture2D>(752.0f, 16.0f, 256.0f, 256.0f, m_WaveSurface.GetTexture(), 0.25f, 0.5f);

	Util::C_String str;
	str.Printf("Wave Equation Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
