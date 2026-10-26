#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CONST_H
#define SOFT_MATHS_HVEC_NEW_CONST_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CONST_H
#define MATHS_HVEC_NEW_CONST_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Constants
/// @{

/// VBool constants
template <>
struct Const< VBool > 
{
	NOALIAS __forceinline static const VBool True();  ///< Constant true
	NOALIAS __forceinline static const VBool False(); ///< Constant false
};


/// Vec4Bool constants
template <>
struct Const< Vec4Bool > 
{
	NOALIAS __forceinline static const Vec4Bool True();  ///< Constant true
	NOALIAS __forceinline static const Vec4Bool False(); ///< Constant false
};


/// VF32 constants
template <>
struct Const< VF32 > 
{
    /// VF32 constant integer template
    template < int I > NOALIAS __forceinline static const VF32 Int();
    
    /// VF32 constant integer fraction template
    template < int Num, int Denom > NOALIAS __forceinline static const VF32 Frac();
    
    // Common VF32 integer constants
	NOALIAS __forceinline static const VF32 Zero();     ///< Constant 0.0f
	NOALIAS __forceinline static const VF32 One();      ///< Constant 1.0f
	NOALIAS __forceinline static const VF32 Two();      ///< Constant 2.0f
	NOALIAS __forceinline static const VF32 Three();    ///< Constant 3.0f
	NOALIAS __forceinline static const VF32 Four();     ///< Constant 4.0f
	NOALIAS __forceinline static const VF32 Five();     ///< Constant 5.0f
	NOALIAS __forceinline static const VF32 Six();      ///< Constant 6.0f
	NOALIAS __forceinline static const VF32 Seven();    ///< Constant 7.0f
	NOALIAS __forceinline static const VF32 Eight();    ///< Constant 8.0f
	NOALIAS __forceinline static const VF32 Nine();     ///< Constant 9.0f
	NOALIAS __forceinline static const VF32 Ten();      ///< Constant 10.0f
	NOALIAS __forceinline static const VF32 Eleven();   ///< Constant 11.0f
	NOALIAS __forceinline static const VF32 Twelve();   ///< Constant 12.0f
	NOALIAS __forceinline static const VF32 Thirteen(); ///< Constant 13.0f
	NOALIAS __forceinline static const VF32 Fourteen(); ///< Constant 14.0f
	NOALIAS __forceinline static const VF32 Fifteen();  ///< Constant 15.0f
	NOALIAS __forceinline static const VF32 Sixteen();  ///< Constant 16.0f
	
	NOALIAS __forceinline static const VF32 MinusOne();   ///< Constant -1.0f
	NOALIAS __forceinline static const VF32 MinusTwo();   ///< Constant -2.0f
	NOALIAS __forceinline static const VF32 MinusThree(); ///< Constant -3.0f
	NOALIAS __forceinline static const VF32 MinusFour();  ///< Constant -4.0f
	
    // Common VF32 integer fraction constants
	NOALIAS __forceinline static const VF32 Half();    ///< Constant 1/2
	NOALIAS __forceinline static const VF32 Third();   ///< Constant 1/3
	NOALIAS __forceinline static const VF32 Quarter(); ///< Constant 1/4
	NOALIAS __forceinline static const VF32 Fifth();   ///< Constant 1/5
	NOALIAS __forceinline static const VF32 Sixth();   ///< Constant 1/6
	NOALIAS __forceinline static const VF32 Seventh(); ///< Constant 1/7
	NOALIAS __forceinline static const VF32 Eighth();  ///< Constant 1/8
	
    // VF32 constants
	NOALIAS __forceinline static const VF32 Min();            ///< Constant FLT_MIN
	NOALIAS __forceinline static const VF32 Max();            ///< Constant FLT_MAX
	NOALIAS __forceinline static const VF32 Epsilon();        ///< Constant FLT_EPSILON
	NOALIAS __forceinline static const VF32 EpsilonSquared(); ///< Constant FLT_EPSILON*FLT_EPSILON
	NOALIAS __forceinline static const VF32 Epsilon2();       ///< Constant 0.01f
	NOALIAS __forceinline static const VF32 Epsilon3();       ///< Constant 1e-12f
	NOALIAS __forceinline static const VF32 Pi();             ///< Constant PI
	NOALIAS __forceinline static const VF32 TwoPi();          ///< Constant PI*2
	NOALIAS __forceinline static const VF32 RcpPi();          ///< Constant 1/PI
	NOALIAS __forceinline static const VF32 Rcp2Pi();         ///< Constant 1/(2*PI)
	NOALIAS __forceinline static const VF32 PiDiv2();         ///< Constant PI/2
	NOALIAS __forceinline static const VF32 Sqrt2();          ///< Constant sqrt(2)
	NOALIAS __forceinline static const VF32 Sqrt2div2();      ///< Constant sqrt(2)/2
	NOALIAS __forceinline static const VF32 Sqrt3div2();      ///< Constant sqrt(3)/2
	NOALIAS __forceinline static const VF32 e();              ///< Constant e (2.7182818284590452353602874713527)
	NOALIAS __forceinline static const VF32 PointNineNine();  ///< Constant 0.99
	
	static const StorageVF32 m_Min;            ///< Constant FLT_MIN
	static const StorageVF32 m_Max;            ///< Constant FLT_MAX
	static const StorageVF32 m_Epsilon;        ///< Constant FLT_EPSILON
	static const StorageVF32 m_EpsilonSquared; ///< Constant FLT_EPSILON*FLT_EPSILON
	static const StorageVF32 m_Epsilon2;       ///< Constant 0.01f
	static const StorageVF32 m_Epsilon3;       ///< Constant 1e-12f
	static const StorageVF32 m_Pi;             ///< Constant PI
	static const StorageVF32 m_TwoPi;          ///< Constant PI*2
	static const StorageVF32 m_RcpPi;          ///< Constant 1/PI
	static const StorageVF32 m_Rcp2Pi;         ///< Constant 1/(2*PI)
	static const StorageVF32 m_PiDiv2;         ///< Constant PI/2
	static const StorageVF32 m_Sqrt2;          ///< Constant sqrt(2)
	static const StorageVF32 m_Sqrt2div2;      ///< Constant sqrt(2)/2
	static const StorageVF32 m_Sqrt3div2;      ///< Constant sqrt(3)/2
	static const StorageVF32 m_e;              ///< Constant e (2.7182818284590452353602874713527)
	static const StorageVF32 m_PointNineNine;  ///< Constant e 0.99
};


/// Vec4F32 constants
template <>
struct Const< Vec4F32 > 
{
    // Vec4F32 constant integer template
    template < int I >
    NOALIAS __forceinline static const Vec4F32 Int(); ///< Constant Vec4F32(I,I,I,I)
    
