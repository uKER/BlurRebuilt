#include "stdafx.h"
#include <animation/common/AnimationMaths.h>
#include <system/Perf.h>
#include <core/maths/hvec/new/IncludeMinimal.h>
#include <core/maths/hvec/new/IncludeMinimal.inl>

namespace Animation
{

const Maths::Vec4F32 gFastArcTanConstants = Maths::MakeVec4F32( 0.28f, 1.5707963268f, 1.f, 0.f );

void QuaternionToEuler( const Maths::Quaternion& q, Maths::Vector3& eulerXYZ )
{
	Maths::Mat33 m = Maths::MakeMat33(q);
	Maths::Vector3 eulerXYZ1 = Maths::GetVector3(Maths::MakeEulerXYZ(m));
	
	// Choose representation with angles closest to zero.
	const Maths::VF32 pi = Maths::Const<Maths::VF32>::Pi();
	const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
	const Maths::Vec4F32 minusPi = Maths::MakeVec4F32(-pi);
	const Maths::Vector3 eulerXYZ2 = Maths::MakeVector3(minusPi) + eulerXYZ1 * Maths::MakeVector3(one,-one,one);
	
	eulerXYZ = Maths::Select(Maths::LengthSquared(eulerXYZ1) < Maths::LengthSquared(eulerXYZ2), eulerXYZ1, eulerXYZ2);
}

void QuaternionFromEuler( Maths::Quaternion& q, const Maths::Vector3& eulerXYZ )
{
	const Maths::Quaternion rx = Maths::MakeUnitQuaternion(Maths::Const<Maths::Normal3>::AxisX(), Maths::X(eulerXYZ));
	const Maths::Quaternion ry = Maths::MakeUnitQuaternion(Maths::Const<Maths::Normal3>::AxisY(), Maths::Y(eulerXYZ));
	const Maths::Quaternion rz = Maths::MakeUnitQuaternion(Maths::Const<Maths::Normal3>::AxisZ(), Maths::Z(eulerXYZ));
	const Maths::Quaternion result = rx * ry * rz;
	q = result;
}

} // namespace Animation
