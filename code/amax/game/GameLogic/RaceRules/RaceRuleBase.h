#ifndef RACE_RULE_BASE_H
#define RACE_RULE_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Util/WideString.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <GameLogic/Layers/RaceLogicLayerEnums.h>

namespace Game
{

enum RACE_MODE_RULE_STRING_TYPE
{
	RACE_MODE_RULE_STRING_TYPE_HUD_TARGET,
	RACE_MODE_RULE_STRING_TYPE_HUD_CURRENT,
	RACE_MODE_RULE_STRING_TYPE_HUD_BONUS,
	RACE_MODE_RULE_STRING_TYPE_RESULTS,	
};

enum RaceModeRuleType
{
	// These must be first, in this order, for event creation in Lua
	E_RaceRule_FinishInPosition,
	E_RaceRule_GetLapTime,
	E_RaceRule_HitMph,

	// The rest are ignored by Lua
	E_RaceRule_BeatDriver,
	E_RaceRule_DriverLastInRace,
	E_RaceRule_PerformActionYTimes,
	E_RaceRule_DriverHasHealth,
	E_RaceRule_DistanceFromDriver,
	E_RaceRule_CauseDamageToDriver,
	E_RaceRule_HitDriverWithPerk,
	E_RaceRule_FireXPerks,
	E_RaceRule_EarnXP,
	E_RaceRule_PerformXTrickShots,
	E_RaceRule_CollectFanPickups,
	E_RaceRule_GetFastestLap,
	E_RaceRule_TimerExpired,
	E_RaceRule_InTriggerRegion,
	E_RaceRule_HitObjects,
	E_RaceRule_TeamPosition,
};

class RaceModeRuleBase
{
public:
	RaceModeRuleBase();
	virtual ~RaceModeRuleBase()	{}

	virtual void				Init(int idx)				{ mIndex = idx; };
	virtual bool				IsTriggered(int vehicleIndex) = 0;
	virtual RaceModeRuleType	GetType() = 0;
	virtual bool				TrackVehicle(XtId driverId) { return false; };
	virtual void				PreCache() {};
	virtual bool				IsValid()					{ return true; }
	virtual void				Update();
	
	void						SetPersistent(bool p)		{  mPersistent = p; };
	bool						IsPerisistent()				{ return mPersistent; };

	virtual void				Serialise( Sys::BinarySerialiserAutoEndian& stream );
	virtual void				Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

	bool						DontShowOnHUD(int vehicleIndex)	{ return mDontShowOnHUD[vehicleIndex]; }

	bool						PulseRuleString()	{ return mPulseString; }

	virtual void				GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule);
	virtual void				CacheStrings() {};

protected:

	int  mIndex;
	bool mPersistent;
	bool mPulseString;
	F32	 mPulseTimer;
	F32	 mPulseFreq;

	Btl::Vector<bool, Btl::VectorParameters_Array<MAX_RACELOGIC_VEHICLES > >	mDontShowOnHUD;
};

} // namespace Game

#endif // RACE_RULE_BASE_H
