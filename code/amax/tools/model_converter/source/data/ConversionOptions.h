#ifndef APP_CONVERSION_OPTIONS_H
#define APP_CONVERSION_OPTIONS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/config/PolyflagConfig.h>
#include <converter_shared/config/ObjectflagConfig.h>

#include <data/ToolConfig.h>


class C_LightingOptions
{
friend class C_DataConversionOptions;

public:
	const bool GetLightScene() const;

	const bool GetUseLightingScreensavers() const;

private:
	bool m_LightModel;
	bool m_UseLightingScreensavers;
};

class C_DataConversionOptions : public Tools::ProcessGraph::C_Data, Tools::Shim::C_Options
{
public:
	bool SetupFromCommandLine( S32 argc, char** argv );

	// inherited from C_Data
	const Util::C_String GetName() const;

	const Util::C_String& GetExecuterXmlFilePath() const;

	const Util::C_String& GetSourceAftkFilename() const;

	const Util::C_String GetOutputFilename( const Tools::E_Platform platform ) const;

	const Util::C_String& GetPreprocessedAftkFilename() const;

	const Util::C_String& GetLightingTempDirectory() const;

	const Util::C_String& GetLitAftkFilename() const;

	const Util::C_String& GetLightingModelFilename() const;

	const Util::C_String GetToolDataPath() const;

	const Util::C_String GetToolConfigXmlFilename() const;

	const C_LightingOptions& GetLightingOptions() const;

	const bool ReuseLastPreprocessedLevel() const;

	const bool ReuseLastLitLevel() const;

	const bool ExportRenderData() const;

	const bool ExportPhysicsData() const;

	const C_ToolConfig& GetToolConfig() const;

	const F32 SceneScale() const;

	const bool RotateY180() const;

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
	Util::C_String			m_ExecuterXmlFilePath; 

	//Generated a3d file lives within temp folder.
	Util::C_String			m_SourceAftkFilename;	

	Util::C_String			m_CacheDirectory;
	Util::C_String			m_PreprocessedAftkFilename;
	Util::C_String			m_LightingTempDirectory;
	Util::C_String			m_LightingModelFilename;
	Util::C_String			m_LitAftkFilename;

	C_LightingOptions		m_LightingOptions;

	F32						m_SceneScale;
	bool					m_RotateY180;

	bool					m_ReuseLastPreprocesedLevel;
	bool					m_ReuseLastLitLevel;
	bool					m_ExportRenderData;
	bool					m_ExportPhysicsData;

	C_ToolConfig			m_ToolConfig;	
	Polyflags::C_Config		m_PolyflagConfig;
	Objectflags::C_Config	m_ObjectflagConfig;
};

#endif
