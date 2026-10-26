#ifndef MATHS_HVEC_SCALAR_H
#define MATHS_HVEC_SCALAR_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_PS3
#include "SIMD.h"
#endif

namespace Maths
{
class HVecX;
class HVecY;
class HVecZ;
class HVecW;

class HVec2;
class HVec3;
class HVec4;

class C_Vertex;
class C_Vector;
class C_Normal;

class HVec3_Multiply;
class HVec3_Add;
class HVec3_Subtract;

//class C_Vertex_Multiply;
//class C_Vertex_Add;
class C_Vertex_Subtract;

//class C_Vector_Multiply;
class C_Vector_Add;
class C_Vector_Subtract;

class HMat33;
class HMat43;
class HMat44;

class C_Quaternion;
class C_Normalised;

template < typename T > class C_SiScalar;
template < typename T > class C_SiVector;
template < typename T > class C_SiVertex;
template < typename T > class C_SiQuaternion;
template < typename T, bool OrthoNormalised > class C_SiRotMatrix;

///
/// Hardware Scalar F32 Class
/// Single float stored in X component of hardware 4 element vector register.
/// Y, Z & W components are set to zero.
///

class ALIGN16 PASSINREG DARWIN64_ABI HF32
{
public:
    
    /// VF32 Constructor & Cast
    NOALIAS __forceinline HF32( const VF32 VECREF v ) : m_F( v.m_V ) {}
    NOALIAS __forceinline operator const VF32() const __restrict { VF32 result; result.m_V = m_F; return result; }
    
    /// Default Constructor
    NOALIAS __forceinline HF32();
    
	/// Zero Init Constructor
	NOALIAS __forceinline HF32( const _ZERO_INIT zeroinit );

    /// Undefined Constructor
    NOALIAS __forceinline HF32( const _UNDEFINED undefined );
    
    /// Copy Constructor
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline HF32( const HF32& copy );
    #endif
    
    /// F32 Constructor
    NOALIAS __forceinline HF32( const F32& f );
    
    /// F64 Constructor
    NOALIAS __forceinline HF32( const F64& f );

    /// S32 Constructor
    NOALIAS __forceinline HF32( const S32& i );
    
    /// U32 Constructor
    NOALIAS __forceinline HF32( const U32& i );
    
    /// F32 Ptr Constructor
    NOALIAS __forceinline HF32( F32 const * ptr );
    
    /// Selector Constructor: Selects fields from the inputs based on the selector mask: result = ( selector ? a : b )
    SYS_DEPRECATED NOALIAS __forceinline HF32( const Scalar_Bool PCREF selector, const HF32 PCREF a, const HF32 PCREF b );
    
    /// V128 Constructor: Explicit construction from unknown data!
    NOALIAS __forceinline explicit HF32( const V128 PCREF src );
    
    /// HVecX Constructor
    NOALIAS __forceinline explicit HF32( const HVecX PCREF x );
    
    /// HVecY Constructor
    NOALIAS __forceinline explicit HF32( const HVecY PCREF y );
    
    /// HVecZ Constructor
    NOALIAS __forceinline explicit HF32( const HVecZ PCREF z );
    
    /// HVecW Constructor
    NOALIAS __forceinline explicit HF32( const HVecW PCREF w );
    
    /// F32 Cast
    NOALIAS __forceinline operator const F32() const __restrict;
    
    /// Copy Assignment
    NOALIAS __forceinline const HF32&           operator = ( const HF32& copy );
    
    /// F32 Assignment
    NOALIAS __forceinline const HF32&           operator = ( const F32& f ) __restrict;
    
    /// S32 Assignment
    NOALIAS __forceinline const HF32&           operator = ( const S32& i ) __restrict;
    
    /// U32 Assignment
    NOALIAS __forceinline const HF32&           operator = ( const U32& i ) __restrict;
    
    /// S32 Conversion
    NOALIAS __forceinline const S32             F2I() const __restrict;
    
    /// Abs
    NOALIAS __forceinline const HF32            Abs() const __restrict;
    
    /// Sign : Returns -1.0f if scalar < 0.0f, +1.0f if scalar > 0.0f, or 0.0f if scalar == 0.0f 
    NOALIAS __forceinline const HF32            Sign() const __restrict;
    
