#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic\RaceRules\RaceRuleBase.h>
#include <GameApp.h>

#include "ui/UiXtConfig.h"

Debug::Logger &g_RaceRuleBaseLogger( Debug::GetLogger("GameMode.RaceRules.Base") );

namespace Game
{

RaceModeRuleBase::RaceModeRuleBase()
{
	for(int i = 0; i < MAX_RACELOGIC_VEHICLES; i++)
		mDontShowOnHUD[i] = false;

	// DG - HUD stuff
	mPulseFreq = 0.0f;
	if( UI::C_UiXtConfig::Instance() )
		mPulseFreq = UI::C_UiXtConfig::Instance()->m_PulseDefaultFreq;

	mPulseString = false;
	mPulseTimer = mPulseFreq;
}

void RaceModeRuleBase::Serialise( Sys::BinarySerialiserAutoEndian& stream )
{
	Sys::BinarySerialiserAutoEndian out( stream );
	out << mIndex;
}

void RaceModeRuleBase::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
{
	Sys::BinaryDeserialiserAutoEndian in( stream );
	in >> mIndex;
}

void RaceModeRuleBase::GetRuleString(U32 idx, UI::C_WideString512& str, RACE_MODE_RULE_STRING_TYPE stringType, bool isPassRule)
{	
	if(isPassRule)
		str.Format(L"<Pass rule unknown>");
	else
		str.Format(L"<Fail rule unknown>");
}

void RaceModeRuleBase::Update()
{
	// Just pulse every 'n' seconds
	if( mPulseFreq > 0.0f )
	{
		mPulseString = false;

		mPulseTimer -= C_GameApp::GetInstance()->GetTimeStep();
		if( mPulseTimer <= 0.0f )
		{
			mPulseTimer += mPulseFreq;
			mPulseString = true;
		}
	}
}

}