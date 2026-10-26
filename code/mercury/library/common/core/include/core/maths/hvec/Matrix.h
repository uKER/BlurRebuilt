#ifndef MATHS_HVEC_MATRIX_H
#define MATHS_HVEC_MATRIX_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

/// Full matrix inverse using Cramers Rule ( FP version )
void Matrix44InverseCramersRuleFP( const F32* mat, F32* dst );

class C_Plane;

//
// ------------------------------------------------
// |-- Rotation classes for matrix construction --|
// ------------------------------------------------
//

class RotX
{
public:
    /// F32 Constructor
    NOALIAS __forceinline RotX( const F32& a );
    
    /// HF32 Constructor
    NOALIAS __forceinline RotX( const HF32 PCREF a );
    
    /// HF32 Cast
    NOALIAS __forceinline operator const HF32 () const __restrict;
    
//private:
    HF32    m_AngleR;
};

class RotY
{
public:
    /// F32 Constructor
    NOALIAS __forceinline RotY( const F32& a );
    
    /// HF32 Constructor
    NOALIAS __forceinline RotY( const HF32 PCREF a );
    
    /// HF32 Cast
    NOALIAS __forceinline operator const HF32 () const __restrict;
    
//private:
    HF32    m_AngleR;
};

class RotZ
{
public:
    /// F32 Constructor
    NOALIAS __forceinline RotZ( const F32& a );
    
    /// HF32 Constructor
    NOALIAS __forceinline RotZ( const HF32 PCREF a );
    
    /// HF32 Cast
    NOALIAS __forceinline operator const HF32 () const __restrict;
    
//private:
    HF32    m_AngleR;
};



//
// --------------------------------------------
// |-- Axis classes for matrix construction --|
// --------------------------------------------
//

class AxisX : public C_Normal
{
public:
    NOALIAS __forceinline AxisX( const C_Normal PCREF ax );
};

class AxisY : public C_Normal
{
public:
    NOALIAS __forceinline AxisY( const C_Normal PCREF ay );
};

class AxisZ : public C_Normal
{
public:
    NOALIAS __forceinline AxisZ( const C_Normal PCREF az );
};





//
// -------------------------------------------
// |-- Scale class for matrix construction --|
// -------------------------------------------
//

class C_MatrixScale
{
public:
    
    /// Constructors
    NOALIAS __forceinline C_MatrixScale();
    NOALIAS __forceinline C_MatrixScale( const HVec3 PCREF scale );
    NOALIAS __forceinline C_MatrixScale( const HF32 PCREF sx, const HF32 PCREF sy, const HF32 PCREF sz );
    
    /// Cast operators
    NOALIAS __forceinline operator const HVec3 () const __restrict;
    
    /// Assignment operators
    NOALIAS __forceinline void          operator = ( const HVec3 PCREF scale ) __restrict;
    
    /// Accessors
    NOALIAS __forceinline const HF32    X() const __restrict;
    NOALIAS __forceinline const HF32    Y() const __restrict;
    NOALIAS __forceinline const HF32    Z() const __restrict;
    
public:
    HVec3           m_Scale;
};


//
// ------------------------------------------------
// |-- Rotate RPH class for matrix construction --|
// ------------------------------------------------
//

class C_MatrixRotateRPH
{
public:
    
    /// Constructors
    NOALIAS __forceinline C_MatrixRotateRPH();
    NOALIAS __forceinline C_MatrixRotateRPH( const HVec3 PCREF rotate );
    NOALIAS __forceinline C_MatrixRotateRPH( const HF32 PCREF r, const HF32 PCREF p, const HF32 PCREF h );
    
    /// Cast operators
    NOALIAS __forceinline operator const HVec3 () const __restrict;
    
    /// Assignment operators
    NOALIAS __forceinline void            operator = ( const HVec3 PCREF rotate ) __restrict;
    
    /// Accessors
    NOALIAS __forceinline const RotZ      Roll() const __restrict;
    NOALIAS __forceinline const RotX      Pitch() const __restrict;
    NOALIAS __forceinline const RotY      Heading() const __restrict;
    
public:
    HVec3           m_Rotate;
};


//
// ------------------------------------------------
// |-- Rotate PHR class for matrix construction --|
// ------------------------------------------------
//

class C_MatrixRotatePHR
{
public:
    
    /// Constructors
    NOALIAS __forceinline C_MatrixRotatePHR();
    NOALIAS __forceinline C_MatrixRotatePHR( const HVec3 PCREF rotate );
    NOALIAS __forceinline C_MatrixRotatePHR( const HF32 PCREF p, const HF32 PCREF h, const HF32 PCREF r );
    
    /// Cast operators
    NOALIAS __forceinline operator const HVec3 () const __restrict;
    
    /// Assignment operators
    NOALIAS __forceinline void            operator = ( const HVec3 PCREF rotate ) __restrict;
    
    /// Accessors
    NOALIAS __forceinline const RotX      Pitch() const __restrict;
    NOALIAS __forceinline const RotY      Heading() const __restrict;
    NOALIAS __forceinline const RotZ      Roll() const __restrict;
    
public:
    HVec3           m_Rotate;
};


//
// -----------------------------------------------
// |-- Translate class for matrix construction --|
// -----------------------------------------------
//

class C_MatrixTranslate
{
public:
    
    /// Constructors
    NOALIAS __forceinline C_MatrixTranslate();
    NOALIAS __forceinline C_MatrixTranslate( const HVec3 PCREF translate );
    NOALIAS __forceinline C_MatrixTranslate( const HF32 PCREF tx, const HF32 PCREF ty, const HF32 PCREF tz );
    
    /// Cast operators
    NOALIAS __forceinline operator const HVec3 () const __restrict;
    
    /// Assignment operators
    NOALIAS __forceinline void            operator = ( const HVec3 PCREF translate ) __restrict;
    
