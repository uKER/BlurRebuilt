#include "StdAfx.h"
#include "ShaderProjectCompiler.h"
#include "ShaderCompiler.h"
#include "CommandLine.h"
#include <System/Misc/StringUtil.h>

int CompileProjectShader(C_ShaderCompiler& shaderCompiler)
{
	shaderCompiler.m_DstFileDir		= shaderCompiler.m_SrcFileDir + shaderCompiler.m_Platform + "\\";
	shaderCompiler.m_DstFilePath	= shaderCompiler.m_DstFileDir + shaderCompiler.m_SrcFileName + shaderCompiler.m_DstFileName + ".obj";

	int errorCode = shaderCompiler.Compile();
	
	if(errorCode != 0)
		return errorCode;

	shaderCompiler.m_CpyFilePath	= shaderCompiler.m_CpyFileDir + "\\" + shaderCompiler.m_SrcFileName + shaderCompiler.m_DstFileName + "." + shaderCompiler.m_CpyFileExt;

	if(!Sys::C_FileSystem::Instance().CreateDirectory(Util::ExtractDirectory(shaderCompiler.m_CpyFilePath)))
	{
		DBError("Failed to create directory %s\n", shaderCompiler.m_CpyFilePath.Chars());
		return 1;
	}

	if(shaderCompiler.m_Platform == "xb2")
	{
		size_t varSize = 0;
		char* varData = NULL;
		shaderCompiler.GetVariable(varSize, varData, "XEDK");

		char args[1024];
		sprintf_s(args, sizeof(args), "\"%s\\bin\\win32\\xbcp\" /y /t /f \"%s\" \"%s\"", varData, shaderCompiler.m_DstFilePath.Chars(), shaderCompiler.m_CpyFilePath.Chars());
		//DBPrint("%s\n", args);

		char outputBuffer[1024];
		LaunchCommandLine(args, outputBuffer, sizeof(outputBuffer));
		//DBPrint("%s\n", outputBuffer);

		free(varData);
	}
	else
	{
		if(CopyFile(shaderCompiler.m_DstFilePath.Chars(), shaderCompiler.m_CpyFilePath.Chars(), FALSE))
		{
			//DBPrint("Copied file from %s to %s\n", shaderCompiler.m_DstFilePath.Chars(), shaderCompiler.m_CpyFilePath.Chars());
		}
		else
		{
			DBError("Failed to copy file from %s to %s\n", shaderCompiler.m_DstFilePath.Chars(), shaderCompiler.m_CpyFilePath.Chars());
			return 1;
		}
	}

	return 0;
}

int ShaderProjectCompiler(int argc, char* argv[])
{
	#if OUTPUT_DEBUG
	for(int i=0; i<argc; i++)
		std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
	#endif

	if(argc < 3)
	{
		DBError("Expecting three or more command line arguments\n");
		return 1;
	}

	C_ShaderCompiler shaderCompiler;

	if(!shaderCompiler.SetPlatform(argv[1]))
	{
		DBError("Unsupported platform %s\n", shaderCompiler.m_Platform.Chars());
		return 1;
	}

	if(!shaderCompiler.SetSrcFilePath(argv[2]))
	{
		DBError("No src file path specified\n");
		return 1;
	}

	if(!shaderCompiler.SetProfileFromSrcFilePath())
	{
		DBError("Unsupported profile for shader %s\n", shaderCompiler.m_SrcFilePath.Chars());
		return 1;
	}

	if(argc > 3)
	{
		Util::C_String copyFileConfig = argv[3];

		shaderCompiler.LoadCpyFileDir(copyFileConfig);
		shaderCompiler.InitCpyFileExt();
	}

	Util::C_String configFilePath = Util::ReplaceFileExtension(shaderCompiler.m_SrcFilePath, "txt");

	//DBPrint("%s\n", configFilePath.Chars());

	std::fstream configFile(configFilePath.Chars(), std::ios::in);

	if(configFile.is_open())
	{
		char configBuffer[512];

		while(!configFile.eof())
		{
			shaderCompiler.m_DstFileName = "";
			shaderCompiler.m_Defines.clear();

			configFile.getline(configBuffer, sizeof(configBuffer));

			#define EatWhiteSpace(x) while(x < sizeof(configBuffer) && (configBuffer[x] == 0 || configBuffer[x] == 9 || configBuffer[x] == 13 || configBuffer[x] == 32)) x++;
			#define FindWhiteSpace(x) while(x < sizeof(configBuffer) && (configBuffer[x] != 0 && configBuffer[x] != 9 && configBuffer[x] != 13 && configBuffer[x] != 32)) x++;

			for(U32 i=0; i<sizeof(configBuffer)-1; i++)
			{
				if(configBuffer[i] == '/' && configBuffer[i+1] == 'N')
				{
					i += 2;
					U32 head = i;
					EatWhiteSpace(head);
					i++;
					U32 tail = i;
					FindWhiteSpace(tail);

					Util::C_String name;
					name.CopyFrom(&configBuffer[head], tail-head);
					//DBPrint("%s\n", name.Chars());
					shaderCompiler.m_DstFileName = name;
				}
				else
				if(configBuffer[i] == '/' && configBuffer[i+1] == 'D')
				{
					i += 2;
					U32 head = i;
					EatWhiteSpace(head);
					i++;
					U32 tail = i;
					FindWhiteSpace(tail);

					Util::C_String define;
					define.CopyFrom(&configBuffer[head], tail-head);
					//DBPrint("%s\n", define.Chars());
					shaderCompiler.m_Defines.push_back(define);
				}
			}

			int errorCode = CompileProjectShader(shaderCompiler);
			if(errorCode != 0)
				return errorCode;
		}
		
		configFile.close();
	}
	else
	{
		int errorCode = CompileProjectShader(shaderCompiler);
		if(errorCode != 0)
			return errorCode;
	}

	return 0;
}
