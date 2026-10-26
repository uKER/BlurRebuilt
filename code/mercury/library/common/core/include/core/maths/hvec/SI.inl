#ifndef SI_INL
#define SI_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

//
// *****************************************************
// *** Implementation of C_SiScalar Member Functions ***
// *****************************************************
//

/// Default Constructor
template < typename LHS >
NOALIAS __forceinline C_SiScalar<LHS>::C_SiScalar()
{
}

/// Undefined Constructor
template < typename LHS >
NOALIAS __forceinline C_SiScalar<LHS>::C_SiScalar( const _UNDEFINED undefined ) :
    m_Scalar( UNDEFINED )
{
}

/// Copy Constructor
template < typename LHS >
NOALIAS __forceinline C_SiScalar<LHS>::C_SiScalar( const C_SiScalar<LHS>& copy ) :
    m_Scalar( copy.m_Scalar )
{
}

/// Copy Assignment
template < typename LHS >
NOALIAS __forceinline void C_SiScalar<LHS>::operator = ( const C_SiScalar<LHS>& copy )
{
    m_Scalar = copy.m_Scalar;
}

/// F32 Constructor
template < typename LHS >
NOALIAS __forceinline C_SiScalar<LHS>::C_SiScalar( const F32& f ) :
    m_Scalar( f )
{
}

/// F32 Assignment
template < typename LHS >
NOALIAS __forceinline void C_SiScalar<LHS>::operator = ( const F32& f ) __restrict
{
    m_Scalar = f;
}

/// HF32 Constructor
template < typename LHS >
NOALIAS __forceinline C_SiScalar<LHS>::C_SiScalar( const HF32 PCREF f ) :
    m_Scalar( f )
{
}

/// HF32 Assignment
template < typename LHS >
NOALIAS __forceinline void C_SiScalar<LHS>::operator = ( const HF32 PCREF f ) __restrict
{
    m_Scalar = f;
}

/// Get HF32
template < typename LHS >
NOALIAS __forceinline const HF32 C_SiScalar<LHS>::Get() const __restrict
{
    return m_Scalar;
}

/// Abs
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiScalar<LHS>::Abs() const __restrict
{
    return C_SiScalar<LHS>( HF32( m_Scalar ).Abs() );
}

/// Min
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiScalar<LHS>::Min( const C_SiScalar<LHS> PCREF src ) const __restrict
{
    return m_Scalar.Min( src.m_Scalar );
}

/// Max
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiScalar<LHS>::Max( const C_SiScalar<LHS> PCREF src ) const __restrict
{
    return m_Scalar.Max( src.m_Scalar );
}

/// Unary Minus
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiScalar<LHS>::operator - () const __restrict
{
    return C_SiScalar<LHS>( -m_Scalar );
}

/// Binary Addition Operator
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiScalar<LHS>::operator + ( const C_SiScalar<LHS> PCREF rhs ) const __restrict
{
    return C_SiScalar<LHS>( HF32( m_Scalar ) + HF32( rhs.m_Scalar ) );
}

/// Binary Subtraction Operator
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiScalar<LHS>::operator - ( const C_SiScalar<LHS> PCREF rhs ) const __restrict
{
    return C_SiScalar<LHS>( HF32( m_Scalar ) - HF32( rhs.m_Scalar ) );
}

/// Binary Multiplication Operator
template < typename LHS >
template < typename RHS >
NOALIAS __forceinline const C_SiScalar< typename SI_Product< LHS, RHS >::Result > C_SiScalar<LHS>::operator * ( const C_SiScalar<RHS> PCREF rhs ) const __restrict
{
    typedef typename SI_Product< LHS, RHS >::Result RESULT;
    return C_SiScalar<RESULT>( HF32( m_Scalar ) * HF32( rhs.m_Scalar ) );
}

/// Binary Division Operator
template < typename LHS >
template < typename RHS >
NOALIAS __forceinline const C_SiScalar< typename SI_Quotient< LHS, RHS >::Result > C_SiScalar<LHS>::operator / ( const C_SiScalar<RHS> PCREF rhs ) const __restrict
{
    typedef typename SI_Quotient< LHS, RHS >::Result RESULT;
    return C_SiScalar<RESULT>( HF32( m_Scalar ) / HF32( rhs.m_Scalar ) );
}

/// Binary Multiplication Operator
template < typename LHS >
template < typename RHS >
NOALIAS __forceinline const C_SiVector< typename SI_Product< LHS, RHS >::Result > C_SiScalar<LHS>::operator * ( const C_SiVector<RHS> PCREF rhs ) const __restrict
{
    typedef typename SI_Product< LHS, RHS >::Result RESULT;
    return C_SiVector<RESULT>( rhs.m_Vector * HF32( m_Scalar ) );
}

/// Update operators

template < typename LHS >
NOALIAS __forceinline void C_SiScalar<LHS>::operator += ( const C_SiScalar<LHS> PCREF rhs ) __restrict
{
    m_Scalar = HF32( m_Scalar ) + HF32( rhs.m_Scalar );
}

template < typename LHS >
NOALIAS __forceinline void C_SiScalar<LHS>::operator -= ( const C_SiScalar<LHS> PCREF rhs ) __restrict
{
    m_Scalar = HF32( m_Scalar ) - HF32( rhs.m_Scalar );
}

template < typename LHS >
NOALIAS __forceinline void C_SiScalar<LHS>::operator *= ( const C_SiScalar<SI_None> PCREF rhs ) __restrict
{
    m_Scalar = HF32( m_Scalar ) * HF32( rhs.m_Scalar );
}

template < typename LHS >
NOALIAS __forceinline void C_SiScalar<LHS>::operator /= ( const C_SiScalar<SI_None> PCREF rhs ) __restrict
{
    m_Scalar = HF32( m_Scalar ) / HF32( rhs.m_Scalar );
}

/// Reciprocal
template < typename LHS >
NOALIAS __forceinline const C_SiScalar< typename SI_Inverse< LHS >::Result > C_SiScalar<LHS>::Reciprocal() const __restrict
{
    typedef typename SI_Inverse< LHS >::Result RESULT;
    return C_SiScalar<RESULT>( m_Scalar.Reciprocal() );
}

/// Comparison Operator <
template < typename LHS >
NOALIAS __forceinline const S32 C_SiScalar<LHS>::operator < ( const C_SiScalar<LHS> PCREF rhs ) const __restrict
{
    return HF32( m_Scalar ) <  HF32( rhs.m_Scalar );
}

/// Comparison Operator >
template < typename LHS >
NOALIAS __forceinline const S32 C_SiScalar<LHS>::operator > ( const C_SiScalar<LHS> PCREF rhs ) const __restrict
{
    return HF32( m_Scalar ) >  HF32( rhs.m_Scalar );
}

/// Comparison Operator <=
template < typename LHS >
NOALIAS __forceinline const S32 C_SiScalar<LHS>::operator <= ( const C_SiScalar<LHS> PCREF rhs ) const __restrict
{
    return HF32( m_Scalar ) <= HF32( rhs.m_Scalar );
}

/// Comparison Operator >=
template < typename LHS >
NOALIAS __forceinline const S32 C_SiScalar<LHS>::operator >= ( const C_SiScalar<LHS> PCREF rhs ) const __restrict
{
    return HF32( m_Scalar ) >= HF32( rhs.m_Scalar );
}

/// Comparison Operator ==
template < typename LHS >
NOALIAS __forceinline const S32 C_SiScalar<LHS>::operator == ( const C_SiScalar<LHS> PCREF rhs ) const __restrict
{
    return HF32( m_Scalar ) == HF32( rhs.m_Scalar );
}

/// Comparison Operator !=
template < typename LHS >
NOALIAS __forceinline const S32 C_SiScalar<LHS>::operator != ( const C_SiScalar<LHS> PCREF rhs ) const __restrict
{
    return HF32( m_Scalar ) != HF32( rhs.m_Scalar );
}

/// Finite Test
template < typename LHS >
NOALIAS __forceinline const Scalar_Bool C_SiScalar<LHS>::Finite() const __restrict
{
    return m_Scalar.Finite();
}


template < typename LHS >
NOALIAS __forceinline const C_SiScalar< typename SI_SquareRoot<LHS>::Result > Sqrt( const C_SiScalar<LHS> PCREF src )
{
    typedef typename SI_SquareRoot<LHS>::Result RESULT;
    return C_SiScalar<RESULT>( src.m_Scalar.Sqrt() );
}

template < typename LHS >
NOALIAS __forceinline const C_SiScalar< typename SI_ReciprocalSquareRoot<LHS>::Result > ReciprocalSqrt( const C_SiScalar<LHS> PCREF src )
{
    typedef typename SI_ReciprocalSquareRoot<LHS>::Result RESULT;
    return C_SiScalar<RESULT>( src.m_Scalar.ReciprocalSqrt() );
}


