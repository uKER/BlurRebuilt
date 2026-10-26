#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT43_INL
#define SOFT_MATHS_HVEC_NEW_MAT43_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT43_INL
#define MATHS_HVEC_NEW_MAT43_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat43
/// @{

/// Load Mat43 from an F32 array
/// NOTE: Matrix transposition is platform dependent!
template <> NOALIAS __forceinline const Mat43 Load( F32 const * const __restrict pSrc )
{
    Mat43 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_load_unaligned( &pSrc[0] );
    result.m_RowY = spu_load_unaligned( &pSrc[3] );
    result.m_RowZ = spu_load_unaligned( &pSrc[6] );
    result.m_RowW = spu_load_unaligned( &pSrc[9] );
    #elif defined( _PPU_ )
    result.m_RowX = vec_load_unaligned( &pSrc[0] );
    result.m_RowY = vec_load_unaligned( &pSrc[3] );
    result.m_RowZ = vec_load_unaligned( &pSrc[6] );
    result.m_RowW = vec_load_unaligned( &pSrc[9] );
    #elif defined( _VMX128_ )
    result.m_ColX = vmx_load_unaligned( &pSrc[0] );
    result.m_ColY = vmx_load_unaligned( &pSrc[4] );
    result.m_ColZ = vmx_load_unaligned( &pSrc[8] );
    #elif defined( _SSE_ )
    result.m_RowX = sse_load_unaligned( &pSrc[0] );
    result.m_RowY = sse_load_unaligned( &pSrc[3] );
    result.m_RowZ = sse_load_unaligned( &pSrc[6] );
    result.m_RowW = sse_load_unaligned( &pSrc[9] );
    #else
    result.m[0][0] = pSrc[0]; result.m[0][1] = pSrc[1];  result.m[0][2] = pSrc[2];
    result.m[1][0] = pSrc[3]; result.m[1][1] = pSrc[4];  result.m[1][2] = pSrc[5];
    result.m[2][0] = pSrc[6]; result.m[2][1] = pSrc[7];  result.m[2][2] = pSrc[8];
    result.m[3][0] = pSrc[9]; result.m[3][1] = pSrc[10]; result.m[3][2] = pSrc[11];
    #endif
    return result;
}

/// Store Mat43 into an F32 array
/// NOTE: Matrix transposition is platform dependent!
NOALIAS __forceinline void Store( const Mat43 MATREF src, F32* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec3( src.m_RowX, &pDst[0] );
    spu_store_unaligned_vec3( src.m_RowY, &pDst[3] );
    spu_store_unaligned_vec3( src.m_RowZ, &pDst[6] );
    spu_store_unaligned_vec3( src.m_RowW, &pDst[9] );
    #elif defined( _PPU_ )
    vec_store_unaligned_vec3( src.m_RowX, &pDst[0] );
    vec_store_unaligned_vec3( src.m_RowY, &pDst[3] );
    vec_store_unaligned_vec3( src.m_RowZ, &pDst[6] );
    vec_store_unaligned_vec3( src.m_RowW, &pDst[9] );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec4( src.m_ColX, &pDst[0] );
    vmx_store_unaligned_vec4( src.m_ColY, &pDst[4] );
    vmx_store_unaligned_vec4( src.m_ColZ, &pDst[8] );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec3( src.m_RowX, &pDst[0] );
    sse_store_unaligned_vec3( src.m_RowY, &pDst[3] );
    sse_store_unaligned_vec3( src.m_RowZ, &pDst[6] );
    sse_store_unaligned_vec3( src.m_RowW, &pDst[9] );
    #else
    pDst[0] = src.m[0][0]; pDst[1]  = src.m[0][1]; pDst[2]  = src.m[0][2];
    pDst[3] = src.m[1][0]; pDst[4]  = src.m[1][1]; pDst[5]  = src.m[1][2];
    pDst[6] = src.m[2][0]; pDst[7]  = src.m[2][1]; pDst[8]  = src.m[2][2];
    pDst[9] = src.m[3][0]; pDst[10] = src.m[3][1]; pDst[11] = src.m[3][2];
    #endif
}


/// Load Mat43 from a Float43
/// NOTE: Matrix transposition is platform dependent!
template <> NOALIAS __forceinline const Mat43 Load( MathsStorage::Float43 const * const __restrict pSrc )
{
    Mat43 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_load_unaligned( &pSrc->rx.x );
    result.m_RowY = spu_load_unaligned( &pSrc->ry.x );
    result.m_RowZ = spu_load_unaligned( &pSrc->rz.x );
    result.m_RowW = spu_load_unaligned( &pSrc->rw.x );
    #elif defined( _PPU_ )
    result.m_RowX = vec_load_unaligned( &pSrc->rx.x );
    result.m_RowY = vec_load_unaligned( &pSrc->ry.x );
    result.m_RowZ = vec_load_unaligned( &pSrc->rz.x );
    result.m_RowW = vec_load_unaligned( &pSrc->rw.x );
    #elif defined( _VMX128_ )
    result.m_ColX = vmx_load_unaligned( &pSrc->cx );
    result.m_ColY = vmx_load_unaligned( &pSrc->cy );
    result.m_ColZ = vmx_load_unaligned( &pSrc->cz );
    #elif defined( _SSE_ )
    result.m_RowX = sse_load_unaligned( &pSrc->rx.x );
    result.m_RowY = sse_load_unaligned( &pSrc->ry.x );
    result.m_RowZ = sse_load_unaligned( &pSrc->rz.x );
    result.m_RowW = sse_load_unaligned( &pSrc->rw.x );
    #else
    result.m[0][0] = pSrc->rx.x; result.m[0][1] = pSrc->rx.y; result.m[0][2] = pSrc->rx.z;
    result.m[1][0] = pSrc->ry.x; result.m[1][1] = pSrc->ry.y; result.m[1][2] = pSrc->ry.z;
    result.m[2][0] = pSrc->rz.x; result.m[2][1] = pSrc->rz.y; result.m[2][2] = pSrc->rz.z;
    result.m[3][0] = pSrc->rw.x; result.m[3][1] = pSrc->rw.y; result.m[3][2] = pSrc->rw.z;
    #endif
    return result;
}

/// Store Mat43 into a Float43
/// NOTE: Matrix transposition is platform dependent!
NOALIAS __forceinline void Store( const Mat43 MATREF src, MathsStorage::Float43* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec3( src.m_RowX, &pDst->rx.x );
    spu_store_unaligned_vec3( src.m_RowY, &pDst->ry.x );
    spu_store_unaligned_vec3( src.m_RowZ, &pDst->rz.x );
    spu_store_unaligned_vec3( src.m_RowW, &pDst->rw.x );
    #elif defined( _PPU_ )
    vec_store_unaligned_vec3( src.m_RowX, &pDst->rx );
    vec_store_unaligned_vec3( src.m_RowY, &pDst->ry );
    vec_store_unaligned_vec3( src.m_RowZ, &pDst->rz );
    vec_store_unaligned_vec3( src.m_RowW, &pDst->rw );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec4( src.m_ColX, &pDst->cx );
    vmx_store_unaligned_vec4( src.m_ColY, &pDst->cy );
    vmx_store_unaligned_vec4( src.m_ColZ, &pDst->cz );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec3( src.m_RowX, &pDst->rx.x );
    sse_store_unaligned_vec3( src.m_RowY, &pDst->ry.x );
    sse_store_unaligned_vec3( src.m_RowZ, &pDst->rz.x );
    sse_store_unaligned_vec3( src.m_RowW, &pDst->rw.x );
    #else
    pDst->rx.x = src.m[0][0]; pDst->rx.y = src.m[0][1]; pDst->rx.z = src.m[0][2];
    pDst->ry.x = src.m[1][0]; pDst->ry.y = src.m[1][1]; pDst->ry.z = src.m[1][2];
    pDst->rz.x = src.m[2][0]; pDst->rz.y = src.m[2][1]; pDst->rz.z = src.m[2][2];
    pDst->rw.x = src.m[3][0]; pDst->rw.y = src.m[3][1]; pDst->rw.z = src.m[3][2];
    #endif
}


NOALIAS __forceinline const Mat43& Const< Mat43 >::Zero()     { return m_Zero; }
NOALIAS __forceinline const Mat43& Const< Mat43 >::Identity() { return m_Identity; }


