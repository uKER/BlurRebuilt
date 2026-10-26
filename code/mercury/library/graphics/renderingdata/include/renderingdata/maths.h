#ifndef RD_MATHS_H
#define RD_MATHS_H
#ifdef _MSC_VER
#pragma once
#endif

#include "core/maths/utmaths/maths.h"

namespace RenderingData
{
	typedef UtMaths::VectorXYZ<float> Vector3Base;
	typedef UtMaths::VectorXYZW<float> Vector4Base;

	typedef UtMaths::MPoint<Vector3Base> Point3;
	typedef UtMaths::MVector<Vector3Base> Vector3;
	typedef UtMaths::MNormal<Vector3Base> Normal3;
	typedef UtMaths::MVector<Vector4Base> Vector4;
}

#endif