//
// Promote HF32's to C_SiScalar<>'s
//

template < typename LHS > NOALIAS __forceinline const C_SiScalar<LHS>   operator + ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs )      { return lhs + C_SiScalar<LHS>( rhs ); }
template < typename LHS > NOALIAS __forceinline const C_SiScalar<LHS>   operator - ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs )      { return lhs - C_SiScalar<LHS>( rhs ); }
template < typename LHS > NOALIAS __forceinline const C_SiScalar<LHS>   operator * ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs )      { return lhs * C_SiScalar<SI_None>( rhs ); }
template < typename LHS > NOALIAS __forceinline const C_SiScalar<LHS>   operator / ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs )      { return lhs / C_SiScalar<SI_None>( rhs ); }

template < typename LHS > NOALIAS __forceinline const S32               operator <  ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs )     { return lhs <  C_SiScalar<LHS>( rhs ); }
template < typename LHS > NOALIAS __forceinline const S32               operator >  ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs )     { return lhs >  C_SiScalar<LHS>( rhs ); }
template < typename LHS > NOALIAS __forceinline const S32               operator <= ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs )     { return lhs <= C_SiScalar<LHS>( rhs ); }
template < typename LHS > NOALIAS __forceinline const S32               operator >= ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs )     { return lhs >= C_SiScalar<LHS>( rhs ); }
template < typename LHS > NOALIAS __forceinline const S32               operator == ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs )     { return lhs == C_SiScalar<LHS>( rhs ); }
template < typename LHS > NOALIAS __forceinline const S32               operator != ( const C_SiScalar<LHS> PCREF lhs, const HF32 PCREF rhs )     { return lhs != C_SiScalar<LHS>( rhs ); }

template < typename RHS > NOALIAS __forceinline const C_SiScalar<RHS>   operator + ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs )      { return C_SiScalar<RHS>( lhs ) + rhs; }
template < typename RHS > NOALIAS __forceinline const C_SiScalar<RHS>   operator - ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs )      { return C_SiScalar<RHS>( lhs ) - rhs; }
template < typename RHS > NOALIAS __forceinline const C_SiScalar<RHS>   operator * ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs )      { return C_SiScalar<SI_None>( lhs ) * rhs; }
template < typename RHS > NOALIAS __forceinline const C_SiScalar<RHS>   operator / ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs )      { return C_SiScalar<SI_None>( lhs ) / rhs; }

template < typename RHS > NOALIAS __forceinline const S32               operator <  ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs )     { return C_SiScalar<RHS>( lhs ) <  rhs; }
template < typename RHS > NOALIAS __forceinline const S32               operator >  ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs )     { return C_SiScalar<RHS>( lhs ) >  rhs; }
template < typename RHS > NOALIAS __forceinline const S32               operator <= ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs )     { return C_SiScalar<RHS>( lhs ) <= rhs; }
template < typename RHS > NOALIAS __forceinline const S32               operator >= ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs )     { return C_SiScalar<RHS>( lhs ) >= rhs; }
template < typename RHS > NOALIAS __forceinline const S32               operator == ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs )     { return C_SiScalar<RHS>( lhs ) == rhs; }
template < typename RHS > NOALIAS __forceinline const S32               operator != ( const HF32 PCREF lhs, const C_SiScalar<RHS> PCREF rhs )     { return C_SiScalar<RHS>( lhs ) != rhs; }

template < typename RHS > NOALIAS __forceinline void                    operator += ( HF32& lhs, const C_SiScalar<RHS> PCREF rhs )          { lhs += rhs.Get(); }
template < typename RHS > NOALIAS __forceinline void                    operator -= ( HF32& lhs, const C_SiScalar<RHS> PCREF rhs )          { lhs -= rhs.Get(); }
template < typename RHS > NOALIAS __forceinline void                    operator *= ( HF32& lhs, const C_SiScalar<RHS> PCREF rhs )          { lhs *= rhs.Get(); }
template < typename RHS > NOALIAS __forceinline void                    operator /= ( HF32& lhs, const C_SiScalar<RHS> PCREF rhs )          { lhs /= rhs.Get(); }

//
// Promote F32's to C_SiScalar<>'s
//

template < typename LHS > NOALIAS __forceinline const C_SiScalar<LHS>   operator + ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs )        { return lhs + C_SiScalar<LHS>( rhs ); }
template < typename LHS > NOALIAS __forceinline const C_SiScalar<LHS>   operator - ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs )        { return lhs - C_SiScalar<LHS>( rhs ); }
template < typename LHS > NOALIAS __forceinline const C_SiScalar<LHS>   operator * ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs )        { return lhs * C_SiScalar<SI_None>( rhs ); }
template < typename LHS > NOALIAS __forceinline const C_SiScalar<LHS>   operator / ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs )        { return lhs / C_SiScalar<SI_None>( rhs ); }

template < typename LHS > NOALIAS __forceinline const S32               operator <  ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs )       { return lhs <  C_SiScalar<LHS>( rhs ); }
template < typename LHS > NOALIAS __forceinline const S32               operator >  ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs )       { return lhs >  C_SiScalar<LHS>( rhs ); }
template < typename LHS > NOALIAS __forceinline const S32               operator <= ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs )       { return lhs <= C_SiScalar<LHS>( rhs ); }
template < typename LHS > NOALIAS __forceinline const S32               operator >= ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs )       { return lhs >= C_SiScalar<LHS>( rhs ); }
template < typename LHS > NOALIAS __forceinline const S32               operator == ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs )       { return lhs == C_SiScalar<LHS>( rhs ); }
template < typename LHS > NOALIAS __forceinline const S32               operator != ( const C_SiScalar<LHS> PCREF lhs, const F32& rhs )       { return lhs != C_SiScalar<LHS>( rhs ); }

template < typename RHS > NOALIAS __forceinline const C_SiScalar<RHS>   operator + ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs )        { return C_SiScalar<RHS>( lhs ) + rhs; }
template < typename RHS > NOALIAS __forceinline const C_SiScalar<RHS>   operator - ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs )        { return C_SiScalar<RHS>( lhs ) - rhs; }
template < typename RHS > NOALIAS __forceinline const C_SiScalar<RHS>   operator * ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs )        { return C_SiScalar<SI_None>( lhs ) * rhs; }
template < typename RHS > NOALIAS __forceinline const C_SiScalar<RHS>   operator / ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs )        { return C_SiScalar<SI_None>( lhs ) / rhs; }

template < typename RHS > NOALIAS __forceinline const S32               operator <  ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs )       { return C_SiScalar<RHS>( lhs ) <  rhs; }
template < typename RHS > NOALIAS __forceinline const S32               operator >  ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs )       { return C_SiScalar<RHS>( lhs ) >  rhs; }
template < typename RHS > NOALIAS __forceinline const S32               operator <= ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs )       { return C_SiScalar<RHS>( lhs ) <= rhs; }
template < typename RHS > NOALIAS __forceinline const S32               operator >= ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs )       { return C_SiScalar<RHS>( lhs ) >= rhs; }
template < typename RHS > NOALIAS __forceinline const S32               operator == ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs )       { return C_SiScalar<RHS>( lhs ) == rhs; }
template < typename RHS > NOALIAS __forceinline const S32               operator != ( const F32& lhs, const C_SiScalar<RHS> PCREF rhs )       { return C_SiScalar<RHS>( lhs ) != rhs; }

template < typename RHS > NOALIAS __forceinline void                    operator += ( F32& lhs, const C_SiScalar<RHS> PCREF rhs )             { lhs += rhs.Get(); }
template < typename RHS > NOALIAS __forceinline void                    operator -= ( F32& lhs, const C_SiScalar<RHS> PCREF rhs )             { lhs -= rhs.Get(); }
template < typename RHS > NOALIAS __forceinline void                    operator *= ( F32& lhs, const C_SiScalar<RHS> PCREF rhs )             { lhs *= rhs.Get(); }
template < typename RHS > NOALIAS __forceinline void                    operator /= ( F32& lhs, const C_SiScalar<RHS> PCREF rhs )             { lhs /= rhs.Get(); }

//
// Promote C_Vector to C_SiVector< SI_None >
//

template < typename LHS > NOALIAS __forceinline const C_SiVector<LHS>   operator * ( const C_SiScalar<LHS> PCREF lhs, const C_Vector PCREF rhs )  { return lhs * C_SiVector<SI_None>( rhs ); }
template < typename RHS > NOALIAS __forceinline const C_SiVector<RHS>   operator * ( const C_Vector PCREF lhs, const C_SiScalar<RHS> PCREF rhs )  { return C_SiVector<SI_None>( lhs ) * rhs; }

