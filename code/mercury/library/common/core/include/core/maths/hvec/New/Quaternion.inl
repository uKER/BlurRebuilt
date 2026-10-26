#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_QUATERNION_INL
#define SOFT_MATHS_HVEC_NEW_QUATERNION_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_QUATERNION_INL
#define MATHS_HVEC_NEW_QUATERNION_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Quaternion
/// @{

/// Load Quaternion from an F32 array
template <>
NOALIAS __forceinline const Quaternion Load( F32 const * const __restrict pSrc )
{
    Quaternion result;
    result.m_Q = Load<Vec4F32>( pSrc );
    return result;
}

/// Store Quaternion into an F32 array
NOALIAS __forceinline void Store( const Quaternion PCREF src, F32* const __restrict pDst )
{
    Store( src.m_Q, pDst );
}


/// Quaternion Get Vec4F32 components: imaginary vector part in result.XYZ, real scalar part in result.W
NOALIAS __forceinline const Vec4F32 GetVec4F32( const Quaternion PCREF q )
{
    return q.m_Q;
}

/// Quaternion Get Vector3 components: imaginary vector part in result.XYZ
NOALIAS __forceinline const Vector3 GetVector3( const Quaternion PCREF q )
{
    return MakeVector3( q.m_Q );
}

/// Quaternion Get Vector4 components: imaginary vector part in result.XYZ, real scalar part in result.W
NOALIAS __forceinline const Vector4 GetVector4( const Quaternion PCREF q )
{
    return MakeVector4( q.m_Q );
}

/// Make generic Quaternion from a Vec4F32
NOALIAS __forceinline const Quaternion MakeQuaternion( const Vec4F32 VECREF v )
{
    Quaternion result;
    result.m_Q = v;
    return result;
}

/// Make generic Quaternion from a Vector3 : the real W component is set to zero
NOALIAS __forceinline const Quaternion MakeQuaternion( const Vector3 PCREF v )
{
    /// @par NOTE: This does NOT normalise the resulting quaternion!
    Quaternion result;
    result.m_Q = v.m_V;
    SetW( &result.m_Q, Const<VF32>::Zero() );
    return result;
}

/// Make generic Quaternion from a Point3 : the real W component is set to zero
NOALIAS __forceinline const Quaternion MakeQuaternion( const Point3 PCREF p )
{
    /// @par NOTE: This does NOT normalise the resulting quaternion!
    Quaternion result;
    result.m_Q = p.m_V;
    SetW( &result.m_Q, Const<VF32>::Zero() );
    return result;
}

/// Make generic Quaternion from a Normal3 : the real W component is set to zero
NOALIAS __forceinline const Quaternion MakeQuaternion( const Normal3 PCREF n )
{
    /// @par NOTE: This does NOT normalise the resulting quaternion!
    Quaternion result;
    result.m_Q = n.m_V;
    SetW( &result.m_Q, Const<VF32>::Zero() );
    return result;
}

/// Make generic Quaternion from a Vector4
NOALIAS __forceinline const Quaternion MakeQuaternion( const Vector4 PCREF v )
{
    Quaternion result;
    result.m_Q = v.m_V;
    return result;
}

/// Make generic Quaternion from a set of 4 VF32 components
NOALIAS __forceinline const Quaternion MakeQuaternion( const VF32 VECREF imag_x, const VF32 VECREF imag_y, const VF32 VECREF imag_z, const VF32 VECREF real_w )
{
    Quaternion result;
    result.m_Q = MakeVec4F32( imag_x, imag_y, imag_z, real_w );
    return result;
}

/// Make unit rotation Quaternion from a Normal3 axis and VF32 rotation angle (radians)
NOALIAS __forceinline const Quaternion MakeUnitQuaternion( const Normal3 PCREF axis, const VF32 VECREF theta )
{
    const VF32 halfTheta( theta * Const<VF32>::Half() );
    VF32 sinHalfTheta, cosHalfTheta;
    SinCos( halfTheta, &sinHalfTheta, &cosHalfTheta );
    const Vector3 imag( axis * sinHalfTheta );
    Quaternion result;
    result.m_Q = MakeVector4( imag, cosHalfTheta ).m_V;
    return result;
}

