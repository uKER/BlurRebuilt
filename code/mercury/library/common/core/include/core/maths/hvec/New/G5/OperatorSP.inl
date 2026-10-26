#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_SP_INL
#define SOFT_MATHS_HVEC_NEW_GA_SP_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_SP_INL
#define MATHS_HVEC_NEW_GA_SP_INL
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


template < BasisBits t_LHS, BasisBits t_RHS >
struct T_SP_Outer
{
    static const BasisBits m_Result  = (BasisBits)( t_LHS ^ t_RHS );
    static const int       m_Scale   = T_CanonicalReorderSign< t_LHS, t_RHS >::m_Scale;
    static const bool      m_Negated = ( m_Scale == -1 );
    static const bool      m_Null    = ( ( t_LHS & t_RHS ) != 0 ) || // If LHS & RHS have basis in common, result is null!
                                       ( m_Result != BasisBits_s );  // Only scalar results valid
};

template < BasisBits t_LHS, BasisBits t_RHS, typename t_Metric >
struct T_SP_Inner
{
    typedef T_MetricOp< t_LHS, t_RHS, t_Metric > TypeMetricOp;
    
    static const BasisBits m_Result  = TypeMetricOp::m_Result;
    static const int       m_Scale   = T_CanonicalReorderSign< t_LHS, t_RHS >::m_Scale * TypeMetricOp::m_Scale;
    static const bool      m_Negated = ( m_Scale == -1 );
    static const bool      m_Null    = ( m_Scale == 0 ) ||
                                       ( m_Result != BasisBits_s ); // Only scalar results valid
    
    #if 0
    static inline void Display()
    {
        #if 0
        TypeMetricOp::Display();
        printf( "T_SP_Inner: LHS bits = 0x%02X, RHS bits = 0x%02X, RESULT bits = 0x%02X, scale = % 2d, negated = %d, null = %d\n",
            t_LHS,
            t_RHS,
            m_Result,
            m_Scale,
            m_Negated,
            m_Null );
        #endif
    }
    #endif
};

/// T_Vector SP Evaluator: sub2
template < typename    t_Metric,
           BasisBits   t_LHS,
           BasisBits   t_RHS,
           typename    t_OpList0 >
struct T_SP_Eval2
{
    typedef T_SP_Outer< t_LHS, t_RHS > OP;
    typedef T_SP_Inner< t_LHS, t_RHS, t_Metric > IP;
        
    typedef typename T_Append< t_LHS, t_RHS, OP::m_Result, OP::m_Negated, OP::m_Null, t_OpList0   >::TypeResult TypeOpList1;
    typedef typename T_Append< t_LHS, t_RHS, IP::m_Result, IP::m_Negated, IP::m_Null, TypeOpList1 >::TypeResult TypeOpList;
    
    #if 0
    static inline void DisplayMetric()
    {
        #if 0
        IP::Display();
        #endif
    }
    #endif
};

/// T_Vector SP Evaluator: sub2: invalid op terminator
template < typename    t_Metric,
           BasisBits   t_LHS,
           typename    t_OpList >
struct T_SP_Eval2< t_Metric, t_LHS, BasisBits_Null, t_OpList >
{
    typedef t_OpList TypeOpList;
    
    #if 0
    static inline void DisplayMetric()
    {
    }
    #endif
};

/// T_Vector SP Evaluator: sub1
template < typename    t_Metric,
           BasisBits   t_LHS,
           ElementBits t_ElementRHS,
           typename    t_OpList >
