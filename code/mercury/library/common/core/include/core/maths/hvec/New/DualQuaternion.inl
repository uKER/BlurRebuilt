#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_DUAL_QUATERNION_INL
#define SOFT_MATHS_HVEC_NEW_DUAL_QUATERNION_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_DUAL_QUATERNION_INL
#define MATHS_HVEC_NEW_DUAL_QUATERNION_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup DualQuaternion
/// @{

/// Make DualQuaternion from a Vec8F32
NOALIAS __forceinline const DualQuaternion MakeDualQuaternion( const Vec8F32 MATREF v )
{
    DualQuaternion result;
    result.m_V = v;
    return result;
}

/// Make DualQuaternion from a Vector8
NOALIAS __forceinline const DualQuaternion MakeDualQuaternion( const Vector8& v )
{
    DualQuaternion result;
    result.m_V = v.m_V;
    return result;
}

/// Make DualQuaternion from a Quaternion
NOALIAS __forceinline const DualQuaternion MakeDualQuaternion( const Quaternion PCREF q )
{
    DualQuaternion result;
    result.m_V.m_V0 = q.m_Q;
    result.m_V.m_V1 = Const<Vec4F32>::Zero();
    return result;
}

/// Make DualQuaternion from a Vector3 translation
/// WARNING: The Dual Quaternion representation of a Point is NOT the same as the representation of a Translation! (There's a factor of 2 difference)
NOALIAS __forceinline const DualQuaternion MakeDualQuaternion( const Vector3 PCREF t )
{
    DualQuaternion result;
    result.m_V.m_V0 = Const<Vec4F32>::UnitW();
    result.m_V.m_V1 = MakeVec4F32( t ) * Const<VF32>::Half();
    return result;
}

/// Make DualQuaternion from a Point3
/// WARNING: The Dual Quaternion representation of a Point is NOT the same as the representation of a Translation! (There's a factor of 2 difference)
NOALIAS __forceinline const DualQuaternion MakeDualQuaternion( const Point3 PCREF p )
{
    DualQuaternion result;
    result.m_V.m_V0 = Const<Vec4F32>::UnitW();
    result.m_V.m_V1 = MakeVec4F32( p, Const<VF32>::Zero() );
    return result;
}

/// Make DualQuaternion from a Quaternion & a Vector3 translation
NOALIAS __forceinline const DualQuaternion MakeDualQuaternion( const Quaternion PCREF q, const Vector3 PCREF t )
{
    /// q = ( q.i, q.j, q.k, q.1, 0,      0,      0,      0 )
    /// t = ( 0,   0,   0,   1,   1/2t.x, 1/2t.y, 1/2t.z, 0 )
    /// 
    /// c=q*t | 0 | 0 | 0 | 1 | 1/2t.x | 1/2t.y | 1/2t.z | 0 |
    /// ------+---+---+---+---+--------+--------+--------+---+
    /// q.i   | 0 | 0 | 0 | i | -e     | -ek    | ej     | 0 |
    /// ------+---+---+---+---+--------+--------+--------+---+
    /// q.j   | 0 | 0 | 0 | j | ek     | -e     | -ei    | 0 |
    /// ------+---+---+---+---+--------+--------+--------+---+
    /// q.k   | 0 | 0 | 0 | k | -ej    | ei     | -e     | 0 |
    /// ------+---+---+---+---+--------+--------+--------+---+
    /// q.1   | 0 | 0 | 0 | 1 | ei     | ej     | ek     | 0 |
    /// ------+---+---+---+---+--------+--------+--------+---+
    /// 0     | 0 | 0 | 0 | 0 | 0      | 0      | 0      | 0 |
    /// ------+---+---+---+---+--------+--------+--------+---+
    /// 0     | 0 | 0 | 0 | 0 | 0      | 0      | 0      | 0 |
    /// ------+---+---+---+---+--------+--------+--------+---+
    /// 0     | 0 | 0 | 0 | 0 | 0      | 0      | 0      | 0 |
    /// ------+---+---+---+---+--------+--------+--------+---+
    /// 0     | 0 | 0 | 0 | 0 | 0      | 0      | 0      | 0 |
    /// ------+---+---+---+---+--------+--------+--------+---+
    ///                                 
    /// c.i  = a.i
    /// c.j  = a.j
    /// c.k  = a.k
    /// c.1  = a.1
    /// 
    /// c.ei = + ( a.i * 0    ) - ( a.j * b.ek ) + ( a.k * b.ej ) + ( a.1 * b.ei )
    /// c.ej = + ( a.i * b.ek ) + ( a.j * 0    ) - ( a.k * b.ei ) + ( a.1 * b.ej )
    /// c.ek = - ( a.i * b.ej ) + ( a.j * b.ei ) + ( a.k * 0    ) + ( a.1 * b.ek )
    /// c.e  = - ( a.i * b.ei ) - ( a.j * b.ej ) - ( a.k * b.ek ) + ( a.1 * 0    )
    
    //return MakeDualQuaternion( q ) * MakeDualQuaternion( t );
    
    const VF32 a0i( X( q.m_Q ) );
    const VF32 a0j( Y( q.m_Q ) );
    const VF32 a0k( Z( q.m_Q ) );
    const VF32 a01( W( q.m_Q ) );
    
    const Vec4F32 be( MakeVec4F32( t ) * Const<VF32>::Half() );
    
    static const int _ei = 0, _ej = 1, _ek = 2, _e1 = 3;
    
    DualQuaternion result;
    result.m_V.m_V0 = q.m_Q;
    result.m_V.m_V1 = ( Negate<0,0,1,1>( a0i * Permute<_e1,_ek,_ej,_ei>( be ) ) +
                        Negate<1,0,0,1>( a0j * Permute<_ek,_e1,_ei,_ej>( be ) ) ) +
                      ( Negate<0,1,0,1>( a0k * Permute<_ej,_ei,_e1,_ek>( be ) ) +
                                         a01 * be );
    return result;
}

