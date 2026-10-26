#ifndef EDITOR_LIB_XT_ACTOR_COMPONENT_TYPE_H
#define EDITOR_LIB_XT_ACTOR_COMPONENT_TYPE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_XtActorComponentType : public I_XtType
	{
	public:
		C_XtActorComponentType(LtStruct* pSchema) : I_XtType(pSchema) {}
		virtual ~C_XtActorComponentType() {}

		virtual E_XtType C_XtActorComponentType::GetType() const {
			return E_XtType_DgXtCompNode;
		}

		virtual bool IsA(E_XtType type) const {
			if (__super::IsA(type))
				return true;
			if (type == C_XtActorComponentType::GetType()) 
				return true;

			return false;
		}

	protected:
		C_XtActorComponentType(const C_XtActorComponentType& rhs);
		const C_XtActorComponentType& operator=(const C_XtActorComponentType& rhs);
	};
}

#endif
