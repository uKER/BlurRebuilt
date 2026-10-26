#include "StdAfx.h"
#include "Samples/Section/SectionCelestial.h"
#include "Render/Render/GridRender.h"

namespace Samples
{

C_SectionCelestial::C_SectionCelestial(const Util::C_String& name) : C_Section(name)
{
	C_CelestialParams params;

	params.m_BodyTextureName	=	Util::C_String("Textures/Celestial/sun.dds");
	params.m_FlareTextureName	=	Util::C_String("Textures/Celestial/sunflare.dds");
	params.m_BodySize			=	0.045f;
	params.m_FlareSize			=	0.135f;
	params.m_BodyIntensity		=	1.000f;
	params.m_FlareIntensity		=	1.000f;
	params.m_Colour				=	Maths::C_RGBA_F32(1.0f, 0.9f, 0.7f, 1.0f);
	params.m_Direction			=	Maths::C_Normal(1.0f, 1.0f, 0.0f);

	m_pCelestialBody = new C_CelestialBody(params);
}

C_SectionCelestial::~C_SectionCelestial()
{
	if (m_pCelestialBody)
	{
		delete m_pCelestialBody;
		m_pCelestialBody = NULL;
	}
}

bool C_SectionCelestial::OnCreate()
{
	return m_pCelestialBody->Create();
}

bool C_SectionCelestial::OnCreateVolatile()
{
	return true;
}

void C_SectionCelestial::OnDestroy()
{
}

void C_SectionCelestial::OnDestroyVolatile()
{
}

void C_SectionCelestial::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	m_pCelestialBody->SetDirection( Maths::C_Normal(1.0f, 1.0f, 0.0f) );
	m_pCelestialBody->Update( Maths::C_TimeS(timer.GetFrameTime()) );
}

void C_SectionCelestial::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionCelestial");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	m_SkyDome.Render();

	m_pCelestialBody->Render(E_CelestialRender_Body);

	Display::BasicRender()->DrawColoured3DBox(Maths::HVec3(0.f, 2.f, -6.f), Maths::HVec3(4.f, 2.f, 0.5f), Display::GREEN);
	Render::DrawGrid();

	m_pCelestialBody->Render(E_CelestialRender_Flare);
	
	Util::C_String str;
	str.Printf("Celestial Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 40.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
