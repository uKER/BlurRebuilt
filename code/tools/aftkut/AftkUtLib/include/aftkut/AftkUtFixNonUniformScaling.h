#ifndef AFTK_UT_FIX_NON_UNIFORM_SCALING_H
#define AFTK_UT_FIX_NON_UNIFORM_SCALING_H


#ifdef _MSC_VER
#pragma once
#endif

namespace AftkUt
{
	class C_FixNonUniformScaling
	{
	public:
		static Aftk::C_Status Fix(
			Aftk::C_Scene& aftkScene);
		
		static bool HasNonUniformScaling(
			const Aftk::C_SceneNode& aftkSceneNode);
	};
}

#endif
