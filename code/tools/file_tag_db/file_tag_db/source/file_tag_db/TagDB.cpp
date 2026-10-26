#include "stdafx.h"

#include <file_tag_db/GenericAttribute.h>
#include <file_tag_db/Tag.h>
#include <file_tag_db/TagDB.h>
#include <file_tag_db/TagDBSax.h>
#include <file_tag_db/TagDBManager.h>
#include <system/misc/StringUtil.h>

#include <sstream>

using namespace FileTag;


C_TagDB::C_TagDB(const C_TagDBManager& tagDBManager) : m_tagDBManager(tagDBManager)
{
}

C_TagDB::~C_TagDB()
{
	for (S32 i = 0; i < m_tags.Size(); i++)
	{
		delete m_tags[i];
	}
}

C_Tag* C_TagDB::AddTag(const char* pFileName)
{
	Util::C_String lower(Util::C_String(pFileName).Lower());
	for (S32 i = 0; i < m_tags.Size(); i++)
		if (m_tags[i]->GetFileName() == lower)
			return m_tags[i];

	C_Tag* pNewTag = new C_Tag();
	assert(pNewTag);
	m_tags.PushBack(pNewTag);
	pNewTag->GetFileName() = lower;
	return pNewTag;
}

U32 C_TagDB::GetNumTags() const
{
	return (U32)m_tags.Size();
}

void C_TagDB::Clear()
{
	for (S32 i = 0; i < m_tags.Size(); i++)
	{
		delete m_tags[i];
	}
	
	m_tags.Clear();
	m_tagDBManager.DirtyQueryCache();
}

bool C_TagDB::DeserialiseXmlDom(const Sys::C_XmlDom& dom)
{
	if (!dom.GetRootNodePtr())
		return false;

	const Sys::C_XmlDomBaseNode* pFileListNode = 
		Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), "files");
	if (pFileListNode)
	{
		for (S32 iChild = 0; iChild < pFileListNode->GetChildren().Size(); iChild++)
		{
			const Sys::C_XmlDomBaseNode* pChild = pFileListNode->GetChildren()[iChild];
			if (pChild && pChild->GetType() == Sys::E_XmlDomNodeType_Element && pChild->GetName() == "file_data")
			{
				if (!DeserialiseTagXmlDom((const Sys::C_XmlDomElement*)pChild))
					return false;
			}
		}
	}

	return true;
}

bool C_TagDB::DeserialiseTagXmlDom(const Sys::C_XmlDomElement* pElement)
{
	//Add a new file record to our database.
	C_Tag* pNewTag = new C_Tag();
	assert(pNewTag);
	m_tags.PushBack(pNewTag);

	C_Tag& tag = *pNewTag;

	for (S32 i = 0; i < pElement->GetAttributes().Elements(); i++)
	{
		const Sys::C_XmlAttribute* pAttr = pElement->GetAttributes()[i];
		if (!pAttr)
			continue;
		if (pAttr->name == "file_name")
		{
			tag.GetFileName() = pAttr->value;
			break;
		}
	}

	for (S32 iChild = 0; iChild < pElement->GetChildren().Size(); iChild++)
	{
		Sys::C_XmlDomBaseNode* pChild = pElement->GetChildren()[iChild];
		if (pChild->GetName() == "values" && pChild->GetType() == Sys::E_XmlDomNodeType_Element)
			if (!DeserialiseContainerXmlDom(tag, (Sys::C_XmlDomElement*)pChild))
				return false;
	}

	return true;
}

