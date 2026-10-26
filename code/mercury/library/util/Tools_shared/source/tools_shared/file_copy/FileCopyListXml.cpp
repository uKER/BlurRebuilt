#include "stdafx.h"

#include <tools_shared/PlatformHelpers.h>
#include <tools_shared/file_copy/FileCopy.h>

#include <system/serialise/C_FileSystem.h>
#include <system/serialise/TextSerialise.h>


using namespace Tools::FileCopy;

C_XmlElement::C_XmlElement()
{
}

C_XmlElement::~C_XmlElement()
{
	for (S32 i=0; i<m_Children.Size(); ++i)
	{
		if (m_Children[i]->DecRef())
		{
			LOGDEBUG( g_log, "XMLElement::~XMLElement - Possible memory leak, element was not destroyed\n");
		}
	}
}

Util::C_String& C_XmlElement::GetName()
{
	return m_Name;
}

const Util::C_String& C_XmlElement::GetName() const
{
	return m_Name;
}


Sys::C_XmlAttributeList& C_XmlElement::GetAttributeList()
{
	return m_Attributes;
}


const Sys::C_XmlAttributeList& C_XmlElement::GetAttributeList() const
{
	return m_Attributes;
}


void C_XmlElement::SetAttributeList(const Sys::C_XmlAttributeList &list)
{
	m_Attributes = list;
}


void C_XmlElement::SetName(const Util::C_String &name)
{
	m_Name = name;
}

XmlElementList& C_XmlElement::GetChildList()
{
	return m_Children;
}

const XmlElementList& C_XmlElement::GetChildList() const
{
	return m_Children;
}




C_FileCopyListXml::C_FileCopyListXml()
:	m_pRootElement(0)
{
}


C_FileCopyListXml::~C_FileCopyListXml()
{
	if (m_pRootElement)
		delete m_pRootElement;

	m_Stack.Clear();
}

void C_FileCopyListXml::Clear()
{
	if (m_pRootElement)
		delete m_pRootElement;
	
	m_Stack.Clear();
}


bool C_FileCopyListXml::StartDocument()
{
	Clear();
	return true;
}


bool C_FileCopyListXml::EndDocument()
{
	return true;
}


bool C_FileCopyListXml::StartElement(const Util::C_String &name, const Sys::C_XmlAttributeList &attributes)
{
	Util::C_String newName = name.Lower();
	
	C_XmlElement* pNewElement = new C_XmlElement();
	if (!pNewElement)
	{
		LOGDEBUG( g_log, "C_FileCopyListXML::StartDocument() - null pointer\n");
		return false;
	}
	pNewElement->SetName(newName);

	pNewElement->SetAttributeList(attributes);

	// If item already on stack, add new item as a child to it
	if (m_Stack.Size() > 0)
	{
		C_XmlElement *node = m_Stack.Back();
		node->GetChildList().PushBack(pNewElement);
	}
	else
	{
		//root node
		m_pRootElement = pNewElement;
	}
	m_Stack.PushBack(pNewElement);

	return true; 
}


bool C_FileCopyListXml::EndElement(const Util::C_String& name)
{
	C_XmlElement* node = m_Stack.Back();
	Util::C_String newName = name.Lower();

	assert(name == node->GetName());
	
	m_Stack.PopBack();
	return true;
}


