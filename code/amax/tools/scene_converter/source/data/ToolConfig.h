#ifndef APP_TOOL_CONFIG_H
#define APP_TOOL_CONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>


//C_LightingModelOptions
class C_LightingModelOptions
{
public:
	C_LightingModelOptions();
	~C_LightingModelOptions();
	
	bool ParseXml( const Sys::C_XmlDomElement& element );
	void Dump() const;

	const Util::C_String& GetName() const;
	const Util::C_String& GetLuxSpecificConfigFilename() const;
	const Util::C_String& GetEffectVariation() const;

public:
	Util::C_String m_Name;
	Util::C_String m_LuxSpecificConfigFilename;
	Util::C_String m_EffectVariation;
};

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

	const S32 GetLightingModelOptionsCount() const;

	const C_LightingModelOptions& GetLightingModelOptions( const S32 index ) const;

	const C_LightingModelOptions* LookupLightingModelOptions( const Util::C_String& name ) const;

	void Dump() const;

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

	Btl::Vector<C_LightingModelOptions> m_LightingModelOptions;
};

#endif
