// ShaderCfg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <conio.h>

#include <system/serialise.h>
#include <system/serialise/C_FileSystem.h>

#include <System/DebugLog.h>

#include "ShaderConfig.h"
#include "ShaderRegistry.h"
#include "ShaderConfigParser.h"
#include "ShaderRegistryParser.h"
#include "Options.h"


namespace App
{

extern Debug::Logger &g_Log;

};

// C_OptionsConfig

C_OptionsConfig::C_OptionsConfig()
{
	m_BatchFileIn = "BatchIn.bat";
	m_BatchFileOut = "BatchOut.bat";

	m_DefineValueFormat = "";
	m_DefineFormat = "";

	m_BinaryFileOut = "";

	m_EndianSwapped = false;
}

C_OptionsConfig::~C_OptionsConfig()
{
}

// C_Options

C_Options::C_Options()
{
	m_ConfigFile = "ShaderCfg.xml";
	m_RegistryFile = "ShaderReg.xml";
}

C_Options::~C_Options()
{
}

void C_Options::Dump()
{
	LOGINFO(App::g_Log, "Options\n");
	LOGINFO(App::g_Log, "\n");
	LOGINFO(App::g_Log, "Config File : %s\n", m_ConfigFile.Chars());
	LOGINFO(App::g_Log, "Registry File : %s\n", m_RegistryFile.Chars());

	for(S32 i=0; i<m_ConfigList.Size(); i++)
	{
		C_OptionsConfig& config = m_ConfigList[i];

		LOGINFO(App::g_Log, "\n");
		LOGINFO(App::g_Log, "Config : %s\n", config.m_ConfigName.Chars());
		LOGINFO(App::g_Log, "   Define Format : %s\n", config.m_DefineFormat.Chars());
		LOGINFO(App::g_Log, "   Define Value Format : %s\n", config.m_DefineValueFormat.Chars());
		LOGINFO(App::g_Log, "   Batch File In : %s\n", config.m_BatchFileIn.Chars());
		LOGINFO(App::g_Log, "   Batch File Out : %s\n", config.m_BatchFileOut.Chars());
		LOGINFO(App::g_Log, "   Binary File Out : %s\n", config.m_BinaryFileOut.Chars());
		LOGINFO(App::g_Log, "   Endian Swapped : %d\n", config.m_EndianSwapped);
	}
}


bool C_Options::ReadXml(Util::C_String fileName)
{
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(fileName);

	if(!pFile)
	{
		LOGERROR(App::g_Log, "Error opening file\n");
		return false;
	}

	char* pBuffer = NULL;
	S32 bufferLength = 0;

	bufferLength = (S32)pFile->Length();
	pBuffer = new char[bufferLength];

	pFile->Read( pBuffer, bufferLength );

	LOGINFO(App::g_Log, "Reading XML\n");
	
	Sys::XmlParse(*this, pBuffer, bufferLength);	

	SAFE_DELETE_PTR(pFile);

	SAFE_DELETE_PTR(pBuffer);

	LOGINFO(App::g_Log, "Finished Reading XML\n");
	LOGINFO(App::g_Log, "\n");

	return true;
}

bool C_Options::StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes)
{
	if(name == "ConfigFile")
	{
		m_ConfigFile = attributes.GetValueByName("value");
	}
	else
	if(name == "RegistryFile")
	{
		m_RegistryFile = attributes.GetValueByName("value");
	}
	else
	if(name == "Config")
	{
		m_ConfigList.PushBack();
		C_OptionsConfig& config = m_ConfigList.Back();

		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByName("name");
		if(pAttribute)
		{
			config.m_ConfigName = pAttribute->value;
		}
	}
	else
	if(m_ConfigList.Size() > 0)
	{
		if(name == "DefineFormat")
		{
			C_OptionsConfig& config = m_ConfigList.Back();

			config.m_DefineFormat = attributes.GetValueByName("value");
		}
		else
		if(name == "DefineValueFormat")
		{
			C_OptionsConfig& config = m_ConfigList.Back();

			config.m_DefineValueFormat = attributes.GetValueByName("value");
		}
		else
		if(name == "BatchFileIn")
		{
			C_OptionsConfig& config = m_ConfigList.Back();

			config.m_BatchFileIn = attributes.GetValueByName("value");
		}
		else
		if(name == "BatchFileOut")
		{
			C_OptionsConfig& config = m_ConfigList.Back();

			config.m_BatchFileOut = attributes.GetValueByName("value");
		}
		else
		if(name == "BinaryFileOut")
		{
			C_OptionsConfig& config = m_ConfigList.Back();

			config.m_BinaryFileOut = attributes.GetValueByName("value");
		}
		else
		if(name == "EndianSwapped")
		{
			C_OptionsConfig& config = m_ConfigList.Back();

			Util::C_String value = attributes.GetValueByName("value");
			if(value == "true")
			{
				config.m_EndianSwapped = true;
			}
			else
			{
				config.m_EndianSwapped = false;
			}
		}
		else
		{
			LOGERROR(App::g_Log, "Unknown Element %s", name.Chars());
			return false;
		}
	}
	else
	{
		LOGERROR(App::g_Log, "Unknown Element %s", name.Chars());
		return false;
	}
	return true;
}
