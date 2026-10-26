#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_CLASS_H
#define SOFT_MATHS_HVEC_NEW_GA_CLASS_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_CLASS_H
#define MATHS_HVEC_NEW_GA_CLASS_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

#include "Core\Containers\String.h"

namespace Maths
{
namespace GA /// 5d Geometric Algebra
{
/// @addtogroup GA
/// @{

/// Enum for specifying the basis blade type of a vector element
enum BasisBits
{
    BasisBits_Null = 0xFFFFFFFF,
    
    BasisBits_s = 0x0, // grade 0
    
    BasisBits_e0 = 0x01, // grade 1
    BasisBits_e1 = 0x02,
    BasisBits_e2 = 0x04,
    BasisBits_e3 = 0x08,
    BasisBits_e4 = 0x10,
    
    BasisBits_eo = BasisBits_e0, // alternative naming convention
    BasisBits_ei = BasisBits_e4,
    
    BasisBits_no = BasisBits_e0, // alternative naming convention
    BasisBits_ni = BasisBits_e4,
    
    BasisBits_e01 = BasisBits_e0 | BasisBits_e1, // grade 2
    BasisBits_e02 = BasisBits_e0 | BasisBits_e2,
    BasisBits_e03 = BasisBits_e0 | BasisBits_e3,
    BasisBits_e04 = BasisBits_e0 | BasisBits_e4,
    BasisBits_e12 = BasisBits_e1 | BasisBits_e2,
    BasisBits_e13 = BasisBits_e1 | BasisBits_e3,
    BasisBits_e14 = BasisBits_e1 | BasisBits_e4,
    BasisBits_e23 = BasisBits_e2 | BasisBits_e3,
    BasisBits_e24 = BasisBits_e2 | BasisBits_e4,
    BasisBits_e34 = BasisBits_e3 | BasisBits_e4,
    
    BasisBits_e012 = BasisBits_e0 | BasisBits_e1 | BasisBits_e2, // grade 3
    BasisBits_e013 = BasisBits_e0 | BasisBits_e1 | BasisBits_e3,
    BasisBits_e014 = BasisBits_e0 | BasisBits_e1 | BasisBits_e4,
    BasisBits_e023 = BasisBits_e0 | BasisBits_e2 | BasisBits_e3,
    BasisBits_e024 = BasisBits_e0 | BasisBits_e2 | BasisBits_e4,
    BasisBits_e034 = BasisBits_e0 | BasisBits_e3 | BasisBits_e4,
    BasisBits_e123 = BasisBits_e1 | BasisBits_e2 | BasisBits_e3,
    BasisBits_e124 = BasisBits_e1 | BasisBits_e2 | BasisBits_e4,
    BasisBits_e134 = BasisBits_e1 | BasisBits_e3 | BasisBits_e4,
    BasisBits_e234 = BasisBits_e2 | BasisBits_e3 | BasisBits_e4,
    
    BasisBits_e0123 = BasisBits_e0 | BasisBits_e1 | BasisBits_e2 | BasisBits_e3, // grade 4
    BasisBits_e0124 = BasisBits_e0 | BasisBits_e1 | BasisBits_e2 | BasisBits_e4,
    BasisBits_e0134 = BasisBits_e0 | BasisBits_e1 | BasisBits_e3 | BasisBits_e4,
    BasisBits_e0234 = BasisBits_e0 | BasisBits_e2 | BasisBits_e3 | BasisBits_e4,
    BasisBits_e1234 = BasisBits_e1 | BasisBits_e2 | BasisBits_e3 | BasisBits_e4,
    
    BasisBits_e01234 = BasisBits_e0 | BasisBits_e1 | BasisBits_e2 | BasisBits_e3 | BasisBits_e4, // grade 5
    
    BasisBits_Terminator = BasisBits_e01234 + 1
};

/// Enum for the bitfield specifying which elements are stored in a vector
enum ElementBits
{
    ElementBits_Null   = 0x00000000,
    
    // Individual element types
    
    ElementBits_s = 1 << BasisBits_s, // Grade 0
    
    ElementBits_e0 = 1 << BasisBits_e0, // Grade 1
    ElementBits_e1 = 1 << BasisBits_e1,
    ElementBits_e2 = 1 << BasisBits_e2,
    ElementBits_e3 = 1 << BasisBits_e3,
    ElementBits_e4 = 1 << BasisBits_e4,
    
    //ElementBits_no = ElementBits_e0, // Alternative naming convention
    //ElementBits_ni = ElementBits_e4, // Alternative naming convention
    
    ElementBits_e01 = 1 << BasisBits_e01, // Grade 2
    ElementBits_e02 = 1 << BasisBits_e02,
    ElementBits_e03 = 1 << BasisBits_e03,
    ElementBits_e04 = 1 << BasisBits_e04,
    ElementBits_e12 = 1 << BasisBits_e12,
    ElementBits_e13 = 1 << BasisBits_e13,
    ElementBits_e14 = 1 << BasisBits_e14,
    ElementBits_e23 = 1 << BasisBits_e23,
    ElementBits_e24 = 1 << BasisBits_e24,
    ElementBits_e34 = 1 << BasisBits_e34,
    
    ElementBits_e012 = 1 << BasisBits_e012, // Grade 3
    ElementBits_e013 = 1 << BasisBits_e013,
    ElementBits_e014 = 1 << BasisBits_e014,
    ElementBits_e023 = 1 << BasisBits_e023,
    ElementBits_e024 = 1 << BasisBits_e024,
    ElementBits_e034 = 1 << BasisBits_e034,
    ElementBits_e123 = 1 << BasisBits_e123,
    ElementBits_e124 = 1 << BasisBits_e124,
    ElementBits_e134 = 1 << BasisBits_e134,
    ElementBits_e234 = 1 << BasisBits_e234,
    
