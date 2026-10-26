#include "SystemPch.h"
#include <core/containers/string.h>

#include <system/serialise.h>
#include <system/serialise/TextSerialise.h>
#include <system/serialise/C_MemoryStream.h>

#include <system/xml/Definitions.h>
#include <system/xml/XmlParser.h>
#include <system/xml/XmlDom.h>
#include <system/misc/StringUtil.h>

using namespace Sys;


C_XmlDom::C_XmlDom() : C_XmlDomContent()
{
	m_pRootNode = 0;
}

C_XmlDom::~C_XmlDom()
{
}

void C_XmlDom::SetRootNodePtr(C_XmlDomBaseNode* pRoot)
{
	m_pRootNode = pRoot;
	m_stack.PushBack(m_pRootNode);
}

C_XmlDomBaseNode* C_XmlDom::GetRootNodePtr()
{
	return m_pRootNode;
}

const C_XmlDomBaseNode* C_XmlDom::GetRootNodePtr() const
{
	return m_pRootNode;
}


bool C_XmlDom::Read(const char* filename)
{
	Sys::I_File *pFile = Sys::GetDefaultFileSystem()->Open(filename);
	
	if (!pFile)
	{
//		LOGERROR(g_Log, "XmlDom::Read - Unable to load file '%s'", filename);
		return false;
	}

	//Read file into buffer
	U32 c = (U32)pFile->Length();
	char *pBuffer = new char[c];
	pFile->Read(pBuffer, c);

	//Create a root node
	m_pRootNode = new C_XmlDomBaseNode();
	m_pRootNode->GetName() = "root";
	m_stack.PushBack(m_pRootNode);

	//Parse xml data
	XmlParse(*this, pBuffer, c);

	//Cleanup
	delete [] pBuffer;
	delete pFile;

	return true;
}

void C_XmlDom::XmlSerialiseIndent(Sys::TextSerialiser& xml, U32 depth) const
{
	for (U32 i = 0; i < depth; i++)
		xml << "\t";
}

void C_XmlDom::XmlSerialiseNewLine(Sys::TextSerialiser& xml) const
{
	xml << "\n";
}

bool C_XmlDom::Write(const char* pFileName) const
{
	if (!m_pRootNode)
		return false;

	//Create output folder.
	if (!Sys::GetDefaultFileSystem()->CreateDirectory(Sys::ExtractDirectory(pFileName)))
		// "Failed to create directory. Check folder permissions."));
		return false;

	Sys::I_File *pFile = Sys::GetDefaultFileSystem()->Open(pFileName, Sys::E_FileAccess_Write);
	if (!pFile)
		return false;

	//Grow in 1Mb chunks.
	Sys::C_MemoryStream xml(1024*1024);
	if (!Serialise(xml, *m_pRootNode))
	{
		delete pFile;
		return false;
	}

	if (!pFile->Write(xml.GetBufferPtr(), (U32)xml.GetBufferSize()))
	{
		delete pFile;
		return false;
	}

	delete pFile;
	return true;
}

void C_XmlDom::XmlSerialiseAttribute(Sys::TextSerialiser& xml, const Sys::C_XmlAttribute& attr) const
{
	xml << " " << attr.name << "=\"" << attr.value << "\"";
}

bool C_XmlDom::XmlSerialiseElement(Sys::TextSerialiser& xml, U32 depth, Sys::C_XmlDomElement& node) const
{
	XmlSerialiseIndent(xml, depth);

	xml << "<";
	xml << node.GetName();
	
	const Sys::C_XmlAttributeList& attributes = node.GetAttributes();

	for (S32 i = 0; i < attributes.Elements(); i++)
		if (attributes[i])
			XmlSerialiseAttribute(xml, *attributes[i]);

	if (node.GetChildren().Size() == 0)
	{
		xml << "/>";
	}
	else
	{
		xml << ">";
	}
	XmlSerialiseNewLine(xml);

	for (S32 iChild = 0; iChild < node.GetChildren().Size(); iChild++)
	{
		Sys::C_XmlDomBaseNode* pChild = node.GetChildren()[iChild];
		if (pChild)
			if (!XmlSerialise(xml, depth+1, *pChild))
				return false;
	}

	if (node.GetChildren().Size() != 0)
	{
		XmlSerialiseIndent(xml, depth);
		xml << "</";
		xml << node.GetName();
		xml << ">";
		XmlSerialiseNewLine(xml);
	}
	return true;
}

bool C_XmlDom::XmlSerialise(Sys::TextSerialiser& xml, U32 depth, Sys::C_XmlDomBaseNode& node) const
{
	//This element ending needs cleaning up but I don't have time right now.
	if (node.GetType() == Sys::E_XmlDomNodeType_Element)
	{
		XmlSerialiseElement(xml, depth, (C_XmlDomElement&)node);
		depth++;
	}
	else
	{
		for (S32 iChild = 0; iChild < node.GetChildren().Size(); iChild++)
		{
			Sys::C_XmlDomBaseNode* pChild = node.GetChildren()[iChild];
			if (pChild)
				if (!XmlSerialise(xml, depth, *pChild))
					return false;
		}
	}

	return true;
}

bool C_XmlDom::Serialise(Sys::I_BinaryStream& xml, Sys::C_XmlDomBaseNode& node) const
{
	if (!m_pRootNode)
		return false;

	Sys::TextSerialiser serialiser(xml);
	return XmlSerialise(serialiser, 0, *m_pRootNode);
}
