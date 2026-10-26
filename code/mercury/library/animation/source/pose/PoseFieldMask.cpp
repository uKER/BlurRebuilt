#include "stdafx.h"
#include <animation/pose/PoseFieldMask.h>

namespace Animation
{

const U32 C_PoseFieldMask::kFieldTypeMasks[Animation::E_PoseFieldType_Count] =
{
	(1 << E_PoseFieldElement_JointTransformRotation) | (1 << E_PoseFieldElement_JointTransformTranslation),
	(1 << E_PoseFieldElement_TrajectoryDeltaTransformRotation) | (1 << E_PoseFieldElement_TrajectoryDeltaTransformTranslation),
	(1 << E_PoseFieldElement_CustomVector3),
	(1 << E_PoseFieldElement_CustomScalar),
	(1 << E_PoseFieldElement_TrajectoryTransformRotation) | (1 << E_PoseFieldElement_TrajectoryTransformTranslation),
	(1 << E_PoseFieldElement_CustomVector4XYZ) | (1 << E_PoseFieldElement_CustomVector4W)
};

} // end Animation