    template < int X, int Y, int Z, int W >
    NOALIAS __forceinline static const Vec4F32 VecInt(); ///< Constant Vec4F32(X,Y,Z,W)
    
    // Vec4F32 constant integer fraction template
    template < int Num, int Denom >
    NOALIAS __forceinline static const Vec4F32 Frac(); ///< Constant Vec4F32( Num/Denom, Num/Denom, Num/Denom, Num/Denom )
    
    // Common Vec4F32 integer constants
    NOALIAS __forceinline static const Vec4F32 Zero();          ///< Constant Vec4F32(0,0,0,0)
    NOALIAS __forceinline static const Vec4F32 One();           ///< Constant Vec4F32(1,1,1,1)
    NOALIAS __forceinline static const Vec4F32 Two();           ///< Constant Vec4F32(2,2,2,2)
    NOALIAS __forceinline static const Vec4F32 Three();         ///< Constant Vec4F32(3,3,3,3)
    NOALIAS __forceinline static const Vec4F32 Four();          ///< Constant Vec4F32(4,4,4,4)
    NOALIAS __forceinline static const Vec4F32 Five();          ///< Constant Vec4F32(5,5,5,5)
    NOALIAS __forceinline static const Vec4F32 Six();           ///< Constant Vec4F32(6,6,6,6)
    NOALIAS __forceinline static const Vec4F32 Seven();         ///< Constant Vec4F32(7,7,7,7)
    NOALIAS __forceinline static const Vec4F32 Eight();         ///< Constant Vec4F32(8,8,8,8)
    NOALIAS __forceinline static const Vec4F32 Nine();          ///< Constant Vec4F32(9,9,9,9)
    NOALIAS __forceinline static const Vec4F32 Ten();           ///< Constant Vec4F32(10,10,10,10)
    NOALIAS __forceinline static const Vec4F32 Eleven();        ///< Constant Vec4F32(11,11,11,11)
    NOALIAS __forceinline static const Vec4F32 Twelve();        ///< Constant Vec4F32(12,12,12,12)
    NOALIAS __forceinline static const Vec4F32 Thirteen();      ///< Constant Vec4F32(13,13,13,13)
    NOALIAS __forceinline static const Vec4F32 Fourteen();      ///< Constant Vec4F32(14,14,14,14)
    NOALIAS __forceinline static const Vec4F32 Fifteen();       ///< Constant Vec4F32(15,15,15,15)
    NOALIAS __forceinline static const Vec4F32 Sixteen();       ///< Constant Vec4F32(16,16,16,16)
    
    NOALIAS __forceinline static const Vec4F32 MinusOne();      ///< Constant Vec4F32(-1,-1,-1,-1)
    NOALIAS __forceinline static const Vec4F32 MinusTwo();      ///< Constant Vec4F32(-2,-2,-2,-2)
    NOALIAS __forceinline static const Vec4F32 MinusThree();    ///< Constant Vec4F32(-3,-3,-3,-3)
    NOALIAS __forceinline static const Vec4F32 MinusFour();     ///< Constant Vec4F32(-4,-4,-4,-4)
    
    NOALIAS __forceinline static const Vec4F32 UnitX();         ///< Constant Vec4F32(1,0,0,0)
    NOALIAS __forceinline static const Vec4F32 UnitY();         ///< Constant Vec4F32(0,1,0,0)
    NOALIAS __forceinline static const Vec4F32 UnitZ();         ///< Constant Vec4F32(0,0,1,0)
    NOALIAS __forceinline static const Vec4F32 UnitW();         ///< Constant Vec4F32(0,0,0,1)
    
    NOALIAS __forceinline static const Vec4F32 NegativeUnitX(); ///< Constant Vec4F32(-1,0,0,0)
    NOALIAS __forceinline static const Vec4F32 NegativeUnitY(); ///< Constant Vec4F32(0,-1,0,0)
    NOALIAS __forceinline static const Vec4F32 NegativeUnitZ(); ///< Constant Vec4F32(0,0,-1,0)
    NOALIAS __forceinline static const Vec4F32 NegativeUnitW(); ///< Constant Vec4F32(0,0,0,-1)
    
    // Common Vec4F32 integer fraction constants
    NOALIAS __forceinline static const Vec4F32 Half();          ///< Constant Vec4F32( 1/2, 1/2, 1/2, 1/2 )
    NOALIAS __forceinline static const Vec4F32 Third();         ///< Constant Vec4F32( 1/3, 1/3, 1/3, 1/3 )
    NOALIAS __forceinline static const Vec4F32 Quarter();       ///< Constant Vec4F32( 1/4, 1/4, 1/4, 1/4 )
    NOALIAS __forceinline static const Vec4F32 Fifth();         ///< Constant Vec4F32( 1/5, 1/5, 1/5, 1/5 )
    NOALIAS __forceinline static const Vec4F32 Sixth();         ///< Constant Vec4F32( 1/6, 1/6, 1/6, 1/6 )
    NOALIAS __forceinline static const Vec4F32 Seventh();       ///< Constant Vec4F32( 1/7, 1/7, 1/7, 1/7 )
    NOALIAS __forceinline static const Vec4F32 Eighth();        ///< Constant Vec4F32( 1/8, 1/8, 1/8, 1/8 )
    
    NOALIAS __forceinline static const Vec4F32 Min();           ///< Constant Vec4F32( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN )
    NOALIAS __forceinline static const Vec4F32 Max();           ///< Constant Vec4F32( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX )
    NOALIAS __forceinline static const Vec4F32 Epsilon();       ///< Constant Vec4F32( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON )
    NOALIAS __forceinline static const Vec4F32 Epsilon2();      ///< Constant Vec4F32( 0.01, 0.01, 0.01, 0.01 )
    NOALIAS __forceinline static const Vec4F32 Epsilon3();      ///< Constant Vec4F32( 1e-12, 1e-12, 1e-12, 1e-12 )
    NOALIAS __forceinline static const Vec4F32 Pi();            ///< Constant Vec4F32( PI, PI, PI, PI )
    NOALIAS __forceinline static const Vec4F32 TwoPi();         ///< Constant Vec4F32( PI*2, PI*2, PI*2, PI*2 )
    NOALIAS __forceinline static const Vec4F32 RcpPi();         ///< Constant Vec4F32( 1/PI, 1/PI, 1/PI, 1/PI )
    NOALIAS __forceinline static const Vec4F32 Rcp2Pi();        ///< Constant Vec4F32( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2) )
    NOALIAS __forceinline static const Vec4F32 PiDiv2();        ///< Constant Vec4F32( PI/2, PI/2, PI/2, PI/2 )
    NOALIAS __forceinline static const Vec4F32 Sqrt2();         ///< Constant Vec4F32( sqrt(2), sqrt(2), sqrt(2), sqrt(2) )
    NOALIAS __forceinline static const Vec4F32 Sqrt2div2();     ///< Constant Vec4F32( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2 )
    NOALIAS __forceinline static const Vec4F32 e();             ///< Constant Vec4F32( e, e, e, e ) : base for natural logarithm e ~= 2.718282
    
