#ifndef CONVERTER_SHARED_XML_HELPERS_H
#define CONVERTER_SHARED_XML_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif



namespace XmlHelpers
{

enum E_Behaviour
{
	E_Behaviour_Fail = 0,
	E_Behaviour_Warn,
	E_Behaviour_Silent
};

bool GetXmlInteger( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, S32& value, const E_Behaviour behaviour = E_Behaviour_Fail );
bool GetXmlFloat( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, F32& value, const E_Behaviour behaviour = E_Behaviour_Fail );
bool GetXmlBool( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, bool& value, const E_Behaviour behaviour = E_Behaviour_Fail );
bool GetXmlString( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, Util::C_String& value, const E_Behaviour behaviour = E_Behaviour_Fail );
bool GetXmlColour( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, Maths::C_RGBA_F32& value, const E_Behaviour behaviour = E_Behaviour_Fail );

} // namespace XmlHelpers

#endif
