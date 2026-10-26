// ***************************************************************
//  SessionEnumerator   version:  1.0   ·  date: 03/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Online session enumeration.
// ***************************************************************

#ifndef SESSION_ENUMERATOR_H
#define SESSION_ENUMERATOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <bdcore\bdSocket\bdCommonAddr.h>
#include <bdsocket\bdQoS\bdQoSProbeListener.h>
#include <Network\DemonwareTask.h>
#include <Network\SessionParameter.h>

// forward declerations
namespace SysProfile
{
	struct BaseProfile;
	struct FullProfile;
	struct GameInvite;
}

namespace NetPlay
{
	// forward declerations.
	struct SessionSecurity;
	class SessionQoSData;
	class C_EnumeratedDemonwareSession;
	class C_SessionEnumeratorListener;

	/// Object for enumerating online game sessions that are currently active.
	/** When using demonware 2.0 we can no longer specify what server parameters to return in the query, we must request that all server attributes are
	 * returned in the same order as they are specified in the web imp matchmaking attributes congig.
	 */
#if defined(LIVE_ON_WINDOWS)
	class C_SessionEnumerator
#else
	class C_SessionEnumerator : public bdQoSProbeListener
#endif
	{
	public:
		explicit C_SessionEnumerator();
		virtual ~C_SessionEnumerator();


		typedef enum
		{
			eSearchNotActive = 0,
			eSearching,  
			eQos,
		}eSearchState;

		/// Structure to hold QoS Data if available
		struct SessionQoSData
		{
			SessionQoSData() : m_Ping(0), m_QoSDataSize(0), m_pQoSData(NULL)
			{}
			U32		m_Ping;		// this is in milliseconds.
			U32		m_QoSDataSize;
			U8*		m_pQoSData;
		};

		friend class C_Session;

		/// Initialise object to allocate dynamic resources.
		/** @param maxSearchProperties maximum number of properties we will use in our search querires.
		 *	@param maxSearchContexts maximum number of contexts we will use in our search queries.
		 *	@param maxSessionsToReturn maximum number of session we can search for once all dynamic resources have been allocated.
		 *	@return true if successfull otherwise false.
		 */
		bool Initialise(const U32 maxSearchProperties, const U32 maxSearchContexts, const U32 maxSessionsToReturn);

		/// registers a listener that listens to enumeration events.
		/**	@param listener object listening to enumeration events.
		 *	@return true if listener has been registered, false if already registered.
		 */
		bool RegisterListener(C_SessionEnumeratorListener* listener);

		/// unregisters a listener that was previously registered.
		/**	@param listener we want to unregister.
		 *	@return true if listener has been unregistered succeffully, false if not found.
		 */
		bool UnregisterListener(C_SessionEnumeratorListener* listener);

		/// Start searching for a list of game sessions.
		/** The matchmaking queries are defined in XLAST for xbox and on Web-IMP for demonware. 
		 *	System defined session parameters (PublicSlotsOpen, PublicSlotsFilled, PrivateSlotsOpen, PrivateSlotsFilled, HostAddress, Secure keys) are returned as default.
		 *	@param localPlayer profile for the player who is starting the search request.
		 *	@param queryId which matchmaking query are we to use.
		 *	@param maxResults limit the sessions returned to this number . NOTE : this MUST must not be greater than the value used for maxSessionsToReturn in the Initialise() function.
		 *	@param pSearchParams pointer to a list of sgeme defined session parameters that are uses in the specified search query.
		 *	@param numSearchParams how many SessionParameters does pSearchParams point to.
		 *	@param returnTypes ignored on xbox360 but when using demonware we need to define the types of server attributes that will be returned, this does not include the
		 *	session key data blob attribute as it's accounted for already. Deomoware session queries return all matchmaking attributes that are listed in the imp config. It's no longer 
		 *	possible to specify what attributes to return via demonware, you get the lot.
		 *	@param qosHasDataStream if true then the QoS probes will expect (and wait) for a small amount of data from the host to be received.
		 *	@return true if the asynchronous search was started or false if not.
		 */
		bool StartSearch(const SysProfile::FullProfile& localPlayer, const U32 queryId, const U32 maxResults, const SessionParameter* pSearchParams, const U32 numSearchParams, const SesionQueryReturnTypes& returnTypes, const bool qosHasDataStream);

		/// Start searching for a a specific game session that we have been invited to.
		/** The matchmaking queries are defined in XLAST for xbox and on Web-IMP for demonware. 
		 *	System defined session parameters (PublicSlotsOpen, PublicSlotsFilled, PrivateSlotsOpen, PrivateSlotsFilled, HostAddress, Secure keys) are returned as default.
		 *	@param localPlayer profile for the player who is starting the search request.
		 *	@param gameInvite SysProfile::GameInvite structure for a specific session. 
		 *	@param returnTypes ignored on xbox360 but when using demonware we need to define the types of server attributes that will be returned, this does not include the
		 *	session key data blob attribute as it's accounted for already. Deomoware session queries return all matchmaking attributes that are listed in the imp config. It's no longer 
		 *	possible to specify what attributes to return via demonware, you get the lot.
		 *	@param qosHasDataStream if true then the QoS probes will expect (and wait) for a small amount of data from the host to be received.
		 *	@return true if the asynchronous search was started or false if not.
		 */
		bool StartSearch(const SysProfile::FullProfile& localPlayer, const SysProfile::GameInvite& gameInvite, const SesionQueryReturnTypes& returnTypes, const bool qosHasDataStream);

		/// Returns true if a search is currently active or not
		bool IsSearching() const {return m_SearchState != eSearchNotActive;}

		/// Perforrms any asynchronous tasks, should call at all times
		void Poll(void);

		/// Rerurns the number of sessions available after a successfull search.
		U32	NumSessionsEnumerated(void) const;

		/// Get access top the hosting information part of a specific enumerated session.
		/** @param sessionIndex 0 to NumSessionsEnumerated() once a search has completed successfully.
		 *	@param sessionSecurity on successfull return this will hold the security info required to connect to the host.
		 *	@param sessionAddress on successfull return this will hold the hosts network address so we can connect.
		 *	@return true if the returned data is valid, false if not..
		 */
		bool GetHostInfoFromEnumerated(const U32 sessionIndex, SessionSecurity& sessionSecurity, bdCommonAddrRef& sessionAddress) const; 

		/// Get the number of open slots and maximum slots in the spsecified session.
		/**	@param sessionIndex 0 to NumSessionsEnumerated() once a search has completed successfully.
		 *	@param openSlots on successfull return this holds how many slots still available to join.
		 *	@param maxSlots on successfull return this holds how many players are allowed to play in the session in total.
		 *	@return true if supplied data is valid, or false if not.
		 */
		bool GetSlotInfoFromEnumerated(const U32 sessionIndex, U32& openSlots, U32& maxSlots) const;

		/// Get a specific parameter for a specitic session.
		/**	@param sessionIndex 0 to NumSessionsEnumerated() once a search has completed successfully.
		 *	@param paramId id of a specific paramer to find within the enumerated session data.
		 *	@param paramData on successfull return  will hold the required parameter data.
		 *	@return true if supplied data is valid, or false if not.
		 */
		bool GetSessionParameterFromEnumerated(const U32 sessionIndex, const U32 paramId, SessionParameter& paramData) const;

		/// Get the list of parameters for a specitic session.
		/**	@param sessionIndex 0 to NumSessionsEnumerated() once a search has completed successfully.
		 *	@param paramData on successfull return will hold the parameters stored by the session.
		 *	@return true if supplied data is valid, or false if not.
		 */
		bool GetParametersFromEnumerated(const U32 sessionIndex, Btl::Vector< SessionParameter>& paramData) const;

		/// Get the QoS data for a specific enumerated session;
		/**	@param sessionIndex 0 to NumSessionsEnumerated() once a search has completed successfully.
		 *	@param qosData on successfull return this holds the QoS information for the specified session.
		 *	@return true if supplied data is valid, or false if not.
		 */
		bool GetQoSDataFromEnumerated(const U32 sessionIndex, SessionQoSData& qosData) const;

		/// Stop search and flush and old found sessions.
		/** This must be called when you are finished with the enumerated data.
		 */
		void KillSearch(void);

		/// Stop search but DO NOT flush old found sessions.
		/** This must be called when you want to manually abort the search but retain the session list.
		 */
		void StopSearch(void);

#if !defined(LIVE_ON_WINDOWS)
		// demonware Qos listener overrides.
		virtual void onQoSProbeSuccess(const bdQoSProbeInfo &info);
		virtual void onQoSProbeFail(bdCommonAddrRef addr);
#endif

	private:
		C_SessionEnumerator(const C_SessionEnumerator& original);			// private copy constructor
		C_SessionEnumerator& operator = (const C_SessionEnumerator rhs);	// private un coded assignment operator
		void CleanUp(void);
		void KillQoS(void);
		void FinaliseResults();	

		private:
		bool			m_Initialised;
		U32				m_MaxSessionToEnumerate;		// what is the maximum number of sessions we can return from the server
		U32				m_NumSessionEnumerated;			// how many sessions did the search find.
		eSearchState	m_SearchState;
		bool			m_QoSHasDataStream;				// should we be waiting for a data stream during the QoS probes
		
		// listeners to enumerator events
		Btl::List<C_SessionEnumeratorListener*> m_listeners; // listens to enumeration events (enumeration stopped / started, session found).

		// qualiy of service..
#if defined(LIVE_ON_WINDOWS)
		void*			m_pAsyncTask;					// used for asynchronous searching.
		void*			m_pSearchProperties;			// pointer to a list of properties used during the search	
		void*			m_pSearchContexts;				// pointer to a list of contexts used during the search
		void*			m_pEnumeratedSessionData;		// holds all the session data after a successfull search.
		U32				m_NumSearchProperties;			// how many properties did we allocate
		U32				m_NumSearchContexts;			// how many contexts did we allocate
		SessionQoSData*	m_pQoSSessionData;				// stores QoS Data for each returned session.
		
		XNADDR**		m_pQoSAddresses;				// list of session net addresses to QoS
		XNKEY**			m_pQoSKeys;						// list of secure session leys to QoS
		XNKID**			m_pQoSKeyIds;					// list of secure key identifiers to QoS			
		XNQOS*			m_pQoSHandle;					// main handle for tracking QoS;
#elif defined(PLATFORM_PS3) || defined(PLATFORM_PC)
		U32								m_NumSessionsToReturnInQuery;
		bool							m_CustomQuery;	
		C_EnumeratedDemonwareSession*	m_pEnumeratedSessionData;	
		bdRemoteTaskRef					m_AsyncTask;
		U32								m_NumQoSProbeRepliesExpected;
		U32								m_NumQoSProbeRepliesReceived;
#endif
	};


