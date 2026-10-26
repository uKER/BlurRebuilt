#ifndef APP_CONVERSION_OPTIONS_H
#define APP_CONVERSION_OPTIONS_H

#ifdef _MSC_VER
#pragma once
#endif

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

	const bool GetAddSunLight() const;
	const F32 GetSunElevation() const;
	const F32 GetSunHeading() const;

	const bool GetAddSkyLight() const;
	const F32 GetSkyIntensity() const;
	const S32 GetBouncePasses() const;
	const F32 GetBounceScale() const;

private:
	bool m_LightScene;
	bool m_UseLightingScreensavers;
	E_LightmapResolution m_LightmapResolution;
	E_LightmapQuality m_LightmapQuality;
	bool m_AddSunLight;
	F32 m_SunElevation;
	F32 m_SunHeading;

	bool m_AddSkyLight;
	F32 m_SkyIntensity;
	S32 m_BouncePasses;
	F32 m_BounceScale;
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

	const Util::C_String GetOutputFilename( const Tools::E_Platform platform, const Util::C_String& filename ) const;

	const Util::C_String& GetPreprocessedAftkFilename() const;

	const Util::C_String& GetLightingTempDirectory() const;

	const Util::C_String& GetLitAftkFilename() const;

	const Util::C_String& GetLightSamplePointsFilename() const;

	const Util::C_String& GetLightSamplesFilename() const;

	const Util::C_String& GetLightingModelFilename() const;

	const Util::C_String GetToolDataPath() const;

	const Util::C_String GetToolConfigXmlFilename() const;

	const C_LightingOptions& GetLightingOptions() const;

	const C_LightingModelOptions& GetLightingModelOptions() const;

	const bool ReuseLastPreprocessedLevel() const;

	const bool ReuseLastLitLevel() const;

	const bool ExportRenderData() const;

	const bool ExportPhysicsData() const;

	const bool ForceFullExport() const;

	const C_ToolConfig& GetToolConfig() const;

	C_ToolConfig& GetToolConfig();

	const Polyflags::C_Config& GetPolyflagConfig() const;

	Polyflags::C_Config& GetPolyflagConfig();

	const Objectflags::C_Config& GetObjectflagConfig() const;

	Objectflags::C_Config& GetObjectflagConfig();

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

	Util::C_String			m_LightingModelType;

	C_LightingOptions		m_LightingOptions;
	C_LightingModelOptions	m_LightingModelOptions;

	bool					m_ReuseLastPreprocesedLevel;
	bool					m_ReuseLastLitLevel;
	bool					m_ExportRenderData;
	bool					m_ExportPhysicsData;
	bool					m_ForceFullExport;

	C_ToolConfig			m_ToolConfig;	
	Polyflags::C_Config		m_PolyflagConfig;
	Objectflags::C_Config	m_ObjectflagConfig;
};

#endif
