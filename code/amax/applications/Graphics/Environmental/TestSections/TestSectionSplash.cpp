#include "StdAfx.h"
#include "TestSections/TestSectionSplash.h"
#include "Render/Render/GridRender.h"
#include <Framework/Application/Application.h>
#include <Framework/Application/UserControls.h>
#include <Input/GameInput.h>
#include <System/Scheduler.h>

#if PLATFORM_PS3
static Scheduler::Policy g_HeightfieldSolverPolicy[] = { 0, 0, 1, 1, 1, 1, 1, 1 };
#else
static Scheduler::Policy g_HeightfieldSolverPolicy[] = { 1, 1, 1, 1, 1, 1 };
#endif

C_TestSectionSplash::C_TestSectionSplash(const Util::C_String& name) : Framework::C_TestSection(name)
{
	m_HeightfieldCpuSolver.SetPolicy(Scheduler::CreatePolicy(g_HeightfieldSolverPolicy));

	Scheduler::Init();
	Scheduler::Start();
	Scheduler::SetGroupPoolSize(4);
}

C_TestSectionSplash::~C_TestSectionSplash()
{
	Scheduler::Shutdown();
}

bool C_TestSectionSplash::OnCreate()
{
	if (!Display::Device()->CreateTexture2D(32, 32, 1, Display::E_Usage_System | Display::E_Format_Linear, Display::E_Format_R32F, &m_SplashTexture))
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

	if (!m_HeightfieldCpuSolver.Create())
		return false;

	return true;
}

bool C_TestSectionSplash::OnCreateVolatile()
{
	if (!m_HeightfieldSurface.CreateVolatile())
		return false;

#if PLATFORM_PS3
	m_HeightfieldCpuSolver.ClearSurface(&m_HeightfieldSurface);
#else
	m_HeightfieldGpuSolver.ClearSurface(&m_HeightfieldSurface);
#endif

	return true;
}

void C_TestSectionSplash::OnDestroyVolatile()
{
	m_HeightfieldSurface.DestroyVolatile();
}

void C_TestSectionSplash::OnDestroy()
{
	m_HeightfieldCpuSolver.Destroy();

	Display::Device()->Destroy(&m_SplashTexture);
}

void C_TestSectionSplash::OnUpdate()
{
#if PLATFORM_PS3
	Display::Device()->BlockOnFence(m_TextureFence);

	if (Input::GetGameInput(0)->GetButtonPressedDebounced(Framework::E_Button_MenuConfirm))
	{
		const F32 fX = m_SplashRand.FRand(0.10f, 0.90f);
		const F32 fY = m_SplashRand.FRand(0.10f, 0.90f);
		const F32 fSize = 0.05f;
		const Display::C_Float4 vTransformU(1.0f / fSize, 0.0f, -fX / fSize, 0.0f);
		const Display::C_Float4 vTransformV(0.0f, 1.0f / fSize, -fY / fSize, 0.0f);
		m_HeightfieldCpuSolver.ModifySurface(&m_HeightfieldSurface, &m_SplashTexture, vTransformU, vTransformV);
		m_HeightfieldCpuSolver.ModifyWait();
	}

	m_HeightfieldCpuSolver.SolveSurface(&m_HeightfieldSurface, m_HeightfieldSurface.GetTexture(), m_HeightfieldSurface.GetTexture(), m_HeightfieldSurface.GetTexture(), m_HeightfieldSurface.GetTexture(), Framework::C_Application::GetInstance()->GetTimer().GetFrameTime()); 
	m_HeightfieldCpuSolver.SolveWait();
#endif
}

void C_TestSectionSplash::OnPreRender()
{
#if !PLATFORM_PS3
	if (Input::GetGameInput(0)->GetButtonPressedDebounced(Framework::E_Button_MenuConfirm))
	{
		const F32 fX = m_SplashRand.FRand(0.10f, 0.90f);
		const F32 fY = m_SplashRand.FRand(0.10f, 0.90f);
		const F32 fSize = 0.05f;
		const Display::C_Float4 vTransformU(1.0f / fSize, 0.0f, -fX / fSize, 0.0f);
		const Display::C_Float4 vTransformV(0.0f, 1.0f / fSize, -fY / fSize, 0.0f);
		m_HeightfieldGpuSolver.ModifySurface(&m_HeightfieldSurface, &m_SplashTexture, vTransformU, vTransformV);
	}

	const F32 fTimeStep = Framework::C_Application::GetInstance()->GetTimer().GetFrameTime();

	m_HeightfieldGpuSolver.BindSolver(&m_HeightfieldSurface);
	m_HeightfieldGpuSolver.SolveInterior(&m_HeightfieldSurface, fTimeStep);
	m_HeightfieldGpuSolver.SolveTop(&m_HeightfieldSurface, m_HeightfieldSurface.GetTexture(), fTimeStep);
	m_HeightfieldGpuSolver.SolveBottom(&m_HeightfieldSurface, m_HeightfieldSurface.GetTexture(), fTimeStep);
	m_HeightfieldGpuSolver.SolveLeft(&m_HeightfieldSurface, m_HeightfieldSurface.GetTexture(), fTimeStep);
	m_HeightfieldGpuSolver.SolveRight(&m_HeightfieldSurface, m_HeightfieldSurface.GetTexture(), fTimeStep);
	m_HeightfieldGpuSolver.UnbindSolver(&m_HeightfieldSurface);
#endif
}

void C_TestSectionSplash::OnRender3DViewWithAA(const S32 viewIndex)
{
	PERF_EVENT("RenderTestSectionSplash");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_SkyDome.Render();

	Render::DrawGrid();

	m_TextureRender.DisplayRed<Display::C_Texture2D>(32.0f, 32.0f, 512.0f, 512.0f, m_HeightfieldSurface.GetTexture(), 0.25f, 0.5f);

#if PLATFORM_PS3
	m_TextureFence = Display::Device()->InsertFence();
#endif
}

void C_TestSectionSplash::OnRender3DViewWithoutAA(const S32 viewIndex)
{
	Util::C_String str;
	str.Printf("Splash Test Section : %.2f fps", Framework::C_Application::GetInstance()->GetTimer().GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}
