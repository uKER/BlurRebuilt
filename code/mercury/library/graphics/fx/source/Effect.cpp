#include "stdafx.h"

#include <display.h>

#include "fx/Effect.h"
#include "fx/Shared.h"
#include "fx/System.h"
#include "fx/Fx.h"
#include "fx/FxContext.h"
#include "fx/Manager.h"

namespace FX
{

/// Get a pointer to an effect parameter by name
Parameter *I_Effect::GetParameter(const char *pszName) const
{
	Effect *pEffect = (Effect*)this;
	for (int i = 0; i < pEffect->Parameters.Size(); i++)
		if (strcmp(pEffect->Parameters[i].Name,pszName) == 0)
		{
			if (!pEffect->Parameters[i].bShared)
				return &pEffect->Parameters[i];
			else
				return GetParameterPtr(FX::GetSharedParameter(pszName));
		}
	return g_System.GetNullParameter();
}

/// Get the index of the technique with the specified name
/// returns -1 if the name was not found
int I_Effect::GetTechnique(const char *psz)
{
	Effect *pEffect = (Effect*)this;
	for (int i = 0; i < pEffect->Techniques.Size(); i++)
		if (strcmp(pEffect->Techniques[i].Name, psz))
			return i;
	return -1;
}

// utility function - returns true if the given parameter index is in one of the bindings
bool GetBindingTechniqueUsage(const Ut::Vector<FunctionBinding> &bindings, int i)
{
	for (int i=0; i<bindings.Size(); i++)
	{
		if (bindings[i].ParameterIdx == i)
			return true;
	}

	return false; // not found
}

/// returns whether a parameter is used on a particular technique
/// returns a bitfield, each bit represents a technique on the effect
/// this is dangerous to use on shared parameters, because even though a shared parameter might not be used on the current effect, it may be needed for a later effect
int I_Effect::GetTechniqueUsage(const ImmediateParameter &param)
{
	assert(0); // TODO: re-implement this

	return -1;
}

}
