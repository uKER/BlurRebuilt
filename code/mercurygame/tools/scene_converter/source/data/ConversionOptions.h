#ifndef APP_CONVERSION_OPTIONS_H
#define APP_CONVERSION_OPTIONS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/misc/Platform.h>
#include <converter_shared/ProcessGraph.h>
#include <converter_shared/config/PolyflagConfig.h>
#include <converter_shared/config/ObjectflagConfig.h>

#include <data/ToolConfig.h>

enum E_LightmapResolution
{
	E_LightmapResolution_Low = 0,
	E_LightmapResolution_Medium,
	E_LightmapResolution_High,
	E_LightmapResolution_VeryHigh,

	E_LightmapResolution_Count
};

enum E_LightmapQuality
{
	E_LightmapQuality_Low = 0,
	E_LightmapQuality_Medium,
	E_LightmapQuality_High,
	E_LightmapQuality_VeryHigh,
};

class C_LightingOptions
{
friend class C_DataConversionOptions;

public:
	const bool GetLightScene() const;

	const bool GetUseLightingScreensavers() const;

	const E_LightmapResolution GetLightmapResolution() const;

	const E_LightmapQuality GetLightmapQuality() const;

	/// the base value for lightmap resolutions
	const F32 GetLightmapDensity() const;

	const F32 GetLightSampleGridSpacing() const;

	const F32 GetLightSampleHeightOffset() const;

	const bool GetAddSunLight() const;
	const F32 GetSunElevation() const;
	const F32 GetSunHeading() const;
	const Maths::C_RGBA_F32 GetSunColour() const;
	const F32 GetSunPenumbra() const;

	const bool GetAddSkyLight() const;
	const F32 GetSkyIntensity() const;
	const S32 GetBouncePasses() const;
	const F32 GetBounceScale() const;
	const Maths::C_RGBA_F32 GetSkyColour() const;

	void Dump();

private:
	bool m_LightScene;
	bool m_UseLightingScreensavers;
	E_LightmapResolution m_LightmapResolution;
	E_LightmapQuality m_LightmapQuality;

	F32 m_LightmapDensity;

	F32 m_LightSampleGridSpacing; // default taken from toolconfig, overridden if executer option present
	F32 m_LightSampleHeightOffset; // default taken from toolconfig, overridden if executer option present

	bool m_AddSunLight;
	F32 m_SunElevation;
	F32 m_SunHeading;
	Maths::C_RGBA_F32 m_SunColour;
	F32 m_SunPenumbra;

	bool m_AddSkyLight;
	F32 m_SkyIntensity;
	S32 m_BouncePasses;
	F32 m_BounceScale;
	Maths::C_RGBA_F32 m_SkyColour;
};

class C_VisibilityOptions
{
	friend class C_DataConversionOptions;

public:
	const bool GetVisibilityScene() const;

	const bool GetUseLightingScreensavers() const;
	void Dump();

private:
	bool m_VisibilityScene;
	bool m_UseLightingScreensavers;
};

class C_DataConversionOptions : public Tools::ProcessGraph::C_Data, Tools::Shim::C_Options
{
public:
	bool SetupFromCommandLine( S32 argc, char** argv );

	// inherited from C_Data
	const Util::C_String GetName() const;

	const Util::C_String& GetExecutableFileNameWin32() const;

	const Util::C_String& GetExecuterXmlFilePath() const;

	const Util::C_String& GetSourceAftkFilename() const;

	const Util::C_String GetOutputFilename( const Util::E_Platform platform, const Util::C_String& filename ) const;

	const Util::C_String& GetPreprocessedAftkFilename() const;

	const Util::C_String& GetLightingTempDirectory() const;

	const Util::C_String& GetImposterTempDirectory() const;

	const Util::C_String& GetImposterSetupFilename() const;

	const Util::C_String& GetTreeTempDirectory() const;

	const Util::C_String& GetTreeSetupFilename() const;

	const Util::C_String& GetTreeLightingModelFilename() const;

	const Util::C_String& GetLitAftkFilename() const;

	const Util::C_String& GetLightSamplePointsFilename() const;

	const Util::C_String& GetLightSamplesFilename() const;

	const Util::C_String& GetLightingModelFilename() const;

	const Util::C_String GetToolDataPath() const;

	const Util::C_String GetToolConfigXmlFilename() const;

	const C_LightingOptions& GetLightingOptions() const;

	const C_LightingModelOptions& GetLightingModelOptions() const;

	const Util::C_String& GetTreeInputFilename() const;

	const Util::C_String& GetTreeOutputFilename() const;

	const C_VisibilityOptions& GetVisibilityOptions() const;