template < typename LHS > NOALIAS __forceinline const C_SiScalar<LHS>   operator | ( const C_SiVector<LHS> PCREF lhs, const C_Vector PCREF rhs )  { return lhs | C_SiVector<SI_None>( rhs ); }
template < typename RHS > NOALIAS __forceinline const C_SiScalar<RHS>   operator | ( const C_Vector PCREF lhs, const C_SiVector<RHS> PCREF rhs )  { return C_SiVector<SI_None>( lhs ) | rhs; }

template < typename LHS > NOALIAS __forceinline const C_SiVector<LHS>   operator ^ ( const C_SiVector<LHS> PCREF lhs, const C_Vector PCREF rhs )  { return lhs ^ C_SiVector<SI_None>( rhs ); }
template < typename RHS > NOALIAS __forceinline const C_SiVector<RHS>   operator ^ ( const C_Vector PCREF lhs, const C_SiVector<RHS> PCREF rhs )  { return C_SiVector<SI_None>( lhs ) ^ rhs; }

//
// Promote C_Normal to C_SiVector< SI_None >
//

template < typename LHS > NOALIAS __forceinline const C_SiVector<LHS>   operator * ( const C_SiScalar<LHS> PCREF lhs, const C_Normal PCREF rhs )  { return lhs * C_SiVector<SI_None>( rhs ); }
template < typename RHS > NOALIAS __forceinline const C_SiVector<RHS>   operator * ( const C_Normal PCREF lhs, const C_SiScalar<RHS> PCREF rhs )  { return C_SiVector<SI_None>( lhs ) * rhs; }

template < typename LHS > NOALIAS __forceinline const C_SiScalar<LHS>   operator | ( const C_SiVector<LHS> PCREF lhs, const C_Normal PCREF rhs )  { return lhs | C_SiVector<SI_None>( rhs ); }
template < typename RHS > NOALIAS __forceinline const C_SiScalar<RHS>   operator | ( const C_Normal PCREF lhs, const C_SiVector<RHS> PCREF rhs )  { return C_SiVector<SI_None>( lhs ) | rhs; }

template < typename LHS > NOALIAS __forceinline const C_SiVector<LHS>   operator ^ ( const C_SiVector<LHS> PCREF lhs, const C_Normal PCREF rhs )  { return lhs ^ C_SiVector<SI_None>( rhs ); }
template < typename RHS > NOALIAS __forceinline const C_SiVector<RHS>   operator ^ ( const C_Normal PCREF lhs, const C_SiVector<RHS> PCREF rhs )  { return C_SiVector<SI_None>( lhs ) ^ rhs; }






//
// *****************************************************
// *** Implementation of C_SiVertex Member Functions ***
// *****************************************************
//

/// Default Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVertex<LHS>::C_SiVertex()
{
}

/// Undefined Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVertex<LHS>::C_SiVertex( const _UNDEFINED undefined ) :
    m_Vertex( UNDEFINED )
{
}

/// Copy Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVertex<LHS>::C_SiVertex( const C_SiVertex<LHS>& copy ) :
    m_Vertex( copy.m_Vertex )
{
}

/// HF32 XYZ Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVertex<LHS>::C_SiVertex( const HF32 PCREF x, const HF32 PCREF y, const HF32 PCREF z ) :
    m_Vertex( x, y, z )
{
}

/// C_SiScalar<LHS> XYZ Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVertex<LHS>::C_SiVertex( const C_SiScalar<LHS> PCREF x, const C_SiScalar<LHS> PCREF y, const C_SiScalar<LHS> PCREF z ) :
    m_Vertex( x.m_Scalar, y.m_Scalar, z.m_Scalar )
{
}

/// HVec3 Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVertex<LHS>::C_SiVertex( const HVec3 PCREF vec ) :
    m_Vertex( vec )
{
}

/// C_Vertex Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVertex<LHS>::C_SiVertex( const C_Vertex PCREF vertex ) :
    m_Vertex( vertex )
{
}

/// HVec3_Multiply Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVertex<LHS>::C_SiVertex( const HVec3_Multiply& src )
    : m_Vertex( HVec3( src ) )
{
}

/// HVec3_Add Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVertex<LHS>::C_SiVertex( const HVec3_Add& src )
    : m_Vertex( HVec3( src ) )
{
}

/// HVec3_Subtract Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVertex<LHS>::C_SiVertex( const HVec3_Subtract& src )
    : m_Vertex( HVec3( src ) )
{
}

/// Copy Assignment
template < typename LHS >
NOALIAS __forceinline void C_SiVertex<LHS>::operator = ( const C_SiVertex<LHS>& copy )
{
    m_Vertex = copy.m_Vertex;
}

/// Get C_Vertex: There is no cast operator to C_Vertex since this would allow the implicit casting away of the unit type!
template < typename LHS >
NOALIAS __forceinline const C_Vertex C_SiVertex<LHS>::Get() const __restrict
{
    return m_Vertex;
}

/// Element Accessor X
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiVertex<LHS>::X() const __restrict
{
    return C_SiScalar<LHS>( m_Vertex.X() );
}

/// Element Accessor Y
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiVertex<LHS>::Y() const __restrict
{
    return C_SiScalar<LHS>( m_Vertex.Y() );
}

/// Element Accessor Z
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiVertex<LHS>::Z() const __restrict
{
    return C_SiScalar<LHS>( m_Vertex.Z() );
}

/// Element Zero X
template < typename LHS >
NOALIAS __forceinline void C_SiVertex<LHS>::ZeroX() __restrict
{
    m_Vertex.ZeroX();
}

/// Element Zero Y
template < typename LHS >
NOALIAS __forceinline void C_SiVertex<LHS>::ZeroY() __restrict
{
    m_Vertex.ZeroY();
}

/// Element Zero Z
template < typename LHS >
NOALIAS __forceinline void C_SiVertex<LHS>::ZeroZ() __restrict
{
    m_Vertex.ZeroZ();
}

/// Element Zero XY
template < typename LHS >
NOALIAS __forceinline void C_SiVertex<LHS>::ZeroXY() __restrict
{
    m_Vertex.ZeroXY();
}

/// Element Zero XZ
template < typename LHS >
NOALIAS __forceinline void C_SiVertex<LHS>::ZeroXZ() __restrict
{
    m_Vertex.ZeroXZ();
}

/// Element Zero YZ
template < typename LHS >
NOALIAS __forceinline void C_SiVertex<LHS>::ZeroYZ() __restrict
{
    m_Vertex.ZeroYZ();
}

/// Zero the vertex if any element exceeds +/- bounds
template < typename LHS >
NOALIAS __forceinline void C_SiVertex<LHS>::ZeroIfOutOfBounds( const Maths::HF32 PCREF bounds ) __restrict
{
    m_Vertex.ZeroIfOutOfBounds( bounds );
}

/// Element Zero X
template < typename LHS >
NOALIAS __forceinline const C_SiVertex<LHS> C_SiVertex<LHS>::ZeroedX() const __restrict
{
    return C_SiVertex<LHS>( m_Vertex.ZeroedX() );
}

/// Element Zero Y
template < typename LHS >
NOALIAS __forceinline const C_SiVertex<LHS> C_SiVertex<LHS>::ZeroedY() const __restrict
{
    return C_SiVertex<LHS>( m_Vertex.ZeroedY() );
}

/// Element Zero Z
template < typename LHS >
NOALIAS __forceinline const C_SiVertex<LHS> C_SiVertex<LHS>::ZeroedZ() const __restrict
{
    return C_SiVertex<LHS>( m_Vertex.ZeroedZ() );
}

/// Element Zero XY
template < typename LHS >
NOALIAS __forceinline const C_SiVertex<LHS> C_SiVertex<LHS>::ZeroedXY() const __restrict
{
    return C_SiVertex<LHS>( m_Vertex.ZeroedXY() );
}

/// Element Zero XZ
template < typename LHS >
NOALIAS __forceinline const C_SiVertex<LHS> C_SiVertex<LHS>::ZeroedXZ() const __restrict
{
    return C_SiVertex<LHS>( m_Vertex.ZeroedXZ() );
}

/// Element Zero YZ
template < typename LHS >
NOALIAS __forceinline const C_SiVertex<LHS> C_SiVertex<LHS>::ZeroedYZ() const __restrict
{
    return C_SiVertex<LHS>( m_Vertex.ZeroedYZ() );
}

/// Lerp
template < typename LHS >
NOALIAS __forceinline const C_SiVertex<LHS> C_SiVertex<LHS>::Lerp( const C_SiVertex<LHS> PCREF rhs, const HF32 PCREF f ) const __restrict
{
    return C_SiVertex<LHS>( m_Vertex + ( rhs.m_Vertex - m_Vertex ) * f );
}

/// Distance
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiVertex<LHS>::Distance( const C_SiVertex<LHS> PCREF rhs ) const __restrict
{
    return C_SiScalar<LHS>( m_Vertex.Distance( rhs.m_Vertex ) );
}