    /// Accessors
    NOALIAS __forceinline const HF32      X() const __restrict;
    NOALIAS __forceinline const HF32      Y() const __restrict;
    NOALIAS __forceinline const HF32      Z() const __restrict;
    
public:
    HVec3           m_Translate;
};


//
// ------------------------------------------------------------
// |-- Scale-Rotate-Translate class for matrix construction --|
// ------------------------------------------------------------
//

class C_MatrixSRT
{
public:
    
    /// Default Constructor
    NOALIAS __forceinline C_MatrixSRT();
    
    /// Scale-Rotate-Translate Constructor
    NOALIAS __forceinline C_MatrixSRT( const C_MatrixScale PCREF scale, const C_MatrixRotateRPH PCREF rotate, const C_MatrixTranslate PCREF translate );
    
    /// Accessor: Scale
    NOALIAS __forceinline const C_MatrixScale         Scale() const __restrict;
    
    /// Accessor: Rotate
    NOALIAS __forceinline const C_MatrixRotateRPH     Rotate() const __restrict;
    
    /// Accessor: Translate
    NOALIAS __forceinline const C_MatrixTranslate     Translate() const __restrict;
    
private:
    
    C_MatrixScale               m_Scale;
    C_MatrixRotateRPH           m_Rotate;
    C_MatrixTranslate           m_Translate;
};






//
// --------------------------------------------------------------------------
// |-- C_Eigen33 : Eigen values & vectors from HMat33 Eigen Decomposition --|
// --------------------------------------------------------------------------
//

class C_Eigen33
{
public:
    
    /// Eigen Value & Vector Constructor
    NOALIAS __forceinline C_Eigen33( const HVec3 PCREF val, const HVec3 PCREF vec0, const HVec3 PCREF vec1, const HVec3 PCREF vec2 );
    
    /// Copy Constructor
    NOALIAS __forceinline C_Eigen33( const C_Eigen33& copy );
    
    /// Copy Assignment
    NOALIAS __forceinline const C_Eigen33& operator = ( const C_Eigen33& copy );
    
    /// Accessor: Value 0
    NOALIAS __forceinline const HF32    Value0() const __restrict;
    
    /// Accessor: Value 1
    NOALIAS __forceinline const HF32    Value1() const __restrict;
    
    /// Accessor: Value 2
    NOALIAS __forceinline const HF32    Value2() const __restrict;
    
    /// Accessor: Values
    NOALIAS __forceinline const HVec3   Values() const __restrict;
    
    /// Accessor: Vector 0
    NOALIAS __forceinline const HVec3   Vector0() const __restrict;
    
    /// Accessor: Vector 1
    NOALIAS __forceinline const HVec3   Vector1() const __restrict;
    
    /// Accessor: Vector 2
    NOALIAS __forceinline const HVec3   Vector2() const __restrict;
    
private:
    
    HVec3       m_Values;
    HVec3       m_Vector0;
    HVec3       m_Vector1;
    HVec3       m_Vector2;
};


//
// -------------------------------------------------------
// |-- HMat33 : Hardware 3 Row * 3 Column Matrix Class --|
// -------------------------------------------------------
//

class HMat33
{
public:
    
    /// Mat33 Constructor
    NOALIAS __forceinline HMat33( const Mat33 MATREF m )
        #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
        : m_AX( m.m_RowX )
        , m_AY( m.m_RowY )
        , m_AZ( m.m_RowZ )
        #elif defined( _VMX128_ )
        : m_CX( m.m_ColX )
        , m_CY( m.m_ColY )
        , m_CZ( m.m_ColZ )
        #endif
    {
        #if !defined( _SIMD_ )
        m_A[0].m_V[0] = m.m[0][0]; m_A[0].m_V[1] = m.m[0][1]; m_A[0].m_V[2] = m.m[0][2];
        m_A[1].m_V[0] = m.m[1][0]; m_A[1].m_V[1] = m.m[1][1]; m_A[1].m_V[2] = m.m[1][2];
        m_A[2].m_V[0] = m.m[2][0]; m_A[2].m_V[1] = m.m[2][1]; m_A[2].m_V[2] = m.m[2][2];
        #endif
    }
    
    /// Mat33 Cast
    NOALIAS __forceinline operator const Mat33() const __restrict
    {
        Mat33 result;
        #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
        result.m_RowX = m_AX;
        result.m_RowY = m_AY;
        result.m_RowZ = m_AZ;
        #elif defined( _VMX128_ )
        result.m_ColX = m_CX;
        result.m_ColY = m_CY;
        result.m_ColZ = m_CZ;
        #else
        result.m[0][0] = m_A[0].m_V[0]; result.m[0][1] = m_A[0].m_V[1]; result.m[0][2] = m_A[0].m_V[2];
        result.m[1][0] = m_A[1].m_V[0]; result.m[1][1] = m_A[1].m_V[1]; result.m[1][2] = m_A[1].m_V[2];
        result.m[2][0] = m_A[2].m_V[0]; result.m[2][1] = m_A[2].m_V[1]; result.m[2][2] = m_A[2].m_V[2];
        #endif
        return result;
    }
    
    /// Default Constructor: Initialised to Identity
    NOALIAS __forceinline HMat33();
    
    /// Undefined Constructor
    NOALIAS __forceinline HMat33( const _UNDEFINED& undefined );
    
	/// Constructor: initialises to the zero matrix.
    NOALIAS __forceinline HMat33( const _ZERO_MATRIX );

    /// Copy Constructor
    NOALIAS __forceinline HMat33( const HMat33& src );
    
    /// Explicit HMat43 Constructor
    NOALIAS __forceinline explicit HMat33( const HMat43& src );
    
