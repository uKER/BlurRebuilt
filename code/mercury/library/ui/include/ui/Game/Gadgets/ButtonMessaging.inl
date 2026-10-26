#ifndef BUTTON_MESSAGING_INL
#error ButtonMessaging.inl should only be included from ButtonMessaging.h
#endif

namespace UI
{

// Store
inline C_DeferredUiMessageStore::C_DeferredUiMessageStore( ) : 
	m_Dest( -1 ),
	m_pMessageData( NULL ), 
	m_ComponentOrMask( 0 )
{

}

inline C_DeferredUiMessageStore::~C_DeferredUiMessageStore( )
{
	if( m_pMessageData != NULL )
		SetMessage( NULL );
}

inline void C_DeferredUiMessageStore::Init(const C_ButtonHandle &dest, const Messaging::C_MessageBase *pMessage)
{
	m_Dest = dest;
	SetMessage(pMessage);
	assert(!IsBroadcast());
}

inline void C_DeferredUiMessageStore::Init(const U32 mask, const Messaging::C_MessageBase* pMessage)
{
	m_Dest = 0;//C_ButtonHandle();
	m_ComponentOrMask = static_cast<U32>(mask);
	SetMessage(pMessage);
	assert(IsBroadcast());
}

inline const bool C_DeferredUiMessageStore::IsBroadcast() const
{
	return (static_cast<U32>(m_Dest) == 0);
}

inline const U32 C_DeferredUiMessageStore::GetBroadcastMask() const
{
	return m_ComponentOrMask;
}

inline const C_ButtonHandle& C_DeferredUiMessageStore::GetDestButton() const
{
	return m_Dest;
}

inline const Messaging::C_MessageBase* C_DeferredUiMessageStore::GetMessage() const 
{
	return reinterpret_cast< const Messaging::C_MessageBase* >( m_pMessageData );
}

// Queue
inline const bool C_DeferredUiMessageQueue::PushMessage(const C_ButtonHandle dest, const Messaging::C_MessageBase* pMessage)
{
	if ((static_cast<U32>(dest) != 0) && (pMessage != NULL))
	{
		C_DeferredUiMessageStore* pStore = m_MessagePool.Allocate();
		pStore->Init( dest, pMessage );
		m_MessageFIFO.PushBack( *pStore );

		return true;
	}

	return false;
}

inline const bool C_DeferredUiMessageQueue::BroadcastMessage(const Messaging::C_MessageBase* pMessage, const U32 mask)
{
	if (pMessage != NULL)
	{
		C_DeferredUiMessageStore* pStore = m_MessagePool.Allocate();
		pStore->Init(mask, pMessage);
		m_MessageFIFO.PushBack(*pStore);
		return true;
	}

	return false;
}

};
