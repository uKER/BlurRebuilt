#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_INCLUDE_COMPLETE_GA_INL
#define SOFT_MATHS_HVEC_NEW_INCLUDE_COMPLETE_GA_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_INCLUDE_COMPLETE_GA_INL
#define MATHS_HVEC_NEW_INCLUDE_COMPLETE_GA_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

#if defined( COMPILER_VC )
#pragma warning( push )
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif

#pragma message( "WARNING: Including complete new maths & GA library!" )

#include "Core/Maths/HVec/New/ClassBool.inl"
#include "Core/Maths/HVec/New/ClassScalar.inl"
#include "Core/Maths/HVec/New/ClassVector.inl"
#include "Core/Maths/HVec/New/ClassMatrix.inl"
#include "Core/Maths/HVec/New/ClassJacobian.inl"
#include "Core/Maths/HVec/New/ClassComplex.inl"
#include "Core/Maths/HVec/New/ClassDual.inl"
#include "Core/Maths/HVec/New/ClassQuaternion.inl"
#include "Core/Maths/HVec/New/ClassGeometry.inl"
#include "Core/Maths/HVec/New/G3V/Class.inl"
#include "Core/Maths/HVec/New/G5/Class.inl"

#include "Core/Maths/HVec/New/Const.inl"

#include "Core/Maths/HVec/New/VBool.inl"
#include "Core/Maths/HVec/New/Vec4Bool.inl"
#include "Core/Maths/HVec/New/Vec8Bool.inl"
#include "Core/Maths/HVec/New/Vec16Bool.inl"
#include "Core/Maths/HVec/New/VecInt8.inl"
#include "Core/Maths/HVec/New/VecInt16.inl"
#include "Core/Maths/HVec/New/VecInt32.inl"
#include "Core/Maths/HVec/New/VecInt64.inl"
#include "Core/Maths/HVec/New/VF32.inl"
#include "Core/Maths/HVec/New/Vec4F32.inl"
#include "Core/Maths/HVec/New/Vec8F32.inl"
#include "Core/Maths/HVec/New/Vec12F32.inl"
#include "Core/Maths/HVec/New/Elementary.inl"

#include "Core/Maths/HVec/New/Mat33.inl"
#include "Core/Maths/HVec/New/Mat43.inl"
#include "Core/Maths/HVec/New/Mat44.inl"
#include "Core/Maths/HVec/New/ScaleMat3.inl"
#include "Core/Maths/HVec/New/ShearMat3.inl"
#include "Core/Maths/HVec/New/Euler.inl"

#include "Core/Maths/HVec/New/Mat26.inl"
#include "Core/Maths/HVec/New/Mat36.inl"
#include "Core/Maths/HVec/New/Mat46.inl"
#include "Core/Maths/HVec/New/Mat55.inl"
#include "Core/Maths/HVec/New/Mat56.inl"
#include "Core/Maths/HVec/New/Mat66.inl"
#include "Core/Maths/HVec/New/Mat76.inl"
#include "Core/Maths/HVec/New/Mat62.inl"
#include "Core/Maths/HVec/New/Mat63.inl"
#include "Core/Maths/HVec/New/Mat64.inl"
#include "Core/Maths/HVec/New/Mat65.inl"
#include "Core/Maths/HVec/New/Mat67.inl"

#include "Core/Maths/HVec/New/Point3.inl"
#include "Core/Maths/HVec/New/HPoint3.inl"
#include "Core/Maths/HVec/New/Vector2.inl"
#include "Core/Maths/HVec/New/Vector3.inl"
#include "Core/Maths/HVec/New/Vector4.inl"
#include "Core/Maths/HVec/New/Vector5.inl"
#include "Core/Maths/HVec/New/Vector6.inl"
#include "Core/Maths/HVec/New/Vector7.inl"
#include "Core/Maths/HVec/New/Vector8.inl"
#include "Core/Maths/HVec/New/Normal3.inl"
#include "Core/Maths/HVec/New/Vector3NL.inl"

