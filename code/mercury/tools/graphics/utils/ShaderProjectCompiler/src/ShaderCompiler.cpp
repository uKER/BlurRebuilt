#include "StdAfx.h"
#include "ShaderCompiler.h"
#include "CommandLine.h"
#include <System/XML/XmlDom.h>

C_ShaderCompiler::C_ShaderCompiler()
{
	m_Platform		= "";
	m_Profile		= "";
	m_Entry			= "main";

	m_SrcFilePath	= "";
	m_SrcFileDir	= "";
	m_SrcFileName	= "";

	m_DstFilePath	= "";
	m_DstFileDir	= "";
	m_DstFileName	= "";

	m_CpyFileDir	= "";
	m_CpyFilePath	= "";
	m_CpyFileExt	= "";
}

C_ShaderCompiler::~C_ShaderCompiler()
{
	m_Defines.clear();
}

void C_ShaderCompiler::GetVariable(size_t& varSize, char*& varData, const char* pPlatform)
{
	getenv_s(&varSize, NULL, 0, pPlatform);
	varData = (char*)malloc(varSize * sizeof(char));
	getenv_s(&varSize, varData, varSize, pPlatform);
}

int C_ShaderCompiler::Compile()
{
	Sys::C_FileSystem::Instance().CreateDirectory(Util::ExtractDirectory(m_DstFilePath));

	if(m_Platform == "pcd3d9")
	{
		size_t varSize = 0;
		char* varData = NULL;
		GetVariable(varSize, varData, "DXSDK_DIR");

		Util::C_String includePath;
		includePath.CopyFrom(m_SrcFileDir.Chars(), m_SrcFileDir.Length()-1); // HACK: Need to add src file dir as include path but without trailing slash

		sprintf_s(m_Args, sizeof(m_Args), "\"%sUtilities/Bin/x86/fxc\" /Zpr /T %s /E %s /I\"%s\" /nologo /Fo \"%s\" \"%s\" /D PC /D PLATFORM_PC",
				varData,
				m_Profile.Chars(),
				m_Entry.Chars(),
				includePath.Chars(),
				m_DstFilePath.Chars(),
				m_SrcFilePath.Chars());

		for(U32 i=0; i<m_Defines.size(); i++)
		{
			sprintf_s(m_Args, sizeof(m_Args), "%s /D %s",
				m_Args,
				m_Defines[i].Chars());
		}

		free(varData);
	}
	else
	if(m_Platform == "ps3gcm")
	{
		size_t varSize = 0;
		char* varData = NULL;
		GetVariable(varSize, varData, "SCE_PS3_ROOT");

		Util::C_String includePath;
		includePath.CopyFrom(m_SrcFileDir.Chars(), m_SrcFileDir.Length()-1); // HACK: Need to add src file dir as include path but without trailing slash

		sprintf_s(m_Args, sizeof(m_Args), "\"%s/host-win32/Cg/bin/sce-cgc\" -quiet -mcgb -profile %s -entry %s -I \"%s\" -o \"%s\" \"%s\" -DPS3 -DPLATFORM_PS3",
				varData,
				m_Profile.Chars(),
				m_Entry.Chars(),
				includePath.Chars(),
				m_DstFilePath.Chars(),
				m_SrcFilePath.Chars());

		for(U32 i=0; i<m_Defines.size(); i++)
		{
			sprintf_s(m_Args, sizeof(m_Args), "%s -D%s",
				m_Args,
				m_Defines[i].Chars());
		}

		free(varData);
	}
	else
	if(m_Platform == "xb2")
	{
		size_t varSize = 0;
		char* varData = NULL;
		GetVariable(varSize, varData, "XEDK");

		Util::C_String includePath;
		includePath.CopyFrom(m_SrcFileDir, m_SrcFileDir.Length()-1); // HACK: Need to add src file dir as include path but without trailing slash

		sprintf_s(m_Args, sizeof(m_Args), "\"%s/bin/win32/fxc\" /Zpr /T %s /E %s /I \"%s\" /nologo /Fo \"%s\" \"%s\" /D XB2 /D PLATFORM_XB2",
				varData,
				m_Profile.Chars(),
				m_Entry.Chars(),
				includePath.Chars(),
				m_DstFilePath.Chars(),
				m_SrcFilePath.Chars());

		for(U32 i=0; i<m_Defines.size(); i++)
		{
			sprintf_s(m_Args, sizeof(m_Args), "%s /D %s",
				m_Args,
				m_Defines[i].Chars());
		}

		free(varData);
	}
	else
	{
		DBError("Unsupported platform %s\n", m_Platform.Chars());
		return 1;
	}


	#if OUTPUT_ARGS
	DBPrint("%s\n", m_Args);
	#endif

	char outputBuffer[1024];

	LaunchCommandLine(m_Args, outputBuffer, sizeof(outputBuffer));

	bool error = false;

	if(m_Platform == "ps3gcm")
	{
		error = outputBuffer[0] != '\0' ? true : false;
	}
	else
	{
		error = strstr(outputBuffer, "compilation failed") ? true : false;
	}

	if(error)
	{
		if(m_Platform == "ps3gcm")
		{
			std::cerr << m_SrcFileDir.Chars(); // Need to append src file dir onto shader file path...
		}

		std::cerr << outputBuffer;

		OutputDebugString(outputBuffer);

		return 1;
	}
	else
	{
		//std::cout << outputBuffer;
		
		//OutputDebugString(outputBuffer);

		return 0;
	}
}

