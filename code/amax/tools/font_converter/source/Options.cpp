#include "stdafx.h"
#include "Options.h"

#define EXECUTER_XML_FONT_CONVERTER_CONTAINER		"export_font_converter_file"
#define EXECUTER_XML_FONT_CONVERTER_CHARLIST		"export_font_converter_charlist"
#define EXECUTER_XML_FONT_CONVERTER_TEXTURE			"export_font_converter_texture"
#define EXECUTER_XML_FONT_CONVERTER_OUTPUT_PATH		"export_font_converter_file_output"

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
		TCLAP::CmdLine cmd("Font Exporter.", ' ', "1.00");

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

	//Where to put it.
	Util::C_String outputDirectory;

	if (!ParseOutputDirectory(dom))
		return false;

	if (!ParseInputAssetFilePath(dom.GetRootNodePtr(), outputDirectory))
		return false;

	m_inputTempFilePath = m_inputCharlistFile.IReplace(m_shimRootAssetPath, m_shimRootTempPath);
	if (m_inputTempFilePath == "")
		return false;

	return true;
}

const Util::C_String& C_Options::GetInputAssetFilePath() const
{
	return m_inputCharlistFile;
}

const Util::C_String& C_Options::GetInputTempFilePath() const
{
	return m_inputTempFilePath;
}

const Util::C_String& C_Options::GetOutputPath() const
{
	return m_outputPath;
}

bool C_Options::ParseOutputDirectory(const Sys::C_XmlDom& dom)
{
	const Sys::C_XmlDomBaseNode* pExportFile = 
		Sys::C_XmlDomUtility::FindXmlNodeByTypeNameAttributeSubStr(dom.GetRootNodePtr(), EXECUTER_XML_FONT_CONVERTER_CONTAINER);
	if (!pExportFile)
	{
		EXECUTER_FATAL("Could not find XML node %s.\n", EXECUTER_XML_FONT_CONVERTER_CONTAINER);
		return false;
	}

	const Sys::C_XmlDomBaseNode* pSceneDirectoryNode = 
		Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(pExportFile, EXECUTER_XML_FONT_CONVERTER_OUTPUT_PATH);
	if (!pSceneDirectoryNode)
	{
		EXECUTER_FATAL("Could not find XML node %s.\n", EXECUTER_XML_FONT_CONVERTER_OUTPUT_PATH);
		return false;
	}

	Util::C_String partialDirectory =
		((Sys::C_XmlDomElement*)pSceneDirectoryNode)->GetAttributes().GetAttributeByName("value")->value;

	partialDirectory = Util::FormatWindowsFileString(partialDirectory);
	partialDirectory = Util::RemoveTrailingSlash(partialDirectory);
	partialDirectory = Util::RemoveLeadingSlash(partialDirectory);

	m_outputPath = 
		m_shimRootTempPath + 
		"\\" + 
		partialDirectory + 
		"\\";

	return true;
}

bool C_Options::ParseInputAssetFilePath(
	const Sys::C_XmlDomBaseNode* pNode,
	const Util::C_String& partialDirectory)
{
	const Sys::C_XmlDomBaseNode* pExportFile = 
		Sys::C_XmlDomUtility::FindXmlNodeByTypeNameAttributeSubStr(pNode, EXECUTER_XML_FONT_CONVERTER_CONTAINER);
	if (!pExportFile)
	{
		EXECUTER_FATAL("Could not find XML node %s.\n", EXECUTER_XML_FONT_CONVERTER_CONTAINER);
		return false;
	}

	const Sys::C_XmlDomBaseNode* pCharlistFileNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(
		pExportFile,
		EXECUTER_XML_FONT_CONVERTER_CHARLIST);
	if (!pCharlistFileNode)
	{
		EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
		return false;
	}

	const Sys::C_XmlDomBaseNode* pTextureFileNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(
		pExportFile, EXECUTER_XML_FONT_CONVERTER_TEXTURE);
	if (!pTextureFileNode)
	{
		EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
		return false;
	}

	const Sys::C_XmlAttribute* pCharlist = 
		((Sys::C_XmlDomElement*)pCharlistFileNode)->GetAttributes().GetAttributeByName("value");
	if (!pCharlist)
	{
		EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
		return false;
	}

	m_inputCharlistFile = pCharlist->value;

	const Sys::C_XmlAttribute* pTexture = 
		((Sys::C_XmlDomElement*)pTextureFileNode)->GetAttributes().GetAttributeByName("value");
	if (!pTexture)
	{
		EXECUTER_FATAL(MISSING_XML_ATTR, pNode->GetName().Chars());
		return false;
	}
	
	m_inputTextureFile = pTexture->value;

	return true;
}

const Util::C_String& C_Options::GetCharlistFilePath() const
{
	return m_inputCharlistFile;
}

const Util::C_String& C_Options::GetTextureFilePath() const
{
	return m_inputTextureFile;
}