#include "stdafx.h"
#include "Display/Render/MemoryRender.h"
#include "Display/Render/BasicRender.h"
#include "Display/Render/FontRender.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

Debug::Int g_debugDisplayMemoryRenderTransparency("Graphics.Display.Render.MemoryRender.Transparency", 64, Debug::Range(0, 255));

namespace Display
{

C_MemoryRender::C_MemoryRender()
{
	assert(g_pMemoryRender == NULL);
	g_pMemoryRender = this;
}

C_MemoryRender::~C_MemoryRender()
{
	assert(g_pMemoryRender == this);
	g_pMemoryRender = NULL; 
}

void C_MemoryRender::DrawManager(const Display2::C_MemoryManager* pMemoryManager, const F32 x, F32& y, const F32 size, const Util::C_String& name)
{
	BIZ_ASSERT(pMemoryManager);

	const Display2::C_MemoryTracker* pMemoryTracker = pMemoryManager->GetTracker();

	BIZ_ASSERT(pMemoryTracker);

	pMemoryManager->Lock();

	Util::C_String str;
	str.Printf("%s %.4f used %.4f free %.4f",
		name.Chars(),
		((F32)pMemoryManager->GetSize())/(1024.0f*1024.0f),
		((F32)pMemoryManager->GetUsed())/(1024.0f*1024.0f),
		((F32)pMemoryManager->GetFree())/(1024.0f*1024.0f));
	FontRender()->Draw(str.Chars(), x, y, size, size, WHITE);
	y += size*1.1f;

	for(U32 i=0; i<Display2::E_MemoryType_Count; i++)
	{
		const Display2::C_MemoryInfo& memoryInfo = pMemoryTracker->m_Info[i];

		F32 x1 = x;

		str.Printf(" %.4f", ((F32)memoryInfo.m_Size)/(1024.0f*1024.0f));
		FontRender()->Draw(str.Chars(), x1, y, size, size, WHITE);
		x1 += size*6.0f;

		str.Printf("%i", memoryInfo.m_Count);
		FontRender()->Draw(str.Chars(), x1, y, size, size, WHITE);
		x1 += size*3.0f;

		str.Printf("%s", pMemoryTracker->GetNameFromType(i).Chars());
		FontRender()->Draw(str.Chars(), x1, y, size, size, WHITE);
		y += size*1.1f;
	}
	y += size*1.1f;

	pMemoryManager->Unlock();
}

void C_MemoryRender::Draw(const F32 offset)
{
	BasicRender()->DrawColoured2DRect(20.f, 20.f, 984.f, 728.f, Maths::RGBA(0, 0, 0, g_debugDisplayMemoryRenderTransparency));

#if USE_DISPLAY2_MEMORY_TRACKING
	{
		F32 x = 50.0f;
		F32 y = 50.0f;
		F32 s = 16.0f;

	#if PLATFORM_PS3
		DrawManager(Display2::Device()->GetLocalHeap(), x, y, s, "LocalHeap");
		DrawManager(Display2::Device()->GetMainHeap(), x, y, s, "MainHeap");
		DrawManager(Display2::Device()->GetCmdBufHeap(), x, y, s, "CmdBufHeap");
	#endif
	}
#endif

#if 0//def USE_DISPLAY_MEMORY_TRACKER
	{
		FontRender()->Draw(":: Memory Usage ::", 50.f, 60.f, 20.f, 20.f, GREEN);

		F32 x = 70.f;
		F32 y = 110.f;
		F32 w = 20.f;
		F32 h = 20.f;

		char name[64];
		char size[64];

		if(Device()->MemoryGetSystemCount() > 0)
		{
			C_MemorySystem* pSystem = Device()->MemoryGetSystem(0);

			for(U32 i = 0; i < E_MemoryType_Count; i++)
			{
				if(pSystem->m_Size[i] > 0)
				{
					switch(i)
					{
					case E_MemoryType_Total:
						sprintf_s(name, 64,"Total");
						break;
					case E_MemoryType_Surface:
						sprintf_s(name, 64,"Surface");
						break;
					case E_MemoryType_Texture2D:
						sprintf_s(name, 64,"Texture2D");
						break;
					case E_MemoryType_Texture3D:
						sprintf_s(name, 64,"Texture3D");
						break;
					case E_MemoryType_TextureCube:
						sprintf_s(name, 64,"TextureCube");
						break;
					case E_MemoryType_VertexBuffer:
						sprintf_s(name, 64,"VertexBuffer");
						break;
					case E_MemoryType_IndexBuffer:
						sprintf_s(name, 64,"IndexBuffer");
						break;
					case E_MemoryType_VertexShader:
						sprintf_s(name, 64,"VertexShader");
						break;
					case E_MemoryType_PixelShader:
						sprintf_s(name, 64,"PixelShader");
						break;
					default:
						sprintf_s(name, 64,"Unknown");
						break;
					}
					sprintf_s(size, 64,"%.4fmb = ", (F32)pSystem->m_Size[i] / (1024.f * 1024.f));
					FontRender()->Draw(size, 300, y, w, h, WHITE, Display::E_FontRenderAlign_Rt);
					FontRender()->Draw(name, 310, y, w, h, WHITE, Display::E_FontRenderAlign_Lt);
					y += h + 10.f;
				}
			}
		}
	}
#endif
}

C_MemoryRender* g_pMemoryRender = NULL;

void MemoryRender(C_MemoryRender* pMemoryRender)
{
	g_pMemoryRender = pMemoryRender;
}

}; // namespace Display
