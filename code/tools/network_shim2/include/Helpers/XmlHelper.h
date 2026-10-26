#ifndef SHIM_XML_HELPER_H
#define SHIM_XML_HELPER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/xml/XmlDom.h>

class XmlHelper
{
public:
	static bool ParseDependsOn(const Sys::C_XmlDomBaseNode* node);
	static const Sys::C_XmlDomBaseNode* FindXmlNodeByShimTag(const Sys::C_XmlDomBaseNode* root, const Util::C_String& tag);
};

#endif