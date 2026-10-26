#include "StdAfx.h"
#include "TestSections/TestSectionDynamicLights.h"

#include "Render/Mesh/MeshLoader.h"
#include "Render/State/RenderState.h"
#include "Render/Utils/Globals.h"
#include "Render/Utils/Helper.h"

#include "Framework/Application/Application.h"

Debug::Vector	g_debugTestSectionDynamicLightsPosition0("TestSection.DynamicLights.Position0",	0.0f, 0.0f, 0.0f);
Debug::Float	g_debugTestSectionDynamicLightsRadius0	("TestSection.DynamicLights.Radius0",	1.0f);
Debug::Colour	g_debugTestSectionDynamicLightsColour0	("TestSection.DynamicLights.Colour0",	1.0f, 0.0f, 0.0f, 1.0f);
Debug::Bool		g_debugTestSectionDynamicLightsEnable0	("TestSection.DynamicLights.Enable0",	false);

Debug::Vector	g_debugTestSectionDynamicLightsPosition1("TestSection.DynamicLights.Position1",	0.0f, 0.0f, 0.0f);
Debug::Float	g_debugTestSectionDynamicLightsRadius1	("TestSection.DynamicLights.Radius1",	1.0f);
Debug::Colour	g_debugTestSectionDynamicLightsColour1	("TestSection.DynamicLights.Colour1",	0.0f, 1.0f, 0.0f, 1.0f);
Debug::Bool		g_debugTestSectionDynamicLightsEnable1	("TestSection.DynamicLights.Enable1",	false);

Debug::Vector	g_debugTestSectionDynamicLightsPosition2("TestSection.DynamicLights.Position2",	0.0f, 0.0f, 0.0f);
Debug::Float	g_debugTestSectionDynamicLightsRadius2	("TestSection.DynamicLights.Radius2",	1.0f);
Debug::Colour	g_debugTestSectionDynamicLightsColour2	("TestSection.DynamicLights.Colour2",	0.0f, 0.0f, 1.0f, 1.0f);
Debug::Bool		g_debugTestSectionDynamicLightsEnable2	("TestSection.DynamicLights.Enable2",	false);

Debug::Vector	g_debugTestSectionDynamicLightsPosition3("TestSection.DynamicLights.Position3",	0.0f, 0.0f, 0.0f);
Debug::Float	g_debugTestSectionDynamicLightsRadius3	("TestSection.DynamicLights.Radius3",	1.0f);
Debug::Colour	g_debugTestSectionDynamicLightsColour3	("TestSection.DynamicLights.Colour3",	1.0f, 1.0f, 0.0f, 1.0f);
Debug::Bool		g_debugTestSectionDynamicLightsEnable3	("TestSection.DynamicLights.Enable3",	false);

Debug::Bool		g_debugTestSectionDynamicLightsRender	("TestSection.DynamicLights.Render",	false);

C_TestSectionDynamicLights::C_TestSectionDynamicLights(const Util::C_String& name) : Framework::C_TestSection(name)
{
//	Render::g_pColourBuffer = &m_ColourTexture;
//	Render::g_pDepthStencilBuffer = &m_DepthTexture;
}

C_TestSectionDynamicLights::~C_TestSectionDynamicLights()
{
}

bool C_TestSectionDynamicLights::OnCreate()
{
	if (!Display::Device()->CreateTexture2D(1280, 720, 1, Display::E_Usage_System, Display::E_Format_A8R8G8B8, &m_ColourTexture))
		return false;
	if (!Display::Device()->CreateTexture2D(1280, 720, 1, Display::E_Usage_System, Display::E_Format_D24FS8, &m_DepthTexture))
		return false;

	if (!Render::Mesh::LoadMesh("Models/Test", &m_Mesh))
		return false;

	m_Mesh.AddInstance();

	return true;
}

bool C_TestSectionDynamicLights::OnCreateVolatile()
{
	if (!Display::Device()->CreateRenderTexture2D(1280, 720, 0, 1, Display::E_Format_A8R8G8B8, Display::E_Multisample_None, 0, &m_NormalRenderTexture))
		return false;

	return true;
}

void C_TestSectionDynamicLights::OnDestroy()
{
	m_Mesh.Destroy();

	Display::Device()->Destroy(&m_ColourTexture);
	Display::Device()->Destroy(&m_DepthTexture);
}

void C_TestSectionDynamicLights::OnDestroyVolatile()
{
	Display::Device()->Destroy(&m_NormalRenderTexture);
}

void C_TestSectionDynamicLights::OnRender3DViewWithAA(const S32 viewIndex)
{
	PERF_EVENT("RenderSectionDynamicLights");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_SkyDome.Render();
	m_Mesh.Render();
}

