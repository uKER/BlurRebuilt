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
NOALIAS __forceinline RotX::RotX( const F32& a ) :
    m_AngleR( a )
{
}

/// HF32 Constructor
NOALIAS __forceinline RotX::RotX( const HF32 PCREF a ) :
    m_AngleR( a )
{
}

/// HF32 Cast
NOALIAS __forceinline RotX::operator const HF32 () const __restrict
{
    return m_AngleR;
}


//
// ------------------------------------------------
// |-- Rotation Y class for matrix construction --|
// ------------------------------------------------
//

/// F32 Constructor
NOALIAS __forceinline RotY::RotY( const F32& a ) :
    m_AngleR( a )
{
}

/// HF32 Constructor
NOALIAS __forceinline RotY::RotY( const HF32 PCREF a ) :
    m_AngleR( a )
{
}

/// HF32 Cast
NOALIAS __forceinline RotY::operator const HF32 () const __restrict
{
    return m_AngleR;
}


//
// ------------------------------------------------
// |-- Rotation Z class for matrix construction --|
// ------------------------------------------------
//

/// F32 Constructor
NOALIAS __forceinline RotZ::RotZ( const F32& a ) :
    m_AngleR( a )
{
}

/// HF32 Constructor
NOALIAS __forceinline RotZ::RotZ( const HF32 PCREF a ) :
    m_AngleR( a )
{
}

/// HF32 Cast
NOALIAS __forceinline RotZ::operator const HF32 () const __restrict
{
    return m_AngleR;
}



//
// --------------------------------------------
// |-- Axis X class for matrix construction --|
// --------------------------------------------
//

/// HVec3 Constructor
NOALIAS __forceinline AxisX::AxisX( const C_Normal PCREF ax ) :
    C_Normal( ax )
{
}

//
// --------------------------------------------
// |-- Axis Y class for matrix construction --|
// --------------------------------------------
//

/// HVec3 Constructor
NOALIAS __forceinline AxisY::AxisY( const C_Normal PCREF ay ) :
    C_Normal( ay )
{
}

//
// --------------------------------------------
// |-- Axis Z class for matrix construction --|
// --------------------------------------------
//

/// HVec3 Constructor
NOALIAS __forceinline AxisZ::AxisZ( const C_Normal PCREF az ) :
    C_Normal( az )
{
}








//
// -------------------------------------------
// |-- Scale class for matrix construction --|
// -------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_MatrixScale::C_MatrixScale()
{
}

/// HVec3 Constructor
NOALIAS __forceinline C_MatrixScale::C_MatrixScale( const HVec3 PCREF scale ) :
    m_Scale( scale )
{
}

/// HF32 XYZ Constructor
NOALIAS __forceinline C_MatrixScale::C_MatrixScale( const HF32 PCREF sx, const HF32 PCREF sy, const HF32 PCREF sz ) :
    m_Scale( HVec3( sx, sy, sz ) )
{
}

/// HVec3 Cast
NOALIAS __forceinline C_MatrixScale::operator const HVec3 () const __restrict
{
    return m_Scale;
}

/// HVec3 Assignment
NOALIAS __forceinline void C_MatrixScale::operator = ( const HVec3 PCREF scale ) __restrict
{
    m_Scale = scale;
}

/// Accessor X
NOALIAS __forceinline const HF32 C_MatrixScale::X() const __restrict
{
    return m_Scale.X();
}

/// Accessor Y
NOALIAS __forceinline const HF32 C_MatrixScale::Y() const __restrict
{
    return m_Scale.Y();
}

/// Accessor Z
NOALIAS __forceinline const HF32 C_MatrixScale::Z() const __restrict
{
    return m_Scale.Z();
}


//
// ------------------------------------------------
// |-- Rotate RPH class for matrix construction --|
// ------------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_MatrixRotateRPH::C_MatrixRotateRPH()
{
}

/// HVec3 Constructor
NOALIAS __forceinline C_MatrixRotateRPH::C_MatrixRotateRPH( const HVec3 PCREF rotate ) :
    m_Rotate( rotate )
{
}

/// HF32 Roll-Pitch-Heading Constructor
NOALIAS __forceinline C_MatrixRotateRPH::C_MatrixRotateRPH( const HF32 PCREF r, const HF32 PCREF p, const HF32 PCREF h ) :
    m_Rotate( HVec3( r, p, h ) )
{
}

/// HVec3 Cast
NOALIAS __forceinline C_MatrixRotateRPH::operator const HVec3 () const __restrict
{
    return m_Rotate;
}

/// HVec3 Assignment
NOALIAS __forceinline void C_MatrixRotateRPH::operator = ( const HVec3 PCREF rotate ) __restrict
{
    m_Rotate = rotate;
}

/// Accessor: Roll
NOALIAS __forceinline const RotZ C_MatrixRotateRPH::Roll() const __restrict
{
    return RotZ( m_Rotate.X() );
}

/// Accessor: Pitch
NOALIAS __forceinline const RotX C_MatrixRotateRPH::Pitch() const __restrict
{
    return RotX( m_Rotate.Y() );
}

/// Accessor: Heading
NOALIAS __forceinline const RotY C_MatrixRotateRPH::Heading() const __restrict
{
    return RotY( m_Rotate.Z() );
}



//
// ------------------------------------------------
// |-- Rotate PHR class for matrix construction --|
// ------------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_MatrixRotatePHR::C_MatrixRotatePHR()
{
}

/// HVec3 Constructor
NOALIAS __forceinline C_MatrixRotatePHR::C_MatrixRotatePHR( const HVec3 PCREF rotate ) :
    m_Rotate( rotate )
{
}

/// HF32 Pitch-Heading-Roll Constructor
NOALIAS __forceinline C_MatrixRotatePHR::C_MatrixRotatePHR( const HF32 PCREF p, const HF32 PCREF h, const HF32 PCREF r ) :
    m_Rotate( HVec3( p, h, r ) )
{
}

/// HVec3 Cast
NOALIAS __forceinline C_MatrixRotatePHR::operator const HVec3 () const __restrict
{
    return m_Rotate;
}

/// HVec3 Assignment
NOALIAS __forceinline void C_MatrixRotatePHR::operator = ( const HVec3 PCREF rotate ) __restrict
{
    m_Rotate = rotate;
}

/// Accessor: Pitch
NOALIAS __forceinline const RotX C_MatrixRotatePHR::Pitch() const __restrict
{
    return RotX( m_Rotate.X() );
}

/// Accessor: Heading
NOALIAS __forceinline const RotY C_MatrixRotatePHR::Heading() const __restrict
{
    return RotY( m_Rotate.Y() );
}

/// Accessor: Roll
NOALIAS __forceinline const RotZ C_MatrixRotatePHR::Roll() const __restrict
{
    return RotZ( m_Rotate.Z() );
}


//
// -----------------------------------------------
// |-- Translate class for matrix construction --|
// -----------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_MatrixTranslate::C_MatrixTranslate()
{
}

/// HVec3 Constructor
NOALIAS __forceinline C_MatrixTranslate::C_MatrixTranslate( const HVec3 PCREF translate ) :
    m_Translate( translate )
{
}

/// HF32 XYZ Constructor
NOALIAS __forceinline C_MatrixTranslate::C_MatrixTranslate( const HF32 PCREF tx, const HF32 PCREF ty, const HF32 PCREF tz ) :
    m_Translate( HVec3( tx, ty, tz ) )
{
}

/// HVec3 Cast
NOALIAS __forceinline C_MatrixTranslate::operator const HVec3 () const __restrict
{
    return m_Translate;
}

/// HVec3 Assignment
NOALIAS __forceinline void C_MatrixTranslate::operator = ( const HVec3 PCREF translate ) __restrict
{
    m_Translate = translate;
}

/// Accessor: X
NOALIAS __forceinline const HF32 C_MatrixTranslate::X() const __restrict
{
    return m_Translate.X();
}

/// Accessor: Y
NOALIAS __forceinline const HF32 C_MatrixTranslate::Y() const __restrict
{
    return m_Translate.Y();
}

/// Accessor: Z
NOALIAS __forceinline const HF32 C_MatrixTranslate::Z() const __restrict
{
    return m_Translate.Z();
}


//
// ------------------------------------------------------------
// |-- Scale-Rotate-Translate class for matrix construction --|
// ------------------------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_MatrixSRT::C_MatrixSRT()
{
}

/// Scale-Rotate-Translate Constructor
NOALIAS __forceinline C_MatrixSRT::C_MatrixSRT( const C_MatrixScale PCREF scale, const C_MatrixRotateRPH PCREF rotate, const C_MatrixTranslate PCREF translate ) :
    m_Scale( scale ),
    m_Rotate( rotate ),
    m_Translate( translate )
{
}

/// Accessor: Scale
NOALIAS __forceinline const C_MatrixScale C_MatrixSRT::Scale() const __restrict
{
    return m_Scale;
}

/// Accessor: Rotate
NOALIAS __forceinline const C_MatrixRotateRPH C_MatrixSRT::Rotate() const __restrict
{
    return m_Rotate;
}

/// Accessor: Translate
NOALIAS __forceinline const C_MatrixTranslate C_MatrixSRT::Translate() const __restrict
{
    return m_Translate;
}




//
// --------------------------------------------------------------------------
// |-- C_Eigen33 : Eigen values & vectors from HMat33 Eigen Decomposition --|
// --------------------------------------------------------------------------
//

/// Eigen Value & Vector Constructor
NOALIAS __forceinline C_Eigen33::C_Eigen33( const HVec3 PCREF val, const HVec3 PCREF vec0, const HVec3 PCREF vec1, const HVec3 PCREF vec2 ) :
    m_Values ( val ),
    m_Vector0( vec0 ),
    m_Vector1( vec1 ),
    m_Vector2( vec2 )
{
}

/// Copy Constructor
NOALIAS __forceinline C_Eigen33::C_Eigen33( const C_Eigen33& copy ) :
    m_Values ( copy.m_Values  ),
    m_Vector0( copy.m_Vector0 ),
    m_Vector1( copy.m_Vector1 ),
    m_Vector2( copy.m_Vector2 )
{
}

/// Copy Assignment
NOALIAS __forceinline const C_Eigen33& C_Eigen33::operator = ( const C_Eigen33& copy )
{
    m_Values  = copy.m_Values;
    m_Vector0 = copy.m_Vector0;
    m_Vector1 = copy.m_Vector1;
    m_Vector2 = copy.m_Vector2;
    return *this;
}

/// Accessor: Value 0
NOALIAS __forceinline const HF32 C_Eigen33::Value0() const __restrict
{
    return m_Values.X();
}

/// Accessor: Value 1
NOALIAS __forceinline const HF32 C_Eigen33::Value1() const __restrict
{
    return m_Values.Y();
}

/// Accessor: Value 2
NOALIAS __forceinline const HF32 C_Eigen33::Value2() const __restrict
{
    return m_Values.Z();
}

/// Accessor: Values
NOALIAS __forceinline const HVec3 C_Eigen33::Values() const __restrict
{
    return m_Values;
}

/// Accessor: Vector 0
NOALIAS __forceinline const HVec3 C_Eigen33::Vector0() const __restrict
{
    return m_Vector0;
}

/// Accessor: Vector 1
NOALIAS __forceinline const HVec3 C_Eigen33::Vector1() const __restrict
{
    return m_Vector1;
}

/// Accessor: Vector 2
NOALIAS __forceinline const HVec3 C_Eigen33::Vector2() const __restrict
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
NOALIAS __forceinline HMat33::HMat33()
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    : m_AX( c_UnitVectorX ),
      m_AY( c_UnitVectorY ),
      m_AZ( c_UnitVectorZ )
    #elif defined( _VMX128_ )
    : m_CX( c_UnitVectorX ),
      m_CY( c_UnitVectorY ),
      m_CZ( c_UnitVectorZ )
    #endif
{
    #if !defined( _SIMD_ )
    m_A[MX] = HVec3( c_UnitVectorX );
    m_A[MY] = HVec3( c_UnitVectorY );
    m_A[MZ] = HVec3( c_UnitVectorZ );
    #endif
}

/// Undefined Constructor
NOALIAS __forceinline HMat33::HMat33( const _UNDEFINED& /*undefined*/ )
{
}

/// Constructor: initialises to the zero matrix.
NOALIAS __forceinline HMat33::HMat33( const _ZERO_MATRIX )
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    : m_AX( c_VectorZero ),
      m_AY( c_VectorZero ),
      m_AZ( c_VectorZero )
    #elif defined( _VMX128_ )
    : m_CX( __vzero() ),
      m_CY( __vzero() ),
      m_CZ( __vzero() )
    #endif
{
    #if !defined( _SIMD_ )
	const HVec3 zero;
	AX( zero );
	AY( zero );
	AZ( zero );
	#endif
}

/// Copy Constructor
NOALIAS __forceinline HMat33::HMat33( const HMat33& src )
    #if defined( _SPU_ ) || defined( _PPU_ )
    : m_AX( src.m_AX ),
      m_AY( src.m_AY ),
      m_AZ( src.m_AZ )
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline HMat33::HMat33( const HMat43& src )
    #if defined( _SPU_ ) || defined( _PPU_ )
    : m_AX( src.m_AX ),
      m_AY( src.m_AY ),
      m_AZ( src.m_AZ )
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline HMat33::HMat33( const HMat44& src )
    #if defined( _SPU_ ) || defined( _PPU_ )
    : m_AX( src.m_AX ),
      m_AY( src.m_AY ),
      m_AZ( src.m_AZ )
    #elif defined( _VMX128_ )
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

#if defined( _SPU_ ) || defined( _PPU_ )
/// V128 Axes XYZ Constructor
NOALIAS __forceinline HMat33::HMat33( const vector float ax, const vector float ay, const vector float az ) :
    m_AX( ax ),
    m_AY( ay ),
    m_AZ( az )
{
}
#elif defined( _VMX128_ )
/// V128 Columns XYZ Constructor
NOALIAS __forceinline HMat33::HMat33( const V128 cx, const V128 cy, const V128 cz ) :
    m_CX( cx ),
    m_CY( cy ),
    m_CZ( cz )
{
}
#elif defined( _SSE_ )
/// V128 Axes XYZ Constructor
NOALIAS __forceinline HMat33::HMat33( const V128 PCREF ax, const V128 PCREF ay, const V128 PCREF az ) :
    m_AX( ax ),
    m_AY( ay ),
    m_AZ( az )
{
}
#endif

/// HVec3 Axis X, Y & Z Constructor
NOALIAS __forceinline HMat33::HMat33( const HVec3 PCREF ax, const HVec3 PCREF ay, const HVec3 PCREF az )
    #if defined( _SPU_ ) || defined( _PPU_ )
    : m_AX( ax.m_V ),
      m_AY( ay.m_V ),
      m_AZ( az.m_V )
    #elif defined( _SSE_ )
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
    #elif !defined( _SIMD_ )
    m_A[MX] = ax;
    m_A[MY] = ay;
    m_A[MZ] = az;
    #endif
}

/// Matrix-Scale Constructor
NOALIAS __forceinline HMat33::HMat33( const C_MatrixScale PCREF scale )
    #if defined( _SPU_ )
    : m_AX( spu_and( scale.m_Scale.m_V, (vector float)Const< vector unsigned int >::Mask(1,0,0,0) ) ),
      m_AY( spu_and( scale.m_Scale.m_V, (vector float)Const< vector unsigned int >::Mask(0,1,0,0) ) ),
      m_AZ( spu_and( scale.m_Scale.m_V, (vector float)Const< vector unsigned int >::Mask(0,0,1,0) ) )
    #elif defined( _PPU_ )
    : m_AX( vec_and( scale.m_Scale.m_V, (vector float)Const< vector unsigned int >::Mask(1,0,0,0) ) ),
      m_AY( vec_and( scale.m_Scale.m_V, (vector float)Const< vector unsigned int >::Mask(0,1,0,0) ) ),
      m_AZ( vec_and( scale.m_Scale.m_V, (vector float)Const< vector unsigned int >::Mask(0,0,1,0) ) )
    #elif defined( _VMX128_ )
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

/// Constructor - Rotation matrix from euler angles - Pitch-Heading-Roll
NOALIAS __forceinline HMat33::HMat33( const C_MatrixRotatePHR PCREF euler )
{
    // Matrix PHR =
    //           [col][X]                            [col][Y]                            [col][Z]
    // [X][row]( cos(h)*cos(r),                      cos(h)*sin(r),                      -sin(h)        )
    // [Y][row]( sin(p)*sin(h)*cos(r)-cos(p)*sin(r), sin(p)*sin(h)*sin(r)+cos(p)*cos(r),  sin(p)*cos(h) )
    // [Z][row]( cos(p)*sin(h)*cos(r)+sin(p)*sin(r), cos(p)*sin(h)*sin(r)-sin(p)*cos(r),  cos(p)*cos(h) )
    
    const HF32 p = euler.Pitch();
    const HF32 h = euler.Heading();
    const HF32 r = euler.Roll();
    
    const HF32 cp( Cos( p ) ); // TODO: Use the HVec4 form of Cos & Sin to do these in parallel!!!
    const HF32 sp( Sin( p ) );
    const HF32 ch( Cos( h ) );
    const HF32 sh( Sin( h ) );
    const HF32 cr( Cos( r ) );
    const HF32 sr( Sin( r ) );
    
    const HVec3 ax( ch*cr,          ch*sr,          -sh    );
    const HVec3 ay( sp*sh*cr-cp*sr, sp*sh*sr+cp*cr,  sp*ch );
    const HVec3 az( cp*sh*cr+sp*sr, cp*sh*sr-sp*cr,  cp*ch );
    
    SetAxes( ax, ay, az );
}

/// Z Axis Constructor
NOALIAS __forceinline HMat33::HMat33( const AxisZ PCREF nz )
{
    const HF32        cos45deg( Constants<HF32>::Sqrt2div2() );
    const C_Normal    axisPosX( C_Normal::AxisPositiveX() );
    const C_Normal    axisPosY( C_Normal::AxisPositiveY() );
    const Scalar_Bool test( Abs( axisPosY | nz ).LE( cos45deg ) );
    const C_Vector    vx1( axisPosY ^ nz );
    const C_Vector    vy2( nz ^ axisPosX );
    const C_Normal    nx1( Normalise(vx1) );
    const C_Normal    ny1( Normalise(nz ^ vx1) );
    const C_Normal    ny2( Normalise(vy2) );
    const C_Normal    nx2( Normalise(vy2 ^ nz) );
    const C_Normal    nx( Select( test, nx1, nx2 ) );
    const C_Normal    ny( Select( test, ny1, ny2 ) );
    SetAxes( nx, ny, nz );
}

