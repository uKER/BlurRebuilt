#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT44_INL
#define SOFT_MATHS_HVEC_NEW_MAT44_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT44_INL
#define MATHS_HVEC_NEW_MAT44_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat44
/// @{

/// Load Mat44 from an F32 array
/// NOTE: Matrix transposition is platform dependent!
template <>
NOALIAS __forceinline const Mat44 Load( F32 const * const __restrict pSrc )
{
    Mat44 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_load_unaligned( &pSrc[0] );
    result.m_RowY = spu_load_unaligned( &pSrc[4] );
    result.m_RowZ = spu_load_unaligned( &pSrc[8] );
    result.m_RowW = spu_load_unaligned( &pSrc[12] );
    #elif defined( _PPU_ )
    result.m_RowX = vec_load_unaligned( &pSrc[0] );
    result.m_RowY = vec_load_unaligned( &pSrc[4] );
    result.m_RowZ = vec_load_unaligned( &pSrc[8] );
    result.m_RowW = vec_load_unaligned( &pSrc[12] );
    #elif defined( _VMX128_ )
    result.m_ColX = vmx_load_unaligned( &pSrc[0] );
    result.m_ColY = vmx_load_unaligned( &pSrc[4] );
    result.m_ColZ = vmx_load_unaligned( &pSrc[8] );
    result.m_ColW = vmx_load_unaligned( &pSrc[12] );
    #elif defined( _SSE_ )
    result.m_RowX = sse_load_unaligned( &pSrc[0] );
    result.m_RowY = sse_load_unaligned( &pSrc[4] );
    result.m_RowZ = sse_load_unaligned( &pSrc[8] );
    result.m_RowW = sse_load_unaligned( &pSrc[12] );
    #else
    result.m[0][0] = pSrc[0];  result.m[0][1] = pSrc[1];  result.m[0][2] = pSrc[2];  result.m[0][2] = pSrc[3];
    result.m[1][0] = pSrc[4];  result.m[1][1] = pSrc[5];  result.m[1][2] = pSrc[6];  result.m[1][2] = pSrc[7];
    result.m[2][0] = pSrc[8];  result.m[2][1] = pSrc[9];  result.m[2][2] = pSrc[10]; result.m[2][2] = pSrc[11];
    result.m[3][0] = pSrc[12]; result.m[3][1] = pSrc[13]; result.m[3][2] = pSrc[14]; result.m[3][2] = pSrc[15];
    #endif
    return result;
}

/// Store Mat44 into an F32 array
/// NOTE: Matrix transposition is platform dependent!
NOALIAS __forceinline void Store( const Mat44 MATREF src, F32* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec4( src.m_RowX, &pDst[0] );
    spu_store_unaligned_vec4( src.m_RowY, &pDst[4] );
    spu_store_unaligned_vec4( src.m_RowZ, &pDst[8] );
    spu_store_unaligned_vec4( src.m_RowW, &pDst[12] );
    #elif defined( _PPU_ )
    vec_store_unaligned_vec4( src.m_RowX, &pDst[0] );
    vec_store_unaligned_vec4( src.m_RowY, &pDst[4] );
    vec_store_unaligned_vec4( src.m_RowZ, &pDst[8] );
    vec_store_unaligned_vec4( src.m_RowW, &pDst[12] );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec4( src.m_ColX, &pDst[0] );
    vmx_store_unaligned_vec4( src.m_ColY, &pDst[4] );
    vmx_store_unaligned_vec4( src.m_ColZ, &pDst[8] );
    vmx_store_unaligned_vec4( src.m_ColW, &pDst[12] );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec4( src.m_RowX, &pDst[0] );
    sse_store_unaligned_vec4( src.m_RowY, &pDst[4] );
    sse_store_unaligned_vec4( src.m_RowZ, &pDst[8] );
    sse_store_unaligned_vec4( src.m_RowW, &pDst[12] );
    #else
    pDst[0]  = src.m[0][0]; pDst[1]  = src.m[0][1]; pDst[2]  = src.m[0][2]; pDst[3]  = src.m[0][2];
    pDst[4]  = src.m[1][0]; pDst[5]  = src.m[1][1]; pDst[6]  = src.m[1][2]; pDst[7]  = src.m[1][2];
    pDst[8]  = src.m[2][0]; pDst[9]  = src.m[2][1]; pDst[10] = src.m[2][2]; pDst[11] = src.m[2][2];
    pDst[12] = src.m[3][0]; pDst[13] = src.m[3][1]; pDst[14] = src.m[3][2]; pDst[15] = src.m[3][2];
    #endif
}

/// Load Mat44 from a Float44
/// NOTE: Matrix transposition is platform dependent!
template <>
NOALIAS __forceinline const Mat44 Load( MathsStorage::Float44 const * const __restrict pSrc )
{
    Mat44 result;
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
    result.m_ColX = vmx_load_unaligned( &pSrc->cx.x );
    result.m_ColY = vmx_load_unaligned( &pSrc->cy.x );
    result.m_ColZ = vmx_load_unaligned( &pSrc->cz.x );
    result.m_ColW = vmx_load_unaligned( &pSrc->cw.x );
    #elif defined( _SSE_ )
    result.m_RowX = sse_load_unaligned( &pSrc->rx.x );
    result.m_RowY = sse_load_unaligned( &pSrc->ry.x );
    result.m_RowZ = sse_load_unaligned( &pSrc->rz.x );
    result.m_RowW = sse_load_unaligned( &pSrc->rw.x );
    #else
    result.m[0][0] = pSrc->rx.x; result.m[0][1] = pSrc->rx.y; result.m[0][2] = pSrc->rx.z; result.m[0][3] = pSrc->rx.w;
    result.m[1][0] = pSrc->ry.x; result.m[1][1] = pSrc->ry.y; result.m[1][2] = pSrc->ry.z; result.m[1][3] = pSrc->ry.w;
    result.m[2][0] = pSrc->rz.x; result.m[2][1] = pSrc->rz.y; result.m[2][2] = pSrc->rz.z; result.m[2][3] = pSrc->rz.w;
    result.m[3][0] = pSrc->rw.x; result.m[3][1] = pSrc->rw.y; result.m[3][2] = pSrc->rw.z; result.m[3][3] = pSrc->rw.w;
    #endif
    return result;
}

/// Store Mat44 into a Float44
/// NOTE: Matrix transposition is platform dependent!
NOALIAS __forceinline void Store( const Mat44 MATREF src, MathsStorage::Float44* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec4( src.m_RowX, &pDst->rx.x );
    spu_store_unaligned_vec4( src.m_RowY, &pDst->ry.x );
    spu_store_unaligned_vec4( src.m_RowZ, &pDst->rz.x );
    spu_store_unaligned_vec4( src.m_RowW, &pDst->rw.x );
    #elif defined( _PPU_ )
    vec_store_unaligned_vec4( src.m_RowX, &pDst->rx.x );
    vec_store_unaligned_vec4( src.m_RowY, &pDst->ry.x );
    vec_store_unaligned_vec4( src.m_RowZ, &pDst->rz.x );
    vec_store_unaligned_vec4( src.m_RowW, &pDst->rw.x );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec4( src.m_ColX, &pDst->cx.x );
    vmx_store_unaligned_vec4( src.m_ColY, &pDst->cy.x );
    vmx_store_unaligned_vec4( src.m_ColZ, &pDst->cz.x );
    vmx_store_unaligned_vec4( src.m_ColW, &pDst->cw.x );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec4( src.m_RowX, &pDst->rx.x );
    sse_store_unaligned_vec4( src.m_RowY, &pDst->ry.x );
    sse_store_unaligned_vec4( src.m_RowZ, &pDst->rz.x );
    sse_store_unaligned_vec4( src.m_RowW, &pDst->rw.x );
    #else
    pDst->rx.x = src.m[0][0]; pDst->rx.y = src.m[0][1]; pDst->rx.z = src.m[0][2]; pDst->rx.w = src.m[0][3];
    pDst->ry.x = src.m[1][0]; pDst->ry.y = src.m[1][1]; pDst->ry.z = src.m[1][2]; pDst->ry.w = src.m[1][3];
    pDst->rz.x = src.m[2][0]; pDst->rz.y = src.m[2][1]; pDst->rz.z = src.m[2][2]; pDst->rz.w = src.m[2][3];
    pDst->rw.x = src.m[3][0]; pDst->rw.y = src.m[3][1]; pDst->rw.z = src.m[3][2]; pDst->rw.w = src.m[3][3];
    #endif
}


/// Load Mat44 from a MatrixRow44
template <>
NOALIAS __forceinline const Mat44 Load( MathsStorage::MatrixRow44 const * const __restrict pSrc )
{
    Mat44 result;
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
    const __vector4 rowX( vmx_load_unaligned( &pSrc->rx.x ) );
    const __vector4 rowY( vmx_load_unaligned( &pSrc->ry.x ) );
    const __vector4 rowZ( vmx_load_unaligned( &pSrc->rz.x ) );
    const __vector4 rowW( vmx_load_unaligned( &pSrc->rw.x ) );
    vmx_transpose_matrix44( rowX, rowY, rowZ, rowW, &result.m_ColX, &result.m_ColY, &result.m_ColZ, &result.m_ColW );
    #elif defined( _SSE_ )
    result.m_RowX = sse_load_unaligned( &pSrc->rx.x );
    result.m_RowY = sse_load_unaligned( &pSrc->ry.x );
    result.m_RowZ = sse_load_unaligned( &pSrc->rz.x );
    result.m_RowW = sse_load_unaligned( &pSrc->rw.x );
    #else
    result.m[0][0] = pSrc->rx.x; result.m[0][1] = pSrc->rx.y; result.m[0][2] = pSrc->rx.z; result.m[0][3] = pSrc->rx.w;
    result.m[1][0] = pSrc->ry.x; result.m[1][1] = pSrc->ry.y; result.m[1][2] = pSrc->ry.z; result.m[1][3] = pSrc->ry.w;
    result.m[2][0] = pSrc->rz.x; result.m[2][1] = pSrc->rz.y; result.m[2][2] = pSrc->rz.z; result.m[2][3] = pSrc->rz.w;
    result.m[3][0] = pSrc->rw.x; result.m[3][1] = pSrc->rw.y; result.m[3][2] = pSrc->rw.z; result.m[3][3] = pSrc->rw.w;
    #endif
    return result;
}

/// Store Mat44 into a MatrixRow44
NOALIAS __forceinline void Store( const Mat44 MATREF src, MathsStorage::MatrixRow44* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec4( src.m_RowX, &pDst->rx.x );
    spu_store_unaligned_vec4( src.m_RowY, &pDst->ry.x );
    spu_store_unaligned_vec4( src.m_RowZ, &pDst->rz.x );
    spu_store_unaligned_vec4( src.m_RowW, &pDst->rw.x );
    #elif defined( _PPU_ )
    vec_store_unaligned_vec4( src.m_RowX, &pDst->rx.x );
    vec_store_unaligned_vec4( src.m_RowY, &pDst->ry.x );
    vec_store_unaligned_vec4( src.m_RowZ, &pDst->rz.x );
    vec_store_unaligned_vec4( src.m_RowW, &pDst->rw.x );
    #elif defined( _VMX128_ )
    __vector4 rowX, rowY, rowZ, rowW;
    vmx_transpose_matrix44( src.m_ColX, src.m_ColY, src.m_ColZ, src.m_ColW, &rowX, &rowY, &rowZ, &rowW );
    vmx_store_unaligned_vec4( rowX, &pDst->rx.x );
    vmx_store_unaligned_vec4( rowY, &pDst->ry.x );
    vmx_store_unaligned_vec4( rowZ, &pDst->rz.x );
    vmx_store_unaligned_vec4( rowW, &pDst->rw.x );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec4( src.m_RowX, &pDst->rx.x );
    sse_store_unaligned_vec4( src.m_RowY, &pDst->ry.x );
    sse_store_unaligned_vec4( src.m_RowZ, &pDst->rz.x );
    sse_store_unaligned_vec4( src.m_RowW, &pDst->rw.x );
    #else
    pDst->rx.x = src.m[0][0]; pDst->rx.y = src.m[0][1]; pDst->rx.z = src.m[0][2]; pDst->rx.w = src.m[0][3];
    pDst->ry.x = src.m[1][0]; pDst->ry.y = src.m[1][1]; pDst->ry.z = src.m[1][2]; pDst->ry.w = src.m[1][3];
    pDst->rz.x = src.m[2][0]; pDst->rz.y = src.m[2][1]; pDst->rz.z = src.m[2][2]; pDst->rz.w = src.m[2][3];
    pDst->rw.x = src.m[3][0]; pDst->rw.y = src.m[3][1]; pDst->rw.z = src.m[3][2]; pDst->rw.w = src.m[3][3];
    #endif
}


