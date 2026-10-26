#include "stdafx.h"

#include <tools_shared/file_copy/FileCopy.h>

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/XmlHelpers.h>

#include <Polyflags.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>

// C_LightingOptions

const bool C_LightingOptions::GetLightScene() const
{
	return m_LightScene;
}

const bool C_LightingOptions::GetUseLightingScreensavers() const
{
	return m_UseLightingScreensavers;
}

const E_LightmapResolution C_LightingOptions::GetLightmapResolution() const
{
	return m_LightmapResolution;
}

const E_LightmapQuality C_LightingOptions::GetLightmapQuality() const
{
	return m_LightmapQuality;
}

const bool C_LightingOptions::GetAddSunLight() const
{
	return m_AddSunLight;
}

const F32 C_LightingOptions::GetSunElevation() const
{
	return m_SunElevation;
}

const F32 C_LightingOptions::GetSunHeading() const
{
	return m_SunHeading;
}

const bool C_LightingOptions::GetAddSkyLight() const
{
	return m_AddSkyLight;
}

const F32 C_LightingOptions::GetSkyIntensity() const
{
	return m_SkyIntensity;
}

const S32 C_LightingOptions::GetBouncePasses() const
{
	return m_BouncePasses;
}

const F32 C_LightingOptions::GetBounceScale() const
{
	return m_BounceScale;
}

// C_DataConversionOptions

bool C_DataConversionOptions::SetupFromCommandLine( S32 argc, char** argv )
{
	bool success = true;

	//Parse our command-line options.
	success = ParseCommandLine(m_ExecuterXmlFilePath, argc, argv);
	if (!success) return false;	

	// read the executer options config file
	{
		success = ReadXml( m_ExecuterXmlFilePath );
		if (!success) return false;	
	}

	// read the tools options config file
	{
		Util::C_String toolConfigXmlFilePath = GetToolConfigXmlFilename();

		success = GetToolConfig().ReadXml(toolConfigXmlFilePath);
		if (!success) return false;	

		GetToolConfig().Dump();
	}	

	// read the polyflags config file
	{
		Util::C_String polyflagConfigFilename = GetToolConfig().GetPolyflagConfigFilename().Chars();

		Btl::Vector< Util::C_String > polyFlagItemNames;
		
		GetPolyflagItemNames( polyFlagItemNames );

		assert( polyFlagItemNames.Size() == E_Polyflag_Count );

		success = GetPolyflagConfig().RegisterItems( polyFlagItemNames );
		if (!success) return false;	

		success = GetPolyflagConfig().ReadXml( polyflagConfigFilename );
		if (!success) return false;	

		GetPolyflagConfig().Dump();
	}

	// read the objectflags config file
	{
		Util::C_String objectflagConfigFilename = GetToolConfig().GetObjectflagConfigFilename().Chars();

		Btl::Vector< Util::C_String > objectFlagItemNames;
		
		GetObjectflagItemNames( objectFlagItemNames );

		assert( objectFlagItemNames.Size() == E_Objectflag_Count );

		success = GetObjectflagConfig().RegisterItems( objectFlagItemNames );
		if (!success) return false;	

		success = GetObjectflagConfig().ReadXml( objectflagConfigFilename );
		if (!success) return false;	

		GetObjectflagConfig().Dump();
	}

	success = BuildDerivedOptions();
	if (!success) return false;	
	
	return true;
}

const Util::C_String C_DataConversionOptions::GetName() const
{
	return Util::C_String("C_DataConversionOptions");
}


