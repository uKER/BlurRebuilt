#ifndef MATHS_HVEC_VECTOR_H
#define MATHS_HVEC_VECTOR_H

#ifdef _MSC_VER
#pragma once
#endif

// 
// Hardware Vector Classes
// Designed for optimal hardware acceleration
// 

//
// Notes on implementation
//
// 1) Unions cannot be used due to the crippling impact they have on performance.
//    All compilers aggressively store back to memory after modification to a
//    union member. A union member will not be kept in a temporary register for
//    subsequent reuse, instead it will have to be reloaded - in case it is
//    modified via a different union member.
//
// 2) Arrays of elements cannot be used, the elements must be stored individually.
//    This is due to an issue with the MSVC compiler: on initialisation, only the
//    first array element will be initialised by the proper constructor. The rest
//    of the array elements are copied from the first in dword chunks. This causes
//    code bloat, load-hit-store stalls and forces the object into memory even if
//    it's a temporary and could have stayed in a register for it's entire lifetime.
//    In addition, arrays cannot be initialised in constructor initialisers.
//
// 3) There's more... 
//
//
//
//

//#include "SIMD.h"

namespace Maths
{

NOALIAS __forceinline HVec3 operator + ( const HVec3_Multiply& ab, const HVec3 PCREF c );
NOALIAS __forceinline HVec3 operator + ( const HVec3 PCREF a, const HVec3_Multiply& bc );
NOALIAS __forceinline HVec3 operator - ( const HVec3_Multiply& ab, const HVec3 PCREF c );
NOALIAS __forceinline HVec3 operator - ( const HVec3 PCREF a, const HVec3_Multiply& bc );

//
// ---------------------------------------------------
// |-- HVec2 : Hardware 2 Element F32 Vector Class --|
// ---------------------------------------------------
//

class ALIGN16 DARWIN64_ABI HVec2
{
public:
    
    /// Vec4F32 Constructor & Cast
    NOALIAS __forceinline explicit HVec2( const Vec4F32 VECREF v )
        #if defined( _SIMD_ )
        : m_V( v.m_V )
        #endif
    {
        #if !defined( _SIMD_ )
        m_V[0] = v.m_V[0];
        m_V[1] = v.m_V[1];
        #endif
    }
    
    NOALIAS __forceinline operator const Vec4F32() const __restrict
    {
        Vec4F32 result;
        #if defined( _SIMD_ )
        result.m_V = m_V;
        #else
        result.m_V[0] = m_V[0];
        result.m_V[1] = m_V[1];
        #endif
        return result;
    }
    
    /// Default Constructor
    NOALIAS __forceinline HVec2();
    
	/// Zero Init Constructor
	NOALIAS __forceinline HVec2( const _ZERO_INIT zeroinit );

    /// Undefined Constructor
    NOALIAS __forceinline HVec2( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline HVec2( const HVec2& copy );
    
    /// Explicit F32 Array Constructor
    NOALIAS __forceinline explicit HVec2( const F32* ptr );
    
    /// HF32 Pair Constructor
    NOALIAS __forceinline HVec2( const HF32 PCREF x, const HF32 PCREF y );
    
    /// Explicit HF32 Broadcast Constructor
    NOALIAS __forceinline explicit HVec2( const HF32 PCREF xy );
    
    /// HVecX Constructor
    NOALIAS __forceinline HVec2( const HVecX PCREF x );
    
    /// HVecY Constructor
    NOALIAS __forceinline HVec2( const HVecY PCREF y );
    
    /// HVec3 Constructor: Explicit construction due to data loss!
    NOALIAS __forceinline explicit HVec2( const HVec3 PCREF src );
    
    /// HVec4 Constructor: Explicit construction due to data loss!
    NOALIAS __forceinline explicit HVec2( const HVec4 PCREF src );
    
    /// C_Vertex Constructor: Explicit construction due to data loss!
    NOALIAS __forceinline explicit HVec2( const C_Vertex PCREF src );
    
    /// C_Vector Constructor: Explicit construction due to data loss!
    NOALIAS __forceinline explicit HVec2( const C_Vector PCREF src );
    
    /// C_Normal Constructor: Explicit construction due to data loss!
    NOALIAS __forceinline explicit HVec2( const C_Normal PCREF src );
    
    /// V128 Constructor: Explicit construction from unknown data!
    NOALIAS __forceinline explicit HVec2( const V128 PCREF src );
    
	 /// V128 Cast
    NOALIAS __forceinline operator const V128 () const __restrict;
    
    /// HVecX Cast
    NOALIAS __forceinline operator                    const HVecX () const;
    
    /// HVecY Cast
    NOALIAS __forceinline operator                    const HVecY () const;
    
    /// Copy Assignment
    NOALIAS __forceinline const HVec2&                operator = ( const HVec2& copy );
    
    /// HVecX Assignment
    NOALIAS __forceinline const HVec2&                operator = ( const HVecX PCREF x );
    
    /// HVecY Assignment
    NOALIAS __forceinline const HVec2&                operator = ( const HVecY PCREF y );
    
    /// Accessor: Get X
    NOALIAS __forceinline const HF32                  X() const;
    
    /// Accessor: Get Y
    NOALIAS __forceinline const HF32                  Y() const;
    
    /// Accessor: Get Z - Required for templated accessors - should never be used!
    NOALIAS __forceinline const HF32                  Z() const;
    
    /// Accessor: Get W - Required for templated accessors - should never be used!
    NOALIAS __forceinline const HF32                  W() const;
    
    /// Accessor: Set X
    NOALIAS __forceinline void                        X( const HF32 PCREF scalar );
    
    /// Accessor: Set Y
    NOALIAS __forceinline void                        Y( const HF32 PCREF scalar );
    
    /// Accessor: Set Z - Required for templated accessors - should never be used!
    NOALIAS __forceinline void                        Z( const HF32 PCREF scalar );
    
    /// Accessor: Set W - Required for templated accessors - should never be used!
    NOALIAS __forceinline void                        W( const HF32 PCREF scalar );
    
    /// Unary Negation Operator
    NOALIAS __forceinline const HVec2                 operator - () const;
    
    /// Binary Addition Operator
    NOALIAS __forceinline const HVec2                 operator + ( const HVec2 PCREF src ) const;
    
    /// Binary Subtraction Operator
    NOALIAS __forceinline const HVec2                 operator - ( const HVec2 PCREF src ) const;
    
    /// Binary Multiplication Operator
    NOALIAS __forceinline const HVec2                 operator * ( const HVec2 PCREF src ) const;
    
    /// Binary Division Operator
    NOALIAS __forceinline const HVec2                 operator / ( const HVec2 PCREF src ) const;
    
    /// Add Update Operator
    NOALIAS __forceinline void                        operator += ( const HVec2 PCREF src );
    
    /// Subtract Update Operator
    NOALIAS __forceinline void                        operator -= ( const HVec2 PCREF src );
    
    /// Multiply Update Operator
    NOALIAS __forceinline void                        operator *= ( const HVec2 PCREF src );
    
    /// Divide Update Operator
    NOALIAS __forceinline void                        operator /= ( const HVec2 PCREF src );
    
    /// HF32 Multiply Operator
    NOALIAS __forceinline const HVec2                 operator * ( const HF32 PCREF src ) const;
    
    /// HF32 Divide Operator
    NOALIAS __forceinline const HVec2                 operator / ( const HF32 PCREF src ) const;
    
    /// HF32 Multiply Update Operator
    NOALIAS __forceinline void                        operator *= ( const HF32 PCREF src );
    
    /// HF32 Divide Update Operator
    NOALIAS __forceinline void                        operator /= ( const HF32 PCREF src );
    
    /// Returns copy with negated Y component
    NOALIAS __forceinline const HVec2                 NegatedY() const;
    
    /// Min
    NOALIAS __forceinline const HVec2                 Min( const HVec2 PCREF src ) const;
    
    /// Max
    NOALIAS __forceinline const HVec2                 Max( const HVec2 PCREF src ) const;
    
    /// Min Update
    NOALIAS __forceinline void                        MinEq( const HVec2 PCREF src );
    
    /// Max Update
    NOALIAS __forceinline void                        MaxEq( const HVec2 PCREF src );
    
    /// Lerp
    NOALIAS __forceinline const HVec2                 Lerp( const HVec2 PCREF dst, const HF32 PCREF f ) const;
    
    /// Horizontal sum of all elements
    NOALIAS __forceinline const HF32                  Sum() const;
    
    /// Dot product
    NOALIAS __forceinline const HF32                  operator | ( const HVec2 PCREF rhs ) const;
    
    /// Cross product
    NOALIAS __forceinline const HF32                  operator ^ ( const HVec2 PCREF rhs ) const;
    
    /// Vector Length
    NOALIAS __forceinline const HF32                  Length() const;
    
    /// Vector Length Squared
    NOALIAS __forceinline const HF32                  Length2() const;
    
    /// Vector Length Squared
    NOALIAS __forceinline const HF32                  LengthSquared() const;
    
    /// Vector Reciprocal of Length
    NOALIAS __forceinline const HF32                  RcpLength() const;
    
    /// Vector Reciprocal of Length
    NOALIAS __forceinline const HF32                  ReciprocalLength() const;
    
    /// Vector Normalised
    NOALIAS __forceinline const HVec2                 Normalised() const;
    
    /// Vector Normalise
    NOALIAS __forceinline void                        Normalise();
    
    /// Vector Normalised - safe version with additional parameter to use for zero length vectors!
    NOALIAS __forceinline const HVec2                 SafeNormalised( const HVec2 PCREF zeroLengthResult = HVec2() ) const;
    
    /// Vector Normalise - safe version with additional parameter to use for zero length vectors!
    NOALIAS __forceinline void                        SafeNormalise( const HVec2 PCREF zeroLengthResult = HVec2() );
    
    /// Comparison Operator ==
    NOALIAS __forceinline const Vector2_Bool     operator == ( const HVec2 PCREF src ) const;
    
    /// Comparison Operator !=
    NOALIAS __forceinline const Vector2_Bool     operator != ( const HVec2 PCREF src ) const;
    
    /// Comparison Operator <=
    NOALIAS __forceinline const Vector2_Bool     operator <= ( const HVec2 PCREF src ) const;
    
    /// Comparison Operator <
    NOALIAS __forceinline const Vector2_Bool     operator <  ( const HVec2 PCREF src ) const;
    
    /// Comparison Operator >=
    NOALIAS __forceinline const Vector2_Bool     operator >= ( const HVec2 PCREF src ) const;
    
    /// Comparison Operator >
    NOALIAS __forceinline const Vector2_Bool     operator >  ( const HVec2 PCREF src ) const;
    
    /// Set this to 'resultIfTrue' if selector is true
    //DEPRECATED void             SetIf( const HVec2 resultIfTrue, const Vector2_Bool& selector );
    
    /// Return 'resultIfTrue' if selector is true, else return this
    //DEPRECATED const HVec2      ReplaceIf( const HVec2 resultIfTrue, const Vector2_Bool& selector ) const;
    
    /// Shuffle HVec2:XY into a HVec3:XZ with zero Y
    NOALIAS __forceinline const HVec3                 ShuffleXYtoXZ() const;
    
    /// Read from Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Serialise( T_Stream& stream ) __restrict;
    
    /// Array Index Operators
    #if defined( _SIMD_ )
    NOALIAS __forceinline const VectorAccessor<HVec2>         operator [] ( const U32 index );
    NOALIAS __forceinline const ConstVecAccessor<HVec2>       operator [] ( const U32 index ) const;
    #else
    NOALIAS __forceinline F32&                                operator [] ( const U32 index );
    NOALIAS __forceinline const F32                           operator [] ( const U32 index ) const;
    #endif
    
    #if defined( _SIMD_ )
    V128    m_V;
    #else
    F32     m_V[2];
    #endif
    
    static const U32    m_Elements = 2;
    typedef F32         T_Element;
    //typedef F32         m_DataType;
    //typedef HF32        m_CrossProductType;
};


NOALIAS __forceinline HVec2 operator * ( const HVec2 PCREF v, const F32& f );
NOALIAS __forceinline HVec2 operator / ( const HVec2 PCREF v, const F32& f );

NOALIAS __forceinline void operator *= ( HVec2& v, const F32& f );
NOALIAS __forceinline void operator /= ( HVec2& v, const F32& f );


template < int DividePower2 >
NOALIAS __forceinline const HVec2 UnpackSignedHalfWordXY( const U32* pXY );






//
// ---------------------------------------------------
// |-- HVec3 : Hardware 3 Element F32 Vector Class --|
// ---------------------------------------------------
//

//template < typename T > class HVec3Pad; // What was this added for? Can't find any references to it...

class ALIGN16 PASSINREG DARWIN64_ABI HVec3
{
public:
	//template < typename T > friend class HVec3Pad; // What was this added for? Can't find any references to it...
    
    /// Vec4F32 Constructor
    NOALIAS __forceinline explicit HVec3( const Vec4F32 VECREF v )
        #if defined( _SIMD_ )
        : m_V( v.m_V )
        #endif
    {
        #if !defined( _SIMD_ )
        m_V[0] = v.m_V[0];
        m_V[1] = v.m_V[1];
        m_V[2] = v.m_V[2];
        #endif
    }
    
    /// Vector3 Constructor
    NOALIAS __forceinline explicit HVec3( const Vector3 PCREF v )
        #if defined( _SIMD_ )
        : m_V( v.m_V.m_V )
        #endif
    {
        #if !defined( _SIMD_ )
        m_V[0] = v.m_V.m_V[0];
        m_V[1] = v.m_V.m_V[1];
        m_V[2] = v.m_V.m_V[2];
        #endif
    }
    
    /// Point3 Constructor
    NOALIAS __forceinline explicit HVec3( const Point3 PCREF v )
        #if defined( _SIMD_ )
        : m_V( v.m_V.m_V )
        #endif
    {
        #if !defined( _SIMD_ )
        m_V[0] = v.m_V.m_V[0];
        m_V[1] = v.m_V.m_V[1];
        m_V[2] = v.m_V.m_V[2];
        #endif
    }
    
    /// Normal3 Constructor
    NOALIAS __forceinline explicit HVec3( const Normal3 PCREF v )
        #if defined( _SIMD_ )
        : m_V( v.m_V.m_V )
        #endif
    {
        #if !defined( _SIMD_ )
        m_V[0] = v.m_V.m_V[0];
        m_V[1] = v.m_V.m_V[1];
        m_V[2] = v.m_V.m_V[2];
        #endif
    }
    
    /// Vec4F32 Cast
    NOALIAS __forceinline operator const Vec4F32() const __restrict
    {
        Vec4F32 result;
        #if defined( _SIMD_ )
        result.m_V = m_V;
        #else
        result.m_V[0] = m_V[0];
        result.m_V[1] = m_V[1];
        result.m_V[2] = m_V[2];
        #endif
        return result;
    }
    
