#include <stdafx.h>
#include "NetTeamBalancer.h"

#include <algorithm>
#include <Network/Player/NetPlayer.h>

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

#ifndef CFG_GOLD
Debug::Bool g_Dump_Team_Stats("Network.Race.Teams.DumpTeamStats", true );
#endif // CFG_GOLD

namespace RacerNet
{
	static bool BucketSort( const C_NetTeamBalancer::NetBucket& lhs, const C_NetTeamBalancer::NetBucket& rhs )
	{
		return ( lhs.Size() > rhs.Size() );
	}
	
	C_NetTeamBalancer::C_NetTeamBalancer()
		: m_Balanced( false ), m_Difference( 0 ), m_FreeSpace( 0 )
	{
	}

	void C_NetTeamBalancer::AssignTeams( NetBucketList& bucketList, const S8 sMaxTeamSize, const S8 sBalanceRange )
	{
		// sort groups into largest first
		std::sort( bucketList.Begin(), bucketList.End(), BucketSort );

#ifndef CFG_GOLD
		if ( g_Dump_Team_Stats )
		{
			LOGINFO(g_Log, "C_NetTeamBalancer::AssignTeams() - %d buckets", bucketList.Size() );

			for ( int i = 0; i < bucketList.Size(); i++ )
				LOGINFO(g_Log, "C_NetTeamBalancer::AssignTeams() - bucket [%d] has %d members", i, bucketList[i].Size() );
		}
#endif

		m_Teams[NET_TEAM_A].Clear();
		m_Teams[NET_TEAM_B].Clear();

		// Best fit items into buckets
		for ( S8 i = 0; i < bucketList.Size(); i++ )
		{
			NetBucket& bucket = bucketList[i];

			const S8 bucketSize = bucket.Size();
			const S8 sizeA = m_Teams[NET_TEAM_A].Size();
			const S8 sizeB = m_Teams[NET_TEAM_B].Size();

			// Bucket A is empty
			if ( sizeA == 0 )
			{
				AddTeamMembers( bucket, NET_TEAM_A );
			}
			else // Bucket B is empty
			if ( sizeB == 0 )
			{
				AddTeamMembers( bucket, NET_TEAM_B );
			}
			else// Fits in A or B
			if ( sizeA + bucketSize <= sMaxTeamSize && sizeB + bucketSize <= sMaxTeamSize )
			{
				// Add to B
				if ( ( sizeA + bucketSize ) > sizeB )
				{
					AddTeamMembers( bucket, NET_TEAM_B );
				}
				else
				{
					AddTeamMembers( bucket, NET_TEAM_A );
				}
			}
			else // Fits in A only
			if ( sizeA + bucketSize <= sMaxTeamSize )
			{
				AddTeamMembers( bucket, NET_TEAM_A );
			}
			else // Fits in B only
			if ( sizeB + bucketSize <= sMaxTeamSize )
			{
				AddTeamMembers( bucket, NET_TEAM_A );
			}
			else
				BIZ_ASSERT( "Cant resolve teams" );
		}

		// Cant start if one team is empty
		m_Valid = ( m_Teams[NET_TEAM_A].Size() && m_Teams[NET_TEAM_B].Size() );

		// Are the teams balanced
		m_Difference = Maths::Abs( m_Teams[NET_TEAM_A].Size() - m_Teams[NET_TEAM_B].Size() );
		m_Balanced = m_Valid ? ( m_Difference < sBalanceRange ) : false;

		// Whats the best free space
		m_FreeSpace = CalculateFreeSpace( bucketList, sMaxTeamSize );
		LOGINFO(g_Log, "C_NetTeamBalancer::AssignTeams() - free space %d", m_FreeSpace );

#ifndef CFG_GOLD
		if ( g_Dump_Team_Stats )
		{
			if ( m_Balanced )
			{
				LOGINFO(g_Log, "C_NetTeamBalancer::AssignTeams() - teams balanced with %d difference", m_Difference );
			}
			else
				LOGINFO(g_Log, "C_NetTeamBalancer::AssignTeams() - teams un-balanced with %d difference", m_Difference );
		}
#endif
	}

	S8 C_NetTeamBalancer::CalculateFreeSpace( const NetBucketList& bucketList, const S8 sMaxTeamSize ) const
	{
		// Work out the max size a new party could be to fit on a team
		S8 sizeA = 0, sizeB = 0;
		for ( S8 i = 0; i < bucketList.Size(); i++ )
		{
			const S8 bucketSize = bucketList[i].Size();
			if ( sizeA == 0 )
			{
				sizeA += bucketSize;
			}
			else // Fits in A only
			if ( sizeA + bucketSize <= sMaxTeamSize )
			{
				sizeA += bucketSize;
			}
			else // Fits in B only
			if ( sizeB + bucketSize <= sMaxTeamSize )
			{
				sizeB += bucketSize;
			}
			else
				BIZ_ASSERT( "Cant resolve free space" );
		}

		const S8 spaceA = sMaxTeamSize - sizeA;
		const S8 spaceB = sMaxTeamSize - sizeB;

		return ( spaceA > spaceB ) ? spaceA : spaceB;
	}
	
	void C_NetTeamBalancer::AddTeamMembers( const NetBucket& bucket, const S8 sTeam )
	{
		BIZ_ASSERT( sTeam >= 0 && sTeam < m_Teams.Size() );

		for ( S8 i = 0; i < bucket.Size(); i++ )
		{
			LOGINFO(g_Log, "C_NetTeamBalancer::AddTeamMembers() - Added %S to team %d", bucket[i]->m_User.m_Name, sTeam );

			bucket[i]->m_RaceData.m_Team = sTeam;

			// Store team data
			m_Teams[sTeam].PushBack( bucket[i]->m_RaceConnection.m_JoinId );
		}
	}

	const C_NetTeamBalancer::NetTeam& C_NetTeamBalancer::GetTeam( const S8 sTeam )
	{
		BIZ_ASSERT( sTeam >= 0 && sTeam < m_Teams.Size() );

		return m_Teams[sTeam];
	}

}; // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