/// Make Mat43 from 12 float elements (row major)
NOALIAS __forceinline const Mat43 MakeMat43( const F32& m00, const F32& m01, const F32& m02,
                                             const F32& m10, const F32& m11, const F32& m12,
                                             const F32& m20, const F32& m21, const F32& m22,
                                             const F32& m30, const F32& m31, const F32& m32 )
{
    #if defined( _SIMD_ )
    #if defined( _VMX128_ )
    const F32 m[12] = { m00, m10, m20, m30,
                        m01, m11, m21, m31,
                        m02, m12, m22, m32 };
    #else
    const F32 m[12] = { m00, m01, m02,
                        m10, m11, m12,
                        m20, m21, m22,
                        m30, m31, m32 };
    #endif
    return Load<Mat43>( m );
    #else
    Mat43 result;
    result.m[0][0] = m00; result.m[0][1] = m01; result.m[0][2] = m02;
    result.m[1][0] = m10; result.m[1][1] = m11; result.m[1][2] = m12;
    result.m[2][0] = m20; result.m[2][1] = m21; result.m[2][2] = m22;
    result.m[3][0] = m30; result.m[3][1] = m31; result.m[3][2] = m32;
    return result;
    #endif
}


/// Make Mat43 from 4 row vectors (3 element)
NOALIAS __forceinline const Mat43 MakeMat43( const Vec4F32 VECREF rowX, const Vec4F32 VECREF rowY, const Vec4F32 VECREF rowZ, const Vec4F32 VECREF rowW )
{
    Mat43 result;
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    result.m_RowX = rowX.m_V;
    result.m_RowY = rowY.m_V;
    result.m_RowZ = rowZ.m_V;
    result.m_RowW = rowW.m_V;
    #elif defined( _VMX128_ )
    vmx_transpose_matrix34( rowX.m_V, rowY.m_V, rowZ.m_V, rowW.m_V, &result.m_ColX, &result.m_ColY, &result.m_ColZ );
    #else
    result.m[0][0] = rowX.m_V[0]; result.m[0][1] = rowX.m_V[1]; result.m[0][2] = rowX.m_V[2];
    result.m[1][0] = rowY.m_V[0]; result.m[1][1] = rowY.m_V[1]; result.m[1][2] = rowY.m_V[2];
    result.m[2][0] = rowZ.m_V[0]; result.m[2][1] = rowZ.m_V[1]; result.m[2][2] = rowZ.m_V[2];
    result.m[3][0] = rowW.m_V[0]; result.m[3][1] = rowW.m_V[1]; result.m[3][2] = rowW.m_V[2];
    #endif
    return result;
}


/// Make Mat43 from 4 row vectors (3 element)
NOALIAS __forceinline const Mat43 MakeMat43( const Vector3 PCREF rowX, const Vector3 PCREF rowY, const Vector3 PCREF rowZ, const Vector3 PCREF rowW )
{
    return MakeMat43( rowX.m_V, rowY.m_V, rowZ.m_V, rowW.m_V );
}

/// Make Mat43 from 4 row normals (3 element)
NOALIAS __forceinline const Mat43 MakeMat43( const Normal3 PCREF rowX, const Normal3 PCREF rowY, const Normal3 PCREF rowZ, const Normal3 PCREF rowW )
{
    return MakeMat43( rowX.m_V, rowY.m_V, rowZ.m_V, rowW.m_V );
}