    /// Square Root
    NOALIAS __forceinline const HF32            Sqrt() const __restrict;
    
    /// Reciprocal
    NOALIAS __forceinline const HF32            Rcp() const __restrict;
    
    /// Reciprocal
    NOALIAS __forceinline const HF32            Reciprocal() const __restrict;
    
    /// Reciprocal Square Root
    NOALIAS __forceinline const HF32            RcpSqrt() const __restrict;
    
    /// Reciprocal Square Root
    NOALIAS __forceinline const HF32            ReciprocalSqrt() const __restrict;
    
    /// Unary Positive Operator ( yes, this is needed, otherwise the compiler will do very stupid things... )
    NOALIAS __forceinline const HF32            operator + () const __restrict;
    
    /// Unary Negation Operator
    NOALIAS __forceinline const HF32            operator - () const __restrict;
    
    /// Binary Addition Operator
    NOALIAS __forceinline const HF32            operator + ( const HF32 PCREF src ) const __restrict;
    
    /// Binary Subtraction Operator
    NOALIAS __forceinline const HF32            operator - ( const HF32 PCREF src ) const __restrict;
    
    /// Binary Multiplication Operator
    NOALIAS __forceinline const HF32            operator * ( const HF32 PCREF src ) const __restrict;
    
    /// Binary Division Operator
    NOALIAS __forceinline const HF32            operator / ( const HF32 PCREF src ) const __restrict;
    
    /// Add Update Operator
    NOALIAS __forceinline const HF32&           operator += ( const HF32 PCREF src ) __restrict;
    
    /// Subtract Update Operator
    NOALIAS __forceinline const HF32&           operator -= ( const HF32 PCREF src ) __restrict;
    
    /// Multiply Update Operator
    NOALIAS __forceinline const HF32&           operator *= ( const HF32 PCREF src ) __restrict;
    
    /// Divide Update Operator
    NOALIAS __forceinline const HF32&           operator /= ( const HF32 PCREF src ) __restrict;
    
    /// Min
    NOALIAS __forceinline const HF32            Min( const HF32 PCREF src ) const __restrict;
    
    /// Max
    NOALIAS __forceinline const HF32            Max( const HF32 PCREF src ) const __restrict;
    
    /// Min Update
    NOALIAS __forceinline const HF32&           MinEq( const HF32 PCREF src ) __restrict;
    
    /// Max Update
    NOALIAS __forceinline const HF32&           MaxEq( const HF32 PCREF src ) __restrict;
    
    /// Clamp to range
    NOALIAS __forceinline const HF32            Clamp( const HF32 PCREF lo, const HF32 PCREF hi ) const __restrict;
    
    /// Comparison operator ==
    NOALIAS __forceinline const Scalar_Bool     operator == ( const HF32 PCREF src ) const __restrict;
    
    /// Comparison operator !=
    NOALIAS __forceinline const Scalar_Bool     operator != ( const HF32 PCREF src ) const __restrict;
    
    /// Comparison operator <=
    NOALIAS __forceinline const Scalar_Bool     operator <= ( const HF32 PCREF src ) const __restrict;
    
    /// Comparison operator <
    NOALIAS __forceinline const Scalar_Bool     operator < ( const HF32 PCREF src ) const __restrict;
    
    /// Comparison operator >=
    NOALIAS __forceinline const Scalar_Bool     operator >= ( const HF32 PCREF src ) const __restrict;
    
    /// Comparison operator >
    NOALIAS __forceinline const Scalar_Bool     operator > ( const HF32 PCREF src ) const __restrict;
    
    /// Generate 4 element bit mask for this == src
    NOALIAS __forceinline const Scalar_Bool     EQ( const HF32 PCREF src ) const __restrict;
    
    /// Generate 4 element bit mask for this != src
    NOALIAS __forceinline const Scalar_Bool     NE( const HF32 PCREF src ) const __restrict;
    
    /// Generate 4 element bit mask for this <= src
    NOALIAS __forceinline const Scalar_Bool     LE( const HF32 PCREF src ) const __restrict;
    
    /// Generate 4 element bit mask for this < src
    NOALIAS __forceinline const Scalar_Bool     LT( const HF32 PCREF src ) const __restrict;
    