/// Vertex Addition Operator
template < typename LHS >
NOALIAS __forceinline const C_SiVertex<LHS> C_SiVertex<LHS>::operator + ( const C_SiVertex<LHS> PCREF rhs ) const __restrict
{
    return C_SiVertex<LHS>( m_Vertex + rhs.m_Vertex );
}

/// Vertex Subtraction Operator
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> C_SiVertex<LHS>::operator - ( const C_SiVertex<LHS> PCREF rhs ) const __restrict
{
    return C_SiVector<LHS>( m_Vertex - rhs.m_Vertex );
}

/// Add Update Operator
template < typename LHS >
NOALIAS __forceinline void C_SiVertex<LHS>::operator += ( const C_SiVertex<LHS> PCREF rhs ) __restrict
{
    m_Vertex += rhs.m_Vertex;
}

/// Subtract Update Operator
template < typename LHS >
NOALIAS __forceinline void C_SiVertex<LHS>::operator -= ( const C_SiVertex<LHS> PCREF rhs ) __restrict
{
    m_Vertex -= rhs.m_Vertex;
}

/// Vertex + Vector Addition Operator
template < typename LHS >
NOALIAS __forceinline const C_SiVertex<LHS> C_SiVertex<LHS>::operator + ( const C_SiVector<LHS> PCREF rhs ) const __restrict
{
    return C_SiVertex<LHS>( m_Vertex + rhs.m_Vector );
}

/// Vertex - Vector Subtraction Operator
template < typename LHS >
NOALIAS __forceinline const C_SiVertex<LHS> C_SiVertex<LHS>::operator - ( const C_SiVector<LHS> PCREF rhs ) const __restrict
{
    return C_SiVertex<LHS>( m_Vertex - rhs.m_Vector );
}

/// Vertex += Vector Addition Operator
template < typename LHS >
NOALIAS __forceinline void C_SiVertex<LHS>::operator += ( const C_SiVector<LHS> PCREF rhs ) __restrict
{
    m_Vertex += rhs.m_Vector;
}

/// Vertex -= Vector Subtraction Operator
template < typename LHS >
NOALIAS __forceinline void C_SiVertex<LHS>::operator -= ( const C_SiVector<LHS> PCREF rhs ) __restrict
{
    m_Vertex -= rhs.m_Vector;
}

/// Binary Multiplication Operator
template < typename LHS >
template < typename RHS >
NOALIAS __forceinline const C_SiVertex< typename SI_Product< LHS, RHS >::Result > C_SiVertex<LHS>::operator * ( const C_SiScalar<RHS> PCREF rhs ) const __restrict
{
    typedef typename SI_Product< LHS, RHS >::Result RESULT;
    return C_SiVertex<RESULT>( m_Vertex * rhs.m_Scalar );
}

/// Binary Division Operator
template < typename LHS >
template < typename RHS >
NOALIAS __forceinline const C_SiVertex< typename SI_Quotient< LHS, RHS >::Result > C_SiVertex<LHS>::operator / ( const C_SiScalar<RHS> PCREF rhs ) const __restrict
{
    typedef typename SI_Quotient< LHS, RHS >::Result RESULT;
    return C_SiVertex<RESULT>( m_Vertex / rhs.m_Scalar );
}

/// Scalar HF32 Multiplication Operator
template < typename LHS >
NOALIAS __forceinline const C_SiVertex<LHS> C_SiVertex<LHS>::operator * ( const HF32 PCREF rhs ) const __restrict
{
    return C_SiVertex<LHS>( m_Vertex * rhs );
}

/// Scalar HF32 Division Operator
template < typename LHS >
NOALIAS __forceinline const C_SiVertex<LHS> C_SiVertex<LHS>::operator / ( const HF32 PCREF rhs ) const __restrict
{
    //return C_SiVertex<LHS>( m_Vertex / rhs );
    return C_SiVertex<LHS>( m_Vertex * rhs.Reciprocal() );
}

/// Comparison Operator <
template < typename LHS >
NOALIAS __forceinline const S32 C_SiVertex<LHS>::operator < ( const C_SiVertex<LHS> PCREF rhs ) const __restrict
{
    return m_Vertex < rhs.m_Vertex;
}

/// Comparison Operator >
template < typename LHS >
NOALIAS __forceinline const S32 C_SiVertex<LHS>::operator > ( const C_SiVertex<LHS> PCREF rhs ) const __restrict
{
    return m_Vertex > rhs.m_Vertex;
}

/// Comparison Operator <=
template < typename LHS >
NOALIAS __forceinline const S32 C_SiVertex<LHS>::operator <= ( const C_SiVertex<LHS> PCREF rhs ) const __restrict
{
    return m_Vertex <= rhs.m_Vertex;
}

/// Comparison Operator >=
template < typename LHS >
NOALIAS __forceinline const S32 C_SiVertex<LHS>::operator >= ( const C_SiVertex<LHS> PCREF rhs ) const __restrict
{
    return m_Vertex >= rhs.m_Vertex;
}

/// Comparison Operator ==
template < typename LHS >
NOALIAS __forceinline const S32 C_SiVertex<LHS>::operator == ( const C_SiVertex<LHS> PCREF rhs ) const __restrict
{
    return m_Vertex == rhs.m_Vertex;
}

/// Comparison Operator !=
template < typename LHS >
NOALIAS __forceinline const S32 C_SiVertex<LHS>::operator != ( const C_SiVertex<LHS> PCREF rhs ) const __restrict
{
    return m_Vertex != rhs.m_Vertex;
}

/// Finite Test
template < typename LHS >
NOALIAS __forceinline const Scalar_Bool C_SiVertex<LHS>::Finite() const __restrict
{
    return m_Vertex.Finite();
}



template < typename LHS > NOALIAS __forceinline C_SiVertex<LHS>   operator * ( const C_SiVertex<LHS> PCREF v, const F32& f )    { return v * HF32( f ); }
template < typename LHS > NOALIAS __forceinline C_SiVertex<LHS>   operator / ( const C_SiVertex<LHS> PCREF v, const F32& f )    { return v / HF32( f ); }

template < typename LHS > NOALIAS __forceinline void              operator *= ( C_SiVertex<LHS>& v, const F32& f )        { v *= HF32( f ); }
template < typename LHS > NOALIAS __forceinline void              operator /= ( C_SiVertex<LHS>& v, const F32& f )        { v /= HF32( f ); }





//
// *****************************************************
// *** Implementation of C_SiVector Member Functions ***
// *****************************************************
//

/// Default Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVector<LHS>::C_SiVector()
{
}

/// Undefined Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVector<LHS>::C_SiVector( const _UNDEFINED undefined ) :
    m_Vector( UNDEFINED )
{
}

/// Copy Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVector<LHS>::C_SiVector( const C_SiVector<LHS>& copy ) :
    m_Vector( copy.m_Vector )
{
}

/// C_SiScalar<LHS> XYZ Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVector<LHS>::C_SiVector( const C_SiScalar<LHS> PCREF x, const C_SiScalar<LHS> PCREF y, const C_SiScalar<LHS> PCREF z ) :
    m_Vector( x.m_Scalar, y.m_Scalar, z.m_Scalar )
{
}

/// HVec3 Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVector<LHS>::C_SiVector( const HVec3 PCREF vec ) :
    m_Vector( vec )
{
}

/// C_Vector Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVector<LHS>::C_SiVector( const C_Vector PCREF vector ) :
    m_Vector( vector )
{
}

/// HVec3_Multiply Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVector<LHS>::C_SiVector( const HVec3_Multiply& src )
    : m_Vector( HVec3( src ) )
{
}

/// HVec3_Add Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVector<LHS>::C_SiVector( const HVec3_Add& src )
    : m_Vector( HVec3( src ) )
{
}

/// HVec3_Subtract Constructor
template < typename LHS >
NOALIAS __forceinline C_SiVector<LHS>::C_SiVector( const HVec3_Subtract& src )
    : m_Vector( HVec3( src ) )
{
}

/// Copy Assignment
template < typename LHS >
NOALIAS __forceinline void C_SiVector<LHS>::operator = ( const C_SiVector<LHS>& copy )
{
    m_Vector = copy.m_Vector;
}

/// Get C_Vector: There is no cast operator to C_Vector since this would allow the implicit casting away of the unit type!
template < typename LHS >
NOALIAS __forceinline const C_Vector C_SiVector<LHS>::Get() const __restrict
{
    return m_Vector;
}

/// Element Accessor X
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiVector<LHS>::X() const __restrict
{
    return C_SiScalar<LHS>( m_Vector.X() );
}

/// Element Accessor Y
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiVector<LHS>::Y() const __restrict
{
    return C_SiScalar<LHS>( m_Vector.Y() );
}

/// Element Accessor Z
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiVector<LHS>::Z() const __restrict
{
    return C_SiScalar<LHS>( m_Vector.Z() );
}

