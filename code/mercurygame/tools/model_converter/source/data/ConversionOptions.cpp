#include "stdafx.h"

#include <tools_shared/file_copy/FileCopy.h>

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/XmlHelpers.h>
#include <converter_shared/config/ABFolderListConfig.h>

#include <Polyflags.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>

#include <core/utility/singleton.h>
#include <tools_shared/utility/MacroExpander.h>

// C_LodConfig
C_LodConfig::C_LodConfig()
	: m_LodsToGenerate(0)
	, m_ReductionFactor(0.5f)
	, m_MinVertices(3)
{
}

bool C_LodConfig::ReadXml(const Util::C_String& xmlFilePath)
{
	bool success = true;

	if(xmlFilePath.Length() == 0)
		return true;

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

bool C_LodConfig::ParseXml(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom)
{
	const Sys::C_XmlDomBaseNode* pRootNode = dom.GetRootNodePtr();

	if(!pRootNode)
		return false;

	bool success = true;

	success = XmlHelpers::GetXmlInteger(xmlFilePath, dom, "lods_to_generate", m_LodsToGenerate);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "reduction_factor", m_ReductionFactor);
	if(!success) return false;

	success = XmlHelpers::GetXmlInteger(xmlFilePath, dom, "min_vertices", m_MinVertices);
	if(!success) return false;

	return true;
}

void C_LodConfig::Dump()
{
	LOGDEBUG( Tools::g_log, "" );

	LOGDEBUG( Tools::g_log, "C_LodConfig::Dump()" );
	LOGDEBUG( Tools::g_log, "	m_LodsToGenerate: %d", m_LodsToGenerate );
	LOGDEBUG( Tools::g_log, "	m_ReductionFactor: %f", m_ReductionFactor );
	LOGDEBUG( Tools::g_log, "	m_MinVertices: %d", m_MinVertices );
}

// C_LightingOptions

const bool C_LightingOptions::GetLightScene() const
{
	return m_LightModel;
}

const bool C_LightingOptions::GetUseLightingScreensavers() const
{
	return m_UseLightingScreensavers;
}

void C_LightingOptions::Dump()
{
	LOGDEBUG( Tools::g_log, "" );

	LOGDEBUG( Tools::g_log, "C_LightingOptions::Dump()" );
	LOGDEBUG( Tools::g_log, "	m_LightModel: %d", m_LightModel );
	LOGDEBUG( Tools::g_log, "	m_UseLightingScreensavers: %d", m_UseLightingScreensavers );

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
		// Make sure you catch t'clap
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

bool C_DataConversionOptions::ParseXml(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom)
{
	if (!Tools::Shim::C_Options::ParseXml(xmlFilePath, dom))
		return false;

	BuildSourceAftkFilename(xmlFilePath, dom);

	//Our Aftk input MUST come from our intermediate directory.
	if (m_SourceAftkFilename.ISearch(m_shimRootTempPath) == -1)
	{
		LOGERROR( Tools::g_log, "Aftk input file is not in correct intermediate directory! It should be in '%s'", m_shimRootTempPath.Chars() );
		//Ah, it doesn't :(
		return false;
	}

	//Find interesting info provided in the XML file from Executer.

	bool success = true;

	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "model_type", m_ModelType);
	if(!success) return false;

	success = XmlHelpers::GetXmlFloat(xmlFilePath, dom, "scene_scale", m_SceneScale);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "rotate_y_180", m_RotateY180);
	if(!success) return false;

	m_LodFade = false;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "lod_fade", m_LodFade, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "reuse_last_lit_level", m_ReuseLastLitLevel);
	if(!success) return false;

	m_ReuseLastVisibilityLevel = false;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "reuse_last_visibility_level", m_ReuseLastVisibilityLevel, XmlHelpers::E_Behaviour_Silent);

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "export_render_data", m_ExportRenderData);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "export_physics_data", m_ExportPhysicsData);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "reuse_last_preproc_level", m_ReuseLastPreprocesedLevel);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "reuse_last_lit_level", m_ReuseLastLitLevel);
	if(!success) return false;

	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "light_model", m_LightingOptions.m_LightModel);
	if(!success) return false;
	
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "use_network_screensavers", m_LightingOptions.m_UseLightingScreensavers);
	if(!success) return false;

	m_VisibilityOptions.m_VisibilityScene = false;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "visibility_scene", m_VisibilityOptions.m_VisibilityScene, XmlHelpers::E_Behaviour_Warn);

	m_VisibilityOptions.m_UseLightingScreensavers = true;
	success = XmlHelpers::GetXmlBool(xmlFilePath, dom, "use_visibility_network_screensavers", m_VisibilityOptions.m_UseLightingScreensavers);

	Util::C_String lodConfigFilename;
	success = XmlHelpers::GetXmlString(xmlFilePath, dom, "lod_config_filename", lodConfigFilename, XmlHelpers::E_Behaviour_Silent);
	if(!success) return false;
	if( lodConfigFilename.Length() > 0)
	{
		m_LodConfigFilename = lodConfigFilename;
	}

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

	m_VisTempDirectory = m_CacheDirectory + "Vis\\";
	m_VisInputFilename = m_VisTempDirectory + "VisInput.dat";
	m_VisOutputFilename = m_VisTempDirectory + "VisOutput.dat";
	m_PVSDataFilename = m_CacheDirectory + "PVSData.dat";

	// Lookup model options
	const C_ModelOptions* pModelOptions = m_ToolConfig.LookupModelOptions( m_ModelType );
	if( pModelOptions )
	{
		m_ModelOptions = *pModelOptions;
	}
	else
	{
		LOGWARN( Tools::g_log, "Invalid model type '%s'", m_ModelType.Chars());
	}

	// if no lod config filename specified, use default from model options
	if( m_LodConfigFilename.Length() == 0)
	{
		m_LodConfigFilename = m_ModelOptions.GetLodConfigFilename();
	}
	Tools::C_MacroExpander::Instance().Expand(m_LodConfigFilename);

	m_LodConfig.ReadXml( m_LodConfigFilename );

	ABFolderList::C_Config::AddToFolderList(m_CacheDirectory);

	return true;
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

