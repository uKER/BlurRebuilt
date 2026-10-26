#ifndef XML_BLOCK_PARSER_H
#define XML_BLOCK_PARSER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/xml/XmlParser.h>

// extension to XmlParser to provide an alternate interface using C_XmlBlock class

namespace XmlBlockParser
{

// C_XmlBlock

// inherit from C_XmlBlock
class C_XmlBlock
{
public:
	virtual ~C_XmlBlock() {}
	virtual C_XmlBlock*	    NewChildBlock( const Util::C_String& name, const Sys::C_XmlAttributeList& attributes ); 
	virtual void		    EndChildBlock(); 
};

// C_XmlParser

// construct C_XmlParser with root block and call XmlParse
class C_XmlParser : public Sys::C_XmlContent
{
public:
	C_XmlParser( C_XmlBlock* pRootBlock );

	virtual bool StartDocument();
	virtual bool EndDocument();

public:
	virtual bool StartElement(const Util::C_String& name, const Sys::C_XmlAttributeList& attributes);
	virtual bool EndElement(const Util::C_String& name);
	
private:	
	C_XmlBlock* m_pRootBlock;
	Ut::Vector<C_XmlBlock*>	m_pBlockStack;
};

} // namespace XmlBlockParser

#endif