// Cross products for deriving third matrix axis from 2 input axes
// const HVec3 ax = ay ^ az;
// const HVec3 ay = az ^ ax;
// const HVec3 az = ax ^ ay;

/// X & Y Axes Constructor: Primary X, Secondary Y
NOALIAS __forceinline HMat33::HMat33( const AxisX PCREF nx, const AxisY PCREF ny )
{
    const C_Normal nz( Normalise(nx ^ ny) );
    SetAxes( nx, nz ^ nx, nz );
}

/// X & Z Axes Constructor: Primary X, Secondary Z
NOALIAS __forceinline HMat33::HMat33( const AxisX PCREF nx, const AxisZ PCREF nz )
{
    const C_Normal ny( Normalise(nz ^ nx) );
    SetAxes( nx, ny, HVec3( nx ^ ny ) );
}

/// Y & X Axes Constructor: Primary Y, Secondary X
NOALIAS __forceinline HMat33::HMat33( const AxisY PCREF ny, const AxisX PCREF nx )
{
    const C_Normal nz( Normalise(nx ^ ny) );
    SetAxes( HVec3( ny ^ nz ), ny, nz );
}

/// Y & Z Axes Constructor: Primary Y, Secondary Z
NOALIAS __forceinline HMat33::HMat33( const AxisY PCREF ny, const AxisZ PCREF nz )
{
    const C_Normal nx( Normalise(ny ^ nz) );
    SetAxes( nx, ny, HVec3( nx ^ ny ) );
}

/// Z & X Axes Constructor: Primary Z, Secondary X
NOALIAS __forceinline HMat33::HMat33( const AxisZ PCREF nz, const AxisX PCREF nx )
{
    const C_Normal ny( Normalise(nz ^ nx) );
    SetAxes( HVec3( ny ^ nz ), ny, nz );
}

/// Z & Y Axes Constructor: Primary Z, Secondary Y
NOALIAS __forceinline HMat33::HMat33( const AxisZ PCREF nz, const AxisY PCREF ny )
{
    const C_Normal nx( Normalise(ny ^ nz) );
    SetAxes( nx, HVec3( nz ^ nx ), nz );
}

/// Copy Assignment
NOALIAS __forceinline const HMat33 HMat33::operator = ( const HMat33& copy )
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX = copy.m_AX;
    m_AY = copy.m_AY;
    m_AZ = copy.m_AZ;
    #elif defined( _VMX128_ )
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

/// Accessor: Set Axes X, Y & Z
NOALIAS __forceinline void HMat33::SetAxes( const HVec3 PCREF ax, const HVec3 PCREF ay, const HVec3 PCREF az ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX = ax.m_V;
    m_AY = ay.m_V;
    m_AZ = az.m_V;
    #elif defined( _VMX128_ )
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

/// /*Private*/ Accessor: Set Axes X, Y & Z
#if defined( _SPU_ ) || defined( _PPU_ )
NOALIAS __forceinline void HMat33::SetAxesXYZ( const vector float ax, const vector float ay, const vector float az )
{
    m_AX = ax;
    m_AY = ay;
    m_AZ = az;
}
#elif defined( _VMX128_ )
NOALIAS __forceinline void HMat33::SetAxesXYZ( const V128 ax, const V128 ay, const V128 az )
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
NOALIAS __forceinline void HMat33::SetAxesXYZ( const V128 PCREF ax, const V128 PCREF ay, const V128 PCREF az ) __restrict
{
    m_AX.m_V = ax;
    m_AY.m_V = ay;
    m_AZ.m_V = az;
}
#endif

/// Accessor: Get Axis X
NOALIAS __forceinline const HVec3 HMat33::AX() const
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = m_AX;
    #elif defined( _VMX128_ )
    result.m_V = __vmrghw( __vmrghw( m_CX, m_CZ ), __vmrghw( m_CY, __vzero() ) );
    #elif defined( _SSE_ )
    result.m_V = m_AX.m_V;
    #else
    result = m_A[MX];
    #endif
    return result;
}

/// Accessor: Get Axis Y
NOALIAS __forceinline const HVec3 HMat33::AY() const
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = m_AY;
    #elif defined( _VMX128_ )
    result.m_V = __vmrglw( __vmrghw( m_CX, m_CZ ), __vmrghw( m_CY, __vzero() ) );
    #elif defined( _SSE_ )
    result.m_V = m_AY.m_V;
    #else
    result = m_A[MY];
    #endif
    return result;
}

/// Accessor: Get Axis Z
NOALIAS __forceinline const HVec3 HMat33::AZ() const
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = m_AZ;
    #elif defined( _VMX128_ )
    result.m_V = __vmrghw( __vmrglw( m_CX, m_CZ ), __vmrglw( m_CY, __vzero() ) );
    #elif defined( _SSE_ )
    result.m_V = m_AZ;
    #else
    result = m_A[MZ];
    #endif
    return result;
}