bool C_DataConversionOptions::ParseCommandLine(
	Util::C_String& xmlFilePath,
	int argc, 
	char* argv[])
{
	//Get Executable FileName
	m_ExecutableFileNameWin32 = argv[0];

	// Wrap everything in a try block.  Do this every time, 
	// because exceptions will be thrown for problems. 
	try
	{  
		//Define the command line object.
		TCLAP::CmdLine cmd("Scene Converter.", ' ', "1.00");

		//Define a value argument and add it to the command line.
		TCLAP::ValueArg<std::string> xmlFileArg(
			"","xml","XML file",true,"","Executer XML file");
		cmd.add(xmlFileArg);
		TCLAP::ValueArg<std::string> aftkFileArg(
			"","scene","Scene file",false,"","Aftk scene file");
		cmd.add(aftkFileArg);

		//Parse the args.
		cmd.parse(argc, argv);

		//Get the values
		xmlFilePath = xmlFileArg.getValue().c_str();

		// if this is not found it is generated from the shim temp path and the executer output name and output directory
		m_SourceAftkFilename = aftkFileArg.getValue().c_str();
	} 
	catch (TCLAP::CmdLineParseException &e)
	{
		std::cerr << "Failed to parse arguments: " << e.argId() << std::endl;
		return false;
	}
	catch (TCLAP::ArgException &e)  // catch any exceptions
	{ 
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
		return false;
	}

	return true;
}

bool C_DataConversionOptions::ReadXml(const Util::C_String& xmlFilePath)
{
	bool success = true;

	LOGDEBUG(Tools::g_pMyApplication->GetLogger(), "Parsing: '%s'\n", xmlFilePath.Chars());
	Sys::C_XmlDom dom;

	success = dom.Read(xmlFilePath.Chars());
	if(!success)
	{
		LOGERROR(Tools::g_pMyApplication->GetLogger(), " Problem reading '%s'\n", xmlFilePath.Chars());
		return false;
	}

	success = ParseXml(xmlFilePath, dom);
	if(!success)
	{
		LOGERROR(Tools::g_pMyApplication->GetLogger(), " Problem parsing '%s'\n", xmlFilePath.Chars());
		return false;
	}

	return true;
}

E_LightmapResolution LookupLightmapResolutionString( const Util::C_String& lightmapResolutionString )
{
	E_LightmapResolution result = E_LightmapResolution_High;
	if( _stricmp( lightmapResolutionString.Chars(), "Low" ) == 0 )
	{
		result = E_LightmapResolution_Low;
	}
	else if( _stricmp( lightmapResolutionString.Chars(), "Medium" ) == 0 )
	{
		result = E_LightmapResolution_Medium;
	}
	else if( _stricmp( lightmapResolutionString.Chars(), "High (Final Quality)" ) == 0 )
	{
		result = E_LightmapResolution_High;
	}
	else if( _stricmp( lightmapResolutionString.Chars(), "Very High" ) == 0 )
	{
		result = E_LightmapResolution_VeryHigh;
	}
	return result;
}

E_LightmapQuality LookupLightmapQualityString( const Util::C_String& lightmapQualityString )
{
	E_LightmapQuality result = E_LightmapQuality_High;
	if( _stricmp( lightmapQualityString.Chars(), "Low" ) == 0 )
	{
		result = E_LightmapQuality_Low;
	}
	else if( _stricmp( lightmapQualityString.Chars(), "Medium" ) == 0 )
	{
		result = E_LightmapQuality_Medium;
	}
	else if( _stricmp( lightmapQualityString.Chars(), "High (Final Quality)" ) == 0 )
	{
		result = E_LightmapQuality_High;
	}
	else if( _stricmp( lightmapQualityString.Chars(), "Very High" ) == 0 )
	{
		result = E_LightmapQuality_VeryHigh;
	}

	return result;
}