void C_TestSectionDynamicLights::OnRender3DViewWithoutAA(const S32 viewIndex)
{
	// resolve colour and depth buffers
#ifdef PLATFORM_XENON
	Display::Device()->Resolve(Display::E_Resolve_Colour, Render::g_pColourBuffer);
	Display::Device()->Resolve(Display::E_Resolve_DepthStencil, Render::g_pDepthStencilBuffer);
#endif

	Maths::HMat44 mTexture;
	{
		Display::C_Matrix mTemp;
		Display::MatrixSetIdentity(&mTemp);
		mTemp._11 = +2.0f;
		mTemp._22 = -2.0f;
		mTemp._41 = -1.0f;
		mTemp._42 = +1.0f;
		mTexture = Maths::Load<Maths::HMat44>(&mTemp);
	}
	const Maths::HMat44 mDeviceToWorld = Display::Device()->GetViewSettings3D().GetWorldToDevice().Inverse();

	MathsStorage::Float4 vDepthScale = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	Display::C_Matrix mToWorld;
	Maths::Store(mTexture * mDeviceToWorld, &mToWorld);

	// create a normal texture
	m_Depth.WorldNormal(&m_NormalRenderTexture, Render::g_pDepthStencilBuffer, vDepthScale, mToWorld);

	// restore the colour buffer
	Display::Device()->SetDepthEnable(Display::E_Bool_False);
	Display::BasicRender()->DrawTextured2DRect(0.0f, 0.0f, 1024.0f, 768.0f, Maths::RGBA(0xFFFFFFFF), Render::g_pColourBuffer);
	Display::Device()->SetDepthEnable(Display::E_Bool_Default);

	if (g_debugTestSectionDynamicLightsEnable0)
	{
		const MathsStorage::Float3 vPosition(g_debugTestSectionDynamicLightsPosition0.Value().vx, g_debugTestSectionDynamicLightsPosition0.Value().vy, g_debugTestSectionDynamicLightsPosition0.Value().vz);
		const F32 fRadius = g_debugTestSectionDynamicLightsRadius0;
		const MathsStorage::Float3 vColour(g_debugTestSectionDynamicLightsColour0.Value().r, g_debugTestSectionDynamicLightsColour0.Value().g, g_debugTestSectionDynamicLightsColour0.Value().b);

		m_DynamicLights.AddLight(vPosition, fRadius, vColour);

		if (g_debugTestSectionDynamicLightsRender)
			Display::BasicRender()->DrawColoured3DSphere(Maths::HVec3(vPosition.x, vPosition.y, vPosition.z), fRadius, Maths::RGBA(0xFF0000FF));
	}

	if (g_debugTestSectionDynamicLightsEnable1)
	{
		const MathsStorage::Float3 vPosition(g_debugTestSectionDynamicLightsPosition1.Value().vx, g_debugTestSectionDynamicLightsPosition1.Value().vy, g_debugTestSectionDynamicLightsPosition1.Value().vz);
		const F32 fRadius = g_debugTestSectionDynamicLightsRadius1;
		const MathsStorage::Float3 vColour(g_debugTestSectionDynamicLightsColour1.Value().r, g_debugTestSectionDynamicLightsColour1.Value().g, g_debugTestSectionDynamicLightsColour1.Value().b);

		m_DynamicLights.AddLight(vPosition, fRadius, vColour);

		if (g_debugTestSectionDynamicLightsRender)
			Display::BasicRender()->DrawColoured3DSphere(Maths::HVec3(vPosition.x, vPosition.y, vPosition.z), fRadius, Maths::RGBA(0xFF0000FF));
	}

	if (g_debugTestSectionDynamicLightsEnable2)
	{
		const MathsStorage::Float3 vPosition(g_debugTestSectionDynamicLightsPosition2.Value().vx, g_debugTestSectionDynamicLightsPosition2.Value().vy, g_debugTestSectionDynamicLightsPosition2.Value().vz);
		const F32 fRadius = g_debugTestSectionDynamicLightsRadius2;
		const MathsStorage::Float3 vColour(g_debugTestSectionDynamicLightsColour2.Value().r, g_debugTestSectionDynamicLightsColour2.Value().g, g_debugTestSectionDynamicLightsColour2.Value().b);

		m_DynamicLights.AddLight(vPosition, fRadius, vColour);

		if (g_debugTestSectionDynamicLightsRender)
			Display::BasicRender()->DrawColoured3DSphere(Maths::HVec3(vPosition.x, vPosition.y, vPosition.z), fRadius, Maths::RGBA(0xFF0000FF));
	}

	if (g_debugTestSectionDynamicLightsEnable3)
	{
		const MathsStorage::Float3 vPosition(g_debugTestSectionDynamicLightsPosition3.Value().vx, g_debugTestSectionDynamicLightsPosition3.Value().vy, g_debugTestSectionDynamicLightsPosition3.Value().vz);
		const F32 fRadius = g_debugTestSectionDynamicLightsRadius3;
		const MathsStorage::Float3 vColour(g_debugTestSectionDynamicLightsColour3.Value().r, g_debugTestSectionDynamicLightsColour3.Value().g, g_debugTestSectionDynamicLightsColour3.Value().b);

		m_DynamicLights.AddLight(vPosition, fRadius, vColour);

		if (g_debugTestSectionDynamicLightsRender)
			Display::BasicRender()->DrawColoured3DSphere(Maths::HVec3(vPosition.x, vPosition.y, vPosition.z), fRadius, Maths::RGBA(0xFF0000FF));
	}

	m_DynamicLights.SetDepthTexture(Render::g_pDepthStencilBuffer);
	m_DynamicLights.SetNormalTexture(&m_NormalRenderTexture.GetTexture());
	m_DynamicLights.Render();

	Util::C_String str;
	str.Printf("Dynamic Lights Section : %.2f fps", Framework::C_Application::GetInstance()->GetTimer().GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}
