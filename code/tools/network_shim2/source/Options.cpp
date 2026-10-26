#include "stdafx.h"
#include "options.h"
#include <tools_shared/shim/ShimErrorHelpers.h>

#include <Helpers/XmlHelper.h>
#include <tools_shared/utility/misc.h>

#define SHIMXML_CONFIG_ALIENBRAIN_CONFIG							"shim_ab_config"
#define SHIMXML_CONFIG_ALIENBRAIN_MAPPING							"shim_ab_mapping"
#define SHIMXML_CONFIG_ALIENBRAIN_PROJECT							"shim_ab_project"
#define SHIMXML_CONFIG_ALIENBRAIN_USER								"shim_ab_user"
#define SHIMXML_CONFIG_ALIENBRAIN_PASS								"shim_ab_pass"
#define SHIMXML_CONFIG_ALIENBRAIN_SCRIPT							"shim_ab_script"


bool C_Options::ParseXml(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom)
{
	if (!Tools::Shim::C_Options::ParseXml(xmlFilePath, dom))
		return false;

	Ut::Vector<const Sys::C_XmlDomBaseNode*> actionData;
	Sys::C_XmlDomUtility::XpathEvaluate(dom.GetRootNodePtr(), "*/type_entry/actiondata", actionData);
	BEGIN_CHECK_CUSTOM(actionData.Size() != 1) ERROR_ERROR("Number of action data definitions does not equal one") END_RETURN_FALSE
	m_pShimActionNode = (Sys::C_XmlDomElement*)actionData[0];

	Ut::Vector<const Sys::C_XmlDomBaseNode*> shimMode;
	Sys::C_XmlDomUtility::XpathEvaluate(dom.GetRootNodePtr(), "*/type_entry/actiondata/shim_mode", shimMode);
	BEGIN_CHECK_CUSTOM(shimMode.Size() != 1) ERROR_ERROR("Number of shim mode definitions does not equal one") END_RETURN_FALSE
	const char* mode = ((Sys::C_XmlDomElement*)shimMode[0])->GetAttributes().GetValueByName("value");
	BEGIN_CHECK_NOTNULL(mode) ERROR_XML_ATTR(shimMode[0]) END_RETURN_FALSE

	if (stricmp(mode, "action") == 0)
		m_shimMode = E_ShimMode_Action;
	else if (stricmp(mode, "ubercopy") == 0)
		m_shimMode = E_ShimMode_Ubercopy;
	else if (stricmp(mode, "scm") == 0)
		m_shimMode = E_ShimMode_Scm;
	else
	{
		BEGIN_ALWAYS ERROR_ERROR("Unknown shim mode (%s)", mode) END_RETURN_FALSE
	}

	m_shimShellExecLaunchState = Sys::ShellExec::E_LaunchState_Minimise;

	// If we're using source control, obtain all the alienbrain configs
	if ((m_shimExportInfo & Tools::Shim::E_ExportInfo_DataSourceSCM) == Tools::Shim::E_ExportInfo_DataSourceSCM || m_shimMode == E_ShimMode_Scm)
	{
		const Sys::C_XmlDomBaseNode* pAlienbrainConfig = Sys::C_XmlDomUtility::FindXmlNodeByName(dom.GetRootNodePtr(), SHIMXML_CONFIG_ALIENBRAIN_CONFIG);
		if (!pAlienbrainConfig)
		{
			LOGFATAL( Tools::g_log, "Failed to find alienbrain config in XML.\n");
			return false;
		}

		const Sys::C_XmlDomBaseNode* pAlienbrainScript = Sys::C_XmlDomUtility::FindXmlNodeByName(pAlienbrainConfig, SHIMXML_CONFIG_ALIENBRAIN_SCRIPT);
		if (!pAlienbrainScript)
		{
			LOGFATAL( Tools::g_log, "Failed to find alienbrain script in XML.\n");
			return false;
		}

		m_shimAlienbrainScript = ((Sys::C_XmlDomElement*)(pAlienbrainScript))->GetAttributes().GetAttributeByName("value")->value;

		const Sys::C_XmlDomBaseNode* pAlienbrainUser = Sys::C_XmlDomUtility::FindXmlNodeByName(pAlienbrainConfig, SHIMXML_CONFIG_ALIENBRAIN_USER);
		if (!pAlienbrainUser)
		{
			LOGFATAL( Tools::g_log, "Failed to find the default alienbrain user in XML.\n");
			return false;
		}

		m_shimAlienbrainDefaultUser = ((Sys::C_XmlDomElement*)(pAlienbrainUser))->GetAttributes().GetAttributeByName("name")->value;

		const Sys::C_XmlDomBaseNode* pAlienbrainPass = Sys::C_XmlDomUtility::FindXmlNodeByName(pAlienbrainConfig, SHIMXML_CONFIG_ALIENBRAIN_PASS);
		if (!pAlienbrainPass)
		{
			LOGFATAL( Tools::g_log, "Failed to find the default alienbrain password in XML.\n");
			return false;
		}

		m_shimAlienbrainDefaultPass = ((Sys::C_XmlDomElement*)(pAlienbrainPass))->GetAttributes().GetAttributeByName("name")->value;

		for (Sys::C_XmlDomBaseNode::Array::ConstIterator child = pAlienbrainConfig->GetChildren().Begin(); child != pAlienbrainConfig->GetChildren().End(); ++child)
		{
			Util::C_String mapping = (*child)->GetName().Lower();
			if ((*child)->GetName().Lower() == "shim_ab_mapping")
			{
				m_shimAlienbrainMappings.PushBack(Ut::MakePair(((Sys::C_XmlDomElement*)(*child))->GetAttributes().GetAttributeByName("local")->value.Replace("\\", "/").Lower(), ((Sys::C_XmlDomElement*)(*child))->GetAttributes().GetAttributeByName("server")->value));
			}
		}
	}

	return true;
}

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
		TCLAP::CmdLine cmd("Network Shim.", ' ', "2.00");

		//Define a value argument and add it to the command line.
		TCLAP::ValueArg<std::string> shimFlagsArg(
			"","flags","Shim flags",false,"","Shim flags");
		TCLAP::ValueArg<std::string> xmlFileArg(
			"","xml","XML file",true,"","Executer XML file");
		cmd.add(shimFlagsArg);
		cmd.add(xmlFileArg);

		//Parse the args.
		cmd.parse(argc, argv);

		//Get the values
		xmlFilePath = xmlFileArg.getValue().c_str();
		m_shimFlags = shimFlagsArg.getValue().c_str();
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

	Tools::C_MacroExpander::Instance().Expand(xmlFilePath);
	Tools::C_MacroExpander::Instance().Expand(m_shimFlags);

	Tools::C_MacroExpander::Instance().Insert("xml_args", xmlFilePath);

	m_shimPlatformName = Tools::GetPlatformString();

	return true;
}