struct T_SP_Eval1
{
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  0 >::m_BasisBits, t_OpList   >::TypeOpList TypeEval_0;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  1 >::m_BasisBits, TypeEval_0 >::TypeOpList TypeEval_1;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  2 >::m_BasisBits, TypeEval_1 >::TypeOpList TypeEval_2;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  3 >::m_BasisBits, TypeEval_2 >::TypeOpList TypeEval_3;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  4 >::m_BasisBits, TypeEval_3 >::TypeOpList TypeEval_4;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  5 >::m_BasisBits, TypeEval_4 >::TypeOpList TypeEval_5;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  6 >::m_BasisBits, TypeEval_5 >::TypeOpList TypeEval_6;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  7 >::m_BasisBits, TypeEval_6 >::TypeOpList TypeEval_7;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  8 >::m_BasisBits, TypeEval_7 >::TypeOpList TypeEval_8;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  9 >::m_BasisBits, TypeEval_8 >::TypeOpList TypeEval_9;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 10 >::m_BasisBits, TypeEval_9 >::TypeOpList TypeEval10;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 11 >::m_BasisBits, TypeEval10 >::TypeOpList TypeEval11;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 12 >::m_BasisBits, TypeEval11 >::TypeOpList TypeEval12;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 13 >::m_BasisBits, TypeEval12 >::TypeOpList TypeEval13;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 14 >::m_BasisBits, TypeEval13 >::TypeOpList TypeEval14;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 15 >::m_BasisBits, TypeEval14 >::TypeOpList TypeEval15;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 16 >::m_BasisBits, TypeEval15 >::TypeOpList TypeEval16;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 17 >::m_BasisBits, TypeEval16 >::TypeOpList TypeEval17;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 18 >::m_BasisBits, TypeEval17 >::TypeOpList TypeEval18;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 19 >::m_BasisBits, TypeEval18 >::TypeOpList TypeEval19;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 20 >::m_BasisBits, TypeEval19 >::TypeOpList TypeEval20;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 21 >::m_BasisBits, TypeEval20 >::TypeOpList TypeEval21;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 22 >::m_BasisBits, TypeEval21 >::TypeOpList TypeEval22;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 23 >::m_BasisBits, TypeEval22 >::TypeOpList TypeEval23;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 24 >::m_BasisBits, TypeEval23 >::TypeOpList TypeEval24;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 25 >::m_BasisBits, TypeEval24 >::TypeOpList TypeEval25;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 26 >::m_BasisBits, TypeEval25 >::TypeOpList TypeEval26;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 27 >::m_BasisBits, TypeEval26 >::TypeOpList TypeEval27;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 28 >::m_BasisBits, TypeEval27 >::TypeOpList TypeEval28;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 29 >::m_BasisBits, TypeEval28 >::TypeOpList TypeEval29;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 30 >::m_BasisBits, TypeEval29 >::TypeOpList TypeEval30;
    typedef typename T_SP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 31 >::m_BasisBits, TypeEval30 >::TypeOpList TypeOpList;
    
    #if 0
    static inline void DisplayMetric()
    {
        #if 0
        TypeEval_0::DisplayMetric();
        TypeEval_1::DisplayMetric();
        TypeEval_2::DisplayMetric();
        TypeEval_3::DisplayMetric();
        TypeEval_4::DisplayMetric();
        TypeEval_5::DisplayMetric();
        TypeEval_6::DisplayMetric();
        TypeEval_7::DisplayMetric();
        TypeEval_8::DisplayMetric();
        TypeEval_9::DisplayMetric();
        TypeEval10::DisplayMetric();
        TypeEval11::DisplayMetric();
        TypeEval12::DisplayMetric();
        TypeEval13::DisplayMetric();
        TypeEval14::DisplayMetric();
        TypeEval15::DisplayMetric();
        TypeEval16::DisplayMetric();
        TypeEval17::DisplayMetric();
        TypeEval18::DisplayMetric();
        TypeEval19::DisplayMetric();
        TypeEval20::DisplayMetric();
        TypeEval21::DisplayMetric();
        TypeEval22::DisplayMetric();
        TypeEval23::DisplayMetric();
        TypeEval24::DisplayMetric();
        TypeEval25::DisplayMetric();
        TypeEval26::DisplayMetric();
        TypeEval27::DisplayMetric();
        TypeEval28::DisplayMetric();
        TypeEval29::DisplayMetric();
        TypeEval30::DisplayMetric();
        TypeEval31::DisplayMetric();
        #endif
    }
    #endif
};

/// T_Vector SP Evaluator: sub1: invalid op terminator
template < typename    t_Metric,
           ElementBits t_ElementRHS,
           typename    t_OpList >
struct T_SP_Eval1< t_Metric, BasisBits_Null, t_ElementRHS, t_OpList >
{
    typedef t_OpList TypeOpList;
    
    #if 0
    static inline void DisplayMetric()
    {
    }
    #endif
};

