#ifndef EDITOR_I_SCALE_SCENE_NODE_CONTEXT_H
#define EDITOR_I_SCALE_SCENE_NODE_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <EditorLib/Misc/Status.h>
#include "IVectorContext.h"

namespace Editor
{
	class C_ScaleSceneNodeContext : public I_VectorContext
	{
	public:
		C_ScaleSceneNodeContext();
		virtual ~C_ScaleSceneNodeContext();

		virtual bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);

		static void* Create();
		static E_ManipulatorContextType GetStaticType();
		static bool ManipulatorContextRegister();

		virtual EditorLib::C_Status Update(const Maths::HVec3& scale);
		virtual EditorLib::C_Status Apply(const Maths::HVec3& scale);
	};
}

#endif
