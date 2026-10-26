
//#include "stdafx.h"

//#include <windows.h>

#include <core/core.h>

#ifdef PLATFORM_PC
#include <winsock2.h>
#endif

#include <core/containers/string.h>
#include <core/threads/thread.h>
#include <core/utility/threads.h>

#include <system/artery/artery.h>
#include <system/debug/variables.h>

#include <ui/game/uilua.h>


/*
	TODO:
	
	All message passing between artery thread and LUA thread have to be thread safe (double buffer instructions?)

	Handle calling other update routines while we're locked in our loop waiting for a single-step message or similar

*/

//#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off )
//#endif

#include "LuaDebugger.h"

extern "C"
{
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

namespace LuaDebugger
{
	struct LDPeer
	{
		ArteryConnection con;
	};

	LDPeer* g_ldPeer(NULL);

	enum DebugState
	{
		DS_Running,			// Just run until you hit a breakpoint
		DS_SingleStep,		// We step over lines one at a time
		DS_StepOver,
		DS_StepOut,
		DS_RunToLine,		// We run until we get to a certain line or hit a breakpoint
		DS_Stop,
	};

	int stepDepthTracker;	// Tracks stack depth during stepping

	struct Breakpoint
	{
		Util::C_String	filename;
		int				line;
	};

	struct State_s
	{
		DebugState	m_debugMode;			// Current operating mode we're in
		bool		m_sendState;			// Do we need to send state to the debugger again?

		struct
		{
			Util::C_String	filename;
			int				line;
		} m_runTo;

		Ut::Vector<Breakpoint> m_breakpoints;

	} m_state;

	//int listvars (lua_State *L, int level)
	//{
	//	lua_Debug ar;
	//	int i;
	//	
	//	if (lua_getstack(L, level, &ar) == 0)
	//		return 0;  // failure: no such level in the stack

	//	i = 1;
	//	while ((const char *name = lua_getlocal(L, &ar, i++)) != NULL)
	//	{
	//		printf("local %d %s\n", i-1, name);
	//		lua_pop(L, 1);  // remove variable value
	//	}

	//	lua_getinfo(L, "f", &ar);  // retrieves function
	//	
	//	i = 1;
	//	while ((const char *name = lua_getupvalue(L, -1, i++)) != NULL)
	//	{
	//		printf("upvalue %d %s\n", i-1, name);
	//		lua_pop(L, 1);  // remove upvalue value
	//	}

	//	return 1;
	//}

#define LEVELS1 12      /* size of the first part of the stack */
#define LEVELS2 10      /* size of the second part of the stack */

	static int errorfb (lua_State *L)
	{
		int level = 1;  /* skip level 0 (it's this function) */
		int firstpart = 1;  /* still before eventual `...' */
		lua_Debug ar;

		if (lua_gettop(L) == 0)
		{
			//lua_pushliteral(L, "");
		}
		else if (!lua_isstring(L, 1))
			return 1;  /* no string message */
		//else
			//lua_pushliteral(L, "\n");

		//lua_pushliteral(L, "stack traceback:");

		while (lua_getstack(L, level++, &ar))
		{
			if (level > LEVELS1 && firstpart)
			{
				/* no more than `LEVELS2' more levels? */
				if (!lua_getstack(L, level+LEVELS2, &ar))
					level--;  /* keep going */
				else
				{
					//lua_pushliteral(L, "\n\t...");  /* too many levels */

					while (lua_getstack(L, level+LEVELS2, &ar))  /* find last levels */
						level++;
				}
				firstpart = 0;
				continue;
			}

			//lua_pushliteral(L, "\n\t");
			lua_getinfo(L, "Snl", &ar);
			//lua_pushfstring(L, "%s:", ar.short_src);

			//if (ar.currentline > 0)
			//	lua_pushfstring(L, "%d:", ar.currentline);

			switch (*ar.namewhat)
			{
			case 'g':  /* global */ 
			case 'l':  /* local */
			case 'f':  /* field */
			case 'm':  /* method */
				{
				int mrf = 10;
				//lua_pushfstring(L, " in function `%s'", ar.name);
				break;
				}

			default:
				{
					int mrf = 10;

					//if (*ar.what == 'm')  /* main? */
					//	lua_pushfstring(L, " in main chunk");
					//else if (*ar.what == 'C' || *ar.what == 't')
					//	lua_pushliteral(L, " ?");  /* C function or tail call */
					//else
					//	lua_pushfstring(L, " in function <%s:%d>", ar.short_src, ar.linedefined);
				}
			}
			//lua_concat(L, lua_gettop(L));
		}
		
		//lua_concat(L, lua_gettop(L));
		
		return 1;
	}

bool bob = false;

bool val = false;

int AddVariableToMessage(ArteryMessageBuilder& b, lua_State *L, int index)
{
	int type = lua_type(L, index);

	int retVal = 0;

	b.Write(type);

	switch (type)
	{
	case LUA_TNUMBER:
		b.Write((float) lua_tonumber(L, index));
		break;

	case LUA_TBOOLEAN:
		b.Write(lua_toboolean(L, index) ? 1 : 0);
		break;

	case LUA_TSTRING:
		{
			const char* name = lua_tostring(L, index);

			if (!strcmp(name, "_G"))
				retVal = 1;

			b.Write(name);
			break;
		}

	case LUA_TFUNCTION:
		//if (lua_iscfunction(L, -1))
		//{
		//	lua_CFunction cf = lua_tocfunction(L, -1);

		//	char addr[20];
		//	sprintf_s(addr, lengthof(addr), "0x%x", (U32)cf);

		//	b.Write(addr);
		//}
		//else
		{
			b.Write("<LUA Function>");
		}
		break;

	case LUA_TTABLE:
		{
			// table is in the stack at index 't'
			lua_pushnil(L);  // first key

			while (lua_next(L, index-1) != 0)
			{ 
				int rval = AddVariableToMessage(b, L, -2);

				if (rval != 1)	// Is it _G?
				{
					AddVariableToMessage(b, L, -1);
				}
				else
				{
					b.Write((int)LUA_TSTRING);
					b.Write("_G");
				}

				//// 'key' is at index -2 and 'value' at index -1
				//char s2[100];
				//sprintf_s(s2, "%s - %s\n", lua_typename(L, lua_type(L, -2)), lua_typename(L, lua_type(L, -1)));
				//OutputDebugString(s2);

				//char skey[100], sval[100];
				//skey[0] = 0;
				//sval[0] = 0;

				//if (lua_type(L, -2) == LUA_TSTRING)
				//{
				//	sprintf_s(skey, "Key(%s)", lua_tostring(L, -2));
				//}
				//else if (lua_type(L, -2) == LUA_TNUMBER)
				//{
				//	sprintf_s(skey, "Key(%f)", lua_tonumber(L, -2));
				//}

				//if (lua_type(L, -1) == LUA_TSTRING)
				//{
				//	sprintf_s(sval, "Val(%s)", lua_tostring(L, -1));
				//}
				//else if (lua_type(L, -1) == LUA_TNUMBER)
				//{
				//	sprintf_s(sval, "Val(%f)", lua_tonumber(L, -1));
				//}

				//OutputDebugString(skey);
				//OutputDebugString(" - ");
				//OutputDebugString(sval);
				//OutputDebugString("\n");

				lua_pop(L, 1);  // removes 'value'; keeps 'key' for next iteration
			}

			b.Write((int)LUA_TNONE);

			break;
		}

	default:
		break;
	}

	return retVal;
}

void setlocals(lua_State *L)
{
	lua_Debug ar;

	if (!lua_getstack(L, 0, &ar))
		return;

	lua_pushboolean(L, val);
	lua_setlocal(L, &ar, 1);
}

void getstack(lua_State *L)
{
	lua_Debug ar;

	ArteryMessageBuilder b;

	b.Write("Stack");
	b.Write((int) 1);		// Version

	int level = 0;

	while (lua_getstack(L, level, &ar) == 1) // 1 = success
	{
		if (lua_getinfo(L, "Sln", &ar) != 0) // no errors
		{ 
			b.Write(ar.name ? ar.name : "<null>");
			b.Write(ar.short_src);
			b.Write(ar.currentline);

			// if ar->what == 'tail', output tail call message
		}
		level++;
	}
	lua_getstack(L, 0, &ar); 
	// return to the state before this function call.

	ArteryMessage m;

	if (b.Build(m))
		g_ldPeer->con.Send(m);
}

void getlocals(lua_State *L)
{
	lua_Debug ar;
	const char *name;

	int stackDepth = 0;

	ArteryMessageBuilder b;

	b.Write("Locals");
	b.Write((int) 1);		// Version

	for (int a=1; ; a++)
	{
		if (!lua_getstack(L, stackDepth, &ar))  /* level out of range? */
			return;
		
		name = lua_getlocal(L, &ar, a);

		if (name)
		{
			if (strcmp("(*temporary)", name))
			{
				int type = lua_type(L, -1);

				b.Write(name);

				AddVariableToMessage(b, L, -1);
			}

			lua_pop(L, 1);
		}
		else
			break;
	}

	ArteryMessage m;

	if (b.Build(m))
		g_ldPeer->con.Send(m);
}



void SendCurrentState(lua_State *state, const lua_Debug* const ar, LDPeer* g_ldPeer)
{
	ArteryMessageBuilder b;

	b.Write("BreakPos");

	b.Write(ar->short_src);
	b.Write(ar->currentline);

	ArteryMessage m;

	if (b.Build(m))
		g_ldPeer->con.Send(m);

	getlocals(state);
	getstack(state);
}

void HookRoutine(lua_State *state, lua_Debug *ar)
{
	lua_getinfo(state, "Slunf", ar);
	
	if (ar->event == LUA_HOOKLINE)
	{
		if ((m_state.m_debugMode == DS_StepOver) && (stepDepthTracker <= 0))
			m_state.m_debugMode = DS_Stop;

		bool stopExecution = (m_state.m_debugMode == DS_Stop);

		// Check breakpoints now
		Ut::Vector<Breakpoint>::ConstIterator bpit = m_state.m_breakpoints.Begin();

		for (; bpit != m_state.m_breakpoints.End(); bpit++)
		{
			// Line first then file
			if ((ar->currentline == bpit->line) && !stricmp(ar->short_src, bpit->filename.Chars()))
			{
				m_state.m_debugMode = DS_Stop;
				stopExecution = true;
			}
		}

		if (m_state.m_debugMode == DS_RunToLine)
		{
			if ((ar->currentline == m_state.m_runTo.line) && !stricmp(ar->short_src, m_state.m_runTo.filename.Chars()))
			{
				m_state.m_debugMode = DS_Stop;
				stopExecution = true;
			}
		}

		if (stopExecution)
		{
			// Send message to debugger to notify them where we are
			// Current file, line
			if (g_ldPeer && g_ldPeer->con.IsConnected())
			{
				SendCurrentState(state, ar, g_ldPeer);
			}

			// Come in here and wait for an event from the debugger to say continue
			while (1)
			{
				if (bob)
				{
					bob = false;

					//errorfb(state);
					setlocals(state);
				}

				if (m_state.m_sendState)
				{
					SendCurrentState(state, ar, g_ldPeer);
					m_state.m_sendState = false;
				}

				if (m_state.m_debugMode != DS_Stop)
				{
					if (m_state.m_debugMode == DS_SingleStep)
					{
						m_state.m_debugMode = DS_Stop;
					}
					else if ((m_state.m_debugMode == DS_StepOver) || (m_state.m_debugMode == DS_StepOut))
					{
						stepDepthTracker = 0;
					}

					break;
				}

				Debug::NetworkUpdate();
				UI::g_pUILua->DoConsole();

				MT::Thread::Sleep(16);
			}
		}
	}
	else if (ar->event == LUA_HOOKCALL)
	{
		stepDepthTracker++;
	}
	else if ((ar->event == LUA_HOOKRET) || (ar->event == LUA_HOOKTAILRET))
	{
		stepDepthTracker--;

		if ((m_state.m_debugMode == DS_StepOut) && (stepDepthTracker == -1))
			m_state.m_debugMode = DS_Stop;
		else if ((m_state.m_debugMode == DS_StepOver) && (stepDepthTracker <= 0))
			m_state.m_debugMode = DS_Stop;
	}
}

bool connected = false;

void Update(lua_State *state)
{
	bool enableHook = false;

	if (connected)
	{
		if ((m_state.m_breakpoints.Size() > 0) || (m_state.m_debugMode != DS_Running))
			enableHook = true;
	}

	lua_sethook(state, HookRoutine, enableHook ? (LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET) : 0, 0);
}

void ProcessNetworkCommunications(ArteryConnection con, void*)
{
	LDPeer dp;
	dp.con = con;

	g_ldPeer = &dp;

	m_state.m_sendState = (m_state.m_debugMode == DS_Stop);
	
	while (con.IsConnected())
	{
		connected = true;

		ArteryMessage msg;

		if (con.Receive(msg))
		{
			ArteryMessageReader msgReader(msg);

			const char* msgString(NULL);

			if (!msgReader.Read(msgString))
				continue;
			
			if (!strcmp(msgString, "BREAK"))
			{
				m_state.m_debugMode = DS_Stop;
			}
			else if (!strcmp(msgString, "BREAKPOINT"))
			{
				const char* filename(NULL);
				int setting, line;
				
				msgReader.Read(setting);
				msgReader.Read(filename);
				msgReader.Read(line);

				if (!setting)
				{
					// Try and remove
					Ut::Vector<Breakpoint>::Iterator bpit = m_state.m_breakpoints.Begin();

					for (; bpit != m_state.m_breakpoints.End(); bpit++)
					{
						// Line first then file
						if ((line == bpit->line) && !stricmp(filename, bpit->filename.Chars()))
						{
							m_state.m_breakpoints.Erase(bpit);
							break;
						}
					}
				}
				else
				{
					Breakpoint bp;
					bp.filename	= filename;
					bp.line		= line;
					m_state.m_breakpoints.PushBack(bp);
				}
			}
			else if (!strcmp(msgString, "RUNTO"))
			{
				const char* filename(NULL);
				int line;
				
				msgReader.Read(filename);
				msgReader.Read(line);

				m_state.m_runTo.filename	= filename;
				m_state.m_runTo.line		= line;

				m_state.m_debugMode			= DS_RunToLine;
			}
			else if (!strcmp(msgString, "STEPINTO"))
			{
				m_state.m_debugMode = DS_SingleStep;
			}
			else if (!strcmp(msgString, "STEPOVER"))
			{
				m_state.m_debugMode = DS_StepOver;
			}
			else if (!strcmp(msgString, "STEPOUT"))
			{
				m_state.m_debugMode = DS_StepOut;
			}
			else if (!strcmp(msgString, "GO"))
			{
				m_state.m_debugMode = DS_Running;
			}
		}
	}

	connected = false;

	g_ldPeer = NULL;
}

int InitialiseNetworkConnection()
{
	m_state.m_debugMode		= DS_Running;
	m_state.m_sendState		= false;

#ifndef CFG_GOLD
	ArteryRegister("LUA Debugger", ProcessNetworkCommunications, 0);
#endif
	return 0;
}

} // LuaDebugger