/// Make DualQuaternion from a QuatTrans
NOALIAS __forceinline const DualQuaternion MakeDualQuaternion( const QuatTrans& qt )
{
    return MakeDualQuaternion( Quat( qt ), Trans( qt ) );
}



/// DualQuaternion Get Quat components as Vec4F32
NOALIAS __forceinline const Vec4F32 GetQuatVec4( const DualQuaternion& a )
{
    return a.m_V.m_V0;
}

/// DualQuaternion Get Dual components as Vec4F32
NOALIAS __forceinline const Vec4F32 GetDualVec4( const DualQuaternion& a )
{
    return a.m_V.m_V1;
}

/// DualQuaternion Get Quaternion components
NOALIAS __forceinline const Quaternion GetQuaternion( const DualQuaternion& a )
{
    return MakeQuaternion( a.m_V.m_V0 );
}

/// DualQuaternion Get Translation components
NOALIAS __forceinline const Vector3 GetTranslation( const DualQuaternion& a )
{
    return GetVector3( !GetQuaternion( a ) * MakeQuaternion( a.m_V.m_V1 ) ) * Const<VF32>::Two();
}

/// DualQuaternion Get Quaternion components
NOALIAS __forceinline const Quaternion Quat( const DualQuaternion& a )
{
    return MakeQuaternion( a.m_V.m_V0 );
}

/// DualQuaternion Get Translation components
NOALIAS __forceinline const Vector3 Trans( const DualQuaternion& a )
{
    return GetVector3( !GetQuaternion( a ) * MakeQuaternion( a.m_V.m_V1 ) ) * Const<VF32>::Two();
}

/// DualQuaternion Get Point components
NOALIAS __forceinline const Point3 GetPoint( const DualQuaternion& a )
{
    return MakePoint3( GetVector3( !GetQuaternion( a ) * MakeQuaternion( a.m_V.m_V1 ) ) );
}


/// DualQuaternion Select
NOALIAS __forceinline const DualQuaternion Select( const VBool PCREF condition, const DualQuaternion& result_if_true, const DualQuaternion& result_if_false )
{
    DualQuaternion result;
    result.m_V = Select( condition, result_if_true.m_V, result_if_false.m_V );
    return result;
}


/// DualQuaternion Norm: Result is a DualScalar number with the real part in X and the dual part in Y
NOALIAS __forceinline const DualScalar Norm( const DualQuaternion& a )
{
    const VF32 norm_q0_squared( LengthSquared4( a.m_V.m_V0 ) );
    VF32 norm_q0, rcp_norm_q0;
    SqrtAndRcpSqrt( norm_q0_squared, &norm_q0, &rcp_norm_q0 );
    const VF32 norm_qe( Dot4( a.m_V.m_V0, a.m_V.m_V1 ) * rcp_norm_q0 );
    return MakeDualScalar( norm_q0, norm_qe );
}

/// DualQuaternion Norm Squared: Result is a DualScalar number with the real part in X and the dual part in Y
NOALIAS __forceinline const DualScalar NormSquared( const DualQuaternion& a )
{
    const VF32 two( Const<VF32>::Two() );
    const VF32 norm_q0_squared( LengthSquared4( a.m_V.m_V0 ) );
    const VF32 norm_qe_squared( Dot4( a.m_V.m_V0, a.m_V.m_V1 ) * two );
    return MakeDualScalar( norm_q0_squared, norm_qe_squared );
}

/// DualQuaternion Conjugate ( Multiplicative Inverse of Unit Dual Quaternion )
/// for Q = q0 + e.qe, multiplicative conjugate of Q, !Q = !q0 + e.!qe ( e (epsilon) is dual unit, e^2 = 0 )
NOALIAS __forceinline const DualQuaternion Conjugate( const DualQuaternion& a )
{
    DualQuaternion result;
    result.m_V.m_V0 = NegateXYZ( a.m_V.m_V0 );
    result.m_V.m_V1 = NegateXYZ( a.m_V.m_V1 );
    return result;
}

