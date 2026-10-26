
#ifndef GAME_MULTIPLAYER_CHALLENGE_SYSTEM_H
#define GAME_MULTIPLAYER_CHALLENGE_SYSTEM_H

#include "UI/Game/Multiplayer/MultiplayerDefines.h"

#include "GameLogic/Structures/MultiplayerChallenge.h"
#include "GameLogic/Profile/MultiplayerChallengeTracker.h"
#include "GameLogic/MultiplayerCarChallenges.h"

// Forward declarations
struct MultiplayerChallengePack;

namespace Sys
{
	class BinarySerialiserAutoEndian;
	class BinaryDeserialiserAutoEndian;
}

// forward declarations
struct AmaxVehicle;

namespace Game
{
/*---------------------------------------------------------------------------------------
	Name : C_MultiplayerChallengeSystem

	Desc : Challenge system manager class which provides functionality to track and maintain
		   the multiplayer challenges.
---------------------------------------------------------------------------------------*/
class C_MultiplayerChallengeSystem
{
public:
	typedef Btl::Vector< C_MultiplayerChallenge, Btl::VectorParameters_Array< UI::E_MULTIPLAYER_CHALLENGE_TYPE_MAX > >	TChallengeList;				// List containing challenges
	typedef Btl::Map< U32, C_MultiplayerCarChallenges >																	TCarMap;					// Map of car challenges.

public:
	C_MultiplayerChallengeSystem();

												// Init
	void										Initialise();
	void										Reset();

												// TODO_MP_TS - remove me post beta and fix up the multiplayer packs
	void										ReloadSorceryData();

	void 										Serialise( Sys::BinarySerialiserAutoEndian& stream );
	void 										Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

												// Calculates the number of challenges complete in each pack and caches them.
	void										CachePackProgress();

												// Returns the current pack that the UI is viewing
	const MultiplayerChallengePack*				GetViewingPack() const;

												// Returns the challenge object for the given type.
	C_MultiplayerChallenge&						GetCarChallenge( const U32 uCarID, const UI::E_MULTIPLAYER_CHALLENGE_TYPE eType );

												// Increases the current amount of the challenge. It is capped to the max tiers target.
	void										IncreaseChallengeAmount( const UI::E_MULTIPLAYER_CHALLENGE_TYPE eType, const S32 sAmount = 1);

												// Increases the current amount of the challenge. It is capped to the max tiers target.
	void										IncreaseCarChallengeAmount( const U32 uCarID,  const UI::E_MULTIPLAYER_CHALLENGE_TYPE eType, const S32 sAmount = 1 );

												// Returns the number of challenges completed in the pack
	S32											GetPackProgressCounter( const UI::E_ChallengePack ePack ) const
												{
													BIZ_ASSERT( ePack >= 0 && ePack < UI::E_ChallengePack_Count );
													return m_PackCounterCache[ePack];
												}

												// Returns the number of challenges completed in the car pack
	S32											GetCarPackCounter() const
												{
													return m_CarPackCounter;
												}

												// Returns the total number of challenges in the car pack
	S32											GetCarPackTotal() const
												{
													return m_CarPackTotal;
												}

	const Btl::BitArray16&						GetCarUpgradesEnabled( const U32 uCarID );

	void										ForceCarUpgrades(const U32 uCarID);

	void										GetCarClassProgress( const UI::E_VehicleClass eClass, S32& sAmount, S32& sMax ) const
												{
													sAmount = m_ClassCounterCache[eClass];
													sMax = m_ClassTotalCache[eClass];
												}

												// Returns the challenge object for the given type.
	const C_MultiplayerChallenge&				GetChallenge( const UI::E_MULTIPLAYER_CHALLENGE_TYPE eType ) const
												{
													return m_Challenges[eType];
												}

												// Returns the challenge object for the given type. // not const because we might be modifying it
	C_MultiplayerChallenge&						GetChallenge( const UI::E_MULTIPLAYER_CHALLENGE_TYPE eType )
												{
													return m_Challenges[eType];
												}

												// Sets the current pack that the UI is viewing.
	void										SetViewingPack( const UI::E_ChallengePack ePack )
												{
													m_PackViewingType = ePack;
												}

	C_MultiplayerChallengeTracker&				GetTracker()
												{
													return m_Tracker;
												}

	UI::E_MULTIPLAYER_CHALLENGE_TYPE			GetLastChallengeCompleted()
												{
													return m_LastChallengeCompleted;
												}

	U8											GetLastChallengeCompletedLevel()
												{
													return m_LastChallengeCompletedLevel;
												}

	U32											GetLastChallengeCompletedCarId()
												{
													return m_LastChallengeCarId;
												}

	bool										IsPackComplete( const UI::E_ChallengePack ePack ) const;

private:
	void										CacheCarProgress( const AmaxVehicle* pCar );

private:
	S32											GetPackUnlockLevel(const MultiplayerChallengePack* pPack) const;
	void										InitCarChallenges( MultiplayerChallengePack& pack);
	void										InitNormalCarChallenges( MultiplayerChallengePack& pack );
	void										InitLegendCarChallenges( MultiplayerChallengePack& pack );
	void										InitDLCCarChallenges( MultiplayerChallengePack& pack );
	C_MultiplayerCarChallenges*					ReloadCarChallengesSorcery( XtId vehicleId, MultiplayerChallengePack& pack);
	S32											ReloadChallengesSorcery();
	void										RemoveInvalidCarChallenges();
	void										InvalidateCarChallenges();
	void										AddCarChallenge(XtId carId, MultiplayerChallengePack& pack);

												// Lets the UI show updated challenge progress with in-race popups 
	void										CheckForUIChallengeProgress( const UI::E_MULTIPLAYER_CHALLENGE_TYPE eType, const float oldProgress );
	void										ShowUIChallengeProgress( const UI::E_MULTIPLAYER_CHALLENGE_TYPE eType );
	bool										ChallengeProgressVisible( const UI::E_MULTIPLAYER_CHALLENGE_TYPE eType );



	UI::E_ChallengePack							m_PackViewingType;			// Current pack the UI is viewing
	TChallengeList								m_Challenges;				// List of all the challenges. This isnt broken into packs.
	TCarMap										m_CarMap;					// Map of car challenges

	C_MultiplayerChallengeTracker				m_Tracker;

	// Cached data
	S32											m_PackCounterCache[UI::E_ChallengePack_Count];
	S32											m_ClassCounterCache[UI::E_VehicleClass_Max];	// Total progress for the car challenges in this class
	S32											m_ClassTotalCache[UI::E_VehicleClass_Max];		// Total number of car challenges in this class

	S32											m_CarPackCounter;	// Sum of class counters
	S32											m_CarPackTotal;		// Sum of class totals

	UI::E_MULTIPLAYER_CHALLENGE_TYPE			m_LastChallengeCompleted;
	U8											m_LastChallengeCompletedLevel;
	U32											m_LastChallengeCarId;

	U32											m_MinChallengeSize;	
	U32											m_IntroZone;		
	U32											m_OutroZone;			
	U32											m_PercentInterval;	

};

}; // namespace Game

#endif // GAME_MULTIPLAYER_CHALLENGE_SYSTEM_H
