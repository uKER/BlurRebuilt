#include "stdafx.h"

#include <EditorLib/Command/CommandProcessor.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneXtActorNode.h>
#include <EditorLib/Level/LevelPack.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

namespace EditorLib
{

C_LevelPack::C_LevelPack(XtId packId)
{
	m_PackId = packId;
}

C_Status C_LevelPack::Open()
{
	Xt::DomPack* pPack = C_Core::Instance().GetXtManager().FindPack(m_PackId);

	if (!pPack)
		return E_Status_Failure;

	C_Core::Instance().GetGlobalSignals().packEvent(*this, E_PackEvent_Opening);

	for (XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator it(&pPack->records); it; ++it)
	{
		if (it->root.type != EditorLib::kXtLevelRecordType)
			continue;

		if (m_Levels.Find(it->id) == m_Levels.End())
		{
			C_XtLevel* newLevel = new C_XtLevel(this);
			newLevel->SetXtRecord(it->id);
			m_Levels[it->id] = newLevel;
		}
	}

	C_Core::Instance().GetGlobalSignals().packEvent(*this, E_PackEvent_Opened);

	return E_Status_Success;
}

C_Status C_LevelPack::Close()
{
	C_Core::Instance().GetGlobalSignals().packEvent(*this, E_PackEvent_Closing);

	for (Btl::OpenHashMap<XtId, C_XtLevel*>::Iterator it = m_Levels.Begin(); it != m_Levels.End(); ++it)
	{
		it->GetValue()->Unload();
		SAFE_DELETE_PTR(it->GetValue());
	}

	m_Levels.Clear();

	C_Core::Instance().GetGlobalSignals().packEvent(*this, E_PackEvent_Closed);

	return E_Status_Success;
}

C_Status C_LevelPack::LoadLevel(XtId recordId)
{
	if (m_Levels[recordId]->GetFlags().isOpen)
		return E_Status_Success;

	C_Status status = m_Levels[recordId]->Load();
	if (!status)
		return status;

	// Set read-only flag on entire level
	m_Levels[recordId]->SetReadOnly(true);

	return E_Status_Success;
}

C_XtLevel* C_LevelPack::GetLevel(XtId recordId)
{
	if (m_Levels.Find(recordId) != m_Levels.End())
		return m_Levels[recordId];
	
	return 0;
}

C_XtLevel* C_LevelPack::NewLevel()
{
	C_XtLevel* newLevel = 0;

	Xt::DomPack* pPack = C_Core::Instance().GetXtManager().FindPack(m_PackId);
	if (!pPack)	return 0;

	XtId recordId = C_Core::Instance().GetXtManager().CreateRecord(m_PackId, kXtLevelRecordType);
	if (recordId == kXtInvalidId) return 0;

	Xt::DomRecord* pLevelRecord = C_Core::Instance().GetXtManager().FindRecord(recordId);
	if (!pLevelRecord) return 0;

	newLevel = new C_XtLevel(this);
	C_Status status;
	
	status = newLevel->SetXtRecord(recordId);
	if (!status)
	{
		SAFE_DELETE_PTR(newLevel);
		return 0;
	}

	m_Levels[recordId] = newLevel;

	status = newLevel->Load();
	if (!status)
	{
		SAFE_DELETE_PTR(newLevel);
		return 0;
	}

	return newLevel;
}

XtId C_LevelPack::GetId() const
{
	return m_PackId;
}

Btl::String C_LevelPack::GetName() const
{
	Xt::DomPack* pLevelPack = EditorLib::C_Core::Instance().GetXtManager().FindPack(m_PackId);
	if (pLevelPack)
		return Btl::String(pLevelPack->name.Text());
	else
		return "";
}

Btl::Vector<C_XtLevel*> C_LevelPack::GetLevels() const
{
	Btl::Vector<C_XtLevel*> levels;

	for (Btl::OpenHashMap<XtId, C_XtLevel*>::ConstIterator it = m_Levels.Begin(); it != m_Levels.End(); ++it)
	{
		levels.PushBack(it->GetValue());
	}

	return levels;
}

}