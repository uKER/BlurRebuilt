
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

enum E_RegistryParseState
{
	E_RegistryParseState_ShaderRegistry,
	E_RegistryParseState_ShaderTable,
	E_RegistryParseState_IndexList,
	E_RegistryParseState_Combination,
	E_RegistryParseState_CombinationEnum,
	E_RegistryParseState_ShaderEnum,
	E_RegistryParseState_EnumItem
};

// C_ShaderRegistryXmlParser


C_ShaderRegistryXmlParser::C_ShaderRegistryXmlParser( C_ShaderRegistry* pShaderRegistry ) : 
	m_pShaderRegistry( pShaderRegistry )
{
}

C_ShaderRegistryXmlParser::~C_ShaderRegistryXmlParser()
{
}

// inherited from C_XmlContent
bool C_ShaderRegistryXmlParser::StartDocument()
{
	m_ParseStateStack.PushBack( E_RegistryParseState_ShaderRegistry );
	return true;
}

bool C_ShaderRegistryXmlParser::EndDocument()
{
	assert( m_ParseStateStack.Back() == E_RegistryParseState_ShaderRegistry );
	m_ParseStateStack.PopBack();

	m_pShaderRegistry->CalcInfo();
	m_pShaderRegistry->BuildTable();

	return true;
}

bool C_ShaderRegistryXmlParser::StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes)
{
	//LOGINFO(g_Log, "%sStartElement %s\n", Util::C_String(' ', m_ParseStateStack.Size() * 4).Chars(), name.Chars() );

	E_RegistryParseState state = (E_RegistryParseState)m_ParseStateStack.Back();

	switch(state)
	{
		case E_RegistryParseState_ShaderRegistry:
		{
			if(name == "ShaderEnum")
			{
				m_ParseStateStack.PushBack( E_RegistryParseState_ShaderEnum );
				return ProcessShaderEnum(attributes);
			}
			else
			if(name == "TableAxis")
			{
				m_ParseStateStack.PushBack( E_RegistryParseState_IndexList );
				return ProcessIndexList(attributes);
			}
			else
			if(name == "ShaderTable")
			{
				m_ParseStateStack.PushBack( E_RegistryParseState_ShaderTable );
				return true;
			}
			else
			{
				LOGERROR(g_Log, "Unexpected element %s\n", name.Chars() );
				return false;
			}
		}
		break;

		case E_RegistryParseState_ShaderEnum:
		{
			m_ParseStateStack.PushBack( E_RegistryParseState_EnumItem );
			return ProcessShaderEnumItem(attributes);
		}
		break;

		case E_RegistryParseState_IndexList:
		{
			if(name == "Combination")
			{
				m_ParseStateStack.PushBack( E_RegistryParseState_Combination );
				return ProcessCombination(attributes);
			}
			else
			{
				LOGERROR(g_Log, "Unexpected element %s\n", name.Chars() );
				return false;
			}
		}
		break;

		case E_RegistryParseState_Combination:
			if(name == "ShaderEnum")
			{
				m_ParseStateStack.PushBack( E_RegistryParseState_CombinationEnum );
				return ProcessCombinationEnum(attributes);
			}
			else
			{
				LOGERROR(g_Log, "Unexpected element %s\n", name.Chars() );
				return false;
			}
		break;

		case E_RegistryParseState_ShaderTable:
			m_ParseStateStack.PushBack( -1 );
			if(name == "OfflineAxis")
			{
				const Sys::C_XmlAttribute* pAttrName = attributes.GetAttributeByName("name");
				if(pAttrName)
				{
					m_pShaderRegistry->m_OfflineAxis = m_pShaderRegistry->LookupAxis(pAttrName->value);
				}
			}
			else
			if(name == "IngameAxis")
			{
				const Sys::C_XmlAttribute* pAttrName = attributes.GetAttributeByName("name");
				if(pAttrName)
				{
					m_pShaderRegistry->m_IngameAxis = m_pShaderRegistry->LookupAxis(pAttrName->value);
				}
			}
			else
			{
				LOGERROR(g_Log, "Unexpected element %s\n", name.Chars() );
				return false;
			}
		break;
	}

	return true;
}

bool C_ShaderRegistryXmlParser::EndElement(const Util::C_String& name)
{
	//LOGINFO(g_Log, "%sEndElement %s\n", Util::C_String(' ', (m_ParseStateStack.Size() -1) * 4).Chars(), name.Chars() );
	m_ParseStateStack.PopBack();
	return true;
}

