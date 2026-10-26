#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VEC8BOOL_H
#define SOFT_MATHS_HVEC_NEW_VEC8BOOL_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VEC8BOOL_H
#define MATHS_HVEC_NEW_VEC8BOOL_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Boolean
/// @{

/// Make Vec8Bool from a set of 8 template bools ( compile time constants )
template < bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7 >
NOALIAS __forceinline const Vec8Bool MakeVec8Bool();

/// Make Vec8Bool from 2 Vec4Bool's
NOALIAS __forceinline const Vec8Bool MakeVec8Bool( const Vec4Bool PCREF a, const Vec4Bool PCREF b );

/// Split Vec8Bool into 2 Vec4Bool's
NOALIAS __forceinline void Split( const Vec8Bool PCREF src, Vec4Bool* const __restrict pA, Vec4Bool* const __restrict pB );


/// Vec8Bool accessor: Get element
template < int I >
NOALIAS __forceinline const VBool Element( const Vec8Bool PCREF v );

/// Vec8Bool accessor: Set element
template < int I >
NOALIAS __forceinline void SetElement( Vec8Bool* const __restrict pDst, const VBool PCREF src );


/// Vec8Bool negation operator
NOALIAS __forceinline const Vec8Bool operator ~ ( const Vec8Bool PCREF x ) __attribute__(( /* pure, */ always_inline ));

/// Vec8Bool logical and operator
NOALIAS __forceinline const Vec8Bool operator & ( const Vec8Bool PCREF x, const Vec8Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec8Bool logical or operator
NOALIAS __forceinline const Vec8Bool operator | ( const Vec8Bool PCREF x, const Vec8Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec8Bool logical xor operator
NOALIAS __forceinline const Vec8Bool operator ^ ( const Vec8Bool PCREF x, const Vec8Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec8Bool equal operator
NOALIAS __forceinline const VBool operator == ( const Vec8Bool PCREF x, const Vec8Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec8Bool not-equal operator
NOALIAS __forceinline const VBool operator != ( const Vec8Bool PCREF x, const Vec8Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec8Bool logical and assignment operator
NOALIAS __forceinline void operator &= ( Vec8Bool& x, const Vec8Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec8Bool logical or assignment operator
NOALIAS __forceinline void operator |= ( Vec8Bool& x, const Vec8Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));
    


/// Vec8Bool & VBool logical and operator
NOALIAS __forceinline const Vec8Bool operator & ( const Vec8Bool PCREF x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool & Vec8Bool logical and operator
NOALIAS __forceinline const Vec8Bool operator & ( const VBool PCREF x, const Vec8Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec8Bool & VBool logical or operator
NOALIAS __forceinline const Vec8Bool operator | ( const Vec8Bool PCREF x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool & Vec8Bool logical or operator
NOALIAS __forceinline const Vec8Bool operator | ( const VBool PCREF x, const Vec8Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec8Bool & VBool logical xor operator
NOALIAS __forceinline const Vec8Bool operator ^ ( const Vec8Bool PCREF x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool & Vec8Bool logical xor operator
NOALIAS __forceinline const Vec8Bool operator ^ ( const VBool PCREF x, const Vec8Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec8Bool & VBool logical and assignment operator
NOALIAS __forceinline void operator &= ( Vec8Bool& x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec8Bool & VBool logical or assignment operator
NOALIAS __forceinline void operator |= ( Vec8Bool& x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


