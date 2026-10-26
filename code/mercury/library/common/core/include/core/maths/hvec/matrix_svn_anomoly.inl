#ifndef MATHS_HVEC_MATRIX_INL
#define MATHS_HVEC_MATRIX_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

//
// ------------------------------------------------
// |-- Rotation X class for matrix construction --|
// ------------------------------------------------
//

/// F32 Constructor
FORCE_INLINE
RotX::RotX( const F32& a ) :
    m_AngleR( a )
{
}

/// HF32 Constructor
FORCE_INLINE
RotX::RotX( const HF32& a ) :
    m_AngleR( a )
{
}

/// HF32 Cast
FORCE_INLINE
RotX::operator const HF32 () const
{
    return m_AngleR;
}


//
// ------------------------------------------------
// |-- Rotation Y class for matrix construction --|
// ------------------------------------------------
//

/// F32 Constructor
FORCE_INLINE
RotY::RotY( const F32& a ) :
    m_AngleR( a )
{
}

/// HF32 Constructor
FORCE_INLINE
RotY::RotY( const HF32& a ) :
    m_AngleR( a )
{
}

/// HF32 Cast
FORCE_INLINE
RotY::operator const HF32 () const
{
    return m_AngleR;
}


//
// ------------------------------------------------
// |-- Rotation Z class for matrix construction --|
// ------------------------------------------------
//

/// F32 Constructor
FORCE_INLINE
RotZ::RotZ( const F32& a ) :
    m_AngleR( a )
{
}

/// HF32 Constructor
FORCE_INLINE
RotZ::RotZ( const HF32& a ) :
    m_AngleR( a )
{
}

/// HF32 Cast
FORCE_INLINE
RotZ::operator const HF32 () const
{
    return m_AngleR;
}



//
// --------------------------------------------
// |-- Axis X class for matrix construction --|
// --------------------------------------------
//

/// HVec3 Constructor
FORCE_INLINE
AxisX::AxisX( const C_Normal& ax ) :
    C_Normal( ax )
{
}

//
// --------------------------------------------
// |-- Axis Y class for matrix construction --|
// --------------------------------------------
//

/// HVec3 Constructor
FORCE_INLINE
AxisY::AxisY( const C_Normal& ay ) :
    C_Normal( ay )
{
}

//
// --------------------------------------------
// |-- Axis Z class for matrix construction --|
// --------------------------------------------
//

/// HVec3 Constructor
FORCE_INLINE
AxisZ::AxisZ( const C_Normal& az ) :
    C_Normal( az )
{
}








//
// -------------------------------------------
// |-- Scale class for matrix construction --|
// -------------------------------------------
//

/// Default Constructor
FORCE_INLINE
C_MatrixScale::C_MatrixScale()
{
}

/// HVec3 Constructor
FORCE_INLINE
C_MatrixScale::C_MatrixScale( const HVec3& scale ) :
    m_Scale( scale )
{
}

/// HF32 XYZ Constructor
FORCE_INLINE
C_MatrixScale::C_MatrixScale( const HF32& sx, const HF32& sy, const HF32& sz ) :
    m_Scale( HVec3( sx, sy, sz ) )
{
}

/// HVec3 Cast
FORCE_INLINE
C_MatrixScale::operator const HVec3 () const
{
    return m_Scale;
}

/// HVec3 Assignment
FORCE_INLINE
void C_MatrixScale::operator = ( const HVec3& scale )
{
    m_Scale = scale;
}

/// Accessor X
FORCE_INLINE
const HF32 C_MatrixScale::X() const
{
    return m_Scale.X();
}

/// Accessor Y
FORCE_INLINE
const HF32 C_MatrixScale::Y() const
{
    return m_Scale.Y();
}

/// Accessor Z
FORCE_INLINE
const HF32 C_MatrixScale::Z() const
{
    return m_Scale.Z();
}


//
// ------------------------------------------------
// |-- Rotate RPH class for matrix construction --|
// ------------------------------------------------
//

/// Default Constructor
FORCE_INLINE
C_MatrixRotateRPH::C_MatrixRotateRPH()
{
}

/// HVec3 Constructor
FORCE_INLINE
C_MatrixRotateRPH::C_MatrixRotateRPH( const HVec3& rotate ) :
    m_Rotate( rotate )
{
}

/// HF32 Roll-Pitch-Heading Constructor
FORCE_INLINE
C_MatrixRotateRPH::C_MatrixRotateRPH( const HF32& r, const HF32& p, const HF32& h ) :
    m_Rotate( HVec3( r, p, h ) )
{
}

/// HVec3 Cast
FORCE_INLINE
C_MatrixRotateRPH::operator const HVec3 () const
{
    return m_Rotate;
}

/// HVec3 Assignment
FORCE_INLINE
void C_MatrixRotateRPH::operator = ( const HVec3& rotate )
{
    m_Rotate = rotate;
}

/// Accessor: Roll
FORCE_INLINE
const RotZ C_MatrixRotateRPH::Roll() const
{
    return RotZ( m_Rotate.X() );
}

/// Accessor: Pitch
FORCE_INLINE
const RotX C_MatrixRotateRPH::Pitch() const
{
    return RotX( m_Rotate.Y() );
}

/// Accessor: Heading
FORCE_INLINE
const RotY C_MatrixRotateRPH::Heading() const
{
    return RotY( m_Rotate.Z() );
}



//
// ------------------------------------------------
// |-- Rotate PHR class for matrix construction --|
// ------------------------------------------------
//

/// Default Constructor
FORCE_INLINE
C_MatrixRotatePHR::C_MatrixRotatePHR()
{
}

/// HVec3 Constructor
FORCE_INLINE
C_MatrixRotatePHR::C_MatrixRotatePHR( const HVec3& rotate ) :
    m_Rotate( rotate )
{
}

/// HF32 Pitch-Heading-Roll Constructor
FORCE_INLINE
C_MatrixRotatePHR::C_MatrixRotatePHR( const HF32& p, const HF32& h, const HF32& r ) :
    m_Rotate( HVec3( p, h, r ) )
{
}

/// HVec3 Cast
FORCE_INLINE
C_MatrixRotatePHR::operator const HVec3 () const
{
    return m_Rotate;
}

/// HVec3 Assignment
FORCE_INLINE
void C_MatrixRotatePHR::operator = ( const HVec3& rotate )
{
    m_Rotate = rotate;
}

/// Accessor: Pitch
FORCE_INLINE
const RotX C_MatrixRotatePHR::Pitch() const
{
    return RotX( m_Rotate.X() );
}

/// Accessor: Heading
FORCE_INLINE
const RotY C_MatrixRotatePHR::Heading() const
{
    return RotY( m_Rotate.Y() );
}

/// Accessor: Roll
FORCE_INLINE
const RotZ C_MatrixRotatePHR::Roll() const
{
    return RotZ( m_Rotate.Z() );
}


//
// -----------------------------------------------
// |-- Translate class for matrix construction --|
// -----------------------------------------------
//

/// Default Constructor
FORCE_INLINE
C_MatrixTranslate::C_MatrixTranslate()
{
}

/// HVec3 Constructor
FORCE_INLINE
C_MatrixTranslate::C_MatrixTranslate( const HVec3& translate ) :
    m_Translate( translate )
{
}

/// HF32 XYZ Constructor
FORCE_INLINE
C_MatrixTranslate::C_MatrixTranslate( const HF32& tx, const HF32& ty, const HF32& tz ) :
    m_Translate( HVec3( tx, ty, tz ) )
{
}

/// HVec3 Cast
FORCE_INLINE
C_MatrixTranslate::operator const HVec3 () const
{
    return m_Translate;
}

/// HVec3 Assignment
FORCE_INLINE
void C_MatrixTranslate::operator = ( const HVec3& translate )
{
    m_Translate = translate;
}

/// Accessor: X
FORCE_INLINE
const HF32 C_MatrixTranslate::X() const
{
    return m_Translate.X();
}

/// Accessor: Y
FORCE_INLINE
const HF32 C_MatrixTranslate::Y() const
{
    return m_Translate.Y();
}

/// Accessor: Z
FORCE_INLINE
const HF32 C_MatrixTranslate::Z() const
{
    return m_Translate.Z();
}


//
// ------------------------------------------------------------
// |-- Scale-Rotate-Translate class for matrix construction --|
// ------------------------------------------------------------
//

/// Default Constructor
FORCE_INLINE
C_MatrixSRT::C_MatrixSRT()
{
}

/// Scale-Rotate-Translate Constructor
FORCE_INLINE
C_MatrixSRT::C_MatrixSRT( const C_MatrixScale& scale, const C_MatrixRotateRPH& rotate, const C_MatrixTranslate& translate ) :
    m_Scale( scale ),
    m_Rotate( rotate ),
    m_Translate( translate )
{
}

/// Accessor: Scale
FORCE_INLINE
const C_MatrixScale C_MatrixSRT::Scale() const
{
    return m_Scale;
}

/// Accessor: Rotate
FORCE_INLINE
const C_MatrixRotateRPH C_MatrixSRT::Rotate() const
{
    return m_Rotate;
}

/// Accessor: Translate
FORCE_INLINE
const C_MatrixTranslate C_MatrixSRT::Translate() const
{
    return m_Translate;
}




//
// --------------------------------------------------------------------------
// |-- C_Eigen33 : Eigen values & vectors from HMat33 Eigen Decomposition --|
// --------------------------------------------------------------------------
//

/// Eigen Value & Vector Constructor
FORCE_INLINE
C_Eigen33::C_Eigen33( const HVec3& val, const HVec3& vec0, const HVec3& vec1, const HVec3& vec2 ) :
    m_Values ( val ),
    m_Vector0( vec0 ),
    m_Vector1( vec1 ),
    m_Vector2( vec2 )
{
}

/// Copy Constructor
FORCE_INLINE
C_Eigen33::C_Eigen33( const C_Eigen33& copy ) :
    m_Values ( copy.m_Values  ),
    m_Vector0( copy.m_Vector0 ),
    m_Vector1( copy.m_Vector1 ),
    m_Vector2( copy.m_Vector2 )
{
}

/// Copy Assignment
FORCE_INLINE
const C_Eigen33& C_Eigen33::operator = ( const C_Eigen33& copy )
{
    m_Values  = copy.m_Values;
    m_Vector0 = copy.m_Vector0;
    m_Vector1 = copy.m_Vector1;
    m_Vector2 = copy.m_Vector2;
    return *this;
}

/// Accessor: Value 0
FORCE_INLINE
const HF32 C_Eigen33::Value0() const
{
    return m_Values.X();
}

/// Accessor: Value 1
FORCE_INLINE
const HF32 C_Eigen33::Value1() const
{
    return m_Values.Y();
}

/// Accessor: Value 2
FORCE_INLINE
const HF32 C_Eigen33::Value2() const
{
    return m_Values.Z();
}

/// Accessor: Values
FORCE_INLINE
const HVec3 C_Eigen33::Values() const
{
    return m_Values;
}

/// Accessor: Vector 0
FORCE_INLINE
const HVec3 C_Eigen33::Vector0() const
{
    return m_Vector0;
}

/// Accessor: Vector 1
FORCE_INLINE
const HVec3 C_Eigen33::Vector1() const
{
    return m_Vector1;
}

/// Accessor: Vector 2
FORCE_INLINE
const HVec3 C_Eigen33::Vector2() const
{
    return m_Vector2;
}



//
// -------------------------------------------------------
// |-- HMat33 : Hardware 3 Row * 3 Column Matrix Class --|
// -------------------------------------------------------
//

// Cross products for deriving third matrix axis from 2 input axes
// const HVec3 ax = ay ^ az;
// const HVec3 ay = az ^ ax;
// const HVec3 az = ax ^ ay;

/// Default Constructor: Initialised to Identity
FORCE_INLINE
HMat33::HMat33()
    #if defined( _VMX128_ )
    : m_CX( c_UnitVectorX ),
      m_CY( c_UnitVectorY ),
      m_CZ( c_UnitVectorZ )
    #elif defined( _SSE_ )
    : m_AX( c_UnitVectorX ),
      m_AY( c_UnitVectorY ),
      m_AZ( c_UnitVectorZ )
    #endif
{
    #if !defined( _SIMD_ )
    m_A[MX] = HVec3( c_UnitVectorX );
    m_A[MY] = HVec3( c_UnitVectorY );
    m_A[MZ] = HVec3( c_UnitVectorZ );
    #endif
}

/// Undefined Constructor
NOALIAS __forceinline HMat33::HMat33( const _UNDEFINED undefined ) __restrict
{
}

/// Constructor: initialises to the zero matrix.
FORCE_INLINE
HMat33::HMat33( const _ZERO_MATRIX )
{
	const HVec3 zero;

	AX( zero );
	AY( zero );
	AZ( zero );
}

/// Copy Constructor
FORCE_INLINE
HMat33::HMat33( const HMat33& src )
    #if defined( _VMX128_ )
    : m_CX( src.m_CX ),
      m_CY( src.m_CY ),
      m_CZ( src.m_CZ )
    #elif defined( _SSE_ )
    : m_AX( src.m_AX.m_V ),
      m_AY( src.m_AY.m_V ),
      m_AZ( src.m_AZ.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_A[MX] = src.m_A[MX];
    m_A[MY] = src.m_A[MY];
    m_A[MZ] = src.m_A[MZ];
    #endif
}

