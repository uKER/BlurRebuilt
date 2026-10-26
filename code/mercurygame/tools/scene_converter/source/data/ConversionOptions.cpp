#include "stdafx.h"

#include <tools_shared/file_copy/FileCopy.h>

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/XmlHelpers.h>
#include <converter_shared/config/ABFolderListConfig.h>

#include <Polyflags.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>

#include <system/time/time.h>

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

const F32 C_LightingOptions::GetLightmapDensity() const
{
	return m_LightmapDensity;
}

const F32 C_LightingOptions::GetLightSampleGridSpacing() const
{
	return m_LightSampleGridSpacing;
}

const F32 C_LightingOptions::GetLightSampleHeightOffset() const
{
	return m_LightSampleHeightOffset;
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

const F32 C_LightingOptions::GetSunPenumbra() const
{
	return m_SunPenumbra;
}

const Maths::C_RGBA_F32 C_LightingOptions::GetSunColour() const
{
	return m_SunColour;
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

const Maths::C_RGBA_F32 C_LightingOptions::GetSkyColour() const
{
	return m_SkyColour;
}


void C_LightingOptions::Dump()
{
	LOGDEBUG( Tools::g_log, "" );
	LOGDEBUG( Tools::g_log, "C_LightingOptions::Dump()" );

	LOGDEBUG( Tools::g_log, "m_LightScene: %d", m_LightScene );
	LOGDEBUG( Tools::g_log, "m_UseLightingScreensavers: %d", m_UseLightingScreensavers );
	LOGDEBUG( Tools::g_log, "m_LightmapResolution: %d", m_LightmapResolution );
	LOGDEBUG( Tools::g_log, "m_LightmapQuality: %d", m_LightmapQuality );
	LOGDEBUG( Tools::g_log, "m_LightmapDensity: %f", m_LightmapDensity );
	LOGDEBUG( Tools::g_log, "m_LightSampleGridSpacing: %f", m_LightSampleGridSpacing );
	LOGDEBUG( Tools::g_log, "m_LightSampleHeightOffset: %f", m_LightSampleHeightOffset );
	LOGDEBUG( Tools::g_log, "m_AddSunLight: %d", m_AddSunLight );
	LOGDEBUG( Tools::g_log, "m_SunElevation: %f", m_SunElevation );
	LOGDEBUG( Tools::g_log, "m_SunHeading: %f", m_SunHeading );
	LOGDEBUG( Tools::g_log, "m_SunColour: %f, %f, %f, %f", m_SunColour.R(), m_SunColour.G(), m_SunColour.B(), m_SunColour.A() );
	if (m_SunPenumbra > 15.f)
		LOGWARN( Tools::g_log, "m_SunPenumbra: %f5.2 degrees (this is probably too high)", m_SunPenumbra );
	else
		LOGDEBUG( Tools::g_log, "m_SunPenumbra: %f5.2 degrees", m_SunPenumbra );
	LOGDEBUG( Tools::g_log, "m_AddSkyLight: %d", m_AddSkyLight );
	LOGDEBUG( Tools::g_log, "m_SkyIntensity: %f", m_SkyIntensity );
	LOGDEBUG( Tools::g_log, "m_BouncePasses: %d", m_BouncePasses );
	LOGDEBUG( Tools::g_log, "m_BounceScale: %f", m_BounceScale );
	LOGDEBUG( Tools::g_log, "m_SkyColour: %f, %f, %f, %f", m_SkyColour.R(), m_SkyColour.G(), m_SkyColour.B(), m_SkyColour.A() );

	LOGDEBUG( Tools::g_log, "" );
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

	m_CrowdRandomSeed = static_cast<U32>(Time::GetCurrentTime());

	success = BuildDerivedOptions();
	if (!success) return false;

	Dump();

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
	catch (TCLAP::ArgException &e)  // catch any exceptions
	{ 
		LOGERROR( Tools::g_log, "Error: %s %s", e.error().c_str(), e.argId().c_str() );
		return false;
	}

	//catch (TCLAP::CmdLineParseException &e)
	//{
	//	std::cerr << "Failed to parse arguments: " << e.argId() << std::endl;
	//	return false;
	//}
	//catch (TCLAP::ArgException &e)  // catch any exceptions
	//{ 
	//	std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
	//	return false;
	//}

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

	m_ReuseLastLitLevel = false;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "reuse_last_lit_level", m_ReuseLastLitLevel, XmlHelpers::E_Behaviour_Warn);

	m_ExportRenderData = true;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "export_render_data", m_ExportRenderData, XmlHelpers::E_Behaviour_Warn);

	m_ExportPhysicsData = true;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "export_physics_data", m_ExportPhysicsData, XmlHelpers::E_Behaviour_Warn);

	m_ReuseLastPreprocesedLevel = false;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "reuse_last_preproc_level", m_ReuseLastPreprocesedLevel, XmlHelpers::E_Behaviour_Warn);

	m_ReuseLastLitLevel = false;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "reuse_last_lit_level", m_ReuseLastLitLevel, XmlHelpers::E_Behaviour_Warn);

	m_ReuseLastVisibilityLevel = false;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "reuse_last_visibility_level", m_ReuseLastVisibilityLevel, XmlHelpers::E_Behaviour_Warn);

	m_ForceFullExport = false;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "force_full_export", m_ForceFullExport, XmlHelpers::E_Behaviour_Warn);

	m_ExportStreamFile = true;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "export_stream_file", m_ExportStreamFile, XmlHelpers::E_Behaviour_Warn);

	m_ForceAllSolid = false;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "force_all_solid", m_ForceAllSolid, XmlHelpers::E_Behaviour_Warn);

	m_RenderCollideMesh = false;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "render_collide_mesh", m_RenderCollideMesh, XmlHelpers::E_Behaviour_Warn);

	m_ForceExtraCollideMeshFlags = false;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "force_extra_collide_mesh_flags", m_ForceExtraCollideMeshFlags, XmlHelpers::E_Behaviour_Silent);
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
	
	// default taken from toolconfig, overridden if executer option present
	m_LightingOptions.m_LightmapDensity = -1.0f;
	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "lightmap_density", m_LightingOptions.m_LightmapDensity, XmlHelpers::E_Behaviour_Silent );
	if(!success) return false;

	// default taken from toolconfig, overridden if executer option present
	m_LightingOptions.m_LightSampleGridSpacing = -1;
	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "light_sample_grid_spacing", m_LightingOptions.m_LightSampleGridSpacing, XmlHelpers::E_Behaviour_Silent );
	if(!success) return false;

	// default taken from toolconfig, overridden if executer option present
	m_LightingOptions.m_LightSampleHeightOffset = -1;
	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "light_sample_height_offset", m_LightingOptions.m_LightSampleHeightOffset, XmlHelpers::E_Behaviour_Silent );
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "add_sun_light", m_LightingOptions.m_AddSunLight);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "sun_elevation", m_LightingOptions.m_SunElevation);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "sun_heading", m_LightingOptions.m_SunHeading);
	if(!success) return false;

	m_LightingOptions.m_SunColour = Maths::HVec4(1.0f, 1.0f, 1.0f, 1.0f);
	success = XmlHelpers::GetXmlColour(xmlFilePath, dom, "sun_colour", m_LightingOptions.m_SunColour, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	m_LightingOptions.m_SunPenumbra = -1.f;
	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "sun_penumbra", m_LightingOptions.m_SunPenumbra, XmlHelpers::E_Behaviour_Silent);
	if (m_LightingOptions.m_SunPenumbra == -1.f)
		m_LightingOptions.m_SunPenumbra = GetToolConfig().GetSunPenumbra();

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "add_sky_light", m_LightingOptions.m_AddSkyLight);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "sky_intensity", m_LightingOptions.m_SkyIntensity);
	if(!success) return false;

	success = XmlHelpers::GetXmlInteger(xmlFilePath, dom, "bounce_passes", m_LightingOptions.m_BouncePasses);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "bounce_scale", m_LightingOptions.m_BounceScale);
	if(!success) return false;

	m_LightingOptions.m_SkyColour = Maths::HVec4(1.0f, 1.0f, 1.0f, 1.0f);
	success = XmlHelpers::GetXmlColour(xmlFilePath, dom, "sky_colour", m_LightingOptions.m_SkyColour, XmlHelpers::E_Behaviour_Silent);

	m_VisibilityOptions.m_VisibilityScene = false;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "visibility_scene", m_VisibilityOptions.m_VisibilityScene, XmlHelpers::E_Behaviour_Warn);

	m_VisibilityOptions.m_UseLightingScreensavers = true;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "use_visibility_network_screensavers", m_VisibilityOptions.m_UseLightingScreensavers, XmlHelpers::E_Behaviour_Warn);

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "default_cubemap_filename", m_DefaultCubemapFilename, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	m_PvsZoneSize = -1;
	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "pvs_zone_size", m_PvsZoneSize, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	m_PvsZoneHeight = -1;
	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "pvs_zone_height", m_PvsZoneHeight, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	m_PvsSampleSpacing = -1.0f;
	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "pvs_sample_spacing", m_PvsSampleSpacing, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	m_PvsClusterThreshold = 7;
	success = XmlHelpers::GetXmlInteger(xmlFilePath, dom, "pvs_cluster_threshold", m_PvsClusterThreshold, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	m_PvsPolyThreshold = 1;
	success = XmlHelpers::GetXmlInteger(xmlFilePath, dom, "pvs_poly_threshold", m_PvsPolyThreshold, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	for(Util::E_Platform platform = Util::E_Platform_Begin; platform!=Util::E_Platform_End; platform++)
	{
		m_ExportForPlatform[platform] = true;
	}

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "export_for_360", m_ExportForPlatform[Util::E_Platform_360], XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "export_for_ps3", m_ExportForPlatform[Util::E_Platform_PS3], XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "export_for_pc", m_ExportForPlatform[Util::E_Platform_PC], XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	ABFolderList::C_Config::ReadABFolders(dom);

	return true;
}

bool C_DataConversionOptions::BuildSourceAftkFilename(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom)
{
	bool success = true;
	if( m_SourceAftkFilename == "" )
	{
		Util::C_String shimMergedFilename;
		success = XmlHelpers::GetXmlString(xmlFilePath, dom, "shim_aftk_scene_collection_merged_filename", shimMergedFilename);
		if(!success) return false;

		Util::C_String shimMergedPath;
		success = XmlHelpers::GetXmlString(xmlFilePath, dom, "shim_aftk_scene_collection_merged_path", shimMergedPath);
		if(!success) return false;

		m_SourceAftkFilename = m_shimRootTempPath + "\\" + shimMergedPath + "\\" + shimMergedFilename + ".a3d";

		LOGDEBUG(Tools::g_log, "Source Aftk Filename not specified, generating from Executer options (%s)", m_SourceAftkFilename.Chars() );
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

	m_ImposterTempDirectory = m_CacheDirectory + "Imposters\\";
	m_ImposterSetupFilename = m_ImposterTempDirectory + "Imposters.a3d";

	m_TreeTempDirectory = m_CacheDirectory + "Trees\\";
	m_TreeLightingModelFilename = m_TreeTempDirectory + "TreeLightingModel.xml";
	m_TreeSetupFilename = m_TreeTempDirectory + "TreeGen.xml";
	m_TreeInputFilename = m_TreeTempDirectory + "TreeInput.a3d";
	m_TreeOutputFilename = m_TreeTempDirectory + "TreeOutput.a3d";

	m_VisTempDirectory = m_CacheDirectory + "Vis\\";
	m_VisInputFilename = m_VisTempDirectory + "VisInput.dat";
	m_VisOutputFilename = m_VisTempDirectory + "VisOutput.dat";
	m_PVSDataFilename = m_CacheDirectory + "PVSData.dat";

	m_CrowdCropSheetDirectory = m_CacheDirectory + "Crowd";

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

	if(m_DefaultCubemapFilename.Length() == 0)
	{
		m_DefaultCubemapFilename = GetToolConfig().GetDefaultCubemapFilename();		
	}

	if( m_LightingOptions.m_LightmapDensity == -1 )
	{
		m_LightingOptions.m_LightmapDensity = GetToolConfig().GetLightmapDensity();
	}

	if( m_LightingOptions.m_LightSampleGridSpacing == -1 )
	{
		m_LightingOptions.m_LightSampleGridSpacing = GetToolConfig().GetLightSampleGridSpacing();
	}

	if( m_LightingOptions.m_LightSampleHeightOffset == -1 )
	{
		m_LightingOptions.m_LightSampleHeightOffset = GetToolConfig().GetLightSampleHeightOffset();
	}
	
	if(m_PvsZoneSize == -1)
	{
		m_PvsZoneSize = GetToolConfig().GetPvsZoneSize();
	}

	if(m_PvsZoneHeight == -1)
	{
		m_PvsZoneHeight = GetToolConfig().GetPvsZoneHeight();
	}

	if(m_PvsSampleSpacing == -1)
	{
		m_PvsSampleSpacing = GetToolConfig().GetPvsSampleSpacing();
	}

	ABFolderList::C_Config::AddToFolderList(m_CacheDirectory);

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

const Util::C_String C_DataConversionOptions::GetOutputFilename( const Util::E_Platform platform, const Util::C_String& filename ) const
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

const Util::C_String& C_DataConversionOptions::GetImposterTempDirectory() const
{
	return m_ImposterTempDirectory;
}

const Util::C_String& C_DataConversionOptions::GetImposterSetupFilename() const
{
	return m_ImposterSetupFilename;
}

const Util::C_String& C_DataConversionOptions::GetTreeTempDirectory() const
{
	return m_TreeTempDirectory;
}

const Util::C_String& C_DataConversionOptions::GetTreeSetupFilename() const
{
	return m_TreeSetupFilename;
}

const Util::C_String& C_DataConversionOptions::GetTreeLightingModelFilename() const
{
	return m_TreeLightingModelFilename;
}

const Util::C_String& C_DataConversionOptions::GetTreeInputFilename() const
{
	return m_TreeInputFilename;
}

const Util::C_String& C_DataConversionOptions::GetTreeOutputFilename() const
{
	return m_TreeOutputFilename;
}

const Util::C_String& C_DataConversionOptions::GetVisTempDirectory() const
{
	return m_VisTempDirectory;
}

const Util::C_String& C_DataConversionOptions::GetVisInputFilename() const
{
	return m_VisInputFilename;
}

const Util::C_String& C_DataConversionOptions::GetVisOutputFilename() const
{
	return m_VisOutputFilename;
}

const Util::C_String& C_DataConversionOptions::GetPVSDataFilename() const
{
	return m_PVSDataFilename;
}

const Util::C_String& C_DataConversionOptions::GetCrowdCropSheetDirectory() const
{
	return m_CrowdCropSheetDirectory;
}

const Util::C_String& C_DataConversionOptions::GetDefaultCubemapFilename() const
{
	return m_DefaultCubemapFilename;	
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

const bool C_DataConversionOptions::ExportStreamFile() const
{
	return m_ExportStreamFile;
}

const bool C_DataConversionOptions::ExportCrowdData() const
{
	return true;
}

const bool C_DataConversionOptions::IgnoreRoutes() const
{
	return false;
}
	
const bool C_DataConversionOptions::ForceAllSolid() const
{
	return m_ForceAllSolid;
}

const bool C_DataConversionOptions::RenderCollideMesh() const
{
	return m_RenderCollideMesh;
}

const bool C_DataConversionOptions::ForceExtraCollideMeshFlags() const
{
	return m_ForceExtraCollideMeshFlags;
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

const F32 C_DataConversionOptions::GetPvsZoneSize() const
{
	return m_PvsZoneSize;
}

const F32 C_DataConversionOptions::GetPvsZoneHeight() const
{
	return m_PvsZoneHeight;
}

const F32 C_DataConversionOptions::GetPvsSampleSpacing() const
{
	return m_PvsSampleSpacing;
}

const S32 C_DataConversionOptions::GetPVSClusterThreshold() const
{
	return m_PvsClusterThreshold;
}

const S32 C_DataConversionOptions::GetPVSPolyThreshold() const
{
	return m_PvsPolyThreshold;
}

const bool C_DataConversionOptions::ExportForPlatform( const Util::E_Platform platform ) const
{
	BIZ_ASSERT( platform < Util::E_Platform_Count);
	return m_ExportForPlatform[platform];
}

void C_DataConversionOptions::Dump()
{
	LOGDEBUG( Tools::g_log, "" );

	LOGDEBUG( Tools::g_log, "C_DataConversionOptions::Dump()" );
	LOGDEBUG( Tools::g_log, "m_ExecutableFileNameWin32: %s", m_ExecutableFileNameWin32.Chars() );
	LOGDEBUG( Tools::g_log, "m_ExecuterXmlFilePath: %s", m_ExecuterXmlFilePath.Chars() );
	LOGDEBUG( Tools::g_log, "m_SourceAftkFilename: %s", m_SourceAftkFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_CacheDirectory: %s", m_CacheDirectory.Chars() );
	LOGDEBUG( Tools::g_log, "m_PreprocessedAftkFilename: %s", m_PreprocessedAftkFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_LightingTempDirectory: %s", m_LightingTempDirectory.Chars() );
	LOGDEBUG( Tools::g_log, "m_LightingModelFilename: %s", m_LightingModelFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_LitAftkFilename: %s", m_LitAftkFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_LightSamplePointsFilename: %s", m_LightSamplePointsFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_LightSamplesFilename: %s", m_LightSamplesFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_LightingModelType: %s", m_LightingModelType.Chars() );
	LOGDEBUG( Tools::g_log, "m_DefaultCubemapFilename: %s", m_DefaultCubemapFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_ReuseLastPreprocesedLevel: %d", m_ReuseLastPreprocesedLevel );
	LOGDEBUG( Tools::g_log, "m_ReuseLastLitLevel: %d", m_ReuseLastLitLevel );
	LOGDEBUG( Tools::g_log, "m_ExportRenderData: %d", m_ExportRenderData );
	LOGDEBUG( Tools::g_log, "m_ExportPhysicsData: %d", m_ExportPhysicsData );
	LOGDEBUG( Tools::g_log, "m_ForceFullExport: %d", m_ForceFullExport );
	LOGDEBUG( Tools::g_log, "m_ExportStreamFile: %d", m_ExportStreamFile );
	LOGDEBUG( Tools::g_log, "m_ForceAllSolid: %d", m_ForceAllSolid );
	LOGDEBUG( Tools::g_log, "m_RenderCollideMesh: %d", m_RenderCollideMesh );
	LOGDEBUG( Tools::g_log, "m_ForceExtraCollideMeshFlags: %d", m_ForceExtraCollideMeshFlags );	
	LOGDEBUG( Tools::g_log, "m_PvsZoneSize: %f", m_PvsZoneSize );
	LOGDEBUG( Tools::g_log, "m_PvsClusterThreshold: %d", m_PvsClusterThreshold );
	LOGDEBUG( Tools::g_log, "m_PvsPolyThreshold: %d", m_PvsPolyThreshold );
	LOGDEBUG( Tools::g_log, "m_PvsSampleSpacing: %f", m_PvsSampleSpacing );

	for(Util::E_Platform platform = Util::E_Platform_Begin; platform!=Util::E_Platform_End; platform++)
	{
		LOGDEBUG( Tools::g_log, "m_ExportForPlatform[%d|%s]: %f", platform, Util::GetPlatformString(platform).Chars(), ExportForPlatform(platform) );
	}


	LOGDEBUG( Tools::g_log, "m_ImposterTempDirectory: %s", m_ImposterTempDirectory.Chars() );
	LOGDEBUG( Tools::g_log, "m_TreeTempDirectory: %s", m_TreeTempDirectory.Chars() );
	LOGDEBUG( Tools::g_log, "m_TreeLightingModelFilename: %s", m_TreeLightingModelFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_TreeSetupFilename: %s", m_TreeSetupFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_TreeInputFilename: %s", m_TreeInputFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_TreeOutputFilename: %s", m_TreeOutputFilename.Chars() );

	LOGDEBUG( Tools::g_log, "m_VisTempDirectory: %s", m_VisTempDirectory.Chars() );
	LOGDEBUG( Tools::g_log, "m_VisInputFilename: %s", m_VisInputFilename.Chars() );
	LOGDEBUG( Tools::g_log, "m_VisOutputFilename: %s", m_VisOutputFilename.Chars() );

	LOGDEBUG( Tools::g_log, "m_CrowdCropSheetDirectory: %s", m_CrowdCropSheetDirectory.Chars() );
	
	LOGDEBUG( Tools::g_log, "" );

	m_LightingOptions.Dump();
	m_LightingModelOptions.Dump();
	m_VisibilityOptions.Dump();
}

const C_VisibilityOptions& C_DataConversionOptions::GetVisibilityOptions() const
{
	return m_VisibilityOptions;
}

const bool C_DataConversionOptions::ReuseLastVisibilityLevel() const
{
	return m_ReuseLastVisibilityLevel;
}
const bool C_VisibilityOptions::GetVisibilityScene() const
{
	return m_VisibilityScene;
}

const bool C_VisibilityOptions::GetUseLightingScreensavers() const
{
	return m_UseLightingScreensavers;
}

void C_VisibilityOptions::Dump()
{
	LOGDEBUG( Tools::g_log, "" );
	LOGDEBUG( Tools::g_log, "C_VisibilityOptions::Dump()" );

	LOGDEBUG( Tools::g_log, "m_LightScene: %d", m_VisibilityScene );
	LOGDEBUG( Tools::g_log, "m_UseLightingScreensavers: %d", m_UseLightingScreensavers );

	LOGDEBUG( Tools::g_log, "" );
}