/// DualQuaternion Dual Conjugate
/// for Q = q0 + e.qe, dual conjugate of Q = q0 - e.qe ( e (epsilon) is dual unit, e^2 = 0 )
NOALIAS __forceinline const DualQuaternion DualConjugate( const DualQuaternion& a )
{
    DualQuaternion result;
    result.m_V.m_V0 =  a.m_V.m_V0;
    result.m_V.m_V1 = -a.m_V.m_V1;
    return result;
}

/// DualQuaternion Versor Conjugate (combination of standard conjugate & dual conjugate)
/// for Q = q0 + e.qe, versor conjugate of Q = !q0 - e.!qe ( e (epsilon) is dual unit, e^2 = 0 )
NOALIAS __forceinline const DualQuaternion VersorConjugate( const DualQuaternion& a )
{
    DualQuaternion result;
    result.m_V.m_V0 = NegateXYZ( a.m_V.m_V0 );
    result.m_V.m_V1 = NegateW( a.m_V.m_V1 );
    return result;
}


/// DualQuaternion Scalar Division (faster but only 22 bit precision)
NOALIAS __forceinline const DualQuaternion Div( const DualQuaternion& a, const VF32 VECREF b )
{
    DualQuaternion result;
    result.m_V = a.m_V / b;
    return result;
}

/// DualQuaternion Scalar Division (slower but full 24 bit precision)
NOALIAS __forceinline const DualQuaternion Div24( const DualQuaternion& a, const VF32 VECREF b )
{
    DualQuaternion result;
    result.m_V = Div24( a.m_V, b );
    return result;
}


/// DualQuaternion Dual Division (faster but only 22 bit precision)
NOALIAS __forceinline const DualQuaternion Div( const DualQuaternion& a, const DualScalar PCREF b )
{
    return a * Inverse( b );
}

/// DualQuaternion Dual Division (slower but full 24 bit precision)
NOALIAS __forceinline const DualQuaternion Div24( const DualQuaternion& a, const DualScalar PCREF b )
{
    return a * Inverse( b );
}



/// DualQuaternion Negation Operator
NOALIAS __forceinline const DualQuaternion operator - ( const DualQuaternion& a )
{
    DualQuaternion result;
    result.m_V = -a.m_V;
    return result;
}

/// DualQuaternion Addition Operator
NOALIAS __forceinline const DualQuaternion operator + ( const DualQuaternion& a, const DualQuaternion& b )
{
    DualQuaternion result;
    result.m_V = a.m_V + b.m_V;
    return result;
}

/// DualQuaternion Subtraction Operator
NOALIAS __forceinline const DualQuaternion operator - ( const DualQuaternion& a, const DualQuaternion& b )
{
    DualQuaternion result;
    result.m_V = a.m_V - b.m_V;
    return result;
}

/// DualQuaternion Addition Update Operator
NOALIAS __forceinline const DualQuaternion operator += ( DualQuaternion& a, const DualQuaternion& b )
{
    a.m_V += b.m_V;
    return a;
}

/// DualQuaternion Subtraction Update Operator
NOALIAS __forceinline const DualQuaternion operator -= ( DualQuaternion& a, const DualQuaternion& b )
{
    a.m_V -= b.m_V;
    return a;
}

/// DualQuaternion Scalar Multiplication Operator
NOALIAS __forceinline const DualQuaternion operator * ( const DualQuaternion& a, const VF32 VECREF b )
{
    DualQuaternion result;
    result.m_V = a.m_V * b;
    return result;
}

/// DualQuaternion Scalar Multiplication Operator
NOALIAS __forceinline const DualQuaternion operator * ( const VF32 VECREF a, const DualQuaternion& b )
{
    DualQuaternion result;
    result.m_V = b.m_V * a;
    return result;
}

/// DualQuaternion Scalar Division Operator
NOALIAS __forceinline const DualQuaternion operator / ( const DualQuaternion& a, const VF32 VECREF b )
{
    DualQuaternion result;
    result.m_V = a.m_V / b;
    return result;
}

/// DualQuaternion DualScalar Multiplication Operator
NOALIAS __forceinline const DualQuaternion operator * ( const DualScalar PCREF a, const DualQuaternion& b )
{
    /// @code
    /// 
    /// c=a*b | b.i | b.j | b.k | b.1 | | b.ei | b.ej | b.ek | b.e |
    /// ------+-----+-----+-----+-----+ +------+------+------+-----+
    /// a.1   | i   | j   | k   | 1   | | ei   | ej   | ek   | e   |
    /// ------+-----+-----+-----+-----+ +------+------+------+-----+
    /// a.e   | ei  | ej  | ek  | e   | | 0    | 0    | 0    | 0   |
    /// ------+-----+-----+-----+-----+ +------+------+------+-----+
    /// 
    /// c.i  = a.1 * b.i
    /// c.j  = a.1 * b.j
    /// c.k  = a.1 * b.k
    /// c.1  = a.1 * b.1
    /// 
    /// c.ei = a.1 * b.ei + a.e * b.i
    /// c.ej = a.1 * b.ej + a.e * b.j
    /// c.ek = a.1 * b.ek + a.e * b.k
    /// c.e  = a.1 * b.e  + a.e * b.1
    /// 
    /// @endcode
    
    DualQuaternion result;
    result.m_V.m_V0 = Real( a ) * b.m_V.m_V0;
    result.m_V.m_V1 = Real( a ) * b.m_V.m_V1 + Dual( a ) * b.m_V.m_V0;
    return result;
}

