#ifndef UTIL_RESOURCE_LISTENER_H
#define UTIL_RESOURCE_LISTENER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Core.h>
#include "resource\ResourceTypes.h"

//!
//! @addtogroup Resource
//! @{

namespace Resource
{

class C_Sender;
class C_Listener;
class C_SenderListenerItem;
class C_ResourceBase;

//! C_Listener. Inherit from this class and register with a resource to recieve load / evict notification messages from resource system.
//! Listeners can be added and removed from resources using AddListener() and RemoveListener(). 
//! If the listener or resource are destroyed the listener will be removed from the resource.
//! The listener's ResourceNotify method will be called when the Resource's data state changes
//! or immediately when the listener is added to the resource if the resources data is ready at this time. 
class C_Listener
{
friend class C_SenderListenerItem;
friend class C_Manager;

public:
	C_Listener();
	virtual ~C_Listener();

	//! Provide this method in the derived class. This will be called when the resource's data state changes.
	virtual void ResourceNotify( C_ResourceBase* pResource, const E_Notify result, void* pData ) = 0;

private:
	C_SenderListenerItem* GetListenerFirstSender() const;

private:
	C_SenderListenerItem*	m_pListenerFirstSender;
};

// C_SenderListenerItem. Internal class for resource / listener data structure.
class C_SenderListenerItem
{
friend class C_Listener;
friend class C_Sender;
friend class C_Manager;

private:
	C_SenderListenerItem( C_Sender*	pSender, C_Listener* pListener, void* pData );
	~C_SenderListenerItem();

	C_Sender* GetSender() const;
	C_Listener* GetListener() const;
	void* GetData() const;

	C_SenderListenerItem* GetListenerNextSender() const;
	C_SenderListenerItem* GetListenerPrevSender() const;
	C_SenderListenerItem* GetSenderNextListener() const;
	C_SenderListenerItem* GetSenderPrevListener() const;

private:
	C_Sender*	m_pSender;
	C_Listener* m_pListener;
	void*		m_pData;

	C_SenderListenerItem*	m_pListenerNextSender;
	C_SenderListenerItem*	m_pListenerPrevSender;

	C_SenderListenerItem*	m_pSenderNextListener;
	C_SenderListenerItem*	m_pSenderPrevListener;
};

// C_Sender. Resource inherits from this class.
// Internal class for resource / listener data structure.
class C_Sender
{
friend class C_SenderListenerItem;
friend class C_Manager;

public:
	C_Sender();
	virtual ~C_Sender();

protected:
	void SenderAddListener( C_Listener* pListener, void* pData = NULL );
	S32 SenderRemoveListener( C_Listener* pListener );

	C_SenderListenerItem* GetSenderFirstListener() const;

	void SendResourceNotify( C_ResourceBase* pResource, const E_Notify notify );

private:
	C_SenderListenerItem*	m_pSenderFirstListener;
};

} // namespace Resource

//! @}

#endif // UTIL_RESOURCE_LISTENER_H
