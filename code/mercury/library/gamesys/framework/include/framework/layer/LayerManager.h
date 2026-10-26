#ifndef FRAMEWORK_LAYER_MANAGER_H
#define FRAMEWORK_LAYER_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Framework/Application/ApplicationTypes.h"
#include "core/threads/Mutex.h"

namespace Framework
{

class C_Layer;
class C_LayerItem;
class C_LayerManager;

typedef Ut::UtSet<C_LayerItem> C_LayerList;

// C_LayerManagerThread
class C_LayerManagerThread : public MT::Thread
{
public:
	C_LayerManagerThread(C_LayerManager* pLayerManager);

	~C_LayerManagerThread();

	void        ThreadMain();

	void		SendQuitMessage();

	bool		ShouldQuit();

private:
	C_LayerManager* m_pLayerManager;
	MT::Event		m_EventSignalQuit;	
};


//! C_LayerItem
class C_LayerItem
{
public:
	C_LayerItem();
	C_LayerItem(C_Layer* pLayer);
	~C_LayerItem();

    C_Layer* GetLayerPtr() const;

	bool operator < (const C_LayerItem& other) const;

private:
	C_Layer* m_pLayer;
};

//! C_LayerManager

class C_LayerManager
{
friend class C_LayerManagerThread;
public:
	C_LayerManager();
	~C_LayerManager();

	void ShutDown();

	//! Inserts a layer into the application.
	void InsertLayer(C_Layer* pLayer);

	//! Removes a layer from the application.
	void DestroyLayer(C_Layer* pLayer);

	//! Inserts a layer into the application.
	void InsertLayerAsync(C_Layer* pLayer);

	//! Removes a layer from the application.
	void DestroyLayerAsync(C_Layer* pLayer);

	// Creates and destroys layers.
	void Update();

	// Creates and destroys layers.
	bool UpdateAsync();

	HRESULT SendMessageToLayers(const U32 message);

	HRESULT SendMessageToLayer(C_Layer* pLayer, const U32 message);

	void Pause( const S32 pauseId );

	void Resume( const S32 pauseId );

	bool IsPaused( const S32 pauseId ) const;

	bool IsPaused() const;

private:

	void CreatePendingLayers();

	void DestroyPendingLayers();

	void StartReadyLayers();

	bool CreatePendingAsyncLayers();

	bool DestroyPendingAsyncLayers();

	void DestroyAllLayers();

	void EraseLayer( C_Layer* pLayer );
	
	void SetStatusDirty();

	void WaitForStatusDirty();

private:
	Ut::UtSet<S32> m_PauseIdList;
	C_LayerList m_Layers;
	Ut::UtList<C_Layer*> m_LayersPendingCreate;
	Ut::UtList<C_Layer*> m_LayersPendingAsyncCreate;

	Ut::UtList<C_Layer*> m_ReadyLayers;

	Ut::UtList<C_Layer*> m_LayersPendingDestroy;
	Ut::UtList<C_Layer*> m_LayersPendingAsyncDestroy;

	MT::Event m_StatusDirtyEvent;
	MT::Lock m_AsyncCreateListLock;
	MT::Lock m_AsyncDestroyListLock;
	MT::Lock m_ReadyListLock;

	C_LayerManagerThread m_LayerManagerThread;
};

}; // namespace Framework

#endif // FRAMEWORK_LAYER_MANAGER_H
