#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VEC4BOOL_H
#define SOFT_MATHS_HVEC_NEW_VEC4BOOL_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VEC4BOOL_H
#define MATHS_HVEC_NEW_VEC4BOOL_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Boolean
/// @{

/// Make Vec4Bool from a set of 4 template bools ( compile time constants )
template < bool x, bool y, bool z, bool w >
NOALIAS __forceinline const Vec4Bool MakeVec4Bool();

/// Make Vec4Bool from a set of 4 bools ( runtime variables )
NOALIAS __forceinline const Vec4Bool MakeVec4Bool( const bool x, const bool y, const bool z, const bool w );

/// Make Vec4Bool from a set of 4 VBool's
NOALIAS __forceinline const Vec4Bool MakeVec4Bool( const VBool PCREF x, const VBool PCREF y, const VBool PCREF z, const VBool PCREF w );


/// Vec4Bool accessor: Get X element
NOALIAS __forceinline const VBool X( const Vec4Bool PCREF v );

/// Vec4Bool accessor: Get Y element
NOALIAS __forceinline const VBool Y( const Vec4Bool PCREF v );

/// Vec4Bool accessor: Get Z element
NOALIAS __forceinline const VBool Z( const Vec4Bool PCREF v );

/// Vec4Bool accessor: Get W element
NOALIAS __forceinline const VBool W( const Vec4Bool PCREF v );


/// Vec4Bool accessor: Set X element
NOALIAS __forceinline void SetX( Vec4Bool* const __restrict pDst, const VBool PCREF x );

/// Vec4Bool accessor: Set Y element
NOALIAS __forceinline void SetY( Vec4Bool* const __restrict pDst, const VBool PCREF y );

/// Vec4Bool accessor: Set Z element
NOALIAS __forceinline void SetZ( Vec4Bool* const __restrict pDst, const VBool PCREF z );

/// Vec4Bool accessor: Set W element
NOALIAS __forceinline void SetW( Vec4Bool* const __restrict pDst, const VBool PCREF w );



/// Vec4Bool negation operator
NOALIAS __forceinline const Vec4Bool operator ~ ( const Vec4Bool PCREF x ) __attribute__(( /* pure, */ always_inline ));

