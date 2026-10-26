#include "stdafx.h"
#include "Options.h"

const char* g_commandStr[E_Command_Count] = 
{
#define DEF(e, str) {str},
#include "Command.inl"
#undef DEF
};

const char* GetCommandStr(E_Command cmd)
{
	return g_commandStr[cmd];
}


E_Command GetCommand(const char* cmd)
{
	for (S32 i = 0; i < E_Command_Count; ++i)
		if (!stricmp(cmd, g_commandStr[i]))
			return E_Command(i);
	return E_Command_Unknown;
}

