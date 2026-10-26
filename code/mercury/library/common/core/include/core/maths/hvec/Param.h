#ifndef MATHS_HVEC_PARAM_H
#define MATHS_HVEC_PARAM_H

#ifdef _MSC_VER
#pragma once
#endif

#if !defined( PLATFORM_PC ) || defined( __INTEL_COMPILER )

#define const_Scalar_Bool   const Maths::Scalar_Bool
#define const_Vector2_Bool  const Maths::Vector2_Bool
#define const_Vector3_Bool  const Maths::Vector3_Bool
#define const_Vector4_Bool  const Maths::Vector4_Bool
#define const_HF32          const Maths::HF32
#define const_HVecX         const Maths::HVecX
#define const_HVecY         const Maths::HVecY
#define const_HVecZ         const Maths::HVecZ
#define const_HVecW         const Maths::HVecW
#define const_HVec2         const Maths::HVec2
#define const_HVec3         const Maths::HVec3
#define const_HVec4         const Maths::HVec4
#define const_HMat33        const Maths::HMat33
#define const_HMat43        const Maths::HMat43
#define const_HMat44        const Maths::HMat44
#define const_C_Vertex      const Maths::C_Vertex
#define const_C_Vector      const Maths::C_Vector
#define const_C_Normal      const Maths::C_Normal
#define const_C_Quaternion  const Maths::C_Quaternion
#define const_C_Plane       const Maths::C_Plane
#define const_C_Sphere      const Maths::C_Sphere

#else

#define const_Scalar_Bool   const Maths::Scalar_Bool&
#define const_Vector2_Bool  const Maths::Vector2_Bool&
#define const_Vector3_Bool  const Maths::Vector3_Bool&
#define const_Vector4_Bool  const Maths::Vector4_Bool&
#define const_HF32          const Maths::HF32&
#define const_HVecX         const Maths::HVecX&
#define const_HVecY         const Maths::HVecY&
#define const_HVecZ         const Maths::HVecZ&
#define const_HVecW         const Maths::HVecW&
#define const_HVec2         const Maths::HVec2&
#define const_HVec3         const Maths::HVec3&
#define const_HVec4         const Maths::HVec4&
#define const_HMat33        const Maths::HMat33&
#define const_HMat43        const Maths::HMat43&
#define const_HMat44        const Maths::HMat44&
#define const_C_Vertex      const Maths::C_Vertex&
#define const_C_Vector      const Maths::C_Vector&
#define const_C_Normal      const Maths::C_Normal&
#define const_C_Quaternion  const Maths::C_Quaternion&
#define const_C_Plane       const Maths::C_Plane&
#define const_C_Sphere      const Maths::C_Sphere&

#endif

#endif // #ifndef MATHS_H