/// DualQuaternion DualScalar Multiplication Operator
NOALIAS __forceinline const DualQuaternion operator * ( const DualQuaternion& a, const DualScalar PCREF b )
{
    /// @code
    /// 
    /// c=a*b | b.1 | b.e |
    /// ------+-----+-----+
    /// a.i   | i   | ei  |
    /// ------+-----+-----+
    /// a.j   | j   | ej  |
    /// ------+-----+-----+
    /// a.k   | k   | ek  |
    /// ------+-----+-----+
    /// a.1   | 1   | e   |
    /// ------+-----+-----+
    /// a.ei  | ei  | 0   |
    /// ------+-----+-----+
    /// a.ej  | ej  | 0   |
    /// ------+-----+-----+
    /// a.ek  | ek  | 0   |
    /// ------+-----+-----+
    /// a.e   | e   | 0   |
    /// ------+-----+-----+
    /// 
    /// c.i  = a.i * b.1
    /// c.j  = a.j * b.1
    /// c.k  = a.k * b.1
    /// c.1  = a.1 * b.1
    /// 
    /// c.ei = a.ei * b.1 + a.i * b.e
    /// c.ej = a.ej * b.1 + a.j * b.e
    /// c.ek = a.ek * b.1 + a.k * b.e
    /// c.e  = a.e  * b.1 + a.1 * b.e
    /// 
    /// @endcode
    
    
    DualQuaternion result;
    result.m_V.m_V0 = a.m_V.m_V0 * Real( b );
    result.m_V.m_V1 = a.m_V.m_V1 * Real( b ) + a.m_V.m_V0 * Dual( b );
    return result;
}

/// DualQuaternion Dual Division Operator
NOALIAS __forceinline const DualQuaternion operator / ( const DualQuaternion& a, const DualScalar PCREF b )
{
    return a * Inverse( b );
}