/// Load Mat44 from a MatrixCol44
template <>
NOALIAS __forceinline const Mat44 Load( MathsStorage::MatrixCol44 const * const __restrict pSrc )
{
    Mat44 result;
    #if defined( _SPU_ )
    const vector float colX( spu_load_unaligned( &pSrc->cx.x ) );
    const vector float colY( spu_load_unaligned( &pSrc->cy.x ) );
    const vector float colZ( spu_load_unaligned( &pSrc->cz.x ) );
    const vector float colW( spu_load_unaligned( &pSrc->cw.x ) );
    spu_transpose_matrix44( colX, colY, colZ, colW, &result.m_RowX, &result.m_RowY, &result.m_RowZ, &result.m_RowW );
    #elif defined( _PPU_ )
    const vector float colX( vec_load_unaligned( &pSrc->cx.x ) );
    const vector float colY( vec_load_unaligned( &pSrc->cy.x ) );
    const vector float colZ( vec_load_unaligned( &pSrc->cz.x ) );
    const vector float colW( vec_load_unaligned( &pSrc->cw.x ) );
    vec_transpose_matrix44( colX, colY, colZ, colW, &result.m_RowX, &result.m_RowY, &result.m_RowZ, &result.m_RowW );
    #elif defined( _VMX128_ )
    result.m_ColX = vmx_load_unaligned( &pSrc->cx.x );
    result.m_ColY = vmx_load_unaligned( &pSrc->cy.x );
    result.m_ColZ = vmx_load_unaligned( &pSrc->cz.x );
    result.m_ColW = vmx_load_unaligned( &pSrc->cw.x );
    #elif defined( _SSE_ )
    const __m128 colX( sse_load_unaligned( &pSrc->cx.x ) );
    const __m128 colY( sse_load_unaligned( &pSrc->cy.x ) );
    const __m128 colZ( sse_load_unaligned( &pSrc->cz.x ) );
    const __m128 colW( sse_load_unaligned( &pSrc->cw.x ) );
    sse_transpose_matrix44( colX, colY, colZ, colW, &result.m_RowX, &result.m_RowY, &result.m_RowZ, &result.m_RowW );
    #else
    result.m[0][0] = pSrc->cx.x; result.m[0][1] = pSrc->cy.x; result.m[0][2] = pSrc->cz.x; result.m[0][3] = pSrc->cw.x;
    result.m[1][0] = pSrc->cx.y; result.m[1][1] = pSrc->cy.y; result.m[1][2] = pSrc->cz.y; result.m[1][3] = pSrc->cw.y;
    result.m[2][0] = pSrc->cx.z; result.m[2][1] = pSrc->cy.z; result.m[2][2] = pSrc->cz.z; result.m[2][3] = pSrc->cw.z;
    result.m[3][0] = pSrc->cx.w; result.m[3][1] = pSrc->cy.w; result.m[3][2] = pSrc->cz.w; result.m[3][3] = pSrc->cw.w;
    #endif
    return result;
}

/// Store Mat44 into a MatrixCol44
NOALIAS __forceinline void Store( const Mat44 MATREF src, MathsStorage::MatrixCol44* const __restrict pDst )
{
    #if defined( _SPU_ )
    vector float colX, colY, colZ, colW;
    spu_transpose_matrix44( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW, &colX, &colY, &colZ, &colW );
    spu_store_unaligned_vec4( colX, &pDst->cx.x );
    spu_store_unaligned_vec4( colY, &pDst->cy.x );
    spu_store_unaligned_vec4( colZ, &pDst->cz.x );
    spu_store_unaligned_vec4( colW, &pDst->cw.x );
    #elif defined( _PPU_ )
    vector float colX, colY, colZ, colW;
    vec_transpose_matrix44( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW, &colX, &colY, &colZ, &colW );
    vec_store_unaligned_vec4( colX, &pDst->cx.x );
    vec_store_unaligned_vec4( colY, &pDst->cy.x );
    vec_store_unaligned_vec4( colZ, &pDst->cz.x );
    vec_store_unaligned_vec4( colW, &pDst->cw.x );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec4( src.m_ColX, &pDst->cx.x );
    vmx_store_unaligned_vec4( src.m_ColY, &pDst->cy.x );
    vmx_store_unaligned_vec4( src.m_ColZ, &pDst->cz.x );
    vmx_store_unaligned_vec4( src.m_ColW, &pDst->cw.x );
    #elif defined( _SSE_ )
    __m128 colX, colY, colZ, colW;
    sse_transpose_matrix44( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW, &colX, &colY, &colZ, &colW );
    sse_store_unaligned_vec4( colX, &pDst->cx.x );
    sse_store_unaligned_vec4( colY, &pDst->cy.x );
    sse_store_unaligned_vec4( colZ, &pDst->cz.x );
    sse_store_unaligned_vec4( colW, &pDst->cw.x );
    #else
    pDst->cx.x = src.m[0][0]; pDst->cy.x = src.m[0][1]; pDst->cz.x = src.m[0][2]; pDst->cw.x = src.m[0][3];
    pDst->cx.y = src.m[1][0]; pDst->cy.y = src.m[1][1]; pDst->cz.y = src.m[1][2]; pDst->cw.y = src.m[1][3];
    pDst->cx.z = src.m[2][0]; pDst->cy.z = src.m[2][1]; pDst->cz.z = src.m[2][2]; pDst->cw.z = src.m[2][3];
    pDst->cx.w = src.m[3][0]; pDst->cy.w = src.m[3][1]; pDst->cz.w = src.m[3][2]; pDst->cw.w = src.m[3][3];
    #endif
}


/// Load Mat44 from a MatrixRow44A16
template <>
NOALIAS __forceinline const Mat44 Load( MathsStorage::MatrixRow44A16 const * const __restrict pSrc )
{
    Mat44 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_load_aligned( &pSrc->rx.x );
    result.m_RowY = spu_load_aligned( &pSrc->ry.x );
    result.m_RowZ = spu_load_aligned( &pSrc->rz.x );
    result.m_RowW = spu_load_aligned( &pSrc->rw.x );
    #elif defined( _PPU_ )
    result.m_RowX = vec_load_aligned( &pSrc->rx.x );
    result.m_RowY = vec_load_aligned( &pSrc->ry.x );
    result.m_RowZ = vec_load_aligned( &pSrc->rz.x );
    result.m_RowW = vec_load_aligned( &pSrc->rw.x );
    #elif defined( _VMX128_ )
    const __vector4 rowX( vmx_load_aligned( &pSrc->rx.x ) );
    const __vector4 rowY( vmx_load_aligned( &pSrc->ry.x ) );
    const __vector4 rowZ( vmx_load_aligned( &pSrc->rz.x ) );
    const __vector4 rowW( vmx_load_aligned( &pSrc->rw.x ) );
    vmx_transpose_matrix44( rowX, rowY, rowZ, rowW, &result.m_ColX, &result.m_ColY, &result.m_ColZ, &result.m_ColW );
    #elif defined( _SSE_ )
    result.m_RowX = sse_load_aligned( &pSrc->rx.x );
    result.m_RowY = sse_load_aligned( &pSrc->ry.x );
    result.m_RowZ = sse_load_aligned( &pSrc->rz.x );
    result.m_RowW = sse_load_aligned( &pSrc->rw.x );
    #else
    result.m[0][0] = pSrc->rx.x; result.m[0][1] = pSrc->rx.y; result.m[0][2] = pSrc->rx.z; result.m[0][3] = pSrc->rx.w;
    result.m[1][0] = pSrc->ry.x; result.m[1][1] = pSrc->ry.y; result.m[1][2] = pSrc->ry.z; result.m[1][3] = pSrc->ry.w;
    result.m[2][0] = pSrc->rz.x; result.m[2][1] = pSrc->rz.y; result.m[2][2] = pSrc->rz.z; result.m[2][3] = pSrc->rz.w;
    result.m[3][0] = pSrc->rw.x; result.m[3][1] = pSrc->rw.y; result.m[3][2] = pSrc->rw.z; result.m[3][3] = pSrc->rw.w;
    #endif
    return result;
}

/// Store Mat44 into a MatrixRow44A16
NOALIAS __forceinline void Store( const Mat44 MATREF src, MathsStorage::MatrixRow44A16* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_aligned_vec4( src.m_RowX, &pDst->rx.x );
    spu_store_aligned_vec4( src.m_RowY, &pDst->ry.x );
    spu_store_aligned_vec4( src.m_RowZ, &pDst->rz.x );
    spu_store_aligned_vec4( src.m_RowW, &pDst->rw.x );
    #elif defined( _PPU_ )
    vec_store_aligned_vec4( src.m_RowX, &pDst->rx.x );
    vec_store_aligned_vec4( src.m_RowY, &pDst->ry.x );
    vec_store_aligned_vec4( src.m_RowZ, &pDst->rz.x );
    vec_store_aligned_vec4( src.m_RowW, &pDst->rw.x );
    #elif defined( _VMX128_ )
    __vector4 rowX, rowY, rowZ, rowW;
    vmx_transpose_matrix44( src.m_ColX, src.m_ColY, src.m_ColZ, src.m_ColW, &rowX, &rowY, &rowZ, &rowW );
    vmx_store_aligned_vec4( rowX, &pDst->rx.x );
    vmx_store_aligned_vec4( rowY, &pDst->ry.x );
    vmx_store_aligned_vec4( rowZ, &pDst->rz.x );
    vmx_store_aligned_vec4( rowW, &pDst->rw.x );
    #elif defined( _SSE_ )
    sse_store_aligned_vec4( src.m_RowX, &pDst->rx.x );
    sse_store_aligned_vec4( src.m_RowY, &pDst->ry.x );
    sse_store_aligned_vec4( src.m_RowZ, &pDst->rz.x );
    sse_store_aligned_vec4( src.m_RowW, &pDst->rw.x );
    #else
    pDst->rx.x = src.m[0][0]; pDst->rx.y = src.m[0][1]; pDst->rx.z = src.m[0][2]; pDst->rx.w = src.m[0][3];
    pDst->ry.x = src.m[1][0]; pDst->ry.y = src.m[1][1]; pDst->ry.z = src.m[1][2]; pDst->ry.w = src.m[1][3];
    pDst->rz.x = src.m[2][0]; pDst->rz.y = src.m[2][1]; pDst->rz.z = src.m[2][2]; pDst->rz.w = src.m[2][3];
    pDst->rw.x = src.m[3][0]; pDst->rw.y = src.m[3][1]; pDst->rw.z = src.m[3][2]; pDst->rw.w = src.m[3][3];
    #endif
}


/// Load Mat44 from a MatrixCol44A16
template <>
NOALIAS __forceinline const Mat44 Load( MathsStorage::MatrixCol44A16 const * const __restrict pSrc )
{
    Mat44 result;
    #if defined( _SPU_ )
    const vector float colX( spu_load_aligned( &pSrc->cx.x ) );
    const vector float colY( spu_load_aligned( &pSrc->cy.x ) );
    const vector float colZ( spu_load_aligned( &pSrc->cz.x ) );
    const vector float colW( spu_load_aligned( &pSrc->cw.x ) );
    spu_transpose_matrix44( colX, colY, colZ, colW, &result.m_RowX, &result.m_RowY, &result.m_RowZ, &result.m_RowW );
    #elif defined( _PPU_ )
    const vector float colX( vec_load_aligned( &pSrc->cx.x ) );
    const vector float colY( vec_load_aligned( &pSrc->cy.x ) );
    const vector float colZ( vec_load_aligned( &pSrc->cz.x ) );
    const vector float colW( vec_load_aligned( &pSrc->cw.x ) );
    vec_transpose_matrix44( colX, colY, colZ, colW, &result.m_RowX, &result.m_RowY, &result.m_RowZ, &result.m_RowW );
    #elif defined( _VMX128_ )
    result.m_ColX = vmx_load_aligned( &pSrc->cx.x );
    result.m_ColY = vmx_load_aligned( &pSrc->cy.x );
    result.m_ColZ = vmx_load_aligned( &pSrc->cz.x );
    result.m_ColW = vmx_load_aligned( &pSrc->cw.x );
    #elif defined( _SSE_ )
    const __m128 colX( sse_load_aligned( &pSrc->cx.x ) );
    const __m128 colY( sse_load_aligned( &pSrc->cy.x ) );
    const __m128 colZ( sse_load_aligned( &pSrc->cz.x ) );
    const __m128 colW( sse_load_aligned( &pSrc->cw.x ) );
    sse_transpose_matrix44( colX, colY, colZ, colW, &result.m_RowX, &result.m_RowY, &result.m_RowZ, &result.m_RowW );
    #else
    result.m[0][0] = pSrc->cx.x; result.m[0][1] = pSrc->cy.x; result.m[0][2] = pSrc->cz.x; result.m[0][3] = pSrc->cw.x;
    result.m[1][0] = pSrc->cx.y; result.m[1][1] = pSrc->cy.y; result.m[1][2] = pSrc->cz.y; result.m[1][3] = pSrc->cw.y;
    result.m[2][0] = pSrc->cx.z; result.m[2][1] = pSrc->cy.z; result.m[2][2] = pSrc->cz.z; result.m[2][3] = pSrc->cw.z;
    result.m[3][0] = pSrc->cx.w; result.m[3][1] = pSrc->cy.w; result.m[3][2] = pSrc->cz.w; result.m[3][3] = pSrc->cw.w;
    #endif
    return result;
}

