#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_VECTOR_CONST_H
#define SOFT_MATHS_HVEC_VECTOR_CONST_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_VECTOR_CONST_H
#define MATHS_HVEC_VECTOR_CONST_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{

/// Helper templates for creating VF32 integer & fraction constants

template < signed char I > struct ConstS8 { static const S8 m_Value; };
template < signed char I > const S8 ConstS8<I>::m_Value = I;

template < unsigned char I > struct ConstU8 { static const U8 m_Value; };
template < unsigned char I > const U8 ConstU8<I>::m_Value = I;

template < signed short I > struct ConstS16 { static const S16 m_Value; };
template < signed short I > const S16 ConstS16<I>::m_Value = I;

template < unsigned short I > struct ConstU16 { static const U16 m_Value; };
template < unsigned short I > const U16 ConstU16<I>::m_Value = I;

template < signed int I > struct ConstS32 { static const S32 m_Value; };
template < signed int I > const S32 ConstS32<I>::m_Value = I;

template < unsigned int I > struct ConstU32 { static const U32 m_Value; };
template < unsigned int I > const U32 ConstU32<I>::m_Value = I;


template < int I > struct ScalarConstInt { static const F32 m_Value; };
template < int I > const F32 ScalarConstInt<I>::m_Value = (F32)I;

struct ALIGN16 VectorConstIntData { const F32 m_X; const F32 m_Y; const F32 m_Z; const F32 m_W; };

template < int X, int Y, int Z, int W >
struct VectorConstInt { static const VectorConstIntData m_Data; };

template < int X, int Y, int Z, int W >
const VectorConstIntData VectorConstInt<X,Y,Z,W>::m_Data = { (F32)X, (F32)Y, (F32)Z, (F32)W };

template < int N, int D > struct ScalarConstFraction { static const F32 m_Value; };
template < int N, int D > const F32 ScalarConstFraction<N,D>::m_Value = (F32)N / (F32)D;

/// Helper templates for creating vector bool constants

template < bool b0, bool b1, bool b2, bool b3 >
struct ConstVec4Bool
{
    static const signed int ALIGN16 m_Data[4];
};

template < bool b0, bool b1, bool b2, bool b3 >
const signed int ALIGN16 ConstVec4Bool<b0,b1,b2,b3>::m_Data[4] =
{
    b0 ? -1 : 0,
    b1 ? -1 : 0,
    b2 ? -1 : 0,
    b3 ? -1 : 0
};

template < bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7 >
struct ConstVec8Bool
{
    static const signed short ALIGN16 m_Data[8];
};

template < bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7 >
const signed short ALIGN16 ConstVec8Bool<b0,b1,b2,b3,b4,b5,b6,b7>::m_Data[8] =
{
    b0 ? -1 : 0,
    b1 ? -1 : 0,
    b2 ? -1 : 0,
    b3 ? -1 : 0,
    b4 ? -1 : 0,
    b5 ? -1 : 0,
    b6 ? -1 : 0,
    b7 ? -1 : 0
};

template < bool b0, bool b1, bool b2,  bool b3,  bool b4,  bool b5,  bool b6,  bool b7,
           bool b8, bool b9, bool b10, bool b11, bool b12, bool b13, bool b14, bool b15 >
struct ConstVec16Bool
{
    static const signed char ALIGN16 m_Data[16];
};

template < bool b0, bool b1, bool b2,  bool b3,  bool b4,  bool b5,  bool b6,  bool b7,
           bool b8, bool b9, bool b10, bool b11, bool b12, bool b13, bool b14, bool b15 >
const signed char ALIGN16 ConstVec16Bool<b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15>::m_Data[16] =
{
    b0  ? -1 : 0,
    b1  ? -1 : 0,
    b2  ? -1 : 0,
    b3  ? -1 : 0,
    b4  ? -1 : 0,
    b5  ? -1 : 0,
    b6  ? -1 : 0,
    b7  ? -1 : 0,
    b8  ? -1 : 0,
    b9  ? -1 : 0,
    b10 ? -1 : 0,
    b11 ? -1 : 0,
    b12 ? -1 : 0,
    b13 ? -1 : 0,
    b14 ? -1 : 0,
    b15 ? -1 : 0
};



template < typename T > class Const
{
};

#if defined( _SPU_ ) || defined( _PPU_ )