/// Element Zero X
template < typename LHS >
NOALIAS __forceinline void C_SiVector<LHS>::ZeroX() __restrict
{
    m_Vector.ZeroX();
}

/// Element Zero Y
template < typename LHS >
NOALIAS __forceinline void C_SiVector<LHS>::ZeroY() __restrict
{
    m_Vector.ZeroY();
}

/// Element Zero Z
template < typename LHS >
NOALIAS __forceinline void C_SiVector<LHS>::ZeroZ() __restrict
{
    m_Vector.ZeroZ();
}

/// Element Zero XY
template < typename LHS >
NOALIAS __forceinline void C_SiVector<LHS>::ZeroXY() __restrict
{
    m_Vector.ZeroXY();
}

/// Element Zero XZ
template < typename LHS >
NOALIAS __forceinline void C_SiVector<LHS>::ZeroXZ() __restrict
{
    m_Vector.ZeroXZ();
}

/// Element Zero YZ
template < typename LHS >
NOALIAS __forceinline void C_SiVector<LHS>::ZeroYZ() __restrict
{
    m_Vector.ZeroYZ();
}

/// Zero the vector if any element exceeds +/- bounds
template < typename LHS >
NOALIAS __forceinline void C_SiVector<LHS>::ZeroIfOutOfBounds( const Maths::HF32 PCREF bounds ) __restrict
{
    m_Vector.ZeroIfOutOfBounds( bounds );
}

/// Element Zero X
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> C_SiVector<LHS>::ZeroedX() const __restrict
{
    return C_SiVector<LHS>( m_Vector.ZeroedX() );
}

/// Element Zero Y
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> C_SiVector<LHS>::ZeroedY() const __restrict
{
    return C_SiVector<LHS>( m_Vector.ZeroedY() );
}

/// Element Zero Z
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> C_SiVector<LHS>::ZeroedZ() const __restrict
{
    return C_SiVector<LHS>( m_Vector.ZeroedZ() );
}

/// Element Zero XY
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> C_SiVector<LHS>::ZeroedXY() const __restrict
{
    return C_SiVector<LHS>( m_Vector.ZeroedXY() );
}

/// Element Zero XZ
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> C_SiVector<LHS>::ZeroedXZ() const __restrict
{
    return C_SiVector<LHS>( m_Vector.ZeroedXZ() );
}

/// Element Zero YZ
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> C_SiVector<LHS>::ZeroedYZ() const __restrict
{
    return C_SiVector<LHS>( m_Vector.ZeroedYZ() );
}

/// Get length of vector
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiVector<LHS>::Length() const __restrict
{
    return C_SiScalar<LHS>( Maths::Length( m_Vector ) );
}

/// Get length squared of vector
template < typename LHS >
NOALIAS __forceinline const C_SiScalar< typename SI_Squared< LHS >::Result > C_SiVector<LHS>::LengthSquared() const __restrict
{
    //return C_SiScalar< typename SI_Squared< LHS >::Result >( m_Vector.LengthSquared() );
    return C_SiScalar< typename SI_Squared< LHS >::Result >( Maths::LengthSquared( m_Vector ) );
}

/// Limit length of vector
template < typename LHS >
NOALIAS __forceinline void C_SiVector<LHS>::Limit( const C_SiScalar<LHS> PCREF limit ) __restrict
{
    #if 0
    const HF32 lengthSquared = LengthSquared().m_Scalar;
    const HF32 one( c_ScalarOne );
    const HF32 nearlyZero( c_ScalarFltMin );
    const HF32 limitMultiplier( ( limit.m_Scalar / Maths::Select( lengthSquared.GT( nearlyZero ), length, one ) ).Min( one ) );
    m_Vector *= limitMultiplier;
    #else
    m_Vector.LimitLength( limit.m_Scalar );
    #endif
}

/// Normal of vector
template < typename LHS >
NOALIAS __forceinline const C_Normalised C_SiVector<LHS>::Normalised() const __restrict
{
    //return m_Vector.Normalised();
    return NormalLengthSquaredAndReciprocal( m_Vector );
}

/// Normal of vector
template < typename LHS >
NOALIAS __forceinline const C_Normalised C_SiVector<LHS>::SafeNormalised( const C_Normal PCREF zeroLengthResult ) const __restrict
{
    return m_Vector.SafeNormalised( zeroLengthResult );
}

/// Unary Negation
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> C_SiVector<LHS>::operator - () const __restrict
{
    return C_SiVector<LHS>( -m_Vector );
}

/// Vector Addition Operator
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> C_SiVector<LHS>::operator + ( const C_SiVector<LHS> PCREF rhs ) const __restrict
{
    return C_SiVector<LHS>( m_Vector + rhs.m_Vector );
}

/// Vector Subtraction Operator
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> C_SiVector<LHS>::operator - ( const C_SiVector<LHS> PCREF rhs ) const __restrict
{
    return C_SiVector<LHS>( m_Vector - rhs.m_Vector );
}

/// Add Update Operator
template < typename LHS >
NOALIAS __forceinline void C_SiVector<LHS>::operator += ( const C_SiVector<LHS> PCREF rhs ) __restrict
{
    m_Vector += rhs.m_Vector;
}

/// Subtract Update Operator
template < typename LHS >
NOALIAS __forceinline void C_SiVector<LHS>::operator -= ( const C_SiVector<LHS> PCREF rhs ) __restrict
{
    m_Vector -= rhs.m_Vector;
}

/// Scalar Multiplication Operator
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> C_SiVector<LHS>::operator * ( const C_SiScalar<SI_None> PCREF rhs ) const __restrict
{
    return C_SiVector<LHS>( m_Vector * rhs.m_Scalar );
}

/// Scalar Multiplication Update Operator
template < typename LHS >
NOALIAS __forceinline void C_SiVector<LHS>::operator *= ( const C_SiScalar<SI_None> PCREF rhs ) __restrict
{
    m_Vector *= rhs.m_Scalar;
}

/// Scalar Division Operator
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> C_SiVector<LHS>::operator / ( const C_SiScalar<SI_None> PCREF rhs ) const __restrict
{
    return C_SiVector<LHS>( m_Vector / rhs.m_Scalar );
}

/// Scalar Division Update Operator
template < typename LHS >
NOALIAS __forceinline void C_SiVector<LHS>::operator /= ( const C_SiScalar<SI_None> PCREF rhs ) __restrict
{
    m_Vector /= rhs.m_Scalar;
}

/// Binary Multiplication Operator
template < typename LHS >
template < typename RHS >
NOALIAS __forceinline const C_SiVector< typename SI_Product< LHS, RHS >::Result > C_SiVector<LHS>::operator * ( const C_SiScalar<RHS> PCREF rhs ) const __restrict
{
    typedef typename SI_Product< LHS, RHS >::Result RESULT;
    return C_SiVector<RESULT>( m_Vector * rhs.m_Scalar );
}

/// Binary Division Operator
template < typename LHS >
template < typename RHS >
NOALIAS __forceinline const C_SiVector< typename SI_Quotient< LHS, RHS >::Result > C_SiVector<LHS>::operator / ( const C_SiScalar<RHS> PCREF rhs ) const __restrict
{
    typedef typename SI_Quotient< LHS, RHS >::Result RESULT;
    return C_SiVector<RESULT>( m_Vector / rhs.m_Scalar );
}

/// Vector Dot Product Operator
template < typename LHS >
template < typename RHS >
NOALIAS __forceinline const C_SiScalar< typename SI_Product< LHS, RHS >::Result > C_SiVector<LHS>::operator | ( const C_SiVector<RHS> PCREF rhs ) const __restrict
{
    typedef typename SI_Product< LHS, RHS >::Result RESULT;
    return C_SiScalar<RESULT>( m_Vector | rhs.m_Vector );
}

/// Vector Cross Product Operator
template < typename LHS >
template < typename RHS >
NOALIAS __forceinline const C_SiVector< typename SI_Product< LHS, RHS >::Result > C_SiVector<LHS>::operator ^ ( const C_SiVector<RHS> PCREF rhs ) const __restrict
{
    typedef typename SI_Product< LHS, RHS >::Result RESULT;
    return C_SiVector<RESULT>( m_Vector ^ rhs.m_Vector );
}

/// Vector Dot Product Operator
//template < typename LHS >
//NOALIAS __forceinline
//const C_SiScalar<LHS> C_SiVector<LHS>::operator | ( const C_Normal rhs ) const
//{
//    return C_SiScalar<LHS>( m_Vector | rhs );
//}

/// Vector Cross Product Operator
//template < typename LHS >
//NOALIAS __forceinline
//const C_SiVector<LHS> C_SiVector<LHS>::operator ^ ( const C_Normal rhs ) const
//{
//    return C_SiVector<LHS>( m_Vector ^ rhs );
//}