    /// Explicit HMat44 Constructor
    NOALIAS __forceinline explicit HMat33( const HMat44& src );
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    /// V128 Axis XYZ Constructor
    inline HMat33( const vector float ax, const vector float ay, const vector float az ) __attribute__(( always_inline ));
    #elif defined( _VMX128_ )
    /// V128 Column XYZ Constructor
    NOALIAS __forceinline HMat33( const V128 cx, const V128 cy, const V128 cz );
    #elif defined( _SSE_ )
    /// V128 Axis XYZ Constructor
    NOALIAS __forceinline HMat33( const V128 PCREF ax, const V128 PCREF ay, const V128 PCREF az );
    #endif
    
    /// HVec3 X, Y & Z Constructor
    NOALIAS __forceinline HMat33( const HVec3 PCREF ax, const HVec3 PCREF ay, const HVec3 PCREF az );
    
    /// X Axis Rotation Constructor - clockwise when viewed back down the X axis
    NOALIAS /*__forceinline*/ HMat33( const RotX PCREF alpha );
    
    /// Y Axis Rotation Constructor - clockwise when viewed back down the Y axis
    NOALIAS /*__forceinline*/ HMat33( const RotY PCREF beta );
    
    /// Z Axis Rotation Constructor - clockwise when viewed back down the Z axis
    NOALIAS /*__forceinline*/ HMat33( const RotZ PCREF gamma );
    
    /// Constructor - Scale matrix
    NOALIAS __forceinline HMat33( const C_MatrixScale PCREF scale );
    
    /// Constructor - Rotation matrix from euler angles - Roll-Pitch-Heading
    NOALIAS /*__forceinline*/ HMat33( const C_MatrixRotateRPH PCREF euler );
    
    /// Constructor - Rotation matrix from euler angles - Pitch-Heading-Roll
    NOALIAS __forceinline HMat33( const C_MatrixRotatePHR PCREF euler );
    
    /// Z Axis Constructor
    NOALIAS __forceinline HMat33( const AxisZ PCREF az );
    
    /// X & Y Axes Constructor: Primary X, Secondary Y
    NOALIAS __forceinline HMat33( const AxisX PCREF ax, const AxisY PCREF ay );
    
    /// X & Z Axes Constructor: Primary X, Secondary Z
    NOALIAS __forceinline HMat33( const AxisX PCREF ax, const AxisZ PCREF az );
    
    /// Y & X Axes Constructor: Primary Y, Secondary X
    NOALIAS __forceinline HMat33( const AxisY PCREF ay, const AxisX PCREF ax );
    
    /// Y & Z Axes Constructor: Primary Y, Secondary Z
    NOALIAS __forceinline HMat33( const AxisY PCREF ay, const AxisZ PCREF az );
    
    /// Z & X Axes Constructor: Primary Z, Secondary X
    NOALIAS __forceinline HMat33( const AxisZ PCREF az, const AxisX PCREF ax );
    
    /// Z & Y Axes Constructor: Primary Z, Secondary Y
    NOALIAS __forceinline HMat33( const AxisZ PCREF az, const AxisY PCREF ay );
    
    #if ( DIRECT3D_VERSION >= 0x0900 )
    
    /// Cast to D3DMATRIX
    NOALIAS __forceinline operator const D3DMATRIX () const;
    
    #endif
    
    /// Copy Assignment
    NOALIAS __forceinline const HMat33              operator = ( const HMat33& copy );
    
    /// Accessor: Get Axis X
    NOALIAS __forceinline const HVec3               AX() const;
    
    /// Accessor: Get Axis Y
    NOALIAS __forceinline const HVec3               AY() const;
    
    /// Accessor: Get Axis Z
    NOALIAS __forceinline const HVec3               AZ() const;

    /// Accessor: Set Axis X
    NOALIAS __forceinline void                      AX( const HVec3 PCREF ax ) __restrict;
    
    /// Accessor: Set Axis Y
    NOALIAS __forceinline void                      AY( const HVec3 PCREF ay ) __restrict;
    
    /// Accessor: Set Axis Z
    NOALIAS __forceinline void                      AZ( const HVec3 PCREF az ) __restrict;
    
	/// Accessor: Get All axes
	NOALIAS __forceinline void                      GetAxes( HVec4& ax, HVec4& ay, HVec4& az ) const __restrict;								
	NOALIAS __forceinline void                      GetAxes( HVec3& ax, HVec3& ay, HVec3& az ) const __restrict;

    /// Private Accessor: Set Axes X, Y & Z
    NOALIAS __forceinline void                      SetAxes( const HVec3 PCREF ax, const HVec3 PCREF ay, const HVec3 PCREF az ) __restrict;
    
    /// Accessor: Set Axes X, Y & Z
    #if defined( _SPU_ ) || defined( _PPU_ )        
    NOALIAS __forceinline void                      SetAxesXYZ( const vector float ax, const vector float ay, const vector float az ) __restrict __attribute__(( always_inline ));
    #elif defined( _VMX128_ )
    NOALIAS __forceinline void                      SetAxesXYZ( const V128 ax, const V128 ay, const V128 az ) __restrict;
    #elif defined( _SSE_ )
    NOALIAS __forceinline void                      SetAxesXYZ( const V128 PCREF ax, const V128 PCREF ay, const V128 PCREF az ) __restrict;
	#endif
    
    #if !defined( _SIMD_ )
    
    /// Const Element Index Accessor
    NOALIAS __forceinline const HVec3               operator [] ( const U32 i ) const __restrict;
    
    /// Element Index Accessor
    NOALIAS __forceinline HVec3&                    operator [] ( const U32 i ) __restrict;
    
    #endif
    
    /// Conversion to Euler angles - Roll-Pitch-Heading
    NOALIAS /*__forceinline*/ const C_MatrixRotateRPH   GetRotationRPH() const __restrict;
    
