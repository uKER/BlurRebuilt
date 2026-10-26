#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_TEMPLATES_INL
#define SOFT_MATHS_HVEC_NEW_GA_TEMPLATES_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_TEMPLATES_INL
#define MATHS_HVEC_NEW_GA_TEMPLATES_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace GA /// 5d Geometric Algebra
{
/// @addtogroup GA
/// @{

#define BOOL_XOR !=


/// Template to get the BasisBits of the N'th element in a vector with the elements specified by T_ElementBits
template < ElementBits t_ElementBits, int N, unsigned int t_ElementIndex = 0 >
struct T_GetBasis
{
    static const BasisBits m_BasisBits =
        ( (N==0) && ( t_ElementBits & (1<<t_ElementIndex) ) )
            ? (BasisBits)t_ElementIndex
            : T_GetBasis< t_ElementBits, ( ( t_ElementBits & (1<<t_ElementIndex) ) ? N-1 : N ), t_ElementIndex+1 >::m_BasisBits;
};

template < ElementBits t_ElementBits, int N >
struct T_GetBasis< t_ElementBits, N, 32 >
{
    static const BasisBits m_BasisBits = BasisBits_Null;
};


/// T_MetricEval: primary definition
template < BasisBits    t_InLHS,
           BasisBits    t_InRHS,
           typename     t_Metric >
struct T_MetricEval
{
};

/// T_MetricEvaluate: for metric of type T_MetricList
template < BasisBits    t_InLHS,
           BasisBits    t_InRHS,
           BasisBits    t_MetricLHS,
           BasisBits    t_MetricRHS,
           BasisBits    t_MetricResult,
           BasisBits    t_MetricSignSwapLHS,
           BasisBits    t_MetricSignSwapRHS,
           int          t_MetricScale,
           typename     t_MetricTail >
struct T_MetricEval< t_InLHS,
                     t_InRHS,
                     T_MetricList< t_MetricLHS,
                                   t_MetricRHS,
                                   t_MetricResult,
                                   t_MetricSignSwapLHS,
                                   t_MetricSignSwapRHS,
                                   t_MetricScale,
                                   t_MetricTail > >
{
    typedef T_MetricEval< t_InLHS, t_InRHS, t_MetricTail > TypeNextEval;
    
    static const bool m_EntryFound = ( ( TypeNextEval::m_OutLHS & t_MetricLHS ) == t_MetricLHS ) &&
                                     ( ( TypeNextEval::m_OutRHS & t_MetricRHS ) == t_MetricRHS );
    
    // Count number of swaps needed to make annihilating elements adjacent
    //static const int m_SwapSign = ( ( T_BitCount< t_InLHS & t_MetricSignSwapLHS >::m_Bits +
    //                                  T_BitCount< t_InRHS & t_MetricSignSwapRHS >::m_Bits ) & 1 ) ? -1 : 1;
    static const int m_SwapSign = ( ( T_BitCount< TypeNextEval::m_OutLHS & t_MetricSignSwapLHS >::m_Bits +
                                      T_BitCount< TypeNextEval::m_OutRHS & t_MetricSignSwapRHS >::m_Bits ) & 1 ) ? -1 : 1;
        
    static const BasisBits m_OutLHS    = m_EntryFound ? (BasisBits)( TypeNextEval::m_OutLHS & ~t_MetricLHS ) : TypeNextEval::m_OutLHS;
    static const BasisBits m_OutRHS    = m_EntryFound ? (BasisBits)( TypeNextEval::m_OutRHS & ~t_MetricRHS ) : TypeNextEval::m_OutRHS;
    static const BasisBits m_OutResult = m_EntryFound ? (BasisBits)( TypeNextEval::m_OutResult | t_MetricResult ) : TypeNextEval::m_OutResult;
    //static const int       m_OutScale  = TypeNextEval::m_OutScale * ( m_EntryFound ? t_MetricScale * m_SwapSign : 1 );
    static const int       m_OutScale  = m_EntryFound ? t_MetricScale * m_SwapSign * TypeNextEval::m_OutScale : TypeNextEval::m_OutScale;
    static const bool      m_Dependent = TypeNextEval::m_Dependent || m_EntryFound;
    
    #if 0
    static inline void Display()
    {
        #if 0
        TypeNextEval::Display();
        if ( m_EntryFound )
        {
            printf( "T_MetricEval: MATCH:   LHS = 0x%02X, RHS = 0x%02X, scale = % 2d, dependent = %d ( matching elements: LHS = 0x%02X, RHS = 0x%02X, scale = % 2d )\n",
                m_OutLHS,
                m_OutRHS,
                m_OutScale,
                true,
                t_MetricLHS,
                t_MetricRHS,
                t_MetricScale );
        }
        #endif
    }
    #endif
};

/// T_MetricEval: T_MetricList terminator
template < BasisBits    t_InLHS,
           BasisBits    t_InRHS >
struct T_MetricEval< t_InLHS,
                     t_InRHS,
                     T_MetricList< VoidMetricList::m_LHS,
                                   VoidMetricList::m_RHS,
                                   VoidMetricList::m_Result,
                                   VoidMetricList::m_SignSwapLHS,
                                   VoidMetricList::m_SignSwapRHS,
                                   VoidMetricList::m_Scale,
                                   typename VoidMetricList::TypeTail > >
{
    static const BasisBits m_OutLHS    = t_InLHS;
    static const BasisBits m_OutRHS    = t_InRHS;
    static const BasisBits m_OutResult = BasisBits_s;
    static const int       m_OutScale  = 1;
    static const bool      m_Dependent = false;
    
    #if 0
    static inline void Display()
    {
    }
    #endif
};


template < BasisBits t_LHS, BasisBits t_RHS, typename t_Metric >
struct T_MetricOp
{
    typedef T_MetricEval< t_LHS, t_RHS, t_Metric > TypeMetricEval;
    
    static const BasisBits m_LHS       = TypeMetricEval::m_OutLHS;
    static const BasisBits m_RHS       = TypeMetricEval::m_OutRHS;
    static const BasisBits m_Result    = (BasisBits)( TypeMetricEval::m_OutLHS | TypeMetricEval::m_OutRHS | TypeMetricEval::m_OutResult );
    static const bool      m_Dependent = TypeMetricEval::m_Dependent;
    static const int       m_Scale     = ( m_Dependent ? TypeMetricEval::m_OutScale : 0 );
    
    #if 0
    static inline void Display()
    {
        #if 0
        printf( "T_MetricEval: INITIAL: LHS = 0x%02X, RHS = 0x%02X, scale = % 2d, dependent = %d\n",
            t_LHS,
            t_RHS,
            1,
            false );
        TypeMetricEval::Display();
        printf( "T_MetricEval: RESULT:  LHS = 0x%02X, RHS = 0x%02X, scale = % 2d, dependent = %d\n",
            m_LHS,
            m_RHS,
            m_Scale,
            m_Dependent );
        #endif
    }
    #endif
};


template < unsigned int t_LHS, unsigned int t_RHS >
struct T_SubCanonicalReorderSign
{
    static const unsigned int m_Swaps = T_SubCanonicalReorderSign< (t_LHS>>1), t_RHS >::m_Swaps +
                                        T_BitCount< t_LHS & t_RHS >::m_Bits;
};

template < unsigned int t_RHS >
struct T_SubCanonicalReorderSign< BasisBits_s, t_RHS >
{
    static const unsigned int m_Swaps = 0;
};

template < BasisBits t_LHS, BasisBits t_RHS >
struct T_CanonicalReorderSign
{
    static const bool m_Negated = ( T_SubCanonicalReorderSign< (t_LHS>>1), t_RHS >::m_Swaps & 1 ) != 0;
    static const int  m_Scale = ( m_Negated ? -1 : 1 );
};

template < BasisBits t_RHS >
struct T_CanonicalReorderSign< BasisBits_s, t_RHS >
{
    static const bool m_Negated = false;
    static const int  m_Scale = 1;
};


template < BasisBits t_LHS,
           BasisBits t_RHS,
           BasisBits t_Result,
           bool      t_Negated,
           bool      t_Null >
struct T_Op
{
    static const BasisBits m_SrcLhsBits = t_LHS;
    static const BasisBits m_SrcRhsBits = t_RHS;
    static const BasisBits m_ResultBits = t_Result;
    static const bool      m_Negated    = t_Negated;
    static const bool      m_Null       = t_Null;
    
    #if 0
    static inline void Display()
    {
        #if 0
        printf( "Op: LHS bits = 0x%02X, RHS bits = 0x%02X, RESULT bits = 0x%02X, negated = %d\n",
            m_SrcLhsBits,
            m_SrcRhsBits,
            m_ResultBits,
            m_Negated );
        #endif
    }
    #endif
};


/// Template list: primary definition
template < typename t_Head, typename t_Tail >
struct T_List
{
    typedef t_Head TypeHead;
    typedef t_Tail TypeTail;
    
    #if 0
    static inline void Display()
    {
        #if 0
        TypeHead::Display();
        TypeTail::Display();
        #endif
    }
    #endif
};

/// Template list: void terminator
//template < typename t_Head >
//struct T_List< t_Head, void >
//{
//    typedef t_Head TypeHead;
//    typedef void   TypeTail;
//    
//    static inline void Display()
//    {
//        TypeHead::Display();
//    }
//};

/// Template list: void list
template <>
struct T_List< void, void >
{
    typedef void TypeHead;
    typedef void TypeTail;
    
    #if 0
    static inline void Display()
    {
    }
    #endif
};

typedef T_List< void, void > VoidList;


/// Append T_Op to template list
template < BasisBits t_LHS,
           BasisBits t_RHS,
           BasisBits t_Result,
           bool      t_Negated,
           bool      t_Null,
           typename  t_Tail >
struct T_Append
{
    typedef T_List< T_Op< t_LHS, t_RHS, t_Result, t_Negated, t_Null >,
                          t_Tail > TypeResult;
};

/// Append T_Op to template list: specialisation, eliminates null ops
template < BasisBits t_LHS,
           BasisBits t_RHS,
           BasisBits t_Result,
           bool      t_Negated,
           typename  t_Tail >
struct T_Append< t_LHS, t_RHS, t_Result, t_Negated, true, t_Tail >
{
    typedef t_Tail TypeResult;
};


/// T_OpStruct: contains 32 lists of ops, one for each result type
template < typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_OpStruct
{
    typedef t_OpList_s      TypeOpList_s;
    typedef t_OpList_e0     TypeOpList_e0;
    typedef t_OpList_e1     TypeOpList_e1;
    typedef t_OpList_e2     TypeOpList_e2;
    typedef t_OpList_e3     TypeOpList_e3;
    typedef t_OpList_e4     TypeOpList_e4;
    typedef t_OpList_e01    TypeOpList_e01;
    typedef t_OpList_e02    TypeOpList_e02;
    typedef t_OpList_e03    TypeOpList_e03;
    typedef t_OpList_e04    TypeOpList_e04;
    typedef t_OpList_e12    TypeOpList_e12;
    typedef t_OpList_e13    TypeOpList_e13;
    typedef t_OpList_e14    TypeOpList_e14;
    typedef t_OpList_e23    TypeOpList_e23;
    typedef t_OpList_e24    TypeOpList_e24;
    typedef t_OpList_e34    TypeOpList_e34;
    typedef t_OpList_e012   TypeOpList_e012;
    typedef t_OpList_e013   TypeOpList_e013;
    typedef t_OpList_e014   TypeOpList_e014;
    typedef t_OpList_e023   TypeOpList_e023;
    typedef t_OpList_e024   TypeOpList_e024;
    typedef t_OpList_e034   TypeOpList_e034;
    typedef t_OpList_e123   TypeOpList_e123;
    typedef t_OpList_e124   TypeOpList_e124;
    typedef t_OpList_e134   TypeOpList_e134;
    typedef t_OpList_e234   TypeOpList_e234;
    typedef t_OpList_e0123  TypeOpList_e0123;
    typedef t_OpList_e0124  TypeOpList_e0124;
    typedef t_OpList_e0134  TypeOpList_e0134;
    typedef t_OpList_e0234  TypeOpList_e0234;
    typedef t_OpList_e1234  TypeOpList_e1234;
    typedef t_OpList_e01234 TypeOpList_e01234;
    
    static const ElementBits m_ElementBits = t_ElementBits;
};

typedef T_OpStruct< VoidList, VoidList, VoidList, VoidList, VoidList, VoidList, VoidList, VoidList,
                    VoidList, VoidList, VoidList, VoidList, VoidList, VoidList, VoidList, VoidList,
                    VoidList, VoidList, VoidList, VoidList, VoidList, VoidList, VoidList, VoidList,
                    VoidList, VoidList, VoidList, VoidList, VoidList, VoidList, VoidList, VoidList,
                    ElementBits_Null > VoidOpStruct;


/// Add T_Op to T_OpStruct: primary definition
template < BasisBits t_LHS,
           BasisBits t_RHS,
           BasisBits t_Result,
           bool      t_Negated,
           bool      t_Null,
           typename  t_OpStruct >
struct T_Add
{
};

/// Add T_Op to T_OpStruct: eliminates null ops
template < BasisBits t_LHS,
           BasisBits t_RHS,
           BasisBits t_Result,
           bool      t_Negated,
           typename  t_OpStruct >
struct T_Add< t_LHS, t_RHS, t_Result, t_Negated, true, t_OpStruct >
{
    typedef t_OpStruct TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_s
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_s, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_s, t_Negated, false, t_OpList_s >::TypeResult TypeNewList;
    
    typedef T_OpStruct< TypeNewList,    t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_s) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e0
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e0, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e0, t_Negated, false, t_OpList_e0 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     TypeNewList,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e0) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e1
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e1, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e1, t_Negated, false, t_OpList_e1 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    TypeNewList,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e1) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e2
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e2, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e2, t_Negated, false, t_OpList_e2 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    TypeNewList,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e2) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e3
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e3, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e3, t_Negated, false, t_OpList_e3 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        TypeNewList,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e3) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e4
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e4, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e4, t_Negated, false, t_OpList_e4 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    TypeNewList,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e4) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e01
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e01, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e01, t_Negated, false, t_OpList_e01 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    TypeNewList,    t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e01) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e02
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e02, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e02, t_Negated, false, t_OpList_e02 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   TypeNewList,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e02) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e03
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e03, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e03, t_Negated, false, t_OpList_e03 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        TypeNewList,    t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e03) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e04
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e04, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e04, t_Negated, false, t_OpList_e04 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   TypeNewList,    t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e04) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e12
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e12, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e12, t_Negated, false, t_OpList_e12 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   TypeNewList,    t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e12) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e13
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e13, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e13, t_Negated, false, t_OpList_e13 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   TypeNewList,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e13) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e14
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e14, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e14, t_Negated, false, t_OpList_e14 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        TypeNewList,    t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e14) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e23
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e23, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e23, t_Negated, false, t_OpList_e23 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   TypeNewList,    t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e23) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e24
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e24, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e24, t_Negated, false, t_OpList_e24 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   TypeNewList,    t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e24) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e34
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e34, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e34, t_Negated, false, t_OpList_e34 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   TypeNewList,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e34) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e012
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e012, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e012, t_Negated, false, t_OpList_e012 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        TypeNewList,    t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e012) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e013
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e013, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e013, t_Negated, false, t_OpList_e013 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  TypeNewList,    t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e013) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e014
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e014, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e014, t_Negated, false, t_OpList_e014 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  TypeNewList,    t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e014) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e023
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e023, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e023, t_Negated, false, t_OpList_e023 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  TypeNewList,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e023) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e024
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e024, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e024, t_Negated, false, t_OpList_e024 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        TypeNewList,    t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e024) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e034
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e034, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e034, t_Negated, false, t_OpList_e034 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  TypeNewList,    t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e034) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e123
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e123, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e123, t_Negated, false, t_OpList_e123 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  TypeNewList,    t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e123) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e124
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e124, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e124, t_Negated, false, t_OpList_e124 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  TypeNewList,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e124) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e134
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e134, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e134, t_Negated, false, t_OpList_e134 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        TypeNewList,    t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e134) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e234
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e234, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e234, t_Negated, false, t_OpList_e234 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  TypeNewList,    t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e234) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e0123
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e0123, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e0123, t_Negated, false, t_OpList_e0123 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  TypeNewList,    t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e0123) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e0124
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e0124, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e0124, t_Negated, false, t_OpList_e0124 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, TypeNewList,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e0124) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e0134
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e0134, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e0134, t_Negated, false, t_OpList_e0134 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        TypeNewList,    t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e0134) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e0234
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e0234, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e0234, t_Negated, false, t_OpList_e0234 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, TypeNewList,    t_OpList_e1234, t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e0234) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e1234
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e1234, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e1234, t_Negated, false, t_OpList_e1234 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, TypeNewList,    t_OpList_e01234,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e1234) ) > TypeResult;
};

