#ifndef MATHS_HVEC_VECMAT_OP_H
#define MATHS_HVEC_VECMAT_OP_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

/// Star function = Cross Product Matrix ( a* Matrix )
NOALIAS const HMat33 Star( const HVec3 PCREF v );

/// HVec3 * HMat33 Multiplication
NOALIAS const HVec3 operator * ( const HVec3 PCREF v, const HMat33& m );

/// HVec3 * HMat33 Abs Transform : Similar to HVec3 * HMat33 Multiplication but takes abs of all products before summing
NOALIAS const HVec3 AbsTransform( const HVec3 PCREF v, const HMat33& m );

/// HVec3 * HMat43 Multiplication
NOALIAS const HVec3 operator * ( const HVec3 PCREF v, const HMat43& m );

/// HVec3 * HMat44 Multiplication
NOALIAS const HVec3 operator * ( const HVec3 PCREF v, const HMat44& m );


}; // namespace Maths

#endif

