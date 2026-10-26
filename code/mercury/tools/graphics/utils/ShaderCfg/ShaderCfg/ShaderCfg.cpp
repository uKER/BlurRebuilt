// ShaderCfg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <conio.h>

#include <system/serialise.h>
#include <system/serialise/C_FileSystem.h>
#include <System/Misc/StringUtil.h>

#include "ShaderConfig.h"
#include "ShaderRegistry.h"
#include "ShaderConfigParser.h"
#include "ShaderRegistryParser.h"
#include "Options.h"

#include <System/DebugLog.h>

#include <system/serialise/TextSerialise.h>

#include "ShaderCfg.h"

namespace App
{

Debug::Logger &g_Log = Debug::GetLogger("ShaderCfgApp");

};

C_Options g_Options;

ShaderCfg::C_ShaderConfig g_ShaderCfg;
ShaderCfg::C_ShaderRegistry g_ShaderReg(&g_ShaderCfg);


bool ProcessBatchFile( C_OptionsConfig& config )
{
	if(config.m_BatchFileIn == "")
		return true;

	Sys::I_File* pInFile = Sys::C_FileSystem::Instance().Open( config.m_BatchFileIn, Sys::E_FileAccess_Read );		
	if(pInFile == NULL)
	{
		LOGERROR(App::g_Log, "Failed to open input batch file %s", config.m_BatchFileIn.Chars());
		return false;
	}

	// create directory for output
	Util::C_String path = Util::ExtractDirectory( config.m_BatchFileOut );
	Sys::C_FileSystem::Instance().CreateDirectory(path);

	Sys::I_File* pOutFile = Sys::C_FileSystem::Instance().Open( config.m_BatchFileOut, Sys::E_FileAccess_Write );		
	if(pOutFile == NULL)
	{
		LOGERROR(App::g_Log, "Failed to open output batch file %s", config.m_BatchFileOut.Chars());
		return false;
	}

	{
		Sys::TextDeserialiser in(*pInFile);
		Sys::TextSerialiser out(*pOutFile);
		


		bool fileEnd = false;
		while(!fileEnd)
		{
			Util::C_String text;			

			// read next line

			bool lineEnd = false;
			while((pInFile->GetPosition() < pInFile->Length()) && !lineEnd)
			{
				char ch;
				in >> ch;

				text << Util::C_String(ch);

				if( ch == '\n' )
				{
					lineEnd = true;
				}

				if(pInFile->GetPosition() >= pInFile->Length())
				{
					lineEnd = true;
					fileEnd = true;
					text << Util::C_String("\n");
				}
			}

			if(text.Search("$(VERTEX_FILENAME)") != -1)
			{
				// process line
				for(S32 shaderNo=0; shaderNo<g_ShaderReg.m_VertexShaderList.Size(); shaderNo++)
				{
					ShaderCfg::C_Shader& shader = g_ShaderReg.m_VertexShaderList[shaderNo];
		
					// build up defines string...
					Util::C_String definesString = "";
		
					for(S32 defineNo=0; defineNo<shader.m_DefineList.Size(); defineNo++)
					{
						S32 defineIndex = shader.m_DefineList.m_DefineList[defineNo];
						ShaderCfg::C_Define& define = g_ShaderCfg.m_DefineList[defineIndex];
		
						Util::C_String defineSubString;
						if(define.m_Value == "")
						{
							defineSubString = config.m_DefineFormat;
						}
						else
						{
							defineSubString = config.m_DefineValueFormat;
						}
						defineSubString = defineSubString.Replace( "$(DEFINE_NAME)", define.m_Define);
						defineSubString = defineSubString.Replace( "$(DEFINE_VALUE)", define.m_Value);
		
						definesString = definesString + defineSubString;
					}
		
					// Build final compile string
					Util::C_String finalCompileString = text;
					finalCompileString = finalCompileString.Replace( "$(VERTEX_FILENAME)", shader.GetName() );
					finalCompileString = finalCompileString.Replace( "$(DEFINES)", definesString );
		
					//LOGINFO( App::g_Log, "%s", finalCompileString.Chars());
		
					out << finalCompileString;
				}
			}
			else
			if(text.Search("$(PIXEL_FILENAME)") != -1)
			{				
				// process line
				for(S32 shaderNo=0; shaderNo<g_ShaderReg.m_PixelShaderList.Size(); shaderNo++)
				{
					ShaderCfg::C_Shader& shader = g_ShaderReg.m_PixelShaderList[shaderNo];
		
					// build up defines string...
					Util::C_String definesString = "";
		
					for(S32 defineNo=0; defineNo<shader.m_DefineList.Size(); defineNo++)
					{
						S32 defineIndex = shader.m_DefineList.m_DefineList[defineNo];
						ShaderCfg::C_Define& define = g_ShaderCfg.m_DefineList[defineIndex];
		
						Util::C_String defineSubString;
						if(define.m_Value == "")
						{
							defineSubString = config.m_DefineFormat;
						}
						else
						{
							defineSubString = config.m_DefineValueFormat;
						}

						defineSubString = defineSubString.Replace( "$(DEFINE_NAME)", define.m_Define);
						defineSubString = defineSubString.Replace( "$(DEFINE_VALUE)", define.m_Value);
		
						definesString = definesString + defineSubString;
					}
		
					// Build final compile string
					Util::C_String finalCompileString = text;
					finalCompileString = finalCompileString.Replace( "$(PIXEL_FILENAME)", shader.GetName() );
					finalCompileString = finalCompileString.Replace( "$(DEFINES)", definesString );
		
					//LOGINFO( App::g_Log, "%s", finalCompileString.Chars());
		
					out << finalCompileString;
				}
			}
			else
			{
				//LOGINFO(App::g_Log, "%s", text.Chars());
				out << text;
			}

			pOutFile->Flush();
		}
	}

	SAFE_DELETE_PTR(pOutFile);
	SAFE_DELETE_PTR(pInFile);

	return true;
}

