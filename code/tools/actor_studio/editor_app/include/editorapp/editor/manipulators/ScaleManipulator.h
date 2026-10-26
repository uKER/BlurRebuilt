#ifndef EDITOR_SCALE_MANIPULATOR_H
#define EDITOR_SCALE_MANIPULATOR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{

	class C_ScaleManipulator : public C_SelectManipulator
	{
	public:

		static const U8 XAxis			= 0x01;
		static const U8 YAxis			= 0x02;
		static const U8 ZAxis			= 0x04;
		static const U8 Centre			= 0x08;
		static const U8 Active			= 0x10;
		static const U8 Visible			= 0x20;
		static const U8 Snap			= 0x40;

		C_ScaleManipulator();
		virtual ~C_ScaleManipulator();

		virtual E_ManipulatorType GetType() const;

		virtual bool IsA(E_ManipulatorType type) const;

		void OnMouseEvent(wxMouseEvent &e);
		void OnMouseCaptureLostEvent(wxMouseCaptureLostEvent &e);
		void OnKeyDown(wxKeyEvent& e);
		void OnKeyUp(wxKeyEvent& e);
		void OnSelectionChanged();

		void EnableSnapScale(bool snap=true, F32 value=1.f);
		bool SnapEnabled() const;
		F32 SnapValue() const;

		void Render(const C_RenderViewport *pViewport) const;

	private:
		void DrawManipulator(const Maths::HVec3& axisX, const Maths::HVec3& axisY, const Maths::HVec3& axisZ) const;

		void UpdateContext(const Maths::HVec3& scale);
		void ApplyContext(const Maths::HVec3& scale);

		Maths::HMat44	m_Transform;
		Maths::HMat44	m_StartTransform;

		wxPoint			m_ClickPosition;
		F32				m_Scale;
		F32				m_ScaleSnap;
		U8				m_Flags;
		U8				m_HoverFlags;
		bool			m_SnapToggle;

	private:
		DECLARE_EVENT_TABLE()
	};

}

#endif