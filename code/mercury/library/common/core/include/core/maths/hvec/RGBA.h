#ifndef RGBA_H
#define RGBA_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

#if defined( _VMX128_ )
extern const __vector4 UnpackRGBA_ScaleOffset;
extern const __vector4 PackRGBA_Scale;
#endif


class C_RGBA_U1555;
class C_RGBA_U8;
class C_RGBA_F32;


//
// C_RGBA_U1555 Class : A:U1, R:U5, G:U5, B:U5
//

class C_RGBA_U1555
{
friend class C_RGBA_U8;
friend class C_RGBA_F32;

public:
    
    /// Default Constructor
    C_RGBA_U1555();
    
    /// Copy Constructor
    C_RGBA_U1555( const C_RGBA_U1555& copy );
    
    /// C_RGBA_U8 Constructor
    C_RGBA_U1555( const C_RGBA_U8& src );
    
    /// U8 RGBA Constructor
    C_RGBA_U1555( const U8 r, const U8 g, const U8 b, const U8 a = 0x1 );
    
    /// S32 RGBA Constructor
	C_RGBA_U1555( const S32 r, const S32 g, const S32 b, const S32 a = 0x1 );
    
    /// F32 RGBA Constructor
	C_RGBA_U1555( const F32 r, const F32 g, const F32 b, const F32 a );
    
    /// F32 RGB Constructor
	C_RGBA_U1555( const F32 r, const F32 g, const F32 b );
    
    /// Explicit Packed U32 Colour Constructor
    explicit C_RGBA_U1555( U16 rgba );
    
    /// Cast to Packed U16 Colour
    operator U16 () const;
    
    /// Copy Assignment
    const C_RGBA_U1555&     operator = ( const C_RGBA_U1555& copy );
    
    /// C_RGBA_U8 Assignment
    const C_RGBA_U1555&     operator = ( const C_RGBA_U8& src );
    
    /// Accessor: Get Alpha
    U8                      A() const;
    
    /// Accessor: Get Red
    U8                      R() const;
    
    /// Accessor: Get Green
    U8                      G() const;
    
    /// Accessor: Get Blue
    U8                      B() const;
    
    /// Accessor: Set Alpha
    void                    A( const U8 a );
    
    /// Accessor: Set Red
    void                    R( const U8 r );
    
    /// Accessor: Set Green
    void                    G( const U8 g );
    
    /// Accessor: Set Blue
    void                    B( const U8 b );
    
    /// Get Packed U16 Colour
    U16                     Colour() const;
    
protected:
    
    static const U32        BitsA = 1;
    static const U32        BitsR = 5;
    static const U32        BitsG = 5;
    static const U32        BitsB = 5;
    
    static const U32        ShiftA = 15;
    static const U32        ShiftR = 10;
    static const U32        ShiftG = 5;
    static const U32        ShiftB = 0;
    
    static const U32        MaskA = 0x8000;
    static const U32        MaskR = 0x7C00;
    static const U32        MaskG = 0x03E0;
    static const U32        MaskB = 0x001F;
    
    U16                     m_ARGB;
};



//
// C_RGBA_U8 Class : A:U8, R:U8, G:U8, B:U8
//

class C_RGBA_U8
{
friend class C_RGBA_U1555;
friend class C_RGBA_F32;

public:
    
    /// Default Constructor
    C_RGBA_U8();
    
    /// Copy Constructor
    C_RGBA_U8( const C_RGBA_U8& copy );
    
    /// C_RGBA_U1555 Constructor
    C_RGBA_U8( const C_RGBA_U1555& src );
    
    /// C_RGBA_F32 Constructor
    C_RGBA_U8( const C_RGBA_F32& src );
    
    /// U8 RGBA Constructor
    C_RGBA_U8( const U8 r, const U8 g, const U8 b, const U8 a = 0xFF );
    
    /// S32 RGBA Constructor
	C_RGBA_U8( const S32 r, const S32 g, const S32 b, const S32 a = 0xFF );
    
    /// F32 RGBA Constructor
	C_RGBA_U8( const F32 r, const F32 g, const F32 b, const F32 a );
    
    /// F32 RGB Constructor
	C_RGBA_U8( const F32 r, const F32 g, const F32 b );
    
    /// Explicit Packed U32 Colour Constructor
    explicit C_RGBA_U8( U32 rgba );
    
    /// Cast to Packed U32 Colour
    operator U32 () const;
    
    /// Copy Assignment
    const C_RGBA_U8&        operator = ( const C_RGBA_U8& copy );
    
    /// C_RGBA_U1555 Assignment
    const C_RGBA_U8&        operator = ( const C_RGBA_U1555& src );
    
