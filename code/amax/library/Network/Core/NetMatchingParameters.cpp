#include <stdafx.h>
#include <bdCore/bdContainers/bdBitBuffer.h>	// [Blur2PC] BD_INT64_NAN (bdBitBufferTypes.h is an X-macro list, only valid inside bdBitBuffer.h)
#include <Core/core.h>
#include "../Misc/NetDebug.h"
#include "NetMatchingParameters.h"
#include "../Router/NetRouterDefines.h"


#ifdef PLATFORM_PS3
#include <bdCore/bdContainers/bdBitBuffer.h>
#endif

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", off )
#endif

namespace RacerNet
{
	void BuildMatchingParameters(NetMatchingParameterList& output)
	{
		NetPlay::SessionParameter tempParam;
		output.Clear();

#if defined ( PLATFORM_PS3 ) || defined ( PLATFORM_PC )
		// first demonware property should be the system game type context, Amax doesnt use this in its queries so we set it to default.
		tempParam.CreateContext(0, 0);
		output.PushBack(tempParam);

		// game defined properties in the same order as they are specified in the matching attributes list within the we imp tool
		// the key attribute is ignored.
		tempParam.CreatePropertyS64(0, 0);
		output.PushBack(tempParam);	
		tempParam.CreatePropertyS32(1, 0);
		output.PushBack(tempParam);
		tempParam.CreatePropertyS32(2, 0);
		output.PushBack(tempParam);
		tempParam.CreatePropertyS32(3, 0);
		output.PushBack(tempParam);
		tempParam.CreatePropertyS32(4, 0);
		output.PushBack(tempParam);	
		tempParam.CreatePropertyS32(5, 0);
		output.PushBack(tempParam);	
		tempParam.CreatePropertyS32(6, 3);
		output.PushBack(tempParam);
		tempParam.CreatePropertyS32(7, 0);
		output.PushBack(tempParam);
		tempParam.CreatePropertyS32(8, 0);
		output.PushBack(tempParam);
		tempParam.CreatePropertyS32(9, 0);
		output.PushBack(tempParam);
#endif
	}

	void BuildMatchingParameters(const NetMatchingServerData& input, NetMatchingParameterList& output, const bool raceSession, const bool modify)
	{
		NetPlay::SessionParameter tempParam;
		output.Clear();

		// race session
		if (modify) 
		{
#if defined ( PLATFORM_PS3 ) || defined ( PLATFORM_PC )
			// first demonware property should be the system game type context, Amax doesnt use this in its queries so we set it to default.
			tempParam.CreateContext(0, 0);
			output.PushBack(tempParam);

			// game defined properties in the same order as they are specified in the matching attributes list within the we imp tool
			// the key attribute is ignored.
			tempParam.CreatePropertyS64(0, input.m_EventId);
			output.PushBack(tempParam);	
			tempParam.CreatePropertyS32(1, input.m_CustomRace);
			output.PushBack(tempParam);
			tempParam.CreatePropertyS32(2, input.m_OpenSlotCount);
			output.PushBack(tempParam);
			tempParam.CreatePropertyS32(3, input.m_Searchable);
			output.PushBack(tempParam);
			tempParam.CreatePropertyS32(4, input.m_RankRestriction);
			output.PushBack(tempParam);
			tempParam.CreatePropertyS32(5, input.m_MatchingMode);
			output.PushBack(tempParam);
			tempParam.CreatePropertyS32(6, input.m_Nat);
			output.PushBack(tempParam);
			tempParam.CreatePropertyS32(7, input.m_PackVersion);
			output.PushBack(tempParam);
			tempParam.CreatePropertyS32(8, input.m_MatchingRegion);
			output.PushBack(tempParam);
			tempParam.CreatePropertyS32(9, input.m_InRace);
			output.PushBack(tempParam);
#endif
		}
		else
		{
#if defined ( PLATFORM_PS3 ) || defined ( PLATFORM_PC )
			// first demonware property should be the system game type context, Amax doesnt use this in its queries so we set it to default.
			tempParam.CreateContext(0, 0);
			output.PushBack(tempParam);

			// game defined properties in the same order as they are specified in the matching attributes list within the we imp tool
			// the key attribute is ignored.
			tempParam.CreatePropertyS64(0, input.m_EventId);
			output.PushBack(tempParam);	
			tempParam.CreatePropertyS32(1, input.m_CustomRace);
			output.PushBack(tempParam);
			tempParam.CreatePropertyS32(2, input.m_OpenSlotCount);
			output.PushBack(tempParam);
			tempParam.CreatePropertyS32(3, input.m_Searchable);
			output.PushBack(tempParam);
			tempParam.CreatePropertyS32(4, input.m_RankRestriction);
			output.PushBack(tempParam);	
			tempParam.CreatePropertyS32(5, input.m_MatchingMode);
			output.PushBack(tempParam);	
			tempParam.CreatePropertyS32(6, input.m_Nat);
			output.PushBack(tempParam);
			tempParam.CreatePropertyS32(7, input.m_PackVersion);
			output.PushBack(tempParam);
			tempParam.CreatePropertyS32(8, input.m_MatchingRegion);
			output.PushBack(tempParam);
			tempParam.CreatePropertyS32(9, input.m_InRace);
			output.PushBack(tempParam);
#endif
		}	
	}

