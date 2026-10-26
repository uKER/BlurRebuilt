#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT33_H
#define SOFT_MATHS_HVEC_NEW_MAT33_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT33_H
#define MATHS_HVEC_NEW_MAT33_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat33
/// @{

/// Load Mat33 from an F32 array
/// NOTE: Matrix transposition is platform dependent!
template <> NOALIAS __forceinline const Mat33 Load( F32 const * const __restrict pSrc );

/// Store Mat33 into an F32 array
/// NOTE: Matrix transposition is platform dependent!
NOALIAS __forceinline void Store( const Mat33 MATREF src, F32* const __restrict pDst );


/// Load Mat33 from a Float33
/// NOTE: Matrix transposition is platform dependent!
template <> NOALIAS __forceinline const Mat33 Load( MathsStorage::Float33 const * const __restrict pSrc );

/// Store Mat33 into a Float33
/// NOTE: Matrix transposition is platform dependent!
NOALIAS __forceinline void Store( const Mat33 MATREF src, MathsStorage::Float33* const __restrict pDst );


/// Load Mat33 from a MatrixRow33
template <> NOALIAS __forceinline const Mat33 Load( MathsStorage::MatrixRow33 const * const __restrict pSrc );

/// Store Mat33 into a MatrixRow33
NOALIAS __forceinline void Store( const Mat33 MATREF src, MathsStorage::MatrixRow33* const __restrict pDst );


/// Load Mat33 from a MatrixCol33
template <> NOALIAS __forceinline const Mat33 Load( MathsStorage::MatrixCol33 const * const __restrict pSrc );

/// Store Mat33 into a MatrixCol33
NOALIAS __forceinline void Store( const Mat33 MATREF src, MathsStorage::MatrixCol33* const __restrict pDst );


/// Mat33 constants
template <> struct Const< Mat33 > 
{
	NOALIAS __forceinline static const Mat33& Zero();             ///< Mat33 constant: Zero
	NOALIAS __forceinline static const Mat33& Identity();         ///< Mat33 constant: Identity
	
	NOALIAS __forceinline static const Mat33& RotateX90CW();      ///< Mat33 constant: Rotation about X axis of 90 degrees ClockWise (viewed back along X axis toward origin)
	NOALIAS __forceinline static const Mat33& RotateY90CW();      ///< Mat33 constant: Rotation about Y axis of 90 degrees ClockWise (viewed back along Y axis toward origin)
	NOALIAS __forceinline static const Mat33& RotateZ90CW();      ///< Mat33 constant: Rotation about Z axis of 90 degrees ClockWise (viewed back along Z axis toward origin)
	
	NOALIAS __forceinline static const Mat33& RotateX90AntiCW();  ///< Mat33 constant: Rotation about X axis of 90 degrees Anti-ClockWise (viewed back along X axis toward origin)
	NOALIAS __forceinline static const Mat33& RotateY90AntiCW();  ///< Mat33 constant: Rotation about Y axis of 90 degrees Anti-ClockWise (viewed back along Y axis toward origin)
	NOALIAS __forceinline static const Mat33& RotateZ90AntiCW();  ///< Mat33 constant: Rotation about Z axis of 90 degrees Anti-ClockWise (viewed back along Z axis toward origin)
	
	NOALIAS __forceinline static const Mat33& RotateX180();       ///< Mat33 constant: Rotation about X axis of 180 degrees
	NOALIAS __forceinline static const Mat33& RotateY180();       ///< Mat33 constant: Rotation about Y axis of 180 degrees
	NOALIAS __forceinline static const Mat33& RotateZ180();       ///< Mat33 constant: Rotation about Z axis of 180 degrees
	
private:
	
	static const Mat33 m_Zero;         ///< Mat33 constant: (private)
	static const Mat33 m_Identity;     ///< Mat33 constant: (private)
                                       
    static const Mat33 m_RotateX90CW;  ///< Mat33 constant: (private)
    static const Mat33 m_RotateY90CW;  ///< Mat33 constant: (private)
    static const Mat33 m_RotateZ90CW;  ///< Mat33 constant: (private)
                                       
    static const Mat33 m_RotateX90ACW; ///< Mat33 constant: (private)
    static const Mat33 m_RotateY90ACW; ///< Mat33 constant: (private)
    static const Mat33 m_RotateZ90ACW; ///< Mat33 constant: (private)
                                       
    static const Mat33 m_RotateX180;   ///< Mat33 constant: (private)
    static const Mat33 m_RotateY180;   ///< Mat33 constant: (private)
    static const Mat33 m_RotateZ180;   ///< Mat33 constant: (private)
};


