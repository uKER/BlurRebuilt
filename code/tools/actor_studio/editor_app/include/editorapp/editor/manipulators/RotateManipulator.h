#ifndef EDITOR_ROTATE_MANIPULATOR_H
#define EDITOR_ROTATE_MANIPULATOR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{

	class C_RotateManipulator : public C_SelectManipulator
	{
	public:
		C_RotateManipulator();
		virtual ~C_RotateManipulator();

		virtual E_ManipulatorType GetType() const;

		virtual bool IsA(E_ManipulatorType type) const;

		void OnMouseEvent(wxMouseEvent &e);
		void OnMouseCaptureLostEvent(wxMouseCaptureLostEvent &e);
		void OnKeyDown(wxKeyEvent& e);
		void OnKeyUp(wxKeyEvent& e);
		void OnSelectionChanged();

		void EnableSnapRotate(bool snap=true, F32 value=45.f);
		bool SnapEnabled() const;
		F32 SnapValue() const;

		void Render(const C_RenderViewport *pViewport) const;

	private:

		static const U8 XAxis			= 0x01;
		static const U8 YAxis			= 0x02;
		static const U8 ZAxis			= 0x04;
		static const U8 Inner			= 0x08;
		static const U8 Outer			= 0x10;
		static const U8 Active			= 0x20;
		static const U8 Visible			= 0x40;
		static const U8 Snap			= 0x80;

		static const U32 Tessellation	= 72;

		void DrawManipulator(const Maths::HVec3& axisX, const Maths::HVec3& axisY, const Maths::HVec3& axisZ,
			const Maths::HVec3& toX, const Maths::HVec3& toY, const Maths::HVec3& toZ,
			const Maths::HVec3& viewX, const Maths::HVec3& viewY) const;

		void UpdateContext(const Maths::HVec3& axis, F32 angle);
		void ApplyContext(const Maths::HVec3& axis, F32 angle);

		Maths::HMat44		m_Transform;
		Maths::HMat44		m_StartTransform;
		Maths::HVec3		m_IntersectionVector;
		Maths::HVec3		m_RotationAxis;

		wxPoint				m_ClickPosition;
		F32					m_StartAngle;
		F32					m_RotateSnap;
		F32					m_Angle;
		U8					m_Flags;
		U8					m_HoverFlags;
		bool				m_SnapToggle;
		DECLARE_EVENT_TABLE()
	};

}

#endif