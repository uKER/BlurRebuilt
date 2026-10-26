#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_BIVECTOR_H
#define SOFT_MATHS_HVEC_NEW_G3V_BIVECTOR_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_BIVECTOR_H
#define MATHS_HVEC_NEW_G3V_BIVECTOR_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup G3V_BiVector
/// @{

/// BiVector constants
template <> struct Const< G3V::BiVector > 
{
	NOALIAS __forceinline static const G3V::BiVector Zero();  ///< Constant BiVector(0,0,0)
	NOALIAS __forceinline static const G3V::BiVector One();   ///< Constant BiVector(1,1,1)
	
	NOALIAS __forceinline static const G3V::BiVector UnitX(); ///< Constant BiVector(1,0,0)
	NOALIAS __forceinline static const G3V::BiVector UnitY(); ///< Constant BiVector(0,1,0)
	NOALIAS __forceinline static const G3V::BiVector UnitZ(); ///< Constant BiVector(0,0,1)
};

/// @}

namespace G3V
{
/// @addtogroup G3V_BiVector
/// @{

/// Make BiVector from 3 F32's
NOALIAS __forceinline const BiVector MakeBiVector( const F32& x, const F32& y, const F32& z );

/// Make BiVector from 3 VF32's
NOALIAS __forceinline const BiVector MakeBiVector( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z );

/// Make BiVector from Vec4F32
NOALIAS __forceinline const BiVector MakeBiVector( const Vec4F32 VECREF v4 );

/// Make BiVector from Vector3
NOALIAS __forceinline const BiVector MakeBiVector( const Vector3 PCREF v3 );


/// BiVector Accessor: Get X
NOALIAS __forceinline const VF32 X( const BiVector PCREF src );

/// BiVector Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const BiVector PCREF src );

/// BiVector Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const BiVector PCREF src );


/// BiVector Accessor: Set X
NOALIAS __forceinline void SetX( BiVector* const __restrict pDst, const VF32 VECREF x );

/// BiVector Accessor: Set Y
NOALIAS __forceinline void SetY( BiVector* const __restrict pDst, const VF32 VECREF y );

/// BiVector Accessor: Set Z
NOALIAS __forceinline void SetZ( BiVector* const __restrict pDst, const VF32 VECREF z );


/// BiVector Permute: Returns a permuted copy of it's Vec4F32 for internal use in implementing the product operators!
template < BiVectorElementIndex IX, BiVectorElementIndex IY, BiVectorElementIndex IZ >
NOALIAS __forceinline const Vector3 BPermute( const BiVector PCREF src );


/// BiVector Negation Operator
NOALIAS __forceinline const BiVector operator - ( const BiVector PCREF x );

/// BiVector Addition Operator
NOALIAS __forceinline const BiVector operator + ( const BiVector PCREF a, const BiVector PCREF b );

/// BiVector Subtraction Operator
NOALIAS __forceinline const BiVector operator - ( const BiVector PCREF a, const BiVector PCREF b );

/// BiVector Addition Update Operator
NOALIAS __forceinline void operator += ( BiVector& a, const BiVector PCREF b );

/// BiVector Subtraction Update Operator
NOALIAS __forceinline void operator -= ( BiVector& a, const BiVector PCREF b );


/// BiVector Reversion
NOALIAS __forceinline const BiVector Reverse( const BiVector PCREF bv );

/// BiVector Grade-Involution
NOALIAS __forceinline const BiVector Involution( const BiVector PCREF bv );









/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