#include "Core/Maths/HVec/New/Complex.inl"
#include "Core/Maths/HVec/New/DualScalar.inl"
#include "Core/Maths/HVec/New/DualVector3.inl"
#include "Core/Maths/HVec/New/Cubic.inl"
#include "Core/Maths/HVec/New/Quaternion.inl"
#include "Core/Maths/HVec/New/QuatTrans.inl"
#include "Core/Maths/HVec/New/DualQuaternion.inl"
#include "Core/Maths/HVec/New/AxisBox.inl"
#include "Core/Maths/HVec/New/OrientedBox.inl"
#include "Core/Maths/HVec/New/Line.inl"
#include "Core/Maths/HVec/New/Triangle.inl"
#include "Core/Maths/HVec/New/Plane.inl"
#include "Core/Maths/HVec/New/Sphere.inl"
#include "Core/Maths/HVec/New/Cylinder.inl"
#include "Core/Maths/HVec/New/Capsule.inl"

#include "Core/Maths/HVec/New/Distance.inl"
#include "Core/Maths/HVec/New/Intersect.inl"
#include "Core/Maths/HVec/New/Closest.inl"
#include "Core/Maths/HVec/New/Volume.inl"

#include "Core/Maths/HVec/New/G3V/G3V.inl"
#include "Core/Maths/HVec/New/G3V/Scalar.inl"
#include "Core/Maths/HVec/New/G3V/Vector.inl"
#include "Core/Maths/HVec/New/G3V/BiVector.inl"
#include "Core/Maths/HVec/New/G3V/TriVector.inl"
#include "Core/Maths/HVec/New/G3V/MultiVector.inl"
#include "Core/Maths/HVec/New/G3V/MultiVecSV.inl"
#include "Core/Maths/HVec/New/G3V/MultiVecSB.inl"
#include "Core/Maths/HVec/New/G3V/MultiVecST.inl"
#include "Core/Maths/HVec/New/G3V/MultiVecVB.inl"
#include "Core/Maths/HVec/New/G3V/MultiVecVT.inl"
#include "Core/Maths/HVec/New/G3V/MultiVecBT.inl"

#include "Core/Maths/HVec/New/G3V/ContractLeft.inl"
#include "Core/Maths/HVec/New/G3V/ContractRight.inl"
#include "Core/Maths/HVec/New/G3V/ScalarProduct.inl"
#include "Core/Maths/HVec/New/G3V/InnerProduct.inl"
#include "Core/Maths/HVec/New/G3V/OuterProduct.inl"
#include "Core/Maths/HVec/New/G3V/GeometricProduct.inl"

#include "Core/Maths/HVec/New/G5/Templates.inl"
#include "Core/Maths/HVec/New/G5/T_Vector.inl"
#include "Core/Maths/HVec/New/G5/Const.inl"
#include "Core/Maths/HVec/New/G5/OperatorGP.inl"
#include "Core/Maths/HVec/New/G5/OperatorOP.inl"
#include "Core/Maths/HVec/New/G5/OperatorIP.inl"
#include "Core/Maths/HVec/New/G5/OperatorSP.inl"
#include "Core/Maths/HVec/New/G5/OperatorLC.inl"
#include "Core/Maths/HVec/New/G5/OperatorRC.inl"
#include "Core/Maths/HVec/New/G5/Involution.inl"
#include "Core/Maths/HVec/New/G5/Reverse.inl"
#include "Core/Maths/HVec/New/G5/Inverse.inl"
#include "Core/Maths/HVec/New/G5/Dual.inl"
#include "Core/Maths/HVec/New/G5/Scalar.inl"
#include "Core/Maths/HVec/New/G5/Vector.inl"
#include "Core/Maths/HVec/New/G5/Point.inl"
#include "Core/Maths/HVec/New/G5/DualPlane.inl"
#include "Core/Maths/HVec/New/G5/DualSphere.inl"
#include "Core/Maths/HVec/New/G5/Scalor.inl"
#include "Core/Maths/HVec/New/G5/Rotor.inl"
#include "Core/Maths/HVec/New/G5/Translator.inl"

#if defined( COMPILER_VC )
#pragma warning( pop )
#endif

#endif // #if defined( INCLUDE_THIS )

