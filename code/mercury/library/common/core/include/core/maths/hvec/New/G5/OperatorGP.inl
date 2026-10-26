#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_GP_INL
#define SOFT_MATHS_HVEC_NEW_GA_GP_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_GP_INL
#define MATHS_HVEC_NEW_GA_GP_INL
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
struct T_GP_Outer
{
    static const BasisBits m_Result  = (BasisBits)( t_LHS ^ t_RHS );
    static const int       m_Scale   = T_CanonicalReorderSign< t_LHS, t_RHS >::m_Scale;
    static const bool      m_Negated = ( m_Scale == -1 );
    static const bool      m_Null    = ( ( t_LHS & t_RHS ) != 0 ); // If LHS & RHS have basis in common, result is null!
};

template < BasisBits t_LHS, BasisBits t_RHS, typename t_Metric >
struct T_GP_Inner
{
    typedef T_MetricOp< t_LHS, t_RHS, t_Metric > TypeMetricOp;
    
    static const BasisBits m_Result  = TypeMetricOp::m_Result;
    static const int       m_Scale   = T_CanonicalReorderSign< t_LHS, t_RHS >::m_Scale * TypeMetricOp::m_Scale;
    static const bool      m_Negated = ( m_Scale == -1 );
    static const bool      m_Null    = ( m_Scale == 0 );
    
