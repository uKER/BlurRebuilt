#ifndef EDITOR_LIB_I_UPDATE_HINT_H
#define EDITOR_LIB_I_UPDATE_HINT_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "Definitions.h"

namespace EditorLib
{
	class I_UpdateHint
	{
	public:
		virtual E_UpdateHint GetType() const = 0;
	};
}

#endif
