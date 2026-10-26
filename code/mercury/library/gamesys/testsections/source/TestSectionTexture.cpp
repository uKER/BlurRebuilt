#include "stdafx.h"
#include "TestSections/TestSectionTexture.h"
#include <Render/Sky/SkyDome.h>
#include <Render/Render/GridRender.h>

namespace Framework
{

Debug::Bool g_ReloadableTextures("Graphics.Render.ReloadableTextures", false);

C_TestSectionTexture::C_TestSectionTexture(const Util::C_String& name) : C_TestSection(name)
{
	m_pSkyDome = new Render::Sky::C_SkyDome();
}

C_TestSectionTexture::~C_TestSectionTexture()
{
	SAFE_DELETE_PTR(m_pSkyDome);
}

bool C_TestSectionTexture::OnCreate()
{
	if(!Display::CreateTexture2DFromFile("Textures/Tests/ghost_mask.dds", &m_Texture2D_L8))
		return false;
	//if(!Display::CreateTexture2DFromFile("Textures/Tests/Test.dds", &m_Texture2D_A8R8G8B8))
	if(!Display::CreateTexture2DFromFile("Textures/Tests/message_script.tga", &m_Texture2D_A8R8G8B8))
		return false;
	if(!Display::CreateTexture3DFromFile("Textures/Tests/Amboy_01_LUT.dds", &m_Texture3D_A8R8G8B8))
		return false;
	if(!Display::CreateTextureCubeFromFile("Textures/Tests/ShowRoom.dds", &m_TextureCube_DXT1))
		return false;

	return true;
}

void C_TestSectionTexture::OnDestroy()
{
	Display::Device()->Destroy(&m_Texture2D_L8);
	Display::Device()->Destroy(&m_Texture2D_A8R8G8B8);
	Display::Device()->Destroy(&m_Texture3D_A8R8G8B8);
	Display::Device()->Destroy(&m_TextureCube_DXT1);
}

void C_TestSectionTexture::OnUpdate()
{
	if(g_ReloadableTextures)
		Display::RefreshTextures("user\\textures\\");
}

void C_TestSectionTexture::OnRender3DViewWithAA(const S32 viewIndex)
{
	m_pSkyDome->Render();

	Render::DrawGrid();

	F32 x = 50.0f;
	F32 y = 50.0f;
	F32 w = 150.0f;
	F32 h = 150.0f;
	{
		PERF_EVENT("Texture2D_L8");
		Display::BasicRender()->DrawTextured2DRect(x, y, w, h, Maths::RGBA(255, 255, 255, 255), &m_Texture2D_L8);
		y += h;
	}
	{
		PERF_EVENT("Texture2D_A8R8G8B8");
		Display::BasicRender()->DrawTextured2DRect(x, y, w, h, Maths::RGBA(255, 255, 255, 255), &m_Texture2D_A8R8G8B8);
		y += h;
	}
#if 1//!PLATFORM_PC
	{
		PERF_EVENT("Texture3D_A8R8G8B8");
		Display::BasicRender()->DrawTextured2DRect(x, y, w, h, Maths::RGBA(255, 255, 255, 255), &m_Texture3D_A8R8G8B8);
		y += h;
	}
	{
		PERF_EVENT("TextureCube_DXT1");
		Display::BasicRender()->DrawTextured2DRect(x, y, w, h, Maths::RGBA(255, 255, 255, 255), &m_TextureCube_DXT1);
		y += h;
	}
#endif
}

}; // namespace Framework