    ElementBits_e0123 = 1 << BasisBits_e0123, // Grade 4
    ElementBits_e0124 = 1 << BasisBits_e0124,
    ElementBits_e0134 = 1 << BasisBits_e0134,
    ElementBits_e0234 = 1 << BasisBits_e0234,
    ElementBits_e1234 = 1 << BasisBits_e1234,
    
    ElementBits_e01234 = 1 << BasisBits_e01234, // Grade 5
    
    
    // Alternative naming convention
    
    ElementBits_eo = ElementBits_e0, // Grade 1
    ElementBits_ei = ElementBits_e4,
    
    ElementBits_eo1 = 1 << BasisBits_e01, // Grade 2
    ElementBits_eo2 = 1 << BasisBits_e02,
    ElementBits_eo3 = 1 << BasisBits_e03,
    ElementBits_eoi = 1 << BasisBits_e04,
    ElementBits_e1i = 1 << BasisBits_e14,
    ElementBits_e2i = 1 << BasisBits_e24,
    ElementBits_e3i = 1 << BasisBits_e34,
    
    ElementBits_eo12 = 1 << BasisBits_e012, // Grade 3
    ElementBits_eo13 = 1 << BasisBits_e013,
    ElementBits_eo1i = 1 << BasisBits_e014,
    ElementBits_eo23 = 1 << BasisBits_e023,
    ElementBits_eo2i = 1 << BasisBits_e024,
    ElementBits_eo3i = 1 << BasisBits_e034,
    ElementBits_e12i = 1 << BasisBits_e124,
    ElementBits_e13i = 1 << BasisBits_e134,
    ElementBits_e23i = 1 << BasisBits_e234,
    
    ElementBits_eo123 = 1 << BasisBits_e0123, // Grade 4
    ElementBits_eo12i = 1 << BasisBits_e0124,
    ElementBits_eo13i = 1 << BasisBits_e0134,
    ElementBits_eo23i = 1 << BasisBits_e0234,
    ElementBits_e123i = 1 << BasisBits_e1234,
    
    ElementBits_eo123i = 1 << BasisBits_e01234, // Grade 5
    
    
    ElementBits_Scalar         = ElementBits_s,      // Grade 0
    ElementBits_PseudoScalarE3 = ElementBits_e123,   // Grade 3
    ElementBits_PseudoScalarC3 = ElementBits_e01234, // Grade 5
    
    // Bitfield combinations of different element types:
    
    // Grade 1 vector
    ElementBits_VectorC3 = ElementBits_e0 | ElementBits_e1 | ElementBits_e2 | ElementBits_e3 | ElementBits_e4,
    
    // Grade 2 vector
    ElementBits_BiVectorC3 = ElementBits_e01 | ElementBits_e02 | ElementBits_e03 | ElementBits_e04 | ElementBits_e12 | ElementBits_e13 | ElementBits_e14 | ElementBits_e23 | ElementBits_e24 | ElementBits_e34,
    
    // Grade 3 vector
    ElementBits_TriVectorC3 = ElementBits_e012 | ElementBits_e013 | ElementBits_e014 | ElementBits_e023 | ElementBits_e024 | ElementBits_e034 | ElementBits_e123 | ElementBits_e124 | ElementBits_e134 | ElementBits_e234,
    
    // Grade 4 vector
    ElementBits_QuadVectorC3 = ElementBits_e0123 | ElementBits_e0124 | ElementBits_e0134 | ElementBits_e0234 | ElementBits_e1234,
    
    // MultiVector (all grades)
    ElementBits_MultiVector = ElementBits_Scalar |
                              ElementBits_VectorC3 |
                              ElementBits_BiVectorC3 |
                              ElementBits_TriVectorC3 |
                              ElementBits_QuadVectorC3 |
                              ElementBits_PseudoScalarC3,
    
    ElementBits_e1_e2_e3 = ElementBits_e1 | ElementBits_e2 | ElementBits_e3, // Euclidean 3d vector embedded in Conformal 5d vector
    
    ElementBits_e0_e1_e2_e3_e4 = ElementBits_e0 | ElementBits_e1 | ElementBits_e2 | ElementBits_e3 | ElementBits_e4,
    ElementBits_eo_e1_e2_e3_ei = ElementBits_eo | ElementBits_e1 | ElementBits_e2 | ElementBits_e3 | ElementBits_ei, // Alternative naming convention
    
    ElementBits_e04_e14_e24_e34 = ElementBits_e04 | ElementBits_e14 | ElementBits_e24 | ElementBits_e34,
    ElementBits_eoi_e1i_e2i_e3i = ElementBits_e04 | ElementBits_e14 | ElementBits_e24 | ElementBits_e34, // Alternative naming convention
    
    ElementBits_e1_e2_e3_e4 = ElementBits_e1 | ElementBits_e2 | ElementBits_e3 | ElementBits_e4,
    ElementBits_e1_e2_e3_ei = ElementBits_e1 | ElementBits_e2 | ElementBits_e3 | ElementBits_ei, // Alternative naming convention
};

/// Enum for the bitfield specifying which grades are stored in a vector
enum GradeBits
{
    GradeBits_Null = 0x00000000,
    
    GradeBits_0 = 1 << 0, // scalar
    GradeBits_1 = 1 << 1, // vector
    GradeBits_2 = 1 << 2, // bivector
    GradeBits_3 = 1 << 3, // trivector
    GradeBits_4 = 1 << 4, // quadvector
    GradeBits_5 = 1 << 5, // pseudoscalar
    
