#ifndef NP_THREADED_FRIEND_ENUMERATOR
#define NP_THREADED_FRIEND_ENUMERATOR

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/vector.h>
#include <Core/Threads/Thread.h>
#include <Core/Threads/Event.h>

namespace SysProfile 
{	
	typedef Btl::Vector<SceNpId, Btl::VectorParameters_Static<MAX_FRIENDS> > NpEnumeratedFriendsList;

	/// Enumerates all online / offline PS3 friends by querying the OS.
	/** Very slow as each friend requires a seperate OS query call.
 	 */
	class C_NpThreadedFriendEnumerator : private MT::Thread
	{
	public :
		typedef enum
		{
			eStatusInvalid = -1,
			eStatusIdle,
			eStatusEnumerating,
			eStatusCancelling,
		}eStatus;

		C_NpThreadedFriendEnumerator();
		virtual ~C_NpThreadedFriendEnumerator();

		/// Creates the thread and events used to enumerate friends.
		bool Initialise(void);

		/// Terminates the thread.
		/** This will block until the thread has be terminated.
		 */
		void Shutdown(void);

		/// Start the enumeration process.
		/** @return true if the process was started or not.
		 *	Only start enumeration if the status is eStatusIdle @see Status().
		 */
		bool StartEnumeration(void);

		/// Attempt to cancel enumeration.
		void CancelEnumeration(void);

		/// Access to the current Status.
		eStatus Status(void);

		/// Attempt to extract the enumerated friends.
		/** Should only call once enumeration has completed.
		 *	@param friends list of enumerated friends
		 *	@return true if friends is valid or false if not.
		 */
		bool ExtractFriends(NpEnumeratedFriendsList& friends);

	protected:
		/// Main logic that runs on the thread.
		void ThreadMain();

	private:
		bool NeedToCancel(void);
		C_NpThreadedFriendEnumerator(const C_NpThreadedFriendEnumerator& original);				// private copy constructor
		
	private:
		bool m_Initialised;
		bool m_Terminate;
		eStatus m_Status;
		MT::Lock m_Lock;
		MT::Event m_EnumerateEvent;
		NpEnumeratedFriendsList m_Friends;
	};
}

#endif //NP_THREADED_FRIEND_ENUMERATOR
