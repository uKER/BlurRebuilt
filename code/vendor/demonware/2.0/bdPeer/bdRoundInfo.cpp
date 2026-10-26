// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPeer/bdRoundInfo.h>

#include <bdPeer/bdRound.h>

bdUInt BD_CALL bdRoundInfo::getInfo(const bdRound *const round,
									bdNChar8 *buf,
									const bdUInt length)
{
	
	bdNChar8 *const end = buf + length;

	const bdNChar8 *const waiting = "BD_WAITING";
	const bdNChar8 *const inLobby = "BD_IN_LOBBY";
	const bdNChar8 *const inRound = "BD_IN_ROUND";
	const bdNChar8 *const updating = "BD_UPDATING";
	const bdNChar8 *const unknown = "**UNKNOWN**";

	const bdNChar8 *status;

	switch(round->getStatus())
	{
		case bdRound::BD_WAITING:
		{
			status  = waiting;
		}
		break;
		case bdRound::BD_IN_LOBBY:
		{
			status  = inLobby;
		}
		break;
		case bdRound::BD_IN_ROUND:
		{
			status  = inRound;
		}
		break;
		case bdRound::BD_UPDATING:
		{
			status  = updating;
		}
		break;
		default:
		{
			status  = unknown;			
		}
	}

	bdInt written = bdSnprintf(buf, static_cast<bdUInt>(end - buf), "Status: %s\n", status);
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

	// return num bytes written (not including the null terminator)
	const bdUInt bytesWritten = length - (end - buf);
	return bytesWritten;
}

bdRoundInfo::bdRoundInfo()
{
}