/// T_Vector SP Evaluator: root
template < typename    t_Metric,
           ElementBits t_ElementLHS,
           ElementBits t_ElementRHS >
struct T_SP_Eval
{
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  0 >::m_BasisBits, t_ElementRHS, VoidList   >::TypeOpList TypeEval_0;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  1 >::m_BasisBits, t_ElementRHS, TypeEval_0 >::TypeOpList TypeEval_1;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  2 >::m_BasisBits, t_ElementRHS, TypeEval_1 >::TypeOpList TypeEval_2;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  3 >::m_BasisBits, t_ElementRHS, TypeEval_2 >::TypeOpList TypeEval_3;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  4 >::m_BasisBits, t_ElementRHS, TypeEval_3 >::TypeOpList TypeEval_4;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  5 >::m_BasisBits, t_ElementRHS, TypeEval_4 >::TypeOpList TypeEval_5;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  6 >::m_BasisBits, t_ElementRHS, TypeEval_5 >::TypeOpList TypeEval_6;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  7 >::m_BasisBits, t_ElementRHS, TypeEval_6 >::TypeOpList TypeEval_7;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  8 >::m_BasisBits, t_ElementRHS, TypeEval_7 >::TypeOpList TypeEval_8;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  9 >::m_BasisBits, t_ElementRHS, TypeEval_8 >::TypeOpList TypeEval_9;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 10 >::m_BasisBits, t_ElementRHS, TypeEval_9 >::TypeOpList TypeEval10;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 11 >::m_BasisBits, t_ElementRHS, TypeEval10 >::TypeOpList TypeEval11;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 12 >::m_BasisBits, t_ElementRHS, TypeEval11 >::TypeOpList TypeEval12;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 13 >::m_BasisBits, t_ElementRHS, TypeEval12 >::TypeOpList TypeEval13;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 14 >::m_BasisBits, t_ElementRHS, TypeEval13 >::TypeOpList TypeEval14;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 15 >::m_BasisBits, t_ElementRHS, TypeEval14 >::TypeOpList TypeEval15;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 16 >::m_BasisBits, t_ElementRHS, TypeEval15 >::TypeOpList TypeEval16;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 17 >::m_BasisBits, t_ElementRHS, TypeEval16 >::TypeOpList TypeEval17;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 18 >::m_BasisBits, t_ElementRHS, TypeEval17 >::TypeOpList TypeEval18;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 19 >::m_BasisBits, t_ElementRHS, TypeEval18 >::TypeOpList TypeEval19;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 20 >::m_BasisBits, t_ElementRHS, TypeEval19 >::TypeOpList TypeEval20;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 21 >::m_BasisBits, t_ElementRHS, TypeEval20 >::TypeOpList TypeEval21;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 22 >::m_BasisBits, t_ElementRHS, TypeEval21 >::TypeOpList TypeEval22;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 23 >::m_BasisBits, t_ElementRHS, TypeEval22 >::TypeOpList TypeEval23;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 24 >::m_BasisBits, t_ElementRHS, TypeEval23 >::TypeOpList TypeEval24;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 25 >::m_BasisBits, t_ElementRHS, TypeEval24 >::TypeOpList TypeEval25;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 26 >::m_BasisBits, t_ElementRHS, TypeEval25 >::TypeOpList TypeEval26;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 27 >::m_BasisBits, t_ElementRHS, TypeEval26 >::TypeOpList TypeEval27;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 28 >::m_BasisBits, t_ElementRHS, TypeEval27 >::TypeOpList TypeEval28;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 29 >::m_BasisBits, t_ElementRHS, TypeEval28 >::TypeOpList TypeEval29;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 30 >::m_BasisBits, t_ElementRHS, TypeEval29 >::TypeOpList TypeEval30;
    typedef typename T_SP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 31 >::m_BasisBits, t_ElementRHS, TypeEval30 >::TypeOpList TypeOpList;
    