    template < bool X, bool Y, bool Z, bool W >
    NOALIAS __forceinline static const Vec4F32 XorNegate();     ///< Constant for negating the selected components of a vector using the XOR operation
    
    NOALIAS __forceinline static const Vec4F32 XorNegateX();    ///< Constant for negating the X component of a vector using the XOR operation
    NOALIAS __forceinline static const Vec4F32 XorNegateY();    ///< Constant for negating the Y component of a vector using the XOR operation
    NOALIAS __forceinline static const Vec4F32 XorNegateZ();    ///< Constant for negating the Z component of a vector using the XOR operation
    NOALIAS __forceinline static const Vec4F32 XorNegateW();    ///< Constant for negating the W component of a vector using the XOR operation
    
    NOALIAS __forceinline static const Vec4F32 XorNegateXY();   ///< Constant for negating the X & Y components of a vector using the XOR operation
    NOALIAS __forceinline static const Vec4F32 XorNegateXZ();   ///< Constant for negating the X & Z components of a vector using the XOR operation
    NOALIAS __forceinline static const Vec4F32 XorNegateXW();   ///< Constant for negating the X & W components of a vector using the XOR operation
    NOALIAS __forceinline static const Vec4F32 XorNegateYZ();   ///< Constant for negating the Y & Z components of a vector using the XOR operation
    NOALIAS __forceinline static const Vec4F32 XorNegateYW();   ///< Constant for negating the Y & W components of a vector using the XOR operation
    NOALIAS __forceinline static const Vec4F32 XorNegateZW();   ///< Constant for negating the Z & W components of a vector using the XOR operation
    
    NOALIAS __forceinline static const Vec4F32 XorNegateXYZ();  ///< Constant for negating the X, Y & Z components of a vector using the XOR operation
    NOALIAS __forceinline static const Vec4F32 XorNegateXYW();  ///< Constant for negating the X, Y & W components of a vector using the XOR operation
    NOALIAS __forceinline static const Vec4F32 XorNegateXZW();  ///< Constant for negating the X, Z & W components of a vector using the XOR operation
    NOALIAS __forceinline static const Vec4F32 XorNegateYZW();  ///< Constant for negating the Y, Z & W components of a vector using the XOR operation
    
    static const Vec4F32 m_XorNegate[16];                       ///< Constant for negating the selected components of a vector using the XOR operation
    
    static const Vec4F32 m_XorNegateX;                          ///< Constant for negating the X component of a vector using the XOR operation
    static const Vec4F32 m_XorNegateY;                          ///< Constant for negating the Y component of a vector using the XOR operation
    static const Vec4F32 m_XorNegateZ;                          ///< Constant for negating the Z component of a vector using the XOR operation
    static const Vec4F32 m_XorNegateW;                          ///< Constant for negating the W component of a vector using the XOR operation
    
    static const Vec4F32 m_XorNegateXY;                         ///< Constant for negating the X & Y components of a vector using the XOR operation
    static const Vec4F32 m_XorNegateXZ;                         ///< Constant for negating the X & Z components of a vector using the XOR operation
    static const Vec4F32 m_XorNegateXW;                         ///< Constant for negating the X & W components of a vector using the XOR operation
    static const Vec4F32 m_XorNegateYZ;                         ///< Constant for negating the Y & Z components of a vector using the XOR operation
    static const Vec4F32 m_XorNegateYW;                         ///< Constant for negating the Y & W components of a vector using the XOR operation
    static const Vec4F32 m_XorNegateZW;                         ///< Constant for negating the Z & W components of a vector using the XOR operation
    
    static const Vec4F32 m_XorNegateXYZ;                        ///< Constant for negating the X, Y & Z components of a vector using the XOR operation
    static const Vec4F32 m_XorNegateXYW;                        ///< Constant for negating the X, Y & W components of a vector using the XOR operation
    static const Vec4F32 m_XorNegateXZW;                        ///< Constant for negating the X, Z & W components of a vector using the XOR operation
    static const Vec4F32 m_XorNegateYZW;                        ///< Constant for negating the Y, Z & W components of a vector using the XOR operation
};


/// Vec8F32 constants
template <>
struct Const< Vec8F32 > 
{
    template < int I > NOALIAS __forceinline static const Vec8F32 Int(); ///< Constant Vec8F32(I,I,I,I,...)
    
    template < int Num, int Denom > NOALIAS __forceinline static const Vec8F32 Frac(); ///< Constant Vec8F32( Num/Denom, Num/Denom, Num/Denom, Num/Denom, ... )
    
    NOALIAS __forceinline static const Vec8F32 Zero();          ///< Constant Vec8F32(0,0,0,0,0,0,0,0)
    NOALIAS __forceinline static const Vec8F32 One();           ///< Constant Vec8F32(1,1,1,1,1,1,1,1)
    NOALIAS __forceinline static const Vec8F32 Two();           ///< Constant Vec8F32(2,2,2,2,2,2,2,2)
    NOALIAS __forceinline static const Vec8F32 Three();         ///< Constant Vec8F32(3,3,3,3,3,3,3,3)
    NOALIAS __forceinline static const Vec8F32 Four();          ///< Constant Vec8F32(4,4,4,4,4,4,4,4)
    NOALIAS __forceinline static const Vec8F32 MinusOne();      ///< Constant Vec8F32(-1,-1,-1,-1,-1,-1,-1,-1)
    NOALIAS __forceinline static const Vec8F32 MinusTwo();      ///< Constant Vec8F32(-2,-2,-2,-2,-2,-2,-2,-2)
    NOALIAS __forceinline static const Vec8F32 MinusThree();    ///< Constant Vec8F32(-3,-3,-3,-3,-3,-3,-3,-3)
    NOALIAS __forceinline static const Vec8F32 MinusFour();     ///< Constant Vec8F32(-4,-4,-4,-4,-4,-4,-4,-4)
    
