#include "FrameworkPch.h"
#include "Framework/Layer/Layer.h"
#include "Framework/Layer/LayerManager.h"

namespace Framework
{

// C_LayerManagerThread

C_LayerManagerThread::C_LayerManagerThread( C_LayerManager* pLayerManager )
	: m_EventSignalQuit("EventSignalQuit")
	, m_pLayerManager(pLayerManager)
{

}

C_LayerManagerThread::~C_LayerManagerThread()
{

}

void C_LayerManagerThread::ThreadMain()
{
	
	//C_Manager* pManager = C_Manager::GetInstance();
	m_pLayerManager->SetStatusDirty();

	while(!ShouldQuit())
	{
		//WaitIfPaused();

		bool didSomething = m_pLayerManager->UpdateAsync();

		if(!didSomething)
		{
			m_pLayerManager->WaitForStatusDirty();
		}
	}
}

bool C_LayerManagerThread::ShouldQuit()
{
	return m_EventSignalQuit.Wait(0);
}

void C_LayerManagerThread::SendQuitMessage()
{
	m_EventSignalQuit.Signal();
}


// C_LayerItem

C_LayerItem::C_LayerItem()
	: m_pLayer( NULL )
{
}

C_LayerItem::C_LayerItem( C_Layer* pLayer )
	: m_pLayer( pLayer )
{
}

C_LayerItem::~C_LayerItem()
{
}

bool C_LayerItem::operator < (const C_LayerItem& other) const
{
	// if priorities are equal, order by pointer
	if( m_pLayer->GetLayerPriority() == other.m_pLayer->GetLayerPriority() )
	{
		return (m_pLayer < other.m_pLayer);
	}

	return m_pLayer->GetLayerPriority() < other.m_pLayer->GetLayerPriority();
}

C_Layer* C_LayerItem::GetLayerPtr() const
{
	return m_pLayer;
}

// C_LayerManager

C_LayerManager::C_LayerManager()
	: m_LayerManagerThread(this)
	, m_StatusDirtyEvent("LayerStatusDirtyEvent")
{
	m_LayerManagerThread.Start(0,0,"LayerManagerThread");
	SetStatusDirty();
}

C_LayerManager::~C_LayerManager()
{
}

void C_LayerManager::ShutDown()
{
	if( m_Layers.Size() != 0 )
	{
		LOGWARN( g_Log, "Layers still active on exit" );
	}

	Ut::UtList<C_Layer*>::Iterator it = m_LayersPendingCreate.Begin();
	while(it != m_LayersPendingCreate.End())
	{
		C_Layer* pLayer = *it;

		m_LayersPendingCreate.Erase(it);
		SAFE_DELETE_PTR(pLayer);

		it = m_LayersPendingCreate.Begin();
	}

	// TODO: shutdown async layers
	SetStatusDirty();


	DestroyAllLayers();
	DestroyPendingLayers();
}

void C_LayerManager::SetStatusDirty()
{
	m_StatusDirtyEvent.Signal();
}

void C_LayerManager::WaitForStatusDirty()
{
	m_StatusDirtyEvent.Wait();
}

void C_LayerManager::InsertLayer(C_Layer* pLayer)
{
	assert(pLayer);

	assert(pLayer->GetLayerState() == E_LayerState_Constructed);

	m_LayersPendingCreate.PushBack(pLayer);
	
	pLayer->SetLayerState( E_LayerState_PendingCreate );
}

void C_LayerManager::DestroyLayer(C_Layer* pLayer)
{
	assert(pLayer);

	assert(pLayer->GetLayerState() == E_LayerState_Running);

	// remove from active lists
	//EraseLayer( pLayer );
	m_LayersPendingDestroy.PushBack(pLayer);

	pLayer->SetLayerState( E_LayerState_PendingDestroy );
}

void C_LayerManager::InsertLayerAsync(C_Layer* pLayer)
{
	MT::AutoLock autoLock(m_AsyncCreateListLock);

	assert(pLayer);

	assert(pLayer->GetLayerState() == E_LayerState_Constructed);

	m_LayersPendingAsyncCreate.PushBack(pLayer);

	pLayer->SetLayerState( E_LayerState_PendingCreate );

	SetStatusDirty();
}

void C_LayerManager::DestroyLayerAsync(C_Layer* pLayer)
{
	assert(pLayer);

	assert(pLayer->GetLayerState() == E_LayerState_Running);

	// remove from active list
	//EraseLayer( pLayer );

	// just state is differnt, both async and non are put into this list
	m_LayersPendingDestroy.PushBack(pLayer);

	pLayer->SetLayerState( E_LayerState_PendingAsyncDestroy );

	SetStatusDirty();
}

void C_LayerManager::DestroyAllLayers()
{
	C_LayerList::Iterator it = m_Layers.Begin();
	while(it != m_Layers.End())
	{
		C_LayerItem layerItem = *it;
		C_Layer* pLayer = layerItem.GetLayerPtr();
		it++;

		if(pLayer->GetLayerState() == E_LayerState_Running)
		{
			DestroyLayer(pLayer);
		}
	}
}

void C_LayerManager::Update()
{
	DestroyPendingLayers();
	CreatePendingLayers();

	//UpdateAsync();

	StartReadyLayers();
}

bool C_LayerManager::UpdateAsync()
{
	bool didSomething = false;
	didSomething |= DestroyPendingAsyncLayers();
	didSomething |= CreatePendingAsyncLayers();
	return didSomething;
}

void C_LayerManager::DestroyPendingLayers()
{
	//TODO: Changed this to use a reverse iterator as there were layers depending on the existence of layers
	//created previously (i.e. the game layer releasing menu items from the debug layer). Destroying them in reverse 
	//order is more likely to work, but there still could be dependancies between layers. 

	Ut::UtList<C_Layer*>::ReverseIterator it = m_LayersPendingDestroy.ReverseBegin();
	while(it != m_LayersPendingDestroy.ReverseEnd())
	{
		C_Layer* pLayer = *it;

		EraseLayer(pLayer);
		m_LayersPendingDestroy.Erase(it);

		if(pLayer->m_LayerState == E_LayerState_PendingAsyncDestroy)
		{
			MT::AutoLock autoLock(m_AsyncDestroyListLock);
			m_LayersPendingAsyncDestroy.PushBack(pLayer);
		}
		else
		{
			SendMessageToLayer( pLayer, E_LayerMessage_Destroy );

			pLayer->SetLayerState( E_LayerState_Constructed );
			SAFE_DELETE_PTR( pLayer );
		}

		it = m_LayersPendingDestroy.ReverseBegin();
	}
}

bool C_LayerManager::DestroyPendingAsyncLayers()
{
	bool didSomething = false;

	Ut::UtList<C_Layer*> layersPendingDestroy;
	{
		MT::AutoLock autoLock(m_AsyncDestroyListLock);

		Ut::UtList<C_Layer*>::ReverseIterator it = m_LayersPendingAsyncDestroy.ReverseBegin();
		while(it != m_LayersPendingAsyncDestroy.ReverseEnd())
		{
			C_Layer* pLayer = *it;

			m_LayersPendingAsyncDestroy.Erase(it);

			layersPendingDestroy.PushBack(pLayer);

			it = m_LayersPendingAsyncDestroy.ReverseBegin();
		}
	}

	//TODO: Changed this to use a reverse iterator as there were layers depending on the existence of layers
	//created previously (i.e. the game layer releasing menu items from the debug layer). Destroying them in reverse 
	//order is more likely to work, but there still could be dependancies between layers. 

	Ut::UtList<C_Layer*>::ReverseIterator it = layersPendingDestroy.ReverseBegin();
	while(it != layersPendingDestroy.ReverseEnd())
	{
		C_Layer* pLayer = *it;

		SendMessageToLayer( pLayer, E_LayerMessage_Destroy );

		layersPendingDestroy.Erase(it);
		pLayer->SetLayerState( E_LayerState_Constructed );
		SAFE_DELETE_PTR( pLayer );

		didSomething = true;

		it = layersPendingDestroy.ReverseBegin();
	}

	return didSomething;
}


void C_LayerManager::EraseLayer( C_Layer* pLayer )
{
	C_LayerList::Iterator it = m_Layers.Begin();
	while(it != m_Layers.End())
	{
		const C_LayerItem layerItem = *it;
		if( layerItem.GetLayerPtr() == pLayer )
		{
			m_Layers.Erase( layerItem );
			return;
		}

		it++;
	}

	assert( 0 && "EraseLayer: layer not found" );
}

void C_LayerManager::CreatePendingLayers()
{
	Ut::UtList<C_Layer*>::Iterator it = m_LayersPendingCreate.Begin();
	while(it != m_LayersPendingCreate.End())
	{
		C_Layer* pLayer = *it;

		m_LayersPendingCreate.Erase(it);

		HRESULT hr = SendMessageToLayer( pLayer, E_LayerMessage_Create );
		if( FAILED(hr) )
		{
			LOGWARN( g_Log, "Failed to create layer" );
		}

		{
			MT::AutoLock autoLock(m_ReadyListLock);

			m_ReadyLayers.PushBack( pLayer );
			pLayer->SetLayerState( E_LayerState_Ready );
		}

		it = m_LayersPendingCreate.Begin();
	}
}

bool C_LayerManager::CreatePendingAsyncLayers()
{
	bool didSomething = false;

	Ut::UtList<C_Layer*> layersPendingCreate;

	{
		MT::AutoLock autoLock(m_AsyncCreateListLock);
	
		// copy list of layers pending create
		Ut::UtList<C_Layer*>::Iterator it = m_LayersPendingAsyncCreate.Begin();
		while(it != m_LayersPendingAsyncCreate.End())
		{
			C_Layer* pLayer = *it;
			m_LayersPendingAsyncCreate.Erase(it);

			layersPendingCreate.PushBack(pLayer);

			it = m_LayersPendingAsyncCreate.Begin();
		}
	}

	Ut::UtList<C_Layer*>::Iterator it = layersPendingCreate.Begin();
	while(it != layersPendingCreate.End())
	{
		C_Layer* pLayer = *it;

		layersPendingCreate.Erase(it);

		HRESULT hr = SendMessageToLayer( pLayer, E_LayerMessage_Create );
		if( FAILED(hr) )
		{
			LOGWARN( g_Log, "Failed to create layer" );
		}

		{
			MT::AutoLock autoLock(m_ReadyListLock);

			m_ReadyLayers.PushBack( pLayer );
			pLayer->SetLayerState( E_LayerState_Ready );
		}


		didSomething = true;

		it = layersPendingCreate.Begin();
	}

	return didSomething;
}

void C_LayerManager::StartReadyLayers()
{
	MT::AutoLock autoLock(m_ReadyListLock);

	Ut::UtList<C_Layer*>::Iterator it = m_ReadyLayers.Begin();
	while(it != m_ReadyLayers.End())
	{
		C_Layer* pLayer = *it;

		m_ReadyLayers.Erase(it);

		m_Layers.Insert( C_LayerItem(pLayer) );
		pLayer->SetLayerState( E_LayerState_Running );

		it = m_ReadyLayers.Begin();
	}
}

HRESULT C_LayerManager::SendMessageToLayers(const U32 message)
{
	HRESULT hr = S_OK;

	// layers list can only be modified in the main thread

	C_LayerList::Iterator it = m_Layers.Begin();
	while(it != m_Layers.End())
	{
		C_LayerItem layerItem = *it;
		C_Layer* pLayer = layerItem.GetLayerPtr();
		it++;

		SendMessageToLayer(pLayer, (E_LayerMessage)message);
	}

	return hr;
}

HRESULT C_LayerManager::SendMessageToLayer(C_Layer* pLayer, const U32 message)
{
	// filter out all but create and destroy messages if layer is not running or disabled
	if((pLayer->GetLayerState() != E_LayerState_Running)
	    || (!pLayer->m_Enabled))
	{
		if(	( message != E_LayerMessage_Create ) &&
			( message != E_LayerMessage_Destroy ) &&
			( message != E_LayerMessage_Resize ) )
			return S_OK;
	}

	pLayer->ProcessMessage((E_LayerMessage)message);

	return S_OK;
}

void C_LayerManager::Pause( const S32 pauseId )
{
	m_PauseIdList.Insert( pauseId );
}

void C_LayerManager::Resume( const S32 pauseId )
{
	m_PauseIdList.Erase( pauseId );
}

bool C_LayerManager::IsPaused() const
{
	return (m_PauseIdList.Size() > 0);
}

bool C_LayerManager::IsPaused( const S32 pauseId ) const
{
	Ut::UtSet<S32>::ConstIterator it = m_PauseIdList.Begin();
	while( it != m_PauseIdList.End() )
	{
		S32 id = *it;
		if( id == pauseId )
			return true;
		++it;
	}

	return false;
}

}; // namespace Framework