/// Accessor: Set Axis X
NOALIAS __forceinline void HMat33::AX( const HVec3 PCREF ax ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX = ax.m_V;
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline void HMat33::AY( const HVec3 PCREF ay ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AY = ay.m_V;
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline void HMat33::AZ( const HVec3 PCREF az ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AZ = az.m_V;
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline const HVec3 HMat33::operator [] ( const U32 i ) const __restrict
{
    return m_A[i];
}

/// Element Index Accessor
NOALIAS __forceinline HVec3& HMat33::operator [] ( const U32 i ) __restrict
{
    return m_A[i];
}

#endif

/// Set to Identity Matrix
NOALIAS __forceinline void HMat33::SetIdentity() __restrict
{
    #if defined( _SPU ) || defined( _PPU_ )
    m_AX = Const< vector float >::UnitX();
    m_AY = Const< vector float >::UnitY();
    m_AZ = Const< vector float >::UnitZ();
    #elif defined( _VMX128_ )
    m_CX = c_UnitVectorX;
    m_CY = c_UnitVectorY;
    m_CZ = c_UnitVectorZ;
    #elif defined( _SSE_ )
    m_AX.m_V = c_UnitVectorX;
    m_AY.m_V = c_UnitVectorY;
    m_AZ.m_V = c_UnitVectorZ;
    #else
    SetAxes( HVec3( c_UnitVectorX ),
             HVec3( c_UnitVectorY ),
             HVec3( c_UnitVectorZ ) );
    #endif
}

/// Transpose of Matrix
NOALIAS __forceinline const HMat33 HMat33::Transposed() const __restrict
{
    HMat33 result( UNDEFINED );
    #if defined( _SPU_ )
    //result.m_AX = (vector float)( spu_extract( m_AX, VX ), spu_extract( m_AY, VX ), spu_extract( m_AZ, VX ), 0.0f );
    //result.m_AY = (vector float)( spu_extract( m_AX, VY ), spu_extract( m_AY, VY ), spu_extract( m_AZ, VY ), 0.0f );
    //result.m_AZ = (vector float)( spu_extract( m_AX, VZ ), spu_extract( m_AY, VZ ), spu_extract( m_AZ, VZ ), 0.0f );
    result.m_AX = spu_insert( spu_extract( m_AX, VX ), spu_insert( spu_extract( m_AY, VX ), spu_insert( spu_extract( m_AZ, VX ), spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    result.m_AY = spu_insert( spu_extract( m_AX, VY ), spu_insert( spu_extract( m_AY, VY ), spu_insert( spu_extract( m_AZ, VY ), spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    result.m_AZ = spu_insert( spu_extract( m_AX, VZ ), spu_insert( spu_extract( m_AY, VZ ), spu_insert( spu_extract( m_AZ, VZ ), spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    #elif defined( _PPU_ )
    const vector float zero = Const< vector float >::Zero();
    const vector float tmp0( vec_mergeh( m_AX, m_AZ ) );
    const vector float tmp1( vec_mergeh( m_AY, zero ) );
    const vector float tmp2( vec_mergel( m_AX, m_AZ ) );
    const vector float tmp3( vec_mergel( m_AY, zero ) );
    const vector float ax  ( vec_mergeh( tmp0, tmp1 ) );
    const vector float ay  ( vec_mergel( tmp0, tmp1 ) );
    const vector float az  ( vec_mergeh( tmp2, tmp3 ) );
    result.m_AX = ax;
    result.m_AY = ay;
    result.m_AZ = az;
    #elif defined( _VMX128_ )
    const V128 tmp0( __vmrghw( m_CX, m_CZ      ) );
    const V128 tmp1( __vmrghw( m_CY, __vzero() ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ      ) );
    const V128 tmp3( __vmrglw( m_CY, __vzero() ) );
    const V128 cx  ( __vmrghw( tmp0, tmp1 ) );
    const V128 cy  ( __vmrglw( tmp0, tmp1 ) );
    const V128 cz  ( __vmrghw( tmp2, tmp3 ) );
    //return HMat33( cx, cy, cz );
    result.m_CX = cx;
    result.m_CY = cy;
    result.m_CZ = cz;
    #elif defined( _SSE_ )
    const V128 zero( SIMD_Zero() );
    const V128 tmp0( SIMD_MergeHigh( m_AX.m_V, m_AZ.m_V ) );
    const V128 tmp1( SIMD_MergeHigh( m_AY.m_V, zero     ) );
    const V128 tmp2( SIMD_MergeLow ( m_AX.m_V, m_AZ.m_V ) );
    const V128 tmp3( SIMD_MergeLow ( m_AY.m_V, zero     ) );
    const V128 ax  ( SIMD_MergeHigh( tmp0, tmp1 ) );
    const V128 ay  ( SIMD_MergeLow ( tmp0, tmp1 ) );
    const V128 az  ( SIMD_MergeHigh( tmp2, tmp3 ) );
    //return HMat33( ax, ay, az );
    result.m_AX.m_V = ax;
    result.m_AY.m_V = ay;
    result.m_AZ.m_V = az;
    #else
    const HVec3 ax( m_A[MX][VX], m_A[MY][VX], m_A[MZ][VX] );
    const HVec3 ay( m_A[MX][VY], m_A[MY][VY], m_A[MZ][VY] );
    const HVec3 az( m_A[MX][VZ], m_A[MY][VZ], m_A[MZ][VZ] );
    //return HMat33( ax, ay, az );
    result.m_A[MX] = ax;
    result.m_A[MY] = ay;
    result.m_A[MZ] = az;
    #endif
    return result;
}

/// Transpose this Matrix
NOALIAS __forceinline void HMat33::Transpose() __restrict
{
    #if defined( _SPU_ )
    const vector float ax = m_AX;
    const vector float ay = m_AY;
    const vector float az = m_AZ;
    //m_AX = (vector float)( spu_extract( ax, VX ), spu_extract( ay, VX ), spu_extract( az, VX ), 0.0f );
    //m_AY = (vector float)( spu_extract( ax, VY ), spu_extract( ay, VY ), spu_extract( az, VY ), 0.0f );
    //m_AZ = (vector float)( spu_extract( ax, VZ ), spu_extract( ay, VZ ), spu_extract( az, VZ ), 0.0f );
    m_AX = spu_insert( spu_extract( ax, VX ), spu_insert( spu_extract( ay, VX ), spu_insert( spu_extract( az, VX ), spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    m_AY = spu_insert( spu_extract( ax, VY ), spu_insert( spu_extract( ay, VY ), spu_insert( spu_extract( az, VY ), spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    m_AZ = spu_insert( spu_extract( ax, VZ ), spu_insert( spu_extract( ay, VZ ), spu_insert( spu_extract( az, VZ ), spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    #elif defined( _PPU_ )
    const vector float zero = Const< vector float >::Zero();
    const vector float tmp0( vec_mergeh( m_AX, m_AZ ) );
    const vector float tmp1( vec_mergeh( m_AY, zero ) );
    const vector float tmp2( vec_mergel( m_AX, m_AZ ) );
    const vector float tmp3( vec_mergel( m_AY, zero ) );
    m_AX = vec_mergeh( tmp0, tmp1 );
    m_AY = vec_mergel( tmp0, tmp1 );
    m_AZ = vec_mergeh( tmp2, tmp3 );
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline const HMat33 HMat33::operator * ( const HF32 PCREF s ) const __restrict
{
    HMat33 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_AX = spu_mul( m_AX, s.m_F );
    result.m_AY = spu_mul( m_AY, s.m_F );
    result.m_AZ = spu_mul( m_AZ, s.m_F );
    #elif defined( _PPU_ )
    result.m_AX = vec_mul( m_AX, s.m_F );
    result.m_AY = vec_mul( m_AY, s.m_F );
    result.m_AZ = vec_mul( m_AZ, s.m_F );
    #elif defined( _VMX128_ )
    result.m_CX = __vmulfp( m_CX, s.m_F );
    result.m_CY = __vmulfp( m_CY, s.m_F );
    result.m_CZ = __vmulfp( m_CZ, s.m_F );
    #elif defined( _SSE_ )
    result.m_AX.m_V = _mm_mul_ps( m_AX.m_V, s.m_F );
    result.m_AY.m_V = _mm_mul_ps( m_AY.m_V, s.m_F );
    result.m_AZ.m_V = _mm_mul_ps( m_AZ.m_V, s.m_F );
    #else
    result.m_A[MX] = AX() * s;
    result.m_A[MY] = AY() * s;
    result.m_A[MZ] = AZ() * s;
    #endif
    return result;
}

/// Scalar Left-Multiplication
NOALIAS __forceinline const HMat33 operator * ( const HF32 PCREF s, const HMat33& m )
{
    HMat33 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_AX = spu_mul( m.m_AX, s.m_F );
    result.m_AY = spu_mul( m.m_AY, s.m_F );
    result.m_AZ = spu_mul( m.m_AZ, s.m_F );
    #elif defined( _PPU_ )
    result.m_AX = vec_mul( m.m_AX, s.m_F );
    result.m_AY = vec_mul( m.m_AY, s.m_F );
    result.m_AZ = vec_mul( m.m_AZ, s.m_F );
    #elif defined( _VMX128_ )
    result.m_CX = __vmulfp( m.m_CX, s.m_F );
    result.m_CY = __vmulfp( m.m_CY, s.m_F );
    result.m_CZ = __vmulfp( m.m_CZ, s.m_F );
    #elif defined( _SSE_ )
    result.m_AX.m_V = _mm_mul_ps( m.m_AX.m_V, s.m_F );
    result.m_AY.m_V = _mm_mul_ps( m.m_AY.m_V, s.m_F );
    result.m_AZ.m_V = _mm_mul_ps( m.m_AZ.m_V, s.m_F );
    #else
    result.m_A[MX] = m.m_A[MX] * s;
    result.m_A[MY] = m.m_A[MY] * s;
    result.m_A[MZ] = m.m_A[MZ] * s;
	#endif
    return result;
}

/// Matrix Addition
NOALIAS __forceinline const HMat33 HMat33::operator + ( const HMat33& m ) const __restrict
{
    HMat33 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_AX = spu_add( m_AX, m.m_AX );
    result.m_AY = spu_add( m_AY, m.m_AY );
    result.m_AZ = spu_add( m_AZ, m.m_AZ );
    #elif defined( _PPU_ )
    result.m_AX = vec_add( m_AX, m.m_AX );
    result.m_AY = vec_add( m_AY, m.m_AY );
    result.m_AZ = vec_add( m_AZ, m.m_AZ );
    #elif defined( _VMX128_ )
    result.m_CX = __vaddfp( m_CX, m.m_CX );
    result.m_CY = __vaddfp( m_CY, m.m_CY );
    result.m_CZ = __vaddfp( m_CZ, m.m_CZ );
    #elif defined( _SSE_ )
    result.m_AX.m_V = _mm_add_ps( m_AX.m_V, m.m_AX.m_V );
    result.m_AY.m_V = _mm_add_ps( m_AY.m_V, m.m_AY.m_V );
    result.m_AZ.m_V = _mm_add_ps( m_AZ.m_V, m.m_AZ.m_V );
    #else
    result.m_A[MX] = AX() + m.AX();
    result.m_A[MY] = AY() + m.AY();
    result.m_A[MZ] = AZ() + m.AZ();
    #endif
    return result;
}

/// Matrix Subtraction
NOALIAS __forceinline const HMat33 HMat33::operator - ( const HMat33& m ) const __restrict
{
    HMat33 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_AX = spu_sub( m_AX, m.m_AX );
    result.m_AY = spu_sub( m_AY, m.m_AY );
    result.m_AZ = spu_sub( m_AZ, m.m_AZ );
    #elif defined( _PPU_ )
    result.m_AX = vec_sub( m_AX, m.m_AX );
    result.m_AY = vec_sub( m_AY, m.m_AY );
    result.m_AZ = vec_sub( m_AZ, m.m_AZ );
    #elif defined( _VMX128_ )
    result.m_CX = __vsubfp( m_CX, m.m_CX );
    result.m_CY = __vsubfp( m_CY, m.m_CY );
    result.m_CZ = __vsubfp( m_CZ, m.m_CZ );
    #elif defined( _SSE_ )
    result.m_AX.m_V = _mm_sub_ps( m_AX.m_V, m.m_AX.m_V );
    result.m_AY.m_V = _mm_sub_ps( m_AY.m_V, m.m_AY.m_V );
    result.m_AZ.m_V = _mm_sub_ps( m_AZ.m_V, m.m_AZ.m_V );
    #else
    result.m_A[MX] = AX() - m.AX();
    result.m_A[MY] = AY() - m.AY();
    result.m_A[MZ] = AZ() - m.AZ();
    #endif
    return result;
}

/// Matrix Multiplication
NOALIAS __forceinline const HMat33 HMat33::operator * ( const HMat33& m2 ) const __restrict
{
    HMat33 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    result.m_AX = spu_madd( spu_splat<VX>( m_AX ), m2.m_AX,
                  spu_madd( spu_splat<VY>( m_AX ), m2.m_AY,
                  spu_mul ( spu_splat<VZ>( m_AX ), m2.m_AZ ) ) );
                                      
    result.m_AY = spu_madd( spu_splat<VX>( m_AY ), m2.m_AX,
                  spu_madd( spu_splat<VY>( m_AY ), m2.m_AY,
                  spu_mul ( spu_splat<VZ>( m_AY ), m2.m_AZ ) ) );
                                      
    result.m_AZ = spu_madd( spu_splat<VX>( m_AZ ), m2.m_AX,
                  spu_madd( spu_splat<VY>( m_AZ ), m2.m_AY,
                  spu_mul ( spu_splat<VZ>( m_AZ ), m2.m_AZ ) ) );
    
    #elif defined( _PPU_ )
    
    result.m_AX = vec_madd( vec_splat( m_AX, VX ), m2.m_AX,
                  vec_madd( vec_splat( m_AX, VY ), m2.m_AY,
                  vec_mul ( vec_splat( m_AX, VZ ), m2.m_AZ ) ) );
    
    result.m_AY = vec_madd( vec_splat( m_AY, VX ), m2.m_AX,
                  vec_madd( vec_splat( m_AY, VY ), m2.m_AY,
                  vec_mul ( vec_splat( m_AY, VZ ), m2.m_AZ ) ) );
    
    result.m_AZ = vec_madd( vec_splat( m_AZ, VX ), m2.m_AX,
                  vec_madd( vec_splat( m_AZ, VY ), m2.m_AY,
                  vec_mul ( vec_splat( m_AZ, VZ ), m2.m_AZ ) ) );
    
    #elif defined( _VMX128_ )
    
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
    
    result.m_CX = __vmrghw( __vmrghw( result_cx_x, result_cx_z ), __vmrghw( result_cx_y, __vzero() ) );
    result.m_CY = __vmrghw( __vmrghw( result_cy_x, result_cy_z ), __vmrghw( result_cy_y, __vzero() ) );
    result.m_CZ = __vmrghw( __vmrghw( result_cz_x, result_cz_z ), __vmrghw( result_cz_y, __vzero() ) );
    
    #elif defined( _SSE_ )
    
    result.m_AX = m_AX * m2;
    result.m_AY = m_AY * m2;
    result.m_AZ = m_AZ * m2;
    
    #else
    
    result.m_A[MX] = m_A[MX] * m2;
    result.m_A[MY] = m_A[MY] * m2;
    result.m_A[MZ] = m_A[MZ] * m2;
    
    #endif
    
    return result;
}

/// Matrix * Vector
NOALIAS __forceinline const HVec3 HMat33::operator * ( const HVec3 PCREF v ) const __restrict
{
    HVec3 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    //const vector float cx = (vector float)( spu_extract( m_AX, VX ), spu_extract( m_AY, VX ), spu_extract( m_AZ, VX ), 0.f );
    //const vector float cy = (vector float)( spu_extract( m_AX, VY ), spu_extract( m_AY, VY ), spu_extract( m_AZ, VY ), 0.f );
    //const vector float cz = (vector float)( spu_extract( m_AX, VZ ), spu_extract( m_AY, VZ ), spu_extract( m_AZ, VZ ), 0.f );
    const vector float cx = spu_insert( spu_extract( m_AX, VX ), spu_insert( spu_extract( m_AY, VX ), spu_insert( spu_extract( m_AZ, VX ), spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    const vector float cy = spu_insert( spu_extract( m_AX, VY ), spu_insert( spu_extract( m_AY, VY ), spu_insert( spu_extract( m_AZ, VY ), spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    const vector float cz = spu_insert( spu_extract( m_AX, VZ ), spu_insert( spu_extract( m_AY, VZ ), spu_insert( spu_extract( m_AZ, VZ ), spu_promote( 0.0f, VW ), VZ ), VY ), VX );
    
    result.m_V = spu_madd( spu_splat<VX>( v.m_V ), cx,
                 spu_madd( spu_splat<VY>( v.m_V ), cy,
                 spu_mul ( spu_splat<VZ>( v.m_V ), cz ) ) );
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float tmp0( vec_mergeh( m_AX, m_AZ ) );
    const vector float tmp1( vec_mergeh( m_AY, zero ) );
    const vector float tmp2( vec_mergel( m_AX, m_AZ ) );
    const vector float tmp3( vec_mergel( m_AY, zero ) );
    const vector float cx  ( vec_mergeh( tmp0, tmp1 ) );
    const vector float cy  ( vec_mergel( tmp0, tmp1 ) );
    const vector float cz  ( vec_mergeh( tmp2, tmp3 ) );
    
    result.m_V = vec_madd( vec_splat( v.m_V, VX ), cx,
                 vec_madd( vec_splat( v.m_V, VY ), cy,
                 vec_mul ( vec_splat( v.m_V, VZ ), cz ) ) );
    
    #elif defined( _VMX128_ )
    
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
    
    result.m_V = __vmrghw( __vmrghw( result_x, result_z ),
                           __vmrghw( result_y, __vzero() ) );
    
    #elif defined( _SSE_ )
    
    const V128 xxxx( SIMD_DotProduct3( m_AX.m_V, v.m_V ) );
    const V128 yyyy( SIMD_DotProduct3( m_AY.m_V, v.m_V ) );
    const V128 zzzz( SIMD_DotProduct3( m_AZ.m_V, v.m_V ) );
    
    result.m_V = SIMD_Vector3( xxxx, yyyy, zzzz );
    
    #else
    
    const F32 x = m_A[MX][VX] * v[VX] + m_A[MX][VY] * v[VY] + m_A[MX][VZ] * v[VZ]; // Matrix * Vector
    const F32 y = m_A[MY][VX] * v[VX] + m_A[MY][VY] * v[VY] + m_A[MY][VZ] * v[VZ];
    const F32 z = m_A[MZ][VX] * v[VX] + m_A[MZ][VY] * v[VY] + m_A[MZ][VZ] * v[VZ];
    result = HVec3( x, y, z );
    
    #endif
    
    return result;
}

/// Combine with HVec3 position to form a HMat43
NOALIAS __forceinline const HMat43 HMat33::operator || ( const HVec3 PCREF pos ) const __restrict
{
    HMat43 result( UNDEFINED );
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_AX = m_AX;
    result.m_AY = m_AY;
    result.m_AZ = m_AZ;
    result.m_AW = pos.m_V;
    #elif defined( _VMX128_ )
    result.m_CX = __vrlimi( m_CX, pos.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    result.m_CY = __vrlimi( m_CY, pos.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    result.m_CZ = __vrlimi( m_CZ, pos.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #elif defined( _SSE_ )
    result.m_Rotate.m_AX.m_V = m_AX.m_V;
    result.m_Rotate.m_AY.m_V = m_AY.m_V;
    result.m_Rotate.m_AZ.m_V = m_AZ.m_V;
    result.m_Translate.m_V = pos.m_V;
    #else
    result.m_Rotate.m_A[MX] = m_A[MX];
    result.m_Rotate.m_A[MY] = m_A[MY];
    result.m_Rotate.m_A[MZ] = m_A[MZ];
    result.m_Translate = pos;
    #endif
    return result;
}

/// Inverse of Rotation - Faster but only valid for matrices with normalised & orthogonal rotations!
NOALIAS __forceinline const HMat33 HMat33::InverseRotation() const __restrict
{
    return Transposed();
}

/// Full Matrix Inverse - This will FAIL if determinant is zero!
NOALIAS __forceinline const HMat33 HMat33::Inverse() const __restrict
{
    const HF32 nearlyZero( 1.0e-30f );
    //const HF32 nearlyZero( c_ScalarEpsilon );
    
    // NOTE: The determinants of a 3x3 matrix and it's transpose are identical!
    const HF32 determinant( Determinant() );
    
    const HF32 reciprocalDeterminant = Reciprocal( determinant );
    
	#if defined( _SPU_ )

    const HVec3 ax( AY().Y() * AZ().Z() - AY().Z() * AZ().Y(),
                    AX().Z() * AZ().Y() - AX().Y() * AZ().Z(),
                    AX().Y() * AY().Z() - AX().Z() * AY().Y() );
    
    const HVec3 ay( AY().Z() * AZ().X() - AY().X() * AZ().Z(),
                    AX().X() * AZ().Z() - AX().Z() * AZ().X(),
                    AX().Z() * AY().X() - AX().X() * AY().Z() );
    
    const HVec3 az( AY().X() * AZ().Y() - AY().Y() * AZ().X(),
                    AX().Y() * AZ().X() - AX().X() * AZ().Y(),
                    AX().X() * AY().Y() - AX().Y() * AY().X() );
    
    #elif defined( _PPU_ )
    
    HVec3 ax( UNDEFINED );
    HVec3 ay( UNDEFINED );
    HVec3 az( UNDEFINED );
    
    ax.m_V = vec_nmsub( vec_permute<VZ,VY,VZ>( m_AY, m_AX, m_AX ),
                        vec_permute<VY,VZ,VY>( m_AZ, m_AZ, m_AY ),
               vec_mul( vec_permute<VY,VZ,VY>( m_AY, m_AX, m_AX ),
                        vec_permute<VZ,VY,VZ>( m_AZ, m_AZ, m_AY ) ) );
    
    ay.m_V = vec_nmsub( vec_permute<VX,VZ,VX>( m_AY, m_AX, m_AX ),
                        vec_permute<VZ,VX,VZ>( m_AZ, m_AZ, m_AY ),
               vec_mul( vec_permute<VZ,VX,VZ>( m_AY, m_AX, m_AX ),
                        vec_permute<VX,VZ,VX>( m_AZ, m_AZ, m_AY ) ) );
    
    az.m_V = vec_nmsub( vec_permute<VY,VX,VY>( m_AY, m_AX, m_AX ),
                        vec_permute<VX,VY,VX>( m_AZ, m_AZ, m_AY ),
               vec_mul( vec_permute<VX,VY,VX>( m_AY, m_AX, m_AX ),
                        vec_permute<VY,VX,VY>( m_AZ, m_AZ, m_AY ) ) );
    
    #elif defined( _VMX128_ )
    
    static const int __ = VW;
    
    const V128 cx( SIMD_SubtractMultiply( SIMD_Multiply( SIMD_Permute<VY,VZ,VY,__>( m_CY, m_CX, m_CX ),
                                                         SIMD_Permute<VZ,VY,VZ,__>( m_CZ, m_CZ, m_CY ) ),
                                                         SIMD_Permute<VZ,VY,VZ,__>( m_CY, m_CX, m_CX ),
                                                         SIMD_Permute<VY,VZ,VY,__>( m_CZ, m_CZ, m_CY ) ) );
                                                                                                    
    const V128 cy( SIMD_SubtractMultiply( SIMD_Multiply( SIMD_Permute<VZ,VX,VZ,__>( m_CY, m_CX, m_CX ),
                                                         SIMD_Permute<VX,VZ,VX,__>( m_CZ, m_CZ, m_CY ) ),
                                                         SIMD_Permute<VX,VZ,VX,__>( m_CY, m_CX, m_CX ),
                                                         SIMD_Permute<VZ,VX,VZ,__>( m_CZ, m_CZ, m_CY ) ) );
                                                                                                    
    const V128 cz( SIMD_SubtractMultiply( SIMD_Multiply( SIMD_Permute<VX,VY,VX,__>( m_CY, m_CX, m_CX ),
                                                         SIMD_Permute<VY,VX,VY,__>( m_CZ, m_CZ, m_CY ) ),
                                                         SIMD_Permute<VY,VX,VY,__>( m_CY, m_CX, m_CX ),
                                                         SIMD_Permute<VX,VY,VX,__>( m_CZ, m_CZ, m_CY ) ) );
    
    #elif defined( _SSE_ )
    
    static const int __ = VW;
    
    const HVec3 ax( SIMD_SubtractMultiply( SIMD_Multiply( SIMD_Permute<VY,VZ,VY,__>( m_AY.m_V, m_AX.m_V, m_AX.m_V ),
                                                          SIMD_Permute<VZ,VY,VZ,__>( m_AZ.m_V, m_AZ.m_V, m_AY.m_V ) ),
                                                          SIMD_Permute<VZ,VY,VZ,__>( m_AY.m_V, m_AX.m_V, m_AX.m_V ),
                                                          SIMD_Permute<VY,VZ,VY,__>( m_AZ.m_V, m_AZ.m_V, m_AY.m_V ) ) );
    
    const HVec3 ay( SIMD_SubtractMultiply( SIMD_Multiply( SIMD_Permute<VZ,VX,VZ,__>( m_AY.m_V, m_AX.m_V, m_AX.m_V ),
                                                          SIMD_Permute<VX,VZ,VX,__>( m_AZ.m_V, m_AZ.m_V, m_AY.m_V ) ),
                                                          SIMD_Permute<VX,VZ,VX,__>( m_AY.m_V, m_AX.m_V, m_AX.m_V ),
                                                          SIMD_Permute<VZ,VX,VZ,__>( m_AZ.m_V, m_AZ.m_V, m_AY.m_V ) ) );
    
    const HVec3 az( SIMD_SubtractMultiply( SIMD_Multiply( SIMD_Permute<VX,VY,VX,__>( m_AY.m_V, m_AX.m_V, m_AX.m_V ),
                                                          SIMD_Permute<VY,VX,VY,__>( m_AZ.m_V, m_AZ.m_V, m_AY.m_V ) ),
                                                          SIMD_Permute<VY,VX,VY,__>( m_AY.m_V, m_AX.m_V, m_AX.m_V ),
                                                          SIMD_Permute<VX,VY,VX,__>( m_AZ.m_V, m_AZ.m_V, m_AY.m_V ) ) );
    
    #else
    
    const HVec3 ax( m_A[MY][VY] * m_A[MZ][VZ] - m_A[MY][VZ] * m_A[MZ][VY],
                    m_A[MX][VZ] * m_A[MZ][VY] - m_A[MX][VY] * m_A[MZ][VZ],
                    m_A[MX][VY] * m_A[MY][VZ] - m_A[MX][VZ] * m_A[MY][VY] );
    
    const HVec3 ay( m_A[MY][VZ] * m_A[MZ][VX] - m_A[MY][VX] * m_A[MZ][VZ],
                    m_A[MX][VX] * m_A[MZ][VZ] - m_A[MX][VZ] * m_A[MZ][VX],
                    m_A[MX][VZ] * m_A[MY][VX] - m_A[MX][VX] * m_A[MY][VZ] );
    
    const HVec3 az( m_A[MY][VX] * m_A[MZ][VY] - m_A[MY][VY] * m_A[MZ][VX],
                    m_A[MX][VY] * m_A[MZ][VX] - m_A[MX][VX] * m_A[MZ][VY],
                    m_A[MX][VX] * m_A[MY][VY] - m_A[MX][VY] * m_A[MY][VX] );
    
    #endif
    
    #if defined( _VMX128_ )
    
    //const Scalar_Bool determinant_valid( Abs( determinant ).GE( nearlyZero ) );
    const __vector4 abs_mask( c_VectorAbsMask );
    const __vector4 determinant_valid( __vcmpgefp( __vand( determinant.m_F, abs_mask ), nearlyZero.m_F ) );
    
    HMat33 result( UNDEFINED );
    result.m_CX = __vsel( __vzero(), __vmulfp( cx, reciprocalDeterminant.m_F ), determinant_valid );
    result.m_CY = __vsel( __vzero(), __vmulfp( cy, reciprocalDeterminant.m_F ), determinant_valid );
    result.m_CZ = __vsel( __vzero(), __vmulfp( cz, reciprocalDeterminant.m_F ), determinant_valid );
    return result;
    
    #else
    
    const HVec3 vzero;
    
    const Scalar_Bool determinant_valid( Abs( determinant ).GE( nearlyZero ) );
    
    return HMat33( Select( determinant_valid, ax * reciprocalDeterminant, vzero ),
                   Select( determinant_valid, ay * reciprocalDeterminant, vzero ),
                   Select( determinant_valid, az * reciprocalDeterminant, vzero ) );
    
    #endif
}

/// Trace of the matrix (sum of the diagonal elements).
NOALIAS __forceinline const HF32 HMat33::Trace() const __restrict
{
    HF32 result( UNDEFINED );
    #if defined( _SPU_ )
    const vector float ax_x = spu_splat<VX>( m_AX );
    const vector float ay_y = spu_splat<VY>( m_AY );
    const vector float az_z = spu_splat<VZ>( m_AZ );
    result.m_F = spu_add( ax_x, spu_add( ay_y, az_z ) );
    #elif defined( _PPU_ )
    const vector float ax_x = vec_splat( m_AX, VX );
    const vector float ay_y = vec_splat( m_AY, VY );
    const vector float az_z = vec_splat( m_AZ, VZ );
    result.m_F = vec_add( ax_x, vec_add( ay_y, az_z ) );
    #else
	result = AX().X() + AY().Y() + AZ().Z();
	#endif
    return result;
}

/// Matrix Determinant
NOALIAS __forceinline const HF32 HMat33::Determinant() const __restrict
{
    HF32 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    
    const vector unsigned char shuffle_yxx = spu_shuffle_pattern_word<V0Y,V0X,V0X,V1W>();
    const vector unsigned char shuffle_zzy = spu_shuffle_pattern_word<V0Z,V0Z,V0Y,V1W>();
    
    const vector float ax = m_AX;
    const vector float ay = m_AY;
    const vector float az = m_AZ;
    
    const vector float ay_yxx = spu_shuffle( ay, zero, shuffle_yxx );
    const vector float ay_zzy = spu_shuffle( ay, zero, shuffle_zzy );
    const vector float az_yxx = spu_shuffle( az, zero, shuffle_yxx );
    const vector float az_zzy = spu_shuffle( az, zero, shuffle_zzy );
    
    const vector float xyz = spu_mul( ax, spu_nmsub( ay_zzy, az_yxx,
                                            spu_mul( ay_yxx, az_zzy ) ) );
    
    const vector float x = spu_splat<VX>( xyz );
    const vector float y = spu_splat<VY>( xyz );
    const vector float z = spu_splat<VZ>( xyz );
    result.m_F = spu_add( spu_sub( x, y ), z );

    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    
    const vector unsigned char permute_yxx = vec_permute_pattern_word<VY,VX,VX,7>();
    const vector unsigned char permute_zzy = vec_permute_pattern_word<VZ,VZ,VY,7>();
    
    const vector float ax = m_AX;
    const vector float ay = m_AY;
    const vector float az = m_AZ;
    
    const vector float ay_yxx = vec_perm( ay, zero, permute_yxx );
    const vector float ay_zzy = vec_perm( ay, zero, permute_zzy );
    const vector float az_yxx = vec_perm( az, zero, permute_yxx );
    const vector float az_zzy = vec_perm( az, zero, permute_zzy );
    
    const vector float xyz = vec_mul( ax, vec_nmsub( ay_zzy, az_yxx,
                                            vec_mul( ay_yxx, az_zzy ) ) );
    
    const vector float x = vec_splat( xyz, VX );
    const vector float y = vec_splat( xyz, VY );
    const vector float z = vec_splat( xyz, VZ );
    result.m_F = vec_add( vec_sub( x, y ), z );
    
    #elif defined( _SIMD_ )
    
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
    
    result.m_F = SIMD_ScalarAdd( SIMD_ScalarSubtract( SIMD_Scalar_Vector3X( xyz ), SIMD_Scalar_Vector3Y( xyz ) ), SIMD_Scalar_Vector3Z( xyz ) );
    
    #else
    
    result.m_F = m_A[MX][VX] * ( m_A[MY][VY] * m_A[MZ][VZ] - m_A[MY][VZ] * m_A[MZ][VY] )
               - m_A[MX][VY] * ( m_A[MY][VX] * m_A[MZ][VZ] - m_A[MY][VZ] * m_A[MZ][VX] )
               + m_A[MX][VZ] * ( m_A[MY][VX] * m_A[MZ][VY] - m_A[MY][VY] * m_A[MZ][VX] );
    
    #endif
    
    return result;
}

/// Normalise matrix: Priority to Z axis, secondary Y
NOALIAS __forceinline void HMat33::NormaliseZY() __restrict
{
    #if defined( _SPU_ )
    
    vector float ax;
    vector float ay = m_AY;
    vector float az = m_AZ;
    
    ax = spu_cross_product3( ay, az );
    ay = spu_cross_product3( az, ax );
    
    const vector float ax_len2 = spu_length_squared3( ax );
    const vector float ay_len2 = spu_length_squared3( ay );
    const vector float az_len2 = spu_length_squared3( az );
    
    vector float ax_rcp_len, ay_rcp_len, az_rcp_len;
    SPU_VECTOR_RECIPROCAL_SQRT3( ax_len2, ay_len2, az_len2, ax_rcp_len, ay_rcp_len, az_rcp_len );
    
    m_AX = spu_mul( ax, ax_rcp_len );
    m_AY = spu_mul( ay, ay_rcp_len );
    m_AZ = spu_mul( az, az_rcp_len );
    
    #elif defined( _PPU_ )
    
    vector float ax;
    vector float ay = m_AY;
    vector float az = m_AZ;
    
    ax = vec_cross_product3( ay, az );
    ay = vec_cross_product3( az, ax );
    
    const vector float ax_len2 = vec_length_squared3( ax );
    const vector float ay_len2 = vec_length_squared3( ay );
    const vector float az_len2 = vec_length_squared3( az );
    
    vector float ax_rcp_len, ay_rcp_len, az_rcp_len;
    PPU_VECTOR_RECIPROCAL_SQRT3( ax_len2, ay_len2, az_len2, ax_rcp_len, ay_rcp_len, az_rcp_len );
    
    m_AX = vec_mul( ax, ax_rcp_len );
    m_AY = vec_mul( ay, ay_rcp_len );
    m_AZ = vec_mul( az, az_rcp_len );
    
    #elif defined( _VMX128_ )
    
    V128 tmp0 = __vmrghw( m_CX, m_CZ      );
    V128 tmp1 = __vmrghw( m_CY, __vzero() );
    V128 tmp2 = __vmrglw( m_CX, m_CZ      );
    V128 tmp3 = __vmrglw( m_CY, __vzero() );
    
    V128 m_ax = __vmrghw( tmp0, tmp1 );
    V128 m_ay = __vmrglw( tmp0, tmp1 );
    V128 m_az = __vmrghw( tmp2, tmp3 );
    
    // Cross products for deriving third matrix axis from 2 input axes
    // const HVec3 ax = ay ^ az;
    // const HVec3 ay = az ^ ax;
    // const HVec3 az = ax ^ ay;
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
    
    const HVec3 az( (HVec3)Normalise( m_A[MZ] ) );
    const HVec3 ax( (HVec3)Normalise( m_A[MY] ^ az ) );
    const HVec3 ay( az ^ ax );
    m_A[MX] = ax;
    m_A[MY] = ay;
    m_A[MZ] = az;
    
    #endif
}
    
/// Normalised copy of matrix: Priority to Z axis, secondary Y
NOALIAS __forceinline const HMat33 HMat33::NormalisedZY() const __restrict
{
    HMat33 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    vector float ax;
    vector float ay = m_AY;
    vector float az = m_AZ;
    
    ax = spu_cross_product3( ay, az );
    ay = spu_cross_product3( az, ax );
    
    const vector float ax_len2 = spu_length_squared3( ax );
    const vector float ay_len2 = spu_length_squared3( ay );
    const vector float az_len2 = spu_length_squared3( az );
    
    vector float ax_rcp_len, ay_rcp_len, az_rcp_len;
    SPU_VECTOR_RECIPROCAL_SQRT3( ax_len2, ay_len2, az_len2, ax_rcp_len, ay_rcp_len, az_rcp_len );
    
    result.m_AX = spu_mul( ax, ax_rcp_len );
    result.m_AY = spu_mul( ay, ay_rcp_len );
    result.m_AZ = spu_mul( az, az_rcp_len );
    
    #elif defined( _PPU_ )
    
    vector float ax;
    vector float ay = m_AY;
    vector float az = m_AZ;
    
    ax = vec_cross_product3( ay, az );
    ay = vec_cross_product3( az, ax );
    
    const vector float ax_len2 = vec_length_squared3( ax );
    const vector float ay_len2 = vec_length_squared3( ay );
    const vector float az_len2 = vec_length_squared3( az );
    
    vector float ax_rcp_len, ay_rcp_len, az_rcp_len;
    PPU_VECTOR_RECIPROCAL_SQRT3( ax_len2, ay_len2, az_len2, ax_rcp_len, ay_rcp_len, az_rcp_len );
    
    result.m_AX = vec_mul( ax, ax_rcp_len );
    result.m_AY = vec_mul( ay, ay_rcp_len );
    result.m_AZ = vec_mul( az, az_rcp_len );
    
    #elif defined( _VMX128_ )
    
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
    
    result.m_CX = __vmrghw( tmp0, tmp1 );
    result.m_CY = __vmrglw( tmp0, tmp1 );
    result.m_CZ = __vmrghw( tmp2, tmp3 );
    
    #elif defined( _SSE_ )
    
    const V128 az( SIMD_Normalise3( m_AZ.m_V ) );
    const V128 ax( SIMD_Normalise3( SIMD_CrossProduct( m_AY.m_V, az ) ) );
    const V128 ay( SIMD_CrossProduct( az, ax ) );
    
    result.m_AX.m_V = ax;
    result.m_AY.m_V = ay;
    result.m_AZ.m_V = az;
    
    #else
    
    const HVec3 az( (HVec3)Normalise( m_A[MZ] ) );
    const HVec3 ax( (HVec3)Normalise( m_A[MY] ^ az ) );
    const HVec3 ay( az ^ ax );
    
    result.AX( ax );
    result.AY( ay );
    result.AZ( az );
    
    #endif
    
    return result;
}

/// Finite test
NOALIAS __forceinline const Scalar_Bool HMat33::Finite() const __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    return HVec3( m_AX ).Finite() &
           HVec3( m_AY ).Finite() &
           HVec3( m_AZ ).Finite();
    #elif defined( _VMX128_ )
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

template < typename T_Stream > __forceinline void HMat33::Deserialise( T_Stream& stream ) __restrict
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

template < typename T_Stream > __forceinline void HMat33::Serialise( T_Stream& stream ) __restrict
{
	Maths::HVec3 x(AX());
	Maths::HVec3 y(AY());
	Maths::HVec3 z(AZ());
	x.Serialise(stream);
	y.Serialise(stream);
	z.Serialise(stream);
    /*AX().Serialise(stream);
    AY().Serialise(stream);
    AZ().Serialise(stream);*/
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
NOALIAS __forceinline HMat43::HMat43()
    #if defined( _SPU_ ) || defined( _PPU_ )
    : m_AX( Const< vector float >::UnitX() ),
      m_AY( Const< vector float >::UnitY() ),
      m_AZ( Const< vector float >::UnitZ() ),
      m_AW( Const< vector float >::Zero() )
    #elif defined( _VMX128_ )
    : m_CX( c_UnitVectorX ),
      m_CY( c_UnitVectorY ),
      m_CZ( c_UnitVectorZ )
    #endif
{
    // For _SSE_ && !_SIMD_ matrix gets intialised in the m_Rotate & m_Translate default constructors!
}

/// Undefined Constructor
NOALIAS __forceinline HMat43::HMat43( const _UNDEFINED /*undefined*/ )
    #if defined( _SSE_ ) || !defined( _SIMD_ )
    : m_Rotate( UNDEFINED ),
      m_Translate( UNDEFINED )
    #endif
{
}

/// Constructor: initialises to the zero matrix.
NOALIAS __forceinline HMat43::HMat43( const _ZERO_MATRIX )
    #if defined( _SPU_ ) || defined( _PPU_ ) // || defined( _SSE_ )
    : m_AX( Const< vector float >::Zero() ),
      m_AY( Const< vector float >::Zero() ),
      m_AZ( Const< vector float >::Zero() ),
      m_AW( Const< vector float >::Zero() )
    #elif defined( _VMX128_ )
    : m_CX( __vzero() ),
      m_CY( __vzero() ),
      m_CZ( __vzero() )
    #else
    : m_Rotate( ZERO_MATRIX ),
      m_Translate( c_VectorZero )
    #endif
{
    //#if !defined( _SIMD_ )
	//const HVec3 zero;
	//AX( zero );
	//AY( zero );
	//AZ( zero );
	//AW( zero );
	//#endif
}

/// Explicit HMat33 Constructor
NOALIAS __forceinline HMat43::HMat43( const HMat33& src ) :
    #if defined( _SPU_ )
    m_AX( src.m_AX ),
    m_AY( src.m_AY ),
    m_AZ( src.m_AZ ),
    m_AW( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    m_AX( src.m_AX ),
    m_AY( src.m_AY ),
    m_AZ( src.m_AZ ),
    m_AW( Const< vector float >::Zero() )
    #elif defined( _VMX128_ )
    m_CX( __vrlimi( src.m_CX, __vzero(), VMX_MASK_INSERT_W, VMX_ROTATE_NONE ) ),
    m_CY( __vrlimi( src.m_CY, __vzero(), VMX_MASK_INSERT_W, VMX_ROTATE_NONE ) ),
    m_CZ( __vrlimi( src.m_CZ, __vzero(), VMX_MASK_INSERT_W, VMX_ROTATE_NONE ) )
    #else
    m_Rotate( src )
    #endif
{
}

/// Copy Constructor
NOALIAS __forceinline HMat43::HMat43( const HMat43& copy ) :
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX( copy.m_AX ),
    m_AY( copy.m_AY ),
    m_AZ( copy.m_AZ ),
    m_AW( copy.m_AW )
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline HMat43::HMat43( const HMat44& src ) :
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX( src.m_AX ),
    m_AY( src.m_AY ),
    m_AZ( src.m_AZ ),
    m_AW( src.m_AW )
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline HMat43::HMat43( const HMat33& rotate, const HVec3 PCREF translate ) :
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX( rotate.m_AX ),
    m_AY( rotate.m_AY ),
    m_AZ( rotate.m_AZ ),
    m_AW( translate.m_V )
    #elif defined( _VMX128_ )
    m_CX( __vrlimi( rotate.m_CX, translate.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W ) ),
    m_CY( __vrlimi( rotate.m_CY, translate.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W ) ),
    m_CZ( __vrlimi( rotate.m_CZ, translate.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W ) )
    #else
    m_Rotate( rotate ),
    m_Translate( translate )
    #endif
{
}

#if defined( _SPU_ ) || defined( _PPU_ )
/// vector float Axis XYZW Constructor
NOALIAS __forceinline HMat43::HMat43( const vector float ax, const vector float ay, const vector float az, const vector float aw ) :
    m_AX( ax ),
    m_AY( ay ),
    m_AZ( az ),
    m_AW( aw )
{
}
#elif defined( _VMX128_ )
/// V128 Column XYZW Constructor
NOALIAS __forceinline HMat43::HMat43( const V128 cx, const V128 cy, const V128 cz ) :
    m_CX( cx ),
    m_CY( cy ),
    m_CZ( cz )
{
}
#elif defined( _SSE_ )
/// V128 Axis XYZW Constructor
NOALIAS __forceinline HMat43::HMat43( const V128 PCREF ax, const V128 PCREF ay, const V128 PCREF az, const V128 PCREF aw ) :
    m_Rotate( ax, ay, az ),
    m_Translate( aw )
{
}
#endif

/// HVec3 Axis XYZW Constructor
NOALIAS __forceinline HMat43::HMat43( const HVec3 PCREF ax, const HVec3 PCREF ay, const HVec3 PCREF az, const HVec3 PCREF aw )
    #if defined( _SPU_ ) || defined( _PPU_ )
    : m_AX( ax.m_V ),
      m_AY( ay.m_V ),
      m_AZ( az.m_V ),
      m_AW( aw.m_V )
    #elif !defined( _VMX128_ )
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
NOALIAS __forceinline HMat43::HMat43( const AxisX PCREF ax, const AxisY PCREF ay, const HVec3 PCREF aw )
    #if defined( _SSE_ ) || !defined( _SIMD_ )
    : m_Rotate( ax, ay ),
      m_Translate( aw )
    #endif
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    const HMat33 rotate( ax, ay );
    m_AX = rotate.m_AX;
    m_AY = rotate.m_AY;
    m_AZ = rotate.m_AZ;
    m_AW = aw.m_V;
    #elif defined( _VMX128_ )
    const HMat33 rotate( ax, ay );
    m_CX = __vrlimi( rotate.m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( rotate.m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( rotate.m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #endif
}

/// Constructor: Primary axis X, secondary axis Z
NOALIAS __forceinline HMat43::HMat43( const AxisX PCREF ax, const AxisZ PCREF az, const HVec3 PCREF aw )
    #if defined( _SSE_ ) || !defined( _SIMD_ )
    : m_Rotate( ax, az ),
      m_Translate( aw )
    #endif
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    const HMat33 rotate( ax, az );
    m_AX = rotate.m_AX;
    m_AY = rotate.m_AY;
    m_AZ = rotate.m_AZ;
    m_AW = aw.m_V;
    #elif defined( _VMX128_ )
    const HMat33 rotate( ax, az );
    m_CX = __vrlimi( rotate.m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( rotate.m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( rotate.m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #endif
}

/// Constructor: Primary axis Y, secondary axis X
NOALIAS __forceinline HMat43::HMat43( const AxisY PCREF ay, const AxisX PCREF ax, const HVec3 PCREF aw )
    #if defined( _SSE_ ) || !defined( _SIMD_ )
    : m_Rotate( ay, ax ),
      m_Translate( aw )
    #endif
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    const HMat33 rotate( ay, ax );
    m_AX = rotate.m_AX;
    m_AY = rotate.m_AY;
    m_AZ = rotate.m_AZ;
    m_AW = aw.m_V;
    #elif defined( _VMX128_ )
    const HMat33 rotate( ay, ax );
    m_CX = __vrlimi( rotate.m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( rotate.m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( rotate.m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #endif
}

/// Constructor: Primary axis Y, secondary axis Z
NOALIAS __forceinline HMat43::HMat43( const AxisY PCREF ay, const AxisZ PCREF az, const HVec3 PCREF aw )
    #if defined( _SSE_ ) || !defined( _SIMD_ )
    : m_Rotate( ay, az ),
      m_Translate( aw )
    #endif
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    const HMat33 rotate( ay, az );
    m_AX = rotate.m_AX;
    m_AY = rotate.m_AY;
    m_AZ = rotate.m_AZ;
    m_AW = aw.m_V;
    #elif defined( _VMX128_ )
    const HMat33 rotate( ay, az );
    m_CX = __vrlimi( rotate.m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( rotate.m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( rotate.m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #endif
}

/// Constructor: Primary axis Z, secondary axis X
NOALIAS __forceinline HMat43::HMat43( const AxisZ PCREF az, const AxisX PCREF ax, const HVec3 PCREF aw )
    #if defined( _SSE_ ) || !defined( _SIMD_ )
    : m_Rotate( az, ax ),
      m_Translate( aw )
    #endif
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    const HMat33 rotate( az, ax );
    m_AX = rotate.m_AX;
    m_AY = rotate.m_AY;
    m_AZ = rotate.m_AZ;
    m_AW = aw.m_V;
    #elif defined( _VMX128_ )
    const HMat33 rotate( az, ax );
    m_CX = __vrlimi( rotate.m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( rotate.m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( rotate.m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #endif
}

/// Constructor: Primary axis Z, secondary axis Y
NOALIAS __forceinline HMat43::HMat43( const AxisZ PCREF az, const AxisY PCREF ay, const HVec3 PCREF aw )
    #if defined( _SSE_ ) || !defined( _SIMD_ )
    : m_Rotate( az, ay ),
      m_Translate( aw )
    #endif
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    const HMat33 rotate( az, ay );
    m_AX = rotate.m_AX;
    m_AY = rotate.m_AY;
    m_AZ = rotate.m_AZ;
    m_AW = aw.m_V;
    #elif defined( _VMX128_ )
    const HMat33 rotate( az, ay );
    m_CX = __vrlimi( rotate.m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( rotate.m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( rotate.m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #endif
}

/// Scale-Rotate-Translate Constructor
NOALIAS __forceinline HMat43::HMat43( const C_MatrixSRT PCREF srt )
    #if defined( _SSE_ ) || !defined( _SIMD_ )
    : m_Rotate( HMat33( srt.Scale() ) * HMat33( srt.Rotate() ) ),
      m_Translate( srt.Translate() )
    #endif
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    const HMat33 rotate( HMat33( srt.Scale() ) * HMat33( srt.Rotate() ) );
    m_AX = rotate.m_AX;
    m_AY = rotate.m_AY;
    m_AZ = rotate.m_AZ;
    m_AW = srt.Translate().m_Translate.m_V;
    #elif defined( _VMX128_ )
    const HMat33 rotate( HMat33( srt.Scale() ) * HMat33( srt.Rotate() ) );
    const V128 aw( srt.Translate().m_Translate.m_V );
    m_CX = __vrlimi( rotate.m_CX, aw, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( rotate.m_CY, aw, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( rotate.m_CZ, aw, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #endif
}


/// Reflection Plane Constructor
NOALIAS __forceinline HMat43::HMat43( const C_Plane PCREF plane )
{
    // XMVECTOR               P;
    // XMVECTOR               S;
    // XMVECTOR               A, B, C, D;
    // XMMATRIX               M;
    // static CONST XMVECTOR  NegativeTwo = {-2.0f, -2.0f, -2.0f, 0.0f};
    
    // XMASSERT(!XMVector3Equal(ReflectionPlane, XMVectorZero()));
    // XMASSERT(!XMPlaneIsInfinite(ReflectionPlane));
    
    // P = XMPlaneNormalize(ReflectionPlane);
    // S = XMVectorMultiply(P, NegativeTwo);
    
    // A = XMVectorSplatX(P);
    // B = XMVectorSplatY(P);
    // C = XMVectorSplatZ(P);
    // D = XMVectorSplatW(P);
    
    // M.r[0] = XMVectorMultiplyAdd(A, S, g_XMIdentityR0);
    // M.r[1] = XMVectorMultiplyAdd(B, S, g_XMIdentityR1);
    // M.r[2] = XMVectorMultiplyAdd(C, S, g_XMIdentityR2);
    // M.r[3] = XMVectorMultiplyAdd(D, S, g_XMIdentityR3);
    
    // P = normalize(Plane);
    // -2 * P.a * P.a + 1  -2 * P.b * P.a      -2 * P.c * P.a        0
    // -2 * P.a * P.b      -2 * P.b * P.b + 1  -2 * P.c * P.b        0
    // -2 * P.a * P.c      -2 * P.b * P.c      -2 * P.c * P.c + 1    0
    // -2 * P.a * P.d      -2 * P.b * P.d      -2 * P.c * P.d        1
    
    const HMat44 I( Constants< HMat44 >::Identity() );
    const HVec4  p( plane.m_V );
    const HF32   minusTwo( -Constants<HF32>::Two() );
    const HF32   zero( Constants<HF32>::Zero() );
    const HVec4  k( minusTwo, minusTwo, minusTwo, zero );
    const HVec4  s( p * k );
    const HVec4  a( p.X() );
    const HVec4  b( p.Y() );
    const HVec4  c( p.Z() );
    const HVec4  d( p.W() );
    
    SetAxes( a * s + I.AX(),
             b * s + I.AY(),
             c * s + I.AZ(),
             d * s + I.AW() );
}


#if ( DIRECT3D_VERSION >= 0x0900 )

/// Convert to D3DMATRIX
NOALIAS __forceinline HMat43::operator const D3DMATRIX () const __restrict
{
    return (D3DMATRIX)HMat44( *this );
}

#endif

/// Accessor: Get Rotation
NOALIAS __forceinline const HMat33 HMat43::Rotation() const __restrict
{
    HMat33 result( UNDEFINED );
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_AX = m_AX;
    result.m_AY = m_AY;
    result.m_AZ = m_AZ;
    #elif defined( _VMX128_ )
    result.m_CX = __vrlimi( m_CX, __vzero(), VMX_MASK_INSERT_W, VMX_ROTATE_NONE );
    result.m_CY = __vrlimi( m_CY, __vzero(), VMX_MASK_INSERT_W, VMX_ROTATE_NONE );
    result.m_CZ = __vrlimi( m_CZ, __vzero(), VMX_MASK_INSERT_W, VMX_ROTATE_NONE );
    #else
    result = m_Rotate;
    #endif
    return result;
}

/// Accessor: Get Translation
NOALIAS __forceinline const HVec3 HMat43::Translation() const __restrict
{
    HVec3 result( UNDEFINED );
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = m_AW;
    #elif defined( _VMX128_ )
    result.m_V = __vmrglw( __vmrglw( m_CX, m_CZ ), __vmrglw( m_CY, __vzero() ) );
    #else
    result = m_Translate;
    #endif
    return result;
}

/// Accessor: Set Rotation
NOALIAS __forceinline void HMat43::Rotation( const HMat33& rotation ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX = rotation.m_AX;
    m_AY = rotation.m_AY;
    m_AZ = rotation.m_AZ;
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline void HMat43::Translation( const HVec3 PCREF translation ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AW = translation.m_V;
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline const HVec3 HMat43::AX() const __restrict
{
    HVec3 result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = m_AX;
    #elif defined( _VMX128_ )
    result.m_V = __vmrghw( __vmrghw( m_CX, m_CZ ), __vmrghw( m_CY, __vzero() ) );
    #elif defined( _SSE_ )
    result.m_V = m_Rotate.m_AX;
    #else
    result = m_Rotate.AX();
    #endif
    return result;
}

/// Accessor: Get Axis Y
NOALIAS __forceinline const HVec3 HMat43::AY() const __restrict
{
    HVec3 result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = m_AY;
    #elif defined( _VMX128_ )
    result.m_V = __vmrglw( __vmrghw( m_CX, m_CZ ), __vmrghw( m_CY, __vzero() ) );
    #elif defined( _SSE_ )
    result.m_V = m_Rotate.m_AY;
    #else
    result = m_Rotate.AY();
    #endif
    return result;
}

/// Accessor: Get Axis Z
NOALIAS __forceinline const HVec3 HMat43::AZ() const __restrict
{
    HVec3 result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = m_AZ;
    #elif defined( _VMX128_ )
    result.m_V = __vmrghw( __vmrglw( m_CX, m_CZ ), __vmrglw( m_CY, __vzero() ) );
    #elif defined( _SSE_ )
    result.m_V = m_Rotate.m_AZ;
    #else
    result = m_Rotate.AZ();
    #endif
    return result;
}

/// Accessor: Get Axis W
NOALIAS __forceinline const HVec3 HMat43::AW() const __restrict
{
    HVec3 result;
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = m_AW;
    #elif defined( _VMX128_ )
    result.m_V = __vmrglw( __vmrglw( m_CX, m_CZ ), __vmrglw( m_CY, __vzero() ) );
    #elif defined( _SSE_ )
    result.m_V = m_Translate.m_V;
    #else
    result = m_Translate;
    #endif
    return result;
}

/// Accessor: Set Axis X
NOALIAS __forceinline void HMat43::AX( const HVec3 PCREF ax ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX = ax.m_V;
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline void HMat43::AY( const HVec3 PCREF ay ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AY = ay.m_V;
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline void HMat43::AZ( const HVec3 PCREF az ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AZ = az.m_V;
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline void HMat43::AW( const HVec3 PCREF aw ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AW = aw.m_V;
    #elif defined( _VMX128_ )
    m_CX = __vrlimi( m_CX, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    m_CY = __vrlimi( m_CY, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    m_CZ = __vrlimi( m_CZ, aw.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #elif defined( _SSE_ )
    m_Translate.m_V = aw.m_V;
    #else
    m_Translate = aw;
    #endif
}

/// Accessor: Get All Axes
NOALIAS __forceinline void HMat43::GetAxes( HVec3& dstAX, HVec3& dstAY, HVec3& dstAZ, HVec3& dstAW ) const __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    dstAX.m_V = m_AX;
    dstAY.m_V = m_AY;
    dstAZ.m_V = m_AZ;
    dstAW.m_V = m_AW;
    #elif defined( _VMX128_ )
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, /*c_UnitVectorW*/ __vzero() ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, /*c_UnitVectorW*/ __vzero() ) );
	dstAX.m_V = __vmrghw( tmp0, tmp1 );
    dstAY.m_V = __vmrglw( tmp0, tmp1 );
    dstAZ.m_V = __vmrghw( tmp2, tmp3 );
    dstAW.m_V = __vmrglw( tmp2, tmp3 );
	#else
	dstAX = this->AX();
	dstAY = this->AY();
	dstAZ = this->AZ();
	dstAW = this->AW();
    #endif
}

/// Accessor: Get All Axes
NOALIAS __forceinline void HMat43::GetAxes( HVec4& dstAX, HVec4& dstAY, HVec4& dstAZ, HVec4& dstAW ) const __restrict
{
    #if defined( _SPU_ )
    const vector float zero = Const< vector float >::Zero();
    const vector float one = spu_splats( 1.0f );
    dstAX.m_V = spu_permute<V0X,V0Y,V0Z,V1W>( m_AX, zero );
    dstAY.m_V = spu_permute<V0X,V0Y,V0Z,V1W>( m_AY, zero );
    dstAZ.m_V = spu_permute<V0X,V0Y,V0Z,V1W>( m_AZ, zero );
    dstAW.m_V = spu_permute<V0X,V0Y,V0Z,V1W>( m_AW, one  );
    #elif defined( _PPU_ )
    const vector float zero = Const< vector float >::Zero();
    //const vector float one = (vector float)( 1.0f, 1.0f, 1.0f, 1.0f );
    const vector float one = Const< vector float >::One();
    dstAX.m_V = vec_permute<V0X,V0Y,V0Z,V1W>( m_AX, zero );
    dstAY.m_V = vec_permute<V0X,V0Y,V0Z,V1W>( m_AY, zero );
    dstAZ.m_V = vec_permute<V0X,V0Y,V0Z,V1W>( m_AZ, zero );
    dstAW.m_V = vec_permute<V0X,V0Y,V0Z,V1W>( m_AW, one  );
    #elif defined( _VMX128_ )
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, c_UnitVectorW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, c_UnitVectorW ) );
    dstAX.m_V = __vmrghw( tmp0, tmp1 );
    dstAY.m_V = __vmrglw( tmp0, tmp1 );
    dstAZ.m_V = __vmrghw( tmp2, tmp3 );
    dstAW.m_V = __vmrglw( tmp2, tmp3 );
    #else
    dstAX = this->AX();
    dstAY = this->AY();
    dstAZ = this->AZ();
    dstAW = this->AW();
    #endif
}

/// Accessor: Get All Axes
NOALIAS __forceinline void HMat33::GetAxes( HVec4& dstAX, HVec4& dstAY, HVec4& dstAZ ) const __restrict
{
    #if defined( _SPU_ )
    const vector float zero = Const< vector float >::Zero();
    dstAX.m_V = spu_permute<V0X,V0Y,V0Z,V1W>( m_AX, zero );
    dstAY.m_V = spu_permute<V0X,V0Y,V0Z,V1W>( m_AY, zero );
    dstAZ.m_V = spu_permute<V0X,V0Y,V0Z,V1W>( m_AZ, zero );
    #elif defined( _PPU_ )
    const vector float zero = Const< vector float >::Zero();
    dstAX.m_V = vec_permute<V0X,V0Y,V0Z,V1W>( m_AX, zero );
    dstAY.m_V = vec_permute<V0X,V0Y,V0Z,V1W>( m_AY, zero );
    dstAZ.m_V = vec_permute<V0X,V0Y,V0Z,V1W>( m_AZ, zero );
    #elif defined( _VMX128_ )
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, c_UnitVectorW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, c_UnitVectorW ) );
    dstAX.m_V = __vmrghw( tmp0, tmp1 );
    dstAY.m_V = __vmrglw( tmp0, tmp1 );
    dstAZ.m_V = __vmrghw( tmp2, tmp3 );
    #else
    dstAX = this->AX();
    dstAY = this->AY();
    dstAZ = this->AZ();
    #endif
}

NOALIAS __forceinline void HMat33::GetAxes( HVec3& dstAX, HVec3& dstAY, HVec3& dstAZ ) const __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    dstAX.m_V = m_AX;
    dstAY.m_V = m_AY;
    dstAZ.m_V = m_AZ;
    #elif defined( _VMX128_ )
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, c_UnitVectorW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, c_UnitVectorW ) );
    dstAX.m_V = __vmrghw( tmp0, tmp1 );
    dstAY.m_V = __vmrglw( tmp0, tmp1 );
    dstAZ.m_V = __vmrghw( tmp2, tmp3 );
    #else
    dstAX = this->AX();
    dstAY = this->AY();
    dstAZ = this->AZ();
    #endif
}

/// Set All Axes (WALTER todo .. version from HVec3 makes more sense too)
NOALIAS __forceinline void HMat43::SetAxes( const HVec4 PCREF srcAX, const HVec4 PCREF srcAY, const HVec4 PCREF srcAZ, const HVec4 PCREF srcAW ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX = srcAX.m_V;
    m_AY = srcAY.m_V;
    m_AZ = srcAZ.m_V;
    m_AW = srcAW.m_V;
    #elif defined( _VMX128_ )
    const V128 tmp0( __vmrghw( srcAX, srcAZ ) );
    const V128 tmp1( __vmrghw( srcAY, srcAW ) );
    const V128 tmp2( __vmrglw( srcAX, srcAZ ) );
    const V128 tmp3( __vmrglw( srcAY, srcAW ) );
    m_CX = __vmrghw( tmp0, tmp1 );
    m_CY = __vmrglw( tmp0, tmp1 );
    m_CZ = __vmrghw( tmp2, tmp3 );
    #else
    // WALTER TODO, this just wants to be permutes on the ps3 aswell.
    // we need to clarify if the matrix is staying the same or not.
    this->AX( HVec3( srcAX ) );
    this->AY( HVec3( srcAY ) );
    this->AZ( HVec3( srcAZ ) );
    this->AW( HVec3( srcAW ) );
    #endif
}

/// Set All Axes
NOALIAS __forceinline void HMat43::SetAxesScaled( const HVec4 PCREF srcAX, const HVec4 PCREF srcAY, const HVec4 PCREF srcAZ, const HVec4 PCREF srcAW, const HVec4 PCREF scaleXYZ ) __restrict
{
    #if defined( _SPU_ )
    m_AX = spu_mul( srcAX.m_V, spu_splat<VX>( scaleXYZ.m_V ) );
    m_AY = spu_mul( srcAY.m_V, spu_splat<VY>( scaleXYZ.m_V ) );
    m_AZ = spu_mul( srcAZ.m_V, spu_splat<VZ>( scaleXYZ.m_V ) );
    m_AW = srcAW.m_V;
    #elif defined( _PPU_ )
    m_AX = vec_mul( srcAX.m_V, vec_splat( scaleXYZ.m_V, VX ) );
    m_AY = vec_mul( srcAY.m_V, vec_splat( scaleXYZ.m_V, VY ) );
    m_AZ = vec_mul( srcAZ.m_V, vec_splat( scaleXYZ.m_V, VZ ) );
    m_AW = srcAW.m_V;
    #elif defined( _VMX128_ )
    const V128 tmp0( __vmrghw( srcAX, srcAZ ) );
    const V128 tmp1( __vmrghw( srcAY, srcAW ) );
    const V128 tmp2( __vmrglw( srcAX, srcAZ ) );
    const V128 tmp3( __vmrglw( srcAY, srcAW ) );
    V128 scaleXYZ1 = __vrlimi( scaleXYZ, c_UnitVectorW, VMX_MASK_INSERT_W, 0 );
    m_CX = __vmulfp( __vmrghw( tmp0, tmp1 ), scaleXYZ1 );
    m_CY = __vmulfp( __vmrglw( tmp0, tmp1 ), scaleXYZ1 );
    m_CZ = __vmulfp( __vmrghw( tmp2, tmp3 ), scaleXYZ1 );
    #else
    // WALTER TODO - Verify this is correct and that the scale should not be transposed. 
    this->AX( HVec3( srcAX * scaleXYZ.X() ) );
    this->AY( HVec3( srcAY * scaleXYZ.Y() ) );
    this->AZ( HVec3( srcAZ * scaleXYZ.Z() ) );
    this->AW( HVec3( srcAW ) );
    #endif
}

/// Set All Axes
NOALIAS __forceinline void HMat43::SetScaleRot( const HVec3 PCREF scaleXYZ, const HMat33& rot ) __restrict
{
	#if defined( _SPU_ )
    m_AX = spu_mul( rot.m_AX, spu_splat<VX>( scaleXYZ.m_V) );
    m_AY = spu_mul( rot.m_AY, spu_splat<VY>( scaleXYZ.m_V) );
    m_AZ = spu_mul( rot.m_AZ, spu_splat<VZ>( scaleXYZ.m_V) );
    m_AW = c_UnitVectorW;
	#elif defined(_PPU_)
    m_AX = vec_mul( rot.m_AX, vec_splat( scaleXYZ.m_V, VX ) );
    m_AY = vec_mul( rot.m_AY, vec_splat( scaleXYZ.m_V, VY ) );
    m_AZ = vec_mul( rot.m_AZ, vec_splat( scaleXYZ.m_V, VZ ) );
    m_AW = c_UnitVectorW;
    #elif defined( _VMX128_ )
	// requires assert that CW is zero.
	m_CX = rot.m_CX * scaleXYZ;
	m_CY = rot.m_CY * scaleXYZ;
	m_CZ = rot.m_CZ * scaleXYZ;
    #else
    // WALTER TODO - Verify this is correct and that the scale should not be transposed. 
    this->AX( HVec3( rot.AX() * scaleXYZ.X() ) );
    this->AY( HVec3( rot.AY() * scaleXYZ.Y() ) );
    this->AZ( HVec3( rot.AZ() * scaleXYZ.Z() ) );
    this->AW( HVec3() );
    #endif
}


/// Set All Axes
NOALIAS __forceinline void HMat43::SetScaleRotTrans( const HVec3 PCREF scale, const HMat33& rot, const HVec3 PCREF trans ) __restrict
{
	#if defined( _SPU_ )
    m_AX = spu_mul( rot.m_AX, spu_splat<VX>( scale.m_V) );
    m_AY = spu_mul( rot.m_AY, spu_splat<VY>( scale.m_V) );
    m_AZ = spu_mul( rot.m_AZ, spu_splat<VZ>( scale.m_V) );
    m_AW = trans.m_V;
	#elif defined(_PPU_)
    m_AX = vec_mul( rot.m_AX, vec_splat( scale.m_V, VX ) );
    m_AY = vec_mul( rot.m_AY, vec_splat( scale.m_V, VY ) );
    m_AZ = vec_mul( rot.m_AZ, vec_splat( scale.m_V, VZ ) );
    m_AW = trans.m_V;
    #elif defined( _VMX128_ )
	
	m_CX = __vrlimi(rot.m_CX * scale, trans, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W);
	m_CY = __vrlimi(rot.m_CY * scale, trans, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W);
	m_CZ = __vrlimi(rot.m_CZ * scale, trans, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W);
    #else
    // WALTER TODO - Verify this is correct and that the scale should not be transposed. 
    this->AX( HVec3( rot.AX() * scale.X() ) );
    this->AY( HVec3( rot.AY() * scale.Y() ) );
    this->AZ( HVec3( rot.AZ() * scale.Z() ) );
    this->AW( trans );
    #endif
}


/// Accessor: Get All Axes
NOALIAS __forceinline void HMat44::GetAxes( HVec4& dstAX, HVec4& dstAY, HVec4& dstAZ, HVec4& dstAW ) const __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    dstAX.m_V = m_AX;
    dstAY.m_V = m_AY;
    dstAZ.m_V = m_AZ;
    dstAW.m_V = m_AW;
    #elif defined( _VMX128_ )
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, m_CW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, m_CW ) );
	dstAX.m_V = __vmrghw( tmp0, tmp1 );
    dstAY.m_V = __vmrglw( tmp0, tmp1 );
    dstAZ.m_V = __vmrghw( tmp2, tmp3 );
    dstAW.m_V = __vmrglw( tmp2, tmp3 );
	#else
	dstAX = this->AX();
	dstAY = this->AY();
	dstAZ = this->AZ();
	dstAW = this->AW();
    #endif
}

#if !defined( _SIMD_ )

/// Accessor: Const Axis Indexed
NOALIAS __forceinline const HVec3 HMat43::operator [] ( const U32 i ) const __restrict
{
    assert( i < 4 );
    return ((HVec3*)this)[i];
}

/// Accessor: Axis Indexed
NOALIAS __forceinline HVec3& HMat43::operator [] ( const U32 i ) __restrict
{
    assert( i < 4 );
    return ((HVec3*)this)[i];
}

#endif

/// HMat33 Assignment
NOALIAS __forceinline const HMat43 HMat43::operator = ( const HMat33& src ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX = src.m_AX;
    m_AY = src.m_AY;
    m_AZ = src.m_AZ;
    m_AW = Const< vector float >::Zero();
    #elif defined( _VMX128_ )
    //m_CX = __vrlimi( m_CX, src.m_CX, VMX_MASK_INSERT_XYZ, VMX_ROTATE_NONE );
    //m_CY = __vrlimi( m_CY, src.m_CY, VMX_MASK_INSERT_XYZ, VMX_ROTATE_NONE );
    //m_CZ = __vrlimi( m_CZ, src.m_CZ, VMX_MASK_INSERT_XYZ, VMX_ROTATE_NONE );
    const __vector4 zero( Const< __vector4 >::Zero() );
    //m_CX = __vrlimi( zero, src.m_CX, VMX_MASK_INSERT_XYZ, VMX_ROTATE_NONE );
    //m_CY = __vrlimi( zero, src.m_CY, VMX_MASK_INSERT_XYZ, VMX_ROTATE_NONE );
    //m_CZ = __vrlimi( zero, src.m_CZ, VMX_MASK_INSERT_XYZ, VMX_ROTATE_NONE );
    m_CX = __vrlimi( src.m_CX, zero, VMX_MASK_INSERT_W, VMX_ROTATE_NONE );
    m_CY = __vrlimi( src.m_CY, zero, VMX_MASK_INSERT_W, VMX_ROTATE_NONE );
    m_CZ = __vrlimi( src.m_CZ, zero, VMX_MASK_INSERT_W, VMX_ROTATE_NONE );
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
NOALIAS __forceinline const HMat43 HMat43::operator = ( const HMat43& copy )
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX = copy.m_AX;
    m_AY = copy.m_AY;
    m_AZ = copy.m_AZ;
    m_AW = copy.m_AW;
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline void HMat43::SetIdentity() __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX = Const< vector float >::UnitX();
    m_AY = Const< vector float >::UnitY();
    m_AZ = Const< vector float >::UnitZ();
    m_AW = Const< vector float >::Zero();
    #elif defined( _VMX128_ )
    m_CX = c_UnitVectorX;
    m_CY = c_UnitVectorY;
    m_CZ = c_UnitVectorZ;
    #else
    m_Rotate.SetIdentity();
    m_Translate = HVec3( c_VectorZero );
    #endif
}

/// Matrix Multiply
NOALIAS __forceinline const HMat43 HMat43::operator * ( const HMat43& m2 ) const __restrict
{
    HMat43 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    const vector float m2_ax( m2.m_AX );
    const vector float m2_ay( m2.m_AY );
    const vector float m2_az( m2.m_AZ );
    const vector float m2_aw( m2.m_AW );
    
    result.m_AX = spu_madd( spu_splat<VX>( m_AX ), m2_ax,
                  spu_madd( spu_splat<VY>( m_AX ), m2_ay,
                  spu_mul ( spu_splat<VZ>( m_AX ), m2_az ) ) );
                                      
    result.m_AY = spu_madd( spu_splat<VX>( m_AY ), m2_ax,
                  spu_madd( spu_splat<VY>( m_AY ), m2_ay,
                  spu_mul ( spu_splat<VZ>( m_AY ), m2_az ) ) );
                                      
    result.m_AZ = spu_madd( spu_splat<VX>( m_AZ ), m2_ax,
                  spu_madd( spu_splat<VY>( m_AZ ), m2_ay,
                  spu_mul ( spu_splat<VZ>( m_AZ ), m2_az ) ) );
                                      
    result.m_AW = spu_madd( spu_splat<VX>( m_AW ), m2_ax,
                  spu_madd( spu_splat<VY>( m_AW ), m2_ay,
                  spu_madd( spu_splat<VZ>( m_AW ), m2_az,
                                                   m2_aw ) ) );
    
    #elif defined( _PPU_ )
    
    const vector float m2_ax( m2.m_AX );
    const vector float m2_ay( m2.m_AY );
    const vector float m2_az( m2.m_AZ );
    const vector float m2_aw( m2.m_AW );
    
    result.m_AX = vec_madd( vec_splat( m_AX, VX ), m2_ax,
                  vec_madd( vec_splat( m_AX, VY ), m2_ay,
                  vec_mul ( vec_splat( m_AX, VZ ), m2_az ) ) );
    
    result.m_AY = vec_madd( vec_splat( m_AY, VX ), m2_ax,
                  vec_madd( vec_splat( m_AY, VY ), m2_ay,
                  vec_mul ( vec_splat( m_AY, VZ ), m2_az ) ) );
    
    result.m_AZ = vec_madd( vec_splat( m_AZ, VX ), m2_ax,
                  vec_madd( vec_splat( m_AZ, VY ), m2_ay,
                  vec_mul ( vec_splat( m_AZ, VZ ), m2_az ) ) );
    
    result.m_AW = vec_madd( vec_splat( m_AW, VX ), m2_ax,
                  vec_madd( vec_splat( m_AW, VY ), m2_ay,
                  vec_madd( vec_splat( m_AW, VZ ), m2_az,
                                                   m2_aw ) ) );
    
    #elif defined( _VMX128_ )
    
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
    
    result.m_CX = __vmrghw( __vmrghw( result_cx_x, result_cx_z ), __vmrghw( result_cx_y, result_cx_w ) );
    result.m_CY = __vmrghw( __vmrghw( result_cy_x, result_cy_z ), __vmrghw( result_cy_y, result_cy_w ) );
    result.m_CZ = __vmrghw( __vmrghw( result_cz_x, result_cz_z ), __vmrghw( result_cz_y, result_cz_w ) );
    
    #else
    
    result = HMat43( m_Rotate * m2.m_Rotate, m_Translate * m2 );
    
    #endif
    
    return result;
}

/// Inverse of Rotation & Translation - Faster but only valid for ortho-normal matrices!
NOALIAS __forceinline const HMat43 HMat43::InverseRotTrans() const __restrict
{
    HMat43 result( UNDEFINED );
    
    #if 0 && defined( _SPU_ )
	// [Jan] this code doesn't work    
    const vector float zero( Const< vector float >::Zero() );
    //const vector float sign_mask( (vector float)spu_splats( 0x80000000 ) );
    const vector float sign_mask( Const< vector float >::SignMask() );
    
    //const vector float inv_rot_ax( (vector float)( spu_extract( m_AX, VX ), spu_extract( m_AY, VX ), spu_extract( m_AZ, VX ), 0.0f ) );
    //const vector float inv_rot_ay( (vector float)( spu_extract( m_AX, VY ), spu_extract( m_AY, VY ), spu_extract( m_AZ, VY ), 0.0f ) );
    //const vector float inv_rot_az( (vector float)( spu_extract( m_AX, VZ ), spu_extract( m_AY, VZ ), spu_extract( m_AZ, VZ ), 0.0f ) );
    const vector float inv_rot_ax( spu_insert( spu_extract( m_AX, VX ), spu_insert( spu_extract( m_AY, VX ), spu_insert( spu_extract( m_AZ, VX ), spu_promote( 0.0f, VW ), VZ ), VY ), VX ) );
    const vector float inv_rot_ay( spu_insert( spu_extract( m_AX, VY ), spu_insert( spu_extract( m_AY, VY ), spu_insert( spu_extract( m_AZ, VY ), spu_promote( 0.0f, VW ), VZ ), VY ), VX ) );
    const vector float inv_rot_az( spu_insert( spu_extract( m_AX, VZ ), spu_insert( spu_extract( m_AY, VZ ), spu_insert( spu_extract( m_AZ, VZ ), spu_promote( 0.0f, VW ), VZ ), VY ), VX ) );
    
    const vector float neg_trans( spu_xor( m_AW, sign_mask ) );
    
    result.m_AX = inv_rot_ax;
    result.m_AY = inv_rot_ay;
    result.m_AZ = inv_rot_az;
    result.m_AW = spu_madd( spu_splat<VX>( neg_trans ), inv_rot_ax,
                  spu_madd( spu_splat<VY>( neg_trans ), inv_rot_ay,
                  spu_mul ( spu_splat<VZ>( neg_trans ), inv_rot_az ) ) );
    
    #elif 0 && defined( _PPU_ )
    
    const vector float zero( Const< vector float >::Zero() );
    //const vector float sign_mask( (vector float)0x80000000 );
    const vector float sign_mask( Const< vector float >::SignMask() );
    
    const vector float tmp0( vec_mergeh( m_AX, m_AZ ) );
    const vector float tmp1( vec_mergeh( m_AY, zero ) );
    const vector float tmp2( vec_mergel( m_AX, m_AZ ) );
    const vector float tmp3( vec_mergel( m_AY, zero ) );
    const vector float inv_rot_ax( vec_mergeh( tmp0, tmp1 ) );
    const vector float inv_rot_ay( vec_mergel( tmp0, tmp1 ) );
    const vector float inv_rot_az( vec_mergeh( tmp2, tmp3 ) );
    
    const vector float neg_trans( vec_xor( m_AW, sign_mask ) );
    
    result.m_AX = inv_rot_ax;
    result.m_AY = inv_rot_ay;
    result.m_AZ = inv_rot_az;
    result.m_AW = vec_madd( vec_splat( neg_trans, VX ), inv_rot_ax,
                  vec_madd( vec_splat( neg_trans, VY ), inv_rot_ay,
                  vec_mul ( vec_splat( neg_trans, VZ ), inv_rot_az ) ) );
    
    #elif defined( _VMX128_ )
    
    const V128 zero( __vzero() );
    const V128 vec_sign_mask( c_VectorSignMask );
    
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, zero ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, zero ) );
    
    const V128 ax( __vmrghw( tmp0, tmp1 ) );
    const V128 ay( __vmrglw( tmp0, tmp1 ) );
    const V128 az( __vmrghw( tmp2, tmp3 ) );
    const V128 aw( __vmrglw( tmp2, tmp3 ) );
    
    const V128 neg_trans( __vxor( aw, vec_sign_mask ) );
    
    const V128 inv_trans_x( __vmsum3fp( neg_trans, ax ) );
    const V128 inv_trans_y( __vmsum3fp( neg_trans, ay ) );
    const V128 inv_trans_z( __vmsum3fp( neg_trans, az ) );
    
    const V128 inv_trans( __vmrghw( __vmrghw( inv_trans_x, inv_trans_z ), __vmrghw( inv_trans_y, zero ) ) );
    
    result.m_CX = __vrlimi( ax, inv_trans, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    result.m_CY = __vrlimi( ay, inv_trans, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    result.m_CZ = __vrlimi( az, inv_trans, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    
    #else
    
    const HMat33 inverseRotation( Rotation().Transposed() );
    
    result = inverseRotation || ( -Translation() * inverseRotation );
    
    #endif
    
    return result;
}

/// Get Scale-Rotate-Translate
NOALIAS __forceinline const C_MatrixSRT HMat43::GetSRT() const __restrict
{
    const HMat33 scaleRotateMatrix( *this );
    
    const C_MatrixRotateRPH rotate( scaleRotateMatrix.NormalisedZY().GetRotationRPH() );
    
    const HMat33 rotateMatrix( rotate );
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    
    const HMat33 scaleMatrix( scaleRotateMatrix * rotateMatrix.Transposed() );
    
    const C_MatrixScale scale( Length( scaleMatrix.AX() ),
                               Length( scaleMatrix.AY() ),
                               Length( scaleMatrix.AZ() ) );
    
    const C_MatrixTranslate translate( Translation() );
    
    #elif defined( _VMX128_ )
    
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

template< typename T_Stream > __forceinline void HMat43::Deserialise( T_Stream& stream ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    HVec3 ax, ay, az, aw;
    stream >> ax;
    stream >> ay;
    stream >> az;
    stream >> aw;
    AX( ax );
    AY( ay );
    AZ( az );
    AW( aw );
    #elif defined( _VMX128_ )
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

template< typename T_Stream > __forceinline void HMat43::Serialise( T_Stream& stream ) __restrict
{
    //m_Rotate.Serialise( stream );
    //m_Translate.Serialise( stream );
    //Rotation().Serialise( stream );
    //Translation().Serialise( stream );

	Maths::HMat33 rot(Rotation());
	Maths::HVec3 trans(Translation());
    rot.Serialise( stream );
    trans.Serialise( stream );
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
NOALIAS __forceinline HMat44::HMat44()
    #if defined( _SPU_ ) || defined( _PPU_ )
    : m_AX( Const< vector float >::UnitX() ),
      m_AY( Const< vector float >::UnitY() ),
      m_AZ( Const< vector float >::UnitZ() ),
      m_AW( Const< vector float >::UnitW() )
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline HMat44::HMat44( const _UNDEFINED /*undefined*/ )
{
}

/// Constructor: initialises to the zero matrix.
NOALIAS __forceinline HMat44::HMat44( const _ZERO_MATRIX )
    #if defined( _SPU_ ) || defined( _PPU_ ) // || defined( _SSE_ )
    : m_AX( Const< vector float >::Zero() ),
      m_AY( Const< vector float >::Zero() ),
      m_AZ( Const< vector float >::Zero() ),
      m_AW( Const< vector float >::Zero() )
    #elif defined( _VMX128_ )
    : m_CX( Const< __vector4 >::Zero() ),
      m_CY( Const< __vector4 >::Zero() ),
      m_CZ( Const< __vector4 >::Zero() ),
      m_CW( Const< __vector4 >::Zero() )
    #elif defined( _SSE_ )
    : m_AX( Const< __m128 >::Zero() ),
      m_AY( Const< __m128 >::Zero() ),
      m_AZ( Const< __m128 >::Zero() ),
      m_AW( Const< __m128 >::Zero() )
    #endif
{
    #if !defined( _SIMD_ )
    m_A[MX] = HVec4( ZERO_INIT );
    m_A[MY] = HVec4( ZERO_INIT );
    m_A[MZ] = HVec4( ZERO_INIT );
    m_A[MW] = HVec4( ZERO_INIT );
    #endif
}

/// Explicit Constructor from HMat33
NOALIAS __forceinline HMat44::HMat44( const HMat33& src )
    #if defined( _SPU_ )
    : m_AX( spu_and( src.m_AX, (vector float)Const< vector unsigned int >::Mask(1,1,1,0) ) ),
      m_AY( spu_and( src.m_AY, (vector float)Const< vector unsigned int >::Mask(1,1,1,0) ) ),
      m_AZ( spu_and( src.m_AZ, (vector float)Const< vector unsigned int >::Mask(1,1,1,0) ) ),
      m_AW( Const< vector float >::UnitW() )
    #elif defined( _PPU_ )
    : m_AX( vec_and( src.m_AX, (vector float)Const< vector unsigned int >::Mask(1,1,1,0) ) ),
      m_AY( vec_and( src.m_AY, (vector float)Const< vector unsigned int >::Mask(1,1,1,0) ) ),
      m_AZ( vec_and( src.m_AZ, (vector float)Const< vector unsigned int >::Mask(1,1,1,0) ) ),
      m_AW( Const< vector float >::UnitW() )
    #elif defined( _VMX128_ )
    : m_CX( __vrlimi( src.m_CX, __vzero(), VMX_MASK_INSERT_W, 0 ) ),
      m_CY( __vrlimi( src.m_CY, __vzero(), VMX_MASK_INSERT_W, 0 ) ),
      m_CZ( __vrlimi( src.m_CZ, __vzero(), VMX_MASK_INSERT_W, 0 ) ),
      m_CW( c_UnitVectorW )
    #elif defined( _SSE_ )
    : m_AX( SIMD_Mask<1,1,1,0>( src.m_AX.m_V ) ),
      m_AY( SIMD_Mask<1,1,1,0>( src.m_AY.m_V ) ),
      m_AZ( SIMD_Mask<1,1,1,0>( src.m_AZ.m_V ) ),
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
NOALIAS __forceinline HMat44::HMat44( const HMat43& src )
    #if defined( _SPU_ )
    : m_AX( spu_and( src.m_AX, (vector float)Const< vector unsigned int >::Mask(1,1,1,0) ) ),
      m_AY( spu_and( src.m_AY, (vector float)Const< vector unsigned int >::Mask(1,1,1,0) ) ),
      m_AZ( spu_and( src.m_AZ, (vector float)Const< vector unsigned int >::Mask(1,1,1,0) ) ),
    //: m_AX( spu_insert( 0.0f, src.m_AX, VW ) ),
    //  m_AY( spu_insert( 0.0f, src.m_AY, VW ) ),
    //  m_AZ( spu_insert( 0.0f, src.m_AZ, VW ) ),
      m_AW( spu_insert( 1.0f, src.m_AW, VW ) )
    #elif defined( _PPU_ )
    : m_AX( vec_and( src.m_AX, (vector float)Const< vector unsigned int >::Mask(1,1,1,0) ) ),
      m_AY( vec_and( src.m_AY, (vector float)Const< vector unsigned int >::Mask(1,1,1,0) ) ),
      m_AZ( vec_and( src.m_AZ, (vector float)Const< vector unsigned int >::Mask(1,1,1,0) ) ),
      m_AW( vec_permute<V0X,V0Y,V0Z,V1W>( src.m_AW, Const< vector float >::One() ) )
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline HMat44::HMat44( const HMat44& copy )
    #if defined( _SPU_ ) || defined( _PPU_ )
    : m_AX( copy.m_AX ),
      m_AY( copy.m_AY ),
      m_AZ( copy.m_AZ ),
      m_AW( copy.m_AW )
    #elif defined( _VMX128_ )
    : m_CX( copy.m_CX ),
      m_CY( copy.m_CY ),
      m_CZ( copy.m_CZ ),
      m_CW( copy.m_CW )
    #elif defined( _SSE_ )
    : m_AX( copy.m_AX.m_V ),
      m_AY( copy.m_AY.m_V ),
      m_AZ( copy.m_AZ.m_V ),
      m_AW( copy.m_AW.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_A[MX] = copy.m_A[MX];
    m_A[MY] = copy.m_A[MY];
    m_A[MZ] = copy.m_A[MZ];
    m_A[MW] = copy.m_A[MW];
    #endif
}

#if defined( _SPU_ ) || defined( _PPU_ )
/// vector float Axis XYZW Constructor
NOALIAS __forceinline HMat44::HMat44( const vector float ax, const vector float ay, const vector float az, const vector float aw ) :
    m_AX( ax ),
    m_AY( ay ),
    m_AZ( az ),
    m_AW( aw )
{
}
#elif defined( _VMX128_ )
/// V128 Axis XYZW Constructor
NOALIAS __forceinline HMat44::HMat44( const V128 cx, const V128 cy, const V128 cz, const V128 cw ) :
    m_CX( cx ),
    m_CY( cy ),
    m_CZ( cz ),
    m_CW( cw )
{
}
#elif defined( _SSE_ )
/// V128 Axis XYZW Constructor
NOALIAS __forceinline HMat44::HMat44( const V128 PCREF ax, const V128 PCREF ay, const V128 PCREF az, const V128 PCREF aw ) :
    m_AX( ax ),
    m_AY( ay ),
    m_AZ( az ),
    m_AW( aw )
{
}
#endif

/// HVec4 Axis XYZW Constructor
NOALIAS __forceinline HMat44::HMat44( const HVec4 PCREF ax, const HVec4 PCREF ay, const HVec4 PCREF az, const HVec4 PCREF aw )
    #if defined( _SPU_ ) || defined( _PPU_ )
    : m_AX( ax.m_V ),
      m_AY( ay.m_V ),
      m_AZ( az.m_V ),
      m_AW( aw.m_V )
    #elif defined( _SSE_ )
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
NOALIAS __forceinline HMat44::HMat44( const RotX PCREF rx, const HVec4 PCREF aw )
{
    const HF32 alpha = rx;
    const HF32 cx( Cos( alpha ) );
    const HF32 sx( Sin( alpha ) );
    #if defined( _SSE_ )
    const __m128 ay = _mm_setr_ps( 0.0f,  cx, sx, 0.0f );
    const __m128 az = _mm_setr_ps( 0.0f, -sx, cx, 0.0f );
    #else
    const Maths::V128 ay = { 0.0f,  cx, sx, 0.0f };
    const Maths::V128 az = { 0.0f, -sx, cx, 0.0f };
    #endif
    SetAxes( HVec4( c_UnitVectorX ), HVec4( ay ), HVec4( az ), aw );
}

/// Y Axis Rotation Constructor : clockwise when viewed back down the Y axis
NOALIAS __forceinline HMat44::HMat44( const RotY PCREF ry, const HVec4 PCREF aw )
{
    const HF32 beta = ry;
    const HF32 cy( Cos( beta ) );
    const HF32 sy( Sin( beta ) );
    #if defined( _SSE_ )
    const __m128 ax = _mm_setr_ps( cy, 0.0f, -sy, 0.0f );
    const __m128 az = _mm_setr_ps( sy, 0.0f,  cy, 0.0f );
    #else
    const Maths::V128 ax = { cy, 0.0f, -sy, 0.0f };
    const Maths::V128 az = { sy, 0.0f,  cy, 0.0f };
    #endif
    SetAxes( HVec4( ax ), HVec4( c_UnitVectorY ), HVec4( az ), aw );
}

/// Z Axis Rotation Constructor : clockwise when viewed back down the Z axis
NOALIAS __forceinline HMat44::HMat44( const RotZ PCREF rz, const HVec4 PCREF aw )
{
    const HF32 gamma = rz;
    const HF32 cz( Cos( gamma ) );
    const HF32 sz( Sin( gamma ) );
    #if defined( _SSE_ )
    const __m128 ax = _mm_setr_ps(  cz, sz, 0.0f, 0.0f );
    const __m128 ay = _mm_setr_ps( -sz, cz, 0.0f, 0.0f );
    #else
    const Maths::V128 ax = {  cz, sz, 0.0f, 0.0f };
    const Maths::V128 ay = { -sz, cz, 0.0f, 0.0f };
    #endif
    SetAxes( HVec4( ax ), HVec4( ay ), HVec4( c_UnitVectorZ ), aw );
}

/// Constructor from primary axis X, secondary axis Y
NOALIAS __forceinline HMat44::HMat44( const AxisX PCREF ax, const AxisY PCREF ay, const HVec4 PCREF aw )
{
    const C_Normal az( Normalise(ax ^ ay) );
    //SetAxes( (HVec4)(HVec3)ax, (HVec4)(HVec3)( az ^ ax ), (HVec4)(HVec3)az, aw );
    SetAxes( ax, az ^ ax, az, aw );
}

/// Constructor from primary axis X, secondary axis Z
NOALIAS __forceinline HMat44::HMat44( const AxisX PCREF ax, const AxisZ PCREF az, const HVec4 PCREF aw )
{
    const C_Normal ay( Normalise(az ^ ax) );
    SetAxes( (HVec4)(HVec3)ax, (HVec4)(HVec3)ay, (HVec4)(HVec3)( ax ^ ay ), aw );
}

/// Constructor from primary axis Y, secondary axis X
NOALIAS __forceinline HMat44::HMat44( const AxisY PCREF ay, const AxisX PCREF ax, const HVec4 PCREF aw )
{
    const C_Normal az( Normalise(ax ^ ay) );
    SetAxes( (HVec4)(HVec3)( ay ^ az ), (HVec4)(HVec3)ay, (HVec4)(HVec3)az, aw );
}

/// Constructor from primary axis Y, secondary axis Z
NOALIAS __forceinline HMat44::HMat44( const AxisY PCREF ay, const AxisZ PCREF az, const HVec4 PCREF aw )
{
    const C_Normal ax( Normalise(ay ^ az) );
    SetAxes( (HVec4)(HVec3)ax, (HVec4)(HVec3)ay, (HVec4)(HVec3)( ax ^ ay ), aw );
}

/// Constructor from primary axis Z, secondary axis X
NOALIAS __forceinline HMat44::HMat44( const AxisZ PCREF az, const AxisX PCREF ax, const HVec4 PCREF aw )
{
    const C_Normal ay( Normalise(az ^ ax) );
    SetAxes( (HVec4)(HVec3)( ay ^ az ), (HVec4)(HVec3)ay, (HVec4)(HVec3)az, aw );
}

/// Constructor from primary axis Z, secondary axis Y
NOALIAS __forceinline HMat44::HMat44( const AxisZ PCREF az, const AxisY PCREF ay, const HVec4 PCREF aw )
{
    const C_Normal ax( Normalise(ay ^ az) );
    SetAxes( (HVec4)(HVec3)ax, (HVec4)(HVec3)( az ^ ax ), (HVec4)(HVec3)az, aw );
}

/// Reflection Plane Constructor
NOALIAS __forceinline HMat44::HMat44( const C_Plane PCREF plane )
{
    // XMVECTOR               P;
    // XMVECTOR               S;
    // XMVECTOR               A, B, C, D;
    // XMMATRIX               M;
    // static CONST XMVECTOR  NegativeTwo = {-2.0f, -2.0f, -2.0f, 0.0f};
    
    // XMASSERT(!XMVector3Equal(ReflectionPlane, XMVectorZero()));
    // XMASSERT(!XMPlaneIsInfinite(ReflectionPlane));
    
    // P = XMPlaneNormalize(ReflectionPlane);
    // S = XMVectorMultiply(P, NegativeTwo);
    
    // A = XMVectorSplatX(P);
    // B = XMVectorSplatY(P);
    // C = XMVectorSplatZ(P);
    // D = XMVectorSplatW(P);
    
    // M.r[0] = XMVectorMultiplyAdd(A, S, g_XMIdentityR0);
    // M.r[1] = XMVectorMultiplyAdd(B, S, g_XMIdentityR1);
    // M.r[2] = XMVectorMultiplyAdd(C, S, g_XMIdentityR2);
    // M.r[3] = XMVectorMultiplyAdd(D, S, g_XMIdentityR3);
    
    // P = normalize(Plane);
    // -2 * P.a * P.a + 1  -2 * P.b * P.a      -2 * P.c * P.a        0
    // -2 * P.a * P.b      -2 * P.b * P.b + 1  -2 * P.c * P.b        0
    // -2 * P.a * P.c      -2 * P.b * P.c      -2 * P.c * P.c + 1    0
    // -2 * P.a * P.d      -2 * P.b * P.d      -2 * P.c * P.d        1
    
    const HMat44 I( Constants< HMat44 >::Identity() );
    const HVec4  p( plane.m_V );
    const HF32   minusTwo( -Constants<HF32>::Two() );
    const HF32   zero( Constants<HF32>::Zero() );
    const HVec4  k( minusTwo, minusTwo, minusTwo, zero );
    const HVec4  s( p * k );
    const HVec4  a( p.X() );
    const HVec4  b( p.Y() );
    const HVec4  c( p.Z() );
    const HVec4  d( p.W() );
    
    SetAxes( a * s + I.AX(),
             b * s + I.AY(),
             c * s + I.AZ(),
             d * s + I.AW() );
}


#if ( DIRECT3D_VERSION >= 0x0900 )

/// Cast to D3DMATRIX
NOALIAS __forceinline HMat44::operator const D3DMATRIX () const __restrict
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
    
    const HMat44 m = *this;
    
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
NOALIAS __forceinline const HVec4 HMat44::AX() const __restrict
{
    HVec4 result( UNDEFINED );
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = m_AX;
    #elif defined( _VMX128_ )
    result.m_V = __vmrghw( __vmrghw( m_CX, m_CZ ), __vmrghw( m_CY, m_CW ) );
    #elif defined( _SSE_ )
    result.m_V = m_AX;
    #else
    result = m_A[MX];
    #endif
    return result;
}

/// Accessor: Get Axis Y
NOALIAS __forceinline const HVec4 HMat44::AY() const __restrict
{
    HVec4 result( UNDEFINED );
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = m_AY;
    #elif defined( _VMX128_ )
    result.m_V = __vmrglw( __vmrghw( m_CX, m_CZ ), __vmrghw( m_CY, m_CW ) );
    #elif defined( _SSE_ )
    result.m_V = m_AY;
    #else
    result = m_A[MY];
    #endif
    return result;
}

/// Accessor: Get Axis Z
NOALIAS __forceinline const HVec4 HMat44::AZ() const __restrict
{
    HVec4 result( UNDEFINED );
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = m_AZ;
    #elif defined( _VMX128_ )
    result.m_V = __vmrghw( __vmrglw( m_CX, m_CZ ), __vmrglw( m_CY, m_CW ) );
    #elif defined( _SSE_ )
    result.m_V = m_AZ;
    #else
    result = m_A[MZ];
    #endif
    return result;
}

/// Accessor: Get Axis W
NOALIAS __forceinline const HVec4 HMat44::AW() const __restrict
{
    HVec4 result( UNDEFINED );
    #if defined( _SPU_ ) || defined( _PPU_ )
    result.m_V = m_AW;
    #elif defined( _VMX128_ )
    result.m_V = __vmrglw( __vmrglw( m_CX, m_CZ ), __vmrglw( m_CY, m_CW ) );
    #elif defined( _SSE_ )
    result.m_V = m_AW;
    #else
    result = m_A[MW];
    #endif
    return result;
}

/// Accessor: Set Axis X
NOALIAS __forceinline void HMat44::AX( const HVec4 PCREF ax ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX = ax.m_V;
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline void HMat44::AY( const HVec4 PCREF ay ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AY = ay.m_V;
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline void HMat44::AZ( const HVec4 PCREF az ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AZ = az.m_V;
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline void HMat44::AW( const HVec4 PCREF aw ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AW = aw.m_V;
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline const HVec4 HMat44::operator [] ( const U32 i ) const __restrict
{
    #if defined( _SIMD_ )
    return (&m_AX)[i];
    #else
    return m_A[i];
    #endif
}

/// Element Index Accessor
NOALIAS __forceinline HVec4& HMat44::operator [] ( const U32 i ) __restrict
{
    #if defined( _SIMD_ )
    return (&m_AX)[i];
    #else
    return m_A[i];
    #endif
}

#else

/// Accessor: Const HVec4 indexed
NOALIAS __forceinline const HVec4 HMat44::operator [] ( const U32 i ) const __restrict
{
	// really bad but just need it to work
	if (i == 0) return AX();
	else if (i == 1) return AY();
	else if (i == 2) return AZ();
	else if (i == 3) return AW(); 
	
	assert(0);
	return AX();
}

#endif

/// Accessor: Set Axes XYZW from HVec4's
NOALIAS __forceinline void HMat44::SetAxes( const HVec4 PCREF ax, const HVec4 PCREF ay, const HVec4 PCREF az, const HVec4 PCREF aw ) __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX = ax.m_V;
    m_AY = ay.m_V;
    m_AZ = az.m_V;
    m_AW = aw.m_V;
    #elif defined( _VMX128_ )
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

#if defined( _SPU_ ) || defined( _PPU_ )
/// Private Accessor: Set Axes XYZW from vector floats
NOALIAS __forceinline void HMat44::SetAxesXYZW( const vector float ax, const vector float ay, const vector float az, const vector float aw ) __restrict
{
    m_AX = ax;
    m_AY = ay;
    m_AZ = az;
    m_AW = aw;
}
#elif defined( _VMX128_ )
/// Private Accessor: Set Axes XYZW from V128's
NOALIAS __forceinline void HMat44::SetAxesXYZW( const V128 ax, const V128 ay, const V128 az, const V128 aw ) __restrict
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
NOALIAS __forceinline void HMat44::SetAxesXYZW( const V128 PCREF ax, const V128 PCREF ay, const V128 PCREF az, const V128 PCREF aw ) __restrict
{
    m_AX.m_V = ax;
    m_AY.m_V = ay;
    m_AZ.m_V = az;
    m_AW.m_V = aw;
}
#endif

/// HMat33 Assignment
NOALIAS __forceinline const HMat44 HMat44::operator = ( const HMat33& src ) __restrict
{
    #if defined( _SPU_ )
    m_AX = spu_insert( 0.0f, src.m_AX, VW );
    m_AY = spu_insert( 0.0f, src.m_AY, VW );
    m_AZ = spu_insert( 0.0f, src.m_AZ, VW );
    m_AW = Const< vector float >::UnitW();
    #elif defined( _PPU_ )
    const vector float maskXYZ = (vector float)Const< vector unsigned int >::Mask(1,1,1,0);
    m_AX = vec_and( src.m_AX, maskXYZ );
    m_AY = vec_and( src.m_AY, maskXYZ );
    m_AZ = vec_and( src.m_AZ, maskXYZ );
    m_AW = Const< vector float >::UnitW();
    #elif defined( _VMX128_ )
    m_CX = __vrlimi( src.m_CX, __vzero(), VMX_MASK_INSERT_W, 0 );
    m_CY = __vrlimi( src.m_CY, __vzero(), VMX_MASK_INSERT_W, 0 );
    m_CZ = __vrlimi( src.m_CZ, __vzero(), VMX_MASK_INSERT_W, 0 );
    m_CW = c_UnitVectorW;
    #elif defined( _SSE_ )
    m_AX.m_V = SIMD_Mask<1,1,1,0>( src.m_AX.m_V );
    m_AY.m_V = SIMD_Mask<1,1,1,0>( src.m_AY.m_V );
    m_AZ.m_V = SIMD_Mask<1,1,1,0>( src.m_AZ.m_V );
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
NOALIAS __forceinline const HMat44 HMat44::operator = ( const HMat43& src ) __restrict
{
    #if defined( _SPU_ )
    m_AX = spu_insert( 0.0f, src.m_AX, VW );
    m_AY = spu_insert( 0.0f, src.m_AY, VW );
    m_AZ = spu_insert( 0.0f, src.m_AZ, VW );
    m_AW = spu_insert( 1.0f, src.m_AW, VW );
    #elif defined( _PPU_ )
    const vector float maskXYZ = (vector float)Const< vector unsigned int >::Mask(1,1,1,0);
    m_AX = vec_and( src.m_AX, maskXYZ );
    m_AY = vec_and( src.m_AY, maskXYZ );
    m_AZ = vec_and( src.m_AZ, maskXYZ );
    m_AW = vec_permute<V0X,V0Y,V0Z,V1W>( src.m_AW, Const< vector float >::UnitW() );
    #elif defined( _VMX128_ )
    m_CX = src.m_CX;
    m_CY = src.m_CY;
    m_CZ = src.m_CZ;
    m_CW = c_UnitVectorW;
    #elif defined( _SSE_ )
    m_AX.m_V = SIMD_Mask<1,1,1,0>( src.m_Rotate.m_AX.m_V );
    m_AY.m_V = SIMD_Mask<1,1,1,0>( src.m_Rotate.m_AY.m_V );
    m_AZ.m_V = SIMD_Mask<1,1,1,0>( src.m_Rotate.m_AZ.m_V );
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
NOALIAS __forceinline const HMat44 HMat44::operator = ( const HMat44& copy )
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX = copy.m_AX;
    m_AY = copy.m_AY;
    m_AZ = copy.m_AZ;
    m_AW = copy.m_AW;
    #elif defined( _VMX128_ )
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
NOALIAS __forceinline void HMat44::SetIdentity() __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    m_AX = Const< vector float >::UnitX();
    m_AY = Const< vector float >::UnitY();
    m_AZ = Const< vector float >::UnitZ();
    m_AW = Const< vector float >::UnitW();
    #else
    SetAxes( HVec4( c_UnitVectorX ), HVec4( c_UnitVectorY ), HVec4( c_UnitVectorZ ), HVec4( c_UnitVectorW ) );
    #endif
}

/// Transpose of Matrix
NOALIAS __forceinline const HMat44 HMat44::Transposed() const __restrict
{
    HMat44 result( UNDEFINED );
    #if defined( _SPU_ )
    //result.m_AX = (vector float)( spu_extract( m_AX, VX ), spu_extract( m_AY, VX ), spu_extract( m_AZ, VX ), spu_extract( m_AW, VX ) );
    //result.m_AY = (vector float)( spu_extract( m_AX, VY ), spu_extract( m_AY, VY ), spu_extract( m_AZ, VY ), spu_extract( m_AW, VY ) );
    //result.m_AZ = (vector float)( spu_extract( m_AX, VZ ), spu_extract( m_AY, VZ ), spu_extract( m_AZ, VZ ), spu_extract( m_AW, VZ ) );
    //result.m_AW = (vector float)( spu_extract( m_AX, VW ), spu_extract( m_AY, VW ), spu_extract( m_AZ, VW ), spu_extract( m_AW, VW ) );
    result.m_AX = spu_insert( spu_extract( m_AX, VX ), spu_insert( spu_extract( m_AY, VX ), spu_insert( spu_extract( m_AZ, VX ), spu_promote( spu_extract( m_AW, VX ), VW ), VZ ), VY ), VX );
    result.m_AY = spu_insert( spu_extract( m_AX, VY ), spu_insert( spu_extract( m_AY, VY ), spu_insert( spu_extract( m_AZ, VY ), spu_promote( spu_extract( m_AW, VY ), VW ), VZ ), VY ), VX );
    result.m_AZ = spu_insert( spu_extract( m_AX, VZ ), spu_insert( spu_extract( m_AY, VZ ), spu_insert( spu_extract( m_AZ, VZ ), spu_promote( spu_extract( m_AW, VZ ), VW ), VZ ), VY ), VX );
    result.m_AW = spu_insert( spu_extract( m_AX, VW ), spu_insert( spu_extract( m_AY, VW ), spu_insert( spu_extract( m_AZ, VW ), spu_promote( spu_extract( m_AW, VW ), VW ), VZ ), VY ), VX );
    #elif defined( _PPU_ )
    const V128 tmp0( vec_mergeh( m_AX, m_AZ ) );
    const V128 tmp1( vec_mergeh( m_AY, m_AW ) );
    const V128 tmp2( vec_mergel( m_AX, m_AZ ) );
    const V128 tmp3( vec_mergel( m_AY, m_AW ) );
    result.m_AX = vec_mergeh( tmp0, tmp1 );
    result.m_AY = vec_mergel( tmp0, tmp1 );
    result.m_AZ = vec_mergeh( tmp2, tmp3 );
    result.m_AW = vec_mergel( tmp2, tmp3 );
    #elif defined( _VMX128_ )
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, m_CW ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, m_CW ) );
    result.m_CX = __vmrghw( tmp0, tmp1 );
    result.m_CY = __vmrglw( tmp0, tmp1 );
    result.m_CZ = __vmrghw( tmp2, tmp3 );
    result.m_CW = __vmrglw( tmp2, tmp3 );
    #elif defined( _SSE_ )
    const V128 tmp0( SIMD_MergeHigh( m_AX.m_V, m_AZ.m_V ) );
    const V128 tmp1( SIMD_MergeHigh( m_AY.m_V, m_AW.m_V ) );
    const V128 tmp2( SIMD_MergeLow ( m_AX.m_V, m_AZ.m_V ) );
    const V128 tmp3( SIMD_MergeLow ( m_AY.m_V, m_AW.m_V ) );
    result.m_AX.m_V = SIMD_MergeHigh( tmp0, tmp1 );
    result.m_AY.m_V = SIMD_MergeLow ( tmp0, tmp1 );
    result.m_AZ.m_V = SIMD_MergeHigh( tmp2, tmp3 );
    result.m_AW.m_V = SIMD_MergeLow ( tmp2, tmp3 );
    #else
    const HMat44 m = *this;
    const HVec4 ax( m[MX][VX], m[MY][VX], m[MZ][VX], m[MW][VX] );
    const HVec4 ay( m[MX][VY], m[MY][VY], m[MZ][VY], m[MW][VY] );
    const HVec4 az( m[MX][VZ], m[MY][VZ], m[MZ][VZ], m[MW][VZ] );
    const HVec4 aw( m[MX][VW], m[MY][VW], m[MZ][VW], m[MW][VW] );
    result.m_A[MX] = ax;
    result.m_A[MY] = ay;
    result.m_A[MZ] = az;
    result.m_A[MW] = aw;
    #endif
    return result;
}

/// Transpose this Matrix
NOALIAS __forceinline void HMat44::Transpose() __restrict
{
    #if defined( _SPU_ )
    //const vector float ax = (vector float)( spu_extract( m_AX, VX ), spu_extract( m_AY, VX ), spu_extract( m_AZ, VX ), spu_extract( m_AW, VX ) );
    //const vector float ay = (vector float)( spu_extract( m_AX, VY ), spu_extract( m_AY, VY ), spu_extract( m_AZ, VY ), spu_extract( m_AW, VY ) );
    //const vector float az = (vector float)( spu_extract( m_AX, VZ ), spu_extract( m_AY, VZ ), spu_extract( m_AZ, VZ ), spu_extract( m_AW, VZ ) );
    //const vector float aw = (vector float)( spu_extract( m_AX, VW ), spu_extract( m_AY, VW ), spu_extract( m_AZ, VW ), spu_extract( m_AW, VW ) );
    const vector float ax = spu_insert( spu_extract( m_AX, VX ), spu_insert( spu_extract( m_AY, VX ), spu_insert( spu_extract( m_AZ, VX ), spu_promote( spu_extract( m_AW, VX ), VW ), VZ ), VY ), VX );
    const vector float ay = spu_insert( spu_extract( m_AX, VY ), spu_insert( spu_extract( m_AY, VY ), spu_insert( spu_extract( m_AZ, VY ), spu_promote( spu_extract( m_AW, VY ), VW ), VZ ), VY ), VX );
    const vector float az = spu_insert( spu_extract( m_AX, VZ ), spu_insert( spu_extract( m_AY, VZ ), spu_insert( spu_extract( m_AZ, VZ ), spu_promote( spu_extract( m_AW, VZ ), VW ), VZ ), VY ), VX );
    const vector float aw = spu_insert( spu_extract( m_AX, VW ), spu_insert( spu_extract( m_AY, VW ), spu_insert( spu_extract( m_AZ, VW ), spu_promote( spu_extract( m_AW, VW ), VW ), VZ ), VY ), VX );
    m_AX = ax;
    m_AY = ay;
    m_AZ = az;
    m_AW = aw;
    #elif defined( _PPU_ )
    const V128 tmp0( vec_mergeh( m_AX, m_AZ ) );
    const V128 tmp1( vec_mergeh( m_AY, m_AW ) );
    const V128 tmp2( vec_mergel( m_AX, m_AZ ) );
    const V128 tmp3( vec_mergel( m_AY, m_AW ) );
    m_AX = vec_mergeh( tmp0, tmp1 );
    m_AY = vec_mergel( tmp0, tmp1 );
    m_AZ = vec_mergeh( tmp2, tmp3 );
    m_AW = vec_mergel( tmp2, tmp3 );
    #elif defined( _VMX128_ )
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
    const HMat44 m = *this;
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
NOALIAS __forceinline const HMat44 HMat44::operator * ( const HMat44& m2 ) const __restrict
{
    HMat44 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    const vector float m2_ax( m2.m_AX );
    const vector float m2_ay( m2.m_AY );
    const vector float m2_az( m2.m_AZ );
    const vector float m2_aw( m2.m_AW );
    
    result.m_AX = spu_madd( spu_splat<VX>( m_AX ), m2_ax,
                  spu_madd( spu_splat<VY>( m_AX ), m2_ay,
                  spu_madd( spu_splat<VZ>( m_AX ), m2_az,
                  spu_mul ( spu_splat<VW>( m_AX ), m2_aw ) ) ) );
                                      
    result.m_AY = spu_madd( spu_splat<VX>( m_AY ), m2_ax,
                  spu_madd( spu_splat<VY>( m_AY ), m2_ay,
                  spu_madd( spu_splat<VZ>( m_AY ), m2_az,
                  spu_mul ( spu_splat<VW>( m_AY ), m2_aw ) ) ) );
                                      
    result.m_AZ = spu_madd( spu_splat<VX>( m_AZ ), m2_ax,
                  spu_madd( spu_splat<VY>( m_AZ ), m2_ay,
                  spu_madd( spu_splat<VZ>( m_AZ ), m2_az,
                  spu_mul ( spu_splat<VW>( m_AZ ), m2_aw ) ) ) );
                                      
    result.m_AW = spu_madd( spu_splat<VX>( m_AW ), m2_ax,
                  spu_madd( spu_splat<VY>( m_AW ), m2_ay,
                  spu_madd( spu_splat<VZ>( m_AW ), m2_az,
                  spu_mul ( spu_splat<VW>( m_AW ), m2_aw ) ) ) );
    
    #elif defined( _PPU_ )
    
    const vector float m2_ax( m2.m_AX );
    const vector float m2_ay( m2.m_AY );
    const vector float m2_az( m2.m_AZ );
    const vector float m2_aw( m2.m_AW );
    
    result.m_AX = vec_madd( vec_splat( m_AX, VX ), m2_ax,
                  vec_madd( vec_splat( m_AX, VY ), m2_ay,
                  vec_madd( vec_splat( m_AX, VZ ), m2_az,
                  vec_mul ( vec_splat( m_AX, VW ), m2_aw ) ) ) );
    
    result.m_AY = vec_madd( vec_splat( m_AY, VX ), m2_ax,
                  vec_madd( vec_splat( m_AY, VY ), m2_ay,
                  vec_madd( vec_splat( m_AY, VZ ), m2_az,
                  vec_mul ( vec_splat( m_AY, VW ), m2_aw ) ) ) );
    
    result.m_AZ = vec_madd( vec_splat( m_AZ, VX ), m2_ax,
                  vec_madd( vec_splat( m_AZ, VY ), m2_ay,
                  vec_madd( vec_splat( m_AZ, VZ ), m2_az,
                  vec_mul ( vec_splat( m_AZ, VW ), m2_aw ) ) ) );
    
    result.m_AW = vec_madd( vec_splat( m_AW, VX ), m2_ax,
                  vec_madd( vec_splat( m_AW, VY ), m2_ay,
                  vec_madd( vec_splat( m_AW, VZ ), m2_az,
                  vec_mul ( vec_splat( m_AW, VW ), m2_aw ) ) ) );
    
    #elif defined( _VMX128_ )
    
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
    
    result.m_CX = __vmrghw( __vmrghw( result_cx_x, result_cx_z ), __vmrghw( result_cx_y, result_cx_w ) );
    result.m_CY = __vmrghw( __vmrghw( result_cy_x, result_cy_z ), __vmrghw( result_cy_y, result_cy_w ) );
    result.m_CZ = __vmrghw( __vmrghw( result_cz_x, result_cz_z ), __vmrghw( result_cz_y, result_cz_w ) );
    result.m_CW = __vmrghw( __vmrghw( result_cw_x, result_cw_z ), __vmrghw( result_cw_y, result_cw_w ) );
    
    #elif defined( _SSE_ )
    
    result.m_AX = m_AX * m2;
    result.m_AY = m_AY * m2;
    result.m_AZ = m_AZ * m2;
    result.m_AW = m_AW * m2;
    
    #else
    
    result.m_A[MX] = m_A[MX] * m2;
    result.m_A[MY] = m_A[MY] * m2;
    result.m_A[MZ] = m_A[MZ] * m2;
    result.m_A[MW] = m_A[MW] * m2;
    
    #endif
    
    return result;
}

/// Full 4x4 Matrix Inverse
NOALIAS __forceinline const HMat44 HMat44::Inverse() const __restrict
{
    #if defined( _SPU_ ) || defined( _PPU_ )
    HMat44 result;
    Matrix44InverseCramersRuleFP( (F32*)&m_AX, (F32*)&result.m_AX );
    return result;
    #elif defined( _SSE_ )
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
NOALIAS __forceinline const HMat44 HMat44::InverseRotTrans() const __restrict
{
    HMat44 result( UNDEFINED );
    
    #if 0 && defined( _SPU_ )
    
    //const vector float sign_mask( (vector float)spu_splats( 0x80000000 ) );
    const vector float sign_mask( Const< vector float >::SignMask() ) );
    
    //const vector float inv_rot_ax( (vector float)( spu_extract( m_AX, VX ), spu_extract( m_AY, VX ), spu_extract( m_AZ, VX ), 0.f ) );
    //const vector float inv_rot_ay( (vector float)( spu_extract( m_AX, VY ), spu_extract( m_AY, VY ), spu_extract( m_AZ, VY ), 0.f ) );
    //const vector float inv_rot_az( (vector float)( spu_extract( m_AX, VZ ), spu_extract( m_AY, VZ ), spu_extract( m_AZ, VZ ), 0.f ) );
    const vector float inv_rot_ax( spu_insert( spu_extract( m_AX, VX ), spu_insert( spu_extract( m_AY, VX ), spu_insert( spu_extract( m_AZ, VX ), spu_promote( 0.0f, VW ), VZ ), VY ), VX ) );
    const vector float inv_rot_ay( spu_insert( spu_extract( m_AX, VY ), spu_insert( spu_extract( m_AY, VY ), spu_insert( spu_extract( m_AZ, VY ), spu_promote( 0.0f, VW ), VZ ), VY ), VX ) );
    const vector float inv_rot_az( spu_insert( spu_extract( m_AX, VZ ), spu_insert( spu_extract( m_AY, VZ ), spu_insert( spu_extract( m_AZ, VZ ), spu_promote( 0.0f, VW ), VZ ), VY ), VX ) );
    
    const vector float neg_trans( spu_xor( m_AW, sign_mask ) );
    
    result.m_AX = inv_rot_ax;
    result.m_AY = inv_rot_ay;
    result.m_AZ = inv_rot_az;
    result.m_AW = spu_madd( spu_splat<VX>( neg_trans ), inv_rot_ax,
                  spu_madd( spu_splat<VY>( neg_trans ), inv_rot_ay,
                  spu_mul ( spu_splat<VZ>( neg_trans ), inv_rot_az ) ) );
    
    #elif 0 && defined( _PPU_ )
    
    const vector float zero( Const< vector float >::Zero() );
    //const vector float sign_mask( (vector float)0x80000000 );
    const vector float sign_mask( Const< vector float >::SignMask() ) );
    
    const vector float tmp0( vec_mergeh( m_AX, m_AZ ) );
    const vector float tmp1( vec_mergeh( m_AY, zero ) );
    const vector float tmp2( vec_mergel( m_AX, m_AZ ) );
    const vector float tmp3( vec_mergel( m_AY, zero ) );
    const vector float inv_rot_ax( vec_mergeh( tmp0, tmp1 ) );
    const vector float inv_rot_ay( vec_mergel( tmp0, tmp1 ) );
    const vector float inv_rot_az( vec_mergeh( tmp2, tmp3 ) );
    
    const vector float neg_trans( vec_xor( m_AW, sign_mask ) );
    
    result.m_AX = inv_rot_ax;
    result.m_AY = inv_rot_ay;
    result.m_AZ = inv_rot_az;
    result.m_AW = vec_madd( vec_splat( neg_trans, VX ), inv_rot_ax,
                  vec_madd( vec_splat( neg_trans, VY ), inv_rot_ay,
                  vec_mul ( vec_splat( neg_trans, VZ ), inv_rot_az ) ) );
    
    #elif defined( _VMX128_ )
    
    const V128 zero( __vzero() );
    const V128 vec_sign_mask( c_VectorSignMask );
    
    const V128 tmp0( __vmrghw( m_CX, m_CZ ) );
    const V128 tmp1( __vmrghw( m_CY, zero ) );
    const V128 tmp2( __vmrglw( m_CX, m_CZ ) );
    const V128 tmp3( __vmrglw( m_CY, zero ) );
    
    const V128 t_cx( __vmrghw( tmp0, tmp1 ) );
    const V128 t_cy( __vmrglw( tmp0, tmp1 ) );
    const V128 t_cz( __vmrghw( tmp2, tmp3 ) );
    const V128 t_cw( __vmrglw( tmp2, tmp3 ) );
    
    const V128 neg_trans( __vxor( t_cw, vec_sign_mask ) );
    
    const V128 inv_trans_x( __vmsum3fp( neg_trans, t_cx ) );
    const V128 inv_trans_y( __vmsum3fp( neg_trans, t_cy ) );
    const V128 inv_trans_z( __vmsum3fp( neg_trans, t_cz ) );
    
    const V128 inv_trans( __vmrghw( __vmrghw( inv_trans_x, inv_trans_z ), __vmrghw( inv_trans_y, zero ) ) );
    
    result.m_CX = __vrlimi( t_cx, inv_trans, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    result.m_CY = __vrlimi( t_cy, inv_trans, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    result.m_CZ = __vrlimi( t_cz, inv_trans, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    result.m_CW = c_UnitVectorW;
    
    #else
    
    const HMat33 invRot( ( HMat33( *this ) ).Transposed() );
    
    #if defined( _SIMD_ )
    result = HMat44( invRot.AX(),
                     invRot.AY(),
                     invRot.AZ(),
                   ( HVec3( -m_AW ) * invRot ) || HVecW( c_UnitVectorW ) );
    #else
    result = HMat44( HVec4( invRot.AX() ),
                     HVec4( invRot.AY() ),
                     HVec4( invRot.AZ() ),
                   ( HVec3( -m_A[MW] ) * invRot ) || HVecW( 1.0f ) );
    #endif
    
    #endif
    
    return result;
}













NOALIAS __forceinline const HMat44 operator * ( const HMat43& a, const HMat44& b )
{
    const HMat44 a44( a );
    return a44 * b;
}

NOALIAS __forceinline const HMat44 operator * ( const HMat44& a, const HMat43& b )
{
    const HMat44 b44( b );
    return a * b44;
}











#if ( DIRECT3D_VERSION >= 0x0900 )

/// Cast to D3DMATRIX
/*FORCE_INLINE
operator const D3DMATRIX (const HMat33 self) const
{
    return (D3DMATRIX)HMat44( self );
}

/// Cast to D3DXMATRIX
FORCE_INLINE
operator const D3DXMATRIX (const HMat33 self) const
{
    return (D3DXMATRIX)HMat44( self );
}*/

#endif

}; // namespace Maths



#endif