/// Vec4Bool logical and operator
NOALIAS __forceinline const Vec4Bool operator & ( const Vec4Bool PCREF x, const Vec4Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec4Bool logical or operator
NOALIAS __forceinline const Vec4Bool operator | ( const Vec4Bool PCREF x, const Vec4Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec4Bool logical xor operator
NOALIAS __forceinline const Vec4Bool operator ^ ( const Vec4Bool PCREF x, const Vec4Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec4Bool equal operator
NOALIAS __forceinline const VBool operator == ( const Vec4Bool PCREF x, const Vec4Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec4Bool not-equal operator
NOALIAS __forceinline const VBool operator != ( const Vec4Bool PCREF x, const Vec4Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec4Bool logical and assignment operator
NOALIAS __forceinline void operator &= ( Vec4Bool& x, const Vec4Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec4Bool logical or assignment operator
NOALIAS __forceinline void operator |= ( Vec4Bool& x, const Vec4Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));
    


/// Vec4Bool & VBool logical and operator
NOALIAS __forceinline const Vec4Bool operator & ( const Vec4Bool PCREF x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool & Vec4Bool logical and operator
NOALIAS __forceinline const Vec4Bool operator & ( const VBool PCREF x, const Vec4Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec4Bool & VBool logical or operator
NOALIAS __forceinline const Vec4Bool operator | ( const Vec4Bool PCREF x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool & Vec4Bool logical or operator
NOALIAS __forceinline const Vec4Bool operator | ( const VBool PCREF x, const Vec4Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec4Bool & VBool logical xor operator
NOALIAS __forceinline const Vec4Bool operator ^ ( const Vec4Bool PCREF x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool & Vec4Bool logical xor operator
NOALIAS __forceinline const Vec4Bool operator ^ ( const VBool PCREF x, const Vec4Bool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec4Bool & VBool logical and assignment operator
NOALIAS __forceinline void operator &= ( Vec4Bool& x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// Vec4Bool & VBool logical or assignment operator
NOALIAS __forceinline void operator |= ( Vec4Bool& x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));


/// Vec4Bool Bitwise NOT: ~a
NOALIAS __forceinline const Vec4Bool NOT( const Vec4Bool PCREF a );

/// Vec4Bool Bitwise OR: a | b
NOALIAS __forceinline const Vec4Bool OR( const Vec4Bool PCREF a, const Vec4Bool PCREF b );

/// Vec4Bool Bitwise AND: a & b
NOALIAS __forceinline const Vec4Bool AND( const Vec4Bool PCREF a, const Vec4Bool PCREF b );

/// Vec4Bool Bitwise XOR: a ^ b : a != b : (a & ~b) | (~a & b)
NOALIAS __forceinline const Vec4Bool XOR( const Vec4Bool PCREF a, const Vec4Bool PCREF b );

/// Vec4Bool Bitwise NOR: ~(a | b)
NOALIAS __forceinline const Vec4Bool NOR( const Vec4Bool PCREF a, const Vec4Bool PCREF b );

/// Vec4Bool Bitwise NAND: ~(a & b)
NOALIAS __forceinline const Vec4Bool NAND( const Vec4Bool PCREF a, const Vec4Bool PCREF b );

/// Vec4Bool Bitwise EQV: ~(a ^ b) : a == b : (a & b) | (~a & ~b)
NOALIAS __forceinline const Vec4Bool EQV( const Vec4Bool PCREF a, const Vec4Bool PCREF b );

/// Vec4Bool Bitwise ORC: a | ~b
NOALIAS __forceinline const Vec4Bool ORC( const Vec4Bool PCREF a, const Vec4Bool PCREF b );

/// Vec4Bool Bitwise ANDC: a & ~b
NOALIAS __forceinline const Vec4Bool ANDC( const Vec4Bool PCREF a, const Vec4Bool PCREF b );


/// Vec4Bool Horizontal AND 3 element
NOALIAS __forceinline const VBool HorizontalAND3( const Vec4Bool PCREF v );

/// Vec4Bool Horizontal AND 4 element
NOALIAS __forceinline const VBool HorizontalAND4( const Vec4Bool PCREF v );


/// Vec4Bool Horizontal OR 3 element
NOALIAS __forceinline const VBool HorizontalOR3( const Vec4Bool PCREF v );
    
/// Vec4Bool Horizontal OR 4 element
NOALIAS __forceinline const VBool HorizontalOR4( const Vec4Bool PCREF v );


/// Vec4Bool All True 3 element
NOALIAS __forceinline const VBool AllTrue3( const Vec4Bool PCREF v );

/// Vec4Bool All False 3 element
NOALIAS __forceinline const VBool AllFalse3( const Vec4Bool PCREF v );

/// Vec4Bool Any True 3 element
NOALIAS __forceinline const VBool AnyTrue3( const Vec4Bool PCREF v );

/// Vec4Bool Any False 3 element
NOALIAS __forceinline const VBool AnyFalse3( const Vec4Bool PCREF v );


/// Vec4Bool All True 4 element
NOALIAS __forceinline const VBool AllTrue4( const Vec4Bool PCREF v );

/// Vec4Bool All False 4 element
NOALIAS __forceinline const VBool AllFalse4( const Vec4Bool PCREF v );

/// Vec4Bool Any True 4 element
NOALIAS __forceinline const VBool AnyTrue4( const Vec4Bool PCREF v );

/// Vec4Bool Any False 4 element
NOALIAS __forceinline const VBool AnyFalse4( const Vec4Bool PCREF v );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