/// Make Mat43 from a Mat33 - missing elements are zeroed
NOALIAS __forceinline const Mat43 MakeMat43( const Mat33 MATREF src )
{
    Mat43 result;
    #if defined( _SPU_ )
    result.m_RowX = src.m_RowX;
    result.m_RowY = src.m_RowY;
    result.m_RowZ = src.m_RowZ;
    result.m_RowW = Const< vector float >::Zero();
    #elif defined( _PPU_ )
    result.m_RowX = src.m_RowX;
    result.m_RowY = src.m_RowY;
    result.m_RowZ = src.m_RowZ;
    result.m_RowW = Const< vector float >::Zero();
    #elif defined( _VMX128_ )
    result.m_ColX = __vrlimi( src.m_ColX, Const< __vector4 >::Zero(), VMX_MASK_INSERT_W, 0 );
    result.m_ColY = __vrlimi( src.m_ColY, Const< __vector4 >::Zero(), VMX_MASK_INSERT_W, 0 );
    result.m_ColZ = __vrlimi( src.m_ColZ, Const< __vector4 >::Zero(), VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    result.m_RowX = src.m_RowX;
    result.m_RowY = src.m_RowY;
    result.m_RowZ = src.m_RowZ;
    result.m_RowW = Const< __m128 >::Zero();
    #else
    result.m[0][0] = src.m[0][0]; result.m[0][1] = src.m[0][1]; result.m[0][2] = src.m[0][2];
    result.m[1][0] = src.m[1][0]; result.m[1][1] = src.m[1][1]; result.m[1][2] = src.m[1][2];
    result.m[2][0] = src.m[2][0]; result.m[2][1] = src.m[2][1]; result.m[2][2] = src.m[2][2];
    result.m[3][0] = 0.0f;        result.m[3][1] = 0.0f;        result.m[3][2] = 0.0f;
    #endif
    return result;
}

/// Make Mat43 from a Mat44
NOALIAS __forceinline const Mat43 MakeMat43( const Mat44 MATREF src )
{
    Mat43 result;
    #if defined( _SPU_ )
    result.m_RowX = src.m_RowX;
    result.m_RowY = src.m_RowY;
    result.m_RowZ = src.m_RowZ;
    result.m_RowW = src.m_RowW;
    #elif defined( _PPU_ )
    result.m_RowX = src.m_RowX;
    result.m_RowY = src.m_RowY;
    result.m_RowZ = src.m_RowZ;
    result.m_RowW = src.m_RowW;
    #elif defined( _VMX128_ )
    result.m_ColX = src.m_ColX;
    result.m_ColY = src.m_ColY;
    result.m_ColZ = src.m_ColZ;
    #elif defined( _SSE_ )
    result.m_RowX = src.m_RowX;
    result.m_RowY = src.m_RowY;
    result.m_RowZ = src.m_RowZ;
    result.m_RowW = src.m_RowW;
    #else
    result.m[0][0] = src.m[0][0]; result.m[0][1] = src.m[0][1]; result.m[0][2] = src.m[0][2];
    result.m[1][0] = src.m[1][0]; result.m[1][1] = src.m[1][1]; result.m[1][2] = src.m[1][2];
    result.m[2][0] = src.m[2][0]; result.m[2][1] = src.m[2][1]; result.m[2][2] = src.m[2][2];
    result.m[3][0] = src.m[3][0]; result.m[3][1] = src.m[3][1]; result.m[3][2] = src.m[3][2];
    #endif
    return result;
}

/// Make Mat43 from a Mat33 rotation matrix & a Vec4F32 translation
NOALIAS __forceinline const Mat43 MakeMat43( const Mat33 MATREF r, const Vec4F32 VECREF t )
{
    Mat43 result;
    #if defined( _SPU_ )
    result.m_RowX = r.m_RowX;
    result.m_RowY = r.m_RowY;
    result.m_RowZ = r.m_RowZ;
    result.m_RowW = t.m_V;
    #elif defined( _PPU_ )
    result.m_RowX = r.m_RowX;
    result.m_RowY = r.m_RowY;
    result.m_RowZ = r.m_RowZ;
    result.m_RowW = t.m_V;
    #elif defined( _VMX128_ )
    result.m_ColX = __vrlimi( r.m_ColX, t.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    result.m_ColY = __vrlimi( r.m_ColY, t.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    result.m_ColZ = __vrlimi( r.m_ColZ, t.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #elif defined( _SSE_ )
    result.m_RowX = r.m_RowX;
    result.m_RowY = r.m_RowY;
    result.m_RowZ = r.m_RowZ;
    result.m_RowW = t.m_V;
    #else
    result.m[0][0] = r.m[0][0]; result.m[0][1] = r.m[0][1]; result.m[0][2] = r.m[0][2];
    result.m[1][0] = r.m[1][0]; result.m[1][1] = r.m[1][1]; result.m[1][2] = r.m[1][2];
    result.m[2][0] = r.m[2][0]; result.m[2][1] = r.m[2][1]; result.m[2][2] = r.m[2][2];
    result.m[3][0] = t.m_V[0];  result.m[3][1] = t.m_V[1];  result.m[3][2] = t.m_V[2];
    #endif
    return result;
}

/// Make Mat43 from a Mat33 rotation matrix & a Vector3 translation
NOALIAS __forceinline const Mat43 MakeMat43( const Mat33 MATREF r, const Vector3 PCREF t )
{
    return MakeMat43( r, t.m_V );
}

/// Make Mat43 from a Mat33 rotation matrix & a Point3 position
NOALIAS __forceinline const Mat43 MakeMat43( const Mat33 MATREF r, const Point3 PCREF p )
{
    return MakeMat43( r, p.m_V );
}

/// Make Mat43 camera 'LookAt' matrix, NOTE: this returns an inverse camera matrix
NOALIAS __forceinline const Mat43 MakeMat43_CameraLookAt( const Point3 PCREF cameraPosition, const Point3 PCREF targetPosition )
{
    const Vector3 vectorToTarget( targetPosition - cameraPosition );
    const Mat33 lookatMatrix( MakeMat33_AxisZ( vectorToTarget ) );
    return Inverse( MakeMat43( lookatMatrix, cameraPosition ) );
}




/// Make Mat43 from a Reflection Plane
NOALIAS __forceinline const Mat43 MakeMat43( const Plane PCREF plane )
{
    // -2 * P.a * P.a + 1  -2 * P.b * P.a      -2 * P.c * P.a        0
    // -2 * P.a * P.b      -2 * P.b * P.b + 1  -2 * P.c * P.b        0
    // -2 * P.a * P.c      -2 * P.b * P.c      -2 * P.c * P.c + 1    0
    // -2 * P.a * P.d      -2 * P.b * P.d      -2 * P.c * P.d        1
    
    const Vec4F32 p( plane.m_V );
    const VF32    minusTwo( Const<VF32>::Int<-2>() );
    const VF32    zero( Const<VF32>::Zero() );
    const Vec4F32 k( MakeVec4F32( minusTwo, minusTwo, minusTwo, zero ) );
    const Vec4F32 s( p * k );
    const Vec4F32 a( MakeVec4F32( X(p) ) );
    const Vec4F32 b( MakeVec4F32( Y(p) ) );
    const Vec4F32 c( MakeVec4F32( Z(p) ) );
    const Vec4F32 d( MakeVec4F32( W(p) ) );
    Mat43 result;
    SetRows( &result,
             MulAdd( a, s, Const<Vec4F32>::UnitX() ),
             MulAdd( b, s, Const<Vec4F32>::UnitY() ),
             MulAdd( c, s, Const<Vec4F32>::UnitZ() ),
             MulAdd( d, s, Const<Vec4F32>::UnitW() ) );
    return result;
}

/// Mat43 Write to Stream
template < typename T_Stream >
void Serialise( const Mat43 MATREF src, T_Stream& stream )
{
    stream << RowX( src );
    stream << RowY( src );
    stream << RowZ( src );
    stream << RowW( src );
}

/// Mat43 Read from Stream
template < typename T_Stream >
void Deserialise( T_Stream& stream, Mat43* const __restrict pDst )
{
    Vector3 rowX, rowY, rowZ, rowW;
    stream >> rowX;
    stream >> rowY;
    stream >> rowZ;
    stream >> rowW;
    *pDst = MakeMat43( rowX, rowY, rowZ, rowW );
}




/// Get rows from a Mat43
NOALIAS __forceinline void GetRows( const Mat43 MATREF src, Vec4F32* __restrict pRowX, Vec4F32* __restrict pRowY, Vec4F32* __restrict pRowZ, Vec4F32* __restrict pRowW )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    pRowX->m_V = src.m_RowX;
    pRowY->m_V = src.m_RowY;
    pRowZ->m_V = src.m_RowZ;
    pRowW->m_V = src.m_RowW;
    #elif defined( _VMX128_ )
    vmx_transpose_matrix43( src.m_ColX, src.m_ColY, src.m_ColZ, &pRowX->m_V, &pRowY->m_V, &pRowZ->m_V, &pRowW->m_V );
    #else
    pRowX->m_V[0] = src.m[0][0]; pRowX->m_V[1] = src.m[0][1]; pRowX->m_V[2] = src.m[0][2];
    pRowY->m_V[0] = src.m[1][0]; pRowY->m_V[1] = src.m[1][1]; pRowY->m_V[2] = src.m[1][2];
    pRowZ->m_V[0] = src.m[2][0]; pRowZ->m_V[1] = src.m[2][1]; pRowZ->m_V[2] = src.m[2][2];
    pRowW->m_V[0] = src.m[3][0]; pRowW->m_V[1] = src.m[3][1]; pRowW->m_V[2] = src.m[3][2];
    #endif
}

/// Set rows of a Mat43
NOALIAS __forceinline void SetRows( Mat43* pDst, const Vec4F32 VECREF rowX, const Vec4F32 VECREF rowY, const Vec4F32 VECREF rowZ, const Vec4F32 VECREF rowW )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    pDst->m_RowX = rowX.m_V;
    pDst->m_RowY = rowY.m_V;
    pDst->m_RowZ = rowZ.m_V;
    pDst->m_RowW = rowW.m_V;
    #elif defined( _VMX128_ )
    vmx_transpose_matrix34( rowX.m_V, rowY.m_V, rowZ.m_V, rowW.m_V, &pDst->m_ColX, &pDst->m_ColY, &pDst->m_ColZ );
    #else
    pDst->m[0][0] = rowX.m_V[0]; pDst->m[0][1] = rowX.m_V[1]; pDst->m[0][2] = rowX.m_V[2];
    pDst->m[1][0] = rowY.m_V[0]; pDst->m[1][1] = rowY.m_V[1]; pDst->m[1][2] = rowY.m_V[2];
    pDst->m[2][0] = rowZ.m_V[0]; pDst->m[2][1] = rowZ.m_V[1]; pDst->m[2][2] = rowZ.m_V[2];
    pDst->m[3][0] = rowW.m_V[0]; pDst->m[3][1] = rowW.m_V[1]; pDst->m[3][2] = rowW.m_V[2];
    #endif
}


/// Get rows from a Mat43
NOALIAS __forceinline void GetRows( const Mat43 MATREF src, Vector4* __restrict pRowX, Vector4* __restrict pRowY, Vector4* __restrict pRowZ, Vector4* __restrict pRowW )
{
    GetRows( src, &pRowX->m_V, &pRowY->m_V, &pRowZ->m_V, &pRowW->m_V );
}

/// Set rows of a Mat43
NOALIAS __forceinline void SetRows( Mat43* pDst, const Vector4 PCREF rowX, const Vector4 PCREF rowY, const Vector4 PCREF rowZ, const Vector4 PCREF rowW )
{
    SetRows( pDst, rowX.m_V, rowY.m_V, rowZ.m_V, rowW.m_V );
}


/// Get Row X from a Mat43
NOALIAS __forceinline const Vector3 RowX( const Mat43 MATREF src )
{
    Vector3 result;
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    result.m_V.m_V = src.m_RowX;
    #elif defined( _VMX128_ )
    //result.m_V.m_V = vmx_merge_xy( vmx_merge_xy( src.m_ColX, src.m_ColZ ),
    //                           vmx_merge_xy( src.m_ColY, Const< __vector4 >::Zero() ) );
    result.m_V.m_V = vmx_transpose_row_x( src.m_ColX, src.m_ColY, src.m_ColZ, Const< __vector4 >::Zero() );
    #else
    result.m_V.m_V[VX] = src.m[0][0];
    result.m_V.m_V[VY] = src.m[0][1];
    result.m_V.m_V[VZ] = src.m[0][2];
    #endif
    return result;
}

/// Get Row Y from a Mat43
NOALIAS __forceinline const Vector3 RowY( const Mat43 MATREF src )
{
    Vector3 result;
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    result.m_V.m_V = src.m_RowY;
    #elif defined( _VMX128_ )
    //result.m_V.m_V = vmx_merge_zw( vmx_merge_xy( src.m_ColX, src.m_ColZ ),
    //                           vmx_merge_xy( src.m_ColY, Const< __vector4 >::Zero() ) );
    result.m_V.m_V = vmx_transpose_row_y( src.m_ColX, src.m_ColY, src.m_ColZ, Const< __vector4 >::Zero() );
    #else
    result.m_V.m_V[VX] = src.m[1][0];
    result.m_V.m_V[VY] = src.m[1][1];
    result.m_V.m_V[VZ] = src.m[1][2];
    #endif
    return result;
}

/// Get Row Z from a Mat43
NOALIAS __forceinline const Vector3 RowZ( const Mat43 MATREF src )
{
    Vector3 result;
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    result.m_V.m_V = src.m_RowZ;
    #elif defined( _VMX128_ )
    //result.m_V.m_V = vmx_merge_xy( vmx_merge_zw( src.m_ColX, src.m_ColZ ),
    //                           vmx_merge_zw( src.m_ColY, Const< __vector4 >::Zero() ) );
    result.m_V.m_V = vmx_transpose_row_z( src.m_ColX, src.m_ColY, src.m_ColZ, Const< __vector4 >::Zero() );
    #else
    result.m_V.m_V[VX] = src.m[2][0];
    result.m_V.m_V[VY] = src.m[2][1];
    result.m_V.m_V[VZ] = src.m[2][2];
    #endif
    return result;
}

/// Get Row W from a Mat43
NOALIAS __forceinline const Vector3 RowW( const Mat43 MATREF src )
{
    Vector3 result;
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    result.m_V.m_V = src.m_RowW;
    #elif defined( _VMX128_ )
    result.m_V.m_V = vmx_transpose_row_w( src.m_ColX, src.m_ColY, src.m_ColZ, Const< __vector4 >::Zero() );
    #else
    result.m_V.m_V[VX] = src.m[3][0];
    result.m_V.m_V[VY] = src.m[3][1];
    result.m_V.m_V[VZ] = src.m[3][2];
    #endif
    return result;
}


/// Get Col X from a Mat43
NOALIAS __forceinline const Vector4 ColX( const Mat43 MATREF src )
{
    Vector4 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_transpose_col_x( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_transpose_col_x( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW );
    #elif defined( _VMX128_ )
    result.m_V.m_V = src.m_ColX;
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_transpose_col_x( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW );
    #else
    result.m_V.m_V[VX] = src.m[0][0];
    result.m_V.m_V[VY] = src.m[1][0];
    result.m_V.m_V[VZ] = src.m[2][0];
    result.m_V.m_V[VW] = src.m[3][0];
    #endif
    return result;
}

/// Get Col Y from a Mat43
NOALIAS __forceinline const Vector4 ColY( const Mat43 MATREF src )
{
    Vector4 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_transpose_col_y( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_transpose_col_y( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW );
    #elif defined( _VMX128_ )
    result.m_V.m_V = src.m_ColY;
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_transpose_col_y( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW );
    #else
    result.m_V.m_V[VX] = src.m[0][1];
    result.m_V.m_V[VY] = src.m[1][1];
    result.m_V.m_V[VZ] = src.m[2][1];
    result.m_V.m_V[VW] = src.m[3][1];
    #endif
    return result;
}

/// Get Col Z from a Mat43
NOALIAS __forceinline const Vector4 ColZ( const Mat43 MATREF src )
{
    Vector4 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_transpose_col_z( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_transpose_col_z( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW );
    #elif defined( _VMX128_ )
    result.m_V.m_V = src.m_ColZ;
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_transpose_col_z( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW );
    #else
    result.m_V.m_V[VX] = src.m[0][2];
    result.m_V.m_V[VY] = src.m[1][2];
    result.m_V.m_V[VZ] = src.m[2][2];
    result.m_V.m_V[VW] = src.m[3][2];
    #endif
    return result;
}


/// Get element from a Mat43
template < unsigned int Row, unsigned int Col > NOALIAS __forceinline const VF32 Element( const Mat43 MATREF src )
{
    BIZ_STATIC_ASSERT( Row < 4 && Col < 3 );
    VF32 result;
    #if defined( _SPU_ )
    vector float row;
    switch ( Row )
    {
        case E_RowX : row = src.m_RowX; break;
        case E_RowY : row = src.m_RowY; break;
        case E_RowZ : row = src.m_RowZ; break;
        case E_RowW : row = src.m_RowW; break;
        default     : row = Const< vector float >::Zero(); break;
    };
    switch ( Col )
    {
        case E_ColX : result.m_V = spu_splat<VX>( row ); break;
        case E_ColY : result.m_V = spu_splat<VY>( row ); break;
        case E_ColZ : result.m_V = spu_splat<VZ>( row ); break;
        default     : result.m_V = Const< vector float >::Zero(); break;
    };
    #elif defined( _PPU_ )
    vector float row;
    switch ( Row )
    {
        case E_RowX : row = src.m_RowX; break;
        case E_RowY : row = src.m_RowY; break;
        case E_RowZ : row = src.m_RowZ; break;
        case E_RowW : row = src.m_RowW; break;
        default     : row = Const< vector float >::Zero(); break;
    };
    switch ( Col )
    {
        case E_ColX : result.m_V = vec_splat( row, VX ); break;
        case E_ColY : result.m_V = vec_splat( row, VY ); break;
        case E_ColZ : result.m_V = vec_splat( row, VZ ); break;
        default     : result.m_V = Const< vector float >::Zero(); break;
    };
    #elif defined( _VMX128_ )
    __vector4 col;
    switch ( Col )
    {
        case E_ColX : col = src.m_ColX; break;
        case E_ColY : col = src.m_ColY; break;
        case E_ColZ : col = src.m_ColZ; break;
        default     : col = Const< __vector4 >::Zero(); break;
    };
    switch ( Row )
    {
        case E_RowX : result.m_V = __vspltw( col, VX ); break;
        case E_RowY : result.m_V = __vspltw( col, VY ); break;
        case E_RowZ : result.m_V = __vspltw( col, VZ ); break;
        case E_RowW : result.m_V = __vspltw( col, VW ); break;
        default     : result.m_V = Const< __vector4 >::Zero(); break;
    };
    #elif defined( _SSE_ )
    __m128 row;
    switch ( Row )
    {
        case E_RowX : row = src.m_RowX; break;
        case E_RowY : row = src.m_RowY; break;
        case E_RowZ : row = src.m_RowZ; break;
        case E_RowW : row = src.m_RowW; break;
        default     : row = Const< __m128 >::Zero(); break;
    };
    switch ( Col )
    {
        case E_ColX : result.m_V = sse_splat<VX>( row ); break;
        case E_ColY : result.m_V = sse_splat<VY>( row ); break;
        case E_ColZ : result.m_V = sse_splat<VZ>( row ); break;
        default     : result.m_V = Const< __m128 >::Zero(); break;
    };
    #else
    result.m_V = src.m[Row][Col];
    #endif
    return result;
}


/// Set Row X of a Mat43
NOALIAS __forceinline void SetRowX( Mat43* pDst, const Vec4F32 VECREF src )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    pDst->m_RowX = src.m_V;
    #elif defined( _VMX128_ )
    pDst->m_ColX = __vrlimi( pDst->m_ColX, src.m_V, VMX_MASK_INSERT_X, VMX_ROTATE_X_TO_X );
    pDst->m_ColY = __vrlimi( pDst->m_ColY, src.m_V, VMX_MASK_INSERT_X, VMX_ROTATE_Y_TO_X );
    pDst->m_ColZ = __vrlimi( pDst->m_ColZ, src.m_V, VMX_MASK_INSERT_X, VMX_ROTATE_Z_TO_X );
    #else
    pDst->m[0][0] = src.m_V[VX];
    pDst->m[0][1] = src.m_V[VY];
    pDst->m[0][2] = src.m_V[VZ];
    #endif
}

/// Set Row Y of a Mat43
NOALIAS __forceinline void SetRowY( Mat43* pDst, const Vec4F32 VECREF src )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    pDst->m_RowY = src.m_V;
    #elif defined( _VMX128_ )
    pDst->m_ColX = __vrlimi( pDst->m_ColX, src.m_V, VMX_MASK_INSERT_Y, VMX_ROTATE_X_TO_Y );
    pDst->m_ColY = __vrlimi( pDst->m_ColY, src.m_V, VMX_MASK_INSERT_Y, VMX_ROTATE_Y_TO_Y );
    pDst->m_ColZ = __vrlimi( pDst->m_ColZ, src.m_V, VMX_MASK_INSERT_Y, VMX_ROTATE_Z_TO_Y );
    #else
    pDst->m[1][0] = src.m_V[VX];
    pDst->m[1][1] = src.m_V[VY];
    pDst->m[1][2] = src.m_V[VZ];
    #endif
}

/// Set Row Z of a Mat43
NOALIAS __forceinline void SetRowZ( Mat43* pDst, const Vec4F32 VECREF src )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    pDst->m_RowZ = src.m_V;
    #elif defined( _VMX128_ )
    pDst->m_ColX = __vrlimi( pDst->m_ColX, src.m_V, VMX_MASK_INSERT_Z, VMX_ROTATE_X_TO_Z );
    pDst->m_ColY = __vrlimi( pDst->m_ColY, src.m_V, VMX_MASK_INSERT_Z, VMX_ROTATE_Y_TO_Z );
    pDst->m_ColZ = __vrlimi( pDst->m_ColZ, src.m_V, VMX_MASK_INSERT_Z, VMX_ROTATE_Z_TO_Z );
    #else
    pDst->m[2][0] = src.m_V[VX];
    pDst->m[2][1] = src.m_V[VY];
    pDst->m[2][2] = src.m_V[VZ];
    #endif
}

/// Set Row W of a Mat43
NOALIAS __forceinline void SetRowW( Mat43* pDst, const Vec4F32 VECREF src )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    pDst->m_RowW = src.m_V;
    #elif defined( _VMX128_ )
    pDst->m_ColX = __vrlimi( pDst->m_ColX, src.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    pDst->m_ColY = __vrlimi( pDst->m_ColY, src.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    pDst->m_ColZ = __vrlimi( pDst->m_ColZ, src.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    #else
    pDst->m[3][0] = src.m_V[VX];
    pDst->m[3][1] = src.m_V[VY];
    pDst->m[3][2] = src.m_V[VZ];
    #endif
}


/// Set Row X of a Mat43
NOALIAS __forceinline void SetRowX( Mat43* pDst, const Vector3 PCREF src )
{
    SetRowX( pDst, src.m_V );
}

/// Set Row Y of a Mat43
NOALIAS __forceinline void SetRowY( Mat43* pDst, const Vector3 PCREF src )
{
    SetRowY( pDst, src.m_V );
}

/// Set Row Z of a Mat43
NOALIAS __forceinline void SetRowZ( Mat43* pDst, const Vector3 PCREF src )
{
    SetRowZ( pDst, src.m_V );
}

/// Set Row W of a Mat43
NOALIAS __forceinline void SetRowW( Mat43* pDst, const Vector3 PCREF src )
{
    SetRowW( pDst, src.m_V );
}


/// Mat43 Get rotation Mat33
NOALIAS __forceinline const Mat33 Rotation( const Mat43 MATREF matrix )
{
    //Mat33 result;
    //#if defined( _SPU_ )
    //result.m_RowX = src.m_RowX;
    //result.m_RowY = src.m_RowY;
    //result.m_RowZ = src.m_RowZ;
    //#elif defined( _PPU_ )
    //result.m_RowX = src.m_RowX;
    //result.m_RowY = src.m_RowY;
    //result.m_RowZ = src.m_RowZ;
    //#elif defined( _VMX128_ )
    //result.m_ColX = src.m_ColX;
    //result.m_ColY = src.m_ColY;
    //result.m_ColZ = src.m_ColZ;
    //#elif defined( _SSE_ )
    //result.m_RowX = src.m_RowX;
    //result.m_RowY = src.m_RowY;
    //result.m_RowZ = src.m_RowZ;
    //#else
    //result.m[0][0] = src.m[0][0]; result.m[0][1] = src.m[0][1]; result.m[0][2] = src.m[0][2];
    //result.m[1][0] = src.m[1][0]; result.m[1][1] = src.m[1][1]; result.m[1][2] = src.m[1][2];
    //result.m[2][0] = src.m[2][0]; result.m[2][1] = src.m[2][1]; result.m[2][2] = src.m[2][2];
    //#endif
    //return result;
    return MakeMat33( matrix );
}

/// Mat43 Get translation Vector3
NOALIAS __forceinline const Vector3 Translation( const Mat43 MATREF matrix )
{
    return RowW( matrix );
}


/// Mat43 Addition operator
NOALIAS __forceinline const Mat43 operator + ( const Mat43 MATREF lhs, const Mat43 MATREF rhs )
{
    Mat43 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_add( lhs.m_RowX, rhs.m_RowX );
    result.m_RowY = spu_add( lhs.m_RowY, rhs.m_RowY );
    result.m_RowZ = spu_add( lhs.m_RowZ, rhs.m_RowZ );
    result.m_RowW = spu_add( lhs.m_RowW, rhs.m_RowW );
    #elif defined( _PPU_ )
    result.m_RowX = vec_add( lhs.m_RowX, rhs.m_RowX );
    result.m_RowY = vec_add( lhs.m_RowY, rhs.m_RowY );
    result.m_RowZ = vec_add( lhs.m_RowZ, rhs.m_RowZ );
    result.m_RowW = vec_add( lhs.m_RowW, rhs.m_RowW );
    #elif defined( _VMX128_ )
    result.m_ColX = __vaddfp( lhs.m_ColX, rhs.m_ColX );
    result.m_ColY = __vaddfp( lhs.m_ColY, rhs.m_ColY );
    result.m_ColZ = __vaddfp( lhs.m_ColZ, rhs.m_ColZ );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_add_ps( lhs.m_RowX, rhs.m_RowX );
    result.m_RowY = _mm_add_ps( lhs.m_RowY, rhs.m_RowY );
    result.m_RowZ = _mm_add_ps( lhs.m_RowZ, rhs.m_RowZ );
    result.m_RowW = _mm_add_ps( lhs.m_RowW, rhs.m_RowW );
    #else
    for ( S32 r=0; r<4; r++ )
    {
        for ( S32 c=0; c<3; c++ )
        {
            result.m[r][c] = lhs.m[r][c] + rhs.m[r][c];
        }
    }
    #endif
    return result;
}

/// Mat43 Subtraction operator
NOALIAS __forceinline const Mat43 operator - ( const Mat43 MATREF lhs, const Mat43 MATREF rhs )
{
    Mat43 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_sub( lhs.m_RowX, rhs.m_RowX );
    result.m_RowY = spu_sub( lhs.m_RowY, rhs.m_RowY );
    result.m_RowZ = spu_sub( lhs.m_RowZ, rhs.m_RowZ );
    result.m_RowW = spu_sub( lhs.m_RowW, rhs.m_RowW );
    #elif defined( _PPU_ )
    result.m_RowX = vec_sub( lhs.m_RowX, rhs.m_RowX );
    result.m_RowY = vec_sub( lhs.m_RowY, rhs.m_RowY );
    result.m_RowZ = vec_sub( lhs.m_RowZ, rhs.m_RowZ );
    result.m_RowW = vec_sub( lhs.m_RowW, rhs.m_RowW );
    #elif defined( _VMX128_ )
    result.m_ColX = __vsubfp( lhs.m_ColX, rhs.m_ColX );
    result.m_ColY = __vsubfp( lhs.m_ColY, rhs.m_ColY );
    result.m_ColZ = __vsubfp( lhs.m_ColZ, rhs.m_ColZ );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_sub_ps( lhs.m_RowX, rhs.m_RowX );
    result.m_RowY = _mm_sub_ps( lhs.m_RowY, rhs.m_RowY );
    result.m_RowZ = _mm_sub_ps( lhs.m_RowZ, rhs.m_RowZ );
    result.m_RowW = _mm_sub_ps( lhs.m_RowW, rhs.m_RowW );
    #else
    for ( S32 r=0; r<4; r++ )
    {
        for ( S32 c=0; c<3; c++ )
        {
            result.m[r][c] = lhs.m[r][c] - rhs.m[r][c];
        }
    }
    #endif
    return result;
}

/// Mat43 * VF32 Multiplication operator
NOALIAS __forceinline const Mat43 operator * ( const Mat43 MATREF matrix, const VF32 VECREF scalar )
{
    Mat43 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_mul( matrix.m_RowX, scalar.m_V );
    result.m_RowY = spu_mul( matrix.m_RowY, scalar.m_V );
    result.m_RowZ = spu_mul( matrix.m_RowZ, scalar.m_V );
    result.m_RowW = spu_mul( matrix.m_RowW, scalar.m_V );
    #elif defined( _PPU_ )
    result.m_RowX = vec_mul( matrix.m_RowX, scalar.m_V );
    result.m_RowY = vec_mul( matrix.m_RowY, scalar.m_V );
    result.m_RowZ = vec_mul( matrix.m_RowZ, scalar.m_V );
    result.m_RowW = vec_mul( matrix.m_RowW, scalar.m_V );
    #elif defined( _VMX128_ )
    result.m_ColX = __vmulfp( matrix.m_ColX, scalar.m_V );
    result.m_ColY = __vmulfp( matrix.m_ColY, scalar.m_V );
    result.m_ColZ = __vmulfp( matrix.m_ColZ, scalar.m_V );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_mul_ps( matrix.m_RowX, scalar.m_V );
    result.m_RowY = _mm_mul_ps( matrix.m_RowY, scalar.m_V );
    result.m_RowZ = _mm_mul_ps( matrix.m_RowZ, scalar.m_V );
    result.m_RowW = _mm_mul_ps( matrix.m_RowW, scalar.m_V );
    #else
    for ( S32 r=0; r<4; r++ )
    {
        for ( S32 c=0; c<3; c++ )
        {
            result.m[r][c] = matrix.m[r][c] * scalar.m_V;
        }
    }
    #endif
    return result;
}

/// VF32 * Mat43 Multiplication operator
NOALIAS __forceinline const Mat43 operator * ( const VF32 VECREF scalar, const Mat43 MATREF matrix )
{
    Mat43 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_mul( matrix.m_RowX, scalar.m_V );
    result.m_RowY = spu_mul( matrix.m_RowY, scalar.m_V );
    result.m_RowZ = spu_mul( matrix.m_RowZ, scalar.m_V );
    result.m_RowW = spu_mul( matrix.m_RowW, scalar.m_V );
    #elif defined( _PPU_ )
    result.m_RowX = vec_mul( matrix.m_RowX, scalar.m_V );
    result.m_RowY = vec_mul( matrix.m_RowY, scalar.m_V );
    result.m_RowZ = vec_mul( matrix.m_RowZ, scalar.m_V );
    result.m_RowW = vec_mul( matrix.m_RowW, scalar.m_V );
    #elif defined( _VMX128_ )
    result.m_ColX = __vmulfp( matrix.m_ColX, scalar.m_V );
    result.m_ColY = __vmulfp( matrix.m_ColY, scalar.m_V );
    result.m_ColZ = __vmulfp( matrix.m_ColZ, scalar.m_V );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_mul_ps( matrix.m_RowX, scalar.m_V );
    result.m_RowY = _mm_mul_ps( matrix.m_RowY, scalar.m_V );
    result.m_RowZ = _mm_mul_ps( matrix.m_RowZ, scalar.m_V );
    result.m_RowW = _mm_mul_ps( matrix.m_RowW, scalar.m_V );
    #else
    for ( S32 r=0; r<4; r++ )
    {
        for ( S32 c=0; c<3; c++ )
        {
            result.m[r][c] = matrix.m[r][c] * scalar.m_V;
        }
    }
    #endif
    return result;
}


/// Vec4F32 * Mat43 Multiplication operator : input = 4 element row vector, output = 4 element row vector
/// NOTE: Mat43 Column W is implicit (0,0,0,1)!
NOALIAS __forceinline const Vec4F32 operator * ( const Vec4F32 VECREF v, const Mat43 MATREF m )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    //result.m_V = spu_add( spu_madd( spu_splat<VX>( v.m_V ), m.m_RowX,
    //                      spu_mul ( spu_splat<VY>( v.m_V ), m.m_RowY ) ),
    //                      spu_madd( spu_splat<VZ>( v.m_V ), m.m_RowZ,
    //                      spu_mul ( spu_splat<VW>( v.m_V ), m.m_RowW ) ) );
    //result.m_V = spu_select<0,0,0,1>( result.m_V, v.m_V );
    result.m_V = spu_multiply_vector4_matrix43( v.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #elif defined( _PPU_ )
    //result.m_V = vec_add( vec_madd( vec_splat( v.m_V, VX ), m.m_RowX,
    //                      vec_mul ( vec_splat( v.m_V, VY ), m.m_RowY ) ),
    //                      vec_madd( vec_splat( v.m_V, VZ ), m.m_RowZ,
    //                      vec_mul ( vec_splat( v.m_V, VW ), m.m_RowW ) ) );
    //result.m_V = vec_select<0,0,0,1>( result.m_V, v.m_V );
    result.m_V = vec_multiply_vector4_matrix43( v.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #elif defined( _VMX128_ )
    //result.m_V = vmx_make_vector4( __vmsum4fp( v.m_V, m.m_ColX ),
    //                               __vmsum4fp( v.m_V, m.m_ColY ),
    //                               __vmsum4fp( v.m_V, m.m_ColZ ),
    //                                 __vspltw( v.m_V, VW ) );
    result.m_V = vmx_multiply_vector4_matrix43( v.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    //result.m_V = _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( v.m_V ), m.m_RowX ),
    //                                     _mm_mul_ps( sse_splat<VY>( v.m_V ), m.m_RowY ) ),
    //                         _mm_add_ps( _mm_mul_ps( sse_splat<VZ>( v.m_V ), m.m_RowZ ),
    //                                     _mm_mul_ps( sse_splat<VW>( v.m_V ), m.m_RowW ) ) );
    //result.m_V = sse_select<0,0,0,1>( result.m_V, v.m_V );
    result.m_V = sse_multiply_vector4_matrix43( v.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #else
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V[c] = v.m_V[0] * m.m[0][c] +
                        v.m_V[1] * m.m[1][c] +
                        v.m_V[2] * m.m[2][c] +
                        v.m_V[3] * m.m[3][c];
    }
    result.m_V[3] = v.m_V[3];
    #endif
    return result;
}

/// Mat43 * Vec4F32 Multiplication operator : input = 4 element col vector, output = 4 element col vector
NOALIAS __forceinline const Vec4F32 operator * ( const Mat43 MATREF m43, const Vec4F32 VECREF v4 )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    const vector float col_w = Const< vector float >::UnitW();
    vector float col_x, col_y, col_z;
    spu_transpose_matrix43( m43.m_RowX, m43.m_RowY, m43.m_RowZ, m43.m_RowW, &col_x, &col_y, &col_z );
    result.m_V = spu_add( spu_madd( col_x, spu_splat<VX>( v4.m_V ),
                          spu_mul ( col_y, spu_splat<VY>( v4.m_V ) ) ),
                          spu_madd( col_z, spu_splat<VZ>( v4.m_V ),
                          spu_mul ( col_w, spu_splat<VW>( v4.m_V ) ) ) );
    #elif defined( _PPU_ )
    const vector float col_w = Const< vector float >::UnitW();
    vector float col_x, col_y, col_z;
    vec_transpose_matrix43( m43.m_RowX, m43.m_RowY, m43.m_RowZ, m43.m_RowW, &col_x, &col_y, &col_z );
    result.m_V = vec_add( vec_madd( col_x, vec_splat( v4.m_V, VX ),
                          vec_mul ( col_y, vec_splat( v4.m_V, VY ) ) ),
                          vec_madd( col_z, vec_splat( v4.m_V, VZ ),
                          vec_mul ( col_w, vec_splat( v4.m_V, VW ) ) ) );
    #elif defined( _VMX128_ )
    __vector4 row_x, row_y, row_z, row_w;
    vmx_transpose_matrix43( m43.m_ColX, m43.m_ColY, m43.m_ColZ, &row_x, &row_y, &row_z, &row_w );
    result.m_V = vmx_make_vector4( __vmsum4fp( row_x, v4.m_V ),
                                   __vmsum4fp( row_y, v4.m_V ),
                                   __vmsum4fp( row_z, v4.m_V ),
                                   __vmsum4fp( row_w, v4.m_V ) );
    #elif defined( _SSE_ )
    const __m128 col_w = Const< __m128 >::UnitW();
    __m128 col_x, col_y, col_z;
    sse_transpose_matrix43( m43.m_RowX, m43.m_RowY, m43.m_RowZ, m43.m_RowW, &col_x, &col_y, &col_z );
    result.m_V = _mm_add_ps( _mm_add_ps( _mm_mul_ps( col_x, sse_splat<VX>( v4.m_V ) ),
                                         _mm_mul_ps( col_y, sse_splat<VY>( v4.m_V ) ) ),
                             _mm_add_ps( _mm_mul_ps( col_z, sse_splat<VZ>( v4.m_V ) ),
                                         _mm_mul_ps( col_w, sse_splat<VW>( v4.m_V ) ) ) );
    #else
    for ( S32 r=0; r<3; r++ )
    {
        result.m_V[r] = m43.m[r][0] * v4.m_V[0] +
                        m43.m[r][1] * v4.m_V[1] +
                        m43.m[r][2] * v4.m_V[2];
    }
    result.m_V[3] = m43.m[3][0] * v4.m_V[0] +
                    m43.m[3][1] * v4.m_V[1] +
                    m43.m[3][2] * v4.m_V[2] +
                                  v4.m_V[3];
    #endif
    return result;
}


/// Vector3 Matrix Multiplication: Vector3 * Mat43
NOALIAS __forceinline const Vector3 operator * ( const Vector3 PCREF v, const Mat43 MATREF m )
{
    /// @code
    ///               | m00 m01 m02 |   [ x.m00 + y.m10 + z.m20 + 0,
    /// [ x y z 0 ] * | m10 m11 m12 | =   x.m01 + y.m11 + z.m21 + 0,
    ///               | m20 m21 m22 |     x.m02 + y.m12 + z.m22 + 0 ]
    ///               | m30 m31 m32 |
    /// @endcode
    Vector3 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_multiply_vector3_matrix43( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_multiply_vector3_matrix43( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    result.m_V.m_V = vmx_multiply_vector3_matrix43( v.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_multiply_vector3_matrix43( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #else
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V.m_V[c] = v.m_V.m_V[0] * m.m[0][c] +
                            v.m_V.m_V[1] * m.m[1][c] +
                            v.m_V.m_V[2] * m.m[2][c];
    }
    #endif
    return result;
}

/// Vector3 Matrix Multiplication: Mat43 * Vector3             
NOALIAS __forceinline const Vector4 operator * ( const Mat43 MATREF m, const Vector3 PCREF v )
{
    /// @code
    /// | m00 m01 m02 |   |x|   | m00.x + m01.y + m02.z |
    /// | m10 m11 m12 | * |x| = | m10.x + m11.y + m12.z |
    /// | m20 m21 m22 |   |z|   | m20.x + m21.y + m22.z |
    /// | m30 m31 m32 |         | m30.x + m31.y + m32.z |
    /// @endcode
    Vector4 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_multiply_matrix43_vector3( m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW, v.m_V.m_V );
    #elif defined( _PPU_ )                                     
    result.m_V.m_V = vec_multiply_matrix43_vector3( m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW, v.m_V.m_V );
    #elif defined( _VMX128_ )                                  
    result.m_V.m_V = vmx_multiply_matrix43_vector3( m.m_ColX, m.m_ColY, m.m_ColZ, v.m_V.m_V );
    #elif defined( _SSE_ )                                     
    result.m_V.m_V = sse_multiply_matrix43_vector3( m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW, v.m_V.m_V );
    #else
    for ( S32 r=0; r<4; r++ )
    {
        result.m_V.m_V[r] = m.m[r][0] * v.m_V.m_V[0] +
                            m.m[r][1] * v.m_V.m_V[1] +
                            m.m[r][2] * v.m_V.m_V[2];
    }
    #endif
    return result;
}
                                                               

/// Vector3 Matrix Multiplication Update: Vector3 *= Mat43
NOALIAS __forceinline void operator *= ( Vector3& v, const Mat43 MATREF m )
{
    #if defined( _SPU_ )
    v.m_V.m_V = spu_multiply_vector3_matrix43( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    v.m_V.m_V = vec_multiply_vector3_matrix43( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    v.m_V.m_V = vmx_multiply_vector3_matrix43( v.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    v.m_V.m_V = sse_multiply_vector3_matrix43( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #else
    Vector3 result;
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V.m_V[c] = v.m_V.m_V[0] * m.m[0][c] +
                            v.m_V.m_V[1] * m.m[1][c] +
                            v.m_V.m_V[2] * m.m[2][c];
    }
    v = result;
    #endif
}


/// Point3 Matrix Multiplication: Point3 * Mat43
NOALIAS __forceinline const Point3 operator * ( const Point3 PCREF p, const Mat43 MATREF m )
{
    Point3 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_multiply_point3_matrix43( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_multiply_point3_matrix43( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #elif defined( _VMX128_ )
    result.m_V.m_V = vmx_multiply_point3_matrix43( p.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_multiply_point3_matrix43( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #else
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V.m_V[c] = p.m_V.m_V[0] * m.m[0][c] +
                            p.m_V.m_V[1] * m.m[1][c] +
                            p.m_V.m_V[2] * m.m[2][c] +
                                           m.m[3][c];
    }
    #endif
    return result;
}

/// Point3 Matrix Multiplication Update: Point3 *= Mat43
NOALIAS __forceinline void operator *= ( Point3& p, const Mat43 MATREF m )
{
    #if defined( _SPU_ )
    p.m_V.m_V = spu_multiply_point3_matrix43( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #elif defined( _PPU_ )
    p.m_V.m_V = vec_multiply_point3_matrix43( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #elif defined( _VMX128_ )
    p.m_V.m_V = vmx_multiply_point3_matrix43( p.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    p.m_V.m_V = sse_multiply_point3_matrix43( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #else
    Point3 result;
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V.m_V[c] = p.m_V.m_V[0] * m.m[0][c] +
                            p.m_V.m_V[1] * m.m[1][c] +
                            p.m_V.m_V[2] * m.m[2][c] +
                                           m.m[3][c];
    }
    p = result;
    #endif
}


/// Mat43 Multiplication operator : NOTE Mat43 has implicit column W (0,0,0,1)
NOALIAS __forceinline const Mat43 operator * ( const Mat43 MATREF lhs, const Mat43 MATREF rhs )
{
    Mat43 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_madd( spu_splat<VX>( lhs.m_RowX ), rhs.m_RowX,
                    spu_madd( spu_splat<VY>( lhs.m_RowX ), rhs.m_RowY,
                    spu_mul ( spu_splat<VZ>( lhs.m_RowX ), rhs.m_RowZ ) ) );
    result.m_RowY = spu_madd( spu_splat<VX>( lhs.m_RowY ), rhs.m_RowX,
                    spu_madd( spu_splat<VY>( lhs.m_RowY ), rhs.m_RowY,
                    spu_mul ( spu_splat<VZ>( lhs.m_RowY ), rhs.m_RowZ ) ) );
    result.m_RowZ = spu_madd( spu_splat<VX>( lhs.m_RowZ ), rhs.m_RowX,
                    spu_madd( spu_splat<VY>( lhs.m_RowZ ), rhs.m_RowY,
                    spu_mul ( spu_splat<VZ>( lhs.m_RowZ ), rhs.m_RowZ ) ) );
    result.m_RowW = spu_madd( spu_splat<VX>( lhs.m_RowW ), rhs.m_RowX,
                    spu_madd( spu_splat<VY>( lhs.m_RowW ), rhs.m_RowY,
                    spu_madd( spu_splat<VZ>( lhs.m_RowW ), rhs.m_RowZ,
                                                           rhs.m_RowW ) ) );
    #elif defined( _PPU_ )
    result.m_RowX = vec_madd( vec_splat( lhs.m_RowX, VX ), rhs.m_RowX,
                    vec_madd( vec_splat( lhs.m_RowX, VY ), rhs.m_RowY,
                    vec_mul ( vec_splat( lhs.m_RowX, VZ ), rhs.m_RowZ ) ) );
    result.m_RowY = vec_madd( vec_splat( lhs.m_RowY, VX ), rhs.m_RowX,
                    vec_madd( vec_splat( lhs.m_RowY, VY ), rhs.m_RowY,
                    vec_mul ( vec_splat( lhs.m_RowY, VZ ), rhs.m_RowZ ) ) );
    result.m_RowZ = vec_madd( vec_splat( lhs.m_RowZ, VX ), rhs.m_RowX,
                    vec_madd( vec_splat( lhs.m_RowZ, VY ), rhs.m_RowY,
                    vec_mul ( vec_splat( lhs.m_RowZ, VZ ), rhs.m_RowZ ) ) );
    result.m_RowW = vec_madd( vec_splat( lhs.m_RowW, VX ), rhs.m_RowX,
                    vec_madd( vec_splat( lhs.m_RowW, VY ), rhs.m_RowY,
                    vec_madd( vec_splat( lhs.m_RowW, VZ ), rhs.m_RowZ,
                                                           rhs.m_RowW ) ) );
    #elif defined( _VMX128_ )
    const __vector4 unitW( Const< __vector4 >::UnitW() );
    __vector4 lhs_row_x, lhs_row_y, lhs_row_z, lhs_row_w;
    vmx_transpose_matrix44( lhs.m_ColX, lhs.m_ColY, lhs.m_ColZ, unitW, &lhs_row_x, &lhs_row_y, &lhs_row_z, &lhs_row_w );
    const V128 result_cx_x( __vmsum4fp( lhs_row_x, rhs.m_ColX ) );
    const V128 result_cx_y( __vmsum4fp( lhs_row_y, rhs.m_ColX ) );
    const V128 result_cx_z( __vmsum4fp( lhs_row_z, rhs.m_ColX ) );
    const V128 result_cx_w( __vmsum4fp( lhs_row_w, rhs.m_ColX ) );
    const V128 result_cy_x( __vmsum4fp( lhs_row_x, rhs.m_ColY ) );
    const V128 result_cy_y( __vmsum4fp( lhs_row_y, rhs.m_ColY ) );
    const V128 result_cy_z( __vmsum4fp( lhs_row_z, rhs.m_ColY ) );
    const V128 result_cy_w( __vmsum4fp( lhs_row_w, rhs.m_ColY ) );
    const V128 result_cz_x( __vmsum4fp( lhs_row_x, rhs.m_ColZ ) );
    const V128 result_cz_y( __vmsum4fp( lhs_row_y, rhs.m_ColZ ) );
    const V128 result_cz_z( __vmsum4fp( lhs_row_z, rhs.m_ColZ ) );
    const V128 result_cz_w( __vmsum4fp( lhs_row_w, rhs.m_ColZ ) );
    result.m_ColX = vmx_make_vector4( result_cx_x, result_cx_y, result_cx_z, result_cx_w );
    result.m_ColY = vmx_make_vector4( result_cy_x, result_cy_y, result_cy_z, result_cy_w );
    result.m_ColZ = vmx_make_vector4( result_cz_x, result_cz_y, result_cz_z, result_cz_w );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( lhs.m_RowX ), rhs.m_RowX ),
                                            _mm_mul_ps( sse_splat<VY>( lhs.m_RowX ), rhs.m_RowY ) ),
                                            _mm_mul_ps( sse_splat<VZ>( lhs.m_RowX ), rhs.m_RowZ ) );
    result.m_RowY = _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( lhs.m_RowY ), rhs.m_RowX ),
                                            _mm_mul_ps( sse_splat<VY>( lhs.m_RowY ), rhs.m_RowY ) ),
                                            _mm_mul_ps( sse_splat<VZ>( lhs.m_RowY ), rhs.m_RowZ ) );
    result.m_RowZ = _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( lhs.m_RowZ ), rhs.m_RowX ),
                                            _mm_mul_ps( sse_splat<VY>( lhs.m_RowZ ), rhs.m_RowY ) ),
                                            _mm_mul_ps( sse_splat<VZ>( lhs.m_RowZ ), rhs.m_RowZ ) );
    result.m_RowW = _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( lhs.m_RowW ), rhs.m_RowX ),
                                            _mm_mul_ps( sse_splat<VY>( lhs.m_RowW ), rhs.m_RowY ) ),
                                _mm_add_ps( _mm_mul_ps( sse_splat<VZ>( lhs.m_RowW ), rhs.m_RowZ ),
                                                                                     rhs.m_RowW ) );
    #else
    for ( S32 r=0; r<3; r++ )
    {
        for ( S32 c=0; c<3; c++ )
        {
            result.m[r][c] = lhs.m[r][0] * rhs.m[0][c] +
                             lhs.m[r][1] * rhs.m[1][c] +
                             lhs.m[r][2] * rhs.m[2][c];
        }
    }
    for ( S32 c=0; c<3; c++ )
    {
        result.m[3][c] = lhs.m[3][0] * rhs.m[0][c] +
                         lhs.m[3][1] * rhs.m[1][c] +
                         lhs.m[3][2] * rhs.m[2][c] +
                                       rhs.m[3][c];
    }
    #endif
    return result;
}


/// Mat43 * Mat33 Multiplication operator
NOALIAS __forceinline const Mat43 operator * ( const Mat43 MATREF lhs, const Mat33 MATREF rhs )
{
    return lhs * MakeMat43( rhs );
}

/// Mat33 * Mat43 Multiplication operator
NOALIAS __forceinline const Mat43 operator * ( const Mat33 MATREF lhs, const Mat43 MATREF rhs )
{
    return MakeMat43( lhs ) * rhs;
}


/// Normalise the rotation axes of a Mat43 : returns normalised result (NOTE: Does NOT orthogonalise the axes!)
NOALIAS __forceinline const Mat43 Normalise( const Mat43 MATREF src )
{
    Mat43 result;
    Vec4F32 row_x, row_y, row_z, row_w;
    GetRows( src, &row_x, &row_y, &row_z, &row_w );
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt( length_squared_xyz ) );
    SetRows( &result, row_x * X( rcp_length_xyz ),
                      row_y * Y( rcp_length_xyz ),
                      row_z * Z( rcp_length_xyz ),
                      row_w );
    return result;
}

/// Normalise the rotation axes of a Mat43 (NOTE: Does NOT orthogonalise the axes!)
NOALIAS __forceinline void Normalise( Mat43* pDst )
{
    Vec4F32 row_x, row_y, row_z, row_w;
    GetRows( *pDst, &row_x, &row_y, &row_z, &row_w );
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt( length_squared_xyz ) );
    SetRows( pDst, row_x * X( rcp_length_xyz ),
                   row_y * Y( rcp_length_xyz ),
                   row_z * Z( rcp_length_xyz ),
                   row_w );
}

// Cross products for deriving third matrix axis from 2 input axes
// ax = ay ^ az;
// ay = az ^ ax;
// az = ax ^ ay;

/// Ortho-Normalise the rotation axes of a Mat43 : returns normalised result (priority given to z axis then y axis with x axis fully recomputed) 
NOALIAS __forceinline const Mat43 OrthoNormalise( const Mat43 MATREF src )
{
    Mat43 result;
    Vec4F32 row_x, row_y, row_z, row_w;
    GetRows( src, &row_x, &row_y, &row_z, &row_w );
    row_x = row_y ^ row_z;
    row_y = row_z ^ row_x;
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt( length_squared_xyz ) );
    const VF32 rcp_length_x( X( rcp_length_xyz ) );
    const VF32 rcp_length_y( Y( rcp_length_xyz ) );
    const VF32 rcp_length_z( Z( rcp_length_xyz ) );
    SetRows( &result, row_x * rcp_length_x,
                      row_y * rcp_length_y,
                      row_z * rcp_length_z,
                      row_w );
    return result;
}

/// Ortho-Normalise the rotation axes of a Mat43 (priority given to z axis then y axis with x axis fully recomputed) 
NOALIAS __forceinline void OrthoNormalise( Mat43* pDst )
{
    Vec4F32 row_x, row_y, row_z, row_w;
    GetRows( *pDst, &row_x, &row_y, &row_z, &row_w );
    row_x = row_y ^ row_z;
    row_y = row_z ^ row_x;
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt( length_squared_xyz ) );
    const VF32 rcp_length_x( X( rcp_length_xyz ) );
    const VF32 rcp_length_y( Y( rcp_length_xyz ) );
    const VF32 rcp_length_z( Z( rcp_length_xyz ) );
    SetRows( pDst, row_x * rcp_length_x,
                   row_y * rcp_length_y,
                   row_z * rcp_length_z,
                   row_w );
}


/// Determinant of Mat43
NOALIAS __forceinline const VF32 Determinant( const Mat43 MATREF src )
{
    // The determinant of a Mat43 [with implicit col W (0,0,0,1)] is the same
    // as the determinant of it's upper-left Mat33 sub-matrix.
    return Determinant( MakeMat33( src ) );
}






/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


