#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VBOOL_H
#define SOFT_MATHS_HVEC_NEW_VBOOL_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VBOOL_H
#define MATHS_HVEC_NEW_VBOOL_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Boolean
/// @{

/// Make VBool from a template bool ( compile time constant )
template < bool x >
NOALIAS __forceinline const VBool MakeVBool();

/// Make VBool from a bool ( runtime variable )
NOALIAS __forceinline const VBool MakeVBool( const bool x );


/// VBool negation operator
NOALIAS __forceinline const VBool operator ~ ( const VBool PCREF x ) __attribute__(( /* pure, */ always_inline ));

/// VBool logical and operator
NOALIAS __forceinline const VBool operator & ( const VBool PCREF x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool logical or operator
NOALIAS __forceinline const VBool operator | ( const VBool PCREF x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool logical xor operator
NOALIAS __forceinline const VBool operator ^ ( const VBool PCREF x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool equal operator
NOALIAS __forceinline const VBool operator == ( const VBool PCREF x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool not-equal operator
NOALIAS __forceinline const VBool operator != ( const VBool PCREF x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool logical and assignment operator
NOALIAS __forceinline void operator &= ( VBool& x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));

/// VBool logical or assignment operator
NOALIAS __forceinline void operator |= ( VBool& x, const VBool PCREF y ) __attribute__(( /* pure, */ always_inline ));
    

/// VBool Bitwise NOT: ~a
NOALIAS __forceinline const VBool NOT( const VBool PCREF a );

/// VBool Bitwise OR: a | b
NOALIAS __forceinline const VBool OR( const VBool PCREF a, const VBool PCREF b );

/// VBool Bitwise AND: a & b
NOALIAS __forceinline const VBool AND( const VBool PCREF a, const VBool PCREF b );

/// VBool Bitwise XOR: a ^ b : a != b : (a & ~b) | (~a & b)
NOALIAS __forceinline const VBool XOR( const VBool PCREF a, const VBool PCREF b );

/// VBool Bitwise NOR: ~(a | b)
NOALIAS __forceinline const VBool NOR( const VBool PCREF a, const VBool PCREF b );

/// VBool Bitwise NAND: ~(a & b)
NOALIAS __forceinline const VBool NAND( const VBool PCREF a, const VBool PCREF b );

/// VBool Bitwise EQV: ~(a ^ b) : a == b : (a & b) | (~a & ~b)
NOALIAS __forceinline const VBool EQV( const VBool PCREF a, const VBool PCREF b );

/// VBool Bitwise ORC: a | ~b
NOALIAS __forceinline const VBool ORC( const VBool PCREF a, const VBool PCREF b );

/// VBool Bitwise ANDC: a & ~b
NOALIAS __forceinline const VBool ANDC( const VBool PCREF a, const VBool PCREF b );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


