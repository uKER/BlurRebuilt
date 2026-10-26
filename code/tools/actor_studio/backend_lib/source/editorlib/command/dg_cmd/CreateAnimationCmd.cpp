#include "stdafx.h"

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Dg_Cmd/CreateAnimationCmd.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneNodeHelpers.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>

#include <EditorLib/Level/Animation.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>

#include <EditorLib/Core.h>

namespace EditorLib
{

C_CreateAnimationCmd::C_CreateAnimationCmd(U32 type) : I_Command(type)
{
}

C_CreateAnimationCmd::~C_CreateAnimationCmd()
{
}

C_Status C_CreateAnimationCmd::Do()
{
	C_Status status;
	C_XtLevel *pLevel = C_Core::Instance().GetLevelManager()->GetActiveLevel();

	Btl::String groupName("");
	const ArgumentDataArray* pArgData = m_args.GetArgData('#');
	if (pArgData && pArgData->Size())
		(*pArgData)[0].GetValue(groupName);

	C_CheckpointHelper c(&m_pChange, GetName());

	EditorLib::Animation::CreateAnimation(pLevel);

	return E_Status_Success;
}

bool C_CreateAnimationCmd::IsUndoable() const
{
	return true;
}

void C_CreateAnimationCmd::GetResults(ResultsArray& results) const
{
	results.PushBack();
	results[0].SetValue(m_results.nodeId.AsString());
}

I_Command* C_CreateAnimationCmd::Create(U32 type)
{
	return new C_CreateAnimationCmd(type);
}

C_Syntax* C_CreateAnimationCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	return pSyntax;
}

const char* C_CreateAnimationCmd::GetName()
{
	static char* p = "createAnimation";
	return p;
}

}