/// DualQuaternion Multiplication Operator
NOALIAS __forceinline const DualQuaternion operator * ( const DualQuaternion& a, const DualQuaternion& b )
{
    /// @code
    /// 
    /// Dual Quaternion Multiplication Table
    /// 
    /// c=a*b | b.i | b.j | b.k | b.1 | | b.ei | b.ej | b.ek | b.e |
    /// ------+-----+-----+-----+-----+ +------+------+------+-----+
    /// a.i   | -1  | -k  | j   | i   | | -e   | -ek  | ej   | ei  |
    /// ------+-----+-----+-----+-----+ +------+------+------+-----+
    /// a.j   | k   | -1  | -i  | j   | | ek   | -e   | -ei  | ej  |
    /// ------+-----+-----+-----+-----+ +------+------+------+-----+
    /// a.k   | -j  | i   | -1  | k   | | -ej  | ei   | -e   | ek  |
    /// ------+-----+-----+-----+-----+ +------+------+------+-----+
    /// a.1   | i   | j   | k   | 1   | | ei   | ej   | ek   | e   |
    /// ------+-----+-----+-----+-----+ +------+------+------+-----+
    ///                                 
    /// ------+-----+-----+-----+-----+ +------+------+------+-----+
    /// a.ei  | -e  | -ek | ej  | ei  | | 0    | 0    | 0    | 0   |
    /// ------+-----+-----+-----+-----+ +------+------+------+-----+
    /// a.ej  | ek  | -e  | -ei | ej  | | 0    | 0    | 0    | 0   |
    /// ------+-----+-----+-----+-----+ +------+------+------+-----+
    /// a.ek  | -ej | ei  | -e  | ek  | | 0    | 0    | 0    | 0   |
    /// ------+-----+-----+-----+-----+ +------+------+------+-----+
    /// a.e   | ei  | ej  | ek  | e   | | 0    | 0    | 0    | 0   |
    /// ------+-----+-----+-----+-----+ +------+------+------+-----+
    ///                                 
    /// c.i  = + ( a.i * b.1 )  - ( a.j * b.k )  + ( a.k * b.j )  + ( a.1 * b.i )
    /// c.j  = + ( a.i * b.k )  + ( a.j * b.1 )  - ( a.k * b.i )  + ( a.1 * b.j )
    /// c.k  = - ( a.i * b.j )  + ( a.j * b.i )  + ( a.k * b.1 )  + ( a.1 * b.k )
    /// c.1  = - ( a.i * b.i )  - ( a.j * b.j )  - ( a.k * b.k )  + ( a.1 * b.1 )
    /// 
    /// c.ei = + ( a.i * b.e  ) - ( a.j * b.ek ) + ( a.k * b.ej ) + ( a.1 * b.ei )  + ( a.ei * b.1 ) - ( a.ej * b.k ) + ( a.ek * b.j ) + ( a.e * b.i )
    /// c.ej = + ( a.i * b.ek ) + ( a.j * b.e  ) - ( a.k * b.ei ) + ( a.1 * b.ej )  + ( a.ei * b.k ) + ( a.ej * b.1 ) - ( a.ek * b.i ) + ( a.e * b.j )
    /// c.ek = - ( a.i * b.ej ) + ( a.j * b.ei ) + ( a.k * b.e  ) + ( a.1 * b.ek )  - ( a.ei * b.j ) + ( a.ej * b.i ) + ( a.ek * b.1 ) + ( a.e * b.k )
    /// c.e  = - ( a.i * b.ei ) - ( a.j * b.ej ) - ( a.k * b.ek ) + ( a.1 * b.e  )  - ( a.ei * b.i ) - ( a.ej * b.j ) - ( a.ek * b.k ) + ( a.e * b.1 )
    /// 
    /// @endcode
    
    static const int _i = 0, _j = 1, _k = 2, _1 = 3, _ei = 0, _ej = 1, _ek = 2, _e1 = 3;
    
    DualQuaternion result;
    
    #if 0
    
    const Quaternion a0( MakeQuaternion( a.m_V.m_V0 ) );
    const Quaternion ae( MakeQuaternion( a.m_V.m_V1 ) );
    const Quaternion b0( MakeQuaternion( b.m_V.m_V0 ) );
    const Quaternion be( MakeQuaternion( b.m_V.m_V1 ) );
    
    result.m_V.m_V0 = GetVec4F32( a0 * b0 );
    result.m_V.m_V1 = GetVec4F32( a0 * be ) + GetVec4F32( ae * b0 );
    
    #else
    
    const VF32 a0i( X( a.m_V.m_V0 ) );
    const VF32 a0j( Y( a.m_V.m_V0 ) );
    const VF32 a0k( Z( a.m_V.m_V0 ) );
    const VF32 a01( W( a.m_V.m_V0 ) );
    
    const VF32 aei( X( a.m_V.m_V1 ) );
    const VF32 aej( Y( a.m_V.m_V1 ) );
    const VF32 aek( Z( a.m_V.m_V1 ) );
    const VF32 ae1( W( a.m_V.m_V1 ) );
    
    const Vec4F32 b0( b.m_V.m_V0 );
    const Vec4F32 be( b.m_V.m_V1 );
    
    const Vec4F32 b01kji( Negate<0,0,1,1>( Permute<_1,_k,_j,_i>( b0 ) ) );
    const Vec4F32 b0k1ij( Negate<1,0,0,1>( Permute<_k,_1,_i,_j>( b0 ) ) );
    const Vec4F32 b0ji1k( Negate<0,1,0,1>( Permute<_j,_i,_1,_k>( b0 ) ) );
    
    const Vec4F32 be1kji( Negate<0,0,1,1>( Permute<_e1,_ek,_ej,_ei>( be ) ) );
    const Vec4F32 bek1ij( Negate<1,0,0,1>( Permute<_ek,_e1,_ei,_ej>( be ) ) );
    const Vec4F32 beji1k( Negate<0,1,0,1>( Permute<_ej,_ei,_e1,_ek>( be ) ) );
    
    /// c.i  = + ( a.i * b.1 )  - ( a.j * b.k )  + ( a.k * b.j )  + ( a.1 * b.i )
    /// c.j  = + ( a.i * b.k )  + ( a.j * b.1 )  - ( a.k * b.i )  + ( a.1 * b.j )
    /// c.k  = - ( a.i * b.j )  + ( a.j * b.i )  + ( a.k * b.1 )  + ( a.1 * b.k )
    /// c.1  = - ( a.i * b.i )  - ( a.j * b.j )  - ( a.k * b.k )  + ( a.1 * b.1 )
    result.m_V.m_V0 = MulAdd( a0i,  b01kji,
                              a0j * b0k1ij ) +
                      MulAdd( a0k,  b0ji1k,
                              a01 * b0 );
    
    /// c.ei = + ( a.i * b.e  ) - ( a.j * b.ek ) + ( a.k * b.ej ) + ( a.1 * b.ei )
    /// c.ej = + ( a.i * b.ek ) + ( a.j * b.e  ) - ( a.k * b.ei ) + ( a.1 * b.ej )
    /// c.ek = - ( a.i * b.ej ) + ( a.j * b.ei ) + ( a.k * b.e  ) + ( a.1 * b.ek )
    /// c.e  = - ( a.i * b.ei ) - ( a.j * b.ej ) - ( a.k * b.ek ) + ( a.1 * b.e  )
    result.m_V.m_V1 = ( MulAdd( a0i,  be1kji,
                                a0j * bek1ij ) +
                        MulAdd( a0k,  beji1k,
                                a01 * be ) ) +
    
    /// c.ei += ( a.ei * b.1 ) - ( a.ej * b.k ) + ( a.ek * b.j ) + ( a.e * b.i )
    /// c.ej += ( a.ei * b.k ) + ( a.ej * b.1 ) - ( a.ek * b.i ) + ( a.e * b.j )
    /// c.ek -= ( a.ei * b.j ) + ( a.ej * b.i ) + ( a.ek * b.1 ) + ( a.e * b.k )
    /// c.e  -= ( a.ei * b.i ) - ( a.ej * b.j ) - ( a.ek * b.k ) + ( a.e * b.1 )
                      ( MulAdd( aei,  b01kji,
                                aej * b0k1ij ) +
                        MulAdd( aek,  b0ji1k,
                                ae1 * b0 ) );
    
    #endif
    
    return result;
}

