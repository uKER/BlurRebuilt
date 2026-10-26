#ifndef EDITOR_PROPS_DG_RECTANGLE_PROP_NODE_CONTEXTS_H
#define EDITOR_PROPS_DG_RECTANGLE_PROP_NODE_CONTEXTS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Editor
{
	class C_ScaleDgRectanglePropNodeContext : public I_VectorContext
	{
	public:
		C_ScaleDgRectanglePropNodeContext();
		virtual ~C_ScaleDgRectanglePropNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& translate);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& translate);
	};
}

#endif
