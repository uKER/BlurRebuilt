#ifndef EDITOR_LIB_IMMEDIATE_REPARENT_HINT_H
#define EDITOR_LIB_IMMEDIATE_REPARENT_HINT_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "Definitions.h"
#include "IImmediateHint.h"

namespace EditorLib
{
	class C_ImmediateReparentHint : public I_ImmediateHint
	{
	//From I_UpdateHint
	public:
		virtual E_ImmediateHint GetType() const
		{
			return E_ImmediateHint_Reparent;
		}

	//From C_ImmediateReparentHint
	public:
		C_ImmediateReparentHint(bool xtStore) : m_xtStore(xtStore) {}

		bool XtStore() const { return m_xtStore; }

	private:
		bool m_xtStore;
	};
}

#endif
