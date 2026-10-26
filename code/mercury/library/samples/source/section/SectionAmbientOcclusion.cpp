#include "StdAfx.h"
#include "Display/Render/Render.h"
#include "Render/Mesh/MeshLoader.h"
#include "Render/Shaders/psConstants.h"
#include "Render/Shaders/vsConstants.h"
#include "Samples/Section/SectionAmbientOcclusion.h"
#include "System/Misc/Random.h"

#if defined(DEVICE_PS3GCM)
#include "Render/Shaders/Effects/PS3/psAmbientOcclusion.shader"
#endif

#if defined(CFG_ALLOW_OPTIMIZE_OFF)
#pragma optimize("", off)
#endif

namespace Samples
{

Debug::Float g_debugAORadius("Section.AmbientOcclusion.Radius", 0.20f);
Debug::Float g_debugAOSubtract("Section.AmbientOcclusion.Subtract", 0.50f);
Debug::Float g_debugAOPower("Section.AmbientOcclusion.Power", 1.0f);
Debug::Float g_debugAONoiseScale("Section.AmbientOcclusion.NoiseScale", 8.0f);
Debug::Colour g_debugAOColour("Section.AmbientOcclusion.Colour", 1.0f, 1.0f, 1.0f, 1.0f);

C_SectionAmbientOcclusion::C_SectionAmbientOcclusion(const Util::C_String& name) : C_Section(name)
{
	const F32 c_RcpSqrt3 = 0.57735026918962576450914878050196f;

	m_f4SamplePositions[0] = MathsStorage::Float4(-0.875f * c_RcpSqrt3, -0.875f * c_RcpSqrt3, -0.875f * c_RcpSqrt3, 1.0f);
	m_f4SamplePositions[1] = MathsStorage::Float4(+0.250f * c_RcpSqrt3, +0.250f * c_RcpSqrt3, +0.250f * c_RcpSqrt3, 1.0f);
	m_f4SamplePositions[2] = MathsStorage::Float4(-0.375f * c_RcpSqrt3, -0.375f * c_RcpSqrt3, +0.375f * c_RcpSqrt3, 1.0f);
	m_f4SamplePositions[3] = MathsStorage::Float4(+0.750f * c_RcpSqrt3, +0.750f * c_RcpSqrt3, -0.750f * c_RcpSqrt3, 1.0f);
	m_f4SamplePositions[5] = MathsStorage::Float4(-1.000f * c_RcpSqrt3, +1.000f * c_RcpSqrt3, +1.000f * c_RcpSqrt3, 1.0f);
	m_f4SamplePositions[6] = MathsStorage::Float4(+0.625f * c_RcpSqrt3, -0.625f * c_RcpSqrt3, -0.625f * c_RcpSqrt3, 1.0f);
	m_f4SamplePositions[7] = MathsStorage::Float4(-0.500f * c_RcpSqrt3, +0.500f * c_RcpSqrt3, -0.500f * c_RcpSqrt3, 1.0f);
	m_f4SamplePositions[8] = MathsStorage::Float4(+0.125f * c_RcpSqrt3, -0.125f * c_RcpSqrt3, +0.125f * c_RcpSqrt3, 1.0f);
}

C_SectionAmbientOcclusion::~C_SectionAmbientOcclusion()
{
}

bool C_SectionAmbientOcclusion::OnCreate()
{
	if (!Render::Mesh::LoadMesh("Meshes/Test", &m_Mesh))
		return false;

	m_Mesh.AddInstance();

	if (!Display::CreatePixelShaderFromFunc("Shaders/Effects/psAmbientOcclusion", 0, &m_PixelShader, sizeof(psAmbientOcclusion), psAmbientOcclusion))
		return false;

	if (!Display::Device()->CreateTexture3D(32, 32, 32, 1, 0, Display::E_Format_A8R8G8B8, &m_Texture3D))
		return false;


	return true;
}

void C_SectionAmbientOcclusion::OnDestroy()
{
	Display::Device()->Destroy(&m_Texture3D);
	Display::Device()->Destroy(&m_PixelShader);

	m_Mesh.Destroy();
}

void C_SectionAmbientOcclusion::OnRenderGameViewWithAA(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionAmbientOcclusionWithAA");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(0, 75, 150, 225));

	m_SkyDome.Render();
	m_Mesh.Render();
}

void C_SectionAmbientOcclusion::OnRenderGameViewWithoutAA(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionAmbientOcclusionWithoutAA");

	// render occlusion to small render target
	const Display::C_ViewSettings3D & viewSettings = Display::Device()->GetViewSettings3D();

	MathsStorage::Float4 f4DepthScale;
	f4DepthScale.x = viewSettings.GetFarClip() * viewSettings.GetNearClip() / (viewSettings.GetNearClip() - viewSettings.GetFarClip());
	f4DepthScale.y = viewSettings.GetFarClip() / (viewSettings.GetFarClip() - viewSettings.GetNearClip());
	f4DepthScale.z = viewSettings.GetFarClip();

	if(Display::Device()->GetDepthFlip())
		f4DepthScale.y = 1.0f - f4DepthScale.y;

	Display::C_Matrix matProj, matInvProj, matInvViewProj;
	Maths::Store(Display::Device()->GetViewSettings3D().m_CameraToDevice, &matProj);
	Maths::Store(Display::Device()->GetViewSettings3D().m_CameraToDevice.Inverse(), &matInvProj);
	Maths::Store(Display::Device()->GetViewSettings3D().m_WorldToDevice.Inverse(), &matInvViewProj);

	// render a full screen quad to calculate the ambient occlusion
	Display::Device()->SetBlendMode(Display::E_BlendMode_Subtractive);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Wrap);
	
	Display::Device()->SetTexture(0, &Display::Device()->GetWindow(0)->m_DepthStencilTexture);
	Display::Device()->SetTexture(1, &m_Texture3D);

	Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS, (F32 *) &matInvProj, 4);
	Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS_4, Display::C_PSConstant(viewSettings.m_ProjMatrix._11, viewSettings.m_ProjMatrix._22), 1);
	Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS_5, (F32 *) &f4DepthScale, 1);
	Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS_6, (F32 *) &g_debugAOColour.Value(), 1);
	Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS_7, Display::C_PSConstant(g_debugAORadius, g_debugAOSubtract, g_debugAOPower, g_debugAONoiseScale), 1);
	Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS_8, (F32 *) m_f4SamplePositions, 8);

	Display::Device()->SetPixelShader(&m_PixelShader);

	Display::Quad::DrawFullScreen();

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Wrap);

	// FPS
	Util::C_String str;
	str.Printf("Sky Dome Section : %.2f fps", timer.GetFrameRate());
	Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
}


}; // namespace Samples
