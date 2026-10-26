#include "stdafx.h"

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/LevelPack.h>
#include <EditorLib/Level/ReferenceLevel.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/Misc/UndoManager.h>
#include <EditorLib/Xt/XtManager.h>

#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

namespace EditorLib
{

C_LevelManager::C_LevelManager()
{
	m_pActiveLevel = 0;

	m_pReferenceLevel = new C_ReferenceLevel();
	m_pReferenceLevel->SetName("References");
	m_pReferenceLevel->Load();

	m_Slots.Connect();
}

C_LevelManager::~C_LevelManager()
{
	Close();

	m_Slots.Disconnect();

	delete m_pReferenceLevel;
}

C_ReferenceLevel* C_LevelManager::GetReferenceLevel()
{
	return m_pReferenceLevel;
}

C_XtLevel* C_LevelManager::GetActiveLevel()
{
	return m_pActiveLevel;
}

void C_LevelManager::SetActiveLevel(C_XtLevel* level)
{
	C_Core::Instance().GetGlobalSignals().levelEvent(m_pActiveLevel, E_LevelEvent_ActiveLevelChanging);

	if (m_pActiveLevel)
	{
		m_pActiveLevel->SetReadOnly(true);
		EditorLib::C_Core::Instance().GetDgManager().FlushDeferredUpdates();
	}

	m_pActiveLevel = level;

	C_Core::Instance().GetGlobalSignals().levelEvent(m_pActiveLevel, E_LevelEvent_ActiveLevelChanged);

	if (m_pActiveLevel)
	{
		if (!m_pActiveLevel->GetFlags().isOpen)
			m_pActiveLevel->Load();

		m_pActiveLevel->SetReadOnly(false);
	}
}

C_Status C_LevelManager::Open(XtId id)
{
	if (C_Core::Instance().GetXtManager().FindPack(id))
	{
		C_Status status = OpenPack(id);
		if (!status)
			return status;

		Btl::Vector<C_XtLevel*> levels = m_Packs[id]->GetLevels();
		for (Btl::Vector<C_XtLevel*>::Iterator it = levels.Begin(); it != levels.End(); ++it)
		{
			if (!m_Packs[id]->LoadLevel((*it)->GetXtRecordId()))
				return E_Status_Failure;
		}

		if (levels.Size())
			SetActiveLevel(levels[0]);

		return E_Status_Success;
	}
	else if (C_Core::Instance().GetXtManager().FindRecord(id))
	{
		return OpenLevel(id);
	}

	return E_Status_Failure;
}

C_Status C_LevelManager::Close()
{
	while (m_Packs.Begin() != m_Packs.End())
	{
		if (ClosePack(m_Packs.Begin()->GetKey()) != E_Status_Success)
			return E_Status_Failure;
	}

	return E_Status_Success;
}

C_Status C_LevelManager::OpenPack(XtId packId)
{
	if (m_Packs.Find(packId) != m_Packs.End())
		return E_Status_Success;

	m_Packs[packId] = new C_LevelPack(packId);

	if (!m_Packs[packId]->Open())
	{
		delete m_Packs[packId];
		m_Packs.Erase(m_Packs.Find(packId));

		return E_Status_Failure;
	}

	return E_Status_Success;
}

C_Status C_LevelManager::ClosePack(XtId packId)
{
	if (m_Packs.Find(packId) == m_Packs.End())
		return E_Status_Failure;

	m_Packs[packId]->Close();

	delete m_Packs[packId];

	m_Packs.Erase(m_Packs.Find(packId));

	return E_Status_Success;
}

C_LevelPack* C_LevelManager::GetPack(XtId packId)
{
	if (m_Packs.Find(packId) == m_Packs.End())
		return 0;
	else
		return m_Packs[packId];
}

C_XtLevel* C_LevelManager::GetLevel(XtId levelId)
{
	for (Btl::OpenHashMap<XtId, C_LevelPack*>::Iterator it = m_Packs.Begin(); it != m_Packs.End(); ++it)
	{
		C_XtLevel* level = it->GetValue()->GetLevel(levelId);
		if (level)
			return level;
	}

	return 0;
}

Btl::Vector<C_LevelPack*> C_LevelManager::GetPacks()
{
	Btl::Vector<C_LevelPack*> returnPacks;

	for (Btl::OpenHashMap<XtId, C_LevelPack*>::Iterator it = m_Packs.Begin(); it != m_Packs.End(); ++it)
	{
		returnPacks.PushBack(it->GetValue());
	}

	return returnPacks;
}

C_Status C_LevelManager::OpenLevel(XtId recordId)
{
	Xt::DomRecord* pRecord = C_Core::Instance().GetXtManager().FindRecord(recordId);
	BIZ_ASSERT(pRecord);

	Xt::DomPack* pPack = Xt::PackOf(pRecord);
	BIZ_ASSERT(pPack);

	C_LevelPack* levelPack = GetPack(pPack->id);
	
	if (!levelPack)
	{
		if (OpenPack(pPack->id) != E_Status_Success)
			return E_Status_Failure;

		levelPack = GetPack(pPack->id);
	}

	BIZ_ASSERT(levelPack);

	if (levelPack->LoadLevel(recordId) != E_Status_Success)
		return E_Status_Failure;

	SetActiveLevel(levelPack->GetLevel(recordId));

	return E_Status_Success;
}

void C_LevelManager::OnLevelOpened(const I_Level& level)
{
	if (!m_pActiveLevel && level.GetType() == E_LevelType_XtLevel)
	{
		m_pActiveLevel = (C_XtLevel*)&level;
	}
}

void C_LevelManager::OnLevelClosing(const I_Level& level)
{
	if (&level == m_pActiveLevel)
	{
		// Find a loaded xt level, and set the active level to that, else it will be set to 0.
		C_XtLevel* loadedLevel = 0;

		for (Btl::OpenHashMap<XtId, C_LevelPack*>::Iterator packIt = m_Packs.Begin(); packIt != m_Packs.End(); ++packIt)
		{
			Btl::Vector<C_XtLevel*> levels = packIt->GetValue()->GetLevels();

			for (Btl::Vector<C_XtLevel*>::Iterator levelIt = levels.Begin(); levelIt != levels.End(); ++levelIt)
			{
				C_XtLevel* xtLevel = *levelIt;

				if (xtLevel && xtLevel != &level)
				{
					if (xtLevel->GetFlags().isOpen)
					{
						loadedLevel = xtLevel;
					}
				}
			}
		}

		SetActiveLevel(loadedLevel);
	}
}

void C_LevelManager::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = C_Core::Instance().GetGlobalSignals();

	s.levelOpening.connect(this, &C_LevelManager::C_Slots::OnLevelOpenedSlot);
	s.levelClosing.connect(this, &C_LevelManager::C_Slots::OnLevelClosingSlot);
}

void C_LevelManager::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = C_Core::Instance().GetGlobalSignals();

	s.levelOpening.disconnect(this);
	s.levelClosing.disconnect(this);
}

void C_LevelManager::C_Slots::OnLevelOpenedSlot(const I_Level& level)
{
	C_Core::Instance().GetLevelManager()->OnLevelOpened(level);
}

void C_LevelManager::C_Slots::OnLevelClosingSlot(const I_Level& level)
{
	C_Core::Instance().GetLevelManager()->OnLevelClosing(level);
}

}