    GradeBits_01 = GradeBits_0 | GradeBits_1,
    GradeBits_02 = GradeBits_0 | GradeBits_2,
    GradeBits_03 = GradeBits_0 | GradeBits_3,
    GradeBits_04 = GradeBits_0 | GradeBits_4,
    GradeBits_05 = GradeBits_0 | GradeBits_5,
    GradeBits_12 = GradeBits_1 | GradeBits_2,
    GradeBits_13 = GradeBits_1 | GradeBits_3,
    GradeBits_14 = GradeBits_1 | GradeBits_4,
    GradeBits_15 = GradeBits_1 | GradeBits_5,
    GradeBits_23 = GradeBits_2 | GradeBits_3,
    GradeBits_24 = GradeBits_2 | GradeBits_4,
    GradeBits_25 = GradeBits_2 | GradeBits_5,
    GradeBits_34 = GradeBits_3 | GradeBits_4,
    GradeBits_35 = GradeBits_3 | GradeBits_5,
    GradeBits_45 = GradeBits_4 | GradeBits_5,
    
    GradeBits_012 = GradeBits_0 | GradeBits_1 | GradeBits_2,
    GradeBits_013 = GradeBits_0 | GradeBits_1 | GradeBits_3,
    GradeBits_014 = GradeBits_0 | GradeBits_1 | GradeBits_4,
    GradeBits_015 = GradeBits_0 | GradeBits_1 | GradeBits_5,
    GradeBits_023 = GradeBits_0 | GradeBits_2 | GradeBits_3,
    GradeBits_024 = GradeBits_0 | GradeBits_2 | GradeBits_4,
    GradeBits_025 = GradeBits_0 | GradeBits_2 | GradeBits_5,
    GradeBits_034 = GradeBits_0 | GradeBits_3 | GradeBits_4,
    GradeBits_035 = GradeBits_0 | GradeBits_3 | GradeBits_5,
    GradeBits_045 = GradeBits_0 | GradeBits_4 | GradeBits_5,
    GradeBits_123 = GradeBits_1 | GradeBits_2 | GradeBits_3,
    GradeBits_124 = GradeBits_1 | GradeBits_2 | GradeBits_4,
    GradeBits_125 = GradeBits_1 | GradeBits_2 | GradeBits_5,
    GradeBits_134 = GradeBits_1 | GradeBits_3 | GradeBits_4,
    GradeBits_135 = GradeBits_1 | GradeBits_3 | GradeBits_5,
    GradeBits_145 = GradeBits_1 | GradeBits_4 | GradeBits_5,
    GradeBits_234 = GradeBits_2 | GradeBits_3 | GradeBits_4,
    GradeBits_235 = GradeBits_2 | GradeBits_3 | GradeBits_5,
    GradeBits_245 = GradeBits_2 | GradeBits_4 | GradeBits_5,
    GradeBits_345 = GradeBits_3 | GradeBits_4 | GradeBits_5,
    
    GradeBits_0123 = GradeBits_0 | GradeBits_1 | GradeBits_2 | GradeBits_3,
    GradeBits_0124 = GradeBits_0 | GradeBits_1 | GradeBits_2 | GradeBits_4,
    GradeBits_0125 = GradeBits_0 | GradeBits_1 | GradeBits_2 | GradeBits_5,
    GradeBits_0134 = GradeBits_0 | GradeBits_1 | GradeBits_3 | GradeBits_4,
    GradeBits_0135 = GradeBits_0 | GradeBits_1 | GradeBits_3 | GradeBits_5,
    GradeBits_0145 = GradeBits_0 | GradeBits_1 | GradeBits_4 | GradeBits_5,
    GradeBits_0234 = GradeBits_0 | GradeBits_2 | GradeBits_3 | GradeBits_4,
    GradeBits_0235 = GradeBits_0 | GradeBits_2 | GradeBits_3 | GradeBits_5,
    GradeBits_0245 = GradeBits_0 | GradeBits_2 | GradeBits_4 | GradeBits_5,
    GradeBits_0345 = GradeBits_0 | GradeBits_3 | GradeBits_4 | GradeBits_5,
    GradeBits_1234 = GradeBits_1 | GradeBits_2 | GradeBits_3 | GradeBits_4,
    GradeBits_1235 = GradeBits_1 | GradeBits_2 | GradeBits_3 | GradeBits_5,
    GradeBits_1245 = GradeBits_1 | GradeBits_2 | GradeBits_4 | GradeBits_5,
    GradeBits_1345 = GradeBits_1 | GradeBits_3 | GradeBits_4 | GradeBits_5,
    GradeBits_2345 = GradeBits_2 | GradeBits_3 | GradeBits_4 | GradeBits_5,
    
    GradeBits_01234 = GradeBits_0 | GradeBits_1 | GradeBits_2 | GradeBits_3 | GradeBits_4,
    GradeBits_01235 = GradeBits_0 | GradeBits_1 | GradeBits_2 | GradeBits_3 | GradeBits_5,
    GradeBits_01245 = GradeBits_0 | GradeBits_1 | GradeBits_2 | GradeBits_4 | GradeBits_5,
    GradeBits_01345 = GradeBits_0 | GradeBits_1 | GradeBits_3 | GradeBits_4 | GradeBits_5,
    GradeBits_02345 = GradeBits_0 | GradeBits_2 | GradeBits_3 | GradeBits_4 | GradeBits_5,
    GradeBits_12345 = GradeBits_1 | GradeBits_2 | GradeBits_3 | GradeBits_4 | GradeBits_5,
    