    /// Vector3 Cast
    NOALIAS __forceinline operator const Vector3() const __restrict
    {
        Vector3 result;
        #if defined( _SIMD_ )
        result.m_V.m_V = m_V;
        #else
        result.m_V.m_V[0] = m_V[0];
        result.m_V.m_V[1] = m_V[1];
        result.m_V.m_V[2] = m_V[2];
        #endif
        return result;
    }
    
    /// Point3 Cast
    NOALIAS __forceinline operator const Point3() const __restrict
    {
        Point3 result;
        #if defined( _SIMD_ )
        result.m_V.m_V = m_V;
        #else
        result.m_V.m_V[0] = m_V[0];
        result.m_V.m_V[1] = m_V[1];
        result.m_V.m_V[2] = m_V[2];
        #endif
        return result;
    }
    
    /// Normal3 Cast
    NOALIAS __forceinline operator const Normal3() const __restrict
    {
        HVec3 normal( *this );
        normal.Normalise();
        Normal3 result;
        #if defined( _SIMD_ )
        result.m_V.m_V = normal.m_V;
        #else
        result.m_V.m_V[0] = normal.m_V[0];
        result.m_V.m_V[1] = normal.m_V[1];
        result.m_V.m_V[2] = normal.m_V[2];
        #endif
        return result;
    }
    
    /// Default Constructor
    NOALIAS __forceinline HVec3();
    
    /// Copy Constructor
    NOALIAS __forceinline HVec3( const HVec3& copy );
    
	/// Zero Init Constructor
	NOALIAS __forceinline HVec3( const _ZERO_INIT zeroinit );

    /// Undefined Constructor
    NOALIAS __forceinline HVec3( const _UNDEFINED undefined );
    
    /// Copy Constructor
    //NOALIAS __forceinline HVec3( const HVec3 copy );
    
    /// HVecX + HVecY + HVecZ Triple Constructor
    NOALIAS __forceinline HVec3( const HVecX PCREF x, const HVecY PCREF y, const HVecZ PCREF z );
    
    /// HVecX Constructor
    NOALIAS __forceinline HVec3( const HVecX PCREF src );
    
    /// HVecY Constructor
    NOALIAS __forceinline HVec3( const HVecY PCREF src );
    
    /// HVecZ Constructor
    NOALIAS __forceinline HVec3( const HVecZ PCREF src );
    
    /// HVecW Constructor : TODO!
    //NOALIAS __forceinline HVec3( const HVecW PCREF src );
    
    /// HVec2 Constructor
    NOALIAS __forceinline HVec3( const HVec2 PCREF src );
    
    /// HVec4 Constructor: Explicit construction due to data loss!
    NOALIAS __forceinline explicit HVec3( const HVec4 PCREF src );
    
    /// C_Vertex Constructor
    NOALIAS __forceinline HVec3( const C_Vertex PCREF src );
    
    /// C_Vector Constructor
    NOALIAS __forceinline HVec3( const C_Vector PCREF src );
    
    /// C_Normal Constructor
    NOALIAS __forceinline HVec3( const C_Normal PCREF src );
    
    /// Explicit HF32 Broadcast Constructor
    NOALIAS __forceinline explicit HVec3( const HF32 PCREF xyz );
    
    /// HF32 Triple Constructor
    NOALIAS __forceinline HVec3( const HF32 PCREF x, const HF32 PCREF y, const HF32 PCREF z );
    
    /// Explicit F32 Array Constructor
    NOALIAS __forceinline explicit HVec3( const F32* ptr );
    
    /// Explicit Constructor from Float3 ptr
    NOALIAS __forceinline explicit HVec3( MathsStorage::Float3 const * pStorage );
    
    /// Cast to Float3
    NOALIAS __forceinline operator const MathsStorage::Float3 () const __restrict;
    
    /// Explicit Constructor from U16 x, y, z
    NOALIAS __forceinline explicit HVec3( U16 const * const __restrict pXYZ );
    
    /// Explicit Constructor from Fixed3_S16 ptr
    template < int T_Precision >
    NOALIAS __forceinline explicit HVec3( MathsStorage::Fixed3_S16<T_Precision> const * pStorage );
    
    /// Cast to Fixed3_S16
    template < int T_Precision >
    NOALIAS __forceinline operator const MathsStorage::Fixed3_S16<T_Precision> () const __restrict;
    
    /// Selector Constructor: Copies selector masks
    NOALIAS __forceinline HVec3( const Vector3_Bool PCREF selector );
    
    /// Selector Constructor: Selects fields from the inputs based on the selector mask: result = ( selector ? a : b )
    NOALIAS __forceinline HVec3( const Vector3_Bool PCREF selector, const HVec3 PCREF a, const HVec3 PCREF b );
    
    /// HVec3_Multiply Constructor: Collapses the delayed multiply
    NOALIAS __forceinline HVec3( const HVec3_Multiply& src );
    
    /// HVec3_Add Constructor: Collapses the delayed add
    NOALIAS __forceinline HVec3( const HVec3_Add& src );

    /// HVec3_Subtract Constructor: Collapses the delayed subtract
    NOALIAS __forceinline HVec3( const HVec3_Subtract& src );

    /// Validate that the elements are all finite and that the W element is zero
    NOALIAS __forceinline void                  DebugValidate() const __restrict;
    
    /// HVecX Cast
    NOALIAS __forceinline operator              const HVecX () const __restrict;
    
    /// HVecY Cast
    NOALIAS __forceinline operator              const HVecY () const __restrict;
    
    /// HVecZ Cast
    NOALIAS __forceinline operator              const HVecZ () const __restrict;
    
    /// Copy Assignment
    NOALIAS __forceinline const HVec3&          operator = ( const HVec3& copy );
    
    /// HVecX Assignment
    NOALIAS __forceinline const HVec3&          operator = ( const HVecX PCREF src ) __restrict;
    
    /// HVecY Assignment
    NOALIAS __forceinline const HVec3&          operator = ( const HVecY PCREF src ) __restrict;
    
    /// HVecZ Assignment
    NOALIAS __forceinline const HVec3&          operator = ( const HVecZ PCREF src ) __restrict;
    
    /// HVec2 Assignment
    NOALIAS __forceinline const HVec3&          operator = ( const HVec2 PCREF src ) __restrict;
    
    /// C_Vector Assignment
    NOALIAS __forceinline const HVec3&          operator = ( const C_Vector PCREF src ) __restrict;
    
    /// C_Vertex Assignment
    NOALIAS __forceinline const HVec3&          operator = ( const C_Vertex PCREF src ) __restrict;
    
    /// C_Normal Assignment
    NOALIAS __forceinline const HVec3&          operator = ( const C_Normal PCREF src ) __restrict;
    
    /// Accessor: Get X
    NOALIAS __forceinline const HF32            X() const __restrict;
    
    /// Accessor: Get Y
    NOALIAS __forceinline const HF32            Y() const __restrict;
    
    /// Accessor: Get Z
    NOALIAS __forceinline const HF32            Z() const __restrict;
    
    /// Accessor: Get W - Required for templated accessors - should never be used!
    NOALIAS __forceinline const HF32            W() const __restrict;
    
    /// Accessor: Set X
    NOALIAS __forceinline void                  X( const HF32 PCREF scalar ) __restrict;
    
    /// Accessor: Set Y
    NOALIAS __forceinline void                  Y( const HF32 PCREF scalar ) __restrict;
    
    /// Accessor: Set Z
    NOALIAS __forceinline void                  Z( const HF32 PCREF scalar ) __restrict;
    
    /// Accessor: Set W - Required for templated accessors - should never be used!
    NOALIAS __forceinline void                  W( const HF32 PCREF scalar ) __restrict;
    
    //#if defined( _SIMD_ )
    ///// Accessor: Get W
    //NOALIAS const HF32                          TestW() const __restrict;
    //#endif
    
    /// Zeroes element X
    SYS_DEPRECATED NOALIAS __forceinline void   ZeroX() __restrict;
    
    /// Zeroes element Y
    SYS_DEPRECATED NOALIAS __forceinline void   ZeroY() __restrict;
    
    /// Zeroes element Z
    SYS_DEPRECATED NOALIAS __forceinline void   ZeroZ() __restrict;
    
    /// Zeroes elements XY
    SYS_DEPRECATED NOALIAS __forceinline void   ZeroXY() __restrict;
    
    /// Zeroes elements XZ
    SYS_DEPRECATED NOALIAS __forceinline void   ZeroXZ() __restrict;
    
    /// Zeroes elements YZ
    SYS_DEPRECATED NOALIAS __forceinline void   ZeroYZ() __restrict;
    
    /// Zero the vector if any element exceeds +/- bounds
    NOALIAS __forceinline void                  ZeroIfOutOfBounds( const Maths::HF32 PCREF bounds ) __restrict;
    
    /// Returns copy with zeroed element X
    SYS_DEPRECATED NOALIAS __forceinline const HVec3    ZeroedX() const __restrict;
    
    /// Returns copy with zeroed element Y
    SYS_DEPRECATED NOALIAS __forceinline const HVec3    ZeroedY() const __restrict;
    
    /// Returns copy with zeroed element Z
    SYS_DEPRECATED NOALIAS __forceinline const HVec3    ZeroedZ() const __restrict;
    
    /// Returns copy with zeroed elements XY
    SYS_DEPRECATED NOALIAS __forceinline const HVec3    ZeroedXY() const __restrict;
    
    /// Returns copy with zeroed elements XZ
    SYS_DEPRECATED NOALIAS __forceinline const HVec3    ZeroedXZ() const __restrict;
    
    /// Returns copy with zeroed elements YZ
    SYS_DEPRECATED NOALIAS __forceinline const HVec3    ZeroedYZ() const __restrict;
    
    /// Shuffle
    template < unsigned IX, unsigned IY, unsigned IZ >
    SYS_DEPRECATED NOALIAS __forceinline const HVec3    Shuffle() const __restrict;
    
    /// Unary Negation Operator
    //NOALIAS const HVec3                 operator - () const __restrict;
    
    /// Binary Addition Operator
    //NOALIAS const HVec3_Add             operator + ( const HVec3 src ) const __restrict;
    
    /// Binary Subtraction Operator
    //NOALIAS const HVec3_Subtract        operator - ( const HVec3 src ) const __restrict;
    
    /// Binary Multiplication Operator
    //NOALIAS const HVec3_Multiply        operator * ( const HVec3 src ) const __restrict;
    
    /// Binary Division Operator
    //NOALIAS const HVec3                 operator / ( const HVec3 src ) const __restrict;
    
    /// Add Update Operator
    //NOALIAS void                        operator += ( const HVec3 src ) __restrict;
    
    /// Subtract Update Operator
    //NOALIAS void                        operator -= ( const HVec3 src ) __restrict;
    
    /// Multiply Update Operator
    //NOALIAS void                        operator *= ( const HVec3 src ) __restrict;
    
    /// Divide Update Operator
    //NOALIAS void                        operator /= ( const HVec3 src ) __restrict;
    
    /// HF32 Multiplication Operator
    //const HVec3                         operator * ( const HF32 src ) const;
    //const HVec3_Multiply                operator * ( const HF32 src ) const;
    
    /// HF32 Division Operator
    //NOALIAS const HVec3_Multiply        operator / ( const HF32 src ) const __restrict;
    
    /// HF32 Multiply Update Operator
    //NOALIAS void                        operator *= ( const HF32 src ) __restrict;
    
    /// HF32 Divide Update Operator
    //NOALIAS void                        operator /= ( const HF32 src ) __restrict;
    
    /// HVecX Multiply Operator
    //NOALIAS const HVec3_Multiply        operator * ( const HVecX src ) const __restrict;
    
    /// HVecY Multiply Operator
    //NOALIAS const HVec3_Multiply        operator * ( const HVecY src ) const __restrict;
    
    /// HVecZ Multiply Operator
    //NOALIAS const HVec3_Multiply        operator * ( const HVecZ src ) const __restrict;
    
    /// HVecW Multiply Operator
    //NOALIAS const HVec3_Multiply        operator * ( const HVecW src ) const __restrict;
    
    /// HVecX Multiply Update Operator
    //NOALIAS void                        operator *= ( const HVecX src ) __restrict;
    
    /// Min
    SYS_DEPRECATED NOALIAS __forceinline const HVec3    Min( const HVec3 PCREF src ) const __restrict;
    
    /// Max
    SYS_DEPRECATED NOALIAS __forceinline const HVec3    Max( const HVec3 PCREF src ) const __restrict;
    
    /// Min Update
    SYS_DEPRECATED NOALIAS __forceinline void           MinEq( const HVec3 PCREF src ) __restrict;
    
    /// Max Update
    SYS_DEPRECATED NOALIAS __forceinline void           MaxEq( const HVec3 PCREF src ) __restrict;
    
    /// Reciprocal of elements
    SYS_DEPRECATED NOALIAS __forceinline const HVec3    Reciprocal() const __restrict;
    
    /// Horizontal sum of all elements
    SYS_DEPRECATED NOALIAS __forceinline const HF32     Sum() const __restrict;
    
    /// Vector Length
    SYS_DEPRECATED NOALIAS __forceinline const HF32     Length() const __restrict;
    
    /// Vector Length Squared
    SYS_DEPRECATED NOALIAS __forceinline const HF32     Length2() const __restrict;
    SYS_DEPRECATED NOALIAS __forceinline const HF32     LengthSquared() const __restrict;
    
    /// Vector Reciprocal of Length
    SYS_DEPRECATED NOALIAS __forceinline const HF32     RcpLength() const __restrict;
    SYS_DEPRECATED NOALIAS __forceinline const HF32     ReciprocalLength() const __restrict;
    
    /// Combine with HVecW to form a HVec4
    //NOALIAS const HVec4                 operator || ( const HVecW src ) const __restrict;
    
    /// Dot product
    //NOALIAS const HF32                  operator | ( const HVec3 r ) const __restrict;
    
    /// Cross Product
    //NOALIAS const HVec3                 operator ^ ( const HVec3 r ) const __restrict;
    
    /// Star function = Cross Product Matrix ( a* Matrix )
    SYS_DEPRECATED NOALIAS __forceinline const HMat33   Star() const __restrict;
    
    /// HMat33 Multiplication
    //NOALIAS const HVec3                 operator * ( const HMat33& m ) const __restrict;
    
    /// HMat43 Multiplication
    //NOALIAS const HVec3                 operator * ( const HMat43& m ) const __restrict;
    
    /// HMat44 Multiplication
    //NOALIAS const HVec3                 operator * ( const HMat44& m ) const __restrict;
    
    /// Transform: Apply rotation then translation
    SYS_DEPRECATED NOALIAS __forceinline const HVec3    Transformed( const C_Quaternion PCREF rotation, const HVec3 PCREF translation ) const __restrict;
    
    /// Vertex Distance
    SYS_DEPRECATED NOALIAS __forceinline const HF32     Distance( const HVec3 PCREF v ) const __restrict;
    
    /// Vertex Distance Squared
    SYS_DEPRECATED NOALIAS __forceinline const HF32     DistanceSquared( const HVec3 PCREF v ) const __restrict;
    
    /// Lerp
    SYS_DEPRECATED NOALIAS __forceinline const HVec3    Lerp( const HVec3 PCREF dst, const HF32 PCREF f ) const __restrict;
    