/// Make Mat33 from 9 float elements (row major)
NOALIAS __forceinline const Mat33 MakeMat33( const F32& m00, const F32& m01, const F32& m02,
                                             const F32& m10, const F32& m11, const F32& m12,
                                             const F32& m20, const F32& m21, const F32& m22 );


/// Make Mat33 from 3 row vectors (3 element)
NOALIAS __forceinline const Mat33 MakeMat33( const Vec4F32 VECREF rowX, const Vec4F32 VECREF rowY, const Vec4F32 VECREF rowZ );

/// Make Mat33 from 3 row vectors (3 element)
NOALIAS __forceinline const Mat33 MakeMat33( const Vector3 PCREF rowX, const Vector3 PCREF rowY, const Vector3 PCREF rowZ );

/// Make Mat33 from 3 row normals (3 element)
NOALIAS __forceinline const Mat33 MakeMat33( const Normal3 PCREF rowX, const Normal3 PCREF rowY, const Normal3 PCREF rowZ );

/// Make Mat33 from a Mat43
NOALIAS __forceinline const Mat33 MakeMat33( const Mat43 MATREF src );

/// Make Mat33 from a Mat44
NOALIAS __forceinline const Mat33 MakeMat33( const Mat44 MATREF src );


/// Make Mat33 from a ScaleMat3
NOALIAS __forceinline const Mat33 MakeMat33( const ScaleMat3 PCREF src );


/// Get rows from a Mat33
NOALIAS __forceinline void GetRows( const Mat33 MATREF src, Vec4F32* __restrict pRowX, Vec4F32* __restrict pRowY, Vec4F32* __restrict pRowZ );

/// Set rows of a Mat33
NOALIAS __forceinline void SetRows( Mat33* pDst, const Vec4F32 VECREF rowX, const Vec4F32 VECREF rowY, const Vec4F32 VECREF rowZ );


/// Get rows from a Mat33
NOALIAS __forceinline void GetRows( const Mat33 MATREF src, Vector3* __restrict pRowX, Vector3* __restrict pRowY, Vector3* __restrict pRowZ );

/// Set rows of a Mat33
NOALIAS __forceinline void SetRows( Mat33* pDst, const Vector3 PCREF rowX, const Vector3 PCREF rowY, const Vector3 PCREF rowZ );

/// Set rows of a Mat33
NOALIAS __forceinline void SetRows( Mat33* pDst, const Normal3 PCREF rowX, const Normal3 PCREF rowY, const Normal3 PCREF rowZ );


/// Get columns from a Mat33
NOALIAS __forceinline void GetColumns( const Mat33 MATREF src, Vector3* __restrict pColX, Vector3* __restrict pColY, Vector3* __restrict pColZ );

/// Set columns of a Mat33
NOALIAS __forceinline void SetColumns( Mat33* pDst, const Vector3 PCREF colX, const Vector3 PCREF colY, const Vector3 PCREF colZ );

/// Set columns of a Mat33
NOALIAS __forceinline void SetColumns( Mat33* pDst, const Normal3 PCREF colX, const Normal3 PCREF colY, const Normal3 PCREF colZ );


/// Get Row X from a Mat33
NOALIAS __forceinline const Vector3 RowX( const Mat33 MATREF src );

/// Get Row Y from a Mat33
NOALIAS __forceinline const Vector3 RowY( const Mat33 MATREF src );

/// Get Row Z from a Mat33
NOALIAS __forceinline const Vector3 RowZ( const Mat33 MATREF src );


/// Get Col X from a Mat33
NOALIAS __forceinline const Vector3 ColX( const Mat33 MATREF src );

/// Get Col Y from a Mat33
NOALIAS __forceinline const Vector3 ColY( const Mat33 MATREF src );

/// Get Col Z from a Mat33
NOALIAS __forceinline const Vector3 ColZ( const Mat33 MATREF src );


