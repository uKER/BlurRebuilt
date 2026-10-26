#ifndef EDITOR_ACTORCREATE_MANIPULATOR_H
#define EDITOR_ACTORCREATE_MANIPULATOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorApp/Editor/Manipulators/Manipulator.h>

namespace Editor
{
	enum E_CreationMode
	{
		E_CreationMode_Single,
		E_CreationMode_Multiple
	};

	enum E_OrientationMode
	{
		E_OrientationMode_Vertical,
		E_OrientationMode_Mesh
	};

	class C_ActorCreateManipulatorContext
	{
	public:
		C_ActorCreateManipulatorContext()
		{
			typeName = "";
			postCreate = "";
		}

		Btl::String typeName;
		Btl::String postCreate;
	};

	class C_ActorCreateManipulator : public I_Manipulator
	{
	public:
		C_ActorCreateManipulator();
		virtual ~C_ActorCreateManipulator();

		virtual E_ManipulatorType GetType() const;
		virtual bool IsA(E_ManipulatorType type) const;

		E_CreationMode GetCreationMode() const;
		void SetCreationMode(E_CreationMode creationMode);

		E_OrientationMode GetOrientationMode() const;
		void SetOrientationMode(E_OrientationMode orientationMode);

		void OnMouseEvent(wxMouseEvent &e);

		virtual void SetData(const void* userData);

	private:
		static E_CreationMode m_CreationMode;
		static E_OrientationMode m_OrientationMode;

		Btl::String m_ActorType;
		Btl::String m_PostCreate;

		DECLARE_EVENT_TABLE()
	};
}

#endif