/// Add T_Op to T_OpStruct: BasisBits_e01234
template < BasisBits t_LHS, BasisBits t_RHS, bool t_Negated,
           typename t_OpList_s,     typename t_OpList_e0,    typename t_OpList_e1,    typename t_OpList_e2,
           typename t_OpList_e3,    typename t_OpList_e4,    typename t_OpList_e01,   typename t_OpList_e02,
           typename t_OpList_e03,   typename t_OpList_e04,   typename t_OpList_e12,   typename t_OpList_e13,
           typename t_OpList_e14,   typename t_OpList_e23,   typename t_OpList_e24,   typename t_OpList_e34,
           typename t_OpList_e012,  typename t_OpList_e013,  typename t_OpList_e014,  typename t_OpList_e023,
           typename t_OpList_e024,  typename t_OpList_e034,  typename t_OpList_e123,  typename t_OpList_e124,
           typename t_OpList_e134,  typename t_OpList_e234,  typename t_OpList_e0123, typename t_OpList_e0124,
           typename t_OpList_e0134, typename t_OpList_e0234, typename t_OpList_e1234, typename t_OpList_e01234,
           ElementBits t_ElementBits >
struct T_Add< t_LHS, t_RHS, BasisBits_e01234, t_Negated, false,
              T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                          t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                          t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                          t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                          t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                          t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                          t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                          t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, t_OpList_e01234,
                          t_ElementBits > >
{
    typedef typename T_Append< t_LHS, t_RHS, BasisBits_e01234, t_Negated, false, t_OpList_e01234 >::TypeResult TypeNewList;
    
    typedef T_OpStruct< t_OpList_s,     t_OpList_e0,    t_OpList_e1,    t_OpList_e2,
                        t_OpList_e3,    t_OpList_e4,    t_OpList_e01,   t_OpList_e02,
                        t_OpList_e03,   t_OpList_e04,   t_OpList_e12,   t_OpList_e13,
                        t_OpList_e14,   t_OpList_e23,   t_OpList_e24,   t_OpList_e34,
                        t_OpList_e012,  t_OpList_e013,  t_OpList_e014,  t_OpList_e023,
                        t_OpList_e024,  t_OpList_e034,  t_OpList_e123,  t_OpList_e124,
                        t_OpList_e134,  t_OpList_e234,  t_OpList_e0123, t_OpList_e0124,
                        t_OpList_e0134, t_OpList_e0234, t_OpList_e1234, TypeNewList,
                        (ElementBits)( t_ElementBits | (1<<BasisBits_e01234) ) > TypeResult;
};



