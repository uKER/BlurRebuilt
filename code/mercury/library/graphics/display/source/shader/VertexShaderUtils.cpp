#include "stdafx.h"
#include "Display/Shader/VertexShaderUtils.h"
#include "Display/Utils/File.h"
#include "Display/Device/Device.h"
#include <System/Time/Time.h>
#include <Display2/Utils/Utils.h>
#include <system/debug/Blur2PCAutomation.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace Display
{

#if defined(DEVICE_PCGL)
Util::C_String g_VertexShaderExt = "pcglv";
#elif defined(PLATFORM_PS3)
Util::C_String g_VertexShaderExt = "p3v";
#else
Util::C_String g_VertexShaderExt = "";
#endif

Util::C_String GetVertexShaderFile(const Util::C_String& fileName)
{
#ifdef DEVICE_PS3GL
	return FormatString(Display::gFileSystem->GetWorkingDirectory() + fileName + "." + g_VertexShaderExt);
#else
	return FormatString(fileName + "." + g_VertexShaderExt);
#endif
}

#ifdef PLATFORM_PC
bool CreateVertexShaderFromFunc(const Util::C_String& name, const U32 flags, C_VertexShader* pVertexShader, const U32 size, const C_ShaderData& data)
{
	assert(pVertexShader);

	const Util::C_String& fileName = FormatString(name);
	const Util::C_String& file = GetVertexShaderFile(fileName);

	const bool created = Device()->CreateVertexShader(size, data, flags, pVertexShader);
	{
		static unsigned traced = 0;
		if (traced++ < 2000)
			Blur2PCAutomation::LogF("DX11 built-in VS %s created=%u native=%p", fileName.Chars(), created,
				pVertexShader->m_pd3dVertexShader);
	}
	if(!created)
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::C_ResourceInfo* pResourceInfo = Display2::Device()->GetVertexShaderResourceInfo(pVertexShader);
	pResourceInfo->m_Name = fileName;
	pResourceInfo->m_Time = Time::GetCurrentTime();
	#endif

	return true;
}
#endif

bool CreateVertexShaderFromFunc(const Util::C_String& name, const U32 flags, C_VertexShader* pVertexShader, const U32 size, const S32* pData)
{
	assert(pVertexShader);

	const Util::C_String& fileName = FormatString(name);
	const Util::C_String& file = GetVertexShaderFile(fileName);

	return Display2::CreateVertexShaderFromFunc(name, flags, pVertexShader, size, (const U8*)pData);
}

bool RecreateVertexShaderFromFile(const Util::C_String& file, C_VertexShader* pVertexShader)
{
#if defined(SHADERS_CG) && (defined(DEVICE_PCGL) || defined(DEVICE_PS3GL))
#ifdef PLATFORM_PC
	CGprogram cgProgram = cgCreateProgramFromFile(Device()->GetCGContext(), CG_OBJECT, file.Chars(), Device()->GetCGProfileVS(), "main", NULL);
#else
	CGprogram cgProgram = cgCreateProgramFromFile(Device()->GetCGContext(), CG_BINARY, file.Chars(), Device()->GetCGProfileVS(), "main", NULL);
#endif
	if(cgProgram)
	{
		cgGLLoadProgram(cgProgram);
		if(pVertexShader->m_cgProgram)
			cgDestroyProgram(pVertexShader->m_cgProgram);
		else
			Device()->m_VertexShaders.Insert(pVertexShader);
		pVertexShader->m_cgProgram = cgProgram;

		pVertexShader->Acquire();

		return true;
	}
	else
	{
		LOGFATAL(g_Log, "Creating cg vertex program from file '%s' = %s\n", file.Chars(), cgGetLastListing(Device()->GetCGContext()));
		return false;
	}
#else
	assert(pVertexShader);
	U32 size = 0;
	U8* pData = NULL;
	if(GetFileData(file, size, &pData))
	{
		Device()->Destroy(pVertexShader);
		
		U32 flags = 0;
		
		#ifdef PLATFORM_PS3
		flags |= E_Usage_Managed;
		#endif

		if(Device()->CreateVertexShader(size, (S32*)pData, flags, pVertexShader))
		{
			#if !defined(USE_CGB_SHADERS)
			SAFE_DELETE_ARY(pData);
			#endif

			return true;
		}
		else
		{
			SAFE_DELETE_ARY(pData);
			return false;
		}
	}
	else
	{
		return false;
	}
#endif
}

bool CreateVertexShaderFromFile(const Util::C_String& name, const U32 flags, C_VertexShader* pVertexShader)
{
	assert(pVertexShader);

	const Util::C_String& fileName = FormatString(name);
	const Util::C_String& file = GetVertexShaderFile(fileName);

	return Display2::CreateVertexShaderFromFile(name, flags, pVertexShader);
}

void RefreshVertexShaders()
{
	RESOURCE_MEMORY_ID("ReloadableVertexShaders");

#if TRACK_DISPLAY2_RESOURCES
	Display2::RefreshShaders(true, false);
#endif
}

}; // namespace Display