const bool C_ShaderRegistryXmlParser::ProcessShaderEnum(const Sys::C_XmlAttributeList& attributes)
{
	const S32 enumIndex = m_pShaderRegistry->m_ShaderEnumList.Size();
	m_pShaderRegistry->m_ShaderEnumList.PushBack();
	C_ShaderEnum& shaderEnum = m_pShaderRegistry->m_ShaderEnumList[enumIndex];

	shaderEnum.m_Index = enumIndex;

	const Sys::C_XmlAttribute* pAttrName = attributes.GetAttributeByName("name");
	if(pAttrName)
	{
		shaderEnum.m_Name = pAttrName->value;
	}

	return true;
}

const bool C_ShaderRegistryXmlParser::ProcessShaderEnumItem(const Sys::C_XmlAttributeList& attributes)
{
	const S32 enumIndex = m_pShaderRegistry->m_ShaderEnumList.Size() - 1;
	C_ShaderEnum& shaderEnum = m_pShaderRegistry->m_ShaderEnumList[enumIndex];

	const S32 enumItemIndex = shaderEnum.m_ItemList.Size();
	shaderEnum.m_ItemList.PushBack();
	C_ShaderEnumItem& shaderEnumItem = shaderEnum.m_ItemList[enumItemIndex];

	const Sys::C_XmlAttribute* pAttrName = attributes.GetAttributeByName("name");
	if(pAttrName)
	{
		shaderEnumItem.m_Name = pAttrName->value;
	}

	return true;
}

const bool C_ShaderRegistryXmlParser::ProcessIndexList(const Sys::C_XmlAttributeList& attributes)
{
	const S32 index = m_pShaderRegistry->m_ShaderTableAxisList.Size();
	m_pShaderRegistry->m_ShaderTableAxisList.PushBack();
	C_ShaderTableAxis& shaderTableAxis = m_pShaderRegistry->m_ShaderTableAxisList[index];

	shaderTableAxis.m_Index = index;

	const Sys::C_XmlAttribute* pAttrName = attributes.GetAttributeByName("name");
	if(pAttrName)
	{
		shaderTableAxis.m_Name = pAttrName->value;
	}

	return true;
}

const bool C_ShaderRegistryXmlParser::ProcessCombination(const Sys::C_XmlAttributeList& attributes)
{
	const S32 indexIndex = m_pShaderRegistry->m_ShaderTableAxisList.Size() - 1;
	C_ShaderTableAxis& shaderTableAxis = m_pShaderRegistry->m_ShaderTableAxisList[indexIndex];

	const S32 combinationIndex = shaderTableAxis.m_AxisCombinationList.Size();
	shaderTableAxis.m_AxisCombinationList.PushBack();
	C_AxisCombination& axisCombination = shaderTableAxis.m_AxisCombinationList[combinationIndex];

	const Sys::C_XmlAttribute* pAttrName = attributes.GetAttributeByName("name");
	if(pAttrName)
	{
		axisCombination.m_Name = pAttrName->value;
	}	

	return true;
}


const bool C_ShaderRegistryXmlParser::ProcessCombinationEnum(const Sys::C_XmlAttributeList& attributes)
{
	const S32 indexIndex = m_pShaderRegistry->m_ShaderTableAxisList.Size() - 1;
	C_ShaderTableAxis& shaderTableAxis = m_pShaderRegistry->m_ShaderTableAxisList[indexIndex];

	const S32 combinationIndex = shaderTableAxis.m_AxisCombinationList.Size() - 1;
	C_AxisCombination& axisCombination = shaderTableAxis.m_AxisCombinationList[combinationIndex];

	const S32 combinationEnumIndex = axisCombination.m_EnumList.Size();
	axisCombination.m_EnumList.PushBack();
	C_AxisCombinationEnum& combinationEnum = axisCombination.m_EnumList[combinationEnumIndex];

	const Sys::C_XmlAttribute* pAttrName = attributes.GetAttributeByName("name");
	if(pAttrName)
	{
		combinationEnum.m_EnumType = m_pShaderRegistry->LookupEnum(pAttrName->value);
		if(combinationEnum.m_EnumType != -1)
		{
			combinationEnum.m_pEnum = &m_pShaderRegistry->m_ShaderEnumList[combinationEnum.m_EnumType];
		}
		else
		{
			LOGERROR(g_Log, "Enum %s not found\n", pAttrName->value );
		}
	}	

	return true;
}

bool C_ShaderRegistryXmlParser::ReadXml( Util::C_String fileName )
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

	ShaderCfg::C_ShaderRegistryXmlParser xmlParser( m_pShaderRegistry );
	Sys::XmlParse(xmlParser, pBuffer, bufferLength);	

	SAFE_DELETE_PTR(pFile);

	SAFE_DELETE_PTR(pBuffer);

	LOGINFO(g_Log, "Finished Reading XML\n");
	LOGINFO(g_Log, "\n");
	
	return true;
}



} // namespace ShaderCfg