    GradeBits_012345 = GradeBits_0 | GradeBits_1 | GradeBits_2 | GradeBits_3 | GradeBits_4 | GradeBits_5
};

//template < unsigned int T_Bits >
//struct T_BitCount
//{
//    static const unsigned int m_Bits = T_BitCount< (T_Bits>>1) >::m_Bits + (T_Bits&1);
//};
//
//template <>
//struct T_BitCount< 0 >
//{
//    static const unsigned int m_Bits = 0;
//};

//public static int bitCount(int i) {
//    // Note that unsigned shifting (>>>) is not required.
//    i = i - ((i >> 1) & 0x55555555);
//    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
//    i = (i + (i >> 4)) & 0x0F0F0F0F;
//    i = i + (i >> 8);
//    i = i + (i >> 16);
//    return i & 0x0000003F;
//    }

template < unsigned int t_Bits >
struct T_BitCount
{
    static const unsigned int i0     = t_Bits;                                       // 32 bits
    static const unsigned int i1     = i0 - ((i0 >> 1) & 0x55555555);                // 16 x 2  bit sums (0-2)
    static const unsigned int i2     = (i1 & 0x33333333) + ((i1 >> 2) & 0x33333333); // 8  x 4  bit sums (0-4)
    static const unsigned int i3     = (i2 + (i2 >> 4))  & 0x0F0F0F0F;               // 4  x 8  bit sums (0-8)
    static const unsigned int i4     = (i3 + (i3 >> 8))  & 0x001F001F;               // 2  x 16 bit sums (0-16)
    static const unsigned int m_Bits = (i4 + (i4 >> 16)) & 0x0000003F;               // 1  x 32 bit sum  (0-32)
};

template < BasisBits t_BasisBits >
struct T_GetBasisGrade
{
    static const GradeBits m_Bits = (GradeBits)( 1 << T_BitCount< t_BasisBits >::m_Bits );
};

template < ElementBits t_ElementBits, BasisBits t_BasisBits = BasisBits_s >
struct T_GetElementGrade
{
    static const GradeBits m_CurrBits = ( t_ElementBits & (1<<t_BasisBits) ) ? T_GetBasisGrade< t_BasisBits >::m_Bits
                                                                             : GradeBits_Null;
    static const GradeBits m_NextBits = T_GetElementGrade< t_ElementBits, (BasisBits)(t_BasisBits+1) >::m_Bits;
    static const GradeBits m_Bits = (GradeBits)( m_CurrBits | m_NextBits );
};

template < ElementBits t_ElementBits >
struct T_GetElementGrade< t_ElementBits, BasisBits_Terminator >
{
    static const GradeBits m_Bits = GradeBits_Null;
};


inline char const * GetString( const BasisBits bits )
{
    switch ( bits )
    {
        case BasisBits_Null :   return "BasisBits_Null";
        case BasisBits_s :      return "BasisBits_s";
        case BasisBits_e0 :     return "BasisBits_e0";
        case BasisBits_e1 :     return "BasisBits_e1";
        case BasisBits_e2 :     return "BasisBits_e2";
        case BasisBits_e3 :     return "BasisBits_e3";
        case BasisBits_e4 :     return "BasisBits_e4";
        case BasisBits_e01 :    return "BasisBits_e01";
        case BasisBits_e02 :    return "BasisBits_e02";
        case BasisBits_e03 :    return "BasisBits_e03";
        case BasisBits_e04 :    return "BasisBits_e04";
        case BasisBits_e12 :    return "BasisBits_e12";
        case BasisBits_e13 :    return "BasisBits_e13";
        case BasisBits_e14 :    return "BasisBits_e14";
        case BasisBits_e23 :    return "BasisBits_e23";
        case BasisBits_e24 :    return "BasisBits_e24";
        case BasisBits_e34 :    return "BasisBits_e34";
        case BasisBits_e012 :   return "BasisBits_e012";
        case BasisBits_e013 :   return "BasisBits_e013";
        case BasisBits_e014 :   return "BasisBits_e014";
        case BasisBits_e023 :   return "BasisBits_e023";
        case BasisBits_e024 :   return "BasisBits_e024";
        case BasisBits_e034 :   return "BasisBits_e034";
        case BasisBits_e123 :   return "BasisBits_e123";
        case BasisBits_e124 :   return "BasisBits_e124";
        case BasisBits_e134 :   return "BasisBits_e134";
        case BasisBits_e234 :   return "BasisBits_e234";
        case BasisBits_e0123 :  return "BasisBits_e0123";
        case BasisBits_e0124 :  return "BasisBits_e0124";
        case BasisBits_e0134 :  return "BasisBits_e0134";
        case BasisBits_e0234 :  return "BasisBits_e0234";
        case BasisBits_e1234 :  return "BasisBits_e1234";
        case BasisBits_e01234 : return "BasisBits_e01234";
    }
    
    return "ERROR!";
}

inline char const * GetShortString( const BasisBits bits )
{
    switch ( bits )
    {
        case BasisBits_Null :   return "(null)";
        case BasisBits_s :      return "s";
        case BasisBits_e0 :     return "e0";
        case BasisBits_e1 :     return "e1";
        case BasisBits_e2 :     return "e2";
        case BasisBits_e3 :     return "e3";
        case BasisBits_e4 :     return "e4";
        case BasisBits_e01 :    return "e01";
        case BasisBits_e02 :    return "e02";
        case BasisBits_e03 :    return "e03";
        case BasisBits_e04 :    return "e04";
        case BasisBits_e12 :    return "e12";
        case BasisBits_e13 :    return "e13";
        case BasisBits_e14 :    return "e14";
        case BasisBits_e23 :    return "e23";
        case BasisBits_e24 :    return "e24";
        case BasisBits_e34 :    return "e34";
        case BasisBits_e012 :   return "e012";
        case BasisBits_e013 :   return "e013";
        case BasisBits_e014 :   return "e014";
        case BasisBits_e023 :   return "e023";
        case BasisBits_e024 :   return "e024";
        case BasisBits_e034 :   return "e034";
        case BasisBits_e123 :   return "e123";
        case BasisBits_e124 :   return "e124";
        case BasisBits_e134 :   return "e134";
        case BasisBits_e234 :   return "e234";
        case BasisBits_e0123 :  return "e0123";
        case BasisBits_e0124 :  return "e0124";
        case BasisBits_e0134 :  return "e0134";
        case BasisBits_e0234 :  return "e0234";
        case BasisBits_e1234 :  return "e1234";
        case BasisBits_e01234 : return "e01234";
    }
    
    return "ERROR!";
}

template < BasisBits t_LHS,
           BasisBits t_RHS,
           BasisBits t_Result,
           BasisBits t_SignSwapLHS,
           BasisBits t_SignSwapRHS,
           int       t_Scale,
           typename  t_Tail >
struct T_MetricList
{
    typedef t_Tail TypeTail;
    
