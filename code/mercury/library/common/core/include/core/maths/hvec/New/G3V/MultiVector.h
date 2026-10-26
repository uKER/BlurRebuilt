#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_MULTIVECTOR_H
#define SOFT_MATHS_HVEC_NEW_G3V_MULTIVECTOR_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_MULTIVECTOR_H
#define MATHS_HVEC_NEW_G3V_MULTIVECTOR_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_MultiVector
/// @{

/// Make MultiVector from Scalar, Vector, BiVector & TriVector
NOALIAS __forceinline const MultiVector MakeMultiVector( const VF32 VECREF s, const Vector PCREF v, const BiVector PCREF b, const TriVector PCREF t );

/// Make MultiVector from a Vector8
NOALIAS __forceinline const MultiVector MakeMultiVector( const Vector8& v8 );

/// Get Vector8 from MultiVector
NOALIAS __forceinline const Vector8 GetVector8( const MultiVector& mv );

/// Get Scalar from MultiVector
NOALIAS __forceinline const VF32 GetScalar( const MultiVector& mv );

/// Get Vector from MultiVector
NOALIAS __forceinline const Vector GetVector( const MultiVector& mv );

/// Get BiVector from MultiVector
NOALIAS __forceinline const BiVector GetBiVector( const MultiVector& mv );

/// Get TriVector from MultiVector
NOALIAS __forceinline const TriVector GetTriVector( const MultiVector& mv );


/// MultiVector Get Element
template < MultiVectorElementIndex I >
NOALIAS __forceinline const VF32 Element( const MultiVector& mv );


/// MultiVector Permute : Indices select 8 result elements [I0,I1,I2,I3,I4,I5,I6,I7] from [ 0:e1, 1:e2, 2:e3, 3:s, 4:e12, 5:e23, 6:e31, 7:e123 ]
template < MultiVectorElementIndex I0,
           MultiVectorElementIndex I1,
           MultiVectorElementIndex I2,
           MultiVectorElementIndex I3,
           MultiVectorElementIndex I4,
           MultiVectorElementIndex I5,
           MultiVectorElementIndex I6,
           MultiVectorElementIndex I7 >
NOALIAS __forceinline const Vector8 MV_Permute( const MultiVector& mv );





/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


