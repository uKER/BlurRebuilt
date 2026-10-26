#include "stdafx.h"

#include <tools_shared/guid/CryptRandom.h>
#include <tools_shared/shim/ShimErrorHelpers.h>
#include <actions/Action.h>
#include <System/Misc/StringUtil.h>

#include <Helpers/XmlHelper.h>
#include <actions/AftkSceneCollectionAction.h>
#include "processes/UberCopyWrapper.h"

#include <System/Time/Time.h>

C_Command::C_Command(const Sys::C_XmlDomBaseNode& dom) : m_Dom(dom)
{
	m_Status = false;
}

const Sys::C_XmlDomBaseNode& C_Command::GetDom()
{
	return m_Dom;
}

void C_Command::SetStatus(bool status)
{
	m_Status = status;
}

bool C_Command::GetStatus() const
{
	return m_Status;
}

Util::C_String C_Command::GetFullNodePath() const
{
	Util::C_String nodePath;
	GetFullNodePath(nodePath, m_Dom);

	return nodePath;
}

Util::C_String C_Command::GetFullNodePath(const Sys::C_XmlDomBaseNode& dom)
{
	Util::C_String nodePath;
	GetFullNodePath(nodePath, dom);

	return nodePath;
}

void C_Command::GetFullNodePath(Util::C_String& nodePath, const Sys::C_XmlDomBaseNode& dom)
{
	BEGIN_CHECK_NOTNULL(((Sys::C_XmlDomElement*)&dom)->GetAttributes().GetValueByName("name")) ERROR_XML_ATTR(&dom) END_RETURN
	
	Util::C_String currentNodeName = ((Sys::C_XmlDomElement*)&dom)->GetAttributes().GetValueByName("name");
	nodePath = "/" + currentNodeName + nodePath;

	Ut::Vector<const Sys::C_XmlDomBaseNode*> typeEntry;
	Sys::C_XmlDomUtility::XpathEvaluate(&dom, "../type_entry", typeEntry);

	if (typeEntry.Size() == 1)
	{
		GetFullNodePath(nodePath, *typeEntry[0]);
	} else {
		Ut::Vector<const Sys::C_XmlDomBaseNode*> parentData;
		Sys::C_XmlDomUtility::XpathEvaluate(&dom, "*/parent_data", parentData);

		if (parentData.Size() == 1)
		{
			GetFullNodePathParentData(nodePath, *parentData[0]);
		}
	}
}

void C_Command::GetFullNodePathParentData(Util::C_String& nodePath, const Sys::C_XmlDomBaseNode& dom)
{
	Ut::Vector<const Sys::C_XmlDomBaseNode*> typeEntry;
	Sys::C_XmlDomUtility::XpathEvaluate(&dom, "*/type_entry", typeEntry);

	if (typeEntry.Size() == 1)
	{
		BEGIN_CHECK_NOTNULL(((Sys::C_XmlDomElement*)typeEntry[0])->GetAttributes().GetValueByName("name")) ERROR_XML_ATTR(typeEntry[0]) END_RETURN
		
		Util::C_String currentNodeName = ((Sys::C_XmlDomElement*)typeEntry[0])->GetAttributes().GetValueByName("name");
		nodePath = "/" + currentNodeName + nodePath;
	}

	Ut::Vector<const Sys::C_XmlDomBaseNode*> parentData;
	Sys::C_XmlDomUtility::XpathEvaluate(&dom, "*/parent_data", parentData);

	if (parentData.Size() == 1)
	{
		GetFullNodePathParentData(nodePath, *parentData[0]);
	}
}

C_Action::C_Action(const C_Options& options) : m_Options(options)
{
}

bool C_Action::Execute(const Sys::C_XmlDomBaseNode& dom)
{
	ExecuteAction(dom);

	int failCount = 0;
	for (int i=0; i<m_Commands.Size(); i++)
	{
		C_Command& command = m_Commands[i];

		ERROR_INFO("Running export (%d/%d) : %s", i+1, m_Commands.Size(), command.GetFullNodePath().Chars());

		bool status = ExecuteCommand(command);
		command.SetStatus(status);
		
		if (!status)
			failCount++;
	}

	return failCount == 0;
}