/// Make unit rotation Quaternion from a Vector3 axis and VF32 rotation angle (radians)
NOALIAS __forceinline const Quaternion MakeUnitQuaternion( const Vector3 PCREF axis, const VF32 VECREF theta )
{
    return MakeUnitQuaternion( Normalise( axis ), theta );
}


/// Get Quaternion Imaginary part as Vector3
NOALIAS __forceinline const Vector3 Imag( const Quaternion PCREF q )
{
    return MakeVector3( q.m_Q );
}

/// Get Quaternion Real part as VF32 == Cos( theta / 2 ), where theta is the rotation angle
NOALIAS __forceinline const VF32 Real( const Quaternion PCREF q )
{
    return W( q.m_Q );
}

/// Quaternion Length
NOALIAS __forceinline const VF32 Length( const Quaternion PCREF q )
{
    return Length4( q.m_Q );
}

/// Quaternion Length Squared
NOALIAS __forceinline const VF32 LengthSquared( const Quaternion PCREF q )
{
    return LengthSquared4( q.m_Q );
}

/// Quaternion Rotation Axis (normalised)
NOALIAS __forceinline const Normal3 RotationAxis( const Quaternion PCREF q )
{
    /// NOTE: for a 360 degree (2*PI) rotation, the quaternion imaginary part is zero,
    /// because 360 degree rotations about any axes are equivalent. In this case,
    /// this function returns a normalised Y axis.
    const Vector3 imag_q( Imag( q ) );
    const Normal3 axis_q( Normalise( imag_q ) );
    const Normal3 axis_y( Const<Normal3>::AxisY() );
    const VF32    epsilon( Const<VF32>::Epsilon() );
    const VF32    length_squared_axis_q( LengthSquared( imag_q ) );
    return Select( length_squared_axis_q > epsilon, axis_q, axis_y );
}

/// Quaternion Rotation Angle
NOALIAS __forceinline const VF32 RotationAngle( const Quaternion PCREF q )
{
    return ArcCos( Real( q ) ) * Const<VF32>::Two();
}

NOALIAS __forceinline const Vector3 RotationVector( const Quaternion PCREF q )
{
	const Maths::VF32 realThreshold(Maths::Const<Maths::VF32>::PointNineNine());
	if(Real( q ) > realThreshold)
	{
		return -Imag(q) * Maths::Const<VF32>::Two();
	}
	else
	{
		const VF32 rotAngle(RotationAngle(q));
		return RotationAxis(q) * -rotAngle;
	}
}

/// Quaternion Select
NOALIAS __forceinline const Quaternion Select( const VBool PCREF condition, const Quaternion PCREF result_if_true, const Quaternion PCREF result_if_false )
{
    Quaternion result;
    result.m_Q = Select( condition, result_if_true.m_Q, result_if_false.m_Q );
    return result;
}

/// Quaternion Negation Operator
NOALIAS __forceinline const Quaternion operator - ( const Quaternion PCREF q )
{
    Quaternion result;
    result.m_Q = -q.m_Q;
    return result;
}

/// Quaternion Addition Operator
NOALIAS __forceinline const Quaternion operator + ( const Quaternion PCREF a, const Quaternion PCREF b )
{
    Quaternion result;
    result.m_Q = a.m_Q + b.m_Q;
    return result;
}

/// Quaternion Subtraction Operator
NOALIAS __forceinline const Quaternion operator - ( const Quaternion PCREF a, const Quaternion PCREF b )
{
    Quaternion result;
    result.m_Q = a.m_Q - b.m_Q;
    return result;
}

