
#include "stdafx.h"

#include <System/DebugLog.h>

#include <core/utility/endian.h>
#include <system/Serialise.h>
#include <system/serialise/C_FileBlock.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

#include "ShaderConfig.h"
#include "ShaderRegistry.h"

#include "ShaderConfigParser.h"
#include "ShaderRegistryParser.h"

namespace ShaderCfg
{

enum E_ConfigParseState
{
	E_ConfigParseState_ShaderConfig,
	E_ConfigParseState_Fragment,
	E_ConfigParseState_Define,
	E_ConfigParseState_FragmentDefine,
	E_ConfigParseState_FragmentCondition
};

// C_ShaderConfigXmlParser


C_ShaderConfigXmlParser::C_ShaderConfigXmlParser( C_ShaderConfig* pShaderConfig ) : 
	m_pShaderConfig( pShaderConfig )
{
}

C_ShaderConfigXmlParser::~C_ShaderConfigXmlParser()
{
}

// inherited from C_XmlContent
bool C_ShaderConfigXmlParser::StartDocument()
{
	m_ParseStateStack.PushBack( E_ConfigParseState_ShaderConfig );
	return true;
}

bool C_ShaderConfigXmlParser::EndDocument()
{
	assert( m_ParseStateStack.Back() == E_ConfigParseState_ShaderConfig );
	m_ParseStateStack.PopBack();

	return true;
}

bool C_ShaderConfigXmlParser::StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes)
{
	//LOGINFO(g_Log, "%sStartElement %s\n", Util::C_String(' ', m_ParseStateStack.Size() * 4).Chars(), name.Chars() );

	E_ConfigParseState state = (E_ConfigParseState)m_ParseStateStack.Back();

	switch(state)
	{
		case E_ConfigParseState_ShaderConfig:
		{
			if(name == "Define")
			{
				m_ParseStateStack.PushBack( E_ConfigParseState_Define );
				ProcessDefine(attributes);
				return true;
			}
			else
			if(name == "Fragment")
			{
				m_ParseStateStack.PushBack( E_ConfigParseState_Fragment );
				return ProcessFragment(attributes);
			}
			else
			{
				LOGERROR(g_Log, "Unexpected element %s\n", name.Chars() );
				return false;
			}
		}
		break;

		case E_ConfigParseState_Fragment:
		{
			if(name == "Define")
			{
				m_ParseStateStack.PushBack( E_ConfigParseState_FragmentDefine );
				return ProcessFragmentDefine( attributes );
			}
			else
			if(name == "Condition")
			{
				m_ParseStateStack.PushBack( E_ConfigParseState_FragmentCondition );
				return ProcessFragmentCondition( attributes );
			}
			else
			{
				LOGERROR(g_Log, "Unexpected element %s\n", name.Chars() );
				return false;
			}
		}
		break;
	}

	return true;
}

bool C_ShaderConfigXmlParser::EndElement(const Util::C_String& name)
{
	//LOGINFO(g_Log, "%sEndElement %s\n", Util::C_String(' ', (m_ParseStateStack.Size() -1) * 4).Chars(), name.Chars() );
	m_ParseStateStack.PopBack();
	return true;
}


const bool C_ShaderConfigXmlParser::ProcessDefine(const Sys::C_XmlAttributeList& attributes)
{
	const S32 index = m_pShaderConfig->m_DefineList.Size();
	m_pShaderConfig->m_DefineList.PushBack();
	C_Define& define = m_pShaderConfig->m_DefineList[index];

	define.m_Index = index;
	for(S32 i=0; i<attributes.Elements(); i++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex(i);

		if(pAttribute->name == "name")
		{
			define.m_Name = pAttribute->value;
		}
		else
		if(pAttribute->name == "define")
		{
			define.m_Define = pAttribute->value; 
		}
		else
		if(pAttribute->name == "value")
		{
			define.m_Value = pAttribute->value;
		}
		else
		if(pAttribute->name == "vertex_shader")
		{
			if(pAttribute->value == "true")
			{
				define.m_VertexShader = true;
			}
			else
			{
				define.m_VertexShader = false;
			}
		}
		else
		if(pAttribute->name == "pixel_shader")
		{
			if(pAttribute->value == "true")
			{
				define.m_PixelShader = true;
			}
			else
			{
				define.m_PixelShader = false;
			}
		}
		else
		if(pAttribute->name == "text_id")
		{
			define.m_TextId = pAttribute->value;
		}
		else
		{
			LOGERROR(g_Log, "Unknown attribute %s\n", pAttribute->name.Chars());
			return false;
		}
	}

	return true;
}

