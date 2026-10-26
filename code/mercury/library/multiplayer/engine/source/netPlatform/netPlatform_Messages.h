#ifndef NETPLATFORM_MESSAGES_H
#define NETPLATFORM_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/UID/FourCC.h>
#include <misc_shared/Messaging/MessageBase.h>
#include "netEnums/netEnums.h"

namespace netPlatform
{
	enum
	{
		MODE_NOT_SET=-1,
		MODE_DEFAULT,		// signed in. Go online if signed in online, else systemlink.
		MODE_OFFLINE,		// signed in. demonware shutdown.
		MODE_SYSTEMLINK,	// signed in. demonware systemlink services available.
		MODE_ONLINE,		// signed in. demonware online and systemlink services available.
	};

	enum E_Msg
	{
		E_ProfMsg_Start = FOURCC_HI20('NETP'), // Profile notification Message Type
		E_Msg_CableConnected,
		E_Msg_CableDisconnected,
		E_Msg_GotTicket,
		E_Msg_ExitGame,
		E_Msg_SignInStart,
		E_Msg_SignInStarted,
		E_Msg_SignInSuccess,
		E_Msg_SignInFailed,
		E_Msg_NetworkServicesStart,
		E_Msg_NetworkServicesStarted,
		E_Msg_NetworkServicesSuccess,
		E_Msg_NetworkServicesFailed,
		E_Msg_StartBandwidthEvaluation,
		E_Msg_BandwidthEvaluationStarted,
		E_Msg_BandwidthEvaluationFinished,
		E_Msg_End,
	};

	class C_Msg_ExitGame: public Messaging::C_MessageDerived<C_Msg_ExitGame, E_Msg_ExitGame>
	{
	public:
		C_Msg_ExitGame()
		{}
		
