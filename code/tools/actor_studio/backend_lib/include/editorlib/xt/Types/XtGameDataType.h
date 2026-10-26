#ifndef EDITOR_LIB_XT_GAME_DATA_TYPE_H
#define EDITOR_LIB_XT_GAME_DATA_TYPE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_XtGameDataType : public C_XtNodeType
	{
	public:
		C_XtGameDataType(LtStruct* pSchema) : C_XtNodeType(pSchema) {}

		virtual ~C_XtGameDataType() {}

		virtual E_XtType GetType() const {
			return E_XtType_DgXtGameDataNode;
		}

		virtual bool IsA(E_XtType type) const {
			if (__super::IsA(type))
				return true;
			if (type == C_XtGameDataType::GetType()) 
				return true;

			return false;
		}

	protected:
		C_XtGameDataType(const C_XtGameDataType& rhs);
		const C_XtGameDataType& operator=(const C_XtGameDataType& rhs);
	};
}

#endif
