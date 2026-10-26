//***************************************************************
//  DemonwareTask   version:  1.0   ·  date: 25/09/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Helper functions for the demonware bdRemotetaskRef
//			class. 
// ***************************************************************

#ifndef NETPLAY_DEMONWARE_TASK_H
#define NETPLAY_DEMONWARE_TASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include "bdCore/bdReference/bdReference.h"

// forward declerations
BD_REFERENCE(bdRemoteTask);

namespace NetPlay
{	
	namespace DemonwareRemoteTask
	{
		/// Defines the states that a demonware remote task can be in.
		typedef enum
		{
			eFailed = 0,		/**< task has failed. */	
			ePending,			/**< task is stil in progress. */
			eDone,				/**< task has completed. */
		}eTaskState;

		/// Checks the state of a given remote task.
		/** @param task current task to receive the state for.
		*	@return state of the task, @see eTaskState.
		*/
		eTaskState CheckState(bdRemoteTaskRef& task);

		/// Is the specified task still in progress.
		bool IsBusy(bdRemoteTaskRef& task);

		/// Close and release a demonware task handle.
		/** If the task is still active then it will be cancelled.
		*/
		void Close(bdRemoteTaskRef& task);
	}
}

#endif
