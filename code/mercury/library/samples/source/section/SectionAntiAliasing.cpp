#include "StdAfx.h"
#include "Display/Render/Render.h"
#include "Render/Mesh/MeshLoader.h"
#include "Render/Shaders/psConstants.h"
#include "Render/Shaders/vsConstants.h"
#include "Samples/Section/SectionAntiAliasing.h"

#if defined(DEVICE_PS3GCM)
#include "Render/Shaders/Effects/PS3/psAntiAliasing.shader"
#endif

#if defined(CFG_ALLOW_OPTIMIZE_OFF)
#pragma optimize("", off)
#endif

namespace Samples
{

Debug::Float g_debugAAEdgeDepth("Section.AntiAliasing.EdgeDepth", 0.5f, Debug::Range(0.0f, 10.0f));
Debug::Float g_debugAATheta("Section.AntiAliasing.Theta", 0.1f, Debug::Range(0.0f, 0.5f * Maths::Pi));
Debug::Bool g_debugAA("Section.AntiAliasing.On", true);

C_SectionAntiAliasing::C_SectionAntiAliasing(const Util::C_String& name) : C_Section(name)
{
}

C_SectionAntiAliasing::~C_SectionAntiAliasing()
{
}

bool C_SectionAntiAliasing::OnCreate()
{
	if (!Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		return false;

	m_Mesh.AddInstance();

	if (!Display::CreatePixelShaderFromFunc("Shaders/Effects/psAntiAliasing", 0, &m_PixelShader, sizeof(psAntiAliasing), psAntiAliasing))
		return false;

	return true;
}

void C_SectionAntiAliasing::OnDestroy()
{
	Display::Device()->Destroy(&m_PixelShader);

	m_Mesh.Destroy();
}

void C_SectionAntiAliasing::OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionAntiAliasingWithAA");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_SkyDome.Render();
	m_Mesh.Render();
}

void C_SectionAntiAliasing::OnRenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionAntiAliasingWithoutAA");

	if (g_debugAA)
	{
		const Display::C_ViewSettings3D & viewSettings = Display::Device()->GetViewSettings3D();

		MathsStorage::Float4 vDepthScale;
		vDepthScale.x = viewSettings.GetFarClip() * viewSettings.GetNearClip() / (viewSettings.GetNearClip() - viewSettings.GetFarClip());
		vDepthScale.y = viewSettings.GetFarClip() / (viewSettings.GetFarClip() - viewSettings.GetNearClip());
		vDepthScale.z = viewSettings.GetFarClip();

		const F32 fSinTheta = sinf(g_debugAATheta);
		const F32 fCosTheta = cosf(g_debugAATheta);
		const F32 fPixelWidth = 1.0f / (F32) Display::Device()->GetWindow(0)->m_Width;
		const F32 fPixelHeight = 1.0f / (F32) Display::Device()->GetWindow(0)->m_Height;

		MathsStorage::Float4 vSamples0123 = MathsStorage::Float4(fPixelWidth * fCosTheta + fPixelHeight * fSinTheta, fPixelHeight * fCosTheta - fPixelWidth * fSinTheta, fPixelWidth * fSinTheta + fPixelHeight * fCosTheta, fPixelHeight * fSinTheta - fPixelWidth * fCosTheta);

		if (Display::Device()->GetDepthFlip())
			vDepthScale.y = 1.0f - vDepthScale.y;

		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
		Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);

		Display::Device()->SetTexture(0, &Display::Device()->GetWindow(0)->m_ColourTexture);
		Display::Device()->SetTexture(1, &Display::Device()->GetWindow(0)->m_DepthStencilTexture);

		Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS, Display::C_PSConstant(g_debugAAEdgeDepth), 1);
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS_1, (F32 *) &vDepthScale, 1);
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS_2, (F32 *) &vSamples0123, 1);

		Display::Device()->SetPixelShader(&m_PixelShader);

		Display::Quad::DrawFullScreen();

		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Wrap);
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Wrap);
	}

	// FPS
	Util::C_String str;
	str.Printf("Sky Dome Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}


}; // namespace Samples