/// Quaternion Dot Product: Returns 4 element dot product == cosine( angle / 2 ) between the 2 quaternions!
NOALIAS __forceinline const VF32 operator | ( const Quaternion PCREF a, const Quaternion PCREF b )
{
    return Dot4( a.m_Q, b.m_Q );
}

/// Quaternion Scalar Multiplication Operator
NOALIAS __forceinline const Quaternion operator * ( const Quaternion PCREF a, const VF32 VECREF b )
{
    Quaternion result;
    result.m_Q = a.m_Q * b;
    return result;
}

/// Quaternion Scalar Multiplication Operator
NOALIAS __forceinline const Quaternion operator * ( const VF32 VECREF a, const Quaternion PCREF b )
{
    Quaternion result;
    result.m_Q = a * b.m_Q;
    return result;
}

/// Quaternion Scalar Division Operator
NOALIAS __forceinline const Quaternion operator / ( const Quaternion PCREF a, const VF32 VECREF b )
{
    Quaternion result;
    result.m_Q = a.m_Q / b;
    return result;
}

/// Quaternion Multiplication Operator
NOALIAS __forceinline const Quaternion operator * ( const Quaternion PCREF a, const Quaternion PCREF b )
{
    #if 0
    
    const Vector3 v0( Imag( a ) );
    const Vector3 v1( Imag( b ) );
    const VF32    s0( Real( a ) );
    const VF32    s1( Real( b ) );
    
    // Scalar * Vector components s0*v1 + s1*v0 +
    // Vector cross product component v0^v1
    //const Vector3 imag( ( v0 * s1 ) + ( v1 * s0 ) + ( v1 ^ v0 ) );
    const Vector3 imag( MulAdd( v0, s1, v1 * s0 ) + ( v1 ^ v0 ) );
    
    // Scalar multiplication component s0*s1 -
    // Vector dot product component v0|v1
    //const VF32 real( ( s0 * s1 ) - ( v0 | v1 ) );
    const VF32 real( MulSub( s0, s1, v0 | v1 ) );
    
    // Combine real & imaginary parts
    Quaternion result1;
    result1.m_Q = imag.m_V;
    SetW( &result1.m_Q, real );
    return result1;
    
    #else
    
    // Cross Product
    // c.xyz = a.yzx * b.zxy - a.zxy * b.yzx
    // c.ijk = a.jki * b.kij - a.kij * b.jki
    // c.i = a.j * b.k - a.k * b.j
    // c.j = a.k * b.i - a.i * b.k
    // c.k = a.i * b.j - a.j * b.i
    
    /// @code
    /// Quaternion Multiplication Table
    /// c=a*b | b.i | b.j | b.k | b.1 |
    /// ------+-----+-----+-----+-----+
    /// a.i   | -1  | -k  | j   | i   |
    /// ------+-----+-----+-----+-----+
    /// a.j   | k   | -1  | -i  | j   |
    /// ------+-----+-----+-----+-----+
    /// a.k   | -j  | i   | -1  | k   |
    /// ------+-----+-----+-----+-----+
    /// a.1   | i   | j   | k   | 1   |
    /// ------+-----+-----+-----+-----+
    /// @endcode
    
    /// c.i = + ( a.i * b.1 ) - ( a.j * b.k ) + ( a.k * b.j ) + ( a.1 * b.i )
    /// c.j = + ( a.i * b.k ) + ( a.j * b.1 ) - ( a.k * b.i ) + ( a.1 * b.j )
    /// c.k = - ( a.i * b.j ) + ( a.j * b.i ) + ( a.k * b.1 ) + ( a.1 * b.k )
    /// c.1 = - ( a.i * b.i ) - ( a.j * b.j ) - ( a.k * b.k ) + ( a.1 * b.1 )
    
    static const int _i = 0, _j = 1, _k = 2, _1 = 3;
    
    Quaternion result2;
    const VF32 ai( X( a.m_Q ) );
    const VF32 aj( Y( a.m_Q ) );
    const VF32 ak( Z( a.m_Q ) );
    const VF32 a1( W( a.m_Q ) );
    //result2.m_Q = ( Negate<0,0,1,1>( ai * Permute<_1,_k,_j,_i>( b.m_Q ) ) +
    //                Negate<1,0,0,1>( aj * Permute<_k,_1,_i,_j>( b.m_Q ) ) ) +
    //              ( Negate<0,1,0,1>( ak * Permute<_j,_i,_1,_k>( b.m_Q ) ) +
    //                                 a1 * b.m_Q );
    result2.m_Q = MulAdd( ai,  Negate<0,0,1,1>( Permute<_1,_k,_j,_i>( b.m_Q ) ),
                          aj * Negate<1,0,0,1>( Permute<_k,_1,_i,_j>( b.m_Q ) ) ) +
                  MulAdd( ak,  Negate<0,1,0,1>( Permute<_j,_i,_1,_k>( b.m_Q ) ),
                          a1 * b.m_Q );
    return result2;
    
    #endif
}

