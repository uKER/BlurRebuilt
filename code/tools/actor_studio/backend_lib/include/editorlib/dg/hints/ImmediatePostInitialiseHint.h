#ifndef EDITOR_LIB_IMMEDIATE_POST_INITIALISE_HINT_H
#define EDITOR_LIB_IMMEDIATE_POST_INITIALISE_HINT_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "Definitions.h"
#include "IImmediateHint.h"

namespace EditorLib
{
	class C_ImmediatePostInitialiseHint : public I_ImmediateHint
	{
	//From I_UpdateHint
	public:
		virtual E_ImmediateHint GetType() const
		{
			return E_ImmediateHint_PostInitialise;
		}
	};
}

#endif