    NOALIAS __forceinline static const Vec8F32 UnitElement0();  ///< Constant Vec8F32(1,0,0,0,0,0,0,0)
    NOALIAS __forceinline static const Vec8F32 UnitElement1();  ///< Constant Vec8F32(0,1,0,0,0,0,0,0)
    NOALIAS __forceinline static const Vec8F32 UnitElement2();  ///< Constant Vec8F32(0,0,1,0,0,0,0,0)
    NOALIAS __forceinline static const Vec8F32 UnitElement3();  ///< Constant Vec8F32(0,0,0,1,0,0,0,0)
    NOALIAS __forceinline static const Vec8F32 UnitElement4();  ///< Constant Vec8F32(0,0,0,0,1,0,0,0)
    NOALIAS __forceinline static const Vec8F32 UnitElement5();  ///< Constant Vec8F32(0,0,0,0,0,1,0,0)
    NOALIAS __forceinline static const Vec8F32 UnitElement6();  ///< Constant Vec8F32(0,0,0,0,0,0,1,0)
    NOALIAS __forceinline static const Vec8F32 UnitElement7();  ///< Constant Vec8F32(0,0,0,0,0,0,0,1)
    
    NOALIAS __forceinline static const Vec8F32 Min();           ///< Constant Vec8F32( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN, ... )
    NOALIAS __forceinline static const Vec8F32 Max();           ///< Constant Vec8F32( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, ... )
    NOALIAS __forceinline static const Vec8F32 Epsilon();       ///< Constant Vec8F32( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, ... )
    NOALIAS __forceinline static const Vec8F32 Epsilon2();      ///< Constant Vec8F32( 0.01, 0.01, 0.01, 0.01, ... )
    NOALIAS __forceinline static const Vec8F32 Epsilon3();      ///< Constant Vec8F32( 1e-12, 1e-12, 1e-12, 1e-12, ... )
    NOALIAS __forceinline static const Vec8F32 Pi();            ///< Constant Vec8F32( PI, PI, PI, PI, ... )
    NOALIAS __forceinline static const Vec8F32 TwoPi();         ///< Constant Vec8F32( PI*2, PI*2, PI*2, PI*2, ... )
    NOALIAS __forceinline static const Vec8F32 RcpPi();         ///< Constant Vec8F32( 1/PI, 1/PI, 1/PI, 1/PI, ... )
    NOALIAS __forceinline static const Vec8F32 Rcp2Pi();        ///< Constant Vec8F32( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2), ... )
    NOALIAS __forceinline static const Vec8F32 PiDiv2();        ///< Constant Vec8F32( PI/2, PI/2, PI/2, PI/2, ... )
    NOALIAS __forceinline static const Vec8F32 Sqrt2();         ///< Constant Vec8F32( sqrt(2), sqrt(2), sqrt(2), sqrt(2), ... )
    NOALIAS __forceinline static const Vec8F32 Sqrt2div2();     ///< Constant Vec8F32( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, ... )
    NOALIAS __forceinline static const Vec8F32 e();             ///< Constant Vec8F32( e, e, e, e, ... ) : base for natural logarithm e ~= 2.718282
};


/// Vec12F32 constants
template <>
struct Const< Vec12F32 > 
{
    template < int I > NOALIAS __forceinline static const Vec12F32 Int(); ///< Constant Vec12F32(I,I,I,I,...)
    
    template < int Num, int Denom > NOALIAS __forceinline static const Vec12F32 Frac(); ///< Constant Vec12F32( Num/Denom, Num/Denom, Num/Denom, Num/Denom, ... )
    
    NOALIAS __forceinline static const Vec12F32 Zero(); ///< Constant Vec12F32(0,0,0,0,0,0,0,0)
};


/// Vector2 constants
template <>
struct Const< Vector2 > 
{
    // Vector2 constant integer template
    template < int I >
    NOALIAS __forceinline static const Vector2 Int();           ///< Constant Vector2(I,I)
    
    // Vector2 constant integer fraction template
    template < int Num, int Denom >
    NOALIAS __forceinline static const Vector2 Frac();          ///< Constant Vector2( Num/Denom, Num/Denom )
    
    NOALIAS __forceinline static const Vector2 Zero();          ///< Constant Vector2(0,0)
    NOALIAS __forceinline static const Vector2 One();           ///< Constant Vector2(1,1)
    NOALIAS __forceinline static const Vector2 Two();           ///< Constant Vector2(2,2)
    NOALIAS __forceinline static const Vector2 Three();         ///< Constant Vector2(3,3)
    NOALIAS __forceinline static const Vector2 Four();          ///< Constant Vector2(4,4)
    NOALIAS __forceinline static const Vector2 MinusOne();      ///< Constant Vector2(-1,-1)
    NOALIAS __forceinline static const Vector2 MinusTwo();      ///< Constant Vector2(-2,-2)
    NOALIAS __forceinline static const Vector2 MinusThree();    ///< Constant Vector2(-3,-3)
    NOALIAS __forceinline static const Vector2 MinusFour();     ///< Constant Vector2(-4,-4)
    
    NOALIAS __forceinline static const Vector2 UnitX();         ///< Constant Vector2(1,0)
    NOALIAS __forceinline static const Vector2 UnitY();         ///< Constant Vector2(0,1)
    
    NOALIAS __forceinline static const Vector2 Min();           ///< Constant Vector2( FLT_MIN, FLT_MIN )
    NOALIAS __forceinline static const Vector2 Max();           ///< Constant Vector2( FLT_MAX, FLT_MAX )
    NOALIAS __forceinline static const Vector2 Epsilon();       ///< Constant Vector2( FLT_EPSILON, FLT_EPSILON )
    NOALIAS __forceinline static const Vector2 Epsilon2();      ///< Constant Vector2( 0.01, 0.01 )
    NOALIAS __forceinline static const Vector2 Epsilon3();      ///< Constant Vector2( 1e-12, 1e-12 )
    NOALIAS __forceinline static const Vector2 Pi();            ///< Constant Vector2( PI, PI )
    NOALIAS __forceinline static const Vector2 TwoPi();         ///< Constant Vector2( PI*2, PI*2 )
    NOALIAS __forceinline static const Vector2 RcpPi();         ///< Constant Vector2( 1/PI, 1/PI )
    NOALIAS __forceinline static const Vector2 Rcp2Pi();        ///< Constant Vector2( 1/(PI*2), 1/(PI*2) )
    NOALIAS __forceinline static const Vector2 PiDiv2();        ///< Constant Vector2( PI/2, PI/2 )
    NOALIAS __forceinline static const Vector2 Sqrt2();         ///< Constant Vector2( sqrt(2), sqrt(2) )
    NOALIAS __forceinline static const Vector2 Sqrt2div2();     ///< Constant Vector2( sqrt(2)/2, sqrt(2)/2 )
    NOALIAS __forceinline static const Vector2 e();             ///< Constant Vector2( e, e ) : base for natural logarithm e ~= 2.718282
};


/// Vector3 constants
template <>
struct Const< Vector3 > 
{
    // Vector3 constant integer template
    template < int I >
    NOALIAS __forceinline static const Vector3 Int();           ///< Constant Vector3(I,I,I)
    
