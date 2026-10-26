#include "stdafx.h"

#include "Options.h"

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
		TCLAP::CmdLine cmd("Example A3D Exporter.", ' ', "1.00");

		//Define a value argument and add it to the command line.
		TCLAP::ValueArg<std::string> xmlFileArg(
			"","xml","XML file",true,"","Executer XML file");
		cmd.add(xmlFileArg);
		TCLAP::ValueArg<std::string> aftkFileArg(
			"","scene","Scene file",true,"","Aftk scene file");
		cmd.add(aftkFileArg);

		//Parse the args.
		cmd.parse(argc, argv);

		//Get the values
		xmlFilePath = xmlFileArg.getValue().c_str();
		m_inputTempFilePath = aftkFileArg.getValue().c_str();
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

	//Our Aftk input MUST come from our intermediate directory.
	if (m_inputTempFilePath.ISearch(m_shimRootTempPath) == -1)
		//Ah, it doesn't :(
		return false;

	const Sys::C_XmlDomBaseNode* pTriangleLimitNode =
		Sys::C_XmlDomUtility::FindXmlNodeByNameAttribute(dom.GetRootNodePtr(), "triangle_limit");
	if (!pTriangleLimitNode) return false;

	m_triangleLimit = atoi(
		((Sys::C_XmlDomElement*)(pTriangleLimitNode))->GetAttributes().GetAttributeByName("value")->value);

	//Where to put it.
	m_outputPath = Util::ExtractDirectory(m_inputTempFilePath);
	return true;
}

const Util::C_String& C_Options::GetInputTempFilePath() const
{
	return m_inputTempFilePath;
}

const Util::C_String& C_Options::GetOuptutPath() const
{
	return m_outputPath;
}

U32 C_Options::GetTriangleLimit() const
{
	return m_triangleLimit;
}
