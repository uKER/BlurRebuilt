#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT33_INL
#define SOFT_MATHS_HVEC_NEW_MAT33_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT33_INL
#define MATHS_HVEC_NEW_MAT33_INL
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
template <> NOALIAS __forceinline const Mat33 Load( F32 const * const __restrict pSrc )
{
    Mat33 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_load_unaligned( &pSrc[0] );
    result.m_RowY = spu_load_unaligned( &pSrc[3] );
    result.m_RowZ = spu_load_unaligned( &pSrc[6] );
    #elif defined( _PPU_ )
    result.m_RowX = vec_load_unaligned( &pSrc[0] );
    result.m_RowY = vec_load_unaligned( &pSrc[3] );
    result.m_RowZ = vec_load_unaligned( &pSrc[6] );
    #elif defined( _VMX128_ )
    result.m_ColX = vmx_load_unaligned( &pSrc[0] );
    result.m_ColY = vmx_load_unaligned( &pSrc[3] );
    result.m_ColZ = vmx_load_unaligned( &pSrc[6] );
    #elif defined( _SSE_ )
    result.m_RowX = sse_load_unaligned( &pSrc[0] );
    result.m_RowY = sse_load_unaligned( &pSrc[3] );
    result.m_RowZ = sse_load_unaligned( &pSrc[6] );
    #else
    result.m[0][0] = pSrc[0]; result.m[0][1] = pSrc[1]; result.m[0][2] = pSrc[2];
    result.m[1][0] = pSrc[3]; result.m[1][1] = pSrc[4]; result.m[1][2] = pSrc[5];
    result.m[2][0] = pSrc[6]; result.m[2][1] = pSrc[7]; result.m[2][2] = pSrc[8];
    #endif
    return result;
}

/// Store Mat33 into an F32 array
/// NOTE: Matrix transposition is platform dependent!
NOALIAS __forceinline void Store( const Mat33 MATREF src, F32* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec3( src.m_RowX, &pDst[0] );
    spu_store_unaligned_vec3( src.m_RowY, &pDst[3] );
    spu_store_unaligned_vec3( src.m_RowZ, &pDst[6] );
    #elif defined( _PPU_ )
    vec_store_unaligned_vec3( src.m_RowX, &pDst[0] );
    vec_store_unaligned_vec3( src.m_RowY, &pDst[3] );
    vec_store_unaligned_vec3( src.m_RowZ, &pDst[6] );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec3( src.m_ColX, &pDst[0] );
    vmx_store_unaligned_vec3( src.m_ColY, &pDst[3] );
    vmx_store_unaligned_vec3( src.m_ColZ, &pDst[6] );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec3( src.m_RowX, &pDst[0] );
    sse_store_unaligned_vec3( src.m_RowY, &pDst[3] );
    sse_store_unaligned_vec3( src.m_RowZ, &pDst[6] );
    #else
    pDst[0] = src.m[0][0]; pDst[1] = src.m[0][1]; pDst[2] = src.m[0][2];
    pDst[3] = src.m[1][0]; pDst[4] = src.m[1][1]; pDst[5] = src.m[1][2];
    pDst[6] = src.m[2][0]; pDst[7] = src.m[2][1]; pDst[8] = src.m[2][2];
    #endif
}


/// Load Mat33 from a Float33
/// NOTE: Matrix transposition is platform dependent!
template <> NOALIAS __forceinline const Mat33 Load( MathsStorage::Float33 const * const __restrict pSrc )
{
    Mat33 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_load_unaligned( &pSrc->rx.x );
    result.m_RowY = spu_load_unaligned( &pSrc->ry.x );
    result.m_RowZ = spu_load_unaligned( &pSrc->rz.x );
    #elif defined( _PPU_ )
    result.m_RowX = vec_load_unaligned( &pSrc->rx.x );
    result.m_RowY = vec_load_unaligned( &pSrc->ry.x );
    result.m_RowZ = vec_load_unaligned( &pSrc->rz.x );
    #elif defined( _VMX128_ )
    result.m_ColX = vmx_load_unaligned( &pSrc->cx );
    result.m_ColY = vmx_load_unaligned( &pSrc->cy );
    result.m_ColZ = vmx_load_unaligned( &pSrc->cz );
    #elif defined( _SSE_ )
    result.m_RowX = sse_load_unaligned( &pSrc->rx.x );
    result.m_RowY = sse_load_unaligned( &pSrc->ry.x );
    result.m_RowZ = sse_load_unaligned( &pSrc->rz.x );
    #else
    result.m[0][0] = pSrc->rx.x; result.m[0][1] = pSrc->rx.y; result.m[0][2] = pSrc->rx.z;
    result.m[1][0] = pSrc->ry.x; result.m[1][1] = pSrc->ry.y; result.m[1][2] = pSrc->ry.z;
    result.m[2][0] = pSrc->rz.x; result.m[2][1] = pSrc->rz.y; result.m[2][2] = pSrc->rz.z;
    #endif
    return result;
}

/// Store Mat33 into a Float33
/// NOTE: Matrix transposition is platform dependent!
NOALIAS __forceinline void Store( const Mat33 MATREF src, MathsStorage::Float33* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec3( src.m_RowX, &pDst->rx.x );
    spu_store_unaligned_vec3( src.m_RowY, &pDst->ry.x );
    spu_store_unaligned_vec3( src.m_RowZ, &pDst->rz.x );
    #elif defined( _PPU_ )
    vec_store_unaligned_vec3( src.m_RowX, &pDst->rx );
    vec_store_unaligned_vec3( src.m_RowY, &pDst->ry );
    vec_store_unaligned_vec3( src.m_RowZ, &pDst->rz );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec3( src.m_ColX, &pDst->cx );
    vmx_store_unaligned_vec3( src.m_ColY, &pDst->cy );
    vmx_store_unaligned_vec3( src.m_ColZ, &pDst->cz );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec3( src.m_RowX, &pDst->rx.x );
    sse_store_unaligned_vec3( src.m_RowY, &pDst->ry.x );
    sse_store_unaligned_vec3( src.m_RowZ, &pDst->rz.x );
    #else
    pDst->rx.x = src.m[0][0]; pDst->rx.y = src.m[0][1]; pDst->rx.z = src.m[0][2];
    pDst->ry.x = src.m[1][0]; pDst->ry.y = src.m[1][1]; pDst->ry.z = src.m[1][2];
    pDst->rz.x = src.m[2][0]; pDst->rz.y = src.m[2][1]; pDst->rz.z = src.m[2][2];
    #endif
}


/// Load Mat33 from a MatrixRow33
template <> NOALIAS __forceinline const Mat33 Load( MathsStorage::MatrixRow33 const * const __restrict pSrc )
{
    Mat33 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_load_unaligned( &pSrc->rx.x );
    result.m_RowY = spu_load_unaligned( &pSrc->ry.x );
    result.m_RowZ = spu_load_unaligned( &pSrc->rz.x );
    #elif defined( _PPU_ )
    result.m_RowX = vec_load_unaligned( &pSrc->rx.x );
    result.m_RowY = vec_load_unaligned( &pSrc->ry.x );
    result.m_RowZ = vec_load_unaligned( &pSrc->rz.x );
    #elif defined( _VMX128_ )
    const __vector4 rowX( vmx_load_unaligned( &pSrc->rx ) );
    const __vector4 rowY( vmx_load_unaligned( &pSrc->ry ) );
    const __vector4 rowZ( vmx_load_unaligned( &pSrc->rz ) );
    vmx_transpose_matrix33( rowX, rowY, rowZ, &result.m_ColX, &result.m_ColY, &result.m_ColZ );
    #elif defined( _SSE_ )
    result.m_RowX = sse_load_unaligned( &pSrc->rx.x );
    result.m_RowY = sse_load_unaligned( &pSrc->ry.x );
    result.m_RowZ = sse_load_unaligned( &pSrc->rz.x );
    #else
    result.m[0][0] = pSrc->v[0]; result.m[0][1] = pSrc->v[1]; result.m[0][2] = pSrc->v[2];
    result.m[1][0] = pSrc->v[3]; result.m[1][1] = pSrc->v[4]; result.m[1][2] = pSrc->v[5];
    result.m[2][0] = pSrc->v[6]; result.m[2][1] = pSrc->v[7]; result.m[2][2] = pSrc->v[8];
    #endif
    return result;
}

/// Store Mat33 into a MatrixRow33
NOALIAS __forceinline void Store( const Mat33 MATREF src, MathsStorage::MatrixRow33* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec3( src.m_RowX, &pDst->rx.x );
    spu_store_unaligned_vec3( src.m_RowY, &pDst->ry.x );
    spu_store_unaligned_vec3( src.m_RowZ, &pDst->rz.x );
    #elif defined( _PPU_ )
    vec_store_unaligned_vec3( src.m_RowX, &pDst->rx );
    vec_store_unaligned_vec3( src.m_RowY, &pDst->ry );
    vec_store_unaligned_vec3( src.m_RowZ, &pDst->rz );
    #elif defined( _VMX128_ )
    __vector4 rowX, rowY, rowZ;
    vmx_transpose_matrix33( src.m_ColX, src.m_ColY, src.m_ColZ, &rowX, &rowY, &rowZ );
    vmx_store_unaligned_vec3( rowX, &pDst->rx );
    vmx_store_unaligned_vec3( rowY, &pDst->ry );
    vmx_store_unaligned_vec3( rowZ, &pDst->rz );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec3( src.m_RowX, &pDst->rx.x );
    sse_store_unaligned_vec3( src.m_RowY, &pDst->ry.x );
    sse_store_unaligned_vec3( src.m_RowZ, &pDst->rz.x );
    #else
    pDst->v[0] = src.m[0][0]; pDst->v[1] = src.m[0][1]; pDst->v[2] = src.m[0][2];
    pDst->v[3] = src.m[1][0]; pDst->v[4] = src.m[1][1]; pDst->v[5] = src.m[1][2];
    pDst->v[6] = src.m[2][0]; pDst->v[7] = src.m[2][1]; pDst->v[8] = src.m[2][2];
    #endif
}


/// Load Mat33 from a MatrixCol33
template <> NOALIAS __forceinline const Mat33 Load( MathsStorage::MatrixCol33 const * const __restrict pSrc )
{
    Mat33 result;
    #if defined( _SPU_ )
    const vector float colX( spu_load_unaligned( &pSrc->cx.x ) );
    const vector float colY( spu_load_unaligned( &pSrc->cy.x ) );
    const vector float colZ( spu_load_unaligned( &pSrc->cz.x ) );
    spu_transpose_matrix33( colX, colY, colZ, &result.m_RowX, &result.m_RowY, &result.m_RowZ );
    #elif defined( _PPU_ )
    const vector float colX( vec_load_unaligned( &pSrc->cx.x ) );
    const vector float colY( vec_load_unaligned( &pSrc->cy.x ) );
    const vector float colZ( vec_load_unaligned( &pSrc->cz.x ) );
    vec_transpose_matrix33( colX, colY, colZ, &result.m_RowX, &result.m_RowY, &result.m_RowZ );
    #elif defined( _VMX128_ )
    result.m_ColX = vmx_load_unaligned( &pSrc->cx );
    result.m_ColY = vmx_load_unaligned( &pSrc->cy );
    result.m_ColZ = vmx_load_unaligned( &pSrc->cz );
    #elif defined( _SSE_ )
    const __m128 colX( sse_load_unaligned( &pSrc->cx.x ) );
    const __m128 colY( sse_load_unaligned( &pSrc->cy.x ) );
    const __m128 colZ( sse_load_unaligned( &pSrc->cz.x ) );
    sse_transpose_matrix33( colX, colY, colZ, &result.m_RowX, &result.m_RowY, &result.m_RowZ );
    #else
    result.m[0][0] = pSrc->v[0]; result.m[0][1] = pSrc->v[3]; result.m[0][2] = pSrc->v[6];
    result.m[1][0] = pSrc->v[1]; result.m[1][1] = pSrc->v[4]; result.m[1][2] = pSrc->v[7];
    result.m[2][0] = pSrc->v[2]; result.m[2][1] = pSrc->v[5]; result.m[2][2] = pSrc->v[8];
    #endif
    return result;
}

/// Store Mat33 into a MatrixCol33
NOALIAS __forceinline void Store( const Mat33 MATREF src, MathsStorage::MatrixCol33* const __restrict pDst )
{
    #if defined( _SPU_ )
    vector float colX, colY, colZ;
    spu_transpose_matrix33( src.m_RowX, src.m_RowY, src.m_RowZ, &colX, &colY, &colZ );
    spu_store_unaligned_vec3( colX, &pDst->cx.x );
    spu_store_unaligned_vec3( colY, &pDst->cy.x );
    spu_store_unaligned_vec3( colZ, &pDst->cz.x );
    #elif defined( _PPU_ )
    vector float colX, colY, colZ;
    vec_transpose_matrix33( src.m_RowX, src.m_RowY, src.m_RowZ, &colX, &colY, &colZ );
    vec_store_unaligned_vec3( colX, &pDst->cx );
    vec_store_unaligned_vec3( colY, &pDst->cy );
    vec_store_unaligned_vec3( colZ, &pDst->cz );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec3( src.m_ColX, &pDst->cx );
    vmx_store_unaligned_vec3( src.m_ColY, &pDst->cy );
    vmx_store_unaligned_vec3( src.m_ColZ, &pDst->cz );
    #elif defined( _SSE_ )
    __m128 colX, colY, colZ;
    sse_transpose_matrix33( src.m_RowX, src.m_RowY, src.m_RowZ, &colX, &colY, &colZ );
    sse_store_unaligned_vec3( colX, &pDst->cx.x );
    sse_store_unaligned_vec3( colY, &pDst->cy.x );
    sse_store_unaligned_vec3( colZ, &pDst->cz.x );
    #else
    pDst->v[0] = src.m[0][0]; pDst->v[1] = src.m[1][0]; pDst->v[2] = src.m[2][0];
    pDst->v[3] = src.m[0][1]; pDst->v[4] = src.m[1][1]; pDst->v[5] = src.m[2][1];
    pDst->v[6] = src.m[0][2]; pDst->v[7] = src.m[1][2]; pDst->v[8] = src.m[2][2];
    #endif
}


NOALIAS __forceinline const Mat33& Const< Mat33 >::Zero()     { return m_Zero; }
NOALIAS __forceinline const Mat33& Const< Mat33 >::Identity() { return m_Identity; }

NOALIAS __forceinline const Mat33& Const< Mat33 >::RotateX90CW() { return m_RotateX90CW; };
NOALIAS __forceinline const Mat33& Const< Mat33 >::RotateY90CW() { return m_RotateY90CW; };
NOALIAS __forceinline const Mat33& Const< Mat33 >::RotateZ90CW() { return m_RotateZ90CW; };

NOALIAS __forceinline const Mat33& Const< Mat33 >::RotateX90AntiCW() { return m_RotateX90ACW; };
NOALIAS __forceinline const Mat33& Const< Mat33 >::RotateY90AntiCW() { return m_RotateY90ACW; };
NOALIAS __forceinline const Mat33& Const< Mat33 >::RotateZ90AntiCW() { return m_RotateZ90ACW; };

NOALIAS __forceinline const Mat33& Const< Mat33 >::RotateX180() { return m_RotateX180; };
NOALIAS __forceinline const Mat33& Const< Mat33 >::RotateY180() { return m_RotateY180; };
NOALIAS __forceinline const Mat33& Const< Mat33 >::RotateZ180() { return m_RotateZ180; };


/// Make Mat33 from 9 float elements (row major)
NOALIAS __forceinline const Mat33 MakeMat33( const F32& m00, const F32& m01, const F32& m02,
                                             const F32& m10, const F32& m11, const F32& m12,
                                             const F32& m20, const F32& m21, const F32& m22 )
{
    #if defined( _SIMD_ )
    #if defined( _VMX128_ )
    const F32 m[9] = { m00, m10, m20,
                       m01, m11, m21,
                       m02, m12, m22 };
    #else
    const F32 m[9] = { m00, m01, m02,
                       m10, m11, m12,
                       m20, m21, m22 };
    #endif
    return Load<Mat33>( &m[0] );
    #else
    Mat33 result;
    result.m[0][0] = m00; result.m[0][1] = m01; result.m[0][2] = m02;
    result.m[1][0] = m10; result.m[1][1] = m11; result.m[1][2] = m12;
    result.m[2][0] = m20; result.m[2][1] = m21; result.m[2][2] = m22;
    return result;
    #endif
}


/// Make Mat33 from 3 row vectors (3 element)
NOALIAS __forceinline const Mat33 MakeMat33( const Vec4F32 VECREF rowX, const Vec4F32 VECREF rowY, const Vec4F32 VECREF rowZ )
{
    Mat33 result;
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    result.m_RowX = rowX.m_V;
    result.m_RowY = rowY.m_V;
    result.m_RowZ = rowZ.m_V;
    #elif defined( _VMX128_ )
    vmx_transpose_matrix33( rowX.m_V, rowY.m_V, rowZ.m_V, &result.m_ColX, &result.m_ColY, &result.m_ColZ );
    #else
    result.m[0][0] = rowX.m_V[0]; result.m[0][1] = rowX.m_V[1]; result.m[0][2] = rowX.m_V[2];
    result.m[1][0] = rowY.m_V[0]; result.m[1][1] = rowY.m_V[1]; result.m[1][2] = rowY.m_V[2];
    result.m[2][0] = rowZ.m_V[0]; result.m[2][1] = rowZ.m_V[1]; result.m[2][2] = rowZ.m_V[2];
    #endif
    return result;
}


/// Make Mat33 from 3 row vectors (3 element)
NOALIAS __forceinline const Mat33 MakeMat33( const Vector3 PCREF rowX, const Vector3 PCREF rowY, const Vector3 PCREF rowZ )
{
    return MakeMat33( rowX.m_V, rowY.m_V, rowZ.m_V );
}

/// Make Mat33 from 3 row normals (3 element)
NOALIAS __forceinline const Mat33 MakeMat33( const Normal3 PCREF rowX, const Normal3 PCREF rowY, const Normal3 PCREF rowZ )
{
    return MakeMat33( rowX.m_V, rowY.m_V, rowZ.m_V );
}


