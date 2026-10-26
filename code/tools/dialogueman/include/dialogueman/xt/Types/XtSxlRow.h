#ifndef DMAN_XT_DIALOGUE_ITEM_TYPE_H
#define DMAN_XT_DIALOGUE_ITEM_TYPE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
	class C_XtDialogueItemType : public I_XtType
	{
	public:
		C_XtDialogueItemType(LarvaType* pSchema) : I_XtType(pSchema) {}

		virtual ~C_XtDialogueItemType() {}

		virtual E_XtType GetType() const {
			return E_XtType_SxlRow;
		}

		virtual bool IsA(E_XtType type) const {
			if (__super::IsA(type))
				return true;
			if (type == C_XtDialogueItemType::GetType()) 
				return true;

			return false;
		}

	protected:
		C_XtDialogueItemType(const C_XtDialogueItemType& rhs);
		const C_XtDialogueItemType& operator=(const C_XtDialogueItemType& rhs);
	};
}

#endif //DMAN_XT_DIALOGUE_ITEM_TYPE_H