template < typename     t_OpList,
           unsigned int t_Dimensions,
           typename     t_Metric,
           ElementBits  t_ElementLHS,
           ElementBits  t_ElementRHS >
struct T_Process
{
};

template < BasisBits    t_LHS,
           BasisBits    t_RHS,
           BasisBits    t_Result,
           bool         t_Negated,
           typename     t_Tail,
           unsigned int t_Dimensions,
           typename     t_Metric,
           ElementBits  t_ElementLHS,
           ElementBits  t_ElementRHS >
struct T_Process< T_List< T_Op< t_LHS, t_RHS, t_Result, t_Negated, false >,
                          t_Tail >,
                  t_Dimensions,
                  t_Metric,
                  t_ElementLHS,
                  t_ElementRHS >
{
    NOALIAS __forceinline static const VF32 Process(
        const T_Vector< t_ElementLHS, t_Dimensions, t_Metric >& lhs,
        const T_Vector< t_ElementRHS, t_Dimensions, t_Metric >& rhs,
        const bool                                              generateCode = false )
    {
        #if defined( MATHS_GA_CODEGEN )
        if ( generateCode )
        {
            if ( t_Negated ) printf( "NegMulSub( Element<%s>(lhs), Element<%s>(rhs), ", GA::GetString( t_LHS ), GA::GetString( t_RHS ) );
            else             printf( "MulAdd( Element<%s>(lhs), Element<%s>(rhs), ", GA::GetString( t_LHS ), GA::GetString( t_RHS ) );
        }
        #endif
        
        //printf( "Process(): List< Op, List >: (a*b+c)\n" );
        const VF32 a( Element< t_LHS >( lhs ) );
        const VF32 b( Element< t_RHS >( rhs ) );
        const VF32 c( T_Process< t_Tail, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS >::Process( lhs, rhs, generateCode ) );
        
        #if defined( MATHS_GA_CODEGEN )
        if ( generateCode )
        {
            printf( " )" );
        }
        #endif
        
        if ( t_Negated )
        {
            return NegMulSub( a, b, c );
        }
        else
        {
            return MulAdd( a, b, c );
        }
    }
};

