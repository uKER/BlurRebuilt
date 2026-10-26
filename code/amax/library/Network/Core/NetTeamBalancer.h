#ifndef __NET_TEAM_BALANCER_H__
#define __NET_TEAM_BALANCER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/vector.h>
#include "../Misc/NetDefines.h"

namespace RacerNet
{
	class C_NetPlayer;

	/*--------------------------------------------------------------------------------------
		Class	: C_NetTeamBalancer

		Desc	: Helper class which manages which players are in each team.
	--------------------------------------------------------------------------------------*/
	class C_NetTeamBalancer
	{
		public:
			typedef Btl::Vector<C_NetPlayer*> NetBucket;										// List of same party players
			typedef Btl::Vector<NetBucket> NetBucketList;										// List of party groups
			typedef Btl::Vector<U8, Btl::VectorParameters_Static<NET_MAX_VEHICLES> > NetTeam;	// List of players on a team

			C_NetTeamBalancer();

							/// Assigns grouped players onto the same time and performs basic balancing
			void			AssignTeams( NetBucketList& bucketList, const S8 sMaxTeamSize, const S8 sBalanceRange );

							/// Returns the number of teams
			S8				NumTeams() const { return m_Teams.Size(); }

							/// Returns true if the teams are balanced
			bool			Balanced() const { return m_Balanced; }

							/// Returns the difference between the min and max players on the teams.
			S8				Difference() const { return m_Difference; }

							/// Returns the largest size a party group can be to fit on a team
			S8				FreeSpace() const { return m_FreeSpace; }

							/// Returns true if there are more than 1 player on each team
			bool			Valid() const { return m_Valid; }

							/// Returns a list of players on a team
			const NetTeam&	GetTeam( const S8 sTeam );

		private:
			void	AddTeamMembers( const NetBucket& bucket, const S8 sTeam );

			S8		CalculateFreeSpace( const NetBucketList& bucketList, const S8 sMaxTeamSize ) const;

		private:
			typedef Btl::Vector<NetTeam, Btl::VectorParameters_Array<NET_MAX_TEAMS> > NetTeamList;	// List of teams

			NetTeamList		m_Teams;
			bool			m_Balanced;
			bool			m_Valid;
			S8				m_Difference;
			S8				m_FreeSpace;
	};

}; // namespace RacerNet

#endif // __NET_TEAM_BALANCER_H__