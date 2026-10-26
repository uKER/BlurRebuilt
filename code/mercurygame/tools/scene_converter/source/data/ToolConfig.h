#ifndef APP_TOOL_CONFIG_H
#define APP_TOOL_CONFIG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>

enum E_LightmapResolution;
namespace LuxInterface { enum E_PolyflagLightmapDensity; }
	
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
	const Util::C_String& GetLightPointSampleDataFormat() const;

public:
	Util::C_String m_Name;
	Util::C_String m_LuxSpecificConfigFilename;
	Util::C_String m_EffectVariation;
	Util::C_String m_LightPointSampleDataFormat;
};

//node flag overrides for the speedtree mesh generator
struct C_SpeedTreeFlags
{
	Util::C_String name;
	Util::C_String value;
};

struct C_CrowdCropSheetOptions
{
	C_CrowdCropSheetOptions();

	bool ParseXml( const Sys::C_XmlDomElement& element );
	bool IsValid() const;
	void Dump() const;

	U32 width, height, border, minImageSize, maxImageSize;
};

typedef Btl::Vector<C_SpeedTreeFlags> SpeedTreeFlagArray;



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

	const Util::C_String& GetImposterFilenameWin32() const;

	const Util::C_String& GetSpeedTreeGenFilenameWin32() const;

	const Util::C_String& GetLuxFilenameWin32() const;

	const Util::C_String& GetLuxFilenameYdl() const;

	const Util::C_String& GetLuxFilenameServer() const;

	const Util::C_String& GetVisFilenameWin32() const;

	const Util::C_String& GetRouteLayerNameBase() const;

	const Util::C_String& GetTextureCachePath() const;

	const Util::C_String& GetFxbPath() const;

	const Util::C_String& GetMissingTextureFilename() const;

	const Util::C_String& GetLocalisedCubemapPath() const;

	const Util::C_String& GetDefaultCubemapFilename() const;

	const F32 GetLightSampleGridSpacing() const;

	const F32 GetLightSampleYSpacing() const;

	const F32 GetLightSampleHeightOffset() const;

	const S32 GetLightSampleMaxCount() const;

	const S32 GetLightingModelOptionsCount() const;

	// the base lightmap density used if none is supplied from executer
	const F32 GetLightmapDensity() const;

	/// The effect the lightmap density polyflag options have on the lightmap density
	const F32 GetLightmapDensityMultiplier( const LuxInterface::E_PolyflagLightmapDensity index ) const;

	/// The effect the lightmap resolution executer dropbox has on the lightmap density
	const F32 GetLightmapResolutionMultiplier( const E_LightmapResolution index ) const;

	const bool GetUseVertexNormalForTangents() const;

	const F32 GetSmoothNormalAngularTolerance() const;

	const F32 GetSmoothTangentAngularTolerance() const;

	const S32 GetMaxConvexHullTriangles() const;

	// the base pvs density used if none is supplied from executer
	const F32 GetPvsZoneSize() const;

	// the base pvs height density used if none is supplied from executer
	const F32 GetPvsZoneHeight() const;

	// the base pvs sample spacing used if none is supplied from executer
	const F32 GetPvsSampleSpacing() const;

	const U32 GetFluidCollisionFlag() const;

	const F32 GetSunPenumbra() const;

	const U32 GetForceExtraCollideMeshFlagsValue() const;

	const C_LightingModelOptions& GetLightingModelOptions( const S32 index ) const;

	const C_LightingModelOptions* LookupLightingModelOptions( const Util::C_String& name ) const;

	const SpeedTreeFlagArray& GetSpeedTreeFlags() const;

	const C_CrowdCropSheetOptions& GetCrowdCropSheetOptions() const;

	const bool CrowdRotateY180() const;

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

	Util::C_String m_VisFilenameWin32;

	Util::C_String m_ImposterFilenameWin32;

	Util::C_String m_SpeedTreeGenFilenameWin32;

	Util::C_String m_RouteLayerNameBase;

	Util::C_String m_TextureCachePath;

	F32 m_LightSampleGridSpacing;

	F32 m_LightSampleYSpacing;

	F32 m_LightSampleHeightOffset;

	F32 m_LightmapDensity;

	S32 m_LightSampleMaxCount;

	bool m_UseVertexNormalForTangents;

	F32 m_SmoothNormalAngularTolerance;

	F32 m_SmoothTangentAngularTolerance;

	S32 m_MaxConvexHullTriangles;

	F32 m_PvsZoneSize;

	F32 m_PvsZoneHeight;

	F32 m_PvsSampleSpacing;

	U32 m_FluidCollisionFlag;

	U32 m_ForceExtraCollideMeshFlagsValue;

	F32 m_SunPenumbra;

	Util::C_String m_FxbPath;

	Util::C_String m_MissingTextureFilename;

	Util::C_String m_LocalisedCubemapPath;

	Util::C_String m_DefaultCubemapFilename;

	Btl::Vector<C_LightingModelOptions> m_LightingModelOptions;

	Btl::Vector<F32> m_LightmapDensityMultipliers;

	Btl::Vector<F32> m_LightmapResolutionMultipliers;

	SpeedTreeFlagArray m_SpeedTreeFlags;

	C_CrowdCropSheetOptions m_CrowdCropSheetOptions;

	bool m_CrowdRotateY180;
};

#endif
