#ifndef UIPOPUPCONTROL_H
#define UIPOPUPCONTROL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIDefines.h"

namespace UI
{

enum UI_POPUPCONTROL_STATE
{
	UI_POPUPCONTROL_STATE_INIT,
	UI_POPUPCONTROL_STATE_START_LOOP,

	UI_POPUPCONTROL_STATE_UPDATE,

	UI_POPUPCONTROL_STATE_END,
	UI_POPUPCONTROL_STATE_END_LOOP,

	UI_POPUPCONTROL_STATE_MAX,
};

enum E_PopupPositionX
{
	E_PopupPositionX_Left = 0,
	E_PopupPositionX_Centre,
	E_PopupPositionX_Right,
	E_PopupPositionX_Count
};
enum E_PopupPositionY
{
	E_PopupPositionY_Top = 0,
	E_PopupPositionY_Middle,
	E_PopupPositionY_Bottom,
	E_PopupPositionY_Count
};
enum E_PopupStatus
{
	E_PopupStatus_None = 0,
	E_PopupStatus_Active,
	E_PopupStatus_Finished_Okay,
	E_PopupStatus_Finished_Cancel,
	E_PopupStatus_Count
};

class UIPopupControl
{
public:
	UIPopupControl(){mActive=false;};

	void Init();
	void Update();

	const S32			StartStandardPopup( const char *pMessage, const bool canCancel = true, const bool darkenBG = true, const E_PopupPositionX pos_x = E_PopupPositionX_Centre, const E_PopupPositionY pos_y = E_PopupPositionY_Middle, const S32 sTimeout = -1 );
	const E_PopupStatus GetStandardPopupState( const S32 handle );

	void SetTimers(float start,float end){mStartTimer=start;mEndTimer=end;};

	void SetLuaPopupDir(const char *d)		{ STRCPY( mLuaPopupDir, m_uMaxScriptNameLength, d ); }
	void SetPopup(const char *ns)			{ mActive = true; STRCPY( mNextPopup, m_uMaxScriptNameLength, ns ); }
	void EndPopup(const char *str);
	void KillPopup( );

	UI_POPUPCONTROL_STATE State( ) { return mState; }

	bool IsActive(){return mActive;};

	void Clear();

private:
	bool mActive;
	float mTimer;

	UI_POPUPCONTROL_STATE mState;

	static const U32	m_uMaxScriptNameLength = 128;

	char				mLuaPopupDir[ m_uMaxScriptNameLength ];
	char				mNextPopup[ m_uMaxScriptNameLength ];	
	char				mNextScreen[ m_uMaxScriptNameLength ];
    
	float mStartTimer;
	float mEndTimer;
	bool m_FatalScriptError;
	bool mResetButtonTimers;
};

extern UIPopupControl gUIPopupControl;

}; // namespace UI

#endif
