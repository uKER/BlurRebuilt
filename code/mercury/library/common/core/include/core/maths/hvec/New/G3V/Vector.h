#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_VECTOR_H
#define SOFT_MATHS_HVEC_NEW_G3V_VECTOR_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_VECTOR_H
#define MATHS_HVEC_NEW_G3V_VECTOR_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup G3V_Vector
/// @{

/// Vector constants
template <> struct Const< G3V::Vector > 
{
	NOALIAS __forceinline static const G3V::Vector Zero();  ///< Constant Vector(0,0,0)
	NOALIAS __forceinline static const G3V::Vector One();   ///< Constant Vector(1,1,1)
	
	NOALIAS __forceinline static const G3V::Vector UnitX(); ///< Constant Vector(1,0,0)
	NOALIAS __forceinline static const G3V::Vector UnitY(); ///< Constant Vector(0,1,0)
	NOALIAS __forceinline static const G3V::Vector UnitZ(); ///< Constant Vector(0,0,1)
};

/// @}

namespace G3V
{
/// @addtogroup G3V_Vector
/// @{

/// Make Vector from 3 F32's
NOALIAS __forceinline const Vector MakeVector( const F32& x, const F32& y, const F32& z );

/// Make Vector from 3 VF32's
NOALIAS __forceinline const Vector MakeVector( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z );

/// Make Vector from Vec4F32
NOALIAS __forceinline const Vector MakeVector( const Vec4F32 VECREF v4 );

/// Make Vector from Vector3
NOALIAS __forceinline const Vector MakeVector( const Vector3 PCREF v3 );

/// Make Vector from Point3
NOALIAS __forceinline const Vector MakeVector( const Point3 PCREF p3 );

/// Make Vector from Normal3
NOALIAS __forceinline const Vector MakeVector( const Normal3 PCREF n3 );


/// Vector Accessor: Get X
NOALIAS __forceinline const VF32 X( const Vector PCREF src );

/// Vector Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const Vector PCREF src );

/// Vector Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const Vector PCREF src );

/// Vector Accessor: Set X
NOALIAS __forceinline void SetX( Vector* const __restrict pDst, const VF32 VECREF x );

/// Vector Accessor: Set Y
NOALIAS __forceinline void SetY( Vector* const __restrict pDst, const VF32 VECREF y );

/// Vector Accessor: Set Z
NOALIAS __forceinline void SetZ( Vector* const __restrict pDst, const VF32 VECREF z );


/// Vector Permute: Returns a permuted copy of it's Vec4F32 for internal use in implementing the product operators!
template < VectorElementIndex IX, VectorElementIndex IY, VectorElementIndex IZ >
NOALIAS __forceinline const Vector3 VPermute( const Vector PCREF src );


/// Vector Negation Operator
NOALIAS __forceinline const Vector operator - ( const Vector PCREF x );

/// Vector Addition Operator
NOALIAS __forceinline const Vector operator + ( const Vector PCREF a, const Vector PCREF b );

/// Vector Subtraction Operator
NOALIAS __forceinline const Vector operator - ( const Vector PCREF a, const Vector PCREF b );

/// Vector Addition Update Operator
NOALIAS __forceinline void operator += ( Vector& a, const Vector PCREF b );

/// Vector Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Vector& a, const Vector PCREF b );


/// Vector Reversion
NOALIAS __forceinline const Vector Reverse( const Vector PCREF v );

/// Vector Grade-Involution
NOALIAS __forceinline const Vector Involution( const Vector PCREF v );




/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


