#ifndef UIREQUIREMENTSMESSAGE_H
#define UIREQUIREMENTSMESSAGE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIGameDefines.h"
#include <ui/Game/Gadgets/Buttons.h>

#include "ui/Util/WideString.h"
#include "Ui/UiMessages.h"

#define REQ_MSG_QUEUE_MAX 8

namespace UI
{

class UIStaticGraphic;
class UIStaticText;
class UIRenderBranch;
class UIStaticBox;

// ---------------------
// UIRequirementsMessage
// ---------------------
class UIRequirementsMessage : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_RequirementsMessage, "REQUIREMENTS_MESSAGE" )

private:
	struct ProgressMessage
	{
		ProgressMessage( const C_UiProgressMessage* m_pProgress )
			: m_Name( m_pProgress->mName ), m_Description( m_pProgress->mDescription ), m_fProgress( m_pProgress->mProgress ),
			m_fGoal( m_pProgress->mGoal ), m_bOneShot( m_pProgress->mOneShot ), m_bBossDemand( false ), 
			m_eStickerCategory( m_pProgress->mStickerCategory ), m_sStickerPart( m_pProgress->mStickerPart ), m_eChallengePack( m_pProgress->mChallengePack )
		{
			//
		}

		ProgressMessage()
			: m_fProgress( 0.0f ), m_fGoal( 1.0f ), m_bOneShot( false ), m_bBossDemand( false ),
			m_eStickerCategory( Game::BLUR_ACHIEVEMENT_CATEGORY_NONE ), m_sStickerPart( -1 ), m_eChallengePack( E_ChallengePack_None )
		{
			//
		}

		C_WideString64						m_Name;
		C_WideString64						m_Description;
		F32									m_fProgress;
		F32									m_fGoal;
		bool								m_bOneShot;
		bool								m_bBossDemand;
		Game::BLUR_ACHIEVEMENT_CATEGORY		m_eStickerCategory;
		S8									m_sStickerPart;
		E_ChallengePack						m_eChallengePack;
	};

public:
	UIRequirementsMessage() : C_GadgetBase(), m_sLocalPlayerIndex( -1 ) {  }

	void				Init();
	void				Setup( lua_State* pState );
	void				PostInit();
	bool				MessageHandler( const Messaging::C_MessageBase* pMessage );
	void				FrameUpdate();

private:
	void				AddMessageToQueue( const C_UiProgressMessage* pMessage );
	void				AddMessageToQueue( const ProgressMessage& msg );
	void				ProcessMessageQueue();
	void				ProcessMessage( const ProgressMessage& msg );
	void				DisplayMessage( const ProgressMessage& msg, const bool bUpdate = false );

	bool				CheckGadgetPointers();

	bool				CheckAndUpdateForDupes( const ProgressMessage& msg );

public:	// exposed for Xt
	F32					m_fProgressMessageTime;

private:
	Btl::Vector< ProgressMessage, Btl::VectorParameters_Static< REQ_MSG_QUEUE_MAX > > m_MessageQueue;

	S32					m_sLocalPlayerIndex;
	S8					m_VehicleIndex;

	UIRenderBranch*		m_pBranch;
	UIStaticBox*		m_pBossPic;
	UIStaticBox*		m_pBossFrame;
	UIStaticGraphic*	m_pStickerIcon;
	UIStaticGraphic*	m_pChallengeIcon;
	UIStaticBox*		m_pBar;
	UIStaticGraphic*	m_pTick;
	UIStaticText*		m_pTitle;
	UIStaticText*		m_pStickerNum;
	UIStaticText*		m_pDesc;
	UIStaticText*		m_pDescComplete;
	UIStaticGraphic*	m_pBackingTop;
	UIStaticGraphic*	m_pBackingTop2;
	UIStaticGraphic*	m_pBackingMiddle;
	UIStaticGraphic*	m_pBackingBottom;

	C_WideString64		m_Name;
	C_WideString64		m_Desc;
	C_WideString32		m_Amount;
	C_WideString16		m_StickerNum;

	F32					m_fDisplayTime;
	F32					m_fBarSizeX;

	ProgressMessage		m_MessageDisplayed;

	C_UISpriteColours	m_ColourNormal;
	C_UISpriteColours	m_ColourMultiplayer;
	C_UISpriteColours	m_ColourPowerup;
	C_UISpriteColours	m_ColourBoss;
	C_UISpriteColours	m_ColourTextSticker;
	C_UISpriteColours	m_ColourTextOther;

	F32					m_fChallengeIconU;
};

};	// namespace UI

#endif /* UIREQUIREMENTSMESSAGE_H */