/// Quaternion Conjugate Operator ( Multiplicative Inverse of Unit Quaternion )
NOALIAS __forceinline const Quaternion operator ! ( const Quaternion PCREF q )
{
    Quaternion result;
    result.m_Q = NegateXYZ( q.m_Q );
    return result;
}

/// Quaternion Right Division: num / denom = num * !denom
NOALIAS __forceinline const Quaternion operator / ( const Quaternion PCREF num, const Quaternion PCREF denom )
{
    return num * !denom;
}

/// Quaternion Vector3 Multiplication Operator: Vector3 * Quaternion --> Vector3
NOALIAS __forceinline const Vector3 operator * ( const Vector3 PCREF v, const Quaternion PCREF q )
{
    #if 0
    return Imag( !q * MakeQuaternion( v ) * q );
    #else
    // Optimised: some of the intermediate calculations are zero and have been eliminated!
    const Vector3 v0( Imag( q ) );
    const VF32    s0( Real( q ) );
    const Vector3 v01( MulAdd( v, s0, v0 ^ v ) );
    const Vector3 v012( MulAdd( v0, v0 | v, v01 * s0 ) + ( v0 ^ v01 ) );
    Vector3 result;
    result.m_V = v012.m_V;
    return result;
    #endif
}

/// Quaternion Point3 Multiplication Operator: Point3 * Quaternion --> Point3
NOALIAS __forceinline const Point3 operator * ( const Point3 PCREF p, const Quaternion PCREF q )
{
    Point3 result;
    #if 0
    result.m_V = Imag( !q * MakeQuaternion( p ) * q ).m_V;
    #else
    // Optimised: some of the intermediate calculations are zero and have been eliminated!
    const Vector3 v( MakeVector3( p ) );
    const Vector3 v0( Imag( q ) );
    const VF32    s0( Real( q ) );
    const Vector3 v01( MulAdd( v, s0, v0 ^ v ) );
    const Vector3 v012( MulAdd( v0, v0 | v, v01 * s0 ) + ( v0 ^ v01 ) );
    result.m_V = v012.m_V;
    #endif
    return result;
}

/// Quaternion Normal3 Multiplication Operator: Normal3 * Quaternion --> Normal3
NOALIAS __forceinline const Normal3 operator * ( const Normal3 PCREF n, const Quaternion PCREF q )
{
    Normal3 result;
    #if 0
    result.m_V = Imag( !q * MakeQuaternion( n ) * q ).m_V;
    #else
    // Optimised: some of the intermediate calculations are zero and have been eliminated!
    const Vector3 v( MakeVector3( n ) );
    const Vector3 v0( Imag( q ) );
    const VF32    s0( Real( q ) );
    //const Vector3 v01( ( v * s0 ) - ( v ^ v0 ) );
    //const Vector3 v01( ( v * s0 ) + ( v0 ^ v ) );
    const Vector3 v01( MulAdd( v, s0, v0 ^ v ) );
    //const Vector3 v012( ( v01 * s0 ) + ( v0 * ( v0 | v ) ) + ( v0 ^ v01 ) );
    const Vector3 v012( MulAdd( v0, v0 | v, v01 * s0 ) + ( v0 ^ v01 ) );
    result.m_V = v012.m_V;
    #endif
    return result;
}