/// Store Mat44 into a MatrixCol44A16
NOALIAS __forceinline void Store( const Mat44 MATREF src, MathsStorage::MatrixCol44A16* const __restrict pDst )
{
    #if defined( _SPU_ )
    vector float colX, colY, colZ, colW;
    spu_transpose_matrix44( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW, &colX, &colY, &colZ, &colW );
    spu_store_aligned_vec4( colX, &pDst->cx.x );
    spu_store_aligned_vec4( colY, &pDst->cy.x );
    spu_store_aligned_vec4( colZ, &pDst->cz.x );
    spu_store_aligned_vec4( colW, &pDst->cw.x );
    #elif defined( _PPU_ )
    vector float colX, colY, colZ, colW;
    vec_transpose_matrix44( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW, &colX, &colY, &colZ, &colW );
    vec_store_aligned_vec4( colX, &pDst->cx.x );
    vec_store_aligned_vec4( colY, &pDst->cy.x );
    vec_store_aligned_vec4( colZ, &pDst->cz.x );
    vec_store_aligned_vec4( colW, &pDst->cw.x );
    #elif defined( _VMX128_ )
    vmx_store_aligned_vec4( src.m_ColX, &pDst->cx.x );
    vmx_store_aligned_vec4( src.m_ColY, &pDst->cy.x );
    vmx_store_aligned_vec4( src.m_ColZ, &pDst->cz.x );
    vmx_store_aligned_vec4( src.m_ColW, &pDst->cw.x );
    #elif defined( _SSE_ )
    __m128 colX, colY, colZ, colW;
    sse_transpose_matrix44( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW, &colX, &colY, &colZ, &colW );
    sse_store_aligned_vec4( colX, &pDst->cx.x );
    sse_store_aligned_vec4( colY, &pDst->cy.x );
    sse_store_aligned_vec4( colZ, &pDst->cz.x );
    sse_store_aligned_vec4( colW, &pDst->cw.x );
    #else
    pDst->cx.x = src.m[0][0]; pDst->cy.x = src.m[0][1]; pDst->cz.x = src.m[0][2]; pDst->cw.x = src.m[0][3];
    pDst->cx.y = src.m[1][0]; pDst->cy.y = src.m[1][1]; pDst->cz.y = src.m[1][2]; pDst->cw.y = src.m[1][3];
    pDst->cx.z = src.m[2][0]; pDst->cy.z = src.m[2][1]; pDst->cz.z = src.m[2][2]; pDst->cw.z = src.m[2][3];
    pDst->cx.w = src.m[3][0]; pDst->cy.w = src.m[3][1]; pDst->cz.w = src.m[3][2]; pDst->cw.w = src.m[3][3];
    #endif
}


NOALIAS __forceinline const Mat44& Const< Mat44 >::Zero()     { return m_Zero; }
NOALIAS __forceinline const Mat44& Const< Mat44 >::Identity() { return m_Identity; }


/// Make Mat44 from 16 float elements (row major)
NOALIAS __forceinline const Mat44 MakeMat44( const F32& m00, const F32& m01, const F32& m02, const F32& m03,
                                             const F32& m10, const F32& m11, const F32& m12, const F32& m13,
                                             const F32& m20, const F32& m21, const F32& m22, const F32& m23,
                                             const F32& m30, const F32& m31, const F32& m32, const F32& m33 )
{
    #if defined( _SIMD_ )
    #if defined( _VMX128_ )
    const F32 m[16] = { m00, m10, m20, m30,
                        m01, m11, m21, m31,
                        m02, m12, m22, m32,
                        m03, m13, m23, m33 };
    #else
    const F32 m[16] = { m00, m01, m02, m03,
                        m10, m11, m12, m13,
                        m20, m21, m22, m23,
                        m30, m31, m32, m33 };
    #endif
    return Load<Mat44>( m );
    #else
    Mat44 result;
    result.m[0][0] = m00; result.m[0][1] = m01; result.m[0][2] = m02; result.m[0][3] = m03;
    result.m[1][0] = m10; result.m[1][1] = m11; result.m[1][2] = m12; result.m[1][3] = m13;
    result.m[2][0] = m20; result.m[2][1] = m21; result.m[2][2] = m22; result.m[2][3] = m23;
    result.m[3][0] = m30; result.m[3][1] = m31; result.m[3][2] = m32; result.m[3][3] = m33;
    return result;
    #endif
}


/// Make Mat44 from 4 row vectors (4 element)
NOALIAS __forceinline const Mat44 MakeMat44( const Vec4F32 VECREF rowX, const Vec4F32 VECREF rowY, const Vec4F32 VECREF rowZ, const Vec4F32 VECREF rowW )
{
    Mat44 result;
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    result.m_RowX = rowX.m_V;
    result.m_RowY = rowY.m_V;
    result.m_RowZ = rowZ.m_V;
    result.m_RowW = rowW.m_V;
    #elif defined( _VMX128_ )
    vmx_transpose_matrix44( rowX.m_V, rowY.m_V, rowZ.m_V, rowW.m_V, &result.m_ColX, &result.m_ColY, &result.m_ColZ, &result.m_ColW );
    #else
    result.m[0][0] = rowX.m_V[0]; result.m[0][1] = rowX.m_V[1]; result.m[0][2] = rowX.m_V[2]; result.m[0][3] = rowX.m_V[3];
    result.m[1][0] = rowY.m_V[0]; result.m[1][1] = rowY.m_V[1]; result.m[1][2] = rowY.m_V[2]; result.m[1][3] = rowY.m_V[3];
    result.m[2][0] = rowZ.m_V[0]; result.m[2][1] = rowZ.m_V[1]; result.m[2][2] = rowZ.m_V[2]; result.m[2][3] = rowZ.m_V[3];
    result.m[3][0] = rowW.m_V[0]; result.m[3][1] = rowW.m_V[1]; result.m[3][2] = rowW.m_V[2]; result.m[3][3] = rowW.m_V[3];
    #endif
    return result;
}


/// Make Mat44 from 4 row vectors (4 element)
NOALIAS __forceinline const Mat44 MakeMat44( const Vector4 PCREF rowX, const Vector4 PCREF rowY, const Vector4 PCREF rowZ, const Vector4 PCREF rowW )
{
    return MakeMat44( rowX.m_V, rowY.m_V, rowZ.m_V, rowW.m_V );
}


/// Make Mat44 from a Mat33 - missing elements are zeroed
NOALIAS __forceinline const Mat44 MakeMat44( const Mat33 MATREF src )
{
    Mat44 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_insert( 0.0f, src.m_RowX, VW );
    result.m_RowY = spu_insert( 0.0f, src.m_RowY, VW );
    result.m_RowZ = spu_insert( 0.0f, src.m_RowZ, VW );
    result.m_RowW = Const< vector float >::Zero();
    #elif defined( _PPU_ )
    result.m_RowX = vec_select<0,0,0,1>( src.m_RowX, Const< vector float >::Zero() );
    result.m_RowY = vec_select<0,0,0,1>( src.m_RowY, Const< vector float >::Zero() );
    result.m_RowZ = vec_select<0,0,0,1>( src.m_RowZ, Const< vector float >::Zero() );
    result.m_RowW = Const< vector float >::Zero();
    #elif defined( _VMX128_ )
    result.m_ColX = __vrlimi( src.m_ColX, Const< __vector4 >::Zero(), VMX_MASK_INSERT_W, 0 );
    result.m_ColY = __vrlimi( src.m_ColY, Const< __vector4 >::Zero(), VMX_MASK_INSERT_W, 0 );
    result.m_ColZ = __vrlimi( src.m_ColZ, Const< __vector4 >::Zero(), VMX_MASK_INSERT_W, 0 );
    result.m_ColW = Const< __vector4 >::Zero();
    #elif defined( _SSE_ )
    result.m_RowX = sse_select<0,0,0,1>( src.m_RowX, Const< __m128 >::Zero() );
    result.m_RowY = sse_select<0,0,0,1>( src.m_RowY, Const< __m128 >::Zero() );
    result.m_RowZ = sse_select<0,0,0,1>( src.m_RowZ, Const< __m128 >::Zero() );
    result.m_RowW = Const< __m128 >::Zero();
    #else
    result.m[0][0] = src.m[0][0]; result.m[0][1] = src.m[0][1]; result.m[0][2] = src.m[0][2]; result.m[0][3] = 0.0f;
    result.m[1][0] = src.m[1][0]; result.m[1][1] = src.m[1][1]; result.m[1][2] = src.m[1][2]; result.m[1][3] = 0.0f;
    result.m[2][0] = src.m[2][0]; result.m[2][1] = src.m[2][1]; result.m[2][2] = src.m[2][2]; result.m[2][3] = 0.0f;
    result.m[3][0] = 0.0f;        result.m[3][1] = 0.0f;        result.m[3][2] = 0.0f;        result.m[3][3] = 0.0f;
    #endif
    return result;
}

/// Make Mat44 from a Mat43 - missing elements are zeroed
NOALIAS __forceinline const Mat44 MakeMat44( const Mat43 MATREF src )
{
    Mat44 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_insert( 0.0f, src.m_RowX, VW );
    result.m_RowY = spu_insert( 0.0f, src.m_RowY, VW );
    result.m_RowZ = spu_insert( 0.0f, src.m_RowZ, VW );
    result.m_RowW = spu_insert( 0.0f, src.m_RowW, VW );
    #elif defined( _PPU_ )
    result.m_RowX = vec_select<0,0,0,1>( src.m_RowX, Const< vector float >::Zero() );
    result.m_RowY = vec_select<0,0,0,1>( src.m_RowY, Const< vector float >::Zero() );
    result.m_RowZ = vec_select<0,0,0,1>( src.m_RowZ, Const< vector float >::Zero() );
    result.m_RowW = vec_select<0,0,0,1>( src.m_RowW, Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    result.m_ColX = __vrlimi( src.m_ColX, Const< __vector4 >::Zero(), VMX_MASK_INSERT_W, 0 );
    result.m_ColY = __vrlimi( src.m_ColY, Const< __vector4 >::Zero(), VMX_MASK_INSERT_W, 0 );
    result.m_ColZ = __vrlimi( src.m_ColZ, Const< __vector4 >::Zero(), VMX_MASK_INSERT_W, 0 );
    result.m_ColW = Const< __vector4 >::Zero();
    #elif defined( _SSE_ )
    result.m_RowX = sse_select<0,0,0,1>( src.m_RowX, Const< __m128 >::Zero() );
    result.m_RowY = sse_select<0,0,0,1>( src.m_RowY, Const< __m128 >::Zero() );
    result.m_RowZ = sse_select<0,0,0,1>( src.m_RowZ, Const< __m128 >::Zero() );
    result.m_RowW = sse_select<0,0,0,1>( src.m_RowW, Const< __m128 >::Zero() );
    #else
    result.m[0][0] = src.m[0][0]; result.m[0][1] = src.m[0][1]; result.m[0][2] = src.m[0][2]; result.m[0][3] = 0.0f;
    result.m[1][0] = src.m[1][0]; result.m[1][1] = src.m[1][1]; result.m[1][2] = src.m[1][2]; result.m[1][3] = 0.0f;
    result.m[2][0] = src.m[2][0]; result.m[2][1] = src.m[2][1]; result.m[2][2] = src.m[2][2]; result.m[2][3] = 0.0f;
    result.m[3][0] = src.m[3][0]; result.m[3][1] = src.m[2][1]; result.m[3][2] = src.m[3][2]; result.m[3][3] = 0.0f;
    #endif
    return result;
}


/// Make Mat44 from a Reflection Plane
NOALIAS __forceinline const Mat44 MakeMat44( const Plane PCREF plane )
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
    Mat44 result;
    SetRows( &result,
             MulAdd( a, s, Const<Vec4F32>::UnitX() ),
             MulAdd( b, s, Const<Vec4F32>::UnitY() ),
             MulAdd( c, s, Const<Vec4F32>::UnitZ() ),
             MulAdd( d, s, Const<Vec4F32>::UnitW() ) );
    return result;
}


