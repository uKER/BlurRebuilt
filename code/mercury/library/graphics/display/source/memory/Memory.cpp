#include "stdafx.h"
#include <core\containers\sortedbinarytree.h>
#include "Display/Memory/Memory.h"
#include "Display/Render/FontRender.h"
#include "Display/Shaders/Constants.h"
#include <algorithm>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

namespace Display
{

Debug::Int g_debugFontX("System.Memory.OutOfMemory.FontX", 30, Debug::Range(0, 1024));
Debug::Int g_debugFontY("System.Memory.OutOfMemory.FontY", 50, Debug::Range(0, 768));
Debug::Int g_debugFontW("System.Memory.OutOfMemory.FontW", 32, Debug::Range(0, 128));
Debug::Int g_debugFontH("System.Memory.OutOfMemory.FontH", 32, Debug::Range(0, 128));

C_MemoryManager::C_MemoryManager()
{
}

C_MemoryManager::~C_MemoryManager()
{
}

void C_MemoryManager::OutOfMemory(const E_Memory memory)
{
	if(!DeviceExists())
		return;

	const bool wasInScene = Device()->InScene();

	if(!wasInScene)
		Device()->BeginScene();

	Device()->SetColourDepthStencilSurface();
	
	Device()->Clear(E_Clear_Colour | E_Clear_Depth, Maths::RGBA(0, 0, 0, 0));

	const Display::C_Float4 clipTransform(0.001953f, -0.002604f, -1.000000f, 1.000000f);
	Device()->SetVertexShaderConstantF(CV_DISPLAY_CLIP_VECTOR, (F32*)&clipTransform, 1);

	F32 x = (F32)g_debugFontX;
	F32 y = (F32)g_debugFontY;
	F32 w = (F32)g_debugFontW;
	F32 h = (F32)g_debugFontH;

	switch(memory)
	{
	case E_Memory_System:
		FontRender()->Draw("!!! OUT OF SYSTEM MEMORY !!!", x, y, w, h, Maths::RGBA(255, 0, 0, 255));
		break;
	case E_Memory_Graphics:
		FontRender()->Draw("!!! OUT OF GRAPHICS MEMORY !!!", x, y, w, h, Maths::RGBA(255, 0, 0, 255));
		break;
	}

	Device()->EndScene();

	Device()->Present();

	DebugBreak();
	
	if(wasInScene)
		Device()->BeginScene();
}

void PrintTexture2Ds()
{
}

void OutOfMemory()
{
	LOGWARN(g_Log, "\n!!! OUT OF GRAPHICS MEMORY !!!\n");

	if(Device())
	{
		#ifdef USE_DISPLAY_MEMORY_TRACKER
		Device()->MemoryPrintSystems();
		#endif

		PrintTexture2Ds();

		const bool wasInScene = Device()->InScene();

		if(!wasInScene)
			Device()->BeginScene();

		Device()->SetColourSurface();
		Device()->SetDepthStencilSurface();
		Device()->Clear(E_Clear_Colour | E_Clear_Depth, Maths::RGBA(0, 0, 0, 0));

		FontRender()->Draw("!!! OUT OF GRAPHICS MEMORY !!!", 30.f, 50.f, 32.f, 32.f, Maths::RGBA(255, 0, 0, 255));

		#ifdef USE_DISPLAY_MEMORY_TRACKER
		{
			F32 y = 150.f;
			F32 w = 30.f;
			F32 h = 30.f;

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
							sprintf_s(name, 64, "Total");
							break;
						case E_MemoryType_Surface:
							sprintf_s(name, 64, "Surface");
							break;
						case E_MemoryType_Texture2D:
							sprintf_s(name, 64, "Texture2D");
							break;
						case E_MemoryType_Texture3D:
							sprintf_s(name, 64, "Texture3D");
							break;
						case E_MemoryType_TextureCube:
							sprintf_s(name, 64, "TextureCube");
							break;
						case E_MemoryType_VertexBuffer:
							sprintf_s(name, 64, "VertexBuffer");
							break;
						case E_MemoryType_IndexBuffer:
							sprintf_s(name, 64, "IndexBuffer");
							break;
						case E_MemoryType_VertexShader:
							sprintf_s(name, 64, "VertexShader");
							break;
						case E_MemoryType_PixelShader:
							sprintf_s(name, 64, "PixelShader");
							break;
						default:
							sprintf_s(name, 64, "Unknown");
							break;
						}
						sprintf_s(size, 64, "%.4fmb = ", (F32)pSystem->m_Size[i] / (1024.f * 1024.f));
						FontRender()->Draw(size, 420, y, w, h, WHITE, Display::E_FontRenderAlign_Rt);
						FontRender()->Draw(name, 430, y, w, h, WHITE, Display::E_FontRenderAlign_Lt);
						y += h + 10.f;
					}
				}
			}
		}
		#endif

		Device()->EndScene();
		Device()->Present();

		if(wasInScene)
			Device()->BeginScene();
	}
	else
	{
		LOGFATAL(g_Log, "No device exists...everything is fubar!\n");
	}

	//LOGFATAL(g_Log, "\n!!! OUT OF GRAPHICS MEMORY !!!\n");
	LOGERROR(g_Log, "\n!!! OUT OF GRAPHICS MEMORY !!!\n");

#ifndef CFG_SPEED
	DebugBreak();
#endif
}

}; // namespace Display
