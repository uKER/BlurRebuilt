// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: XENON
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>

#include <bdNet/bdNet.h>

#include <Xonline.h>
#include <bdExample/bdExampleCommonLobby/LiveServices.h>

#ifndef XONLINE_LSP_ATTR_TSADDR
#define XONLINE_LSP_ATTR_TSADDR 0x80200001
#endif // XONLINE_LSP_ATTR_TSADDR

#ifndef XONLINE_LSP_ATTR_XNKID
#define XONLINE_LSP_ATTR_XNKID 0x80200002
#endif // XONLINE_LSP_ATTR_XNKID

#ifndef XONLINE_LSP_ATTR_KEK
#define XONLINE_LSP_ATTR_KEK 0x80200003
#endif // XONLINE_LSP_ATTR_KEK

#ifndef XONLINE_LSP_DEFAULT_DATASET_ID
#define XONLINE_LSP_DEFAULT_DATASET_ID 0x0000AAAA
#endif // XONLINE_LSP_DEFAULT_DATASET_ID

#define USERDATA_LEN (256)

// This must be a value between 1 & XTITLE_SERVER_MAX_LSP_INFO [100]

#define MAX_LSP_INFO_TO_RETRIEVE 50u
BD_COMPILE_ASSERT(((MAX_LSP_INFO_TO_RETRIEVE >= 1) &&
				   (MAX_LSP_INFO_TO_RETRIEVE <= XTITLE_SERVER_MAX_LSP_INFO)) ,
				   BAD_VALUE_USED_FOR_NUM_LSP_INFOS);

#define BD_LOG_LEVEL "bdExampleCommon/LiveServices"

LiveServices::LiveServices()
: m_signedOn(false),
  m_notifyListener(BD_NULL)
{
}

bdBool LiveServices::signOn(bdUInt32)
{
	m_notifyListener = XNotifyCreateListener(XNOTIFY_LIVE);
	BD_ASSERT(m_notifyListener != BD_NULL, "Failed to create Live events listener.");
	return true;
}

bdBool LiveServices::pump()
{
	bdBool result = true;

	DWORD notificationId;
	ULONG_PTR param;

	if (XNotifyGetNext(m_notifyListener, XN_LIVE_CONNECTIONCHANGED, &notificationId, &param))
	{
		if (notificationId == XN_LIVE_CONNECTIONCHANGED)
		{
			HRESULT hr = static_cast<HRESULT>(param);
			if (XONLINE_S_LOGON_CONNECTION_ESTABLISHED == hr)
			{
				m_signedOn = true;
			}
			else if (XONLINE_E_INVALID_TITLE_ID == hr)
			{
				BD_ERR(BD_LOG_LEVEL , "Invalid TitleID specified..");
				result = false;
			}
			else if(XONLINE_S_LOGON_DISCONNECTED == hr)
			{
				BD_ERR(BD_LOG_LEVEL , "Logon Disconnected " );
				result = false;
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL , "Error encountered while logging on : % u" , hr);
				result = false;
			}
		}
	}

	return result;
}

bdBool LiveServices::isSignedOn()
{
	DWORD dwUserIndex(0);
		
	if (XUserGetSigninState(dwUserIndex) == eXUserSigninState_SignedInToLive)
	{
		m_signedOn = true;
	}

	return m_signedOn;
}

