#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VEC16BOOL_H
#define SOFT_MATHS_HVEC_NEW_VEC16BOOL_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VEC16BOOL_H
#define MATHS_HVEC_NEW_VEC16BOOL_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Boolean
/// @{

/// Make Vec16Bool from a set of 16 template bools ( compile time constants )
template < bool b0,  bool b1,  bool b2,  bool b3,
           bool b4,  bool b5,  bool b6,  bool b7,
           bool b8,  bool b9,  bool b10, bool b11,
           bool b12, bool b13, bool b14, bool b15 >
NOALIAS __forceinline const Vec16Bool MakeVec16Bool();

/// Make Vec16Bool from 4 Vec4Bool's
NOALIAS __forceinline const Vec16Bool MakeVec16Bool( const Vec4Bool PCREF a, const Vec4Bool PCREF b, const Vec4Bool PCREF c, const Vec4Bool PCREF d );

/// Split Vec16Bool into 4 Vec4Bool's
NOALIAS __forceinline void Split( const Vec16Bool PCREF src,
                                  Vec4Bool* const __restrict pA,
                                  Vec4Bool* const __restrict pB,
                                  Vec4Bool* const __restrict pC,
                                  Vec4Bool* const __restrict pD );

/// Make Vec16Bool from 2 Vec8Bool's
NOALIAS __forceinline const Vec16Bool MakeVec16Bool( const Vec8Bool PCREF a, const Vec8Bool PCREF b );

/// Split Vec16Bool into 2 Vec8Bool's
NOALIAS __forceinline void Split( const Vec16Bool PCREF src,
                                  Vec8Bool* const __restrict pA,
                                  Vec8Bool* const __restrict pB );

/// Vec16Bool accessor: Get element
template < int I >
NOALIAS __forceinline const VBool Element( const Vec16Bool PCREF v );

/// Vec16Bool accessor: Set element
template < int I >
NOALIAS __forceinline void SetElement( Vec16Bool* const __restrict pDst, const VBool PCREF src );


/// Vec16Bool negation operator
NOALIAS __forceinline const Vec16Bool operator ~ ( const Vec16Bool PCREF x ) __attribute__(( /* pure, */ always_inline ));

/// Vec16Bool logical and operator
NOALIAS __forceinline const Vec16Bool operator & ( const Vec16Bool PCREF x, const Vec16Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec16Bool logical or operator
NOALIAS __forceinline const Vec16Bool operator | ( const Vec16Bool PCREF x, const Vec16Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec16Bool logical xor operator
NOALIAS __forceinline const Vec16Bool operator ^ ( const Vec16Bool PCREF x, const Vec16Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec16Bool equal operator
NOALIAS __forceinline const VBool operator == ( const Vec16Bool PCREF x, const Vec16Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec16Bool not-equal operator
NOALIAS __forceinline const VBool operator != ( const Vec16Bool PCREF x, const Vec16Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec16Bool logical and assignment operator
NOALIAS __forceinline void operator &= ( Vec16Bool& x, const Vec16Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec16Bool logical or assignment operator
NOALIAS __forceinline void operator |= ( Vec16Bool& x, const Vec16Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));
    


/// Vec16Bool & VBool logical and operator
NOALIAS __forceinline const Vec16Bool operator & ( const Vec16Bool PCREF x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool & Vec16Bool logical and operator
NOALIAS __forceinline const Vec16Bool operator & ( const VBool PCREF x, const Vec16Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec16Bool & VBool logical or operator
NOALIAS __forceinline const Vec16Bool operator | ( const Vec16Bool PCREF x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool & Vec16Bool logical or operator
NOALIAS __forceinline const Vec16Bool operator | ( const VBool PCREF x, const Vec16Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec16Bool & VBool logical xor operator
NOALIAS __forceinline const Vec16Bool operator ^ ( const Vec16Bool PCREF x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool & Vec16Bool logical xor operator
NOALIAS __forceinline const Vec16Bool operator ^ ( const VBool PCREF x, const Vec16Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec16Bool & VBool logical and assignment operator
NOALIAS __forceinline void operator &= ( Vec16Bool& x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec16Bool & VBool logical or assignment operator
NOALIAS __forceinline void operator |= ( Vec16Bool& x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


