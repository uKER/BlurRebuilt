#ifndef EDITOR_LIB_XT_ACTOR_TYPE_H
#define EDITOR_LIB_XT_ACTOR_TYPE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_XtNodeType : public I_XtType
	{
	public:
		C_XtNodeType(LtStruct* pSchema) : I_XtType(pSchema) {}
		virtual ~C_XtNodeType() {}

		virtual E_XtType GetType() const {
			return E_XtType_DgXtNode;
		}

		virtual bool IsA(E_XtType type) const {
			if (__super::IsA(type))
				return true;
			if (type == C_XtNodeType::GetType()) 
				return true;

			return false;
		}

	protected:
		C_XtNodeType(const C_XtNodeType& rhs);
		const C_XtNodeType& operator=(const C_XtNodeType& rhs);
	};
}

#endif
