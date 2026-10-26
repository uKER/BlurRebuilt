#ifndef EDITOR_LIB_IMMEDIATE_TRANSFORM_HINT_H
#define EDITOR_LIB_IMMEDIATE_TRANSFORM_HINT_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "Definitions.h"
#include "IImmediateHint.h"

namespace EditorLib
{
	class C_ImmediateTransformHint : public I_ImmediateHint
	{
	//From I_UpdateHint
	public:
		virtual E_ImmediateHint GetType() const
		{
			return E_ImmediateHint_Transform;
		}

	//From C_ImmediateTransformHint
	public:
		C_ImmediateTransformHint(bool xtStore) : m_xtStore(xtStore) {}

		bool XtStore() const { return m_xtStore; }

	private:
		bool m_xtStore;
	};
}

#endif
