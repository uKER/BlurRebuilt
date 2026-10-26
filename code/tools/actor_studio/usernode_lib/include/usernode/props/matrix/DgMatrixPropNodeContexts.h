#ifndef EDITOR_MOVE_MATRIX_PROP_CONTEXT_H
#define EDITOR_MOVE_MATRIX_PROP_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_ScaleDgMatrixPropNodeContext : public I_VectorContext
	{
	public:
		C_ScaleDgMatrixPropNodeContext();
		virtual ~C_ScaleDgMatrixPropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& scale);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& scale);
	};

	class C_RotateDgMatrixPropNodeContext : public I_OrientationContext
	{
	public:
		C_RotateDgMatrixPropNodeContext ();
		virtual ~C_RotateDgMatrixPropNodeContext ();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& axis, F32 angle);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& axis, F32 angle);
	};

	class C_MoveDgMatrixPropNodeContext : public I_VectorContext
	{
	public:
		C_MoveDgMatrixPropNodeContext ();
		virtual ~C_MoveDgMatrixPropNodeContext ();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& translate);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& translate);
	};
}

#endif