/// Make Mat33 from a Mat43
NOALIAS __forceinline const Mat33 MakeMat33( const Mat43 MATREF src )
{
    Mat33 result;
    #if defined( _SPU_ )
    result.m_RowX = src.m_RowX;
    result.m_RowY = src.m_RowY;
    result.m_RowZ = src.m_RowZ;
    #elif defined( _PPU_ )
    result.m_RowX = src.m_RowX;
    result.m_RowY = src.m_RowY;
    result.m_RowZ = src.m_RowZ;
    #elif defined( _VMX128_ )
    result.m_ColX = src.m_ColX;
    result.m_ColY = src.m_ColY;
    result.m_ColZ = src.m_ColZ;
    #elif defined( _SSE_ )
    result.m_RowX = src.m_RowX;
    result.m_RowY = src.m_RowY;
    result.m_RowZ = src.m_RowZ;
    #else
    result.m[0][0] = src.m[0][0]; result.m[0][1] = src.m[0][1]; result.m[0][2] = src.m[0][2];
    result.m[1][0] = src.m[1][0]; result.m[1][1] = src.m[1][1]; result.m[1][2] = src.m[1][2];
    result.m[2][0] = src.m[2][0]; result.m[2][1] = src.m[2][1]; result.m[2][2] = src.m[2][2];
    #endif
    return result;
}

/// Make Mat33 from a Mat44
NOALIAS __forceinline const Mat33 MakeMat33( const Mat44 MATREF src )
{
    Mat33 result;
    #if defined( _SPU_ )
    result.m_RowX = src.m_RowX;
    result.m_RowY = src.m_RowY;
    result.m_RowZ = src.m_RowZ;
    #elif defined( _PPU_ )
    result.m_RowX = src.m_RowX;
    result.m_RowY = src.m_RowY;
    result.m_RowZ = src.m_RowZ;
    #elif defined( _VMX128_ )
    result.m_ColX = src.m_ColX;
    result.m_ColY = src.m_ColY;
    result.m_ColZ = src.m_ColZ;
    #elif defined( _SSE_ )
    result.m_RowX = src.m_RowX;
    result.m_RowY = src.m_RowY;
    result.m_RowZ = src.m_RowZ;
    #else
    result.m[0][0] = src.m[0][0]; result.m[0][1] = src.m[0][1]; result.m[0][2] = src.m[0][2];
    result.m[1][0] = src.m[1][0]; result.m[1][1] = src.m[1][1]; result.m[1][2] = src.m[1][2];
    result.m[2][0] = src.m[2][0]; result.m[2][1] = src.m[2][1]; result.m[2][2] = src.m[2][2];
    #endif
    return result;
}

/// Make Mat33 from a ScaleMat3
NOALIAS __forceinline const Mat33 MakeMat33( const ScaleMat3 PCREF src )
{
    Mat33 result;
    //#if defined( _SPU_ )
    //result.m_RowX = spu_select<1,0,0,0>( Const< vector float >::Zero(), src.m_Scale.m_V.m_V );
    //result.m_RowY = spu_select<0,1,0,0>( Const< vector float >::Zero(), src.m_Scale.m_V.m_V );
    //result.m_RowZ = spu_select<0,0,1,0>( Const< vector float >::Zero(), src.m_Scale.m_V.m_V );
    //#elif defined( _PPU_ )
    //result.m_RowX = vec_select<1,0,0,0>( Const< vector float >::Zero(), src.m_Scale.m_V.m_V );
    //result.m_RowY = vec_select<0,1,0,0>( Const< vector float >::Zero(), src.m_Scale.m_V.m_V );
    //result.m_RowZ = vec_select<0,0,1,0>( Const< vector float >::Zero(), src.m_Scale.m_V.m_V );
    //#elif defined( _VMX128_ )
    //result.m_ColX = __vrlimi( Const< __vector4 >::Zero(), src.m_Scale.m_V.m_V, VMX_MASK_INSERT_X, 0 );
    //result.m_ColY = __vrlimi( Const< __vector4 >::Zero(), src.m_Scale.m_V.m_V, VMX_MASK_INSERT_Y, 0 );
    //result.m_ColZ = __vrlimi( Const< __vector4 >::Zero(), src.m_Scale.m_V.m_V, VMX_MASK_INSERT_Z, 0 );
    //#elif defined( _SSE_ )
    //result.m_RowX = sse_select<1,0,0,0>( Const< __m128 >::Zero(), src.m_Scale.m_V.m_V );
    //result.m_RowY = sse_select<0,1,0,0>( Const< __m128 >::Zero(), src.m_Scale.m_V.m_V );
    //result.m_RowZ = sse_select<0,0,1,0>( Const< __m128 >::Zero(), src.m_Scale.m_V.m_V );
    //#else
    //result.m[0][0] = src.m_Scale.m_V.m_V[0]; result.m[0][1] = 0.0f;                   result.m[0][2] = 0.0f;             
    //result.m[1][0] = 0.0f;                   result.m[1][1] = src.m_Scale.m_V.m_V[1]; result.m[1][2] = 0.0f;             
    //result.m[2][0] = 0.0f;                   result.m[2][1] = 0.0f;                   result.m[2][2] = src.m_Scale.m_V.m_V[2];
    //#endif
    const Vector3 zero( Const<Vector3>::Zero() );
    Vector3 rowX( zero );
    Vector3 rowY( zero );
    Vector3 rowZ( zero );
    SetX( &rowX, GetSX( src ) );
    SetY( &rowY, GetSY( src ) );
    SetZ( &rowZ, GetSZ( src ) );
    #if defined( _VMX128_ )
    result.m_ColX = rowX.m_V.m_V; // transpose(S) == S 
    result.m_ColY = rowY.m_V.m_V;
    result.m_ColZ = rowZ.m_V.m_V;
    #else
    SetRowX( &result, rowX );
    SetRowY( &result, rowY );
    SetRowZ( &result, rowZ );
    #endif
    return result;
}

/// Make Mat33 from a ScaleMat3
NOALIAS __forceinline const Mat33 MakeMat33( const ShearMat3 PCREF src )
{
    // Shear elements m10=XY, m20=XZ, m21=YZ
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    const VF32 xy( GetXY( src ) );
    const VF32 xz( GetXZ( src ) );
    const VF32 yz( GetYZ( src ) );
    const Vector3 rowX( Const<Vector3>::UnitX() );
    const Vector3 rowY( MakeVector3( xy, one, zero ) );
    const Vector3 rowZ( MakeVector3( xz, yz, one ) );
    return MakeMat33( rowX, rowY, rowZ );
}


/// Get rows from a Mat33
NOALIAS __forceinline void GetRows( const Mat33 MATREF src, Vec4F32* __restrict pRowX, Vec4F32* __restrict pRowY, Vec4F32* __restrict pRowZ )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    pRowX->m_V = src.m_RowX;
    pRowY->m_V = src.m_RowY;
    pRowZ->m_V = src.m_RowZ;
    #elif defined( _VMX128_ )
    vmx_transpose_matrix33( src.m_ColX, src.m_ColY, src.m_ColZ, &pRowX->m_V, &pRowY->m_V, &pRowZ->m_V );
    #else
    pRowX->m_V[0] = src.m[0][0]; pRowX->m_V[1] = src.m[0][1]; pRowX->m_V[2] = src.m[0][2];
    pRowY->m_V[0] = src.m[1][0]; pRowY->m_V[1] = src.m[1][1]; pRowY->m_V[2] = src.m[1][2];
    pRowZ->m_V[0] = src.m[2][0]; pRowZ->m_V[1] = src.m[2][1]; pRowZ->m_V[2] = src.m[2][2];
    #endif
}

/// Set rows of a Mat33
NOALIAS __forceinline void SetRows( Mat33* pDst, const Vec4F32 VECREF rowX, const Vec4F32 VECREF rowY, const Vec4F32 VECREF rowZ )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    pDst->m_RowX = rowX.m_V;
    pDst->m_RowY = rowY.m_V;
    pDst->m_RowZ = rowZ.m_V;
    #elif defined( _VMX128_ )
    vmx_transpose_matrix33( rowX.m_V, rowY.m_V, rowZ.m_V, &pDst->m_ColX, &pDst->m_ColY, &pDst->m_ColZ );
    #else
    pDst->m[0][0] = rowX.m_V[0]; pDst->m[0][1] = rowX.m_V[1]; pDst->m[0][2] = rowX.m_V[2];
    pDst->m[1][0] = rowY.m_V[0]; pDst->m[1][1] = rowY.m_V[1]; pDst->m[1][2] = rowY.m_V[2];
    pDst->m[2][0] = rowZ.m_V[0]; pDst->m[2][1] = rowZ.m_V[1]; pDst->m[2][2] = rowZ.m_V[2];
    #endif
}


/// Get rows from a Mat33
NOALIAS __forceinline void GetRows( const Mat33 MATREF src, Vector3* __restrict pRowX, Vector3* __restrict pRowY, Vector3* __restrict pRowZ )
{
    GetRows( src, &pRowX->m_V, &pRowY->m_V, &pRowZ->m_V );
}

/// Set rows of a Mat33
NOALIAS __forceinline void SetRows( Mat33* pDst, const Vector3 PCREF rowX, const Vector3 PCREF rowY, const Vector3 PCREF rowZ )
{
    SetRows( pDst, rowX.m_V, rowY.m_V, rowZ.m_V );
}

/// Set rows of a Mat33
NOALIAS __forceinline void SetRows( Mat33* pDst, const Normal3 PCREF rowX, const Normal3 PCREF rowY, const Normal3 PCREF rowZ )
{
    SetRows( pDst, rowX.m_V, rowY.m_V, rowZ.m_V );
}


/// Get columns from a Mat33
NOALIAS __forceinline void GetColumns( const Mat33 MATREF src, Vector3* __restrict pColX, Vector3* __restrict pColY, Vector3* __restrict pColZ )
{
    #if defined( _VMX128_ )
    pColX->m_V.m_V = src.m_ColX;
    pColY->m_V.m_V = src.m_ColY;
    pColZ->m_V.m_V = src.m_ColZ;
    #else
    const Mat33 src_transposed( Transpose( src ) );
    *pColX = RowX( src_transposed );
    *pColY = RowY( src_transposed );
    *pColZ = RowZ( src_transposed );
    #endif
}

/// Set columns of a Mat33
NOALIAS __forceinline void SetColumns( Mat33* pDst, const Vector3 PCREF colX, const Vector3 PCREF colY, const Vector3 PCREF colZ )
{
    #if defined( _VMX128_ )
    pDst->m_ColX = colX.m_V.m_V;
    pDst->m_ColY = colY.m_V.m_V;
    pDst->m_ColZ = colZ.m_V.m_V;
    #else
    const Mat33 dst_transposed( MakeMat33( colX, colY, colZ ) );
    *pDst = Transpose( dst_transposed );
    #endif
}

/// Set columns of a Mat33
NOALIAS __forceinline void SetColumns( Mat33* pDst, const Normal3 PCREF colX, const Normal3 PCREF colY, const Normal3 PCREF colZ )
{
    #if defined( _VMX128_ )
    pDst->m_ColX = colX.m_V.m_V;
    pDst->m_ColY = colY.m_V.m_V;
    pDst->m_ColZ = colZ.m_V.m_V;
    #else
    const Mat33 dst_transposed( MakeMat33( colX, colY, colZ ) );
    *pDst = Transpose( dst_transposed );
    #endif
}


/// Get Row X from a Mat33
NOALIAS __forceinline const Vector3 RowX( const Mat33 MATREF src )
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

/// Get Row Y from a Mat33
NOALIAS __forceinline const Vector3 RowY( const Mat33 MATREF src )
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

/// Get Row Z from a Mat33
NOALIAS __forceinline const Vector3 RowZ( const Mat33 MATREF src )
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