    // Vector3 constant integer fraction template
    template < int Num, int Denom >
    NOALIAS __forceinline static const Vector3 Frac();          ///< Constant Vector3( Num/Denom, Num/Denom, Num/Denom )
    
	NOALIAS __forceinline static const Vector3 Zero();          ///< Constant Vector3(0,0,0)
	NOALIAS __forceinline static const Vector3 One();           ///< Constant Vector3(1,1,1)
    NOALIAS __forceinline static const Vector3 Two();           ///< Constant Vector3(2,2,2)
    NOALIAS __forceinline static const Vector3 Three();         ///< Constant Vector3(3,3,3)
    NOALIAS __forceinline static const Vector3 Four();          ///< Constant Vector3(4,4,4)
    NOALIAS __forceinline static const Vector3 MinusOne();      ///< Constant Vector3(-1,-1,-1)
    NOALIAS __forceinline static const Vector3 MinusTwo();      ///< Constant Vector3(-2,-2,-2)
    NOALIAS __forceinline static const Vector3 MinusThree();    ///< Constant Vector3(-3,-3,-3)
    NOALIAS __forceinline static const Vector3 MinusFour();     ///< Constant Vector3(-4,-4,-4)
	
	NOALIAS __forceinline static const Vector3 UnitX();         ///< Constant Vector3(1,0,0)
	NOALIAS __forceinline static const Vector3 UnitY();         ///< Constant Vector3(0,1,0)
	NOALIAS __forceinline static const Vector3 UnitZ();         ///< Constant Vector3(0,0,1)
    
    NOALIAS __forceinline static const Vector3 Min();           ///< Constant Vector3( FLT_MIN, FLT_MIN, FLT_MIN )
    NOALIAS __forceinline static const Vector3 Max();           ///< Constant Vector3( FLT_MAX, FLT_MAX, FLT_MAX )
    NOALIAS __forceinline static const Vector3 Epsilon();       ///< Constant Vector3( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON )
    NOALIAS __forceinline static const Vector3 Epsilon2();      ///< Constant Vector3( 0.01, 0.01, 0.01 )
    NOALIAS __forceinline static const Vector3 Epsilon3();      ///< Constant Vector3( 1e-12, 1e-12, 1e-12 )
    NOALIAS __forceinline static const Vector3 Pi();            ///< Constant Vector3( PI, PI, PI )
    NOALIAS __forceinline static const Vector3 TwoPi();         ///< Constant Vector3( PI*2, PI*2, PI*2 )
    NOALIAS __forceinline static const Vector3 RcpPi();         ///< Constant Vector3( 1/PI, 1/PI, 1/PI )
    NOALIAS __forceinline static const Vector3 Rcp2Pi();        ///< Constant Vector3( 1/(PI*2), 1/(PI*2), 1/(PI*2) )
    NOALIAS __forceinline static const Vector3 PiDiv2();        ///< Constant Vector3( PI/2, PI/2, PI/2 )
    NOALIAS __forceinline static const Vector3 Sqrt2();         ///< Constant Vector3( sqrt(2), sqrt(2), sqrt(2) )
    NOALIAS __forceinline static const Vector3 Sqrt2div2();     ///< Constant Vector3( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2 )
    NOALIAS __forceinline static const Vector3 e();             ///< Constant Vector3( e, e, e ) : base for natural logarithm e ~= 2.718282
};


/// Vector4 constants
template <>
struct Const< Vector4 > 
{
    // Vector4 constant integer template
    template < int I >
    NOALIAS __forceinline static const Vector4 Int();           ///< Constant Vector4(I,I,I,I)
    
    // Vector4 constant integer fraction template
    template < int Num, int Denom >
    NOALIAS __forceinline static const Vector4 Frac();          ///< Constant Vector4( Num/Denom, Num/Denom, Num/Denom, Num/Denom )
    
	NOALIAS __forceinline static const Vector4 Zero();          ///< Constant Vector4(0,0,0,0)
	NOALIAS __forceinline static const Vector4 One();           ///< Constant Vector4(1,1,1,1)
    NOALIAS __forceinline static const Vector4 Two();           ///< Constant Vector4(2,2,2,2)
    NOALIAS __forceinline static const Vector4 Three();         ///< Constant Vector4(3,3,3,3)
    NOALIAS __forceinline static const Vector4 Four();          ///< Constant Vector4(4,4,4,4)
    NOALIAS __forceinline static const Vector4 MinusOne();      ///< Constant Vector4(-1,-1,-1,-1)
    NOALIAS __forceinline static const Vector4 MinusTwo();      ///< Constant Vector4(-2,-2,-2,-2)
    NOALIAS __forceinline static const Vector4 MinusThree();    ///< Constant Vector4(-3,-3,-3,-3)
    NOALIAS __forceinline static const Vector4 MinusFour();     ///< Constant Vector4(-4,-4,-4,-4)
    
	NOALIAS __forceinline static const Vector4 UnitX();         ///< Constant Vector4(1,0,0,0)
	NOALIAS __forceinline static const Vector4 UnitY();         ///< Constant Vector4(0,1,0,0)
	NOALIAS __forceinline static const Vector4 UnitZ();         ///< Constant Vector4(0,0,1,0)
	NOALIAS __forceinline static const Vector4 UnitW();         ///< Constant Vector4(0,0,0,1)
	
    NOALIAS __forceinline static const Vector4 Min();           ///< Constant Vector4( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN )
    NOALIAS __forceinline static const Vector4 Max();           ///< Constant Vector4( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX )
    NOALIAS __forceinline static const Vector4 Epsilon();       ///< Constant Vector4( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON )
    NOALIAS __forceinline static const Vector4 Epsilon2();      ///< Constant Vector4( 0.01, 0.01, 0.01, 0.01 )
    NOALIAS __forceinline static const Vector4 Epsilon3();      ///< Constant Vector4( 1e-12, 1e-12, 1e-12, 1e-12 )
    NOALIAS __forceinline static const Vector4 Pi();            ///< Constant Vector4( PI, PI, PI, PI )
    NOALIAS __forceinline static const Vector4 TwoPi();         ///< Constant Vector4( PI*2, PI*2, PI*2, PI*2 )
    NOALIAS __forceinline static const Vector4 RcpPi();         ///< Constant Vector4( 1/PI, 1/PI, 1/PI, 1/PI )
    NOALIAS __forceinline static const Vector4 Rcp2Pi();        ///< Constant Vector4( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2) )
    NOALIAS __forceinline static const Vector4 PiDiv2();        ///< Constant Vector4( PI/2, PI/2, PI/2, PI/2 )
    NOALIAS __forceinline static const Vector4 Sqrt2();         ///< Constant Vector4( sqrt(2), sqrt(2), sqrt(2), sqrt(2) )
    NOALIAS __forceinline static const Vector4 Sqrt2div2();     ///< Constant Vector4( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2 )
    NOALIAS __forceinline static const Vector4 e();             ///< Constant Vector4( e, e, e, e ) : base for natural logarithm e ~= 2.718282
};


