
#include "stdafx.h"

/*
	TODO:
	
	All message passing between artery thread and LUA thread have to be thread safe (double buffer instructions?)

	Handle calling other update routines while we're locked in our loop waiting for a single-step message or similar

*/

//#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
//#endif

#include <System/Artery/Artery.h>

#include "LuaDebugger.h"

#include "ui\game\uilua.h"

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
		DS_RunToLine,		// We run until we get to a certain line or hit a breakpoint
		DS_Stop,
	};

	struct Breakpoint
	{
		Util::C_String	filename;
		int				line;
	};

	struct
	{
		DebugState	m_debugMode;			// Current operating mode we're in

		struct
		{
			Util::C_String	filename;
			int				line;
		} m_runTo;

		Util::C_String		m_scriptsRoot;

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
bool getWatch = false;

bool val = false;

void setlocals(lua_State *L)
{
	lua_Debug ar;

	if (!lua_getstack(L, 0, &ar))
		return;

	lua_pushboolean(L, val);
	lua_setlocal(L, &ar, 1);
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
				b.Write(type);

				switch (type)
				{
				case LUA_TNUMBER:
					b.Write((float) lua_tonumber(L, -1));
					break;

				case LUA_TBOOLEAN:
					b.Write(lua_toboolean(L, -1) ? 1 : 0);
					break;

				case LUA_TSTRING:
					b.Write(lua_tostring(L, -1));
					break;

				case LUA_TFUNCTION:
					if (lua_iscfunction(L, -1))
					{
						lua_CFunction cf = lua_tocfunction(L, -1);

						char addr[20];
						sprintf_s(addr, "0x%x", cf);

						b.Write(addr);
					}
					else
					{
						b.Write("<LUA Function>");
					}
					break;

				case LUA_TTABLE:
					{
						int tableSize = luaL_getn(L, -1);

						char s[100];
						sprintf_s(s, "Table Size %d\n", tableSize);
						OutputDebugString(s);

						for (int iCount=1; iCount <= tableSize; iCount++)
						{
						//	lua_rawgeti(L, -1, iCount);

						//	int type2 = lua_type(L, -1);

						//	lua_pop(L, 1);
						}

						break;
					}

				default:
					break;
				}
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


void HookRoutine(lua_State *state, lua_Debug *ar)
{
	lua_getinfo(state, "Slunf", ar);
	
	if (ar->event == LUA_HOOKLINE)
	{
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
				ArteryMessageBuilder b;

				b.Write("BreakPos");
				
				b.Write(ar->short_src);
				b.Write(ar->currentline);

				ArteryMessage m;

				if (b.Build(m))
					g_ldPeer->con.Send(m);

				getlocals(state);
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

				if (getWatch)
				{
					getWatch = false;

					getlocals(state);
				}

				if (m_state.m_debugMode != DS_Stop)
				{
					if (m_state.m_debugMode == DS_SingleStep)
						m_state.m_debugMode = DS_Stop;

					break;
				}

				Debug::NetworkUpdate();
				UI::g_pUILua->DoConsole();

				SleepEx(16, true);
			}
		}
	}
}

bool connected = false;

void Update(lua_State *state)
{
	if (connected)
	{
		lua_sethook(state, HookRoutine, LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET, 0);
	}
}

void ProcessNetworkCommunications(ArteryConnection con, void*)
{
	LDPeer dp;
	dp.con = con;

	g_ldPeer = &dp;
	
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
			else if (!strcmp(msgString, "STEPIN"))
			{
				m_state.m_debugMode = DS_SingleStep;
			}
			else if (!strcmp(msgString, "GO"))
			{
				m_state.m_debugMode = DS_Running;
			}
			else if (!strcmp(msgString, "GETWATCH"))
			{
				getWatch = true;
			}
		}
	}

	connected = false;

	g_ldPeer = NULL;
}

int InitialiseNetworkConnection(const Util::C_String& scriptRoot)
{
	m_state.m_scriptsRoot	= scriptRoot;
	m_state.m_debugMode		= DS_Running;

#ifndef CFG_GOLD
	ArteryRegister("LUA Debugger", ProcessNetworkCommunications, 0);
#endif
	return 0;
}

} // LuaDebugger
