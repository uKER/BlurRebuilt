#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_INCLUDE_MINIMAL_H
#define SOFT_MATHS_HVEC_NEW_INCLUDE_MINIMAL_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_INCLUDE_MINIMAL_H
#define MATHS_HVEC_NEW_INCLUDE_MINIMAL_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

#if defined( COMPILER_VC )
#pragma warning( push )
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif

//
// Minimal set of includes, allowing all the maths classes to be stored in structures/classes or
// passed as parameters, with accessors and basic arithmetic operators & functions declared for
// the bool scalar & vector classes and the float scalar, vector & matrix classes.
//

#include <Core/Maths.h>

#include "Core/Maths/HVec/New/ClassBool.h"
#include "Core/Maths/HVec/New/ClassScalar.h"
#include "Core/Maths/HVec/New/ClassVector.h"
#include "Core/Maths/HVec/New/ClassMatrix.h"
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
#include "Core/Maths/HVec/New/Normal3.h"
#include "Core/Maths/HVec/New/Vector3NL.h"

//#include "Core/Maths/HVec/New/Quaternion.h"

#if defined( COMPILER_VC )
#pragma warning( pop )
#endif

#endif // #if defined( INCLUDE_THIS )

