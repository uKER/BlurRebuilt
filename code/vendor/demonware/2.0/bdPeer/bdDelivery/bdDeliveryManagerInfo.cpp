// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdDelivery/bdDeliveryManagerInfo.h>

#include <bdPeer/bdDelivery/bdDeliveryManager.h>

#define BD_DELIVERY_RECOVERY_SIZE (deliveryManager->m_messageLimit)
#define BD_DELIVERY_WINDOW_NUM_MESSAGES ((2*deliveryManager->m_messageLimit)+1)
#define BD_DELIVERY_WINDOW_SIZE (BD_DELIVERY_RECOVERY_SIZE + BD_DELIVERY_WINDOW_NUM_MESSAGES)

bdUInt BD_CALL bdDeliveryManagerInfo::getInfo(const bdDeliveryManager *const deliveryManager,
											  bdNChar8 *buf,
											  const bdUInt length)
{
	bdNChar8 *const end = buf + length;	
	
	const bdNChar8 *const statusNotJoined = "BD_DELIVERY_NOT_JOINED";
	const bdNChar8 *const statusJoining = "BD_DELIVERY_JOINING";
	const bdNChar8 *const statusJoined = "BD_DELIVERY_JOINED";
	const bdNChar8 *const statusActive = "BD_DELIVERY_ACTIVE";
	const bdNChar8 *const statusUnknown = "BD_DELIVERY_UNKNOWN";

	const bdNChar8* deliveryStatus;
	switch(deliveryManager->m_status)
	{
		case bdDeliveryManager::BD_DELIVERY_NOT_JOINED:
		{
			deliveryStatus = statusNotJoined;
		}
		break;
		case bdDeliveryManager::BD_DELIVERY_JOINING:
		{
			deliveryStatus = statusJoining;
		}
		break;
		case bdDeliveryManager::BD_DELIVERY_JOINED:
		{
			deliveryStatus = statusJoined;
		}
		break;
		case bdDeliveryManager::BD_DELIVERY_ACTIVE:
		{
			deliveryStatus = statusActive;
		}
		break;
		default:
		{
			deliveryStatus = statusUnknown;
		}
	}
	
	const bdUInt outgoingMsgNum = deliveryManager->m_outgoingMsgNum;
	const bdUInt dispatchNum = deliveryManager->m_dispatchNum;
	const bdUInt numActivePeers = deliveryManager->m_activePeers.getSize();
	const bdUInt numJoinedPeers = deliveryManager->m_joinedPeers.getSize();

	const bdInt written = bdSnprintf(buf, static_cast<bdUInt>(end - buf), 
					"%u message num\n"
					"%u dispatch num\n"
					"%s (%u/%u)\n",
					outgoingMsgNum, dispatchNum, deliveryStatus, numActivePeers, numJoinedPeers);
	if(written > 0 && written <= static_cast<bdInt>(end - buf))
	{
		buf += written;
	}
	else
	{
		buf = end - 1;
		if(length > 0)
		{
			*buf = '\0';
		}			
	}

	const bdUInt dispatchIndex = dispatchNum % BD_DELIVERY_WINDOW_SIZE;
	for(bdUInt i = 0; i < BD_DELIVERY_WINDOW_SIZE; i++)
	{
		bdDeliveryManager::bdMessageBatch& batch = deliveryManager->m_messageWindow[(dispatchIndex + i) % BD_DELIVERY_WINDOW_SIZE];

		const bdNChar8 *const comma = ",";
		const bdNChar8 *const openBracket = "[";
		const bdNChar8 *const closeBracket = "]\n";
		const bdNChar8 *const empty = "";

		const bdNChar8 *post = comma;
		const bdNChar8 *pre = empty;
		if(i == BD_DELIVERY_WINDOW_SIZE-1)
		{
			post = closeBracket;
		}
		if(i == 0)
		{
			pre = openBracket;
		}

		const bdInt numWritten = bdSnprintf(buf, static_cast<bdUInt>(end - buf), "%s%u%s", pre, batch.getSize(), post);
		if(numWritten > 0 && numWritten <= static_cast<bdInt>(end - buf))
		{
			buf += numWritten;
		}
		else
		{
			buf = end - 1;
			if(length > 0)
			{
				*buf = '\0';
			}			
		}
	}

	// return num bytes written (not including the null terminator)
	const bdUInt bytesWritten = length - (end - buf);
	return bytesWritten;
}

bdDeliveryManagerInfo::bdDeliveryManagerInfo()
{
}