    /// Conversion to Euler angles - Pitch-Heading-Roll
    NOALIAS /*__forceinline*/ const C_MatrixRotatePHR   GetRotationPHR() const __restrict;
    
    /// Set to Identity Matrix
    NOALIAS __forceinline void                      SetIdentity() __restrict;
    
    /// Transpose of Matrix
    NOALIAS __forceinline const HMat33              Transposed() const __restrict;
    
    /// Transpose this Matrix
    NOALIAS __forceinline void                      Transpose() __restrict;
    
    /// Scalar Right-Multiplication
    NOALIAS __forceinline const HMat33              operator * ( const HF32 PCREF s ) const __restrict;

    /// Matrix Addition
    NOALIAS __forceinline const HMat33              operator + ( const HMat33& m ) const __restrict;
    
    /// Matrix Subtraction
    NOALIAS __forceinline const HMat33              operator - ( const HMat33& m ) const __restrict;
    
    /// Matrix Multiplication
    NOALIAS __forceinline const HMat33              operator * ( const HMat33& m ) const __restrict;
    
    /// Matrix * Vector
    NOALIAS __forceinline const HVec3               operator * ( const HVec3 PCREF v ) const __restrict;
    
    /// Combine with HVec3 position to form a HMat43
    NOALIAS __forceinline const HMat43              operator || ( const HVec3 PCREF pos ) const __restrict;
    
	/// Trace of the matrix (sum of the diagonal elements).
    NOALIAS __forceinline const HF32                Trace() const __restrict;
    
    /// Matrix Determinant
    NOALIAS __forceinline const HF32                Determinant() const __restrict;
    
    /// Inverse of Rotation - Faster but only valid for matrices with normalised & orthogonal rotations!
    NOALIAS __forceinline const HMat33              InverseRotation() const __restrict;
    
    /// Full Matrix Inverse - This will FAIL if determinant is zero!
    NOALIAS __forceinline const HMat33              Inverse() const __restrict;
    
    /// Normalise matrix: Priority to Z axis, secondary Y
    NOALIAS __forceinline void                      NormaliseZY() __restrict;
    
    /// Normalised copy of matrix: Priority to Z axis, secondary Y
    NOALIAS __forceinline const HMat33              NormalisedZY() const __restrict;
    
    /// Returns the 3 eigenvectors of the matrix
    NOALIAS __forceinline const HMat33              EigenVectors() const __restrict;
    
    /// CONVERTED FROM TOOLS TEAM CODE - NEED DESCRIPTION FROM THEM
    NOALIAS /*__forceinline*/ void                  Align( HF32& a, const HVec3 PCREF vNormal ) __restrict;
    
    /// Jacobi Transformation Matrix for elimination of off-diagonal [ p, q ] NOTE: matrix symmetric & p != q !
    NOALIAS /*__forceinline*/ const HMat33          JacobiTransformation( const U32 p, const U32 q ) const __restrict;
    
    /// QR Decomposition
    NOALIAS /*__forceinline*/ void                  QR_Decomposition( HMat33& Q, HMat33& R ) const __restrict;
    
    /// RQ Decomposition
    NOALIAS /*__forceinline*/ void                  RQ_Decomposition( HMat33& R, HMat33& Q ) const __restrict;
    
    /// Solve R * x = y, where R is an Upper Triangular matrix
    NOALIAS /*__forceinline*/ const HVec3           Solve_Rx_Eq_y( const HVec3 PCREF y ) const __restrict;
    
    /// Eigen Decomposition
    NOALIAS /*__forceinline*/ const C_Eigen33       EigenDecomposition() const __restrict;
    
    /// Finite test
    NOALIAS __forceinline const Scalar_Bool         Finite() const __restrict;
    
    template < typename T_Stream > __forceinline void Deserialise( T_Stream& stream ) __restrict;

    template < typename T_Stream > __forceinline void Serialise( T_Stream& stream ) __restrict;
    
public:
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector float        m_AX;
    vector float        m_AY;
    vector float        m_AZ;
    #elif defined( _VMX128_ )
    V128                m_CX;
    V128                m_CY;
    V128                m_CZ;
    #elif defined( _SSE_ )
    HVec3               m_AX;
    HVec3               m_AY;
    HVec3               m_AZ;
    #else
    HVec3               m_A[3];
    #endif
};

/// Scalar Left-Multiplication
NOALIAS __forceinline const HMat33 operator * ( const HF32 PCREF s, const HMat33& m );
    




//
// -------------------------------------------------------
// |-- HMat43 : Hardware 4 Row * 3 Column Matrix Class --|
// -------------------------------------------------------
//

class HMat43
{
public:
    
    /// Mat43 Constructor
    NOALIAS __forceinline HMat43( const Mat43 MATREF m )
        #if defined( _SPU_ ) || defined( _PPU_ )
        : m_AX( m.m_RowX )
        , m_AY( m.m_RowY )
        , m_AZ( m.m_RowZ )
        , m_AW( m.m_RowW )
        #elif defined( _VMX128_ )
        : m_CX( m.m_ColX )
        , m_CY( m.m_ColY )
        , m_CZ( m.m_ColZ )
        #elif defined( _SSE_ )
        : m_Rotate( m.m_RowX, m.m_RowY, m.m_RowZ )
        , m_Translate( m.m_RowW )
        #endif
    {
        #if !defined( _SIMD_ )
        m_Rotate.m_A[0].m_V[0] = m.m[0][0]; m_Rotate.m_A[0].m_V[1] = m.m[0][1]; m_Rotate.m_A[0].m_V[2] = m.m[0][2];
        m_Rotate.m_A[1].m_V[0] = m.m[1][0]; m_Rotate.m_A[1].m_V[1] = m.m[1][1]; m_Rotate.m_A[1].m_V[2] = m.m[1][2];
        m_Rotate.m_A[2].m_V[0] = m.m[2][0]; m_Rotate.m_A[2].m_V[1] = m.m[2][1]; m_Rotate.m_A[2].m_V[2] = m.m[2][2];
        m_Translate.m_V[0]     = m.m[3][0]; m_Translate.m_V[1]     = m.m[3][1]; m_Translate.m_V[2]     = m.m[3][2];
        #endif
    }
    