/// Get element from a Mat33
template < unsigned int Row, unsigned int Col > NOALIAS __forceinline const VF32 Element( const Mat33 MATREF src );


/// Set Row X of a Mat33
NOALIAS __forceinline void SetRowX( Mat33* pDst, const Vec4F32 VECREF src );

/// Set Row Y of a Mat33
NOALIAS __forceinline void SetRowY( Mat33* pDst, const Vec4F32 VECREF src );

/// Set Row Z of a Mat33
NOALIAS __forceinline void SetRowZ( Mat33* pDst, const Vec4F32 VECREF src );


/// Set Col X of a Mat33
NOALIAS __forceinline void SetColX( Mat33* pDst, const Vec4F32 VECREF src );

/// Set Col Y of a Mat33
NOALIAS __forceinline void SetColY( Mat33* pDst, const Vec4F32 VECREF src );

/// Set Col Z of a Mat33
NOALIAS __forceinline void SetColZ( Mat33* pDst, const Vec4F32 VECREF src );


/// Set Row X of a Mat33
NOALIAS __forceinline void SetRowX( Mat33* pDst, const Vector3 PCREF src );

/// Set Row Y of a Mat33
NOALIAS __forceinline void SetRowY( Mat33* pDst, const Vector3 PCREF src );

/// Set Row Z of a Mat33
NOALIAS __forceinline void SetRowZ( Mat33* pDst, const Vector3 PCREF src );


/// Set Col X of a Mat33
NOALIAS __forceinline void SetColX( Mat33* pDst, const Vector3 PCREF src );

/// Set Col Y of a Mat33
NOALIAS __forceinline void SetColY( Mat33* pDst, const Vector3 PCREF src );

/// Set Col Z of a Mat33
NOALIAS __forceinline void SetColZ( Mat33* pDst, const Vector3 PCREF src );


/// Create a Mat33 rotation matrix of alpha radians about the X axis [ clockwise when viewed back down the X axis ]
NOALIAS __forceinline const Mat33 MakeMat33_RotX( const VF32 VECREF alpha );

/// Create a Mat33 rotation matrix of beta radians about the Y axis [ clockwise when viewed back down the Y axis ]
NOALIAS __forceinline const Mat33 MakeMat33_RotY( const VF32 VECREF beta );

/// Create a Mat33 rotation matrix of gamma radians about the Z axis [ clockwise when viewed back down the Z axis ]
NOALIAS __forceinline const Mat33 MakeMat33_RotZ( const VF32 VECREF gamma );


/// Create a Mat33 rotation matrix of theta radians about an arbitrary axis
NOALIAS __forceinline const Mat33 MakeMat33_AxisAngle( const Normal3 PCREF axis, const VF32 VECREF theta );

/// Create a Mat33 rotation matrix of theta radians about an arbitrary axis: Note: normalises axis and then calls the Normal3 variant of MakeMat33_AxisAngle!
NOALIAS __forceinline const Mat33 MakeMat33_AxisAngle( const Vector3 PCREF axis, const VF32 VECREF theta );


/// Create a Mat33 rotation matrix from roll-pitch-heading Euler angles
NOALIAS __forceinline const Mat33 MakeMat33_RotRPH( const VF32 VECREF roll, const VF32 VECREF pitch, const VF32 VECREF heading );

/// Create a Mat33 rotation matrix from pitch-heading-roll Euler angles
NOALIAS __forceinline const Mat33 MakeMat33_RotPHR( const VF32 VECREF pitch, const VF32 VECREF heading, const VF32 VECREF roll );


/// Create an ortho-normal Mat33 with the z axis pointing along the vector vz
NOALIAS __forceinline const Mat33 MakeMat33_AxisZ( const Vec4F32 VECREF vz );

/// Create an ortho-normal Mat33 with the z axis pointing along the vector vz and the y axis in the plane vz^vy
NOALIAS __forceinline const Mat33 MakeMat33_AxisZY( const Vec4F32 VECREF vz, const Vec4F32 VECREF vy );

/// Create an ortho-normal Mat33 with the z axis pointing along the vector vz and the x axis in the plane vz^vx
NOALIAS __forceinline const Mat33 MakeMat33_AxisZX( const Vec4F32 VECREF vz, const Vec4F32 VECREF vx );