template <> class Const< vector float >
{
public:
    
    template < int I >
    inline static const vector float    Int() __attribute__(( pure, always_inline ));
    
    template < int N, int D >
    inline static const vector float    Frac() __attribute__(( pure, always_inline ));
    
    inline static const vector float    Zero() __attribute__(( const, always_inline ));
    
    inline static const vector float    Half() __attribute__(( pure, always_inline ));
    inline static const vector float    Quarter() __attribute__(( pure, always_inline ));
    
    inline static const vector float    One()   __attribute__(( pure, always_inline ));
    inline static const vector float    Two()   __attribute__(( pure, always_inline ));
    inline static const vector float    Three() __attribute__(( pure, always_inline ));
    inline static const vector float    Four()  __attribute__(( pure, always_inline ));
    
    inline static const vector float    MinusOne()   __attribute__(( pure, always_inline ));
    inline static const vector float    MinusTwo()   __attribute__(( pure, always_inline ));
    inline static const vector float    MinusThree() __attribute__(( pure, always_inline ));
    inline static const vector float    MinusFour()  __attribute__(( pure, always_inline ));
    
    inline static const vector float    UnitX() __attribute__(( pure, always_inline ));
    inline static const vector float    UnitY() __attribute__(( pure, always_inline ));
    inline static const vector float    UnitZ() __attribute__(( pure, always_inline ));
    inline static const vector float    UnitW() __attribute__(( pure, always_inline ));
    
    inline static const vector float    NegativeUnitX() __attribute__(( pure, always_inline ));
    inline static const vector float    NegativeUnitY() __attribute__(( pure, always_inline ));
    inline static const vector float    NegativeUnitZ() __attribute__(( pure, always_inline ));
    inline static const vector float    NegativeUnitW() __attribute__(( pure, always_inline ));
    
    inline static const vector float    Epsilon() __attribute__(( const, always_inline ));
    inline static const vector float    OneMinusEpsilon() __attribute__(( const, always_inline ));
    
    inline static const vector float    Pi()       __attribute__(( const, always_inline ));
    inline static const vector float    PiDivTwo() __attribute__(( const, always_inline ));
    inline static const vector float    TwoPi()    __attribute__(( const, always_inline ));
    inline static const vector float    RcpPi()    __attribute__(( const, always_inline ));
    inline static const vector float    RcpTwoPi() __attribute__(( const, always_inline ));
    
    inline static const vector float    Log2() __attribute__(( const, always_inline ));
    inline static const vector float    Log10() __attribute__(( const, always_inline ));
    
    inline static const vector float    FltMin() __attribute__(( const, always_inline ));
    inline static const vector float    FltMax() __attribute__(( const, always_inline ));
    
    inline static const vector float    NegFltMin() __attribute__(( const, always_inline ));
    inline static const vector float    NegFltMax() __attribute__(( const, always_inline ));
    
    inline static const vector float    AbsMask() __attribute__(( pure, always_inline ));
    inline static const vector float    SignMask() __attribute__(( pure, always_inline ));
    
    inline static const vector float    Mask( const bool x, const bool y, const bool z, const bool w ) __attribute__(( pure, always_inline ));
    
    template < bool x, bool y, bool z, bool w >
    inline static const vector float    Mask() __attribute__(( pure, always_inline ));
    
private:
    
    static const vector float           m_UnitX;
    static const vector float           m_UnitY;
    static const vector float           m_UnitZ;
    static const vector float           m_UnitW;
};

template <> class Const< vector signed int >
{
public:
    
    inline static const vector signed int   Zero() __attribute__(( const, always_inline ));
    
private:
    
};

template <> class Const< vector unsigned int >
{
public:
    
    inline static const vector unsigned int     Zero() __attribute__(( const, always_inline ));
    
    template < unsigned int I >
    inline static const vector unsigned int     Int() __attribute__(( pure, always_inline ));
    
    inline static const vector unsigned int     AbsMask() __attribute__(( pure, always_inline ));
    inline static const vector unsigned int     SignMask() __attribute__(( pure, always_inline ));
    
    inline static const vector unsigned int     Mask( const int mask ) __attribute__(( pure, always_inline ));
    inline static const vector unsigned int     Mask( const bool x, const bool y, const bool z, const bool w ) __attribute__(( pure, always_inline ));
    
    template < bool x, bool y, bool z, bool w >
    inline static const vector unsigned int     Mask() __attribute__(( pure, always_inline ));
    
    #if defined( _SPU_ )
    
    inline static const vector unsigned int     True() __attribute__(( pure, always_inline ));
    inline static const vector unsigned int     False() __attribute__(( pure, always_inline ));
    
    inline static const vector unsigned int     Bool( const bool x ) __attribute__(( pure, always_inline ));
    inline static const vector unsigned int     Bool( const bool x, bool y, bool z, bool w ) __attribute__(( pure, always_inline ));
    
    template < bool x >
    inline static const vector unsigned int     Bool() __attribute__(( pure, always_inline ));
    
    template < bool x, bool y, bool z, bool w >
    inline static const vector unsigned int     Bool() __attribute__(( pure, always_inline ));
    
    #endif
    
//private:
    
    static const vector unsigned int            m_AbsMask;
    static const vector unsigned int            m_SignMask;
    
    #if defined( _PPU_ )
    static const vector unsigned int            m_Mask[16];
    #endif
};

