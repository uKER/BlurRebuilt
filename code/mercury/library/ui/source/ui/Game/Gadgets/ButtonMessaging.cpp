#include "stdafx.h"

#include <misc_shared/Messaging/Messaging.h>

#include "ui/Game/Gadgets/ButtonController.h"
#include "ui/Game/Gadgets/ButtonMessaging.h"

namespace UI
{

// Store
void C_DeferredUiMessageStore::SetMessage(const Messaging::C_MessageBase* pMessage)
{
	if (pMessage != GetMessage())
	{
		SAFE_DELETE_ARY(m_pMessageData);

		if (pMessage != NULL) // TODO : pool this too?
		{
			m_pMessageData = new U8[pMessage->Size()];
			memcpy(m_pMessageData, pMessage, pMessage->Size());
		}
	}
}

// Queue
C_DeferredUiMessageQueue::C_DeferredUiMessageQueue()
{
}

C_DeferredUiMessageQueue::~C_DeferredUiMessageQueue()
{
}

const U32 C_DeferredUiMessageQueue::ProcessMessages( C_ButtonController* pController )
{
	PERF_EVENT("UI.UiProcessMessages");
	assert(pController != NULL);

	U32 noMessages;
	for( noMessages = 0; !m_MessageFIFO.Empty(); ++noMessages )
	{
		C_DeferredUiMessageStore* pStore = &m_MessageFIFO.Front();
		m_MessageFIFO.PopFront();

		if( pStore->IsBroadcast() )
			pController->BroadcastMessageNow( pStore->GetBroadcastMask(), pStore->GetMessage() );
		else
			pController->SendMessageNow( pStore->GetDestButton(), pStore->GetMessage() );

		m_MessagePool.Free( pStore );
	}

	return noMessages;
}

}; // namespace UI