/// DualQuaternion Conjugate Operator ( Multiplicative Inverse of Unit Dual Quaternion )
NOALIAS __forceinline const DualQuaternion operator ! ( const DualQuaternion& a )
{
    return Conjugate( a );
}

/// DualQuaternion Right Division Operator: num / denom = num * !denom
NOALIAS __forceinline const DualQuaternion operator / ( const DualQuaternion& num, const DualQuaternion& denom )
{
    return num * !denom;
}


/// DualQuaternion * Quaternion Multiplication Operator
NOALIAS __forceinline const DualQuaternion operator * ( const DualQuaternion& a, const Quaternion PCREF b )
{
    return a * MakeDualQuaternion( b );
}

/// Quaternion * DualQuaternion Multiplication Operator
NOALIAS __forceinline const DualQuaternion operator * ( const Quaternion PCREF a, const DualQuaternion& b )
{
    return MakeDualQuaternion( a ) * b;
}


/// DualQuaternion Point3 Multiplication Operator: Point3 * DualQuaternion --> Point3
NOALIAS __forceinline const Point3 operator * ( const Point3 PCREF p, const DualQuaternion& dq )
{
    return GetPoint( VersorConjugate( dq ) * MakeDualQuaternion( p ) * dq );
}

/// DualQuaternion Vector3 Multiplication Operator: Vector3 * DualQuaternion --> Vector3
NOALIAS __forceinline const Vector3 operator * ( const Vector3 PCREF v, const DualQuaternion& dq )
{
    return v * GetQuaternion( dq );
}

/// DualQuaternion Normal3 Multiplication Operator: Normal3 * DualQuaternion --> Normal3
NOALIAS __forceinline const Normal3 operator * ( const Normal3 PCREF n, const DualQuaternion& dq )
{
    return n * GetQuaternion( dq );
}


/// DualQuaternion Nearest: Returns whichever one of the two equivalent destinations is nearest to the source
NOALIAS __forceinline const DualQuaternion Nearest( const DualQuaternion& src, const DualQuaternion& dst )
{
    /// @par NOTE: This function returns the nearest destination rather than source so that it's result and
    /// @par parameters are consistent with the NearestLerp/NearestSlerp functions! This also means that
    /// @par the results of an interpolation do not flip hemispheres relative to the source.
    
    return MakeDualQuaternion( Nearest( GetQuaternion( src ), GetQuaternion( dst ) ), GetTranslation( dst ) );
}


/// DualQuaternion Left Division: denom \ num = !denom * num
NOALIAS __forceinline const DualQuaternion LeftDivide( const DualQuaternion& denom, const DualQuaternion& num )
{
    return !denom * num;
}

/// DualQuaternion Right Division: num / denom = num * !denom
NOALIAS __forceinline const DualQuaternion RightDivide( const DualQuaternion& num, const DualQuaternion& denom )
{
    return num * !denom;
}

/// DualQuaternion Left Division: denom \ num = !denom * num - negates denom if necessary
NOALIAS __forceinline const DualQuaternion NearestLeftDivide( const DualQuaternion& denom, const DualQuaternion& num )
{
    return !Nearest( num, denom ) * num;
}

/// DualQuaternion Nearest Right Division: num / denom = num * !denom - negates denom if necessary
NOALIAS __forceinline const DualQuaternion NearestRightDivide( const DualQuaternion& num, const DualQuaternion& denom )
{
    return num * !Nearest( num, denom );
}


/// DualQuaternion Inverse (faster but only 22 bit precision)
/// NOTE: The inverse of a unit dual quaternion is just the conjugate!
NOALIAS __forceinline const DualQuaternion Inverse( const DualQuaternion& a )
{
    return Conjugate( a ) / NormSquared( a );
}

/// DualQuaternion Inverse (slower but full 24 bit precision)
/// NOTE: The inverse of a unit dual quaternion is just the conjugate!
NOALIAS __forceinline const DualQuaternion Inverse24( const DualQuaternion& a )
{
    return Div24( Conjugate( a ), NormSquared( a ) );
}

/// DualQuaternion Normalise (faster but only 22 bit precision)
NOALIAS __forceinline const DualQuaternion Normalise( const DualQuaternion& a )
{
    return a / Norm( a );
}

/// DualQuaternion Normalise (slower but full 24 bit precision)
NOALIAS __forceinline const DualQuaternion Normalise24( const DualQuaternion& a )
{
    return Div24( a, Norm( a ) );
}