#if defined( _PPU_ )

template <> class Const< vector bool int >
{
public:
    
    inline static const vector bool int     True() __attribute__(( const, always_inline ));
    inline static const vector bool int     False() __attribute__(( const, always_inline ));
    
    inline static const vector bool int     Bool( const bool x ) __attribute__(( pure, always_inline ));
    inline static const vector bool int     Bool( const bool x, const bool y, const bool z, const bool w ) __attribute__(( pure, always_inline ));
    
    template < bool x >
    inline static const vector bool int     Bool() __attribute__(( pure, always_inline ));
    
    template < bool x, bool y, bool z, bool w >
    inline static const vector bool int     Bool() __attribute__(( pure, always_inline ));
    
private:
    
    static const vector bool int            m_Bool[2];
};

template <> class Const< vector bool short >
{
public:
    
    template < bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7 >
    inline static const vector bool short   Bool() __attribute__(( pure, always_inline ));
};

template <> class Const< vector bool char >
{
public:
    
    template < bool b0, bool b1, bool b2,  bool b3,  bool b4,  bool b5,  bool b6,  bool b7,
               bool b8, bool b9, bool b10, bool b11, bool b12, bool b13, bool b14, bool b15 >
    inline static const vector bool char    Bool() __attribute__(( pure, always_inline ));
};

#endif // #if defined( _PPU_ )

template <> class Const< vector unsigned short >
{
public:
    
    inline static const vector unsigned short   Zero() __attribute__(( const, always_inline ));
    
    template < unsigned short I >
    inline static const vector unsigned short   Int() __attribute__(( pure, always_inline ));
    
    #if defined( _SPU_ )
    
    template < bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7 >
    inline static const vector unsigned short   Bool() __attribute__(( pure, always_inline ));
    
    #endif
};

template <> class Const< vector unsigned char >
{
public:
    
    inline static const vector unsigned char    Zero() __attribute__(( const, always_inline ));
    inline static const vector unsigned char    Pattern_UnpackS16_XY() __attribute__(( pure, always_inline ));
    
    #if defined( _SPU_ )
    
    template < bool b0, bool b1, bool b2,  bool b3,  bool b4,  bool b5,  bool b6,  bool b7,
               bool b8, bool b9, bool b10, bool b11, bool b12, bool b13, bool b14, bool b15 >
    inline static const vector unsigned char    Bool() __attribute__(( const, always_inline ));
    
    #endif
    
private:
    
    static const vector unsigned char           m_UnpackPatternS16_XY;
};

template <> class Const< vector signed char >
{
public:
    
    inline static const vector signed char      Zero() __attribute__(( const, always_inline ));
};

#endif // #if defined( _SPU_ ) || defined( _PPU_ )

#if defined( _VMX128_ )