    /// Mat43 Cast
    NOALIAS __forceinline operator const Mat43() const __restrict
    {
        Mat43 result;
        #if defined( _SPU_ ) || defined( _PPU_ )
        result.m_RowX = m_AX;
        result.m_RowY = m_AY;
        result.m_RowZ = m_AZ;
        result.m_RowW = m_AW;
        #elif defined( _VMX128_ )
        result.m_ColX = m_CX;
        result.m_ColY = m_CY;
        result.m_ColZ = m_CZ;
        #elif defined( _SSE_ )
        result.m_RowX = m_Rotate.m_AX.m_V;
        result.m_RowY = m_Rotate.m_AY.m_V;
        result.m_RowZ = m_Rotate.m_AZ.m_V;
        result.m_RowW = m_Translate.m_V;
        #else
        result.m[0][0] = m_Rotate.m_A[0].m_V[0]; result.m[0][1] = m_Rotate.m_A[0].m_V[1]; result.m[0][2] = m_Rotate.m_A[0].m_V[2];
        result.m[1][0] = m_Rotate.m_A[1].m_V[0]; result.m[1][1] = m_Rotate.m_A[1].m_V[1]; result.m[1][2] = m_Rotate.m_A[1].m_V[2];
        result.m[2][0] = m_Rotate.m_A[2].m_V[0]; result.m[2][1] = m_Rotate.m_A[2].m_V[1]; result.m[2][2] = m_Rotate.m_A[2].m_V[2];
        result.m[3][0] = m_Translate.m_V[0];     result.m[3][1] = m_Translate.m_V[1];     result.m[3][2] = m_Translate.m_V[2];
        #endif
        return result;
    }
    
    /// Default Constructor: Initialised to Identity
    NOALIAS __forceinline HMat43();
    
    /// Undefined Constructor
    NOALIAS __forceinline HMat43( const _UNDEFINED undefined );
	
	/// Constructor: initialises to the zero matrix.
    NOALIAS __forceinline HMat43( const _ZERO_MATRIX );
    
    /// Explicit HMat33 Constructor
    NOALIAS __forceinline explicit HMat43( const HMat33& src );
    
    /// Copy Constructor
    NOALIAS __forceinline HMat43( const HMat43& copy );
    
    /// Explicit HMat44 Constructor
    NOALIAS __forceinline explicit HMat43( const HMat44& src );
    
    /// Rotation + Translation Constructor
    NOALIAS __forceinline HMat43( const HMat33& rotate, const HVec3 PCREF translate );

    /// Scale, Rotation & Translation Constructor 
	NOALIAS __forceinline HMat43( const HVec3 PCREF scale, const HMat33& rotate, const HVec3 PCREF translate );
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    /// vector float Axis XYZW Constructor
    inline HMat43( const vector float ax, const vector float ay, const vector float az, const vector float aw ) __attribute__(( always_inline ));
    #elif defined( _VMX128_ )
    /// V128 Column XYZ Constructor
    NOALIAS __forceinline HMat43( const V128 cx, const V128 cy, const V128 cz );
    #elif defined( _SSE_ )
    /// V128 Axis XYZW Constructor
    NOALIAS __forceinline HMat43( const V128 PCREF ax, const V128 PCREF ay, const V128 PCREF az, const V128 PCREF aw );
    #endif
    
    /// HVec3 Axis XYZW Constructor
    NOALIAS __forceinline HMat43( const HVec3 PCREF ax, const HVec3 PCREF ay, const HVec3 PCREF az, const HVec3 PCREF aw );
    
    /// X Axis Rotation Constructor - clockwise when viewed back down the X axis
    NOALIAS /*__forceinline*/ HMat43( const RotX PCREF rx, const HVec3 PCREF aw = HVec3( c_VectorZero ) );
    
    /// Y Axis Rotation Constructor - clockwise when viewed back down the Y axis
    NOALIAS /*__forceinline*/ HMat43( const RotY PCREF ry, const HVec3 PCREF aw = HVec3( c_VectorZero ) );
    
    /// Z Axis Rotation Constructor - clockwise when viewed back down the Z axis
    NOALIAS /*__forceinline*/ HMat43( const RotZ PCREF rz, const HVec3 PCREF aw = HVec3( c_VectorZero ) );
    
    /// Constructor: Primary axis X, secondary axis Y
    NOALIAS __forceinline HMat43( const AxisX PCREF ax, const AxisY PCREF ay, const HVec3 PCREF aw = HVec3( c_VectorZero ) );
    
    /// Constructor: Primary axis X, secondary axis Z
    NOALIAS __forceinline HMat43( const AxisX PCREF ax, const AxisZ PCREF az, const HVec3 PCREF aw = HVec3( c_VectorZero ) );
    
    /// Constructor: Primary axis Y, secondary axis X
    NOALIAS __forceinline HMat43( const AxisY PCREF ay, const AxisX PCREF ax, const HVec3 PCREF aw = HVec3( c_VectorZero ) );
    
    /// Constructor: Primary axis Y, secondary axis Z
    NOALIAS __forceinline HMat43( const AxisY PCREF ay, const AxisZ PCREF az, const HVec3 PCREF aw = HVec3( c_VectorZero ) );
    
    /// Constructor: Primary axis Z, secondary axis X
    NOALIAS __forceinline HMat43( const AxisZ PCREF az, const AxisX PCREF ax, const HVec3 PCREF aw = HVec3( c_VectorZero ) );
    
