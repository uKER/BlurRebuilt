#ifndef APP_TOOL_CONFIG_H
#define APP_TOOL_CONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>

//C_ModelOptions
class C_ModelOptions
{
public:
	C_ModelOptions();
	~C_ModelOptions();
	
	bool ParseXml( const Sys::C_XmlDomElement& element );
	void Dump() const;

	const Util::C_String& GetName() const;
	const Util::C_String& GetDefaultEffect() const;
	const Util::C_String& GetLodConfigFilename() const;
	const Util::C_String& GetPVSConfigFilename() const;
	const bool GetUseSharedTextures() const;
public:
	Util::C_String m_Name;
	Util::C_String m_DefaultEffect;
	Util::C_String m_LodConfigFilename;
	Util::C_String m_PVSConfigFilename;
	bool m_UseSharedTextures;
};

//! These are per-project options for configuring the tool. These are read from an xml file in the tool's config directory.
class C_ToolConfig
{
friend class C_DataConversionOptions;
public:

	C_ToolConfig();
	
	~C_ToolConfig();

	const Util::C_String& GetPolyflagConfigFilename() const;

	const Util::C_String& GetObjectflagConfigFilename() const;

	const Util::C_String& GetLuxBaseConfigFilename() const;

	const Util::C_String& GetLuxFilenameWin32() const;

	const Util::C_String& GetLuxFilenameYdl() const;

	const Util::C_String& GetLuxFilenameServer() const;

	const Util::C_String& GetRouteLayerNameBase() const;

	const Util::C_String& GetTextureCachePath() const;
	
	const Util::C_String& GetFxbPath() const;

	const Util::C_String& GetMissingTextureFilename() const;

	const Util::C_String& GetVisFilenameWin32() const;

	const bool GetUseVertexNormalForTangents() const;

	const S32 GetMaxConvexHullTriangles() const;

	const F32 GetSmoothNormalAngularTolerance() const;

	const F32 GetSmoothTangentAngularTolerance() const;

	const S32 GetModelOptionsCount() const;

	const C_ModelOptions& GetModelOptions( const S32 index ) const;

	const C_ModelOptions* LookupModelOptions( const Util::C_String& name ) const;

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

	Util::C_String m_VisFilenameWin32;

	Util::C_String m_LuxFilenameServer;

	Util::C_String m_RouteLayerNameBase;

	Util::C_String m_TextureCachePath;

	Util::C_String m_FxbPath;

	Util::C_String m_MissingTextureFilename;

	S32 m_MaxConvexHullTriangles;

	bool m_UseVertexNormalForTangents;

	F32 m_SmoothNormalAngularTolerance;

	F32 m_SmoothTangentAngularTolerance;

	Btl::Vector<C_ModelOptions> m_ModelOptions;
};

#endif
