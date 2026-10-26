#include "stdafx.h"

#include <System/Xml/Xml.h>

#include <Aftk/AftkTypedefs.h>
#include <Aftk/AftkEnum.h>
#include <Aftk/AftkBounds.h>
#include <Aftk/AftkMaths.h>
#include <Aftk/AftkEuler.h>

#include <EditorLib/Aftk/AftkRefManager.h>
#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Aftk/AftkRefHelpers.h>

#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Command/ICommand.h>
#include <EditorLib/Command/Ext_Actors_Cmd/ExtActorsRefresh.h>
#include <EditorLib/Command/Ext_Actors_Cmd/ExtActorsRefreshCmd.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneAftkNode.h>
#include <EditorLib/Dg/SceneAftkActorNode.h>
#include <EditorLib/Dg/SceneNodeIterator.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgNodeHelpers.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/XtNodeHelpers.h>

#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Misc/UndoManager.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtFieldHelper.h>

#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Core.h>

namespace EditorLib
{

C_ExtActorsRefreshCmd::C_ExtActorsRefreshCmd(U32 type) : I_Command(type)
{
}

C_ExtActorsRefreshCmd::~C_ExtActorsRefreshCmd()
{
}

C_Status C_ExtActorsRefreshCmd::Do()
{
	C_CheckpointHelper c(&m_pChange, GetName());

	C_XtLevel* pLevel = C_Core::Instance().GetLevelManager()->GetActiveLevel();
	if (!pLevel)
		return E_Status_Failure;

	XtId recordId = pLevel->GetXtRecordId();
	Xt::DomRecord* pLevelRecord = C_Core::Instance().GetXtManager().FindRecord(recordId);
	if (!pLevelRecord)
		return E_Status_Failure;

	if (m_args.IsSet('c'))
	{
		ClearRecord(pLevelRecord);
		return E_Status_Success;
	}

	//Perform refresh...
	C_ExtActorsDbStats dbStats;

	//Do record.
	bool status = true;
	{
		dbStats.packStatsArray.PushBack();
		Xt::DomPack* pPack = Xt::PackOf(pLevelRecord);
		dbStats.packStatsArray.Back().packName = pLevelRecord->name.Text();
		dbStats.packStatsArray.Back().recordStatsArray.PushBack();
		//Create record stats structure.
		C_ExtActorsRecordStats& recordStats = dbStats.packStatsArray.Back().recordStatsArray.Back();
		status &= DoRecord(recordStats, pLevelRecord);
	}

	C_Core::Instance().GetXtManager().EnsureUniqueUids(true);

	DisplayRefreshSummary(dbStats);
	return status ? E_Status_Success : E_Status_Failure;
}

C_Status C_ExtActorsRefreshCmd::ProcessOnline(
	C_ExtActorsRecordStats& stats,
	const C_AftkRef* pRef)
{
	C_XtLevel* pLevel = C_Core::Instance().GetLevelManager()->GetActiveLevel();
	if (!pLevel)
		return E_Status_Failure;
	XtId recordId = pLevel->GetXtRecordId();
	Xt::DomRecord* pLevelRecord = C_Core::Instance().GetXtManager().FindRecord(recordId);
	if (!pLevelRecord)
		return E_Status_Failure;

	Btl::Vector<C_AftkActorPlaceholder> aftkActorPlaceholders;
	if (!BuildOnlineRefInfo(
		aftkActorPlaceholders,
		pRef))
		return E_Status_Failure;

	S32 nActorsCreated = CreateActors(pLevelRecord, pRef->GetOriginalFilePath(), aftkActorPlaceholders);
	if (nActorsCreated == -1)
		return E_Status_Failure;

	stats.nActorsCreated += nActorsCreated;
	stats.refArray.PushBack();
	stats.refArray.Back().nActors = nActorsCreated;
	stats.refArray.Back().originalFilePath = pRef->GetOriginalFilePath();
	return E_Status_Success;
}

bool C_ExtActorsRefreshCmd::BuildOnlineRefInfo(
	Btl::Vector<C_AftkActorPlaceholder>& aftkActorPlaceholders, const C_AftkRef* pRef) const
{
	//Recreate externally created actors.
	const C_SceneNode* pSceneNode = pRef->GetSceneRootNode();
	if (!pSceneNode)
		//scene is not loaded.
		return E_Status_Failure;

	for (C_SceneNodeConstIterator it(pSceneNode->GetChild()); it; ++it)
	{
		const C_SceneNode* pSceneNode = it;
		if (!pSceneNode->IsA(E_ObjType_SceneAftkActorNode))
			continue;

		const C_SceneAftkActorNode* pSceneAftkActorNode = (const C_SceneAftkActorNode*)pSceneNode;
		//Skip base instances.
		if (pSceneAftkActorNode->GetNodeFlags().baseInstance)
			continue;

		aftkActorPlaceholders.PushBack();
		aftkActorPlaceholders.Back().attrs = pSceneAftkActorNode->GetAftkProperties();
		aftkActorPlaceholders.Back().name = pSceneAftkActorNode->GetName();
		DgNodeHelpers::BuildPath(aftkActorPlaceholders.Back().path, pSceneNode);
		aftkActorPlaceholders.Back().refFilePath = pRef->GetOriginalFilePath();
		aftkActorPlaceholders.Back().worldMatrix = pSceneAftkActorNode->GetWorldMatrix();
	}

	return true;
}

bool C_ExtActorsRefreshCmd::DoRecord(C_ExtActorsRecordStats& stats, Xt::DomRecord* pRecord)
{
	I_Level* pLevel = C_Core::Instance().GetLevelManager()->GetActiveLevel();
	if (!pLevel)
		return false;

	stats.recordName = pRecord->name.Text();
	stats.nActorsCreated = 0;
	//Build a list of existing actors for merging into.
	C_AftkRefManager& manager = C_Core::Instance().GetAftkRefManager();
	ConstAftkRefList refList;
	manager.GetRefs(refList);
	for (ConstAftkRefList::ConstIterator it = refList.Begin(); it != refList.End(); ++it)
	{
		const C_AftkRef* pRef = *it;
		if (pRef->IsVisualisation())
			continue;

		const C_AftkLevelRef* pLevelRef = pLevel->GetAftkRef(pRef->GetOriginalFilePath());
		if (!pLevelRef || !pLevelRef->isExtActor)
			continue;

		C_Status status = ProcessOnline(stats, pRef);
		if (!status) return status;
	}

	return E_Status_Success;
}

bool C_ExtActorsRefreshCmd::IsUndoable() const
{
	return true;
}

I_Command* C_ExtActorsRefreshCmd::Create(U32 type)
{
	return new C_ExtActorsRefreshCmd(type);
}

C_Syntax* C_ExtActorsRefreshCmd::CreateSyntax()
{
	C_Syntax* pSyntax = new C_Syntax();
	{
		C_Argument arg(
			'c', 
			"clear",
			"clear external actors",
			C_Argument::E_Required_False);
		pSyntax->AddArg(arg);
	}
	return pSyntax;
}

const char* C_ExtActorsRefreshCmd::GetName()
{
	static char* p = "extActorsRefresh";
	return p;
}

}
