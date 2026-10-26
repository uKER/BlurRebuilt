
#ifndef MATHS_HVEC_VECTOR_CONST_GLOBALS_INL
#define MATHS_HVEC_VECTOR_CONST_GLOBALS_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{ 

#if defined( _SPU_ ) || defined( _PPU_ )

const vector float Const< vector float >::m_UnitX = (vector float)( 1.0f, 0.0f, 0.0f, 0.0f );
const vector float Const< vector float >::m_UnitY = (vector float)( 0.0f, 1.0f, 0.0f, 0.0f );
const vector float Const< vector float >::m_UnitZ = (vector float)( 0.0f, 0.0f, 1.0f, 0.0f );
const vector float Const< vector float >::m_UnitW = (vector float)( 0.0f, 0.0f, 0.0f, 1.0f );

const vector unsigned int Const< vector unsigned int >::m_AbsMask  = (vector unsigned int)( 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF );
const vector unsigned int Const< vector unsigned int >::m_SignMask = (vector unsigned int)( 0x80000000, 0x80000000, 0x80000000, 0x80000000 );

#if defined( _PPU_ )
const vector unsigned int Const< vector unsigned int >::m_Mask[16] =
{
    (vector unsigned int)( 0x00000000, 0x00000000, 0x00000000, 0x00000000 ),
    (vector unsigned int)( 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF ),
    (vector unsigned int)( 0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000 ),
    (vector unsigned int)( 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF ),
    (vector unsigned int)( 0x00000000, 0xFFFFFFFF, 0x00000000, 0x00000000 ),
    (vector unsigned int)( 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF ),
    (vector unsigned int)( 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 ),
    (vector unsigned int)( 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF ),
    (vector unsigned int)( 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000 ),
    (vector unsigned int)( 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFFFFFFFF ),
    (vector unsigned int)( 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000 ),
    (vector unsigned int)( 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF ),
    (vector unsigned int)( 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000 ),
    (vector unsigned int)( 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF ),
    (vector unsigned int)( 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 ),
    (vector unsigned int)( 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF )
};
#endif

#if defined( _SPU_ )
//const vector unsigned int Const< vector unsigned int >::m_Bool[2] =
//{
//    (vector unsigned int)( 0x00000000, 0x00000000, 0x00000000, 0x00000000 ),
//    (vector unsigned int)( 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF )
//};
#elif defined( _PPU_ )
const vector bool int Const< vector bool int >::m_Bool[2] =
{
    (vector bool int)( 0x00000000, 0x00000000, 0x00000000, 0x00000000 ),
    (vector bool int)( 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF )
};
#endif

const vector unsigned char Const< vector unsigned char >::m_UnpackPatternS16_XY = (vector unsigned char)( 0x80, 0x80, 0x00, 0x01,
                                                                                                          0x80, 0x80, 0x02, 0x03,
                                                                                                          0x80, 0x80, 0x80, 0x80,
                                                                                                          0x80, 0x80, 0x80, 0x80 );

#endif // #if defined( _SPU_ ) || defined( _PPU_ )


#if defined( _VMX128_ )

const __vector4 Const< __vector4 >::m_Half    = { 0.5f, 0.5f, 0.5f, 0.5f };
const __vector4 Const< __vector4 >::m_Quarter = { 0.25f, 0.25f, 0.25f, 0.25f };

const __vector4 Const< __vector4 >::m_One   = { 1.0f, 1.0f, 1.0f, 1.0f };
const __vector4 Const< __vector4 >::m_Two   = { 2.0f, 2.0f, 2.0f, 2.0f };
const __vector4 Const< __vector4 >::m_Three = { 3.0f, 3.0f, 3.0f, 3.0f };
const __vector4 Const< __vector4 >::m_Four  = { 4.0f, 4.0f, 4.0f, 4.0f };

const __vector4 Const< __vector4 >::m_MinusOne   = { -1.0f, -1.0f, -1.0f, -1.0f };
const __vector4 Const< __vector4 >::m_MinusTwo   = { -2.0f, -2.0f, -2.0f, -2.0f };
const __vector4 Const< __vector4 >::m_MinusThree = { -3.0f, -3.0f, -3.0f, -3.0f };
const __vector4 Const< __vector4 >::m_MinusFour  = { -4.0f, -4.0f, -4.0f, -4.0f };

