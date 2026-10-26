#pragma once

#include "system\serialise.h"
#include "core\containers.h"

#include "system\xml\xmlparser.h"
#include "core\containers\string.h"

namespace ResourceBundler
{
	class C_Bundler;

	class C_RDFParser
	{
	public:
		C_RDFParser();

		bool LoadRDF(C_Bundler *pBundler);

	protected:
	};

	class I_ProcessTexture;

	class C_RDFContent : public Sys::C_XmlContent
	{
	public:
		C_RDFContent(C_Bundler *pBundler);
		virtual ~C_RDFContent();

		virtual bool StartDocument();

		virtual bool EndDocument();

		virtual bool StartElement(
			const Util::C_String& name, 
			const Sys::C_XmlAttributeList& attributes);

		virtual bool EndElement(const Util::C_String& name);

		bool Parse();

	protected:
		C_Bundler* m_pBundler;
		Ut::UtList<I_ProcessTexture*>	m_elementsToProcess;
	};
}
