#ifndef DMAN_XT_DIALOGUE_TYPE_H
#define DMAN_XT_DIALOGUE_TYPE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_XtDialogueType : public I_XtType
	{
	public:
		C_XtDialogueType(LarvaType* pSchema) : I_XtType(pSchema) {}

		virtual ~C_XtDialogueType() {}

		virtual E_XtType GetType() const {
			return E_XtType_SxlRecord;
		}

		virtual bool IsA(E_XtType type) const {
			if (__super::IsA(type))
				return true;
			if (type == C_XtDialogueType::GetType()) 
				return true;

			return false;
		}

	protected:
		C_XtDialogueType(const C_XtDialogueType& rhs);
		const C_XtDialogueType& operator=(const C_XtDialogueType& rhs);
	};
}

#endif //DMAN_XT_DIALOGUE_ITEM_TYPE_H
