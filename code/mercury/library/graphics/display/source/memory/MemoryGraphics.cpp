#include "stdafx.h"
#include "Display/Memory/MemoryGraphics.h"
#include "Display/Device/Device.h"
#include <System/Serialise/TextSerialise.h>
#include <algorithm>
#include <system/debug/log/debuglog.h>
#include <system/debug/log/subscribers/LogFileSubscriber.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

Debug::Bool g_debugForceOutOfGraphicsMemory("System.Memory.ForceOutOfGraphicsMemory", false);

namespace Display
{

OutOfMemoryCallBackFunction* C_MemoryManagerGraphics::m_pOutOfMemoryCallBackFunction = NULL;
Sys::TextSerialiser* C_MemoryManagerGraphics::m_pTextSerialiser = NULL;

C_MemoryManagerGraphics::C_MemoryManagerGraphics()
{
}

C_MemoryManagerGraphics::~C_MemoryManagerGraphics()
{
}

void C_MemoryManagerGraphics::SetOutOfMemoryCallBackFunction(OutOfMemoryCallBackFunction* pFunc)
{
	m_pOutOfMemoryCallBackFunction = pFunc;
}

void C_MemoryManagerGraphics::OutOfMemory()
{
	if(m_pOutOfMemoryCallBackFunction)
	{
		(*m_pOutOfMemoryCallBackFunction)();
	}
	else
	{
		C_MemoryManager::OutOfMemory(E_Memory_Graphics);
	}
}

void C_MemoryManagerGraphics::Update()
{
	if(g_debugForceOutOfGraphicsMemory)
	{
		OutOfMemory();
		g_debugForceOutOfGraphicsMemory = false;
	}
}

void C_MemoryManagerGraphics::DumpUsage(const bool outputSystems, const bool outputTexture2Ds)
{
	if(outputSystems)
		DumpUsageSystem(false);
	if(outputTexture2Ds)
		DumpUsageTexture2D(false);
}

void C_MemoryManagerGraphics::DumpUsage(const Util::C_String &filePath, const bool outputSystems, const bool outputTexture2Ds)
{
	const Util::C_String txtFilePath = filePath + ".txt";
	const Util::C_String csvFilePath = filePath + ".csv";

	// output to txt file
	{
		Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(txtFilePath.Chars(), Sys::E_FileAccess_Write);
		if(pFile)
		{
			m_pTextSerialiser = new Sys::TextSerialiser(*pFile);

			if(outputSystems)
				DumpUsageSystem(false);
			if(outputTexture2Ds)
				DumpUsageTexture2D(false);

			SAFE_DELETE_PTR(m_pTextSerialiser);
			SAFE_DELETE_PTR(pFile);
		}
		else
		{
			LOGERROR(g_Log, "Failed to open file '%s'\n", txtFilePath.Chars());
		}
	}

	// output to csv file
	{
		Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(csvFilePath.Chars(), Sys::E_FileAccess_Write);
		if(pFile)
		{
			m_pTextSerialiser = new Sys::TextSerialiser(*pFile);

			if(outputSystems)
				DumpUsageSystem(true);

			SAFE_DELETE_PTR(m_pTextSerialiser);
			SAFE_DELETE_PTR(pFile);
		}
		else
		{
			LOGERROR(g_Log, "Failed to open file '%s'\n", csvFilePath.Chars());
		}
	}
}

void C_MemoryManagerGraphics::DumpUsageSystem(const bool bCSV)
{
	if(!DeviceExists())
		return;

	if(bCSV)
	{
		Output("---------------------------------------------------------------------------\n");
		Output("Name, Size, Count\n");
	}
	else
	{
		Output("\n:: Graphics Memory Usage ::\n\n");
	}

#ifdef USE_DISPLAY_MEMORY_TRACKER
	char temp[1024];

	const S32 systemCount = Device()->MemoryGetSystemCount();
	for(S32 i=0; i<systemCount; i++)
	{
		C_MemorySystem* pSystem = Device()->MemoryGetSystem(i);

		if(pSystem->m_Size[E_MemoryType_Total] > 0)
		{
			const F32 size = (F32)pSystem->m_Size[E_MemoryType_Total] / (1024.f * 1024.f);

			if(bCSV)
			{
				sprintf_s(temp, sizeof(temp), "%s, %.5f, %i", pSystem->m_Name.Chars(), size, pSystem->m_Count);
			}
			else
			{
				if(pSystem->m_MaxSize != -1)
				{
					const F32 maxSize = (F32)pSystem->m_MaxSize / (1024.f * 1024.f);
					sprintf_s(temp, sizeof(temp), "Size '%.5fmb / %.5fmb'", size, maxSize);
				}
				else
				{
					sprintf_s(temp, sizeof(temp), "Size '%.5fmb'", size);
				}

				sprintf_s(temp, sizeof(temp), "%s Count '%i' Name '%s'", temp, pSystem->m_Count, pSystem->m_Name.Chars());
			}

			if(!bCSV)
			{
				for(U32 type = 1; type < E_MemoryType_Count; type++)
				{
					if((F32)pSystem->m_Size[type] >= (0.1f * 1024.f * 1024.f))
					{
						switch (type)
						{
						case E_MemoryType_Surface:		sprintf_s(temp, sizeof(temp), "%s Surface '%.1fmb'",		temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
						case E_MemoryType_Texture2D:	sprintf_s(temp, sizeof(temp), "%s Texture2D '%.1fmb'",		temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
						case E_MemoryType_Texture3D:	sprintf_s(temp, sizeof(temp), "%s Texture3D '%.1fmb'",		temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
						case E_MemoryType_TextureCube:	sprintf_s(temp, sizeof(temp), "%s TextureCube '%.1fmb'",	temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
						case E_MemoryType_VertexBuffer: sprintf_s(temp, sizeof(temp), "%s VertexBuffer '%.1fmb'",	temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
						case E_MemoryType_IndexBuffer:	sprintf_s(temp, sizeof(temp), "%s IndexBuffer '%.1fmb'",	temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
						case E_MemoryType_VertexShader:	sprintf_s(temp, sizeof(temp), "%s VertexShader '%.1fmb'",	temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
						case E_MemoryType_PixelShader:	sprintf_s(temp, sizeof(temp), "%s PixelShader '%.1fmb'",	temp, (F32)pSystem->m_Size[type] / (1024.f * 1024.f)); break;
						}
					}
				}
			}

			Output("%s\n", temp);
		}
	}
#endif
}

struct C_GraphicsMemoryManagerTexture
{
	U32						m_Size;
	U32						m_Width;
	U32						m_Height;
	Display::E_Format	m_Format;
	S32						m_SystemID;
	char					m_Name[64];
	
	const bool operator<(const C_GraphicsMemoryManagerTexture& other) const
	{
		if(m_SystemID > other.m_SystemID)
			return true;
		else
		if(m_SystemID < other.m_SystemID)
			return false;
		else
		if(m_Size > other.m_Size)
			return true;
		else
		if(m_Size < other.m_Size)
			return false;
		else
			return true;
	}
};

void C_MemoryManagerGraphics::DumpUsageTexture2D(const bool bCSV)
{
}

void C_MemoryManagerGraphics::Output(char* output, ...)
{
	char buffer[1024];
    va_list arglist;
    va_start(arglist, output);
    vsprintf_s(buffer, 1024, output, arglist);
    va_end(arglist);

	if(m_pTextSerialiser)
	{
		const char* constBuffer = buffer;
		(*m_pTextSerialiser) << constBuffer;
	}
	else
	{
		#ifdef PLATFORM_PS3
		printf(buffer);
		#else
		OutputDebugString(buffer);
		#endif
	}
}

}; // namespace Display