    static const BasisBits m_LHS         = t_LHS;
    static const BasisBits m_RHS         = t_RHS;
    static const BasisBits m_Result      = t_Result;
    static const BasisBits m_SignSwapLHS = t_LHS;
    static const BasisBits m_SignSwapRHS = t_RHS;
    static const int       m_Scale       = t_Scale;
    
    static Util::C_String GetString()
    {
        Util::C_String result;
        result.Printf( "T_MetricList< %s, %s, %s, %d, %s >",
            GA::GetString( t_LHS ),
            GA::GetString( t_RHS ),
            GA::GetString( t_Result ),
            t_Scale,
            t_Tail::GetString().Chars() );
        return result;
    }
}; 

template <>
struct T_MetricList< BasisBits_Null, BasisBits_Null, BasisBits_Null, BasisBits_Null, BasisBits_Null, 0, void >
{
    typedef void TypeTail;
    
    static const BasisBits m_LHS         = BasisBits_Null;
    static const BasisBits m_RHS         = BasisBits_Null;
    static const BasisBits m_Result      = BasisBits_Null;
    static const BasisBits m_SignSwapLHS = BasisBits_Null;
    static const BasisBits m_SignSwapRHS = BasisBits_Null;
    static const int       m_Scale       = 0;
    
    static Util::C_String GetString()
    {
        return Util::C_String( "VoidMetricList" );
    }
}; 


typedef T_MetricList< BasisBits_Null, BasisBits_Null, BasisBits_Null, BasisBits_Null, BasisBits_Null, 0, void > VoidMetricList;

//typedef T_MetricList< BasisBits_e0, BasisBits_e0, BasisBits_s, BasisBits_s, BasisBits_s, 1,
//        T_MetricList< BasisBits_e1, BasisBits_e1, BasisBits_s, BasisBits_s, BasisBits_s, 1,
//        T_MetricList< BasisBits_e2, BasisBits_e2, BasisBits_s, BasisBits_s, BasisBits_s, 1,
//        T_MetricList< BasisBits_e3, BasisBits_e3, BasisBits_s, BasisBits_s, BasisBits_s, 1,
//        T_MetricList< BasisBits_e4, BasisBits_e4, BasisBits_s, BasisBits_s, BasisBits_s, 1,
//        VoidMetricList > > > > > Euclidean5D;

//typedef T_MetricList< BasisBits_e0, BasisBits_e0, BasisBits_s, BasisBits_s, BasisBits_s,  1,
//        T_MetricList< BasisBits_e1, BasisBits_e1, BasisBits_s, BasisBits_s, BasisBits_s,  1,
//        T_MetricList< BasisBits_e2, BasisBits_e2, BasisBits_s, BasisBits_s, BasisBits_s,  1,
//        T_MetricList< BasisBits_e3, BasisBits_e3, BasisBits_s, BasisBits_s, BasisBits_s,  1,
//        T_MetricList< BasisBits_e4, BasisBits_e4, BasisBits_s, BasisBits_s, BasisBits_s, -1,
//        VoidMetricList > > > > > Minkowski5D;

typedef T_MetricList< BasisBits_e1, BasisBits_e1, BasisBits_s, BasisBits_s, BasisBits_s, 1,
        T_MetricList< BasisBits_e2, BasisBits_e2, BasisBits_s, BasisBits_s, BasisBits_s, 1,
        T_MetricList< BasisBits_e3, BasisBits_e3, BasisBits_s, BasisBits_s, BasisBits_s, 1,
        
        T_MetricList< BasisBits_e0, BasisBits_e0, BasisBits_s, BasisBits_s,    BasisBits_s,     0,
        T_MetricList< BasisBits_e0, BasisBits_e4, BasisBits_s, BasisBits_e123, BasisBits_e123, -1,
        T_MetricList< BasisBits_e4, BasisBits_e0, BasisBits_s, BasisBits_e123, BasisBits_e123,  1, // NOTE: becomes -1 with canonical reordering!
        T_MetricList< BasisBits_e4, BasisBits_e4, BasisBits_s, BasisBits_s,    BasisBits_s,     0,
        
        //T_MetricList< BasisBits_e0,  BasisBits_e04, BasisBits_e0, BasisBits_e123, BasisBits_e123,  1,
        //T_MetricList< BasisBits_e4,  BasisBits_e04, BasisBits_e4, BasisBits_e123, BasisBits_e123,  1, // NOTE: becomes -1 with canonical reordering!
        //T_MetricList< BasisBits_e04, BasisBits_e0,  BasisBits_e0, BasisBits_e123, BasisBits_e123,  1, // NOTE: becomes -1 with canonical reordering!
        //T_MetricList< BasisBits_e04, BasisBits_e4,  BasisBits_e4, BasisBits_e123, BasisBits_e123,  1,
        //T_MetricList< BasisBits_e04, BasisBits_e04, BasisBits_s,  BasisBits_e123, BasisBits_e123, -1, // NOTE: becomes +1 with canonical reordering!
        T_MetricList< BasisBits_e0,  BasisBits_e04, BasisBits_e0, BasisBits_e123, BasisBits_e123,  1,
        T_MetricList< BasisBits_e4,  BasisBits_e04, BasisBits_e4, BasisBits_e123, BasisBits_e123,  1, // NOTE: becomes -1 with canonical reordering!
        T_MetricList< BasisBits_e04, BasisBits_e0,  BasisBits_e0, BasisBits_e123, BasisBits_e123,  1, // NOTE: becomes -1 with canonical reordering!
        T_MetricList< BasisBits_e04, BasisBits_e4,  BasisBits_e4, BasisBits_e123, BasisBits_e123,  1,
        T_MetricList< BasisBits_e04, BasisBits_e04, BasisBits_s,  BasisBits_e123, BasisBits_e123, -1, // NOTE: becomes +1 with canonical reordering!
        