bool C_ShaderCompiler::LoadDefines(const Util::C_String& filePath)
{
	std::fstream file;
	file.open(filePath.Chars(), std::ios::in);
	if(!file.is_open())
		return false;

	char temp[128];

	while(!file.eof())
	{
		temp[0] = '\0';

		file.getline(temp, sizeof(temp));

		if(temp[0] != 0 && temp[0] != '\0' && temp[0] != '/' && temp[1] != '/')
		{
			m_Defines.push_back(temp);
		}
	}

	file.close();
	
	return true;
}

bool C_ShaderCompiler::LoadCpyFileDir(const Util::C_String& filePath)
{
	Sys::C_XmlDom xmlDom;
	if(!xmlDom.Read(filePath))
	{
		DBError("Failed to open xml file %s\n", filePath.Chars());
		return false;
	}

	const Sys::C_XmlDomBaseNode* pRoot = xmlDom.GetRootNodePtr();

	Util::C_String platform = "";

	for(S32 i=0; i<pRoot->GetChildren().Size(); i++)
	{
		Sys::C_XmlDomBaseNode* pPlatform = pRoot->GetChildren()[i];
		if(pPlatform && pPlatform->GetName() == "platform")
		{
			const Sys::C_XmlAttribute* pName = ((Sys::C_XmlDomElement*)(pPlatform))->GetAttributes().GetAttributeByName("name");
			if (pName)
				platform = pName->value;

			const Sys::C_XmlAttribute* pDstDir = ((Sys::C_XmlDomElement*)(pPlatform))->GetAttributes().GetAttributeByName("dstdir");
			if (pDstDir && platform == m_Platform)
				m_CpyFileDir = pDstDir->value;
		}
	}

	return true;
}

bool C_ShaderCompiler::InitCpyFilePath()
{
	m_CpyFilePath = m_CpyFileDir + "\\" + m_DstFileName + "." + m_CpyFileExt;

	return true;
}

bool C_ShaderCompiler::InitCpyFileExt()
{
	if(m_Platform == "pcd3d9")
	{
		if(m_Profile.Search("vs") != UINT_MAX)
		{
			m_CpyFileExt = "pcvsh";
		}
		else
		if(m_Profile.Search("ps") != UINT_MAX)
		{
			m_CpyFileExt = "pcpsh";
		}
		else
		{
			DBError("Unsupported profile %s\n", m_Profile.Chars());
			return false;
		}
	}
	else
	if(m_Platform == "ps3gcm")
	{
		if(m_Profile.Search("sce_vp_rsx") != UINT_MAX)
		{
			m_CpyFileExt = "p3vsh";
		}
		else
		if(m_Profile.Search("sce_fp_rsx") != UINT_MAX)
		{
			m_CpyFileExt = "p3psh";
		}
		else
		{
			DBError("Unsupported profile %s\n", m_Profile.Chars());
			return false;
		}
	}
	else
	if(m_Platform == "xb2")
	{
		if(m_Profile.Search("vs") != UINT_MAX)
		{
			m_CpyFileExt = "x2vsh";
		}
		else
		if(m_Profile.Search("ps") != UINT_MAX)
		{
			m_CpyFileExt = "x2psh";
		}
		else
		{
			DBError("Unsupported profile %s\n", m_Profile.Chars());
			return false;
		}
	}
	else
	{
		DBError("Unsupported platform %s\n", m_Platform.Chars());
		return false;
	}
	
	return true;
}

bool C_ShaderCompiler::SetPlatform(const char* platform)
{
	m_Platform = platform;

	if(m_Platform != "pcd3d9" && m_Platform != "ps3gcm" && m_Platform != "xb2")
		return false;

	#if OUTPUT_DEBUG
	DBPrint("m_Platform '%s'\n", m_Platform.Chars());
	#endif

	return true;
}

bool C_ShaderCompiler::SetSrcFilePath(const char* srcFilePath)
{
	m_SrcFilePath 	= srcFilePath;
	m_SrcFileDir	= Util::ExtractDirectory(m_SrcFilePath);
	m_SrcFileName	= Util::RemoveFileExtension(Util::ExtractFilename(m_SrcFilePath));

	if(m_SrcFilePath == "")
		return false;

	#if OUTPUT_DEBUG
	DBPrint("m_SrcFilePath '%s'\n", m_SrcFilePath.Chars());
	DBPrint("m_SrcFileDir '%s'\n", m_SrcFileDir.Chars());
	DBPrint("m_SrcFileName '%s'\n", m_SrcFileName.Chars());
	#endif

	return true;
}

bool C_ShaderCompiler::SetProfileFromSrcFilePath()
{
	if(m_Profile == "")
	{
		if(m_SrcFilePath.Search("vs") != UINT_MAX)
		{
			if(m_Platform == "ps3gcm")
			{
				m_Profile = "sce_vp_rsx";
			}
			else
			{
				m_Profile = "vs_3_0";
			}
		}
		else
		if(m_SrcFilePath.Search("ps") != UINT_MAX)
		{
			if(m_Platform == "ps3gcm")
			{
				m_Profile = "sce_fp_rsx";
			}
			else
			{
				m_Profile = "ps_3_0";
			}
		}
		else
		{
			return false;
		}
	}

	#if OUTPUT_DEBUG
	DBPrint("m_Profile '%s'\n", m_Profile.Chars());
	#endif

	return true;
}