bool C_TagDB::DeserialiseContainerXmlDom(C_Tag& tag, const Sys::C_XmlDomElement* pFileParamNode)
{
	const Sys::C_XmlAttribute* pNodeName = 
		pFileParamNode->GetAttributes().GetAttributeByName("name");
	if (pFileParamNode->GetName() == "container")
	{
		const Sys::C_XmlAttribute* pType = 
			pFileParamNode->GetAttributes().GetAttributeByName("type");

		const Sys::C_XmlAttribute* pValue = 
			pFileParamNode->GetAttributes().GetAttributeByName("value");

		if (pType && pValue)
		{
			if (pType->value == "int")
			{
				tag.GetAttributes().PushBack();
				C_GenericAttribute& genericAttr = tag.GetAttributes().Back();
				genericAttr.GetName() = pNodeName->value;
				genericAttr.SetValue(atoi(pValue->value.Chars()));
			}
			if (pType->value == "s64")
			{
				tag.GetAttributes().PushBack();
				C_GenericAttribute& genericAttr = tag.GetAttributes().Back();
				genericAttr.GetName() = pNodeName->value;
				std::stringstream ss;
				ss << pValue->value.Chars();
				S64 s64;
				ss >> s64;
				genericAttr.SetValue(s64);
			}
			else if (pType->value == "float")
			{
				tag.GetAttributes().PushBack();
				C_GenericAttribute& genericAttr = tag.GetAttributes().Back();
				genericAttr.GetName() = pNodeName->value;
				genericAttr.SetValue(atof(pValue->value.Chars()));
			}
			if (pType->value == "f64")
			{
				tag.GetAttributes().PushBack();
				C_GenericAttribute& genericAttr = tag.GetAttributes().Back();
				genericAttr.GetName() = pNodeName->value;
				std::stringstream ss;
				ss << pValue->value.Chars();
				F64 f64;
				ss >> f64;
				genericAttr.SetValue(f64);
			}
			else if (pType->value == "bool")
			{
				tag.GetAttributes().PushBack();
				C_GenericAttribute& genericAttr = tag.GetAttributes().Back();
				genericAttr.GetName() = pNodeName->value;
				if (pValue->value == "1" || pValue->value == "true")
					genericAttr.SetValue(true);
				else
					genericAttr.SetValue(false);
			}
			else if (pType->value == "string")
			{
				tag.GetAttributes().PushBack();
				C_GenericAttribute& genericAttr = tag.GetAttributes().Back();
				genericAttr.GetName() = pNodeName->value;
				genericAttr.SetValue(pValue->value, C_GenericAttribute::E_DataType_String);
			}
			else if (pType->value == "enum")
			{
				tag.GetAttributes().PushBack();
				C_GenericAttribute& genericAttr = tag.GetAttributes().Back();
				genericAttr.GetName() = pNodeName->value;
				genericAttr.SetValue(pValue->value, C_GenericAttribute::E_DataType_StringEnum);
			}
			else if (pType->value == "filename")
			{
				tag.GetAttributes().PushBack();
				C_GenericAttribute& genericAttr = tag.GetAttributes().Back();
				genericAttr.GetName() = pNodeName->value;
				genericAttr.SetValue(pValue->value, C_GenericAttribute::E_DataType_StringFilename);
			}
		}
	}

	for (S32 iChild = 0; iChild < pFileParamNode->GetChildren().Size(); iChild++)
	{
		Sys::C_XmlDomBaseNode* pChild = pFileParamNode->GetChildren()[iChild];
		if (pChild->GetName() == "container" && pChild->GetType() == Sys::E_XmlDomNodeType_Element)
			if (!DeserialiseContainerXmlDom(tag, (Sys::C_XmlDomElement*)pChild))
				return false;
	}

	return true;
}

bool C_TagDB::SerialiseContainerXmlDom(Sys::C_XmlDomElement* pContainerNode, const C_GenericAttribute& attr) const
{
	pContainerNode->GetName() = "container";
	pContainerNode->GetAttributes().Add("name", attr.GetName());

	Util::C_String valueStr;
	switch (attr.GetDataType())
	{
		case C_GenericAttribute::E_DataType_S32:
			{
				pContainerNode->GetAttributes().Add("type", "int");
				std::stringstream ss;
				S32 value;
				attr.GetValue(value);
				ss << value;
				valueStr = ss.str().c_str();
				break;
			}
		case C_GenericAttribute::E_DataType_S64:
			{
				pContainerNode->GetAttributes().Add("type", "s64");
				std::stringstream ss;
				S64 value;
				attr.GetValue(value);
				ss << value;
				valueStr = ss.str().c_str();
				break;
			}
		case C_GenericAttribute::E_DataType_F32:
			{
				pContainerNode->GetAttributes().Add("type", "float");
				std::stringstream ss;
				F32 value;
				attr.GetValue(value);
				ss << value;
				valueStr = ss.str().c_str();
				break;
			}
		case C_GenericAttribute::E_DataType_F64:
			{
				pContainerNode->GetAttributes().Add("type", "f64");
				std::stringstream ss;
				F64 value;
				attr.GetValue(value);
				ss << value;
				valueStr = ss.str().c_str();
				break;
			}
		case C_GenericAttribute::E_DataType_Boolean:
			{
				pContainerNode->GetAttributes().Add("type", "bool");
				bool value;
				attr.GetValue(value);
				valueStr = value ? "true" : "false";
				break;
			}
		case C_GenericAttribute::E_DataType_String: // Can also be an enum really
			{
				pContainerNode->GetAttributes().Add("type", "string");
				attr.GetValue(valueStr);
				break;
			}
		case C_GenericAttribute::E_DataType_StringEnum: // Can also be an enum really
			{
				pContainerNode->GetAttributes().Add("type", "enum");
				attr.GetValue(valueStr);
				break;
			}
		case C_GenericAttribute::E_DataType_StringFilename: // Can also be an enum really
			{
				pContainerNode->GetAttributes().Add("type", "filename");
				attr.GetValue(valueStr);
				break;
			}
	}
	pContainerNode->GetAttributes().Add("value", valueStr);

	return true;
}

