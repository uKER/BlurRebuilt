#include "stdafx.h"
#include "Display/Shader/PixelShaderUtils.h"
#include "Display/Utils/File.h"
#include "Display/Device/Device.h"
#include <System/Time/Time.h>
#include <System/Memory/MemoryTracker.h>
#include <Display2/Utils/Utils.h>
#include <system/debug/Blur2PCAutomation.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace Display
{

#if defined(DEVICE_PCGL)
Util::C_String g_PixelShaderExt = "pcglp";
#elif defined(PLATFORM_PS3)
Util::C_String g_PixelShaderExt = "p3p";
#else
Util::C_String g_PixelShaderExt = "";
#endif

Util::C_String GetPixelShaderFile(const Util::C_String& fileName)
{
#ifdef DEVICE_PS3GL
	return FormatString(Display::gFileSystem->GetWorkingDirectory() + fileName + "." + g_PixelShaderExt);
#else
	return FormatString(fileName + "." + g_PixelShaderExt);
#endif
}

#ifdef PLATFORM_PC
bool CreatePixelShaderFromFunc(const Util::C_String& name, const U32 flags, C_PixelShader* pPixelShader, const U32 size, const C_ShaderData& data)
{
	assert(pPixelShader);

	const Util::C_String& fileName = FormatString(name);
	const Util::C_String& file = GetPixelShaderFile(fileName);

	const bool created = Device()->CreatePixelShader(size, data, flags, pPixelShader);
	{
		static unsigned traced = 0;
		if (traced++ < 2000)
			Blur2PCAutomation::LogF("DX11 built-in PS %s created=%u native=%p", fileName.Chars(), created,
				pPixelShader->m_pd3dPixelShader);
	}
	if(!created)
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::C_ResourceInfo* pResourceInfo = Display2::Device()->GetPixelShaderResourceInfo(pPixelShader);
	pResourceInfo->m_Name = fileName;
	pResourceInfo->m_Time = Time::GetCurrentTime();
	#endif

	return true;
}
#endif

bool CreatePixelShaderFromFunc(const Util::C_String& name, const U32 flags, C_PixelShader* pPixelShader, const U32 size, const S32* pData)
{
	assert(pPixelShader);

	const Util::C_String& fileName = FormatString(name);
	const Util::C_String& file = GetPixelShaderFile(fileName);

	return Display2::CreatePixelShaderFromFunc(name, flags, pPixelShader, size, (const U8*)pData);
}

bool RecreatePixelShaderFromFile(const Util::C_String& file, C_PixelShader* pPixelShader)
{
#if defined(SHADERS_CG) && (defined(DEVICE_PCGL) || defined(DEVICE_PS3GL))
#ifdef PLATFORM_PC
	CGprogram cgProgram = cgCreateProgramFromFile(Device()->GetCGContext(), CG_OBJECT, file.Chars(), Device()->GetCGProfilePS(), "main", NULL);
#else
	CGprogram cgProgram = cgCreateProgramFromFile(Device()->GetCGContext(), CG_BINARY, file.Chars(), Device()->GetCGProfilePS(), "main", NULL);
#endif
	if(cgProgram)
	{
		cgGLLoadProgram(cgProgram);
		if(pPixelShader->m_cgProgram)
			cgDestroyProgram(pPixelShader->m_cgProgram);
		else
			Device()->m_PixelShaders.Insert(pPixelShader);
		pPixelShader->m_cgProgram = cgProgram;

		pPixelShader->Acquire();

		return true;
	}
	else
	{
		LOGFATAL(g_Log, "Creating cg pixel program from file '%s' = %s\n", file.Chars(), cgGetLastListing(Device()->GetCGContext()));
		return false;
	}
#else
	MEMORY_TRACKER_PUSH("graphics.display.pixelshaders.headers");
	assert(pPixelShader);
	U32 size = 0;
	U8* pData = NULL;
	if(GetFileData(file, size, &pData))
	{
		Device()->Destroy(pPixelShader);

		U32 flags = 0;
		
		#ifdef PLATFORM_PS3
		flags |= E_Usage_Managed;
		#endif

		if(Device()->CreatePixelShader(size, (S32*)pData, flags, pPixelShader))
		{
			#if !defined(USE_CGB_SHADERS)
			SAFE_DELETE_ARY(pData);
			#endif

			MEMORY_TRACKER_POP("graphics.display.pixelshaders.headers");
			return true;
		}
		else
		{
			delete[] pData;
			MEMORY_TRACKER_POP("graphics.display.pixelshaders.headers");
			return false;
		}
	}
	else
	{
		MEMORY_TRACKER_POP("graphics.display.pixelshaders.headers");
		return false;
	}
#endif
}

bool CreatePixelShaderFromFile(const Util::C_String& name, const U32 flags, C_PixelShader* pPixelShader)
{
	assert(pPixelShader);
	
	const Util::C_String& fileName = FormatString(name);
	const Util::C_String& file = GetPixelShaderFile(fileName);

	return Display2::CreatePixelShaderFromFile(name, flags, pPixelShader);
}

void RefreshPixelShaders()
{
	RESOURCE_MEMORY_ID("ReloadablePixelShaders");

#if TRACK_DISPLAY2_RESOURCES
	Display2::RefreshShaders(false, true);
#endif
}

}; // namespace Display
