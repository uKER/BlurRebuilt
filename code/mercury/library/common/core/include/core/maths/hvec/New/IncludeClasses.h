#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_INCLUDE_CLASSES_H
#define SOFT_MATHS_HVEC_NEW_INCLUDE_CLASSES_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_INCLUDE_CLASSES_H
#define MATHS_HVEC_NEW_INCLUDE_CLASSES_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

#include <Core/Maths.h>

#include "Core/Maths/storage.h"

#include "Core/Maths/HVec/SIMD.h"
#include "Core/Maths/HVec/VectorConst.h"
#include "Core/Maths/HVec/Storage.h"

#if defined( _PPU_ )
#include "Core/Maths/HVec/SIMD_PPU.inl"
#elif defined( _SPU_ )
#include "Core/Maths/HVec/SIMD_SPU.inl"
#elif defined( _VMX128_ )
#include "Core/Maths/HVec/SIMD_VMX.inl"
#elif defined( _SSE_ )
#include "Core/Maths/HVec/SIMD_SSE.inl"
#endif

//
// Includes for class definitions only, allowing all the maths classes to be stored in structures/classes or
// passed as parameters, but NO operators or functions are defined for them other than the basic copy
// constructors and assignment operators!
//

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

#include "Core/Maths/HVec/New/ClassBool.inl"
#include "Core/Maths/HVec/New/ClassScalar.inl"
#include "Core/Maths/HVec/New/ClassVector.inl"
#include "Core/Maths/HVec/New/ClassMatrix.inl"
#include "Core/Maths/HVec/New/ClassJacobian.inl"
#include "Core/Maths/HVec/New/ClassComplex.inl"
#include "Core/Maths/HVec/New/ClassDual.inl"
#include "Core/Maths/HVec/New/ClassQuaternion.inl"
#include "Core/Maths/HVec/New/ClassGeometry.inl"
//#include "Core/Maths/HVec/New/G3V/Class.inl"

#include "Core/Maths/HVec/New/Const.h"
#include "Core/Maths/HVec/New/Const.inl"

#endif // #if defined( INCLUDE_THIS )

