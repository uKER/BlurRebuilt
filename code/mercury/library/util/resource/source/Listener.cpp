
#include "stdafx.h"

#include <resource/Resource.h>

namespace Resource
{

// C_Listener
C_Listener::C_Listener()
	: m_pListenerFirstSender( NULL )
{
}

C_Listener::~C_Listener()
{
	// remove all senders
	C_Manager::GetInstance()->MutexWait();

	C_SenderListenerItem* pCurrItem = GetListenerFirstSender();
	while(pCurrItem)
	{
		C_SenderListenerItem* pNextItem = pCurrItem->GetListenerNextSender();
		
		SAFE_DELETE_PTR( pCurrItem );

		pCurrItem = pNextItem;
	}

	C_Manager::GetInstance()->MutexRelease();
}

C_SenderListenerItem* C_Listener::GetListenerFirstSender() const
{
	return m_pListenerFirstSender;
}

// C_SenderListenerItem

C_SenderListenerItem::C_SenderListenerItem( C_Sender* pSender, C_Listener* pListener, void* pData )
	: m_pSender( pSender )
	, m_pListener( pListener )
	, m_pData( pData )
{
	// PushFirst into sender's list

	if ( pSender->m_pSenderFirstListener )
    {
        pSender->m_pSenderFirstListener->m_pSenderPrevListener = this;
        m_pSenderNextListener = pSender->m_pSenderFirstListener;
    }
	else
	{
		m_pSenderNextListener = NULL;
	}

	m_pSenderPrevListener = NULL;
    pSender->m_pSenderFirstListener = this;

	// PushFirst into listener's list

	if ( pListener->m_pListenerFirstSender )
    {
        pListener->m_pListenerFirstSender->m_pListenerPrevSender = this;
        m_pListenerNextSender = pListener->m_pListenerFirstSender;
    }
	else
	{
		m_pListenerNextSender = NULL;
	}

	m_pListenerPrevSender = NULL;
    pListener->m_pListenerFirstSender = this;

}

C_SenderListenerItem::~C_SenderListenerItem()
{
	// remove from sender's lists
	{
		if ( m_pSender->m_pSenderFirstListener == this )
		{
			m_pSender->m_pSenderFirstListener = m_pSenderNextListener;
		}

		C_SenderListenerItem* itemPrev = m_pSenderPrevListener;
		C_SenderListenerItem* itemNext = m_pSenderNextListener;
	    
		if ( itemPrev )
		{
			itemPrev->m_pSenderNextListener = itemNext;
		}
	    
		if ( itemNext )
		{
			itemNext->m_pSenderPrevListener = itemPrev;
		}
	}
    
	m_pSenderPrevListener = NULL;
	m_pSenderNextListener = NULL;

	// remove from listener's lists
	{
		if ( m_pListener->m_pListenerFirstSender == this )
		{
			m_pListener->m_pListenerFirstSender = m_pListenerNextSender;
		}

		C_SenderListenerItem* itemPrev = m_pListenerPrevSender;
		C_SenderListenerItem* itemNext = m_pListenerNextSender;
	    
		if ( itemPrev )
		{
			itemPrev->m_pListenerNextSender = itemNext;
		}
	    
		if ( itemNext )
		{
			itemNext->m_pListenerPrevSender = itemPrev;
		}
	    
		m_pListenerPrevSender = NULL;
		m_pListenerNextSender = NULL;
	}
}

C_Sender* C_SenderListenerItem::GetSender() const
{
	return m_pSender;
}

C_Listener* C_SenderListenerItem::GetListener() const
{
	return m_pListener;
}

void* C_SenderListenerItem::GetData() const
{
	return m_pData;
}

C_SenderListenerItem* C_SenderListenerItem::GetListenerNextSender() const
{
	return m_pListenerNextSender;
}

C_SenderListenerItem* C_SenderListenerItem::GetListenerPrevSender() const
{
	return m_pListenerPrevSender;
}

C_SenderListenerItem* C_SenderListenerItem::GetSenderNextListener() const
{
	return m_pSenderNextListener;
}

C_SenderListenerItem* C_SenderListenerItem::GetSenderPrevListener() const
{
	return m_pSenderPrevListener;
}

// C_Sender

C_Sender::C_Sender()
	: m_pSenderFirstListener( NULL )
{
}

C_Sender::~C_Sender()
{
	// remove all listeners

	C_Manager::GetInstance()->MutexWait();

	C_SenderListenerItem* pCurrItem = GetSenderFirstListener();
	while(pCurrItem)
	{
		C_SenderListenerItem* pNextItem = pCurrItem->GetSenderNextListener();
		
		SAFE_DELETE_PTR( pCurrItem );

		pCurrItem = pNextItem;
	}

	C_Manager::GetInstance()->MutexRelease();
}

void C_Sender::SenderAddListener( C_Listener* pListener, void* pData )
{
	C_Manager::GetInstance()->MutexWait();

	new C_SenderListenerItem( this, pListener, pData );

	C_Manager::GetInstance()->MutexRelease();
}

S32 C_Sender::SenderRemoveListener( C_Listener* pListener )
{
	S32 removedCount = 0;

	C_Manager::GetInstance()->MutexWait();

	C_SenderListenerItem* pCurrItem = GetSenderFirstListener();
	while(pCurrItem)
	{
		C_SenderListenerItem* pNextItem = pCurrItem->GetSenderNextListener();
		
		if(pCurrItem->GetListener() == pListener)
		{
			SAFE_DELETE_PTR( pCurrItem );
			removedCount++;
		}

		pCurrItem = pNextItem;
	}

	C_Manager::GetInstance()->MutexRelease();

	return removedCount;
}

C_SenderListenerItem* C_Sender::GetSenderFirstListener() const
{
	return m_pSenderFirstListener;
}

void C_Sender::SendResourceNotify( C_ResourceBase* pResource, const E_Notify notify )
{
	C_Manager::GetInstance()->MutexWait();

	C_SenderListenerItem* pCurrItem = GetSenderFirstListener();
	while(pCurrItem)
	{
		pCurrItem->GetListener()->ResourceNotify( pResource, notify, pCurrItem->GetData() );

		pCurrItem = pCurrItem->GetSenderNextListener();
	}

	C_Manager::GetInstance()->MutexRelease();
}

} // namespace Resource
