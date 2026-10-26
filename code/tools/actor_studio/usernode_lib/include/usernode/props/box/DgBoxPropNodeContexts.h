#ifndef EDITOR_PROPS_DG_BOX_PROP_NODE_CONTEXTS_H
#define EDITOR_PROPS_DG_BOX_PROP_NODE_CONTEXTS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Editor
{
	class C_ScaleDgBoxPropNodeContext : public I_VectorContext
	{
	public:
		C_ScaleDgBoxPropNodeContext();
		virtual ~C_ScaleDgBoxPropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& translate);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& translate);
	};

	class C_RotateDgBoxPropNodeContext : public I_OrientationContext
	{
	public:
		C_RotateDgBoxPropNodeContext();
		virtual ~C_RotateDgBoxPropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& axis, F32 angle);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& axis, F32 angle);
	};

	//Move box property indirectly by transforming it's parent
	//(box properties do not have their own position)
	class C_MoveDgBoxPropNodeContext : public I_VectorContext
	{
	public:
		C_MoveDgBoxPropNodeContext();
		virtual ~C_MoveDgBoxPropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& translate);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& translate);
	};
}

#endif