        VoidMetricList > > > > > > > > > > > > Conformal3D;

template < typename t_Metric >
inline Util::C_String GetString()
{
    return t_Metric::GetString();
}

//template <>
//inline Util::C_String GetString< Euclidean5D >()
//{
//    return Util::C_String( "Euclidean5D" );
//}

//template <>
//inline Util::C_String GetString< Minkowski5D >()
//{
//    return Util::C_String( "Minkowski5D" );
//}

template <>
inline Util::C_String GetString< Conformal3D >()
{
    return Util::C_String( "Conformal3D" );
}


#if defined( PLATFORM_PC )
#pragma warning( push )
#pragma warning( disable: 4200 )
#endif

template < ElementBits  t_ElementBits,
           unsigned int t_Dimensions,
           typename     t_Metric >
class T_Vector
{
public:
    
    typedef T_Vector< t_ElementBits, t_Dimensions, t_Metric > TypeVector;
    
    NOALIAS __forceinline T_Vector(); ///< Default constructor - undefined
    
    //NOALIAS __forceinline T_Vector( const TypeVector& copy ); ///< Copy constructor
    template < ElementBits t_CopyElementBits >
    NOALIAS __forceinline T_Vector( const T_Vector< t_CopyElementBits, t_Dimensions, t_Metric >& copy ); ///< Copy constructor
    
    //NOALIAS __forceinline const TypeVector& operator = ( const TypeVector& copy ); ///< Copy assignment
    template < ElementBits t_CopyElementBits >
    NOALIAS __forceinline const TypeVector& operator = ( const T_Vector< t_CopyElementBits, t_Dimensions, t_Metric >& copy ); ///< Copy assignment
    
