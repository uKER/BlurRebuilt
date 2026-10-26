#include "stdafx.h"

#include <EditorLib/GlobalSignals.h>

namespace EditorLib
{

C_GlobalSignals::C_GlobalSignals() : m_slots(this)
{
	m_slots.Connect();
}

C_GlobalSignals::~C_GlobalSignals()
{
	m_slots.Disconnect();
}

C_GlobalSignals::C_Slots::C_Slots(C_GlobalSignals* pGlobalSignals) : m_pGlobalSignals(pGlobalSignals)
{
	m_mainThreadId = GetCurrentThread();
}

void C_GlobalSignals::C_Slots::Connect()
{
	m_pGlobalSignals->quit.connect(this, &C_GlobalSignals::C_Slots::QuitSlot);

	m_pGlobalSignals->aftkRefLoaded.connect(this, &C_GlobalSignals::C_Slots::AftkRefLoadedSlot);
	m_pGlobalSignals->aftkRefUnloaded.connect(this, &C_GlobalSignals::C_Slots::AftkRefUnloadedSlot);

	m_pGlobalSignals->databaseEvent.connect(this, &C_GlobalSignals::C_Slots::DatabaseEventSlot);

	m_pGlobalSignals->levelOpening.connect(this, &C_GlobalSignals::C_Slots::LevelOpeningSlot);
	m_pGlobalSignals->levelOpened.connect(this, &C_GlobalSignals::C_Slots::LevelOpenedSlot);
	m_pGlobalSignals->levelClosing.connect(this, &C_GlobalSignals::C_Slots::LevelClosingSlot);
	m_pGlobalSignals->levelClosed.connect(this, &C_GlobalSignals::C_Slots::LevelClosedSlot);

	m_pGlobalSignals->levelAftkRefAdded.connect(this, &C_GlobalSignals::C_Slots::LevelAftkRefAddedSlot);
	m_pGlobalSignals->levelAftkRefRemoved.connect(this, &C_GlobalSignals::C_Slots::LevelAftkRefRemovedSlot);
	m_pGlobalSignals->levelNameChanged.connect(this, &C_GlobalSignals::C_Slots::LevelNameChangedSlot);
	
	m_pGlobalSignals->layerEvent.connect(this, &C_GlobalSignals::C_Slots::LayerEventSlot);

	m_pGlobalSignals->layerNodeAdded.connect(this, &C_GlobalSignals::C_Slots::LayerNodeAddedSlot);
	m_pGlobalSignals->layerNodeRemoved.connect(this, &C_GlobalSignals::C_Slots::LayerNodeRemovedSlot);

	m_pGlobalSignals->dgNodeCreated.connect(this, &C_GlobalSignals::C_Slots::DgNodeCreatedSlot);
	m_pGlobalSignals->dgNodeDestroy.connect(this, &C_GlobalSignals::C_Slots::DgNodeDestroySlot);
		
	m_pGlobalSignals->coreSelectionChanged.connect(this, &C_GlobalSignals::C_Slots::CoreSelectionChangedSlot);
	m_pGlobalSignals->coreContextChanged.connect(this, &C_GlobalSignals::C_Slots::CoreContextChangedSlot);
	m_pGlobalSignals->coreEnableViews.connect(this, &C_GlobalSignals::C_Slots::CoreEnableViewsSlot);	
	m_pGlobalSignals->coreDirtyViews.connect(this, &C_GlobalSignals::C_Slots::CoreDirtyViewsSlot);

	m_pGlobalSignals->dgNodeUpdated.connect(this, &C_GlobalSignals::C_Slots::DgNodeUpdatedSlot);
}

void C_GlobalSignals::C_Slots::Disconnect()
{
	m_pGlobalSignals->quit.disconnect(this);

	m_pGlobalSignals->aftkRefLoaded.disconnect(this);
	m_pGlobalSignals->aftkRefUnloaded.disconnect(this);

	m_pGlobalSignals->databaseEvent.disconnect(this);

	m_pGlobalSignals->levelOpening.disconnect(this);
	m_pGlobalSignals->levelOpened.disconnect(this);
	m_pGlobalSignals->levelClosing.disconnect(this);
	m_pGlobalSignals->levelClosed.disconnect(this);

	m_pGlobalSignals->levelAftkRefAdded.disconnect(this);
	m_pGlobalSignals->levelAftkRefRemoved.disconnect(this);
	m_pGlobalSignals->levelNameChanged.disconnect(this);

	m_pGlobalSignals->layerEvent.disconnect(this);

	m_pGlobalSignals->layerNodeAdded.disconnect(this);
	m_pGlobalSignals->layerNodeRemoved.disconnect(this);

	m_pGlobalSignals->dgNodeCreated.disconnect(this);
	m_pGlobalSignals->dgNodeDestroy.disconnect(this);
		
	m_pGlobalSignals->coreSelectionChanged.disconnect(this);
	m_pGlobalSignals->coreContextChanged.disconnect(this);
	m_pGlobalSignals->coreEnableViews.disconnect(this);	
	m_pGlobalSignals->coreDirtyViews.disconnect(this);

	m_pGlobalSignals->dgNodeUpdated.disconnect(this);
}

////// Signals from C_Core //////
void C_GlobalSignals::C_Slots::QuitSlot()
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

////// Signals from C_AftkRefManager //////
void C_GlobalSignals::C_Slots::AftkRefLoadedSlot(
	const Btl::String& originalFilePath, 
	const Aftk::C_Scene& aftkScene, 
	const EditorLib::C_SceneNode& sceneNode, 
	const EditorLib::AftkDgMap& aftkDgMap)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

void C_GlobalSignals::C_Slots::AftkRefUnloadedSlot(const Btl::String& originalFilePath)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

void C_GlobalSignals::C_Slots::DatabaseEventSlot(Xt::DomDb& db, E_DatabaseEvent evt)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

////// Signals from C_XtLevel //////
void C_GlobalSignals::C_Slots::LevelOpeningSlot(const I_Level& level)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

void C_GlobalSignals::C_Slots::LevelOpenedSlot(const I_Level& level)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

void C_GlobalSignals::C_Slots::LevelClosingSlot(const I_Level& level)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

void C_GlobalSignals::C_Slots::LevelClosedSlot(const I_Level& level)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

void C_GlobalSignals::C_Slots::LevelAftkRefAddedSlot(const Btl::String& originalFilePath)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

void C_GlobalSignals::C_Slots::LevelAftkRefRemovedSlot(const Btl::String& originalFilePath)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

void C_GlobalSignals::C_Slots::LevelNameChangedSlot(const I_Level& level)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

//signals from C_Layer
void C_GlobalSignals::C_Slots::LayerEventSlot(const I_Layer& layer, E_LayerEvent e)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

void C_GlobalSignals::C_Slots::LayerNodeAddedSlot(const I_Layer& layer, const I_DgNode& dgNode)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

void C_GlobalSignals::C_Slots::LayerNodeRemovedSlot(const I_Layer& layer, const I_DgNode& dgNode)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

////// Signals from C_DgManager //////
void C_GlobalSignals::C_Slots::DgNodeCreatedSlot(const I_DgNode& dgNode)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

void C_GlobalSignals::C_Slots::DgNodeDestroySlot(const I_DgNode& dgNode)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

////// Signals from C_Core //////
void C_GlobalSignals::C_Slots::CoreSelectionChangedSlot()
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

void C_GlobalSignals::C_Slots::CoreContextChangedSlot(const Btl::String& context, const void* contextData)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

void C_GlobalSignals::C_Slots::CoreEnableViewsSlot(bool enable)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

void C_GlobalSignals::C_Slots::CoreDirtyViewsSlot(E_DirtyEventType eventType)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

////// Signals from Dg node and its derived classes. //////
void C_GlobalSignals::C_Slots::DgNodeUpdatedSlot(const I_DgNode& dgNode, const C_UpdateHintCollection* pHints)
{
	BIZ_ASSERT(m_mainThreadId == GetCurrentThread());
}

}

