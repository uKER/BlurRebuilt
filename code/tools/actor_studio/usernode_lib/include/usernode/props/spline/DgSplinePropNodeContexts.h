#ifndef EDITOR_PROPS_DG_SPLINE_PROP_NODE_CONTEXTS_H
#define EDITOR_PROPS_DG_SPLINE_PROP_NODE_CONTEXTS_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Context/DeleteContextMapping.h>

namespace Editor
{
	class C_ScaleDgSplinePropNodeContext : public I_VectorContext
	{
	public:
		C_ScaleDgSplinePropNodeContext();
		virtual ~C_ScaleDgSplinePropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& scale);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& scale);

	private:
		Maths::HVec3 m_PrevValue;
	};

	class C_MoveDgSplinePropNodeContext : public I_VectorContext
	{
	public:
		C_MoveDgSplinePropNodeContext();
		virtual ~C_MoveDgSplinePropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& translate);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& translate);

	private:
		Maths::HVec3 m_PrevValue;
	};

	class C_DeleteDgSplinePropNodeContext : public EditorLib::I_DeleteContext
	{
	public:
		static void* Create();
		static EditorLib::E_DeleteContextType GetStaticType();
		static bool DeleteContextRegister();

		virtual bool Delete(EditorLib::I_DgNode* node, EditorLib::C_Selection& selection);
	};
}

#endif