		static Messaging::TMsgType StaticType() {return E_Msg_ExitGame; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "ExitGame"); 
		}
		#endif
	};

	class C_Msg_GotTicket: public Messaging::C_MessageDerived<C_Msg_GotTicket, E_Msg_GotTicket>
	{
	public:
		C_Msg_GotTicket(bool success)
		: m_success(success)
		{}
		
		static Messaging::TMsgType StaticType() {return E_Msg_GotTicket; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "GotTicket"); 
		}
		#endif

		bool m_success;
	};

	class C_Msg_CableDisconnected: public Messaging::C_MessageDerived<C_Msg_CableDisconnected, E_Msg_CableDisconnected>
	{
	public:
		C_Msg_CableDisconnected()
		{}
		static Messaging::TMsgType StaticType() {return E_Msg_CableDisconnected; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "CableDisconnected"); 
		}
		#endif
	 };

	class C_Msg_CableConnected: public Messaging::C_MessageDerived<C_Msg_CableConnected, E_Msg_CableConnected>
	{
	public:
		C_Msg_CableConnected()
		{}
		static Messaging::TMsgType StaticType() {return E_Msg_CableConnected; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "CableConnected"); 
		}
		#endif
	};

	class C_Msg_StartBandwidthEvaluation: public Messaging::C_MessageDerived<C_Msg_StartBandwidthEvaluation, E_Msg_StartBandwidthEvaluation>
	{
	public:
		C_Msg_StartBandwidthEvaluation()
		{}
		
		static Messaging::TMsgType StaticType() {return E_Msg_StartBandwidthEvaluation; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "StartBandwidthEvaluation"); 
		}
		#endif
	};

	class C_Msg_BandwidthEvaluationStarted: public Messaging::C_MessageDerived<C_Msg_BandwidthEvaluationStarted, E_Msg_BandwidthEvaluationStarted>
	{
	public:
		C_Msg_BandwidthEvaluationStarted()
		{}
		
		static Messaging::TMsgType StaticType() {return E_Msg_BandwidthEvaluationStarted; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "BandwidthEvaluationStarted"); 
		}
		#endif
	};

	class C_Msg_BandwidthEvaluationFinished: public Messaging::C_MessageDerived<C_Msg_BandwidthEvaluationFinished, E_Msg_BandwidthEvaluationFinished>
	{
	public:
		C_Msg_BandwidthEvaluationFinished(bool success, F32 uploadKBps, F32 downloadKBps)
		: m_success(success)
		, m_uploadKBps(uploadKBps)
		, m_downloadKBps(downloadKBps)
		{}
		
		static Messaging::TMsgType StaticType() {return E_Msg_BandwidthEvaluationFinished; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "BandwidthEvaluationFinished"); 
		}
		#endif

		bool m_success;
		F32 m_uploadKBps;
		F32 m_downloadKBps;
	};

	class C_Msg_SignInStart: public Messaging::C_MessageDerived<C_Msg_SignInStart, E_Msg_SignInStart>
	{
	public:
		C_Msg_SignInStart()
		{}
		
		static Messaging::TMsgType StaticType() {return E_Msg_SignInStart; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "SignInStart"); 
		}
		#endif
	};

	class C_Msg_SignInStarted: public Messaging::C_MessageDerived<C_Msg_SignInStarted, E_Msg_SignInStarted>
	{
	public:
		C_Msg_SignInStarted()
		{}
		
		static Messaging::TMsgType StaticType() {return E_Msg_SignInStarted; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "SignInStarted"); 
		}
		#endif
	};

	class C_Msg_SignInFailed: public Messaging::C_MessageDerived<C_Msg_SignInFailed, E_Msg_SignInFailed>
	{
	public:
		C_Msg_SignInFailed()
		{}
		
		static Messaging::TMsgType StaticType() {return E_Msg_SignInFailed; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "SignInFailed"); 
		}
		#endif
	};

	class C_Msg_SignInSuccess: public Messaging::C_MessageDerived<C_Msg_SignInSuccess, E_Msg_SignInSuccess>
	{
	public:
		C_Msg_SignInSuccess()
		{}
		
		static Messaging::TMsgType StaticType() {return E_Msg_SignInSuccess; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "Msg_SignInSuccess"); 
		}
		#endif
	};

	class C_Msg_NetworkServicesStart: public Messaging::C_MessageDerived<C_Msg_NetworkServicesStart, E_Msg_NetworkServicesStart>
	{
	public:
		C_Msg_NetworkServicesStart(int mode)
		: m_mode(mode)
		{}
		
		static Messaging::TMsgType StaticType() {return E_Msg_NetworkServicesStart; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "NetworkServicesStart"); 
		}
		#endif

		int m_mode;
	};

	class C_Msg_NetworkServicesStarted: public Messaging::C_MessageDerived<C_Msg_NetworkServicesStarted, E_Msg_NetworkServicesStarted>
	{
	public:
		C_Msg_NetworkServicesStarted(int mode)
		: m_mode(mode)
		{}
		
		static Messaging::TMsgType StaticType() {return E_Msg_NetworkServicesStarted; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "NetworkServicesStarted"); 
		}
		#endif

		int m_mode;
	 };

	class C_Msg_NetworkServicesFailed: public Messaging::C_MessageDerived<C_Msg_NetworkServicesFailed, E_Msg_NetworkServicesFailed>
	{
	public:
		C_Msg_NetworkServicesFailed(int mode, int status)
		: m_mode(mode)
		, m_status(status)
		{}
		
		static Messaging::TMsgType StaticType() {return E_Msg_NetworkServicesFailed; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "NetworkServicesFailed"); 
		}
		#endif

		int m_mode;
		int m_status;
	};

	class C_Msg_NetworkServicesSuccess: public Messaging::C_MessageDerived<C_Msg_NetworkServicesSuccess, E_Msg_NetworkServicesSuccess>
	{
	public:
		C_Msg_NetworkServicesSuccess(int mode)
		: m_mode(mode)
		{}
		
		static Messaging::TMsgType StaticType() {return E_Msg_NetworkServicesSuccess; }

		#if !defined( CFG_GOLD )
		static void Register() 
		{
			Messaging::RegisterDebugInfo(StaticType(), "netPlatform::E_Msg_" "NetworkServicesSuccess"); 
		}
		#endif

		int m_mode;
	};
};

#endif //NETPLATFORM_MESSAGES