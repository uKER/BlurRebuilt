#ifndef VARIABLE_STORE_H
#define VARIABLE_STORE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/UIVariableStore.h>

namespace UI
{

class GameVariableStore : public I_VariableStore
{
public:
	S32 Get(const CHAR8 *name,const S32 index=-1);
	void Set(const CHAR8 *name,const S32 value,S32 index=-1);
	void Default(void);

public:
	S32		m_sTest;
};

};		// namespace UI
#endif // VARIABLE_STORE_H
