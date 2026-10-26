// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdExample/bdExampleCommonLobby/bdExampleCommonLobby.h>

void BD_CALL IndexStrings(const bdNChar8 *const strings[],
						  const bdUInt numStrings,
						  const bdUInt index,
						  bdString& dest)
{
	if(index < numStrings)
	{
		dest = strings[index];
	}
	else
	{
		dest = "String index out of range";
	}
}

void BD_CALL LobbyErrorCodeToString(const bdLobbyErrorCode code,
									bdNChar8 *const buffer,
									const bdUWord bufferSize)
{
// set up the macros to create an array of enum values
#define BD_LOBBY_ERROR_CODES_BEGIN	bdLobbyErrorCode lookup[] = {
#define BD_LOBBY_ERROR_CODE(c, v)	c,
#define BD_LOBBY_ERROR_CODES_END(c)	c };

// include the error values
#include <bdLobby/bdLobbyErrorCodes.h>

#undef BD_LOBBY_ERROR_CODES_BEGIN
#undef BD_LOBBY_ERROR_CODE
#undef BD_LOBBY_ERROR_CODES_END

// set up the macros to create an array of strings
#define BD_LOBBY_ERROR_CODES_BEGIN	const bdNChar8* lobbyErrorCodeDescs[] = {
#define BD_LOBBY_ERROR_CODE(c, v)	#c,
#define BD_LOBBY_ERROR_CODES_END(c)	#c };

// include the error values
#include <bdLobby/bdLobbyErrorCodes.h>

#undef BD_LOBBY_ERROR_CODES_BEGIN
#undef BD_LOBBY_ERROR_CODE
#undef BD_LOBBY_ERROR_CODES_END

	const bdUInt numCodes = sizeof(lobbyErrorCodeDescs) / sizeof(bdNChar8*);

	// find the index of the code in the lookup array
	const bdNChar8 *desc = "Unknown bdLobbyErrorCode";

	bdBool found = false;
	for(bdUInt i = 0; !found && (i < numCodes); i++)
	{
		if(lookup[i] == code)
		{
			desc = lobbyErrorCodeDescs[i];
			found = true;
		}
	}

	bdStrlcpy(buffer, desc, bufferSize);
}


#ifndef BD_PLATFORM_XENON

TaskStatus BD_CALL StartGetHostByName(const bdNChar8 *const domainNameStr,
									  bdGetHostByName **getHostByName,
									  const bdInetAddr resolvedAddr)
{
	// fail safe
	TaskStatus taskStatus = FAILED;

	// is the address already resolved
	if(!resolvedAddr.isValid())
	{
		// DNS lookup objects are transitory so we create them on the heap
		bdGetHostByName *getHostByNamePtr = BD_NULL;

		if(*getHostByName == BD_NULL)
		{
			*getHostByName = new bdGetHostByName();
			getHostByNamePtr = *getHostByName;
		}

		if(getHostByNamePtr)
		{
			bdGetHostByNameConfig config;
			config.m_timeout = DNS_LOOKUP_TIMEOUT;
			if(getHostByNamePtr->start(domainNameStr, config))
			{
				taskStatus = PENDING;
			}
		}
	}
	else
	{
		taskStatus = DONE;
	}

	return taskStatus;
}

TaskStatus BD_CALL PumpGetHostByName(bdGetHostByName **getHostByName,
									 bdInetAddr &resolvedAddr)
{
	// fail safe
	TaskStatus taskStatus = FAILED;

	if(getHostByName)
	{
		bdGetHostByName *const getHostByNamePtr = *getHostByName;
		if(getHostByNamePtr)
		{
			const bdGetHostByName::bdStatus getHostByNameStatus = getHostByNamePtr->getStatus();

			// there is no need for a switch here, but with it in the compiler will
			// warn us if any new states are added to bdGetHostByName::Status
			switch(getHostByNameStatus)
			{
				case bdGetHostByName::BD_LOOKUP_PENDING:
				{
					// wait
					getHostByNamePtr->pump();
					taskStatus = PENDING;
				}
				break;
				case bdGetHostByName::BD_LOOKUP_SUCCEEDED:
				{
					// finished
					const bdBool addrResolved = getHostByNamePtr->getNumAddresses() > 0;
					if(addrResolved)
					{
						const bdInAddr inAddr = getHostByNamePtr->getAddressAt(0);
						resolvedAddr.set(inAddr);
					}

					if(addrResolved && resolvedAddr.isValid())
					{
						taskStatus = DONE;
					}
				}
				break;
				case bdGetHostByName::BD_LOOKUP_UNINITIALIZED:
				case bdGetHostByName::BD_LOOKUP_FAILED:
				case bdGetHostByName::BD_LOOKUP_CANCELLED:
				case bdGetHostByName::BD_LOOKUP_TIMED_OUT:
				case bdGetHostByName::BD_LOOKUP_ERROR:
				{
					// error conditions
					taskStatus = FAILED;
				}
				break;
			}

			if((taskStatus == FAILED) || (taskStatus == DONE))
			{
				getHostByNamePtr->quit();
				delete getHostByNamePtr;
				*getHostByName = BD_NULL;
			}
		}
	}

	return taskStatus;
}
#endif //#ifndef BD_PLATFORM_XENON


TaskStatus BD_CALL CheckRemoteTaskStatus(bdRemoteTaskRef remoteTask)
{
	// fail safe
	TaskStatus status = FAILED;

	if(remoteTask)
	{
		const bdRemoteTask::bdStatus remoteTaskStatus = remoteTask->getStatus();
		switch(remoteTaskStatus)
		{
			case bdRemoteTask::BD_EMPTY:
			case bdRemoteTask::BD_TIMED_OUT:
			{
				status = FAILED;
			}
			break;
			case bdRemoteTask::BD_PENDING:
			{
				status = PENDING;
			}
			break;
			case bdRemoteTask::BD_DONE:
			{
				status = DONE;
			}
			break;
			default:
				break;
		}
	}
	return status;
}

void BD_CALL CloseRemoteTask(bdRemoteTaskRef remoteTask)
{
	if(remoteTask)
	{
		remoteTask = BD_NULL;
	}
}


void BD_CALL printInfo(const bdChannelInfo& channelInfo,
					   bdNChar8 *buf,
					   const bdUInt length)
{
	bdSnprintf(buf, length, "%u (%u/%u)", channelInfo.m_channelID, channelInfo.m_numSubscribers, channelInfo.m_maxSubscribers);
}

void BD_CALL printInfo(const bdChannelMember& memberInfo,
					   bdNChar8 *buf,
					   const bdUInt length)
{
	bdStrlcpy(buf, memberInfo.m_userName, length);
}

