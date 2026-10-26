#ifndef ACTORCONTEXT_7BDB95B3_D60C_44DB_A9B2_5244C471CCB3_H
#define ACTORCONTEXT_7BDB95B3_D60C_44DB_A9B2_5244C471CCB3_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Context/I_Anything.h>


namespace Sys
{

/**
@ingroup actor_group_utility

When creating an actor, an implementation of this interface is needed to 
supply any context required by specific services.
*/
class I_ContextProvider
{
public:
	virtual ~I_ContextProvider() {}

	/**
	@brief Requests a specific context.
	@param typeId identifies the context required.
	@returns NULL if the context is not present.
	*/
	virtual const I_Anything* GetContext(const C_AnythingUid typeId) const = 0;

	/**
	@brief Requests a specific context.
	@param T_Context type to obtain.
	@returns NULL if the context is not present.
	*/
	template <class T_Context>
	const T_Context* GetContext() const
	{
		const I_Anything* pAnything = GetContext(T_Context::kClassUid);
		return (pAnything != NULL) ? static_cast<const T_Context*>(pAnything) : NULL;
	}

};

} // namespace Sys


#endif // ACTORCONTEXT_7BDB95B3_D60C_44DB_A9B2_5244C471CCB3_H
