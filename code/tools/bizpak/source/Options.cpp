#include "stdafx.h"
#include "Options.h"

const char* g_commandStr[E_Command_Count] = 
{
#define DEF(e, str) {str},
#include "Command.inl"
#undef DEF
};

const char* g_compressTypeStr[E_CompressType_Count] = 
{
#define DEF(e, str) {str},
#include "CompressTypes.inl"
#undef DEF
};

const char* GetCommandStr(E_Command cmd)
{
	return g_commandStr[cmd];
}

const char* GetCompressTypeStr(E_CompressType type)
{
	return g_compressTypeStr[type];
}

E_Command GetCommand(const char* cmd)
{
	for (S32 i = 0; i < E_Command_Count; ++i)
		if (!stricmp(cmd, g_commandStr[i]))
			return E_Command(i);
	return E_Command_Unknown;
}

E_CompressType GetCompressType(const char* type)
{
	for (S32 i = 0; i < E_CompressType_Count; ++i)
		if (!stricmp(type, g_compressTypeStr[i]))
			return E_CompressType(i);
	return E_CompressType_Unknown;
}
