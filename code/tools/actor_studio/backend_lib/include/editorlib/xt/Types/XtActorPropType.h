#ifndef EDITOR_LIB_XT_PROP_TYPE_H
#define EDITOR_LIB_XT_PROP_TYPE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_XtActorPropType : public I_XtType
	{
	public:
		C_XtActorPropType(LarvaType* pSchema) : I_XtType(pSchema) {}

		virtual ~C_XtActorPropType() {}

		virtual E_XtType GetType() const {
			return E_XtType_DgXtPropNode;
		}

		virtual bool IsA(E_XtType type) const {
			if (__super::IsA(type))
				return true;
			if (type == C_XtActorPropType::GetType()) 
				return true;

			return false;
		}

	protected:
		C_XtActorPropType(const C_XtActorPropType& rhs);
		const C_XtActorPropType& operator=(const C_XtActorPropType& rhs);
	};
}

#endif