/// Mat44 Write to Stream
template < typename T_Stream > void Serialise( const Mat44 MATREF src, T_Stream& stream )
{
    stream << RowX( src );
    stream << RowY( src );
    stream << RowZ( src );
    stream << RowW( src );
}

/// Mat44 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Mat44* const __restrict pDst )
{
    Vector4 rowX, rowY, rowZ, rowW;
    stream >> rowX;
    stream >> rowY;
    stream >> rowZ;
    stream >> rowW;
    *pDst = MakeMat44( rowX, rowY, rowZ, rowW );
}




/// Get rows from a Mat44
NOALIAS __forceinline void GetRows( const Mat44 MATREF src, Vec4F32* __restrict pRowX, Vec4F32* __restrict pRowY, Vec4F32* __restrict pRowZ, Vec4F32* __restrict pRowW )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    pRowX->m_V = src.m_RowX;
    pRowY->m_V = src.m_RowY;
    pRowZ->m_V = src.m_RowZ;
    pRowW->m_V = src.m_RowW;
    #elif defined( _VMX128_ )
    vmx_transpose_matrix44( src.m_ColX, src.m_ColY, src.m_ColZ, src.m_ColW, &pRowX->m_V, &pRowY->m_V, &pRowZ->m_V, &pRowW->m_V );
    #else
    pRowX->m_V[0] = src.m[0][0]; pRowX->m_V[1] = src.m[0][1]; pRowX->m_V[2] = src.m[0][2]; pRowX->m_V[3] = src.m[0][3];
    pRowY->m_V[0] = src.m[1][0]; pRowY->m_V[1] = src.m[1][1]; pRowY->m_V[2] = src.m[1][2]; pRowY->m_V[3] = src.m[1][3];
    pRowZ->m_V[0] = src.m[2][0]; pRowZ->m_V[1] = src.m[2][1]; pRowZ->m_V[2] = src.m[2][2]; pRowZ->m_V[3] = src.m[2][3];
    pRowW->m_V[0] = src.m[3][0]; pRowW->m_V[1] = src.m[3][1]; pRowW->m_V[2] = src.m[3][2]; pRowW->m_V[3] = src.m[3][3];
    #endif
}

/// Set rows of a Mat44
NOALIAS __forceinline void SetRows( Mat44* pDst, const Vec4F32 VECREF rowX, const Vec4F32 VECREF rowY, const Vec4F32 VECREF rowZ, const Vec4F32 VECREF rowW )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    pDst->m_RowX = rowX.m_V;
    pDst->m_RowY = rowY.m_V;
    pDst->m_RowZ = rowZ.m_V;
    pDst->m_RowW = rowW.m_V;
    #elif defined( _VMX128_ )
    vmx_transpose_matrix44( rowX.m_V, rowY.m_V, rowZ.m_V, rowW.m_V, &pDst->m_ColX, &pDst->m_ColY, &pDst->m_ColZ, &pDst->m_ColW );
    #else
    pDst->m[0][0] = rowX.m_V[0]; pDst->m[0][1] = rowX.m_V[1]; pDst->m[0][2] = rowX.m_V[2]; pDst->m[0][3] = rowX.m_V[3];
    pDst->m[1][0] = rowY.m_V[0]; pDst->m[1][1] = rowY.m_V[1]; pDst->m[1][2] = rowY.m_V[2]; pDst->m[1][3] = rowY.m_V[3];
    pDst->m[2][0] = rowZ.m_V[0]; pDst->m[2][1] = rowZ.m_V[1]; pDst->m[2][2] = rowZ.m_V[2]; pDst->m[2][3] = rowZ.m_V[3];
    pDst->m[3][0] = rowW.m_V[0]; pDst->m[3][1] = rowW.m_V[1]; pDst->m[3][2] = rowW.m_V[2]; pDst->m[3][3] = rowW.m_V[3];
    #endif
}


/// Get Row X from a Mat44
NOALIAS __forceinline const Vector4 RowX( const Mat44 MATREF src )
{
    Vector4 result;
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    result.m_V.m_V = src.m_RowX;
    #elif defined( _VMX128_ )
    //result.m_V.m_V = vmx_merge_xy( vmx_merge_xy( src.m_ColX, src.m_ColZ ),
    //                           vmx_merge_xy( src.m_ColY, src.m_ColW ) );
    result.m_V.m_V = vmx_transpose_row_x( src.m_ColX, src.m_ColY, src.m_ColZ, src.m_ColW );
    #else
    result.m_V.m_V[VX] = src.m[0][0];
    result.m_V.m_V[VY] = src.m[0][1];
    result.m_V.m_V[VZ] = src.m[0][2];
    result.m_V.m_V[VW] = src.m[0][3];
    #endif
    return result;
}

/// Get Row Y from a Mat44
NOALIAS __forceinline const Vector4 RowY( const Mat44 MATREF src )
{
    Vector4 result;
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    result.m_V.m_V = src.m_RowY;
    #elif defined( _VMX128_ )
    //result.m_V.m_V = vmx_merge_zw( vmx_merge_xy( src.m_ColX, src.m_ColZ ),
    //                           vmx_merge_xy( src.m_ColY, src.m_ColW ) );
    result.m_V.m_V = vmx_transpose_row_y( src.m_ColX, src.m_ColY, src.m_ColZ, src.m_ColW );
    #else
    result.m_V.m_V[VX] = src.m[1][0];
    result.m_V.m_V[VY] = src.m[1][1];
    result.m_V.m_V[VZ] = src.m[1][2];
    result.m_V.m_V[VW] = src.m[1][3];
    #endif
    return result;
}

/// Get Row Z from a Mat44
NOALIAS __forceinline const Vector4 RowZ( const Mat44 MATREF src )
{
    Vector4 result;
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    result.m_V.m_V = src.m_RowZ;
    #elif defined( _VMX128_ )
    //result.m_V.m_V = vmx_merge_xy( vmx_merge_zw( src.m_ColX, src.m_ColZ ),
    //                           vmx_merge_zw( src.m_ColY, src.m_ColW ) );
    result.m_V.m_V = vmx_transpose_row_z( src.m_ColX, src.m_ColY, src.m_ColZ, src.m_ColW );
    #else
    result.m_V.m_V[VX] = src.m[2][0];
    result.m_V.m_V[VY] = src.m[2][1];
    result.m_V.m_V[VZ] = src.m[2][2];
    result.m_V.m_V[VW] = src.m[2][3];
    #endif
    return result;
}

/// Get Row W from a Mat44
NOALIAS __forceinline const Vector4 RowW( const Mat44 MATREF src )
{
    Vector4 result;
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    result.m_V.m_V = src.m_RowW;
    #elif defined( _VMX128_ )
    //result.m_V.m_V = vmx_merge_zw( vmx_merge_zw( src.m_ColX, src.m_ColZ ),
    //                           vmx_merge_zw( src.m_ColY, src.m_ColW ) );
    result.m_V.m_V = vmx_transpose_row_w( src.m_ColX, src.m_ColY, src.m_ColZ, src.m_ColW );
    #else
    result.m_V.m_V[VX] = src.m[3][0];
    result.m_V.m_V[VY] = src.m[3][1];
    result.m_V.m_V[VZ] = src.m[3][2];
    result.m_V.m_V[VW] = src.m[3][3];
    #endif
    return result;
}


/// Get Col X from a Mat44
NOALIAS __forceinline const Vector4 ColX( const Mat44 MATREF src )
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

/// Get Col Y from a Mat44
NOALIAS __forceinline const Vector4 ColY( const Mat44 MATREF src )
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

/// Get Col Z from a Mat44
NOALIAS __forceinline const Vector4 ColZ( const Mat44 MATREF src )
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

/// Get Col W from a Mat44
NOALIAS __forceinline const Vector4 ColW( const Mat44 MATREF src )
{
    Vector4 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_transpose_col_w( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_transpose_col_w( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW );
    #elif defined( _VMX128_ )
    result.m_V.m_V = src.m_ColW;
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_transpose_col_w( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW );
    #else
    result.m_V.m_V[VX] = src.m[0][3];
    result.m_V.m_V[VY] = src.m[1][3];
    result.m_V.m_V[VZ] = src.m[2][3];
    result.m_V.m_V[VW] = src.m[3][3];
    #endif
    return result;
}



/// Set Row X of a Mat44
NOALIAS __forceinline void SetRowX( Mat44* pDst, const Vec4F32 VECREF src )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    pDst->m_RowX = src.m_V;
    #elif defined( _VMX128_ )
    pDst->m_ColX = __vrlimi( pDst->m_ColX, src.m_V, VMX_MASK_INSERT_X, VMX_ROTATE_X_TO_X );
    pDst->m_ColY = __vrlimi( pDst->m_ColY, src.m_V, VMX_MASK_INSERT_X, VMX_ROTATE_Y_TO_X );
    pDst->m_ColZ = __vrlimi( pDst->m_ColZ, src.m_V, VMX_MASK_INSERT_X, VMX_ROTATE_Z_TO_X );
    pDst->m_ColW = __vrlimi( pDst->m_ColW, src.m_V, VMX_MASK_INSERT_X, VMX_ROTATE_W_TO_X );
    #else
    pDst->m[0][0] = src.m_V[VX];
    pDst->m[0][1] = src.m_V[VY];
    pDst->m[0][2] = src.m_V[VZ];
    pDst->m[0][3] = src.m_V[VW];
    #endif
}

/// Set Row Y of a Mat44
NOALIAS __forceinline void SetRowY( Mat44* pDst, const Vec4F32 VECREF src )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    pDst->m_RowY = src.m_V;
    #elif defined( _VMX128_ )
    pDst->m_ColX = __vrlimi( pDst->m_ColX, src.m_V, VMX_MASK_INSERT_Y, VMX_ROTATE_X_TO_Y );
    pDst->m_ColY = __vrlimi( pDst->m_ColY, src.m_V, VMX_MASK_INSERT_Y, VMX_ROTATE_Y_TO_Y );
    pDst->m_ColZ = __vrlimi( pDst->m_ColZ, src.m_V, VMX_MASK_INSERT_Y, VMX_ROTATE_Z_TO_Y );
    pDst->m_ColW = __vrlimi( pDst->m_ColW, src.m_V, VMX_MASK_INSERT_Y, VMX_ROTATE_W_TO_Y );
    #else
    pDst->m[1][0] = src.m_V[VX];
    pDst->m[1][1] = src.m_V[VY];
    pDst->m[1][2] = src.m_V[VZ];
    pDst->m[1][3] = src.m_V[VW];
    #endif
}

/// Set Row Z of a Mat44
NOALIAS __forceinline void SetRowZ( Mat44* pDst, const Vec4F32 VECREF src )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    pDst->m_RowZ = src.m_V;
    #elif defined( _VMX128_ )
    pDst->m_ColX = __vrlimi( pDst->m_ColX, src.m_V, VMX_MASK_INSERT_Z, VMX_ROTATE_X_TO_Z );
    pDst->m_ColY = __vrlimi( pDst->m_ColY, src.m_V, VMX_MASK_INSERT_Z, VMX_ROTATE_Y_TO_Z );
    pDst->m_ColZ = __vrlimi( pDst->m_ColZ, src.m_V, VMX_MASK_INSERT_Z, VMX_ROTATE_Z_TO_Z );
    pDst->m_ColW = __vrlimi( pDst->m_ColW, src.m_V, VMX_MASK_INSERT_Z, VMX_ROTATE_W_TO_Z );
    #else
    pDst->m[2][0] = src.m_V[VX];
    pDst->m[2][1] = src.m_V[VY];
    pDst->m[2][2] = src.m_V[VZ];
    pDst->m[2][3] = src.m_V[VW];
    #endif
}

