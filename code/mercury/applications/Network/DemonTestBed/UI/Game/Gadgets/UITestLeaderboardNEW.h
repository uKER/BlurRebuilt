#ifndef UITESTLEADERBOARDNEW_H
#define UITESTLEADERBOARDNEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>
#include <ui/Game/Gadgets/UIStaticText.h>
#include <ui/Game/Gadgets/UILeaderboard.h>
#include "UIManager.h"

namespace UI
{

class UITestLeaderboardNEW : public UIStaticText
{
public:
	void Init();
	void Setup();
	void Render();

	static const UIButtonTypeID		TypeID()		{ return E_ButtonType_LeaderboardTest; }
	static const Util::C_String32	TypeName()		{ return "LEADERBOARD_TEST"; }

private:
	C_UILeaderboard					m_Leaderboard;	// the leaderboard
};

};	// namespace UI

#endif	// UITESTLEADERBOARDNEW_H