void C_Action::PrintStatus()
{
	int failCount = 0;
	for (int i=0; i<m_Commands.Size(); i++)
	{
		if (!m_Commands[i].GetStatus())
			failCount++;
	}

	ERROR_INFO("Export status:");

	if (m_Commands.Size() != failCount)
	{
		ERROR_INFO("Succeeded (%d/%d)", m_Commands.Size()-failCount, m_Commands.Size());
		for (int i=0; i<m_Commands.Size(); i++)
		{
			if (m_Commands[i].GetStatus())
				ERROR_INFO(m_Commands[i].GetFullNodePath());
		}
	}

	if (failCount)
	{
		ERROR_ERROR("Failed (%d/%d)", failCount, m_Commands.Size());
		for (int i=0; i<m_Commands.Size(); i++)
		{
			if (!m_Commands[i].GetStatus())
				ERROR_ERROR(m_Commands[i].GetFullNodePath());
		}

		ERROR_ERROR("ACTIONS FAILED");
	} else {
		ERROR_INFO("ACTIONS SUCCEEDED");
	}
}

bool C_Action::ExecuteAction(const Sys::C_XmlDomBaseNode& dom)
{
	Ut::Vector<const Sys::C_XmlDomBaseNode*> action;
	Sys::C_XmlDomUtility::XpathEvaluate(&dom, "type_entry/tagdata/shim_action", action);

	BEGIN_CHECK_CUSTOM(action.Size() != 1) ERROR_ERROR("Number of shim_actions in tagdata does not equal one") END_RETURN_FALSE

	const char* actionType = ((Sys::C_XmlDomElement*)action[0])->GetAttributes().GetValueByName("type");

	BEGIN_CHECK_NOTNULL(actionType) ERROR_ERROR("Unknown action type (Unknown)") END_RETURN_FALSE

	return ExecuteAction(dom, actionType);
}

bool C_Action::ExecuteAction(const Sys::C_XmlDomBaseNode& dom, Btl::String action)
{
	if (action == "batch")
	{
		return ExecuteBatch(dom);
	}
	else if (action == "command")
	{
		C_Command command(dom);
		m_Commands.PushBack(command);
		return true;
	}

	BEGIN_ALWAYS ERROR_ERROR("Unknown action type (%s)", action.Chars()) END_RETURN_FALSE
}

bool C_Action::ExecuteBatch(const Sys::C_XmlDomBaseNode& dom)
{
	Ut::Vector<const Sys::C_XmlDomBaseNode*> typeEntries;
	Sys::C_XmlDomUtility::XpathEvaluate(&dom, "type_entry/type_entry", typeEntries);

	for (int i=0; i<typeEntries.Size(); i++)
	{
		BEGIN_CHECK_CUSTOM(!ExecuteAction(*typeEntries[i])) ERROR_ERROR("Unable to execute action") END_RETURN_FALSE
	}

	return true;
}