/// Quaternion Normalise: Returns normalised copy of this quaternion
NOALIAS __forceinline const Quaternion Normalise( const Quaternion PCREF q )
{
    Quaternion result;
    result.m_Q = Normalise4( q.m_Q );
    return result;
}

/// Quaternion Normalise: Normalise this quaternion
NOALIAS __forceinline void Normalise( Quaternion* const __restrict pQ )
{
    Normalise4( &pQ->m_Q );
}

/// Quaternion Nearest: Returns whichever one of the two equivalent destinations is nearest to the source
NOALIAS __forceinline const Quaternion Nearest( const Quaternion PCREF src, const Quaternion PCREF dst )
{
    /// @par NOTE: This function returns the nearest destination rather than source so that it's result and
    /// @par parameters are consistent with the NearestLerp/NearestSlerp functions!
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 cosHalfTheta( src | dst );
    return Select( cosHalfTheta >= zero, dst, -dst );
}


/// Quaternion MulAdd: a * b + c
NOALIAS __forceinline const Quaternion MulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    Quaternion result;
    result.m_Q = MulAdd( a.m_Q, b, c.m_Q );
    return result;
}

/// Quaternion MulSub: a * b - c
NOALIAS __forceinline const Quaternion MulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    Quaternion result;
    result.m_Q = MulSub( a.m_Q, b, c.m_Q );
    return result;
}

/// Quaternion NegMulAdd: -( a * b + c )
NOALIAS __forceinline const Quaternion NegMulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    Quaternion result;
    result.m_Q = NegMulAdd( a.m_Q, b, c.m_Q );
    return result;
}

/// Quaternion NegMulSub: -( a * b - c ) = c - a * b
NOALIAS __forceinline const Quaternion NegMulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    Quaternion result;
    result.m_Q = NegMulSub( a.m_Q, b, c.m_Q );
    return result;
}


/// Quaternion Nearest MulAdd: Nearest(c,a) * b + c
NOALIAS __forceinline const Quaternion NearMulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    return MulAdd( Nearest(c,a), b, c );
}

/// Quaternion Nearest MulSub: Nearest(c,a) * b - c
NOALIAS __forceinline const Quaternion NearMulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    return MulSub( Nearest(c,a), b, c );
}

/// Quaternion Nearest NegMulAdd: -( Nearest(c,a) * b + c )
NOALIAS __forceinline const Quaternion NearNegMulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    return NegMulAdd( Nearest(c,a), b, c );
}

/// Quaternion Nearest NegMulSub: -( Nearest(c,a) * b - c ) = c - Nearest(c,a) * b
NOALIAS __forceinline const Quaternion NearNegMulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    return NegMulSub( Nearest(c,a), b, c );
}


/// Quaternion Normalised MulAdd: Normalise( a * b + c )
NOALIAS __forceinline const Quaternion NormMulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    return Normalise( MulAdd( a, b, c ) );
}

/// Quaternion Normalised MulSub: Normalise( a * b - c )
NOALIAS __forceinline const Quaternion NormMulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    return Normalise( MulSub( a, b, c ) );
}

/// Quaternion Normalised NegMulAdd: Normalise( -( a * b + c ) )
NOALIAS __forceinline const Quaternion NormNegMulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    return Normalise( NegMulAdd( a, b, c ) );
}

/// Quaternion Normalised NegMulSub: Normalise( -( a * b - c ) ) = Normalise( c - a * b )
NOALIAS __forceinline const Quaternion NormNegMulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    return Normalise( NegMulSub( a, b, c ) );
}


