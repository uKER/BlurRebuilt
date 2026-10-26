#ifndef UITimer_H
#define UITimer_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtGame.h>

namespace UI
{

class UITimer
{
public:
	enum E_Type
	{
		E_Type_Wrap = 0,
		E_Type_Bounce,
		E_Type_Stop,
	};

public:
	UITimer( )
	{
		m_Name = XtSz8(NULL);
		m_LuaVariable = XtSz8(NULL);
	}

	void Init(E_Type type, F32 time, F32 start_time);
	void Update();

	E_Type		m_DoWhat;
	XtSz8		m_Name;
	XtSz8		m_LuaVariable;
	XtF32		m_Time;
	XtF32		m_StartTime;				// This the time to set when the SCREEN_TIME changes, not the loop
	XtF32		m_DoTime;
	XtF32		m_fStartBW;
	S32			m_Player;
	S32			m_sCurve;
	char		m_Variable[32];
	bool		m_TimerDirc;
	bool		m_UseLuaGlobalVariable;
};

}; // namespace UI
#endif // UITimer_H