    NOALIAS __forceinline T_Vector( const Vec4F32 VECREF v0 );
    NOALIAS __forceinline T_Vector( const Vec4F32 VECREF v0, const Vec4F32 VECREF v1 );
    NOALIAS __forceinline T_Vector( const Vec4F32 VECREF v0, const Vec4F32 VECREF v1, const Vec4F32 VECREF v2 );
    NOALIAS __forceinline T_Vector( const Vec4F32 VECREF v0, const Vec4F32 VECREF v1, const Vec4F32 VECREF v2, const Vec4F32 VECREF v3 );
    NOALIAS __forceinline T_Vector( const Vec4F32 VECREF v0, const Vec4F32 VECREF v1, const Vec4F32 VECREF v2, const Vec4F32 VECREF v3, const Vec4F32 VECREF v4 );
    NOALIAS __forceinline T_Vector( const Vec4F32 VECREF v0, const Vec4F32 VECREF v1, const Vec4F32 VECREF v2, const Vec4F32 VECREF v3, const Vec4F32 VECREF v4, const Vec4F32 VECREF v5 );
    NOALIAS __forceinline T_Vector( const Vec4F32 VECREF v0, const Vec4F32 VECREF v1, const Vec4F32 VECREF v2, const Vec4F32 VECREF v3, const Vec4F32 VECREF v4, const Vec4F32 VECREF v5, const Vec4F32 VECREF v6 );
    NOALIAS __forceinline T_Vector( const Vec4F32 VECREF v0, const Vec4F32 VECREF v1, const Vec4F32 VECREF v2, const Vec4F32 VECREF v3, const Vec4F32 VECREF v4, const Vec4F32 VECREF v5, const Vec4F32 VECREF v6, const Vec4F32 VECREF v7 );
    
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0 );                                                                ///< 1 element T_Vector constructor
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0, const VF32 VECREF v1 );                                           ///< 2 element T_Vector constructor
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2 );                      ///< 3 element T_Vector constructor
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2, const VF32 VECREF v3 ); ///< 4 element T_Vector constructor
    
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2, const VF32 VECREF v3, ///< 5 element T_Vector constructor
                                    const VF32 VECREF v4 );
    
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2, const VF32 VECREF v3, ///< 6 element T_Vector constructor
                                    const VF32 VECREF v4, const VF32 VECREF v5 );
    
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2, const VF32 VECREF v3, ///< 7 element T_Vector constructor
                                    const VF32 VECREF v4, const VF32 VECREF v5, const VF32 VECREF v6 );
    
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2, const VF32 VECREF v3, ///< 8 element T_Vector constructor
                                    const VF32 VECREF v4, const VF32 VECREF v5, const VF32 VECREF v6, const VF32 VECREF v7 );
    
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2, const VF32 VECREF v3, ///< 9 element T_Vector constructor
                                    const VF32 VECREF v4, const VF32 VECREF v5, const VF32 VECREF v6, const VF32 VECREF v7,
                                    const VF32 VECREF v8 );
    
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2, const VF32 VECREF v3, ///< 10 element T_Vector constructor
                                    const VF32 VECREF v4, const VF32 VECREF v5, const VF32 VECREF v6, const VF32 VECREF v7,
                                    const VF32 VECREF v8, const VF32 VECREF v9 );
    
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2, const VF32 VECREF v3, ///< 11 element T_Vector constructor
                                    const VF32 VECREF v4, const VF32 VECREF v5, const VF32 VECREF v6, const VF32 VECREF v7,
                                    const VF32 VECREF v8, const VF32 VECREF v9, const VF32 VECREF v10 );
    
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2,  const VF32 VECREF v3, ///< 12 element T_Vector constructor
                                    const VF32 VECREF v4, const VF32 VECREF v5, const VF32 VECREF v6,  const VF32 VECREF v7,
                                    const VF32 VECREF v8, const VF32 VECREF v9, const VF32 VECREF v10, const VF32 VECREF v11 );
    
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0, const VF32 VECREF v1, const VF32 VECREF v2,  const VF32 VECREF v3, ///< 13 element T_Vector constructor
                                    const VF32 VECREF v4, const VF32 VECREF v5, const VF32 VECREF v6,  const VF32 VECREF v7,
                                    const VF32 VECREF v8, const VF32 VECREF v9, const VF32 VECREF v10, const VF32 VECREF v11,
                                    const VF32 VECREF v12 );
    
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0,  const VF32 VECREF v1, const VF32 VECREF v2,  const VF32 VECREF v3, ///< 14 element T_Vector constructor
                                    const VF32 VECREF v4,  const VF32 VECREF v5, const VF32 VECREF v6,  const VF32 VECREF v7,
                                    const VF32 VECREF v8,  const VF32 VECREF v9, const VF32 VECREF v10, const VF32 VECREF v11,
                                    const VF32 VECREF v12, const VF32 VECREF v13 );
    
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0,  const VF32 VECREF v1,  const VF32 VECREF v2,  const VF32 VECREF v3, ///< 15 element T_Vector constructor
                                    const VF32 VECREF v4,  const VF32 VECREF v5,  const VF32 VECREF v6,  const VF32 VECREF v7,
                                    const VF32 VECREF v8,  const VF32 VECREF v9,  const VF32 VECREF v10, const VF32 VECREF v11,
                                    const VF32 VECREF v12, const VF32 VECREF v13, const VF32 VECREF v14 );
    
    NOALIAS __forceinline T_Vector( const VF32 VECREF v0,  const VF32 VECREF v1,  const VF32 VECREF v2,  const VF32 VECREF v3, ///< 16 element T_Vector constructor
                                    const VF32 VECREF v4,  const VF32 VECREF v5,  const VF32 VECREF v6,  const VF32 VECREF v7,
                                    const VF32 VECREF v8,  const VF32 VECREF v9,  const VF32 VECREF v10, const VF32 VECREF v11,
                                    const VF32 VECREF v12, const VF32 VECREF v13, const VF32 VECREF v14, const VF32 VECREF v15 );
    
    typedef t_Metric TypeMetric;                                                          ///< Metric of the Clifford algebra
    static const ElementBits  m_ElementBits = t_ElementBits;                              ///< Bitfield combination of ElementBits specifying the subset of basis elements that are stored in the vector
    static const unsigned int m_Dimensions  = t_Dimensions;                               ///< Dimensions of the Clifford algebra
    static const GradeBits    m_GradeBits   = T_GetElementGrade< t_ElementBits >::m_Bits; ///< Bitfield combination of GradeBits specifying the subset of grades that are stored in the vector
    static const unsigned int m_Elements    = T_BitCount< t_ElementBits >::m_Bits;        ///< Number of F32 elements stored in m_V
    static const unsigned int m_Vectors     = (m_Elements+3)/4;                           ///< Number of Vec4F32 elements stored in m_V
    
    //F32 m_V[ m_Elements ];
    Vec4F32 m_V[ m_Vectors ];
    
private:
    
    #if defined(PLATFORM_PC)
    
    NOALIAS __forceinline void CopyVec4( const TypeVector& src, const unsigned int I )
    {
        m_V[I] = src.m_V[I];
    }
    
    template < unsigned int I >
    NOALIAS __forceinline static void ForEachVec4(
        void (TypeVector::*f)( const TypeVector& s, const unsigned int i ),
        TypeVector* const __restrict pDst,
        const TypeVector& src )
    {
        (pDst->*f)( src, I );
        ForEachVec4<I+1>( f, pDst, src );
    }
    
    template <>
    NOALIAS __forceinline static void ForEachVec4< TypeVector::m_Vectors >(
        void (TypeVector::*f)( const TypeVector& s, const unsigned int i ),
        TypeVector* const __restrict pDst,
        const TypeVector& src )
    {
    }
    
    #endif
};

#if defined( PLATFORM_PC )
#pragma warning( pop )
#endif


typedef T_Vector< ElementBits_Scalar,         5, Conformal3D > Scalar;
typedef T_Vector< ElementBits_VectorC3,       5, Conformal3D > VectorC3;
typedef T_Vector< ElementBits_BiVectorC3,     5, Conformal3D > BiVectorC3;
typedef T_Vector< ElementBits_TriVectorC3,    5, Conformal3D > TriVectorC3;
typedef T_Vector< ElementBits_QuadVectorC3,   5, Conformal3D > QuadVectorC3;
typedef T_Vector< ElementBits_PseudoScalarC3, 5, Conformal3D > PseudoScalar;

//typedef T_Vector< ElementBits_VectorC3,     5, Conformal3D > VectorG1; // Grade 1 vector (Vector)
//typedef T_Vector< ElementBits_BiVectorC3,   5, Conformal3D > VectorG2; // Grade 2 vector (BiVector)
//typedef T_Vector< ElementBits_TriVectorC3,  5, Conformal3D > VectorG3; // Grade 3 vector (TriVector)
//typedef T_Vector< ElementBits_QuadVectorC3, 5, Conformal3D > VectorG4; // Grade 4 vector (QuadVector)

