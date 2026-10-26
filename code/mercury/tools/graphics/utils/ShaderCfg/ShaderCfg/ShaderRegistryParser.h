#ifndef SHADER_REGISTRY_PARSER_H
#define SHADER_REGISTRY_PARSER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Containers.h>
#include <System/Xml/XmlBlockParser.h>
#include <system/Serialise.h>
#include "ShaderConfig.h"

namespace ShaderCfg
{

// C_ShaderRegistryXmlParser

class C_ShaderRegistryXmlParser : public Sys::C_XmlContent
{	
public:
	C_ShaderRegistryXmlParser( C_ShaderRegistry* pShaderRegistry );
	~C_ShaderRegistryXmlParser();

	bool ReadXml( Util::C_String fileName );

public:

	// inherited from C_XmlContent
	virtual bool StartDocument();
	virtual bool EndDocument();
	virtual bool StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes);
	virtual bool EndElement(const Util::C_String& name);

	const bool ProcessShaderEnum(const Sys::C_XmlAttributeList& attributes);	
	const bool ProcessShaderEnumItem(const Sys::C_XmlAttributeList& attributes);	
	const bool ProcessIndexList(const Sys::C_XmlAttributeList& attributes);
	const bool ProcessCombination(const Sys::C_XmlAttributeList& attributes);
	const bool ProcessCombinationEnum(const Sys::C_XmlAttributeList& attributes);
	

public:
	C_ShaderRegistry*	m_pShaderRegistry;
	Ut::Vector<S32>		m_ParseStateStack;
};

} // namespace ShaderCfg

#endif // SHADER_REGISTRY_PARSER_H
