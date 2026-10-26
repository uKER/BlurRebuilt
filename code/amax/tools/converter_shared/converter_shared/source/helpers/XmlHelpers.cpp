
#include "stdafx.h"

#include <converter_shared/helpers/XmlHelpers.h>

namespace XmlHelpers
{

bool GetXmlInteger( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, S32& value )
{
	const Sys::C_XmlDomBaseNode* pInteger = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(dom.GetRootNodePtr(), attributeName);

	if (!pInteger) 
	{
		LOGWARN(Tools::g_log, "Failed to find XML integer parameter '%s' in file '%s'", attributeName.Chars(), xmlFilePath.Chars() );
		return false;
	}

	value = atoi(((Sys::C_XmlDomElement*)(pInteger))->GetAttributes().GetAttributeByName("value")->value.Chars());

	return true;
}


bool GetXmlFloat( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, F32& value )
{
	const Sys::C_XmlDomBaseNode* pInteger = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(dom.GetRootNodePtr(), attributeName);

	if (!pInteger) 
	{
		LOGWARN(Tools::g_log, "Failed to find XML float parameter '%s' in file '%s'", attributeName.Chars(), xmlFilePath.Chars() );
		return false;
	}

	value = (F32)atof(((Sys::C_XmlDomElement*)(pInteger))->GetAttributes().GetAttributeByName("value")->value.Chars());

	return true;
}

bool GetXmlString( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, Util::C_String& value )
{
	const Sys::C_XmlDomBaseNode* pInteger = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(dom.GetRootNodePtr(), attributeName);

	if (!pInteger) 
	{
		LOGWARN( Tools::g_log, "Failed to find XML string parameter '%s' in file '%s'", attributeName.Chars(), xmlFilePath.Chars() );
		return false;
	}

	value = ((Sys::C_XmlDomElement*)(pInteger))->GetAttributes().GetAttributeByName("value")->value;

	return true;
}

bool GetXmlBool( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, bool& value )
{
	Util::C_String stringValue;

	value = false;

	bool success = GetXmlString(xmlFilePath, dom, attributeName, stringValue);
	if(!success)
	{
		LOGWARN( Tools::g_log, "Failed to find XML bool parameter '%s' in file '%s'", attributeName.Chars(), xmlFilePath.Chars() );
		return false;
	}

	if( _stricmp( stringValue.Chars(), "true" ) == 0)
		value = true;

	if( _stricmp( stringValue.Chars(), "false" ) == 0)
		value = false;

	return true;
}


} // namespace AftkHelpers