void AnyKey()
{
	while(!_kbhit());
	_getch();
}


int main(int argc, char* argv[])
{
	Debug::AutoConfigureDebugLog();

	if(argc != 2)
	{
		LOGERROR(App::g_Log, "Usage %s <options.xml>\n", argv[0]);
		LOGINFO(App::g_Log, "Press any key...\n");
		AnyKey();
		return -1;
	}

	LOGINFO( App::g_Log, "Read Options File\n" );

	g_Options.ReadXml(argv[1]);

	g_Options.Dump();

	LOGINFO( App::g_Log, "Read Shader Config\n" );

	bool result;	
	
	result = ShaderCfg::C_ShaderConfigXmlParser(&g_ShaderCfg).ReadXml(g_Options.m_ConfigFile);

	if(!result)
		return -1;

	//g_ShaderCfg.Dump();

	LOGINFO( App::g_Log, "Read Shader Registry\n" );

	result = ShaderCfg::C_ShaderRegistryXmlParser(&g_ShaderReg).ReadXml(g_Options.m_RegistryFile);

	if(!result)
		return -1;

	//g_ShaderReg.Dump();

	ShaderRegistry::C_Registry gameRegistry;
	g_ShaderReg.CreateGameRegistry( gameRegistry );

	const S32 configCount = g_Options.m_ConfigList.Size();
	for(S32 configNo=0; configNo<configCount; configNo++)
	{
		C_OptionsConfig& config = g_Options.m_ConfigList[configNo];

		LOGINFO( App::g_Log, "Process Batch File for %s\n", config.m_ConfigName.Chars() );

		ProcessBatchFile(config);

		LOGINFO( App::g_Log, "Save Binary File for %s\n", config.m_ConfigName.Chars() );

		{
			// create directory for output
			Util::C_String path = Util::ExtractDirectory( config.m_BinaryFileOut );
			Sys::C_FileSystem::Instance().CreateDirectory(path);

			// write output file
			Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open( config.m_BinaryFileOut, Sys::E_FileAccess_Write );		
			if(pFile == NULL)
			{
				LOGERROR(App::g_Log, "Failed to open output file");
				return -1;
			}
			
			if(config.m_EndianSwapped)
			{
				pFile->SetEndianSwapFlag( true );
			}
				
			gameRegistry.Save(*pFile);
			
			SAFE_DELETE_PTR( pFile );
		}

	}

	LOGINFO(App::g_Log, "\n");
	LOGINFO(App::g_Log, "Press any key...\n");
	AnyKey();

	gameRegistry.Dump();


	/*
	LOGINFO( App::g_Log, "Test Loading Binary File\n" );
	{
		Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open( g_Options.m_BinaryFileOut, Sys::E_FileAccess_Read );		
		if(pFile == NULL)
		{
			LOGERROR(App::g_Log, "Failed to open output file");
			return -1;
		}
		
		if(g_Options.m_EndianSwapped)
		{
			pFile->SetEndianSwapFlag( true );
		}

		ShaderRegistry::C_Registry reg;

		reg.Load( *pFile );

		SAFE_DELETE_PTR( pFile );

		reg.Dump();
	}
	*/

	LOGINFO(App::g_Log, "\n");
	LOGINFO(App::g_Log, "Finished.\n");
	LOGINFO(App::g_Log, "Press any key...\n");
	AnyKey();

	return 0;
}