/// Quaternion Normalised Nearest MulAdd: Normalise( Nearest(c,a) * b + c )
NOALIAS __forceinline const Quaternion NormNearMulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    return Normalise( MulAdd( Nearest(c,a), b, c ) );
}

/// Quaternion Normalised Nearest MulSub: Normalise( Nearest(c,a) * b - c )
NOALIAS __forceinline const Quaternion NormNearMulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    return Normalise( MulSub( Nearest(c,a), b, c ) );
}

/// Quaternion Normalised Nearest NegMulAdd: Normalise( -( Nearest(c,a) * b + c ) )
NOALIAS __forceinline const Quaternion NormNearNegMulAdd( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    return Normalise( NegMulAdd( Nearest(c,a), b, c ) );
}

/// Quaternion Normalised Nearest NegMulSub: Normalise( -( Nearest(c,a) * b - c ) ) = Normalise( c - Nearest(c,a) * b )
NOALIAS __forceinline const Quaternion NormNearNegMulSub( const Quaternion PCREF a, const VF32 VECREF b, const Quaternion PCREF c )
{
    return Normalise( NegMulSub( Nearest(c,a), b, c ) );
}



/// Quaternion Right Division: num / denom = num * !denom  - negates denom if necessary
NOALIAS __forceinline const Quaternion NearestDivide( const Quaternion PCREF num, const Quaternion PCREF denom )
{
    return num * !Nearest( num, denom );
}

/// Quaternion Difference ( Left Division )
NOALIAS __forceinline const Quaternion Difference( const Quaternion PCREF src, const Quaternion PCREF dst )
{
    return !src * dst;
}

/// Quaternion Nearest Difference ( Left Division ) : returns difference quaternion between src and the nearest representation of dst to src
NOALIAS __forceinline const Quaternion NearestDifference( const Quaternion PCREF src, const Quaternion PCREF dst )
{
    return !src * Nearest( src, dst );
}

/// Quaternion Differential ( Differential of the left division of dst by src )
NOALIAS __forceinline const Vector3 Differential( const Quaternion PCREF src, const Quaternion PCREF dst )
{
    return Imag( !src * Nearest( src, dst ) ) * Const<VF32>::Two();
}


/// Quaternion Left Division: denom \ num = !denom * num
NOALIAS __forceinline const Quaternion LeftDivide( const Quaternion PCREF denom, const Quaternion PCREF num )
{
    return !denom * num;
}

/// Quaternion Right Division: num / denom = num * !denom
NOALIAS __forceinline const Quaternion RightDivide( const Quaternion PCREF num, const Quaternion PCREF denom )
{
    return num * !denom;
}

/// Quaternion Nearest Left Division: denom \ num = !denom * num  - negates denom if necessary
NOALIAS __forceinline const Quaternion NearestLeftDivide( const Quaternion PCREF denom, const Quaternion PCREF num )
{
    return !Nearest( num, denom ) * num;
}

/// Quaternion Nearest Right Division: num / denom = num * !denom  - negates denom if necessary
NOALIAS __forceinline const Quaternion NearestRightDivide( const Quaternion PCREF num, const Quaternion PCREF denom )
{
    return num * !Nearest( num, denom );
}


/// Quaternion Left Differential ( Differential of the left division of num by denom )
NOALIAS __forceinline const Vector3 LeftDifferential( const Quaternion PCREF denom, const Quaternion PCREF num )
{
    return Imag( !denom * num ) * Const<VF32>::Two();
}

/// Quaternion Right Differential ( Differential of the right division of num by denom )
NOALIAS __forceinline const Vector3 RightDifferential( const Quaternion PCREF num, const Quaternion PCREF denom )
{
    return Imag( num * !denom ) * Const<VF32>::Two();
}

