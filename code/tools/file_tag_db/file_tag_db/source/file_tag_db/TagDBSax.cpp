#include "stdafx.h"
#include <sstream>
#include <system/xml/XmlParser.h>
#include <file_tag_db/GenericAttribute.h>
#include <file_tag_db/Tag.h>
#include <file_tag_db/TagDB.h>
#include <file_tag_db/TagDBManager.h>
#include <file_tag_db/TagDBSax.h>
#include <system/misc/StringUtil.h>

using namespace FileTag;

C_TagDBSax::~C_TagDBSax()
{
}

bool C_TagDBSax::StartDocument() 
{ 
	m_pCurrentTag = 0;
	return true;
}

bool C_TagDBSax::EndDocument() 
{
	return true; 
}

bool C_TagDBSax::StartElement(
	const Util::C_String& _name, 
	const Sys::C_XmlAttributeList& attributes)
{
	if (_name == "file_data")
	{
		const Sys::C_XmlAttribute* pAttr = attributes.GetAttributeByName("file_name");
		if (pAttr)
		{
			assert(!m_pCurrentTag);
			Util::C_String tagFilePath = m_dbPath + pAttr->value.Lower();
//			printf("Added tag for %s\n", tagFilePath.Chars());
			m_pCurrentTag = C_TagDBManager::Instance().AddTag(tagFilePath);
		}
	}
	else if (_name == "container")
	{
		if (!DeserialiseContainer(attributes))
			return false;
	}

	return true; 
}

bool C_TagDBSax::EndElement(const Util::C_String& _name)
{
	if (_name == "file_data")
	{
		m_pCurrentTag = 0;
	}

	return true;
}

bool C_TagDBSax::DeserialiseContainer(const Sys::C_XmlAttributeList& attributes)
{
	const Sys::C_XmlAttribute* pName = 
		attributes.GetAttributeByName("name");

	const Sys::C_XmlAttribute* pType = 
		attributes.GetAttributeByName("type");

	const Sys::C_XmlAttribute* pValue = 
		attributes.GetAttributeByName("value");

	if (pName && pType && pValue)
	{
		if (pType->value == "int")
		{
			m_pCurrentTag->GetAttributes().PushBack();
			C_GenericAttribute& genericAttr = m_pCurrentTag->GetAttributes().Back();
			genericAttr.GetName() = pName->value;
			genericAttr.SetValue(atoi(pValue->value.Chars()));
		}
		else if (pType->value == "s64")
		{
			m_pCurrentTag->GetAttributes().PushBack();
			C_GenericAttribute& genericAttr = m_pCurrentTag->GetAttributes().Back();
			genericAttr.GetName() = pName->value;
			S64 val;
			std::stringstream ss; ss << pValue->value.Chars(); ss >> val;
			genericAttr.SetValue(val);
		}
		else if (pType->value == "float")
		{
			m_pCurrentTag->GetAttributes().PushBack();
			C_GenericAttribute& genericAttr = m_pCurrentTag->GetAttributes().Back();
			genericAttr.GetName() = pName->value;
			genericAttr.SetValue(atof(pValue->value.Chars()));
		}
		else if (pType->value == "f64")
		{
			m_pCurrentTag->GetAttributes().PushBack();
			C_GenericAttribute& genericAttr = m_pCurrentTag->GetAttributes().Back();
			genericAttr.GetName() = pName->value;
			F64 val;
			std::stringstream ss; ss << pValue->value.Chars(); ss >> val;
			genericAttr.SetValue(val);
		}
		else if (pType->value == "bool")
		{
			m_pCurrentTag->GetAttributes().PushBack();
			C_GenericAttribute& genericAttr = m_pCurrentTag->GetAttributes().Back();
			genericAttr.GetName() = pName->value;
			if (pValue->value == "1" || pValue->value == "true")
				genericAttr.SetValue(true);
			else
				genericAttr.SetValue(false);
		}
		else if (pType->value == "string")
		{
			m_pCurrentTag->GetAttributes().PushBack();
			C_GenericAttribute& genericAttr = m_pCurrentTag->GetAttributes().Back();
			genericAttr.GetName() = pName->value;
			genericAttr.SetValue(pValue->value, C_GenericAttribute::E_DataType_String);
		}
		else if (pType->value == "enum")
		{
			m_pCurrentTag->GetAttributes().PushBack();
			C_GenericAttribute& genericAttr = m_pCurrentTag->GetAttributes().Back();
			genericAttr.GetName() = pName->value;
			genericAttr.SetValue(pValue->value, C_GenericAttribute::E_DataType_StringEnum);
		}
		else if (pType->value == "filename")
		{
			m_pCurrentTag->GetAttributes().PushBack();
			C_GenericAttribute& genericAttr = m_pCurrentTag->GetAttributes().Back();
			genericAttr.GetName() = pName->value;
			genericAttr.SetValue(pValue->value, C_GenericAttribute::E_DataType_StringFilename);
		}
	}

	return true;
}

bool C_TagDBSax::Read(const char* filePath)
{
	Sys::I_File *pFile = Sys::C_FileSystem::Instance().Open(filePath);
	m_dbPath = Util::ExtractDirectory(filePath).Lower();
	
	if (!pFile)
	{
//		LOGERROR(g_Log, "XmlDom::Read - Unable to load file '%s'", filename);
		return false;
	}

	//Read file into buffer
	U32 c = (U32)pFile->Length();
	char *pBuffer = new char[c];
	pFile->Read(pBuffer, c);

	//Parse xml data
	XmlParse(*this, pBuffer, c);

	//Cleanup
	delete pBuffer;
	delete pFile;

	return true;
}