	const Util::C_String& GetVisTempDirectory() const;

	const Util::C_String& GetVisInputFilename() const;

	const Util::C_String& GetVisOutputFilename() const;

	const Util::C_String& GetPVSDataFilename() const;

	const S32 GetPVSClusterThreshold() const;

	const S32 GetPVSPolyThreshold() const;

	const Util::C_String& GetCrowdCropSheetDirectory() const;

	const Util::C_String& GetDefaultCubemapFilename() const;

	const bool ReuseLastPreprocessedLevel() const;

	const bool ReuseLastLitLevel() const;

	const bool ReuseLastVisibilityLevel() const;

	const bool ExportRenderData() const;

	const bool ExportPhysicsData() const;

	const bool ForceFullExport() const;

	const bool ExportStreamFile() const;

	const bool ExportCrowdData() const;

	const bool IgnoreRoutes() const;
	
	const bool ForceAllSolid() const;

	const bool RenderCollideMesh() const;

	const bool ForceExtraCollideMeshFlags() const;

	const F32 GetPvsZoneSize() const;

	const F32 GetPvsZoneHeight() const;

	const F32 GetPvsSampleSpacing() const;

	const bool ExportForPlatform( const Util::E_Platform platform ) const;

	const C_ToolConfig& GetToolConfig() const;

	C_ToolConfig& GetToolConfig();

	const Polyflags::C_Config& GetPolyflagConfig() const;

	Polyflags::C_Config& GetPolyflagConfig();

	const Objectflags::C_Config& GetObjectflagConfig() const;

	Objectflags::C_Config& GetObjectflagConfig();

	const U32 GetCrowdRandomSeed() const { return m_CrowdRandomSeed; }

	void Dump();

private:
	bool ParseCommandLine( Util::C_String& xmlFilePath, int argc, char* argv[]);
	bool ReadXml(const Util::C_String& xmlFilePath);
	bool ParseXml(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom);
	bool BuildSourceAftkFilename(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom);
	bool BuildDerivedOptions();

private:
	Util::C_String			m_ExecutableFileNameWin32;

	Util::C_String			m_ExecuterXmlFilePath; 

	//Generated a3d file lives within temp folder.
	Util::C_String			m_SourceAftkFilename;	

	Util::C_String			m_CacheDirectory;
	Util::C_String			m_PreprocessedAftkFilename;
	Util::C_String			m_LightingTempDirectory;
	Util::C_String			m_LightingModelFilename;
	Util::C_String			m_LitAftkFilename;
	Util::C_String			m_LightSamplePointsFilename;
	Util::C_String			m_LightSamplesFilename;

	Util::C_String			m_ImposterTempDirectory;
	Util::C_String			m_ImposterSetupFilename;

	Util::C_String			m_TreeTempDirectory;
	Util::C_String			m_TreeLightingModelFilename;
	Util::C_String			m_TreeSetupFilename;
	Util::C_String			m_TreeInputFilename;
	Util::C_String			m_TreeOutputFilename;

	Util::C_String			m_VisTempDirectory;
	Util::C_String			m_VisInputFilename;
	Util::C_String			m_VisOutputFilename;
	Util::C_String			m_PVSDataFilename;

	Util::C_String			m_CrowdCropSheetDirectory;

	Util::C_String			m_DefaultCubemapFilename;

	C_LightingOptions		m_LightingOptions;

	Util::C_String			m_LightingModelType;
	C_LightingModelOptions	m_LightingModelOptions;

	C_VisibilityOptions		m_VisibilityOptions;

	bool					m_ReuseLastPreprocesedLevel;
	bool					m_ReuseLastLitLevel;
	bool					m_ReuseLastVisibilityLevel;
	bool					m_ExportRenderData;
	bool					m_ExportPhysicsData;
	bool					m_ForceFullExport;
	bool					m_ExportStreamFile;
	bool					m_ForceAllSolid;
	bool					m_RenderCollideMesh;
	bool					m_ForceExtraCollideMeshFlags;

	F32						m_PvsZoneSize; // default taken from toolconfig, overridden if executer option present
	F32						m_PvsZoneHeight; // default taken from toolconfig, overridden if executer option present
	F32						m_PvsSampleSpacing; // default taken from toolconfig, overridden if executer option present

	S32						m_PvsClusterThreshold;
	S32						m_PvsPolyThreshold;

	bool					m_ExportCrowdData;

	C_ToolConfig			m_ToolConfig;	
	Polyflags::C_Config		m_PolyflagConfig;
	Objectflags::C_Config	m_ObjectflagConfig;

	U32						m_CrowdRandomSeed;

	bool					m_ExportForPlatform[Util::E_Platform_Count];
};

#endif