bool C_Action::GenerateTempXmlFilename(Util::C_String& tempFilename)
{
	Tools::C_CryptRandom cr;
	if (!cr.Initialise())
	{
		assert(!"Could not initialise CryptRandom!");
		return false;
	}

	Time::C_Time currentTime = Time::GetCurrentTime();
	Time::C_DecomposedTime decomp(currentTime);
	
	for (int i=0; i<128; i++)
	{
		Util::C_String xmlFilepath = m_Options.GetXmlFilePath().Replace("/", "\\");
		S32 lastSlash = xmlFilepath.RevSearch('\\');
		BEGIN_CHECK_CUSTOM(lastSlash<0) ERROR_ERROR("Invalid xml filename (%s)", m_Options.GetXmlFilePath().Chars()) END_RETURN_FALSE;
		Util::C_String newDirectory = Util::C_String::Format("%s\\%d-%d-%d\\", xmlFilepath(0, lastSlash).Chars(), 1900+decomp.m_year, decomp.m_mon+1, decomp.m_mday);
		Util::C_String newFilename = Util::C_String::Format("%sNs_%x.%03x", newDirectory.Chars(), (U32)cr(), ((U32)cr())%4096);
		BEGIN_CHECK_CUSTOM(!Sys::GetDefaultFileSystem()->CreateDirectory(newDirectory)) ERROR_ERROR("Can't create directory (%s)", newDirectory.Chars()) END_RETURN_FALSE;
		HANDLE newFileHandle = CreateFile(newFilename.Chars(), GENERIC_READ|GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
		if (newFileHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(newFileHandle);
			tempFilename = newFilename;
			return true;
		}
	}	

	return false;
}

bool C_Action::GenerateTempXml(const Sys::C_XmlDomBaseNode& dom, Util::C_String& xmlLocation, Util::C_String& newNodeName, const Sys::C_XmlDomElement* additionalData)
{
	// Generate a random filename, and copy out the current node to it, copying all the global action data to the root

	Util::C_String newFilename;

	BEGIN_CHECK_CUSTOM (!GenerateTempXmlFilename(newFilename)) ERROR_ERROR("Unable to find unique xml filename!") END_RETURN_FALSE

	Sys::C_XmlDom* tempXml = new Sys::C_XmlDom();
	Sys::C_XmlDomElement* element = new Sys::C_XmlDomElement((Sys::C_XmlDomElement&)(dom));
	
	Sys::C_XmlDomBaseNode::Array children = m_Options.GetActionNode()->GetChildren();
	for (int i=0; i<children.Size(); i++)
	{
		Sys::C_XmlDomElement* actionElement = new Sys::C_XmlDomElement(*(Sys::C_XmlDomElement*)children[i]);
		element->AddChild(actionElement);
	}

	if (additionalData)
		element->AddChild(new Sys::C_XmlDomElement(*additionalData));

	Util::C_String pathHash;
	pathHash = pathHash.Format("%x", Btl::HashKeyOpString::GetHash(C_Command::GetFullNodePath(dom)));
	if (!CheckPathHash(C_Command::GetFullNodePath(dom), pathHash))
	{
		delete tempXml;
		return false;
	}

	newNodeName = element->GetAttributes().GetAttributeByName("name")->value;
	newNodeName = newNodeName + "_" + pathHash;
	element->GetAttributes().GetAttributeByName("name")->value = newNodeName;

	tempXml->SetRootNodePtr(element);
	
	if (!tempXml->Write(newFilename))
	{
		ERROR_ERROR("Failed to write data to xml file (%s)!", newFilename.Chars());
		ERROR_INFO("Root node (%s)", tempXml->GetRootNodePtr()->GetName().Chars());
		for (Sys::C_XmlDomBaseNode::Array::ConstIterator it = tempXml->GetRootNodePtr()->GetChildren().Begin(); it != tempXml->GetRootNodePtr()->GetChildren().End(); ++it)
		{
			ERROR_INFO("Child node: %s", (*it)->GetName().Chars());
		}

		delete tempXml;
		
		return false;
	}

	delete tempXml;

	xmlLocation = Util::ConvertLocalPathToABNetworkPath(newFilename, m_Options.GetLaunchComputerName());

	return true;
}

bool C_Action::CheckPathHash(const Util::C_String& path, const Util::C_String& hash)
{
	if (m_Hashes.Find(hash) != m_Hashes.End())
	{
		if (m_Hashes[hash] != path)
		{
			ERROR_ERROR("Hash collision the two paths %s and %s, please report this to the tools team", m_Hashes[hash].Chars(), path.Chars());
			return false;
		}
	} else {
		m_Hashes.Insert(hash, path);
	}
	return true;
}

bool C_Action::ExecuteCommand(C_Command& command)
{
	Ut::Vector<Util::C_String> assets;

	const Sys::C_XmlDomBaseNode& dom = command.GetDom();

	// Process all assets *before* running the command
	BEGIN_CHECK_CUSTOM(!C_Action::ExecuteAssetProcess(dom, assets)) ERROR_ERROR("Asset processing failed") END_RETURN_FALSE

	Ut::Vector<const Sys::C_XmlDomBaseNode*> commandDef;
	Sys::C_XmlDomUtility::XpathEvaluate(&dom, "type_entry/tagdata/shim_action/command", commandDef);

	Ut::Vector<const Sys::C_XmlDomBaseNode*> arguments;
	Sys::C_XmlDomUtility::XpathEvaluate(&dom, "type_entry/tagdata/shim_action/arguments", arguments);

	BEGIN_CHECK_CUSTOM(commandDef.Size() != 1) ERROR_ERROR("Number of command definitions in shim_action does not equal one") END_RETURN_FALSE
	BEGIN_CHECK_CUSTOM(arguments.Size() != 1) ERROR_ERROR("Number of argument definitions in shim_action does not equal one") END_RETURN_FALSE

	const char* commandAttribute = ((Sys::C_XmlDomElement*)commandDef[0])->GetAttributes().GetValueByName("value");
	const char* argumentsAttribute = ((Sys::C_XmlDomElement*)arguments[0])->GetAttributes().GetValueByName("value");

	BEGIN_CHECK_NOTNULL(commandAttribute) ERROR_ERROR("Command has no value") END_RETURN_FALSE
	BEGIN_CHECK_NOTNULL(argumentsAttribute) ERROR_ERROR("Arguments has no value") END_RETURN_FALSE

	Util::C_String commandValue = commandAttribute;
	Tools::C_MacroExpander::Instance().Expand(commandValue);

	Util::C_String argumentsValue = argumentsAttribute;
	Tools::C_MacroExpander::Instance().Expand(argumentsValue);

	for (int i=0; i<assets.Size(); i++)
	{
		argumentsValue += " ";
		argumentsValue = argumentsValue + assets[i];
	}

	ERROR_INFO("Command: %s", commandValue.Chars())
	ERROR_INFO("Arguments: %s", argumentsValue.Chars())

	Util::C_String newFilename;
	Util::C_String newNodeName;
	BEGIN_CHECK_CUSTOM(!GenerateTempXml(dom, newFilename, newNodeName)) ERROR_ERROR("Unable to generate temp XML") END_RETURN_FALSE;

	argumentsValue = argumentsValue.Replace("%args_xml%", newFilename);
	Util::C_String commandFilePath = ExpandToolPath(m_Options, commandValue);
	Util::C_String fileName = Util::ExtractFilename(commandFilePath);
	Util::C_String workingFolder = Util::ExtractDirectory(commandFilePath);

	ERROR_INFO("Launching %s\n", fileName.Chars());
	ERROR_DEBUG("NetworkShim calling command(%s), arguments(%s), working Folder(%s)", commandFilePath.Chars(), argumentsValue.Chars(), workingFolder.Chars());

	S32 exitCode = Sys::ShellExec::ExecuteCommandAndWait(commandFilePath, argumentsValue, workingFolder, m_Options.GetShellExecLaunchState(), &Tools::g_pMyApplication->GetLogger());

	BEGIN_CHECK_CUSTOM(exitCode != 0) ERROR_ERROR("NetworkShim command (%s %s) returned a failure code. (%d)", commandFilePath.Chars(), argumentsValue.Chars(), exitCode) END_RETURN_FALSE

	ERROR_DEBUG("NetworkShim command returned success.");

	Ut::Vector<const Sys::C_XmlDomBaseNode*> postExport;
	Sys::C_XmlDomUtility::XpathEvaluate(m_Options.GetActionNode(), "actiondata/shim_mode/postexport", postExport);
	BEGIN_CHECK_CUSTOM(postExport.Size() != 1) ERROR_ERROR("Number of post export definitions does not equal one") END_RETURN_FALSE
	BEGIN_CHECK_NOTNULL(((Sys::C_XmlDomElement*)postExport[0])->GetAttributes().GetValueByName("type")) ERROR_XML_ATTR(postExport[0]) END_RETURN_FALSE
	Util::C_String type = ((Sys::C_XmlDomElement*)postExport[0])->GetAttributes().GetValueByName("type");
	BEGIN_CHECK_CUSTOM(type != "ubercopy") ERROR_ERROR("Post export type is (%s), only ubercopy is currently supported", type.Chars()) END_RETURN_FALSE

	BEGIN_CHECK_NOTNULL(((Sys::C_XmlDomElement*)postExport[0])->GetAttributes().GetValueByName("frequency")) ERROR_XML_ATTR(postExport[0]) END_RETURN_FALSE
	Util::C_String frequency = ((Sys::C_XmlDomElement*)postExport[0])->GetAttributes().GetValueByName("frequency");

	BEGIN_CHECK_NOTNULL(((Sys::C_XmlDomElement*)&dom)->GetAttributes().GetValueByName("name")) ERROR_XML_ATTR(&dom) END_RETURN_FALSE
	BEGIN_CHECK_NOTNULL(((Sys::C_XmlDomElement*)&dom)->GetAttributes().GetValueByName("type_name")) ERROR_XML_ATTR(&dom) END_RETURN_FALSE
	Util::C_String name = ((Sys::C_XmlDomElement*)&dom)->GetAttributes().GetValueByName("name");
	Util::C_String typeName = ((Sys::C_XmlDomElement*)&dom)->GetAttributes().GetValueByName("type_name");

	Util::C_String taskName = Util::C_String::Format("%s_%x", newNodeName.Chars(), Ut::HashKeyOpString::GetHash(typeName.Chars()));

	if (frequency == "pernode")
		BEGIN_CHECK_CUSTOM(!UberCopyWrapper::PostExportCopy(m_Options, taskName)) ERROR_ERROR("Ubercopy failed") END_RETURN_FALSE
	else
		BEGIN_CHECK_CUSTOM(!UberCopyWrapper::AddTask(taskName)) ERROR_ERROR("Duplicate node name/type detected (%s of type %s), cannot continue processing", name.Chars(), typeName.Chars()) END_RETURN_FALSE

	return true;
}

bool C_Action::ExecuteAssetProcess(const Sys::C_XmlDomBaseNode& dom, Ut::Vector<Util::C_String>& assets)
{
	Ut::Vector<const Sys::C_XmlDomBaseNode*> asset;
	Sys::C_XmlDomUtility::XpathEvaluate(&dom, "type_entry/tagdata/shim_action/asset", asset);

	if (asset.Size() < 1)
		return true;

	for (int i=0; i<asset.Size(); i++)
	{
		const char* assetAttribute = ((Sys::C_XmlDomElement*)asset[i])->GetAttributes().GetValueByName("type");

		BEGIN_CHECK_NOTNULL(assetAttribute) ERROR_ERROR("Asset has no type") END_RETURN_FALSE

		if (stricmp(assetAttribute, "aftk") == 0)
		{
			BEGIN_CHECK_CUSTOM(!ExecuteAftkConversion(dom, *asset[i], assets)) ERROR_ERROR("Failure during aftk asset processing") END_RETURN_FALSE;
		}
		else if (stricmp(assetAttribute, "command") == 0)
		{
			BEGIN_CHECK_CUSTOM(!ExecuteCommandConversion(dom, *asset[i], assets)) ERROR_ERROR("Failure during command asset processing") END_RETURN_FALSE;
		}
		else
			BEGIN_ALWAYS ERROR_ERROR("Unknown asset conversion type (%s)", assetAttribute) END_RETURN_FALSE
	}

	return true;
}

bool C_Action::GenerateParentData(const Sys::C_XmlDomBaseNode& dom, Sys::C_XmlDomBaseNode* node)
{
	Ut::Vector<const Sys::C_XmlDomBaseNode*> typeEntry;
	Sys::C_XmlDomUtility::XpathEvaluate(&dom, "type_entry", typeEntry);
	if (typeEntry.Size() == 1)
	{
		Sys::C_XmlDomElement* typeEntryElement = new Sys::C_XmlDomElement();
		typeEntryElement->GetName() = "type_entry";
		typeEntryElement->GetAttributes() = ((Sys::C_XmlDomElement*)typeEntry[0])->GetAttributes();

		Ut::Vector<const Sys::C_XmlDomBaseNode*> parentValues;
		Sys::C_XmlDomUtility::XpathEvaluate(&dom, "type_entry/values", parentValues);
		BEGIN_CHECK_CUSTOM(parentValues.Size() != 1) ERROR_ERROR("Number of values elements in parent of node does not equal one") END_RETURN_FALSE

		typeEntryElement->AddChild(new Sys::C_XmlDomElement(*(Sys::C_XmlDomElement*)parentValues[0]));

		const Sys::C_XmlDomBaseNode* parent = dom.GetParent();
		if (parent)
		{
			Sys::C_XmlDomElement* parentElement = new Sys::C_XmlDomElement();
			parentElement->GetName() = "parent_data";

			if (!GenerateParentData(*parent, parentElement))
				return false;

			typeEntryElement->AddChild(parentElement);
		}

		node->AddChild(typeEntryElement);

		return true;
	}
	else
		return true;
}

bool C_Action::ProcessCommandAsset(const Sys::C_XmlDomBaseNode& dom, const Util::C_String& command, const Util::C_String& arguments)
{
	BEGIN_CHECK_NOTNULL(((Sys::C_XmlDomElement*)&dom)->GetAttributes().GetValueByName("name")) ERROR_XML_ATTR(&dom) END_RETURN_FALSE
	BEGIN_CHECK_NOTNULL(((Sys::C_XmlDomElement*)&dom)->GetAttributes().GetValueByName("type_name")) ERROR_XML_ATTR(&dom) END_RETURN_FALSE
	Util::C_String name = ((Sys::C_XmlDomElement*)&dom)->GetAttributes().GetValueByName("name");	
	Util::C_String typeName = ((Sys::C_XmlDomElement*)&dom)->GetAttributes().GetValueByName("type_name");

	Util::C_String commandValue = command;
	Tools::C_MacroExpander::Instance().Expand(commandValue);

	Util::C_String argumentsValue = arguments;
	Tools::C_MacroExpander::Instance().Expand(argumentsValue);

	Sys::C_XmlDomElement* tempData = new Sys::C_XmlDomElement();
	tempData->GetName() = "parent_data";
	GenerateParentData(*dom.GetParent(), tempData);

	Util::C_String newFilename;
	Util::C_String newNodeName;
	BEGIN_CHECK_CUSTOM(!GenerateTempXml(dom, newFilename, newNodeName, tempData)) ERROR_ERROR("Unable to generate temp XML") END_RETURN_FALSE;

	delete tempData;

	argumentsValue = argumentsValue.Replace("%args_xml%", newFilename);
	Util::C_String commandFilePath = ExpandToolPath(m_Options, commandValue);
	Util::C_String fileName = Util::ExtractFilename(commandFilePath);
	Util::C_String workingFolder = Util::ExtractDirectory(commandFilePath);

	ERROR_INFO("Launching %s\n", fileName.Chars());
	ERROR_DEBUG("NetworkShim calling command(%s), arguments(%s), working Folder(%s)", commandFilePath.Chars(), argumentsValue.Chars(), workingFolder.Chars());

	S32 exitCode = Sys::ShellExec::ExecuteCommandAndWait(commandFilePath, argumentsValue, workingFolder, m_Options.GetShellExecLaunchState(), &Tools::g_pMyApplication->GetLogger());

	BEGIN_CHECK_CUSTOM(exitCode != 0) ERROR_ERROR("NetworkShim command (%s %s) returned a failure code. (%d)", commandFilePath.Chars(), argumentsValue.Chars(), exitCode) END_RETURN_FALSE

	Util::C_String taskName = Util::C_String::Format("%s_%x", newNodeName.Chars(), Ut::HashKeyOpString::GetHash(typeName.Chars()));
	BEGIN_CHECK_CUSTOM(!UberCopyWrapper::PostExportCopy(m_Options, taskName)) ERROR_ERROR("Ubercopy failed") END_RETURN_FALSE

	return true;
}

bool C_Action::ExecuteCommandConversion(const Sys::C_XmlDomBaseNode& dom, const Sys::C_XmlDomBaseNode& assetTag, Ut::Vector<Util::C_String>& assets)
{
	Ut::Vector<const Sys::C_XmlDomBaseNode*> command;
	Sys::C_XmlDomUtility::XpathEvaluate(&assetTag, "asset/command", command);
	BEGIN_CHECK_CUSTOM(command.Size() != 1) ERROR_ERROR("Number of command definitions in shim_action/asset does not equal one") END_RETURN_FALSE

	Ut::Vector<const Sys::C_XmlDomBaseNode*> arguments;
	Sys::C_XmlDomUtility::XpathEvaluate(&assetTag, "asset/arguments", arguments);
	BEGIN_CHECK_CUSTOM(arguments.Size() != 1) ERROR_ERROR("Number of argument definitions in shim_action/asset does not equal one") END_RETURN_FALSE

	BEGIN_CHECK_NOTNULL(((Sys::C_XmlDomElement*)&assetTag)->GetAttributes().GetValueByName("path")) ERROR_ERROR("Asset processing definition has no path attribute") END_RETURN_FALSE
	BEGIN_CHECK_NOTNULL(((Sys::C_XmlDomElement*)command[0])->GetAttributes().GetValueByName("value")) ERROR_ERROR("Command definition has no value attribute") END_RETURN_FALSE
	BEGIN_CHECK_NOTNULL(((Sys::C_XmlDomElement*)arguments[0])->GetAttributes().GetValueByName("value")) ERROR_ERROR("Argument definition has no value attribute") END_RETURN_FALSE
	
	Util::C_String assetPath = ((Sys::C_XmlDomElement*)&assetTag)->GetAttributes().GetValueByName("path");
	Util::C_String commandValue = ((Sys::C_XmlDomElement*)command[0])->GetAttributes().GetValueByName("value");
	Util::C_String argumentsValue = ((Sys::C_XmlDomElement*)arguments[0])->GetAttributes().GetValueByName("value");

	Ut::Vector<const Sys::C_XmlDomBaseNode*> assetNodes;
	Sys::C_XmlDomUtility::XpathEvaluate(&dom, assetPath, assetNodes);

	for (int i=0; i<assetNodes.Size(); i++)
	{
		BEGIN_CHECK_CUSTOM(!ProcessCommandAsset(*assetNodes[i], commandValue, argumentsValue)) ERROR_ERROR("Asset pre-processing failed") END_RETURN_FALSE
	}

	return true;
}

bool C_Action::ExecuteAftkConversion(const Sys::C_XmlDomBaseNode& dom, const Sys::C_XmlDomBaseNode& assetTag, Ut::Vector<Util::C_String>& assets)
{
	Ut::Vector<const Sys::C_XmlDomBaseNode*> process;
	Sys::C_XmlDomUtility::XpathEvaluate(&assetTag, "asset/process", process);

	BEGIN_CHECK_CUSTOM(process.Size() != 1) ERROR_ERROR("Number of process definitions in shim_action/asset does not equal one") END_RETURN_FALSE

	const char* processAttribute = ((Sys::C_XmlDomElement*)process[0])->GetAttributes().GetValueByName("type");

	BEGIN_CHECK_NOTNULL(processAttribute) ERROR_ERROR("Process has no type") END_RETURN_FALSE

	Util::C_String processType = processAttribute;

	E_ShimActionTag actionTag;
	
	if (processType == "combine")
		actionTag = E_ShimActionTag_AftkSceneCombine;
	else if (processType == "multiple")
		actionTag = E_ShimActionTag_AftkSceneMultiple;
	else if (processType == "single")
		actionTag = E_ShimActionTag_AftkSceneSingle;
	else
	{
		BEGIN_ALWAYS ERROR_ERROR("Unknown aftk conversion type: %s", processAttribute) END_RETURN_FALSE;
	}

	return AftkSceneCollectionAction::Process(m_Options, dom, actionTag, assets);
}