//typedef T_Vector< ElementBits_Scalar,         5, Conformal3D > Blade0;
//typedef T_Vector< ElementBits_VectorC3,       5, Conformal3D > Blade1;
//typedef T_Vector< ElementBits_BiVectorC3,     5, Conformal3D > Blade2;
//typedef T_Vector< ElementBits_TriVectorC3,    5, Conformal3D > Blade3;
//typedef T_Vector< ElementBits_QuadVectorC3,   5, Conformal3D > Blade4;
//typedef T_Vector< ElementBits_PseudoScalarC3, 5, Conformal3D > Blade5;

typedef T_Vector< ElementBits_MultiVector, 5, Conformal3D > MultiVector;

typedef T_Vector< ElementBits_e0, 5, Conformal3D > Origin;
typedef T_Vector< ElementBits_e1, 5, Conformal3D > Vector_e1;
typedef T_Vector< ElementBits_e2, 5, Conformal3D > Vector_e2;
typedef T_Vector< ElementBits_e3, 5, Conformal3D > Vector_e3;
typedef T_Vector< ElementBits_e4, 5, Conformal3D > Infinity;

typedef T_Vector< ElementBits_PseudoScalarE3, 5, Conformal3D > PseudoScalarE3; ///< Euclidean 3d pseudoscalar (Embedded in Conformal 3d)

typedef T_Vector< ElementBits_e1_e2_e3, 5, Conformal3D > Vector; ///< Vector

typedef T_Vector< ElementBits_eo_e1_e2_e3_ei, 5, Conformal3D > Point; ///< Weighted Point

typedef T_Vector< ElementBits_eo_e1_e2_e3_ei, 5, Conformal3D > PointN; ///< Normalised Point. (weight=1) No current way to model implicit 1.0 for 'no'!

//typedef T_Vector< (ElementBits)( ElementBits_e04 |
//                                 ElementBits_e14 | ElementBits_e24 | ElementBits_e34 ), 5, Conformal3D > FlatPoint; ///< Flat Point
typedef T_Vector< ElementBits_eoi_e1i_e2i_e3i, 5, Conformal3D > FlatPoint; ///< Flat Point

typedef T_Vector< (ElementBits)( ElementBits_e04 |
                                 ElementBits_e01 | ElementBits_e02 | ElementBits_e03 |
                                 ElementBits_e12 | ElementBits_e13 | ElementBits_e23 |
                                 ElementBits_e14 | ElementBits_e24 | ElementBits_e34 ), 5, Conformal3D > PointPair; ///< Point Pair

typedef T_Vector< (ElementBits)( ElementBits_e124 | ElementBits_e134 | ElementBits_e234 |
                                 ElementBits_e014 | ElementBits_e024 | ElementBits_e034 ), 5, Conformal3D > Line; ///< Line

typedef T_Vector< (ElementBits)( ElementBits_e12 | ElementBits_e13 | ElementBits_e23 |
                                 ElementBits_e14 | ElementBits_e24 | ElementBits_e34 ), 5, Conformal3D > DualLine; ///< Dual Line

typedef T_Vector< (ElementBits)( ElementBits_e0124 | ElementBits_e0134 | ElementBits_e0234 |
                                 ElementBits_e1234 ), 5, Conformal3D > Plane; ///< Plane

typedef T_Vector< ElementBits_e1_e2_e3_ei, 5, Conformal3D > DualPlane; ///< Dual Plane

typedef T_Vector< (ElementBits)( ElementBits_e124 | ElementBits_e134 | ElementBits_e234 |
                                 ElementBits_e014 | ElementBits_e024 | ElementBits_e034 |
                                 ElementBits_e012 | ElementBits_e013 | ElementBits_e023 |
                                 ElementBits_e123 ), 5, Conformal3D > Circle; ///< Circle

typedef T_Vector< (ElementBits)( ElementBits_e0123 |
                                 ElementBits_e0124 | ElementBits_e0134 | ElementBits_e0234 |
                                 ElementBits_e1234 ), 5, Conformal3D > Sphere; ///< Sphere

typedef T_Vector< ElementBits_eo_e1_e2_e3_ei, 5, Conformal3D > DualSphere; ///< Dual Sphere

typedef T_Vector< (ElementBits)( ElementBits_e14 | ElementBits_e24 | ElementBits_e34 ), 5, Conformal3D > FreeVector; ///< Free Vector

typedef T_Vector< (ElementBits)( ElementBits_e124 | ElementBits_e134 | ElementBits_e234 ), 5, Conformal3D > FreeBiVector; ///< Free BiVector

typedef T_Vector< ElementBits_e1234, 5, Conformal3D > FreeTriVector; ///< Free TriVector

typedef T_Vector< (ElementBits)( ElementBits_e04 |
                                 ElementBits_e01 | ElementBits_e02 | ElementBits_e03 |
                                 ElementBits_e12 | ElementBits_e13 | ElementBits_e23 |
                                 ElementBits_e14 | ElementBits_e24 | ElementBits_e34 ), 5, Conformal3D > TangentVector; ///< Tangent Vector

typedef T_Vector< (ElementBits)( ElementBits_e124 | ElementBits_e134 | ElementBits_e234 |
                                 ElementBits_e014 | ElementBits_e024 | ElementBits_e034 |
                                 ElementBits_e012 | ElementBits_e013 | ElementBits_e023 |
                                 ElementBits_e123 ), 5, Conformal3D > TangentBiVector; ///< Tangent BiVector

typedef T_Vector< (ElementBits)( ElementBits_s | ElementBits_e14 | ElementBits_e24 | ElementBits_e34 ), 5, Conformal3D > Translator;
typedef T_Vector< (ElementBits)( ElementBits_s | ElementBits_e12 | ElementBits_e13 | ElementBits_e23 ), 5, Conformal3D > Rotor;
typedef T_Vector< (ElementBits)( ElementBits_s | ElementBits_e04 ), 5, Conformal3D >                                     Scalor;





/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


