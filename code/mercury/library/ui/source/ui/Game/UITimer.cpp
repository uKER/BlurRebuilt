#include "stdafx.h"

#include "ui/UIDefines.h"
#include "ui/Game/UIVariableStore.h"
#include "ui/Graphics/SimpleTimer.h"
#include "ui/Game/UILUA.h"

#include "ui/Game/UITimer.h"

namespace UI
{

void UITimer::Init(E_Type type, F32 time, F32 start_time)
{
	m_DoWhat				= type;
	m_DoTime				= time;
	m_TimerDirc				= true;
	m_Time					= start_time;
	m_StartTime				= start_time;
	m_Player				= -1;
	m_Variable[0]			= NULL;
	m_fStartBW				= -1.0f;
	m_sCurve				= -1;
}

void UITimer::Update()
{
	bool bIsActive = false;

	// PR - this is ballsed up - not sure why you're doing it like this - see me if it's wrong!

	/*if(m_Variable[0])
	{
		if(m_Player == -1)
			bIsActive = (GetVariableStore()->Get(m_Variable) != 0);
		else
			bIsActive = (GetVariableStore()->Get(m_Variable,m_Player) != 0);
	}
	else
	*/
	if(m_Variable && m_Variable[0])
	{
		if(m_UseLuaGlobalVariable) // Get out of UIGlobals
		{
			lua_State *pState = g_pUILua->GetState();

			lua_getglobal(pState, LUA_GLOBAL_TABLE_NAME);
			if(lua_isnil(pState,-1))
			{
				lua_pop(pState,1);
			}
			else
			{
				lua_pushstring(pState, m_Variable);
				lua_rawget(pState, -2);
				if(lua_isnumber(pState,-1))
				{
					bIsActive = (S32)(lua_tonumber(pState,-1) != 0);
					lua_pop(pState,1);
				}
				else
					lua_pop(pState,1);

				lua_pop(pState,1);
			}
		}
		else
		{
		}
		if(!m_DoTime)
			bIsActive=false;
	}
	else
		bIsActive = true;

	if ( bIsActive )
	{
		F32 fPrevTime = m_Time;

		m_Time += ( theFETimer.FrameTime() * ((m_TimerDirc) ? 1.0f : -1.0f) );
		
		if(m_DoTime)
		{
			if(m_Time > m_DoTime)
			{
				if(m_DoWhat == E_Type_Stop)
				{
					m_Time=m_DoTime;
				}
				else if(m_DoWhat == E_Type_Wrap)
				{
					if ( m_fStartBW != -1.0f )
						m_Time = m_fStartBW;
					else
						m_Time = m_StartTime;
				}
				else if(m_DoWhat == E_Type_Bounce)
				{
					m_Time=m_DoTime;
					m_TimerDirc=false;
				}
			}
			else if ( ( m_Time < m_StartTime )  || ( ( m_fStartBW != -1 ) && ( m_Time < m_fStartBW ) && ( fPrevTime > m_fStartBW ) ) )
			{
				if ( m_fStartBW != -1.0f )
					m_Time = m_fStartBW;
				else
					m_Time = m_StartTime;

				if(m_DoWhat != E_Type_Stop)
				{
					m_TimerDirc=true;
				}
			}
		}
	}
	else
	{
		// The way timers work on variables is slightly different

		/*
		 * Bounce	_/\/\/\/\/\_ End 
		 *			  ________
		 * Stop		_/        \_ End
		 *
		 * Wrap		_/|/|/|/|/|_ End
		 * 
		 */ 

		// An non-active variable timer
		if(m_Time != m_StartTime)
		{
			switch(m_DoWhat)
			{
			case E_Type_Wrap: // Wrap and stop at start
				m_Time += theFETimer.FrameTime();
				if(m_Time >= m_DoTime)
				{
					if ( m_fStartBW != -1.0f )
						m_Time = m_fStartBW;
					else
						m_Time = m_StartTime;
				}
				break;

			case E_Type_Bounce:
			case E_Type_Stop: // Go backwards back to the start
				m_Time -= theFETimer.FrameTime();
				if(m_Time < m_StartTime)
				{
					if ( m_fStartBW != -1.0f )
						m_Time = m_fStartBW;
					else
						m_Time = m_StartTime;
				}
				break;
			}
		}
	}
}

}; // namespace UI