/// Matrix Multiplication Operator
template < typename LHS >
template < typename RHS, bool RHS_OrthoNorm >
NOALIAS __forceinline const C_SiVector< typename SI_Product< LHS, RHS >::Result > C_SiVector<LHS>::operator * ( const C_SiRotMatrix<RHS,RHS_OrthoNorm> PCREF rhs ) const __restrict
{
    typedef typename SI_Product< LHS, RHS >::Result RESULT;
    return C_SiVector<RESULT>( m_Vector * rhs.m_Matrix );
}

/// Get component of this vector parallel to input normal
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> C_SiVector<LHS>::Parallel( const C_Normal PCREF normal ) const __restrict
{
    return C_SiVector<LHS>( m_Vector.Parallel( normal ) );
}

/// Get component of this vector perpendicular to input normal
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> C_SiVector<LHS>::Perpendicular( const C_Normal PCREF normal ) const __restrict
{
    return C_SiVector<LHS>( m_Vector.Perpendicular( normal ) );
}

/// Get components of this vector parallel & perpendicular to input normal
template < typename LHS >
NOALIAS __forceinline void C_SiVector<LHS>::ParaPerp( const C_Normal PCREF normal, C_SiVector<LHS>& para, C_SiVector<LHS>& perp ) const __restrict
{
    m_Vector.ParaPerp( normal, para.m_Vector, perp.m_Vector );
}

/// Star function = Cross Product Matrix ( a* Matrix )
template < typename LHS >
NOALIAS __forceinline const C_SiRotMatrix<LHS,false> C_SiVector<LHS>::Star() const __restrict
{
    return m_Vector.Star();
}

/// Star function = Cross Product Matrix ( a* Matrix )
template < typename LHS >
NOALIAS __forceinline const C_SiRotMatrix<LHS,false> C_SiVector<LHS>::StarTransposed() const __restrict
{
    return m_Vector.Star().Transposed();
}

/// Comparison Operator <
template < typename LHS >
NOALIAS __forceinline const S32 C_SiVector<LHS>::operator <  ( const C_SiVector<LHS> PCREF rhs ) const __restrict
{
    return m_Vector < rhs.m_Vector;
}

/// Comparison Operator >
template < typename LHS >
NOALIAS __forceinline const S32 C_SiVector<LHS>::operator >  ( const C_SiVector<LHS> PCREF rhs ) const __restrict
{
    return m_Vector > rhs.m_Vector;
}

/// Comparison Operator <=
template < typename LHS >
NOALIAS __forceinline const S32 C_SiVector<LHS>::operator <= ( const C_SiVector<LHS> PCREF rhs ) const __restrict
{
    return m_Vector <= rhs.m_Vector;
}

/// Comparison Operator >=
template < typename LHS >
NOALIAS __forceinline const S32 C_SiVector<LHS>::operator >= ( const C_SiVector<LHS> PCREF rhs ) const __restrict
{
    return m_Vector >= rhs.m_Vector;
}

/// Comparison Operator ==
template < typename LHS >
NOALIAS __forceinline const S32 C_SiVector<LHS>::operator == ( const C_SiVector<LHS> PCREF rhs ) const __restrict
{
    return m_Vector == rhs.m_Vector;
}

/// Comparison Operator !=
template < typename LHS >
NOALIAS __forceinline const S32 C_SiVector<LHS>::operator != ( const C_SiVector<LHS> PCREF rhs ) const __restrict
{
    return m_Vector != rhs.m_Vector;
}

/// Finite Test
template < typename LHS >
NOALIAS __forceinline const Scalar_Bool C_SiVector<LHS>::Finite() const __restrict
{
    return m_Vector.Finite();
}


template < typename LHS > NOALIAS __forceinline C_SiVector<LHS>   operator * ( const C_SiVector<LHS> PCREF v, const F32& f )    { return v * HF32( f ); }
template < typename LHS > NOALIAS __forceinline C_SiVector<LHS>   operator / ( const C_SiVector<LHS> PCREF v, const F32& f )    { return v / HF32( f ); }

template < typename LHS > NOALIAS __forceinline void              operator *= ( C_SiVector<LHS>& v, const F32& f )        { v *= HF32( f ); }
template < typename LHS > NOALIAS __forceinline void              operator /= ( C_SiVector<LHS>& v, const F32& f )        { v /= HF32( f ); }









//
// *********************************************************
// *** Implementation of C_SiQuaternion Member Functions ***
// *********************************************************
//

/// Default Constructor
template < typename LHS >
NOALIAS __forceinline C_SiQuaternion<LHS>::C_SiQuaternion()
{
}

/// Undefined Constructor
template < typename LHS >
NOALIAS __forceinline C_SiQuaternion<LHS>::C_SiQuaternion( const _UNDEFINED undefined ) :
    m_Quaternion( UNDEFINED )
{
}

/// Copy Constructor
template < typename LHS >
NOALIAS __forceinline C_SiQuaternion<LHS>::C_SiQuaternion( const C_SiQuaternion<LHS>& copy ) :
    m_Quaternion( copy.m_Quaternion )
{
}

/// Quaternion Constructor
template < typename LHS >
NOALIAS __forceinline C_SiQuaternion<LHS>::C_SiQuaternion( const C_Quaternion PCREF quaternion ) :
    m_Quaternion( quaternion )
{
}

/// C_SiVector Constructor: Vector direction is the axis and vector length is the angle in radians
//template < typename LHS >
//NOALIAS __forceinline
//C_SiQuaternion<LHS>::C_SiQuaternion( const C_SiVector<LHS>& vector ) :
//    m_Quaternion( vector.Normalised(), vector.Length() )
//{
//}

/// Axis + Angle Constructor
template < typename LHS >
NOALIAS __forceinline C_SiQuaternion<LHS>::C_SiQuaternion( const C_SiVector<SI_None> PCREF vector, const C_SiScalar<SI_Rad> PCREF theta ) :
    m_Quaternion( HVec3(vector.m_Vector), theta.m_Scalar )
{
}

/// C_SiVector Constructor: Imaginary part is set directly from the input vector, Real part is zero
template < typename LHS >
NOALIAS __forceinline C_SiQuaternion<LHS>::C_SiQuaternion( const C_SiVector<LHS> PCREF vector ) :
    m_Quaternion( vector.m_Vector )
{
}

/// C_SiRotMatrix Constructor
template < typename LHS >
NOALIAS __forceinline C_SiQuaternion<LHS>::C_SiQuaternion( const C_SiRotMatrix<LHS,true>& matrix ) :
    m_Quaternion( matrix.m_Matrix )
{
}

/// HMat33 Constructor
template < typename LHS >
NOALIAS __forceinline C_SiQuaternion<LHS>::C_SiQuaternion( const HMat33& matrix ) :
    m_Quaternion( matrix )
{
}

/// Copy Assignment
template < typename LHS >
NOALIAS __forceinline void C_SiQuaternion<LHS>::operator = ( const C_SiQuaternion<LHS>& copy )
{
    m_Quaternion = copy.m_Quaternion;
}

/// Get Quaternion: There is no cast operator to C_Quaternion since this would allow the implicit casting away of the unit type!
template < typename LHS >
NOALIAS __forceinline const C_Quaternion C_SiQuaternion<LHS>::Get() const __restrict
{
    return m_Quaternion;
}

/// Accessor: Get axis of rotation
template < typename LHS >
NOALIAS __forceinline const C_Normal C_SiQuaternion<LHS>::Axis() const __restrict
{
    return C_Normal( m_Quaternion.Imag() );
}

/// Accessor: Get angle of rotation
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> C_SiQuaternion<LHS>::Angle() const __restrict
{
    const HF32 cosHalfTheta( HF32( m_Quaternion.Real() ) );
    const HF32 halfTheta( ArcCos( cosHalfTheta ) );
    return C_SiScalar<LHS>( halfTheta + halfTheta );
}

/// Addition Operator
template < typename LHS >
NOALIAS __forceinline const C_SiQuaternion<LHS> C_SiQuaternion<LHS>::operator + ( const C_SiQuaternion<LHS> PCREF src ) const __restrict
{
    return C_SiQuaternion<LHS>( m_Quaternion + src.m_Quaternion );
}

/// Add Update Operator : TODO: Not supported by C_Quaternion yet!
//template < typename LHS >
//NOALIAS __forceinline
//void C_SiQuaternion<LHS>::operator += ( const C_SiQuaternion<LHS>& src )
//{
//    m_Quaternion += src.m_Quaternion;
//}

/// Multiplication Operator
template < typename LHS >
template < typename RHS >
NOALIAS __forceinline const C_SiQuaternion< typename SI_Product< LHS, RHS >::Result > C_SiQuaternion<LHS>::operator * ( const C_SiQuaternion<RHS> PCREF src ) const __restrict
{
    typedef typename SI_Product< LHS, RHS >::Result RESULT;
    return C_SiQuaternion<RESULT>( m_Quaternion * src.m_Quaternion );
}