	/// Object for returning information about an enumeration curently running.
	/** returns session parameters when a session has been found, when a a search started, and 
		when a search ended.
	 */
	class C_SessionEnumeratorListener
	{
	public:
		virtual ~C_SessionEnumeratorListener(){}
		
		/// Called once a session as been enumerated successfully (and the QoS has been resolved),
		/**	@param SessionEnumerator the enumerator responding to the callback.
		 *	@param SessionIndex the enumerated session index in the enumerator session list.
		 *	@param SessionParameters the session parameters as returned by the enumeration. 
		 *	@param SessionQoSData the qos data returned by the session host.
		 *	@param SessionSecurity security credentials of the enumerated session.
		 *	@param SessionHostAddress the address of the session host.
		 *	@param SessionOpenSlots hhow many open slots in the session.
		 *	@param SessionMaxSlots how many slots max in the session.
		 *	@return true if supplied data is valid, or false if not.
		 */
		virtual void OnSessionEnumerated(	
						NetPlay::C_SessionEnumerator*						SessionEnumerator,
						U32													SessionIndex,
						const Btl::Vector< NetPlay::SessionParameter >&		SessionParameters,
						const NetPlay::C_SessionEnumerator::SessionQoSData&	SessionQoSData,
						const NetPlay::SessionSecurity&						SessionSecurity, 
						bdCommonAddrRef										SessionHostAddress,
						U32													SessionOpenSlots, 
						U32													SessionMaxSlots){}

		/// Called once a session enumeration has been started.
		/**	@param SessionEnumerator the enumerator responding to the callback.
		*/
		virtual void  OnSessionEnumeratorStarted(NetPlay::C_SessionEnumerator* enumerator){}

		/// Called once a session enumeration has stopped.
		/**	Could be after the enumeration finished, or the enumeration was aborted, or after an enumeration error.
		 *	@param SessionEnumerator the enumerator responding to the callback.
		 *	@param uSessionCount number of sessions enumerated.
		*/
		virtual void OnSessionEnumeratorFinished(NetPlay::C_SessionEnumerator* enumerator, int uSessionCount){}
	};
}



#endif