/// Explicit HMat43 Constructor
FORCE_INLINE
HMat33::HMat33( const HMat43& src )
    #if defined( _VMX128_ )
    : m_CX( __vrlimi( src.m_CX, __vzero(), VMX_MASK_INSERT_W, 0 ) ),
      m_CY( __vrlimi( src.m_CY, __vzero(), VMX_MASK_INSERT_W, 0 ) ),
      m_CZ( __vrlimi( src.m_CZ, __vzero(), VMX_MASK_INSERT_W, 0 ) )
    #elif defined( _SSE_ )
    : m_AX( src.m_Rotate.m_AX.m_V ),
      m_AY( src.m_Rotate.m_AY.m_V ),
      m_AZ( src.m_Rotate.m_AZ.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_A[MX] = src.m_Rotate.m_A[MX];
    m_A[MY] = src.m_Rotate.m_A[MY];
    m_A[MZ] = src.m_Rotate.m_A[MZ];
    #endif
}

/// Explicit HMat44 Constructor
FORCE_INLINE
HMat33::HMat33( const HMat44& src )
    #if defined( _VMX128_ )
    : m_CX( __vrlimi( src.m_CX, __vzero(), VMX_MASK_INSERT_W, 0 ) ),
      m_CY( __vrlimi( src.m_CY, __vzero(), VMX_MASK_INSERT_W, 0 ) ),
      m_CZ( __vrlimi( src.m_CZ, __vzero(), VMX_MASK_INSERT_W, 0 ) )
    #elif defined( _SSE_ )
    : m_AX( SIMD_AND( src.m_AX.m_V, SIMD_Mask<1,1,1,0>() ) ),
      m_AY( SIMD_AND( src.m_AY.m_V, SIMD_Mask<1,1,1,0>() ) ),
      m_AZ( SIMD_AND( src.m_AZ.m_V, SIMD_Mask<1,1,1,0>() ) )
    #endif
{
    #if !defined( _SIMD_ )
    
    m_A[MX].m_V[VX] = src.m_A[MX].m_V[VX];
    m_A[MX].m_V[VY] = src.m_A[MX].m_V[VY];
    m_A[MX].m_V[VZ] = src.m_A[MX].m_V[VZ];
    
    m_A[MY].m_V[VX] = src.m_A[MY].m_V[VX];
    m_A[MY].m_V[VY] = src.m_A[MY].m_V[VY];
    m_A[MY].m_V[VZ] = src.m_A[MY].m_V[VZ];
    
    m_A[MZ].m_V[VX] = src.m_A[MZ].m_V[VX];
    m_A[MZ].m_V[VY] = src.m_A[MZ].m_V[VY];
    m_A[MZ].m_V[VZ] = src.m_A[MZ].m_V[VZ];
    
    #endif
}

#if defined( _VMX128_ )
/// V128 Columns XYZ Constructor
FORCE_INLINE
HMat33::HMat33( const V128 cx, const V128 cy, const V128 cz ) :
    m_CX( cx ),
    m_CY( cy ),
    m_CZ( cz )
{
}
#elif defined( _SSE_ )
/// V128 Axes XYZ Constructor
FORCE_INLINE
HMat33::HMat33( const V128& ax, const V128& ay, const V128& az ) :
    m_AX( ax ),
    m_AY( ay ),
    m_AZ( az )
{
}
#endif

/// HVec3 Axis X, Y & Z Constructor
FORCE_INLINE
HMat33::HMat33( const HVec3& ax, const HVec3& ay, const HVec3& az )
    #if defined( _SSE_ )
    : m_AX( ax.m_V ),
      m_AY( ay.m_V ),
      m_AZ( az.m_V )
    #endif
{
    #if defined( _VMX128_ )
    const V128 tmp0( __vmrghw( ax.m_V, az.m_V    ) );
    const V128 tmp1( __vmrghw( ay.m_V, __vzero() ) );
    const V128 tmp2( __vmrglw( ax.m_V, az.m_V    ) );
    const V128 tmp3( __vmrglw( ay.m_V, __vzero() ) );
    m_CX = __vmrghw( tmp0, tmp1 );
    m_CY = __vmrglw( tmp0, tmp1 );
    m_CZ = __vmrghw( tmp2, tmp3 );
    #elif !defined( _SSE_ )
    m_A[MX] = ax;
    m_A[MY] = ay;
    m_A[MZ] = az;
    #endif
}