/// Dual Quaternion Logarithm (faster but only 22 bit precision)
NOALIAS __forceinline const DualQuaternion Log( const DualQuaternion& q )
{
    const VF32 zero( Const<VF32>::Zero() );
    
    const DualScalar cosHalfTheta( MakeDualScalar( W( GetQuatVec4( q ) ),
                                                   W( GetDualVec4( q ) ) ) );
    const DualScalar halfTheta( ArcCos( cosHalfTheta ) );
    const DualScalar sinHalfTheta( Sin( halfTheta ) );
    
    DualQuaternion s( q );
    SetW( &s.m_V.m_V0, zero );
    SetW( &s.m_V.m_V1, zero );
    //s = s / sinHalfTheta;
    //return s * halfTheta;
    return s * ( halfTheta / sinHalfTheta );
}

/// Dual Quaternion Logarithm (slower but full 24 bit precision)
NOALIAS __forceinline const DualQuaternion Log24( const DualQuaternion& q )
{
    const VF32 zero( Const<VF32>::Zero() );
    
    const DualScalar cosHalfTheta( MakeDualScalar( W( GetQuatVec4( q ) ),
                                                   W( GetDualVec4( q ) ) ) );
    const DualScalar halfTheta( ArcCos( cosHalfTheta ) );
    const DualScalar sinHalfTheta( Sin( halfTheta ) );
    
    DualQuaternion s( q );
    SetW( &s.m_V.m_V0, zero );
    SetW( &s.m_V.m_V1, zero );
    //s = Div24( s, sinHalfTheta );
    //return s * halfTheta;
    return s * Div24( halfTheta, sinHalfTheta );
}

/// Dual Quaternion Exponential (faster but only 22 bit precision)
/// p must have zero scalar part and non-zero non-dual part [ i.e. result from Log() ]
NOALIAS __forceinline const DualQuaternion Exp( const DualQuaternion& p )
{
    const DualScalar     halfTheta( Norm( p ) );
    const DualQuaternion s( p / halfTheta );
    
    DualScalar sinHalfTheta, cosHalfTheta;
    SinCos( halfTheta, &sinHalfTheta, &cosHalfTheta );
    
    DualQuaternion result( s * sinHalfTheta );
    SetW( &result.m_V.m_V0, Real( cosHalfTheta ) );
    SetW( &result.m_V.m_V1, Dual( cosHalfTheta ) );
    return result;
}

/// Dual Quaternion Exponential (slower but full 24 bit precision)
/// p must have zero scalar part and non-zero non-dual part [ i.e. result from Log() ]
NOALIAS __forceinline const DualQuaternion Exp24( const DualQuaternion& p )
{
    const DualScalar     halfTheta( Norm( p ) );
    const DualQuaternion s( Div24( p, halfTheta ) );
    
    DualScalar sinHalfTheta, cosHalfTheta;
    SinCos( halfTheta, &sinHalfTheta, &cosHalfTheta );
    
    DualQuaternion result( s * sinHalfTheta );
    SetW( &result.m_V.m_V0, Real( cosHalfTheta ) );
    SetW( &result.m_V.m_V1, Dual( cosHalfTheta ) );
    return result;
}

/// Dual Quaternion Power: q^u = exp( u * log(q) ) (faster but only 22 bit precision)
NOALIAS __forceinline const DualQuaternion Pow( const DualQuaternion& q, const VF32 VECREF u )
{
    ///
    /// q = cos( theta / 2 ) + s * sin( theta / 2 )
    /// q: unit dual quaternion
    /// s: unit dual quaternion with zero scalar part
    /// theta: dual scalar
    ///
    
    const VF32 zero( Const<VF32>::Zero() );
    
    const DualScalar cosHalfTheta( MakeDualScalar( W( GetQuatVec4( q ) ),
                                                   W( GetDualVec4( q ) ) ) );
    const DualScalar halfTheta( ArcCos( cosHalfTheta ) );
    const DualScalar sinHalfTheta( Sin( halfTheta ) );
    
    DualQuaternion s( q );
    SetW( &s.m_V.m_V0, zero );
    SetW( &s.m_V.m_V1, zero );
    
    DualScalar sinHalfThetaU, cosHalfThetaU;
    SinCos( halfTheta * u, &sinHalfThetaU, &cosHalfThetaU );
    
    DualQuaternion qu( s * ( sinHalfThetaU / sinHalfTheta ) );
    SetW( &qu.m_V.m_V0, Real( cosHalfThetaU ) );
    SetW( &qu.m_V.m_V1, Dual( cosHalfThetaU ) );
    return qu;
}

/// Dual Quaternion Power: q^u = exp( u * log(q) ) (slower but full 24 bit precision)
NOALIAS __forceinline const DualQuaternion Pow24( const DualQuaternion& q, const VF32 VECREF u )
{
    const VF32 zero( Const<VF32>::Zero() );
    
    const DualScalar cosHalfTheta( MakeDualScalar( W( GetQuatVec4( q ) ),
                                                   W( GetDualVec4( q ) ) ) );
    const DualScalar halfTheta( ArcCos( cosHalfTheta ) );
    const DualScalar sinHalfTheta( Sin( halfTheta ) );
    
    DualQuaternion s( q );
    SetW( &s.m_V.m_V0, zero );
    SetW( &s.m_V.m_V1, zero );
    
    DualScalar sinHalfThetaU, cosHalfThetaU;
    SinCos( halfTheta * u, &sinHalfThetaU, &cosHalfThetaU );
    
    DualQuaternion qu( s * Div24( sinHalfThetaU, sinHalfTheta ) );
    SetW( &qu.m_V.m_V0, Real( cosHalfThetaU ) );
    SetW( &qu.m_V.m_V1, Dual( cosHalfThetaU ) );
    return qu;
}