bool C_FileCopyListXml::ReadFromFileCopyList(
	const C_FileCopyList &list,
	const Ut::UtVector<Ut::ArrayVector<C_PlatformCopyLocations, Util::E_Platform_Count> >& 
		locations)
{
	Clear();
	m_pRootElement = new C_XmlElement();
	if (!m_pRootElement)
	{
		LOGDEBUG( g_log, "C_FileCopyListXML::ReadFromFileCopyList() - null pointer\n");
		return false;
	}
	m_pRootElement->SetName("filecopylist");

	for (S32 i=0; i < Util::E_Platform_Count; ++i)
	{
		C_XmlElement *pPlatformElement = new C_XmlElement();
		if (!pPlatformElement)
		{
			LOGDEBUG( g_log, "FileCopyListXML::ReadFromFileCopyList() - null pointer\n");
			return false;
		}
		pPlatformElement->SetName("platform");
		pPlatformElement->GetAttributeList().Add("type", GetPlatformName((Util::E_Platform)i));

		const FileCopyOptionArray& fileCopyOptionArray = list.GetFileCopyOptionArray((Util::E_Platform)i);

		if (locations[i].devKit != "")
			pPlatformElement->GetAttributeList().Add("console", locations[i].devKit);

		for (S32 j=0; j<fileCopyOptionArray.Size(); ++j)
			pPlatformElement->GetAttributeList().Add(fileCopyOptionArray[j].m_Option, fileCopyOptionArray[j].m_Value);

		m_pRootElement->GetChildList().PushBack(pPlatformElement);


		const FileCopyItemArray& fileCopyItemArray = list.GetFileCopyItemArray((Util::E_Platform)i);
		for (S32 j=0; j<fileCopyItemArray.Size(); ++j)
		{
			C_XmlElement *pItemElement = new C_XmlElement();
			if (!pItemElement)
			{
				LOGDEBUG( g_log, "FileCopyListXML::ReadFromFileCopyList() - null pointer\n");
				return false;
			}
			if (fileCopyItemArray[j].m_FileCopyType == C_FileCopyItem::E_FileCopyItemType_Directory)
				pItemElement->SetName("directory");
			else if (fileCopyItemArray[j].m_FileCopyType == C_FileCopyItem::E_FileCopyItemType_File)
				pItemElement->SetName("file");

			pPlatformElement->GetChildList().PushBack(pItemElement);


			pItemElement->GetAttributeList().Add("src", fileCopyItemArray[j].m_FileCopySource);
			pItemElement->GetAttributeList().Add("dst", fileCopyItemArray[j].m_FileCopyDestination);
/*
			C_XmlElement *pSrcElement = new XmlElement();
			if (!pSrcElement)
			{
				LOGDEBUG( g_log, "FileCopyListXML::GenerateFromFileCopyList() - null pointer\n");
				return false;
			}
			pSrcElement->SetName("source");
			pSrcElement->GetAttributeList().Add("path", FileCopyItemArray[j].m_FileCopySource);

			pItemElement->GetChildList().PushBack(pSrcElement);


			C_XmlElement *pDstElement = new XmlElement();
			if (!pDstElement)
			{
				LOGDEBUG( g_log, "FileCopyListXML::GenerateFromFileCopyList() - null pointer\n");
				return false;
			}
			pDstElement->SetName("destination");
			pDstElement->GetAttributeList().Add("path", FileCopyItemArray[j].m_FileCopyDestination);

			pItemElement->GetChildList().PushBack(pDstElement);
*/
		}
	}
	return true;
}