const __vector4 Const< __vector4 >::m_UnitX = { 1.0f, 0.0f, 0.0f, 0.0f };
const __vector4 Const< __vector4 >::m_UnitY = { 0.0f, 1.0f, 0.0f, 0.0f };
const __vector4 Const< __vector4 >::m_UnitZ = { 0.0f, 0.0f, 1.0f, 0.0f };
const __vector4 Const< __vector4 >::m_UnitW = { 0.0f, 0.0f, 0.0f, 1.0f };

const __vector4 Const< __vector4 >::m_Epsilon = { FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON };
const __vector4 Const< __vector4 >::m_OneMinusEpsilon = { 1.0f - FLT_EPSILON, 1.0f - FLT_EPSILON, 1.0f - FLT_EPSILON, 1.0f - FLT_EPSILON };

const __vector4 Const< __vector4 >::m_Pi       = { 3.1415926535897932384626433832795f,  3.1415926535897932384626433832795f,  3.1415926535897932384626433832795f,  3.1415926535897932384626433832795f  };
const __vector4 Const< __vector4 >::m_PiDivTwo = { 1.5707963267948966192313216916398f,  1.5707963267948966192313216916398f,  1.5707963267948966192313216916398f,  1.5707963267948966192313216916398f  };
const __vector4 Const< __vector4 >::m_TwoPi    = { 6.283185307179586476925286766559f,   6.283185307179586476925286766559f,   6.283185307179586476925286766559f,   6.283185307179586476925286766559f   };
const __vector4 Const< __vector4 >::m_RcpPi    = { 0.31830988618379067153776752674503f, 0.31830988618379067153776752674503f, 0.31830988618379067153776752674503f, 0.31830988618379067153776752674503f };
const __vector4 Const< __vector4 >::m_RcpTwoPi = { 0.15915494309189533576888376337251f, 0.15915494309189533576888376337251f, 0.15915494309189533576888376337251f, 0.15915494309189533576888376337251f };

const __vector4 Const< __vector4 >::m_Log2  = { 0.69314718055994530941723212145818f, 0.69314718055994530941723212145818f, 0.69314718055994530941723212145818f, 0.69314718055994530941723212145818f };
const __vector4 Const< __vector4 >::m_Log10 = { 2.3025850929940456840179914546844f,  2.3025850929940456840179914546844f,  2.3025850929940456840179914546844f,  2.3025850929940456840179914546844f  };

const __vector4 Const< __vector4 >::m_FltMin = { FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN };
const __vector4 Const< __vector4 >::m_FltMax = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };

const __vector4 Const< __vector4 >::m_NegFltMin = { -FLT_MIN, -FLT_MIN, -FLT_MIN, -FLT_MIN };
const __vector4 Const< __vector4 >::m_NegFltMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };

const __vector4 Const< __vector4 >::m_AbsMask  = { TypePun_IntToFloat( 0x7FFFFFFF ), TypePun_IntToFloat( 0x7FFFFFFF ), TypePun_IntToFloat( 0x7FFFFFFF ), TypePun_IntToFloat( 0x7FFFFFFF ) };
const __vector4 Const< __vector4 >::m_SignMask = { TypePun_IntToFloat( 0x80000000 ), TypePun_IntToFloat( 0x80000000 ), TypePun_IntToFloat( 0x80000000 ), TypePun_IntToFloat( 0x80000000 ) };

const __vector4 Const< __vector4 >::m_Mask[16] =
{
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ) },
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ) },
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ) },
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ) }
};

const __vector4 Const< __vector4 >::m_Bool[2] =
{
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ) }
};

#endif // #if defined( _VMX128_ )



#if defined( _SSE_ )

const __m128 Const< __m128 >::m_Half    = { 0.5f, 0.5f, 0.5f, 0.5f };
const __m128 Const< __m128 >::m_Quarter = { 0.25f, 0.25f, 0.25f, 0.25f };

const __m128 Const< __m128 >::m_One   = { 1.0f, 1.0f, 1.0f, 1.0f };
const __m128 Const< __m128 >::m_Two   = { 2.0f, 2.0f, 2.0f, 2.0f };
const __m128 Const< __m128 >::m_Three = { 3.0f, 3.0f, 3.0f, 3.0f };
const __m128 Const< __m128 >::m_Four  = { 4.0f, 4.0f, 4.0f, 4.0f };

const __m128 Const< __m128 >::m_MinusOne   = { -1.0f, -1.0f, -1.0f, -1.0f };
const __m128 Const< __m128 >::m_MinusTwo   = { -2.0f, -2.0f, -2.0f, -2.0f };
const __m128 Const< __m128 >::m_MinusThree = { -3.0f, -3.0f, -3.0f, -3.0f };
const __m128 Const< __m128 >::m_MinusFour  = { -4.0f, -4.0f, -4.0f, -4.0f };

