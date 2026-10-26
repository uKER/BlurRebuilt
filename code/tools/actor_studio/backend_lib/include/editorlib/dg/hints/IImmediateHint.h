#ifndef EDITOR_LIB_I_IMMEDIATE_HINT_H
#define EDITOR_LIB_I_IMMEDIATE_HINT_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "Definitions.h"

namespace EditorLib
{
	class I_ImmediateHint
	{
	public:
		virtual E_ImmediateHint GetType() const = 0;
	};
}

#endif
