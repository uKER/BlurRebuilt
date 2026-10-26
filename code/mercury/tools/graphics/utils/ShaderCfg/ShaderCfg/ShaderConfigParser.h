#ifndef SHADER_CONFIG_PARSER_H
#define SHADER_CONFIG_PARSER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Containers.h>
#include <System/Xml/XmlBlockParser.h>
#include <system/Serialise.h>
#include "ShaderConfig.h"

namespace ShaderCfg
{

// C_ShaderConfigXmlParser

class C_ShaderConfigXmlParser : public Sys::C_XmlContent
{	
public:
	C_ShaderConfigXmlParser( C_ShaderConfig* pShaderConfig );
	~C_ShaderConfigXmlParser();

	bool ReadXml( Util::C_String fileName );

public:

	// inherited from C_XmlContent
	virtual bool StartDocument();
	virtual bool EndDocument();
	virtual bool StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes);
	virtual bool EndElement(const Util::C_String& name);

	const bool ProcessDefine(const Sys::C_XmlAttributeList& attributes);
	const bool ProcessFragment(const Sys::C_XmlAttributeList& attributes);

	const bool ProcessFragmentDefine(const Sys::C_XmlAttributeList& attributes);
	const bool ProcessFragmentCondition(const Sys::C_XmlAttributeList& attributes);

public:
	C_ShaderConfig*		m_pShaderConfig;
	Ut::Vector<S32>		m_ParseStateStack;
};



} // namespace ShaderCfg

#endif // SHADER_CONFIG_PARSER_H