/// Vector5 constants
template <>
struct Const< Vector5 > 
{
    // Vector5 constant integer template
    template < int I >
    NOALIAS __forceinline static const Vector5 Int();           ///< Constant Vector5(I,I,I,I,I)
    
    // Vector5 constant integer fraction template
    template < int Num, int Denom >
    NOALIAS __forceinline static const Vector5 Frac();          ///< Constant Vector5( Num/Denom, Num/Denom, Num/Denom, Num/Denom, Num/Denom )
    
    NOALIAS __forceinline static const Vector5 Zero();          ///< Constant Vector5(0,0,0,0,0)
    NOALIAS __forceinline static const Vector5 One();           ///< Constant Vector5(1,1,1,1,1)
    NOALIAS __forceinline static const Vector5 Two();           ///< Constant Vector5(2,2,2,2,2)
    NOALIAS __forceinline static const Vector5 Three();         ///< Constant Vector5(3,3,3,3,3)
    NOALIAS __forceinline static const Vector5 Four();          ///< Constant Vector5(4,4,4,4,4)
    NOALIAS __forceinline static const Vector5 MinusOne();      ///< Constant Vector5(-1,-1,-1,-1,-1)
    NOALIAS __forceinline static const Vector5 MinusTwo();      ///< Constant Vector5(-2,-2,-2,-2,-2)
    NOALIAS __forceinline static const Vector5 MinusThree();    ///< Constant Vector5(-3,-3,-3,-3,-3)
    NOALIAS __forceinline static const Vector5 MinusFour();     ///< Constant Vector5(-4,-4,-4,-4,-4)
    
    NOALIAS __forceinline static const Vector5 UnitElement0();  ///< Constant Vector5(1,0,0,0,0)
    NOALIAS __forceinline static const Vector5 UnitElement1();  ///< Constant Vector5(0,1,0,0,0)
    NOALIAS __forceinline static const Vector5 UnitElement2();  ///< Constant Vector5(0,0,1,0,0)
    NOALIAS __forceinline static const Vector5 UnitElement3();  ///< Constant Vector5(0,0,0,1,0)
    NOALIAS __forceinline static const Vector5 UnitElement4();  ///< Constant Vector5(0,0,0,0,1)
	
    NOALIAS __forceinline static const Vector5 Min();           ///< Constant Vector5( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN, ... )
    NOALIAS __forceinline static const Vector5 Max();           ///< Constant Vector5( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, ... )
    NOALIAS __forceinline static const Vector5 Epsilon();       ///< Constant Vector5( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, ... )
    NOALIAS __forceinline static const Vector5 Epsilon2();      ///< Constant Vector5( 0.01, 0.01, 0.01, 0.01, ... )
    NOALIAS __forceinline static const Vector5 Epsilon3();      ///< Constant Vector5( 1e-12, 1e-12, 1e-12, 1e-12, ... )
    NOALIAS __forceinline static const Vector5 Pi();            ///< Constant Vector5( PI, PI, PI, PI, ... )
    NOALIAS __forceinline static const Vector5 TwoPi();         ///< Constant Vector5( PI*2, PI*2, PI*2, PI*2, ... )
    NOALIAS __forceinline static const Vector5 RcpPi();         ///< Constant Vector5( 1/PI, 1/PI, 1/PI, 1/PI, ... )
    NOALIAS __forceinline static const Vector5 Rcp2Pi();        ///< Constant Vector5( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2), ... )
    NOALIAS __forceinline static const Vector5 PiDiv2();        ///< Constant Vector5( PI/2, PI/2, PI/2, PI/2, ... )
    NOALIAS __forceinline static const Vector5 Sqrt2();         ///< Constant Vector5( sqrt(2), sqrt(2), sqrt(2), sqrt(2), ... )
    NOALIAS __forceinline static const Vector5 Sqrt2div2();     ///< Constant Vector5( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, ... )
    NOALIAS __forceinline static const Vector5 e();             ///< Constant Vector5( e, e, e, e, ... ) : base for natural logarithm e ~= 2.718282
};


/// Vector6 constants
template <>
struct Const< Vector6 > 
{
    // Vector6 constant integer template
    template < int I >
    NOALIAS __forceinline static const Vector6 Int();           ///< Constant Vector6(I,I,I,I,...)
    
    // Vector6 constant integer fraction template
    template < int Num, int Denom >
    NOALIAS __forceinline static const Vector6 Frac();          ///< Constant Vector6( Num/Denom, Num/Denom, Num/Denom, Num/Denom, ... )
    
	NOALIAS __forceinline static const Vector6 Zero();          ///< Constant Vector6(0,0,0,0,0,...)
    NOALIAS __forceinline static const Vector6 One();           ///< Constant Vector6(1,1,1,1,...)
    NOALIAS __forceinline static const Vector6 Two();           ///< Constant Vector6(2,2,2,2,...)
    NOALIAS __forceinline static const Vector6 Three();         ///< Constant Vector6(3,3,3,3,...)
    NOALIAS __forceinline static const Vector6 Four();          ///< Constant Vector6(4,4,4,4,...)
    NOALIAS __forceinline static const Vector6 MinusOne();      ///< Constant Vector6(-1,-1,-1,-1,...)
    NOALIAS __forceinline static const Vector6 MinusTwo();      ///< Constant Vector6(-2,-2,-2,-2,...)
    NOALIAS __forceinline static const Vector6 MinusThree();    ///< Constant Vector6(-3,-3,-3,-3,...)
    NOALIAS __forceinline static const Vector6 MinusFour();     ///< Constant Vector6(-4,-4,-4,-4,...)
    
    NOALIAS __forceinline static const Vector6 UnitElement0();  ///< Constant Vector6(1,0,0,0,0,0)
    NOALIAS __forceinline static const Vector6 UnitElement1();  ///< Constant Vector6(0,1,0,0,0,0)
    NOALIAS __forceinline static const Vector6 UnitElement2();  ///< Constant Vector6(0,0,1,0,0,0)
    NOALIAS __forceinline static const Vector6 UnitElement3();  ///< Constant Vector6(0,0,0,1,0,0)
    NOALIAS __forceinline static const Vector6 UnitElement4();  ///< Constant Vector6(0,0,0,0,1,0)
    NOALIAS __forceinline static const Vector6 UnitElement5();  ///< Constant Vector6(0,0,0,0,0,1)
	
