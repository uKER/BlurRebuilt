#ifndef EDITOR_PROPS_RADIUS_PROP_CONTEXTS_H
#define EDITOR_PROPS_RADIUS_PROP_CONTEXTS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	class C_ScaleDgRadiusPropNodeContext : public I_VectorContext
	{
	public:
		C_ScaleDgRadiusPropNodeContext ();
		virtual ~C_ScaleDgRadiusPropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& translate);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& translate);
	};
}

#endif