    /// Vector Normalised
    SYS_DEPRECATED NOALIAS __forceinline const C_Normalised     Normalised() const __restrict;
    
    /// Vector Normalise
    SYS_DEPRECATED NOALIAS __forceinline void           Normalise() __restrict;
    
    // If the vector's length is longer the specified value, reduces it length to that value. 
    NOALIAS __forceinline void                  LimitLength( const HF32 PCREF length ) __restrict;
    
    /// Vector Normalised - safe version with additional parameter to use for zero length vectors!
    NOALIAS __forceinline const C_Normalised    SafeNormalised( const HVec3 PCREF zeroLengthResult = HVec3() ) const __restrict;
    
    /// Vector Normalise - safe version with additional parameter to use for zero length vectors!
    NOALIAS __forceinline void                  SafeNormalise( const HVec3 PCREF zeroLengthResult = HVec3() ) __restrict;
    
    /// Get Vector of Magnitude
    NOALIAS __forceinline const HVec3           VectorOfMagnitude( const HF32 PCREF mag ) const __restrict;

    /// Get component of this vector parallel to input normal
    NOALIAS __forceinline const HVec3           Parallel( const C_Normal PCREF normal ) const __restrict;
    
    /// Get component of this vector perpendicular to input normal
    NOALIAS __forceinline const HVec3           Perpendicular( const C_Normal PCREF normal ) const __restrict;
    
    /// Get components of this vector parallel & perpendicular to input normal
    NOALIAS __forceinline void                  ParaPerp( const C_Normal PCREF normal, HVec3& result_para, HVec3& result_perp ) const __restrict;
    
    /// Get axes perpendicular to vector.
    NOALIAS __forceinline void                  PerpendicularAxes( HVec3& u, HVec3& v ) const __restrict;
    
    /// Shuffle HVec3:XZ into a HVec2:XY
    NOALIAS __forceinline const HVec2           ShuffleXZtoXY() const __restrict;
    
    /// Round to integer towards minus infinity
    NOALIAS __forceinline const HVec3           RoundToIntMinusInf() const __restrict;
    
    /// Round to integer towards plus infinity
    NOALIAS __forceinline const HVec3           RoundToIntPlusInf() const __restrict;
    
    /// Round to integer towards nearest
    NOALIAS __forceinline const HVec3           RoundToIntNearest() const __restrict;
    
    /// Round to integer towards zero
    NOALIAS __forceinline const HVec3           RoundToIntZero() const __restrict;
    