template <> class Const< __vector4 >
{
public:
    
    template < int I >
    NOALIAS __forceinline static const __vector4    Int();
    
    template < int N, int D >
    NOALIAS __forceinline static const __vector4    Frac();
    
    NOALIAS __forceinline static const __vector4    Zero();
    
    NOALIAS __forceinline static const __vector4    Half();
    NOALIAS __forceinline static const __vector4    Quarter();
    
    NOALIAS __forceinline static const __vector4    One();
    NOALIAS __forceinline static const __vector4    Two();
    NOALIAS __forceinline static const __vector4    Three();
    NOALIAS __forceinline static const __vector4    Four();
    
    NOALIAS __forceinline static const __vector4    MinusOne();
    NOALIAS __forceinline static const __vector4    MinusTwo();
    NOALIAS __forceinline static const __vector4    MinusThree();
    NOALIAS __forceinline static const __vector4    MinusFour();
    
    NOALIAS __forceinline static const __vector4    UnitX();
    NOALIAS __forceinline static const __vector4    UnitY();
    NOALIAS __forceinline static const __vector4    UnitZ();
    NOALIAS __forceinline static const __vector4    UnitW();
    
    NOALIAS __forceinline static const __vector4    NegativeUnitX();
    NOALIAS __forceinline static const __vector4    NegativeUnitY();
    NOALIAS __forceinline static const __vector4    NegativeUnitZ();
    NOALIAS __forceinline static const __vector4    NegativeUnitW();
    
    NOALIAS __forceinline static const __vector4    Epsilon();
    NOALIAS __forceinline static const __vector4    OneMinusEpsilon();
    
    NOALIAS __forceinline static const __vector4    Pi();
    NOALIAS __forceinline static const __vector4    PiDivTwo();
    NOALIAS __forceinline static const __vector4    TwoPi();
    NOALIAS __forceinline static const __vector4    RcpPi();
    NOALIAS __forceinline static const __vector4    RcpTwoPi();
    
    NOALIAS __forceinline static const __vector4    Log2();
    NOALIAS __forceinline static const __vector4    Log10();
    
    NOALIAS __forceinline static const __vector4    FltMin();
    NOALIAS __forceinline static const __vector4    FltMax();
    
    NOALIAS __forceinline static const __vector4    NegFltMin();
    NOALIAS __forceinline static const __vector4    NegFltMax();
    
    NOALIAS __forceinline static const __vector4    AbsMask();
    NOALIAS __forceinline static const __vector4    SignMask();
    
    template < bool x, bool y, bool z, bool w >
    NOALIAS __forceinline static const __vector4    Mask();
    
    NOALIAS __forceinline static const __vector4    Bool( const bool x );
    NOALIAS __forceinline static const __vector4    Bool( const bool x, bool y, bool z, bool w );
    
    template < bool x >
    NOALIAS __forceinline static const __vector4    Bool();
    
    template < bool x, bool y, bool z, bool w >
    NOALIAS __forceinline static const __vector4    Bool();
    
    template < bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7 >
    NOALIAS __forceinline static const __vector4    Bool();
    
    template < bool b0, bool b1, bool b2,  bool b3,  bool b4,  bool b5,  bool b6,  bool b7,
               bool b8, bool b9, bool b10, bool b11, bool b12, bool b13, bool b14, bool b15 >
    NOALIAS __forceinline static const __vector4    Bool();
    
private:
    
    static const __vector4  m_Half;
    static const __vector4  m_Quarter;
    
    static const __vector4  m_One;
    static const __vector4  m_Two;
    static const __vector4  m_Three;
    static const __vector4  m_Four;
    
    static const __vector4  m_MinusOne;
    static const __vector4  m_MinusTwo;
    static const __vector4  m_MinusThree;
    static const __vector4  m_MinusFour;
    
    static const __vector4  m_UnitX;
    static const __vector4  m_UnitY;
    static const __vector4  m_UnitZ;
    static const __vector4  m_UnitW;
    
    static const __vector4  m_Epsilon;
    static const __vector4  m_OneMinusEpsilon;
    
    static const __vector4  m_Pi;
    static const __vector4  m_PiDivTwo;
    static const __vector4  m_TwoPi;
    static const __vector4  m_RcpPi;
    static const __vector4  m_RcpTwoPi;
    
    static const __vector4  m_Log2;
    static const __vector4  m_Log10;
    
    static const __vector4  m_FltMin;
    static const __vector4  m_FltMax;
    
    static const __vector4  m_NegFltMin;
    static const __vector4  m_NegFltMax;
    
    static const __vector4  m_AbsMask;
    static const __vector4  m_SignMask;
    
    static const __vector4  m_Mask[16];
    static const __vector4  m_Bool[2];
};

#endif // #if defined( _VMX128_ )

#if defined( _SSE_ )