bool C_DataConversionOptions::ParseXml(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom)
{
	if (!Tools::Shim::C_Options::ParseXml(xmlFilePath, dom))
		return false;

	bool success = true;

	BuildSourceAftkFilename(xmlFilePath, dom);

	//Our Aftk input MUST come from our intermediate directory.
	if (m_SourceAftkFilename.ISearch(m_shimRootTempPath) == -1)
	{
		LOGWARN(Tools::g_log, "Aftk input file is not in correct intermediate directory" );
		//Ah, it doesn't :(
		return false;
	}

	//Find interesting info provided in the XML file from Executer.

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "reuse_last_lit_level", m_ReuseLastLitLevel);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "export_render_data", m_ExportRenderData);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "export_physics_data", m_ExportPhysicsData);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "reuse_last_preproc_level", m_ReuseLastPreprocesedLevel);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "reuse_last_lit_level", m_ReuseLastLitLevel);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "force_full_export", m_ForceFullExport);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "light_scene", m_LightingOptions.m_LightScene);
	if(!success) return false;
	
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "use_network_screensavers", m_LightingOptions.m_UseLightingScreensavers);
	if(!success) return false;

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "lighting_model_type", m_LightingModelType);
	if(!success) return false;

	Util::C_String lightmapResolutionString;
	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "lightmap_resolution", lightmapResolutionString);
	if(!success) return false;

	m_LightingOptions.m_LightmapResolution = LookupLightmapResolutionString( lightmapResolutionString );

	Util::C_String lightmapQualityString;
	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "lightmap_quality", lightmapQualityString);
	if(!success) return false;

	m_LightingOptions.m_LightmapQuality = LookupLightmapQualityString( lightmapQualityString );

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "add_sun_light", m_LightingOptions.m_AddSunLight);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "sun_heading", m_LightingOptions.m_SunElevation);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "sun_elevation", m_LightingOptions.m_SunHeading);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "add_sky_light", m_LightingOptions.m_AddSkyLight);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "sky_intensity", m_LightingOptions.m_SkyIntensity);
	if(!success) return false;

	success = XmlHelpers::GetXmlInteger(xmlFilePath, dom, "bounce_passes", m_LightingOptions.m_BouncePasses);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "bounce_scale", m_LightingOptions.m_BounceScale);
	if(!success) return false;

	return true;
}

bool C_DataConversionOptions::BuildSourceAftkFilename(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom)
{
	bool success = true;
	if( m_SourceAftkFilename == "" )
	{
		LOGDEBUG(Tools::g_log, "Source Aftk Filename not specified, generating from Executer options" );

		Util::C_String shimMergedFilename;
		success = XmlHelpers::GetXmlString(xmlFilePath, dom, "shim_aftk_scene_collection_merged_filename", shimMergedFilename);
		if(!success) return false;

		Util::C_String shimMergedPath;
		success = XmlHelpers::GetXmlString(xmlFilePath, dom, "shim_aftk_scene_collection_merged_path", shimMergedPath);
		if(!success) return false;

		m_SourceAftkFilename = m_shimRootTempPath + "\\" + shimMergedPath + "\\" + shimMergedFilename + ".a3d";
	}

	return true;
}

bool C_DataConversionOptions::BuildDerivedOptions()
{
	Util::C_String sourceAftkFilename = Util::RemoveFileExtension( Util::ExtractFilename(m_SourceAftkFilename) );
	m_CacheDirectory = Util::ExtractDirectory(m_SourceAftkFilename) + sourceAftkFilename + "_cached\\";

	m_PreprocessedAftkFilename = m_CacheDirectory + "Input_Preproc.a3d";
	m_LightingTempDirectory = m_CacheDirectory;
	m_LitAftkFilename = m_LightingTempDirectory + "Output_Lit.a3d";
	m_LightingModelFilename = m_LightingTempDirectory + "LightingModel.xml";
	m_LightSamplePointsFilename = m_LightingTempDirectory + "SamplePoints.dat";
	m_LightSamplesFilename = m_LightingTempDirectory + "SampleData.dat";

	// Lookup lighting model options
	const C_LightingModelOptions* pLightingModelOptions = m_ToolConfig.LookupLightingModelOptions( m_LightingModelType );
	if( pLightingModelOptions )
	{
		m_LightingModelOptions = *pLightingModelOptions;
	}
	else
	{
		LOGWARN( Tools::g_log, "Invalid lighting model type '%s'", m_LightingModelType.Chars());
	}

	return true;
}