    /// C_RGBA_F32 Assignment
    const C_RGBA_U8&        operator = ( const C_RGBA_F32& src );
    
    /// Accessor: Get Alpha
    U8                      A() const;
    
    /// Accessor: Get Red
    U8                      R() const;
    
    /// Accessor: Get Green
    U8                      G() const;
    
    /// Accessor: Get Blue
    U8                      B() const;
    
    /// Accessor: Set Alpha
    void                    A( const U8 a );
    
    /// Accessor: Set Red
    void                    R( const U8 r );
    
    /// Accessor: Set Green
    void                    G( const U8 g );
    
    /// Accessor: Set Blue
    void                    B( const U8 b );
    
    /// Get Packed U32 Colour
    U32                     Colour() const;
    
protected:
    
    static const U32        BitsA = 8;
    static const U32        BitsR = 8;
    static const U32        BitsG = 8;
    static const U32        BitsB = 8;
    
    static const U32        ShiftA = 24;
    static const U32        ShiftR = 16;
    static const U32        ShiftG = 8;
    static const U32        ShiftB = 0;
    
    static const U32        MaskA = 0xFF000000;
    static const U32        MaskR = 0x00FF0000;
    static const U32        MaskG = 0x0000FF00;
    static const U32        MaskB = 0x000000FF;
    
    U32                     m_ARGB;
};

typedef C_RGBA_U8 RGBA;




//
// C_RGBA_F32 Class : R:F32, G:F32, B:F32, A:F32
//

class C_RGBA_F32
{
friend class C_RGBA_U1555;
friend class C_RGBA_U8;

//template < typename T_Vector, typename T_Storage > friend NOALIAS const T_Vector Load( T_Storage const * const __restrict pStorage );
//template < typename T_Vector, typename T_Storage > friend NOALIAS void Store( const T_Vector PCREF vector, T_Storage * const __restrict pStorage );

public:
    
    /// Default Constructor
    C_RGBA_F32();
    
    /// Copy Constructor
    C_RGBA_F32( const C_RGBA_F32& copy );
    
    /// C_RGBA_U8 Constructor
    C_RGBA_F32( const C_RGBA_U8& rgba );
    
    /// HVec3 Constructor
    C_RGBA_F32( const HVec3 PCREF src );
    
    /// HVec4 Constructor
    C_RGBA_F32( const HVec4 PCREF src );
    
    /// HF32 Constructor
    C_RGBA_F32( const HF32 PCREF r, const HF32 PCREF g, const HF32 PCREF b, const HF32 PCREF a );
    
    /// Copy Assignment
    const C_RGBA_F32&       operator = ( const C_RGBA_F32& copy );
    
    /// C_RGBA_U8 Assignment
    const C_RGBA_F32&       operator = ( const C_RGBA_U8 src );
    
    /// Accessor: Get R
    HF32                    R() const;
    
    /// Accessor: Get G
    HF32                    G() const;
    
    /// Accessor: Get B
    HF32                    B() const;
    
    /// Accessor: Get A
    HF32                    A() const;
    
    /// Accessor: Set R
    void                    R( const HF32 PCREF r );
    
    /// Accessor: Set G
    void                    G( const HF32 PCREF g );
    
    /// Accessor: Set B
    void                    B( const HF32 PCREF b );
    
    /// Accessor: Set A
    void                    A( const HF32 PCREF a );
    
    /// Negation operator
    C_RGBA_F32              operator - () const;
    
    /// Addition operator
    C_RGBA_F32              operator + ( const C_RGBA_F32 PCREF src ) const;
    
    /// Subtraction operator
    C_RGBA_F32              operator - ( const C_RGBA_F32 PCREF src ) const;
    
    /// Multiplication operator
    C_RGBA_F32              operator * ( const C_RGBA_F32 PCREF src ) const;
    
    /// Division operator
    C_RGBA_F32              operator / ( const C_RGBA_F32 PCREF src ) const;
    
    /// Scalar Multiplication operator
    C_RGBA_F32              operator * ( const HF32 PCREF src ) const;
    
    /// Scalar Division operator
    C_RGBA_F32              operator / ( const HF32 PCREF src ) const;
    
    /// Min
    C_RGBA_F32              Min( const C_RGBA_F32 PCREF src ) const;
    
    /// Min
    C_RGBA_F32              Max( const C_RGBA_F32 PCREF src ) const;
    
    /// Saturate
    C_RGBA_F32              Saturate() const;
    
    /// Saturate
    C_RGBA_F32              Clamp( const C_RGBA_F32 PCREF lo, const C_RGBA_F32 PCREF hi ) const;
    
    
//protected:
public:
    
    HVec4                   m_V;
};



}; // namespace Maths



#endif
