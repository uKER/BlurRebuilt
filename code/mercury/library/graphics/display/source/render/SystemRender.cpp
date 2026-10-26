#include "stdafx.h"
#include "Display/Render/SystemRender.h"
#include "Display/Render/BasicRender.h"
#include "Display/Render/FontRender.h"
#include "Display/Device/Device.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

Debug::Int g_debugDisplaySystemRenderOffset("Graphics.Display.Render.SystemRender.Offset", 0, Debug::Range(0, 255));
Debug::Int g_debugDisplaySystemRenderTransparency("Graphics.Display.Render.SystemRender.Transparency", 220, Debug::Range(0, 255));

namespace Display
{

C_SystemRender::C_SystemRender()
{
	assert(g_pSystemRender == NULL);
	g_pSystemRender = this;
}

C_SystemRender::~C_SystemRender()
{
	assert(g_pSystemRender == this);
	g_pSystemRender = NULL; 
}

void C_SystemRender::Draw()
{
	BasicRender()->DrawColoured2DRect(20.f, 20.f, 984.f, 728.f, Maths::RGBA(0, 0, 0, g_debugDisplaySystemRenderTransparency));
	FontRender()->Draw(":: System Usage ::", 50.f, 60.f, 20.f, 20.f, GREEN);

#ifdef USE_DISPLAY_MEMORY_TRACKER
	F32 x = 70.f;
	F32 y = 110.f;

	const S32 systemCount = Device()->MemoryGetSystemCount();
	
	F32 w = 1024.f - 30.f;
	F32 h = 50.f + 25.f * (F32)systemCount;
	
	char temp[256];
	sprintf_s(temp, sizeof(temp), "Total systems %i", systemCount);
	FontRender()->Draw(temp, x, y, 20, 20, Maths::RGBA(255, 255, 255, 255));
	y += 30.f;

	F32 bigSize = 0.f;
	for(S32 systemID = 0; systemID < systemCount; systemID++)
	{
		C_MemorySystem* pSystem = Device()->MemoryGetSystem(systemID);

		const F32 size	= (F32)pSystem->m_Size[Display::E_MemoryType_Total] / (1024.f * 1024.f);
		if(size > bigSize)
			bigSize = size;
	}

	for(S32 systemID = g_debugDisplaySystemRenderOffset; systemID < systemCount; systemID++)
	{
		C_MemorySystem* pSystem = Device()->MemoryGetSystem(systemID);

		const F32 size = (F32)pSystem->m_Size[Display::E_MemoryType_Total] / (1024.f * 1024.f);
		if(size < 0.01)
			continue;
		
		x = 200.f;
		const Util::C_String& name = pSystem->m_Name;
		sprintf_s(temp, sizeof(temp), "%s =", name.Chars());
		FontRender()->Draw(temp, x, y, 15, 15, Maths::RGBA(255, 255, 255, 255), E_FontRenderAlign_Rt);
		x += 70.f;

		const S32 count	= pSystem->m_Count;
		sprintf_s(temp, sizeof(temp), "%i = ", count);
		FontRender()->Draw(temp, x, y, 15, 15, Maths::RGBA(255, 255, 255, 255), E_FontRenderAlign_Rt);
		x += 5.f;

		const F32 barSize = 300.f;
		w  = 1.f + (size / bigSize) * barSize;
		BasicRender()->DrawColoured2DRect(x, y - 7.5f, w, 15.f, Maths::RGBA(0, 255, 0, 255));
		x += 5.f + w;

		sprintf_s(temp, sizeof(temp), "%.2fmb", size);
		
		bool addBrackets = false;
		for(U32 type = 1; type < E_MemoryType_Count; type++)
		{
			if((F32)pSystem->m_Size[type] >= (0.1f * 1024.f * 1024.f))
			{
				addBrackets = true;
			}
		}

		if(addBrackets)
			sprintf_s(temp, sizeof(temp), "%s (", temp);

		for(U32 type = 1; type < Display::E_MemoryType_Count; type++)
		{
			if((F32)pSystem->m_Size[type] >= (0.1f * 1024.f * 1024.f))
			{
				switch(type)
				{
				case E_MemoryType_Surface:		sprintf_s(temp, sizeof(temp), "%s s%.1f",	temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
				case E_MemoryType_Texture2D:	sprintf_s(temp, sizeof(temp), "%s t%.1f",	temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
				case E_MemoryType_Texture3D:	sprintf_s(temp, sizeof(temp), "%s v%.1f",	temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
				case E_MemoryType_TextureCube:	sprintf_s(temp, sizeof(temp), "%s c%.1f",	temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
				case E_MemoryType_VertexBuffer: sprintf_s(temp, sizeof(temp), "%s vb%.1f",	temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
				case E_MemoryType_IndexBuffer:	sprintf_s(temp, sizeof(temp), "%s ib%.1f",	temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
				case E_MemoryType_VertexShader:	sprintf_s(temp, sizeof(temp), "%s vs%.1f",	temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
				case E_MemoryType_PixelShader:	sprintf_s(temp, sizeof(temp), "%s ps%.1f",	temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
				}
			}
		}

		if(addBrackets)
			sprintf_s(temp, sizeof(temp), "%s )", temp);

		FontRender()->Draw(temp, x, y, 15, 15, Maths::RGBA(255, 255, 255, 255));

		y += 25.f;	
	}
#endif
}

C_SystemRender* g_pSystemRender = NULL;

void SystemRender(C_SystemRender* pSystemRender)
{
	g_pSystemRender = pSystemRender;
}

}; // namespace Display
