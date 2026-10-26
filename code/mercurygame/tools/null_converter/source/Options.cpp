#include "stdafx.h"
#include "Options.h"

#define EXECUTER_XML_NULL_CONVERTER_FILE			"export_null_converter_file"
#define EXECUTER_XML_NULL_CONVERTER_FOLDER			"export_null_converter_folder"
#define EXECUTER_XML_NULL_CONVERTER_FILENAME		"export_null_converter_filename"
#define EXECUTER_XML_NULL_CONVERTER_OUTPUT_PATH		"export_null_converter_file_output_path"

#define MISSING_XML_ATTR "Could not find required XML attribute in %s.\n"


bool C_Options::ParseCommandLine(
	Sys::C_FilePath& xmlFilePath,
	int argc, 
	char* argv[])
{
	// Wrap everything in a try block.  Do this every time, 
	// because exceptions will be thrown for problems. 
	try
	{  
		//Define the command line object.
		TCLAP::CmdLine cmd("Null Exporter.", ' ', "1.00");

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

bool C_Options::ParseXml(const Sys::C_FilePath& xmlFilePath, const Sys::C_XmlDom& dom)
{
	memset(exportForPlatform, 1, sizeof(exportForPlatform));

	if (!Tools::Shim::C_Options::ParseXml(xmlFilePath.Chars(), dom))
		return false;

	copyType = E_CopyType_Unknown;
	const Sys::C_XmlDomBaseNode* pExportType = 0;
	 
	{
		const Sys::C_XmlDomBaseNode* pNode = Sys::C_XmlDomUtility::FindXmlNodeByTypeNameAttributeSubStr(dom.GetRootNodePtr(), EXECUTER_XML_NULL_CONVERTER_FILE);
		if (pNode)
		{
			copyType = E_CopyType_File;
			pExportType = pNode;
		}
	}
	{
		const Sys::C_XmlDomBaseNode* pNode = Sys::C_XmlDomUtility::FindXmlNodeByTypeNameAttributeSubStr(dom.GetRootNodePtr(), EXECUTER_XML_NULL_CONVERTER_FOLDER);
		if (pNode)
		{
			copyType = E_CopyType_Folder;
			pExportType = pNode;
		}
	}
	if (copyType == E_CopyType_Unknown)
	{
		return false;
	}

	//Where to put it.
	Sys::C_FilePath outputDirectory;

	if (!ParseOutputDirectory(pExportType))
		return false;

	if (!ParseInputAssetFilePath(pExportType, outputDirectory))
		return false;

	inputTempFilePath = inputAssetFilePath.IReplace(m_shimRootAssetPath, m_shimRootTempPath);
	if (inputTempFilePath == "")
		return false;

	return true;
}

bool C_Options::ParseOutputDirectory(const Sys::C_XmlDomBaseNode* pExportType)
{
	const Sys::C_XmlDomBaseNode* pSceneDirectoryNode = 
		Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(pExportType, EXECUTER_XML_NULL_CONVERTER_OUTPUT_PATH);
	if (!pSceneDirectoryNode)
	{
		EXECUTER_FATAL("Could not find XML node %s.\n", EXECUTER_XML_NULL_CONVERTER_OUTPUT_PATH);
		return false;
	}

	{
		const Sys::C_XmlDomBaseNode* pExport = 
			Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(pExportType, "export_for_360");
		if (pExport)
		{
			const Sys::C_XmlAttribute* pEnabled = 
				((Sys::C_XmlDomElement*)pExport)->GetAttributes().GetAttributeByName("value");
			if (!pEnabled || pEnabled->value == "false")
				exportForPlatform[Util::E_Platform_360] = false;
		}
	}
	{
		const Sys::C_XmlDomBaseNode* pExport = 
			Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(pExportType, "export_for_pc");
		if (pExport)
		{
			const Sys::C_XmlAttribute* pEnabled = 
				((Sys::C_XmlDomElement*)pExport)->GetAttributes().GetAttributeByName("value");
			if (!pEnabled || pEnabled->value == "false")
				exportForPlatform[Util::E_Platform_PC] = false;
		}
	}
	{
		const Sys::C_XmlDomBaseNode* pExport = 
			Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(pExportType, "export_for_ps3");
		if (pExport)
		{
			const Sys::C_XmlAttribute* pEnabled = 
				((Sys::C_XmlDomElement*)pExport)->GetAttributes().GetAttributeByName("value");
			if (!pEnabled || pEnabled->value == "false")
				exportForPlatform[Util::E_Platform_PS3] = false;
		}
	}

	Sys::C_FilePath partialDirectory =
		((Sys::C_XmlDomElement*)pSceneDirectoryNode)->GetAttributes().GetAttributeByName("value")->value;

	partialDirectory = Sys::RemoveTrailingSlash(partialDirectory);
	partialDirectory = Sys::RemoveLeadingSlash(partialDirectory);

	outputPath = partialDirectory;

	return true;
}

bool C_Options::ParseInputAssetFilePath(
	const Sys::C_XmlDomBaseNode* pExportType,
	const Sys::C_FilePath& partialDirectory)
{
	{
		const Sys::C_XmlDomBaseNode* pFileNameNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(
			pExportType, EXECUTER_XML_NULL_CONVERTER_FILENAME);
		if (!pFileNameNode)
		{
			EXECUTER_FATAL(MISSING_XML_ATTR, pFileNameNode->GetName().Chars());
			return false;
		}

		const Sys::C_XmlAttribute* pFileName = 
			((Sys::C_XmlDomElement*)pFileNameNode)->GetAttributes().GetAttributeByName("value");
		if (!pFileName)
		{
			EXECUTER_FATAL(MISSING_XML_ATTR, pFileNameNode->GetName().Chars());
			return false;
		}
		inputAssetFilePath = pFileName->value;
	}
	{
		const Sys::C_XmlDomBaseNode* pFileRootNode = Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(
			pExportType, "export_null_converter_file_root");
		if (pFileRootNode)
		{
			const Sys::C_XmlAttribute* pFileRoot = 
				((Sys::C_XmlDomElement*)pFileRootNode)->GetAttributes().GetAttributeByName("value");
			if (pFileRoot)
				inputAssetRootPath = pFileRoot->value;
		}

		//Default if not set.
		if (inputAssetRootPath.Length() == 0)
			inputAssetRootPath = inputAssetFilePath;
	}


	return true;
}
