#ifndef ANIMATION_DEBUGRENDER_H
#define ANIMATION_DEBUGRENDER_H

namespace DbUtil
{
	class I_DebugRender;
}

namespace Animation
{
	class C_Pose;
	class C_Skeleton;
	
	//
	// Debug rendering utility functions. NB. none of these functions are defined in CFG_GOLD builds.
	//

	#ifndef CFG_GOLD

	/// Renders the axes, names and parent-child connections (bones) for each transform in the pose.
	void DebugRenderPoseTransforms(
		DbUtil::I_DebugRender* pDebugRender,
		const C_Pose& pose,
		const C_Skeleton* skeleton,
		const Maths::Mat43& worldMatrix,
		bool renderTransforms = true,
		bool renderJointNames = false,
		bool renderBones = false,
		const Maths::RGBA& boneColour = Maths::RGBA(0xaa, 0xf0, 0xaa, 0xff),
		F32 jointAxisScale = 0.1f
	);
	
	/// Renders a matrix as 3 simple lines.
	void DebugRenderJointMatrix(DbUtil::I_DebugRender* pDebugRender, const Maths::Mat43& jointMatrix, F32 size);
	
	void DebugRenderStats(DbUtil::I_DebugRender* pDebugRender);
	
	#endif
}

#endif
