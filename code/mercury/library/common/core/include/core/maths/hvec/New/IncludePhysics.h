#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_INCLUDE_PHYSICS_H
#define SOFT_MATHS_HVEC_NEW_INCLUDE_PHYSICS_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_INCLUDE_PHYSICS_H
#define MATHS_HVEC_NEW_INCLUDE_PHYSICS_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

#if defined( COMPILER_VC )
#pragma warning( push )
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif

// Physics includes. 

#include <Core/Maths.h>

#include "Core/Maths/HVec/New/ClassBool.h"
#include "Core/Maths/HVec/New/ClassScalar.h"
#include "Core/Maths/HVec/New/ClassVector.h"
#include "Core/Maths/HVec/New/ClassMatrix.h"
#include "Core/Maths/HVec/New/ClassJacobian.h"
#include "Core/Maths/HVec/New/ClassComplex.h"
#include "Core/Maths/HVec/New/ClassDual.h"
#include "Core/Maths/HVec/New/ClassQuaternion.h"
#include "Core/Maths/HVec/New/ClassGeometry.h"
//#include "Core/Maths/HVec/New/G3V/Class.h"

#include "Core/Maths/HVec/New/Const.h"

#include "Core/Maths/HVec/New/VBool.h"
#include "Core/Maths/HVec/New/Vec4Bool.h"
#include "Core/Maths/HVec/New/Vec8Bool.h"
#include "Core/Maths/HVec/New/Vec16Bool.h"
#include "Core/Maths/HVec/New/VecInt8.h"
#include "Core/Maths/HVec/New/VecInt16.h"
#include "Core/Maths/HVec/New/VecInt32.h"
#include "Core/Maths/HVec/New/VecInt64.h"
#include "Core/Maths/HVec/New/VF32.h"
#include "Core/Maths/HVec/New/Vec4F32.h"
#include "Core/Maths/HVec/New/Vec8F32.h"
#include "Core/Maths/HVec/New/Vec12F32.h"
#include "Core/Maths/HVec/New/Elementary.h"

#include "Core/Maths/HVec/New/Mat33.h"
#include "Core/Maths/HVec/New/Mat43.h"
#include "Core/Maths/HVec/New/Mat44.h"
#include "Core/Maths/HVec/New/ScaleMat3.h"
#include "Core/Maths/HVec/New/ShearMat3.h"
#include "Core/Maths/HVec/New/Euler.h"

#include "Core/Maths/HVec/New/Point3.h"
#include "Core/Maths/HVec/New/HPoint3.h"
#include "Core/Maths/HVec/New/Vector2.h"
#include "Core/Maths/HVec/New/Vector3.h"
#include "Core/Maths/HVec/New/Vector4.h"
#include "Core/Maths/HVec/New/Vector5.h"
#include "Core/Maths/HVec/New/Vector6.h"
#include "Core/Maths/HVec/New/Vector7.h"
#include "Core/Maths/HVec/New/Vector8.h"
#include "Core/Maths/HVec/New/Normal3.h"
#include "Core/Maths/HVec/New/Vector3NL.h"

#include "Core/Maths/HVec/New/Mat26.h"
#include "Core/Maths/HVec/New/Mat36.h"
#include "Core/Maths/HVec/New/Mat46.h"
#include "Core/Maths/HVec/New/Mat55.h"
#include "Core/Maths/HVec/New/Mat56.h"
#include "Core/Maths/HVec/New/Mat66.h"
#include "Core/Maths/HVec/New/Mat76.h"
#include "Core/Maths/HVec/New/Mat62.h"
#include "Core/Maths/HVec/New/Mat63.h"
#include "Core/Maths/HVec/New/Mat64.h"
#include "Core/Maths/HVec/New/Mat65.h"
#include "Core/Maths/HVec/New/Mat67.h"

#include "Core/Maths/HVec/New/Complex.h"
#include "Core/Maths/HVec/New/DualScalar.h"
#include "Core/Maths/HVec/New/DualVector3.h"
#include "Core/Maths/HVec/New/Cubic.h"
#include "Core/Maths/HVec/New/Quaternion.h"
#include "Core/Maths/HVec/New/QuatTrans.h"
#include "Core/Maths/HVec/New/DualQuaternion.h"
#include "Core/Maths/HVec/New/AxisBox.h"
#include "Core/Maths/HVec/New/OrientedBox.h"
#include "Core/Maths/HVec/New/Line.h"
#include "Core/Maths/HVec/New/Triangle.h"
#include "Core/Maths/HVec/New/Plane.h"
#include "Core/Maths/HVec/New/Sphere.h"
#include "Core/Maths/HVec/New/Cylinder.h"
#include "Core/Maths/HVec/New/Capsule.h"

#include "Core/Maths/HVec/New/Distance.h"
#include "Core/Maths/HVec/New/Intersect.h"
#include "Core/Maths/HVec/New/Closest.h"
#include "Core/Maths/HVec/New/Volume.h"

#if defined( COMPILER_VC )
#pragma warning( pop )
#endif

#endif // #if defined( INCLUDE_THIS )

