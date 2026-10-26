#ifndef EDITOR_I_ROTATE_SCENE_NODE_CONTEXT_H
#define EDITOR_I_ROTATE_SCENE_NODE_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorLib/Misc/Status.h>
#include "IOrientationContext.h"

namespace Editor
{
	class C_RotateSceneNodeContext : public I_OrientationContext
	{
	public:
		C_RotateSceneNodeContext();
		virtual ~C_RotateSceneNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& axis, F32 angle);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& axis, F32 angle);
	};
}

#endif