    #if 0
    static inline void DisplayMetric()
    {
        #if 0
        TypeEval_0::DisplayMetric();
        TypeEval_1::DisplayMetric();
        TypeEval_2::DisplayMetric();
        TypeEval_3::DisplayMetric();
        TypeEval_4::DisplayMetric();
        TypeEval_5::DisplayMetric();
        TypeEval_6::DisplayMetric();
        TypeEval_7::DisplayMetric();
        TypeEval_8::DisplayMetric();
        TypeEval_9::DisplayMetric();
        TypeEval10::DisplayMetric();
        TypeEval11::DisplayMetric();
        TypeEval12::DisplayMetric();
        TypeEval13::DisplayMetric();
        TypeEval14::DisplayMetric();
        TypeEval15::DisplayMetric();
        TypeEval16::DisplayMetric();
        TypeEval17::DisplayMetric();
        TypeEval18::DisplayMetric();
        TypeEval19::DisplayMetric();
        TypeEval20::DisplayMetric();
        TypeEval21::DisplayMetric();
        TypeEval22::DisplayMetric();
        TypeEval23::DisplayMetric();
        TypeEval24::DisplayMetric();
        TypeEval25::DisplayMetric();
        TypeEval26::DisplayMetric();
        TypeEval27::DisplayMetric();
        TypeEval28::DisplayMetric();
        TypeEval29::DisplayMetric();
        TypeEval30::DisplayMetric();
        TypeEval31::DisplayMetric();
        #endif
    }
    #endif
    
    #if 0
    static void Display()
    {
        #if 0
        TypeOpList::Display();
        //printf( "Metric:\n" );
        //DisplayMetric();
        //printf( "\n" );
        #endif
    }
    #endif
};


/// T_Vector Scalar Product
template < unsigned int t_Dimensions,
           typename     t_Metric,
           ElementBits  t_ElementLHS,
           ElementBits  t_ElementRHS >
NOALIAS GA_OPERATOR_INLINE
    const Scalar
    operator % ( const T_Vector< t_ElementLHS, t_Dimensions, t_Metric >& lhs,
                 const T_Vector< t_ElementRHS, t_Dimensions, t_Metric >& rhs )
{
    // Validate that lhs & rhs are vectors of the same grade!
    BIZ_STATIC_ASSERT( lhs.m_GradeBits == rhs.m_GradeBits );         // *** ASSERT: lhs & rhs must be of the same grade! ***
    //BIZ_STATIC_ASSERT( T_BitCount< lhs.m_GradeBits >::m_Bits <= 1 ); // *** ASSERT: lhs & rhs must be scalars or single grade vectors! ***
    
    typedef typename T_SP_Eval< t_Metric, t_ElementLHS, t_ElementRHS >::TypeOpList TypeOpList;
    
    #if defined( MATHS_GA_CODEGEN )
    static bool generateCode = true;
    if ( generateCode )
    {
        Util::C_String metricString = GetString< t_Metric >();
        printf( "\n****** GA CODEGEN START ******\n" );
        //printf( "template <>\n" );
        printf( "NOALIAS GA_CODEGEN_INLINE\n" );
        printf( "const Scalar\n" );
        printf( "operator %% ( const T_Vector< (ElementBits)0x%X, %d, %s >& lhs,\n", t_ElementLHS, t_Dimensions, metricString.Chars() );
        printf( "             const T_Vector< (ElementBits)0x%X, %d, %s >& rhs )\n", t_ElementRHS, t_Dimensions, metricString.Chars() );
        printf( "{\n" );
        printf( "    Scalar result;\n" );
        //printf( "    return Scalar( " );
    }
    #else
    const bool generateCode = false;
    #endif
    
    Scalar result;
    T_ProcessElement< BasisBits_s, TypeOpList, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, ElementBits_s >::ProcessElement( lhs, rhs, &result, generateCode );
    
    #if defined( MATHS_GA_CODEGEN )
    if ( generateCode )
    {
        //printf( " );\n" );
        printf( "    return result;\n" );
        printf( "}\n" );
        printf( "****** GA CODEGEN END ******\n\n" );
        generateCode = false;
    }
    #endif
    
    return result;
}