/// Matrix-Scale Constructor
FORCE_INLINE
HMat33::HMat33( const C_MatrixScale& scale )
    #if defined( _VMX128_ )
    : m_CX( __vrlimi( __vzero(), scale.m_Scale.m_V, VMX_MASK_INSERT_X, 0 ) ),
      m_CY( __vrlimi( __vzero(), scale.m_Scale.m_V, VMX_MASK_INSERT_Y, 0 ) ),
      m_CZ( __vrlimi( __vzero(), scale.m_Scale.m_V, VMX_MASK_INSERT_Z, 0 ) )
    #elif defined( _SSE_ )
    : m_AX( HVecX( scale.X() ) ),
      m_AY( HVecY( scale.Y() ) ),
      m_AZ( HVecZ( scale.Z() ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_A[MX] = HVecX( scale.X() );
    m_A[MY] = HVecY( scale.Y() );
    m_A[MZ] = HVecZ( scale.Z() );
    #endif
}

/// Z Axis Constructor
FORCE_INLINE
HMat33::HMat33( const AxisZ& nz )
{
    const HF32              cos45deg( Constants<HF32>::Sqrt2div2() );
    const C_Normal          axisPosX( C_Normal::AxisPositiveX() );
    const C_Normal          axisPosY( C_Normal::AxisPositiveY() );
    const Scalar_Bool  test( Abs( axisPosY | nz ).LE( cos45deg ) );
    const C_Vector          vx1( axisPosY ^ nz );
    const C_Vector          vy2( nz ^ axisPosX );
    const C_Normal          nx1( vx1 );
    const C_Normal          ny1( nz ^ vx1 );
    const C_Normal          ny2( vy2 );
    const C_Normal          nx2( vy2 ^ nz );
    const C_Normal          nx( Select( test, nx1, nx2 ) );
    const C_Normal          ny( Select( test, ny1, ny2 ) );
    SetAxesXYZ( nx, ny, nz );
}

/// X & Y Axes Constructor: Primary X, Secondary Y
FORCE_INLINE
HMat33::HMat33( const AxisX& nx, const AxisY& ny )
{
    const C_Normal nz( nx ^ ny );
    SetAxesXYZ( nx, nz ^ nx, nz );
}

/// X & Z Axes Constructor: Primary X, Secondary Z
FORCE_INLINE
HMat33::HMat33( const AxisX& nx, const AxisZ& nz )
{
    const C_Normal ny( nz ^ nx );
    SetAxesXYZ( nx, ny, HVec3( nx ^ ny ) );
}

/// Y & X Axes Constructor: Primary Y, Secondary X
FORCE_INLINE
HMat33::HMat33( const AxisY& ny, const AxisX& nx )
{
    const C_Normal nz( nx ^ ny );
    SetAxesXYZ( HVec3( ny ^ nz ), ny, nz );
}

/// Y & Z Axes Constructor: Primary Y, Secondary Z
FORCE_INLINE
HMat33::HMat33( const AxisY& ny, const AxisZ& nz )
{
    const C_Normal nx( ny ^ nz );
    SetAxesXYZ( nx, ny, HVec3( nx ^ ny ) );
}

/// Z & X Axes Constructor: Primary Z, Secondary X
FORCE_INLINE
HMat33::HMat33( const AxisZ& nz, const AxisX& nx )
{
    const C_Normal ny( nz ^ nx );
    SetAxesXYZ( HVec3( ny ^ nz ), ny, nz );
}

/// Z & Y Axes Constructor: Primary Z, Secondary Y
FORCE_INLINE
HMat33::HMat33( const AxisZ& nz, const AxisY& ny )
{
    const C_Normal nx( ny ^ nz );
    SetAxesXYZ( nx, HVec3( nz ^ nx ), nz );
}

/// Copy Assignment
FORCE_INLINE
const HMat33& HMat33::operator = ( const HMat33& copy )
{
    #if defined( _VMX128_ )
    m_CX = copy.m_CX;
    m_CY = copy.m_CY;
    m_CZ = copy.m_CZ;
    #elif defined( _SSE_ )
    m_AX.m_V = copy.m_AX.m_V;
    m_AY.m_V = copy.m_AY.m_V;
    m_AZ.m_V = copy.m_AZ.m_V;
    #else
    m_A[MX] = copy.m_A[MX];
    m_A[MY] = copy.m_A[MY];
    m_A[MZ] = copy.m_A[MZ];
    #endif
    return *this;
}

/// Private Accessor: Set Axes X, Y & Z
FORCE_INLINE
void HMat33::SetAxesXYZ( const HVec3& ax, const HVec3& ay, const HVec3& az )
{
    #if defined( _VMX128_ )
    const V128 tmp0( __vmrghw( ax.m_V, az.m_V    ) );
    const V128 tmp1( __vmrghw( ay.m_V, __vzero() ) );
    const V128 tmp2( __vmrglw( ax.m_V, az.m_V    ) );
    const V128 tmp3( __vmrglw( ay.m_V, __vzero() ) );
    m_CX = __vmrghw( tmp0, tmp1 );
    m_CY = __vmrglw( tmp0, tmp1 );
    m_CZ = __vmrghw( tmp2, tmp3 );
    #elif defined( _SSE_ )
    m_AX.m_V = ax.m_V;
    m_AY.m_V = ay.m_V;
    m_AZ.m_V = az.m_V;
    #else
    m_A[MX] = ax;
    m_A[MY] = ay;
    m_A[MZ] = az;
    #endif
}

/// Private Accessor: Set Axes X, Y & Z
#if defined( _VMX128_ )
FORCE_INLINE
void HMat33::SetAxesXYZ( const V128 ax, const V128 ay, const V128 az )
{
    const V128 tmp0( __vmrghw( ax, az        ) );
    const V128 tmp1( __vmrghw( ay, __vzero() ) );
    const V128 tmp2( __vmrglw( ax, az        ) );
    const V128 tmp3( __vmrglw( ay, __vzero() ) );
    m_CX = __vmrghw( tmp0, tmp1 );
    m_CY = __vmrglw( tmp0, tmp1 );
    m_CZ = __vmrghw( tmp2, tmp3 );
}
#elif defined( _SSE_ )
FORCE_INLINE
void HMat33::SetAxesXYZ( const V128& ax, const V128& ay, const V128& az )
{
    m_AX.m_V = ax;
    m_AY.m_V = ay;
    m_AZ.m_V = az;
}
#endif

/// Accessor: Get Axis X
FORCE_INLINE
const HVec3 HMat33::AX() const
{
    #if defined( _VMX128_ )
    return HVec3( __vmrghw( __vmrghw( m_CX, m_CZ ), __vmrghw( m_CY, __vzero() ) ) );
    #elif defined( _SSE_ )
    return m_AX;
    #else
    return m_A[MX];
    #endif
}

/// Accessor: Get Axis Y
FORCE_INLINE
const HVec3 HMat33::AY() const
{
    #if defined( _VMX128_ )
    return HVec3( __vmrglw( __vmrghw( m_CX, m_CZ ), __vmrghw( m_CY, __vzero() ) ) );
    #elif defined( _SSE_ )
    return m_AY;
    #else
    return m_A[MY];
    #endif
}

/// Accessor: Get Axis Z
FORCE_INLINE
const HVec3 HMat33::AZ() const
{
    #if defined( _VMX128_ )
    return HVec3( __vmrghw( __vmrglw( m_CX, m_CZ ), __vmrglw( m_CY, __vzero() ) ) );
    #elif defined( _SSE_ )
    return m_AZ;
    #else
    return m_A[MZ];
    #endif
}

/// Accessor: Set Axis X
FORCE_INLINE
void HMat33::AX( const HVec3& ax )
{
    #if defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, ax.m_V, VMX_MASK_INSERT_X, 0 );
    m_CY = __vrlimi( m_CY, ax.m_V, VMX_MASK_INSERT_X, 1 );
    m_CZ = __vrlimi( m_CZ, ax.m_V, VMX_MASK_INSERT_X, 2 );
    #elif defined( _SSE_ )
    m_AX.m_V = ax.m_V;
    #else
    m_A[MX] = ax;
    #endif
}

/// Accessor: Set Axis Y
FORCE_INLINE
void HMat33::AY( const HVec3& ay )
{
    #if defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, ay.m_V, VMX_MASK_INSERT_Y, 3 );
    m_CY = __vrlimi( m_CY, ay.m_V, VMX_MASK_INSERT_Y, 0 );
    m_CZ = __vrlimi( m_CZ, ay.m_V, VMX_MASK_INSERT_Y, 1 );
    #elif defined( _SSE_ )
    m_AY.m_V = ay.m_V;
    #else
    m_A[MY] = ay;
    #endif
}

/// Accessor: Set Axis Z
FORCE_INLINE
void HMat33::AZ( const HVec3& az )
{
    #if defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, az.m_V, VMX_MASK_INSERT_Z, 2 );
    m_CY = __vrlimi( m_CY, az.m_V, VMX_MASK_INSERT_Z, 3 );
    m_CZ = __vrlimi( m_CZ, az.m_V, VMX_MASK_INSERT_Z, 0 );
    #elif defined( _SSE_ )
    m_AZ.m_V = az.m_V;
    #else
    m_A[MZ] = az;
    #endif
}

#if !defined( _SIMD_ )

/// Const Element Index Accessor
FORCE_INLINE
const HVec3& HMat33::operator [] ( const U32 i ) const
{
    return m_A[i];
}

/// Element Index Accessor
FORCE_INLINE
HVec3& HMat33::operator [] ( const U32 i )
{
    return m_A[i];
}

#endif

/// Set to Identity Matrix
FORCE_INLINE
void HMat33::SetIdentity()
{
    #if defined( _VMX128_ )
    m_CX = c_UnitVectorX;
    m_CY = c_UnitVectorY;
    m_CZ = c_UnitVectorZ;
    #elif defined( _SSE_ )
    m_AX.m_V = c_UnitVectorX;
    m_AY.m_V = c_UnitVectorY;
    m_AZ.m_V = c_UnitVectorZ;
    #else
    SetAxesXYZ( HVec3( c_UnitVectorX ),
                HVec3( c_UnitVectorY ),
                HVec3( c_UnitVectorZ ) );
    #endif
}

/// Transpose of Matrix
FORCE_INLINE
const HMat33 HMat33::Transposed() const
{
    #if defined( _VMX128_ )
    const V128 tmp0( __vmrghw( m_CX, m_CZ      ) );
    const V128 tmp1( __vmrghw( m_CY, __vzero() ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ      ) );
    const V128 tmp3( __vmrglw( m_CY, __vzero() ) );
    const V128 cx  ( __vmrghw( tmp0, tmp1 ) );
    const V128 cy  ( __vmrglw( tmp0, tmp1 ) );
    const V128 cz  ( __vmrghw( tmp2, tmp3 ) );
    return HMat33( cx, cy, cz );
    #elif defined( _SSE_ )
    const V128 zero( SIMD_Zero() );
    const V128 tmp0( SIMD_MergeHigh( m_AX.m_V, m_AZ.m_V ) );
    const V128 tmp1( SIMD_MergeHigh( m_AY.m_V, zero     ) );
    const V128 tmp2( SIMD_MergeLow ( m_AX.m_V, m_AZ.m_V ) );
    const V128 tmp3( SIMD_MergeLow ( m_AY.m_V, zero     ) );
    const V128 ax  ( SIMD_MergeHigh( tmp0, tmp1 ) );
    const V128 ay  ( SIMD_MergeLow ( tmp0, tmp1 ) );
    const V128 az  ( SIMD_MergeHigh( tmp2, tmp3 ) );
    return HMat33( ax, ay, az );
    #else
    const HVec3 ax( m_A[MX][VX], m_A[MY][VX], m_A[MZ][VX] );
    const HVec3 ay( m_A[MX][VY], m_A[MY][VY], m_A[MZ][VY] );
    const HVec3 az( m_A[MX][VZ], m_A[MY][VZ], m_A[MZ][VZ] );
    return HMat33( ax, ay, az );
    #endif
}

/// Transpose this Matrix
FORCE_INLINE
void HMat33::Transpose()
{
    #if defined( _VMX128_ )
    
    const V128 tmp0( __vmrghw( m_CX, m_CZ      ) );
    const V128 tmp1( __vmrghw( m_CY, __vzero() ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ      ) );
    const V128 tmp3( __vmrglw( m_CY, __vzero() ) );
    m_CX = __vmrghw( tmp0, tmp1 );
    m_CY = __vmrglw( tmp0, tmp1 );
    m_CZ = __vmrghw( tmp2, tmp3 );
    
    #elif defined( _SIMD_ )
    
    const V128 zero( SIMD_Zero() );
    const V128 tmp0( SIMD_MergeHigh( m_AX.m_V, m_AZ.m_V ) );
    const V128 tmp1( SIMD_MergeHigh( m_AY.m_V, zero     ) );
    const V128 tmp2( SIMD_MergeLow ( m_AX.m_V, m_AZ.m_V ) );
    const V128 tmp3( SIMD_MergeLow ( m_AY.m_V, zero     ) );
    m_AX.m_V = SIMD_MergeHigh( tmp0, tmp1 );
    m_AY.m_V = SIMD_MergeLow ( tmp0, tmp1 );
    m_AZ.m_V = SIMD_MergeHigh( tmp2, tmp3 );
    
    #else
    
    const HVec3 ax( m_A[MX][VX], m_A[MY][VX], m_A[MZ][VX] );
    const HVec3 ay( m_A[MX][VY], m_A[MY][VY], m_A[MZ][VY] );
    const HVec3 az( m_A[MX][VZ], m_A[MY][VZ], m_A[MZ][VZ] );
    m_A[MX] = ax;
    m_A[MY] = ay;
    m_A[MZ] = az;
    
    #endif
}

/// Scalar Right-Multiplication
FORCE_INLINE
const HMat33 HMat33::operator * ( const HF32& s ) const
{
    #if defined( _VMX128_ )
    return HMat33( __vmulfp( m_CX, s.m_F ),
                   __vmulfp( m_CY, s.m_F ),
                   __vmulfp( m_CZ, s.m_F ) );
    #else
    return HMat33( AX() * s,
                   AY() * s,
                   AZ() * s );
    #endif
}

/// Scalar Left-Multiplication
FORCE_INLINE
const HMat33 operator * ( const HF32& s, const HMat33& m )
{
	return m * s;
}

/// Matrix Addition
FORCE_INLINE
const HMat33 HMat33::operator + ( const HMat33& m ) const
{
    #if defined( _VMX128_ )
    return HMat33( __vaddfp( m_CX, m.m_CX ),
                   __vaddfp( m_CY, m.m_CY ),
                   __vaddfp( m_CZ, m.m_CZ ) );
    #else
    return HMat33( AX() + m.AX(),
                   AY() + m.AY(),
                   AZ() + m.AZ() );
    #endif
}

/// Matrix Subtraction
FORCE_INLINE
const HMat33 HMat33::operator - ( const HMat33& m ) const
{
    #if defined( _VMX128_ )
    return HMat33( __vsubfp( m_CX, m.m_CX ),
                   __vsubfp( m_CY, m.m_CY ),
                   __vsubfp( m_CZ, m.m_CZ ) );
    #else
    return HMat33( AX() - m.AX(),
                   AY() - m.AY(),
                   AZ() - m.AZ() );
    #endif
}

/// Matrix Multiplication
FORCE_INLINE
const HMat33 HMat33::operator * ( const HMat33& m2 ) const
{
    #if defined( _VMX128_ )
    
    const V128 m2_cx( m2.m_CX );
    const V128 m2_cy( m2.m_CY );
    const V128 m2_cz( m2.m_CZ );
    
    const V128 tmp0( __vmrghw( m_CX, m_CZ      ) );
    const V128 tmp1( __vmrghw( m_CY, __vzero() ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ      ) );
    const V128 tmp3( __vmrglw( m_CY, __vzero() ) );
    
    const V128 m1_ax( __vmrghw( tmp0, tmp1 ) );
    const V128 m1_ay( __vmrglw( tmp0, tmp1 ) );
    const V128 m1_az( __vmrghw( tmp2, tmp3 ) );
    
    const V128 result_cx_x( __vmsum3fp( m2_cx, m1_ax ) );
    const V128 result_cx_y( __vmsum3fp( m2_cx, m1_ay ) );
    const V128 result_cx_z( __vmsum3fp( m2_cx, m1_az ) );
    
    const V128 result_cy_x( __vmsum3fp( m2_cy, m1_ax ) );
    const V128 result_cy_y( __vmsum3fp( m2_cy, m1_ay ) );
    const V128 result_cy_z( __vmsum3fp( m2_cy, m1_az ) );
    
    const V128 result_cz_x( __vmsum3fp( m2_cz, m1_ax ) );
    const V128 result_cz_y( __vmsum3fp( m2_cz, m1_ay ) );
    const V128 result_cz_z( __vmsum3fp( m2_cz, m1_az ) );
    
    return HMat33( __vmrghw( __vmrghw( result_cx_x, result_cx_z ), __vmrghw( result_cx_y, __vzero() ) ),
                   __vmrghw( __vmrghw( result_cy_x, result_cy_z ), __vmrghw( result_cy_y, __vzero() ) ),
                   __vmrghw( __vmrghw( result_cz_x, result_cz_z ), __vmrghw( result_cz_y, __vzero() ) ) );
    
    #elif defined( _SSE_ )
    
    return HMat33( m_AX * m2,
                   m_AY * m2,
                   m_AZ * m2 );
    
    #else
    
    return HMat33( m_A[MX] * m2,
                   m_A[MY] * m2,
                   m_A[MZ] * m2 );
    
    #endif
}

/// Matrix * Vector
FORCE_INLINE
const HVec3 HMat33::operator * ( const HVec3& v ) const // Matrix * Vector
{
    #if defined( _VMX128_ )
    
    const V128 tmp0( __vmrghw( m_CX, m_CZ      ) );
    const V128 tmp1( __vmrghw( m_CY, __vzero() ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ      ) );
    const V128 tmp3( __vmrglw( m_CY, __vzero() ) );
    
    const V128 m_ax( __vmrghw( tmp0, tmp1 ) );
    const V128 m_ay( __vmrglw( tmp0, tmp1 ) );
    const V128 m_az( __vmrghw( tmp2, tmp3 ) );
    
    const V128 result_x( __vmsum3fp( m_ax, v.m_V ) );
    const V128 result_y( __vmsum3fp( m_ay, v.m_V ) );
    const V128 result_z( __vmsum3fp( m_az, v.m_V ) );
    
    return HVec3( __vmrghw( __vmrghw( result_x, result_z ), __vmrghw( result_y, __vzero() ) ) );
    
    #elif defined( _SSE_ )
    
    const V128 xxxx( SIMD_DotProduct3( m_AX.m_V, v.m_V ) );
    const V128 yyyy( SIMD_DotProduct3( m_AY.m_V, v.m_V ) );
    const V128 zzzz( SIMD_DotProduct3( m_AZ.m_V, v.m_V ) );
    return HVec3( SIMD_Vector3( xxxx, yyyy, zzzz ) );
    
    #else
    
    const F32 x = m_A[MX][VX] * v[VX] + m_A[MX][VY] * v[VY] + m_A[MX][VZ] * v[VZ]; // Matrix * Vector
    const F32 y = m_A[MY][VX] * v[VX] + m_A[MY][VY] * v[VY] + m_A[MY][VZ] * v[VZ];
    const F32 z = m_A[MZ][VX] * v[VX] + m_A[MZ][VY] * v[VY] + m_A[MZ][VZ] * v[VZ];
    return HVec3( x, y, z );
    
    #endif
}

/// Combine with HVec3 position to form a HMat43
FORCE_INLINE
const HMat43 HMat33::operator || ( const HVec3& pos ) const
{
    #if defined( _VMX128_ )
    return HMat43( __vrlimi( m_CX, pos.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W ),
                   __vrlimi( m_CY, pos.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W ),
                   __vrlimi( m_CZ, pos.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W ) );
    #elif defined( _SSE_ )
    return HMat43( m_AX.m_V, m_AY.m_V, m_AZ.m_V, pos.m_V );
    #else
    return HMat43( m_A[MX], m_A[MY], m_A[MZ], pos );
    #endif
}

/// Inverse of Rotation - Faster but only valid for matrices with normalised & orthogonal rotations!
FORCE_INLINE
const HMat33 HMat33::InverseRotation() const
{
    return Transposed();
}

/// Trace of the matrix (sum of the diagonal elements).
FORCE_INLINE
const HF32 HMat33::Trace() const
{
	return AX().X() + AY().Y() + AZ().Z();
}

/// Matrix Determinant
FORCE_INLINE
const HF32 HMat33::Determinant() const
{
    #if defined( _SIMD_ )
    
    #if defined( _VMX128_ )
    // NOTE: The determinants of a 3x3 matrix and it's transpose are identical!
    const V128 ax( m_CX );
    const V128 ay( m_CY );
    const V128 az( m_CZ );
    #else
    const V128 ax( m_AX.m_V );
    const V128 ay( m_AY.m_V );
    const V128 az( m_AZ.m_V );
    #endif
    
    static const int __ = VW;
    
    const V128 xyz = SIMD_Multiply( ax, SIMD_SubtractMultiply( SIMD_Multiply( SIMD_Permute<VY,VX,VX,__>( ay ), SIMD_Permute<VZ,VZ,VY,__>( az ) ),
                                                                              SIMD_Permute<VZ,VZ,VY,__>( ay ), SIMD_Permute<VY,VX,VX,__>( az ) ) );
    
    return HF32( SIMD_ScalarAdd( SIMD_ScalarSubtract( SIMD_Scalar_Vector3X( xyz ), SIMD_Scalar_Vector3Y( xyz ) ), SIMD_Scalar_Vector3Z( xyz ) ) );
    
    #else
    
    return HF32( m_A[MX][VX] * ( m_A[MY][VY] * m_A[MZ][VZ] - m_A[MY][VZ] * m_A[MZ][VY] ) -
                 m_A[MX][VY] * ( m_A[MY][VX] * m_A[MZ][VZ] - m_A[MY][VZ] * m_A[MZ][VX] ) +
                 m_A[MX][VZ] * ( m_A[MY][VX] * m_A[MZ][VY] - m_A[MY][VY] * m_A[MZ][VX] ) );
    
    #endif
}

/// Normalise matrix: Priority to Z axis, secondary Y
FORCE_INLINE
void HMat33::NormaliseZY()
{
    #if defined( _VMX128_ )
    
    V128 tmp0 = __vmrghw( m_CX, m_CZ      );
    V128 tmp1 = __vmrghw( m_CY, __vzero() );
    V128 tmp2 = __vmrglw( m_CX, m_CZ      );
    V128 tmp3 = __vmrglw( m_CY, __vzero() );
    
    V128 m_ax = __vmrghw( tmp0, tmp1 );
    V128 m_ay = __vmrglw( tmp0, tmp1 );
    V128 m_az = __vmrghw( tmp2, tmp3 );
    
    m_az = SIMD_Normalise3( m_az );
    m_ax = SIMD_Normalise3( SIMD_CrossProduct( m_ay, m_az ) );
    m_ay = SIMD_CrossProduct( m_az, m_ax );
    
    tmp0 = __vmrghw( m_ax, m_az      );
    tmp1 = __vmrghw( m_ay, __vzero() );
    tmp2 = __vmrglw( m_ax, m_az      );
    tmp3 = __vmrglw( m_ay, __vzero() );
    
    m_CX = __vmrghw( tmp0, tmp1 );
    m_CY = __vmrglw( tmp0, tmp1 );
    m_CZ = __vmrghw( tmp2, tmp3 );
    
    #elif defined( _SSE_ )
    
    m_AZ.m_V = SIMD_Normalise3( m_AZ.m_V );
    m_AX.m_V = SIMD_Normalise3( SIMD_CrossProduct( m_AY.m_V, m_AZ.m_V ) );
    m_AY.m_V = SIMD_CrossProduct( m_AZ.m_V, m_AX.m_V );
    
    #else
    
    const HVec3 az( (HVec3)C_Normal( m_A[MZ] ) );
    const HVec3 ax( (HVec3)C_Normal( m_A[MY] ^ az ) );
    const HVec3 ay( az ^ ax );
    m_A[MX] = ax;
    m_A[MY] = ay;
    m_A[MZ] = az;
    
    #endif
}
    
/// Normalised copy of matrix: Priority to Z axis, secondary Y
FORCE_INLINE
const HMat33 HMat33::NormalisedZY() const
{
    #if defined( _VMX128_ )
    
    V128 tmp0 = __vmrghw( m_CX, m_CZ      );
    V128 tmp1 = __vmrghw( m_CY, __vzero() );
    V128 tmp2 = __vmrglw( m_CX, m_CZ      );
    V128 tmp3 = __vmrglw( m_CY, __vzero() );
    
    V128 ax = __vmrghw( tmp0, tmp1 );
    V128 ay = __vmrglw( tmp0, tmp1 );
    V128 az = __vmrghw( tmp2, tmp3 );
    
    az = SIMD_Normalise3( az );
    ax = SIMD_Normalise3( SIMD_CrossProduct( ay, az ) );
    ay = SIMD_CrossProduct( az, ax );
    
    tmp0 = __vmrghw( ax, az        );
    tmp1 = __vmrghw( ay, __vzero() );
    tmp2 = __vmrglw( ax, az        );
    tmp3 = __vmrglw( ay, __vzero() );
    
    ax = __vmrghw( tmp0, tmp1 );
    ay = __vmrglw( tmp0, tmp1 );
    az = __vmrghw( tmp2, tmp3 );
    
    #elif defined( _SSE_ )
    
    const V128 az( SIMD_Normalise3( m_AZ.m_V ) );
    const V128 ax( SIMD_Normalise3( SIMD_CrossProduct( m_AY.m_V, az ) ) );
    const V128 ay( SIMD_CrossProduct( az, ax ) );
    
    #else
    
    const HVec3 az( (HVec3)C_Normal( m_A[MZ] ) );
    const HVec3 ax( (HVec3)C_Normal( m_A[MY] ^ az ) );
    const HVec3 ay( az ^ ax );
    
    #endif
    
    return HMat33( ax, ay, az );
}

/// Finite test
FORCE_INLINE
const Scalar_Bool HMat33::Finite() const
{
    #if defined( _VMX128_ )
    return HVec3( m_CX ).Finite() &
           HVec3( m_CY ).Finite() &
           HVec3( m_CZ ).Finite();
    #elif defined( _SSE_ )
    return m_AX.Finite() &
           m_AY.Finite() &
           m_AZ.Finite();
    #else
    return m_A[MX].Finite() &
           m_A[MY].Finite() &
           m_A[MZ].Finite();
    #endif
}

template< typename T_Stream > 
void HMat33::Deserialise( T_Stream& stream )
{
    HVec3 ax;
    HVec3 ay;
    HVec3 az;
    stream >> ax;
    stream >> ay;
    stream >> az;
    AX(ax);
    AY(ay);
    AZ(az);
}

template< typename T_Stream > 
void HMat33::Serialise( T_Stream& stream )
{
    AX().Serialise(stream);
    AY().Serialise(stream);
    AZ().Serialise(stream);
}












//
// -------------------------------------------------------
// |-- HMat43 : Hardware 4 Row * 3 Column Matrix Class --|
// -------------------------------------------------------
//

// Cross products for deriving third matrix axis from 2 input axes
// const HVec3 ax = ay ^ az;
// const HVec3 ay = az ^ ax;
// const HVec3 az = ax ^ ay;

/// Default Constructor: Initialised to Identity
FORCE_INLINE
HMat43::HMat43()
    #if defined( _VMX128_ )
    : m_CX( c_UnitVectorX ),
      m_CY( c_UnitVectorY ),
      m_CZ( c_UnitVectorZ )
    #endif
{
}

/// Undefined Constructor
NOALIAS __forceinline HMat43::HMat43( const _UNDEFINED undefined )
{
}

/// Explicit HMat33 Constructor
FORCE_INLINE
HMat43::HMat43( const HMat33& src ) :
    #if defined( _VMX128_ )
    m_CX( __vrlimi( src.m_CX, __vzero(), VMX_MASK_INSERT_W, VMX_ROTATE_NONE ) ),
    m_CY( __vrlimi( src.m_CY, __vzero(), VMX_MASK_INSERT_W, VMX_ROTATE_NONE ) ),
    m_CZ( __vrlimi( src.m_CZ, __vzero(), VMX_MASK_INSERT_W, VMX_ROTATE_NONE ) )
    #else
    m_Rotate( src )
    #endif
{
}

/// Copy Constructor
FORCE_INLINE
HMat43::HMat43( const HMat43& copy ) :
    #if defined( _VMX128_ )
    m_CX( copy.m_CX ),
    m_CY( copy.m_CY ),
    m_CZ( copy.m_CZ )
    #else
    m_Rotate( copy.m_Rotate ),
    m_Translate( copy.m_Translate )
    #endif
{
}

// Explicit HMat44 Constructor
FORCE_INLINE
HMat43::HMat43( const HMat44& src ) :
    #if defined( _VMX128_ )
    m_CX( src.m_CX ),
    m_CY( src.m_CY ),
    m_CZ( src.m_CZ )
    #elif defined( _SSE_ )
    m_Rotate(    SIMD_AND( src.m_AX.m_V, SIMD_Mask<1,1,1,0>() ),
                 SIMD_AND( src.m_AY.m_V, SIMD_Mask<1,1,1,0>() ),
                 SIMD_AND( src.m_AZ.m_V, SIMD_Mask<1,1,1,0>() ) ),
    m_Translate( SIMD_AND( src.m_AW.m_V, SIMD_Mask<1,1,1,0>() ) )
    #else
    m_Rotate(    HVec3( src.m_A[MX] ),
                 HVec3( src.m_A[MY] ),
                 HVec3( src.m_A[MZ] ) ),
    m_Translate( HVec3( src.m_A[MW] ) )
    #endif
{
}

/// Rotation + Translation Constructor
FORCE_INLINE
HMat43::HMat43( const HMat33& rotate, const HVec3& translate ) :
    #if defined( _VMX128_ )
    m_CX( __vrlimi( rotate.m_CX, translate.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W ) ),
    m_CY( __vrlimi( rotate.m_CY, translate.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W ) ),
    m_CZ( __vrlimi( rotate.m_CZ, translate.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W ) )
    #else
    m_Rotate( rotate ),
    m_Translate( translate )
    #endif
{
}

#if defined( _VMX128_ )
/// V128 Column XYZW Constructor
FORCE_INLINE
HMat43::HMat43( const V128 cx, const V128 cy, const V128 cz ) :
    m_CX( cx ),
    m_CY( cy ),
    m_CZ( cz )
{
}
#elif defined( _SSE_ )
/// V128 Axis XYZW Constructor
FORCE_INLINE
HMat43::HMat43( const V128& ax, const V128& ay, const V128& az, const V128& aw ) :
    m_Rotate( ax, ay, az ),
    m_Translate( aw )
{
}
#endif

/// HVec3 Axis XYZW Constructor
FORCE_INLINE
HMat43::HMat43( const HVec3& ax, const HVec3& ay, const HVec3& az, const HVec3& aw )
    #if !defined( _VMX128_ )
    : m_Rotate( ax, ay, az ),
      m_Translate( aw )
    #endif
{
    #if defined( _VMX128_ )
    const V128 tmp0( __vmrghw( ax.m_V, az.m_V ) );
    const V128 tmp1( __vmrghw( ay.m_V, aw.m_V ) );
    const V128 tmp2( __vmrglw( ax.m_V, az.m_V ) );
    const V128 tmp3( __vmrglw( ay.m_V, aw.m_V ) );
    m_CX = __vmrghw( tmp0, tmp1 );
    m_CY = __vmrglw( tmp0, tmp1 );
    m_CZ = __vmrghw( tmp2, tmp3 );
    #endif
}

/// Constructor: Primary axis X, secondary axis Y
FORCE_INLINE
HMat43::HMat43( const AxisX& ax, const AxisY& ay, const HVec3& aw )
    #if !defined( _VMX128_ )
    : m_Rotate( ax, ay ),
      m_Translate( aw )
    #endif
{
    #if defined( _VMX128_ )
    const HMat33 rotate( ax, ay );
    m_CX = __vrlimi( rotate.m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( rotate.m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( rotate.m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #endif
}

/// Constructor: Primary axis X, secondary axis Z
FORCE_INLINE
HMat43::HMat43( const AxisX& ax, const AxisZ& az, const HVec3& aw )
    #if !defined( _VMX128_ )
    : m_Rotate( ax, az ),
      m_Translate( aw )
    #endif
{
    #if defined( _VMX128_ )
    const HMat33 rotate( ax, az );
    m_CX = __vrlimi( rotate.m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( rotate.m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( rotate.m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #endif
}

/// Constructor: Primary axis Y, secondary axis X
FORCE_INLINE
HMat43::HMat43( const AxisY& ay, const AxisX& ax, const HVec3& aw )
    #if !defined( _VMX128_ )
    : m_Rotate( ay, ax ),
      m_Translate( aw )
    #endif
{
    #if defined( _VMX128_ )
    const HMat33 rotate( ay, ax );
    m_CX = __vrlimi( rotate.m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( rotate.m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( rotate.m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #endif
}

/// Constructor: Primary axis Y, secondary axis Z
FORCE_INLINE
HMat43::HMat43( const AxisY& ay, const AxisZ& az, const HVec3& aw )
    #if !defined( _VMX128_ )
    : m_Rotate( ay, az ),
      m_Translate( aw )
    #endif
{
    #if defined( _VMX128_ )
    const HMat33 rotate( ay, az );
    m_CX = __vrlimi( rotate.m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( rotate.m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( rotate.m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #endif
}

/// Constructor: Primary axis Z, secondary axis X
FORCE_INLINE
HMat43::HMat43( const AxisZ& az, const AxisX& ax, const HVec3& aw )
    #if !defined( _VMX128_ )
    : m_Rotate( az, ax ),
      m_Translate( aw )
    #endif
{
    #if defined( _VMX128_ )
    const HMat33 rotate( az, ax );
    m_CX = __vrlimi( rotate.m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( rotate.m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( rotate.m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #endif
}

/// Constructor: Primary axis Z, secondary axis Y
FORCE_INLINE
HMat43::HMat43( const AxisZ& az, const AxisY& ay, const HVec3& aw )
    #if !defined( _VMX128_ )
    : m_Rotate( az, ay ),
      m_Translate( aw )
    #endif
{
    #if defined( _VMX128_ )
    const HMat33 rotate( az, ay );
    m_CX = __vrlimi( rotate.m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( rotate.m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( rotate.m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #endif
}

/// Scale-Rotate-Translate Constructor
FORCE_INLINE
HMat43::HMat43( const C_MatrixSRT& srt )
    #if !defined( _VMX128_ )
    : m_Rotate( HMat33( srt.Scale() ) * HMat33( srt.Rotate() ) ),
      m_Translate( srt.Translate() )
    #endif
{
    #if defined( _VMX128_ )
    const HMat33 rotate( HMat33( srt.Scale() ) * HMat33( srt.Rotate() ) );
    const V128 aw( srt.Translate().m_Translate.m_V );
    m_CX = __vrlimi( rotate.m_CX, aw, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( rotate.m_CY, aw, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( rotate.m_CZ, aw, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #endif
}

#if ( DIRECT3D_VERSION >= 0x0900 )

/// Convert to D3DMATRIX
FORCE_INLINE
HMat43::operator const D3DMATRIX () const
{
    return (D3DMATRIX)HMat44( *this );
}

/// Convert to D3DXMATRIX
FORCE_INLINE
HMat43::operator const D3DXMATRIX () const
{
    return (D3DXMATRIX)HMat44( *this );
}

#endif

/// Accessor: Get Rotation
FORCE_INLINE
const HMat33 HMat43::Rotation() const
{
    #if defined( _VMX128_ )
    return HMat33( __vrlimi( m_CX, __vzero(), VMX_MASK_INSERT_W, VMX_ROTATE_NONE ),
                   __vrlimi( m_CY, __vzero(), VMX_MASK_INSERT_W, VMX_ROTATE_NONE ),
                   __vrlimi( m_CZ, __vzero(), VMX_MASK_INSERT_W, VMX_ROTATE_NONE ) );
    #else
    return m_Rotate;
    #endif
}

/// Accessor: Get Translation
FORCE_INLINE
const HVec3 HMat43::Translation() const
{
    #if defined( _VMX128_ )
    return HVec3( __vmrglw( __vmrglw( m_CX, m_CZ ), __vmrglw( m_CY, __vzero() ) ) );
    #else
    return m_Translate;
    #endif
}

/// Accessor: Set Rotation
FORCE_INLINE
void HMat43::Rotation( const HMat33& rotation )
{
    #if defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, rotation.m_CX, VMX_MASK_INSERT_XYZ, VMX_ROTATE_NONE );
    m_CY = __vrlimi( m_CY, rotation.m_CY, VMX_MASK_INSERT_XYZ, VMX_ROTATE_NONE );
    m_CZ = __vrlimi( m_CZ, rotation.m_CZ, VMX_MASK_INSERT_XYZ, VMX_ROTATE_NONE );
    #elif defined( _SSE_ )
    m_Rotate.m_AX.m_V = rotation.m_AX.m_V;
    m_Rotate.m_AY.m_V = rotation.m_AY.m_V;
    m_Rotate.m_AZ.m_V = rotation.m_AZ.m_V;
    #else
    m_Rotate = rotation;
    #endif
}

/// Accessor: Set Translation
FORCE_INLINE
void HMat43::Translation( const HVec3& translation )
{
    #if defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, translation.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( m_CY, translation.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( m_CZ, translation.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #elif defined( _SSE_ )
    m_Translate.m_V = translation.m_V;
    #else
    m_Translate = translation;
    #endif
}

/// Accessor: Get Axis X
FORCE_INLINE
const HVec3 HMat43::AX() const
{
    #if defined( _VMX128_ )
    return HVec3( __vmrghw( __vmrghw( m_CX, m_CZ ), __vmrghw( m_CY, __vzero() ) ) );
    #elif defined( _SSE_ )
    return m_Rotate.m_AX;
    #else
    return m_Rotate.AX();
    #endif
}

/// Accessor: Get Axis Y
FORCE_INLINE
const HVec3 HMat43::AY() const
{
    #if defined( _VMX128_ )
    return HVec3( __vmrglw( __vmrghw( m_CX, m_CZ ), __vmrghw( m_CY, __vzero() ) ) );
    #elif defined( _SSE_ )
    return m_Rotate.m_AY;
    #else
    return m_Rotate.AY();
    #endif
}

/// Accessor: Get Axis Z
FORCE_INLINE
const HVec3 HMat43::AZ() const
{
    #if defined( _VMX128_ )
    return HVec3( __vmrghw( __vmrglw( m_CX, m_CZ ), __vmrglw( m_CY, __vzero() ) ) );
    #elif defined( _SSE_ )
    return m_Rotate.m_AZ;
    #else
    return m_Rotate.AZ();
    #endif
}

/// Accessor: Get Axis W
FORCE_INLINE
const HVec3 HMat43::AW() const
{
    #if defined( _VMX128_ )
    return HVec3( __vmrglw( __vmrglw( m_CX, m_CZ ), __vmrglw( m_CY, __vzero() ) ) );
    #else
    return m_Translate;
    #endif
}

/// Accessor: Get All Axes
FORCE_INLINE
void HMat43::GetAxes(HVec3&	 dstAX,HVec3&	 dstAY,HVec3&	 dstAZ,HVec3&	 dstAW) const __restrict
{

    #if defined( _VMX128_ )

    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, c_UnitVectorW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, c_UnitVectorW ) );
	dstAX.SetV128(__vmrghw( tmp0, tmp1 ));
    dstAY.SetV128(__vmrglw( tmp0, tmp1 ));
    dstAZ.SetV128(__vmrghw( tmp2, tmp3 ));
    dstAW.SetV128(__vmrglw( tmp2, tmp3 ));

	#else

	dstAX = this->AX(); dstAY = this->AY(); dstAZ = this->AZ(); dstAW = this->AW();

    #endif
}

//#if 0

/// Accessor: Get All Axes
FORCE_INLINE
void HMat43::GetAxes(HVec4&	 dstAX,HVec4&	 dstAY,HVec4&	 dstAZ,HVec4&	 dstAW) const __restrict
{

    #if defined( _VMX128_ )

    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, c_UnitVectorW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, c_UnitVectorW ) );
	dstAX.SetV128(__vmrghw( tmp0, tmp1 ));
    dstAY.SetV128(__vmrglw( tmp0, tmp1 ));
    dstAZ.SetV128(__vmrghw( tmp2, tmp3 ));
    dstAW.SetV128(__vmrglw( tmp2, tmp3 ));

	#else

	dstAX = this->AX();
	dstAY = this->AY();
	dstAZ = this->AZ();
	dstAW = this->AW();
//#error TODO -....

    #endif
}

/// Accessor: Get All Axes
FORCE_INLINE
void HMat33::GetAxes(
			HVec4&	 dstAX,
			HVec4&	 dstAY,
			HVec4&	 dstAZ
			) const __restrict
{
    #if defined( _VMX128_ )

    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, c_UnitVectorW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, c_UnitVectorW ) );
	dstAX.SetV128(__vmrghw( tmp0, tmp1 ));
    dstAY.SetV128(__vmrglw( tmp0, tmp1 ));
    dstAZ.SetV128(__vmrghw( tmp2, tmp3 ));
    //dstAW.SetV128(__vmrglw( tmp2, tmp3 ));
	#else
//#error TODO -....
	dstAX = this->AX();
	dstAY = this->AY();
	dstAZ = this->AZ();
    #endif
}

FORCE_INLINE
void HMat33::GetAxes(
			HVec3&	 dstAX,
			HVec3&	 dstAY,
			HVec3&	 dstAZ
			) const __restrict
{
    #if defined( _VMX128_ )

    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, c_UnitVectorW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, c_UnitVectorW ) );
	dstAX.SetV128(__vmrghw( tmp0, tmp1 ));
    dstAY.SetV128(__vmrglw( tmp0, tmp1 ));
    dstAZ.SetV128(__vmrghw( tmp2, tmp3 ));
    //dstAW.SetV128(__vmrglw( tmp2, tmp3 ));
	#else
	dstAX = this->AX();
	dstAY = this->AY();
	dstAZ = this->AZ();

    #endif
}

/// Set All Axes (todo .. version from HVec3 makes more sense too)
FORCE_INLINE
void	HMat43::SetAxes(const HVec4&	 srcAX,const HVec4&	 srcAY,const HVec4&	 srcAZ,const HVec4&	 srcAW) __restrict
{

    #if defined( _VMX128_ )

    const V128 tmp0( __vmrghw( srcAX, srcAZ )			);
    const V128 tmp1( __vmrghw( srcAY, srcAW )	);
    const V128 tmp2( __vmrglw( srcAX, srcAZ )			);
    const V128 tmp3( __vmrglw( srcAY, srcAW )	);

	m_CX	=	__vmrghw( tmp0, tmp1 );
    m_CY	=	__vmrglw( tmp0, tmp1 );
    m_CZ	=	__vmrghw( tmp2, tmp3 );
    //dstAW	=	__vmrglw( tmp2, tmp3 );

	#else

	// TODO, this just wants to be permutes on the ps3 aswell.
	// we need to clarify if the matrix is staying the same or not.

	this->AX(HVec3(srcAX));
	this->AY(HVec3(srcAY));
	this->AZ(HVec3(srcAZ));
	this->AW(HVec3(srcAW));


    #endif
}

/// Set All Axes
FORCE_INLINE
void	HMat43::SetAxesScaled(const HVec4&	 srcAX,const HVec4&	 srcAY,const HVec4&	 srcAZ,const HVec4&	 srcAW, const HVec4&  scaleXYZ) __restrict
{

#if defined( _VMX128_ )

    const V128 tmp0( __vmrghw( srcAX, srcAZ )			);
    const V128 tmp1( __vmrghw( srcAY, srcAW )	);
    const V128 tmp2( __vmrglw( srcAX, srcAZ )			);
    const V128 tmp3( __vmrglw( srcAY, srcAW )	);

	V128 scaleXYZ1 = __vrlimi(scaleXYZ, c_UnitVectorW, VMX_MASK_INSERT_W, 0);
	m_CX	=	__vmrghw( tmp0, tmp1 ) * scaleXYZ1;
    m_CY	=	__vmrglw( tmp0, tmp1 ) * scaleXYZ1;
    m_CZ	=	__vmrghw( tmp2, tmp3 ) * scaleXYZ1;

	#else


//#error TODO -....
	// TODO - Verify this is correct and that the scale should not be transposed. 
	this->AX( HVec3(srcAX * scaleXYZ.X()) );
	this->AY( HVec3(srcAY * scaleXYZ.Y()) );
	this->AZ( HVec3(srcAZ * scaleXYZ.Z()) );
	this->AW( HVec3(srcAW) );

    #endif
}

/// Accessor: Get All Axes
FORCE_INLINE
void	HMat44::GetAxes(HVec4&	 dstAX,HVec4&	 dstAY,HVec4&	 dstAZ,HVec4&	 dstAW) const __restrict
{
	// TODO - NEED JASON TO SORT THIS..
	// which macros correspond to matrix being transposed
	// under what circumstances is a HMat43 really a HMat44
	// TODO - VERIFY WITH JASON THAT THE HMAT43 IS EVEN THE OPTIMUM SKINNING 
	// TEMPORARY... IT CONVERTS TO A Mat33 + Vec3 position!!!! Surely thats wrong :(
	// I THINK WE NEED EXPLICIT D3D STYLE MATRIX ALL THROUGH THE RENDERING PIPELINE 
	// FOR SKELETON CODE

    #if defined( _VMX128_ )

    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, m_CW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, m_CW ) );
	dstAX.SetV128(__vmrghw( tmp0, tmp1 ));
    dstAY.SetV128(__vmrglw( tmp0, tmp1 ));
    dstAY.SetV128(__vmrghw( tmp2, tmp3 ));
    dstAW.SetV128(__vmrglw( tmp2, tmp3 ));

	#else


//#error TODO -....
	dstAX = this->AX();
	dstAY = this->AY();
	dstAZ = this->AZ();
	dstAW = this->AW();

    #endif
}

//#endif

//const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
//const V128 tmp1( __vmrghw( m_CY, __vzero() ) );
//const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
//const V128 tmp3( __vmrglw( m_CY, __vzero() ) );
//ax = __vmrghw( tmp0, tmp1 );
//ay = __vmrglw( tmp0, tmp1 );
//az = __vmrghw( tmp2, tmp3 );
//aw = __vmrglw( tmp2, tmp3 );

/// Accessor: Set Axis X
FORCE_INLINE
void HMat43::AX( const HVec3& ax )
{
    #if defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, ax.m_V, VMX_MASK_INSERT_X, VMX_ROTATE_X_TO_X );
    m_CY = __vrlimi( m_CY, ax.m_V, VMX_MASK_INSERT_X, VMX_ROTATE_Y_TO_X );
    m_CZ = __vrlimi( m_CZ, ax.m_V, VMX_MASK_INSERT_X, VMX_ROTATE_Z_TO_X );
    #elif defined( _SSE_ )
    m_Rotate.m_AX.m_V = ax.m_V;
    #else
    m_Rotate.AX( ax );
    #endif
}

/// Accessor: Set Axis Y
FORCE_INLINE
void HMat43::AY( const HVec3& ay )
{
    #if defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, ay.m_V, VMX_MASK_INSERT_Y, VMX_ROTATE_X_TO_Y );
    m_CY = __vrlimi( m_CY, ay.m_V, VMX_MASK_INSERT_Y, VMX_ROTATE_Y_TO_Y );
    m_CZ = __vrlimi( m_CZ, ay.m_V, VMX_MASK_INSERT_Y, VMX_ROTATE_Z_TO_Y );
    #elif defined( _SSE_ )
    m_Rotate.m_AY.m_V = ay.m_V;
    #else
    m_Rotate.AY( ay );
    #endif
}

/// Accessor: Set Axis Z
FORCE_INLINE
void HMat43::AZ( const HVec3& az )
{
    #if defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, az.m_V, VMX_MASK_INSERT_Z, VMX_ROTATE_X_TO_Z );
    m_CY = __vrlimi( m_CY, az.m_V, VMX_MASK_INSERT_Z, VMX_ROTATE_Y_TO_Z );
    m_CZ = __vrlimi( m_CZ, az.m_V, VMX_MASK_INSERT_Z, VMX_ROTATE_Z_TO_Z );
    #elif defined( _SSE_ )
    m_Rotate.m_AZ.m_V = az.m_V;
    #else
    m_Rotate.AZ( az );
    #endif
}

/// Accessor: Set Axis W
FORCE_INLINE
void HMat43::AW( const HVec3& aw )
{
    #if defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #elif defined( _SSE_ )
    m_Translate.m_V = aw.m_V;
    #else
    m_Translate = aw;
    #endif
}

#if !defined( _SIMD_ )

/// Accessor: Const Axis Indexed
FORCE_INLINE
const HVec3& HMat43::operator [] ( const U32 i ) const
{
    assert( i < 4 );
    return ((HVec3*)this)[i];
}

/// Accessor: Axis Indexed
FORCE_INLINE
HVec3& HMat43::operator [] ( const U32 i )
{
    assert( i < 4 );
    return ((HVec3*)this)[i];
}

#endif

/// HMat33 Assignment
FORCE_INLINE
const HMat43& HMat43::operator = ( const HMat33& src )
{
    #if defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, src.m_CX, VMX_MASK_INSERT_XYZ, VMX_ROTATE_NONE );
    m_CY = __vrlimi( m_CY, src.m_CY, VMX_MASK_INSERT_XYZ, VMX_ROTATE_NONE );
    m_CZ = __vrlimi( m_CZ, src.m_CZ, VMX_MASK_INSERT_XYZ, VMX_ROTATE_NONE );
    #elif defined( _SSE_ )
    m_Rotate.m_AX.m_V = src.m_AX.m_V;
    m_Rotate.m_AY.m_V = src.m_AY.m_V;
    m_Rotate.m_AZ.m_V = src.m_AZ.m_V;
    m_Translate.m_V   = c_VectorZero;
    #else
    m_Rotate    = src;
    m_Translate = HVec3( c_VectorZero );
    #endif
    return *this;
}

/// Copy Assignment
FORCE_INLINE
const HMat43& HMat43::operator = ( const HMat43& copy )
{
    #if defined( _VMX128_ )
    m_CX = copy.m_CX;
    m_CY = copy.m_CY;
    m_CZ = copy.m_CZ;
    #elif defined( _SSE_ )
    m_Rotate.m_AX.m_V = copy.m_Rotate.m_AX.m_V;
    m_Rotate.m_AY.m_V = copy.m_Rotate.m_AY.m_V;
    m_Rotate.m_AZ.m_V = copy.m_Rotate.m_AZ.m_V;
    m_Translate.m_V   = copy.m_Translate.m_V;
    #else
    m_Rotate    = copy.m_Rotate;
    m_Translate = copy.m_Translate;
    #endif
    return *this;
}

/// Set to Identity Matrix
FORCE_INLINE
void HMat43::SetIdentity()
{
    #if defined( _VMX128_ )
    m_CX = c_UnitVectorX;
    m_CY = c_UnitVectorY;
    m_CZ = c_UnitVectorZ;
    #else
    m_Rotate.SetIdentity();
    m_Translate = HVec3( c_VectorZero );
    #endif
}

/// Matrix Multiply
FORCE_INLINE
const HMat43 HMat43::operator * ( const HMat43& m2 ) const
{
    #if defined( _VMX128_ )
    
    const V128 unit_w( c_UnitVectorW );
    
    const V128 m2_cx( m2.m_CX );
    const V128 m2_cy( m2.m_CY );
    const V128 m2_cz( m2.m_CZ );
    
    const V128 tmp0( __vmrghw( m_CX, m_CZ   ) );
    const V128 tmp1( __vmrghw( m_CY, unit_w ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ   ) );
    const V128 tmp3( __vmrglw( m_CY, unit_w ) );
    
    const V128 m1_ax( __vmrghw( tmp0, tmp1 ) );
    const V128 m1_ay( __vmrglw( tmp0, tmp1 ) );
    const V128 m1_az( __vmrghw( tmp2, tmp3 ) );
    const V128 m1_aw( __vmrglw( tmp2, tmp3 ) );
    
    const V128 result_cx_x( __vmsum4fp( m2_cx, m1_ax ) );
    const V128 result_cx_y( __vmsum4fp( m2_cx, m1_ay ) );
    const V128 result_cx_z( __vmsum4fp( m2_cx, m1_az ) );
    const V128 result_cx_w( __vmsum4fp( m2_cx, m1_aw ) );
    
    const V128 result_cy_x( __vmsum4fp( m2_cy, m1_ax ) );
    const V128 result_cy_y( __vmsum4fp( m2_cy, m1_ay ) );
    const V128 result_cy_z( __vmsum4fp( m2_cy, m1_az ) );
    const V128 result_cy_w( __vmsum4fp( m2_cy, m1_aw ) );
    
    const V128 result_cz_x( __vmsum4fp( m2_cz, m1_ax ) );
    const V128 result_cz_y( __vmsum4fp( m2_cz, m1_ay ) );
    const V128 result_cz_z( __vmsum4fp( m2_cz, m1_az ) );
    const V128 result_cz_w( __vmsum4fp( m2_cz, m1_aw ) );
    
    return HMat43( __vmrghw( __vmrghw( result_cx_x, result_cx_z ), __vmrghw( result_cx_y, result_cx_w ) ),
                   __vmrghw( __vmrghw( result_cy_x, result_cy_z ), __vmrghw( result_cy_y, result_cy_w ) ),
                   __vmrghw( __vmrghw( result_cz_x, result_cz_z ), __vmrghw( result_cz_y, result_cz_w ) ) );
    
    #else
    
    return HMat43( m_Rotate * m2.m_Rotate, m_Translate * m2 );
    
    #endif
}

/// Inverse of Rotation & Translation - Faster but only valid for matrices with normalised & orthogonal rotations!
FORCE_INLINE
const HMat43 HMat43::InverseRotTrans() const
{
    #if defined( _VMX128_ )
    
    const V128 zero( __vzero() );
    //const V128 vec3_sign_mask( c_Vector3_SignMask );
    const V128 vec_sign_mask( c_VectorSignMask );
    
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, zero ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, zero ) );
    
    const V128 t_cx( __vmrghw( tmp0, tmp1 ) );
    const V128 t_cy( __vmrglw( tmp0, tmp1 ) );
    const V128 t_cz( __vmrghw( tmp2, tmp3 ) );
    const V128 t_cw( __vmrglw( tmp2, tmp3 ) );
    
    //const V128 neg_trans( __vxor( t_cw, vec3_sign_mask ) );
    const V128 neg_trans( __vxor( t_cw, vec_sign_mask ) );
    
    const V128 inv_trans_x( __vmsum3fp( neg_trans, t_cx ) );
    const V128 inv_trans_y( __vmsum3fp( neg_trans, t_cy ) );
    const V128 inv_trans_z( __vmsum3fp( neg_trans, t_cz ) );
    
    const V128 inv_trans( __vmrghw( __vmrghw( inv_trans_x, inv_trans_z ), __vmrghw( inv_trans_y, zero ) ) );
    
    return HMat43( __vrlimi( t_cx, inv_trans, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W ),
                   __vrlimi( t_cy, inv_trans, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W ),
                   __vrlimi( t_cz, inv_trans, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W ) );
    
    #else
    
    const HMat33 inverseRotation( m_Rotate.Transposed() );
    
    return HMat43( inverseRotation.AX(),
                   inverseRotation.AY(),
                   inverseRotation.AZ(),
                   -m_Translate * inverseRotation );
    
    #endif
}

/// Get Scale-Rotate-Translate
FORCE_INLINE
const C_MatrixSRT HMat43::GetSRT() const
{
    const HMat33 scaleRotateMatrix( *this );
    
    const C_MatrixRotateRPH rotate( scaleRotateMatrix.NormalisedZY().GetRotationRPH() );
    
    const HMat33 rotateMatrix( rotate );
    
    #if defined( _VMX128_ )
    
    const HMat33 transposedScaleMatrix( rotateMatrix * scaleRotateMatrix.Transposed() );
    
    const HVec3 sx( transposedScaleMatrix.m_CX );
    const HVec3 sy( transposedScaleMatrix.m_CY );
    const HVec3 sz( transposedScaleMatrix.m_CZ );
    
    const C_MatrixScale scale( Length( sx ), Length( sy ), Length( sz ) );
    
    const C_MatrixTranslate translate( AW() );
    
    #elif defined( _SSE_ )
    
    const HMat33 scaleMatrix( scaleRotateMatrix * rotateMatrix.Transposed() );
    
    const C_MatrixScale scale( Length( scaleMatrix.m_AX ),
                               Length( scaleMatrix.m_AY ),
                               Length( scaleMatrix.m_AZ ) );
    
    const C_MatrixTranslate translate( m_Translate );
    
    #else
    
    const HMat33 scaleMatrix( scaleRotateMatrix * rotateMatrix.Transposed() );
    
    const C_MatrixScale scale( Length( scaleMatrix.m_A[MX] ),
                               Length( scaleMatrix.m_A[MY] ),
                               Length( scaleMatrix.m_A[MZ] ) );
    
    const C_MatrixTranslate translate( m_Translate );
    
    #endif
    
    return C_MatrixSRT( scale, rotate, translate );
}

template< typename T_Stream > 
FORCE_INLINE
void HMat43::Deserialise( T_Stream& stream )
{
    #if defined( _VMX128_ )
    HVec3 ax, ay, az, aw;
    stream >> ax;
    stream >> ay;
    stream >> az;
    stream >> aw;
    const V128 tmp0( __vmrghw( ax.m_V, az.m_V ) );
    const V128 tmp1( __vmrghw( ay.m_V, aw.m_V ) );
    const V128 tmp2( __vmrglw( ax.m_V, az.m_V ) );
    const V128 tmp3( __vmrglw( ay.m_V, aw.m_V ) );
    m_CX = __vmrghw( tmp0, tmp1 );
    m_CY = __vmrglw( tmp0, tmp1 );
    m_CZ = __vmrghw( tmp2, tmp3 );
    #else
    stream >> m_Rotate;
    stream >> m_Translate;
    #endif
}

template< typename T_Stream > 
FORCE_INLINE
void HMat43::Serialise( T_Stream& stream )
{
    m_Rotate.Serialise(stream);
    m_Translate.Serialise(stream);
}






//
// -------------------------------------------------------
// |-- HMat44 : Hardware 4 Row * 4 Column Matrix Class --|
// -------------------------------------------------------
//

// Cross products for deriving third matrix axis from 2 input axes
// const HVec3 ax = ay ^ az;
// const HVec3 ay = az ^ ax;
// const HVec3 az = ax ^ ay;

/// Default Constructor: Initialise to Identity
FORCE_INLINE
HMat44::HMat44()
    #if defined( _VMX128_ )
    : m_CX( c_UnitVectorX ),
      m_CY( c_UnitVectorY ),
      m_CZ( c_UnitVectorZ ),
      m_CW( c_UnitVectorW )
    #elif defined( _SSE_ )
    : m_AX( c_UnitVectorX ),
      m_AY( c_UnitVectorY ),
      m_AZ( c_UnitVectorZ ),
      m_AW( c_UnitVectorW )
    #endif
{
    #if !defined( _SIMD_ )
    m_A[MX] = HVec4( c_UnitVectorX );
    m_A[MY] = HVec4( c_UnitVectorY );
    m_A[MZ] = HVec4( c_UnitVectorZ );
    m_A[MW] = HVec4( c_UnitVectorW );
    #endif
}

/// Undefined Constructor
NOALIAS __forceinline HMat44::HMat44( const _UNDEFINED undefined ) __restrict
{
}

/// Explicit Constructor from HMat33
FORCE_INLINE
HMat44::HMat44( const HMat33& src )
    #if defined( _VMX128_ )
    : m_CX( src.m_CX ),
      m_CY( src.m_CY ),
      m_CZ( src.m_CZ ),
      m_CW( c_UnitVectorW )
    #elif defined( _SSE_ )
    : m_AX( src.m_AX.m_V ),
      m_AY( src.m_AY.m_V ),
      m_AZ( src.m_AZ.m_V ),
      m_AW( c_UnitVectorW )
    #endif
{
    #if !defined( _SIMD_ )
    m_A[MX] = src.AX();
    m_A[MY] = src.AY();
    m_A[MZ] = src.AZ();
    m_A[MW] = HVec4( c_UnitVectorW );
    #endif
}

/// Explicit Constructor from HMat43
FORCE_INLINE
HMat44::HMat44( const HMat43& src )
    #if defined( _VMX128_ )
    : m_CX( src.m_CX ),
      m_CY( src.m_CY ),
      m_CZ( src.m_CZ ),
      m_CW( c_UnitVectorW )
    #elif defined( _SSE_ )
    : m_AX( src.m_Rotate.m_AX.m_V ),
      m_AY( src.m_Rotate.m_AY.m_V ),
      m_AZ( src.m_Rotate.m_AZ.m_V ),
      m_AW( src.m_Translate || HVecW( c_UnitVectorW ) )
    #endif
{
    #if !defined( _SIMD_ )
    m_A[MX] = src.AX();
    m_A[MY] = src.AY();
    m_A[MZ] = src.AZ();
    m_A[MW] = src.AW() || HVecW( 1.0f );
    #endif
}

/// Copy Constructor
FORCE_INLINE
HMat44::HMat44( const HMat44& src )
    #if defined( _VMX128_ )
    : m_CX( src.m_CX ),
      m_CY( src.m_CY ),
      m_CZ( src.m_CZ ),
      m_CW( src.m_CW )
    #elif defined( _SSE_ )
    : m_AX( src.m_AX.m_V ),
      m_AY( src.m_AY.m_V ),
      m_AZ( src.m_AZ.m_V ),
      m_AW( src.m_AW.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_A[MX] = src.m_A[MX];
    m_A[MY] = src.m_A[MY];
    m_A[MZ] = src.m_A[MZ];
    m_A[MW] = src.m_A[MW];
    #endif
}

#if defined( _VMX128_ )
/// V128 Axis XYZW Constructor
FORCE_INLINE
HMat44::HMat44( const V128 cx, const V128 cy, const V128 cz, const V128 cw ) :
    m_CX( cx ),
    m_CY( cy ),
    m_CZ( cz ),
    m_CW( cw )
{
}
#elif defined( _SSE_ )
/// V128 Axis XYZW Constructor
FORCE_INLINE
HMat44::HMat44( const V128& ax, const V128& ay, const V128& az, const V128& aw ) :
    m_AX( ax ),
    m_AY( ay ),
    m_AZ( az ),
    m_AW( aw )
{
}
#endif

/// HVec4 Axis XYZW Constructor
FORCE_INLINE
HMat44::HMat44( const HVec4& ax, const HVec4& ay, const HVec4& az, const HVec4& aw )
    #if defined( _SSE_ )
    : m_AX( ax.m_V ),
      m_AY( ay.m_V ),
      m_AZ( az.m_V ),
      m_AW( aw.m_V )
    #endif
{
    #if defined( _VMX128_ )
    const V128 tmp0( __vmrghw( ax.m_V, az.m_V ) );
    const V128 tmp1( __vmrghw( ay.m_V, aw.m_V ) );
    const V128 tmp2( __vmrglw( ax.m_V, az.m_V ) );
    const V128 tmp3( __vmrglw( ay.m_V, aw.m_V ) );
    m_CX = __vmrghw( tmp0, tmp1 );
    m_CY = __vmrglw( tmp0, tmp1 );
    m_CZ = __vmrghw( tmp2, tmp3 );
    m_CW = __vmrglw( tmp2, tmp3 );
    #elif !defined( _SIMD_ )
    m_A[MX] = ax;
    m_A[MY] = ay;
    m_A[MZ] = az;
    m_A[MW] = aw;
    #endif
}

/// X Axis Rotation Constructor : clockwise when viewed back down the X axis
FORCE_INLINE
HMat44::HMat44( const RotX& rx, const HVec4& aw )
{
    const HF32 alpha = rx;
    const HF32 cx( Cos( alpha ) );
    const HF32 sx( Sin( alpha ) );
    const Maths::V128 ay = { 0.0f,  cx, sx, 0.0f };
    const Maths::V128 az = { 0.0f, -sx, cx, 0.0f };
    SetAxesXYZW( HVec4( c_UnitVectorX ), HVec4( ay ), HVec4( az ), aw );
}

/// Y Axis Rotation Constructor : clockwise when viewed back down the Y axis
FORCE_INLINE
HMat44::HMat44( const RotY& ry, const HVec4& aw )
{
    const HF32 beta = ry;
    const HF32 cy( Cos( beta ) );
    const HF32 sy( Sin( beta ) );
    const Maths::V128 ax = { cy, 0.0f, -sy, 0.0f };
    const Maths::V128 az = { sy, 0.0f,  cy, 0.0f };
    SetAxesXYZW( HVec4( ax ), HVec4( c_UnitVectorY ), HVec4( az ), aw );
}

/// Z Axis Rotation Constructor : clockwise when viewed back down the Z axis
FORCE_INLINE
HMat44::HMat44( const RotZ& rz, const HVec4& aw )
{
    const HF32 gamma = rz;
    const HF32 cz( Cos( gamma ) );
    const HF32 sz( Sin( gamma ) );
    const Maths::V128 ax = {  cz, sz, 0.0f, 0.0f };
    const Maths::V128 ay = { -sz, cz, 0.0f, 0.0f };
    SetAxesXYZW( HVec4( ax ), HVec4( ay ), HVec4( c_UnitVectorZ ), aw );
}

/// Constructor from primary axis X, secondary axis Y
FORCE_INLINE
HMat44::HMat44( const AxisX& ax, const AxisY& ay, const HVec4& aw )
{
    const C_Normal az( ax ^ ay );
    SetAxesXYZW( HVec4( ax ), HVec4( az ^ ax ), HVec4( az ), aw );
}

/// Constructor from primary axis X, secondary axis Z
FORCE_INLINE
HMat44::HMat44( const AxisX& ax, const AxisZ& az, const HVec4& aw )
{
    const C_Normal ay( az ^ ax );
    SetAxesXYZW( HVec4( ax ), HVec4( ay ), HVec4( ax ^ ay ), aw );
}

/// Constructor from primary axis Y, secondary axis X
FORCE_INLINE
HMat44::HMat44( const AxisY& ay, const AxisX& ax, const HVec4& aw )
{
    const C_Normal az( ax ^ ay );
    SetAxesXYZW( HVec4( ay ^ az ), HVec4( ay ), HVec4( az ), aw );
}

/// Constructor from primary axis Y, secondary axis Z
FORCE_INLINE
HMat44::HMat44( const AxisY& ay, const AxisZ& az, const HVec4& aw )
{
    const C_Normal ax( ay ^ az );
    SetAxesXYZW( HVec4( ax ), HVec4( ay ), HVec4( ax ^ ay ), aw );
}

/// Constructor from primary axis Z, secondary axis X
FORCE_INLINE
HMat44::HMat44( const AxisZ& az, const AxisX& ax, const HVec4& aw )
{
    const C_Normal ay( az ^ ax );
    SetAxesXYZW( HVec4( ay ^ az ), HVec4( ay ), HVec4( az ), aw );
}

/// Constructor from primary axis Z, secondary axis Y
FORCE_INLINE
HMat44::HMat44( const AxisZ& az, const AxisY& ay, const HVec4& aw )
{
    const C_Normal ax( ay ^ az );
    SetAxesXYZW( HVec4( ax ), HVec4( az ^ ax ), HVec4( az ), aw );
}

#if ( DIRECT3D_VERSION >= 0x0900 )

/// Cast to D3DMATRIX
FORCE_INLINE
HMat44::operator const D3DMATRIX () const
{
    D3DMATRIX result;
    
    #if defined( _VMX128_ )
    
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, m_CW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, m_CW ) );
    
    __storeunalignedvector( __vmrghw( tmp0, tmp1 ), &result._11 );
    __storeunalignedvector( __vmrglw( tmp0, tmp1 ), &result._21 );
    __storeunalignedvector( __vmrghw( tmp2, tmp3 ), &result._31 );
    __storeunalignedvector( __vmrglw( tmp2, tmp3 ), &result._41 );
    
    #elif defined( _SSE_ )
    
    SIMD_StoreUnaligned( m_AX.m_V, &result._11 );
    SIMD_StoreUnaligned( m_AY.m_V, &result._21 );
    SIMD_StoreUnaligned( m_AZ.m_V, &result._31 );
    SIMD_StoreUnaligned( m_AW.m_V, &result._41 );
    
    #else
    
    const HMat44& m = *this;
    
    result._11 = m[MX][VX];
    result._12 = m[MX][VY];
    result._13 = m[MX][VZ];
    result._14 = m[MX][VW];
    
    result._21 = m[MY][VX];
    result._22 = m[MY][VY];
    result._23 = m[MY][VZ];
    result._24 = m[MY][VW];
    
    result._31 = m[MZ][VX];
    result._32 = m[MZ][VY];
    result._33 = m[MZ][VZ];
    result._34 = m[MZ][VW];
    
    result._41 = m[MW][VX];
    result._42 = m[MW][VY];
    result._43 = m[MW][VZ];
    result._44 = m[MW][VW];
    
    #endif
    
    return result;
}

/// Cast to D3DXMATRIX
FORCE_INLINE
HMat44::operator const D3DXMATRIX () const
{
    D3DXMATRIX result;
    
    #if defined( _VMX128_ )
    
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, m_CW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, m_CW ) );
    
    __storeunalignedvector( __vmrghw( tmp0, tmp1 ), &result._11 );
    __storeunalignedvector( __vmrglw( tmp0, tmp1 ), &result._21 );
    __storeunalignedvector( __vmrghw( tmp2, tmp3 ), &result._31 );
    __storeunalignedvector( __vmrglw( tmp2, tmp3 ), &result._41 );
    
    #elif defined( _SSE_ )
    
    SIMD_StoreUnaligned( m_AX.m_V, &result._11 );
    SIMD_StoreUnaligned( m_AY.m_V, &result._21 );
    SIMD_StoreUnaligned( m_AZ.m_V, &result._31 );
    SIMD_StoreUnaligned( m_AW.m_V, &result._41 );
    
    #else
    
    const HMat44& m = *this;
    
    result._11 = m[MX][VX];
    result._12 = m[MX][VY];
    result._13 = m[MX][VZ];
    result._14 = m[MX][VW];
    
    result._21 = m[MY][VX];
    result._22 = m[MY][VY];
    result._23 = m[MY][VZ];
    result._24 = m[MY][VW];
    
    result._31 = m[MZ][VX];
    result._32 = m[MZ][VY];
    result._33 = m[MZ][VZ];
    result._34 = m[MZ][VW];
    
    result._41 = m[MW][VX];
    result._42 = m[MW][VY];
    result._43 = m[MW][VZ];
    result._44 = m[MW][VW];
    
    #endif
    
    return result;
}

#endif

/// Accessor: Get Axis X
FORCE_INLINE
const HVec4 HMat44::AX() const
{
    #if defined( _VMX128_ )
    return HVec4( __vmrghw( __vmrghw( m_CX, m_CZ ), __vmrghw( m_CY, m_CW ) ) );
    #elif defined( _SSE_ )
    return m_AX;
    #else
    return m_A[MX];
    #endif
}

/// Accessor: Get Axis Y
FORCE_INLINE
const HVec4 HMat44::AY() const
{
    #if defined( _VMX128_ )
    return HVec4( __vmrglw( __vmrghw( m_CX, m_CZ ), __vmrghw( m_CY, m_CW ) ) );
    #elif defined( _SSE_ )
    return m_AY;
    #else
    return m_A[MY];
    #endif
}

/// Accessor: Get Axis Z
FORCE_INLINE
const HVec4 HMat44::AZ() const
{
    #if defined( _VMX128_ )
    return HVec4( __vmrghw( __vmrglw( m_CX, m_CZ ), __vmrglw( m_CY, m_CW ) ) );
    #elif defined( _SSE_ )
    return m_AZ;
    #else
    return m_A[MZ];
    #endif
}

/// Accessor: Get Axis W
FORCE_INLINE
const HVec4 HMat44::AW() const
{
    #if defined( _VMX128_ )
    return HVec4( __vmrglw( __vmrglw( m_CX, m_CZ ), __vmrglw( m_CY, m_CW ) ) );
    #elif defined( _SSE_ )
    return m_AW;
    #else
    return m_A[MW];
    #endif
}

/// Accessor: Set Axis X
FORCE_INLINE
void HMat44::AX( const HVec4& ax )
{
    #if defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, ax.m_V, VMX_MASK_INSERT_X, VMX_ROTATE_X_TO_X );
    m_CY = __vrlimi( m_CY, ax.m_V, VMX_MASK_INSERT_X, VMX_ROTATE_Y_TO_X );
    m_CZ = __vrlimi( m_CZ, ax.m_V, VMX_MASK_INSERT_X, VMX_ROTATE_Z_TO_X );
    m_CW = __vrlimi( m_CW, ax.m_V, VMX_MASK_INSERT_X, VMX_ROTATE_W_TO_X );
    #elif defined( _SSE_ )
    m_AX.m_V = ax.m_V;
    #else
    m_A[MX] = ax;
    #endif
}

/// Accessor: Set Axis Y
FORCE_INLINE
void HMat44::AY( const HVec4& ay )
{
    #if defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, ay.m_V, VMX_MASK_INSERT_Y, VMX_ROTATE_X_TO_Y );
    m_CY = __vrlimi( m_CY, ay.m_V, VMX_MASK_INSERT_Y, VMX_ROTATE_Y_TO_Y );
    m_CZ = __vrlimi( m_CZ, ay.m_V, VMX_MASK_INSERT_Y, VMX_ROTATE_Z_TO_Y );
    m_CW = __vrlimi( m_CW, ay.m_V, VMX_MASK_INSERT_Y, VMX_ROTATE_W_TO_Y );
    #elif defined( _SSE_ )
    m_AY.m_V = ay.m_V;
    #else
    m_A[MY] = ay;
    #endif
}

/// Accessor: Set Axis Z
FORCE_INLINE
void HMat44::AZ( const HVec4& az )
{
    #if defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, az.m_V, VMX_MASK_INSERT_Z, VMX_ROTATE_X_TO_Z );
    m_CY = __vrlimi( m_CY, az.m_V, VMX_MASK_INSERT_Z, VMX_ROTATE_Y_TO_Z );
    m_CZ = __vrlimi( m_CZ, az.m_V, VMX_MASK_INSERT_Z, VMX_ROTATE_Z_TO_Z );
    m_CW = __vrlimi( m_CW, az.m_V, VMX_MASK_INSERT_Z, VMX_ROTATE_W_TO_Z );
    #elif defined( _SSE_ )
    m_AZ.m_V = az.m_V;
    #else
    m_A[MZ] = az;
    #endif
}

/// Accessor: Set Axis W
FORCE_INLINE
void HMat44::AW( const HVec4& aw )
{
    #if defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    m_CW = __vrlimi( m_CW, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_W_TO_W );
    #elif defined( _SSE_ )
    m_AW.m_V = aw.m_V;
    #else
    m_A[MW] = aw;
    #endif
}

#if !defined( _SIMD_ )

/// Const Element Index Accessor
FORCE_INLINE
const HVec4& HMat44::operator [] ( const U32 i ) const
{
    #if defined( _SIMD_ )
    return (&m_AX)[i];
    #else
    return m_A[i];
    #endif
}

/// Element Index Accessor
FORCE_INLINE
HVec4& HMat44::operator [] ( const U32 i )
{
    #if defined( _SIMD_ )
    return (&m_AX)[i];
    #else
    return m_A[i];
    #endif
}

#endif

/// Private Accessor: Set Axes XYZW from HVec4's
FORCE_INLINE
void HMat44::SetAxesXYZW( const HVec4& ax, const HVec4& ay, const HVec4& az, const HVec4& aw )
{
    #if defined( _VMX128_ )
    const V128 tmp0( __vmrghw( ax.m_V, az.m_V ) );
    const V128 tmp1( __vmrghw( ay.m_V, aw.m_V ) );
    const V128 tmp2( __vmrglw( ax.m_V, az.m_V ) );
    const V128 tmp3( __vmrglw( ay.m_V, aw.m_V ) );
    m_CX = __vmrghw( tmp0, tmp1 );
    m_CY = __vmrglw( tmp0, tmp1 );
    m_CZ = __vmrghw( tmp2, tmp3 );
    m_CW = __vmrglw( tmp2, tmp3 );
    #elif defined( _SSE_ )
    m_AX.m_V = ax.m_V;
    m_AY.m_V = ay.m_V;
    m_AZ.m_V = az.m_V;
    m_AW.m_V = aw.m_V;
    #else
    m_A[MX] = ax;
    m_A[MY] = ay;
    m_A[MZ] = az;
    m_A[MW] = aw;
    #endif
}

#if defined( _VMX128_ )
/// Private Accessor: Set Axes XYZW from V128's
FORCE_INLINE
void HMat44::SetAxesXYZW( const V128 ax, const V128 ay, const V128 az, const V128 aw )
{
    const V128 tmp0( __vmrghw( ax, az ) );
    const V128 tmp1( __vmrghw( ay, aw ) );
    const V128 tmp2( __vmrglw( ax, az ) );
    const V128 tmp3( __vmrglw( ay, aw ) );
    m_CX = __vmrghw( tmp0, tmp1 );
    m_CY = __vmrglw( tmp0, tmp1 );
    m_CZ = __vmrghw( tmp2, tmp3 );
    m_CW = __vmrglw( tmp2, tmp3 );
}
#elif defined( _SSE_ )
/// Private Accessor: Set Axes XYZW from V128's
FORCE_INLINE
void HMat44::SetAxesXYZW( const V128& ax, const V128& ay, const V128& az, const V128& aw )
{
    m_AX.m_V = ax;
    m_AY.m_V = ay;
    m_AZ.m_V = az;
    m_AW.m_V = aw;
}
#endif

/// HMat33 Assignment
FORCE_INLINE
const HMat44& HMat44::operator = ( const HMat33& src )
{
    #if defined( _VMX128_ )
    m_CX = src.m_CX;
    m_CY = src.m_CY;
    m_CZ = src.m_CZ;
    m_CW = c_UnitVectorW;
    #elif defined( _SSE_ )
    m_AX.m_V = src.m_AX.m_V;
    m_AY.m_V = src.m_AY.m_V;
    m_AZ.m_V = src.m_AZ.m_V;
    m_AW.m_V = c_UnitVectorW;
    #else
    m_A[MX] = src.AX();
    m_A[MY] = src.AY();
    m_A[MZ] = src.AZ();
    m_A[MW] = HVec4( c_UnitVectorW );
    #endif
    return *this;
}

/// HMat43 Assignment
FORCE_INLINE
const HMat44& HMat44::operator = ( const HMat43& src )
{
    #if defined( _VMX128_ )
    m_CX = src.m_CX;
    m_CY = src.m_CY;
    m_CZ = src.m_CZ;
    m_CW = c_UnitVectorW;
    #elif defined( _SSE_ )
    m_AX.m_V = src.m_Rotate.m_AX.m_V;
    m_AY.m_V = src.m_Rotate.m_AY.m_V;
    m_AZ.m_V = src.m_Rotate.m_AZ.m_V;
    m_AW.m_V = ( src.m_Translate || HVecW( c_UnitVectorW ) ).m_V;
    #else
    m_A[MX] = src.AX();
    m_A[MY] = src.AY();
    m_A[MZ] = src.AZ();
    m_A[MW] = src.AW() || HVecW( 1.0f );
    #endif
    return *this;
}

/// Copy Assignment
FORCE_INLINE
const HMat44& HMat44::operator = ( const HMat44& copy )
{
    #if defined( _VMX128_ )
    m_CX = copy.m_CX;
    m_CY = copy.m_CY;
    m_CZ = copy.m_CZ;
    m_CW = copy.m_CW;
    #elif defined( _SSE_ )
    m_AX.m_V = copy.m_AX.m_V;
    m_AY.m_V = copy.m_AY.m_V;
    m_AZ.m_V = copy.m_AZ.m_V;
    m_AW.m_V = copy.m_AW.m_V;
    #else
    m_A[MX] = copy.m_A[MX];
    m_A[MY] = copy.m_A[MY];
    m_A[MZ] = copy.m_A[MZ];
    m_A[MW] = copy.m_A[MW];
    #endif
    return *this;
}

/// Set to Identity Matrix
FORCE_INLINE
void HMat44::SetIdentity()
{
    SetAxesXYZW( HVec4( c_UnitVectorX ), HVec4( c_UnitVectorY ), HVec4( c_UnitVectorZ ), HVec4( c_UnitVectorW ) );
}

//const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
//const V128 tmp1( __vmrghw( m_CY, m_CW ) );
//const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
//const V128 tmp3( __vmrglw( m_CY, m_CW ) );
//const V128 ax( __vmrghw( tmp0, tmp1 ) );
//const V128 ay( __vmrglw( tmp0, tmp1 ) );
//const V128 az( __vmrghw( tmp2, tmp3 ) );
//const V128 aw( __vmrglw( tmp2, tmp3 ) );

/// Transpose of Matrix
FORCE_INLINE
const HMat44 HMat44::Transposed() const
{
    #if defined( _VMX128_ )
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, m_CW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, m_CW ) );
    const V128 ax( __vmrghw( tmp0, tmp1 ) );
    const V128 ay( __vmrglw( tmp0, tmp1 ) );
    const V128 az( __vmrghw( tmp2, tmp3 ) );
    const V128 aw( __vmrglw( tmp2, tmp3 ) );
    return HMat44( ax, ay, az, aw );
    #elif defined( _SSE_ )
    const V128 tmp0( SIMD_MergeHigh( m_AX.m_V, m_AZ.m_V ) );
    const V128 tmp1( SIMD_MergeHigh( m_AY.m_V, m_AW.m_V ) );
    const V128 tmp2( SIMD_MergeLow ( m_AX.m_V, m_AZ.m_V ) );
    const V128 tmp3( SIMD_MergeLow ( m_AY.m_V, m_AW.m_V ) );
    const V128 ax  ( SIMD_MergeHigh( tmp0, tmp1 ) );
    const V128 ay  ( SIMD_MergeLow ( tmp0, tmp1 ) );
    const V128 az  ( SIMD_MergeHigh( tmp2, tmp3 ) );
    const V128 aw  ( SIMD_MergeLow ( tmp2, tmp3 ) );
    return HMat44( ax, ay, az, aw );
    #else
    const HMat44& m = *this;
    const HVec4 ax( m[MX][VX], m[MY][VX], m[MZ][VX], m[MW][VX] );
    const HVec4 ay( m[MX][VY], m[MY][VY], m[MZ][VY], m[MW][VY] );
    const HVec4 az( m[MX][VZ], m[MY][VZ], m[MZ][VZ], m[MW][VZ] );
    const HVec4 aw( m[MX][VW], m[MY][VW], m[MZ][VW], m[MW][VW] );
    return HMat44( ax, ay, az, aw );
    #endif
}

/// Transpose this Matrix
FORCE_INLINE
void HMat44::Transpose()
{
    #if defined( _VMX128_ )
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, m_CW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, m_CW ) );
    m_CX = __vmrghw( tmp0, tmp1 );
    m_CY = __vmrglw( tmp0, tmp1 );
    m_CZ = __vmrghw( tmp2, tmp3 );
    m_CW = __vmrglw( tmp2, tmp3 );
    #elif defined( _SSE_ )
    const V128 tmp0( SIMD_MergeHigh( m_AX.m_V, m_AZ.m_V ) );
    const V128 tmp1( SIMD_MergeHigh( m_AY.m_V, m_AW.m_V ) );
    const V128 tmp2( SIMD_MergeLow ( m_AX.m_V, m_AZ.m_V ) );
    const V128 tmp3( SIMD_MergeLow ( m_AY.m_V, m_AW.m_V ) );
    m_AX.m_V = SIMD_MergeHigh( tmp0, tmp1 );
    m_AY.m_V = SIMD_MergeLow ( tmp0, tmp1 );
    m_AZ.m_V = SIMD_MergeHigh( tmp2, tmp3 );
    m_AW.m_V = SIMD_MergeLow ( tmp2, tmp3 );
    #else
    const HMat44& m = *this;
    const HVec4 ax( m[MX][VX], m[MY][VX], m[MZ][VX], m[MW][VX] );
    const HVec4 ay( m[MX][VY], m[MY][VY], m[MZ][VY], m[MW][VY] );
    const HVec4 az( m[MX][VZ], m[MY][VZ], m[MZ][VZ], m[MW][VZ] );
    const HVec4 aw( m[MX][VW], m[MY][VW], m[MZ][VW], m[MW][VW] );
    m_A[MX] = ax;
    m_A[MY] = ay;
    m_A[MZ] = az;
    m_A[MW] = aw;
    #endif
}

/// Matrix Multiply
FORCE_INLINE
const HMat44 HMat44::operator * ( const HMat44& m2 ) const
{
    #if defined( _VMX128_ )
    
    const V128 m2_cx( m2.m_CX );
    const V128 m2_cy( m2.m_CY );
    const V128 m2_cz( m2.m_CZ );
    const V128 m2_cw( m2.m_CW );
    
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, m_CW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, m_CW ) );
    
    const V128 m1_ax( __vmrghw( tmp0, tmp1 ) );
    const V128 m1_ay( __vmrglw( tmp0, tmp1 ) );
    const V128 m1_az( __vmrghw( tmp2, tmp3 ) );
    const V128 m1_aw( __vmrglw( tmp2, tmp3 ) );
    
    const V128 result_cx_x( __vmsum4fp( m2_cx, m1_ax ) );
    const V128 result_cx_y( __vmsum4fp( m2_cx, m1_ay ) );
    const V128 result_cx_z( __vmsum4fp( m2_cx, m1_az ) );
    const V128 result_cx_w( __vmsum4fp( m2_cx, m1_aw ) );
    
    const V128 result_cy_x( __vmsum4fp( m2_cy, m1_ax ) );
    const V128 result_cy_y( __vmsum4fp( m2_cy, m1_ay ) );
    const V128 result_cy_z( __vmsum4fp( m2_cy, m1_az ) );
    const V128 result_cy_w( __vmsum4fp( m2_cy, m1_aw ) );
    
    const V128 result_cz_x( __vmsum4fp( m2_cz, m1_ax ) );
    const V128 result_cz_y( __vmsum4fp( m2_cz, m1_ay ) );
    const V128 result_cz_z( __vmsum4fp( m2_cz, m1_az ) );
    const V128 result_cz_w( __vmsum4fp( m2_cz, m1_aw ) );
    
    const V128 result_cw_x( __vmsum4fp( m2_cw, m1_ax ) );
    const V128 result_cw_y( __vmsum4fp( m2_cw, m1_ay ) );
    const V128 result_cw_z( __vmsum4fp( m2_cw, m1_az ) );
    const V128 result_cw_w( __vmsum4fp( m2_cw, m1_aw ) );
    
    return HMat44( __vmrghw( __vmrghw( result_cx_x, result_cx_z ), __vmrghw( result_cx_y, result_cx_w ) ),
                   __vmrghw( __vmrghw( result_cy_x, result_cy_z ), __vmrghw( result_cy_y, result_cy_w ) ),
                   __vmrghw( __vmrghw( result_cz_x, result_cz_z ), __vmrghw( result_cz_y, result_cz_w ) ),
                   __vmrghw( __vmrghw( result_cw_x, result_cw_z ), __vmrghw( result_cw_y, result_cw_w ) ) );
    
    #elif defined( _SSE_ )
    
    return HMat44( m_AX * m2,
                   m_AY * m2,
                   m_AZ * m2,
                   m_AW * m2 );
    
    #else
    
    return HMat44( m_A[MX] * m2,
                   m_A[MY] * m2,
                   m_A[MZ] * m2,
                   m_A[MW] * m2 );
    
    #endif
}