/// Create an ortho-normal Mat33 with the z axis pointing along the vector vz
NOALIAS __forceinline const Mat33 MakeMat33_AxisZ( const Vector3 PCREF vz );

/// Create an ortho-normal Mat33 with the z axis pointing along the normal axisZ
NOALIAS __forceinline const Mat33 MakeMat33_AxisZ( const Normal3 PCREF axisZ );


/// Create an ortho-normal Mat33 with the z axis pointing along the vector vz and the y axis in the plane vz^vy
NOALIAS __forceinline const Mat33 MakeMat33_AxisZY( const Vector3 PCREF vz, const Vector3 PCREF vy );

/// Create an ortho-normal Mat33 with the z axis pointing along the vector vz and the x axis in the plane vz^vx
NOALIAS __forceinline const Mat33 MakeMat33_AxisZX( const Vector3 PCREF vz, const Vector3 PCREF vx );



/// Create a skew-symmetric matrix from a Vector3 (Cross-Product Matrix)
NOALIAS __forceinline const Mat33 MakeMat33_SkewSymmetric( const Vector3 PCREF a );


/// Create a Mat33 from an EulerXYZ
NOALIAS __forceinline const Mat33 MakeMat33( const EulerXYZ& src );

/// Create a Mat33 from an EulerXZY
NOALIAS __forceinline const Mat33 MakeMat33( const EulerXZY& src );

/// Create a Mat33 from an EulerYXZ
NOALIAS __forceinline const Mat33 MakeMat33( const EulerYXZ& src );

/// Create a Mat33 from an EulerYZX
NOALIAS __forceinline const Mat33 MakeMat33( const EulerYZX& src );

/// Create a Mat33 from an EulerZXY
NOALIAS __forceinline const Mat33 MakeMat33( const EulerZXY& src );

/// Create a Mat33 from an EulerZYX
NOALIAS __forceinline const Mat33 MakeMat33( const EulerZYX& src );


/// Mat33 Write to Stream
template < typename T_Stream > void Serialise( const Mat33 MATREF src, T_Stream& stream );

/// Mat33 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Mat33* const __restrict pDst );


/// Mat33 Negation
NOALIAS __forceinline const Mat33 operator - ( const Mat33 MATREF src );

/// Mat33 Addition operator
NOALIAS __forceinline const Mat33 operator + ( const Mat33 MATREF lhs, const Mat33 MATREF rhs );

/// Mat33 Subtraction operator
NOALIAS __forceinline const Mat33 operator - ( const Mat33 MATREF lhs, const Mat33 MATREF rhs );

/// Mat33 * VF32 Multiplication operator
NOALIAS __forceinline const Mat33 operator * ( const Mat33 MATREF matrix, const VF32 VECREF scalar );

/// VF32 * Mat33 Multiplication operator
NOALIAS __forceinline const Mat33 operator * ( const VF32 VECREF scalar, const Mat33 MATREF matrix );

/// Mat33 / VF32 Division operator
NOALIAS __forceinline const Mat33 operator / ( const Mat33 MATREF matrix, const VF32 VECREF scalar );


/// Vec4F32 * Mat33 Multiplication operator : input = 3 element row vector, output = 3 element row vector
NOALIAS __forceinline const Vec4F32 operator * ( const Vec4F32 VECREF vector3, const Mat33 MATREF matrix );

/// Mat33 * Vec4F32 Multiplication operator : input = 3 element col vector, output = 3 element col vector 
NOALIAS __forceinline const Vec4F32 operator * ( const Mat33 MATREF matrix, const Vec4F32 VECREF vector3 );


/// Vector3 Matrix Multiplication: Vector3 * Mat33
NOALIAS __forceinline const Vector3 operator * ( const Vector3 PCREF v, const Mat33 MATREF m );

/// Vector3 Matrix Multiplication: Mat33 * Vector3
NOALIAS __forceinline const Vector3 operator * ( const Mat33 MATREF m, const Vector3 PCREF v );

/// Vector3 Matrix Multiplication Update: Vector3 *= Mat33
NOALIAS __forceinline void operator *= ( Vector3& v, const Mat33 MATREF m );


/// Point3 Matrix Multiplication: Point3 * Mat33
NOALIAS __forceinline const Point3 operator * ( const Point3 PCREF p, const Mat33 MATREF m );