/// Quaternion Nearest Left Differential ( Differential of the left division of num by denom )
NOALIAS __forceinline const Vector3 NearestLeftDifferential( const Quaternion PCREF denom, const Quaternion PCREF num )
{
    return Imag( !Nearest( num, denom ) * num ) * Const<VF32>::Two();
}

/// Quaternion Nearest Right Differential ( Differential of the right division of num by denom )
NOALIAS __forceinline const Vector3 NearestRightDifferential( const Quaternion PCREF num, const Quaternion PCREF denom )
{
    return Imag( num * !Nearest( num, denom ) ) * Const<VF32>::Two();
}



/// Quaternion Lerp: Linear Interpolation
NOALIAS __forceinline const Quaternion Lerp( const Quaternion PCREF src, const Quaternion PCREF dst, const VF32 VECREF t )
{
    return src + ( ( dst - src ) * t );
}

/// Quaternion Lerp: Linear Interpolation with normalisation
NOALIAS __forceinline const Quaternion LerpNormalised( const Quaternion PCREF src, const Quaternion PCREF dst, const VF32 VECREF t )
{
    return Normalise( src + ( ( dst - src ) * t ) );
}

/// Quaternion Nearest Lerp: Nearest Linear Interpolation - Negates dst if necessary
NOALIAS __forceinline const Quaternion NearestLerp( const Quaternion PCREF src, const Quaternion PCREF dst, const VF32 VECREF t )
{
    return src + ( ( Nearest( src, dst ) - src ) * t );
}

/// Quaternion Nearest Lerp: Nearest Linear Interpolation - Negates dst if necessary - with normalisation
NOALIAS __forceinline const Quaternion NearestLerpNormalised( const Quaternion PCREF src, const Quaternion PCREF dst, const VF32 VECREF t )
{
    return Normalise( src + ( ( Nearest( src, dst ) - src ) * t ) );
}

//static const VF32 g_QuaternionSlerpThreshold( MakeVF32( 1.0f - 0.0005f ) );

/// Quaternion Slerp: Spherical Linear Interpolation
NOALIAS __forceinline const Quaternion Slerp( const Quaternion PCREF src, const Quaternion PCREF dst, const VF32 VECREF t )
{
    #if 0
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 half( Const<VF32>::Half() );
    const VF32 one( Const<VF32>::One() );
    const VF32 minusOne( -one );
    const VF32 cosHalfTheta( src | dst );
    const VF32 oneMinusT( one - t );
    
    // From Fundamental Identity: sin^2(x) + cos^2(x) = 1
    const VF32 reciprocalSinHalfTheta( RcpSqrt( one - ( cosHalfTheta * cosHalfTheta ) ) );
    
    const VF32 halfTheta( ArcCos( Clamped( cosHalfTheta, minusOne, one ) ) );
    const Vec4F32 angles( MakeVec4F32( oneMinusT * halfTheta, t * halfTheta, zero, zero ) );
    const Vec4F32 sinAngles( Sin( angles ) );
    const VF32 scale0( X( sinAngles ) );
    const VF32 scale1( Y( sinAngles ) );
    
    const Quaternion q_delta( dst - src );
    const Quaternion result_slerp( ( ( src * scale0 ) + ( dst * scale1 ) ) * reciprocalSinHalfTheta );
    const Quaternion result_nlerp( Normalise( src + ( q_delta * t ) ) );
    
    //return Select( one - cosHalfTheta >= g_QuaternionSlerpThreshold, result_slerp, result_nlerp );
    //return Select( one - g_QuaternionSlerpThreshold >= cosHalfTheta, result_slerp, result_nlerp );
    return Select( cosHalfTheta <= one - g_QuaternionSlerpThreshold,
                   result_slerp,
                   result_nlerp );
    
    #else
    
    const Quaternion delta( !src * dst );
    const Normal3    axis( RotationAxis( delta ) );
    const VF32       halfTheta( ArcCos( Real( delta ) ) * t );
    VF32 sinHalfTheta, cosHalfTheta;
    SinCos( halfTheta, &sinHalfTheta, &cosHalfTheta );
    return src * MakeQuaternion( MakeVector4( axis * sinHalfTheta, cosHalfTheta ) );
    
    #endif
}

