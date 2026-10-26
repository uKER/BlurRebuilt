#ifndef UIVARIABLE_STORE_H
#define UIVARIABLE_STORE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace UI
{

// Interface class
class I_VariableStore
{
public:
	virtual ~I_VariableStore() {}
	virtual	S32		Get(const CHAR8 *name,const S32 index=-1) = 0;
	virtual	void	Set(const CHAR8 *name,const S32 value,S32 index=-1) = 0;
	virtual	void	Default(void) = 0;
};

I_VariableStore* GetVariableStore();

};		// namespace UI
#endif	// UIVARIABLE_STORE_H