/// Point3 Matrix Multiplication Update: Point3 *= Mat33
NOALIAS __forceinline void operator *= ( Point3& p, const Mat33 MATREF m );


/// HPoint3 Matrix Multiplication: HPoint3 * Mat33
NOALIAS __forceinline const HPoint3 operator * ( const HPoint3 PCREF p, const Mat33 MATREF m );

/// HPoint3 Matrix Multiplication Update: HPoint3 *= Mat33
NOALIAS __forceinline void operator *= ( HPoint3& p, const Mat33 MATREF m );


/// Mat33 Multiplication operator
NOALIAS __forceinline const Mat33 operator * ( const Mat33 MATREF lhs, const Mat33 MATREF rhs );


/// Vector Space Tensor Product of a 3x1 column vector and a 1x3 row vector -> Mat33
NOALIAS __forceinline const Mat33 TensorProduct( const Vector3 PCREF lhs, const Vector3 PCREF rhs );


/// Normalise the axes of a Mat33 : returns normalised result (NOTE: Does NOT orthogonalise the axes!)
/// (faster but only 22 bit precision)
NOALIAS __forceinline const Mat33 Normalise( const Mat33 MATREF src );

/// Normalise the axes of a Mat33 : returns normalised result (NOTE: Does NOT orthogonalise the axes!)
/// (slower but full 24 bit precision)
NOALIAS __forceinline const Mat33 Normalise24( const Mat33 MATREF src );


/// Normalise the axes of a Mat33 (NOTE: Does NOT orthogonalise the axes!)
/// (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise( Mat33* pDst );

/// Normalise the axes of a Mat33 (NOTE: Does NOT orthogonalise the axes!)
/// (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise24( Mat33* pDst );


/// Ortho-Normalise the axes of a Mat33 : returns normalised result (priority given to z axis then y axis with x axis fully recomputed) 
/// (faster but only 22 bit precision)
NOALIAS __forceinline const Mat33 OrthoNormalise( const Mat33 MATREF src );

/// Ortho-Normalise the axes of a Mat33 : returns normalised result (priority given to z axis then y axis with x axis fully recomputed) 
/// (slower but full 24 bit precision)
NOALIAS __forceinline const Mat33 OrthoNormalise24( const Mat33 MATREF src );


/// Ortho-Normalise the axes of a Mat33 (priority given to z axis then y axis with x axis fully recomputed) 
/// (faster but only 22 bit precision)
NOALIAS __forceinline void OrthoNormalise( Mat33* pDst );

/// Ortho-Normalise the axes of a Mat33 (priority given to z axis then y axis with x axis fully recomputed) 
/// (slower but full 24 bit precision)
NOALIAS __forceinline void OrthoNormalise24( Mat33* pDst );


/// Ortho-Normalise the axes of a Mat33 : returns normalised result (priority given to z axis then x axis with y axis fully recomputed) 
NOALIAS const Mat33 OrthoNormaliseZX( const Mat33 MATREF src );


/// Ortho-Normalise the axes of a Mat33 : returns normalised result (priority given to x axis then y axis with z axis fully recomputed) 
/// (faster but only 22 bit precision)
NOALIAS const Mat33 OrthoNormaliseXY( const Mat33 MATREF src );

/// Ortho-Normalise the axes of a Mat33 : returns normalised result (priority given to x axis then y axis with z axis fully recomputed) 
/// (slower but full 24 bit precision)
NOALIAS const Mat33 OrthoNormaliseXY_24( const Mat33 MATREF src );


/// Ortho-Normalise the axes of a Mat33 (priority given to z axis then x axis with y axis fully recomputed) 
NOALIAS void OrthoNormaliseZX( Mat33* pDst );


/// Ortho-Normalise the axes of a Mat33 (priority given to x axis then y axis with z axis fully recomputed) 
NOALIAS void OrthoNormaliseXY( Mat33* pDst );


/// Transpose Mat33 : returns transposed result
NOALIAS __forceinline const Mat33 Transpose( const Mat33 MATREF src );

/// Transpose Mat33 : transposes pDst
NOALIAS __forceinline void Transpose( Mat33 * const __restrict pDst );


