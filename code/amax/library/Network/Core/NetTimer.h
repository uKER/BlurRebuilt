// ***************************************************************
//  NetTimer   version:  1.0   ·  date: 04/15/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	timing class that expands on Demonwares bdStopwatch
// ***************************************************************

#ifndef __NET_TIMER_H__
#define __NET_TIMER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <bdCore/bdTiming/bdStopwatch.h>

namespace RacerNet
{
	class C_NetTimer
	{
	public:
		C_NetTimer();
		virtual ~C_NetTimer();

		/// Start the timer.
		/** If the timer is already running this will restart it.
		 */
		void Start(void);

		/// Is the timer currently active.
		/** @see Start()
		 *	@return true if the timer is running, false if not.
		 */
		bool IsActive(void) const {return m_Active;}

		/// How much time has passed since we called Start()
		/** if the timer is not active calling this will return 0.0f.
		 *	@return time passed since Start() was called (in seconds).
		 */
		F32 ElapsedTimeInSeconds(void);

		/// Stop the timer.
		/** After this is called ElapsedTimeInSeconds() will return 0.
		 */
		void Stop();
		
	private:
		bool m_Active;
		bdStopwatch m_Timer;
	};

}// end namespace RacerNet

#endif //__NET_TIMER_H__
