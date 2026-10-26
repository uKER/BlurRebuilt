
#include "stdafx.h"
#include <camera2/CamObj.h>

namespace Cam2
{

using namespace Maths;

// Object space position to world space:
HVec3 C_CamObj::ToWorldSpace(const HVec3& ospos) const
{
	HMat33 matrix = orient;
	return position + ospos * matrix;
}

// Object space orientation to world space:
C_Quaternion C_CamObj::ToWorldSpace(const C_Quaternion& oso) const
{
	return orient * oso;
}

} // End of namespace