    NOALIAS __forceinline static const Vector6 Min();           ///< Constant Vector6( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN, ... )
    NOALIAS __forceinline static const Vector6 Max();           ///< Constant Vector6( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, ... )
    NOALIAS __forceinline static const Vector6 Epsilon();       ///< Constant Vector6( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, ... )
    NOALIAS __forceinline static const Vector6 Epsilon2();      ///< Constant Vector6( 0.01, 0.01, 0.01, 0.01, ... )
    NOALIAS __forceinline static const Vector6 Epsilon3();      ///< Constant Vector6( 1e-12, 1e-12, 1e-12, 1e-12, ... )
    NOALIAS __forceinline static const Vector6 Pi();            ///< Constant Vector6( PI, PI, PI, PI, ... )
    NOALIAS __forceinline static const Vector6 TwoPi();         ///< Constant Vector6( PI*2, PI*2, PI*2, PI*2, ... )
    NOALIAS __forceinline static const Vector6 RcpPi();         ///< Constant Vector6( 1/PI, 1/PI, 1/PI, 1/PI, ... )
    NOALIAS __forceinline static const Vector6 Rcp2Pi();        ///< Constant Vector6( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2), ... )
    NOALIAS __forceinline static const Vector6 PiDiv2();        ///< Constant Vector6( PI/2, PI/2, PI/2, PI/2, ... )
    NOALIAS __forceinline static const Vector6 Sqrt2();         ///< Constant Vector6( sqrt(2), sqrt(2), sqrt(2), sqrt(2), ... )
    NOALIAS __forceinline static const Vector6 Sqrt2div2();     ///< Constant Vector6( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, ... )
    NOALIAS __forceinline static const Vector6 e();             ///< Constant Vector6( e, e, e, e, ... ) : base for natural logarithm e ~= 2.718282
};


/// Vector7 constants
template <>
struct Const< Vector7 > 
{
    // Vector7 constant integer template
    template < int I >
    NOALIAS __forceinline static const Vector7 Int();           ///< Constant Vector7(I,I,I,I,...)
    
    // Vector7 constant integer fraction template
    template < int Num, int Denom >
    NOALIAS __forceinline static const Vector7 Frac();          ///< Constant Vector7( Num/Denom, Num/Denom, Num/Denom, Num/Denom, ... )
    
	NOALIAS __forceinline static const Vector7 Zero();          ///< Constant Vector7(0,0,0,0,...)
    NOALIAS __forceinline static const Vector7 One();           ///< Constant Vector7(1,1,1,1,...)
    NOALIAS __forceinline static const Vector7 Two();           ///< Constant Vector7(2,2,2,2,...)
    NOALIAS __forceinline static const Vector7 Three();         ///< Constant Vector7(3,3,3,3,...)
    NOALIAS __forceinline static const Vector7 Four();          ///< Constant Vector7(4,4,4,4,...)
    NOALIAS __forceinline static const Vector7 MinusOne();      ///< Constant Vector7(-1,-1,-1,-1,...)
    NOALIAS __forceinline static const Vector7 MinusTwo();      ///< Constant Vector7(-2,-2,-2,-2,...)
    NOALIAS __forceinline static const Vector7 MinusThree();    ///< Constant Vector7(-3,-3,-3,-3,...)
    NOALIAS __forceinline static const Vector7 MinusFour();     ///< Constant Vector7(-4,-4,-4,-4,...)
    
    NOALIAS __forceinline static const Vector7 UnitElement0();  ///< Constant Vector7(1,0,0,0,0,0,0)
    NOALIAS __forceinline static const Vector7 UnitElement1();  ///< Constant Vector7(0,1,0,0,0,0,0)
    NOALIAS __forceinline static const Vector7 UnitElement2();  ///< Constant Vector7(0,0,1,0,0,0,0)
    NOALIAS __forceinline static const Vector7 UnitElement3();  ///< Constant Vector7(0,0,0,1,0,0,0)
    NOALIAS __forceinline static const Vector7 UnitElement4();  ///< Constant Vector7(0,0,0,0,1,0,0)
    NOALIAS __forceinline static const Vector7 UnitElement5();  ///< Constant Vector7(0,0,0,0,0,1,0)
    NOALIAS __forceinline static const Vector7 UnitElement6();  ///< Constant Vector7(0,0,0,0,0,0,1)
	
    NOALIAS __forceinline static const Vector7 Min();           ///< Constant Vector7( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN, ... )
    NOALIAS __forceinline static const Vector7 Max();           ///< Constant Vector7( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, ... )
    NOALIAS __forceinline static const Vector7 Epsilon();       ///< Constant Vector7( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, ... )
    NOALIAS __forceinline static const Vector7 Epsilon2();      ///< Constant Vector7( 0.01, 0.01, 0.01, 0.01, ... )
    NOALIAS __forceinline static const Vector7 Epsilon3();      ///< Constant Vector7( 1e-12, 1e-12, 1e-12, 1e-12, ... )
    NOALIAS __forceinline static const Vector7 Pi();            ///< Constant Vector7( PI, PI, PI, PI, ... )
    NOALIAS __forceinline static const Vector7 TwoPi();         ///< Constant Vector7( PI*2, PI*2, PI*2, PI*2, ... )
    NOALIAS __forceinline static const Vector7 RcpPi();         ///< Constant Vector7( 1/PI, 1/PI, 1/PI, 1/PI, ... )
    NOALIAS __forceinline static const Vector7 Rcp2Pi();        ///< Constant Vector7( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2), ... )
    NOALIAS __forceinline static const Vector7 PiDiv2();        ///< Constant Vector7( PI/2, PI/2, PI/2, PI/2, ... )
    NOALIAS __forceinline static const Vector7 Sqrt2();         ///< Constant Vector7( sqrt(2), sqrt(2), sqrt(2), sqrt(2), ... )
    NOALIAS __forceinline static const Vector7 Sqrt2div2();     ///< Constant Vector7( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, ... )
    NOALIAS __forceinline static const Vector7 e();             ///< Constant Vector7( e, e, e, e, ... ) : base for natural logarithm e ~= 2.718282
};


/// Vector8 constants
template <>
struct Const< Vector8 > 
{
    // Vector8 constant integer template
    template < int I >
    NOALIAS __forceinline static const Vector8 Int();           ///< Constant Vector8(I,I,I,I,...)
    
    // Vector8 constant integer fraction template
    template < int Num, int Denom >
    NOALIAS __forceinline static const Vector8 Frac();          ///< Constant Vector8( Num/Denom, Num/Denom, Num/Denom, Num/Denom, ... )
    
