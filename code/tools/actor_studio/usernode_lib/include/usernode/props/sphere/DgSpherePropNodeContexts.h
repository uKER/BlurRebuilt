#ifndef EDITOR_PROPS_SPHERE_PROP_CONTEXTS_H
#define EDITOR_PROPS_SPHERE_PROP_CONTEXTS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_ScaleDgSpherePropNodeContext : public I_VectorContext
	{
	public:
		C_ScaleDgSpherePropNodeContext ();
		virtual ~C_ScaleDgSpherePropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& translate);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& translate);
	};

	class C_MoveDgSpherePropNodeContext : public I_VectorContext
	{
	public:
		C_MoveDgSpherePropNodeContext();
		virtual ~C_MoveDgSpherePropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& translate);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& translate);
	};
}

#endif