const Util::C_String& C_DataConversionOptions::GetExecutableFileNameWin32() const
{
	return m_ExecutableFileNameWin32;
}

const Util::C_String& C_DataConversionOptions::GetExecuterXmlFilePath() const
{
	return m_ExecuterXmlFilePath;
}

const Util::C_String& C_DataConversionOptions::GetSourceAftkFilename() const
{
	return m_SourceAftkFilename;
}

const Util::C_String& C_DataConversionOptions::GetPreprocessedAftkFilename() const
{
	return m_PreprocessedAftkFilename;
}

const Util::C_String& C_DataConversionOptions::GetLitAftkFilename() const
{
	return m_LitAftkFilename;
}


const Util::C_String& C_DataConversionOptions::GetLightSamplePointsFilename() const
{
	return m_LightSamplePointsFilename;
}

const Util::C_String& C_DataConversionOptions::GetLightSamplesFilename() const
{
	return m_LightSamplesFilename;
}

const Util::C_String C_DataConversionOptions::GetOutputFilename( const Tools::E_Platform platform, const Util::C_String& filename ) const
{
	Util::C_String relativeOutputFile = GetSourceAftkFilename().IReplace(GetRootTempPath(), "");
	relativeOutputFile = Util::RemoveFileExtension(relativeOutputFile);
		// duplicate level name as directory
		//Util::C_String filename = Util::ExtractFilename(relativeOutputFile);
	Util::C_String outputFilename = GetCopyLocations()[(S32)platform].workingPath + relativeOutputFile + "\\" + filename;

	return outputFilename;
}

const Util::C_String C_DataConversionOptions::GetToolDataPath() const
{
	Util::C_String executableName = Tools::FileCopy::FileCopyUtils::GetExecutableName();

	return GetRootToolsPath() + "\\data\\" + executableName;
}

const Util::C_String C_DataConversionOptions::GetToolConfigXmlFilename() const
{
	return GetToolDataPath() + "\\config\\ToolConfig.xml";
}

const Util::C_String& C_DataConversionOptions::GetLightingModelFilename() const
{
	return m_LightingModelFilename;
}

const Util::C_String& C_DataConversionOptions::GetLightingTempDirectory() const
{
	return m_LightingTempDirectory;
}

const bool C_DataConversionOptions::ReuseLastPreprocessedLevel() const
{
	return m_ReuseLastPreprocesedLevel;
}

const bool C_DataConversionOptions::ReuseLastLitLevel() const
{
	return m_ReuseLastLitLevel;
}

const C_LightingOptions& C_DataConversionOptions::GetLightingOptions() const
{
	return m_LightingOptions;
}

const C_LightingModelOptions& C_DataConversionOptions::GetLightingModelOptions() const
{
	return m_LightingModelOptions;
}

const bool C_DataConversionOptions::ExportRenderData() const
{
	return m_ExportRenderData;
}

const bool C_DataConversionOptions::ExportPhysicsData() const
{
	return m_ExportPhysicsData;
}

const bool C_DataConversionOptions::ForceFullExport() const
{
	return m_ForceFullExport;
}

const C_ToolConfig& C_DataConversionOptions::GetToolConfig() const
{
	return m_ToolConfig;
}

C_ToolConfig& C_DataConversionOptions::GetToolConfig()
{
	return m_ToolConfig;
}

const Polyflags::C_Config& C_DataConversionOptions::GetPolyflagConfig() const
{
	return m_PolyflagConfig;
}

Polyflags::C_Config& C_DataConversionOptions::GetPolyflagConfig()
{
	return m_PolyflagConfig;
}

const Objectflags::C_Config& C_DataConversionOptions::GetObjectflagConfig() const
{
	return m_ObjectflagConfig;
}

Objectflags::C_Config& C_DataConversionOptions::GetObjectflagConfig()
{
	return m_ObjectflagConfig;
}

