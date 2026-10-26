#include "StdAfx.h"
#include "ShaderCompiler.h"

//#define DISABLE_PS20_FOR_PCD3D9 // [LloydW] Uncomment this to get ps_2_0 hell...

namespace ShaderCompiler
{

// C_Shader

C_Shader::C_Shader()
{
	m_Size			= 0;
	m_Args[0]		= '\0';
	m_Profile		= E_ShaderProfile_Unknown;
	m_DstFileName	= "";
	m_DstFilePath	= "";
}

C_Shader::~C_Shader()
{
}

// C_ShaderCompiler

C_ShaderCompiler::C_ShaderCompiler()
{
	m_Type			= E_ShaderType_Unknown;
	m_Platform		= E_ShaderPlatform_Unknown;
	m_DstFileName	= "";
	m_DstFileDir	= "";
	m_DstFilePath	= "";
	m_SrcFilePath	= "";
	m_SrcFileDir	= "";
	m_SrcFileName	= "";
	m_Entry			= "main";
}

C_ShaderCompiler::~C_ShaderCompiler()
{
	m_Defines.clear();
	m_Shaders.clear();
}

void C_ShaderCompiler::GetVariable(size_t& varSize, char*& varData, const char* pPlatform)
{
	getenv_s(&varSize, NULL, 0, pPlatform);
	varData = (char*)malloc(varSize * sizeof(char));
	getenv_s(&varSize, varData, varSize, pPlatform);
}

std::string C_ShaderCompiler::GetOutputDir()
{
	switch(m_Platform)
	{
	case E_ShaderPlatform_PCD3D9:
		return "pcd3d9";
	case E_ShaderPlatform_XB2:
		return "xb2";
	case E_ShaderPlatform_PS3:
		return "ps3";
	default:
		assert(0 && "Unsupported platform");
		return "";
	}
}

std::string C_ShaderCompiler::GetType()
{
	switch(m_Type)
	{
	case E_ShaderType_Pixel:
		return "SHADER_PS";
	case E_ShaderType_Vertex:
		return "SHADER_VS";
	default:
		return "";
	}
}

std::string C_ShaderCompiler::GetProfile(const E_ShaderProfile profile)
{
	std::string strProfile = "";

	switch(m_Platform)
	{
	case E_ShaderPlatform_PCD3D9:
		switch(m_Type)
		{
		case E_ShaderType_Pixel:
			switch(profile)
			{
			#ifndef DISABLE_PS20_FOR_PCD3D9
			case E_ShaderProfile_2_0:
				strProfile = "ps_2_0";
				break;
			#else
			case E_ShaderProfile_2_0:
			#endif
			case E_ShaderProfile_3_0:
				strProfile = "ps_3_0";
				break;
			}
			break;
		case E_ShaderType_Vertex:
			switch(profile)
			{
			#ifndef DISABLE_PS20_FOR_PCD3D9
			case E_ShaderProfile_2_0:
				strProfile = "vs_2_0";
				break;
			#else
			case E_ShaderProfile_2_0:
			#endif
			case E_ShaderProfile_3_0:
				strProfile = "vs_3_0";
				break;
			}
			break;
		}
		break;
	case E_ShaderPlatform_XB2:
		switch(m_Type)
		{
		case E_ShaderType_Pixel:
			strProfile = "ps_3_0";
			break;
		case E_ShaderType_Vertex:
			strProfile = "vs_3_0";
			break;
		}
		break;
	case E_ShaderPlatform_PS3:
		switch(m_Type)
		{
		case E_ShaderType_Pixel:
			strProfile = "sce_fp_rsx";
			break;
		case E_ShaderType_Vertex:
			strProfile = "sce_vp_rsx";
			break;
		}
		break;
	}

	return strProfile;
}

void C_ShaderCompiler::BuildArgs(C_Shader& shader)
{
	switch(m_Platform)
	{
	case E_ShaderPlatform_PCD3D9:
		{
			size_t varSize = 0;
			char* varData = NULL;
			GetVariable(varSize, varData, "DXSDK_DIR");

			std::string shaderModel = "";

			switch(m_Type)
			{
			case E_ShaderType_Pixel:
				shaderModel = "PS_";
				break;
			case E_ShaderType_Vertex:
				shaderModel = "VS_";
				break;
			}

			switch(shader.m_Profile)
			{
			case E_ShaderProfile_2_0:
				#ifdef DISABLE_PS20_FOR_PCD3D9
				shaderModel += "3_0";
				#else
				shaderModel += "2_0";
				#endif
				shader.m_DstFileName = m_DstFileName + "_2_0";
				break;
			case E_ShaderProfile_3_0:
				shaderModel += "3_0";
				shader.m_DstFileName += m_DstFileName + "_3_0";
				break;
			}
			
			shader.m_DstFilePath = m_DstFileDir + "\\" + shader.m_DstFileName + ".shader";

			std::string includePath(m_SrcFileDir, 0, m_SrcFileDir.size()-1); // HACK: Need to add src file dir as include path but without trailing slash.

			std::string type = GetType();

			std::string profile = GetProfile(shader.m_Profile);

			sprintf_s(shader.m_Args, sizeof(shader.m_Args), "\"%sUtilities/Bin/x86/fxc\" /T %s /E %s /I\"%s\" /nologo /Fo \"%s\" \"%s\" /D PCD3D9 /D PLATFORM_PC /D %s /D %s",
				varData,
				profile.c_str(),
				m_Entry.c_str(),
				includePath.c_str(),
				shader.m_DstFilePath.c_str(),
				m_SrcFilePath.c_str(),
				type.c_str(),
				shaderModel.c_str());

			for(unsigned int i=0; i<m_Defines.size(); i++)
			{
				sprintf_s(shader.m_Args, sizeof(shader.m_Args), "%s /D %s", shader.m_Args, m_Defines[i].c_str());
			}

			free(varData);
		}
		break;
	case E_ShaderPlatform_XB2:
		{
			size_t varSize = 0;
			char* varData = NULL;
			GetVariable(varSize, varData, "XEDK");

			std::string shaderModel = "";

			switch(m_Type)
			{
			case E_ShaderType_Pixel:
				shaderModel = "PS_3_0";
				break;
			case E_ShaderType_Vertex:
				shaderModel = "VS_3_0";
				break;
			}

			shader.m_DstFileName = m_DstFileName;

			shader.m_DstFilePath = m_DstFileDir + "\\" + shader.m_DstFileName + ".shader";

			std::string includePath(m_SrcFileDir, 0, m_SrcFileDir.size()-1); // HACK: Need to add src file dir as include path but without trailing slash.

			std::string type = GetType();

			std::string profile = GetProfile(shader.m_Profile);

			sprintf_s(shader.m_Args, sizeof(shader.m_Args), "\"%s/bin/win32/fxc\" /T %s /E %s /I \"%s\" /nologo /Fo \"%s\" \"%s\" /D XB2 /D PLATFORM_XENON /D %s /D %s",
				varData,
				profile.c_str(),
				m_Entry.c_str(),
				includePath.c_str(),
				shader.m_DstFilePath.c_str(),
				m_SrcFilePath.c_str(),
				type.c_str(),
				shaderModel.c_str());

			for(unsigned int i=0; i<m_Defines.size(); i++)
			{
				sprintf_s(shader.m_Args, sizeof(shader.m_Args), "%s /D %s", shader.m_Args, m_Defines[i].c_str());
			}

			free(varData);
		}
		break;
	case E_ShaderPlatform_PS3:
		{
			size_t varSize = 0;
			char* varData = NULL;
			GetVariable(varSize, varData, "SCE_PS3_ROOT");

			std::string shaderModel = "";

			switch(m_Type)
			{
			case E_ShaderType_Pixel:
				shaderModel = "PS_3_0";
				break;
			case E_ShaderType_Vertex:
				shaderModel = "VS_3_0";
				break;
			}
			
			shader.m_DstFileName = m_DstFileName;

			shader.m_DstFilePath = m_DstFileDir + "\\" + shader.m_DstFileName + ".shader";

			std::string includePath(m_SrcFileDir, 0, m_SrcFileDir.size()-1); // HACK: Need to add src file dir as include path but without trailing slash.

			std::string type = GetType();

			std::string profile = GetProfile(shader.m_Profile);

			sprintf_s(shader.m_Args, sizeof(shader.m_Args), "\"%s/host-win32/Cg/bin/sce-cgc\" -mcgb -q -fastmath -p %s -I \"%s\" -o \"%s\" \"%s\" -DPS3 -DPLATFORM_PS3 -D%s -D%s",
				varData,
				profile.c_str(),
				includePath.c_str(),
				shader.m_DstFilePath.c_str(),
				m_SrcFilePath.c_str(),
				type.c_str(),
				shaderModel.c_str());

			for(unsigned int i=0; i<m_Defines.size(); i++)
			{
				sprintf_s(shader.m_Args, sizeof(shader.m_Args), "%s -D%s", shader.m_Args, m_Defines[i].c_str());
			}

			free(varData);
		}
		break;
	}
}

bool C_ShaderCompiler::Compile(C_Shader& shader)
{
	HANDLE hOutputReadTmp, hOutputRead, hOutputWrite;
	HANDLE hInputWriteTmp, hInputRead, hInputWrite;
	HANDLE hErrorWrite;

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if(!CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 0))
	{
		DBError("Creating the output pipe\n");
		return false;
	}

