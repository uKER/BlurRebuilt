#ifndef EDITOR_LIB_XT_PATCH_HELPERS_H
#define EDITOR_LIB_XT_PATCH_HELPERS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{

namespace XtMergeHelpers
{
	bool CanMerge(Xt::Change* cl, Xt::Change* cr);

	bool ResolveConflicts(Xt::Change** result, Xt::Change* c);
}

}

#endif
