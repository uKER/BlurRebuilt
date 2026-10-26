
#include "stdafx.h"

#include <converter_shared/helpers/XmlHelpers.h>

namespace XmlHelpers
{

bool ShouldPrintWarning( const E_Behaviour behaviour)
{
	switch( behaviour )
	{
		case E_Behaviour_Fail:
			return true;
		case E_Behaviour_Warn:
			return true;
		case E_Behaviour_Silent:
			return false;
	}
	return false;
}

bool AllowFail( const E_Behaviour behaviour)
{
	switch( behaviour )
	{
		case E_Behaviour_Fail:
			return true;
		case E_Behaviour_Warn:
			return false;
		case E_Behaviour_Silent:
			return false;
	}
	return false;
}

bool GetXmlInteger( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, S32& value, const E_Behaviour behaviour )
{
	const Sys::C_XmlDomBaseNode* pNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(dom.GetRootNodePtr(), attributeName);
	const Sys::C_XmlAttribute* pXmlAttr = 
		pNode ? ((Sys::C_XmlDomElement*)(pNode))->GetAttributes().GetAttributeByName("value") : 0;

	if (!pXmlAttr)
	{
		if(ShouldPrintWarning( behaviour ))
		{
			LOGWARN(Tools::g_log, "Failed to find XML integer parameter '%s' in file '%s'", attributeName.Chars(), xmlFilePath.Chars() );
		}
		return !AllowFail(behaviour);
	}

	value = atoi(pXmlAttr->value);
	return true;
}


bool GetXmlFloat( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, F32& value, const E_Behaviour behaviour )
{
	const Sys::C_XmlDomBaseNode* pNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(dom.GetRootNodePtr(), attributeName);
	const Sys::C_XmlAttribute* pXmlAttr = 
		pNode ? ((Sys::C_XmlDomElement*)(pNode))->GetAttributes().GetAttributeByName("value") : 0;

	if (!pXmlAttr)
	{
		if(ShouldPrintWarning( behaviour ))
		{
			LOGWARN(Tools::g_log, "Failed to find XML float parameter '%s' in file '%s'", attributeName.Chars(), xmlFilePath.Chars() );
		}
		return !AllowFail(behaviour);
	}

	value = (F32)atof(pXmlAttr->value);
	return true;
}

bool GetXmlString( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, Util::C_String& value, const E_Behaviour behaviour )
{
	const Sys::C_XmlDomBaseNode* pNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(dom.GetRootNodePtr(), attributeName);
	const Sys::C_XmlAttribute* pXmlAttr = 
		pNode ? ((Sys::C_XmlDomElement*)(pNode))->GetAttributes().GetAttributeByName("value") : 0;

	if (!pXmlAttr)
	{
		if( ShouldPrintWarning( behaviour ) )
		{
			LOGWARN( Tools::g_log, "Failed to find XML string parameter '%s' in file '%s'", attributeName.Chars(), xmlFilePath.Chars() );
		}
		return !AllowFail(behaviour);
	}

	value = pXmlAttr->value;
	return true;
}

bool GetXmlBool( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, bool& value, const E_Behaviour behaviour )
{
	//bool success = GetXmlString(xmlFilePath, dom, attributeName, stringValue);
	const Sys::C_XmlDomBaseNode* pNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(dom.GetRootNodePtr(), attributeName);
	const Sys::C_XmlAttribute* pXmlAttr = 
		pNode ? ((Sys::C_XmlDomElement*)(pNode))->GetAttributes().GetAttributeByName("value") : 0;

	if (!pXmlAttr)
	{
		if( ShouldPrintWarning( behaviour ) )
		{
			LOGWARN( Tools::g_log, "Failed to find XML bool parameter '%s' in file '%s'", attributeName.Chars(), xmlFilePath.Chars() );
		}
		return !AllowFail(behaviour);
	}

	Util::C_String stringValue = pXmlAttr->value;

	value = false;

	if( _stricmp( stringValue.Chars(), "true" ) == 0)
		value = true;

	if( _stricmp( stringValue.Chars(), "false" ) == 0)
		value = false;

	return true;
}

bool GetXmlColour( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom, const Util::C_String& attributeName, Maths::C_RGBA_F32& value, const E_Behaviour behaviour)
{
	const Sys::C_XmlDomBaseNode* pNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(dom.GetRootNodePtr(), attributeName);
	const Sys::C_XmlAttribute* pXmlAttr = 
		pNode ? ((Sys::C_XmlDomElement*)(pNode))->GetAttributes().GetAttributeByName("value") : 0;

	if (!pXmlAttr)
	{
		if( ShouldPrintWarning( behaviour ) )
		{
			LOGWARN( Tools::g_log, "Failed to find XML string parameter '%s' in file '%s'", attributeName.Chars(), xmlFilePath.Chars() );
		}
		return !AllowFail(behaviour);
	}

	F32 a,r,g,b;
	
	sscanf( pXmlAttr->value.Chars(), "%f %f %f %f", &a, &r, &g, &b);

	value = Maths::C_RGBA_F32(r,g,b,a);

	return true;
}

} // namespace AftkHelpers