    /// Constructor: Primary axis Z, secondary axis Y
    NOALIAS __forceinline HMat43( const AxisZ PCREF az, const AxisY PCREF ay, const HVec3 PCREF aw = HVec3( c_VectorZero ) );
    
    /// Scale-Rotate-Translate Constructor
    NOALIAS __forceinline HMat43( const C_MatrixSRT PCREF srt );
    
    /// Reflection Plane Constructor
    NOALIAS __forceinline HMat43( const C_Plane PCREF plane );
    
    #if ( DIRECT3D_VERSION >= 0x0900 )
    
    /// Convert to D3DMATRIX
    NOALIAS __forceinline operator const D3DMATRIX () const __restrict;
    
    #endif
    
    /// Accessor: Get Rotation
    NOALIAS __forceinline const HMat33          Rotation() const __restrict;
    
    /// Accessor: Get Translation
    NOALIAS __forceinline const HVec3           Translation() const __restrict;
    
    /// Accessor: Set Rotation
    NOALIAS __forceinline void                  Rotation( const HMat33& rotation ) __restrict;
    
    /// Accessor: Set Translation
    NOALIAS __forceinline void                  Translation( const HVec3 PCREF translation ) __restrict;
    
    /// Accessor: Get Axis X
    NOALIAS __forceinline const HVec3           AX() const __restrict;
    
    /// Accessor: Get Axis Y
    NOALIAS __forceinline const HVec3           AY() const __restrict;
    
    /// Accessor: Get Axis Z
    NOALIAS __forceinline const HVec3           AZ() const __restrict;
    
    /// Accessor: Get Axis W
    NOALIAS __forceinline const HVec3           AW() const __restrict;

    /// Accessor: Set Axis X
    NOALIAS __forceinline void                  AX( const HVec3 PCREF ax ) __restrict;
    
    /// Accessor: Set Axis Y
    NOALIAS __forceinline void                  AY( const HVec3 PCREF ay ) __restrict;
    
    /// Accessor: Set Axis Z
    NOALIAS __forceinline void                  AZ( const HVec3 PCREF az ) __restrict;
    
    /// Accessor: Set Axis W
    NOALIAS __forceinline void                  AW( const HVec3 PCREF aw ) __restrict;
    
    /// Accessor: Get all axes
    NOALIAS __forceinline void                  GetAxes( HVec3& dstAX, HVec3& dstAY, HVec3& dstAZ, HVec3& dstAW ) const __restrict;
    NOALIAS __forceinline void                  GetAxes( HVec4& dstAX, HVec4& dstAY, HVec4& dstAZ, HVec4& dstAW ) const __restrict;
    NOALIAS __forceinline void                  SetAxes( const HVec4 PCREF srcAX, const HVec4 PCREF srcAY, const HVec4 PCREF srcAZ, const HVec4 PCREF srcAW ) __restrict;
    NOALIAS __forceinline void                  SetAxesScaled( const HVec4 PCREF srcAX, const HVec4 PCREF srcAY, const HVec4 PCREF srcAZ, const HVec4 PCREF srcAW, const HVec4 PCREF ScaleValue ) __restrict;

    NOALIAS __forceinline void                  SetScaleRot( const HVec3 PCREF scale, const HMat33& rot ) __restrict;
    NOALIAS __forceinline void                  SetScaleRotTrans( const HVec3 PCREF scale, const HMat33& rot, const HVec3 PCREF translate ) __restrict;
    
    #if !defined( _SIMD_ )
    
    /// Accessor: Const Axis Indexed
    NOALIAS __forceinline const HVec3           operator [] ( const U32 i ) const __restrict;
    
    /// Accessor: Axis Indexed
    NOALIAS __forceinline HVec3&                operator [] ( const U32 i ) __restrict;
    
    #endif
    
    /// HMat33 Assignment
    NOALIAS __forceinline const HMat43         operator = ( const HMat33& src ) __restrict;
    
    /// Copy Assignment
    NOALIAS __forceinline const HMat43         operator = ( const HMat43& copy );
    
    /// Set to Identity Matrix
    NOALIAS __forceinline void                  SetIdentity() __restrict;
    
    /// Matrix Multiply
    NOALIAS __forceinline const HMat43          operator * ( const HMat43& m ) const __restrict;
    
    /// Inverse of Rotation & Translation - Faster but only valid for matrices with normalised & orthogonal rotations!
    NOALIAS __forceinline const HMat43          InverseRotTrans() const __restrict;
    
    /// Get Scale-Rotate-Translate
    NOALIAS __forceinline const C_MatrixSRT     GetSRT() const __restrict;
    
    template< typename T_Stream > __forceinline void Deserialise( T_Stream& stream ) __restrict;
                                  
    template< typename T_Stream > __forceinline void Serialise( T_Stream& stream ) __restrict;

public:
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector float                m_AX;
    vector float                m_AY;
    vector float                m_AZ;
    vector float                m_AW;
    #elif defined( _VMX128_ )
    V128                        m_CX;
    V128                        m_CY;
    V128                        m_CZ;
    #else
    HMat33                      m_Rotate;
    HVec3                       m_Translate;
    #endif
};






//
// -------------------------------------------------------
// |-- HMat44 : Hardware 4 Row * 4 Column Matrix Class --|
// -------------------------------------------------------
//

class HMat44
{
public:
    