/// Quaternion Nearest Slerp: Spherical Linear Interpolation - Negates dst if necessary
NOALIAS __forceinline const Quaternion NearestSlerp( const Quaternion PCREF src, const Quaternion PCREF dst, const VF32 VECREF t )
{
    return Slerp( src, Nearest( src, dst ), t );
}



/// Quaternion get matrix row X
NOALIAS __forceinline const Vector3 MatrixRowX( const Quaternion PCREF q )
{
    const Vector3 unit_x( Const<Vector3>::UnitX() );
    const Vec4F32 xyza( q.m_Q );
    const Vec4F32 _2xyza( xyza + xyza );
    const Vec4F32 _2y_xyza( Y(xyza) * _2xyza );
    const Vec4F32 _2z_xyza( Z(xyza) * _2xyza );
    const VF32    _2xy( X( _2y_xyza ) );
    const VF32    _2xz( X( _2z_xyza ) );
    const VF32    _2yy( Y( _2y_xyza ) );
    const VF32    _2zz( Z( _2z_xyza ) );
    const VF32    _2ay( W( _2y_xyza ) );
    const VF32    _2az( W( _2z_xyza ) );
    return unit_x + NegateX( MakeVector3( _2yy, _2xy, _2xz ) ) + NegateXZ( MakeVector3( _2zz, _2az, _2ay ) );
}

/// Quaternion get matrix row Y
NOALIAS __forceinline const Vector3 MatrixRowY( const Quaternion PCREF q )
{
    const Vector3 unit_y( Const<Vector3>::UnitY() );
    const Vec4F32 xyza( q.m_Q );
    const Vec4F32 _2xyza( xyza + xyza );
    const Vec4F32 _2x_xyza( X(xyza) * _2xyza );
    const Vec4F32 _2z_xyza( Z(xyza) * _2xyza );
    const VF32    _2xx( X( _2x_xyza ) );
    const VF32    _2xy( Y( _2x_xyza ) );
    const VF32    _2yz( Y( _2z_xyza ) );
    const VF32    _2zz( Z( _2z_xyza ) );
    const VF32    _2ax( W( _2x_xyza ) );
    const VF32    _2az( W( _2z_xyza ) );
    return unit_y + NegateY( MakeVector3( _2xy, _2xx, _2yz ) ) + NegateXY( MakeVector3( _2az, _2zz, _2ax ) );
}

/// Quaternion get matrix row Z
NOALIAS __forceinline const Vector3 MatrixRowZ( const Quaternion PCREF q )
{
    const Vector3 unit_z( Const<Vector3>::UnitZ() );
    const Vec4F32 xyza( q.m_Q );
    const Vec4F32 _2xyza( xyza + xyza );
    const Vec4F32 _2x_xyza( X(xyza) * _2xyza );
    const Vec4F32 _2y_xyza( Y(xyza) * _2xyza );
    const VF32    _2xx( X( _2x_xyza ) );
    const VF32    _2xz( Z( _2x_xyza ) );
    const VF32    _2yy( Y( _2y_xyza ) );
    const VF32    _2yz( Z( _2y_xyza ) );
    const VF32    _2ax( W( _2x_xyza ) );
    const VF32    _2ay( W( _2y_xyza ) );
    return unit_z + NegateZ( MakeVector3( _2xz, _2yz, _2xx ) ) + NegateYZ( MakeVector3( _2ay, _2ax, _2yy ) );
}







/// Quaternion Serialise to Stream
template < typename T_Stream > void Serialise( const Quaternion PCREF src, T_Stream& stream )
{
    Serialise( src.m_Q, stream );
}

/// Quaternion Deserialise from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Quaternion* const __restrict pDst )
{
    Deserialise( stream, &pDst->m_Q );
}




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