/// Set Row W of a Mat44
NOALIAS __forceinline void SetRowW( Mat44* pDst, const Vec4F32 VECREF src )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    pDst->m_RowW = src.m_V;
    #elif defined( _VMX128_ )
    pDst->m_ColX = __vrlimi( pDst->m_ColX, src.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
    pDst->m_ColY = __vrlimi( pDst->m_ColY, src.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
    pDst->m_ColZ = __vrlimi( pDst->m_ColZ, src.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
    pDst->m_ColW = __vrlimi( pDst->m_ColW, src.m_V, VMX_MASK_INSERT_W, VMX_ROTATE_W_TO_W );
    #else
    pDst->m[3][0] = src.m_V[VX];
    pDst->m[3][1] = src.m_V[VY];
    pDst->m[3][2] = src.m_V[VZ];
    pDst->m[3][3] = src.m_V[VW];
    #endif
}


/// Set Row X of a Mat44
NOALIAS __forceinline void SetRowX( Mat44* pDst, const Vector4 PCREF src )
{
    SetRowX( pDst, src.m_V );
}

/// Set Row Y of a Mat44
NOALIAS __forceinline void SetRowY( Mat44* pDst, const Vector4 PCREF src )
{
    SetRowY( pDst, src.m_V );
}

/// Set Row Z of a Mat44
NOALIAS __forceinline void SetRowZ( Mat44* pDst, const Vector4 PCREF src )
{
    SetRowZ( pDst, src.m_V );
}

/// Set Row W of a Mat44
NOALIAS __forceinline void SetRowW( Mat44* pDst, const Vector4 PCREF src )
{
    SetRowW( pDst, src.m_V );
}


/// Mat44 Get element
template < unsigned int Row, unsigned int Col >
NOALIAS __forceinline const VF32 Element( const Mat44 MATREF src )
{
    BIZ_STATIC_ASSERT( Row < 4 && Col < 4 );
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
        case E_ColW : result.m_V = spu_splat<VW>( row ); break;
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
        case E_ColW : result.m_V = vec_splat( row, VW ); break;
        default     : result.m_V = Const< vector float >::Zero(); break;
    };
    #elif defined( _VMX128_ )
    __vector4 col;
    switch ( Col )
    {
        case E_ColX : col = src.m_ColX; break;
        case E_ColY : col = src.m_ColY; break;
        case E_ColZ : col = src.m_ColZ; break;
        case E_ColW : col = src.m_ColW; break;
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
        case E_ColW : result.m_V = sse_splat<VW>( row ); break;
        default     : result.m_V = Const< __m128 >::Zero(); break;
    };
    #else
    result.m_V = src.m[Row][Col];
    #endif
    return result;
}

/// Mat44 Get element
//template < unsigned int Row, unsigned int Col >
//NOALIAS __forceinline const VF32 Element( const Mat44 MATREF src )
//{
//    BIZ_STATIC_ASSERT( ( Row < 4 ) && ( Col < 4 ) );
//    #if defined( _VXM128_ )
//    Vector4 col;
//    switch ( Col )
//    {
//        case 0  : col = ColX( src ); break;
//        case 1  : col = ColY( src ); break;
//        case 2  : col = ColZ( src ); break;
//        case 3  : col = ColW( src ); break;
//        default : col = Const<Vector4>::Zero(); break;
//    }
//    return Element<Row>( col );
//    #else
//    Vector4 row;
//    switch ( Row )
//    {
//        case 0  : row = RowX( src ); break;
//        case 1  : row = RowY( src ); break;
//        case 2  : row = RowZ( src ); break;
//        case 3  : row = RowW( src ); break;
//        default : row = Const<Vector4>::Zero(); break;
//    }
//    return Element<Col>( row );
//    #endif
//}

/// Mat44 Set element
template < unsigned int Row, unsigned int Col >
NOALIAS __forceinline void SetElement( Mat44* const __restrict pDst, const VF32 VECREF e )
{
    BIZ_STATIC_ASSERT( ( Row < 4 ) && ( Col < 4 ) );
    #if defined( _VXM128_ )
    Vector4 col;
    switch ( Col )
    {
        case 0  : col = ColX( *pDst ); break;
        case 1  : col = ColY( *pDst ); break;
        case 2  : col = ColZ( *pDst ); break;
        case 3  : col = ColW( *pDst ); break;
        default : col = Const<Vector4>::Zero(); break;
    }
    SetElement<Row>( &col, e );
    switch ( Col )
    {
        case 0  : SetColX( pDst, col ); break;
        case 1  : SetColY( pDst, col ); break;
        case 2  : SetColZ( pDst, col ); break;
        case 3  : SetColW( pDst, col ); break;
        default : break;
    }
    #else
    Vector4 row;
    switch ( Row )
    {
        case 0  : row = RowX( *pDst ); break;
        case 1  : row = RowY( *pDst ); break;
        case 2  : row = RowZ( *pDst ); break;
        case 3  : row = RowW( *pDst ); break;
        default : row = Const<Vector4>::Zero(); break;
    }
    SetElement<Col>( &row, e );
    switch ( Row )
    {
        case 0  : SetRowX( pDst, row ); break;
        case 1  : SetRowY( pDst, row ); break;
        case 2  : SetRowZ( pDst, row ); break;
        case 3  : SetRowW( pDst, row ); break;
        default : break;
    }
    #endif
}


/// Mat44 Assign column from another Mat44
template < unsigned int DstCol, unsigned int SrcCol >
NOALIAS __forceinline void ReplaceColumn( Mat44* const __restrict pDst, const Mat44 MATREF src )
{
    BIZ_STATIC_ASSERT( ( DstCol < 4 ) && ( SrcCol < 4 ) );
    #if defined( _SPU_ )
    SetElement<0,DstCol>( pDst, Element<0,SrcCol>( src ) );
    SetElement<1,DstCol>( pDst, Element<1,SrcCol>( src ) );
    SetElement<2,DstCol>( pDst, Element<2,SrcCol>( src ) );
    SetElement<3,DstCol>( pDst, Element<3,SrcCol>( src ) );
    #elif defined( _PPU_ )
    SetElement<0,DstCol>( pDst, Element<0,SrcCol>( src ) );
    SetElement<1,DstCol>( pDst, Element<1,SrcCol>( src ) );
    SetElement<2,DstCol>( pDst, Element<2,SrcCol>( src ) );
    SetElement<3,DstCol>( pDst, Element<3,SrcCol>( src ) );
    #elif defined( _VMX128_ )
    //SetElement<0,DstCol>( pDst, Element<0,SrcCol>( src ) );
    //SetElement<1,DstCol>( pDst, Element<1,SrcCol>( src ) );
    //SetElement<2,DstCol>( pDst, Element<2,SrcCol>( src ) );
    //SetElement<3,DstCol>( pDst, Element<3,SrcCol>( src ) );
    __vector4 column;
    switch ( SrcCol )
    {
        case 0  : column = src.m_ColX; break;
        case 1  : column = src.m_ColY; break;
        case 2  : column = src.m_ColZ; break;
        case 3  : column = src.m_ColW; break;
        default : column = __vzero();  break;
    }
    switch ( DstCol )
    {
        case 0  : pDst->m_ColX = column; break;
        case 1  : pDst->m_ColY = column; break;
        case 2  : pDst->m_ColZ = column; break;
        case 3  : pDst->m_ColW = column; break;
        default : break;
    }
    #elif defined( _SSE_ )
    SetElement<0,DstCol>( pDst, Element<0,SrcCol>( src ) );
    SetElement<1,DstCol>( pDst, Element<1,SrcCol>( src ) );
    SetElement<2,DstCol>( pDst, Element<2,SrcCol>( src ) );
    SetElement<3,DstCol>( pDst, Element<3,SrcCol>( src ) );
    #else
    SetElement<0,DstCol>( pDst, Element<0,SrcCol>( src ) );
    SetElement<1,DstCol>( pDst, Element<1,SrcCol>( src ) );
    SetElement<2,DstCol>( pDst, Element<2,SrcCol>( src ) );
    SetElement<3,DstCol>( pDst, Element<3,SrcCol>( src ) );
    #endif
}


/// Mat44 Negation operator
NOALIAS __forceinline const Mat44 operator - ( const Mat44 MATREF src )
{
    Mat44 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_negate( src.m_RowX );
    result.m_RowY = spu_negate( src.m_RowY );
    result.m_RowZ = spu_negate( src.m_RowZ );
    result.m_RowW = spu_negate( src.m_RowW );
    #elif defined( _PPU_ )
    result.m_RowX = vec_negate( src.m_RowX );
    result.m_RowY = vec_negate( src.m_RowY );
    result.m_RowZ = vec_negate( src.m_RowZ );
    result.m_RowW = vec_negate( src.m_RowW );
    #elif defined( _VMX128_ )
    result.m_ColX = vmx_negate( src.m_ColX );
    result.m_ColY = vmx_negate( src.m_ColY );
    result.m_ColZ = vmx_negate( src.m_ColZ );
    result.m_ColW = vmx_negate( src.m_ColW );
    #elif defined( _SSE_ )
    result.m_RowX = sse_negate( src.m_RowX );
    result.m_RowY = sse_negate( src.m_RowY );
    result.m_RowZ = sse_negate( src.m_RowZ );
    result.m_RowW = sse_negate( src.m_RowW );
    #else
    for ( S32 r=0; r<4; r++ )
    {
        for ( S32 c=0; c<4; c++ )
        {
            result.m[r][c] = -src.m[r][c];
        }
    }
    #endif
    return result;
}

/// Mat44 Addition operator
NOALIAS __forceinline const Mat44 operator + ( const Mat44 MATREF lhs, const Mat44 MATREF rhs )
{
    Mat44 result;
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
    result.m_ColW = __vaddfp( lhs.m_ColW, rhs.m_ColW );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_add_ps( lhs.m_RowX, rhs.m_RowX );
    result.m_RowY = _mm_add_ps( lhs.m_RowY, rhs.m_RowY );
    result.m_RowZ = _mm_add_ps( lhs.m_RowZ, rhs.m_RowZ );
    result.m_RowW = _mm_add_ps( lhs.m_RowW, rhs.m_RowW );
    #else
    for ( S32 r=0; r<4; r++ )
    {
        for ( S32 c=0; c<4; c++ )
        {
            result.m[r][c] = lhs.m[r][c] + rhs.m[r][c];
        }
    }
    #endif
    return result;
}

/// Mat44 Subtraction operator
NOALIAS __forceinline const Mat44 operator - ( const Mat44 MATREF lhs, const Mat44 MATREF rhs )
{
    Mat44 result;
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
    result.m_ColW = __vsubfp( lhs.m_ColW, rhs.m_ColW );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_sub_ps( lhs.m_RowX, rhs.m_RowX );
    result.m_RowY = _mm_sub_ps( lhs.m_RowY, rhs.m_RowY );
    result.m_RowZ = _mm_sub_ps( lhs.m_RowZ, rhs.m_RowZ );
    result.m_RowW = _mm_sub_ps( lhs.m_RowW, rhs.m_RowW );
    #else
    for ( S32 r=0; r<4; r++ )
    {
        for ( S32 c=0; c<4; c++ )
        {
            result.m[r][c] = lhs.m[r][c] - rhs.m[r][c];
        }
    }
    #endif
    return result;
}

/// Mat44 * VF32 Multiplication operator
NOALIAS __forceinline const Mat44 operator * ( const Mat44 MATREF matrix, const VF32 VECREF scalar )
{
    Mat44 result;
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
    result.m_ColW = __vmulfp( matrix.m_ColW, scalar.m_V );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_mul_ps( matrix.m_RowX, scalar.m_V );
    result.m_RowY = _mm_mul_ps( matrix.m_RowY, scalar.m_V );
    result.m_RowZ = _mm_mul_ps( matrix.m_RowZ, scalar.m_V );
    result.m_RowW = _mm_mul_ps( matrix.m_RowW, scalar.m_V );
    #else
    for ( S32 r=0; r<4; r++ )
    {
        for ( S32 c=0; c<4; c++ )
        {
            result.m[r][c] = matrix.m[r][c] * scalar.m_V;
        }
    }
    #endif
    return result;
}

/// VF32 * Mat44 Multiplication operator
NOALIAS __forceinline const Mat44 operator * ( const VF32 VECREF scalar, const Mat44 MATREF matrix )
{
    Mat44 result;
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
    result.m_ColW = __vmulfp( matrix.m_ColW, scalar.m_V );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_mul_ps( matrix.m_RowX, scalar.m_V );
    result.m_RowY = _mm_mul_ps( matrix.m_RowY, scalar.m_V );
    result.m_RowZ = _mm_mul_ps( matrix.m_RowZ, scalar.m_V );
    result.m_RowW = _mm_mul_ps( matrix.m_RowW, scalar.m_V );
    #else
    for ( S32 r=0; r<4; r++ )
    {
        for ( S32 c=0; c<4; c++ )
        {
            result.m[r][c] = matrix.m[r][c] * scalar.m_V;
        }
    }
    #endif
    return result;
}


/// Vec4F32 * Mat44 Multiplication operator : input = 4 element row vector, output = 4 element row vector
NOALIAS __forceinline const Vec4F32 operator * ( const Vec4F32 VECREF v, const Mat44 MATREF m )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    //result.m_V = spu_add( spu_madd( spu_splat<VX>( v.m_V ), m.m_RowX,
    //                      spu_mul ( spu_splat<VY>( v.m_V ), m.m_RowY ) ),
    //                      spu_madd( spu_splat<VZ>( v.m_V ), m.m_RowZ,
    //                      spu_mul ( spu_splat<VW>( v.m_V ), m.m_RowW ) ) );
    result.m_V = spu_multiply_vector4_matrix44( v.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #elif defined( _PPU_ )
    //result.m_V = vec_add( vec_madd( vec_splat( v.m_V, VX ), m.m_RowX,
    //                      vec_mul ( vec_splat( v.m_V, VY ), m.m_RowY ) ),
    //                      vec_madd( vec_splat( v.m_V, VZ ), m.m_RowZ,
    //                      vec_mul ( vec_splat( v.m_V, VW ), m.m_RowW ) ) );
    result.m_V = vec_multiply_vector4_matrix44( v.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #elif defined( _VMX128_ )
    //result.m_V = vmx_make_vector4( __vmsum4fp( v.m_V, m.m_ColX ),
    //                               __vmsum4fp( v.m_V, m.m_ColY ),
    //                               __vmsum4fp( v.m_V, m.m_ColZ ),
    //                               __vmsum4fp( v.m_V, m.m_ColW ) );
    result.m_V = vmx_multiply_vector4_matrix44( v.m_V, m.m_ColX, m.m_ColY, m.m_ColZ, m.m_ColW );
    #elif defined( _SSE_ )
    //result.m_V = _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( v.m_V ), m.m_RowX ),
    //                                     _mm_mul_ps( sse_splat<VY>( v.m_V ), m.m_RowY ) ),
    //                         _mm_add_ps( _mm_mul_ps( sse_splat<VZ>( v.m_V ), m.m_RowZ ),
    //                                     _mm_mul_ps( sse_splat<VW>( v.m_V ), m.m_RowW ) ) );
    result.m_V = sse_multiply_vector4_matrix44( v.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #else
    for ( S32 c=0; c<4; c++ )
    {
        result.m_V[c] = v.m_V[0] * m.m[0][c] +
                        v.m_V[1] * m.m[1][c] +
                        v.m_V[2] * m.m[2][c] +
                        v.m_V[3] * m.m[3][c];
    }
    #endif
    return result;
}

/// Mat44 * Vec4F32 Multiplication operator : input = 4 element col vector, output = 4 element col vector
NOALIAS __forceinline const Vec4F32 operator * ( const Mat44 MATREF m, const Vec4F32 VECREF v )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    vector float col_x, col_y, col_z, col_w;
    spu_transpose_matrix44( m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW, &col_x, &col_y, &col_z, &col_w );
    result.m_V = spu_add( spu_madd( col_x, spu_splat<VX>( v.m_V ),
                          spu_mul ( col_y, spu_splat<VY>( v.m_V ) ) ),
                          spu_madd( col_z, spu_splat<VZ>( v.m_V ),
                          spu_mul ( col_w, spu_splat<VW>( v.m_V ) ) ) );
    #elif defined( _PPU_ )
    vector float col_x, col_y, col_z, col_w;
    vec_transpose_matrix44( m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW, &col_x, &col_y, &col_z, &col_w );
    result.m_V = vec_add( vec_madd( col_x, vec_splat( v.m_V, VX ),
                          vec_mul ( col_y, vec_splat( v.m_V, VY ) ) ),
                          vec_madd( col_z, vec_splat( v.m_V, VZ ),
                          vec_mul ( col_w, vec_splat( v.m_V, VW ) ) ) );
    #elif defined( _VMX128_ )
    __vector4 row_x, row_y, row_z, row_w;
    vmx_transpose_matrix44( m.m_ColX, m.m_ColY, m.m_ColZ, m.m_ColW, &row_x, &row_y, &row_z, &row_w );
    result.m_V = vmx_make_vector4( __vmsum4fp( row_x, v.m_V ),
                                   __vmsum4fp( row_y, v.m_V ),
                                   __vmsum4fp( row_z, v.m_V ),
                                   __vmsum4fp( row_w, v.m_V ) );
    #elif defined( _SSE_ )
    __m128 col_x, col_y, col_z, col_w;
    sse_transpose_matrix44( m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW, &col_x, &col_y, &col_z, &col_w );
    result.m_V = _mm_add_ps( _mm_add_ps( _mm_mul_ps( col_x, sse_splat<VX>( v.m_V ) ),
                                         _mm_mul_ps( col_y, sse_splat<VY>( v.m_V ) ) ),
                             _mm_add_ps( _mm_mul_ps( col_z, sse_splat<VZ>( v.m_V ) ),
                                         _mm_mul_ps( col_w, sse_splat<VW>( v.m_V ) ) ) );
    #else
    for ( S32 r=0; r<4; r++ )
    {
        result.m_V[r] = m.m[r][0] * v.m_V[0] +
                        m.m[r][1] * v.m_V[1] +
                        m.m[r][2] * v.m_V[2] +
                        m.m[r][3] * v.m_V[3];
    }
    #endif
    return result;
}


/// Vector3 Matrix Multiplication: Vector3 * Mat44
NOALIAS __forceinline const Vector4 operator * ( const Vector3 PCREF v, const Mat44 MATREF m )
{
    /// @code
    ///               | m00 m01 m02 m03 |   [ x.m00 + y.m10 + z.m20 + 0,
    /// [ x y z 0 ] * | m10 m11 m12 m13 | =   x.m01 + y.m11 + z.m21 + 0,
    ///               | m20 m21 m22 m23 |     x.m02 + y.m12 + z.m22 + 0,
    ///               | m30 m31 m32 m33 |     x.m03 + y.m13 + z.m23 + 0 ]
    /// @endcode
    Vector4 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_multiply_vector3_matrix44( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_multiply_vector3_matrix44( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #elif defined( _VMX128_ )
    result.m_V.m_V = vmx_multiply_vector3_matrix44( v.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ, m.m_ColW );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_multiply_vector3_matrix44( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ );
    #else
    for ( S32 c=0; c<4; c++ )
    {
        result.m_V.m_V[c] = v.m_V.m_V[0] * m.m[0][c] +
                            v.m_V.m_V[1] * m.m[1][c] +
                            v.m_V.m_V[2] * m.m[2][c];
    }
    #endif
    return result;
}

/// Vector3 Matrix Multiplication: Mat44 * Vector3             
NOALIAS __forceinline const Vector4 operator * ( const Mat44 MATREF m, const Vector3 PCREF v )
{
    /// @code
    /// | m00 m01 m02 m03 |   |x|   | m00.x + m01.y + m02.z + 0 |
    /// | m10 m11 m12 m13 | * |x| = | m10.x + m11.y + m12.z + 0 |
    /// | m20 m21 m22 m23 |   |z|   | m20.x + m21.y + m22.z + 0 |
    /// | m30 m31 m32 m33 |   |0|   | m30.x + m31.y + m32.z + 0 |
    /// @endcode
    Vector4 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_multiply_matrix44_vector3( m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW, v.m_V.m_V );
    #elif defined( _PPU_ )                                     
    result.m_V.m_V = vec_multiply_matrix44_vector3( m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW, v.m_V.m_V );
    #elif defined( _VMX128_ )                                  
    result.m_V.m_V = vmx_multiply_matrix44_vector3( m.m_ColX, m.m_ColY, m.m_ColZ, m.m_ColW, v.m_V.m_V );
    #elif defined( _SSE_ )                                     
    result.m_V.m_V = sse_multiply_matrix44_vector3( m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW, v.m_V.m_V );
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


/// Point3 Matrix Multiplication: Point3 * Mat44
//NOALIAS __forceinline const Vector4 operator * ( const Point3 PCREF p, const Mat44 MATREF m )
NOALIAS __forceinline const HPoint3 operator * ( const Point3 PCREF p, const Mat44 MATREF m )
{
    /// @code
    ///               | m00 m01 m02 m03 |   [ x.m00 + y.m10 + z.m20 + m30,
    /// [ x y z 1 ] * | m10 m11 m12 m13 | =   x.m01 + y.m11 + z.m21 + m31,
    ///               | m20 m21 m22 m23 |     x.m02 + y.m12 + z.m22 + m32,
    ///               | m30 m31 m32 m33 |     x.m03 + y.m13 + z.m23 + m33 ]
    /// @endcode
    
    //Vector4 result;
    HPoint3 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_multiply_point3_matrix44( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_multiply_point3_matrix44( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #elif defined( _VMX128_ )
    result.m_V.m_V = vmx_multiply_point3_matrix44( p.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ, m.m_ColW );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_multiply_point3_matrix44( p.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #else
    for ( S32 c=0; c<4; c++ )
    {
        result.m_V.m_V[c] = p.m_V.m_V[0] * m.m[0][c] +
                            p.m_V.m_V[1] * m.m[1][c] +
                            p.m_V.m_V[2] * m.m[2][c] +
                                           m.m[3][c];
    }
    #endif
    return result;
}


/// Mat44 Multiplication operator
NOALIAS __forceinline const Mat44 operator * ( const Mat44 MATREF lhs, const Mat44 MATREF rhs )
{
    Mat44 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_add( spu_madd( spu_splat<VX>( lhs.m_RowX ), rhs.m_RowX,
                             spu_mul ( spu_splat<VY>( lhs.m_RowX ), rhs.m_RowY ) ),
                             spu_madd( spu_splat<VZ>( lhs.m_RowX ), rhs.m_RowZ,
                             spu_mul ( spu_splat<VW>( lhs.m_RowX ), rhs.m_RowW ) ) );
    result.m_RowY = spu_add( spu_madd( spu_splat<VX>( lhs.m_RowY ), rhs.m_RowX,
                             spu_mul ( spu_splat<VY>( lhs.m_RowY ), rhs.m_RowY ) ),
                             spu_madd( spu_splat<VZ>( lhs.m_RowY ), rhs.m_RowZ,
                             spu_mul ( spu_splat<VW>( lhs.m_RowY ), rhs.m_RowW ) ) );
    result.m_RowZ = spu_add( spu_madd( spu_splat<VX>( lhs.m_RowZ ), rhs.m_RowX,
                             spu_mul ( spu_splat<VY>( lhs.m_RowZ ), rhs.m_RowY ) ),
                             spu_madd( spu_splat<VZ>( lhs.m_RowZ ), rhs.m_RowZ,
                             spu_mul ( spu_splat<VW>( lhs.m_RowZ ), rhs.m_RowW ) ) );
    result.m_RowW = spu_add( spu_madd( spu_splat<VX>( lhs.m_RowW ), rhs.m_RowX,
                             spu_mul ( spu_splat<VY>( lhs.m_RowW ), rhs.m_RowY ) ),
                             spu_madd( spu_splat<VZ>( lhs.m_RowW ), rhs.m_RowZ,
                             spu_mul ( spu_splat<VW>( lhs.m_RowW ), rhs.m_RowW ) ) );
    #elif defined( _PPU_ )
    result.m_RowX = vec_add( vec_madd( vec_splat( lhs.m_RowX, VX ), rhs.m_RowX,
                             vec_mul ( vec_splat( lhs.m_RowX, VY ), rhs.m_RowY ) ),
                             vec_madd( vec_splat( lhs.m_RowX, VZ ), rhs.m_RowZ,
                             vec_mul ( vec_splat( lhs.m_RowX, VW ), rhs.m_RowW ) ) );
    result.m_RowY = vec_add( vec_madd( vec_splat( lhs.m_RowY, VX ), rhs.m_RowX,
                             vec_mul ( vec_splat( lhs.m_RowY, VY ), rhs.m_RowY ) ),
                             vec_madd( vec_splat( lhs.m_RowY, VZ ), rhs.m_RowZ,
                             vec_mul ( vec_splat( lhs.m_RowY, VW ), rhs.m_RowW ) ) );
    result.m_RowZ = vec_add( vec_madd( vec_splat( lhs.m_RowZ, VX ), rhs.m_RowX,
                             vec_mul ( vec_splat( lhs.m_RowZ, VY ), rhs.m_RowY ) ),
                             vec_madd( vec_splat( lhs.m_RowZ, VZ ), rhs.m_RowZ,
                             vec_mul ( vec_splat( lhs.m_RowZ, VW ), rhs.m_RowW ) ) );
    result.m_RowW = vec_add( vec_madd( vec_splat( lhs.m_RowW, VX ), rhs.m_RowX,
                             vec_mul ( vec_splat( lhs.m_RowW, VY ), rhs.m_RowY ) ),
                             vec_madd( vec_splat( lhs.m_RowW, VZ ), rhs.m_RowZ,
                             vec_mul ( vec_splat( lhs.m_RowW, VW ), rhs.m_RowW ) ) );
    #elif defined( _VMX128_ )
    __vector4 lhs_row_x, lhs_row_y, lhs_row_z, lhs_row_w;
    vmx_transpose_matrix44( lhs.m_ColX, lhs.m_ColY, lhs.m_ColZ, lhs.m_ColW, &lhs_row_x, &lhs_row_y, &lhs_row_z, &lhs_row_w );
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
    const V128 result_cw_x( __vmsum4fp( lhs_row_x, rhs.m_ColW ) );
    const V128 result_cw_y( __vmsum4fp( lhs_row_y, rhs.m_ColW ) );
    const V128 result_cw_z( __vmsum4fp( lhs_row_z, rhs.m_ColW ) );
    const V128 result_cw_w( __vmsum4fp( lhs_row_w, rhs.m_ColW ) );
    result.m_ColX = vmx_make_vector4( result_cx_x, result_cx_y, result_cx_z, result_cx_w );
    result.m_ColY = vmx_make_vector4( result_cy_x, result_cy_y, result_cy_z, result_cy_w );
    result.m_ColZ = vmx_make_vector4( result_cz_x, result_cz_y, result_cz_z, result_cz_w );
    result.m_ColW = vmx_make_vector4( result_cw_x, result_cw_y, result_cw_z, result_cw_w );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( lhs.m_RowX ), rhs.m_RowX ),
                                            _mm_mul_ps( sse_splat<VY>( lhs.m_RowX ), rhs.m_RowY ) ),
                                _mm_add_ps( _mm_mul_ps( sse_splat<VZ>( lhs.m_RowX ), rhs.m_RowZ ),
                                            _mm_mul_ps( sse_splat<VW>( lhs.m_RowX ), rhs.m_RowW ) ) );
    result.m_RowY = _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( lhs.m_RowY ), rhs.m_RowX ),
                                            _mm_mul_ps( sse_splat<VY>( lhs.m_RowY ), rhs.m_RowY ) ),
                                _mm_add_ps( _mm_mul_ps( sse_splat<VZ>( lhs.m_RowY ), rhs.m_RowZ ),
                                            _mm_mul_ps( sse_splat<VW>( lhs.m_RowY ), rhs.m_RowW ) ) );
    result.m_RowZ = _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( lhs.m_RowZ ), rhs.m_RowX ),
                                            _mm_mul_ps( sse_splat<VY>( lhs.m_RowZ ), rhs.m_RowY ) ),
                                _mm_add_ps( _mm_mul_ps( sse_splat<VZ>( lhs.m_RowZ ), rhs.m_RowZ ),
                                            _mm_mul_ps( sse_splat<VW>( lhs.m_RowZ ), rhs.m_RowW ) ) );
    result.m_RowW = _mm_add_ps( _mm_add_ps( _mm_mul_ps( sse_splat<VX>( lhs.m_RowW ), rhs.m_RowX ),
                                            _mm_mul_ps( sse_splat<VY>( lhs.m_RowW ), rhs.m_RowY ) ),
                                _mm_add_ps( _mm_mul_ps( sse_splat<VZ>( lhs.m_RowW ), rhs.m_RowZ ),
                                            _mm_mul_ps( sse_splat<VW>( lhs.m_RowW ), rhs.m_RowW ) ) );
    #else
    for ( S32 r=0; r<4; r++ )
    {
        for ( S32 c=0; c<4; c++ )
        {
            result.m[r][c] = lhs.m[r][0] * rhs.m[0][c] +
                             lhs.m[r][1] * rhs.m[1][c] +
                             lhs.m[r][2] * rhs.m[2][c] +
                             lhs.m[r][3] * rhs.m[3][c];
        }
    }
    #endif
    return result;
}



