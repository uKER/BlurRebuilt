// ***************************************************************
//  BandwidthEvaluator   version:  1.0   ·  date: 02/08/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Bandwidth evaluation to help with network play.
// ***************************************************************

#ifndef BANDWIDTH_EVALUATOR_H
#define BANDWIDTH_EVALUATOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <bdLobby/bdLSGServices/bdBandwidthTest.h> //  for demonware
#include <bdCore/bdTiming/bdStopwatch.h>

namespace NetPlay
{
	/// A wrapper for evaluating the bandwidth of an online player.
	/** Evaluate the upstream and downstream bandwidth of the internet connection.
	 *	use this functionality outide of the network play as it will flood your connection and 
	 *	possibly disconnect you.
	 */
	class C_BandwidthEvaluator
	{
	public:
		explicit C_BandwidthEvaluator(const U32 minUpstream = 65536, const U32 minDownstream = 65536); 
		virtual ~C_BandwidthEvaluator(); 

		/// Start evaluating a players bandwidth asynchronously.
		/**	@return true if the bandwidth enumeration was started.
		 *	see Update()
		 */
		bool Start(void);

		/// Continue the process of evaluating a players bandwidth.
		/** This needs to be called each frame to progess the scan.
		 *	Check the progress by polling IsActive() until it returns false, at that point 
		 *	the bandwith will be obtained. 
		 */
		void Update(void);

		/// Is the bandwidth scan currently active.
		/**	@return false if the bandwidth scan is not currently active.  
		 */
		bool IsActive(void);

		/// Stop the current bandwidth scan if it is active.
		/**	This can be used to stop a scan that is currently in progress.
		 */
		void Stop(void);

		/// Retrieve the current cached upstream and downstream bandwidth figures.
		/** If you call this while an evaulation is still in progress then the returned figures will be from the last evaluation not the current ones.
		 *	@param upStream holds our new up stream bandwidth value in bps.
		 *	@param downStream holds our new down stream bandwidth value in bps.
		 */
		void GetResults(U32& upStream, U32& downStream) const {upStream = m_UpStream; downStream = m_DownStream;}

	private:
		C_BandwidthEvaluator(const C_BandwidthEvaluator& original);				// private copy constructor
		C_BandwidthEvaluator& operator = (const C_BandwidthEvaluator rhs);		// private un coded assignment operator

		U32	m_UpStream;
		U32 m_DownStream;
		U32	m_MinUpStream;
		U32 m_MinDownStream;
#if defined(LIVE_ON_WINDOWS)
		XNQOS*	m_pQos;
#elif defined(PLATFORM_PS3)
		bdBandwidthTestClient*	m_pClient;
		bdStopwatch*			m_pTimer;
		bool					m_Delaying;
		U32						m_RetryCount;
#elif defined(PLATFORM_PC)
		bool					m_Delaying;
		U32						m_RetryCount;
		bdBandwidthTestClient*	m_pClient;
		bdStopwatch*			m_pTimer;
#endif
	};
}

#endif