    /// Mat44 Constructor
    NOALIAS __forceinline HMat44( const Mat44 MATREF m )
        #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
        : m_AX( m.m_RowX )
        , m_AY( m.m_RowY )
        , m_AZ( m.m_RowZ )
        , m_AW( m.m_RowW )
        #elif defined( _VMX128_ )
        : m_CX( m.m_ColX )
        , m_CY( m.m_ColY )
        , m_CZ( m.m_ColZ )
        , m_CW( m.m_ColW )
        #endif
    {
        #if !defined( _SIMD_ )
        m_A[0].m_V[0] = m.m[0][0]; m_A[0].m_V[1] = m.m[0][1]; m_A[0].m_V[2] = m.m[0][2]; m_A[0].m_V[3] = m.m[0][3];
        m_A[1].m_V[0] = m.m[1][0]; m_A[1].m_V[1] = m.m[1][1]; m_A[1].m_V[2] = m.m[1][2]; m_A[1].m_V[3] = m.m[1][3];
        m_A[2].m_V[0] = m.m[2][0]; m_A[2].m_V[1] = m.m[2][1]; m_A[2].m_V[2] = m.m[2][2]; m_A[2].m_V[3] = m.m[2][3];
        m_A[3].m_V[0] = m.m[3][0]; m_A[3].m_V[1] = m.m[3][1]; m_A[3].m_V[2] = m.m[3][2]; m_A[3].m_V[3] = m.m[3][3];
        #endif
    }
    
    /// Mat44 Cast
    NOALIAS __forceinline operator const Mat44() const __restrict
    {
        Mat44 result;
        #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
        result.m_RowX = m_AX;
        result.m_RowY = m_AY;
        result.m_RowZ = m_AZ;
        result.m_RowW = m_AW;
        #elif defined( _VMX128_ )
        result.m_ColX = m_CX;
        result.m_ColY = m_CY;
        result.m_ColZ = m_CZ;
        result.m_ColW = m_CW;
        #else
        result.m[0][0] = m_A[0].m_V[0]; result.m[0][1] = m_A[0].m_V[1]; result.m[0][2] = m_A[0].m_V[2]; result.m[0][3] = m_A[0].m_V[3];
        result.m[1][0] = m_A[1].m_V[0]; result.m[1][1] = m_A[1].m_V[1]; result.m[1][2] = m_A[1].m_V[2]; result.m[1][3] = m_A[1].m_V[3];
        result.m[2][0] = m_A[2].m_V[0]; result.m[2][1] = m_A[2].m_V[1]; result.m[2][2] = m_A[2].m_V[2]; result.m[2][3] = m_A[2].m_V[3];
        result.m[3][0] = m_A[3].m_V[0]; result.m[3][1] = m_A[3].m_V[1]; result.m[3][2] = m_A[3].m_V[2]; result.m[3][3] = m_A[3].m_V[3];
        #endif
        return result;
    }
    
    /// Default Constructor: Initialise to Identity
    NOALIAS __forceinline HMat44();
    
    /// Undefined Constructor
    NOALIAS __forceinline HMat44( const _UNDEFINED undefined );
	
	/// Constructor: initialises to the zero matrix.
    NOALIAS __forceinline HMat44( const _ZERO_MATRIX );

	/// Explicit Constructor from HMat33
    NOALIAS __forceinline explicit HMat44( const HMat33& src );
    
    /// Explicit Constructor from HMat43
    NOALIAS __forceinline explicit HMat44( const HMat43& src );
    
    /// Copy Constructor
    NOALIAS __forceinline HMat44( const HMat44& src );
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    /// vector float Axis XYZW Constructor
    inline HMat44( const vector float ax, const vector float ay, const vector float az, const vector float aw ) __attribute__(( always_inline ));
    #elif defined( _VMX128_ )
    /// V128 Column XYZW Constructor
    NOALIAS __forceinline HMat44( const V128 cx, const V128 cy, const V128 cz, const V128 cw );
    #elif defined( _SSE_ )
    /// V128 Axis XYZW Constructor
    NOALIAS __forceinline HMat44( const V128 PCREF ax, const V128 PCREF ay, const V128 PCREF az, const V128 PCREF aw );
    #endif
    
    /// HVec4 Axis XYZW Constructor
    NOALIAS __forceinline HMat44( const HVec4 PCREF ax, const HVec4 PCREF ay, const HVec4 PCREF az, const HVec4 PCREF aw );
    
    /// X Axis Rotation Constructor : clockwise when viewed back down the X axis
    NOALIAS __forceinline HMat44( const RotX PCREF rx, const HVec4 PCREF aw = HVec4( c_UnitVectorW ) );
    
    /// Y Axis Rotation Constructor : clockwise when viewed back down the Y axis
    NOALIAS __forceinline HMat44( const RotY PCREF ry, const HVec4 PCREF aw = HVec4( c_UnitVectorW ) );
    
    /// Z Axis Rotation Constructor : clockwise when viewed back down the Z axis
    NOALIAS __forceinline HMat44( const RotZ PCREF rz, const HVec4 PCREF aw = HVec4( c_UnitVectorW ) );
    
    /// Constructor from primary axis X, secondary axis Y
    NOALIAS __forceinline HMat44( const AxisX PCREF ax, const AxisY PCREF ay, const HVec4 PCREF aw = HVec4( c_UnitVectorW ) );
    
    /// Constructor from primary axis X, secondary axis Z
    NOALIAS __forceinline HMat44( const AxisX PCREF ax, const AxisZ PCREF az, const HVec4 PCREF aw = HVec4( c_UnitVectorW ) );
    
    /// Constructor from primary axis Y, secondary axis X
    NOALIAS __forceinline HMat44( const AxisY PCREF ay, const AxisX PCREF ax, const HVec4 PCREF aw = HVec4( c_UnitVectorW ) );
    
    /// Constructor from primary axis Y, secondary axis Z
    NOALIAS __forceinline HMat44( const AxisY PCREF ay, const AxisZ PCREF az, const HVec4 PCREF aw = HVec4( c_UnitVectorW ) );
    