bdBool LiveServices::resolveTitleAddr(bdUInt32 titleServerID,
									  bdUInt16 port,
									  bdAddr &addr,
									  bdSecurityID&,
									  bdSecurityKey&)
{
	bdBool result = true;

	XTITLE_SERVER_INFO resultsa[MAX_LSP_INFO_TO_RETRIEVE];
	XONLINE_SERVICE_INFO info;
	HANDLE serverEnumHandle = BD_NULL;
	DWORD serverCount = 0;
	IN_ADDR secureAddr;
	// Setup an initial value.
	secureAddr.S_un.S_addr = 0;
	DWORD itemBufferRequiredSize = 0;

	const DWORD res = XOnlineGetServiceInfo(titleServerID, &info);

	if(ERROR_SUCCESS == res)
	{
		//A successful connection to the service has been made. 
		BD_INFO(BD_LOG_LEVEL , "Service info successfully retrieved.");
	}
	else if(ERROR_FUNCTION_FAILED == res)
	{
		//An unspecified error occurred. 
		//For example, the specified service does not exist in the service database. 
		BD_ERR(BD_LOG_LEVEL , "Unspecified error occurred");
		result = false;
	}
	else if (ERROR_SERVICE_NOT_FOUND  == res)
	{
		BD_ERR(BD_LOG_LEVEL , "The service was not configured for this title.");
		result = false;
	}
	else if (ERROR_CONNECTION_INVALID == res)
	{
		BD_ERR(BD_LOG_LEVEL , "There is currently no Xbox Live connection.");
		result = false;
	}
	else
	{
		//error unexpected result code...
		BD_ERR(BD_LOG_LEVEL , "Undocumented return code while getting service info : 0x%0x." , res);
		result = false;
	}

	if(true == result)
	{

		ZeroMemory(resultsa, sizeof(resultsa));

		// This parameter is used to ensure only specific LSP servers 
		// for the title are returned. Using an empty string results in all
		// entries being returned.
		//bdNChar8* serverDesc = "DW LSP EXAMPLE STRING DESC";
		bdNChar8* serverDesc = "";

		// Create a title server enumerator
		const DWORD createResult = XTitleServerCreateEnumerator(serverDesc, MAX_LSP_INFO_TO_RETRIEVE, &itemBufferRequiredSize, &serverEnumHandle);
		if(ERROR_SUCCESS == createResult)
		{
			BD_INFO(BD_LOG_LEVEL , "Title Server enumerator successfully created.");
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Failed to create a server query: 0x%0x.", createResult);
			result = false;
			CloseHandle(serverEnumHandle);
		}
	}

	if(true == result)
	{
		if(sizeof(resultsa) >= itemBufferRequiredSize)
		{
			// Enumerate the title servers
			const DWORD enumerateResult = XEnumerate(serverEnumHandle, resultsa, sizeof(resultsa), &serverCount, NULL);
			if(ERROR_SUCCESS == enumerateResult)
			{
				BD_INFO(BD_LOG_LEVEL , "Title Servers successfully enumerated.");
				// We should find at least one server
				if (serverCount < 1)
				{
					BD_ERR(BD_LOG_LEVEL, "Got no results for LSP servers query.");
					result = false;
				}
			}
			else if (ERROR_NO_MORE_FILES == enumerateResult)
			{
				BD_ERR(BD_LOG_LEVEL, "No more files ???");
				result = false;
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Failed to enumerate servers: 0x%0x.", result);
				result = false;
			}
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Insufficient memory allocated, cannot safely call XEnumerate:  Mem req: %u Bytes, allocated %u Bytes", itemBufferRequiredSize , sizeof(resultsa));
			result = false;
		}

		CloseHandle(serverEnumHandle);
	}

	if(true == result)
	{
		// Choose one of the returned SG-Lites at random from the list.
		// This facilitates load balancing.
		bdRandom r;
		const bdUInt randomIndex = ((r.nextUInt())%serverCount);
		BD_INFO(BD_LOG_LEVEL , "Found %u server(s), Connecting to server at index: %u." , serverCount, randomIndex);

		const INT convResult = XNetServerToInAddr(resultsa[randomIndex].inaServer, titleServerID, &secureAddr);
		if (0 == convResult )
		{
			BD_INFO(BD_LOG_LEVEL , "Server addr converted OK.");
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "Failed to convert IP addr to secure IP addr");
			result = false;
		}
	}

	if(true == result)
	{
		// Now we can construct server's addr.
		addr = bdAddr(bdInetAddr(secureAddr.S_un.S_addr), port);
	}

	return result;
}

bdUInt64 LiveServices::getXUserId()
{
	bdUInt64 onlineUserId;
	BD_ASSERT(XUserGetXUID(0, &onlineUserId) == ERROR_SUCCESS, "Cannot get XUID.");

	return onlineUserId;
}

void LiveServices::getXUserName(bdNChar8 buffer[BD_MAX_USERNAME_LENGTH])
{
	BD_ASSERT(XUserGetName(0, buffer, BD_MAX_USERNAME_LENGTH) == ERROR_SUCCESS, "Cannot get username");
}