bool C_FileCopyListXml::CopyToFileCopyList(C_FileCopyList &list) const
{
	if (!m_pRootElement)
		return true;

	Util::C_String rootName = m_pRootElement->GetName().Lower();
	if (rootName == "filecopylist")
	{
		for (S32 i=0; i<m_pRootElement->GetChildList().Size(); ++i)
		{
			C_XmlElement *pPlatformElement = m_pRootElement->GetChildList()[i];
			Util::C_String elementName = pPlatformElement->GetName().Lower();
			if (elementName != "platform")
			{
				LOGWARN( g_log, "FileCopyListXML::CopyToFileCopyList() - skipping unknown node\n");
				continue;
			}

			Util::C_String platformName;
			Sys::C_XmlAttributeList platformAttrList = pPlatformElement->GetAttributeList();
			const Sys::C_XmlAttribute *pPlatformAttr = platformAttrList.GetAttributeByName("type");

			if (pPlatformAttr)
				platformName = pPlatformAttr->value;
			else
			{
				LOGWARN( g_log, "FileCopyListXML::CopyToFileCopyList() - no platform type specified ... skipping\n");
				continue;
			}

			Util::E_Platform platform;

			if (platformName == GetPlatformName(Util::E_Platform_PC))
				platform = Util::E_Platform_PC;
			else if (platformName == GetPlatformName(Util::E_Platform_360))
				platform = Util::E_Platform_360;
			else if (platformName == GetPlatformName(Util::E_Platform_PS3))
				platform = Util::E_Platform_PS3;
			else
			{
				LOGWARN( g_log, "FileCopyListXML::CopyToFileCopyList() - skipping unknown platform\n");
				continue;
			}

			{
				const Sys::C_XmlAttribute *pConsoleName = platformAttrList.GetAttributeByName("console");
				if (pConsoleName)
				{
					if (platform == Util::E_Platform_360)
						list.AddFileCopyOption(Util::E_Platform_360, C_FileCopyOption(pConsoleName->name, pConsoleName->value));
					else
						LOGWARN( g_log, "FileCopyListXML::CopyToFileCopyList() - platform is not 360, ignoring console %s\n", pConsoleName->value);
				}
			}

			for (S32 j=0; j<pPlatformElement->GetChildList().Size(); ++j)
			{
				Util::C_String source, destination, type;

				C_XmlElement *pItemElement = pPlatformElement->GetChildList()[j];
				type = pItemElement->GetName().Lower();
				C_FileCopyItem::E_FileCopyItemType FileCopyType;

				if (type=="directory")
					FileCopyType = C_FileCopyItem::E_FileCopyItemType_Directory;
				else if (type=="file")
					FileCopyType = C_FileCopyItem::E_FileCopyItemType_File;
				else
				{
					LOGWARN( g_log, "FileCopyListXML::CopyToFileCopyList() - skipping unknown item\n");
					continue;
				}

				Sys::C_XmlAttributeList attributeList = pItemElement->GetAttributeList();
				
				const Sys::C_XmlAttribute *attribute;
				attribute = attributeList.GetAttributeByName("src");
				if (!attribute)
				{
					LOGWARN( g_log, "FileCopyListXML::CopyToFileCopyList() - no source path found ... skipping\n");
					continue;
				}
				source = attribute->value;

				attribute = attributeList.GetAttributeByName("dst");
				if (!attribute)
				{
					LOGWARN( g_log, "FileCopyListXML::CopyToFileCopyList() - no destination path found ... skipping\n");
					continue;
				}
				destination = attribute->value;

				list.AddFileCopyItem(platform, C_FileCopyItem(source, destination, FileCopyType));
			}
		}
	}
	else
		return false;

	return true;
}

C_XmlElement* C_FileCopyListXml::GetRoot()
{
	return m_pRootElement;
}


const C_XmlElement* C_FileCopyListXml::GetRoot() const
{
	return m_pRootElement;
}



void WriteElement(const C_XmlElement *element, Sys::TextSerialiser& stream, U32 depth)
{
	const char* indent = "  ";
	for (U32 i=0; i<depth; ++i)
		stream << indent;

	stream << "<" << element->GetName();
	for (S32 i=0; i<element->GetAttributeList().Elements(); ++i)
	{
		const Sys::C_XmlAttribute* attr = element->GetAttributeList()[i];
		stream << " " << attr->name << "=\"" << attr->value << "\"";
	}

	U32 numChildren = element->GetChildList().Size();
	if (numChildren>0)
	{
		stream << ">\n";
		for (U32 i=0; i<numChildren; ++i)
		{
			WriteElement(element->GetChildList()[i], stream, depth+1);
		}

		for (U32 i=0; i<depth; ++i)
			stream << indent;

		stream << "</" << element->GetName() << ">\n";
	}
	else
	{
		stream << "/>\n";
	}
}


bool Tools::FileCopy::XmlWrite(const C_FileCopyListXml& content, const char* document)
{
	Sys::C_FileSystem& fileSystem = Sys::C_FileSystem::Instance();

	Sys::I_File *xmlOutputFile = fileSystem.Open(
		document,
		Sys::E_FileAccess_Write);
	if (!xmlOutputFile)
	{
		LOGERROR( g_log, "FileCopy::XmlWrite() - Could not create file %s\n", document);
		return false;
	}

	Sys::TextSerialiser stream(*xmlOutputFile);

	const C_XmlElement *rootElement = content.GetRoot();
	if (rootElement)
		WriteElement(rootElement, stream, 0);

	xmlOutputFile->Overflow();
	xmlOutputFile->Flush();

	delete xmlOutputFile;

	return true;
}