	if(!DuplicateHandle(GetCurrentProcess(), hOutputWrite, GetCurrentProcess(), &hErrorWrite, 0, TRUE, DUPLICATE_SAME_ACCESS))
	{
		DBError("Duplicating the output write handle\n");
		return false;
	}

	if(!CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 0))
	{
		DBError("Creating the input pipe\n");
		return false;
	}

	if(!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp, GetCurrentProcess(), &hOutputRead, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		DBError("Duplicating the output read handle\n");
		return false;
	}

	if(!DuplicateHandle(GetCurrentProcess(), hInputWriteTmp, GetCurrentProcess(), &hInputWrite, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		DBError("Duplicating the input write handle\n");
		return false;
	}

	if(!CloseHandle(hOutputReadTmp))
	{
		DBError("Closing the output read handle\n");
		return false;
	}

	if(!CloseHandle(hInputWriteTmp))
	{
		DBError("Closing the input write handle\n");
		return false;
	}

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));

	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = hOutputWrite;
	si.hStdInput = hInputRead;
	si.hStdError = hErrorWrite;
	
	//std::cout << shader.m_Args << std::endl;
	//OutputDebugString(shader.m_Args);
	//OutputDebugString("\n");

	PROCESS_INFORMATION pi;
	HANDLE hChildProcess = NULL;
	if(CreateProcess(0, shader.m_Args, 0, 0, TRUE, 0, 0, 0, &si, &pi) != 0)
	{
		hChildProcess = pi.hProcess;
	}
	else
	{
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			0,
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);
		DBError("Failed to create the process %s %s\n", (LPCTSTR)lpMsgBuf, shader.m_Args);
		return false;
	}

	WaitForSingleObject(hChildProcess, INFINITE);

	DWORD exitCode = 666;
	if(GetExitCodeProcess(hChildProcess, &exitCode) == 0)
	{
		DBError("Getting the exit code from the handle\n");
		return false;
	}

	if(!hChildProcess)
	{
		DBError("Failed to create the process\n");
		return false;
	}

	if(!CloseHandle(hOutputWrite))
	{
		DBError("Closing the output writehandle\n");
		return false;
	}

	if(!CloseHandle(hInputRead))
	{
		DBError("Closing the input read handle\n");
		return false;
	}

	if(!CloseHandle(hErrorWrite))
	{
		DBError("Closing the error write handle\n");
		return false;
	}

	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	DWORD bytesRead;
	ReadFile(hOutputRead, buffer, sizeof(buffer) - 4, &bytesRead, NULL);
	buffer[bytesRead] = '\0';
	
	if(!CloseHandle(hOutputRead))
	{
		DBError("Closing the output read handle\n");
		return false;
	}

	if(!CloseHandle(hInputWrite))
	{
		DBError("Closing the input write handle\n");
		return false;
	}

	if(exitCode != 0)
	{
		std::cerr << buffer;

		return false;
	}
	else
	{
		if(m_Platform == E_ShaderPlatform_PS3 && strlen(buffer) > 0)
		{
			std::cout << buffer;
		}

		return true;
	}

}

