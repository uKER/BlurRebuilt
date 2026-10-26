#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_GEOMETRIC_PRODUCT_H
#define SOFT_MATHS_HVEC_NEW_G3V_GEOMETRIC_PRODUCT_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_GEOMETRIC_PRODUCT_H
#define MATHS_HVEC_NEW_G3V_GEOMETRIC_PRODUCT_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_GeometricProduct
/// @{

/// Geometric Product Operator: VF32 * Vector
NOALIAS __forceinline const Vector operator * ( const VF32 VECREF a, const Vector PCREF b );

/// Geometric Product Operator: Vector * VF32
NOALIAS __forceinline const Vector operator * ( const Vector PCREF a, const VF32 VECREF b );

/// Geometric Product Operator: VF32 * BiVector
NOALIAS __forceinline const BiVector operator * ( const VF32 VECREF a, const BiVector PCREF b );

/// Geometric Product Operator: BiVector * VF32
NOALIAS __forceinline const BiVector operator * ( const BiVector PCREF a, const VF32 VECREF b );

/// Geometric Product Operator: VF32 * TriVector
NOALIAS __forceinline const TriVector operator * ( const VF32 VECREF a, const TriVector PCREF b );

/// Geometric Product Operator: TriVector * VF32
NOALIAS __forceinline const TriVector operator * ( const TriVector PCREF a, const VF32 VECREF b );


/// Geometric Product Operator: Vector * Vector
NOALIAS __forceinline const MultiVecSB operator * ( const Vector PCREF a, const Vector PCREF b );


/// Geometric Product Operator: Vector * BiVector
NOALIAS __forceinline const MultiVecVT operator * ( const Vector PCREF a, const BiVector PCREF b );

/// Geometric Product Operator: BiVector * Vector
NOALIAS __forceinline const MultiVecVT operator * ( const BiVector PCREF a, const Vector PCREF b );


/// Geometric Product Operator: Vector * TriVector
NOALIAS __forceinline const BiVector operator * ( const Vector PCREF a, const TriVector PCREF b );

/// Geometric Product Operator: TriVector * Vector
NOALIAS __forceinline const BiVector operator * ( const TriVector PCREF a, const Vector PCREF b );


/// Geometric Product Operator: BiVector * BiVector
NOALIAS __forceinline const MultiVecSB operator * ( const BiVector PCREF a, const BiVector PCREF b );


/// Geometric Product Operator: BiVector * TriVector
NOALIAS __forceinline const Vector operator * ( const BiVector PCREF a, const TriVector PCREF b );

/// Geometric Product Operator: TriVector * BiVector
NOALIAS __forceinline const Vector operator * ( const TriVector PCREF a, const BiVector PCREF b );


/// Geometric Product Operator: TriVector * TriVector
NOALIAS __forceinline const VF32 operator * ( const TriVector PCREF a, const TriVector PCREF b );


/// Geometric Product Operator: MultiVector * MultiVector
NOALIAS __forceinline const MultiVector operator * ( const MultiVector& a, const MultiVector& b );



/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


