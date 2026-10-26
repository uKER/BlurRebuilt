#include <StdAfx.h>
#include <Network/Network.h>
#include <Network/PendingFriendEnumerator.h>

namespace NetPlay
{
	C_PendingFriendsEnumerator::C_PendingFriendsEnumerator() : m_Initialised(false), m_Mode(eModeNoneSet)
	{
	}

	C_PendingFriendsEnumerator::~C_PendingFriendsEnumerator()
	{
		Cleanup();
	}

	void C_PendingFriendsEnumerator::Cleanup(void)
	{
		Stop();
		m_Initialised = false;
	}

	bool C_PendingFriendsEnumerator::Initialise(void)
	{
		if (m_Initialised)
			return true;

		return m_Initialised;
	}

	bool C_PendingFriendsEnumerator::StartEnumeration(const SysProfile::FullProfile& inUser, ePendingEnumerateMode mode)
	{
		if (!m_Initialised)
			return false;
		return false;
	}

	bool C_PendingFriendsEnumerator::StartRevokeFriendRequest(const PendingFriendProfile& toRevoke)
	{
		if (!m_Initialised)
			return false;

		return false;
	}

	bool C_PendingFriendsEnumerator::StartAnswerFriendRequest(const PendingFriendProfile& toReply, const bool accept)
	{
		if (!m_Initialised)
			return false;

		return false;
	}

	bool C_PendingFriendsEnumerator::ContinueTask(bool& errorOccurred)
	{
		errorOccurred = false;
		if (!m_Initialised)
			return true;

		return true;
	}

	void C_PendingFriendsEnumerator::Stop(void)
	{
		if (!m_Initialised)
			return;
	}

	U32 C_PendingFriendsEnumerator::ExtractPendingFriends(PendingFriendProfile* pOutput, const U32 maxOutput)
	{
		if (!m_Initialised)
			return 0;
		return 0;
	}

	bool C_PendingFriendsEnumerator::IsBusy(void) const
	{
		if (!m_Initialised)
			return false;

		return false;
	}

}// end namespace

