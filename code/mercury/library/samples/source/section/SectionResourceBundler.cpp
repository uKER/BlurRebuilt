#include "StdAfx.h"

#include <Samples/Section/SectionResourceBundler.h>

namespace Samples
{

C_SectionResourceBundler::C_SectionResourceBundler(const Util::C_String& name) : C_Section(name)
{
	m_TextureIndex	= -1;
	m_BundleName	= "";
	m_pBundle		= new Display::ResourceBundler::C_ResourceBundleSingle();
}

C_SectionResourceBundler::~C_SectionResourceBundler()
{
	SAFE_DELETE_PTR(m_pBundle);
}

bool C_SectionResourceBundler::OnCreate()
{
	RESOURCE_MEMORY_ID("Bundles");

	#if defined(PLATFORM_PC)
	m_BundleName = "./Bundles/ui_frontend.bpr";
	#elif defined(PLATFORM_PS3)
	m_BundleName = "./Bundles/ui_frontend.bpr";
	#endif

	if(!m_pBundle->Create(m_BundleName))
	{
		LOGERROR(g_Log, "Failed to open file '%s'\n", m_BundleName.Chars());
		return false;
	}

	m_TextureIndex = 0;

	return true;
}

void C_SectionResourceBundler::OnDestroy()
{
	m_pBundle->Destroy();
}

void C_SectionResourceBundler::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionResourceBundler");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(160, 160, 160, 255));

	const S32 textureCount = m_pBundle->GetTexture2DCount();

	if(input.IsButtonDn(E_Button_DnPad) || input.IsKeyDn(34))
	{
		m_TextureIndex--;
		if(m_TextureIndex < 0)
			m_TextureIndex = 0;
	}
	else
	if(input.IsButtonDn(E_Button_UpPad) || input.IsKeyDn(33))
	{
		m_TextureIndex++;
		if(m_TextureIndex > textureCount-1)
			m_TextureIndex = textureCount-1;
	}

	Display::C_Texture2D* pTexture = m_pBundle->GetTexture2D(m_TextureIndex);
	
	if(pTexture)
	{
		if(pTexture->IsCreated())
		{
			Display::BasicRender()->DrawTextured2DRect(0, 0, 1024, 768, Display::WHITE, pTexture);

			Util::C_String str;
			str.Printf("%i/%i : %s", m_TextureIndex+1, textureCount, pTexture->GetFileName().Chars());
			Display::FontRender()->Draw(str.Chars(), 40.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 255, 255, 255));
		}
		else
		{
			Util::C_String str;
			str.Printf("Texture '%i' not created in bundle '%s'", m_TextureIndex, m_BundleName.Chars());
			Display::FontRender()->Draw(str.Chars(), 40.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 0, 0, 255));
		}
	}
	else
	{
		Util::C_String str;
		str.Printf("Texture '%i' not found in bundle '%s'", m_TextureIndex, m_BundleName.Chars());
		Display::FontRender()->Draw(str.Chars(), 40.f, 45.f, 20.f, 20.f, Maths::RGBA(255, 0, 0, 255));
	}
}

}; // namespace Samples