/// Mat44 * Mat33 Multiplication operator
NOALIAS __forceinline const Mat44 operator * ( const Mat44 MATREF lhs, const Mat33 MATREF rhs )
{
    return lhs * MakeMat44( rhs );
}

/// Mat33 * Mat44 Multiplication operator
NOALIAS __forceinline const Mat44 operator * ( const Mat33 MATREF lhs, const Mat44 MATREF rhs )
{
    return MakeMat44( lhs ) * rhs;
}


/// Mat44 * Mat43 Multiplication operator
NOALIAS __forceinline const Mat44 operator * ( const Mat44 MATREF lhs, const Mat43 MATREF rhs )
{
    return lhs * MakeMat44( rhs );
}

/// Mat43 * Mat44 Multiplication operator
NOALIAS __forceinline const Mat44 operator * ( const Mat43 MATREF lhs, const Mat44 MATREF rhs )
{
    return MakeMat44( lhs ) * rhs;
}



/// Transpose Mat44 : returns transposed result
NOALIAS __forceinline const Mat44 Transpose( const Mat44 MATREF src )
{
    Mat44 result;
    #if defined( _SPU_ )
    spu_transpose_matrix44( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW, &result.m_RowX, &result.m_RowY, &result.m_RowZ, &result.m_RowW );
    #elif defined( _PPU_ )                                                                                                
    vec_transpose_matrix44( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW, &result.m_RowX, &result.m_RowY, &result.m_RowZ, &result.m_RowW );
    #elif defined( _VMX128_ )                                                                                             
    vmx_transpose_matrix44( src.m_ColX, src.m_ColY, src.m_ColZ, src.m_ColW, &result.m_ColX, &result.m_ColY, &result.m_ColZ, &result.m_ColW );
    #elif defined( _SSE_ )                                                                                                
    sse_transpose_matrix44( src.m_RowX, src.m_RowY, src.m_RowZ, src.m_RowW, &result.m_RowX, &result.m_RowY, &result.m_RowZ, &result.m_RowW );
    #else
    for ( S32 r=0; r<4; r++ )
    {
        for ( S32 c=0; c<4; c++ )
        {
            result.m[r][c] = src.m[c][r];
        }
    }
    #endif
    return result;
}

