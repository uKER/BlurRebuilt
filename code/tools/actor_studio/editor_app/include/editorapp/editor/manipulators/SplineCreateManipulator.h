#ifndef EDITOR_SPLINECREATE_MANIPULATOR_H
#define EDITOR_SPLINECREATE_MANIPULATOR_H

#ifdef _MSC_VER
#pragma once
#endif

namespace EditorLib
{
	class C_SceneNode;
	class C_DgSplinePropNode;
}

namespace Editor
{
	class C_SplineCreateManipulator : public I_Manipulator
	{
	public:
		C_SplineCreateManipulator();
		virtual ~C_SplineCreateManipulator();

		virtual E_ManipulatorType GetType() const;
		virtual bool IsA(E_ManipulatorType type) const;

		virtual void SetData(const void* userData);

		void OnMouseEvent(wxMouseEvent &e);

	private:
		DECLARE_EVENT_TABLE()
	};
}

#endif