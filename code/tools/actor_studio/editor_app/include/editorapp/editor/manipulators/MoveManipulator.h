#ifndef EDITOR_MOVE_MANIPULATOR_H
#define EDITOR_MOVE_MANIPULATOR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_MoveManipulator : public C_SelectManipulator
	{
	public:
		C_MoveManipulator();
		virtual ~C_MoveManipulator();

		virtual E_ManipulatorType GetType() const;

		virtual bool IsA(E_ManipulatorType type) const;

		void OnMouseEvent(wxMouseEvent &e);
		void OnMouseCaptureLostEvent(wxMouseCaptureLostEvent &e);
		void OnKeyDown(wxKeyEvent& e);
		void OnKeyUp(wxKeyEvent& e);
		void OnSelectionChanged();

		void EnableSnapMove(bool snap=true, F32 value=1.f);
		bool SnapEnabled() const;
		F32 SnapValue() const;

		void Render(const C_RenderViewport *pViewport) const;

	private:
		enum E_Constraint
		{
			E_Constraint_None,
			E_Constraint_XY,
			E_Constraint_XZ,
			E_Constraint_YZ,
		};

		static const U8 XAxis			= 0x01;
		static const U8 YAxis			= 0x02;
		static const U8 ZAxis			= 0x04;
		static const U8 Centre			= 0x08;
		static const U8 Active			= 0x10;
		static const U8 Visible			= 0x20;
		static const U8 Snap			= 0x40;

		static const U32 Tessellation	= 8;

		void DrawManipulator(
			const Maths::HVec3& axisX, 
			const Maths::HVec3& axisY, 
			const Maths::HVec3& axisZ,
			
			const Maths::HVec3& toX, 
			const Maths::HVec3& toY, 
			const Maths::HVec3& toZ) const;

		void UpdateContext(const Maths::HVec3& translate);
		void ApplyContext(const Maths::HVec3& translate);

		U8	PickAxis(const wxPoint& pos);

		Maths::HVec3 PointSnap(
			const wxPoint& mousePos,
			const Maths::HVec3& manipulatorPos);

		Maths::HVec3 GridSnap(
			const wxPoint& mousePos,
			const Maths::HVec3& manipulatorPos);

		///	Find the closest edge to the mouse position
		/// Results are stored in member vars: m_pEdgeSelectionResult, m_pNearestPointResult, m_pEdge
		/// @return bool true if an edge is found, false otherwise
		bool FindClosestEdge(const wxPoint& mousePos);

		Maths::HMat44		m_StartTransform;
		Maths::HMat44		m_Transform;
		wxPoint				m_ClickPosition;
		F32					m_SnapMove;
		U8					m_Flags;
		U8					m_HoverFlags;
		
		struct SnapFlags
		{
			bool PointSnap : 1;
			bool EdgeSnap : 1;
			bool GridSnap : 1;
		};

		SnapFlags			m_SnapFlags;

		E_Constraint		m_Constraint;

		EditorLib::C_CollisionEdge *m_pEdge;

		bool				m_SnapToggle;

	private:
		DECLARE_EVENT_TABLE()
	};

}

#endif