/// Trace of Mat33 : sum of diagonal elements
NOALIAS __forceinline const VF32 Trace( const Mat33 MATREF src );

/// Determinant of Mat33
NOALIAS __forceinline const VF32 Determinant( const Mat33 MATREF src );


/// Inverse of Mat33
/// (faster but only 22 bit precision)
NOALIAS __forceinline const Mat33 Inverse( const Mat33 MATREF src );

/// Inverse of Mat33
/// (slower but full 24 bit precision)
NOALIAS __forceinline const Mat33 Inverse24( const Mat33 MATREF src );


/// Inverse of Mat33: returns true if matrix was invertible
/// (faster but only 22 bit precision)
//NOALIAS __forceinline const bool Inverse( const Mat33 MATREF src, Mat33* const __restrict pDst );
NOALIAS __forceinline const VBool Inverse( const Mat33 MATREF src, Mat33* const __restrict pDst );

/// Inverse of Mat33: returns true if matrix was invertible
/// (slower but full 24 bit precision)
NOALIAS __forceinline const VBool Inverse24( const Mat33 MATREF src, Mat33* const __restrict pDst );


/// Inverse transpose of Mat33
NOALIAS __forceinline const Mat33 InverseTranspose( const Mat33 MATREF src );


/// Mat33 Select
NOALIAS __forceinline const Mat33 Select( const VBool PCREF condition, const Mat33 MATREF result_if_true, const Mat33 MATREF result_if_false );

/// Mat33 maximum absolute value of all elements
NOALIAS __forceinline const VF32 MaxAbsValue( const Mat33 MATREF src );

/// Mat33 Extract Scale And Shear
NOALIAS const bool ExtractScaleAndShear( Mat33* const __restrict pMatrix, Vector3* const __restrict pScale, Vector3* const __restrict pShear );

/// Mat33 Extract Scale Shear And Rotate
//NOALIAS const bool DecomposeScaleShearRotate( const Mat33 MATREF                matrix,
NOALIAS const VBool DecomposeScaleShearRotate( const Mat33 MATREF                matrix,
                                               ScaleMat3* const __restrict pScale,
                                               ShearMat3* const __restrict pShear,
                                               Mat33*     const __restrict pRotate );

/// Mat33 equality test with tolerance
NOALIAS __forceinline const VBool Equal( const Mat33 MATREF a, const Mat33 MATREF b, const VF32 VECREF tolerance = Const<VF32>::Frac<1,100>() );


/// Cramers Rule: Solves for xyz, given 3 simultaneous equations
/// a1.x + b1.y + c1.z = d1,
/// a2.x + b2.y + c2.z = d2,
/// a3.x + b3.y + c3.z = d3
/// (faster but only 22 bit precision)
NOALIAS __forceinline const VBool CramersRule( const Mat33 MATREF abc, const Vector3 PCREF d, Vector3* const __restrict pXYZ );

/// Cramers Rule: Solves for xyz, given 3 simultaneous equations
/// a1.x + b1.y + c1.z = d1,
/// a2.x + b2.y + c2.z = d2,
/// a3.x + b3.y + c3.z = d3
/// (slower but full 24 bit precision)
NOALIAS __forceinline const VBool CramersRule24( const Mat33 MATREF abc, const Vector3 PCREF d, Vector3* const __restrict pXYZ );

/// Mat33 RQ Decomposition
NOALIAS const VBool RQ_Decomposition( const Mat33 MATREF A, Mat33* const __restrict pR, Mat33* const __restrict pQ );

/// Solve R * x = y, where R is an Upper Triangular matrix
NOALIAS const VBool Solve_Rx_Eq_y( const Mat33 MATREF R, const Vector3 PCREF y, Vector3* const __restrict pX );

/// Mat33 Eigen Decomposition: A=input matrix, P=eigen vector matrix, D=eigen value diagonal matrix, A = P * D * inverse(P)
NOALIAS const VBool SymmetricEigenDecomposition(
    const Mat33 MATREF A,
    Mat33    * const __restrict pEigenVectorMatrix,
    ScaleMat3* const __restrict pEigenValueMatrix );

/// Mat33 Sqrt of Symmetric Matrix
NOALIAS const VBool SymmetricSqrt( const Mat33 MATREF A, Mat33* const pResult );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


