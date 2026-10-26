#include <stdafx.h>
#include <Core/Core.h>
#include <Network/Network.h>
#include "NpThreadedFriendEnumerator.h"

namespace SysProfile
{
	C_NpThreadedFriendEnumerator::C_NpThreadedFriendEnumerator() : m_Initialised(false), m_Terminate(false), m_Status(eStatusInvalid)
	{
	}

	C_NpThreadedFriendEnumerator::~C_NpThreadedFriendEnumerator()
	{
		Shutdown();
	}

	bool C_NpThreadedFriendEnumerator::Initialise(void)
	{
		if (m_Initialised)
			return true;

		m_Friends.Clear();

		Start(0x10000, 0, "np friends enumeration");
		m_Lock.Enter();
		m_Initialised = true;
		m_Status = eStatusIdle;
		m_Lock.Exit();

		return m_Initialised;
	}


	void C_NpThreadedFriendEnumerator::ThreadMain()
	{
		bool exitThread = false;
		do
		{
			// this should make the thread sleep when not tasked with enumerating friends.
			m_EnumerateEvent.Wait(INFINITE);

			// how many friends does the OS want us to retrieve.
			U32 numFriends = 0;

			bool moveToEnumeration = false;
			S32 res = 0;

			do 
			{
				if (!NeedToCancel())
				{
					res = sceNpBasicGetFriendListEntryCount(&numFriends);
					switch (res)
					{
					case 0:	// count retrieved ok
						moveToEnumeration = true;
						break;

					case SCE_NP_BASIC_ERROR_BUSY: // still getting friends from server so wait a bit
						Sleep(50);
						break;

					default: // big error assume no friends to enumerate.
						numFriends = 0;
						moveToEnumeration = true;
						break;
					}
				}
				else
				{
					numFriends = 0;
					moveToEnumeration = true;
				}
			} while(!moveToEnumeration);

			// enumerate each friend in turn.
			if (numFriends)
			{
				for (U32 i = 0; i < numFriends; )
				{
					if (!NeedToCancel())
					{
						SceNpId npId;
						bool addFriend = true;
						res = sceNpBasicGetFriendListEntry(i, &npId);
						switch (res)
						{
						case 0:  // got friend
							i += 1;
							m_Lock.Enter();
							for (S32 z=0; z<m_Friends.Size() && addFriend; z++)
							{
								// if friend already exists then dont add them twice.
								if (sceNpUtilCmpNpId(&npId, &m_Friends[z]) == 0)
									addFriend = false;
							}

							if (addFriend && m_Friends.Size() != m_Friends.Capacity())
								m_Friends.PushBack(npId);
							m_Lock.Exit();
							break;
						
						case SCE_NP_BASIC_ERROR_BUSY: // wait a bit before trying again
							Sleep(50);
							break;

						default: 
							i += 1; // skip to next friend if we got an error retrieving this one.
							break;
						}
					}
					else
					{
						// skip to end of enumeration to force a cancellation.
						i = numFriends;
					}
				}
			}

			// enumeration complete so move to idle state
			m_Lock.Enter();
			m_Status = eStatusIdle;
			exitThread = m_Terminate;
			m_Lock.Exit();
		} while (!exitThread);
	}

	bool C_NpThreadedFriendEnumerator::StartEnumeration(void)
	{
		if (!m_Initialised)
			return false;

		bool started = false;
		m_Lock.Enter();
		if (eStatusIdle == m_Status)
		{
			m_Friends.Clear();
			m_Status = eStatusEnumerating;
			m_EnumerateEvent.Signal();
			started = true;
		}
		m_Lock.Exit();

		return started;
	}

	void C_NpThreadedFriendEnumerator::CancelEnumeration(void)
	{
		if (!m_Initialised)
			return;

		m_Lock.Enter();
		if (eStatusEnumerating == m_Status)
			m_Status = eStatusCancelling;
		m_Lock.Exit();
	}

	C_NpThreadedFriendEnumerator::eStatus C_NpThreadedFriendEnumerator::Status(void)
	{
		if (!m_Initialised)
			return eStatusInvalid;

		eStatus status = eStatusInvalid;
		m_Lock.Enter();
		status = m_Status;
		m_Lock.Exit();
		return status;
	}

	bool C_NpThreadedFriendEnumerator::ExtractFriends(NpEnumeratedFriendsList& friends)
	{
		friends.Clear();
		if (!m_Initialised)
			return false;
		bool ok = false;
		m_Lock.Enter();
		if (eStatusIdle == m_Status)
		{
			friends = m_Friends;
			ok = true;
		}
		m_Lock.Exit();
		return ok;
	}

	void C_NpThreadedFriendEnumerator::Shutdown(void)
	{
		if (!m_Initialised)
			return;

		// wake up the thread and flag it to cancel and terminate.
		// if the thread is already running then signaling the event will be ignored.
		m_Lock.Enter();
		m_Terminate = true;
		m_Status = eStatusCancelling;
		m_EnumerateEvent.Signal();		
		m_Lock.Exit();

		// wait for thread to terminate.
		Join();

		m_Status = eStatusInvalid;
		m_Terminate = false;
		m_Initialised = false;
	}

	bool C_NpThreadedFriendEnumerator::NeedToCancel(void)
	{
		if (!m_Initialised)
			return false;

		bool cancel = false;
		m_Lock.Enter();
		cancel = (eStatusCancelling == m_Status) ? true : false;
		m_Lock.Exit();
		return cancel;
	}
}