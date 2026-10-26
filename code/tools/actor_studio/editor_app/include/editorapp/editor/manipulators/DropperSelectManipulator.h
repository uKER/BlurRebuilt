#ifndef EDITOR_DROPPERSELECT_MANIPULATOR_H
#define EDITOR_DROPPERSELECT_MANIPULATOR_H


#ifdef _MSC_VER
#pragma once
#endif

#include <EditorApp/Editor/Manipulators/Manipulator.h>

namespace Editor
{
	struct DropperUserData
	{
		LarvaField *pLvField;
		Xt::DomElement *pDomElement;
	};

	class C_DropperSelectManipulator : public I_Manipulator
	{
	public:
		C_DropperSelectManipulator();
		~C_DropperSelectManipulator();

		virtual E_ManipulatorType GetType() const;
		virtual bool IsA(E_ManipulatorType type) const;

		void SetData(const void* userData);

		void OnMouseEvent(wxMouseEvent &e);

	private:

		Xt::DomElement	*m_pDomElement;
		LarvaField		*m_pLvField;

	private:
		DECLARE_EVENT_TABLE()
	};

}

#endif