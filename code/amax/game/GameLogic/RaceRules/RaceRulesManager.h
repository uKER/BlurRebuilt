#ifndef RACE_RULE_MANAGER_H
#define RACE_RULE_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\RaceRuleBase.h>
#include <misc_shared/Messaging/MessageHandler.h>	// [Blur2PC]
#include <GameLogic\RaceRules\RaceAction_Base.h>
#include <GameLogic\GameData\GameEventData.h>
#include <GameLogic\Messaging\GameLogicMessages.h>
#include <GameLogic\Structures\LevelResult.h>	// [Blur2PC] Game::C_LevelResult

namespace Game
{

class C_RaceRulesManager : public Messaging::C_MessageHandler
{
public:
	~C_RaceRulesManager() {RemoveAllSubscribers();};

	void Init();

	static C_RaceRulesManager& Instance(){return s_instance;};
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	void ClearAllRules();
	void InitRulesFromEvent(int numCars, Game::C_LevelResult& levelResult, AmaxEvent* ev);
	void Update();

	void AddPassRule(RaceModeRuleBase *r,int index);
	void AddFailRule(RaceModeRuleBase *r,int index);
	void AddLapAction(RaceActionBase* act);
	void AddTimeAction(RaceActionBase* act);

	void SetInstaPass(bool ip)   { mInstaPass = ip; };
	void SetInstaFail(bool insf) { mInstaFail = insf; };

	bool TrackVehicle(int idx);
	int  GetNumPassRules() { return ( mCurrentRule > -1  && mRaceRulesPass.Size() ) ? 1 : mRaceRulesPass.Size(); };
	bool ValidPassRules();
	int  GetNumFailRules() { return mRaceRulesFail.Size(); };
	const int GetNumTimeActions() const { return mRaceTimeActions.Size(); }
	bool HasPassed(int vehicleIndex, int ruleIdx, bool pass);
	

	void GetRuleStringOverride( UI::C_WideString512& str);  

	int  GetNumEliminated() { return mNumEliminated; };
	void IncrementNumEliminated() { mNumEliminated++; };

	const void GetPassRuleString(U32 idx, UI::C_WideString512& str, int rule, RACE_MODE_RULE_STRING_TYPE strType );
	const void GetFailRuleString(U32 idx, UI::C_WideString512& str, int rule, RACE_MODE_RULE_STRING_TYPE strType );
	
	void SendPassMessage(int vehIdx);
	void SendFailMessage(int vehIdx);

	Btl::Vector<RaceActionBase*>&	GetTimeActions()	{ return mRaceTimeActions; }

	RaceModeRuleBase* GetRaceRulePass( S32 i );
	RaceModeRuleBase* GetRaceRuleFail( S32 i )		{ return ( i >= 0 && i < mRaceRulesFail.Size() ) ? mRaceRulesFail[ i ] : NULL; }
	RaceActionBase* GetTimeAction( S32 i );

private:
	static C_RaceRulesManager s_instance;

	Btl::Vector<RaceModeRuleBase*, Btl::VectorParameters_Dynamic<> >	mRaceRulesPass;
	Btl::Vector<RaceModeRuleBase*, Btl::VectorParameters_Dynamic<> >	mRaceRulesFail;
	Btl::Vector<RaceActionBase*, Btl::VectorParameters_Dynamic<> >		mRaceLapActions;
	Btl::Vector<RaceActionBase*, Btl::VectorParameters_Dynamic<> >		mRaceTimeActions;

	bool				mInstaPass;
	bool				mInstaFail;
	int					mCurrentRule;
	UI::C_TextStringID	mRuleStrOverride;
	int					mNumEliminated;

	Btl::Vector<UI::C_TextStringID> mRuleSeqStrOverride;
	bool mSequentialRuleSet;
};

} // namespace Game

#endif // RACE_RULE_MANAGER_H
