#ifndef EDITOR_PROPS_DG_ANIMATION_PROP_NODE_CONTEXTS_H
#define EDITOR_PROPS_DG_ANIMATION_PROP_NODE_CONTEXTS_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Context/DeleteContextMapping.h>

namespace Editor
{
	class C_RotateDgAnimationPropNodeContext : public I_OrientationContext
	{
	public:
		C_RotateDgAnimationPropNodeContext();
		virtual ~C_RotateDgAnimationPropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& axis, F32 angle);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& axis, F32 angle);
	private:
		Maths::C_Quaternion m_PrevValue;
	};

	class C_MoveDgAnimationPropNodeContext : public I_VectorContext
	{
	public:
		C_MoveDgAnimationPropNodeContext();
		virtual ~C_MoveDgAnimationPropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& translate);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& translate);

	private:
		Maths::HVec3 m_PrevValue;
	};

	class C_DeleteDgAnimationPropNodeContext : public EditorLib::I_DeleteContext
	{
	public:
		static void* Create();
		static EditorLib::E_DeleteContextType GetStaticType();
		static bool DeleteContextRegister();

		virtual bool Delete(EditorLib::I_DgNode* node, EditorLib::C_Selection& selection);
	};
}

#endif