    #if 0
    static inline void Display()
    {
        #if 0
        TypeMetricOp::Display();
        printf( "T_GP_Inner: LHS bits = 0x%02X, RHS bits = 0x%02X, RESULT bits = 0x%02X, scale = % 2d, negated = %d, null = %d\n",
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

/// T_Vector GP Evaluator: sub2
template < typename    t_Metric,
           BasisBits   t_LHS,
           BasisBits   t_RHS,
           typename    t_OpStruct0 >
struct T_GP_Eval2
{
    typedef T_GP_Outer< t_LHS, t_RHS > OP;
    typedef T_GP_Inner< t_LHS, t_RHS, t_Metric > IP;
        
    typedef typename T_Add< t_LHS, t_RHS, OP::m_Result, OP::m_Negated, OP::m_Null, t_OpStruct0   >::TypeResult TypeOpStruct1;
    typedef typename T_Add< t_LHS, t_RHS, IP::m_Result, IP::m_Negated, IP::m_Null, TypeOpStruct1 >::TypeResult TypeOpStruct;
    
    #if 0
    static inline void DisplayMetric()
    {
        #if 0
        IP::Display();
        #endif
    }
    #endif
};

/// T_Vector GP Evaluator: sub2: invalid op terminator
template < typename    t_Metric,
           BasisBits   t_LHS,
           typename    t_OpStruct >
struct T_GP_Eval2< t_Metric, t_LHS, BasisBits_Null, t_OpStruct >
{
    typedef t_OpStruct TypeOpStruct;
    
    #if 0
    static inline void DisplayMetric()
    {
    }
    #endif
};

/// T_Vector GP Evaluator: sub1
template < typename    t_Metric,
           BasisBits   t_LHS,
           ElementBits t_ElementRHS,
           typename    t_OpStruct >
struct T_GP_Eval1
{
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  0 >::m_BasisBits, t_OpStruct >::TypeOpStruct TypeEval_0;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  1 >::m_BasisBits, TypeEval_0 >::TypeOpStruct TypeEval_1;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  2 >::m_BasisBits, TypeEval_1 >::TypeOpStruct TypeEval_2;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  3 >::m_BasisBits, TypeEval_2 >::TypeOpStruct TypeEval_3;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  4 >::m_BasisBits, TypeEval_3 >::TypeOpStruct TypeEval_4;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  5 >::m_BasisBits, TypeEval_4 >::TypeOpStruct TypeEval_5;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  6 >::m_BasisBits, TypeEval_5 >::TypeOpStruct TypeEval_6;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  7 >::m_BasisBits, TypeEval_6 >::TypeOpStruct TypeEval_7;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  8 >::m_BasisBits, TypeEval_7 >::TypeOpStruct TypeEval_8;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS,  9 >::m_BasisBits, TypeEval_8 >::TypeOpStruct TypeEval_9;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 10 >::m_BasisBits, TypeEval_9 >::TypeOpStruct TypeEval10;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 11 >::m_BasisBits, TypeEval10 >::TypeOpStruct TypeEval11;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 12 >::m_BasisBits, TypeEval11 >::TypeOpStruct TypeEval12;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 13 >::m_BasisBits, TypeEval12 >::TypeOpStruct TypeEval13;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 14 >::m_BasisBits, TypeEval13 >::TypeOpStruct TypeEval14;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 15 >::m_BasisBits, TypeEval14 >::TypeOpStruct TypeEval15;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 16 >::m_BasisBits, TypeEval15 >::TypeOpStruct TypeEval16;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 17 >::m_BasisBits, TypeEval16 >::TypeOpStruct TypeEval17;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 18 >::m_BasisBits, TypeEval17 >::TypeOpStruct TypeEval18;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 19 >::m_BasisBits, TypeEval18 >::TypeOpStruct TypeEval19;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 20 >::m_BasisBits, TypeEval19 >::TypeOpStruct TypeEval20;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 21 >::m_BasisBits, TypeEval20 >::TypeOpStruct TypeEval21;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 22 >::m_BasisBits, TypeEval21 >::TypeOpStruct TypeEval22;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 23 >::m_BasisBits, TypeEval22 >::TypeOpStruct TypeEval23;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 24 >::m_BasisBits, TypeEval23 >::TypeOpStruct TypeEval24;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 25 >::m_BasisBits, TypeEval24 >::TypeOpStruct TypeEval25;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 26 >::m_BasisBits, TypeEval25 >::TypeOpStruct TypeEval26;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 27 >::m_BasisBits, TypeEval26 >::TypeOpStruct TypeEval27;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 28 >::m_BasisBits, TypeEval27 >::TypeOpStruct TypeEval28;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 29 >::m_BasisBits, TypeEval28 >::TypeOpStruct TypeEval29;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 30 >::m_BasisBits, TypeEval29 >::TypeOpStruct TypeEval30;
    typedef typename T_GP_Eval2< t_Metric, t_LHS, T_GetBasis< t_ElementRHS, 31 >::m_BasisBits, TypeEval30 >::TypeOpStruct TypeOpStruct;
    
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

/// T_Vector GP Evaluator: sub1: invalid op terminator
template < typename    t_Metric,
           ElementBits t_ElementRHS,
           typename    t_OpStruct >
struct T_GP_Eval1< t_Metric, BasisBits_Null, t_ElementRHS, t_OpStruct >
{
    typedef t_OpStruct TypeOpStruct;
    
    #if 0
    static inline void DisplayMetric()
    {
    }
    #endif
};

/// T_Vector GP Evaluator: root
template < typename    t_Metric,
           ElementBits t_ElementLHS,
           ElementBits t_ElementRHS >
struct T_GP_Eval
{
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  0 >::m_BasisBits, t_ElementRHS, VoidOpStruct >::TypeOpStruct TypeEval_0;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  1 >::m_BasisBits, t_ElementRHS, TypeEval_0 >::TypeOpStruct TypeEval_1;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  2 >::m_BasisBits, t_ElementRHS, TypeEval_1 >::TypeOpStruct TypeEval_2;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  3 >::m_BasisBits, t_ElementRHS, TypeEval_2 >::TypeOpStruct TypeEval_3;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  4 >::m_BasisBits, t_ElementRHS, TypeEval_3 >::TypeOpStruct TypeEval_4;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  5 >::m_BasisBits, t_ElementRHS, TypeEval_4 >::TypeOpStruct TypeEval_5;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  6 >::m_BasisBits, t_ElementRHS, TypeEval_5 >::TypeOpStruct TypeEval_6;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  7 >::m_BasisBits, t_ElementRHS, TypeEval_6 >::TypeOpStruct TypeEval_7;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  8 >::m_BasisBits, t_ElementRHS, TypeEval_7 >::TypeOpStruct TypeEval_8;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS,  9 >::m_BasisBits, t_ElementRHS, TypeEval_8 >::TypeOpStruct TypeEval_9;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 10 >::m_BasisBits, t_ElementRHS, TypeEval_9 >::TypeOpStruct TypeEval10;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 11 >::m_BasisBits, t_ElementRHS, TypeEval10 >::TypeOpStruct TypeEval11;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 12 >::m_BasisBits, t_ElementRHS, TypeEval11 >::TypeOpStruct TypeEval12;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 13 >::m_BasisBits, t_ElementRHS, TypeEval12 >::TypeOpStruct TypeEval13;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 14 >::m_BasisBits, t_ElementRHS, TypeEval13 >::TypeOpStruct TypeEval14;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 15 >::m_BasisBits, t_ElementRHS, TypeEval14 >::TypeOpStruct TypeEval15;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 16 >::m_BasisBits, t_ElementRHS, TypeEval15 >::TypeOpStruct TypeEval16;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 17 >::m_BasisBits, t_ElementRHS, TypeEval16 >::TypeOpStruct TypeEval17;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 18 >::m_BasisBits, t_ElementRHS, TypeEval17 >::TypeOpStruct TypeEval18;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 19 >::m_BasisBits, t_ElementRHS, TypeEval18 >::TypeOpStruct TypeEval19;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 20 >::m_BasisBits, t_ElementRHS, TypeEval19 >::TypeOpStruct TypeEval20;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 21 >::m_BasisBits, t_ElementRHS, TypeEval20 >::TypeOpStruct TypeEval21;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 22 >::m_BasisBits, t_ElementRHS, TypeEval21 >::TypeOpStruct TypeEval22;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 23 >::m_BasisBits, t_ElementRHS, TypeEval22 >::TypeOpStruct TypeEval23;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 24 >::m_BasisBits, t_ElementRHS, TypeEval23 >::TypeOpStruct TypeEval24;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 25 >::m_BasisBits, t_ElementRHS, TypeEval24 >::TypeOpStruct TypeEval25;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 26 >::m_BasisBits, t_ElementRHS, TypeEval25 >::TypeOpStruct TypeEval26;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 27 >::m_BasisBits, t_ElementRHS, TypeEval26 >::TypeOpStruct TypeEval27;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 28 >::m_BasisBits, t_ElementRHS, TypeEval27 >::TypeOpStruct TypeEval28;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 29 >::m_BasisBits, t_ElementRHS, TypeEval28 >::TypeOpStruct TypeEval29;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 30 >::m_BasisBits, t_ElementRHS, TypeEval29 >::TypeOpStruct TypeEval30;
    typedef typename T_GP_Eval1< t_Metric, T_GetBasis< t_ElementLHS, 31 >::m_BasisBits, t_ElementRHS, TypeEval30 >::TypeOpStruct TypeOpStruct;
    
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
        TypeOpStruct::Display();
        //printf( "Metric:\n" );
        //DisplayMetric();
        //printf( "\n" );
        #endif
    }
    #endif
};


template < unsigned int t_Dimensions,
           typename     t_Metric,
           ElementBits  t_ElementLHS,
           ElementBits  t_ElementRHS >
struct T_GP_Result
{
    typedef T_GP_Eval< t_Metric, t_ElementLHS, t_ElementRHS > TypeEval;
    typedef T_Vector< TypeEval::TypeOpStruct::m_ElementBits, t_Dimensions, t_Metric > TypeResult;
    
    static const unsigned int m_Dimensions    = t_Dimensions;
    static const ElementBits  m_ElementLHS    = t_ElementLHS;
    static const ElementBits  m_ElementRHS    = t_ElementRHS;
    static const ElementBits  m_ElementResult = TypeEval::TypeOpStruct::m_ElementBits;
};


/// T_Vector Geometric Product
template < unsigned int t_Dimensions,
           typename     t_Metric,
           ElementBits  t_ElementLHS,
           ElementBits  t_ElementRHS >
NOALIAS GA_OPERATOR_INLINE
    const typename T_GP_Result< t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS >::TypeResult
    operator * ( const T_Vector< t_ElementLHS, t_Dimensions, t_Metric >& lhs,
                 const T_Vector< t_ElementRHS, t_Dimensions, t_Metric >& rhs )
{
    typedef T_GP_Result< t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS > GP_Result;
    typedef typename GP_Result::TypeResult  TypeResult;
    typedef typename GP_Result::TypeEval    TypeEval;
    typedef typename TypeEval::TypeOpStruct TypeOpStruct;
    
    #if 0
    
    //GP_Result::TypeEval::DisplayMetric();
    printf( "Elements = %2d, bitfield = 0x%08X\n", TypeResult::m_Elements, TypeResult::m_ElementBits );
    
    if ( TypeResult::m_ElementBits != ElementBits_Null )
    {
        printf( "elements = " );
        if ( TypeResult::m_ElementBits & ElementBits_s ) printf( "s " );
        if ( TypeResult::m_ElementBits & ElementBits_e0 ) printf( "e0 " );
        if ( TypeResult::m_ElementBits & ElementBits_e1 ) printf( "e1 " );
        if ( TypeResult::m_ElementBits & ElementBits_e2 ) printf( "e2 " );
        if ( TypeResult::m_ElementBits & ElementBits_e3 ) printf( "e3 " );
        if ( TypeResult::m_ElementBits & ElementBits_e4 ) printf( "e4 " );
        if ( TypeResult::m_ElementBits & ElementBits_e01 ) printf( "e01 " );
        if ( TypeResult::m_ElementBits & ElementBits_e02 ) printf( "e02 " );
        if ( TypeResult::m_ElementBits & ElementBits_e03 ) printf( "e03 " );
        if ( TypeResult::m_ElementBits & ElementBits_e04 ) printf( "e04 " );
        if ( TypeResult::m_ElementBits & ElementBits_e12 ) printf( "e12 " );
        if ( TypeResult::m_ElementBits & ElementBits_e13 ) printf( "e13 " );
        if ( TypeResult::m_ElementBits & ElementBits_e14 ) printf( "e14 " );
        if ( TypeResult::m_ElementBits & ElementBits_e23 ) printf( "e23 " );
        if ( TypeResult::m_ElementBits & ElementBits_e24 ) printf( "e24 " );
        if ( TypeResult::m_ElementBits & ElementBits_e34 ) printf( "e34 " );
        if ( TypeResult::m_ElementBits & ElementBits_e012 ) printf( "e012 " );
        if ( TypeResult::m_ElementBits & ElementBits_e013 ) printf( "e013 " );
        if ( TypeResult::m_ElementBits & ElementBits_e014 ) printf( "e014 " );
        if ( TypeResult::m_ElementBits & ElementBits_e023 ) printf( "e023 " );
        if ( TypeResult::m_ElementBits & ElementBits_e024 ) printf( "e024 " );
        if ( TypeResult::m_ElementBits & ElementBits_e034 ) printf( "e034 " );
        if ( TypeResult::m_ElementBits & ElementBits_e123 ) printf( "e123 " );
        if ( TypeResult::m_ElementBits & ElementBits_e124 ) printf( "e124 " );
        if ( TypeResult::m_ElementBits & ElementBits_e134 ) printf( "e134 " );
        if ( TypeResult::m_ElementBits & ElementBits_e234 ) printf( "e234 " );
        if ( TypeResult::m_ElementBits & ElementBits_e0123 ) printf( "e0123 " );
        if ( TypeResult::m_ElementBits & ElementBits_e0124 ) printf( "e0124 " );
        if ( TypeResult::m_ElementBits & ElementBits_e0134 ) printf( "e0134 " );
        if ( TypeResult::m_ElementBits & ElementBits_e0234 ) printf( "e0234 " );
        if ( TypeResult::m_ElementBits & ElementBits_e1234 ) printf( "e1234 " );
        if ( TypeResult::m_ElementBits & ElementBits_e01234 ) printf( "e01234 " );
        printf( "\n" );
    }
    else
    {
        printf( "elements = Null\n" );
    }
    
    TypeOpStruct::TypeOpList_s::Display();
    TypeOpStruct::TypeOpList_e0::Display();
    TypeOpStruct::TypeOpList_e1::Display();
    TypeOpStruct::TypeOpList_e2::Display();
    TypeOpStruct::TypeOpList_e3::Display();
    TypeOpStruct::TypeOpList_e4::Display();
    TypeOpStruct::TypeOpList_e01::Display();
    TypeOpStruct::TypeOpList_e02::Display();
    TypeOpStruct::TypeOpList_e03::Display();
    TypeOpStruct::TypeOpList_e04::Display();
    TypeOpStruct::TypeOpList_e12::Display();
    TypeOpStruct::TypeOpList_e13::Display();
    TypeOpStruct::TypeOpList_e14::Display();
    TypeOpStruct::TypeOpList_e23::Display();
    TypeOpStruct::TypeOpList_e24::Display();
    TypeOpStruct::TypeOpList_e34::Display();
    TypeOpStruct::TypeOpList_e012::Display();
    TypeOpStruct::TypeOpList_e013::Display();
    TypeOpStruct::TypeOpList_e014::Display();
    TypeOpStruct::TypeOpList_e023::Display();
    TypeOpStruct::TypeOpList_e024::Display();
    TypeOpStruct::TypeOpList_e034::Display();
    TypeOpStruct::TypeOpList_e123::Display();
    TypeOpStruct::TypeOpList_e124::Display();
    TypeOpStruct::TypeOpList_e134::Display();
    TypeOpStruct::TypeOpList_e234::Display();
    TypeOpStruct::TypeOpList_e0123::Display();
    TypeOpStruct::TypeOpList_e0124::Display();
    TypeOpStruct::TypeOpList_e0134::Display();
    TypeOpStruct::TypeOpList_e0234::Display();
    TypeOpStruct::TypeOpList_e1234::Display();
    TypeOpStruct::TypeOpList_e01234::Display();
    
    #endif
    
    #if defined( MATHS_GA_CODEGEN )
    static bool generateCode = true;
    if ( generateCode )
    {
        Util::C_String metricString = GetString< t_Metric >();
        printf( "\n****** GA CODEGEN START ******\n" );
        //printf( "template <>\n" );
        printf( "NOALIAS GA_CODEGEN_INLINE\n" );
        printf( "const T_Vector< (ElementBits)0x%X, %d, %s >\n", TypeResult::m_ElementBits, t_Dimensions, metricString.Chars() );
        printf( "operator * ( const T_Vector< (ElementBits)0x%X, %d, %s >& lhs,\n", t_ElementLHS, t_Dimensions, metricString.Chars() );
        printf( "             const T_Vector< (ElementBits)0x%X, %d, %s >& rhs )\n", t_ElementRHS, t_Dimensions, metricString.Chars() );
        printf( "{\n" );
        printf( "    T_Vector< (ElementBits)0x%X, %d, %s > result;\n", TypeResult::m_ElementBits, t_Dimensions, metricString.Chars() );
    }
    #else
    const bool generateCode = false;
    #endif
    
    TypeResult result;
    
    T_ProcessElement< BasisBits_s, typename TypeOpStruct::TypeOpList_s, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    
    T_ProcessElement< BasisBits_e0, typename TypeOpStruct::TypeOpList_e0, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e1, typename TypeOpStruct::TypeOpList_e1, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e2, typename TypeOpStruct::TypeOpList_e2, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e3, typename TypeOpStruct::TypeOpList_e3, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e4, typename TypeOpStruct::TypeOpList_e4, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    
    T_ProcessElement< BasisBits_e01, typename TypeOpStruct::TypeOpList_e01, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e02, typename TypeOpStruct::TypeOpList_e02, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e03, typename TypeOpStruct::TypeOpList_e03, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e04, typename TypeOpStruct::TypeOpList_e04, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e12, typename TypeOpStruct::TypeOpList_e12, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e13, typename TypeOpStruct::TypeOpList_e13, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e14, typename TypeOpStruct::TypeOpList_e14, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e23, typename TypeOpStruct::TypeOpList_e23, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e24, typename TypeOpStruct::TypeOpList_e24, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e34, typename TypeOpStruct::TypeOpList_e34, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    
    T_ProcessElement< BasisBits_e012, typename TypeOpStruct::TypeOpList_e012, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e013, typename TypeOpStruct::TypeOpList_e013, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e014, typename TypeOpStruct::TypeOpList_e014, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e023, typename TypeOpStruct::TypeOpList_e023, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e024, typename TypeOpStruct::TypeOpList_e024, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e034, typename TypeOpStruct::TypeOpList_e034, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e123, typename TypeOpStruct::TypeOpList_e123, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e124, typename TypeOpStruct::TypeOpList_e124, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e134, typename TypeOpStruct::TypeOpList_e134, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e234, typename TypeOpStruct::TypeOpList_e234, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    
    T_ProcessElement< BasisBits_e0123, typename TypeOpStruct::TypeOpList_e0123, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e0124, typename TypeOpStruct::TypeOpList_e0124, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e0134, typename TypeOpStruct::TypeOpList_e0134, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e0234, typename TypeOpStruct::TypeOpList_e0234, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    T_ProcessElement< BasisBits_e1234, typename TypeOpStruct::TypeOpList_e1234, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    
    T_ProcessElement< BasisBits_e01234, typename TypeOpStruct::TypeOpList_e01234, t_Dimensions, t_Metric, t_ElementLHS, t_ElementRHS, TypeResult::m_ElementBits >::ProcessElement( lhs, rhs, &result, generateCode );
    
    #if defined( MATHS_GA_CODEGEN )
    if ( generateCode )
    {
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
const T_Vector< (ElementBits)0x1161669, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x10116, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x10116, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x1161669, 5, Conformal3D > result;
    SetElement< BasisBits_s >( &result, NegMulSub( Element<BasisBits_e4>(lhs), Element<BasisBits_e0>(rhs), MulAdd( Element<BasisBits_e3>(lhs), Element<BasisBits_e3>(rhs), MulAdd( Element<BasisBits_e2>(lhs), Element<BasisBits_e2>(rhs), MulAdd( Element<BasisBits_e1>(lhs), Element<BasisBits_e1>(rhs), NegMulSub( Element<BasisBits_e0>(lhs), Element<BasisBits_e4>(rhs), Const<VF32>::Zero() ) ) ) ) ) );
    SetElement< BasisBits_e01 >( &result, NegMulSub( Element<BasisBits_e1>(lhs), Element<BasisBits_e0>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e1>(rhs) ) );
    SetElement< BasisBits_e02 >( &result, NegMulSub( Element<BasisBits_e2>(lhs), Element<BasisBits_e0>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e2>(rhs) ) );
    SetElement< BasisBits_e03 >( &result, NegMulSub( Element<BasisBits_e3>(lhs), Element<BasisBits_e0>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e3>(rhs) ) );
    SetElement< BasisBits_e04 >( &result, NegMulSub( Element<BasisBits_e4>(lhs), Element<BasisBits_e0>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e4>(rhs) ) );
    SetElement< BasisBits_e12 >( &result, NegMulSub( Element<BasisBits_e2>(lhs), Element<BasisBits_e1>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e2>(rhs) ) );
    SetElement< BasisBits_e13 >( &result, NegMulSub( Element<BasisBits_e3>(lhs), Element<BasisBits_e1>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e3>(rhs) ) );
    SetElement< BasisBits_e14 >( &result, NegMulSub( Element<BasisBits_e4>(lhs), Element<BasisBits_e1>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e4>(rhs) ) );
    SetElement< BasisBits_e23 >( &result, NegMulSub( Element<BasisBits_e3>(lhs), Element<BasisBits_e2>(rhs), Element<BasisBits_e2>(lhs) * Element<BasisBits_e3>(rhs) ) );
    SetElement< BasisBits_e24 >( &result, NegMulSub( Element<BasisBits_e4>(lhs), Element<BasisBits_e2>(rhs), Element<BasisBits_e2>(lhs) * Element<BasisBits_e4>(rhs) ) );
    SetElement< BasisBits_e34 >( &result, NegMulSub( Element<BasisBits_e4>(lhs), Element<BasisBits_e3>(rhs), Element<BasisBits_e3>(lhs) * Element<BasisBits_e4>(rhs) ) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x1, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x1, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x1, 5, Conformal3D > result;
    SetElement< BasisBits_s >( &result, Element<BasisBits_s>(lhs) * Element<BasisBits_s>(rhs) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x10116, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x10116, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x10116, 5, Conformal3D > result;
    SetElement< BasisBits_e0 >( &result, Element<BasisBits_e0>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e1 >( &result, Element<BasisBits_e1>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e2 >( &result, Element<BasisBits_e2>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e3 >( &result, Element<BasisBits_e3>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e4 >( &result, Element<BasisBits_e4>(lhs) * Element<BasisBits_s>(rhs) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x10114, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x10114, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x10114, 5, Conformal3D > result;
    SetElement< BasisBits_e1 >( &result, Element<BasisBits_e1>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e2 >( &result, Element<BasisBits_e2>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e3 >( &result, Element<BasisBits_e3>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e4 >( &result, Element<BasisBits_e4>(lhs) * Element<BasisBits_s>(rhs) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x1140000, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x114, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x10000, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x1140000, 5, Conformal3D > result;
    SetElement< BasisBits_e14 >( &result, Element<BasisBits_e1>(lhs) * Element<BasisBits_e4>(rhs) );
    SetElement< BasisBits_e24 >( &result, Element<BasisBits_e2>(lhs) * Element<BasisBits_e4>(rhs) );
    SetElement< BasisBits_e34 >( &result, Element<BasisBits_e3>(lhs) * Element<BasisBits_e4>(rhs) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x1140001, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x1140001, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x1140001, 5, Conformal3D > result;
    SetElement< BasisBits_s >( &result, Element<BasisBits_s>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e14 >( &result, Element<BasisBits_e14>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e24 >( &result, Element<BasisBits_e24>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e34 >( &result, Element<BasisBits_e34>(lhs) * Element<BasisBits_s>(rhs) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x20001, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x2, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x10000, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x20001, 5, Conformal3D > result;
    SetElement< BasisBits_s >( &result, NegMulSub( Element<BasisBits_e0>(lhs), Element<BasisBits_e4>(rhs), Const<VF32>::Zero() ) );
    SetElement< BasisBits_e04 >( &result, Element<BasisBits_e0>(lhs) * Element<BasisBits_e4>(rhs) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x20001, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x10000, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x2, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x20001, 5, Conformal3D > result;
    SetElement< BasisBits_s >( &result, NegMulSub( Element<BasisBits_e4>(lhs), Element<BasisBits_e0>(rhs), Const<VF32>::Zero() ) );
    SetElement< BasisBits_e04 >( &result, NegMulSub( Element<BasisBits_e4>(lhs), Element<BasisBits_e0>(rhs), Const<VF32>::Zero() ) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x16690116, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x1140001, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x10116, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x16690116, 5, Conformal3D > result;
    SetElement< BasisBits_e0 >( &result, Element<BasisBits_s>(lhs) * Element<BasisBits_e0>(rhs) );
    SetElement< BasisBits_e1 >( &result, NegMulSub( Element<BasisBits_e14>(lhs), Element<BasisBits_e0>(rhs), Element<BasisBits_s>(lhs) * Element<BasisBits_e1>(rhs) ) );
    SetElement< BasisBits_e2 >( &result, NegMulSub( Element<BasisBits_e24>(lhs), Element<BasisBits_e0>(rhs), Element<BasisBits_s>(lhs) * Element<BasisBits_e2>(rhs) ) );
    SetElement< BasisBits_e3 >( &result, NegMulSub( Element<BasisBits_e34>(lhs), Element<BasisBits_e0>(rhs), Element<BasisBits_s>(lhs) * Element<BasisBits_e3>(rhs) ) );
    SetElement< BasisBits_e4 >( &result, NegMulSub( Element<BasisBits_e34>(lhs), Element<BasisBits_e3>(rhs), NegMulSub( Element<BasisBits_e24>(lhs), Element<BasisBits_e2>(rhs), NegMulSub( Element<BasisBits_e14>(lhs), Element<BasisBits_e1>(rhs), Element<BasisBits_s>(lhs) * Element<BasisBits_e4>(rhs) ) ) ) );
    SetElement< BasisBits_e014 >( &result, Element<BasisBits_e14>(lhs) * Element<BasisBits_e0>(rhs) );
    SetElement< BasisBits_e024 >( &result, Element<BasisBits_e24>(lhs) * Element<BasisBits_e0>(rhs) );
    SetElement< BasisBits_e034 >( &result, Element<BasisBits_e34>(lhs) * Element<BasisBits_e0>(rhs) );
    SetElement< BasisBits_e124 >( &result, MulAdd( Element<BasisBits_e24>(lhs), Element<BasisBits_e1>(rhs), NegMulSub( Element<BasisBits_e14>(lhs), Element<BasisBits_e2>(rhs), Const<VF32>::Zero() ) ) );
    SetElement< BasisBits_e134 >( &result, MulAdd( Element<BasisBits_e34>(lhs), Element<BasisBits_e1>(rhs), NegMulSub( Element<BasisBits_e14>(lhs), Element<BasisBits_e3>(rhs), Const<VF32>::Zero() ) ) );
    SetElement< BasisBits_e234 >( &result, MulAdd( Element<BasisBits_e34>(lhs), Element<BasisBits_e2>(rhs), NegMulSub( Element<BasisBits_e24>(lhs), Element<BasisBits_e3>(rhs), Const<VF32>::Zero() ) ) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x2280116, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x1140001, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x2, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x2280116, 5, Conformal3D > result;
    SetElement< BasisBits_e0 >( &result, Element<BasisBits_s>(lhs) * Element<BasisBits_e0>(rhs) );
    SetElement< BasisBits_e1 >( &result, NegMulSub( Element<BasisBits_e14>(lhs), Element<BasisBits_e0>(rhs), Const<VF32>::Zero() ) );
    SetElement< BasisBits_e2 >( &result, NegMulSub( Element<BasisBits_e24>(lhs), Element<BasisBits_e0>(rhs), Const<VF32>::Zero() ) );
    SetElement< BasisBits_e3 >( &result, NegMulSub( Element<BasisBits_e34>(lhs), Element<BasisBits_e0>(rhs), Const<VF32>::Zero() ) );
    SetElement< BasisBits_e014 >( &result, Element<BasisBits_e14>(lhs) * Element<BasisBits_e0>(rhs) );
    SetElement< BasisBits_e024 >( &result, Element<BasisBits_e24>(lhs) * Element<BasisBits_e0>(rhs) );
    SetElement< BasisBits_e034 >( &result, Element<BasisBits_e34>(lhs) * Element<BasisBits_e0>(rhs) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x16690116, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x2280116, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1140001, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x16690116, 5, Conformal3D > result;
    SetElement< BasisBits_e0 >( &result, Element<BasisBits_e0>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e1 >( &result, MulAdd( Element<BasisBits_e1>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e14>(rhs) ) );
    SetElement< BasisBits_e2 >( &result, MulAdd( Element<BasisBits_e2>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e24>(rhs) ) );
    SetElement< BasisBits_e3 >( &result, MulAdd( Element<BasisBits_e3>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e34>(rhs) ) );
    SetElement< BasisBits_e4 >( &result, NegMulSub( Element<BasisBits_e034>(lhs), Element<BasisBits_e34>(rhs), NegMulSub( Element<BasisBits_e024>(lhs), Element<BasisBits_e24>(rhs), NegMulSub( Element<BasisBits_e014>(lhs), Element<BasisBits_e14>(rhs), MulAdd( Element<BasisBits_e3>(lhs), Element<BasisBits_e34>(rhs), MulAdd( Element<BasisBits_e2>(lhs), Element<BasisBits_e24>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e14>(rhs) ) ) ) ) ) );
    SetElement< BasisBits_e014 >( &result, MulAdd( Element<BasisBits_e014>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e14>(rhs) ) );
    SetElement< BasisBits_e024 >( &result, MulAdd( Element<BasisBits_e024>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e24>(rhs) ) );
    SetElement< BasisBits_e034 >( &result, MulAdd( Element<BasisBits_e034>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e34>(rhs) ) );
    SetElement< BasisBits_e124 >( &result, MulAdd( Element<BasisBits_e024>(lhs), Element<BasisBits_e14>(rhs), NegMulSub( Element<BasisBits_e014>(lhs), Element<BasisBits_e24>(rhs), NegMulSub( Element<BasisBits_e2>(lhs), Element<BasisBits_e14>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e24>(rhs) ) ) ) );
    SetElement< BasisBits_e134 >( &result, MulAdd( Element<BasisBits_e034>(lhs), Element<BasisBits_e14>(rhs), NegMulSub( Element<BasisBits_e014>(lhs), Element<BasisBits_e34>(rhs), NegMulSub( Element<BasisBits_e3>(lhs), Element<BasisBits_e14>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e34>(rhs) ) ) ) );
    SetElement< BasisBits_e234 >( &result, MulAdd( Element<BasisBits_e034>(lhs), Element<BasisBits_e24>(rhs), NegMulSub( Element<BasisBits_e024>(lhs), Element<BasisBits_e34>(rhs), NegMulSub( Element<BasisBits_e3>(lhs), Element<BasisBits_e24>(rhs), Element<BasisBits_e2>(lhs) * Element<BasisBits_e34>(rhs) ) ) ) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x16690116, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x16690116, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1140001, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x16690116, 5, Conformal3D > result;
    SetElement< BasisBits_e0 >( &result, Element<BasisBits_e0>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e1 >( &result, MulAdd( Element<BasisBits_e1>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e14>(rhs) ) );
    SetElement< BasisBits_e2 >( &result, MulAdd( Element<BasisBits_e2>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e24>(rhs) ) );
    SetElement< BasisBits_e3 >( &result, MulAdd( Element<BasisBits_e3>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e34>(rhs) ) );
    SetElement< BasisBits_e4 >( &result, NegMulSub( Element<BasisBits_e034>(lhs), Element<BasisBits_e34>(rhs), NegMulSub( Element<BasisBits_e024>(lhs), Element<BasisBits_e24>(rhs), NegMulSub( Element<BasisBits_e014>(lhs), Element<BasisBits_e14>(rhs), MulAdd( Element<BasisBits_e4>(lhs), Element<BasisBits_s>(rhs), MulAdd( Element<BasisBits_e3>(lhs), Element<BasisBits_e34>(rhs), MulAdd( Element<BasisBits_e2>(lhs), Element<BasisBits_e24>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e14>(rhs) ) ) ) ) ) ) );
    SetElement< BasisBits_e014 >( &result, MulAdd( Element<BasisBits_e014>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e14>(rhs) ) );
    SetElement< BasisBits_e024 >( &result, MulAdd( Element<BasisBits_e024>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e24>(rhs) ) );
    SetElement< BasisBits_e034 >( &result, MulAdd( Element<BasisBits_e034>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e34>(rhs) ) );
    SetElement< BasisBits_e124 >( &result, MulAdd( Element<BasisBits_e124>(lhs), Element<BasisBits_s>(rhs), MulAdd( Element<BasisBits_e024>(lhs), Element<BasisBits_e14>(rhs), NegMulSub( Element<BasisBits_e014>(lhs), Element<BasisBits_e24>(rhs), NegMulSub( Element<BasisBits_e2>(lhs), Element<BasisBits_e14>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e24>(rhs) ) ) ) ) );
    SetElement< BasisBits_e134 >( &result, MulAdd( Element<BasisBits_e134>(lhs), Element<BasisBits_s>(rhs), MulAdd( Element<BasisBits_e034>(lhs), Element<BasisBits_e14>(rhs), NegMulSub( Element<BasisBits_e014>(lhs), Element<BasisBits_e34>(rhs), NegMulSub( Element<BasisBits_e3>(lhs), Element<BasisBits_e14>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e34>(rhs) ) ) ) ) );
    SetElement< BasisBits_e234 >( &result, MulAdd( Element<BasisBits_e234>(lhs), Element<BasisBits_s>(rhs), MulAdd( Element<BasisBits_e034>(lhs), Element<BasisBits_e24>(rhs), NegMulSub( Element<BasisBits_e024>(lhs), Element<BasisBits_e34>(rhs), NegMulSub( Element<BasisBits_e3>(lhs), Element<BasisBits_e24>(rhs), Element<BasisBits_e2>(lhs) * Element<BasisBits_e34>(rhs) ) ) ) ) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x114, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x114, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x114, 5, Conformal3D > result;
    SetElement< BasisBits_e1 >( &result, Element<BasisBits_e1>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e2 >( &result, Element<BasisBits_e2>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e3 >( &result, Element<BasisBits_e3>(lhs) * Element<BasisBits_s>(rhs) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x1441, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x114, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x114, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x1441, 5, Conformal3D > result;
    SetElement< BasisBits_s >( &result, MulAdd( Element<BasisBits_e3>(lhs), Element<BasisBits_e3>(rhs), MulAdd( Element<BasisBits_e2>(lhs), Element<BasisBits_e2>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e1>(rhs) ) ) );
    SetElement< BasisBits_e12 >( &result, NegMulSub( Element<BasisBits_e2>(lhs), Element<BasisBits_e1>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e2>(rhs) ) );
    SetElement< BasisBits_e13 >( &result, NegMulSub( Element<BasisBits_e3>(lhs), Element<BasisBits_e1>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e3>(rhs) ) );
    SetElement< BasisBits_e23 >( &result, NegMulSub( Element<BasisBits_e3>(lhs), Element<BasisBits_e2>(rhs), Element<BasisBits_e2>(lhs) * Element<BasisBits_e3>(rhs) ) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x1441, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x1441, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x1441, 5, Conformal3D > result;
    SetElement< BasisBits_s >( &result, Element<BasisBits_s>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e12 >( &result, Element<BasisBits_e12>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e13 >( &result, Element<BasisBits_e13>(lhs) * Element<BasisBits_s>(rhs) );
    SetElement< BasisBits_e23 >( &result, Element<BasisBits_e23>(lhs) * Element<BasisBits_s>(rhs) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x14416996, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x1441, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x10116, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x14416996, 5, Conformal3D > result;
    SetElement< BasisBits_e0 >( &result, Element<BasisBits_s>(lhs) * Element<BasisBits_e0>(rhs) );
    SetElement< BasisBits_e1 >( &result, MulAdd( Element<BasisBits_e13>(lhs), Element<BasisBits_e3>(rhs), MulAdd( Element<BasisBits_e12>(lhs), Element<BasisBits_e2>(rhs), Element<BasisBits_s>(lhs) * Element<BasisBits_e1>(rhs) ) ) );
    SetElement< BasisBits_e2 >( &result, MulAdd( Element<BasisBits_e23>(lhs), Element<BasisBits_e3>(rhs), NegMulSub( Element<BasisBits_e12>(lhs), Element<BasisBits_e1>(rhs), Element<BasisBits_s>(lhs) * Element<BasisBits_e2>(rhs) ) ) );
    SetElement< BasisBits_e3 >( &result, NegMulSub( Element<BasisBits_e23>(lhs), Element<BasisBits_e2>(rhs), NegMulSub( Element<BasisBits_e13>(lhs), Element<BasisBits_e1>(rhs), Element<BasisBits_s>(lhs) * Element<BasisBits_e3>(rhs) ) ) );
    SetElement< BasisBits_e4 >( &result, Element<BasisBits_s>(lhs) * Element<BasisBits_e4>(rhs) );
    SetElement< BasisBits_e012 >( &result, Element<BasisBits_e12>(lhs) * Element<BasisBits_e0>(rhs) );
    SetElement< BasisBits_e013 >( &result, Element<BasisBits_e13>(lhs) * Element<BasisBits_e0>(rhs) );
    SetElement< BasisBits_e023 >( &result, Element<BasisBits_e23>(lhs) * Element<BasisBits_e0>(rhs) );
    SetElement< BasisBits_e123 >( &result, MulAdd( Element<BasisBits_e23>(lhs), Element<BasisBits_e1>(rhs), NegMulSub( Element<BasisBits_e13>(lhs), Element<BasisBits_e2>(rhs), Element<BasisBits_e12>(lhs) * Element<BasisBits_e3>(rhs) ) ) );
    SetElement< BasisBits_e124 >( &result, Element<BasisBits_e12>(lhs) * Element<BasisBits_e4>(rhs) );
    SetElement< BasisBits_e134 >( &result, Element<BasisBits_e13>(lhs) * Element<BasisBits_e4>(rhs) );
    SetElement< BasisBits_e234 >( &result, Element<BasisBits_e23>(lhs) * Element<BasisBits_e4>(rhs) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x2882, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x1441, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x2, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x2882, 5, Conformal3D > result;
    SetElement< BasisBits_e0 >( &result, Element<BasisBits_s>(lhs) * Element<BasisBits_e0>(rhs) );
    SetElement< BasisBits_e012 >( &result, Element<BasisBits_e12>(lhs) * Element<BasisBits_e0>(rhs) );
    SetElement< BasisBits_e013 >( &result, Element<BasisBits_e13>(lhs) * Element<BasisBits_e0>(rhs) );
    SetElement< BasisBits_e023 >( &result, Element<BasisBits_e23>(lhs) * Element<BasisBits_e0>(rhs) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x2882, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x2882, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1441, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x2882, 5, Conformal3D > result;
    SetElement< BasisBits_e0 >( &result, NegMulSub( Element<BasisBits_e023>(lhs), Element<BasisBits_e23>(rhs), NegMulSub( Element<BasisBits_e013>(lhs), Element<BasisBits_e13>(rhs), NegMulSub( Element<BasisBits_e012>(lhs), Element<BasisBits_e12>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_s>(rhs) ) ) ) );
    SetElement< BasisBits_e012 >( &result, MulAdd( Element<BasisBits_e023>(lhs), Element<BasisBits_e13>(rhs), NegMulSub( Element<BasisBits_e013>(lhs), Element<BasisBits_e23>(rhs), MulAdd( Element<BasisBits_e012>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e12>(rhs) ) ) ) );
    SetElement< BasisBits_e013 >( &result, NegMulSub( Element<BasisBits_e023>(lhs), Element<BasisBits_e12>(rhs), MulAdd( Element<BasisBits_e013>(lhs), Element<BasisBits_s>(rhs), MulAdd( Element<BasisBits_e012>(lhs), Element<BasisBits_e23>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e13>(rhs) ) ) ) );
    SetElement< BasisBits_e023 >( &result, MulAdd( Element<BasisBits_e023>(lhs), Element<BasisBits_s>(rhs), MulAdd( Element<BasisBits_e013>(lhs), Element<BasisBits_e12>(rhs), NegMulSub( Element<BasisBits_e012>(lhs), Element<BasisBits_e13>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e23>(rhs) ) ) ) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x14416996, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x14416996, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1441, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x14416996, 5, Conformal3D > result;
    SetElement< BasisBits_e0 >( &result, NegMulSub( Element<BasisBits_e023>(lhs), Element<BasisBits_e23>(rhs), NegMulSub( Element<BasisBits_e013>(lhs), Element<BasisBits_e13>(rhs), NegMulSub( Element<BasisBits_e012>(lhs), Element<BasisBits_e12>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_s>(rhs) ) ) ) );
    SetElement< BasisBits_e1 >( &result, NegMulSub( Element<BasisBits_e123>(lhs), Element<BasisBits_e23>(rhs), NegMulSub( Element<BasisBits_e3>(lhs), Element<BasisBits_e13>(rhs), NegMulSub( Element<BasisBits_e2>(lhs), Element<BasisBits_e12>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_s>(rhs) ) ) ) );
    SetElement< BasisBits_e2 >( &result, MulAdd( Element<BasisBits_e123>(lhs), Element<BasisBits_e13>(rhs), NegMulSub( Element<BasisBits_e3>(lhs), Element<BasisBits_e23>(rhs), MulAdd( Element<BasisBits_e2>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e12>(rhs) ) ) ) );
    SetElement< BasisBits_e3 >( &result, NegMulSub( Element<BasisBits_e123>(lhs), Element<BasisBits_e12>(rhs), MulAdd( Element<BasisBits_e3>(lhs), Element<BasisBits_s>(rhs), MulAdd( Element<BasisBits_e2>(lhs), Element<BasisBits_e23>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e13>(rhs) ) ) ) );
    SetElement< BasisBits_e4 >( &result, NegMulSub( Element<BasisBits_e234>(lhs), Element<BasisBits_e23>(rhs), NegMulSub( Element<BasisBits_e134>(lhs), Element<BasisBits_e13>(rhs), NegMulSub( Element<BasisBits_e124>(lhs), Element<BasisBits_e12>(rhs), Element<BasisBits_e4>(lhs) * Element<BasisBits_s>(rhs) ) ) ) );
    SetElement< BasisBits_e012 >( &result, MulAdd( Element<BasisBits_e023>(lhs), Element<BasisBits_e13>(rhs), NegMulSub( Element<BasisBits_e013>(lhs), Element<BasisBits_e23>(rhs), MulAdd( Element<BasisBits_e012>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e12>(rhs) ) ) ) );
    SetElement< BasisBits_e013 >( &result, NegMulSub( Element<BasisBits_e023>(lhs), Element<BasisBits_e12>(rhs), MulAdd( Element<BasisBits_e013>(lhs), Element<BasisBits_s>(rhs), MulAdd( Element<BasisBits_e012>(lhs), Element<BasisBits_e23>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e13>(rhs) ) ) ) );
    SetElement< BasisBits_e023 >( &result, MulAdd( Element<BasisBits_e023>(lhs), Element<BasisBits_s>(rhs), MulAdd( Element<BasisBits_e013>(lhs), Element<BasisBits_e12>(rhs), NegMulSub( Element<BasisBits_e012>(lhs), Element<BasisBits_e13>(rhs), Element<BasisBits_e0>(lhs) * Element<BasisBits_e23>(rhs) ) ) ) );
    SetElement< BasisBits_e123 >( &result, MulAdd( Element<BasisBits_e123>(lhs), Element<BasisBits_s>(rhs), MulAdd( Element<BasisBits_e3>(lhs), Element<BasisBits_e12>(rhs), NegMulSub( Element<BasisBits_e2>(lhs), Element<BasisBits_e13>(rhs), Element<BasisBits_e1>(lhs) * Element<BasisBits_e23>(rhs) ) ) ) );
    SetElement< BasisBits_e124 >( &result, MulAdd( Element<BasisBits_e234>(lhs), Element<BasisBits_e13>(rhs), NegMulSub( Element<BasisBits_e134>(lhs), Element<BasisBits_e23>(rhs), MulAdd( Element<BasisBits_e124>(lhs), Element<BasisBits_s>(rhs), Element<BasisBits_e4>(lhs) * Element<BasisBits_e12>(rhs) ) ) ) );
    SetElement< BasisBits_e134 >( &result, NegMulSub( Element<BasisBits_e234>(lhs), Element<BasisBits_e12>(rhs), MulAdd( Element<BasisBits_e134>(lhs), Element<BasisBits_s>(rhs), MulAdd( Element<BasisBits_e124>(lhs), Element<BasisBits_e23>(rhs), Element<BasisBits_e4>(lhs) * Element<BasisBits_e13>(rhs) ) ) ) );
    SetElement< BasisBits_e234 >( &result, MulAdd( Element<BasisBits_e234>(lhs), Element<BasisBits_s>(rhs), MulAdd( Element<BasisBits_e134>(lhs), Element<BasisBits_e12>(rhs), NegMulSub( Element<BasisBits_e124>(lhs), Element<BasisBits_e13>(rhs), Element<BasisBits_e4>(lhs) * Element<BasisBits_e23>(rhs) ) ) ) );
    return result;
}

//template <>
NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x1, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x20000, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x20000, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x1, 5, Conformal3D > result;
    SetElement< BasisBits_s >( &result, Element<BasisBits_e04>(lhs) * Element<BasisBits_e04>(rhs) );
    return result;
}

NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x80000000, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x80000000, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x80000000, 5, Conformal3D > result;
    SetElement< BasisBits_e01234 >( &result, Element<BasisBits_e01234>(lhs) * Element<BasisBits_s>(rhs) );
    return result;
}

NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x4000, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x4000, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x1, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x4000, 5, Conformal3D > result;
    SetElement< BasisBits_e123 >( &result, Element<BasisBits_e123>(lhs) * Element<BasisBits_s>(rhs) );
    return result;
}

NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x2, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x20000, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x2, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x2, 5, Conformal3D > result;
    SetElement< BasisBits_e0 >( &result, NegMulSub( Element<BasisBits_e04>(lhs), Element<BasisBits_e0>(rhs), Const<VF32>::Zero() ) );
    return result;
}

NOALIAS GA_CODEGEN_INLINE
const T_Vector< (ElementBits)0x10000, 5, Conformal3D >
operator * ( const T_Vector< (ElementBits)0x20000, 5, Conformal3D >& lhs,
             const T_Vector< (ElementBits)0x10000, 5, Conformal3D >& rhs )
{
    T_Vector< (ElementBits)0x10000, 5, Conformal3D > result;
    SetElement< BasisBits_e4 >( &result, Element<BasisBits_e04>(lhs) * Element<BasisBits_e4>(rhs) );
    return result;
}




/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )



