#include "stdafx.h"
#include <tools_shared/shim/ShimErrorHelpers.h>
#include <actions/action.h>
#include <actions/UberCopyAction.h>

#include <Helpers/XmlHelper.h>
#include <assets/AftkSceneCollection.h>

#include <processes/UberCopyWrapper.h>

#define SHIMXML_ACTION_CONFIG_COPY_TO_TARGET						"shim_copy_to_target"

bool UberCopyAction::Execute(const C_Options& options, const Sys::C_XmlDom& dom)
{	
	ERROR_INFO("Launching Ubercopy action\n");

	Util::C_String actionName;
	const Sys::C_XmlDomElement* pAction = options.GetActionNode();
	
	Ut::Vector<const Sys::C_XmlDomBaseNode*> copyType;
	Sys::C_XmlDomUtility::XpathEvaluate(pAction, "actiondata/shim_mode/copytype", copyType);
	BEGIN_CHECK_CUSTOM(copyType.Size() != 1) ERROR_ERROR("Number of copy type definitions does not equal one") END_RETURN_FALSE

	const char* type = ((Sys::C_XmlDomElement*)copyType[0])->GetAttributes().GetValueByName("value");
	BEGIN_CHECK_NOTNULL(type) ERROR_XML_ATTR(copyType[0]) END_RETURN_FALSE

	BEGIN_CHECK_CUSTOM(stricmp(type, SHIMXML_ACTION_CONFIG_COPY_TO_TARGET)!=0) ERROR_ERROR("Unknown copy type (%s)", type) END_RETURN_FALSE

	BEGIN_CHECK_NOTNULL(((Sys::C_XmlDomElement*)copyType[0])->GetAttributes().GetValueByName("target")) ERROR_XML_ATTR(copyType[0]) END_RETURN_FALSE
	Util::C_String target = ((Sys::C_XmlDomElement*)copyType[0])->GetAttributes().GetValueByName("target");

	// Calculate ubercopy filename
	Sys::C_XmlDomElement* pTypeEntry = (Sys::C_XmlDomElement*)dom.GetRootNodePtr()->GetChildren()[0];
	Util::C_String nodeName = Util::C_String::Format("%s_%x_%x", pTypeEntry->GetAttributes().GetAttributeByName("name")->value.Chars(), Btl::HashKeyOpString::GetHash(C_Command::GetFullNodePath(*pTypeEntry)), Ut::HashKeyOpString::GetHash(pTypeEntry->GetAttributes().GetAttributeByName("type_name")->value.Chars()));

	if (!UberCopyWrapper::CopyToGame(options, target, UberCopyWrapper::GetTaskDescriptionXmlFilePath(options, nodeName)))
	{
		MessageBox(
			0,
			"One or more files have failed to copy.",
			"Problems Copying!",
			MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_SYSTEMMODAL);
	}
	else
	{
		Util::C_String text = "All files have been successfully copied for ";
		if (target.Length() == 0)
			text += "all platforms";
		else
			text += target;
		text += ".";
		MessageBox(
			0,
			text.Chars(),
			"Copy Successful!",
			MB_ICONINFORMATION | MB_SETFOREGROUND | MB_SYSTEMMODAL);
	}

	return true;
}