/// Division Operator
template < typename LHS >
template < typename RHS >
NOALIAS __forceinline const C_SiQuaternion< typename SI_Quotient< LHS, RHS >::Result > C_SiQuaternion<LHS>::operator / ( const C_SiQuaternion<RHS> PCREF src ) const __restrict
{
    typedef typename SI_Product< LHS, RHS >::Result RESULT;
    return C_SiQuaternion<RESULT>( m_Quaternion / src.m_Quaternion );
}

/// 'Shortest' Division
template < typename LHS >
template < typename RHS >
NOALIAS __forceinline const C_SiQuaternion< typename SI_Quotient< LHS, RHS >::Result > C_SiQuaternion<LHS>::ShortestDivide( const C_SiQuaternion<RHS> PCREF src ) const __restrict
{
    typedef typename SI_Product< LHS, RHS >::Result RESULT;
    return C_SiQuaternion<RESULT>( m_Quaternion.ShortestDivide( src.m_Quaternion ) );
}

/// Slerp
template < typename LHS >
NOALIAS __forceinline const C_SiQuaternion<LHS> C_SiQuaternion<LHS>::Slerp( const C_SiQuaternion<LHS> PCREF src, const HF32 PCREF f ) const __restrict
{
    const HF32 zero( ZERO_INIT );
    const HF32 one( c_ScalarOne );
    return C_SiQuaternion<LHS>( m_Quaternion.Slerp( src.m_Quaternion, Maths::Min( Maths::Max( f, zero ), one ) ) );
}

/// Normalise: Get normalised copy of quaternion
template < typename LHS >
NOALIAS __forceinline const C_SiQuaternion<LHS> C_SiQuaternion<LHS>::Normalised() const __restrict
{
    return m_Quaternion.Normalised();
}

/// Finite Test
template < typename LHS >
NOALIAS __forceinline const Scalar_Bool C_SiQuaternion<LHS>::Finite() const __restrict
{
    return m_Quaternion.Finite();
}







//
// ********************************************************
// *** Implementation of C_SiRotMatrix Member Functions ***
// ********************************************************
//

/// Default Constructor
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline C_SiRotMatrix<LHS,LHS_OrthoNorm>::C_SiRotMatrix()
{
}

/// Undefined Constructor
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline C_SiRotMatrix<LHS,LHS_OrthoNorm>::C_SiRotMatrix( const _UNDEFINED undefined ) :
    m_Matrix( UNDEFINED )
{
}

/// Copy Constructor
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline C_SiRotMatrix<LHS,LHS_OrthoNorm>::C_SiRotMatrix( const C_SiRotMatrix<LHS,LHS_OrthoNorm>& copy ) :
    m_Matrix( copy.m_Matrix )
{
    #if !defined( CFG_SPEED ) && defined( USER_JDENTON )
    if ( LHS_OrthoNorm )
    {
        const HF32 one( Constants<HF32>::One() );
        const HF32 tolerance( 0.01f );
        const HF32 length_ax( Length( m_Matrix.AX() ) );
        const HF32 length_ay( Length( m_Matrix.AY() ) );
        const HF32 length_az( Length( m_Matrix.AZ() ) );
        if ( ( Abs( length_ax - one ).GT( tolerance ) ) |
             ( Abs( length_ay - one ).GT( tolerance ) ) |
             ( Abs( length_az - one ).GT( tolerance ) ) )
        {
            DebugBreak();
        }
    }
    #endif
}

/// HMat33 Constructor
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline C_SiRotMatrix<LHS,LHS_OrthoNorm>::C_SiRotMatrix( const HMat33& matrix ) :
    m_Matrix( matrix )
{
    #if !defined( CFG_SPEED ) && defined( USER_JDENTON )
    if ( LHS_OrthoNorm )
    {
        const HF32 one( Constants<HF32>::One() );
        const HF32 tolerance( 0.01f );
        const HF32 length_ax( Length( m_Matrix.AX() ) );
        const HF32 length_ay( Length( m_Matrix.AY() ) );
        const HF32 length_az( Length( m_Matrix.AZ() ) );
        if ( ( Abs( length_ax - one ).GT( tolerance ) ) |
             ( Abs( length_ay - one ).GT( tolerance ) ) |
             ( Abs( length_az - one ).GT( tolerance ) ) )
        {
            DebugBreak();
        }
    }
    #endif
}

/// C_SiQuaternion Constructor
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline C_SiRotMatrix<LHS,LHS_OrthoNorm>::C_SiRotMatrix( const C_SiQuaternion<LHS> PCREF quaternion ) :
    m_Matrix( quaternion.m_Quaternion )
{
    #if !defined( CFG_SPEED ) && defined( USER_JDENTON )
    if ( LHS_OrthoNorm )
    {
        const HF32 one( Constants<HF32>::One() );
        const HF32 tolerance( 0.01f );
        const HF32 length_ax( Length( m_Matrix.AX() ) );
        const HF32 length_ay( Length( m_Matrix.AY() ) );
        const HF32 length_az( Length( m_Matrix.AZ() ) );
        if ( ( Abs( length_ax - one ).GT( tolerance ) ) |
             ( Abs( length_ay - one ).GT( tolerance ) ) |
             ( Abs( length_az - one ).GT( tolerance ) ) )
        {
            DebugBreak();
        }
    }
    #endif
}

/// C_SiVector Axis X, Y & Z Constructor
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline C_SiRotMatrix<LHS,LHS_OrthoNorm>::C_SiRotMatrix( const C_SiVector<LHS> PCREF ax, const C_SiVector<LHS> PCREF ay, const C_SiVector<LHS> PCREF az ) :
    m_Matrix( ax.Get(), ay.Get(), az.Get() )
{
    //#if !defined( CFG_SPEED ) && defined( USER_JDENTON )
    //if ( LHS_OrthoNorm )
    //{
    //    const HF32 one( Constants<HF32>::One() );
    //    const HF32 tolerance( 0.01f );
    //    const HF32 length_ax( Length( m_Matrix.AX() ) );
    //    const HF32 length_ay( Length( m_Matrix.AY() ) );
    //    const HF32 length_az( Length( m_Matrix.AZ() ) );
    //    if ( ( Abs( length_ax - one ).GT( tolerance ) ) |
    //         ( Abs( length_ay - one ).GT( tolerance ) ) |
    //         ( Abs( length_az - one ).GT( tolerance ) ) )
    //    {
    //        DebugBreak();
    //    }
    //}
    //#endif
}

/// Get HMat33: There is no cast operator to HMat33 since this would allow the implicit casting away of the unit type!
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline const HMat33 C_SiRotMatrix<LHS,LHS_OrthoNorm>::Get() const __restrict
{
    return m_Matrix;
}

/// Copy Assignment
template < typename LHS, bool LHS_OrthoNorm >
template < bool RHS_OrthoNorm >
NOALIAS __forceinline void C_SiRotMatrix<LHS,LHS_OrthoNorm>::operator = ( const C_SiRotMatrix< LHS, LHS_OrthoNorm || RHS_OrthoNorm >& copy )
{
    m_Matrix = copy.m_Matrix;
    
    #if !defined( CFG_SPEED ) && defined( USER_JDENTON )
    if ( LHS_OrthoNorm )
    {
        const HF32 one( Constants<HF32>::One() );
        const HF32 tolerance( 0.01f );
        const HF32 length_ax( Length( m_Matrix.AX() ) );
        const HF32 length_ay( Length( m_Matrix.AY() ) );
        const HF32 length_az( Length( m_Matrix.AZ() ) );
        if ( ( Abs( length_ax - one ).GT( tolerance ) ) |
             ( Abs( length_ay - one ).GT( tolerance ) ) |
             ( Abs( length_az - one ).GT( tolerance ) ) )
        {
            DebugBreak();
        }
    }
    #endif
}

/// Axis X Accessor
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline const C_SiVector<LHS> C_SiRotMatrix<LHS,LHS_OrthoNorm>::AX() const __restrict
{
    return m_Matrix.AX();
}

/// Axis Y Accessor
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline const C_SiVector<LHS> C_SiRotMatrix<LHS,LHS_OrthoNorm>::AY() const __restrict
{
    return m_Matrix.AY();
}

/// Axis Z Accessor
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline const C_SiVector<LHS> C_SiRotMatrix<LHS,LHS_OrthoNorm>::AZ() const __restrict
{
    return m_Matrix.AZ();
}

/// Matrix Multiply Operator
template < typename LHS, bool LHS_OrthoNorm >
template < typename RHS, bool RHS_OrthoNorm >
NOALIAS __forceinline const C_SiRotMatrix< typename SI_Product< LHS, RHS >::Result, LHS_OrthoNorm && RHS_OrthoNorm > C_SiRotMatrix<LHS,LHS_OrthoNorm>::operator * ( const C_SiRotMatrix<RHS,RHS_OrthoNorm>& src ) const __restrict
{
    typedef typename SI_Product< LHS, RHS >::Result RESULT;
    return C_SiRotMatrix< RESULT, LHS_OrthoNorm && RHS_OrthoNorm >( m_Matrix * src.m_Matrix );
}

