#ifndef CONVERTER_SHARED_XML_HELPERS_H
#define CONVERTER_SHARED_XML_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif



namespace XmlHelpers
{

bool GetXmlInteger( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, S32& value );
bool GetXmlFloat( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, F32& value );
bool GetXmlBool( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, bool& value );
bool GetXmlString( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, Util::C_String& value );

} // namespace XmlHelpers

#endif