/// Transpose Mat44 : transposes pDst
NOALIAS __forceinline void Transpose( Mat44 * const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_transpose_matrix44( pDst->m_RowX, pDst->m_RowY, pDst->m_RowZ, pDst->m_RowW, &pDst->m_RowX, &pDst->m_RowY, &pDst->m_RowZ, &pDst->m_RowW );
    #elif defined( _PPU_ )                                                                                                
    vec_transpose_matrix44( pDst->m_RowX, pDst->m_RowY, pDst->m_RowZ, pDst->m_RowW, &pDst->m_RowX, &pDst->m_RowY, &pDst->m_RowZ, &pDst->m_RowW );
    #elif defined( _VMX128_ )                                                                                             
    vmx_transpose_matrix44( pDst->m_ColX, pDst->m_ColY, pDst->m_ColZ, pDst->m_ColW, &pDst->m_ColX, &pDst->m_ColY, &pDst->m_ColZ, &pDst->m_ColW );
    #elif defined( _SSE_ )                                                                                                
    sse_transpose_matrix44( pDst->m_RowX, pDst->m_RowY, pDst->m_RowZ, pDst->m_RowW, &pDst->m_RowX, &pDst->m_RowY, &pDst->m_RowZ, &pDst->m_RowW );
    #else
    for ( S32 r=0; r<4; r++ )
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


/// Trace of Mat44 : sum of diagonal elements
NOALIAS __forceinline const VF32 Trace( const Mat44 MATREF src )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_add( spu_add( spu_splat<VX>( src.m_RowX ),
                                   spu_splat<VY>( src.m_RowY ) ),
                          spu_add( spu_splat<VZ>( src.m_RowZ ),
                                   spu_splat<VW>( src.m_RowW ) ) );
    #elif defined( _PPU_ )
    result.m_V = vec_add( vec_add( vec_splat( src.m_RowX, VX ),
                                   vec_splat( src.m_RowY, VY ) ),
                          vec_add( vec_splat( src.m_RowZ, VZ ),
                                   vec_splat( src.m_RowW, VW ) ) );
    #elif defined( _VMX128_ )
    result.m_V = __vaddfp( __vaddfp( __vspltw( src.m_ColX, VX ),
                                     __vspltw( src.m_ColY, VY ) ),
                           __vaddfp( __vspltw( src.m_ColZ, VZ ),
                                     __vspltw( src.m_ColW, VW ) ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_add_ps( _mm_add_ps( sse_splat<VX>( src.m_RowX ),
                                         sse_splat<VY>( src.m_RowY ) ),
                             _mm_add_ps( sse_splat<VZ>( src.m_RowZ ),
                                         sse_splat<VW>( src.m_RowW ) ) );
    #else
    result.m_V = src.m[0][0] + src.m[1][1] + src.m[2][2] + src.m[3][3];
    #endif
    return result;
}


/// Create a Mat33 sub-matrix from a Mat44 by deleting Row X and Col X
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowX, E_ColX >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = __vpermwi( src.m_ColY, VPERMWI_CONST(VY,VZ,VW,__) );
    result.m_ColY = __vpermwi( src.m_ColZ, VPERMWI_CONST(VY,VZ,VW,__) );
    result.m_ColZ = __vpermwi( src.m_ColW, VPERMWI_CONST(VY,VZ,VW,__) );
    return result;
    #else
    return MakeMat33( MakeVector3( Permute<VY,VZ,VW,__>( RowY( src ) ) ),
                      MakeVector3( Permute<VY,VZ,VW,__>( RowZ( src ) ) ),
                      MakeVector3( Permute<VY,VZ,VW,__>( RowW( src ) ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row X and Col Y
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowX, E_ColY >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = __vpermwi( src.m_ColX, VPERMWI_CONST(VY,VZ,VW,__) );
    result.m_ColY = __vpermwi( src.m_ColZ, VPERMWI_CONST(VY,VZ,VW,__) );
    result.m_ColZ = __vpermwi( src.m_ColW, VPERMWI_CONST(VY,VZ,VW,__) );
    return result;
    #else
    return MakeMat33( MakeVector3( Permute<VX,VZ,VW,__>( RowY( src ) ) ),
                      MakeVector3( Permute<VX,VZ,VW,__>( RowZ( src ) ) ),
                      MakeVector3( Permute<VX,VZ,VW,__>( RowW( src ) ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row X and Col Z
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowX, E_ColZ >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = __vpermwi( src.m_ColX, VPERMWI_CONST(VY,VZ,VW,__) );
    result.m_ColY = __vpermwi( src.m_ColY, VPERMWI_CONST(VY,VZ,VW,__) );
    result.m_ColZ = __vpermwi( src.m_ColW, VPERMWI_CONST(VY,VZ,VW,__) );
    return result;
    #else
    return MakeMat33( MakeVector3( Permute<VX,VY,VW,__>( RowY( src ) ) ),
                      MakeVector3( Permute<VX,VY,VW,__>( RowZ( src ) ) ),
                      MakeVector3( Permute<VX,VY,VW,__>( RowW( src ) ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row X and Col W
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowX, E_ColW >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = __vpermwi( src.m_ColX, VPERMWI_CONST(VY,VZ,VW,__) );
    result.m_ColY = __vpermwi( src.m_ColY, VPERMWI_CONST(VY,VZ,VW,__) );
    result.m_ColZ = __vpermwi( src.m_ColZ, VPERMWI_CONST(VY,VZ,VW,__) );
    return result;
    #else
    return MakeMat33( MakeVector3( RowY( src ) ),
                      MakeVector3( RowZ( src ) ),
                      MakeVector3( RowW( src ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row Y and Col X
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowY, E_ColX >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = __vpermwi( src.m_ColY, VPERMWI_CONST(VX,VZ,VW,__) );
    result.m_ColY = __vpermwi( src.m_ColZ, VPERMWI_CONST(VX,VZ,VW,__) );
    result.m_ColZ = __vpermwi( src.m_ColW, VPERMWI_CONST(VX,VZ,VW,__) );
    return result;
    #else
    return MakeMat33( MakeVector3( Permute<VY,VZ,VW,__>( RowX( src ) ) ),
                      MakeVector3( Permute<VY,VZ,VW,__>( RowZ( src ) ) ),
                      MakeVector3( Permute<VY,VZ,VW,__>( RowW( src ) ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row Y and Col Y
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowY, E_ColY >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = __vpermwi( src.m_ColX, VPERMWI_CONST(VX,VZ,VW,__) );
    result.m_ColY = __vpermwi( src.m_ColZ, VPERMWI_CONST(VX,VZ,VW,__) );
    result.m_ColZ = __vpermwi( src.m_ColW, VPERMWI_CONST(VX,VZ,VW,__) );
    return result;
    #else
    return MakeMat33( MakeVector3( Permute<VX,VZ,VW,__>( RowX( src ) ) ),
                      MakeVector3( Permute<VX,VZ,VW,__>( RowZ( src ) ) ),
                      MakeVector3( Permute<VX,VZ,VW,__>( RowW( src ) ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row Y and Col Z
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowY, E_ColZ >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = __vpermwi( src.m_ColX, VPERMWI_CONST(VX,VZ,VW,__) );
    result.m_ColY = __vpermwi( src.m_ColY, VPERMWI_CONST(VX,VZ,VW,__) );
    result.m_ColZ = __vpermwi( src.m_ColW, VPERMWI_CONST(VX,VZ,VW,__) );
    return result;
    #else
    return MakeMat33( MakeVector3( Permute<VX,VY,VW,__>( RowX( src ) ) ),
                      MakeVector3( Permute<VX,VY,VW,__>( RowZ( src ) ) ),
                      MakeVector3( Permute<VX,VY,VW,__>( RowW( src ) ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row Y and Col W
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowY, E_ColW >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = __vpermwi( src.m_ColX, VPERMWI_CONST(VX,VZ,VW,__) );
    result.m_ColY = __vpermwi( src.m_ColY, VPERMWI_CONST(VX,VZ,VW,__) );
    result.m_ColZ = __vpermwi( src.m_ColZ, VPERMWI_CONST(VX,VZ,VW,__) );
    return result;
    #else
    return MakeMat33( MakeVector3( RowX( src ) ),
                      MakeVector3( RowZ( src ) ),
                      MakeVector3( RowW( src ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row Z and Col X
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowZ, E_ColX >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = __vpermwi( src.m_ColY, VPERMWI_CONST(VX,VY,VW,__) );
    result.m_ColY = __vpermwi( src.m_ColZ, VPERMWI_CONST(VX,VY,VW,__) );
    result.m_ColZ = __vpermwi( src.m_ColW, VPERMWI_CONST(VX,VY,VW,__) );
    return result;
    #else
    return MakeMat33( MakeVector3( Permute<VY,VZ,VW,__>( RowX( src ) ) ),
                      MakeVector3( Permute<VY,VZ,VW,__>( RowY( src ) ) ),
                      MakeVector3( Permute<VY,VZ,VW,__>( RowW( src ) ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row Z and Col Y
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowZ, E_ColY >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = __vpermwi( src.m_ColX, VPERMWI_CONST(VX,VY,VW,__) );
    result.m_ColY = __vpermwi( src.m_ColZ, VPERMWI_CONST(VX,VY,VW,__) );
    result.m_ColZ = __vpermwi( src.m_ColW, VPERMWI_CONST(VX,VY,VW,__) );
    return result;
    #else
    return MakeMat33( MakeVector3( Permute<VX,VZ,VW,__>( RowX( src ) ) ),
                      MakeVector3( Permute<VX,VZ,VW,__>( RowY( src ) ) ),
                      MakeVector3( Permute<VX,VZ,VW,__>( RowW( src ) ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row Z and Col Z
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowZ, E_ColZ >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = __vpermwi( src.m_ColX, VPERMWI_CONST(VX,VY,VW,__) );
    result.m_ColY = __vpermwi( src.m_ColY, VPERMWI_CONST(VX,VY,VW,__) );
    result.m_ColZ = __vpermwi( src.m_ColW, VPERMWI_CONST(VX,VY,VW,__) );
    return result;
    #else
    return MakeMat33( MakeVector3( Permute<VX,VY,VW,__>( RowX( src ) ) ),
                      MakeVector3( Permute<VX,VY,VW,__>( RowY( src ) ) ),
                      MakeVector3( Permute<VX,VY,VW,__>( RowW( src ) ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row Z and Col W
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowZ, E_ColW >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = __vpermwi( src.m_ColX, VPERMWI_CONST(VX,VY,VW,__) );
    result.m_ColY = __vpermwi( src.m_ColY, VPERMWI_CONST(VX,VY,VW,__) );
    result.m_ColZ = __vpermwi( src.m_ColZ, VPERMWI_CONST(VX,VY,VW,__) );
    return result;
    #else
    return MakeMat33( MakeVector3( RowX( src ) ),
                      MakeVector3( RowY( src ) ),
                      MakeVector3( RowW( src ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row W and Col X
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowW, E_ColX >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = src.m_ColY;
    result.m_ColY = src.m_ColZ;
    result.m_ColZ = src.m_ColW;
    return result;
    #else
    return MakeMat33( MakeVector3( Permute<VY,VZ,VW,__>( RowX( src ) ) ),
                      MakeVector3( Permute<VY,VZ,VW,__>( RowY( src ) ) ),
                      MakeVector3( Permute<VY,VZ,VW,__>( RowZ( src ) ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row W and Col Y
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowW, E_ColY >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = src.m_ColX;
    result.m_ColY = src.m_ColZ;
    result.m_ColZ = src.m_ColW;
    return result;
    #else
    return MakeMat33( MakeVector3( Permute<VX,VZ,VW,__>( RowX( src ) ) ),
                      MakeVector3( Permute<VX,VZ,VW,__>( RowY( src ) ) ),
                      MakeVector3( Permute<VX,VZ,VW,__>( RowZ( src ) ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row W and Col Z
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowW, E_ColZ >( const Mat44 MATREF src )
{
    static const int __ = VW;
    #if defined( _VMX128_ )
    Mat33 result;
    result.m_ColX = src.m_ColX;
    result.m_ColY = src.m_ColY;
    result.m_ColZ = src.m_ColW;
    return result;
    #else
    return MakeMat33( MakeVector3( Permute<VX,VY,VW,__>( RowX( src ) ) ),
                      MakeVector3( Permute<VX,VY,VW,__>( RowY( src ) ) ),
                      MakeVector3( Permute<VX,VY,VW,__>( RowZ( src ) ) ) );
    #endif
}

/// Create a Mat33 sub-matrix from a Mat44 by deleting Row W and Col W
template <> NOALIAS __forceinline const Mat33 DeleteRowCol< E_RowW, E_ColW >( const Mat44 MATREF src )
{
    return MakeMat33( src );
}



/// Mat44 split into top-left Mat33, top-right column Vector3, bottom-left row Vector3 & bottom-right VF32
NOALIAS __forceinline void Split( const Mat44 MATREF src,
                                  Mat33   * const __restrict pTL,
                                  Vector3 * const __restrict pTR,
                                  Vector3 * const __restrict pBL,
                                  VF32    * const __restrict pBR )
{
    // [ TL TL TL TR ]
    // [ TL TL TL TR ]
    // [ TL TL TL TR ]
    // [ BL BL BL BR ]
    
    *pTL = MakeMat33( src );
    *pTR = MakeVector3( ColW( src ) );
    *pBL = MakeVector3( RowW( src ) );
    *pBR = Element<3,3>( src );
}

/// Make Mat44 from top-left Mat33, top-right column Vector3, bottom-left row Vector3 & bottom-right VF32
NOALIAS __forceinline const Mat44 MakeMat44(
    const Mat33 MATREF        tl,
    const Vector3 PCREF tr,
    const Vector3 PCREF bl,
    const VF32    PCREF br )
{
    return MakeMat44( MakeVector4( RowX( tl ), X( tr ) ),
                      MakeVector4( RowY( tl ), Y( tr ) ),
                      MakeVector4( RowZ( tl ), Z( tr ) ),
                      MakeVector4( bl, br ) );
}


/// Mat44 split into top-left VF32, top-right row Vector3, bottom-left column Vector3 & bottom-right Mat33
NOALIAS __forceinline void Split( const Mat44 MATREF src,
                                  VF32    * const __restrict pTL,
                                  Vector3 * const __restrict pTR,
                                  Vector3 * const __restrict pBL,
                                  Mat33   * const __restrict pBR )
{
    // [ TL TR TR TR ]
    // [ BL BR BR BR ]
    // [ BL BR BR BR ]
    // [ BL BR BR BR ]
    
    *pTL = Element<0,0>( src );
    *pTR = MakeVector3( RotateLeft<1>( RowX( src ) ) );
    *pBL = MakeVector3( RotateLeft<1>( ColX( src ) ) );
    *pBR = MakeMat33( MakeVector3( RotateLeft<1>( RowY( src ) ) ),
                      MakeVector3( RotateLeft<1>( RowZ( src ) ) ),
                      MakeVector3( RotateLeft<1>( RowW( src ) ) ) );
}

/// Make Mat44 from top-left VF32, top-right row Vector3, bottom-left column Vector3 & bottom-right Mat33
NOALIAS __forceinline const Mat44 MakeMat44(
    const VF32    PCREF tl,
    const Vector3 PCREF tr,
    const Vector3 PCREF bl,
    const Mat33 MATREF        br )
{
    return MakeMat44( MakeVector4( tl, tr ),
                      MakeVector4( X( bl ), RowX( br ) ),
                      MakeVector4( Y( bl ), RowY( br ) ),
                      MakeVector4( Z( bl ), RowZ( br ) ) );
}


/// Mat44 split into top-left row Vector3, top-right VF32, bottom-left Mat33 & bottom-right column Vector3
NOALIAS __forceinline void Split( const Mat44 MATREF src,
                                  Vector3 * const __restrict pTL,
                                  VF32    * const __restrict pTR,
                                  Mat33   * const __restrict pBL,
                                  Vector3 * const __restrict pBR )
{
    // [ TL TL TL TR ]
    // [ BL BL BL BR ]
    // [ BL BL BL BR ]
    // [ BL BL BL BR ]
    
    *pTL = MakeVector3( RowX( src ) );
    *pTR = Element<0,3>( src );
    *pBL = MakeMat33( MakeVector3( RowY( src ) ),
                      MakeVector3( RowZ( src ) ),
                      MakeVector3( RowW( src ) ) );
    *pBR = MakeVector3( RotateLeft<1>( ColW( src ) ) );
}

/// Make Mat44 from top-left row Vector3, top-right VF32, bottom-left Mat33 & bottom-right column Vector3
NOALIAS __forceinline const Mat44 MakeMat44(
    const Vector3 PCREF tl,
    const VF32    PCREF tr,
    const Mat33 MATREF        bl,
    const Vector3 PCREF br )
{
    return MakeMat44( MakeVector4( tl, tr ),
                      MakeVector4( RowX( bl ), X( br ) ),
                      MakeVector4( RowY( bl ), Y( br ) ),
                      MakeVector4( RowZ( bl ), Z( br ) ) );
}


/// Mat44 split into top-left column Vector3, top-right Mat33, bottom-left VF32 & bottom-right row Vector3
NOALIAS __forceinline void Split( const Mat44 MATREF src,
                                  Vector3 * const __restrict pTL,
                                  Mat33   * const __restrict pTR,
                                  VF32    * const __restrict pBL,
                                  Vector3 * const __restrict pBR )
{
    // [ TL0 TR00 TR01 TR02 ]
    // [ TL1 TR10 TR11 TR12 ]
    // [ TL2 TR20 TR21 TR22 ]
    // [ BL  BR0  BR1  BR2  ]
    
    *pTL = MakeVector3( ColX( src ) );
    *pTR = MakeMat33( MakeVector3( RotateLeft<1>( RowX( src ) ) ),
                      MakeVector3( RotateLeft<1>( RowY( src ) ) ),
                      MakeVector3( RotateLeft<1>( RowZ( src ) ) ) );
    *pBL = Element<3,0>( src );
    *pBR = MakeVector3( RotateLeft<1>( RowW( src ) ) );
}

/// Make Mat44 from top-left column Vector3, top-right Mat33, bottom-left VF32 & bottom-right row Vector3
NOALIAS __forceinline const Mat44 MakeMat44(
    const Vector3 PCREF tl,
    const Mat33 MATREF        tr,
    const VF32    PCREF bl,
    const Vector3 PCREF br )
{
    return MakeMat44( MakeVector4( X( tl ), RowX( tr ) ),
                      MakeVector4( Y( tl ), RowY( tr ) ),
                      MakeVector4( Z( tl ), RowZ( tr ) ),
                      MakeVector4( bl, br ) );
}




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


