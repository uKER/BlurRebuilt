#ifndef EDITOR_LIB_I_XT_TYPE
#define EDITOR_LIB_I_XT_TYPE

#ifdef _MSC_VER_
#pragma once
#endif

#include <editorlib/xt/XtDefinitions.h>

namespace EditorLib
{
	class I_XtType
	{
	public:
		I_XtType(LarvaType* pLarvaType)	{
			m_pLarvaType = pLarvaType;
		}

		virtual ~I_XtType() {}

		virtual E_XtType GetType() const {
			return E_XtType_IXtType;
		}

		virtual bool IsA(E_XtType type) const {
			if (type == I_XtType::GetType()) 
				return true;

			return false;
		}

		//This should be const, sorcery API needs work for this to be possible.
		LarvaType* GetXtType() const	{
			//TODO: FIX THIS HACK
			return const_cast<I_XtType*>(this)->m_pLarvaType;
		}

	protected:
		I_XtType(const I_XtType& rhs);
		const I_XtType& operator=(const I_XtType& rhs);

		LarvaType* m_pLarvaType;
	};
}

#endif