/// Dual Quaternion Linear Blend (DLB)
NOALIAS __forceinline const DualQuaternion Lerp( const DualQuaternion& src, const DualQuaternion& dst, const VF32 VECREF t )
{
    const VF32 one( Const<VF32>::One() );
    return Normalise( ( src * ( one - t ) ) + ( dst * t ) );
}

/// Dual Quaternion Nearest Linear Blend (Nearest DLB)
NOALIAS __forceinline const DualQuaternion NearestLerp( const DualQuaternion& src, const DualQuaternion& dst, const VF32 VECREF t )
{
    const VF32 one( Const<VF32>::One() );
    return Normalise( ( src * ( one - t ) ) + ( Nearest( src, dst ) * t ) );
}


/// Dual Quaternion Linear Blend (DLB)
NOALIAS __forceinline const DualQuaternion DLB(
    const DualQuaternion& dq1,
    const DualQuaternion& dq2,
    const DualQuaternion& dq3,
    const VF32 VECREF w1,
    const VF32 VECREF w2,
    const VF32 VECREF w3 )
{
    return Normalise( ( dq1 * w1 ) + ( dq2 * w2 ) + ( dq3 * w3 ) );
}

/// Dual Quaternion Linear Blend (DLB)
NOALIAS __forceinline const DualQuaternion DLB(
    const DualQuaternion& dq1,
    const DualQuaternion& dq2,
    const DualQuaternion& dq3,
    const DualQuaternion& dq4,
    const VF32 VECREF w1,
    const VF32 VECREF w2,
    const VF32 VECREF w3,
    const VF32 VECREF w4 )
{
    return Normalise( ( ( dq1 * w1 ) + ( dq2 * w2 ) ) + ( ( dq3 * w3 ) + ( dq4 * w4 ) ) );
}

/// Dual Quaternion Linear Blend (DLB)
NOALIAS __forceinline const DualQuaternion DLB(
    DualQuaternion const * const __restrict pQ, ///< Array of dual quaternions to blend
    VF32           const * const __restrict pW, ///< Array of weights for each dual quaternion
    const U32                               n ) ///< Number of elements in arrays
{
    DualQuaternion result = pQ[0] * pW[0];
    for ( U32 i=1; i<n; i++ )
    {
        result += pQ[i] * pW[i];
    }
    return Normalise( result );
}


/// DualQuaternion Slerp: Screw Linear Interpolation
NOALIAS __forceinline const DualQuaternion Slerp( const DualQuaternion& src, const DualQuaternion& dst, const VF32 VECREF t )
{
    return src * Pow( !src * dst, t );
}

/// DualQuaternion Nearest Slerp: Screw Linear Interpolation - Negates dst if necessary
NOALIAS __forceinline const DualQuaternion NearestSlerp( const DualQuaternion& src, const DualQuaternion& dst, const VF32 VECREF t )
{
    return Slerp( src, Nearest( src, dst ), t );
}


/// Dual Quaternion Iterative Blending (DIB)
NOALIAS __forceinline const DualQuaternion DIB(
    DualQuaternion const * const __restrict pQ, ///< Array of dual quaternions to blend
    VF32           const * const __restrict pW, ///< Array of weights for each dual quaternion
    const U32                               n,  ///< Number of elements in arrays
    const VF32 VECREF                        p ) ///< Required precision
{
    DualQuaternion b = DLB( pQ, pW, n );
    DualQuaternion x;
    do
    {
        const DualQuaternion b_conjugate( !b );
        x = Log( b_conjugate * pQ[0] ) * pW[0];
        for ( U32 i=1; i<n; i++ )
        {
            x += Log( b_conjugate * pQ[i] ) * pW[i];
        }
        b = b * Exp( x );
        
    } while ( Real( Norm( x ) ) > p );
    
    return b;
}


/// Make Mat43 from DualQuaternion
NOALIAS __forceinline const Mat43 MakeMat43( const DualQuaternion& dq )
{
    return MakeMat43( MakeMat33( GetQuaternion( dq ) ), GetTranslation( dq ) );
}


/// DualQuaternion Serialise to Stream
template < typename T_Stream >
void Serialise( const DualQuaternion& src, T_Stream& stream )
{
    Serialise( GetQuaternion( src ), stream );
    Serialise( GetTranslation( src ), stream );
}

/// DualQuaternion Deserialise from Stream
template < typename T_Stream >
void Deserialise( T_Stream& stream, DualQuaternion* const __restrict pDst )
{
    Quaternion q;
    Vector3    t;
    Deserialise( stream, &q );
    Deserialise( stream, &t );
    return MakeDualQuaternion( q, t );
}







/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