/// Full 4x4 Matrix Inverse
FORCE_INLINE
const HMat44 HMat44::Inverse() const
{
    #if defined( _SSE_ )
    return Matrix44InverseCramersRuleSSE();
    #elif defined( _VMX128_ )
    HMat44 result;
    Matrix44InverseCramersRuleFP( (F32*)&m_CX.x, (F32*)&result.m_CX.x );
    return result;
    #else
    HMat44 result;
    Matrix44InverseCramersRuleFP( (F32*)m_A, (F32*)result.m_A );
    return result;
    #endif
}

/// Inverse of Rotation & Translation - Faster but only valid for matrices with ortho-normal rotations!
FORCE_INLINE
const HMat44 HMat44::InverseRotTrans() const
{
    #if defined( _VMX128_ )
    
    const V128 zero( __vzero() );
    //const V128 vec3_sign_mask( c_Vector3_SignMask );
    const V128 vec_sign_mask( c_VectorSignMask );
    
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, zero ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, zero ) );
    
    const V128 t_cx( __vmrghw( tmp0, tmp1 ) );
    const V128 t_cy( __vmrglw( tmp0, tmp1 ) );
    const V128 t_cz( __vmrghw( tmp2, tmp3 ) );
    const V128 t_cw( __vmrglw( tmp2, tmp3 ) );
    
    //const V128 neg_trans( __vxor( t_cw, vec3_sign_mask ) );
    const V128 neg_trans( __vxor( t_cw, vec_sign_mask ) );
    
    const V128 inv_trans_x( __vmsum3fp( neg_trans, t_cx ) );
    const V128 inv_trans_y( __vmsum3fp( neg_trans, t_cy ) );
    const V128 inv_trans_z( __vmsum3fp( neg_trans, t_cz ) );
    
    const V128 inv_trans( __vmrghw( __vmrghw( inv_trans_x, inv_trans_z ), __vmrghw( inv_trans_y, zero ) ) );
    
    return HMat44( __vrlimi( t_cx, inv_trans, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W ),
                   __vrlimi( t_cy, inv_trans, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W ),
                   __vrlimi( t_cz, inv_trans, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W ),
                   c_UnitVectorW );
    
    #else
    
    const HMat33 invRot( ( HMat33( *this ) ).Transposed() );
    
    #if defined( _SIMD_ )
    return HMat44( invRot.AX(),
                   invRot.AY(),
                   invRot.AZ(),
                   ( HVec3( -m_AW ) * invRot ) || HVecW( c_UnitVectorW ) );
    #else
    return HMat44( HVec4(invRot.AX()),
                   HVec4(invRot.AY()),
                   HVec4(invRot.AZ()),
                   ( HVec3( -m_A[MW] ) * invRot ) || HVecW( 1.0f ) );
    #endif
    
    #endif
}

#if ( DIRECT3D_VERSION >= 0x0900 )

/// Cast to D3DMATRIX
/*FORCE_INLINE
operator const D3DMATRIX (const HMat33& self) const
{
    return (D3DMATRIX)HMat44( self );
}

/// Cast to D3DXMATRIX
FORCE_INLINE
operator const D3DXMATRIX (const HMat33& self) const
{
    return (D3DXMATRIX)HMat44( self );
}*/

#endif

}; // namespace Maths



#endif