/// Get Col X from a Mat33
NOALIAS __forceinline const Vector3 ColX( const Mat33 MATREF src )
{
    Vector3 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_transpose_col_x( src.m_RowX, src.m_RowY, src.m_RowZ, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_transpose_col_x( src.m_RowX, src.m_RowY, src.m_RowZ, Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    result.m_V.m_V = src.m_ColX;
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_transpose_col_x( src.m_RowX, src.m_RowY, src.m_RowZ, Const< __m128 >::Zero() );
    #else
    result.m_V.m_V[VX] = src.m[0][0];
    result.m_V.m_V[VY] = src.m[1][0];
    result.m_V.m_V[VZ] = src.m[2][0];
    #endif
    return result;
}

/// Get Col Y from a Mat33
NOALIAS __forceinline const Vector3 ColY( const Mat33 MATREF src )
{
    Vector3 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_transpose_col_y( src.m_RowX, src.m_RowY, src.m_RowZ, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_transpose_col_y( src.m_RowX, src.m_RowY, src.m_RowZ, Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    result.m_V.m_V = src.m_ColY;
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_transpose_col_y( src.m_RowX, src.m_RowY, src.m_RowZ, Const< __m128 >::Zero() );
    #else
    result.m_V.m_V[VX] = src.m[0][1];
    result.m_V.m_V[VY] = src.m[1][1];
    result.m_V.m_V[VZ] = src.m[2][1];
    #endif
    return result;
}

/// Get Col Z from a Mat33
NOALIAS __forceinline const Vector3 ColZ( const Mat33 MATREF src )
{
    Vector3 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_transpose_col_z( src.m_RowX, src.m_RowY, src.m_RowZ, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_transpose_col_z( src.m_RowX, src.m_RowY, src.m_RowZ, Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    result.m_V.m_V = src.m_ColZ;
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_transpose_col_z( src.m_RowX, src.m_RowY, src.m_RowZ, Const< __m128 >::Zero() );
    #else
    result.m_V.m_V[VX] = src.m[0][2];
    result.m_V.m_V[VY] = src.m[1][2];
    result.m_V.m_V[VZ] = src.m[2][2];
    #endif
    return result;
}


/// Get element from a Mat33
template < unsigned int Row, unsigned int Col > NOALIAS __forceinline const VF32 Element( const Mat33 MATREF src )
{
    BIZ_STATIC_ASSERT( Row < 3 && Col < 3 );
    VF32 result;
    #if defined( _SPU_ )
    vector float row;
    switch ( Row )
    {
        case E_RowX : row = src.m_RowX; break;
        case E_RowY : row = src.m_RowY; break;
        case E_RowZ : row = src.m_RowZ; break;
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
        default     : result.m_V = Const< __vector4 >::Zero(); break;
    };
    #elif defined( _SSE_ )
    __m128 row;
    switch ( Row )
    {
        case E_RowX : row = src.m_RowX; break;
        case E_RowY : row = src.m_RowY; break;
        case E_RowZ : row = src.m_RowZ; break;
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


/// Set Row X of a Mat33
NOALIAS __forceinline void SetRowX( Mat33* pDst, const Vec4F32 VECREF src )
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

/// Set Row Y of a Mat33
NOALIAS __forceinline void SetRowY( Mat33* pDst, const Vec4F32 VECREF src )
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

/// Set Row Z of a Mat33
NOALIAS __forceinline void SetRowZ( Mat33* pDst, const Vec4F32 VECREF src )
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


/// Set Col X of a Mat33
NOALIAS __forceinline void SetColX( Mat33* pDst, const Vec4F32 VECREF src )
{
    #if defined( _SPU_ )
    pDst->m_RowX = spu_select<1,0,0,0>( pDst->m_RowX, src.m_V );
    pDst->m_RowY = spu_select<1,0,0,0>( pDst->m_RowY, spu_splat<VY>( src.m_V ) );
    pDst->m_RowZ = spu_select<1,0,0,0>( pDst->m_RowZ, spu_splat<VZ>( src.m_V ) );
    #elif defined( _PPU_ )
    pDst->m_RowX = vec_select<1,0,0,0>( pDst->m_RowX, src.m_V );
    pDst->m_RowY = vec_select<1,0,0,0>( pDst->m_RowY, vec_splat( src.m_V, VY ) );
    pDst->m_RowZ = vec_select<1,0,0,0>( pDst->m_RowZ, vec_splat( src.m_V, VZ ) );
    #elif defined( _VMX128_ )
    pDst->m_ColX = src.m_V;
    #elif defined( _SSE_ )
    pDst->m_RowX = sse_select<1,0,0,0>( pDst->m_RowX, src.m_V );
    pDst->m_RowY = sse_select<1,0,0,0>( pDst->m_RowY, sse_splat<VY>( src.m_V ) );
    pDst->m_RowZ = sse_select<1,0,0,0>( pDst->m_RowZ, sse_splat<VZ>( src.m_V ) );
    #else
    pDst->m[0][0] = src.m_V[VX];
    pDst->m[1][0] = src.m_V[VY];
    pDst->m[2][0] = src.m_V[VZ];
    #endif
}

/// Set Col Y of a Mat33
NOALIAS __forceinline void SetColY( Mat33* pDst, const Vec4F32 VECREF src )
{
    #if defined( _SPU_ )
    pDst->m_RowX = spu_select<0,1,0,0>( pDst->m_RowX, spu_splat<VX>( src.m_V ) );
    pDst->m_RowY = spu_select<0,1,0,0>( pDst->m_RowY, src.m_V );
    pDst->m_RowZ = spu_select<0,1,0,0>( pDst->m_RowZ, spu_splat<VZ>( src.m_V ) );
    #elif defined( _PPU_ )      
    pDst->m_RowX = vec_select<0,1,0,0>( pDst->m_RowX, vec_splat( src.m_V, VX ) );
    pDst->m_RowY = vec_select<0,1,0,0>( pDst->m_RowY, src.m_V );
    pDst->m_RowZ = vec_select<0,1,0,0>( pDst->m_RowZ, vec_splat( src.m_V, VZ ) );
    #elif defined( _VMX128_ )   
    pDst->m_ColY = src.m_V;     
    #elif defined( _SSE_ )      
    pDst->m_RowX = sse_select<0,1,0,0>( pDst->m_RowX, sse_splat<VX>( src.m_V ) );
    pDst->m_RowY = sse_select<0,1,0,0>( pDst->m_RowY, src.m_V );
    pDst->m_RowZ = sse_select<0,1,0,0>( pDst->m_RowZ, sse_splat<VZ>( src.m_V ) );
    #else
    pDst->m[0][1] = src.m_V[VX];
    pDst->m[1][1] = src.m_V[VY];
    pDst->m[2][1] = src.m_V[VZ];
    #endif
}

/// Set Col Z of a Mat33
NOALIAS __forceinline void SetColZ( Mat33* pDst, const Vec4F32 VECREF src )
{
    #if defined( _SPU_ )
    pDst->m_RowX = spu_select<0,0,1,0>( pDst->m_RowX, spu_splat<VX>( src.m_V ) );
    pDst->m_RowY = spu_select<0,0,1,0>( pDst->m_RowY, spu_splat<VY>( src.m_V ) );
    pDst->m_RowZ = spu_select<0,0,1,0>( pDst->m_RowZ, src.m_V );
    #elif defined( _PPU_ )        
    pDst->m_RowX = vec_select<0,0,1,0>( pDst->m_RowX, vec_splat( src.m_V, VX ) );
    pDst->m_RowY = vec_select<0,0,1,0>( pDst->m_RowY, vec_splat( src.m_V, VY ) );
    pDst->m_RowZ = vec_select<0,0,1,0>( pDst->m_RowZ, src.m_V );
    #elif defined( _VMX128_ )     
    pDst->m_ColZ = src.m_V;       
    #elif defined( _SSE_ )        
    pDst->m_RowX = sse_select<0,0,1,0>( pDst->m_RowX, sse_splat<VX>( src.m_V ) );
    pDst->m_RowY = sse_select<0,0,1,0>( pDst->m_RowY, sse_splat<VY>( src.m_V ) );
    pDst->m_RowZ = sse_select<0,0,1,0>( pDst->m_RowZ, src.m_V );
    #else
    pDst->m[0][2] = src.m_V[VX];
    pDst->m[1][2] = src.m_V[VY];
    pDst->m[2][2] = src.m_V[VZ];
    #endif
}


/// Set Row X of a Mat33
NOALIAS __forceinline void SetRowX( Mat33* pDst, const Vector3 PCREF src )
{
    SetRowX( pDst, src.m_V );
}

/// Set Row Y of a Mat33
NOALIAS __forceinline void SetRowY( Mat33* pDst, const Vector3 PCREF src )
{
    SetRowY( pDst, src.m_V );
}

/// Set Row Z of a Mat33
NOALIAS __forceinline void SetRowZ( Mat33* pDst, const Vector3 PCREF src )
{
    SetRowZ( pDst, src.m_V );
}


/// Set Col X of a Mat33
NOALIAS __forceinline void SetColX( Mat33* pDst, const Vector3 PCREF src )
{
    SetColX( pDst, src.m_V );
}

/// Set Col Y of a Mat33
NOALIAS __forceinline void SetColY( Mat33* pDst, const Vector3 PCREF src )
{
    SetColY( pDst, src.m_V );
}

/// Set Col Z of a Mat33
NOALIAS __forceinline void SetColZ( Mat33* pDst, const Vector3 PCREF src )
{
    SetColZ( pDst, src.m_V );
}


/// Create a Mat33 rotation matrix of alpha radians about the X axis [ clockwise when viewed back down the X axis ]
NOALIAS __forceinline const Mat33 MakeMat33_RotX( const VF32 VECREF alpha )
{
    /// @code
    /// Euler angle rotation about X axis [ clockwise when viewed back down the axis ]
    /// [ 1,  0,      0      ]
    /// [ 0,  cos(x), sin(x) ]
    /// [ 0, -sin(x), cos(x) ]
    /// @endcode
    
    Mat33 result;
    const VF32 zero( Const<VF32>::Zero() );
    #if 1
    const VF32 sinAlpha( Sin( alpha ) );
    const VF32 cosAlpha( Cos( alpha ) );
    #else
    VF32 sinAlpha, cosAlpha;
    SinCos( alpha, &sinAlpha, &cosAlpha );
    #endif
    SetRows( &result, Const<Vec4F32>::UnitX(),
                      MakeVec4F32( zero,  cosAlpha, sinAlpha ),
                      MakeVec4F32( zero, -sinAlpha, cosAlpha ) );
    return result;
}

/// Create a Mat33 rotation matrix of beta radians about the Y axis [ clockwise when viewed back down the Y axis ]
NOALIAS __forceinline const Mat33 MakeMat33_RotY( const VF32 VECREF beta )
{
    /// @code
    /// Euler angle rotation about Y axis [ clockwise when viewed back down the axis ]
    /// [ cos(y), 0, -sin(y) ]
    /// [ 0,      1,  0      ]
    /// [ sin(y), 0,  cos(y) ]
    /// @endcode
    
    Mat33 result;
    const VF32 zero( Const<VF32>::Zero() );
    #if 1
    const VF32 sinBeta( Sin( beta ) );
    const VF32 cosBeta( Cos( beta ) );
    #else
    VF32 sinBeta, cosBeta;
    SinCos( beta, &sinBeta, &cosBeta );
    #endif
    SetRows( &result, MakeVec4F32( cosBeta, zero, -sinBeta ),
                      Const<Vec4F32>::UnitY(),
                      MakeVec4F32( sinBeta, zero,  cosBeta ) );
    return result;
}

/// Create a Mat33 rotation matrix of gamma radians about the Z axis [ clockwise when viewed back down the Z axis ]
NOALIAS __forceinline const Mat33 MakeMat33_RotZ( const VF32 VECREF gamma )
{
    /// @code
    /// Euler angle rotation about Z axis [ clockwise when viewed back down the axis ]
    /// [  cos(z), sin(z), 0 ]
    /// [ -sin(z), cos(z), 0 ]
    /// [  0,      0,      1 ]
    /// @endcode
    
    Mat33 result;
    const VF32 zero( Const<VF32>::Zero() );
    #if 1
    const VF32 sinGamma( Sin( gamma ) );
    const VF32 cosGamma( Cos( gamma ) );
    #else
    VF32 sinGamma, cosGamma;
    SinCos( gamma, &sinGamma, &cosGamma );
    #endif
    SetRows( &result, MakeVec4F32(  cosGamma, sinGamma, zero ),
                      MakeVec4F32( -sinGamma, cosGamma, zero ),
                      Const<Vec4F32>::UnitZ() );
    return result;
}


/// Create a Mat33 rotation matrix of theta radians about an arbitrary axis
NOALIAS __forceinline const Mat33 MakeMat33_AxisAngle( const Normal3 PCREF axis, const VF32 VECREF theta )
{
    VF32 sn, cs;
    SinCos( theta, &sn, &cs );
    
    const VF32 one( Const<VF32>::One() );
    const VF32 oneMinusCos( one - cs );
    const VF32 axis_x( X( axis ) );
    const VF32 axis_y( Y( axis ) );
    const VF32 axis_z( Z( axis ) );
    
    #if 0
    
    return MakeMat33( MakeVector3(           cs + ( axis_x * axis_x * oneMinusCos ),
                                    axis_z * sn + ( axis_y * axis_x * oneMinusCos ),
                                   -axis_y * sn + ( axis_z * axis_x * oneMinusCos ) ),
                      
                      MakeVector3( -axis_z * sn + ( axis_x * axis_y * oneMinusCos ),
                                             cs + ( axis_y * axis_y * oneMinusCos ),
                                    axis_x * sn + ( axis_z * axis_y * oneMinusCos ) ),
                      
                      MakeVector3(  axis_y * sn + ( axis_x * axis_z * oneMinusCos ),
                                   -axis_x * sn + ( axis_y * axis_z * oneMinusCos ),
                                             cs + ( axis_z * axis_z * oneMinusCos ) ) );
    #else
    
    const Vector3 axis_x_oneMinusCos( axis * oneMinusCos );
    
    return MakeMat33( MakeVector3(  one,     axis_z, -axis_y ) * MakeVector3( cs, sn, sn ) + axis_x_oneMinusCos * axis_x,
                      MakeVector3( -axis_z,  one,     axis_x ) * MakeVector3( sn, cs, sn ) + axis_x_oneMinusCos * axis_y,
                      MakeVector3(  axis_y, -axis_x,  one    ) * MakeVector3( sn, sn, cs ) + axis_x_oneMinusCos * axis_z );
    
    #endif
}

/// Create a Mat33 rotation matrix of theta radians about an arbitrary axis
NOALIAS __forceinline const Mat33 MakeMat33_AxisAngle( const Vector3 PCREF axis, const VF32 VECREF theta )
{
    return MakeMat33_AxisAngle( Normalise( axis ), theta );
}


/// Create a Mat33 rotation matrix from roll-pitch-heading Euler angles
NOALIAS __forceinline const Mat33 MakeMat33_RotRPH( const VF32 VECREF roll, const VF32 VECREF pitch, const VF32 VECREF heading )
{
    // Matrix RPH =
    //        colX                                   colY            colZ
    // rowX(  cos(r)*cos(h) + sin(r)*sin(p)*sin(h),  sin(r)*cos(p), -cos(r)*sin(h) + sin(r)*sin(p)*cos(h) );
    // rowY( -sin(r)*cos(h) + cos(r)*sin(p)*sin(h),  cos(r)*cos(p),  sin(r)*sin(h) + cos(r)*sin(p)*cos(h) );
    // rowZ(  cos(p)*sin(h),                        -sin(p),         cos(p)*cos(h) );
    
    Mat33 result;
    VF32 sin_r, cos_r,
         sin_p, cos_p,
         sin_h, cos_h;
    SinCos2( roll, pitch, &sin_r, &cos_r, &sin_p, &cos_p );
    SinCos( heading, &sin_h, &cos_h );
    SetRows( &result, MakeVec4F32(  cos_r*cos_h + sin_r*sin_p*sin_h,  sin_r*cos_p, -cos_r*sin_h + sin_r*sin_p*cos_h ),
                      MakeVec4F32( -sin_r*cos_h + cos_r*sin_p*sin_h,  cos_r*cos_p,  sin_r*sin_h + cos_r*sin_p*cos_h ),
                      MakeVec4F32(  cos_p*sin_h,                     -sin_p,        cos_p*cos_h                     ) );
    return result;
}

/// Create a Mat33 rotation matrix from pitch-heading-roll Euler angles
NOALIAS __forceinline const Mat33 MakeMat33_RotPHR( const VF32 VECREF pitch, const VF32 VECREF heading, const VF32 VECREF roll )
{
    // Matrix PHR =
    //       colX                                colY                                 colZ
    // rowX( cos(h)*cos(r),                      cos(h)*sin(r),                      -sin(h)        )
    // rowY( sin(p)*sin(h)*cos(r)-cos(p)*sin(r), sin(p)*sin(h)*sin(r)+cos(p)*cos(r),  sin(p)*cos(h) )
    // rowZ( cos(p)*sin(h)*cos(r)+sin(p)*sin(r), cos(p)*sin(h)*sin(r)-sin(p)*cos(r),  cos(p)*cos(h) )
    
    Mat33 result;
    VF32 sin_p, cos_p,
         sin_h, cos_h,
         sin_r, cos_r;
    SinCos2( pitch, heading, &sin_p, &cos_p, &sin_h, &cos_h );
    SinCos( roll, &sin_r, &cos_r );
    SetRows( &result, MakeVec4F32( cos_h*cos_r,                   cos_h*sin_r,                   -sin_h       ),
                      MakeVec4F32( sin_p*sin_h*cos_r-cos_p*sin_r, sin_p*sin_h*sin_r+cos_p*cos_r,  sin_p*cos_h ),
                      MakeVec4F32( cos_p*sin_h*cos_r+sin_p*sin_r, cos_p*sin_h*sin_r-sin_p*cos_r,  cos_p*cos_h ) );
    return result;
}


/// Create a skew-symmetric matrix from a Vector3 (Cross-Product Matrix)
NOALIAS __forceinline const Mat33 MakeMat33_SkewSymmetric( const Vector3 PCREF a )
{
    static const int a1=VX, a2=VY, a3=VZ, zero=VW;
    const Vector4 xyz0( MakeVector4( a ) );
    const Vector3 rowX( MakeVector3( NegateY( Permute< zero, a3, a2, zero >( xyz0 ) ) ) );
    const Vector3 rowY( MakeVector3( NegateZ( Permute< a3, zero, a1, zero >( xyz0 ) ) ) );
    const Vector3 rowZ( MakeVector3( NegateX( Permute< a2, a1, zero, zero >( xyz0 ) ) ) );
    return MakeMat33( rowX, rowY, rowZ );
}


// Euler angle rotation about X axis [ clockwise when viewed back down the axis ]
// [ 1,  0,      0      ]
// [ 0,  cos(x), sin(x) ]
// [ 0, -sin(x), cos(x) ]

// Euler angle rotation about Y axis [ clockwise when viewed back down the axis ]
// [ cos(y), 0, -sin(y) ]
// [ 0,      1,  0      ]
// [ sin(y), 0,  cos(y) ]

// Euler angle rotation about Z axis [ clockwise when viewed back down the axis ]
// [  cos(z), sin(z), 0 ]
// [ -sin(z), cos(z), 0 ]
// [  0,      0,      1 ]

/// Create a Mat33 from an EulerXYZ
NOALIAS __forceinline const Mat33 MakeMat33( const EulerXYZ& src )
{
    // [ 1,  0,      0      ]   [ cos(y), 0, -sin(y) ]   [  cos(z), sin(z), 0 ]
    // [ 0,  cos(x), sin(x) ] * [ 0,      1,  0      ] * [ -sin(z), cos(z), 0 ]
    // [ 0, -sin(x), cos(x) ]   [ sin(y), 0,  cos(y) ]   [  0,      0,      1 ]
    
    // [ cos(y),         0,      -sin(y)        ]   [  cos(z), sin(z), 0 ]
    // [ sin(x).sin(y),  cos(x),  sin(x).cos(y) ] * [ -sin(z), cos(z), 0 ]
    // [ cos(x).sin(y), -sin(x),  cos(x).cos(y) ]   [  0,      0,      1 ]
    
    // [ cos(y).cos(z),                        cos(y).sin(z),                        -sin(y)        ]
    // [ sin(x).sin(y).cos(z) - cos(x).sin(z), sin(x).sin(y).sin(z) + cos(x).cos(z),  sin(x).cos(y) ] 
    // [ cos(x).sin(y).cos(z) + sin(x).sin(z), cos(x).sin(y).sin(z) - sin(x).cos(z),  cos(x).cos(y) ]
    
    #if 0
    
    const Mat33 rotX( MakeMat33_RotX( X( src ) ) );
    const Mat33 rotY( MakeMat33_RotY( Y( src ) ) );
    const Mat33 rotZ( MakeMat33_RotZ( Z( src ) ) );
    
    return rotX * rotY * rotZ;
    
    #else
    
    const Vector3 sinXYZ( Sin( GetVector3( src ) ) );
    const Vector3 cosXYZ( Cos( GetVector3( src ) ) );
    
    //const VF32 sinX( X( sinXYZ ) );
    //const VF32 sinY( Y( sinXYZ ) );
    //const VF32 sinZ( Z( sinXYZ ) );
    //const VF32 cosX( X( cosXYZ ) );
    //const VF32 cosY( Y( cosXYZ ) );
    //const VF32 cosZ( Z( cosXYZ ) );
    //
    //const Vector3 rowX( MakeVector3( cosY * cosZ,                      cosY * sinZ,                      -sinY        ) );
    //const Vector3 rowY( MakeVector3( sinX * sinY * cosZ - cosX * sinZ, sinX * sinY * sinZ + cosX * cosZ,  sinX * cosY ) ); 
    //const Vector3 rowZ( MakeVector3( cosX * sinY * cosZ + sinX * sinZ, cosX * sinY * sinZ - sinX * cosZ,  cosX * cosY ) );
    
    const Vector4 a( MakeVector4( sinXYZ, Const<VF32>::Zero() ) );
    const Vector4 b( MakeVector4( cosXYZ, Const<VF32>::One() ) );
    
    #define sinX 0
    #define sinY 1
    #define sinZ 2
    #define zero 3
    #define cosX 4
    #define cosY 5
    #define cosZ 6
    #define one  7
    
    //const Vector3 rowX( MakeVector3( cosY * cosZ,
    //                                 cosY * sinZ,
    //                                       -sinY ) );
    const Vector3 rowX( NegateZ( Permute<cosY,cosY,one>(a,b) * Permute<cosZ,sinZ,sinY>(a,b) ) );
    
    //const Vector3 rowY( MakeVector3( sinX * sinY * cosZ - cosX * sinZ,
    //                                 sinX * sinY * sinZ + cosX * cosZ,
    //                                                      sinX * cosY ) ); 
    const Vector3 rowY( Permute<sinX,sinX,zero>(a,b) * Permute<sinY,sinY,zero>(a,b) * Permute<cosZ,sinZ,zero>(a,b) +
                        NegateX( Permute<cosX,cosX,sinX>(a,b) ) * Permute<sinZ,cosZ,cosY>(a,b) );
    
    //const Vector3 rowZ( MakeVector3( cosX * sinY * cosZ + sinX * sinZ,
    //                                 cosX * sinY * sinZ - sinX * cosZ,
    //                                                      cosX * cosY ) );
    const Vector3 rowZ( Permute<cosX,cosX,zero>(a,b) * Permute<sinY,sinY,zero>(a,b) * Permute<cosZ,sinZ,zero>(a,b) +
                        NegateY( Permute<sinX,sinX,cosX>(a,b) ) * Permute<sinZ,cosZ,cosY>(a,b) );
    
    #undef sinX
    #undef sinY
    #undef sinZ
    #undef zero
    #undef cosX
    #undef cosY
    #undef cosZ
    #undef one
    
    return MakeMat33( rowX, rowY, rowZ );
    
    #endif
}

/// Create a Mat33 from an EulerXZY
NOALIAS __forceinline const Mat33 MakeMat33( const EulerXZY& src )
{
    // [ 1,  0,      0      ]   [  cos(z), sin(z), 0 ]   [ cos(y), 0, -sin(y) ]
    // [ 0,  cos(x), sin(x) ] * [ -sin(z), cos(z), 0 ] * [ 0,      1,  0      ]
    // [ 0, -sin(x), cos(x) ]   [  0,      0,      1 ]   [ sin(y), 0,  cos(y) ]
    
    // [  cos(z),         sin(z),        0      ]   [ cos(y), 0, -sin(y) ]
    // [ -cos(x).sin(z),  cos(x).cos(z), sin(x) ] * [ 0,      1,  0      ]
    // [  sin(x).sin(z), -sin(x).cos(z), cos(x) ]   [ sin(y), 0,  cos(y) ]
    
    // [  cos(z).cos(y),                         sin(z),        -cos(z).sin(y)                        ]
    // [ -cos(x).sin(z).cos(y) + sin(x).sin(y),  cos(x).cos(z),  cos(x).sin(z).sin(y) + sin(x).cos(y) ]
    // [  sin(x).sin(z).cos(y) + cos(x).sin(y), -sin(x).cos(z), -sin(x).sin(z).sin(y) + cos(x).cos(y) ]
    
    #if 0
    
    const Mat33 rotX( MakeMat33_RotX( X( src ) ) );
    const Mat33 rotY( MakeMat33_RotY( Y( src ) ) );
    const Mat33 rotZ( MakeMat33_RotZ( Z( src ) ) );
    
    return rotX * rotZ * rotY;
    
    #else
    
    const Vector3 sinXYZ( Sin( GetVector3( src ) ) );
    const Vector3 cosXYZ( Cos( GetVector3( src ) ) );
    
    //const VF32 sinX( X( sinXYZ ) );
    //const VF32 sinY( Y( sinXYZ ) );
    //const VF32 sinZ( Z( sinXYZ ) );
    //const VF32 cosX( X( cosXYZ ) );
    //const VF32 cosY( Y( cosXYZ ) );
    //const VF32 cosZ( Z( cosXYZ ) );
    //
    //const Vector3 rowX( MakeVector3(  cosZ * cosY,                       sinZ,        -cosZ * sinY                      ) );
    //const Vector3 rowY( MakeVector3( -cosX * sinZ * cosY + sinX * sinY,  cosX * cosZ,  cosX * sinZ * sinY + sinX * cosY ) );
    //const Vector3 rowZ( MakeVector3(  sinX * sinZ * cosY + cosX * sinY, -sinX * cosZ, -sinX * sinZ * sinY + cosX * cosY ) );
    
    const Vector4 a( MakeVector4( sinXYZ, Const<VF32>::Zero() ) );
    const Vector4 b( MakeVector4( cosXYZ, Const<VF32>::One() ) );
    
    #define sinX 0
    #define sinY 1
    #define sinZ 2
    #define zero 3
    #define cosX 4
    #define cosY 5
    #define cosZ 6
    #define one  7
    
    //const Vector3 rowX( MakeVector3(  cosZ * cosY,
    //                                         sinZ,
    //                                 -cosZ * sinY ) );
    const Vector3 rowX( NegateZ( Permute<cosZ,one,cosZ>(a,b) * Permute<cosY,sinZ,sinY>(a,b) ) );
    
    //const Vector3 rowY( MakeVector3( -cosX * sinZ * cosY + sinX * sinY,
    //                                                       cosX * cosZ,
    //                                  cosX * sinZ * sinY + sinX * cosY ) );
    const Vector3 rowY( NegateX( Permute<cosX,zero,cosX>(a,b) ) * Permute<sinZ,zero,sinZ>(a,b) * Permute<cosY,zero,sinY>(a,b) +
                        Permute<sinX,cosX,sinX>(a,b) * Permute<sinY,cosZ,cosY>(a,b) );
    
    //const Vector3 rowZ( MakeVector3(  sinX * sinZ * cosY + cosX * sinY,
    //                                                     - sinX * cosZ,
    //                                 -sinX * sinZ * sinY + cosX * cosY ) );
    const Vector3 rowZ( NegateZ( Permute<sinX,zero,sinX>(a,b) ) * Permute<sinZ,zero,sinZ>(a,b) * Permute<cosY,zero,sinY>(a,b) +
                        NegateY( Permute<cosX,sinX,cosX>(a,b) * Permute<sinY,cosZ,cosY>(a,b) ) );
    
    #undef sinX
    #undef sinY
    #undef sinZ
    #undef zero
    #undef cosX
    #undef cosY
    #undef cosZ
    #undef one
    
    return MakeMat33( rowX, rowY, rowZ );
    
    #endif
}

/// Create a Mat33 from an EulerYXZ
NOALIAS __forceinline const Mat33 MakeMat33( const EulerYXZ& src )
{
    // [ cos(y), 0, -sin(y) ]   [ 1,  0,      0      ]   [  cos(z), sin(z), 0 ]
    // [ 0,      1,  0      ] * [ 0,  cos(x), sin(x) ] * [ -sin(z), cos(z), 0 ]
    // [ sin(y), 0,  cos(y) ]   [ 0, -sin(x), cos(x) ]   [  0,      0,      1 ]
    
    // [ cos(y),  sin(y).sin(x), -sin(y).cos(x) ]   [  cos(z), sin(z), 0 ]
    // [ 0,       cos(x),         sin(x)        ] * [ -sin(z), cos(z), 0 ]
    // [ sin(y), -cos(y).sin(x),  cos(y).cos(x) ]   [  0,      0,      1 ]
    
    // [  cos(y).cos(z) - sin(y).sin(x).sin(z), cos(y).sin(z) + sin(y).sin(x).cos(z), -sin(y).cos(x) ]
    // [ -cos(x).sin(z),                        cos(x).cos(z),                         sin(x)        ]
    // [  sin(y).cos(z) + cos(y).sin(x).sin(z), sin(y).sin(z) - cos(y).sin(x).cos(z),  cos(y).cos(x) ]
    
    #if 0
    
    const Mat33 rotX( MakeMat33_RotX( X( src ) ) );
    const Mat33 rotY( MakeMat33_RotY( Y( src ) ) );
    const Mat33 rotZ( MakeMat33_RotZ( Z( src ) ) );
    
    return rotY * rotX * rotZ;
    
    #else
    
    const Vector3 sinXYZ( Sin( GetVector3( src ) ) );
    const Vector3 cosXYZ( Cos( GetVector3( src ) ) );
    
    //const VF32 sinX( X( sinXYZ ) );
    //const VF32 sinY( Y( sinXYZ ) );
    //const VF32 sinZ( Z( sinXYZ ) );
    //const VF32 cosX( X( cosXYZ ) );
    //const VF32 cosY( Y( cosXYZ ) );
    //const VF32 cosZ( Z( cosXYZ ) );
    //
    //const Vector3 rowX( MakeVector3(  cosY * cosZ - sinY * sinX * sinZ, cosY * sinZ + sinY * sinX * cosZ, -sinY * cosX ) );
    //const Vector3 rowY( MakeVector3( -cosX * sinZ,                      cosX * cosZ,                       sinX        ) );
    //const Vector3 rowZ( MakeVector3(  sinY * cosZ + cosY * sinX * sinZ, sinY * sinZ - cosY * sinX * cosZ,  cosY * cosX ) );
    
    const Vector4 a( MakeVector4( sinXYZ, Const<VF32>::Zero() ) );
    const Vector4 b( MakeVector4( cosXYZ, Const<VF32>::One() ) );
    
    #define sinX 0
    #define sinY 1
    #define sinZ 2
    #define zero 3
    #define cosX 4
    #define cosY 5
    #define cosZ 6
    #define one  7
    
    //const Vector3 rowX( MakeVector3( -sinY * sinX * sinZ + cosY * cosZ,
    //                                  sinY * sinX * cosZ + cosY * sinZ,
    //                                                     - sinY * cosX ) );
    const Vector3 rowX( NegateX( Permute<sinY,sinY,zero>(a,b) ) * Permute<sinX,sinX,zero>(a,b) * Permute<sinZ,cosZ,zero>(a,b) +
                        NegateZ( Permute<cosY,cosY,sinY>(a,b) * Permute<cosZ,sinZ,cosX>(a,b) ) );
    
    //const Vector3 rowY( MakeVector3( -cosX * sinZ,
    //                                  cosX * cosZ,
    //                                         sinX ) );
    const Vector3 rowY( NegateX( Permute<cosX,cosX,one>(a,b) * Permute<sinZ,cosZ,sinX>(a,b) ) );
    
    //const Vector3 rowZ( MakeVector3(  cosY * sinX * sinZ + sinY * cosZ,
    //                                 -cosY * sinX * cosZ + sinY * sinZ,
    //                                                       cosY * cosX ) );
    const Vector3 rowZ( NegateY( Permute<cosY,cosY,zero>(a,b) ) * Permute<sinX,sinX,zero>(a,b) * Permute<sinZ,cosZ,zero>(a,b) +
                        Permute<sinY,sinY,cosY>(a,b) * Permute<cosZ,sinZ,cosX>(a,b) );
    
    #undef sinX
    #undef sinY
    #undef sinZ
    #undef zero
    #undef cosX
    #undef cosY
    #undef cosZ
    #undef one
    
    return MakeMat33( rowX, rowY, rowZ );
    
    #endif
}

/// Create a Mat33 from an EulerYZX
NOALIAS __forceinline const Mat33 MakeMat33( const EulerYZX& src )
{
    // [ cos(y), 0, -sin(y) ]   [  cos(z), sin(z), 0 ]   [ 1,  0,      0      ]
    // [ 0,      1,  0      ] * [ -sin(z), cos(z), 0 ] * [ 0,  cos(x), sin(x) ]
    // [ sin(y), 0,  cos(y) ]   [  0,      0,      1 ]   [ 0, -sin(x), cos(x) ]
    
    // [  cos(y).cos(z), cos(y).sin(z), -sin(y) ]   [ 1,  0,      0      ]
    // [ -sin(z),        cos(z),         0      ] * [ 0,  cos(x), sin(x) ]
    // [  sin(y).cos(z), sin(y).sin(z),  cos(y) ]   [ 0, -sin(x), cos(x) ]
    
    // [  cos(y).cos(z), cos(y).sin(z).cos(x) + sin(y).sin(x), cos(y).sin(z).sin(x) - sin(y).cos(x) ]
    // [ -sin(z),        cos(z).cos(x),                        cos(z).sin(x)                        ]
    // [  sin(y).cos(z), sin(y).sin(z).cos(x) - cos(y).sin(x), sin(y).sin(z).sin(x) + cos(y).cos(x) ]
    
    #if 0
    
    const Mat33 rotX( MakeMat33_RotX( X( src ) ) );
    const Mat33 rotY( MakeMat33_RotY( Y( src ) ) );
    const Mat33 rotZ( MakeMat33_RotZ( Z( src ) ) );
    
    return rotY * rotZ * rotX;
    
    #else
    
    const Vector3 sinXYZ( Sin( GetVector3( src ) ) );
    const Vector3 cosXYZ( Cos( GetVector3( src ) ) );
    
    //const VF32 sinX( X( sinXYZ ) );
    //const VF32 sinY( Y( sinXYZ ) );
    //const VF32 sinZ( Z( sinXYZ ) );
    //const VF32 cosX( X( cosXYZ ) );
    //const VF32 cosY( Y( cosXYZ ) );
    //const VF32 cosZ( Z( cosXYZ ) );
    //
    //const Vector3 rowX( MakeVector3(  cosY * cosZ, cosY * sinZ * cosX + sinY * sinX, cosY * sinZ * sinX - sinY * cosX ) );
    //const Vector3 rowY( MakeVector3( -sinZ,        cosZ * cosX,                      cosZ * sinX                      ) );
    //const Vector3 rowZ( MakeVector3(  sinY * cosZ, sinY * sinZ * cosX - cosY * sinX, sinY * sinZ * sinX + cosY * cosX ) );
    
    const Vector4 a( MakeVector4( sinXYZ, Const<VF32>::Zero() ) );
    const Vector4 b( MakeVector4( cosXYZ, Const<VF32>::One() ) );
    
    #define sinX 0
    #define sinY 1
    #define sinZ 2
    #define zero 3
    #define cosX 4
    #define cosY 5
    #define cosZ 6
    #define one  7
    
    //const Vector3 rowX( MakeVector3(                      cosY * cosZ,
    //                                 cosY * sinZ * cosX + sinY * sinX,
    //                                 cosY * sinZ * sinX - sinY * cosX ) );
    const Vector3 rowX( Permute<zero,cosY,cosY>(a,b) * Permute<zero,sinZ,sinZ>(a,b) * Permute<zero,cosX,sinX>(a,b) +
                        NegateZ( Permute<cosY,sinY,sinY>(a,b) ) * Permute<cosZ,sinX,cosX>(a,b) );
    
    //const Vector3 rowY( MakeVector3(       -sinZ,
    //                                 cosZ * cosX,
    //                                 cosZ * sinX ) );
    const Vector3 rowY( NegateX( Permute<one,cosZ,cosZ>(a,b) * Permute<sinZ,cosX,sinX>(a,b) ) );
    
    //const Vector3 rowZ( MakeVector3(                      sinY * cosZ,
    //                                 sinY * sinZ * cosX - cosY * sinX,
    //                                 sinY * sinZ * sinX + cosY * cosX ) );
    const Vector3 rowZ( Permute<zero,sinY,sinY>(a,b) * Permute<zero,sinZ,sinZ>(a,b) * Permute<zero,cosX,sinX>(a,b) +
                        NegateY( Permute<sinY,cosY,cosY>(a,b) ) * Permute<cosZ,sinX,cosX>(a,b) );
    
    #undef sinX
    #undef sinY
    #undef sinZ
    #undef zero
    #undef cosX
    #undef cosY
    #undef cosZ
    #undef one
    
    return MakeMat33( rowX, rowY, rowZ );
    
    #endif
}

/// Create a Mat33 from an EulerZXY
NOALIAS __forceinline const Mat33 MakeMat33( const EulerZXY& src )
{
    // [  cos(z), sin(z), 0 ]   [ 1,  0,      0      ]   [ cos(y), 0, -sin(y) ]
    // [ -sin(z), cos(z), 0 ] * [ 0,  cos(x), sin(x) ] * [ 0,      1,  0      ]
    // [  0,      0,      1 ]   [ 0, -sin(x), cos(x) ]   [ sin(y), 0,  cos(y) ]
    
    // [  cos(z),  sin(z).cos(x), sin(z).sin(x) ]   [ cos(y), 0, -sin(y) ]
    // [ -sin(z),  cos(z).cos(x), cos(z).sin(x) ] * [ 0,      1,  0      ]
    // [  0,      -sin(x),        cos(x)        ]   [ sin(y), 0,  cos(y) ]
    
    // [  cos(z).cos(y) + sin(z).sin(x).sin(y),  sin(z).cos(x), -cos(z).sin(y) + sin(z).sin(x).cos(y) ]
    // [ -sin(z).cos(y) + cos(z).sin(x).sin(y),  cos(z).cos(x),  sin(z).sin(y) + cos(z).sin(x).cos(y) ]
    // [  cos(x).sin(y),                        -sin(x),         cos(x).cos(y)                        ]
    
    #if 0
    
    const Mat33 rotX( MakeMat33_RotX( X( src ) ) );
    const Mat33 rotY( MakeMat33_RotY( Y( src ) ) );
    const Mat33 rotZ( MakeMat33_RotZ( Z( src ) ) );
    
    return rotZ * rotX * rotY;
    
    #else
    
    const Vector3 sinXYZ( Sin( GetVector3( src ) ) );
    const Vector3 cosXYZ( Cos( GetVector3( src ) ) );
    
    //const VF32 sinX( X( sinXYZ ) );
    //const VF32 sinY( Y( sinXYZ ) );
    //const VF32 sinZ( Z( sinXYZ ) );
    //const VF32 cosX( X( cosXYZ ) );
    //const VF32 cosY( Y( cosXYZ ) );
    //const VF32 cosZ( Z( cosXYZ ) );
    //
    //const Vector3 rowX( MakeVector3(  cosZ * cosY + sinZ * sinX * sinY,  sinZ * cosX, -cosZ * sinY + sinZ * sinX * cosY ) );
    //const Vector3 rowY( MakeVector3( -sinZ * cosY + cosZ * sinX * sinY,  cosZ * cosX,  sinZ * sinY + cosZ * sinX * cosY ) );
    //const Vector3 rowZ( MakeVector3(  cosX * sinY,                      -sinX,         cosX * cosY                      ) );
    
    const Vector4 a( MakeVector4( sinXYZ, Const<VF32>::Zero() ) );
    const Vector4 b( MakeVector4( cosXYZ, Const<VF32>::One() ) );
    
    #define sinX 0
    #define sinY 1
    #define sinZ 2
    #define zero 3
    #define cosX 4
    #define cosY 5
    #define cosZ 6
    #define one  7
    
    //const Vector3 rowX( MakeVector3( sinZ * sinX * sinY + cosZ * cosY,
    //                                                      sinZ * cosX,
    //                                 sinZ * sinX * cosY - cosZ * sinY ) );
    const Vector3 rowX( Permute<sinZ,zero,sinZ>(a,b) * Permute<sinX,zero,sinX>(a,b) * Permute<sinY,zero,cosY>(a,b) +
                        NegateZ( Permute<cosZ,sinZ,cosZ>(a,b) ) * Permute<cosY,cosX,sinY>(a,b) );
    
    //const Vector3 rowY( MakeVector3( cosZ * sinX * sinY - sinZ * cosY,
    //                                                      cosZ * cosX,
    //                                 cosZ * sinX * cosY + sinZ * sinY ) );
    const Vector3 rowY( Permute<cosZ,zero,cosZ>(a,b) * Permute<sinX,zero,sinX>(a,b) * Permute<sinY,zero,cosY>(a,b) +
                        NegateX( Permute<sinZ,cosZ,sinZ>(a,b) ) * Permute<cosY,cosX,sinY>(a,b) );
    
    //const Vector3 rowZ( MakeVector3( cosX * sinY,
    //                                       -sinX,
    //                                 cosX * cosY ) );
    const Vector3 rowZ( NegateY( Permute<cosX,one,cosX>(a,b) * Permute<sinY,sinX,cosY>(a,b) ) );
    
    #undef sinX
    #undef sinY
    #undef sinZ
    #undef zero
    #undef cosX
    #undef cosY
    #undef cosZ
    #undef one
    
    return MakeMat33( rowX, rowY, rowZ );
    
    #endif
}

/// Create a Mat33 from an EulerZYX
NOALIAS __forceinline const Mat33 MakeMat33( const EulerZYX& src )
{
    // [  cos(z), sin(z), 0 ]   [ cos(y), 0, -sin(y) ]   [ 1,  0,      0      ]
    // [ -sin(z), cos(z), 0 ] * [ 0,      1,  0      ] * [ 0,  cos(x), sin(x) ]
    // [  0,      0,      1 ]   [ sin(y), 0,  cos(y) ]   [ 0, -sin(x), cos(x) ]
    
    // [  cos(z).cos(y), sin(z), -cos(z).sin(y) ]   [ 1,  0,      0      ]
    // [ -sin(z).cos(y), cos(z),  sin(z).sin(y) ] * [ 0,  cos(x), sin(x) ]
    // [  sin(y),        0,       cos(y)        ]   [ 0, -sin(x), cos(x) ]
    
    // [  cos(z).cos(y),  sin(z).cos(x) + cos(z).sin(y).sin(x), sin(z).sin(x) - cos(z).sin(y).cos(x) ]
    // [ -sin(z).cos(y),  cos(z).cos(x) - sin(z).sin(y).sin(x), cos(z).sin(x) + sin(z).sin(y).cos(x) ]
    // [  sin(y),        -cos(y).sin(x),                        cos(y).cos(x)                        ]
    
    #if 0
    
    const Mat33 rotX( MakeMat33_RotX( X( src ) ) );
    const Mat33 rotY( MakeMat33_RotY( Y( src ) ) );
    const Mat33 rotZ( MakeMat33_RotZ( Z( src ) ) );
    
    return rotZ * rotY * rotX;
    
    #else
    
    const Vector3 sinXYZ( Sin( GetVector3( src ) ) );
    const Vector3 cosXYZ( Cos( GetVector3( src ) ) );
    
    //const VF32 sinX( X( sinXYZ ) );
    //const VF32 sinY( Y( sinXYZ ) );
    //const VF32 sinZ( Z( sinXYZ ) );
    //const VF32 cosX( X( cosXYZ ) );
    //const VF32 cosY( Y( cosXYZ ) );
    //const VF32 cosZ( Z( cosXYZ ) );
    
    //const Vector3 rowX( MakeVector3(  cosZ * cosY,  sinZ * cosX + cosZ * sinY * sinX, sinZ * sinX - cosZ * sinY * cosX ) );
    //const Vector3 rowY( MakeVector3( -sinZ * cosY,  cosZ * cosX - sinZ * sinY * sinX, cosZ * sinX + sinZ * sinY * cosX ) );
    //const Vector3 rowZ( MakeVector3(  sinY,        -cosY * sinX,                      cosY * cosX                      ) );
    //
    //const Vector4 a( MakeVector4( sinXYZ, Const<VF32>::Zero() ) );
    //const Vector4 b( MakeVector4( cosXYZ, Const<VF32>::One()  ) );
    
    const Vector4 a( MakeVector4( sinXYZ, Const<VF32>::Zero() ) );
    const Vector4 b( MakeVector4( cosXYZ, Const<VF32>::One() ) );
    
    #define sinX 0
    #define sinY 1
    #define sinZ 2
    #define zero 3
    #define cosX 4
    #define cosY 5
    #define cosZ 6
    #define one  7
    
    //const Vector3 rowX( MakeVector3(                       cosZ * cosY,
    //                                  cosZ * sinY * sinX + sinZ * cosX,
    //                                 -cosZ * sinY * cosX + sinZ * sinX  ) );
    const Vector3 rowX( NegateZ( Permute<zero,cosZ,cosZ>(a,b) ) * Permute<zero,sinY,sinY>(a,b) * Permute<zero,sinX,cosX>(a,b) +
                        Permute<cosZ,sinZ,sinZ>(a,b) * Permute<cosY,cosX,sinX>(a,b) );
    
    //const Vector3 rowY( MakeVector3(                      -sinZ * cosY,
    //                                 -sinZ * sinY * sinX + cosZ * cosX,
    //                                  sinZ * sinY * cosX + cosZ * sinX ) );
    const Vector3 rowY( NegateY( Permute<zero,sinZ,sinZ>(a,b) ) * Permute<zero,sinY,sinY>(a,b) * Permute<zero,sinX,cosX>(a,b) +
                        NegateX( Permute<sinZ,cosZ,cosZ>(a,b) ) * Permute<cosY,cosX,sinX>(a,b) );
    
    //const Vector3 rowZ( MakeVector3(         sinY,
    //                                 -cosY * sinX,
    //                                  cosY * cosX ) );
    const Vector3 rowZ( NegateY( Permute<one,cosY,cosY>(a,b) * Permute<sinY,sinX,cosX>(a,b) ) );
    
    #undef sinX
    #undef sinY
    #undef sinZ
    #undef zero
    #undef cosX
    #undef cosY
    #undef cosZ
    #undef one
    
    return MakeMat33( rowX, rowY, rowZ );
    
    #endif
}




/// Mat33 Write to Stream
template < typename T_Stream > void Serialise( const Mat33 MATREF src, T_Stream& stream )
{
    stream << RowX( src );
    stream << RowY( src );
    stream << RowZ( src );
}

/// Mat33 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Mat33* const __restrict pDst )
{
    Vector3 rowX, rowY, rowZ;
    stream >> rowX;
    stream >> rowY;
    stream >> rowZ;
    *pDst = MakeMat33( rowX, rowY, rowZ );
}




/// Mat33 Negation
NOALIAS __forceinline const Mat33 operator - ( const Mat33 MATREF src )
{
    Mat33 result;
    #if defined( _VMX128_ )
    const __vector4 sign_mask( Const<__vector4>::SignMask() );
    result.m_ColX = __vxor( src.m_ColX, sign_mask );
    result.m_ColY = __vxor( src.m_ColY, sign_mask );
    result.m_ColZ = __vxor( src.m_ColZ, sign_mask );
    #else
    SetRowX( &result, -RowX( src ) );
    SetRowY( &result, -RowY( src ) );
    SetRowZ( &result, -RowZ( src ) );
    #endif
    return result;
}

/// Mat33 Addition operator
NOALIAS __forceinline const Mat33 operator + ( const Mat33 MATREF lhs, const Mat33 MATREF rhs )
{
    Mat33 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_add( lhs.m_RowX, rhs.m_RowX );
    result.m_RowY = spu_add( lhs.m_RowY, rhs.m_RowY );
    result.m_RowZ = spu_add( lhs.m_RowZ, rhs.m_RowZ );
    #elif defined( _PPU_ )
    result.m_RowX = vec_add( lhs.m_RowX, rhs.m_RowX );
    result.m_RowY = vec_add( lhs.m_RowY, rhs.m_RowY );
    result.m_RowZ = vec_add( lhs.m_RowZ, rhs.m_RowZ );
    #elif defined( _VMX128_ )
    result.m_ColX = __vaddfp( lhs.m_ColX, rhs.m_ColX );
    result.m_ColY = __vaddfp( lhs.m_ColY, rhs.m_ColY );
    result.m_ColZ = __vaddfp( lhs.m_ColZ, rhs.m_ColZ );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_add_ps( lhs.m_RowX, rhs.m_RowX );
    result.m_RowY = _mm_add_ps( lhs.m_RowY, rhs.m_RowY );
    result.m_RowZ = _mm_add_ps( lhs.m_RowZ, rhs.m_RowZ );
    #else
    for ( S32 r=0; r<3; r++ )
    {
        for ( S32 c=0; c<3; c++ )
        {
            result.m[r][c] = lhs.m[r][c] + rhs.m[r][c];
        }
    }
    #endif
    return result;
}

/// Mat33 Subtraction operator
NOALIAS __forceinline const Mat33 operator - ( const Mat33 MATREF lhs, const Mat33 MATREF rhs )
{
    Mat33 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_sub( lhs.m_RowX, rhs.m_RowX );
    result.m_RowY = spu_sub( lhs.m_RowY, rhs.m_RowY );
    result.m_RowZ = spu_sub( lhs.m_RowZ, rhs.m_RowZ );
    #elif defined( _PPU_ )
    result.m_RowX = vec_sub( lhs.m_RowX, rhs.m_RowX );
    result.m_RowY = vec_sub( lhs.m_RowY, rhs.m_RowY );
    result.m_RowZ = vec_sub( lhs.m_RowZ, rhs.m_RowZ );
    #elif defined( _VMX128_ )
    result.m_ColX = __vsubfp( lhs.m_ColX, rhs.m_ColX );
    result.m_ColY = __vsubfp( lhs.m_ColY, rhs.m_ColY );
    result.m_ColZ = __vsubfp( lhs.m_ColZ, rhs.m_ColZ );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_sub_ps( lhs.m_RowX, rhs.m_RowX );
    result.m_RowY = _mm_sub_ps( lhs.m_RowY, rhs.m_RowY );
    result.m_RowZ = _mm_sub_ps( lhs.m_RowZ, rhs.m_RowZ );
    #else
    for ( S32 r=0; r<3; r++ )
    {
        for ( S32 c=0; c<3; c++ )
        {
            result.m[r][c] = lhs.m[r][c] - rhs.m[r][c];
        }
    }
    #endif
    return result;
}

/// Mat33 * VF32 Multiplication operator
NOALIAS __forceinline const Mat33 operator * ( const Mat33 MATREF matrix, const VF32 VECREF scalar )
{
    Mat33 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_mul( matrix.m_RowX, scalar.m_V );
    result.m_RowY = spu_mul( matrix.m_RowY, scalar.m_V );
    result.m_RowZ = spu_mul( matrix.m_RowZ, scalar.m_V );
    #elif defined( _PPU_ )
    result.m_RowX = vec_mul( matrix.m_RowX, scalar.m_V );
    result.m_RowY = vec_mul( matrix.m_RowY, scalar.m_V );
    result.m_RowZ = vec_mul( matrix.m_RowZ, scalar.m_V );
    #elif defined( _VMX128_ )
    result.m_ColX = __vmulfp( matrix.m_ColX, scalar.m_V );
    result.m_ColY = __vmulfp( matrix.m_ColY, scalar.m_V );
    result.m_ColZ = __vmulfp( matrix.m_ColZ, scalar.m_V );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_mul_ps( matrix.m_RowX, scalar.m_V );
    result.m_RowY = _mm_mul_ps( matrix.m_RowY, scalar.m_V );
    result.m_RowZ = _mm_mul_ps( matrix.m_RowZ, scalar.m_V );
    #else
    for ( S32 r=0; r<3; r++ )
    {
        for ( S32 c=0; c<3; c++ )
        {
            result.m[r][c] = matrix.m[r][c] * scalar.m_V;
        }
    }
    #endif
    return result;
}

/// VF32 * Mat33 Multiplication operator
NOALIAS __forceinline const Mat33 operator * ( const VF32 VECREF scalar, const Mat33 MATREF matrix )
{
    Mat33 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_mul( matrix.m_RowX, scalar.m_V );
    result.m_RowY = spu_mul( matrix.m_RowY, scalar.m_V );
    result.m_RowZ = spu_mul( matrix.m_RowZ, scalar.m_V );
    #elif defined( _PPU_ )
    result.m_RowX = vec_mul( matrix.m_RowX, scalar.m_V );
    result.m_RowY = vec_mul( matrix.m_RowY, scalar.m_V );
    result.m_RowZ = vec_mul( matrix.m_RowZ, scalar.m_V );
    #elif defined( _VMX128_ )
    result.m_ColX = __vmulfp( matrix.m_ColX, scalar.m_V );
    result.m_ColY = __vmulfp( matrix.m_ColY, scalar.m_V );
    result.m_ColZ = __vmulfp( matrix.m_ColZ, scalar.m_V );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_mul_ps( matrix.m_RowX, scalar.m_V );
    result.m_RowY = _mm_mul_ps( matrix.m_RowY, scalar.m_V );
    result.m_RowZ = _mm_mul_ps( matrix.m_RowZ, scalar.m_V );
    #else
    for ( S32 r=0; r<3; r++ )
    {
        for ( S32 c=0; c<3; c++ )
        {
            result.m[r][c] = matrix.m[r][c] * scalar.m_V;
        }
    }
    #endif
    return result;
}

/// Mat33 / VF32 Division operator
NOALIAS __forceinline const Mat33 operator / ( const Mat33 MATREF matrix, const VF32 VECREF scalar )
{
    return matrix * Rcp( scalar );
}



/// Vec4F32 * Mat33 Multiplication operator : input = 3 element row vector, output = 3 element row vector
NOALIAS __forceinline const Vec4F32 operator * ( const Vec4F32 VECREF v, const Mat33 MATREF m )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    //result.m_V = spu_madd( spu_splat<VX>( v.m_V ), m.m_RowX,
    //             spu_madd( spu_splat<VY>( v.m_V ), m.m_RowY,
    //             spu_mul ( spu_splat<VZ>( v.m_V ), m.m_RowZ ) ) );
    result.m_V = spu_multiply_vector3_matrix33( v.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    //result.m_V = vec_madd( vec_splat( v.m_V, VX ), m.m_RowX,
    //             vec_madd( vec_splat( v.m_V, VY ), m.m_RowY,
    //             vec_mul ( vec_splat( v.m_V, VZ ), m.m_RowZ ) ) );
    result.m_V = vec_multiply_vector3_matrix33( v.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    //result.m_V = vmx_make_vector3( __vmsum3fp( v.m_V, m.m_ColX ),
    //                               __vmsum3fp( v.m_V, m.m_ColY ),
    //                               __vmsum3fp( v.m_V, m.m_ColZ ) );
    result.m_V = vmx_multiply_vector3_matrix33( v.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    //result.m_V = _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( v.m_V ), m.m_RowX ),
    //                                     _mm_mul_ps( sse_splat<VY>( v.m_V ), m.m_RowY ) ),
    //                                     _mm_mul_ps( sse_splat<VZ>( v.m_V ), m.m_RowZ ) );
    result.m_V = sse_multiply_vector3_matrix33( v.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #else
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V[c] = v.m_V[0] * m.m[0][c] +
                        v.m_V[1] * m.m[1][c] +
                        v.m_V[2] * m.m[2][c];
    }
    #endif
    return result;
}

/// Mat33 * Vec4F32 Multiplication operator : input = 3 element col vector, output = 3 element col vector 
NOALIAS __forceinline const Vec4F32 operator * ( const Mat33 MATREF m, const Vec4F32 VECREF v )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    vector float col_x, col_y, col_z;
    spu_transpose_matrix33( m.m_RowX, m.m_RowY, m.m_RowZ, &col_x, &col_y, &col_z );
    result.m_V = spu_madd( col_x, spu_splat<VX>( v.m_V ),
                 spu_madd( col_y, spu_splat<VY>( v.m_V ),
                 spu_mul ( col_z, spu_splat<VZ>( v.m_V ) ) ) );
    #elif defined( _PPU_ )
    vector float col_x, col_y, col_z;
    vec_transpose_matrix33( m.m_RowX, m.m_RowY, m.m_RowZ, &col_x, &col_y, &col_z );
    result.m_V = vec_madd( col_x, vec_splat( v.m_V, VX ),
                 vec_madd( col_y, vec_splat( v.m_V, VY ),
                 vec_mul ( col_z, vec_splat( v.m_V, VZ ) ) ) );
    #elif defined( _VMX128_ )
    __vector4 row_x, row_y, row_z;
    vmx_transpose_matrix33( m.m_ColX, m.m_ColY, m.m_ColZ, &row_x, &row_y, &row_z );
    result.m_V = vmx_make_vector3( __vmsum3fp( row_x, v.m_V ),
                                   __vmsum3fp( row_y, v.m_V ),
                                   __vmsum3fp( row_z, v.m_V ) );
    #elif defined( _SSE_ )
    __m128 col_x, col_y, col_z;
    sse_transpose_matrix33( m.m_RowX, m.m_RowY, m.m_RowZ, &col_x, &col_y, &col_z );
    result.m_V = _mm_add_ps( _mm_add_ps( _mm_mul_ps( col_x, sse_splat<VX>( v.m_V ) ),
                                         _mm_mul_ps( col_y, sse_splat<VY>( v.m_V ) ) ),
                                         _mm_mul_ps( col_z, sse_splat<VZ>( v.m_V ) ) );
    #else
    for ( S32 r=0; r<3; r++ )
    {
        result.m_V[r] = m.m[r][0] * v.m_V[0] +
                        m.m[r][1] * v.m_V[1] +
                        m.m[r][2] * v.m_V[2];
    }
    #endif
    return result;
}


/// Vector3 Matrix Multiplication: Vector3 * Mat33
NOALIAS __forceinline const Vector3 operator * ( const Vector3 PCREF v, const Mat33 MATREF m )
{
    /// @code
    ///             | m00 m01 m02 |   [ x.m00 + y.m10 + z.m20,
    /// [ x y z ] * | m10 m11 m12 | =   x.m01 + y.m11 + z.m21,
    ///             | m20 m21 m22 |     x.m02 + y.m12 + z.m22 ]
    /// @endcode
    Vector3 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_multiply_vector3_matrix33( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_multiply_vector3_matrix33( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    result.m_V.m_V = vmx_multiply_vector3_matrix33( v.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_multiply_vector3_matrix33( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
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

/// Vector3 Matrix Multiplication: Mat33 * Vector3
NOALIAS __forceinline const Vector3 operator * ( const Mat33 MATREF m, const Vector3 PCREF v )
{
    /// @code
    /// | m00 m01 m02 |   |x|   | m00.x + m01.y + m02.z |
    /// | m10 m11 m12 | * |x| = | m10.x + m11.y + m12.z |
    /// | m20 m21 m22 |   |z|   | m20.x + m21.y + m22.z |
    /// @endcode
    Vector3 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_multiply_matrix33_vector3( m.m_RowX, m.m_RowY, m.m_RowZ, v.m_V.m_V );
    #elif defined( _PPU_ )                                                        
    result.m_V.m_V = vec_multiply_matrix33_vector3( m.m_RowX, m.m_RowY, m.m_RowZ, v.m_V.m_V );
    #elif defined( _VMX128_ )                                                     
    result.m_V.m_V = vmx_multiply_matrix33_vector3( m.m_ColX, m.m_ColY, m.m_ColZ, v.m_V.m_V );
    #elif defined( _SSE_ )                                                        
    result.m_V.m_V = sse_multiply_matrix33_vector3( m.m_RowX, m.m_RowY, m.m_RowZ, v.m_V.m_V );
    #else
    for ( S32 r=0; r<3; r++ )
    {
        result.m_V.m_V[r] = m.m[r][0] * v.m_V.m_V[0] +
                            m.m[r][1] * v.m_V.m_V[1] +
                            m.m[r][2] * v.m_V.m_V[2];
    }
    #endif
    return result;
}

/// Vector3 Matrix Multiplication Update: Vector3 *= Mat33
NOALIAS __forceinline void operator *= ( Vector3& v, const Mat33 MATREF m )
{
    #if defined( _SPU_ )
    v.m_V.m_V = spu_multiply_vector3_matrix33( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    v.m_V.m_V = vec_multiply_vector3_matrix33( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    v.m_V.m_V = vmx_multiply_vector3_matrix33( v.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    v.m_V.m_V = sse_multiply_vector3_matrix33( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
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


/// Point3 Matrix Multiplication: Point3 * Mat33
NOALIAS __forceinline const Point3 operator * ( const Point3 PCREF p, const Mat33 MATREF m )
{
    Point3 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_multiply_point3_matrix33( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_multiply_point3_matrix33( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    result.m_V.m_V = vmx_multiply_point3_matrix33( p.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_multiply_point3_matrix33( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #else
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V.m_V[c] = p.m_V.m_V[0] * m.m[0][c] +
                            p.m_V.m_V[1] * m.m[1][c] +
                            p.m_V.m_V[2] * m.m[2][c];
    }
    #endif
    return result;
}

/// Point3 Matrix Multiplication Update: Point3 *= Mat33
NOALIAS __forceinline void operator *= ( Point3& p, const Mat33 MATREF m )
{
    #if defined( _SPU_ )
    p.m_V.m_V = spu_multiply_point3_matrix33( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    p.m_V.m_V = vec_multiply_point3_matrix33( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    p.m_V.m_V = vmx_multiply_point3_matrix33( p.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    p.m_V.m_V = sse_multiply_point3_matrix33( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #else
    Point3 result;
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V.m_V[c] = p.m_V.m_V[0] * m.m[0][c] +
                            p.m_V.m_V[1] * m.m[1][c] +
                            p.m_V.m_V[2] * m.m[2][c];
    }
    p = result;
    #endif
}


/// HPoint3 Matrix Multiplication: Point3 * Mat33
NOALIAS __forceinline const HPoint3 operator * ( const HPoint3 PCREF p, const Mat33 MATREF m )
{
    HPoint3 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_multiply_vector4_matrix33( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_multiply_vector4_matrix33( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    result.m_V.m_V = vmx_multiply_vector4_matrix33( p.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_multiply_vector4_matrix33( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #else
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V.m_V[c] = p.m_V.m_V[0] * m.m[0][c] +
                            p.m_V.m_V[1] * m.m[1][c] +
                            p.m_V.m_V[2] * m.m[2][c];
    }
    #endif
    SetW( &result, W(p) );
    return result;
}

/// HPoint3 Matrix Multiplication Update: HPoint3 *= Mat33
NOALIAS __forceinline void operator *= ( HPoint3& p, const Mat33 MATREF m )
{
    #if defined( _SPU_ )
    p.m_V.m_V = spu_multiply_vector4_matrix33( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    p.m_V.m_V = vec_multiply_vector4_matrix33( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    p.m_V.m_V = vmx_multiply_vector4_matrix33( p.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    p.m_V.m_V = sse_multiply_vector4_matrix33( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #else
    Point3 result;
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V.m_V[c] = p.m_V.m_V[0] * m.m[0][c] +
                            p.m_V.m_V[1] * m.m[1][c] +
                            p.m_V.m_V[2] * m.m[2][c];
    }
    for ( S32 c=0; c<3; c++ )
    {
        p.m_V.m_V[c] = result.m_V.m_V[c];
    }
    #endif
}


/// Mat33 Multiplication operator
NOALIAS __forceinline const Mat33 operator * ( const Mat33 MATREF lhs, const Mat33 MATREF rhs )
{
    Mat33 result;
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
    #elif defined( _VMX128_ )
    __vector4 lhs_row_x, lhs_row_y, lhs_row_z;
    vmx_transpose_matrix33( lhs.m_ColX, lhs.m_ColY, lhs.m_ColZ, &lhs_row_x, &lhs_row_y, &lhs_row_z );
    const V128 result_cx_x( __vmsum3fp( lhs_row_x, rhs.m_ColX ) );
    const V128 result_cx_y( __vmsum3fp( lhs_row_y, rhs.m_ColX ) );
    const V128 result_cx_z( __vmsum3fp( lhs_row_z, rhs.m_ColX ) );
    const V128 result_cy_x( __vmsum3fp( lhs_row_x, rhs.m_ColY ) );
    const V128 result_cy_y( __vmsum3fp( lhs_row_y, rhs.m_ColY ) );
    const V128 result_cy_z( __vmsum3fp( lhs_row_z, rhs.m_ColY ) );
    const V128 result_cz_x( __vmsum3fp( lhs_row_x, rhs.m_ColZ ) );
    const V128 result_cz_y( __vmsum3fp( lhs_row_y, rhs.m_ColZ ) );
    const V128 result_cz_z( __vmsum3fp( lhs_row_z, rhs.m_ColZ ) );
    result.m_ColX = vmx_make_vector3( result_cx_x, result_cx_y, result_cx_z );
    result.m_ColY = vmx_make_vector3( result_cy_x, result_cy_y, result_cy_z );
    result.m_ColZ = vmx_make_vector3( result_cz_x, result_cz_y, result_cz_z );
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
    #endif
    return result;
}


/// Vector Space Tensor Product of a 3x1 column vector and a 1x3 row vector -> Mat33
NOALIAS __forceinline const Mat33 TensorProduct( const Vector3 PCREF lhs, const Vector3 PCREF rhs )
{
    return TensorProduct33( lhs.m_V, rhs.m_V );
}



// Cross products for deriving third matrix axis from 2 input axes
// ax = ay ^ az;
// ay = az ^ ax;
// az = ax ^ ay;

/// Create an ortho-normal Mat33 with the z axis pointing along the vector vz
NOALIAS __forceinline const Mat33 MakeMat33_AxisZ( const Vec4F32 VECREF vz )
{
    Mat33 result;
    const VF32    cos45deg( Const<VF32>::Sqrt2div2() );
    const Vec4F32 unit_x( Const<Vec4F32>::UnitX() );
    const Vec4F32 unit_y( Const<Vec4F32>::UnitY() );
    const VBool   test( Abs( unit_y | vz ) <= cos45deg );
    const Vec4F32 vx1( unit_y ^ vz );
    const Vec4F32 vy2( vz ^ unit_x );
    const Vec4F32 vy1( vz ^ vx1 );
    const Vec4F32 vx2( vy2 ^ vz );
    const Vec4F32 vx( Select( test, vx1, vx2 ) );
    const Vec4F32 vy( Select( test, vy1, vy2 ) );
    SetRows( &result, vx, vy, vz );
    Normalise( &result );
    return result;
}

/// Create an ortho-normal Mat33 with the z axis pointing along the vector vz and the y axis in the plane vz^vy
NOALIAS __forceinline const Mat33 MakeMat33_AxisZY( const Vec4F32 VECREF vz, const Vec4F32 VECREF vy0 )
{
    Mat33 result;
    const Vec4F32 vx( vy0 ^ vz );
    const Vec4F32 vy( vz ^ vx );
    SetRows( &result, vx, vy, vz );
    Normalise( &result );
    return result;
}

/// Create an ortho-normal Mat33 with the z axis pointing along the vector vz and the x axis in the plane vz^vx 
NOALIAS __forceinline const Mat33 MakeMat33_AxisZX( const Vec4F32 VECREF vz, const Vec4F32 VECREF vx0 )
{
    Mat33 result;
    const Vec4F32 vy( vz ^ vx0 );
    const Vec4F32 vx( vy ^ vz );
    SetRows( &result, vx, vy, vz );
    Normalise( &result );
    return result;
}


/// Create an ortho-normal Mat33 with the z axis pointing along the vector vz
NOALIAS __forceinline const Mat33 MakeMat33_AxisZ( const Vector3 PCREF vz )
{
    return MakeMat33_AxisZ( vz.m_V );
}

/// Create an ortho-normal Mat33 with the z axis pointing along the normal axisZ
NOALIAS __forceinline const Mat33 MakeMat33_AxisZ( const Normal3 PCREF axisZ )
{
    Mat33 result;
    const VF32    cos45deg( Const<VF32>::Sqrt2div2() );
    const Normal3 unitX( Const<Normal3>::AxisX() );
    const Normal3 unitY( Const<Normal3>::AxisY() );
    const VBool   test( Abs( unitY | axisZ ) <= cos45deg );
    const Vector3 vx1( unitY ^ axisZ );
    const Vector3 vy2( axisZ ^ unitX );
    const Vector3 vy1( axisZ ^ vx1 );
    const Vector3 vx2( vy2 ^ axisZ );
    const Normal3 axisX( Normalise( Select( test, vx1, vx2 ) ) );
    const Normal3 axisY( Normalise( Select( test, vy1, vy2 ) ) );
    SetRows( &result, axisX, axisY, axisZ );
    return result;
}


/// Create an ortho-normal Mat33 with the z axis pointing along the vector vz and the y axis in the plane vz^vy
NOALIAS __forceinline const Mat33 MakeMat33_AxisZY( const Vector3 PCREF vz, const Vector3 PCREF vy )
{
    return MakeMat33_AxisZY( vz.m_V, vy.m_V );
}

/// Create an ortho-normal Mat33 with the z axis pointing along the vector vz and the x axis in the plane vz^vx
NOALIAS __forceinline const Mat33 MakeMat33_AxisZX( const Vector3 PCREF vz, const Vector3 PCREF vx )
{
    return MakeMat33_AxisZX( vz.m_V, vx.m_V );
}




/// Normalise the axes of a Mat33 : returns normalised result (NOTE: Does NOT orthogonalise the axes!)
/// (faster but only 22 bit precision)
NOALIAS __forceinline const Mat33 Normalise( const Mat33 MATREF src )
{
    Mat33 result;
    Vec4F32 row_x, row_y, row_z;
    GetRows( src, &row_x, &row_y, &row_z );
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt( length_squared_xyz ) );
    SetRows( &result, row_x * X( rcp_length_xyz ),
                      row_y * Y( rcp_length_xyz ),
                      row_z * Z( rcp_length_xyz ) );
    return result;
}

/// Normalise the axes of a Mat33 : returns normalised result (NOTE: Does NOT orthogonalise the axes!)
/// (slower but full 24 bit precision)
NOALIAS __forceinline const Mat33 Normalise24( const Mat33 MATREF src )
{
    Mat33 result;
    Vec4F32 row_x, row_y, row_z;
    GetRows( src, &row_x, &row_y, &row_z );
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt24( length_squared_xyz ) );
    SetRows( &result, row_x * X( rcp_length_xyz ),
                      row_y * Y( rcp_length_xyz ),
                      row_z * Z( rcp_length_xyz ) );
    return result;
}


/// Normalise the axes of a Mat33 (NOTE: Does NOT orthogonalise the axes!)
/// (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise( Mat33* pDst )
{
    Vec4F32 row_x, row_y, row_z;
    GetRows( *pDst, &row_x, &row_y, &row_z );
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt( length_squared_xyz ) );
    SetRows( pDst, row_x * X( rcp_length_xyz ),
                   row_y * Y( rcp_length_xyz ),
                   row_z * Z( rcp_length_xyz ) );
}

/// Normalise the axes of a Mat33 (NOTE: Does NOT orthogonalise the axes!)
/// (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise24( Mat33* pDst )
{
    Vec4F32 row_x, row_y, row_z;
    GetRows( *pDst, &row_x, &row_y, &row_z );
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt24( length_squared_xyz ) );
    SetRows( pDst, row_x * X( rcp_length_xyz ),
                   row_y * Y( rcp_length_xyz ),
                   row_z * Z( rcp_length_xyz ) );
}


// Cross products for deriving third matrix axis from 2 input axes
// ax = ay ^ az;
// ay = az ^ ax;
// az = ax ^ ay;

/// Ortho-Normalise the axes of a Mat33 : returns normalised result (priority given to z axis then y axis with x axis fully recomputed) 
/// (faster but only 22 bit precision)
NOALIAS __forceinline const Mat33 OrthoNormalise( const Mat33 MATREF src )
{
    Mat33 result;
    Vec4F32 row_x, row_y, row_z;
    GetRows( src, &row_x, &row_y, &row_z );
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
                      row_z * rcp_length_z );
    return result;
}

/// Ortho-Normalise the axes of a Mat33 : returns normalised result (priority given to z axis then y axis with x axis fully recomputed) 
/// (slower but full 24 bit precision)
NOALIAS __forceinline const Mat33 OrthoNormalise24( const Mat33 MATREF src )
{
    Mat33 result;
    Vec4F32 row_x, row_y, row_z;
    GetRows( src, &row_x, &row_y, &row_z );
    row_x = row_y ^ row_z;
    row_y = row_z ^ row_x;
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt24( length_squared_xyz ) );
    const VF32 rcp_length_x( X( rcp_length_xyz ) );
    const VF32 rcp_length_y( Y( rcp_length_xyz ) );
    const VF32 rcp_length_z( Z( rcp_length_xyz ) );
    SetRows( &result, row_x * rcp_length_x,
                      row_y * rcp_length_y,
                      row_z * rcp_length_z );
    return result;
}


/// Ortho-Normalise the axes of a Mat33 (priority given to z axis then y axis with x axis fully recomputed) 
/// (faster but only 22 bit precision)
NOALIAS __forceinline void OrthoNormalise( Mat33* pDst )
{
    Vec4F32 row_x, row_y, row_z;
    GetRows( *pDst, &row_x, &row_y, &row_z );
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
                   row_z * rcp_length_z );
}

/// Ortho-Normalise the axes of a Mat33 (priority given to z axis then y axis with x axis fully recomputed) 
/// (slower but full 24 bit precision)
NOALIAS __forceinline void OrthoNormalise24( Mat33* pDst )
{
    Vec4F32 row_x, row_y, row_z;
    GetRows( *pDst, &row_x, &row_y, &row_z );
    row_x = row_y ^ row_z;
    row_y = row_z ^ row_x;
    const VF32 length_squared_x( LengthSquared3( row_x ) );
    const VF32 length_squared_y( LengthSquared3( row_y ) );
    const VF32 length_squared_z( LengthSquared3( row_z ) );
    const Vec4F32 length_squared_xyz( MakeVec4F32( length_squared_x, length_squared_y, length_squared_z ) );
    const Vec4F32 rcp_length_xyz( RcpSqrt24( length_squared_xyz ) );
    const VF32 rcp_length_x( X( rcp_length_xyz ) );
    const VF32 rcp_length_y( Y( rcp_length_xyz ) );
    const VF32 rcp_length_z( Z( rcp_length_xyz ) );
    SetRows( pDst, row_x * rcp_length_x,
                   row_y * rcp_length_y,
                   row_z * rcp_length_z );
}




/// Transpose Mat33 : returns transposed result
NOALIAS __forceinline const Mat33 Transpose( const Mat33 MATREF src )
{
    Mat33 result;
    #if defined( _SPU_ )
    spu_transpose_matrix33( src.m_RowX, src.m_RowY, src.m_RowZ, &result.m_RowX, &result.m_RowY, &result.m_RowZ );
    #elif defined( _PPU_ )
    vec_transpose_matrix33( src.m_RowX, src.m_RowY, src.m_RowZ, &result.m_RowX, &result.m_RowY, &result.m_RowZ );
    #elif defined( _VMX128_ )
    vmx_transpose_matrix33( src.m_ColX, src.m_ColY, src.m_ColZ, &result.m_ColX, &result.m_ColY, &result.m_ColZ );
    #elif defined( _SSE_ )
    sse_transpose_matrix33( src.m_RowX, src.m_RowY, src.m_RowZ, &result.m_RowX, &result.m_RowY, &result.m_RowZ );
    #else
    for ( S32 r=0; r<3; r++ )
    {
        for ( S32 c=0; c<3; c++ )
        {
            result.m[r][c] = src.m[c][r];
        }
    }
    #endif
    return result;
}

/// Transpose Mat33 : transposes pDst
NOALIAS __forceinline void Transpose( Mat33 * const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_transpose_matrix33( pDst->m_RowX, pDst->m_RowY, pDst->m_RowZ, &pDst->m_RowX, &pDst->m_RowY, &pDst->m_RowZ );
    #elif defined( _PPU_ )
    vec_transpose_matrix33( pDst->m_RowX, pDst->m_RowY, pDst->m_RowZ, &pDst->m_RowX, &pDst->m_RowY, &pDst->m_RowZ );
    #elif defined( _VMX128_ )
    vmx_transpose_matrix33( pDst->m_ColX, pDst->m_ColY, pDst->m_ColZ, &pDst->m_ColX, &pDst->m_ColY, &pDst->m_ColZ );
    #elif defined( _SSE_ )
    sse_transpose_matrix33( pDst->m_RowX, pDst->m_RowY, pDst->m_RowZ, &pDst->m_RowX, &pDst->m_RowY, &pDst->m_RowZ );
    #else
    for ( S32 r=0; r<3; r++ )
    {
        for ( S32 c=0; c<r; c++ )
        {
            const F32 tmp = pDst->m[r][c];
            pDst->m[r][c] = pDst->m[c][r];
            pDst->m[c][r] = tmp;
        }
    }
    #endif
}


/// Trace of Mat33 : sum of diagonal elements
NOALIAS __forceinline const VF32 Trace( const Mat33 MATREF src )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_add( spu_add( spu_splat<VX>( src.m_RowX ),
                                   spu_splat<VY>( src.m_RowY ) ),
                                   spu_splat<VZ>( src.m_RowZ ) );
    #elif defined( _PPU_ )
    result.m_V = vec_add( vec_add( vec_splat( src.m_RowX, VX ),
                                   vec_splat( src.m_RowY, VY ) ),
                                   vec_splat( src.m_RowZ, VZ ) );
    #elif defined( _VMX128_ )
    result.m_V = __vaddfp( __vaddfp( __vspltw( src.m_ColX, VX ),
                                     __vspltw( src.m_ColY, VY ) ),
                                     __vspltw( src.m_ColZ, VZ ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_add_ps( _mm_add_ps( sse_splat<VX>( src.m_RowX ),
                                         sse_splat<VY>( src.m_RowY ) ),
                                         sse_splat<VZ>( src.m_RowZ ) );
    #else
    result.m_V = src.m[0][0] + src.m[1][1] + src.m[2][2];
    #endif
    return result;
}


/// Determinant of Mat33
NOALIAS __forceinline const VF32 Determinant( const Mat33 MATREF src )
{
    VF32 result;
    const Vec4F32 XorNegateY( Const<Vec4F32>::XorNegateY() );
    #if defined( _SPU_ )
    const vector float zero = Const< vector float >::Zero();
    static const int __ = V1W;
    const vector unsigned char shuffle_yxx = spu_shuffle_pattern_word<V0Y,V0X,V0X,__>();
    const vector unsigned char shuffle_zzy = spu_shuffle_pattern_word<V0Z,V0Z,V0Y,__>();
    result.m_V = spu_sum3( spu_xor( spu_mul( src.m_RowX, spu_nmsub( spu_shuffle( src.m_RowY, zero, shuffle_zzy ),
                                                                    spu_shuffle( src.m_RowZ, zero, shuffle_yxx ),
                                                           spu_mul( spu_shuffle( src.m_RowY, zero, shuffle_yxx ),
                                                                    spu_shuffle( src.m_RowZ, zero, shuffle_zzy ) ) ) ), XorNegateY.m_V ) );
    #elif defined( _PPU_ )
    const vector float zero = Const< vector float >::Zero();
    static const int __ = V1W;
    const vector unsigned char permute_yxx = vec_permute_pattern_word<V0Y,V0X,V0X,__>();
    const vector unsigned char permute_zzy = vec_permute_pattern_word<V0Z,V0Z,V0Y,__>();
    result.m_V = vec_sum3( vec_xor( vec_mul( src.m_RowX, vec_nmsub( vec_perm( src.m_RowY, zero, permute_zzy ),
                                                                    vec_perm( src.m_RowZ, zero, permute_yxx ),
                                                           vec_mul( vec_perm( src.m_RowY, zero, permute_yxx ),
                                                                    vec_perm( src.m_RowZ, zero, permute_zzy ) ) ) ), XorNegateY.m_V ) );
    #elif defined( _VMX128_ )
    // NOTE: The determinants of a 3x3 matrix and it's transpose are identical!
    const __vector4 one( Const< __vector4 >::One() );
    static const int __ = VW;
    result.m_V = __vmsum3fp( one, __vxor( __vmulfp( src.m_ColX, __vnmsubfp( __vpermwi( src.m_ColY, VPERMWI_CONST(VZ,VZ,VY,__) ),
                                                                            __vpermwi( src.m_ColZ, VPERMWI_CONST(VY,VX,VX,__) ),
                                                                  __vmulfp( __vpermwi( src.m_ColY, VPERMWI_CONST(VY,VX,VX,__) ),
                                                                            __vpermwi( src.m_ColZ, VPERMWI_CONST(VZ,VZ,VY,__) ) ) ) ), XorNegateY.m_V ) );
    #elif defined( _SSE_ )
    static const int __ = VW;
    result.m_V = sse_sum3( _mm_xor_ps( _mm_mul_ps( src.m_RowX, _mm_sub_ps( _mm_mul_ps( sse_permute<VY,VX,VX,__>( src.m_RowY ),
                                                                                       sse_permute<VZ,VZ,VY,__>( src.m_RowZ ) ),
                                                                           _mm_mul_ps( sse_permute<VZ,VZ,VY,__>( src.m_RowY ),
                                                                                       sse_permute<VY,VX,VX,__>( src.m_RowZ ) ) ) ), XorNegateY.m_V ) );
    #else
    result.m_V = src.m[0][0] * ( src.m[1][1] * src.m[2][2] - src.m[1][2] * src.m[2][1] )
               - src.m[0][1] * ( src.m[1][0] * src.m[2][2] - src.m[1][2] * src.m[2][0] )
               + src.m[0][2] * ( src.m[1][0] * src.m[2][1] - src.m[1][1] * src.m[2][0] );
    #endif
    return result;
}


/// Inverse of Mat33
/// (faster but only 22 bit precision)
NOALIAS __forceinline const Mat33 Inverse( const Mat33 MATREF src )
{
    Mat33 result;
    
    const VF32 rcpDeterminant( Rcp( Determinant( src ) ) );
    
	#if defined( _SPU_ )

    const vector float row_x = spu_nmsub( spu_permute<VZ,VY,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VY,VZ,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 spu_mul( spu_permute<VY,VZ,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VZ,VY,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                         
    const vector float row_y = spu_nmsub( spu_permute<VX,VZ,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VZ,VX,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 spu_mul( spu_permute<VZ,VX,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VX,VZ,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                         
    const vector float row_z = spu_nmsub( spu_permute<VY,VX,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VX,VY,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 spu_mul( spu_permute<VX,VY,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VY,VX,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
    
    result.m_RowX = spu_mul( row_x, rcpDeterminant.m_V );
    result.m_RowY = spu_mul( row_y, rcpDeterminant.m_V );
    result.m_RowZ = spu_mul( row_z, rcpDeterminant.m_V );
    
    #elif defined( _PPU_ )
    
    const vector float row_x = vec_nmsub( vec_permute<VZ,VY,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VY,VZ,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 vec_mul( vec_permute<VY,VZ,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VZ,VY,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                            
    const vector float row_y = vec_nmsub( vec_permute<VX,VZ,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VZ,VX,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 vec_mul( vec_permute<VZ,VX,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VX,VZ,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                            
    const vector float row_z = vec_nmsub( vec_permute<VY,VX,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VX,VY,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 vec_mul( vec_permute<VX,VY,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VY,VX,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
    
    result.m_RowX = vec_mul( row_x, rcpDeterminant.m_V );
    result.m_RowY = vec_mul( row_y, rcpDeterminant.m_V );
    result.m_RowZ = vec_mul( row_z, rcpDeterminant.m_V );
    
    #elif defined( _VMX128_ )
    
    // NOTE: The determinants of a 3x3 matrix and it's transpose are identical!
    
    const __vector4 col_x = __vnmsubfp( vmx_permute<VZ,VY,VZ>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VY,VZ,VY>( src.m_ColZ, src.m_ColZ, src.m_ColY ),
                              __vmulfp( vmx_permute<VY,VZ,VY>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VZ,VY,VZ>( src.m_ColZ, src.m_ColZ, src.m_ColY ) ) );
                                                                                          
    const __vector4 col_y = __vnmsubfp( vmx_permute<VX,VZ,VX>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VZ,VX,VZ>( src.m_ColZ, src.m_ColZ, src.m_ColY ),
                              __vmulfp( vmx_permute<VZ,VX,VZ>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VX,VZ,VX>( src.m_ColZ, src.m_ColZ, src.m_ColY ) ) );
                                                                                          
    const __vector4 col_z = __vnmsubfp( vmx_permute<VY,VX,VY>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VX,VY,VX>( src.m_ColZ, src.m_ColZ, src.m_ColY ),
                              __vmulfp( vmx_permute<VX,VY,VX>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VY,VX,VY>( src.m_ColZ, src.m_ColZ, src.m_ColY ) ) );
    
    result.m_ColX = __vmulfp( col_x, rcpDeterminant.m_V );
    result.m_ColY = __vmulfp( col_y, rcpDeterminant.m_V );
    result.m_ColZ = __vmulfp( col_z, rcpDeterminant.m_V );
    
    #elif defined( _SSE_ )
    
    const __m128 row_x( _mm_sub_ps( _mm_mul_ps( sse_permute<VY,VZ,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VZ,VY,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ),
                                    _mm_mul_ps( sse_permute<VZ,VY,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VY,VZ,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) ) );
    
    const __m128 row_y( _mm_sub_ps( _mm_mul_ps( sse_permute<VZ,VX,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VX,VZ,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ),
                                    _mm_mul_ps( sse_permute<VX,VZ,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VZ,VX,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) ) );
    
    const __m128 row_z( _mm_sub_ps( _mm_mul_ps( sse_permute<VX,VY,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VY,VX,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ),
                                    _mm_mul_ps( sse_permute<VY,VX,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VX,VY,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) ) );
    
    result.m_RowX = _mm_mul_ps( row_x, rcpDeterminant.m_V );
    result.m_RowY = _mm_mul_ps( row_y, rcpDeterminant.m_V );
    result.m_RowZ = _mm_mul_ps( row_z, rcpDeterminant.m_V );
    
    #else
    
    result.m[0][0] = ( src.m[1][1] * src.m[2][2] - src.m[1][2] * src.m[2][1] ) * rcpDeterminant.m_V;
    result.m[0][1] = ( src.m[0][2] * src.m[2][1] - src.m[0][1] * src.m[2][2] ) * rcpDeterminant.m_V;
    result.m[0][2] = ( src.m[0][1] * src.m[1][2] - src.m[0][2] * src.m[1][1] ) * rcpDeterminant.m_V;
                                                                 
    result.m[1][0] = ( src.m[1][2] * src.m[2][0] - src.m[1][0] * src.m[2][2] ) * rcpDeterminant.m_V;
    result.m[1][1] = ( src.m[0][0] * src.m[2][2] - src.m[0][2] * src.m[2][0] ) * rcpDeterminant.m_V;
    result.m[1][2] = ( src.m[0][2] * src.m[1][0] - src.m[0][0] * src.m[1][2] ) * rcpDeterminant.m_V;
                                                                 
    result.m[2][0] = ( src.m[1][0] * src.m[2][1] - src.m[1][1] * src.m[2][0] ) * rcpDeterminant.m_V;
    result.m[2][1] = ( src.m[0][1] * src.m[2][0] - src.m[0][0] * src.m[2][1] ) * rcpDeterminant.m_V;
    result.m[2][2] = ( src.m[0][0] * src.m[1][1] - src.m[0][1] * src.m[1][0] ) * rcpDeterminant.m_V;
    
    #endif
    
    return result;
}


/// Inverse of Mat33
/// (slower but full 24 bit precision)
NOALIAS __forceinline const Mat33 Inverse24( const Mat33 MATREF src )
{
    Mat33 result;
    
    const VF32 rcpDeterminant( Rcp24( Determinant( src ) ) );
    
	#if defined( _SPU_ )

    const vector float row_x = spu_nmsub( spu_permute<VZ,VY,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VY,VZ,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 spu_mul( spu_permute<VY,VZ,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VZ,VY,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                         
    const vector float row_y = spu_nmsub( spu_permute<VX,VZ,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VZ,VX,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 spu_mul( spu_permute<VZ,VX,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VX,VZ,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                         
    const vector float row_z = spu_nmsub( spu_permute<VY,VX,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VX,VY,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 spu_mul( spu_permute<VX,VY,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VY,VX,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
    
    result.m_RowX = spu_mul( row_x, rcpDeterminant.m_V );
    result.m_RowY = spu_mul( row_y, rcpDeterminant.m_V );
    result.m_RowZ = spu_mul( row_z, rcpDeterminant.m_V );
    
    #elif defined( _PPU_ )
    
    const vector float row_x = vec_nmsub( vec_permute<VZ,VY,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VY,VZ,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 vec_mul( vec_permute<VY,VZ,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VZ,VY,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                            
    const vector float row_y = vec_nmsub( vec_permute<VX,VZ,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VZ,VX,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 vec_mul( vec_permute<VZ,VX,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VX,VZ,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                            
    const vector float row_z = vec_nmsub( vec_permute<VY,VX,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VX,VY,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 vec_mul( vec_permute<VX,VY,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VY,VX,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
    
    result.m_RowX = vec_mul( row_x, rcpDeterminant.m_V );
    result.m_RowY = vec_mul( row_y, rcpDeterminant.m_V );
    result.m_RowZ = vec_mul( row_z, rcpDeterminant.m_V );
    
    #elif defined( _VMX128_ )
    
    // NOTE: The determinants of a 3x3 matrix and it's transpose are identical!
    
    const __vector4 col_x = __vnmsubfp( vmx_permute<VZ,VY,VZ>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VY,VZ,VY>( src.m_ColZ, src.m_ColZ, src.m_ColY ),
                              __vmulfp( vmx_permute<VY,VZ,VY>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VZ,VY,VZ>( src.m_ColZ, src.m_ColZ, src.m_ColY ) ) );
                                                                                          
    const __vector4 col_y = __vnmsubfp( vmx_permute<VX,VZ,VX>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VZ,VX,VZ>( src.m_ColZ, src.m_ColZ, src.m_ColY ),
                              __vmulfp( vmx_permute<VZ,VX,VZ>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VX,VZ,VX>( src.m_ColZ, src.m_ColZ, src.m_ColY ) ) );
                                                                                          
    const __vector4 col_z = __vnmsubfp( vmx_permute<VY,VX,VY>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VX,VY,VX>( src.m_ColZ, src.m_ColZ, src.m_ColY ),
                              __vmulfp( vmx_permute<VX,VY,VX>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VY,VX,VY>( src.m_ColZ, src.m_ColZ, src.m_ColY ) ) );
    
    result.m_ColX = __vmulfp( col_x, rcpDeterminant.m_V );
    result.m_ColY = __vmulfp( col_y, rcpDeterminant.m_V );
    result.m_ColZ = __vmulfp( col_z, rcpDeterminant.m_V );
    
    #elif defined( _SSE_ )
    
    const __m128 row_x( _mm_sub_ps( _mm_mul_ps( sse_permute<VY,VZ,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VZ,VY,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ),
                                    _mm_mul_ps( sse_permute<VZ,VY,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VY,VZ,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) ) );
    
    const __m128 row_y( _mm_sub_ps( _mm_mul_ps( sse_permute<VZ,VX,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VX,VZ,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ),
                                    _mm_mul_ps( sse_permute<VX,VZ,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VZ,VX,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) ) );
    
    const __m128 row_z( _mm_sub_ps( _mm_mul_ps( sse_permute<VX,VY,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VY,VX,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ),
                                    _mm_mul_ps( sse_permute<VY,VX,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VX,VY,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) ) );
    
    result.m_RowX = _mm_mul_ps( row_x, rcpDeterminant.m_V );
    result.m_RowY = _mm_mul_ps( row_y, rcpDeterminant.m_V );
    result.m_RowZ = _mm_mul_ps( row_z, rcpDeterminant.m_V );
    
    #else
    
    result.m[0][0] = ( src.m[1][1] * src.m[2][2] - src.m[1][2] * src.m[2][1] ) * rcpDeterminant.m_V;
    result.m[0][1] = ( src.m[0][2] * src.m[2][1] - src.m[0][1] * src.m[2][2] ) * rcpDeterminant.m_V;
    result.m[0][2] = ( src.m[0][1] * src.m[1][2] - src.m[0][2] * src.m[1][1] ) * rcpDeterminant.m_V;
                                                                 
    result.m[1][0] = ( src.m[1][2] * src.m[2][0] - src.m[1][0] * src.m[2][2] ) * rcpDeterminant.m_V;
    result.m[1][1] = ( src.m[0][0] * src.m[2][2] - src.m[0][2] * src.m[2][0] ) * rcpDeterminant.m_V;
    result.m[1][2] = ( src.m[0][2] * src.m[1][0] - src.m[0][0] * src.m[1][2] ) * rcpDeterminant.m_V;
                                                                 
    result.m[2][0] = ( src.m[1][0] * src.m[2][1] - src.m[1][1] * src.m[2][0] ) * rcpDeterminant.m_V;
    result.m[2][1] = ( src.m[0][1] * src.m[2][0] - src.m[0][0] * src.m[2][1] ) * rcpDeterminant.m_V;
    result.m[2][2] = ( src.m[0][0] * src.m[1][1] - src.m[0][1] * src.m[1][0] ) * rcpDeterminant.m_V;
    
    #endif
    
    return result;
}


/// Inverse of Mat33: returns true if matrix was invertible
/// (faster but only 22 bit precision)
//NOALIAS __forceinline const bool Inverse( const Mat33 MATREF src, Mat33* const __restrict pDst )
NOALIAS __forceinline const VBool Inverse( const Mat33 MATREF src, Mat33* const __restrict pDst )
{
    const VF32 epsilon( Const<VF32>::Epsilon() );
    const VF32 det( Determinant( src ) );
    const VF32 rcpDet( Rcp( det ) );
    
	#if defined( _SPU_ )

    const vector float row_x = spu_nmsub( spu_permute<VZ,VY,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VY,VZ,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 spu_mul( spu_permute<VY,VZ,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VZ,VY,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                         
    const vector float row_y = spu_nmsub( spu_permute<VX,VZ,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VZ,VX,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 spu_mul( spu_permute<VZ,VX,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VX,VZ,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                         
    const vector float row_z = spu_nmsub( spu_permute<VY,VX,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VX,VY,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 spu_mul( spu_permute<VX,VY,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VY,VX,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
    
    pDst->m_RowX = spu_mul( row_x, rcpDet.m_V );
    pDst->m_RowY = spu_mul( row_y, rcpDet.m_V );
    pDst->m_RowZ = spu_mul( row_z, rcpDet.m_V );
    
    #elif defined( _PPU_ )
    
    const vector float row_x = vec_nmsub( vec_permute<VZ,VY,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VY,VZ,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 vec_mul( vec_permute<VY,VZ,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VZ,VY,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                            
    const vector float row_y = vec_nmsub( vec_permute<VX,VZ,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VZ,VX,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 vec_mul( vec_permute<VZ,VX,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VX,VZ,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                            
    const vector float row_z = vec_nmsub( vec_permute<VY,VX,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VX,VY,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 vec_mul( vec_permute<VX,VY,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VY,VX,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
    
    pDst->m_RowX = vec_mul( row_x, rcpDet.m_V );
    pDst->m_RowY = vec_mul( row_y, rcpDet.m_V );
    pDst->m_RowZ = vec_mul( row_z, rcpDet.m_V );
    
    #elif defined( _VMX128_ )
    
    // NOTE: The determinants of a 3x3 matrix and it's transpose are identical!
    
    const __vector4 col_x = __vnmsubfp( vmx_permute<VZ,VY,VZ>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VY,VZ,VY>( src.m_ColZ, src.m_ColZ, src.m_ColY ),
                              __vmulfp( vmx_permute<VY,VZ,VY>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VZ,VY,VZ>( src.m_ColZ, src.m_ColZ, src.m_ColY ) ) );
                                                                                          
    const __vector4 col_y = __vnmsubfp( vmx_permute<VX,VZ,VX>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VZ,VX,VZ>( src.m_ColZ, src.m_ColZ, src.m_ColY ),
                              __vmulfp( vmx_permute<VZ,VX,VZ>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VX,VZ,VX>( src.m_ColZ, src.m_ColZ, src.m_ColY ) ) );
                                                                                          
    const __vector4 col_z = __vnmsubfp( vmx_permute<VY,VX,VY>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VX,VY,VX>( src.m_ColZ, src.m_ColZ, src.m_ColY ),
                              __vmulfp( vmx_permute<VX,VY,VX>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VY,VX,VY>( src.m_ColZ, src.m_ColZ, src.m_ColY ) ) );
    
    pDst->m_ColX = __vmulfp( col_x, rcpDet.m_V );
    pDst->m_ColY = __vmulfp( col_y, rcpDet.m_V );
    pDst->m_ColZ = __vmulfp( col_z, rcpDet.m_V );
    
    #elif defined( _SSE_ )
    
    const __m128 row_x( _mm_sub_ps( _mm_mul_ps( sse_permute<VY,VZ,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VZ,VY,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ),
                                    _mm_mul_ps( sse_permute<VZ,VY,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VY,VZ,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) ) );
    
    const __m128 row_y( _mm_sub_ps( _mm_mul_ps( sse_permute<VZ,VX,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VX,VZ,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ),
                                    _mm_mul_ps( sse_permute<VX,VZ,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VZ,VX,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) ) );
    
    const __m128 row_z( _mm_sub_ps( _mm_mul_ps( sse_permute<VX,VY,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VY,VX,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ),
                                    _mm_mul_ps( sse_permute<VY,VX,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VX,VY,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) ) );
    
    pDst->m_RowX = _mm_mul_ps( row_x, rcpDet.m_V );
    pDst->m_RowY = _mm_mul_ps( row_y, rcpDet.m_V );
    pDst->m_RowZ = _mm_mul_ps( row_z, rcpDet.m_V );
    
    #else
    
    pDst->m[0][0] = ( src.m[1][1] * src.m[2][2] - src.m[1][2] * src.m[2][1] ) * rcpDet.m_V;
    pDst->m[0][1] = ( src.m[0][2] * src.m[2][1] - src.m[0][1] * src.m[2][2] ) * rcpDet.m_V;
    pDst->m[0][2] = ( src.m[0][1] * src.m[1][2] - src.m[0][2] * src.m[1][1] ) * rcpDet.m_V;
                                                                 
    pDst->m[1][0] = ( src.m[1][2] * src.m[2][0] - src.m[1][0] * src.m[2][2] ) * rcpDet.m_V;
    pDst->m[1][1] = ( src.m[0][0] * src.m[2][2] - src.m[0][2] * src.m[2][0] ) * rcpDet.m_V;
    pDst->m[1][2] = ( src.m[0][2] * src.m[1][0] - src.m[0][0] * src.m[1][2] ) * rcpDet.m_V;
                                                                 
    pDst->m[2][0] = ( src.m[1][0] * src.m[2][1] - src.m[1][1] * src.m[2][0] ) * rcpDet.m_V;
    pDst->m[2][1] = ( src.m[0][1] * src.m[2][0] - src.m[0][0] * src.m[2][1] ) * rcpDet.m_V;
    pDst->m[2][2] = ( src.m[0][0] * src.m[1][1] - src.m[0][1] * src.m[1][0] ) * rcpDet.m_V;
    
    #endif
    
    return Abs( det ) >= epsilon;
}


/// Inverse of Mat33: returns true if matrix was invertible
/// (slower but full 24 bit precision)
NOALIAS __forceinline const VBool Inverse24( const Mat33 MATREF src, Mat33* const __restrict pDst )
{
    const VF32 epsilon( Const<VF32>::Epsilon() );
    const VF32 det( Determinant( src ) );
    const VF32 rcpDet( Rcp24( det ) );
    
	#if defined( _SPU_ )

    const vector float row_x = spu_nmsub( spu_permute<VZ,VY,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VY,VZ,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 spu_mul( spu_permute<VY,VZ,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VZ,VY,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                         
    const vector float row_y = spu_nmsub( spu_permute<VX,VZ,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VZ,VX,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 spu_mul( spu_permute<VZ,VX,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VX,VZ,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                         
    const vector float row_z = spu_nmsub( spu_permute<VY,VX,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VX,VY,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 spu_mul( spu_permute<VX,VY,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          spu_permute<VY,VX,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
    
    pDst->m_RowX = spu_mul( row_x, rcpDet.m_V );
    pDst->m_RowY = spu_mul( row_y, rcpDet.m_V );
    pDst->m_RowZ = spu_mul( row_z, rcpDet.m_V );
    
    #elif defined( _PPU_ )
    
    const vector float row_x = vec_nmsub( vec_permute<VZ,VY,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VY,VZ,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 vec_mul( vec_permute<VY,VZ,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VZ,VY,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                            
    const vector float row_y = vec_nmsub( vec_permute<VX,VZ,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VZ,VX,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 vec_mul( vec_permute<VZ,VX,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VX,VZ,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
                                                                                            
    const vector float row_z = vec_nmsub( vec_permute<VY,VX,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VX,VY,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ),
                                 vec_mul( vec_permute<VX,VY,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                          vec_permute<VY,VX,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) );
    
    pDst->m_RowX = vec_mul( row_x, rcpDet.m_V );
    pDst->m_RowY = vec_mul( row_y, rcpDet.m_V );
    pDst->m_RowZ = vec_mul( row_z, rcpDet.m_V );
    
    #elif defined( _VMX128_ )
    
    // NOTE: The determinants of a 3x3 matrix and it's transpose are identical!
    
    const __vector4 col_x = __vnmsubfp( vmx_permute<VZ,VY,VZ>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VY,VZ,VY>( src.m_ColZ, src.m_ColZ, src.m_ColY ),
                              __vmulfp( vmx_permute<VY,VZ,VY>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VZ,VY,VZ>( src.m_ColZ, src.m_ColZ, src.m_ColY ) ) );
                                                                                          
    const __vector4 col_y = __vnmsubfp( vmx_permute<VX,VZ,VX>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VZ,VX,VZ>( src.m_ColZ, src.m_ColZ, src.m_ColY ),
                              __vmulfp( vmx_permute<VZ,VX,VZ>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VX,VZ,VX>( src.m_ColZ, src.m_ColZ, src.m_ColY ) ) );
                                                                                          
    const __vector4 col_z = __vnmsubfp( vmx_permute<VY,VX,VY>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VX,VY,VX>( src.m_ColZ, src.m_ColZ, src.m_ColY ),
                              __vmulfp( vmx_permute<VX,VY,VX>( src.m_ColY, src.m_ColX, src.m_ColX ),
                                        vmx_permute<VY,VX,VY>( src.m_ColZ, src.m_ColZ, src.m_ColY ) ) );
    
    pDst->m_ColX = __vmulfp( col_x, rcpDet.m_V );
    pDst->m_ColY = __vmulfp( col_y, rcpDet.m_V );
    pDst->m_ColZ = __vmulfp( col_z, rcpDet.m_V );
    
    #elif defined( _SSE_ )
    
    const __m128 row_x( _mm_sub_ps( _mm_mul_ps( sse_permute<VY,VZ,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VZ,VY,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ),
                                    _mm_mul_ps( sse_permute<VZ,VY,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VY,VZ,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) ) );
    
    const __m128 row_y( _mm_sub_ps( _mm_mul_ps( sse_permute<VZ,VX,VZ>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VX,VZ,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ),
                                    _mm_mul_ps( sse_permute<VX,VZ,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VZ,VX,VZ>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) ) );
    
    const __m128 row_z( _mm_sub_ps( _mm_mul_ps( sse_permute<VX,VY,VX>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VY,VX,VY>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ),
                                    _mm_mul_ps( sse_permute<VY,VX,VY>( src.m_RowY, src.m_RowX, src.m_RowX ),
                                                sse_permute<VX,VY,VX>( src.m_RowZ, src.m_RowZ, src.m_RowY ) ) ) );
    
    pDst->m_RowX = _mm_mul_ps( row_x, rcpDet.m_V );
    pDst->m_RowY = _mm_mul_ps( row_y, rcpDet.m_V );
    pDst->m_RowZ = _mm_mul_ps( row_z, rcpDet.m_V );
    
    #else
    
    pDst->m[0][0] = ( src.m[1][1] * src.m[2][2] - src.m[1][2] * src.m[2][1] ) * rcpDet.m_V;
    pDst->m[0][1] = ( src.m[0][2] * src.m[2][1] - src.m[0][1] * src.m[2][2] ) * rcpDet.m_V;
    pDst->m[0][2] = ( src.m[0][1] * src.m[1][2] - src.m[0][2] * src.m[1][1] ) * rcpDet.m_V;
                                                                 
    pDst->m[1][0] = ( src.m[1][2] * src.m[2][0] - src.m[1][0] * src.m[2][2] ) * rcpDet.m_V;
    pDst->m[1][1] = ( src.m[0][0] * src.m[2][2] - src.m[0][2] * src.m[2][0] ) * rcpDet.m_V;
    pDst->m[1][2] = ( src.m[0][2] * src.m[1][0] - src.m[0][0] * src.m[1][2] ) * rcpDet.m_V;
                                                                 
    pDst->m[2][0] = ( src.m[1][0] * src.m[2][1] - src.m[1][1] * src.m[2][0] ) * rcpDet.m_V;
    pDst->m[2][1] = ( src.m[0][1] * src.m[2][0] - src.m[0][0] * src.m[2][1] ) * rcpDet.m_V;
    pDst->m[2][2] = ( src.m[0][0] * src.m[1][1] - src.m[0][1] * src.m[1][0] ) * rcpDet.m_V;
    
    #endif
    
    return Abs( det ) >= epsilon;
}


/// Inverse transpose of Mat33
NOALIAS __forceinline const Mat33 InverseTranspose( const Mat33 MATREF src )
{
    /// NOTE: Inverse( Transpose( A ) ) == Transpose( Inverse( A ) ) for invertible matrix A
    #if defined( _SIMD_ )
    /// TODO: Optimisation: multiplication by reciprocal of determinant in the inverse cancels out with the multiplication by the determinant here!
    return Inverse( Transpose( src ) ) * Determinant( src );
    #else
    
    Mat33 result;
    
    //const VF32 rcpDeterminant( Rcp( Determinant( src ) ) );
    //
    //result.m[0][0] = ( src.m[1][1] * src.m[2][2] - src.m[2][1] * src.m[1][2] ) * rcpDeterminant.m_V;
    //result.m[0][1] = ( src.m[2][0] * src.m[1][2] - src.m[1][0] * src.m[2][2] ) * rcpDeterminant.m_V;
    //result.m[0][2] = ( src.m[1][0] * src.m[2][1] - src.m[2][0] * src.m[1][1] ) * rcpDeterminant.m_V;
    //                                                                          
    //result.m[1][0] = ( src.m[2][1] * src.m[0][2] - src.m[0][1] * src.m[2][2] ) * rcpDeterminant.m_V;
    //result.m[1][1] = ( src.m[0][0] * src.m[2][2] - src.m[2][0] * src.m[0][2] ) * rcpDeterminant.m_V;
    //result.m[1][2] = ( src.m[2][0] * src.m[0][1] - src.m[0][0] * src.m[2][1] ) * rcpDeterminant.m_V;
    //                                                                          
    //result.m[2][0] = ( src.m[0][1] * src.m[1][2] - src.m[1][1] * src.m[0][2] ) * rcpDeterminant.m_V;
    //result.m[2][1] = ( src.m[1][0] * src.m[0][2] - src.m[0][0] * src.m[1][2] ) * rcpDeterminant.m_V;
    //result.m[2][2] = ( src.m[0][0] * src.m[1][1] - src.m[1][0] * src.m[0][1] ) * rcpDeterminant.m_V;
    
    result.m[0][0] = ( src.m[1][1] * src.m[2][2] - src.m[1][2] * src.m[2][1] );
    result.m[1][0] = ( src.m[0][2] * src.m[2][1] - src.m[0][1] * src.m[2][2] );
    result.m[2][0] = ( src.m[0][1] * src.m[1][2] - src.m[0][2] * src.m[1][1] );
                                                                  
    result.m[0][1] = ( src.m[1][2] * src.m[2][0] - src.m[1][0] * src.m[2][2] );
    result.m[1][1] = ( src.m[0][0] * src.m[2][2] - src.m[0][2] * src.m[2][0] );
    result.m[2][1] = ( src.m[0][2] * src.m[1][0] - src.m[0][0] * src.m[1][2] );
                                                                  
    result.m[0][2] = ( src.m[1][0] * src.m[2][1] - src.m[1][1] * src.m[2][0] );
    result.m[1][2] = ( src.m[0][1] * src.m[2][0] - src.m[0][0] * src.m[2][1] );
    result.m[2][2] = ( src.m[0][0] * src.m[1][1] - src.m[0][1] * src.m[1][0] );
    
    return result;
    
    #endif
}


/// Mat33 Select
NOALIAS __forceinline const Mat33 Select( const VBool PCREF condition, const Mat33 MATREF result_if_true, const Mat33 MATREF result_if_false )
{
    Mat33 result;
    #if defined( _VMX128_ )
    SetRowX( &result, Select( condition, ColX( result_if_true ), ColX( result_if_false ) ) );
    SetRowY( &result, Select( condition, ColY( result_if_true ), ColY( result_if_false ) ) );
    SetRowZ( &result, Select( condition, ColZ( result_if_true ), ColZ( result_if_false ) ) );
    Transpose( &result );
    #else
    SetRowX( &result, Select( condition, RowX( result_if_true ), RowX( result_if_false ) ) );
    SetRowY( &result, Select( condition, RowY( result_if_true ), RowY( result_if_false ) ) );
    SetRowZ( &result, Select( condition, RowZ( result_if_true ), RowZ( result_if_false ) ) );
    #endif
    return result;
}


/// Mat33 maximum absolute value of all elements
NOALIAS __forceinline const VF32 MaxAbsValue( const Mat33 MATREF src )
{
    #if defined( _VMX128_ )
    return Max( Max( MaxAbsValue( ColX( src ) ),
                     MaxAbsValue( ColY( src ) ) ),
                     MaxAbsValue( ColZ( src ) ) );
    #else
    return Max( Max( MaxAbsValue( RowX( src ) ),
                     MaxAbsValue( RowY( src ) ) ),
                     MaxAbsValue( RowZ( src ) ) );
    #endif
}


/// Mat33 equality test with tolerance
NOALIAS __forceinline const VBool Equal( const Mat33 MATREF a, const Mat33 MATREF b, const VF32 VECREF tolerance )
{
    const Vector3 v0( RowX( a ) - RowX( b ) );
    const Vector3 v1( RowY( a ) - RowY( b ) );
    const Vector3 v2( RowZ( a ) - RowZ( b ) );
    
    return ( Length( v0 ) < tolerance ) &
           ( Length( v1 ) < tolerance ) &
           ( Length( v2 ) < tolerance );
}


/// Cramers Rule: Solves for xyz, given 3 simultaneous equations
/// a1.x + b1.y + c1.z = d1,
/// a2.x + b2.y + c2.z = d2,
/// a3.x + b3.y + c3.z = d3
/// (faster but only 22 bit precision)
NOALIAS __forceinline const VBool CramersRule( const Mat33 MATREF abc, const Vector3 PCREF d, Vector3* const __restrict pXYZ )
{
    // http://mathworld.wolfram.com/CramersRule.html
    
    Mat33 dbc( abc ),
          adc( abc ),
          abd( abc );
    
    SetColX( &dbc, d );
    SetColY( &adc, d );
    SetColZ( &abd, d );
    
    const VF32 D( Determinant( abc ) );
    const VF32 rcp_D( Rcp( D ) );
    
    const Vector3 xyz( MakeVector3( Determinant( dbc ) * rcp_D,
                                    Determinant( adc ) * rcp_D,
                                    Determinant( abd ) * rcp_D ) );
    
    *pXYZ = xyz;
    
    return Abs( D ) > Const<VF32>::Epsilon();
}

/// Cramers Rule: Solves for xyz, given 3 simultaneous equations
/// a1.x + b1.y + c1.z = d1,
/// a2.x + b2.y + c2.z = d2,
/// a3.x + b3.y + c3.z = d3
/// (slower but full 24 bit precision)
NOALIAS __forceinline const VBool CramersRule24( const Mat33 MATREF abc, const Vector3 PCREF d, Vector3* const __restrict pXYZ )
{
    // http://mathworld.wolfram.com/CramersRule.html
    
    Mat33 dbc( abc ),
          adc( abc ),
          abd( abc );
    
    SetColX( &dbc, d );
    SetColY( &adc, d );
    SetColZ( &abd, d );
    
    const VF32 D( Determinant( abc ) );
    const VF32 rcp_D( Rcp24( D ) );
    
    const Vector3 xyz( MakeVector3( Determinant( dbc ) * rcp_D,
                                    Determinant( adc ) * rcp_D,
                                    Determinant( abd ) * rcp_D ) );
    
    *pXYZ = xyz;
    
    return Abs( D ) > Const<VF32>::Epsilon();
}






/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