template < BasisBits    t_LHS,
           BasisBits    t_RHS,
           BasisBits    t_Result,
           bool         t_Negated,
           unsigned int t_Dimensions,
           typename     t_Metric,
           ElementBits  t_ElementLHS,
           ElementBits  t_ElementRHS >
struct T_Process< T_List< T_Op< t_LHS, t_RHS, t_Result, t_Negated, false >,
                          VoidList >,
                  t_Dimensions,
                  t_Metric,
                  t_ElementLHS,
                  t_ElementRHS >
{
    NOALIAS __forceinline static const VF32 Process(
        const T_Vector< t_ElementLHS, t_Dimensions, t_Metric >& lhs,
        const T_Vector< t_ElementRHS, t_Dimensions, t_Metric >& rhs,
        const bool                                              generateCode = false )
    {
        #if defined( MATHS_GA_CODEGEN )
        if ( generateCode )
        {
            if ( t_Negated ) printf( "NegMulSub( Element<%s>(lhs), Element<%s>(rhs), Const<VF32>::Zero() )", GA::GetString( t_LHS ), GA::GetString( t_RHS ) );
            else             printf( "Element<%s>(lhs) * Element<%s>(rhs)", GA::GetString( t_LHS ), GA::GetString( t_RHS ) );
        }
        #endif
        
        //printf( "Process(): List< Op, VoidList >: (a*b)\n" );
        const VF32 a( Element< t_LHS >( lhs ) );
        const VF32 b( Element< t_RHS >( rhs ) );
        
        if ( t_Negated )
        {
            //return -( a * b );
            const VF32 zero( Const<VF32>::Zero() );
            return NegMulSub( a, b, zero );
        }
        else
        {
            return a * b;
        }
    }
};

