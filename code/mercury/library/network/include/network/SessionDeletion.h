// ***************************************************************
//  SessionDeletion   version:  1.0   ·  date: 06/04/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Sessions get added to this static object so we can 
//			delete 1 at a time.
// ***************************************************************

#ifndef SESSION_DELETION_H
#define SESSION_DELETION_H

#ifdef _MSC_VER
#pragma once
#endif


namespace NetPlay
{
	// forward declerations.
	class C_Session;
	class C_SessionDeletionCommand;


	/// Static class used for deleting multiple sessions.
	/** Active sessions can be added to an internal queue which wil lbe processed one at a time.
	 */
	class C_SessionDeletion
	{
	public:
		friend class C_Session;

		/// Get a reference to the static class instance.
		static C_SessionDeletion& Instance(void)	{return s_SessionDeletion;}

		/// Initialize all dynamic data, call once before using another function within this object.
		/** @param deleteQueueSize how many sessions we wish to queue up for deletion.
		 *	@return true if everything was correctly setup or false if not.
		 */
		bool Initialise(const U32 deleteQueueSize);

		/// deallocate dynamic data.
		/** Static instance becomes unusable after this call.
		 */
		void Shutdown(void) {Cleanup();}

		/// Update the deletion process.
		/** Call this once per frame to continue the asynchronous session deletion progress.
		 */
		void Poll(void);

		/// How many sessions are currently queued up for deletion.
		U32 ActiveCount(void);

		/// How many deletion slots are still remaining in the queue.
		U32 Remaining(void);

		// has this object been initialised.
		bool IsActive(void) const {return m_Initialised;} 

	private:
		explicit C_SessionDeletion();
		~C_SessionDeletion();
		C_SessionDeletion(const C_SessionDeletion& original);			// private copy constructor
		C_SessionDeletion& operator = (const C_SessionDeletion rhs);	// private un coded assignment operator

		// C_Session class will call this when it needs to delete itself.
		bool Add(C_Session& toDelete);		 

		void Cleanup(void);
		
	private:
		static C_SessionDeletion	s_SessionDeletion;
		bool						m_Initialised;				// dynamic data been allocated ?
		C_SessionDeletionCommand*	m_pDeleteCommandStorage;	// holds an array of command objects for when deleting sessions..
		void*						m_pDeleteCommandPool;		// linked list of available command objects for session deleting
		void*						m_pDeleteCommandList;		// linked list of all deletion commands currently awaiting processing.
	};
}

#endif