    /// Comparison Operator ==
    NOALIAS __forceinline const Vector3_Bool    operator == ( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Operator !=
    NOALIAS __forceinline const Vector3_Bool    operator != ( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Operator <=
    NOALIAS __forceinline const Vector3_Bool    operator <= ( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Operator <
    NOALIAS __forceinline const Vector3_Bool    operator < ( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Operator >=
    NOALIAS __forceinline const Vector3_Bool    operator >= ( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Operator >
    NOALIAS __forceinline const Vector3_Bool    operator > ( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Function == Returns 1 if all fields EQ, else 0
    NOALIAS __forceinline const U32             All_EQ( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Function != Returns 1 if all fields NE, else 0
    NOALIAS __forceinline const U32             All_NE( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Function <= Returns 1 if all fields LE, else 0
    NOALIAS __forceinline const U32             All_LE( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Function <  Returns 1 if all fields LT, else 0
    NOALIAS __forceinline const U32             All_LT( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Function >= Returns 1 if all fields GE, else 0
    NOALIAS __forceinline const U32             All_GE( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Function >  Returns 1 if all fields GT, else 0
    NOALIAS __forceinline const U32             All_GT( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Function == Returns 1 if any fields EQ, else 0
    NOALIAS __forceinline const U32             Any_EQ( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Function != Returns 1 if any fields NE, else 0
    NOALIAS __forceinline const U32             Any_NE( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Function <= Returns 1 if any fields LE, else 0
    NOALIAS __forceinline const U32             Any_LE( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Function <  Returns 1 if any fields LT, else 0
    NOALIAS __forceinline const U32             Any_LT( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Function >= Returns 1 if any fields GE, else 0
    NOALIAS __forceinline const U32             Any_GE( const HVec3 PCREF src ) const __restrict;
    
    /// Comparison Function >  Returns 1 if any fields GT, else 0
    NOALIAS __forceinline const U32             Any_GT( const HVec3 PCREF src ) const __restrict;
    
    /// Bitwise AND: THIS & SRC
    NOALIAS __forceinline const HVec3           AND( const HVec3 PCREF src ) const __restrict;
    
    /// Bitwise AND-NOT: THIS & ~SRC
    NOALIAS __forceinline const HVec3           ANDNOT( const HVec3 PCREF src ) const __restrict;
    
    /// Bitwise OR: THIS | SRC
    NOALIAS __forceinline const HVec3           OR( const HVec3 PCREF src ) const __restrict;
    
    /// Bitwise XOR: THIS ^ SRC
    NOALIAS __forceinline const HVec3           XOR( const HVec3 PCREF src ) const __restrict;
    
    /// Finite Test
    NOALIAS __forceinline const Scalar_Bool     Finite() const __restrict;
	
    #if defined( _SIMD_ )
    NOALIAS __forceinline const V128            GetV128() const __restrict                  { return m_V; }
    NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) __restrict  { m_V = src; }
    #else
    NOALIAS __forceinline const V128            GetV128() const __restrict                  { const V128 v = { m_V[0], m_V[1], m_V[2], 0.0f }; return v; }
    NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) __restrict  { *this = HVec3(src); }
    #endif
    
    template < typename Archive >
    NOALIAS __forceinline void Deserialise( Archive& ar ) __restrict;
    
    template < typename Archive >
    NOALIAS __forceinline void Serialise( Archive& ar ) __restrict;

    /// Array Index Operators
    #if defined( _SIMD_ )
    NOALIAS __forceinline const VectorAccessor<HVec3>                 operator [] ( const U32 index ) __restrict;
    NOALIAS __forceinline const ConstVecAccessor<HVec3>               operator [] ( const U32 index ) const __restrict;
    #else
    NOALIAS __forceinline F32&                                        operator [] ( const U32 index ) __restrict;
    NOALIAS __forceinline const F32                                   operator [] ( const U32 index ) const __restrict;
    #endif
    
public:
    
    /// V128 Constructor: Explicit construction from unknown data!
    NOALIAS __forceinline explicit HVec3( const V128 src );
    
    /// V128 Cast
    NOALIAS __forceinline operator const V128 () const __restrict;
    
public:

    #if defined( _SIMD_ )
    V128                m_V;
    #else
    F32                 m_V[3];
    #endif
    
public:
    
    static const U32    m_Elements = 3;
    typedef F32         T_Element;
    //typedef F32         m_DataType;
    //typedef HVec3       m_CrossProductType;
};









//
// ---------------------------------------------------
// |-- HVec4 : Hardware 4 Element F32 Vector Class --|
// ---------------------------------------------------
//

class ALIGN16 PASSINREG DARWIN64_ABI HVec4 
{
public:
    
    /// Vec4F32 Constructor & Cast
    NOALIAS __forceinline explicit HVec4( const Vec4F32 VECREF v )
        #if defined( _SIMD_ )
        : m_V( v.m_V )
        #endif
    {
        #if !defined( _SIMD_ )
        m_V[0] = v.m_V[0];
        m_V[1] = v.m_V[1];
        m_V[2] = v.m_V[2];
        m_V[3] = v.m_V[3];
        #endif
    }
    
    NOALIAS __forceinline operator const Vec4F32() const __restrict
    {
        Vec4F32 result;
        #if defined( _SIMD_ )
        result.m_V = m_V;
        #else
        result.m_V[0] = m_V[0];
        result.m_V[1] = m_V[1];
        result.m_V[2] = m_V[2];
        result.m_V[3] = m_V[3];
        #endif
        return result;
    }
    
    /// Default Constructor
    NOALIAS __forceinline HVec4();
    
	/// Zero Init Constructor
	NOALIAS __forceinline HVec4( const _ZERO_INIT zeroinit );

    /// Undefined Constructor
    NOALIAS __forceinline HVec4( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline HVec4( const HVec4& copy );
    
    /// HVecX Constructor
    NOALIAS __forceinline HVec4( const HVecX PCREF x );
    
    /// HVecY Constructor
    NOALIAS __forceinline HVec4( const HVecY PCREF y );
    
    /// HVecZ Constructor
    NOALIAS __forceinline HVec4( const HVecZ PCREF z );
    
    /// HVecW Constructor
    NOALIAS __forceinline HVec4( const HVecW PCREF w );
    
    /// HVec2 Constructor
    NOALIAS __forceinline explicit HVec4( const HVec2 PCREF xy );
    
    /// HVec3 Constructor
    NOALIAS __forceinline explicit HVec4( const HVec3 PCREF xyz );
    
    /// C_Vertex Constructor
    NOALIAS __forceinline HVec4( const C_Vertex PCREF xyz );
    
    /// C_Vector Constructor
    NOALIAS __forceinline HVec4( const C_Vector PCREF xyz );
    
    /// C_Normal Constructor
    NOALIAS __forceinline HVec4( const C_Normal PCREF xyz );
    
    /// HF32 Quad Constructor
    NOALIAS __forceinline HVec4( const HF32 PCREF x, const HF32 PCREF y, const HF32 PCREF z, const HF32 PCREF w );
    
    /// HF32 Broadcast Constructor
    NOALIAS __forceinline explicit HVec4( const HF32 PCREF xyzw );
    
    /// Explicit F32 Array Constructor
    NOALIAS __forceinline explicit HVec4( const F32* ptr );
    
    /// V128 Constructor
    NOALIAS __forceinline explicit HVec4( const V128 PCREF src );
    
    /// V128 Cast
    NOALIAS __forceinline operator const V128 () const __restrict;
    
    /// Validate that elements are all finite
    NOALIAS __forceinline void                  DebugValidate() const __restrict;
    
    /// HVecX Cast
    NOALIAS __forceinline operator              const HVecX () const __restrict;
    
    /// HVecY Cast
    NOALIAS __forceinline operator              const HVecY () const __restrict;
    
    /// HVecZ Cast
    NOALIAS __forceinline operator              const HVecZ () const __restrict;
    
    /// HVecW Cast
    NOALIAS __forceinline operator              const HVecW () const __restrict;
    
    /// Copy Assignment
    NOALIAS __forceinline const HVec4&          operator = ( const HVec4& copy );
    
    /// HVecX Assignment
    NOALIAS __forceinline const HVec4&          operator = ( const HVecX PCREF x ) __restrict;
    
    /// HVecY Assignment
    NOALIAS __forceinline const HVec4&          operator = ( const HVecY PCREF y ) __restrict;
    
    /// HVecZ Assignment
    NOALIAS __forceinline const HVec4&          operator = ( const HVecZ PCREF z ) __restrict;
    
    /// HVecW Assignment
    NOALIAS __forceinline const HVec4&          operator = ( const HVecW PCREF w ) __restrict;
    
    /// HVec2 Assignment
    NOALIAS __forceinline const HVec4&          operator = ( const HVec2 PCREF xy ) __restrict;
    
    /// HVec3 Assignment
    NOALIAS __forceinline const HVec4&          operator = ( const HVec3 PCREF xyz ) __restrict;
    
    /// Accessor: Get X
    NOALIAS __forceinline const HF32            X() const __restrict;
    
    /// Accessor: Get Y
    NOALIAS __forceinline const HF32            Y() const __restrict;
    
    /// Accessor: Get Z
    NOALIAS __forceinline const HF32            Z() const __restrict;
    
    /// Accessor: Get W
    NOALIAS __forceinline const HF32            W() const __restrict;
    
    /// Accessor: Get XYZ
    NOALIAS __forceinline const HVec3           XYZ() const __restrict;
    
    /// Accessor: Get YZW
    NOALIAS __forceinline const HVec3           YZW() const __restrict;
    
    /// Accessor: Set X
    NOALIAS __forceinline void                  X( const HF32 PCREF scalar ) __restrict;
    
    /// Accessor: Set Y
    NOALIAS __forceinline void                  Y( const HF32 PCREF scalar ) __restrict;
    
    /// Accessor: Set Z
    NOALIAS __forceinline void                  Z( const HF32 PCREF scalar ) __restrict;
    
    /// Accessor: Set W
    NOALIAS __forceinline void                  W( const HF32 PCREF scalar ) __restrict;
    
    /// Unary Negation Operator
    NOALIAS __forceinline const HVec4           operator - () const __restrict;
    
    /// Binary Addition Operator
    NOALIAS __forceinline const HVec4           operator + ( const HVec4 PCREF src ) const __restrict;
    
    /// Addition Operator
	NOALIAS __forceinline void					operator += ( const HVec4 PCREF src ) __restrict;

    /// Binary Subtraction Operator
    NOALIAS __forceinline const HVec4           operator - ( const HVec4 PCREF src ) const __restrict;
    
    /// Binary Subtraction Operator
    NOALIAS __forceinline void					operator -= ( const HVec4 PCREF src ) __restrict;

    /// Binary Multiplication Operator
    NOALIAS __forceinline const HVec4           operator * ( const HVec4 PCREF src ) const __restrict;
    
    /// Binary Division Operator
    NOALIAS __forceinline const HVec4           operator / ( const HVec4 PCREF src ) const __restrict;
    
    /// HF32 Multiplication Operator
    NOALIAS __forceinline const HVec4           operator * ( const HF32 PCREF src ) const __restrict;
    
    /// HF32 Division Operator
    NOALIAS __forceinline const HVec4           operator / ( const HF32 PCREF src ) const __restrict;
    
    /// HF32 Multiply Update Operator
    NOALIAS __forceinline void                  operator *= ( const HF32 PCREF src ) __restrict;
    
    /// HF32 Divide Update Operator
    NOALIAS __forceinline void                  operator /= ( const HF32 PCREF src ) __restrict;
    
    /// Min
    NOALIAS __forceinline const HVec4           Min( const HVec4 PCREF src ) const __restrict;
    
    /// Max
    NOALIAS __forceinline const HVec4           Max( const HVec4 PCREF src ) const __restrict;
    
    /// Horizontal sum of all elements
    NOALIAS __forceinline const HF32            Sum() const __restrict;
    
    /// Vector Length
    NOALIAS __forceinline const HF32            Length() const __restrict;
    
    /// Vector Length Squared
    NOALIAS __forceinline const HF32            Length2() const __restrict;
    NOALIAS __forceinline const HF32            LengthSquared() const __restrict;
    
    /// Vector Reciprocal of Length
    NOALIAS __forceinline const HF32            RcpLength() const __restrict;
    NOALIAS __forceinline const HF32            ReciprocalLength() const __restrict;
    
    /// Vector Normalised
    NOALIAS __forceinline const HVec4           Normalised() const __restrict;
    
    /// Vector Normalise
    NOALIAS __forceinline void                  Normalise() __restrict;
    
    /// Vector Normalised - safe version with additional parameter to use for zero length vectors!
    NOALIAS __forceinline const HVec4           SafeNormalised( const HVec4 PCREF zeroLengthResult = HVec4() ) const __restrict;
    
    /// Vector Normalise - safe version with additional parameter to use for zero length vectors!
    NOALIAS __forceinline void                  SafeNormalise( const HVec4 PCREF zeroLengthResult = HVec4() ) __restrict;

    #if defined(_SIMD_)
    NOALIAS __forceinline const V128            GetV128() const __restrict                  { return m_V; }
    NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) __restrict  { m_V=src; }
    #else
    NOALIAS __forceinline const V128            GetV128() const __restrict                  { V128 v = { m_V[0], m_V[1], m_V[2], m_V[3] }; return v; }
    NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) __restrict  { *this = HVec4(src); }
    #endif

    /// Dot product
    NOALIAS __forceinline const HF32            operator | ( const HVec4 PCREF r ) const __restrict;
    
    // Vector * Matrix Multiplication
    NOALIAS __forceinline const HVec4           operator * ( const HMat44& m ) const __restrict;
    
    /// Comparison Operator ==
    NOALIAS __forceinline const Vector4_Bool    operator == ( const HVec4 PCREF src ) const __restrict;
    
    /// Comparison Operator !=
    NOALIAS __forceinline const Vector4_Bool    operator != ( const HVec4 PCREF src ) const __restrict;
    
    /// Comparison Operator <=
    NOALIAS __forceinline const Vector4_Bool    operator <= ( const HVec4 PCREF src ) const __restrict;
    
    /// Comparison Operator <
    NOALIAS __forceinline const Vector4_Bool    operator < ( const HVec4 PCREF src ) const __restrict;
    
    /// Comparison Operator >=
    NOALIAS __forceinline const Vector4_Bool    operator >= ( const HVec4 PCREF src ) const __restrict;
    
    /// Comparison Operator >
    NOALIAS __forceinline const Vector4_Bool    operator > ( const HVec4 PCREF src ) const __restrict;
    
    /// Bitwise AND: THIS & SRC
    NOALIAS __forceinline const HVec4           AND( const HVec4 PCREF src ) const __restrict;
    
    /// Bitwise AND-NOT: THIS & ~SRC
    NOALIAS __forceinline const HVec4           ANDNOT( const HVec4 PCREF src ) const __restrict;
    
    /// Bitwise OR: THIS | SRC
    NOALIAS __forceinline const HVec4           OR( const HVec4 PCREF src ) const __restrict;
    
    /// Bitwise XOR: THIS ^ SRC
    NOALIAS __forceinline const HVec4           XOR( const HVec4 PCREF src ) const __restrict;
    
    /// Finite Test
    NOALIAS __forceinline const Scalar_Bool     Finite() const __restrict;
    
    /// Read from Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Serialise( T_Stream& stream ) __restrict;
    
    /// Array Index Operators
    #if defined( _SIMD_ )
    NOALIAS __forceinline const VectorAccessor<HVec4>                 operator [] ( const U32 index ) __restrict;
    NOALIAS __forceinline const ConstVecAccessor<HVec4>               operator [] ( const U32 index ) const __restrict;
    #else
    NOALIAS __forceinline F32&                                        operator [] ( const U32 index ) __restrict;
    NOALIAS __forceinline const F32                                   operator [] ( const U32 index ) const __restrict;
    #endif
    
public:
    
	#if defined(_SIMD_)
    V128                m_V;
    #else
    F32                 m_V[4];
    
	NOALIAS __forceinline void	            SetComponents( F32 x, F32 y, F32 z, F32 w ) { m_V[0]=x; m_V[1]=y; m_V[2]=z; m_V[3]=w;}
	NOALIAS __forceinline void	            SetAllComponents( F32 f ) { m_V[0]=f; m_V[1]=f; m_V[2]=f; m_V[3]=f; }
    #endif
    
public:
    
    static const U32    m_Elements = 4;
    typedef F32         T_Element;
    //typedef F32         m_DataType;
    //typedef void        m_CrossProductType;
};

NOALIAS __forceinline const HVec4 operator * ( const HF32 PCREF f, const HVec4 PCREF v );
NOALIAS __forceinline const HVec4 operator / ( const HF32 PCREF f, const HVec4 PCREF v );

NOALIAS __forceinline const HVec4 operator * ( const HVec4 PCREF v, const F32& f );
NOALIAS __forceinline const HVec4 operator / ( const HVec4 PCREF v, const F32& f );

NOALIAS __forceinline void operator *= ( HVec4& v, const F32& f );
NOALIAS __forceinline void operator /= ( HVec4& v, const F32& f );


/// Round to integer towards minus infinity
NOALIAS __forceinline const HVec4 RoundToIntMinusInf( const HVec4 PCREF src );

/// Round to integer towards plus infinity
NOALIAS __forceinline const HVec4 RoundToIntPlusInf( const HVec4 PCREF src );

/// Round to integer towards nearest
NOALIAS __forceinline const HVec4 RoundToIntNearest( const HVec4 PCREF src );

/// Round to integer towards zero
NOALIAS __forceinline const HVec4 RoundToIntZero( const HVec4 PCREF src );

/// Returns a HVec4 with each element having it's magnitude taken from the respective elements in 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const HVec4 CopySign( const HVec4 PCREF magnitude, const HVec4 PCREF sign );

/// Returns a HVec4 with each element having it's magnitude taken from the respective elements in 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const HF32 CopySign( const HF32 PCREF magnitude, const HF32 PCREF sign );

/// Sum of Products: ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z )
NOALIAS __forceinline const HF32 MultiplySum3( const HVec4 PCREF a, const HVec4 PCREF b );

/// Sum of Products: ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z ) + ( a.w * b.w )
NOALIAS __forceinline const HF32 MultiplySum4( const HVec4 PCREF a, const HVec4 PCREF b );




//
// --------------------------------------------------------------------
// |-- C_Origin : Origin for conversion between C_Vertex & C_Vector --|
// --------------------------------------------------------------------
//

class C_Origin
{
public:
    
    /// Addition: Origin + Vector --> Vertex
    NOALIAS __forceinline const C_Vertex operator + ( const C_Vector PCREF src ) const;
    
    /// Subtraction: Origin - Vertex --> Vector
    NOALIAS __forceinline const C_Vector operator - ( const C_Vertex PCREF src ) const;
};




//
// ----------------------------------------------
// |-- C_Vertex : HVec3 with vertex semantics --|
// ----------------------------------------------
//

class C_Vertex
{
public:
    
    /// Vec4F32 Constructor
    NOALIAS __forceinline explicit C_Vertex( const Vec4F32 VECREF v )
        #if defined( _SIMD_ )
        : m_V( v.m_V )
        #endif
    {
        #if !defined( _SIMD_ )
        m_V.m_V[0] = v.m_V[0];
        m_V.m_V[1] = v.m_V[1];
        m_V.m_V[2] = v.m_V[2];
        #endif
    }
    
    /// Vec4F32 Cast
    NOALIAS __forceinline operator const Vec4F32() const __restrict
    {
        Vec4F32 result;
        #if defined( _SIMD_ )
        result.m_V = m_V.m_V;
        #else
        result.m_V[0] = m_V[0];
        result.m_V[1] = m_V[1];
        result.m_V[2] = m_V[2];
        #endif
        return result;
    }
    
    /// Point3 Constructor
    NOALIAS __forceinline C_Vertex( const Point3 PCREF v )
        #if defined( _SIMD_ )
        : m_V( v.m_V.m_V )
        #endif
    {
        #if !defined( _SIMD_ )
        m_V.m_V[0] = v.m_V.m_V[0];
        m_V.m_V[1] = v.m_V.m_V[1];
        m_V.m_V[2] = v.m_V.m_V[2];
        #endif
    }
    
    /// Point3 Cast
    NOALIAS __forceinline operator const Point3() const __restrict
    {
        Point3 result;
        #if defined( _SIMD_ )
        result.m_V.m_V = m_V.m_V;
        #else
        result.m_V.m_V[0] = m_V.m_V[0];
        result.m_V.m_V[1] = m_V.m_V[1];
        result.m_V.m_V[2] = m_V.m_V[2];
        #endif
        return result;
    }
    
    /// Default Constructor
    NOALIAS __forceinline C_Vertex();
    
	/// Zero Init Constructor
	NOALIAS __forceinline C_Vertex( const _ZERO_INIT zeroinit );

    /// Undefined Constructor
    NOALIAS __forceinline C_Vertex( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline C_Vertex( const C_Vertex& copy );
    
    /// Origin Constructor
    NOALIAS __forceinline C_Vertex( const C_Origin PCREF src );
    
    /// Explicit Vector Constructor
    NOALIAS __forceinline explicit C_Vertex( const C_Vector PCREF src );
    
    /// HVec3 Constructor
    NOALIAS __forceinline C_Vertex( const HVec3 PCREF src );
    
    /// HVec4 Constructor
    NOALIAS __forceinline explicit C_Vertex( const HVec4 PCREF src );
    
    /// HF32 XYZ Constructor
    NOALIAS __forceinline C_Vertex( const HF32 PCREF x, const HF32 PCREF y, const HF32 PCREF z );
    
    /// Explicit F32 Array Constructor
    NOALIAS __forceinline explicit C_Vertex( const F32* xyz );
    
    /// Explicit HF32 XYZ Broadcast Constructor
    NOALIAS __forceinline explicit C_Vertex( const HF32 PCREF xyz );
    
    /// Explicit Constructor from Float3 ptr
    NOALIAS __forceinline explicit C_Vertex( MathsStorage::Float3 const * pStorage );
    
    /// Cast to Float3
    NOALIAS __forceinline operator const MathsStorage::Float3 () const __restrict;
    
    /// Explicit Constructor from Fixed3_S16 ptr
    template < int T_Precision >
    NOALIAS __forceinline explicit C_Vertex( MathsStorage::Fixed3_S16<T_Precision> const * pStorage );
    
    /// Cast to Fixed3_S16
    template < int T_Precision >
    NOALIAS __forceinline operator const MathsStorage::Fixed3_S16<T_Precision> () const __restrict;
    
    /// Copy Assignment
    NOALIAS __forceinline const C_Vertex       operator = ( const C_Vertex& copy );
    
    /// Origin Assignment
    NOALIAS __forceinline const C_Vertex       operator = ( const C_Origin PCREF src ) __restrict;
    
    /// C_Vertex_Multiply Assignment
    NOALIAS __forceinline const C_Vertex       operator = ( const HVec3_Multiply& src ) __restrict;
    
    /// C_Vertex_Add Assignment
    NOALIAS __forceinline const C_Vertex       operator = ( const HVec3_Add& src ) __restrict;
    
    /// C_Vertex_Subtract Assignment
    NOALIAS __forceinline const C_Vertex       operator = ( const C_Vertex_Subtract& src ) __restrict;
    
    /// Accessor: X
    NOALIAS __forceinline const HF32            X() const __restrict;
    
    /// Accessor: Y
    NOALIAS __forceinline const HF32            Y() const __restrict;
    
    /// Accessor: Z
    NOALIAS __forceinline const HF32            Z() const __restrict;

	    /// Accessor: Set X
	NOALIAS __forceinline void                  X( const HF32 PCREF x ) __restrict {m_V.X(x);}
    
    /// Accessor: Set Y
    NOALIAS __forceinline void                  Y( const HF32 PCREF y ) __restrict {m_V.Y(y);}
    
    /// Accessor: Set Z
    NOALIAS __forceinline void                  Z( const HF32 PCREF z ) __restrict {m_V.Z(z);}
    
    /// Element Zeroing Functions
    NOALIAS __forceinline void                  ZeroX() __restrict;
    NOALIAS __forceinline void                  ZeroY() __restrict;
    NOALIAS __forceinline void                  ZeroZ() __restrict;
    NOALIAS __forceinline void                  ZeroXY() __restrict;
    NOALIAS __forceinline void                  ZeroXZ() __restrict;
    NOALIAS __forceinline void                  ZeroYZ() __restrict;
    
    /// Return copy with elements zeroed
    NOALIAS __forceinline const C_Vertex        ZeroedX() const __restrict;
    NOALIAS __forceinline const C_Vertex        ZeroedY() const __restrict;
    NOALIAS __forceinline const C_Vertex        ZeroedZ() const __restrict;
    NOALIAS __forceinline const C_Vertex        ZeroedXY() const __restrict;
    NOALIAS __forceinline const C_Vertex        ZeroedXZ() const __restrict;
    NOALIAS __forceinline const C_Vertex        ZeroedYZ() const __restrict;
    
    /// Addition: Vertex + Vertex --> Vertex
    NOALIAS __forceinline const C_Vertex        operator + ( const C_Vertex PCREF src ) const __restrict;
    
    /// Addition: Vertex + Vector --> Vertex
    NOALIAS __forceinline const C_Vertex        operator + ( const C_Vector PCREF src ) const __restrict;
    
    /// Addition: Vertex + Normal --> Vertex
    NOALIAS __forceinline const C_Vertex        operator + ( const C_Normal PCREF src ) const __restrict;
    
    /// Addition: Vertex += Vertex
    //NOALIAS __forceinline void                  operator += ( const C_Vertex PCREF src ) __restrict;
    
    /// Addition: Vertex += Vector
    NOALIAS __forceinline void                  operator += ( const C_Vector PCREF src ) __restrict;
    
    /// Subtraction: Vertex - Vertex --> Vector
    NOALIAS __forceinline const C_Vector        operator - ( const C_Vertex PCREF src ) const __restrict;
    
    /// Subtraction: Vertex - Vector --> Vertex
    NOALIAS __forceinline const C_Vertex        operator - ( const C_Vector PCREF src ) const __restrict;
    
    /// Subtraction: Vertex -= Vector
    NOALIAS __forceinline void                  operator -= ( const C_Vector PCREF src ) __restrict;
    
    /// Subtraction: Vertex - Normal --> Vertex
    NOALIAS __forceinline const C_Vertex        operator - ( const C_Normal PCREF src ) const __restrict;
    
    /// Subtraction: Vertex - Origin --> Vector
    NOALIAS __forceinline const C_Vector        operator - ( const C_Origin PCREF src ) const __restrict;
    
    /// Multiplication: Vertex * HF32 --> Vertex
    //NOALIAS __forceinline const C_Vertex        operator * ( const HF32 PCREF scalar ) const __restrict;
    
    /// Multiplication: Vertex * HMat43 --> Vertex
    NOALIAS __forceinline const C_Vertex        operator * ( const HMat43& matrix ) const __restrict;
    
    /// Multiplication: Vertex * C_QuatTrans --> Vertex
    //const C_Vertex              operator * ( const C_QuatTrans& qt ) const __restrict;

    NOALIAS __forceinline const C_Vertex        Transformed( const C_Quaternion PCREF rotation, const C_Vector PCREF translation ) const __restrict;
    
    /// Multiplication: Vertex *= HF32
    NOALIAS __forceinline void                  operator *= ( const HF32 PCREF scalar ) __restrict;
    
    /// Minimum: Vertex min Vertex --> Vertex
    NOALIAS __forceinline const C_Vertex        Min( const C_Vertex PCREF src ) const __restrict;
    
    /// Minimum: Vertex max Vertex --> Vertex
    NOALIAS __forceinline const C_Vertex        Max( const C_Vertex PCREF src ) const __restrict;
    
    /// Distance
    NOALIAS __forceinline const HF32            Distance( const C_Vertex PCREF src ) const __restrict;
    
    /// DistanceSquared
    NOALIAS __forceinline const HF32            DistanceSquared( const C_Vertex PCREF src ) const __restrict;
    
    /// Lerp
    NOALIAS __forceinline const C_Vertex        Lerp( const C_Vertex PCREF src, const HF32 PCREF t ) const __restrict;
    
    /// Round to integer towards minus infinity
    NOALIAS __forceinline const C_Vertex        RoundToIntMinusInf() const __restrict;
    
    /// Round to integer towards plus infinity
    NOALIAS __forceinline const C_Vertex        RoundToIntPlusInf() const __restrict;
    
    /// Round to integer towards nearest
    NOALIAS __forceinline const C_Vertex        RoundToIntNearest() const __restrict;
    
    /// Round to integer towards zero
    NOALIAS __forceinline const C_Vertex        RoundToIntZero() const __restrict;
    
    /// Comparison - Equal
    NOALIAS __forceinline const U32				operator == ( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison - Not Equal
    NOALIAS __forceinline const U32				operator != ( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison - Less Than
    /*NOALIAS __forceinline const Vector3_Bool    operator < ( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison - Less Equal
    NOALIAS __forceinline const Vector3_Bool    operator <= ( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison - Greater Than
    NOALIAS __forceinline const Vector3_Bool    operator > ( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison - Greater Equal
    NOALIAS __forceinline const Vector3_Bool    operator >= ( const C_Vertex PCREF src ) const __restrict;*/
    
    /// Comparison Function ==
    NOALIAS __forceinline const U32             All_EQ( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison Function !=
    NOALIAS __forceinline const U32             All_NE( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison Function <=
    NOALIAS __forceinline const U32             All_LE( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison Function <
    NOALIAS __forceinline const U32             All_LT( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison Function >=
    NOALIAS __forceinline const U32             All_GE( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison Function >
    NOALIAS __forceinline const U32             All_GT( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison Function ==
    NOALIAS __forceinline const U32             Any_EQ( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison Function !=
    NOALIAS __forceinline const U32             Any_NE( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison Function <=
    NOALIAS __forceinline const U32             Any_LE( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison Function <
    NOALIAS __forceinline const U32             Any_LT( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison Function >=
    NOALIAS __forceinline const U32             Any_GE( const C_Vertex PCREF src ) const __restrict;
    
    /// Comparison Function >
    NOALIAS __forceinline const U32             Any_GT( const C_Vertex PCREF src ) const __restrict;
    
    /// Finite test
    NOALIAS __forceinline const Scalar_Bool     Finite() const __restrict;
    
    /// Read from Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Serialise( T_Stream& stream ) __restrict;
    
public:
    
    HVec3   m_V;
};


//NOALIAS const C_Vertex operator * ( const C_Vertex PCREF v, const F32& f );
//NOALIAS const C_Vertex operator / ( const C_Vertex PCREF v, const F32& f );

//void operator *= ( C_Vertex& v, const F32& f );
//void operator /= ( C_Vertex& v, const F32& f );






///
/// -----------------------------------------------------------------------------------------------------------
/// |-- C_Normal : HVec3 with unit length normal semantics - caveat: uninitialised normals are zero length! --|
/// -----------------------------------------------------------------------------------------------------------
///

class C_Normal 
{
public:
    
    /// Vec4F32 Constructor
    NOALIAS __forceinline explicit C_Normal( const Vec4F32 VECREF v )
        #if defined( _SIMD_ )
        : m_V( v.m_V )
        #endif
    {
        #if !defined( _SIMD_ )
        m_V[0] = v.m_V[0];
        m_V[1] = v.m_V[1];
        m_V[2] = v.m_V[2];
        #endif
    }
    
    /// Vec4F32 Cast
    NOALIAS __forceinline operator const Vec4F32() const __restrict
    {
        Vec4F32 result;
        #if defined( _SIMD_ )
        result.m_V = m_V;
        #else
        result.m_V[0] = m_V[0];
        result.m_V[1] = m_V[1];
        result.m_V[2] = m_V[2];
        #endif
        return result;
    }
    
    /// Normal3 Constructor
    NOALIAS __forceinline explicit C_Normal( const Normal3 PCREF v )
        #if defined( _SIMD_ )
        : m_V( v.m_V.m_V )
        #endif
    {
        #if !defined( _SIMD_ )
        m_V[0] = v.m_V.m_V[0];
        m_V[1] = v.m_V.m_V[1];
        m_V[2] = v.m_V.m_V[2];
        #endif
    }
    
    /// Normal3 Cast
    NOALIAS __forceinline operator const Normal3() const __restrict
    {
        Normal3 result;
        #if defined( _SIMD_ )
        result.m_V.m_V = m_V.m_V;
        #else
        result.m_V.m_V[0] = m_V.m_V[0];
        result.m_V.m_V[1] = m_V.m_V[1];
        result.m_V.m_V[2] = m_V.m_V[2];
        #endif
        return result;
    }
    
    enum _UNIT_LENGTH_VECTOR
    {
        UNIT_LENGTH_VECTOR
    };
    
    /// Positive X Axis Aligned Normal Constructor
    NOALIAS __forceinline static const C_Normal AxisPositiveX();
    
    /// Negative X Axis Aligned Normal Constructor
    NOALIAS __forceinline static const C_Normal AxisNegativeX();
    
    /// Positive Y Axis Aligned Normal Constructor
    NOALIAS __forceinline static const C_Normal AxisPositiveY();
    
    /// Negative Y Axis Aligned Normal Constructor
    NOALIAS __forceinline static const C_Normal AxisNegativeY();
    
    /// Positive Z Axis Aligned Normal Constructor
    NOALIAS __forceinline static const C_Normal AxisPositiveZ();
    
    /// Negative Z Axis Aligned Normal Constructor
    NOALIAS __forceinline static const C_Normal AxisNegativeZ();
    
    /// Default Constructor : Zero length normal!
    NOALIAS __forceinline C_Normal();
    
	/// Zero Init Constructor
	NOALIAS __forceinline C_Normal( const _ZERO_INIT zeroinit );

    /// Undefined Constructor
    NOALIAS __forceinline C_Normal( const _UNDEFINED undefined );
    
    /// Unit Length Vector Constructor
    NOALIAS __forceinline C_Normal( const HVec3 PCREF src, const _UNIT_LENGTH_VECTOR unit_length_vector );
    
    /// Copy Constructor
    NOALIAS __forceinline C_Normal( const C_Normal& copy );
    
    /// C_Normalised Constructor
    NOALIAS __forceinline C_Normal( const C_Normalised& src );
    
    /// HVec3 Constructor
    NOALIAS __forceinline explicit C_Normal( const HVec3 PCREF src );
    
    /// Explicit HVec4 Constructor
    NOALIAS __forceinline explicit C_Normal( const HVec4 PCREF src );
    
    /// Vector Constructor
    NOALIAS __forceinline explicit C_Normal( const C_Vector PCREF src );
    
    /// Explicit C_SiVector Constructor
    template < class LHS >
    NOALIAS __forceinline explicit C_Normal( const C_SiVector<LHS> PCREF src );
    
    /// HF32 XYZ Constructor
    NOALIAS __forceinline C_Normal( const HF32 PCREF x, const HF32 PCREF y, const HF32 PCREF z );
    
    /// F32 array constructor
    NOALIAS __forceinline C_Normal( F32 const * pXYZ );
    
    /// Explicit Constructor from Float3 ptr
    NOALIAS __forceinline explicit C_Normal( MathsStorage::Float3 const * pStorage );
    
    /// Cast to Float3
    NOALIAS __forceinline operator const MathsStorage::Float3 () const __restrict;
    
    /// Selector Constructor: Selects fields from the inputs based on the selector mask: result = ( selector ? a : b )
    //C_Normal( const Vector3_Bool& selector, const C_Normal a, const C_Normal b );
    
    /// Copy Assignment
    NOALIAS __forceinline const C_Normal               operator = ( const C_Normal& copy );
    
    /// HVec3 Assignment
    NOALIAS __forceinline const C_Normal               operator = ( const HVec3 PCREF src ) __restrict;
    
    /// Vector Assignment
    NOALIAS __forceinline const C_Normal               operator = ( const C_Vector PCREF src ) __restrict;
    
    /// C_SiVector Assignment
    template < class LHS >
    NOALIAS __forceinline const C_Normal               operator = ( const C_SiVector<LHS> PCREF src ) __restrict;
    
    /// Accessor: X
    NOALIAS __forceinline const HF32                    X() const __restrict;
    
    /// Accessor: Y
    NOALIAS __forceinline const HF32                    Y() const __restrict;
    
    /// Accessor: Z
    NOALIAS __forceinline const HF32                    Z() const __restrict;
    
    /// Negation: Normal --> Normal
    NOALIAS __forceinline const C_Normal                operator - () const __restrict;
    
    /// Multiplication: Normal * HF32 -> Vector
    //NOALIAS __forceinline const C_Vector                operator * ( const HF32 PCREF scalar ) const __restrict;
    
    /// Multiplication: Normal * HMat33 --> Vector
    NOALIAS __forceinline const C_Vector                operator * ( const HMat33& matrix ) const __restrict;
    
    /// Multiplication: Normal * C_Quaternion --> Normal
    //NOALIAS __forceinline const C_Normal                operator * ( const C_Quaternion q ) const __restrict;
    
    /// Dot Product: Normal | HVec3 --> HF32
    NOALIAS __forceinline const HF32                    operator | ( const HVec3 PCREF src ) const __restrict;
    
    /// Dot Product: Normal | Vector --> HF32
    NOALIAS __forceinline const HF32                    operator | ( const C_Vector PCREF src ) const __restrict;
    
    /// Dot Product: Normal | Normal --> HF32
    NOALIAS __forceinline const HF32                    operator | ( const C_Normal PCREF src ) const __restrict;
    
    /// Cross Product: Normal ^ HVec3 --> Vector
    NOALIAS __forceinline const C_Vector                operator ^ ( const HVec3 PCREF src ) const __restrict;
    
    /// Cross Product: Normal ^ Vector --> Vector
    NOALIAS __forceinline const C_Vector                operator ^ ( const C_Vector PCREF src ) const __restrict;
    
    /// Cross Product: Normal ^ Normal --> Vector
    NOALIAS __forceinline const C_Vector                operator ^ ( const C_Normal PCREF src ) const __restrict;
    
    /// Selects fields from the inputs based on the selector mask: result = ( selector ? a : b )
    //DEPRECATED void             Select( const Vector3_Bool& selector, const C_Normal a, const C_Normal b );
    
    /// Finite test
    NOALIAS __forceinline const Scalar_Bool             Finite() const __restrict;
    
    /// Read from Stream
    template < typename T_Stream >
    NOALIAS __forceinline void   Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream >
    NOALIAS __forceinline void   Serialise( T_Stream& stream ) __restrict;
    
public:
    
    #if defined( _SIMD_ )
    /// Explicit V128 Constructor
    NOALIAS __forceinline explicit C_Normal( const V128 PCREF src );
    #endif
    
public:
    
    HVec3                       m_V;
};


//NOALIAS __forceinline C_Vector operator * ( const C_Normal PCREF v, const F32& f );
//NOALIAS __forceinline C_Vector operator / ( const C_Normal PCREF v, const F32& f );




//
// ----------------------------------------------
// |-- C_Vector : HVec3 with vector semantics --|
// ----------------------------------------------
//

class C_Vector
{
public:
    
    /// Vec4F32 Constructor
    NOALIAS __forceinline explicit C_Vector( const Vec4F32 VECREF v )
        #if defined( _SIMD_ )
        : m_V( v.m_V )
        #endif
    {
        #if !defined( _SIMD_ )
        m_V.m_V[0] = v.m_V[0];
        m_V.m_V[1] = v.m_V[1];
        m_V.m_V[2] = v.m_V[2];
        #endif
    }
    
    /// Vec4F32 Cast
    NOALIAS __forceinline operator const Vec4F32() const __restrict
    {
        Vec4F32 result;
        #if defined( _SIMD_ )
        result.m_V = m_V.m_V;
        #else
        result.m_V[0] = m_V.m_V[0];
        result.m_V[1] = m_V.m_V[1];
        result.m_V[2] = m_V.m_V[2];
        #endif
        return result;
    }
    
    /// Vector3 Constructor
    NOALIAS __forceinline explicit C_Vector( const Vector3 PCREF v )
        #if defined( _SIMD_ )
        : m_V( v.m_V.m_V )
        #endif
    {
        #if !defined( _SIMD_ )
        m_V.m_V[0] = v.m_V.m_V[0];
        m_V.m_V[1] = v.m_V.m_V[1];
        m_V.m_V[2] = v.m_V.m_V[2];
        #endif
    }
    
    /// Vector3 Cast
    NOALIAS __forceinline operator const Vector3() const __restrict
    {
        Vector3 result;
        #if defined( _SIMD_ )
        result.m_V.m_V = m_V.m_V;
        #else
        result.m_V.m_V[0] = m_V.m_V[0];
        result.m_V.m_V[1] = m_V.m_V[1];
        result.m_V.m_V[2] = m_V.m_V[2];
        #endif
        return result;
    }
    
    /// Default Constructor
    NOALIAS __forceinline C_Vector();
    
    /// Zero Init Constructor
    NOALIAS __forceinline C_Vector( const _ZERO_INIT undefined );

    /// Undefined Constructor
    NOALIAS __forceinline C_Vector( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline C_Vector( const C_Vector& copy );
    
    /// Explicit Vertex Constructor
    NOALIAS __forceinline explicit C_Vector( const C_Vertex PCREF src );
    
    /// Explicit Normal Constructor
    NOALIAS __forceinline explicit C_Vector( const C_Normal PCREF src );
    
    /// HVec3 Constructor
    NOALIAS __forceinline C_Vector( const HVec3 PCREF src );
    
    /// Explicit HVec4 Constructor
    NOALIAS __forceinline explicit C_Vector( const HVec4 PCREF src );
    
    /// HF32 XYZ Constructor
    NOALIAS __forceinline C_Vector( const HF32 PCREF x, const HF32 PCREF y, const HF32 PCREF z );
    
    /// Explicit F32 Array Constructor
    NOALIAS __forceinline explicit C_Vector( const F32* xyz );
    
    /// Explicit HF32 XYZ Broadcast Constructor
    NOALIAS __forceinline explicit C_Vector( const HF32 PCREF xyz );
    
    /// Explicit Constructor from Float3 ptr
    NOALIAS __forceinline explicit C_Vector( MathsStorage::Float3 const * pStorage );
    
    /// Cast to Float3
    NOALIAS __forceinline operator const MathsStorage::Float3 () const __restrict;
    
    /// Explicit Constructor from Fixed3_S16 ptr
    template < int T_Precision >
    NOALIAS __forceinline explicit C_Vector( MathsStorage::Fixed3_S16<T_Precision> const * pStorage );
    
    /// Cast to Fixed3_S16
    template < int T_Precision >
    NOALIAS __forceinline operator const MathsStorage::Fixed3_S16<T_Precision> () const __restrict;
    
    /// Copy Assignment
    NOALIAS __forceinline const C_Vector       operator = ( const C_Vector& copy );
    
    /// HVec3 Assignment
    NOALIAS __forceinline const C_Vector       operator = ( const HVec3 PCREF src ) __restrict;
    
    /// C_Normal Assignment
    NOALIAS __forceinline const C_Vector       operator = ( const C_Normal PCREF src ) __restrict;
    
    /// C_Vector_Multiply Assignment
    //NOALIAS __forceinline const C_Vector       operator = ( const C_Vector_Multiply& src ) __restrict;
    
    /// C_Vector_Add Assignment
    NOALIAS __forceinline const C_Vector       operator = ( const C_Vector_Add& src ) __restrict;
    
    /// C_Vector_Subtract Assignment
    NOALIAS __forceinline const C_Vector       operator = ( const C_Vector_Subtract& src ) __restrict;
    
    /// Accessor: Get X
    NOALIAS __forceinline const HF32            X() const __restrict;
    
    /// Accessor: Get Y
    NOALIAS __forceinline const HF32            Y() const __restrict;
    
    /// Accessor: Get Z
    NOALIAS __forceinline const HF32            Z() const __restrict;
    
    /// Accessor: Set X
    NOALIAS __forceinline void                  X( const HF32 PCREF x ) __restrict;
    
    /// Accessor: Set Y
    NOALIAS __forceinline void                  Y( const HF32 PCREF y ) __restrict;
    
    /// Accessor: Set Z
    NOALIAS __forceinline void                  Z( const HF32 PCREF z ) __restrict;
    
    /// Element Zeroing Functions
    NOALIAS __forceinline void                  ZeroX() __restrict;
    NOALIAS __forceinline void                  ZeroY() __restrict;
    NOALIAS __forceinline void                  ZeroZ() __restrict;
    NOALIAS __forceinline void                  ZeroXY() __restrict;
    NOALIAS __forceinline void                  ZeroXZ() __restrict;
    NOALIAS __forceinline void                  ZeroYZ() __restrict;
    
    /// Return copy with elements zeroed
    NOALIAS __forceinline const C_Vector        ZeroedX() const __restrict;
    NOALIAS __forceinline const C_Vector        ZeroedY() const __restrict;
    NOALIAS __forceinline const C_Vector        ZeroedZ() const __restrict;
    NOALIAS __forceinline const C_Vector        ZeroedXY() const __restrict;
    NOALIAS __forceinline const C_Vector        ZeroedXZ() const __restrict;
    NOALIAS __forceinline const C_Vector        ZeroedYZ() const __restrict;
    
    /// Negation: Vector --> Vector
    NOALIAS __forceinline const C_Vector        operator - () const __restrict;
    
    /// Addition: Vector + Vector --> Vector
    NOALIAS __forceinline const C_Vector        operator + ( const C_Vector PCREF src ) const __restrict;
    
    /// Addition: Vector + Normal --> Vector
    NOALIAS __forceinline const C_Vector        operator + ( const C_Normal PCREF src ) const __restrict;
    
    /// Addition: Vector += Vector
    NOALIAS __forceinline void                  operator += ( const C_Vector PCREF src ) __restrict;
    
    /// Addition: Vector += Normal
    NOALIAS __forceinline void                  operator += ( const C_Normal PCREF src ) __restrict;
    
    /// Addition: Vector + Vertex --> Vertex
    NOALIAS __forceinline const C_Vertex        operator + ( const C_Vertex PCREF src ) const __restrict;
    
    /// Addition: Vector + Origin --> Vertex
    NOALIAS __forceinline const C_Vertex        operator + ( const C_Origin PCREF src ) const __restrict;
    
    /// Subtraction: Vector - Vector --> Vector
    NOALIAS __forceinline const C_Vector        operator - ( const C_Vector PCREF src ) const __restrict;
    
    /// Subtraction: Vector -= Vector
    NOALIAS __forceinline void                  operator -= ( const C_Vector PCREF src ) __restrict;
    
    /// Subtraction: Vector -= Normal
    NOALIAS __forceinline void                  operator -= ( const C_Normal PCREF src ) __restrict;
    
    /// Multiplication: Vector * Vector --> Vector
    NOALIAS __forceinline const C_Vector        operator * ( const C_Vector PCREF src ) const __restrict;
    
    /// Multiplication: Vector * HF32 --> Vector
    //NOALIAS __forceinline const C_Vector        operator * ( const HF32 PCREF scalar ) const __restrict;
    
    /// Multiplication: Vector *= HF32
    NOALIAS __forceinline void                  operator *= ( const HF32 PCREF scalar ) __restrict;
    
    /// Division: Vector / HF32 --> Vector
    //NOALIAS __forceinline const C_Vector        operator / ( const HF32 PCREF scalar ) const __restrict;
    
    /// Division: Vector /= HF32
    NOALIAS __forceinline void                  operator /= ( const HF32 PCREF scalar ) __restrict;
    
    /// Multiplication: Vector * HMat33 --> Vector
    NOALIAS __forceinline const C_Vector        operator * ( const HMat33& matrix ) const __restrict;
    
    /// Multiplication: Vector * C_Quaternion --> Vector
    //const C_Vector              operator * ( const C_Quaternion& q ) const;
    
    /// Dot Product: Vector | Vector --> HF32
    NOALIAS __forceinline const HF32            operator | ( const C_Vector PCREF src ) const __restrict;
    
    /// Dot Product: Vector | Normal --> HF32
    NOALIAS __forceinline const HF32            operator | ( const C_Normal PCREF src ) const __restrict;
    
    /// Cross Product: Vector ^ Vector --> Vector
    NOALIAS __forceinline const C_Vector        operator ^ ( const C_Vector PCREF src ) const __restrict;
    
    /// Cross Product: Vector ^ Normal --> Vector
    NOALIAS __forceinline const C_Vector        operator ^ ( const C_Normal PCREF src ) const __restrict;
    
    /// Star function = Cross Product Matrix ( a* Matrix )
    NOALIAS __forceinline const HMat33          Star() const __restrict;
    
    /// Normalised
    NOALIAS __forceinline const C_Normalised    Normalised() const __restrict;
    
    /// Normalised
    NOALIAS __forceinline const C_Normalised    SafeNormalised( const C_Normal PCREF zeroLengthResult = C_Normal() ) const __restrict;
    
    /// Vector Length
    NOALIAS __forceinline const HF32            Length() const __restrict;
    
    /// Vector Length Squared
    NOALIAS __forceinline const HF32            LengthSquared() const __restrict;
    
    /// Vector Reciprocal of Length
    NOALIAS __forceinline const HF32            ReciprocalLength() const __restrict;
    
    /// Get component of this vector parallel to input normal
    NOALIAS __forceinline const C_Vector        Parallel( const C_Normal PCREF normal ) const __restrict;
    
    /// Get component of this vector perpendicular to input normal
    NOALIAS __forceinline const C_Vector        Perpendicular( const C_Normal PCREF normal ) const __restrict;
    
    /// Get components of this vector parallel & perpendicular to input normal
    NOALIAS __forceinline void                  ParaPerp( const C_Normal PCREF normal, C_Vector& result_para, C_Vector& result_perp ) const __restrict;
    
    /// Comparison - Equal
    NOALIAS __forceinline const U32				operator == ( const C_Vector PCREF src ) const __restrict;
    
    /// Comparison - Not Equal
    NOALIAS __forceinline const U32				operator != ( const C_Vector PCREF src ) const __restrict;
    
    /// Comparison - Less Than
    /*NOALIAS __forceinline const Vector3_Bool    operator < ( const C_Vector PCREF src ) const __restrict;
    
    /// Comparison - Less Equal
    NOALIAS __forceinline const Vector3_Bool    operator <= ( const C_Vector PCREF src ) const __restrict;
    
    /// Comparison - Greater Than
    NOALIAS __forceinline const Vector3_Bool    operator > ( const C_Vector PCREF src ) const __restrict;
    
    /// Comparison - Greater Equal
    NOALIAS __forceinline const Vector3_Bool    operator >= ( const C_Vector PCREF src ) const __restrict;*/
    
    /// Finite Test
    NOALIAS __forceinline const Scalar_Bool     Finite() const __restrict;
    
    /// Read from Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Serialise( T_Stream& stream ) __restrict;

	NOALIAS __forceinline HVec3                 GetHVec() const __restrict { return m_V; }
    
public:
    
    HVec3   m_V;
};


//NOALIAS __forceinline const C_Vector operator * ( const C_Vector PCREF v, const F32& f );
//NOALIAS __forceinline const C_Vector operator / ( const C_Vector PCREF v, const F32& f );

//NOALIAS __forceinline void operator *= ( C_Vector& v, const F32& f );
//NOALIAS __forceinline void operator /= ( C_Vector& v, const F32& f );




//
// ----------------------------------------------------
// |-- C_VectorLength : HVec3 vector + HVecW length --|
// ----------------------------------------------------
//

class C_VectorLength
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline C_VectorLength();
    
    /// Copy Constructor
    NOALIAS __forceinline C_VectorLength( const C_VectorLength& copy );
    
    /// Copy Assignment
    NOALIAS __forceinline const C_VectorLength& operator = ( const C_VectorLength& copy );
    
    /// C_Vector Constructor
    NOALIAS __forceinline C_VectorLength( const C_Vector PCREF vector );
    
    /// C_Normal + length Constructor
    NOALIAS __forceinline C_VectorLength( const C_Normal PCREF normal, const HF32 PCREF length );
    
    /// Vector Accessor
    NOALIAS __forceinline const C_Vector      Vector() const __restrict;
    
    /// Length Accessor
    NOALIAS __forceinline const HF32          Length() const __restrict;
    
    /// Read from Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Serialise( T_Stream& stream ) __restrict;
    
public:
    /// Protected HVec4 Constructor
    NOALIAS __forceinline explicit C_VectorLength( const HVec4 PCREF src );
    
    HVec4               m_VectorLength;
};


//
// -----------------------------------------------------------------|
// |-- C_VertexRcpLength : HVec3 vertex + HVecW reciprocal length --|
// -----------------------------------------------------------------|
//

class C_VertexRcpLength
{
friend class C_Line;

public:
    
    /// Default Constructor
    NOALIAS __forceinline C_VertexRcpLength();
    
    /// Copy Constructor
    NOALIAS __forceinline C_VertexRcpLength( const C_VertexRcpLength& copy );
    
    /// Copy Assignment
    NOALIAS __forceinline const C_VertexRcpLength& operator = ( const C_VertexRcpLength& copy );
    
    /// C_Vertex + reciprocal length Constructor
    NOALIAS __forceinline C_VertexRcpLength( const C_Vertex PCREF vertex, const HF32 PCREF rcpLength );
    
    /// Vertex Accessor
    NOALIAS __forceinline const C_Vertex      Vertex() const __restrict;
    
    /// Reciprocal Length Accessor
    NOALIAS __forceinline const HF32          RcpLength() const __restrict;
    
    /// Read from Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Serialise( T_Stream& stream ) __restrict;
    
public:
    
    HVec4               m_VertexRcpLength;
};


//
// -------------------------------------------------------------------------------------------------------
// |-- C_PackedNormalLength : HVec3 normal + HVecW length, packed into a single 4 element float vector --|
// -------------------------------------------------------------------------------------------------------
//

class C_PackedNormalLength
{
friend class C_Line;

public:
    
    /// Default Constructor
    NOALIAS __forceinline C_PackedNormalLength();
    
    /// Copy Constructor
    NOALIAS __forceinline C_PackedNormalLength( const C_PackedNormalLength& copy );
    
    /// Copy Assignment
    NOALIAS __forceinline const C_PackedNormalLength& operator = ( const C_PackedNormalLength& copy );
    
    /// C_Vector Constructor
    NOALIAS __forceinline C_PackedNormalLength( const C_Vector PCREF vector );
    
    /// C_Normal + length Constructor
    NOALIAS __forceinline C_PackedNormalLength( const C_Normal PCREF normal, const HF32 PCREF length );
    
    /// Normal Accessor
    NOALIAS __forceinline const C_Normal      Normal() const __restrict;
    
    /// Vector Accessor
    NOALIAS __forceinline const C_Vector      Vector() const __restrict;
    
    /// Length Accessor
    NOALIAS __forceinline const HF32          Length() const __restrict;
    
    /// Read from Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Serialise( T_Stream& stream ) __restrict;
    
public:
    
    HVec4               m_NormalLength;
};


//
// ----------------------------------------------------------------------
// |-- C_Normalised : HVec3 normal + HF32 length, length^2 & 1/length --|
// ----------------------------------------------------------------------
//

class C_Normalised
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline C_Normalised();
    
    /// Undefined Constructor
    NOALIAS __forceinline C_Normalised( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS __forceinline C_Normalised( const C_Normalised& copy );
    
    /// Copy Assignment
    NOALIAS __forceinline const C_Normalised& operator = ( const C_Normalised& copy );
    
    /// C_Normal + HF32 length, length^2 & 1/length Constructor
    NOALIAS __forceinline C_Normalised( const C_Normal PCREF normal, const HF32 PCREF length, const HF32 PCREF length2, const HF32 PCREF rcpLength );
    
    /// C_Normal Cast
    NOALIAS __forceinline operator const C_Normal () const __restrict;
    
    /// Normal Accessor
    NOALIAS __forceinline const C_Normal      Normal() const __restrict;
    
    /// Length Accessor
    NOALIAS __forceinline const HF32          Length() const __restrict;
    
    /// Length Squared Accessor
    NOALIAS __forceinline const HF32          Length2() const __restrict;
    
    /// Reciprocal Length Accessor
    NOALIAS __forceinline const HF32          RcpLength() const __restrict;
    
    /// Read from Stream
    //template < typename T_Stream > __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    //template < typename T_Stream > __forceinline void Serialise( T_Stream& stream ) __restrict;
    
public:
    
    C_Normal            m_Normal;
    HF32                m_Length;
    HF32                m_Length2;
    HF32                m_RcpLength;
};


//
// ---------------------------------------------------------------------
// |-- Trilinear coordinates for a vertex position within a triangle --|
// ---------------------------------------------------------------------
//

class C_Trilinear
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline C_Trilinear();
    
    /// Copy Constructor
    NOALIAS __forceinline C_Trilinear( const C_Trilinear& copy );
    
    /// Copy Assignment
    NOALIAS __forceinline const C_Trilinear& operator = ( const C_Trilinear& copy );
    
    /// HVec3 Constructor
    NOALIAS __forceinline C_Trilinear( const HVec3 PCREF src );
    
    /// HF32 XYZ Constructor
    NOALIAS __forceinline C_Trilinear( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF c );
    
    /// Explicit F32 Array Constructor
    NOALIAS __forceinline explicit C_Trilinear( const F32* xyz );
    
    /// HVec3 Cast
    NOALIAS __forceinline operator const HVec3 () const __restrict;
    
    /// Accessor: Alpha
    NOALIAS __forceinline const HF32                  Alpha() const __restrict;
    
    /// Accessor: Beta
    NOALIAS __forceinline const HF32                  Beta() const __restrict;
    
    /// Accessor: Gamma
    NOALIAS __forceinline const HF32                  Gamma() const __restrict;
    
    /// Coordinates Inside Triangle Test
    NOALIAS __forceinline const Scalar_Bool           Inside() const __restrict;
    
    /// Finite test
    NOALIAS __forceinline const Scalar_Bool           Finite() const __restrict;
    
    /// Read from Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Serialise( T_Stream& stream ) __restrict;
    
public:
    
    HVec3   m_V;
};


//
// -----------------------------------------------------------------------------
// |-- Areal Barycentric coordinates for a vertex position within a triangle --|
// -----------------------------------------------------------------------------
//

class C_Areal
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline C_Areal();
    
    /// Copy Constructor
    NOALIAS __forceinline C_Areal( const C_Areal& copy );
    
    /// Copy Assignment
    NOALIAS __forceinline const C_Areal& operator = ( const C_Areal& copy );
    
    /// HVec3 Constructor
    NOALIAS __forceinline C_Areal( const HVec3 PCREF src );
    
    /// HF32 XYZ Constructor
    NOALIAS __forceinline C_Areal( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF c );
    
    /// Explicit F32 Array Constructor
    NOALIAS __forceinline explicit C_Areal( const F32* xyz );
    
    /// C_Trilinear & scale constructor
    NOALIAS __forceinline C_Areal( const C_Trilinear PCREF trilinear, const HVec3 PCREF trilinearToAreal );
    
    /// HVec3 Cast
    NOALIAS __forceinline operator const HVec3 () const __restrict;
    
    /// Accessor: Alpha
    NOALIAS __forceinline const HF32                  Alpha() const __restrict;
    
    /// Accessor: Beta
    NOALIAS __forceinline const HF32                  Beta() const __restrict;
    
    /// Accessor: Gamma
    NOALIAS __forceinline const HF32                  Gamma() const __restrict;
    
    /// Finite test
    NOALIAS __forceinline const Scalar_Bool           Finite() const __restrict;
    
    /// Read from Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Deserialise( T_Stream& stream ) __restrict;
    
    /// Write to Stream
    template < typename T_Stream >
    NOALIAS __forceinline void Serialise( T_Stream& stream ) __restrict;
    
public:
    
    HVec3   m_V;
};












//
// -------------------------
// |-- Support Functions --|
// -------------------------
//

/// TODO: Make all vector classes which these functions can work on inherit a base class to indicate they support these functions!

/// Selector: Result = ( selector ? resultIfTrue : resultIfFalse )
NOALIAS __forceinline const HVec2 Select( const Vector2_Bool PCREF selector, const HVec2 PCREF resultIfTrue, const HVec2 PCREF resultIfFalse );

/// Selector: Result = ( selector ? resultIfTrue : resultIfFalse )
NOALIAS __forceinline const HVec3 Select( const Vector3_Bool PCREF selector, const HVec3 PCREF resultIfTrue, const HVec3 PCREF resultIfFalse );

/// Selector: Result = ( selector ? resultIfTrue : resultIfFalse )
NOALIAS __forceinline const HVec4 Select( const Vector4_Bool PCREF selector, const HVec4 PCREF resultIfTrue, const HVec4 PCREF resultIfFalse );

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? a : b )
NOALIAS __forceinline const C_Vertex Select( const Vector3_Bool PCREF selector, const C_Vertex PCREF a, const C_Vertex PCREF b );

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? a : b )
NOALIAS __forceinline const C_Vector Select( const Vector3_Bool PCREF selector, const C_Vector PCREF a, const C_Vector PCREF b );

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? a : b )
NOALIAS __forceinline const C_Normal Select( const Scalar_Bool PCREF selector, const C_Normal PCREF a, const C_Normal PCREF b );



/// Refelct vector about normal
NOALIAS __forceinline const HVec3 Reflect( const HVec3 PCREF vector, const C_Normal PCREF normal );



/// Returns the average of the elements of a Vector
template < class T_Vector >
NOALIAS __forceinline const HF32 Average( const T_Vector PCREF src );

/// Returns the length of a Vector
template < class T_Vector >
NOALIAS __forceinline const HF32 Length( const T_Vector PCREF src );

/// Returns the squared length of a Vector
template < class T_Vector >
NOALIAS __forceinline const HF32 Length2( const T_Vector PCREF src );

/// Returns the squared length of a Vector
//template < class T_Vector >
//const HF32 LengthSquared( const T_Vector& src );

/// Returns the reciprocal of the length of a Vector
template < class T_Vector >
NOALIAS __forceinline const HF32 RcpLength( const T_Vector PCREF src );

/// Returns the reciprocal of the length of a Vector
//template < class T_Vector >
//const HF32 ReciprocalLength( const T_Vector& src );

/// Returns a normalised vector
//template < class T_Vector >
//const C_Normal Normalised( const T_Vector PCREF src );

/// Returns a normalised vector - safe version with additional parameter to use for zero length vectors!
template < class T_Vector >
NOALIAS __forceinline const C_Normal SafeNormalise( const T_Vector PCREF src, const C_Normal PCREF zeroLengthResult = C_Normal() );

/// Returns the dot product of two vectors
template < class T_Vector >
NOALIAS __forceinline const HF32 DotProduct( const T_Vector PCREF lhs, const T_Vector PCREF rhs );

/// Returns the cross product of two vectors
template < class T_Vector >
NOALIAS __forceinline const typename T_Vector::CrossProductType CrossProduct( const T_Vector PCREF lhs, const T_Vector PCREF rhs );

/// Shuffle
//template < unsigned SX, unsigned SY, unsigned SZ, class T_Vector3 >
//const T_Vector3 Shuffle( const T_Vector3& src );

/// Horizontal sum of all elements
template < class T_Vector >
NOALIAS __forceinline const HF32 Sum( const T_Vector PCREF src );

/// Box Half Surface Area : Vector represents dimensions of box
//template < class T_Vector3 >
//const HF32 BoxHalfSurfaceArea( const T_Vector3& dimensionsXYZ );
NOALIAS __forceinline const HF32 BoxHalfSurfaceArea( const HVec3 PCREF dimensionsXYZ );

/// Box Surface Area : Vector represents dimensions of box
template < class T_Vector3 >
NOALIAS __forceinline const HF32 BoxSurfaceArea( const T_Vector3 PCREF dimensionsXYZ );

/// Returns the bitwise AND of the two inputs: lhs & rhs
template < class T_Vector >
NOALIAS __forceinline const T_Vector AND( const T_Vector PCREF lhs, const T_Vector PCREF rhs );

/// Returns the bitwise AND-NOT of the two inputs: lhs & ~rhs
template < class T_Vector >
NOALIAS __forceinline const T_Vector ANDNOT( const T_Vector PCREF lhs, const T_Vector PCREF rhs );

/// Returns the bitwise OR of the two inputs: lhs | rhs
template < class T_Vector >
NOALIAS __forceinline const T_Vector OR( const T_Vector PCREF lhs, const T_Vector PCREF rhs );

/// Returns the bitwise XOR of the two inputs: lhs ^ rhs
template < class T_Vector >
NOALIAS __forceinline const T_Vector XOR( const T_Vector PCREF lhs, const T_Vector PCREF rhs );

/// Returns the transpose of the matrix
template < class T_Matrix >
NOALIAS __forceinline const T_Matrix Transpose( const T_Matrix& src );

/// Returns the inverse of the matrix
template < class T_Matrix >
NOALIAS __forceinline const T_Matrix Inverse( const T_Matrix& src );

/// Returns the transpose of the inverse of a matrix ( for transforming normals ).
/// INFO: http://www.gignews.com/realtime020100.htm
template < class T_Matrix >
NOALIAS __forceinline const T_Matrix TransposeInverse( const T_Matrix& src );



// Matrix multiplication of a column vector (lhs) by a row vector (rhs).
NOALIAS __forceinline const HMat33 TensorProduct( const HVec3 PCREF columnVector, const HVec3 PCREF rowVector );




/// Calculate normal, length, length^2 & 1/length of vector
NOALIAS __forceinline void NormalLength( const HVec3 PCREF vector, C_Normal& out_normal, HF32& out_length, HF32& out_length_squared, HF32& out_rcp_length );




//
// ---------------------------------------------------------------------------
// |-- HVec3_DelayedOp : Pair of HVec3's to be multiplied/added/subtracted --|
// |-- Used for collapsing a multiply & add into a multiply-add etc        --|
// ---------------------------------------------------------------------------
//

template < typename T_Op >
class ALIGN16 DARWIN64_ABI HVec3_DelayedOp
{
public:
    
    /// Accessor: Get X
    NOALIAS __forceinline HF32                        X() const;
    
    /// Accessor: Get Y
    NOALIAS __forceinline HF32                        Y() const;
    
    /// Accessor: Get Z
    NOALIAS __forceinline HF32                        Z() const;
    
    /// Accessor: Get W - Required for templated accessors - should never be used!
    NOALIAS __forceinline HF32                        W() const;
    
    //#if defined( _SIMD_ )
    ///// Accessor: Get W
    //HF32                        TestW() const;
    //#endif
    
    /// Shuffle
    template < unsigned IX, unsigned IY, unsigned IZ >
    NOALIAS __forceinline HVec3                       Shuffle() const;
    
    /// Returns copy with zeroed element X
    NOALIAS __forceinline HVec3                       ZeroedX() const;
    
    /// Returns copy with zeroed element Y
    NOALIAS __forceinline HVec3                       ZeroedY() const;
    
    /// Returns copy with zeroed element Z
    NOALIAS __forceinline HVec3                       ZeroedZ() const;
    
    /// Returns copy with zeroed elements XY
    NOALIAS __forceinline HVec3                       ZeroedXY() const;
    
    /// Returns copy with zeroed elements XZ
    NOALIAS __forceinline HVec3                       ZeroedXZ() const;
    
    /// Returns copy with zeroed elements YZ
    NOALIAS __forceinline HVec3                       ZeroedYZ() const;
    
    /// Unary Negation Operator
    NOALIAS __forceinline HVec3                       operator - () const;
    
    /// Binary Addition Operator
    NOALIAS __forceinline HVec3_Add                   operator + ( const HVec3 PCREF src ) const;
    
    /// Binary Subtraction Operator
    NOALIAS __forceinline HVec3_Subtract              operator - ( const HVec3 PCREF src ) const;
    
    /// Binary Multiplication Operator
    NOALIAS __forceinline HVec3_Multiply              operator * ( const HVec3 PCREF src ) const;
    
    /// Binary Division Operator
    NOALIAS __forceinline HVec3                       operator / ( const HVec3 PCREF src ) const;
    
    /// HF32 Multiplication Operator
    NOALIAS __forceinline HVec3                       operator * ( const HF32 PCREF src ) const;
    
    /// HF32 Division Operator
    NOALIAS __forceinline HVec3                       operator / ( const HF32 PCREF src ) const;
    
    /// HVecX Multiply Operator
    NOALIAS __forceinline HVec3                       operator * ( const HVecX PCREF src ) const;
    
    /// HVecY Multiply Operator
    NOALIAS __forceinline HVec3                       operator * ( const HVecY PCREF src ) const;
    
    /// HVecZ Multiply Operators
    NOALIAS __forceinline HVec3                       operator * ( const HVecZ PCREF src ) const;
    
    /// HVecW Multiply Operators
    NOALIAS __forceinline HVec3                       operator * ( const HVecW PCREF src ) const;
    
    /// Min
    NOALIAS __forceinline HVec3                       Min( const HVec3 PCREF src ) const;
    
    /// Max
    NOALIAS __forceinline HVec3                       Max( const HVec3 PCREF src ) const;
    
    /// Reciprocal of elements
    NOALIAS __forceinline HVec3                       Reciprocal() const;
    
    /// Horizontal sum of all elements
    NOALIAS __forceinline HF32                        Sum() const;
    
    /// Vector Length
    NOALIAS __forceinline HF32                        Length() const;
    
    /// Vector Length Squared
    NOALIAS __forceinline HF32                        Length2() const;
    NOALIAS __forceinline HF32                        LengthSquared() const;
    
    /// Vector Reciprocal of Length
    NOALIAS __forceinline HF32                        RcpLength() const;
    NOALIAS __forceinline HF32                        ReciprocalLength() const;
    
    /// Dot product
    NOALIAS __forceinline HF32                        operator | ( const HVec3 PCREF r ) const;
    
    /// Cross Product
    NOALIAS __forceinline HVec3                       operator ^ ( const HVec3 PCREF r ) const;
    
    /// Star function = Cross Product Matrix ( a* Matrix )
    NOALIAS __forceinline HMat33                      Star() const;
    
    /// HMat33 Multiplication
    NOALIAS __forceinline HVec3                       operator * ( const HMat33& m ) const;
    
    /// HMat43 Multiplication
    NOALIAS __forceinline HVec3                       operator * ( const HMat43& m ) const;
    
    /// HMat44 Multiplication
    NOALIAS __forceinline HVec3                       operator * ( const HMat44& m ) const;
    
    /// Quaternion Multiplication
    NOALIAS __forceinline HVec3                       operator * ( const C_Quaternion PCREF q ) const;
    
    /// Vertex Distance
    NOALIAS __forceinline HF32                        Distance( const HVec3 PCREF v ) const;
    
    /// Vertex Distance Squared
    NOALIAS __forceinline HF32                        DistanceSquared( const HVec3 PCREF v ) const;
    
    /// Lerp
    NOALIAS __forceinline HVec3                       Lerp( const HVec3 PCREF dst, const HF32 PCREF f ) const;
    
    /// Vector Normalised
    NOALIAS __forceinline C_Normalised                Normalised() const;
    
    /// Vector Normalised - safe version with additional parameter to use for zero length vectors!
    NOALIAS __forceinline C_Normalised                SafeNormalised( const HVec3 PCREF zeroLengthResult = HVec3() ) const;
    
    /// Get Vector of Magnitude
    NOALIAS __forceinline HVec3                       VectorOfMagnitude( const HF32 PCREF mag ) const;
    
    /// Get component of this vector parallel to input normal
    NOALIAS __forceinline HVec3                       Parallel( const C_Normal PCREF normal ) const;
    
    /// Get component of this vector perpendicular to input normal
    NOALIAS __forceinline HVec3                       Perpendicular( const C_Normal PCREF normal ) const;
    
    /// Get components of this vector parallel & perpendicular to input normal
    NOALIAS __forceinline void                        ParaPerp( const C_Normal PCREF normal, HVec3& result_para, HVec3& result_perp ) const;
    
    /// Get axes perpendicular to vector.
    NOALIAS __forceinline void                        PerpendicularAxes( HVec3& result_u, HVec3& result_v ) const;
    
    /// Shuffle HVec3:XZ into a HVec2:XY
    NOALIAS __forceinline HVec2                       ShuffleXZtoXY() const;
    
    /// Comparison Operator ==
    NOALIAS __forceinline Vector3_Bool           operator == ( const HVec3 PCREF src ) const;
    
    /// Comparison Operator !=
    NOALIAS __forceinline Vector3_Bool           operator != ( const HVec3 PCREF src ) const;
    
    /// Comparison Operator <=
    NOALIAS __forceinline Vector3_Bool           operator <= ( const HVec3 PCREF src ) const;
    
    /// Comparison Operator <
    NOALIAS __forceinline Vector3_Bool           operator < ( const HVec3 PCREF src ) const;
    
    /// Comparison Operator >=
    NOALIAS __forceinline Vector3_Bool           operator >= ( const HVec3 PCREF src ) const;
    
    /// Comparison Operator >
    NOALIAS __forceinline Vector3_Bool           operator > ( const HVec3 PCREF src ) const;
    
    /// Comparison Function ==
    NOALIAS __forceinline U32                         EQ( const HVec3 PCREF src ) const;
    
    /// Comparison Function !=
    NOALIAS __forceinline U32                         NE( const HVec3 PCREF src ) const;
    
    /// Comparison Function <=
    NOALIAS __forceinline U32                         LE( const HVec3 PCREF src ) const;
    
    /// Comparison Function <
    NOALIAS __forceinline U32                         LT( const HVec3 PCREF src ) const;
    
    /// Comparison Function >=
    NOALIAS __forceinline U32                         GE( const HVec3 PCREF src ) const;
    
    /// Comparison Function >
    NOALIAS __forceinline U32                         GT( const HVec3 PCREF src ) const;
    
    /// Bitwise AND: THIS & SRC
    NOALIAS __forceinline HVec3                       AND( const HVec3 PCREF src ) const;
    
    /// Bitwise AND-NOT: THIS & ~SRC
    NOALIAS __forceinline HVec3                       ANDNOT( const HVec3 PCREF src ) const;
    
    /// Bitwise OR: THIS | SRC
    NOALIAS __forceinline HVec3                       OR( const HVec3 PCREF src ) const;
    
    /// Bitwise XOR: THIS ^ SRC
    NOALIAS __forceinline HVec3                       XOR( const HVec3 PCREF src ) const;
    
public:
    
    /// Accessor: Get reference to sub-type
    NOALIAS __forceinline T_Op&         SubType();
    
    /// Accessor: Get const reference to sub-type
    NOALIAS __forceinline const T_Op&   SubType() const;
};


//
// --------------------------------------------------------------------
// |-- HVec3_Multiply : Pair of HVec3's to be multiplied            --|
// |-- Used for collapsing a multiply & add into a multiply-add etc --|
// --------------------------------------------------------------------
//

class ALIGN16 DARWIN64_ABI HVec3_Multiply : public HVec3_DelayedOp< HVec3_Multiply >
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline HVec3_Multiply();
    
    /// HVec3 Pair Constructor
    NOALIAS __forceinline HVec3_Multiply( const HVec3 PCREF a, const HVec3 PCREF b );
    
    /// Copy Constructor
    NOALIAS __forceinline HVec3_Multiply( const HVec3_Multiply& copy );
    
    /// Copy Assignment
    NOALIAS __forceinline const HVec3_Multiply& operator = ( const HVec3_Multiply& copy );
    
public:
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector float    m_A;
    vector float    m_B;
    #elif defined( _VMX128_ )
    V128            m_A;
    V128            m_B;
    #else
    HVec3           m_A;
    HVec3           m_B;
    #endif
};


//
// --------------------------------------------------------------------
// |-- HVec3_Add : Pair of HVec3's to be added                      --|
// |-- Used for collapsing a multiply & add into a multiply-add etc --|
// --------------------------------------------------------------------
//

class ALIGN16 DARWIN64_ABI HVec3_Add : public HVec3_DelayedOp< HVec3_Add >
{
//friend class HVec3;
//friend class C_Vertex_Add;

public:
    
    /// Default Constructor
    NOALIAS __forceinline HVec3_Add();
    
    /// HVec3 Pair Constructor
    NOALIAS __forceinline HVec3_Add( const HVec3 PCREF a, const HVec3 PCREF b );
    
    /// Copy Constructor
    NOALIAS __forceinline HVec3_Add( const HVec3_Add& copy );
    
    /// Copy Assignment
    NOALIAS __forceinline const HVec3_Add& operator = ( const HVec3_Add& copy );
    
public:
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector float    m_A;
    vector float    m_B;
    #elif defined( _VMX128_ )
    V128            m_A;
    V128            m_B;
    #else
    HVec3           m_A;
    HVec3           m_B;
    #endif
};


//
// --------------------------------------------------------------------
// |-- HVec3_Subtract : Pair of HVec3's to be subtracted            --|
// |-- Used for collapsing a multiply & add into a multiply-add etc --|
// --------------------------------------------------------------------
//

class ALIGN16 DARWIN64_ABI HVec3_Subtract : public HVec3_DelayedOp< HVec3_Subtract >
{
//friend class HVec3;
//friend class C_Vertex_Subtract;

public:
    
    /// Default Constructor
    NOALIAS __forceinline HVec3_Subtract();
    
    /// HVec3 Pair Constructor
    NOALIAS __forceinline HVec3_Subtract( const HVec3 PCREF a, const HVec3 PCREF b );
    
    /// Copy Constructor
    NOALIAS __forceinline HVec3_Subtract( const HVec3_Subtract& copy );
    
    /// Copy Assignment
    NOALIAS __forceinline const HVec3_Subtract& operator = ( const HVec3_Subtract& copy );
    
public:
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector float    m_A;
    vector float    m_B;
    #elif defined( _VMX128_ )
    V128            m_A;
    V128            m_B;
    #else
    HVec3           m_A;
    HVec3           m_B;
    #endif
};


// --------------------------------------------------------------------
// |-- HVec3_Multiply/Add/Subtract operators                        --|
// |-- Used for collapsing a multiply & add into a multiply-add etc --|
// --------------------------------------------------------------------

/// HVec3 Multiply-Add operator: a * b + c
NOALIAS __forceinline HVec3 operator + ( const HVec3_Multiply& ab, const HVec3 PCREF c );

/// HVec3 Add-Multiply operator: a + b * c
NOALIAS __forceinline HVec3 operator + ( const HVec3 PCREF a, const HVec3_Multiply& bc );

/// HVec3 Multiply-Subtract operator: a * b - c
NOALIAS __forceinline HVec3 operator - ( const HVec3_Multiply& ab, const HVec3 PCREF c );

/// HVec3 Subtract-Multiply operator: a - b * c
NOALIAS __forceinline HVec3 operator - ( const HVec3 PCREF a, const HVec3_Multiply& bc );

/// HVec3 operator: ( a * b ) + ( c * d ) : Need this to resolve ambiguity!
NOALIAS __forceinline HVec3 operator + ( const HVec3_Multiply& ab, const HVec3_Multiply& cd );

/// HVec3 operator: ( a * b ) - ( c * d ) : Need this to resolve ambiguity!
NOALIAS __forceinline HVec3 operator - ( const HVec3_Multiply& ab, const HVec3_Multiply& cd );

/// HVec3 ( a + b ) + ( c * d ) : Need this to resolve ambiguity!
NOALIAS __forceinline HVec3 operator + ( const HVec3_Add& ab, const HVec3_Multiply& cd );

/// HVec3 ( a - b ) + ( c * d ) : Need this to resolve ambiguity!
NOALIAS __forceinline HVec3 operator + ( const HVec3_Subtract& ab, const HVec3_Multiply& cd );

/// HVec3 ( a + b ) - ( c * d ) : Need this to resolve ambiguity!
NOALIAS __forceinline HVec3 operator - ( const HVec3_Add& ab, const HVec3_Multiply& cd );

/// HVec3 ( a - b ) - ( c * d ) : Need this to resolve ambiguity!
NOALIAS __forceinline HVec3 operator - ( const HVec3_Subtract& ab, const HVec3_Multiply& cd );



NOALIAS __forceinline HVec3 operator * ( const HVec3_Multiply& v, const HF32 PCREF f );
NOALIAS __forceinline HVec3 operator / ( const HVec3_Multiply& v, const HF32 PCREF f );

NOALIAS __forceinline HVec3 operator * ( const HVec3_Add& v, const HF32 PCREF f );
NOALIAS __forceinline HVec3 operator / ( const HVec3_Add& v, const HF32 PCREF f );

NOALIAS __forceinline HVec3 operator * ( const HVec3_Subtract& v, const HF32 PCREF f );
NOALIAS __forceinline HVec3 operator / ( const HVec3_Subtract& v, const HF32 PCREF f );



NOALIAS __forceinline HVec3 operator * ( const HVec3_Multiply& v, const F32& f );
NOALIAS __forceinline HVec3 operator / ( const HVec3_Multiply& v, const F32& f );

NOALIAS __forceinline HVec3 operator * ( const HVec3_Add& v, const F32& f );
NOALIAS __forceinline HVec3 operator / ( const HVec3_Add& v, const F32& f );

NOALIAS __forceinline HVec3 operator * ( const HVec3_Subtract& v, const F32& f );
NOALIAS __forceinline HVec3 operator / ( const HVec3_Subtract& v, const F32& f );



/// Combine HVec3 with HF32 to form a HVec4
NOALIAS __forceinline const HVec4 operator || ( const HVec3 PCREF xyz, const HF32 PCREF w ) __attribute__(( always_inline ));

/// Combine HF32 with HVec3 to form a HVec4
NOALIAS __forceinline const HVec4 operator || ( const HF32 PCREF x, const HVec3 PCREF yzw ) __attribute__(( always_inline ));








//
// ---------------------------------------------------------------------------------
// |-- C_Vertex_DelayedOp : Pair of C_Vertex's to be multiplied/added/subtracted --|
// |-- Used for collapsing a multiply & add into a multiply-add etc              --|
// ---------------------------------------------------------------------------------
//

template < typename T_Op >
class ALIGN16 DARWIN64_ABI C_Vertex_DelayedOp
{
public:
    
public:
    
    /// Accessor: Get reference to sub-type
    NOALIAS __forceinline T_Op&                       SubType();
    
    /// Accessor: Get const reference to sub-type
    NOALIAS __forceinline const T_Op&                 SubType() const;
};


//
// --------------------------------------------------------------------
// |-- C_Vertex_Multiply : Pair of C_Vertex's to be multiplied      --|
// |-- Used for collapsing a multiply & add into a multiply-add etc --|
// --------------------------------------------------------------------
//
/*
class ALIGN16 DARWIN64_ABI C_Vertex_Multiply : public C_Vertex_DelayedOp< C_Vertex_Multiply >
{
//friend class C_Vertex;

public:
    
    /// C_Vertex Pair Constructor
    C_Vertex_Multiply( const C_Vertex PCREF a, const C_Vertex PCREF b );
    
    /// HVec3_Multiply Constructor
    C_Vertex_Multiply( const HVec3_Multiply& src );
    
    /// Copy Constructor
    C_Vertex_Multiply( const C_Vertex_Multiply& copy );
    
    /// Copy Assignment
    const C_Vertex_Multiply& operator = ( const C_Vertex_Multiply& copy );
    
    /// C_Vertex Cast operator
    operator C_Vertex () const;
    
public:
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector float    m_A;
    vector float    m_B;
    #elif defined( _VMX128_ )
    V128            m_A;
    V128            m_B;
    #else
    C_Vertex        m_A;
    C_Vertex        m_B;
    #endif
};*/


//
// --------------------------------------------------------------------
// |-- C_Vertex_Add : Pair of C_Vertex's to be added                --|
// |-- Used for collapsing a multiply & add into a multiply-add etc --|
// --------------------------------------------------------------------
//
/*
class ALIGN16 DARWIN64_ABI C_Vertex_Add : public C_Vertex_DelayedOp< C_Vertex_Add >
{
//friend class C_Vertex;

public:
    
    /// C_Vertex Pair Constructor
    C_Vertex_Add( const C_Vertex PCREF a, const C_Vertex PCREF b );
    
    /// HVec3_Add Constructor
    C_Vertex_Add( const HVec3_Add& src );
    
    /// Copy Constructor
    C_Vertex_Add( const C_Vertex_Add& copy );
    
    /// Copy Assignment
    const C_Vertex_Add& operator = ( const C_Vertex_Add& copy );
    
    /// C_Vertex Cast operator
    operator C_Vertex () const;
    
public:
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector float    m_A;
    vector float    m_B;
    #elif defined( _VMX128_ )
    V128            m_A;
    V128            m_B;
    #else
    C_Vertex        m_A;
    C_Vertex        m_B;
    #endif
};*/


//
// --------------------------------------------------------------------
// |-- C_Vertex_Subtract : Pair of C_Vertex's to be subtracted      --|
// |-- Used for collapsing a multiply & add into a multiply-add etc --|
// --------------------------------------------------------------------
//

class ALIGN16 DARWIN64_ABI C_Vertex_Subtract : public C_Vertex_DelayedOp< C_Vertex_Subtract >
{
//friend class C_Vertex;

public:
    
    /// C_Vertex Pair Constructor
    NOALIAS __forceinline C_Vertex_Subtract( const C_Vertex PCREF a, const C_Vertex PCREF b );
    
    /// HVec3_Subtract Constructor
    NOALIAS __forceinline C_Vertex_Subtract( const HVec3_Subtract& src );
    
    /// Copy Constructor
    NOALIAS __forceinline C_Vertex_Subtract( const C_Vertex_Subtract& copy );
    
    /// Copy Assignment
    NOALIAS __forceinline const C_Vertex_Subtract& operator = ( const C_Vertex_Subtract& copy );
    
    /// C_Vertex Cast operator
    NOALIAS __forceinline operator C_Vertex () const;
    
public:
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector float    m_A;
    vector float    m_B;
    #elif defined( _VMX128_ )
    V128            m_A;
    V128            m_B;
    #else
    C_Vertex        m_A;
    C_Vertex        m_B;
    #endif
};


NOALIAS __forceinline  void	VecSplatXYZ(const HVec4 PCREF srcIn, HVec4& xxxxOut, HVec4& yyyyOut, HVec4& zzzzOut);
NOALIAS __forceinline  void	VecTranspose(HVec4& dstAX, HVec4& dstAY, HVec4& dstAZ, HVec4& dstAW,
					                     const HVec4 PCREF srcAX, const HVec4 PCREF srcAY, const HVec4 PCREF srcAZ, const HVec4 PCREF srcAW);




NOALIAS __forceinline const bool finite( const HVec3 PCREF v )
{
      return _finite( v.X() ) &&
             _finite( v.Y() ) &&
             _finite( v.Z() );
}


























}; // namespace Maths

#endif // #ifndef MATHS_HVEC_VECTOR_H