const __m128 Const< __m128 >::m_UnitX = { 1.0f, 0.0f, 0.0f, 0.0f };
const __m128 Const< __m128 >::m_UnitY = { 0.0f, 1.0f, 0.0f, 0.0f };
const __m128 Const< __m128 >::m_UnitZ = { 0.0f, 0.0f, 1.0f, 0.0f };
const __m128 Const< __m128 >::m_UnitW = { 0.0f, 0.0f, 0.0f, 1.0f };

const __m128 Const< __m128 >::m_Epsilon = { FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON };
const __m128 Const< __m128 >::m_OneMinusEpsilon = { 1.0f - FLT_EPSILON, 1.0f - FLT_EPSILON, 1.0f - FLT_EPSILON, 1.0f - FLT_EPSILON };

const __m128 Const< __m128 >::m_Pi       = { 3.1415926535897932384626433832795f,  3.1415926535897932384626433832795f,  3.1415926535897932384626433832795f,  3.1415926535897932384626433832795f  };
const __m128 Const< __m128 >::m_PiDivTwo = { 1.5707963267948966192313216916398f,  1.5707963267948966192313216916398f,  1.5707963267948966192313216916398f,  1.5707963267948966192313216916398f  };
const __m128 Const< __m128 >::m_TwoPi    = { 6.283185307179586476925286766559f,   6.283185307179586476925286766559f,   6.283185307179586476925286766559f,   6.283185307179586476925286766559f   };
const __m128 Const< __m128 >::m_RcpPi    = { 0.31830988618379067153776752674503f, 0.31830988618379067153776752674503f, 0.31830988618379067153776752674503f, 0.31830988618379067153776752674503f };
const __m128 Const< __m128 >::m_RcpTwoPi = { 0.15915494309189533576888376337251f, 0.15915494309189533576888376337251f, 0.15915494309189533576888376337251f, 0.15915494309189533576888376337251f };

const __m128 Const< __m128 >::m_Log2  = { 0.69314718055994530941723212145818f, 0.69314718055994530941723212145818f, 0.69314718055994530941723212145818f, 0.69314718055994530941723212145818f };
const __m128 Const< __m128 >::m_Log10 = { 2.3025850929940456840179914546844f,  2.3025850929940456840179914546844f,  2.3025850929940456840179914546844f,  2.3025850929940456840179914546844f  };

const __m128 Const< __m128 >::m_FltMin = { FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN };
const __m128 Const< __m128 >::m_FltMax = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };

const __m128 Const< __m128 >::m_NegFltMin = { -FLT_MIN, -FLT_MIN, -FLT_MIN, -FLT_MIN };
const __m128 Const< __m128 >::m_NegFltMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };

const __m128 Const< __m128 >::m_AbsMask  = { TypePun_IntToFloat( 0x7FFFFFFF ), TypePun_IntToFloat( 0x7FFFFFFF ), TypePun_IntToFloat( 0x7FFFFFFF ), TypePun_IntToFloat( 0x7FFFFFFF ) };
const __m128 Const< __m128 >::m_SignMask = { TypePun_IntToFloat( 0x80000000 ), TypePun_IntToFloat( 0x80000000 ), TypePun_IntToFloat( 0x80000000 ), TypePun_IntToFloat( 0x80000000 ) };

const __m128 Const< __m128 >::m_Mask[16] =
{
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ) },
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ) },
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ) },
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0xFFFFFFFF ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ) }
};

const __m128 Const< __m128 >::m_Bool[2] =
{
    { TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ), TypePun_IntToFloat( 0x00000000 ) },
    { TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ), TypePun_IntToFloat( 0xFFFFFFFF ) }
};

const __m128i Const< __m128i >::m_SignMaskF32     = _mm_set1_epi32( 0x80000000 ); // { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
const __m128i Const< __m128i >::m_ExponentMaskF32 = _mm_set1_epi32( 0x7F800000 ); // { 0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000 };
const __m128i Const< __m128i >::m_MantissaMaskF32 = _mm_set1_epi32( 0x007FFFFF ); // { 0x007FFFFF, 0x007FFFFF, 0x007FFFFF, 0x007FFFFF };

#endif // #if defined( _SSE_ )



}; // namespace Maths

#endif // #ifndef MATHS_HVEC_VECTOR_CONST_GLOBALS_INL
