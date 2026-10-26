#ifndef APP_TOOL_CONFIG_H
#define APP_TOOL_CONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>

//! These are per-project options for configuring the tool. These are read from an xml file in the tool's config directory.
class C_ToolConfig
{
friend class C_DataConversionOptions;
public:
	const Util::C_String& GetPolyflagConfigFilename() const;

	const Util::C_String& GetObjectflagConfigFilename() const;

	const Util::C_String& GetLuxBaseConfigFilename() const;

	const Util::C_String& GetLuxFilenameWin32() const;

	const Util::C_String& GetLuxFilenameYdl() const;

	const Util::C_String& GetLuxFilenameServer() const;

	const Util::C_String& GetRouteLayerNameBase() const;

	const Util::C_String& GetTextureCachePath() const;
	
	const Util::C_String& GetFxbPath() const;

	void Dump();

private:
	bool ReadXml(const Util::C_String& xmlFilePath);
	bool ParseXml( const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom );
	bool ParseXml( const Sys::C_XmlDomBaseNode& root );
	bool ParseXmlItem( const Sys::C_XmlDomElement& element );

private:

	Util::C_String m_PolyflagConfigFilename;

	Util::C_String m_ObjectflagConfigFilename;

	Util::C_String m_LuxBaseConfigFilename;

	Util::C_String m_LuxFilenameWin32;

	Util::C_String m_LuxFilenameYdl;

	Util::C_String m_LuxFilenameServer;

	Util::C_String m_RouteLayerNameBase;

	Util::C_String m_TextureCachePath;

	Util::C_String m_FxbPath;
};

#endif
