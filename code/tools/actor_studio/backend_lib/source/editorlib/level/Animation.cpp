#include "stdafx.h"

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtSemantic.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/SceneNodeHelpers.h>

#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Level/LevelHelpers.h>

#include <EditorLib/Core.h>

namespace EditorLib
{
namespace Animation
{

void CreateAnimation(C_XtLevel* pLevel)
{
	LevelHelpers::CreateNode(pLevel, "AnimationClip");
}

}

}