	NOALIAS __forceinline static const Vector8 Zero();          ///< Constant Vector8(0,0,0,0,...)
    NOALIAS __forceinline static const Vector8 One();           ///< Constant Vector8(1,1,1,1,...)
    NOALIAS __forceinline static const Vector8 Two();           ///< Constant Vector8(2,2,2,2,...)
    NOALIAS __forceinline static const Vector8 Three();         ///< Constant Vector8(3,3,3,3,...)
    NOALIAS __forceinline static const Vector8 Four();          ///< Constant Vector8(4,4,4,4,...)
    NOALIAS __forceinline static const Vector8 MinusOne();      ///< Constant Vector8(-1,-1,-1,-1,...)
    NOALIAS __forceinline static const Vector8 MinusTwo();      ///< Constant Vector8(-2,-2,-2,-2,...)
    NOALIAS __forceinline static const Vector8 MinusThree();    ///< Constant Vector8(-3,-3,-3,-3,...)
    NOALIAS __forceinline static const Vector8 MinusFour();     ///< Constant Vector8(-4,-4,-4,-4,...)
    
    NOALIAS __forceinline static const Vector8 UnitElement0();  ///< Constant Vector8(1,0,0,0,0,0,0,0)
    NOALIAS __forceinline static const Vector8 UnitElement1();  ///< Constant Vector8(0,1,0,0,0,0,0,0)
    NOALIAS __forceinline static const Vector8 UnitElement2();  ///< Constant Vector8(0,0,1,0,0,0,0,0)
    NOALIAS __forceinline static const Vector8 UnitElement3();  ///< Constant Vector8(0,0,0,1,0,0,0,0)
    NOALIAS __forceinline static const Vector8 UnitElement4();  ///< Constant Vector8(0,0,0,0,1,0,0,0)
    NOALIAS __forceinline static const Vector8 UnitElement5();  ///< Constant Vector8(0,0,0,0,0,1,0,0)
    NOALIAS __forceinline static const Vector8 UnitElement6();  ///< Constant Vector8(0,0,0,0,0,0,1,0)
    NOALIAS __forceinline static const Vector8 UnitElement7();  ///< Constant Vector8(0,0,0,0,0,0,0,1)
	
    NOALIAS __forceinline static const Vector8 Min();           ///< Constant Vector8( FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN, ... )
    NOALIAS __forceinline static const Vector8 Max();           ///< Constant Vector8( FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, ... )
    NOALIAS __forceinline static const Vector8 Epsilon();       ///< Constant Vector8( FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, ... )
    NOALIAS __forceinline static const Vector8 Epsilon2();      ///< Constant Vector8( 0.01, 0.01, 0.01, 0.01, ... )
    NOALIAS __forceinline static const Vector8 Epsilon3();      ///< Constant Vector8( 1e-12, 1e-12, 1e-12, 1e-12, ... )
    NOALIAS __forceinline static const Vector8 Pi();            ///< Constant Vector8( PI, PI, PI, PI, ... )
    NOALIAS __forceinline static const Vector8 TwoPi();         ///< Constant Vector8( PI*2, PI*2, PI*2, PI*2, ... )
    NOALIAS __forceinline static const Vector8 RcpPi();         ///< Constant Vector8( 1/PI, 1/PI, 1/PI, 1/PI, ... )
    NOALIAS __forceinline static const Vector8 Rcp2Pi();        ///< Constant Vector8( 1/(PI*2), 1/(PI*2), 1/(PI*2), 1/(PI*2), ... )
    NOALIAS __forceinline static const Vector8 PiDiv2();        ///< Constant Vector8( PI/2, PI/2, PI/2, PI/2, ... )
    NOALIAS __forceinline static const Vector8 Sqrt2();         ///< Constant Vector8( sqrt(2), sqrt(2), sqrt(2), sqrt(2), ... )
    NOALIAS __forceinline static const Vector8 Sqrt2div2();     ///< Constant Vector8( sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, sqrt(2)/2, ... )
    NOALIAS __forceinline static const Vector8 e();             ///< Constant Vector8( e, e, e, e, ... ) : base for natural logarithm e ~= 2.718282
};


/// Point3 constants
template <>
struct Const< Point3 > 
{
	NOALIAS __forceinline static const Point3 Zero();  ///< Constant Point3(0,0,0)
	
	NOALIAS __forceinline static const Point3 UnitX(); ///< Constant Point3(1,0,0)
	NOALIAS __forceinline static const Point3 UnitY(); ///< Constant Point3(0,1,0)
	NOALIAS __forceinline static const Point3 UnitZ(); ///< Constant Point3(0,0,1)
};


/// HPoint3 constants
template <>
struct Const< HPoint3 > 
{
	NOALIAS __forceinline static const HPoint3 Zero();  ///< Constant HPoint3(0,0,0,1)
	
	NOALIAS __forceinline static const HPoint3 UnitX(); ///< Constant HPoint3(1,0,0,1)
	NOALIAS __forceinline static const HPoint3 UnitY(); ///< Constant HPoint3(0,1,0,1)
	NOALIAS __forceinline static const HPoint3 UnitZ(); ///< Constant HPoint3(0,0,1,1)
};


/// Normal3 constants
template <>
struct Const< Normal3 > 
{
	NOALIAS __forceinline static const Normal3 Zero();          ///< Constant Normal3(0,0,0)
	
	NOALIAS __forceinline static const Normal3 AxisX();         ///< Constant Normal3(1,0,0)
	NOALIAS __forceinline static const Normal3 AxisY();         ///< Constant Normal3(0,1,0)
	NOALIAS __forceinline static const Normal3 AxisZ();         ///< Constant Normal3(0,0,1)
	
	NOALIAS __forceinline static const Normal3 NegativeAxisX(); ///< Constant Normal3(-1,0,0)
	NOALIAS __forceinline static const Normal3 NegativeAxisY(); ///< Constant Normal3(0,-1,0)
	NOALIAS __forceinline static const Normal3 NegativeAxisZ(); ///< Constant Normal3(0,0,-1)
};


/// Quaternion constants
template <>
struct Const< Quaternion > 
{
	NOALIAS __forceinline static const Quaternion Zero();     ///< Quaternion constant Zero
	NOALIAS __forceinline static const Quaternion Identity(); ///< Quaternion constant Identity
};


/// QuatTrans constants
template <>
struct Const< QuatTrans > 
{
	NOALIAS __forceinline static const QuatTrans Zero();     ///< QuatTrans constant Zero
	NOALIAS __forceinline static const QuatTrans Identity(); ///< QuatTrans constant Identity
};


/// DualQuaternion constants
template <>
struct Const< DualQuaternion > 
{
	NOALIAS __forceinline static const DualQuaternion Zero();       ///< DualQuaternion constant Zero
	NOALIAS __forceinline static const DualQuaternion Identity();   ///< DualQuaternion constant Identity
};




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

