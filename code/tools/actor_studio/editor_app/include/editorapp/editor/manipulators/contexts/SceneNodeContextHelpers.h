#ifndef EDITOR_SCENE_NODE_CONTEXT_HELPERS_H
#define EDITOR_SCENE_NODE_CONTEXT_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	namespace SceneNodeContextHelpers
	{
		bool GetInitialMatrix(Maths::HMat44& matrix, E_ManipulatorSpace space);
	}
}

#endif
