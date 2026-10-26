#ifndef EDITOR_SELECT_MANIPULATOR_H
#define EDITOR_SELECT_MANIPULATOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorApp/Editor/Manipulators/Manipulator.h>

namespace Editor
{
	enum E_SelectionMode
	{
		E_SelectionMode_Object,
		E_SelectionMode_Component
	};

	struct SelectionMask
	{
		union
		{
			struct
			{
				//object level
				bool mesh		: 1;
				bool locator	: 1;
				bool property	: 1;

				//component level
			};
			U32 mask;
		};

		SelectionMask()
		: mask(0xffffffff)
		{ }
	};

	class C_SelectManipulator : public I_Manipulator
	{
	public:
		C_SelectManipulator();
		virtual ~C_SelectManipulator();

		virtual E_ManipulatorType GetType() const;

		virtual bool IsA(E_ManipulatorType type) const;

		bool IsActive() const;

		static E_SelectionMode GetSelectionMode();
		static void SetSelectionMode(E_SelectionMode mode);
		static SelectionMask GetSelectionMask();
		static void SetSelectionMask(SelectionMask mask);

		void AttachToViewport(C_RenderViewport *pViewport);

		void OnMouseEvent(wxMouseEvent &e);
		void OnMouseCaptureLostEvent(wxMouseCaptureLostEvent &e);

		bool IsSelectable(const EditorLib::I_DgNode* pNode);

		void Render(const C_RenderViewport *pViewport) const;

	private:
		void DrawSelectionRect() const;

		static const U8	Active			= 0x01;
		static const U8 Dirty			= 0x02;

		wxPoint		m_StartPosition;
		wxPoint		m_EndPosition;
		mutable U8	m_Flags;

		static E_SelectionMode	m_SelectionMode;
		static SelectionMask	m_SelectionMask;

		DECLARE_EVENT_TABLE()
	};

}

#endif