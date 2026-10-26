#include "StdAfx.h"
#include "Samples/Section/SectionOcean.h"
#include "Render/Render/GridRender.h"

namespace Samples
{

C_SectionOcean::C_SectionOcean(const Util::C_String& name) : C_Section(name), m_Ocean(0.0f)
{
}

C_SectionOcean::~C_SectionOcean()
{
}

bool C_SectionOcean::OnCreate()
{
	if (!m_Ocean.Create())
		return false;
	
	m_Ocean.SetSunDir( Maths::C_Normal(1.0f, 1.0f, 0.0f) );
	m_Ocean.SetSunColour( Maths::C_RGBA_F32(0.882f * 5.0f, 0.863f * 5.0f, 0.682f * 5.0f, 1.0f) );
	m_Ocean.SetFogColour( Maths::HVec4(0.5f, 0.5f, 0.5f, 1.0f) );
	m_Ocean.SetFogInfo( Maths::HVec4(0.008f, 20.0f, 0.0f, 1.0f) );

	return true;
}

bool C_SectionOcean::OnCreateVolatile()
{
	return true;
}

void C_SectionOcean::OnDestroy()
{
	m_Ocean.Destroy();
}

void C_SectionOcean::OnDestroyVolatile()
{
}

void C_SectionOcean::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	m_Ocean.Update( Maths::C_TimeS(timer.GetFrameTime()) );
}

void C_SectionOcean::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionOcean");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	m_SkyDome.Render();

#ifdef PLATFORM_PS3
	m_Ocean.PatchPixelShader();
#endif
	m_Ocean.Render();
	
	Util::C_String str;
	str.Printf("Ocean Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 40.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}

}; // namespace Samples