	static S32 ConvertNatToMatchingQueryVal(const S8 natIn)
	{
		switch (natIn)
		{
		case eNatOpen:
			return eNatStrict;	// if we have an open nat we can join any sessions with a strict nat or better.
			break;
		case eNatModerate:
			return eNatModerate; // if we have a moderate nat we can join any sessions with a moderate nat or better.
			break;
		case eNatStrict:
			return eNatOpen;	// if we have a strict nat we can only join sessions with open nats.
			break;
		default:
			BIZ_ASSERT_FAIL("RacerNet::ConvertNatToMatchingQueryVal() - invalid local nat type.");
			break;
		}
		return eNatStrict;
	}

	void BuildMatchingParameters(const NetMatchingSearchData& input, NetMatchingParameterList& output)
	{
		NetPlay::SessionParameter tempParam;
		output.Clear();

#if defined ( PLATFORM_PS3 ) || defined ( PLATFORM_PC )
		tempParam.CreatePropertyS64(0, input.m_EventId > -1 ? input.m_EventId : BD_INT64_NAN);
		output.PushBack(tempParam);	
		tempParam.CreatePropertyS32(1, input.m_CustomRace > -1 ? input.m_CustomRace : BD_INT32_NAN);
		output.PushBack(tempParam);
		tempParam.CreatePropertyS32(2, input.m_NumToJoin > -1 ? input.m_NumToJoin : BD_INT32_NAN);
		output.PushBack(tempParam);
		tempParam.CreatePropertyS32(3, input.m_RankRestriction > -1 ? input.m_RankRestriction : BD_INT32_NAN);
		output.PushBack(tempParam);	
		tempParam.CreatePropertyS32(4, input.m_MatchingMode > -1 ? input.m_MatchingMode : BD_INT32_NAN);
		output.PushBack(tempParam);	
		tempParam.CreatePropertyS32(5, input.m_Nat > -1 ? ConvertNatToMatchingQueryVal(input.m_Nat) : BD_INT32_NAN);
		output.PushBack(tempParam);	
		tempParam.CreatePropertyS32(6, input.m_PackVersion > -1 ? input.m_PackVersion : BD_INT32_NAN);
		output.PushBack(tempParam);	
		tempParam.CreatePropertyS32(7, input.m_MatchingRegion > -1 ? input.m_MatchingRegion : BD_INT32_NAN);
		output.PushBack(tempParam);	
		tempParam.CreatePropertyS32(8, input.m_LobbyOnly > -1 ? !input.m_LobbyOnly : BD_INT32_NAN);
		output.PushBack(tempParam);	
#endif
	}

	void BuildMatchingReturnTypes(NetPlay::SesionQueryReturnTypes& output)
	{
		output.Clear();

#if defined ( PLATFORM_PS3 ) || defined ( PLATFORM_PC )
		output.PushBack(NetPlay::SessionParameter::Int64);	//mm_group_id	
		output.PushBack(NetPlay::SessionParameter::Int32);	//mm_user_created_group
		output.PushBack(NetPlay::SessionParameter::Int32);	//mm_open_slots
		output.PushBack(NetPlay::SessionParameter::Int32);	//mm_searchable
		output.PushBack(NetPlay::SessionParameter::Int32);	//mm_rank_restriction
		output.PushBack(NetPlay::SessionParameter::Int32);	//mm_matching_mode
		output.PushBack(NetPlay::SessionParameter::Int32);	//mm_nat
		output.PushBack(NetPlay::SessionParameter::Int32);	//mm_pack_version
		output.PushBack(NetPlay::SessionParameter::Int32);	//mm_matching_region
		output.PushBack(NetPlay::SessionParameter::Int32);	//mm_inrace
#endif

	}

	U32 MatchingQueryId(void)
	{
#if defined ( PLATFORM_PS3 ) || defined ( PLATFORM_PC )
		// demonware query id to use. defined in web imp
		return 1;
#endif
		return 0;
	}

} // end namespace RacerNet

#if defined ( RACER_NET_OPTIMIZE_OFF )
	#pragma optimize( "", on )
#endif
