#include <stdafx.h>
#include <Core/core.h>
#include <bdNet/bdNet.h>
#include <bdCore/bdCore.h>
#include <Network/Core.h>
#include <Network/Securekey.h>
#include <bdCore/bdSocket/bdCommonAddr.h>
#include <system/time/time.h>
#include "NetJoinHelper.h"
#include "NetLogger.h"
#include "../Router/NetRouterBase.h"
#include "../Misc/NetDebug.h"

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	C_NetJoinHelper::C_NetJoinHelper()
	{
		m_JoinState = eJoinStateNotActive;
		m_LastJoinError = eJoinErrorInvalid;
		m_EnumeratedSessionIndexToJoin = 0;
		m_HostConsoleId = 0;
		m_PeerFailureNameValid = false;
	}

	C_NetJoinHelper::~C_NetJoinHelper()
	{
	}

	void C_NetJoinHelper::JoinPollFailures(C_NetRouterBase& router)
	{
		if (m_ConnectionFailures.Size() == 0)
			return;
		
		const U64 ticks = Time::GetPerformanceCounter(); // microseconds

		C_NetMessage& msg = router.GetOutgoingUnReliable();

		for (S32 i=0; i<m_ConnectionFailures.Size(); i++)
		{
			ConnectionFailure& data = m_ConnectionFailures[i];
			const F64 deltaTicks = (ticks - data.m_LastTick) / 1000.0f;

			// send join fail every 500ms
			if (deltaTicks >= 500.0f)
			{
				data.m_LastTick = ticks;
				data.m_Count +=1;

				// send join failed message.
				msg.Flush();
				if (msg.CreateUnReliable(data.m_MsgType, data.m_SenderId, data.m_PartyMessage))
				{
					// write failed reason only.
					bool ok = msg.WriteU8(data.m_FailCode);
					ok = ok && msg.WriteBuffer(&data.m_Name, sizeof(SysProfile::BaseProfile));

					msg.TerminateMessage();

					if (ok && data.m_Addr.notNull())
						router.SendMessage(msg, data.m_Addr);
				}
			}
		}

		// remove any failed connections that have been been active long enough.
		S32 size = m_ConnectionFailures.Size();
		S32 index = 0;
		for (S32 i=0; i<size; i++)
		{
			ConnectionFailure& data = m_ConnectionFailures[index];
			if (data.m_Count > 5)
			{
				router.Disconnect(data.m_Addr);
				data.m_Addr = BD_NULL;
				m_ConnectionFailures.EraseByIndex(index); 
			}
			else
				index += 1;
		}
	}

	void C_NetJoinHelper::JoinFailConnection(const U8 returnMsgType, const U8 reason, const U64 consoleId, const U8 senderId, bdAddrHandleRef& addr, C_NetRouterBase& router, bool partySession)
	{
		if (addr.isNull())
			return;

		if (m_ConnectionFailures.Size() == m_ConnectionFailures.Capacity())
		{
			// too many join failures to track so just disconnect.
			LOGWARN(g_Log, "C_NetJoinHelper::JoinFailConnection() - ran out of storage.");
			router.Disconnect(addr);
			addr = BD_NULL;
		}
		else
		{
			// dont add the same console more than once.
			bool add = true;
			for (S32 i=0; i<m_ConnectionFailures.Size(); i++)
			{
				if (consoleId == m_ConnectionFailures[i].m_ConsoleId)
					add = false;
			}

			if (add)
			{
				SysProfile::BaseProfile blank;
				ConnectionFailure f;
				f.m_MsgType			= returnMsgType;
				f.m_FailCode		= reason;
				f.m_Addr			= addr;
				f.m_Count			= 0;
				f.m_SenderId		= senderId;
				f.m_ConsoleId		= consoleId;
				f.m_PartyMessage	= partySession;
				f.m_LastTick		= 0;
				f.m_Name			= blank;	
				m_ConnectionFailures.PushBack(f);
			}
		}
	}

	void C_NetJoinHelper::JoinFailConnectionWithName(const U8 returnMsgType, const U8 reason, const SysProfile::BaseProfile& name, const U64 consoleId, const U8 senderId, bdAddrHandleRef& addr, C_NetRouterBase& router, bool partySession)
	{
		if (addr.isNull())
			return;

		if (m_ConnectionFailures.Size() == m_ConnectionFailures.Capacity())
		{
			// too many join failures to track so just disconnect.
			LOGWARN(g_Log, "C_NetJoinHelper::JoinFailConnection() - ran out of storage.");
			router.Disconnect(addr);
			addr = BD_NULL;
		}
		else
		{
			// dont add the same console more than once.
			bool add = true;
			for (S32 i=0; i<m_ConnectionFailures.Size(); i++)
			{
				if (consoleId == m_ConnectionFailures[i].m_ConsoleId)
					add = false;
			}

			if (add)
			{
				ConnectionFailure f;
				f.m_MsgType			= returnMsgType;
				f.m_FailCode		= reason;
				f.m_Addr			= addr;
				f.m_Count			= 0;
				f.m_SenderId		= senderId;
				f.m_ConsoleId		= consoleId;
				f.m_PartyMessage	= partySession;
				f.m_LastTick		= 0;
				f.m_Name			= name;	
				m_ConnectionFailures.PushBack(f);
			}
		}
	}

	bool C_NetJoinHelper::JoinIsConsoleFlaggedToFail(const U64 consoleId)
	{
		for (S32 i=0; i<m_ConnectionFailures.Size(); i++)
		{
			if (consoleId == m_ConnectionFailures[i].m_ConsoleId)
				return true;
		}
		return false;
	}

	void C_NetJoinHelper::FlushJoinHelper(const bool resetAll, C_NetRouterBase& router)
	{
		m_PeerFailureNameValid = false;
		m_JoinState = eJoinStateNotActive;
		m_HostConsoleId = 0;
		m_EnumeratedSessionIndexToJoin = 0;
		m_JoinTimer.Stop();
		m_ReSendTimer.Stop();

		// kill any connection failures we may have queued up.
		while(m_ConnectionFailures.Size())
		{
			if (m_ConnectionFailures[0].m_Addr.notNull())
				router.Disconnect(m_ConnectionFailures[0].m_Addr);

			m_ConnectionFailures[0].m_Addr = BD_NULL;
			m_ConnectionFailures.EraseByIndex(0);
		}

		// disconnect host address if its valid.
		if (m_HostAddress.notNull())
		{
			router.Disconnect(m_HostAddress);

			// remove reference if it still exists.
			m_HostAddress = BD_NULL;
		}

		if (resetAll)
			m_LastJoinError = eJoinErrorInvalid;			
	}

	bool C_NetJoinHelper::IsJoining(void) const
	{
		// check if we are actually flagged as joining .
		switch (m_JoinState)
		{
		case eJoinStateNotActive:
		case eJoinStateFailed:
		case eJoinStateComplete:
			return false;
			break;
		}

		return true;
	}

	bool C_NetJoinHelper::JoinStart(const NetPlay::SessionSecurity& security, const U8* pHostCommonAddress, const U64 hostConsoleId)
	{
		if (NULL == pHostCommonAddress)
			return false;

		bdNetImpl *const net = bdNet::getInstance();
		bdCommonAddrRef myCommonAddress = net->getLocalCommonAddr();
		if (myCommonAddress.isNull())
			return false;

		if (NULL == net->getSocketRouter())
			return false;

		m_PeerFailureNameValid = false;
		m_JoinState = eJoinStateWaitingForHostReply;
		m_LastJoinError = eJoinErrorInvalid;
		m_HostConsoleId = hostConsoleId;
		m_JoinTimer.Start();
		m_ReSendTimer.Start();

		U8 tempHostCommon[BD_COMMON_ADDR_SERIALIZED_SIZE];	
		for (U32 i=0; i<BD_COMMON_ADDR_SERIALIZED_SIZE; i++)
			tempHostCommon[i] = pHostCommonAddress[i];

		// create hosts secure ip address so we can attempt to connect.
		bdCommonAddrRef host = new bdCommonAddr();
		if (host->deserialize(myCommonAddress, tempHostCommon))
		{
			bool ok = false; 
			if (net->getSocketRouter()->getAddressMap()->getAddrHandle(host, security.m_Id, m_HostAddress))
			{
				ok = net->getSocketRouter()->connect(m_HostAddress);
				if (!ok)
				{
					net->getSocketRouter()->disconnect(m_HostAddress);
					m_HostAddress = BD_NULL;
				}
			}
			else
				m_HostAddress = BD_NULL;
			host = BD_NULL;
			return ok;
		}
	
		host = BD_NULL;
		m_HostAddress = BD_NULL;
		return false;
	}

	// connection to host has been completed, now connecting to peers
	bool C_NetJoinHelper::JoinEnterPeerConnectionPhase(void)
	{
		if (eJoinStateWaitingForPeerData == m_JoinState)
		{
			m_JoinState = eJoinStateConnectingPeers;
			m_ReSendTimer.Stop();
			m_JoinTimer.Stop();
			m_JoinTimer.Start();
			m_ReSendTimer.Start();
			return true;
		}
		else
		{
			LOGERROR(g_Log, "C_NetJoinHelper::JoinEnterPeerConnectionPhase() - incorrect state prior to function entry.");
		}

		return false;
	}

	void C_NetJoinHelper::JoinSetFinished(void)
	{
		m_JoinState = eJoinStateComplete;
		m_LastJoinError = eJoinErrorNone;
		m_HostConsoleId = 0;
		m_JoinTimer.Stop();
		m_ReSendTimer.Stop();
		if (m_HostAddress.notNull())
			m_HostAddress = BD_NULL; // game code should have already took a reference to the hosts address during the join procedure.
	}

	bool C_NetJoinHelper::JoinTimedOut(const float maxTimeInSeconds)
	{
		if (!m_JoinTimer.IsActive())
			return true;

		bdNetImpl *const net = bdNet::getInstance();

		// no host address means join must fail asap.
		if (m_HostAddress.isNull() || NULL == net->getSocketRouter())
			return true;

		// if socket goes into a failed state we may as well stop the join as it will never complete.
		switch (net->getSocketRouter()->getStatus(m_HostAddress))
		{
		case BD_SOCKET_IDLE:
		case BD_SOCKET_LOST:
			return true;
			break;
		}

		return m_JoinTimer.ElapsedTimeInSeconds() >= maxTimeInSeconds ? true : false;
	}

	bool C_NetJoinHelper::JoinHostConnectionComplete(void)
	{
		bdNetImpl *const net = bdNet::getInstance();

		//no address means no connection.
		if (m_HostAddress.isNull() || NULL == net->getSocketRouter())
			return false;

		// test for completed connection.
		switch (net->getSocketRouter()->getStatus(m_HostAddress))
		{
		case BD_SOCKET_CONNECTED:
			return true;
			break;
		}
		
		return false;
	}

	bool C_NetJoinHelper::JoinReSendRequired(const float timeSlice)
	{
		if (!m_ReSendTimer.IsActive())
			return false;

		if (m_ReSendTimer.ElapsedTimeInSeconds() >= timeSlice)
		{
			// restart the timer
			m_ReSendTimer.Start();
			return true;
		}

		return false;
	}
	} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif