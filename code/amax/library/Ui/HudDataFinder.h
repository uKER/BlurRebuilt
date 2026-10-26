#ifndef HUDDATAFINDER_H
#define HUDDATAFINDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/UIDataFinder.h>
#include <actor_system/ActorDeclare.h>

namespace UI
{

// ---------------
// C_HudDataFinder
// ---------------
class C_HudDataFinder : public C_UIBaseGameDataFinder
{
private:
	C_HudDataFinder() : C_UIBaseGameDataFinder( "HUD_" )		{ Init(); }
	C_HudDataFinder( const C_HudDataFinder& o )					{  }

public:
	void									Init();

	const WCHAR*							GetVariableText( const char* pVariableName );

	void									SetBonusString( const Util::C_String& string )					{ m_BonusString = string; }
	void									SetScoreString( const Util::C_String& string )					{ m_ScoreString = string; }

	bool									DistanceFromDriverFailing()										{ return m_bDistanceFromDriverFailing; }
	S32										NumActiveRules()												{ return m_sNumActiveRules; }

	void									AddTriggerRegionActor( const Actor::C_ActorHandle& handle )		{ m_TriggerRegionActors.PushBack( handle ); }
	Btl::Vector<Actor::C_ActorHandle>&		GetTriggerRegionActors()										{ return m_TriggerRegionActors; }

private:
	const WCHAR*							GetVariableTextSP( const char* pVariableName );

public:
	static C_HudDataFinder&					Instance()						{ return m_sInstance; }
private:
	static C_HudDataFinder					m_sInstance;

private:
	Util::C_String							m_BonusString;		// C_String - wtf - TODO - change!
	Util::C_String							m_ScoreString;

	bool									m_bTlmSentMpStampTimer;
	bool									m_bTlmSentMpShowTimer;

	bool									m_bDistanceFromDriverFailing;
	S32										m_sNumActiveRules;

	Btl::Vector<Actor::C_ActorHandle>		m_TriggerRegionActors;

	Btl::Vector< S32, Btl::VectorParameters_Array< 4 > >	m_LocalPlayerPosition;
};

}; //namespace UI

#endif /* HUDDATAFINDER_H */
