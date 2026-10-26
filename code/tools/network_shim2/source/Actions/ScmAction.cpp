#include "stdafx.h"
#include <tools_shared/shim/ShimErrorHelpers.h>
#include <actions/ScmAction.h>
#include "processes/AlienBrainWrapper.h"

#include <Helpers/XmlHelper.h>

bool ScmAction::Execute(const C_Options& options, const Sys::C_XmlDom& dom)
{	
	return AlienBrainWrapper::GetLatest(options, dom, false);
}
