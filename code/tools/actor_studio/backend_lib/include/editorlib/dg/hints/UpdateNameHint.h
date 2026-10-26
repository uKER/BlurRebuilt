#ifndef EDITOR_LIB_UPDATE_NAME_HINT_H
#define EDITOR_LIB_UPDATE_NAME_HINT_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "Definitions.h"
#include "IUpdateHint.h"

namespace EditorLib
{
	class C_UpdateNameHint : public I_UpdateHint
	{
	//From I_UpdateHint
	public:
		virtual E_UpdateHint GetType() const
		{
			return E_UpdateHint_Name;
		}
	};
}

#endif