template <> class Const< __m128 >
{
public:
    
    template < int I >
    NOALIAS __forceinline static const __m128   Int();
    
    template < int N, int D >
    NOALIAS __forceinline static const __m128   Frac();
    
    NOALIAS __forceinline static const __m128   Zero();
    
    NOALIAS __forceinline static const __m128   Half();
    NOALIAS __forceinline static const __m128   Quarter();
    
    NOALIAS __forceinline static const __m128   One();
    NOALIAS __forceinline static const __m128   Two();
    NOALIAS __forceinline static const __m128   Three();
    NOALIAS __forceinline static const __m128   Four();
    
    NOALIAS __forceinline static const __m128   MinusOne();
    NOALIAS __forceinline static const __m128   MinusTwo();
    NOALIAS __forceinline static const __m128   MinusThree();
    NOALIAS __forceinline static const __m128   MinusFour();
    
    NOALIAS __forceinline static const __m128   UnitX();
    NOALIAS __forceinline static const __m128   UnitY();
    NOALIAS __forceinline static const __m128   UnitZ();
    NOALIAS __forceinline static const __m128   UnitW();
    
    NOALIAS __forceinline static const __m128   NegativeUnitX();
    NOALIAS __forceinline static const __m128   NegativeUnitY();
    NOALIAS __forceinline static const __m128   NegativeUnitZ();
    NOALIAS __forceinline static const __m128   NegativeUnitW();
    
    NOALIAS __forceinline static const __m128   Epsilon();
    NOALIAS __forceinline static const __m128   OneMinusEpsilon();
    
    NOALIAS __forceinline static const __m128   Pi();
    NOALIAS __forceinline static const __m128   PiDivTwo();
    NOALIAS __forceinline static const __m128   TwoPi();
    NOALIAS __forceinline static const __m128   RcpPi();
    NOALIAS __forceinline static const __m128   RcpTwoPi();
    
    NOALIAS __forceinline static const __m128   Log2();
    NOALIAS __forceinline static const __m128   Log10();
    
    NOALIAS __forceinline static const __m128   FltMin();
    NOALIAS __forceinline static const __m128   FltMax();
    
    NOALIAS __forceinline static const __m128   NegFltMin();
    NOALIAS __forceinline static const __m128   NegFltMax();
    
    NOALIAS __forceinline static const __m128   AbsMask();
    NOALIAS __forceinline static const __m128   SignMask();
    
    template < bool x, bool y, bool z, bool w >
    NOALIAS __forceinline static const __m128   Mask();
    
    NOALIAS __forceinline static const __m128   Bool( const bool x );
    NOALIAS __forceinline static const __m128   Bool( const bool x, bool y, bool z, bool w );
    
    template < bool x >
    NOALIAS __forceinline static const __m128   Bool();
    
    template < bool x, bool y, bool z, bool w >
    NOALIAS __forceinline static const __m128   Bool();
    
private:
    
    static const __m128     m_Half;
    static const __m128     m_Quarter;
    
    static const __m128     m_One;
    static const __m128     m_Two;
    static const __m128     m_Three;
    static const __m128     m_Four;
    
    static const __m128     m_MinusOne;
    static const __m128     m_MinusTwo;
    static const __m128     m_MinusThree;
    static const __m128     m_MinusFour;
    
    static const __m128     m_UnitX;
    static const __m128     m_UnitY;
    static const __m128     m_UnitZ;
    static const __m128     m_UnitW;
    
    static const __m128     m_Epsilon;
    static const __m128     m_OneMinusEpsilon;
    
    static const __m128     m_Pi;
    static const __m128     m_PiDivTwo;
    static const __m128     m_TwoPi;
    static const __m128     m_RcpPi;
    static const __m128     m_RcpTwoPi;
    
    static const __m128     m_Log2;
    static const __m128     m_Log10;
    
    static const __m128     m_FltMin;
    static const __m128     m_FltMax;
    
    static const __m128     m_NegFltMin;
    static const __m128     m_NegFltMax;
    
    static const __m128     m_AbsMask;
    static const __m128     m_SignMask;
    
    static const __m128     m_Mask[16];
    static const __m128     m_Bool[2];
};

template <> class Const< __m128i >
{
public:
    
    NOALIAS __forceinline static const __m128i  Zero();
    
    template < bool b0 >
    NOALIAS __forceinline static const __m128i  Bool();
    
    template < bool b0, bool b1, bool b2, bool b3 >
    NOALIAS __forceinline static const __m128i  Bool();
    
    template < bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7 >
    NOALIAS __forceinline static const __m128i  Bool();
    
    template < bool b0, bool b1, bool b2,  bool b3,  bool b4,  bool b5,  bool b6,  bool b7,
               bool b8, bool b9, bool b10, bool b11, bool b12, bool b13, bool b14, bool b15 >
    NOALIAS __forceinline static const __m128i  Bool();
    
    NOALIAS __forceinline static const __m128i  SignMaskF32();
    NOALIAS __forceinline static const __m128i  ExponentMaskF32();
    NOALIAS __forceinline static const __m128i  MantissaMaskF32();
    
    static const __m128i    m_SignMaskF32;
    static const __m128i    m_ExponentMaskF32;
    static const __m128i    m_MantissaMaskF32;
};
    
#endif // #if defined( _SSE_ )

}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )



