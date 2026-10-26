#include "stdafx.h"
#include "Options.h"

#include <tools_shared/shim/ShimErrorHelpers.h>

#define EXECUTER_XML_NULL_CONVERTER_FILE			"export_null_converter_file"
#define EXECUTER_XML_NULL_CONVERTER_FILENAME		"export_null_converter_filename"
#define EXECUTER_XML_NULL_CONVERTER_OUTPUT_PATH		"export_null_converter_file_output_path"

#define MISSING_XML_ATTR "Could not find required XML attribute in %s.\n"

bool C_Options::ParseCommandLine(
	Util::C_String& xmlFilePath,
	int argc, 
	char* argv[])
{
	// Wrap everything in a try block.  Do this every time, 
	// because exceptions will be thrown for problems. 
	try
	{  
		//Define the command line object.
		TCLAP::CmdLine cmd("Ai Converter.", ' ', "1.00");

		//Define a value argument and add it to the command line.
		TCLAP::ValueArg<std::string> xmlFileArg(
			"","xml","XML file",true,"","Executer XML file");
		cmd.add(xmlFileArg);

		//Parse the args.
		cmd.parse(argc, argv);

		//Get the values
		xmlFilePath = xmlFileArg.getValue().c_str();
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

bool C_Options::ParseXml(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom)
{
	if (!Tools::Shim::C_Options::ParseXml(xmlFilePath, dom))
		return false;

	Ut::Vector<const Sys::C_XmlDomBaseNode*> mayaScene;
	Sys::C_XmlDomUtility::XpathEvaluate(dom.GetRootNodePtr(), "**/container[@name='ai_maya_scene']", mayaScene);
	Ut::Vector<const Sys::C_XmlDomBaseNode*> outputName;
	Sys::C_XmlDomUtility::XpathEvaluate(dom.GetRootNodePtr(), "*/type_entry/values/**/container[@name='ai_output_name']", outputName);
	Ut::Vector<const Sys::C_XmlDomBaseNode*> dontExport;
	Sys::C_XmlDomUtility::XpathEvaluate(dom.GetRootNodePtr(), "**/container[@name='ai_override_dont_export']", dontExport);
	Ut::Vector<const Sys::C_XmlDomBaseNode*> levelPath;
	Sys::C_XmlDomUtility::XpathEvaluate(dom.GetRootNodePtr(), "*/type_entry/tagdata/levelpath", levelPath);

	BEGIN_CHECK_CUSTOM(mayaScene.Size() != 1) ERROR_ERROR("Cannot find maya scene container") END_RETURN_FALSE
	BEGIN_CHECK_CUSTOM(outputName.Size() != 1) ERROR_ERROR("Cannot find output name container") END_RETURN_FALSE
	BEGIN_CHECK_CUSTOM(dontExport.Size() != 1) ERROR_ERROR("Cannot find don't export container") END_RETURN_FALSE
	BEGIN_CHECK_CUSTOM(levelPath.Size() != 1) ERROR_ERROR("Cannot find level path in tagdata container") END_RETURN_FALSE

	BEGIN_CHECK_CUSTOM(!((Sys::C_XmlDomElement*)mayaScene[0])->GetAttributes().GetAttributeByName("value")) ERROR_ERROR("Value of maya scene does not exist") END_RETURN_FALSE
	BEGIN_CHECK_CUSTOM(!((Sys::C_XmlDomElement*)outputName[0])->GetAttributes().GetAttributeByName("value")) ERROR_ERROR("Value of output name does not exist") END_RETURN_FALSE
	BEGIN_CHECK_CUSTOM(!((Sys::C_XmlDomElement*)dontExport[0])->GetAttributes().GetAttributeByName("value")) ERROR_ERROR("Value of don't export does not exist") END_RETURN_FALSE
	BEGIN_CHECK_CUSTOM(!((Sys::C_XmlDomElement*)levelPath[0])->GetAttributes().GetAttributeByName("value")) ERROR_ERROR("Value of level path tagdata does not exist") END_RETURN_FALSE

	m_mayaSceneValue = ((Sys::C_XmlDomElement*)mayaScene[0])->GetAttributes().GetAttributeByName("value")->value;
	m_outputNameValue = ((Sys::C_XmlDomElement*)outputName[0])->GetAttributes().GetAttributeByName("value")->value;
	m_levelPathValue = ((Sys::C_XmlDomElement*)levelPath[0])->GetAttributes().GetAttributeByName("value")->value;
	m_shouldExport = ((Sys::C_XmlDomElement*)dontExport[0])->GetAttributes().GetAttributeByName("value")->value == "false";
	
	// Calculate the output location -- if the parent data is available, use that, else get the data from the local containers

	Ut::Vector<const Sys::C_XmlDomBaseNode*> mergedFilename;
	Sys::C_XmlDomUtility::XpathEvaluate(dom.GetRootNodePtr(), "*/type_entry/parent_data/**/container[@shim_tag='shim_output_file']", mergedFilename);
	Ut::Vector<const Sys::C_XmlDomBaseNode*> mergedPath;
	Sys::C_XmlDomUtility::XpathEvaluate(dom.GetRootNodePtr(), "*/type_entry/parent_data/**/container[@shim_tag='shim_output_path']", mergedPath);

	// Parent data doesn't exist, use local data
	if (mergedFilename.Size() != 1)
		Sys::C_XmlDomUtility::XpathEvaluate(dom.GetRootNodePtr(), "*/type_entry/values/**/container[@name='ai_level_name']", mergedFilename);
	if (mergedPath.Size() != 1)
		Sys::C_XmlDomUtility::XpathEvaluate(dom.GetRootNodePtr(), "*/type_entry/values/**/container[@name='ai_output_directory']", mergedPath);

	BEGIN_CHECK_CUSTOM(mergedFilename.Size() != 1) ERROR_ERROR("Cannot find merged filename container") END_RETURN_FALSE
	BEGIN_CHECK_CUSTOM(mergedPath.Size() != 1) ERROR_ERROR("Cannot find merged path container") END_RETURN_FALSE

	BEGIN_CHECK_CUSTOM(!((Sys::C_XmlDomElement*)mergedFilename[0])->GetAttributes().GetAttributeByName("value")) ERROR_ERROR("Value of merged filename does not exist") END_RETURN_FALSE
	BEGIN_CHECK_CUSTOM(!((Sys::C_XmlDomElement*)mergedPath[0])->GetAttributes().GetAttributeByName("value")) ERROR_ERROR("Value of merged path does not exist") END_RETURN_FALSE
	m_trackName = ((Sys::C_XmlDomElement*)mergedFilename[0])->GetAttributes().GetAttributeByName("value")->value;
	m_outputPath = ((Sys::C_XmlDomElement*)mergedPath[0])->GetAttributes().GetAttributeByName("value")->value;

	m_outputPath = Util::FormatWindowsFileString(m_outputPath);
	m_outputPath = Util::RemoveTrailingSlash(m_outputPath);
	m_outputPath = Util::RemoveLeadingSlash(m_outputPath);

	m_outputFilename = m_outputNameValue + ".aid";

	m_inputTempFilePath =  C_Options::GetRootTempPath() + m_outputPath;

	return true;
}

const Util::C_String& C_Options::GetInputTempFilePath() const
{
	return m_inputTempFilePath;
}

const Util::C_String& C_Options::GetOutputFilename() const
{
	return m_outputFilename;
}

const Util::C_String& C_Options::GetOutputPath() const
{
	return m_outputPath;
}

const Util::C_String& C_Options::GetMayaSceneValue() const
{
	return m_mayaSceneValue;
}

const Util::C_String& C_Options::GetLevelPath() const
{
	return m_levelPathValue;
}

const Util::C_String& C_Options::GetTrackName() const
{
	return m_trackName;
}

bool C_Options::ShouldExport() const
{
	return m_shouldExport;
}