    /// Constructor from primary axis Z, secondary axis X
    NOALIAS __forceinline HMat44( const AxisZ PCREF az, const AxisX PCREF ax, const HVec4 PCREF aw = HVec4( c_UnitVectorW ) );
    
    /// Constructor from primary axis Z, secondary axis Y
    NOALIAS __forceinline HMat44( const AxisZ PCREF az, const AxisY PCREF ay, const HVec4 PCREF aw = HVec4( c_UnitVectorW ) );
    
    /// Reflection Plane Constructor
    NOALIAS __forceinline HMat44( const C_Plane PCREF plane );
    
    #if ( DIRECT3D_VERSION >= 0x0900 )
    
    /// Cast to D3DMATRIX
    NOALIAS __forceinline operator const D3DMATRIX () const __restrict;
    
    #endif
    
    /// Accessor: Get Axis X
    NOALIAS __forceinline const HVec4       AX() const __restrict;
    
    /// Accessor: Get Axis Y
    NOALIAS __forceinline const HVec4       AY() const __restrict;
    
    /// Accessor: Get Axis Z
    NOALIAS __forceinline const HVec4       AZ() const __restrict;
    
    /// Accessor: Get Axis W
    NOALIAS __forceinline const HVec4       AW() const __restrict;
    
    /// Accessor: Set Axis X
    NOALIAS __forceinline void              AX( const HVec4 PCREF ax ) __restrict;
    
    /// Accessor: Set Axis Y
    NOALIAS __forceinline void              AY( const HVec4 PCREF ay ) __restrict;
    
    /// Accessor: Set Axis Z
    NOALIAS __forceinline void              AZ( const HVec4 PCREF az ) __restrict;
    
    /// Accessor: Set Axis W
    NOALIAS __forceinline void              AW( const HVec4 PCREF aw ) __restrict;

	NOALIAS __forceinline void              GetAxes( HVec4& dstAX, HVec4& dstAY, HVec4& dstAZ, HVec4& dstAW ) const __restrict;
    
    /// Set Axes X, Y, Z & W from HVec4's
    NOALIAS __forceinline void              SetAxes( const HVec4 PCREF ax, const HVec4 PCREF ay, const HVec4 PCREF az, const HVec4 PCREF aw ) __restrict;
    
    /// Accessor: Set Axes X, Y, Z & W from vector floats / V128's
    #if defined( _SPU_ ) || defined( _PPU_ )
    inline void                             SetAxesXYZW( const vector float ax, const vector float ay, const vector float az, const vector float aw ) __restrict __attribute__(( always_inline ));
    #elif defined( _VMX128_ )
    NOALIAS __forceinline void              SetAxesXYZW( const V128 ax, const V128 ay, const V128 az, const V128 aw ) __restrict;
    #elif defined( _SSE_ )
    NOALIAS __forceinline void              SetAxesXYZW( const V128 PCREF ax, const V128 PCREF ay, const V128 PCREF az, const V128 PCREF aw ) __restrict;
	#endif
    
    #if !defined( _SIMD_ )
    /// Accessor: Const HVec4 indexed
    NOALIAS __forceinline const HVec4       operator [] ( const U32 i ) const __restrict;
    /// Accessor: HVec4 indexed
    NOALIAS __forceinline HVec4&            operator [] ( const U32 i ) __restrict;
	#else
	/// Accessor: Const HVec4 indexed
	NOALIAS __forceinline const HVec4       operator [] ( const U32 i ) const __restrict;
    /// Accessor: HVec4 indexed
    //NOALIAS __forceinline HVec4&            operator [] ( const U32 i ) __restrict;
    #endif
    
    /// HMat33 Assignment
    NOALIAS __forceinline const HMat44     operator = ( const HMat33& src ) __restrict;
    
    /// HMat43 Assignment
    NOALIAS __forceinline const HMat44     operator = ( const HMat43& src ) __restrict;
    
    /// Copy Assignment
    NOALIAS __forceinline const HMat44     operator = ( const HMat44& copy );
    
    /// Set to Identity Matrix
    NOALIAS __forceinline void              SetIdentity() __restrict;
    
    /// Transpose of Matrix
    NOALIAS __forceinline const HMat44      Transposed() const __restrict;
    
    /// Transpose this Matrix
    NOALIAS __forceinline void              Transpose() __restrict;
    
    /// Matrix Multiply
    NOALIAS __forceinline const HMat44      operator * ( const HMat44& m ) const __restrict;
    
    #if defined( _SIMD_ )
    /// Full 4x4 Matrix Inverse using Cramers Rule ( SSE SIMD version )
    NOALIAS /*__forceinline*/ const HMat44  Matrix44InverseCramersRuleSSE() const __restrict;
    #endif
    
    /// Full 4x4 Matrix Inverse
    NOALIAS __forceinline const HMat44      Inverse() const __restrict;
    
    /// Inverse of Rotation & Translation - Faster but only valid for matrices with normalised & orthogonal rotations!
    NOALIAS __forceinline const HMat44      InverseRotTrans() const __restrict;
    
public:
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector float                m_AX;
    vector float                m_AY;
    vector float                m_AZ;
    vector float                m_AW;
    #elif defined( _VMX128_ )
    V128                        m_CX;
    V128                        m_CY;
    V128                        m_CZ;
    V128                        m_CW;
    #elif defined( _SSE_ )
    HVec4                       m_AX;
    HVec4                       m_AY;
    HVec4                       m_AZ;
    HVec4                       m_AW;
    #else
    HVec4                       m_A[4];
    #endif
};




NOALIAS __forceinline const HMat44 operator * ( const HMat43& a, const HMat44& b );

NOALIAS __forceinline const HMat44 operator * ( const HMat44& a, const HMat43& b );




}; // namespace Maths


#endif // #ifndef MATHS_HVEC_MATRIX_H