template < unsigned int t_Dimensions,
           typename     t_Metric,
           ElementBits  t_ElementLHS,
           ElementBits  t_ElementRHS >
struct T_Process< VoidList,
                  t_Dimensions,
                  t_Metric,
                  t_ElementLHS,
                  t_ElementRHS >
{
    NOALIAS __forceinline static const VF32 Process(
        const T_Vector< t_ElementLHS, t_Dimensions, t_Metric >& lhs,
        const T_Vector< t_ElementRHS, t_Dimensions, t_Metric >& rhs,
        const bool                                              generateCode = false )
    {
        //printf( "Process(): VoidList: (null)\n" );
        return Const<VF32>::Zero();
    }
};



template < BasisBits    t_Basis,
           typename     t_OpList,
           unsigned int t_Dimensions,
           typename     t_Metric,
           ElementBits  t_ElementLHS,
           ElementBits  t_ElementRHS,
           ElementBits  t_ElementResult >
struct T_ProcessElement
{
    NOALIAS __forceinline static void ProcessElement(
        const T_Vector< t_ElementLHS,    t_Dimensions, t_Metric >& lhs,
        const T_Vector< t_ElementRHS,    t_Dimensions, t_Metric >& rhs,
              T_Vector< t_ElementResult, t_Dimensions, t_Metric >* pResult,
        const bool                                                 generateCode )
    {
        if ( t_ElementResult & (1<<t_Basis) )
        {
            #if defined( MATHS_GA_CODEGEN )
            if ( generateCode )
            {
                printf( "    SetElement< %s >( &result, ", GA::GetString( t_Basis ) );
            }
            #endif
            
            SetElement< t_Basis >( pResult, T_Process< t_OpList, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS >::Process( lhs, rhs, generateCode ) );
            
            #if defined( MATHS_GA_CODEGEN )
            if ( generateCode )
            {
                printf( " );\n" );
            }
            #endif
        }
    }
};




/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )



