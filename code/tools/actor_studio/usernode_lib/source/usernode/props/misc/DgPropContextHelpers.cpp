#include "stdafx.h"

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Dg/DgXtPropNodeBase.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>

#include <UserNode/Props/Misc/DgPropContextHelpers.h>

namespace Editor
{
namespace PropContextHelpers
{

const EditorLib::C_SceneNode* GetTopLevelParent(
	const EditorLib::C_DgXtPropNodeBase& dgProperty)
{
	const EditorLib::C_DgXtCompNode* pParentComponent =
		dgProperty.GetParent();
	assert(pParentComponent);
	const EditorLib::I_DgNode* pTopLevelDgParent = pParentComponent->GetParent();
	assert(pTopLevelDgParent);
	//Bail if our top level parent isn't a scene node.
	if (!pTopLevelDgParent->IsA(EditorLib::E_ObjType_SceneNode))
		return 0;

	return (EditorLib::C_SceneNode*)pTopLevelDgParent;
}

EditorLib::C_SceneNode* GetTopLevelParent(
	EditorLib::C_DgXtPropNodeBase& dgProperty)
{
	EditorLib::C_DgXtCompNode* pParentComponent =
		dgProperty.GetParent();
	assert(pParentComponent);
	EditorLib::I_DgNode* pTopLevelDgParent = pParentComponent->GetParent();
	assert(pTopLevelDgParent);
	//Bail if our top level parent isn't a scene node.
	if (!pTopLevelDgParent->IsA(EditorLib::E_ObjType_SceneNode))
		return 0;

	return (EditorLib::C_SceneNode*)pTopLevelDgParent;
}

}
}