Sys::ShellExec::E_LaunchState C_Options::GetShellExecLaunchState() const
{
	return m_shimShellExecLaunchState;
}

E_ShimMode C_Options::GetMode() const
{
	return m_shimMode;
}

const Sys::C_XmlDomElement* C_Options::GetActionNode() const
{
	return m_pShimActionNode;
}

const Util::C_String& C_Options::GetPlatformName() const
{
	return m_shimPlatformName;
}

const Util::C_String& C_Options::GetFlags() const
{
	return m_shimFlags;
}

const Util::C_String& C_Options::GetAlienbrainScript() const
{
	return m_shimAlienbrainScript;
}

const Util::C_String* C_Options::GetAlienbrainServer(const Util::C_String& folder) const
{
	const Util::C_String* server = 0;

	int maxMatch = 0;
	for (Ut::Vector<Ut::Pair<Util::C_String, Util::C_String> >::ConstIterator mapping = m_shimAlienbrainMappings.Begin(); mapping != m_shimAlienbrainMappings.End(); ++mapping)
	{
		if (mapping->first.Length() > maxMatch && folder.Lower().Search(mapping->first)==0)
		{
			maxMatch = mapping->first.Length();
			server = &mapping->second;
		}
		
	}

	return server;
}

const Util::C_String& C_Options::GetAlienbrainUser(const Util::C_String& server) const
{
	return m_shimAlienbrainDefaultUser;
}

const Util::C_String& C_Options::GetAlienbrainPass(const Util::C_String& server) const
{
	return m_shimAlienbrainDefaultPass;
}