//template <>
NOALIAS GA_CODEGEN_INLINE
const Scalar
operator % ( const T_Vector< (ElementBits)0x10116, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x10116, 5, Conformal3D >& rhs )
{
    Scalar result;
    SetElement< BasisBits_s >( &result, NegMulSub( Element<BasisBits_e4>(lhs), Element<BasisBits_e0>(rhs), MulAdd( Element<BasisBits_e3>(lhs), Element<BasisBits_e3>(rhs), MulAdd( Element<BasisBits_e2>(lhs), Element<BasisBits_e2>(rhs), MulAdd( Element<BasisBits_e1>(lhs), Element<BasisBits_e1>(rhs), NegMulSub( Element<BasisBits_e0>(lhs), Element<BasisBits_e4>(rhs), Const<VF32>::Zero() ) ) ) ) ) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const Scalar
operator % ( const T_Vector< (ElementBits)0x10114, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x10114, 5, Conformal3D >& rhs )
{
    Scalar result;
    SetElement< BasisBits_s >( &result, MulAdd( Element<BasisBits_e3>(lhs), Element<BasisBits_e3>(rhs), MulAdd( Element<BasisBits_e2>(lhs), Element<BasisBits_e2>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e1>(rhs) ) ) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const Scalar
operator % ( const T_Vector< (ElementBits)0x10116, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x10114, 5, Conformal3D >& rhs )
{
    Scalar result;
    SetElement< BasisBits_s >( &result, MulAdd( Element<BasisBits_e3>(lhs), Element<BasisBits_e3>(rhs), MulAdd( Element<BasisBits_e2>(lhs), Element<BasisBits_e2>(rhs), MulAdd( Element<BasisBits_e1>(lhs), Element<BasisBits_e1>(rhs), NegMulSub( Element<BasisBits_e0>(lhs), Element<BasisBits_e4>(rhs), Const<VF32>::Zero() ) ) ) ) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const Scalar
operator % ( const T_Vector< (ElementBits)0x1140001, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1140001, 5, Conformal3D >& rhs )
{
    Scalar result;
    SetElement< BasisBits_s >( &result, Element<BasisBits_s>(lhs) * Element<BasisBits_s>(rhs) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const Scalar
operator % ( const T_Vector< (ElementBits)0x2, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x10000, 5, Conformal3D >& rhs )
{
    Scalar result;
    SetElement< BasisBits_s >( &result, NegMulSub( Element<BasisBits_e0>(lhs), Element<BasisBits_e4>(rhs), Const<VF32>::Zero() ) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const Scalar
operator % ( const T_Vector< (ElementBits)0x10000, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x2, 5, Conformal3D >& rhs )
{
    Scalar result;
    SetElement< BasisBits_s >( &result, NegMulSub( Element<BasisBits_e4>(lhs), Element<BasisBits_e0>(rhs), Const<VF32>::Zero() ) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const Scalar
operator % ( const T_Vector< (ElementBits)0x114, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x114, 5, Conformal3D >& rhs )
{
    Scalar result;
    SetElement< BasisBits_s >( &result, MulAdd( Element<BasisBits_e3>(lhs), Element<BasisBits_e3>(rhs), MulAdd( Element<BasisBits_e2>(lhs), Element<BasisBits_e2>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e1>(rhs) ) ) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const Scalar
operator % ( const T_Vector< (ElementBits)0x1441, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1441, 5, Conformal3D >& rhs )
{
    Scalar result;
    SetElement< BasisBits_s >( &result, NegMulSub( Element<BasisBits_e23>(lhs), Element<BasisBits_e23>(rhs), NegMulSub( Element<BasisBits_e13>(lhs), Element<BasisBits_e13>(rhs), NegMulSub( Element<BasisBits_e12>(lhs), Element<BasisBits_e12>(rhs), Element<BasisBits_s>(lhs) * Element<BasisBits_s>(rhs) ) ) ) );
    return result;
}

NOALIAS GA_CODEGEN_INLINE
const Scalar
operator % ( const T_Vector< (ElementBits)0x80000000, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x80000000, 5, Conformal3D >& rhs )
{
    Scalar result;
    SetElement< BasisBits_s >( &result, NegMulSub( Element<BasisBits_e01234>(lhs), Element<BasisBits_e01234>(rhs), Const<VF32>::Zero() ) );
    return result;
}

NOALIAS GA_CODEGEN_INLINE
const Scalar
operator % ( const T_Vector< (ElementBits)0x4000, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x4000, 5, Conformal3D >& rhs )
{
    Scalar result;
    SetElement< BasisBits_s >( &result, NegMulSub( Element<BasisBits_e123>(lhs), Element<BasisBits_e123>(rhs), Const<VF32>::Zero() ) );
    return result;
}



/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )



