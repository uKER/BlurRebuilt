#include "DebugSystemsPch.h"
#include <DebugSystems/DebugHUDResource.h>
#include <Render/font/Font.h>
#include <Render/debug/DebugRender.h>

Debug::Bool g_StatsShowResourceUsage("System.Resource.ShowResourceUsage", false );
Debug::Int g_ResourceStatsScale("System.Resource.Scale", 96 * 5, Debug::Range(0,1000));
Debug::Float g_ResourceStatsOffsetX("System.Resource.OffsetX", 0.f, Debug::Range(-3000.0f, 800.f));
Debug::Float g_ResourceStatsOffsetY("System.Resource.OffsetY", 0.f, Debug::Range(-3000.0f, 600.f) );
Debug::Bool g_ResourceStatsStack("System.Resource.Stack", false);
Debug::Int g_ResourceFontScale("System.Resource.FontScale", 24, Debug::Range(5, 40));

DebugSystems::C_DebugHUDResource::C_DebugHUDResource()
{
	m_pFontRenderer = NULL;
}

DebugSystems::C_DebugHUDResource::~C_DebugHUDResource()
{
	if (m_pFontRenderer)
		delete m_pFontRenderer;
}


void DebugSystems::C_DebugHUDResource::Render()
{
	if (g_StatsShowResourceUsage)
	{
		PERF_EVENT("DebugHUDMemory.Resource.Render");

		if (m_pFontRenderer == NULL)
		{
			//Create the font renderer
			m_pFontRenderer = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");

			m_pFontRenderer->SetShadowEnable( true );
			m_pFontRenderer->SetColourOutline(Maths::RGBA(0,0,25,255));
			m_pFontRenderer->SetOutlineSize(0.35f);
		}

		//Set Font Size
		m_pFontRenderer->SetScaleX(F32(g_ResourceFontScale));
		m_pFontRenderer->SetScaleY(F32(g_ResourceFontScale));

		const Maths::HVec2 barOffset((F32)g_ResourceStatsOffsetX + 128.f, (F32)g_ResourceStatsOffsetY);
		const F32 spacing = m_pFontRenderer->GetScaleY();
		const F32 pixelSize = (F32)g_ResourceStatsScale;

		F32 drawRange = 0.f;
		
		for (S32 systemID = 0; systemID < Display::Device()->MemoryGetSystemCount(); systemID++)
		{
			const Display::C_MemorySystem* pSystem = Display::Device()->MemoryGetSystem(systemID);
			const F32 size = pSystem->m_Size[Display::E_MemoryType_Total] / (1024.f * 1024.f);
			drawRange = size > drawRange ? size : drawRange;
		}

		F32 start = 0.f;
		/*const F32 s = 50.f;
		Maths::HVec2 tl(s, s);
		Maths::HVec2 br(1024.f - s - s, 768.f - s - s);

		Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Alpha);
		Render::Debug::Immediate()->Draw2dBar(tl, br, Maths::RGBA(0x00, 0x00, 0x00, 0x66));
		Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);*/

		Maths::HVec2 pos(128.f, 128.f);
		for (S32 systemID = 0; systemID <  Display::Device()->MemoryGetSystemCount(); systemID++)
		{
			//if (systemID < g_MemoryStatsShowResourceUsageOffset)
			//	continue;
			
			//Name bar	
			Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Alpha);
			Render::Debug::Immediate()->Draw2dBar( Maths::HVec2(0.f, pos.Y() + barOffset.Y()), Maths::HVec2(pos.X() + barOffset.X() , m_pFontRenderer->GetScaleY() ), Maths::RGBA(0,0,0,80));
			Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);


			//Odd bar
			if (systemID%2)
			{
				Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Alpha);
				Render::Debug::Immediate()->Draw2dBar( Maths::HVec2(0.f, pos.Y() + barOffset.Y()), Maths::HVec2( 1024.f, m_pFontRenderer->GetScaleY() ), Maths::RGBA(0,0,0,80));
				Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);
			}

			const Display::C_MemorySystem* pSystem = Display::Device()->MemoryGetSystem(systemID);
			const F32 size = pSystem->m_Size[Display::E_MemoryType_Total] / (1024.f * 1024.f);
			//if (size < 0.01f)
			//	continue;
			
			Maths::RGBA colour;
			char string[512];
			if (pSystem->m_MaxSize != -1)
			{
				const F32 maxSize = pSystem->m_MaxSize / (1024.f * 1024.f);
				colour = size <= maxSize ? Maths::RGBA(0xFF00FF00) : Maths::RGBA(0xFFFF0000);
				sprintf(string, "%.2f / %.2f MB", size, maxSize);
			}
			else
			{
				colour = Maths::RGBA(0xFF0000FF);
				sprintf(string, "%.2f MB", size);
			}

			bool addBrackets = false;
			for (U32 type = 1; type < Display::E_MemoryType_Count; type++)
			{
				if ((F32)pSystem->m_Size[type] >= (0.1f * 1024.f * 1024.f))
				{
					addBrackets = true;
				}
			}

			if (addBrackets)
				sprintf(string, "%s (", string);

			for (U32 type = 1; type < Display::E_MemoryType_Count; type++)
			{
				if ((F32)pSystem->m_Size[type] >= (0.1f * 1024.f * 1024.f))
				{
					switch (type)
					{
					case Display::E_MemoryType_Surface:		sprintf(string, "%s s%.1f",		string, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
					case Display::E_MemoryType_Texture2D:	sprintf(string, "%s t%.1f",		string, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
					case Display::E_MemoryType_Texture3D:	sprintf(string, "%s v%.1f",		string, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
					case Display::E_MemoryType_TextureCube:	sprintf(string, "%s c%.1f",		string, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
					case Display::E_MemoryType_VertexBuffer: sprintf(string, "%s vb%.1f",	string, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
					case Display::E_MemoryType_IndexBuffer:	sprintf(string, "%s ib%.1f",	string, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
					case Display::E_MemoryType_VertexShader:	sprintf(string, "%s vs%.1f",	string, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
					case Display::E_MemoryType_PixelShader:	sprintf(string, "%s ps%.1f",	string, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
					}
				}
			}

			if (addBrackets)
				sprintf(string, "%s )", string);

			const F32 drawScale = size / drawRange;
			const F32 drawValue = pixelSize * drawScale;
			F32 end = start + drawValue;
			
			Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Alpha);
			Render::Debug::Immediate()->Draw2dBar(pos + barOffset + Maths::HVec2(start, 4.f), Maths::HVec2(end - start, m_pFontRenderer->GetScaleY() - 8.f), colour);    	    
			Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);

			m_pFontRenderer->Print(pos + barOffset + Maths::HVec2(-16.f, 0.f), pSystem->m_Name, Render::Font::E_Justification_Right);
			m_pFontRenderer->Print(pos + barOffset + Maths::HVec2(end + 8.f, 0.f), string);
    		
			pos += Maths::HVec2(0.f, spacing);
    		if (g_ResourceStatsStack && systemID != 0)
    		{
    			start = end;
    		}
		}
	}
}