bool C_TagDB::SerialiseTagXmlDom(
	Sys::C_XmlDomElement& tagParent, 
	const C_Tag& tag) const
{
	//Create tag node
	Sys::C_XmlDomElement* pTagNode = new Sys::C_XmlDomElement();
	pTagNode->GetName() = "file_data";
	tagParent.AddChild(pTagNode);

	pTagNode->GetAttributes().Add("type_name", "image");
	pTagNode->GetAttributes().Add("file_name", tag.GetFileName());

	Sys::C_XmlDomElement* pValuesNode = new Sys::C_XmlDomElement();
	pValuesNode->GetName() = "values";
	pTagNode->AddChild(pValuesNode);

	Sys::C_XmlDomElement* pRootContainerNode = new Sys::C_XmlDomElement();
	pRootContainerNode->GetName() = "container";
	pValuesNode->AddChild(pRootContainerNode);
	pRootContainerNode->GetAttributes().Add("name", "texture_properties");
	pRootContainerNode->GetAttributes().Add("type", "basic");

	for (U32 i = 0; i < tag.GetNumAttributes(); i++)
	{
		Sys::C_XmlDomElement* pContainer = new Sys::C_XmlDomElement();
		const FileTag::C_GenericAttribute& attr = 
			tag.GetAttributes()[i];
		if (!SerialiseContainerXmlDom(pContainer, attr))
			return false;

		pRootContainerNode->AddChild(pContainer);
	}

	return true;
}

bool C_TagDB::SerialiseXmlDom(Sys::C_XmlDom& dom) const
{
	//Create a root node
	Sys::C_XmlDomElement* pRootNode = new Sys::C_XmlDomElement();
	pRootNode->GetName() = "root";
	dom.SetRootNodePtr(pRootNode);

	Sys::C_XmlDomElement* pFilesNode = new Sys::C_XmlDomElement();
	pFilesNode->GetName() = "files";
	pRootNode->AddChild(pFilesNode);

	//Loop thru files.
	for (S32 i = 0; i < m_tags.Size(); i++)
		if (!SerialiseTagXmlDom(*pFilesNode, *m_tags[i]))
			return false;

	return true;
}

bool C_TagDB::Read(const char* pFileName)
{
	if (!Sys::C_FileSystem::Instance().FileExists(pFileName))
		return false;

	Clear();
/*	
	Sys::C_XmlDom dom;
	if (!dom.Read(pFileName))
		return false;
   
	//Parse.
	if (!DeserialiseXmlDom(dom))
		return false;
*/
	C_TagDBSax sax;
	if (!sax.Read(pFileName))
		return false;

	return true;
}

bool C_TagDB::Write(const char* pFileName) const
{
	Sys::C_XmlDom dom;

	if (!SerialiseXmlDom(dom))
		return false;

	if (!dom.Write(pFileName))
		return false;

	return true;
}

S32 C_TagDB::GetTagIndex(const char* pFileName) const
{
	for (U32 i = 0; i < (U32)m_tags.Size(); i++)
	{
		if (m_tags[i]->GetFileName().Lower() == Util::ExtractFilename(Util::C_String(pFileName).Lower()))
			return i;
	}
	return -1;
}

C_Tag* C_TagDB::GetTagPtr(U32 index)
{
	return m_tags[index];
}

const C_Tag* C_TagDB::GetTagPtr(U32 index) const
{
	return m_tags[index];
}

const C_Tag* C_TagDB::GetTagPtr(const char* pFileName) const
{
	return const_cast<C_TagDB*>(this)->GetTagPtr(pFileName);
}

C_Tag* C_TagDB::GetTagPtr(const char* pFileName)
{
	S32 index = GetTagIndex(pFileName);
	if (index == -1)
		return 0;

	return GetTagPtr(index);
}