const Util::C_String C_DataConversionOptions::GetOutputFilename( const Util::E_Platform platform ) const
{
	Util::C_String relativeOutputFile = GetSourceAftkFilename().IReplace(GetRootTempPath(), "");
	relativeOutputFile = Util::RemoveFileExtension(relativeOutputFile);
		// duplicate level name as directory
		//Util::C_String filename = Util::ExtractFilename(relativeOutputFile);
	Util::C_String outputFilename = GetCopyLocations()[(S32)platform].workingPath + relativeOutputFile + ".model";

	return outputFilename;
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

const Util::C_String& C_DataConversionOptions::GetModelType() const
{
	return m_ModelType;
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

const C_ModelOptions& C_DataConversionOptions::GetModelOptions() const
{
	return m_ModelOptions;
}

const Util::C_String& C_DataConversionOptions::GetLodConfigFilename() const
{
	return m_LodConfigFilename;
}

const C_LodConfig& C_DataConversionOptions::GetLodConfig() const
{
	return m_LodConfig;
}

const F32 C_DataConversionOptions::SceneScale() const
{
	return m_SceneScale;
}

const bool C_DataConversionOptions::RotateY180() const
{
	return m_RotateY180;
}

const bool C_DataConversionOptions::LodFade() const
{
	return m_LodFade;
}

const bool C_DataConversionOptions::ExportRenderData() const
{
	return m_ExportRenderData;
}

const bool C_DataConversionOptions::ExportPhysicsData() const
{
	return m_ExportPhysicsData;
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

void C_DataConversionOptions::Dump()
{
	LOGDEBUG( Tools::g_log, "" );

	LOGDEBUG( Tools::g_log, "C_DataConversionOptions::Dump()" );
	LOGDEBUG( Tools::g_log, "	m_ExecuterXmlFilePath: %s", m_ExecuterXmlFilePath.Chars() );
	LOGDEBUG( Tools::g_log, "	m_SourceAftkFilename: %s", m_SourceAftkFilename.Chars() );
	LOGDEBUG( Tools::g_log, "	m_CacheDirectory: %s", m_CacheDirectory.Chars() );
	LOGDEBUG( Tools::g_log, "	m_PreprocessedAftkFilename: %s", m_PreprocessedAftkFilename.Chars() );
	LOGDEBUG( Tools::g_log, "	m_LightingTempDirectory: %s", m_LightingTempDirectory.Chars() );
	LOGDEBUG( Tools::g_log, "	m_LightingModelFilename: %s", m_LightingModelFilename.Chars() );
	LOGDEBUG( Tools::g_log, "	m_LitAftkFilename: %s", m_LitAftkFilename.Chars() );
	LOGDEBUG( Tools::g_log, "	m_ModelType: %s", m_ModelType.Chars() );
	LOGDEBUG( Tools::g_log, "	m_SceneScale: %f", m_SceneScale );
	LOGDEBUG( Tools::g_log, "	m_RotateY180: %d", m_RotateY180 );
	LOGDEBUG( Tools::g_log, "	m_LodFade: %d", m_LodFade);
	LOGDEBUG( Tools::g_log, "	m_ReuseLastPreprocesedLevel: %d", m_ReuseLastPreprocesedLevel );
	LOGDEBUG( Tools::g_log, "	m_ReuseLastLitLevel: %d", m_ReuseLastLitLevel );
	LOGDEBUG( Tools::g_log, "	m_ExportRenderData: %d", m_ExportRenderData );
	LOGDEBUG( Tools::g_log, "	m_ExportPhysicsData: %d", m_ExportPhysicsData );
	LOGDEBUG( Tools::g_log, "	m_LodConfigFilename: %s", m_LodConfigFilename.Chars() );
	LOGDEBUG( Tools::g_log, "   m_VisTempDirectory: %s", m_VisTempDirectory.Chars() );
	LOGDEBUG( Tools::g_log, "   m_VisInputFilename: %s", m_VisInputFilename.Chars() );
	LOGDEBUG( Tools::g_log, "   m_VisOutputFilename: %s", m_VisOutputFilename.Chars() );
	LOGDEBUG( Tools::g_log, "" );

	m_LightingOptions.Dump();
	m_ModelOptions.Dump();
	m_LodConfig.Dump();
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
