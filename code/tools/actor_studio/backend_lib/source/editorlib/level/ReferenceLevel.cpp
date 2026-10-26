#include "stdafx.h"

#include <EditorLib/Aftk/AftkRefManager.h>
#include <EditorLib/Aftk/AftkRefHelpers.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/ReferenceLevel.h>
#include <EditorLib/Xt/XtVisualisation.h>
#include <EditorLib/Core.h>
#include <EditorLib/CoreConfig.h>
#include <EditorLib/GlobalSignals.h>

using namespace EditorLib;

C_ReferenceLevel::C_ReferenceLevel()
{
	m_Slots.Connect();
}

C_ReferenceLevel::~C_ReferenceLevel()
{
	m_Slots.Disconnect();
}

E_LevelType C_ReferenceLevel::GetType() const
{
	return E_LevelType_ReferenceLevel;
}

void C_ReferenceLevel::OnDatabaseEvent(Xt::DomDb& db, E_DatabaseEvent e)
{
	if (e == E_DatabaseEvent_Opened)
	{
		//Add default visualisation reference scenes (we don't necessarily need to add these to the level record)...
		if (C_Core::Instance().GetConfig().appType == E_AppType_Interactive)
		{
			Btl::Vector<Btl::String> paths;
			C_Core::Instance().GetXtVisualisationData().BuildResourceFileList(paths);
			for (S32 i=0; i<paths.Size(); ++i)
			{
				const C_AftkRef *pScene = C_Core::Instance().GetAftkRefManager().CreateRef(
					paths[i].Chars(), true);
	#if 0
				if (!pScene->IsLoaded())
					pScene->Load();
	#endif
				C_AftkLevelRef* pAftkLevelRef = new C_AftkLevelRef;
				pAftkLevelRef->isExtActor = false;
				pAftkLevelRef->pAftkRef = pScene;
				m_aftkRefs.PushBack(pAftkLevelRef);
				C_Core::Instance().GetGlobalSignals().levelAftkRefAdded(paths[i]);
			}
		}
	}
}

void C_ReferenceLevel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.connect(this, &C_ReferenceLevel::C_Slots::OnDatabaseEventSlot);
}

void C_ReferenceLevel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.databaseEvent.disconnect(this);
}

void C_ReferenceLevel::C_Slots::OnDatabaseEventSlot(Xt::DomDb& db, E_DatabaseEvent e)
{
	EditorLib::C_Core::Instance().GetLevelManager()->GetReferenceLevel()->OnDatabaseEvent(db, e);
}