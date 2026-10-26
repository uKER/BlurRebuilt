
#ifndef CAMERA2_CAMOBJ_H
#define CAMERA2_CAMOBJ_H

namespace Cam2
{

using namespace Maths;

// An object which the camera would be interested in following, e.g. a player character, a bullet, a car...
class C_CamObj
{
public:
	C_CamObj(const HVec3& p = Maths::HVec3(), const C_Quaternion& o = C_Quaternion())
	{
		position = p;
		orient = o;
	}

	HVec3 ToWorldSpace(const HVec3& ospos) const;
	C_Quaternion ToWorldSpace(const C_Quaternion& ospos) const;

	HVec3 position;
	C_Quaternion orient;
};

} // End of namespace Cam2

#endif
