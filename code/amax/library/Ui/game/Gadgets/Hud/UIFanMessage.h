#ifndef UIFANMESSAGE_H
#define UIFANMESSAGE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIGameDefines.h"
#include <ui/Game/Gadgets/Buttons.h>

#include "GameLogic/RaceLogicCore/Data/RLEnums.h"
#include "GameLogic/RaceModes/RaceModeDefines.h"
#include "ui/Util/WideString.h"

namespace UI
{

class UIStaticGraphic;
class UIStaticText;
class UIRenderBranch;

class C_UiScoringEventMinor;
class C_UiScoringEventChain;
class C_UiScoringEventPerk;
class C_UiScoringEventFans;
class C_UiFanDemand;

// ------------
// UIFanMessage
// ------------
class UIFanMessage : public C_GadgetBase
{
public:
	enum E_Viewports
	{
		E_Viewports_Max = 4,
	};

	typedef enum E_Mode
	{
		E_Mode_None,
		E_Mode_Once,
		E_Mode_Added,
	};

public:
	DECLARE_GADGET( E_UIGameButtonType_FanMessage, "FAN_MESSAGE" )

public:
	UIFanMessage() : C_GadgetBase(), m_sLocalPlayerIndex( -1 ) {  }

public:	
	void				Init();
	void				PostInit();
	bool				MessageHandler( const Messaging::C_MessageBase* pMessage );

private:
	void				ProcessBonus( const C_UiScoringEventChain* pMessage );
	void				ProcessHitByPerk( const C_UiScoringEventPerk* pMessage );
	void				ProcessFanDemand( const C_UiFanDemand* pMessage );

	//void				CheckPerkEffects( const Messaging::C_MessageBase* pMessage );
	//void				DoTimeLine( const char* pName, const bool bActive = true, const F32 fTime = 0.0f );

	void				CallChildLine( const char* name, const bool active, const F32 time = -1.0f, const bool reset = false );

public: // exposed for Xt
	F32					m_fFanGapX;

private:
	S32					m_sLocalPlayerIndex;
	S8					m_VehicleIndex;

	ButtonArray			m_Children;

	UIRenderBranch*		m_FanBranch;
	UIRenderBranch*		m_FanDemandBranch;

	// fan scoring
	UIRenderBranch*		m_pFanMessageRoot;
	UIStaticText*		m_pFanMessage;
	UIStaticText*		m_pFanMessageName;
	UIStaticText*		m_pFanMessageNum;
	UIRenderBranch*		m_pFanBackingSingle;
	UIRenderBranch*		m_pFanBackingDouble;
	UIStaticGraphic*	m_pFanBackingSingleBlob;
	UIStaticGraphic*	m_pFanBackingDoubleBlob;

	C_WideString128		m_Message;
	C_WideString32		m_MessageName;
	C_WideString16		m_FansAdded;

	// bonus scoring
	UIRenderBranch*		m_pFanBonusMessageRoot;
	UIStaticText*		m_pFanBonusMessage;
	UIStaticText*		m_pFanBonusMessageNum;
	UIStaticGraphic*	m_pFanBonusBackingSingleBlob;

	C_WideString128		m_BonusString;
	C_WideString16		m_BonusFansAdded;

	// fan demands
	UIStaticGraphic*	m_pFdIcon;
	UIStaticText*		m_pFdName;
	UIStaticText*		m_pFdAmount;
	UIStaticGraphic*	m_pFdBlob;
	UIStaticGraphic*	m_pFdWinIcon;
	UIStaticText*		m_pFdWinName;
	UIStaticText*		m_pFdWinAmount;
	UIStaticGraphic*	m_pFdWinBlob;
	UIStaticGraphic*	m_pFdFailIcon;
	UIStaticText*		m_pFdFailName;
	UIStaticGraphic*	m_pFdFailBlob;

	F32					m_fFdMaxNumLength;
	S32					m_sFdFans;

	C_WideString128		m_FdName;
	C_WideString16		m_FdAmount;
	C_WideString16		m_FdAmountWin;

	Game::E_RaceMode	m_eRaceMode;
	bool				m_bSplitscreen;
};

};	// namespace UI

#endif /* UIFANMESSAGE_H */