bool C_ShaderCompiler::WriteShader(C_File& outputFile, const std::string& name, const int size, int* pData)
{
	outputFile.Write("static int ");
	outputFile.Write(name);
	outputFile.Write("[");
	outputFile.Write(size);
	outputFile.Write("] = {");

	for(int i=0; i<size; i++)
	{
		int value = pData[i];
		
		if(m_Platform == E_ShaderPlatform_XB2 || m_Platform == E_ShaderPlatform_PS3)
		{
			value = _byteswap_ulong(value);
		}

		outputFile.Write(value);
		outputFile.Write(",");
	}

	outputFile.Write("};\n");

	return true;
}

bool C_ShaderCompiler::Compile(int argc, _TCHAR* argv[])
{
	for(int i=1; i<argc; i++)
	{
		if(i == 1)
		{
			#ifdef OUTPUT_DEBUG
			DBPrint("Platform = %s\n", argv[i]);
			#endif

			if(strcmp(argv[i], "pcd3d9") == 0)
			{
				m_Platform = E_ShaderPlatform_PCD3D9;
			}
			else
			if(strcmp(argv[i], "xb2") == 0)
			{
				m_Platform = E_ShaderPlatform_XB2;
			}
			else
			if(strcmp(argv[i], "ps3") == 0)
			{
				m_Platform = E_ShaderPlatform_PS3;
			}
			else
			{
				DBError("Unsupported platform %s\n", argv[i]);
				return false;
			}
		}
		else
		if(i == 2)
		{
			#ifdef OUTPUT_DEBUG
			DBPrint("DstFileName = %s\n", argv[i]);
			#endif

			m_DstFileName = argv[i];
		}
		else
		if(i == 3)
		{
			#ifdef OUTPUT_DEBUG
			DBPrint("SrcFilePath = %s\n", argv[i]);
			#endif

			m_SrcFilePath = argv[i];
		}
		else
		{
			if(strcmp(argv[i], "/P") == 0 || strcmp(argv[i], "-P") == 0 )
			{
				i++;

				#ifdef OUTPUT_DEBUG
				DBPrint("Profile = %s\n", argv[i]);
				#endif

				if(strcmp(argv[i], "ps_2_0") == 0)
				{
					m_Type	= E_ShaderType_Pixel;
				}
				else
				if(strcmp(argv[i], "ps_3_0") == 0)
				{
					m_Type	= E_ShaderType_Pixel;
				}
				else
				if(strcmp(argv[i], "vs_2_0") == 0)
				{
					m_Type	= E_ShaderType_Vertex;
				}
				else
				if(strcmp(argv[i], "vs_3_0") == 0)
				{
					m_Type	= E_ShaderType_Vertex;
				}
				else
				{
					DBError("Unsupported profile %s\n", argv[i]);
					return false;
				}
			}
			else
			if(strcmp(argv[i], "/E") == 0 || strcmp(argv[i], "-E") == 0)
			{
				i++;

				#ifdef OUTPUT_DEBUG
				DBPrint("Entry = %s\n", argv[i]);
				#endif

				m_Entry = argv[i];
			}
			else
			if(strcmp(argv[i], "/D") == 0 || strcmp(argv[i], "-D") == 0)
			{
				i++;

				#ifdef OUTPUT_DEBUG
				DBPrint("Defines = %s\n", argv[i]);
				#endif

				m_Defines.push_back(argv[i]);
			}
		}
	}

	if(m_Type == E_ShaderType_Unknown)
	{
		DBError("You must specify a valid profile\n");
		return false;
	}
	
	if(m_DstFileName == "")
	{
		DBError("You must specify a destination file name\n");
		return false;
	}

	if(m_SrcFilePath == "")
	{
		DBError("You must specify a source file path\n");
		return false;
	}

	size_t p1 = m_SrcFilePath.find_last_of("\\");
	size_t p2 = m_SrcFilePath.find_last_of(".");

	m_SrcFileDir = m_SrcFilePath.substr(0, p1 + 1);
	m_SrcFileName = m_SrcFilePath.substr(p1 + 1, p2 - p1 - 1);

	m_DstFileDir = m_SrcFileDir + GetOutputDir();
	m_DstFilePath = m_DstFileDir + "\\" + m_DstFileName + ".shader";
	
	C_Shader shader;
	switch(m_Platform)
	{
	case E_ShaderPlatform_PCD3D9:
		shader.m_Profile = E_ShaderProfile_2_0;
		m_Shaders.push_back(shader);
		shader.m_Profile = E_ShaderProfile_3_0;
		m_Shaders.push_back(shader);
		break;
	case E_ShaderPlatform_XB2:
	case E_ShaderPlatform_PS3:
		shader.m_Profile = E_ShaderProfile_3_0;
		m_Shaders.push_back(shader);
		break;
	default:
		assert(0 && "Unsupported platform");
		break;
	}

	for(unsigned int i=0; i<m_Shaders.size(); i++)
	{
		C_Shader& shader = m_Shaders[i];

		BuildArgs(shader);

		#ifdef OUTPUT_DEBUG
		DBPrint("Args = %s\n", shader.m_Args);
		#endif

		std::string srcFileName = m_SrcFilePath.substr(m_SrcFilePath.find_last_of("\\") + 1);
		std::string dstFileName = shader.m_DstFilePath.substr(shader.m_DstFilePath.find_last_of("\\") + 1);

		std::cout << "Performing Custom Build Step - " << srcFileName.c_str() << " to " << dstFileName.c_str() << std::endl;

		CreateDirectory(m_DstFileDir.c_str(), NULL);

		if(!Compile(shader))
		{
			DBError("Failed to compile the shader\n");
			return false;
		}
	}

	switch(m_Platform)
	{
	case E_ShaderPlatform_PCD3D9:
		{
			C_File outputFile;
			if(!outputFile.Open(m_DstFilePath, true))
			{
				DBError("Failed to open file %s\n", m_DstFilePath.c_str());
				return false;
			}

			for(unsigned int i=0; i<m_Shaders.size(); i++)
			{
				C_Shader& shader = m_Shaders[i];
				int size = 0;
				int* pData = NULL;
				C_File inputFile;
				if(!inputFile.Open(shader.m_DstFilePath, false))
				{
					DBError("Failed to open file %s\n", shader.m_DstFilePath.c_str());
					return false;
				}
				inputFile.Read(size, &pData);
				WriteShader(outputFile, shader.m_DstFileName, size, pData);
				delete[] pData;
				shader.m_Size = size;
			}

#if 1
			outputFile.Write("static C_ShaderData ");
			outputFile.Write(m_DstFileName);
			outputFile.Write(" = {");
			for(unsigned int i=0; i<m_Shaders.size(); i++)
			{
				C_Shader& shader = m_Shaders[i];
				outputFile.Write(shader.m_Size);
				outputFile.Write(",");
				outputFile.Write(shader.m_DstFileName);
				outputFile.Write(",");
			}
			outputFile.Write("};\n");
#else
			outputFile.Write("static const int* ");
			outputFile.Write(m_DstFileName);
			outputFile.Write("[");
			outputFile.Write(m_Shaders.size());
			outputFile.Write("] = {");
			for(unsigned int i=0; i<m_Shaders.size(); i++)
			{
				C_Shader& shader = m_Shaders[i];
				outputFile.Write(shader.m_DstFileName);
				outputFile.Write(",");
			}
			outputFile.Write("};\n");
#endif

			outputFile.Close();
		}
		break;
	default:
		{
			int size = 0;
			int* pData = NULL;
			
			C_File inputFile;
			if(!inputFile.Open(m_DstFilePath, false))
			{
				DBError("Failed to open file %s\n", m_DstFilePath.c_str());
				return false;
			}
			inputFile.Read(size, &pData);
			inputFile.Close();

			C_File outputFile;
			if(!outputFile.Open(m_DstFilePath, true))
			{
				delete[] pData;
				DBError("Failed to open file %s\n", m_DstFilePath.c_str());
				return false;
			}
			if(!WriteShader(outputFile, m_DstFileName, size, pData))
			{
				return false;
			}
			outputFile.Close();
			
			delete[] pData;
		}
		break;
	}

	return true;
}

}; // namespace ShaderCompiler
