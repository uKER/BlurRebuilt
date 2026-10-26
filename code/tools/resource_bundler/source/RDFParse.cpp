#include "stdafx.h"

#include "RDFParse.h"
#include "BundlerOutputOptions.h"
#include "Bundler.h"
#include "export_resource/ExportTextureOptions.h"
#include "process_texture/IProcessTexture.h"
#include "process_texture/ProcessTexture2D.h"
#include "process_texture/ProcessArrayOfTextures2D.h"
#include "process_texture/ProcessTexture3D.h"
#include "process_texture/ProcessTextureArray2D.h"
#include "process_texture/ProcessTextureCubeMap.h"

using namespace ResourceBundler;

C_RDFParser::C_RDFParser()
{
}

bool C_RDFParser::LoadRDF(C_Bundler *pBundler)
{
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(pBundler->RDFileName());

	if (!pFile)
		return false;

	int len = (int) pFile->Length();

	char* buf = new char[len];
	pFile->Read(buf, len);
	delete pFile;

	C_RDFContent rdfContent(pBundler);
	Sys::E_XmlResult res = Sys::XmlParse(rdfContent, buf, len);
	delete[] buf;
	if (res != Sys::E_XmlResult_Success)
	{
		printf("Error: Couldn't parse '%s'\n", pBundler->RDFileName().Chars());
		return false;
	}

	if (!rdfContent.Parse())
		return false;

	return true;
}



C_RDFContent::C_RDFContent(C_Bundler *pBundler)
{
	m_pBundler = pBundler;
}

C_RDFContent::~C_RDFContent()
{
	Ut::UtList<I_ProcessTexture*>::Iterator iter = m_elementsToProcess.Begin();

	while (iter != m_elementsToProcess.End())
	{
		delete *iter;
		++iter;
	}
}

inline bool C_RDFContent::StartDocument()
{
	return true;
}

inline bool C_RDFContent::EndDocument()
{
	return true;
}

inline bool C_RDFContent::StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes)
{
	Util::C_String n = name.Lower();

	I_ProcessTexture* pElem = 0;

	if (n == "texture")
		pElem = new C_ProcessTexture2D;
	else if (n == "arraytexture")
		pElem = new C_ProcessTextureArray2D;
	else if (n == "volume")
		pElem = new C_ProcessTexture3D;
	else if (n == "cubemap")
		pElem = new C_ProcessTextureCubeMap;

	if (pElem)
	{
		if (!pElem->Read(*m_pBundler, attributes))
		{
			delete pElem;
			return false;
		}

		m_elementsToProcess.PushBack(pElem);
	}

	return true;
}

inline bool C_RDFContent::EndElement(const Util::C_String& name)
{
	return true;
}

bool C_RDFContent::Parse()
{
	Ut::UtList<I_ProcessTexture*>::Iterator iter = m_elementsToProcess.Begin();
	while (iter != m_elementsToProcess.End())
	{
		if (!(*iter)->Process(*m_pBundler))
			return false;

		++iter;
	}

	return true;
}
