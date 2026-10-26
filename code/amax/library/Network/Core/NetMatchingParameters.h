// ***************************************************************
//  NetMatchingParameters  version:  1.0   ·  date: 01/30/2009
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	builds the matching parameter list for all online 
//			session types.
// ***************************************************************

#ifndef __NET_MATCHING_PARAMETERS_H__
#define __NET_MATCHING_PARAMETERS_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/vector.h>
#include <Network/SessionParameter.h>

namespace RacerNet
{
	/// defines the different game modes we matchmake on.
	typedef enum eNetMatchingModes
	{
		eNetMatchingModeNormal = 0,
	};

	/// variables passed to the matching servers when creating / updating a session.
	struct NetMatchingServerData
	{
		NetMatchingServerData() :	m_MatchingMode(0), m_RankRestriction(0), m_MatchingRegion(0), m_PackVersion(0), m_EventId(0), 
									m_CustomRace(0), m_OpenSlotCount(0), m_Searchable(1), m_InRace(0), m_Nat(1)
		{}
		S32	m_MatchingMode;				// what mode we creating.
		S32 m_RankRestriction;			// current level of the host. defines the lowest level of player who should enter.
		S32	m_MatchingRegion;			// region of the host player for filtering purposes.
		S32 m_PackVersion;				// sorcery pack version.
		S64	m_EventId;					// can be either sorcery Id or a user created group.
		U8	m_CustomRace;				// if m_GroupId is a sorcery record than this should be 0 otherwise 1
		U8	m_OpenSlotCount;			// how many player slots are still available to be joined.
		U8	m_Searchable;				// 1 if the session should be returned in response to a search query, 0 means hide the session.
		U8	m_Nat;						// most restrictive Nat in the session
		U8  m_InRace;					// if the game is in the race and not in lobby.
	};

	/// variables passed to the matching servers when we wish to search for a session.
	/** Passing -1 for any of the values results in that paramreter being ignored in the server query
	 */
	struct NetMatchingSearchData
	{
		NetMatchingSearchData() :	m_MatchingMode(-1), m_RankRestriction(-1), m_MatchingRegion(-1), m_PackVersion(-1), m_EventId(-1), 
									m_CustomRace(-1), m_NumToJoin(-1), m_Nat(-1), m_LobbyOnly(-1)
		{}
		S32	m_MatchingMode;				// what mode are we searching for.			
		S32	m_RankRestriction;			// defines the level of the player who is searching. 
		S32	m_MatchingRegion;			// defines the matching region of the player who is searching.
		S32	m_PackVersion;				// sorcery pack version.
		S64	m_EventId;					// can be either sorcery Id or a user created group.
		S8	m_CustomRace;				// if m_GroupId is a sorcery record than this should be 0 otherwise 1
		S8	m_NumToJoin;				// how many players will be required to join the session.
		S8	m_Nat;						// most restrictive Nat that wishes to join
		S8  m_LobbyOnly;				// if look for games that are in lobby only.
	};

	/// list of possible parameters that we create online sessions with. 
	typedef Btl::Vector<NetPlay::SessionParameter, Btl::VectorParameters_Static<20> > NetMatchingParameterList;
	
	/// Build list of default parameters.
	/** This is used for single player stats only sessions, where the server session is created but is not joinable.
	 *	@param output number of parameters required to service the matching session.
	 */
	void BuildMatchingParameters(NetMatchingParameterList& output);

	/// Build a list of parameters for creating / updating a session session.
	/** This version is used for network games. 
	 *	@param input should contain correctlt setup variables that will be used to build the output parameters from.
	 *	@param output number of parameters required to service the matching session.
 	 *	will contain 0 parameters if an error occurred.
	 *	@param raceSesion true if the input/out parameters relate to a race session or false they relate to a party session.
 	 *	@param modify true if we are attempting to modify an already running session or false if
	 *	we are creating the initial session.
	 */
	void BuildMatchingParameters(const NetMatchingServerData& input, NetMatchingParameterList& output, const bool raceSession, const bool modify);

	/// Build a list of parameters required when searching for matching sessions.
	/** This version is ised for network games. 
	 *	@param input should contain correctly setup variables that will be used to build the output parameters from.
	 *	@param output number of parameters required to service the matching session.
	 *	will contain 0 parameters if an error occurred.
	 */
	void BuildMatchingParameters(const NetMatchingSearchData& input, NetMatchingParameterList& output);

	/// When using demonware the session enumeration returns all server attributes that were used to create the session,
	/** we need to pass the types of the variables that will be returned so the enumerator can parse the data stream correctly.
	 *	@param output will contain the types the server parameters in the correct order that should be returned. This order can be 
	 *	obtained from the matching Attributes within the web imp tool. This is not required on xbox360. 
	 */
	void BuildMatchingReturnTypes(NetPlay::SesionQueryReturnTypes& output);

	/// Returns the query Id to use when enumerating matching sessions.
	U32 MatchingQueryId(void);

}// end namespace RacerNet

#endif //__NET_MATCHING_PARAMETERS_H__