/// Matrix Multiply Update Operator
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline void C_SiRotMatrix<LHS,LHS_OrthoNorm>::operator *= ( const C_SiRotMatrix<LHS,LHS_OrthoNorm>& src ) __restrict
{
    m_Matrix *= src.m_Matrix;
}

/// Add Update Operator: Element by element addition
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline const C_SiRotMatrix<LHS,false> C_SiRotMatrix<LHS,LHS_OrthoNorm>::operator + ( const C_SiRotMatrix<LHS,LHS_OrthoNorm>& src ) const __restrict
{
    //return C_SiRotMatrix<LHS>( m_Matrix + src.m_Matrix );
    return C_SiRotMatrix<LHS,LHS_OrthoNorm>( HVec3( m_Matrix.AX() + src.m_Matrix.AX() ),
                                             HVec3( m_Matrix.AY() + src.m_Matrix.AY() ),
                                             HVec3( m_Matrix.AZ() + src.m_Matrix.AZ() ) );
}

/// Addition Update Operator: Element by element addition
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline void C_SiRotMatrix<LHS,LHS_OrthoNorm>::operator += ( const C_SiRotMatrix<LHS,LHS_OrthoNorm>& src ) __restrict
{
    m_Matrix.AX( HVec3( m_Matrix.AX() + src.m_Matrix.AX() ) );
    m_Matrix.AY( HVec3( m_Matrix.AY() + src.m_Matrix.AY() ) );
    m_Matrix.AZ( HVec3( m_Matrix.AZ() + src.m_Matrix.AZ() ) );
}

/// Scalar Multiplication Operator
template < typename LHS, bool LHS_OrthoNorm >
template < typename RHS >
NOALIAS __forceinline const C_SiRotMatrix< typename SI_Product< LHS, RHS >::Result, false > C_SiRotMatrix<LHS,LHS_OrthoNorm>::operator * ( const C_SiScalar<RHS> PCREF rhs ) const __restrict
{
    typedef typename SI_Product< LHS, RHS >::Result RESULT;
    return C_SiRotMatrix<RESULT,false>( C_SiVector<LHS>( m_Matrix.AX() ) * rhs,
                                        C_SiVector<LHS>( m_Matrix.AY() ) * rhs,
                                        C_SiVector<LHS>( m_Matrix.AZ() ) * rhs );
}

/// Vector Multiplication Operator
template < typename LHS, bool LHS_OrthoNorm >
template < typename RHS >
NOALIAS __forceinline const C_SiVector< typename SI_Product< LHS, RHS >::Result > C_SiRotMatrix<LHS,LHS_OrthoNorm>::operator * ( const C_SiVector<RHS> PCREF rhs ) const __restrict
{
    typedef typename SI_Product< LHS, RHS >::Result RESULT;
    return C_SiVector<RESULT>( m_Matrix * rhs.m_Vector );
}

/// Scalar Division Operator
template < typename LHS, bool LHS_OrthoNorm >
template < typename RHS >
NOALIAS __forceinline const C_SiRotMatrix< typename SI_Quotient< LHS, RHS >::Result, false > C_SiRotMatrix<LHS,LHS_OrthoNorm>::operator / ( const C_SiScalar<RHS> PCREF rhs ) const __restrict
{
    typedef typename SI_Quotient< LHS, RHS >::Result RESULT;
    const HF32 reciprocalRhs( Reciprocal( rhs.Get() ) );
    return C_SiRotMatrix<LHS,false>( m_Matrix.AX() * reciprocalRhs,
                                     m_Matrix.AY() * reciprocalRhs,
                                     m_Matrix.AZ() * reciprocalRhs );
}

/// Transpose
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline const C_SiRotMatrix< typename SI_Inverse< LHS >::Result, LHS_OrthoNorm > C_SiRotMatrix<LHS,LHS_OrthoNorm>::Transpose() const __restrict
{
    typedef typename SI_Inverse< LHS >::Result RESULT;
    return C_SiRotMatrix<RESULT,LHS_OrthoNorm>( m_Matrix.Transposed() );
}

/// Inverse
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline const C_SiRotMatrix< typename SI_Inverse< LHS >::Result, LHS_OrthoNorm > C_SiRotMatrix<LHS,LHS_OrthoNorm>::Inverse() const __restrict
{
    typedef typename SI_Inverse< LHS >::Result RESULT;
    return C_SiRotMatrix<RESULT,LHS_OrthoNorm>( m_Matrix.Inverse() );
}

/// Normalise
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline void C_SiRotMatrix<LHS,LHS_OrthoNorm>::Normalise() __restrict
{
	m_Matrix.AX( (Maths::HVec3)m_Matrix.AX().Normalised() );
    m_Matrix.AY( (Maths::HVec3)m_Matrix.AY().Normalised() );
    m_Matrix.AZ( (Maths::HVec3)m_Matrix.AZ().Normalised() );
}

/// Ortho-Normalise
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline void C_SiRotMatrix<LHS,LHS_OrthoNorm>::OrthoNormalise() __restrict
{
    m_Matrix.NormaliseZY();
}

/// Finite Test
template < typename LHS, bool LHS_OrthoNorm >
NOALIAS __forceinline const Scalar_Bool C_SiRotMatrix<LHS,LHS_OrthoNorm>::Finite() const __restrict
{
    return m_Matrix.Finite();
}








//
// -------------------------------------------
// |-- Non-Member Friend Support Functions --|
// -------------------------------------------
//

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? resultIfTrue : resultIfFalse )
template < typename LHS >
NOALIAS __forceinline const C_SiScalar<LHS> Select( const Scalar_Bool PCREF selector, const C_SiScalar<LHS> PCREF resultIfTrue, const C_SiScalar<LHS> PCREF resultIfFalse )
{
    return C_SiScalar<LHS>( Select( selector, resultIfTrue.m_Scalar, resultIfFalse.m_Scalar ) );
}

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? resultIfTrue : resultIfFalse )
template < typename LHS >
NOALIAS __forceinline const C_SiVertex<LHS> Select( const Vector3_Bool PCREF selector, const C_SiVertex<LHS> PCREF resultIfTrue, const C_SiVertex<LHS> PCREF resultIfFalse )
{
    return C_SiVertex<LHS>( Select( selector, resultIfTrue.m_Vertex, resultIfFalse.m_Vertex ) );
}

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? resultIfTrue : resultIfFalse )
template < typename LHS >
NOALIAS __forceinline const C_SiVector<LHS> Select( const Vector3_Bool PCREF selector, const C_SiVector<LHS> PCREF resultIfTrue, const C_SiVector<LHS> PCREF resultIfFalse )
{
    return C_SiVector<LHS>( Select( selector, resultIfTrue.m_Vector, resultIfFalse.m_Vector ) );
}

/// Selector: Selects fields from the inputs based on the selector mask: result = ( selector ? resultIfTrue : resultIfFalse )
template < typename LHS >
NOALIAS __forceinline const C_SiQuaternion<LHS> Select( const Vector4_Bool PCREF selector, const C_SiQuaternion<LHS> PCREF resultIfTrue, const C_SiQuaternion<LHS> PCREF resultIfFalse )
{
    return C_SiQuaternion<LHS>( Select( selector, resultIfTrue.m_Quaternion, resultIfFalse.m_Quaternion ) );
}





/// Parallel Axis Theorem: Calculate the inertia tensor of an object at a point offset from the centre of mass
NOALIAS __forceinline
const C_InertiaTensorKgM2 ParallelAxis( const C_InertiaTensorKgM2& it, const C_VectorM PCREF axis, const C_MassKg PCREF m )
{
    // http://scienceworld.wolfram.com/physics/ParallelAxisTheorem.html
    
    const C_DistanceM Ax( axis.X() );
    const C_DistanceM Ay( axis.Y() );
    const C_DistanceM Az( axis.Z() );
    
    const C_VectorTensorKgM2 offset_it_x = C_SiVector<SI_M2>(  Ay * Ay + Az * Az, -Ax * Ay,           -Ax * Az           ) * m;
    const C_VectorTensorKgM2 offset_it_y = C_SiVector<SI_M2>( -Ax * Ay,            Ax * Ax + Az * Az, -Ay * Az           ) * m;
    const C_VectorTensorKgM2 offset_it_z = C_SiVector<SI_M2>( -Ax * Az,           -Ay * Az,            Ax * Ax + Ay * Ay ) * m;
    
    return it + C_InertiaTensorKgM2( offset_it_x, offset_it_y, offset_it_z );
}




}; // namespace Maths




#endif