const bool C_ShaderConfigXmlParser::ProcessFragment(const Sys::C_XmlAttributeList& attributes)
{
	const S32 index = m_pShaderConfig->m_FragmentList.Size();
	m_pShaderConfig->m_FragmentList.PushBack();
	C_Fragment& fragment = m_pShaderConfig->m_FragmentList[index];

	fragment.m_pShaderConfig = m_pShaderConfig;
	fragment.m_Index = index;
	for(S32 i=0; i<attributes.Elements(); i++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex(i);

		if(pAttribute->name == "name")
		{
			fragment.m_Name = pAttribute->value;
		}
		else
		{
			LOGERROR(g_Log, "Unknown attribute %s\n", pAttribute->name.Chars());
			return false;
		}
	}

	return true;
}

const bool C_ShaderConfigXmlParser::ProcessFragmentDefine(const Sys::C_XmlAttributeList& attributes)
{
	const S32 index = m_pShaderConfig->m_FragmentList.Size() - 1;
	C_Fragment& fragment = m_pShaderConfig->m_FragmentList[index];

	for(S32 i=0; i<attributes.Elements(); i++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex(i);

		if(pAttribute->name == "name")
		{
			C_Define* pDefine = m_pShaderConfig->LookupDefine( pAttribute->value );
			if( pDefine != NULL )
			{
				fragment.m_DefineList.PushBack( pDefine->m_Index );				
			}
			else
			{
				LOGERROR(g_Log, "Define \"%s\" not found\n", pAttribute->value.Chars());
				return false;
			}
		}
		else
		{
			LOGERROR(g_Log, "Unknown attribute %s\n", pAttribute->name.Chars());
			return false;
		}
	}

	return true;
}

const bool C_ShaderConfigXmlParser::ProcessFragmentCondition(const Sys::C_XmlAttributeList& attributes)
{
	const S32 fragmentIndex = m_pShaderConfig->m_FragmentList.Size() - 1;
	C_Fragment& fragment = m_pShaderConfig->m_FragmentList[fragmentIndex];

	fragment.m_ConditionList.PushBack();
	const S32 conditionIndex = fragment.m_ConditionList.Size() - 1;
	C_Condition& condition = fragment.m_ConditionList[conditionIndex];

	for(S32 i=0; i<attributes.Elements(); i++)
	{
		const Sys::C_XmlAttribute* pAttribute = attributes.GetAttributeByIndex(i);

		if(pAttribute->name == "type")
		{
			if(pAttribute->value == "Required")
			{
				condition.m_Type = E_ConditionType_Required;
			}
			else
			if(pAttribute->value == "Exclude")
			{
				condition.m_Type = E_ConditionType_Exclude;
			}
			else
			if(pAttribute->value == "Mutex")
			{
				condition.m_Type = E_ConditionType_Mutex;
			}
			else				
			{
				LOGERROR(g_Log, "Unknown condition type %s\n", pAttribute->value.Chars());
				return false;
			}
		}
		else
		if(pAttribute->name == "id")
		{
			condition.m_Id = pAttribute->value;
		}

		else
		{
			LOGERROR(g_Log, "Unknown attribute %s\n", pAttribute->name.Chars());
			return false;
		}
	}

	return true;
}

bool C_ShaderConfigXmlParser::ReadXml( Util::C_String fileName )
{
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(fileName);

	if(!pFile)
	{
		LOGERROR(g_Log, "Error opening file\n");
		return false;
	}

	char* pBuffer = NULL;
	S32 bufferLength = 0;

	bufferLength = (S32)pFile->Length();
	pBuffer = new char[bufferLength];

	pFile->Read( pBuffer, bufferLength );

	LOGINFO(g_Log, "Reading XML\n");

	ShaderCfg::C_ShaderConfigXmlParser xmlParser( m_pShaderConfig );
	Sys::XmlParse(xmlParser, pBuffer, bufferLength);	

	SAFE_DELETE_PTR(pFile);

	SAFE_DELETE_PTR(pBuffer);

	LOGINFO(g_Log, "Finished Reading XML\n");
	LOGINFO(g_Log, "\n");

	return true;
}

} // namespace ShaderCfg