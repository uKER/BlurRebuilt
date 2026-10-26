#include "Stdafx.h"

#include "GameVariableStore.h"
#include "UI/UILogger.h"

namespace UI
{

static GameVariableStore gGameVariableStore;

typedef struct UIVar
{
	S32 *	value;
	CHAR8	name[64];
} UIVar;

static UIVar variable_list[] =
{
	{	&gGameVariableStore.m_sTest,		"TEST_VAR"			},
};

// Declare the interface
I_VariableStore* GetVariableStore()
{
	return &gGameVariableStore;
}

// The class bit
void GameVariableStore::Set(const CHAR8 *name,S32 value,const S32 index)
{
	if(strlen(name)==0) return;

	const S32 size = sizeof(variable_list)/sizeof(UIVar);

	for(S32 a=0;a<size;a++)
	{
		if(!strcmp(name,variable_list[a].name))
		{
			if(index==-1)
				*variable_list[a].value=value;
			else
				variable_list[a].value[index]=value;
			return;
		}
	}   

	LOGWARN( g_Log,  "- Trying to set a variable that doesn't exist : %s\n", name);
}


S32 GameVariableStore::Get(const CHAR8 *name, const S32 index )
{
	if(strlen(name)==0) return 0;

	const S32 size = sizeof(variable_list)/sizeof(UIVar);

	for(S32 a=0;a<size;a++)
	{
		if(!strcmp(name,variable_list[a].name))
		{
			if(index==-1)
				return(*variable_list[a].value);
			else
				return(variable_list[a].value[index]);
		}
	}

#ifndef PLATFORM_PS3
	LOGWARN( g_Log,  "GameVariableStore::Get - Trying to get a variable that doesn't exist\n");
#else
	printf("GameVariableStore::Get - Trying to get a variable that doesn't exist\n");
#endif
	return 0;
}

void GameVariableStore::Default(void)
{
}

}; // namespace UI