    /// Generate 4 element bit mask for this >= src
    NOALIAS __forceinline const Scalar_Bool     GE( const HF32 PCREF src ) const __restrict;
    
    /// Generate 4 element bit mask for this > src
    NOALIAS __forceinline const Scalar_Bool     GT( const HF32 PCREF src ) const __restrict;
    
    //
    // Promote Non-HF32 RHS Operands to HF32
    //
    
    NOALIAS __forceinline const HF32            operator + ( const F32& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator - ( const F32& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator * ( const F32& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator / ( const F32& src ) const __restrict;
    
    NOALIAS __forceinline const HF32            operator + ( const U32& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator - ( const U32& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator * ( const U32& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator / ( const U32& src ) const __restrict;
    
    NOALIAS __forceinline const HF32            operator + ( const S32& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator - ( const S32& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator * ( const S32& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator / ( const S32& src ) const __restrict;
    
    NOALIAS __forceinline const HF32            operator + ( const U16& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator - ( const U16& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator * ( const U16& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator / ( const U16& src ) const __restrict;
    
    NOALIAS __forceinline const HF32            operator + ( const S16& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator - ( const S16& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator * ( const S16& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator / ( const S16& src ) const __restrict;
    
    NOALIAS __forceinline const HF32            operator + ( const U8& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator - ( const U8& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator * ( const U8& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator / ( const U8& src ) const __restrict;
    
    NOALIAS __forceinline const HF32            operator + ( const S8& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator - ( const S8& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator * ( const S8& src ) const __restrict;
    NOALIAS __forceinline const HF32            operator / ( const S8& src ) const __restrict;
    
    
    NOALIAS __forceinline const HF32&           operator += ( const F32& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator -= ( const F32& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator *= ( const F32& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator /= ( const F32& src ) __restrict;
    
    NOALIAS __forceinline const HF32&           operator += ( const S32& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator -= ( const S32& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator *= ( const S32& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator /= ( const S32& src ) __restrict;
    
    NOALIAS __forceinline const HF32&           operator += ( const U32& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator -= ( const U32& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator *= ( const U32& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator /= ( const U32& src ) __restrict;
    
    NOALIAS __forceinline const HF32&           operator += ( const S16& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator -= ( const S16& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator *= ( const S16& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator /= ( const S16& src ) __restrict;
    
    NOALIAS __forceinline const HF32&           operator += ( const U16& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator -= ( const U16& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator *= ( const U16& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator /= ( const U16& src ) __restrict;
    
    NOALIAS __forceinline const HF32&           operator += ( const S8& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator -= ( const S8& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator *= ( const S8& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator /= ( const S8& src ) __restrict;
    
    NOALIAS __forceinline const HF32&           operator += ( const U8& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator -= ( const U8& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator *= ( const U8& src ) __restrict;
    NOALIAS __forceinline const HF32&           operator /= ( const U8& src ) __restrict;
    
    
    NOALIAS __forceinline const HF32            Min( const F32& src ) const __restrict;
    NOALIAS __forceinline const HF32            Max( const F32& src ) const __restrict;
    
    NOALIAS __forceinline const HF32&           MinEq( const F32& src ) __restrict;
    NOALIAS __forceinline const HF32&           MaxEq( const F32& src ) __restrict;
    
    
    NOALIAS __forceinline const S32             operator == ( const F32& src ) const __restrict;
    NOALIAS __forceinline const S32             operator != ( const F32& src ) const __restrict;
    NOALIAS __forceinline const S32             operator <= ( const F32& src ) const __restrict;
    NOALIAS __forceinline const S32             operator <  ( const F32& src ) const __restrict;
    NOALIAS __forceinline const S32             operator >= ( const F32& src ) const __restrict;
    NOALIAS __forceinline const S32             operator >  ( const F32& src ) const __restrict;
    
    /// Finite Test
    NOALIAS __forceinline const Scalar_Bool     Finite() const __restrict;
    
    /// Debug validation
    NOALIAS __forceinline void                  DebugValidate() const __restrict;
    
    /// Read from Stream
    template < typename T_Stream >
    NOALIAS __forceinline void                  Deserialise( T_Stream& stream );
    
    /// Write to Stream
    template < typename T_Stream >
    NOALIAS __forceinline void                  Serialise( T_Stream& stream );
	
	#if defined( _SIMD_ )
    NOALIAS __forceinline const V128            GetV128() const             { return m_F; }
	NOALIAS __forceinline void			        SetV128( const V128 src )   { m_F = src; }
	#else
	NOALIAS __forceinline const V128            GetV128() const             { V128 v; v.m_X = v.m_Y = v.m_Z = v.m_W = m_F; return v;  }
	NOALIAS __forceinline void			        SetV128( const V128 src )   { m_F = src.m_X; }
	#endif
    
//private:
public: // For __declspec( passinreg )
    
    #if defined( _SIMD_ )
    V128                    m_F;
    #else
    F32                     m_F;
    #endif
};


//
// Promote non-HF32 LHS operands to HF32 when in binary ops with HF32 RHS
//

NOALIAS __forceinline const HF32 operator + ( const F32& lhs, const HF32 PCREF rhs );
NOALIAS __forceinline const HF32 operator - ( const F32& lhs, const HF32 PCREF rhs );
NOALIAS __forceinline const HF32 operator * ( const F32& lhs, const HF32 PCREF rhs );
NOALIAS __forceinline const HF32 operator / ( const F32& lhs, const HF32 PCREF rhs );

NOALIAS __forceinline const HF32 operator + ( const S32& lhs, const HF32 PCREF rhs );
NOALIAS __forceinline const HF32 operator - ( const S32& lhs, const HF32 PCREF rhs );
NOALIAS __forceinline const HF32 operator * ( const S32& lhs, const HF32 PCREF rhs );
NOALIAS __forceinline const HF32 operator / ( const S32& lhs, const HF32 PCREF rhs );

NOALIAS __forceinline const HF32 operator + ( const U32& lhs, const HF32 PCREF rhs );
NOALIAS __forceinline const HF32 operator - ( const U32& lhs, const HF32 PCREF rhs );
NOALIAS __forceinline const HF32 operator * ( const U32& lhs, const HF32 PCREF rhs );
NOALIAS __forceinline const HF32 operator / ( const U32& lhs, const HF32 PCREF rhs );

//
// Update operators for non-HF32 LHS operands taking HF32 RHS operands
//

NOALIAS __forceinline void operator += ( F32& lhs, const HF32 PCREF rhs );
NOALIAS __forceinline void operator -= ( F32& lhs, const HF32 PCREF rhs );
NOALIAS __forceinline void operator *= ( F32& lhs, const HF32 PCREF rhs );
NOALIAS __forceinline void operator /= ( F32& lhs, const HF32 PCREF rhs );

//
// Non-Member Support functions
//

#undef min
#undef max

NOALIAS __forceinline const HF32 Min( const HF32 PCREF a, const HF32 PCREF b );
NOALIAS __forceinline const HF32 Min( const F32& a, const HF32 PCREF b );
NOALIAS __forceinline const HF32 Min( const HF32 PCREF a, const F32& b );

NOALIAS __forceinline const HF32 Max( const HF32 PCREF a, const HF32 PCREF b );
NOALIAS __forceinline const HF32 Max( const F32& a, const HF32 PCREF b );
NOALIAS __forceinline const HF32 Max( const HF32 PCREF a, const F32& b );

/// MulAdd: a * b + c
NOALIAS __forceinline const HF32 MulAdd( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF c );

/// NegMulSub: -( a * b - c ) == c - a * b
NOALIAS __forceinline const HF32 NegMulSub( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF c );

NOALIAS __forceinline const HF32 Abs( const HF32 PCREF src );
NOALIAS __forceinline const HF32 Sqrt( const HF32 PCREF src );
NOALIAS __forceinline const HF32 Rcp( const HF32 PCREF src );
NOALIAS __forceinline const HF32 Reciprocal( const HF32 PCREF src );
NOALIAS __forceinline const HF32 RcpSqrt( const HF32 PCREF src );
NOALIAS __forceinline const HF32 ReciprocalSqrt( const HF32 PCREF src );

//#if 0
//NOALIAS __forceinline const HF32 Exp( const HF32 PCREF x );
//NOALIAS __forceinline const HF32 Log( const HF32 PCREF x );
//NOALIAS __forceinline const HF32 Pow( const HF32 PCREF x, const HF32 PCREF y );
//#endif

NOALIAS __forceinline const S32         IsNonZero( const HF32 PCREF src );
NOALIAS __forceinline const S32         IsZero( const HF32 PCREF src );
NOALIAS __forceinline const Scalar_Bool Finite( const HF32 PCREF src );


/// Select if a == b
NOALIAS __forceinline const HF32 Select_EQ( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF result_if_true, const HF32 PCREF result_if_false );

/// Select if a != b
NOALIAS __forceinline const HF32 Select_NE( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF result_if_true, const HF32 PCREF result_if_false );

/// Select if a >= b
NOALIAS __forceinline const HF32 Select_GE( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF result_if_true, const HF32 PCREF result_if_false );

/// Select if a < b
NOALIAS __forceinline const HF32 Select_LT( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF result_if_true, const HF32 PCREF result_if_false );

/// Select if a <= b
NOALIAS __forceinline const HF32 Select_LE( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF result_if_true, const HF32 PCREF result_if_false );

/// Select if a > b
NOALIAS __forceinline const HF32 Select_GT( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF result_if_true, const HF32 PCREF result_if_false );


/// Returns a HF32 with each element having it's magnitude taken from the respective elements in 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const HF32 CopySign( const HF32 PCREF magnitude, const HF32 PCREF sign );


/// Round to integer towards minus infinity
NOALIAS __forceinline const HF32 RoundToIntMinusInf( const HF32 PCREF src ) __attribute__(( always_inline ));

/// Round to integer towards plus infinity
NOALIAS __forceinline const HF32 RoundToIntPlusInf( const HF32 PCREF src ) __attribute__(( always_inline ));

/// Round to integer towards nearest
NOALIAS __forceinline const HF32 RoundToIntNearest( const HF32 PCREF src ) __attribute__(( always_inline ));

/// Round to integer towards zero
NOALIAS __forceinline const HF32 RoundToIntZero( const HF32 PCREF src ) __attribute__(( always_inline ));

NOALIAS __forceinline const HF32 Floor( const HF32 PCREF f ) __attribute__(( always_inline ));
NOALIAS __forceinline const HF32 Ceil ( const HF32 PCREF f ) __attribute__(( always_inline ));
NOALIAS __forceinline const HF32 Round( const HF32 PCREF f ) __attribute__(( always_inline ));
NOALIAS __forceinline const HF32 Trunc( const HF32 PCREF f ) __attribute__(( always_inline ));

/// Generate 4 element bit mask for a == b
NOALIAS __forceinline const Scalar_Bool EQ( const HF32 PCREF a, const HF32 PCREF b ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_Bool EQ( const HF32 PCREF a, const HF32 PCREF b ) { return a.EQ( b ); }

/// Generate 4 element bit mask for a != b
NOALIAS __forceinline const Scalar_Bool NE( const HF32 PCREF a, const HF32 PCREF b ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_Bool NE( const HF32 PCREF a, const HF32 PCREF b ) { return a.NE( b ); }

/// Generate 4 element bit mask for a <= b
NOALIAS __forceinline const Scalar_Bool LE( const HF32 PCREF a, const HF32 PCREF b ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_Bool LE( const HF32 PCREF a, const HF32 PCREF b ) { return a.LE( b ); }

/// Generate 4 element bit mask for a < b
NOALIAS __forceinline const Scalar_Bool LT( const HF32 PCREF a, const HF32 PCREF b ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_Bool LT( const HF32 PCREF a, const HF32 PCREF b ) { return a.LT( b ); }

/// Generate 4 element bit mask for a >= b
NOALIAS __forceinline const Scalar_Bool GE( const HF32 PCREF a, const HF32 PCREF b ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_Bool GE( const HF32 PCREF a, const HF32 PCREF b ) { return a.GE( b ); }

/// Generate 4 element bit mask for a > b
NOALIAS __forceinline const Scalar_Bool GT( const HF32 PCREF a, const HF32 PCREF b ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_Bool GT( const HF32 PCREF a, const HF32 PCREF b ) { return a.GT( b ); }



//
// Hardware 1 Element F32 Vector X Component Class
//

class HVecX
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline HVecX();

	/// Zero Init Constructor
	NOALIAS __forceinline HVecX( const _ZERO_INIT zeroinit );
    
    /// Undefined Constructor
    NOALIAS __forceinline HVecX( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline HVecX( const HVecX& copy );
    
    /// HF32 Constructor
    NOALIAS __forceinline HVecX( const HF32 PCREF x );
    
    /// HVecY Constructor
    NOALIAS __forceinline explicit HVecX( const HVecY PCREF src );
    
    /// HVecZ Constructor
    NOALIAS __forceinline explicit HVecX( const HVecZ PCREF src );
    
    /// HVecW Constructor
    NOALIAS __forceinline explicit HVecX( const HVecW PCREF src );
    
    /// HVec3 Constructor
    NOALIAS __forceinline explicit HVecX( const HVec2 PCREF src );

	/// HVec3 Constructor : PS3FIX
    NOALIAS __forceinline explicit HVecX( const HVec3 PCREF src );
    
    #if defined( _SIMD_ )
    /// V128 Constructor: Explicit construction from unknown data!
    NOALIAS __forceinline explicit HVecX( const V128 PCREF src );
    #else
    /// F32 Array Constructor: Explicit construction from unknown data!
    NOALIAS __forceinline explicit HVecX( const F32* ptr );
    #endif
    
    /// Copy Assignment
    NOALIAS __forceinline const HVecX&        operator = ( const HVecX& copy );
    
    /// HF32 Assignment
    NOALIAS __forceinline const HVecX&        operator = ( const HF32 PCREF x );
    
    /// Accessor: Set HF32
    NOALIAS __forceinline void                X( const HF32 PCREF scalar );
    
    /// Accessor: Get HF32
    NOALIAS __forceinline const HF32          X() const;
    
    /// Debug validation
    NOALIAS __forceinline void                DebugValidate() const;
    
public:
    
    #if defined( _SIMD_ )
    V128                m_X;
    #else
    F32                 m_X;
    #endif
};



//
// Hardware 1 Element F32 Vector Y Component Class
//

class HVecY
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline HVecY();
    
	/// Zero Init Constructor
	NOALIAS __forceinline HVecY( const _ZERO_INIT zeroinit );

    /// Undefined Constructor
    NOALIAS __forceinline HVecY( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline HVecY( const HVecY& copy );
    
    /// HVecX Constructor
    NOALIAS __forceinline explicit HVecY( const HVecX PCREF src );
    
    /// HVecZ Constructor
    NOALIAS __forceinline explicit HVecY( const HVecZ PCREF src );
    
    /// HVecW Constructor
    NOALIAS __forceinline explicit HVecY( const HVecW PCREF src );
    
    /// HVec2 Constructor
    NOALIAS __forceinline explicit HVecY( const HVec2 PCREF src );

    /// HVec3 Constructor
    NOALIAS __forceinline explicit HVecY( const HVec3 PCREF src );
    
    /// HF32 Constructor
    NOALIAS __forceinline HVecY( const HF32 PCREF y );
    
    #if defined( _SIMD_ )
    /// V128 Constructor: Explicit construction from unknown data!
    NOALIAS __forceinline explicit HVecY( const V128 PCREF src );
    #else
    /// F32 Array Constructor: Explicit construction from unknown data!
    NOALIAS __forceinline explicit HVecY( const F32* ptr );
    #endif
    
    /// Copy Assignment
    NOALIAS __forceinline const HVecY&        operator = ( const HVecY& copy );
    
    /// HF32 Assignment
    NOALIAS __forceinline const HVecY&        operator = ( const HF32 PCREF y );
    
    /// Accessor: Set HF32
    NOALIAS __forceinline void                Y( const HF32 PCREF scalar );
    
    /// Accessor: Get HF32
    NOALIAS __forceinline const HF32          Y() const;
    
    /// Debug validation
    NOALIAS __forceinline void                DebugValidate() const;
    
public:
    
    #if defined( _SIMD_ )
    V128                m_Y;
    #else
    F32                 m_Y;
    #endif
};



//
// Hardware 1 Element F32 Vector Z Component Class
//

class HVecZ
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline HVecZ();
    
	/// Zero Init Constructor
	NOALIAS __forceinline HVecZ( const _ZERO_INIT zeroinit );

    /// Undefined Constructor
    NOALIAS __forceinline HVecZ( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline HVecZ( const HVecZ& copy );
    
    /// HVecX Constructor
    NOALIAS __forceinline explicit HVecZ( const HVecX PCREF src );
    
    /// HVecY Constructor
    NOALIAS __forceinline explicit HVecZ( const HVecY PCREF src );
    
    /// HVecW Constructor
    NOALIAS __forceinline explicit HVecZ( const HVecW PCREF src );
    
    /// HVec3 Constructor
    NOALIAS __forceinline explicit HVecZ( const HVec3 PCREF src );
    
    /// HF32 Constructor
    NOALIAS __forceinline HVecZ( const HF32 PCREF z );
    
    #if defined( _SIMD_ )
    /// V128 Constructor: Explicit construction from unknown data!
    NOALIAS __forceinline explicit HVecZ( const V128 PCREF src );
    #else
    /// F32 Array Constructor: Explicit construction from unknown data!
    NOALIAS __forceinline explicit HVecZ( const F32* ptr );
    #endif
    
    /// Copy Assignment
    NOALIAS __forceinline const HVecZ&        operator = ( const HVecZ& copy );
    
    /// HF32 Assignment
    NOALIAS __forceinline const HVecZ&        operator = ( const HF32 PCREF z );
    
    /// Accessor: Set HF32
    NOALIAS __forceinline void                Z( const HF32 PCREF scalar );
    
    /// Accessor: Get HF32
    NOALIAS __forceinline const HF32          Z() const;
    
    /// Debug validation
    NOALIAS __forceinline void                DebugValidate() const;
    
public:
    
    #if defined( _SIMD_ )
    V128    m_Z;
    #else
    F32     m_Z;
    #endif
};



//
// Hardware 1 Element F32 Vector W Component Class
//

class HVecW
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline HVecW();
    
	/// Zero Init Constructor
	NOALIAS __forceinline HVecW( const _ZERO_INIT zeroinit );

    /// Undefined Constructor
    NOALIAS __forceinline HVecW( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline HVecW( const HVecW& copy );
    
    /// HVecX Constructor
    NOALIAS __forceinline explicit HVecW( const HVecX PCREF src );
    
    /// HVecY Constructor
    NOALIAS __forceinline explicit HVecW( const HVecY PCREF src );
    
    /// HVecZ Constructor
    NOALIAS __forceinline explicit HVecW( const HVecZ PCREF src );
    
    /// Explicit HVec4 constructor
    NOALIAS __forceinline explicit HVecW( const HVec4 PCREF src );
    
    /// HF32 Constructor
    NOALIAS __forceinline HVecW( const HF32 PCREF w );
    
    #if defined( _SIMD_ )
    /// V128 Constructor: Explicit construction from unknown data!
    NOALIAS __forceinline explicit HVecW( const V128 PCREF src );
    #else
    /// F32 Array Constructor: Explicit construction from unknown data!
    NOALIAS __forceinline explicit HVecW( const F32* ptr );
    #endif
    
    /// Copy Assignment
    NOALIAS __forceinline const HVecW&        operator = ( const HVecW& copy );
    
    /// HF32 Assignment
    NOALIAS __forceinline const HVecW&        operator = ( const HF32 PCREF w );
    
    /// Accessor: Set HF32
    NOALIAS __forceinline void                W( const HF32 PCREF scalar );
    
    /// Accessor: Get HF32
    NOALIAS __forceinline const HF32          W() const;
    
    /// Debug validation
    NOALIAS __forceinline void                DebugValidate() const;
    
public:
    
    #if defined( _SIMD_ )
    V128    m_W;
    #else
    F32     m_W;
    #endif
};



NOALIAS __forceinline const bool finite( const HF32 PCREF s )
{
    return _finite( (F32)s ) != 0;
}



}; // namespace Maths

#endif
