#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_STORAGE_INL
#define SOFT_MATHS_HVEC_STORAGE_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_STORAGE_INL
#define MATHS_HVEC_STORAGE_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{

//
// HF32 LOAD & STORE FUNCTIONS
//

/// Load HF32 from an F32
template <>
NOALIAS __forceinline const Maths::HF32 Load( F32 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    HF32 result( UNDEFINED );
    result.m_F = __vspltw( __lvlx( pSrc, 0x0 ), VX );
    return result;
    #else
    return HF32( pSrc );
    #endif
}

/// Store HF32 into an F32
template <>
NOALIAS __forceinline void Store( const Maths::HF32& src, F32* const __restrict pDst )
{
    #if defined( _VMX128_ )
    __stvewx( src.m_F, pDst, 0x0 );
    #else
    *pDst = (F32)src;
    #endif
}



//
// VECTOR LOAD & STORE FUNCTIONS
//

/// Load HVec2 from a Float2
template <>
NOALIAS __forceinline const Maths::HVec2 Load( MathsStorage::Float2 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    HVec2 result( UNDEFINED );
    result.m_V = __vrlimi( __loadunalignedvector( pSrc ), __vzero(), VMX_MASK_INSERT_W, 0 );
    return result;
    #elif defined ( _SSE_ )
    HVec2 result( UNDEFINED );
    result.m_V = _mm_and_ps( _mm_loadu_ps( (const F32*)pSrc ), SIMD_Mask<1,1,0,0>() );
    return result;
    #else
    return HVec2( (F32*)pSrc );
    #endif
}

/// Load HVec2 from a Float2A16
template <>
NOALIAS __forceinline const Maths::HVec2 Load( MathsStorage::Float2A16 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    HVec2 result( UNDEFINED );
    result.m_V = __lvx(pSrc, 0);
    return result;
    #elif defined ( _SSE_ )
    HVec2 result( UNDEFINED );
    result.m_V = _mm_and_ps( _mm_load_ps( (const F32*)pSrc ), SIMD_Mask<1,1,0,0>() );
    return result;
    #else
    return HVec2( (F32*)pSrc );
    #endif
}

/// Store HVec2 into a Float2
template <> NOALIAS __forceinline void Store( const Maths::HVec2& src, MathsStorage::Float2* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VY ), pDst, 0x04 ); ReadWriteBarrier();
    #elif defined ( _SSE_ )
    _mm_store_ss( &pDst->x, src.m_V );
    _mm_store_ss( &pDst->y, _mm_shuffle_ps( src.m_V, src.m_V, _MM_SHUFFLE(VY,VY,VY,VY) ) );
    #else
    pDst->x = src.X();
    pDst->y = src.Y();
    #endif
}

/// Store HVec2 into a Float2A16
template <> NOALIAS __forceinline void Store( const Maths::HVec2& src, MathsStorage::Float2A16* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VY ), pDst, 0x04 ); ReadWriteBarrier();
    #elif defined ( _SSE_ )
    _mm_store_ss( &pDst->x, src.m_V );
    _mm_store_ss( &pDst->y, _mm_shuffle_ps( src.m_V, src.m_V, _MM_SHUFFLE(VY,VY,VY,VY) ) );
    #else
    pDst->x = src.X();
    pDst->y = src.Y();
    #endif
}

/// Load HVec3 from a Float3
template <> NOALIAS __forceinline const Maths::HVec3 Load( MathsStorage::Float3 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __vrlimi( __loadunalignedvector( pSrc ), __vzero(), VMX_MASK_INSERT_W, 0 );
    return result;
    #elif defined ( _SSE_ )
    HVec3 result( UNDEFINED );
    result.m_V = _mm_and_ps( _mm_loadu_ps( (const F32*)pSrc ), SIMD_Mask<1,1,1,0>() );
    return result;
    #else
    return HVec3( pSrc );
    #endif
}

/// Store HVec3 into a Float3
template <> NOALIAS __forceinline void Store( const Maths::HVec3& src, MathsStorage::Float3* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    #elif defined ( _SSE_ )
    _mm_store_ss( &pDst->x, src.m_V );
    _mm_store_ss( &pDst->y, _mm_shuffle_ps( src.m_V, src.m_V, _MM_SHUFFLE(VY,VY,VY,VY) ) );
    _mm_store_ss( &pDst->z, _mm_shuffle_ps( src.m_V, src.m_V, _MM_SHUFFLE(VZ,VZ,VZ,VZ) ) );
    #else
    pDst->x = src.X();
    pDst->y = src.Y();
    pDst->z = src.Z();
    #endif
}

/// Load HVec3 from a Float3A16
template <> NOALIAS __forceinline const Maths::HVec3 Load( MathsStorage::Float3A16 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __lvx( pSrc, 0 );
    return result;
    #elif defined( _SSE_ )
    HVec3 result( UNDEFINED );
    result.m_V = _mm_load_ps( (const F32*)pSrc );
    return result;
    #else
    return HVec3( (F32*)pSrc );
    #endif
}

/// Store HVec3 into a Float3A16
template <> NOALIAS __forceinline void Store( const Maths::HVec3& src, MathsStorage::Float3A16* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    ReadWriteBarrier();
    Maths::V128 AlignedV;
    AlignedV = __vsldoi(src.m_V, src.m_V, 3 << 2);
    __stvrx(AlignedV, pDst, 12);
    #elif defined( _SSE_ )
    _mm_store_ss( &pDst->x, src.m_V );
    _mm_store_ss( &pDst->y, _mm_shuffle_ps( src.m_V, src.m_V, _MM_SHUFFLE(VY,VY,VY,VY) ) );
    _mm_store_ss( &pDst->z, _mm_shuffle_ps( src.m_V, src.m_V, _MM_SHUFFLE(VZ,VZ,VZ,VZ) ) );
    #else
    pDst->x = src.X();
    pDst->y = src.Y();
    pDst->z = src.Z();
    #endif
}

/// Load HVec3 from a Float4A16 
template <> NOALIAS __forceinline const Maths::HVec3 Load( MathsStorage::Float4A16 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    HVec3 result( UNDEFINED );
    result.m_V = __lvx( pSrc, 0 );
    return result;
    #elif defined( _SSE_ )
    HVec3 result( UNDEFINED );
    result.m_V = _mm_load_ps( (const F32*)pSrc );
    return result;
    #else
    return HVec3( (F32*)pSrc );
    #endif
}   

/// Store HVec3 into a Float4A16
template <> NOALIAS __forceinline void Store( const Maths::HVec3& src, MathsStorage::Float4A16* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    __stvx( src.m_V, pDst, 0 );
    #elif defined( _SSE_ )
    _mm_store_ps( (F32*)pDst, src.m_V );
    #else
    pDst->x = src.X();
    pDst->y = src.Y();
    pDst->z = src.Z();
    #endif
}

/// Load HVec3 from a HenDN3
template <> NOALIAS __forceinline const Maths::HVec3 Load( MathsStorage::HenDN3 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    __vector4               V;
    __vector4               ShiftR;
    static const __vector4i Scale = {0x3A802008, 0x3A802008, 0x3B004020, 0}; // {1.0f / (FLOAT)((1 << (11 - 1)) - 1), ..., 1.0f / (FLOAT)((1 << (10 - 1)) - 1)};
    static const __vector4i Shift = {((32 - 11) << 16) | (32 - 11), ((32 - 11) << 16) | (32 - 22), ((32 - 10) << 16) | (32 - 32), 0};

     V = __lvlx(pSrc, 0);
    ShiftR = __vspltisw(-16); // Shift 16
    V = __vspltw(V, 0);
    ShiftR = __vsrw(*(__vector4*)Shift, ShiftR);
    V = __vslw(V, *(__vector4*)Shift);
    V = __vsraw(V, ShiftR);
    V = __vcfsx(V, 0);
    V = __vmulfp(V, *(__vector4*)Scale);
	return Maths::HVec3(V);
    #else
    static const U32 SignExtendXY[] = {0x00000000, 0xFFFFF800};
    static const U32 SignExtendZ[] = {0x00000000, 0xFFFFFC00};

    HVec3 v;
    U32 element = pSrc->v & 0x7FF;
    v.X((F32)(S16)(element | SignExtendXY[element >> 10]) / 1023.0f);
    element = (pSrc->v >> 11) & 0x7FF;
    v.Y((F32)(S16)(element | SignExtendXY[element >> 10]) / 1023.0f);
    element = (pSrc->v >> 22) & 0x3FF;
    v.Z((F32)(S16)(element | SignExtendZ[element >> 9]) / 511.0f);

    return v;
    #endif
}

/// Store HVec3 into a HenDN3
template <> NOALIAS __forceinline void Store( const Maths::HVec3& src, MathsStorage::HenDN3* const __restrict pDst )
{
    using namespace Maths;
    
    #if defined( _VMX128_ )
    
    __vector4               Convert;
    __vector4               Select;
    __vector4               Scale, ScaleXY;
    __vector4               X, Y, Z, XY;
    __vector4               S;
    static const __vector4i C0 = {0x43FF8000, 0x0007FF0B, 0x3FFFFF16, 0x447FC000}; // (FLOAT)((1 << (10 - 1)) - 1), ..., (FLOAT)((1 << (11 - 1)) - 1)

    ScaleXY = __vspltw(*(__vector4*)C0, 3);
    Scale = __vsldoi(ScaleXY, *(__vector4*)C0, 2 << 2);

    Convert = __vmulfp(src.m_V, Scale);
    Select = __vspltisw(8);
    Convert = __vrfin(Convert);
    Select = __vsrw(*(__vector4*)C0, Select);
    Convert = __vctsxs(Convert, 0);
    XY = __vspltw(Select, 1);
    S = __vspltw(Select, 2);
    Convert = __vslw(Convert, *(__vector4*)C0);
    X = __vspltw(Convert, 0);
    Y = __vspltw(Convert, 1);
    Z = __vspltw(Convert, 2);
    XY = __vsel(Y, X, XY);
    S = __vsel(Z, XY, S);

    __stvewx(S, pDst, 0);
    
    #else
    
    HVec3 scale( 1023.0f, 1023.0f, 511.0f );
    HVec3 v( src );
    
    v = v * scale;
    
    pDst->v =   (((S32)v.Z() & 0x3FF) << 22) |
                (((S32)v.Y() & 0x7FF) << 11) |
                (((S32)v.X() & 0x7FF));
    
    #endif
}

/// Load HVec4 from a Float4
template <> NOALIAS __forceinline const Maths::HVec4 Load( MathsStorage::Float4 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    HVec4 result( UNDEFINED );
    result.m_V = __loadunalignedvector( pSrc );
    return result;
    #else
    return HVec4( (F32*)pSrc );
    #endif
}

/// Store HVec4 into a Float4
template <> NOALIAS __forceinline void Store( const Maths::HVec4& src, MathsStorage::Float4* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VW ), pDst, 0x0C ); ReadWriteBarrier();
    #else
    pDst->x = src.X();
    pDst->y = src.Y();
    pDst->z = src.Z();
    pDst->w = src.W();
    #endif
}

/// Load HVec4 from a Float4A16 
template <> NOALIAS __forceinline const Maths::HVec4 Load( MathsStorage::Float4A16 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    HVec4 result( UNDEFINED );
    result.m_V = __lvx( pSrc, 0 );
    return result;
    #elif defined( _SSE_ )
    HVec4 result( UNDEFINED );
    result.m_V = _mm_load_ps( (const F32*)pSrc );
    return result;
    #else
    return HVec4( (F32*)pSrc );
    #endif
}   

/// Store HVec4 into a Float4A16
template <> NOALIAS __forceinline void Store( const Maths::HVec4& src, MathsStorage::Float4A16* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    __stvx( src.m_V, pDst, 0 );
    #elif defined( _SSE_ )
    _mm_store_ps( (F32*)pDst, src.m_V );
    #else
    pDst->x = src.X();
    pDst->y = src.Y();
    pDst->z = src.Z();
    pDst->w = src.W();
    #endif
}


/// Load C_RGBA_F32 from a Float4
template <> NOALIAS __forceinline const Maths::C_RGBA_F32 Load( MathsStorage::Float4 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    C_RGBA_F32 result;
    result.m_V.m_V = __loadunalignedvector( pSrc );
    return result;
    #else
    return C_RGBA_F32( pSrc->x, pSrc->y, pSrc->z, pSrc->w );
    #endif
}

/// Store C_RGBA_F32 into a Float4
template <> NOALIAS __forceinline void Store( const Maths::C_RGBA_F32& src, MathsStorage::Float4* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V.m_V, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V.m_V, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V.m_V, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V.m_V, VW ), pDst, 0x0C ); ReadWriteBarrier();
    #else
    pDst->x = src.R();
    pDst->y = src.G();
    pDst->z = src.B();
    pDst->w = src.A();
    #endif
}

/// Load C_RGBA_F32 from a Float4A16
template <> NOALIAS __forceinline const Maths::C_RGBA_F32 Load( MathsStorage::Float4A16 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    C_RGBA_F32 result;
    result.m_V.m_V = __lvx( pSrc, 0 );
    return result;
    #elif defined( _SSE_ )
    C_RGBA_F32 result;
    result.m_V.m_V = _mm_load_ps( (const F32*)pSrc );
    return result;
    #else
    return C_RGBA_F32(HVec4( (F32*)pSrc ));
    #endif
}

/// Store C_RGBA_F32 into a Float4A16
template <> NOALIAS __forceinline void Store( const Maths::C_RGBA_F32& src, MathsStorage::Float4A16* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    __stvx( src.m_V.m_V, pDst, 0 );
    #elif defined( _SSE_ )
    _mm_store_ps( (F32*)pDst, src.m_V.m_V );
    #else
    pDst->x = src.R();
    pDst->y = src.G();
    pDst->z = src.B();
    pDst->w = src.A();
    #endif
}


/// Load C_Quaternion from a Float4
template <> NOALIAS __forceinline const Maths::C_Quaternion Load( MathsStorage::Float4 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    C_Quaternion result;
    result.m_V = __loadunalignedvector( pSrc );
    return result;
    #else
    return C_Quaternion( pSrc->x, pSrc->y, pSrc->z, pSrc->w );
    #endif
}

/// Store C_Quaternion into a Float4
template <> NOALIAS __forceinline void Store( const Maths::C_Quaternion& src, MathsStorage::Float4* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VW ), pDst, 0x0C ); ReadWriteBarrier();
    #else
    pDst->x = src.Imag().X();
    pDst->y = src.Imag().Y();
    pDst->z = src.Imag().Z();
    pDst->w = src.Real().W();
    #endif
}

/// Load C_Quaternion from a Float4A16
template <> NOALIAS __forceinline const Maths::C_Quaternion Load( MathsStorage::Float4A16 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    C_Quaternion result;
    result.m_V = __lvx( pSrc, 0 );
    return result;
    #elif defined( _SSE_ )
    C_Quaternion result;
    result.m_V.m_V = _mm_load_ps( (const F32*)pSrc );
    return result;
    #else
    return C_Quaternion(HVec4( (F32*)pSrc ));
    #endif
}

/// Store C_Quaternion into a Float4A16
template <> NOALIAS __forceinline void Store( const Maths::C_Quaternion& src, MathsStorage::Float4A16* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    __stvx( src.m_V, pDst, 0 );
    #elif defined( _SSE_ )
    _mm_store_ps( (F32*)pDst, src.m_V.m_V );
    #else
    pDst->x = src.Imag().X();
    pDst->y = src.Imag().Y();
    pDst->z = src.Imag().Z();
    pDst->w = src.Real().W();
    #endif
}



/// Load C_Vertex from a Float3
template <> NOALIAS __forceinline const Maths::C_Vertex Load( MathsStorage::Float3 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    return C_Vertex( HVec3( __vrlimi( __loadunalignedvector( pSrc ), __vzero(), VMX_MASK_INSERT_W, 0 ) ) );
    #else
    return C_Vertex( pSrc );
    #endif
}

/// Store C_Vertex into a Float3
template <> NOALIAS __forceinline void Store( const Maths::C_Vertex& src, MathsStorage::Float3* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V.m_V, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V.m_V, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V.m_V, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    #else
    pDst->x = src.X();
    pDst->y = src.Y();
    pDst->z = src.Z();
    #endif
}


/// Load C_Vector from a Float3
template <> NOALIAS __forceinline const Maths::C_Vector Load( MathsStorage::Float3 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    return C_Vector( HVec3( __vrlimi( __loadunalignedvector( pSrc ), __vzero(), VMX_MASK_INSERT_W, 0 ) ) );
    #else
    return C_Vector( pSrc );
    #endif
}

/// Store C_Vector into a Float3
template <> NOALIAS __forceinline void Store( const Maths::C_Vector& src, MathsStorage::Float3* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V.m_V, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V.m_V, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V.m_V, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    #else
    pDst->x = src.X();
    pDst->y = src.Y();
    pDst->z = src.Z();
    #endif
}


/// Load C_Normal from a Float3
template <> NOALIAS __forceinline const Maths::C_Normal Load( MathsStorage::Float3 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    return C_Normal( HVec3( __vrlimi( __loadunalignedvector( pSrc ), __vzero(), VMX_MASK_INSERT_W, 0 ) ), C_Normal::UNIT_LENGTH_VECTOR );
    #else
    return C_Normal( pSrc );
    #endif
}

/// Store C_Normal into a Float3
template <> NOALIAS __forceinline void Store( const Maths::C_Normal& src, MathsStorage::Float3* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V.m_V, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V.m_V, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V.m_V, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    #else
    pDst->x = src.X();
    pDst->y = src.Y();
    pDst->z = src.Z();
    #endif
}


/// Load V128 from a Float4A16 
template <> NOALIAS __forceinline const Maths::V128 Load( MathsStorage::Float4A16 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _SPU_ )
    //return (vector float){ pSrc->x, pSrc->y, pSrc->z, pSrc->w };
    return *((vector float*)pSrc);
    #elif defined( _PPU_ )
    return vec_lvx( 0x0, pSrc->v );
    #elif defined( _VMX128_ )
    return __lvx( pSrc, 0 );
    #elif defined( _SSE_ )
    return _mm_load_ps( (const F32*)pSrc );
    #else
    Maths::V128 v;
    v.m_X = pSrc->x;
    v.m_Y = pSrc->y;
    v.m_Z = pSrc->z;
    v.m_W = pSrc->w;
    return v;
    #endif
}   

/// Store V128 into a Float4A16
template <> NOALIAS __forceinline void Store( const Maths::V128& src, MathsStorage::Float4A16* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _SPU_ )
    *((vector float*)pDst) = src;
    #elif defined( _PPU_ )
    vec_stvx( src, 0x0, pDst->v );
    #elif defined( _VMX128_ )
    __stvx( src, pDst, 0 );
    #elif defined( _SSE_ )
    _mm_store_ps( (F32*)pDst, src );
    #else
    pDst->x = src.m_X;
    pDst->y = src.m_Y;
    pDst->z = src.m_Z;
    pDst->w = src.m_W;
    #endif
}

/// Load V128 from a Float4
template <> NOALIAS __forceinline const Maths::V128 Load( MathsStorage::Float4 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _SPU_ )
    //return (vector float){ pSrc->x, pSrc->y, pSrc->z, pSrc->w };
    return spu_load_unaligned( &pSrc->x );
    #elif defined( _PPU_ )
    return vec_load_unaligned( pSrc->v );
    #elif defined( _VMX128_ )
    return __loadunalignedvector( pSrc );
    #elif defined( _SSE_ )
    return _mm_loadu_ps( (const F32*)pSrc );
    #else
    Maths::V128 result;
    result.m_X = pSrc->x;
    result.m_Y = pSrc->y;
    result.m_Z = pSrc->z;
    result.m_W = pSrc->w;
    return result;
    #endif
}

/// Store V128 into a Float4
template <> NOALIAS __forceinline void Store( const Maths::V128& src, MathsStorage::Float4* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _SPU_ )
    pDst->x = spu_extract( src, VX );
    pDst->y = spu_extract( src, VY );
    pDst->z = spu_extract( src, VZ );
    pDst->w = spu_extract( src, VW );
    #elif defined( _PPU_ )
    vec_store_unaligned_vec4( src, pDst->v );
    #elif defined( _VMX128_ )
    ReadWriteBarrier();
    __stvewx( __vspltw( src, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    __stvewx( __vspltw( src, VW ), pDst, 0x0C ); ReadWriteBarrier();
    #elif defined( _SSE_ )
    _mm_storeu_ps( (F32*)pDst, src );
    #else
    pDst->x = src.m_X;
    pDst->y = src.m_Y;
    pDst->z = src.m_Z;
    pDst->w = src.m_W;
    #endif
}

/// Load V128 from a Float3A16
template <> NOALIAS __forceinline const Maths::V128 Load( MathsStorage::Float3A16 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _SPU_ )
    //return (vector float){ pSrc->x, pSrc->y, pSrc->z, 0.0f };
    return spu_insert( 0.0f, *((vector float*)pSrc), VW );
    #elif defined( _PPU_ )
    return vec_lvx( 0x0, pSrc->v );
    #elif defined( _VMX128_ )
    return __lvx( pSrc, 0 );
    #elif defined( _SSE_ )
    return _mm_load_ps( (const F32*)pSrc );
    #else
    Maths::V128 v;
    v.m_X = pSrc->x;
    v.m_Y = pSrc->y;
    v.m_Z = pSrc->z;
    return v;
    #endif
}

/// Store V128 into a Float3A16
template <> NOALIAS __forceinline void Store( const Maths::V128& src, MathsStorage::Float3A16* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _SPU_ )
    pDst->x = spu_extract( src, VX );
    pDst->y = spu_extract( src, VY );
    pDst->z = spu_extract( src, VZ );
    #elif defined( _PPU_ )
    vec_stvrx( (vector signed char)vec_vsldoi( src, src, 3 << 2 ), 0xC, (signed char*)pDst->v );
    #elif defined( _VMX128_ )
     ReadWriteBarrier();
    Maths::V128 AlignedV;
    AlignedV = __vsldoi(src, src, 3 << 2);
    __stvrx(AlignedV, pDst, 12);
    #elif defined( _SSE_ )
    _mm_store_ss( &pDst->x, src );
    _mm_store_ss( &pDst->y, _mm_shuffle_ps( src, src, _MM_SHUFFLE(VY,VY,VY,VY) ) );
    _mm_store_ss( &pDst->z, _mm_shuffle_ps( src, src, _MM_SHUFFLE(VZ,VZ,VZ,VZ) ) );
    #else
    pDst->x = src.m_X;
    pDst->y = src.m_Y;
    pDst->z = src.m_Z;
    #endif
}

/// Load V128 from a Float3
template <> NOALIAS __forceinline const Maths::V128 Load( MathsStorage::Float3 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _SPU_ )
    //return (vector float){ pSrc->x, pSrc->y, pSrc->z, 0.0f };
    return spu_insert( 0.0f, spu_load_unaligned( &pSrc->x ), VW );
    #elif defined( _PPU_ )
    //return vec_vrlimi( vec_load_unaligned( pSrc->v ), Const< vector float >::Zero(), VMX_MASK_INSERT_W, 0 );
    return vec_select<0,0,0,1>( vec_load_unaligned( pSrc->v ), Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    return __vrlimi( __loadunalignedvector( pSrc ), __vzero(), VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    return _mm_loadu_ps( (const F32*)pSrc );
    #else
    Maths::V128 result;
    result.m_X = pSrc->x;
    result.m_Y = pSrc->y;
    result.m_Z = pSrc->z;
    return result;
    #endif
}

/// Store V128 into a Float3
template <> NOALIAS __forceinline void Store( const Maths::V128& src, MathsStorage::Float3* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _SPU_ )
    pDst->x = spu_extract( src, VX );
    pDst->y = spu_extract( src, VY );
    pDst->z = spu_extract( src, VZ );
    #elif defined( _PPU_ )
    vec_ste( vec_splat( src, VX ), 0, &pDst->x );
    vec_ste( vec_splat( src, VY ), 0, &pDst->y );
    vec_ste( vec_splat( src, VZ ), 0, &pDst->z );
    #elif defined( _VMX128_ )
    ReadWriteBarrier();
    __stvewx( __vspltw( src, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    #elif defined( _SSE_ )
    _mm_store_ss( &pDst->x, src );
    _mm_store_ss( &pDst->y, _mm_shuffle_ps( src, src, _MM_SHUFFLE(VY,VY,VY,VY) ) );
    _mm_store_ss( &pDst->z, _mm_shuffle_ps( src, src, _MM_SHUFFLE(VZ,VZ,VZ,VZ) ) );
    #else
    pDst->x = src.m_X;
    pDst->y = src.m_Y;
    pDst->z = src.m_Z;
    #endif
}

/// Load V128 from a HenDN3
template <> NOALIAS __forceinline const Maths::V128 Load( MathsStorage::HenDN3 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    __vector4               V;
    __vector4               ShiftR;
    static const __vector4i Scale = {0x3A802008, 0x3A802008, 0x3B004020, 0}; // {1.0f / (FLOAT)((1 << (11 - 1)) - 1), ..., 1.0f / (FLOAT)((1 << (10 - 1)) - 1)};
    static const __vector4i Shift = {((32 - 11) << 16) | (32 - 11), ((32 - 11) << 16) | (32 - 22), ((32 - 10) << 16) | (32 - 32), 0};

     V = __lvlx(pSrc, 0);
    ShiftR = __vspltisw(-16); // Shift 16
    V = __vspltw(V, 0);
    ShiftR = __vsrw(*(__vector4*)Shift, ShiftR);
    V = __vslw(V, *(__vector4*)Shift);
    V = __vsraw(V, ShiftR);
    V = __vcfsx(V, 0);
    V = __vmulfp(V, *(__vector4*)Scale);
    return V;
    #else
    static const U32 SignExtendXY[] = {0x00000000, 0xFFFFF800};
    static const U32 SignExtendZ[] = {0x00000000, 0xFFFFFC00};

    HVec3 v;
    U32 element = pSrc->v & 0x7FF;
    v.X((F32)(S16)(element | SignExtendXY[element >> 10]) / 1023.0f);
    element = (pSrc->v >> 11) & 0x7FF;
    v.Y((F32)(S16)(element | SignExtendXY[element >> 10]) / 1023.0f);
    element = (pSrc->v >> 22) & 0x3FF;
    v.Z((F32)(S16)(element | SignExtendZ[element >> 9]) / 511.0f);

    return v;
    #endif
}

/// Store V128 into a HenDN3
template <> NOALIAS __forceinline void Store( const Maths::V128& src, MathsStorage::HenDN3* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    __vector4               Convert;
    __vector4               Select;
    __vector4               Scale, ScaleXY;
    __vector4               X, Y, Z, XY;
    __vector4               S;
    static const __vector4i C0 = {0x43FF8000, 0x0007FF0B, 0x3FFFFF16, 0x447FC000}; // (FLOAT)((1 << (10 - 1)) - 1), ..., (FLOAT)((1 << (11 - 1)) - 1)

    ScaleXY = __vspltw(*(__vector4*)C0, 3);
    Scale = __vsldoi(ScaleXY, *(__vector4*)C0, 2 << 2);

    Convert = __vmulfp(src, Scale);
    Select = __vspltisw(8);
    Convert = __vrfin(Convert);
    Select = __vsrw(*(__vector4*)C0, Select);
    Convert = __vctsxs(Convert, 0);
    XY = __vspltw(Select, 1);
    S = __vspltw(Select, 2);
    Convert = __vslw(Convert, *(__vector4*)C0);
    X = __vspltw(Convert, 0);
    Y = __vspltw(Convert, 1);
    Z = __vspltw(Convert, 2);
    XY = __vsel(Y, X, XY);
    S = __vsel(Z, XY, S);

    __stvewx(S, pDst, 0);
    #else
    HVec3 scale(1023.0f, 1023.0f, 511.0f);
    HVec3 v(src);
    
    v = v * scale;
    pDst->v =   (((S32)v.Z() & 0x3FF) << 22) |
                (((S32)v.Y() & 0x7FF) << 11) |
                (((S32)v.X() & 0x7FF));
    #endif
}

/// Load V128 from a UByteN4
template <> NOALIAS __forceinline const Maths::V128 Load( MathsStorage::UByteN4 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    __vector4   Convert;
    __vector4   V0, V1;
    __vector4   Permute;
    __vector4   Zero;
    static const __vector4i UnpackMove = {0x10111201, 0x14151602, 0x18191A03, 0x1C1D1E00};
    static const __vector4  UnpackScale = {3.921568627e-3f, 3.921568627e-3f, 3.921568627e-3f, 3.921568627e-3f};
    static const __vector4i SwizzleWXYZ = {XM_PERMUTE_0W, XM_PERMUTE_0X, XM_PERMUTE_0Y, XM_PERMUTE_0Z};
    static const __vector4i NegativeZero = {0x80000000, 0x80000000, 0x80000000, 0x80000000};

    V0 = __lvx(pSrc, 0);
    Permute = __lvsl(pSrc, 0);
    V1 = __lvx(pSrc, 3);
    Zero = __vspltisw(0);
    Convert = __vperm(V0, V1, Permute);
    Convert = __vperm(Convert, Zero, *(__vector4 *) UnpackMove);
    Convert = __vcfux(Convert, 0);
    Convert = __vmaddfp(Convert, UnpackScale, *(__vector4 *) NegativeZero);
    Convert = __vperm(Convert, Convert, *(__vector4 *) SwizzleWXYZ);

    return Convert;
    #else
    HVec4 v;

    F32 scale = 1.0f / 255.0f;

    v.X(pSrc->x * scale);
    v.Y(pSrc->y * scale);
    v.Z(pSrc->z * scale);
    v.W(pSrc->w * scale);

    return v;
    #endif
}

/// Store V128 into a UByteN4
template <> NOALIAS __forceinline void Store( const Maths::V128& src, MathsStorage::UByteN4* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    __vector4 Select, Convert;
    __vector4 Zero;
    __vector4 LessThanZero;
    static const __vector4  PackScale = {6.07967376708984375e-5f, 6.07967376708984375e-5f, 6.07967376708984375e-5f, 6.07967376708984375e-5f}; // 255.0f / (FLOAT)(1 << 22)
    static const __vector4  Three = {3.0f, 3.0f, 3.0f, 3.0f};
    static const __vector4i Permute = {0x07010305, 0x07010305, 0x07010305, 0x07010305};
    static const __vector4i SwizzleYZWX = {XM_PERMUTE_0Y, XM_PERMUTE_0Z, XM_PERMUTE_0W, XM_PERMUTE_0X};

    Zero = __vspltisw(0);
    Select = __vperm(src, src, *(XMVECTOR*)SwizzleYZWX);
    LessThanZero = __vcmpgtfp(Zero, src);
    Convert = __vmaddfp(Select, PackScale, Three);
    LessThanZero = __vpkuhus(LessThanZero, LessThanZero);
    Convert = __vpkuhus(Convert, Convert);
    Convert = __vsel(Convert, Zero, LessThanZero);
    Convert = __vperm(Convert, Convert, *(__vector4*)Permute);

    __stvewx(Convert, pDst, 0);
    #else
    HVec4 scale(255.0f, 255.0f, 255.0f, 255.0f);
    HVec4 v(src);
    
    v = v * scale;
    pDst->v =   (((U32)v.W() & 0xFF) << 24) |
                (((U32)v.Z() & 0xFF) << 16) |
                (((U32)v.Y() & 0xFF) << 8) |
                (((U32)v.X() & 0xFF));
    #endif
}

//
// MATRIX LOAD & STORE FUNCTIONS
//

/// Load HMat33 from a Matrix33
template <> NOALIAS __forceinline const Maths::HMat33 Load( MathsStorage::Matrix33 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    HMat33 result( Load<HMat33>((MathsStorage::Float33*)pSrc ));
    return result.Transposed();
    #else
    return Load<HMat33>((MathsStorage::Float33*)pSrc );
    #endif
}

/// Store HMat33 into a Matrix33
template <> NOALIAS __forceinline void Store( const Maths::HMat33& src, MathsStorage::Matrix33* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    HMat33 store(src);
    store.Transpose();
    Store(store, (MathsStorage::Float33*)pDst);
    #else
    Store(src, (MathsStorage::Float33*)pDst);
    #endif
}



/// Load HMat33 from a Float33
template <> NOALIAS __forceinline const Maths::HMat33 Load( MathsStorage::Float33 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _SPU_ )
    HMat33 result( UNDEFINED );
    //result.m_AX = (vector float){ pSrc->rx_x, pSrc->rx_y, pSrc->rx_z, 0.0f };
    //result.m_AY = (vector float){ pSrc->ry_x, pSrc->ry_y, pSrc->ry_z, 0.0f };
    //result.m_AZ = (vector float){ pSrc->rz_x, pSrc->rz_y, pSrc->rz_z, 0.0f };
    result.m_AX = spu_insert( 0.0f, spu_load_unaligned( &pSrc->rx_x ), VW );
    result.m_AY = spu_insert( 0.0f, spu_load_unaligned( &pSrc->ry_x ), VW );
    result.m_AZ = spu_insert( 0.0f, spu_load_unaligned( &pSrc->rz_x ), VW );
    return result;
    #elif defined( _PPU_ )
    //return HMat33( HVec3( vec_load_unaligned( &pSrc->rx_x ) ),
    //               HVec3( vec_load_unaligned( &pSrc->ry_x ) ),
    //               HVec3( vec_load_unaligned( &pSrc->rz_x ) ) );
    //const vector float mask_xyz = (vector float)(vector unsigned int){ (unsigned int)-1, (unsigned int)-1, (unsigned int)-1, (unsigned int)0 };
    const vector float mask_xyz = (vector float)Const< vector unsigned int >::Mask(1,1,1,0);
    HMat33 result( UNDEFINED );
    result.m_AX = vec_and( vec_load_unaligned( &pSrc->rx_x ), mask_xyz );
    result.m_AY = vec_and( vec_load_unaligned( &pSrc->ry_x ), mask_xyz );
    result.m_AZ = vec_and( vec_load_unaligned( &pSrc->rz_x ), mask_xyz );
    return result;
    #elif defined( _VMX128_ )
    HMat33 result( UNDEFINED );
    result.m_CX = __loadunalignedvector( &pSrc->cx_x );
    result.m_CY = __loadunalignedvector( &pSrc->cy_x );
    result.m_CZ = __loadunalignedvector( &pSrc->cz_x );
    return result;
    #else
    return HMat33( HVec3( pSrc->rx_x, pSrc->rx_y, pSrc->rx_z ),
                   HVec3( pSrc->ry_x, pSrc->ry_y, pSrc->ry_z ),
                   HVec3( pSrc->rz_x, pSrc->rz_y, pSrc->rz_z ) );
    #endif
}

/// Store HMat33 into a Float33
template <> NOALIAS __forceinline void Store( const Maths::HMat33& src, MathsStorage::Float33* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _SPU_ )
    pDst->rx_x = spu_extract( src.m_AX, VX );
    pDst->rx_y = spu_extract( src.m_AX, VY );
    pDst->rx_z = spu_extract( src.m_AX, VZ );
    pDst->ry_x = spu_extract( src.m_AY, VX );
    pDst->ry_y = spu_extract( src.m_AY, VY );
    pDst->ry_z = spu_extract( src.m_AY, VZ );
    pDst->rz_x = spu_extract( src.m_AZ, VX );
    pDst->rz_y = spu_extract( src.m_AZ, VY );
    pDst->rz_z = spu_extract( src.m_AZ, VZ );
    #elif defined( _PPU_ )
    vec_ste( vec_splat( src.m_AX, VX ), 0x0, &pDst->rx_x );
    vec_ste( vec_splat( src.m_AX, VY ), 0x0, &pDst->rx_y );
    vec_ste( vec_splat( src.m_AX, VZ ), 0x0, &pDst->rx_z );
    vec_ste( vec_splat( src.m_AY, VX ), 0x0, &pDst->ry_x );
    vec_ste( vec_splat( src.m_AY, VY ), 0x0, &pDst->ry_y );
    vec_ste( vec_splat( src.m_AY, VZ ), 0x0, &pDst->ry_z );
    vec_ste( vec_splat( src.m_AZ, VX ), 0x0, &pDst->rz_x );
    vec_ste( vec_splat( src.m_AZ, VY ), 0x0, &pDst->rz_y );
    vec_ste( vec_splat( src.m_AZ, VZ ), 0x0, &pDst->rz_z );
    #elif defined( _VMX128_ )
    ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CX, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CX, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CX, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CY, VX ), pDst, 0x0C ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CY, VY ), pDst, 0x10 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CY, VZ ), pDst, 0x14 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CZ, VX ), pDst, 0x18 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CZ, VY ), pDst, 0x1C ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CZ, VZ ), pDst, 0x20 ); ReadWriteBarrier();
    #elif defined( _SSE_ )
    const __m128 __ = _mm_setzero_ps();
    _mm_store_ss( &pDst->rx_x, src.m_AX.m_V );
    _mm_store_ss( &pDst->rx_y, _mm_movehl_ps( __, _mm_unpacklo_ps( src.m_AX.m_V, __ ) ) );
    _mm_store_ss( &pDst->rx_z, _mm_movehl_ps( __, src.m_AX.m_V ) );
    _mm_store_ss( &pDst->ry_x, src.m_AY.m_V );
    _mm_store_ss( &pDst->ry_y, _mm_movehl_ps( __, _mm_unpacklo_ps( src.m_AY.m_V, __ ) ) );
    _mm_store_ss( &pDst->ry_z, _mm_movehl_ps( __, src.m_AY.m_V ) );
    _mm_store_ss( &pDst->rz_x, src.m_AZ.m_V );
    _mm_store_ss( &pDst->rz_y, _mm_movehl_ps( __, _mm_unpacklo_ps( src.m_AZ.m_V, __ ) ) );
    _mm_store_ss( &pDst->rz_z, _mm_movehl_ps( __, src.m_AZ.m_V ) );
    #else
    pDst->rx_x = src.m_A[AX].m_V[VX]; pDst->rx_y = src.m_A[AX].m_V[VY]; pDst->rx_z = src.m_A[AX].m_V[VZ];
    pDst->ry_x = src.m_A[AY].m_V[VX]; pDst->ry_y = src.m_A[AY].m_V[VY]; pDst->ry_z = src.m_A[AY].m_V[VZ];
    pDst->rz_x = src.m_A[AZ].m_V[VX]; pDst->rz_y = src.m_A[AZ].m_V[VY]; pDst->rz_z = src.m_A[AZ].m_V[VZ];
    #endif
}





/// Load HMat43 from a Float43
template <> NOALIAS __forceinline const Maths::HMat43 Load( MathsStorage::Float43 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _SPU_ )
    HMat43 result( UNDEFINED );
    //result.m_AX = (vector float)( pSrc->rx_x, pSrc->rx_y, pSrc->rx_z, 0.0f );
    //result.m_AY = (vector float)( pSrc->ry_x, pSrc->ry_y, pSrc->ry_z, 0.0f );
    //result.m_AZ = (vector float)( pSrc->rz_x, pSrc->rz_y, pSrc->rz_z, 0.0f );
    //result.m_AW = (vector float)( pSrc->rw_x, pSrc->rw_y, pSrc->rw_z, 0.0f );
    result.m_AX = spu_insert( 0.0f, spu_load_unaligned( &pSrc->rx_x ), VW );
    result.m_AY = spu_insert( 0.0f, spu_load_unaligned( &pSrc->ry_x ), VW );
    result.m_AZ = spu_insert( 0.0f, spu_load_unaligned( &pSrc->rz_x ), VW );
    result.m_AW = spu_insert( 0.0f, spu_load_unaligned( &pSrc->rw_x ), VW );
    return result;
    #elif defined( _PPU_ )
    //const vector float mask_xyz = (vector float)(vector unsigned int){ (unsigned int)-1, (unsigned int)-1, (unsigned int)-1, (unsigned int)0 };
    const vector float mask_xyz = (vector float)Const< vector unsigned int >::Mask(1,1,1,0);
    HMat43 result( UNDEFINED );
    result.m_AX = vec_and( vec_load_unaligned( &pSrc->rx_x ), mask_xyz );
    result.m_AY = vec_and( vec_load_unaligned( &pSrc->ry_x ), mask_xyz );
    result.m_AZ = vec_and( vec_load_unaligned( &pSrc->rz_x ), mask_xyz );
    result.m_AW = vec_and( vec_load_unaligned( &pSrc->rw_x ), mask_xyz );
    return result;
    #elif defined( _VMX128_ )
    HMat43 result( UNDEFINED );
    result.m_CX = __loadunalignedvector( &pSrc->cx_x );
    result.m_CY = __loadunalignedvector( &pSrc->cy_x );
    result.m_CZ = __loadunalignedvector( &pSrc->cz_x );
    return result;
    #else
    return HMat43( HVec3( pSrc->rx_x, pSrc->rx_y, pSrc->rx_z ),
                   HVec3( pSrc->ry_x, pSrc->ry_y, pSrc->ry_z ),
                   HVec3( pSrc->rz_x, pSrc->rz_y, pSrc->rz_z ),
                   HVec3( pSrc->rw_x, pSrc->rw_y, pSrc->rw_z ) );
    #endif
}

/// Store HMat43 into a Float43
template <> NOALIAS __forceinline void Store( const Maths::HMat43& src, MathsStorage::Float43* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _SPU_ )
    pDst->rx_x = spu_extract( src.m_AX, VX );
    pDst->rx_y = spu_extract( src.m_AX, VY );
    pDst->rx_z = spu_extract( src.m_AX, VZ );
    pDst->ry_x = spu_extract( src.m_AY, VX );
    pDst->ry_y = spu_extract( src.m_AY, VY );
    pDst->ry_z = spu_extract( src.m_AY, VZ );
    pDst->rz_x = spu_extract( src.m_AZ, VX );
    pDst->rz_y = spu_extract( src.m_AZ, VY );
    pDst->rz_z = spu_extract( src.m_AZ, VZ );
    pDst->rw_x = spu_extract( src.m_AW, VX );
    pDst->rw_y = spu_extract( src.m_AW, VY );
    pDst->rw_z = spu_extract( src.m_AW, VZ );
    #elif defined( _PPU_ )
    vec_ste( vec_splat( src.m_AX, VX ), 0x0, &pDst->rx_x );
    vec_ste( vec_splat( src.m_AX, VY ), 0x0, &pDst->rx_y );
    vec_ste( vec_splat( src.m_AX, VZ ), 0x0, &pDst->rx_z );
    vec_ste( vec_splat( src.m_AY, VX ), 0x0, &pDst->ry_x );
    vec_ste( vec_splat( src.m_AY, VY ), 0x0, &pDst->ry_y );
    vec_ste( vec_splat( src.m_AY, VZ ), 0x0, &pDst->ry_z );
    vec_ste( vec_splat( src.m_AZ, VX ), 0x0, &pDst->rz_x );
    vec_ste( vec_splat( src.m_AZ, VY ), 0x0, &pDst->rz_y );
    vec_ste( vec_splat( src.m_AZ, VZ ), 0x0, &pDst->rz_z );
    vec_ste( vec_splat( src.m_AW, VX ), 0x0, &pDst->rw_x );
    vec_ste( vec_splat( src.m_AW, VY ), 0x0, &pDst->rw_y );
    vec_ste( vec_splat( src.m_AW, VZ ), 0x0, &pDst->rw_z );
    #elif defined( _VMX128_ )
    // NOTE: This will CRASH if you use it to store to Write-Combined or Uncached memory! Use StoreUncached() instead!
    ReadWriteBarrier();
    __storeunalignedvector( src.m_CX, &pDst->cx_x ); ReadWriteBarrier();
    __storeunalignedvector( src.m_CY, &pDst->cy_x ); ReadWriteBarrier();
    __storeunalignedvector( src.m_CZ, &pDst->cz_x ); ReadWriteBarrier();
    #elif defined( _SSE_ )
        //#if defined( __ICL )
        const __m128 __ = _mm_setzero_ps();
        _mm_store_ss( &pDst->rx_x, src.m_Rotate.m_AX.m_V );
        _mm_store_ss( &pDst->rx_y, _mm_movehl_ps( __, _mm_unpacklo_ps( src.m_Rotate.m_AX.m_V, __ ) ) );
        _mm_store_ss( &pDst->rx_z, _mm_movehl_ps( __, src.m_Rotate.m_AX.m_V ) );
        _mm_store_ss( &pDst->ry_x, src.m_Rotate.m_AY.m_V );
        _mm_store_ss( &pDst->ry_y, _mm_movehl_ps( __, _mm_unpacklo_ps( src.m_Rotate.m_AY.m_V, __ ) ) );
        _mm_store_ss( &pDst->ry_z, _mm_movehl_ps( __, src.m_Rotate.m_AY.m_V ) );
        _mm_store_ss( &pDst->rz_x, src.m_Rotate.m_AZ.m_V );
        _mm_store_ss( &pDst->rz_y, _mm_movehl_ps( __, _mm_unpacklo_ps( src.m_Rotate.m_AZ.m_V, __ ) ) );
        _mm_store_ss( &pDst->rz_z, _mm_movehl_ps( __, src.m_Rotate.m_AZ.m_V ) );
        _mm_store_ss( &pDst->rw_x, src.m_Translate.m_V );
        _mm_store_ss( &pDst->rw_y, _mm_movehl_ps( __, _mm_unpacklo_ps( src.m_Translate.m_V, __ ) ) );
        _mm_store_ss( &pDst->rw_z, _mm_movehl_ps( __, src.m_Translate.m_V ) );
        //#else
        //pDst->rx_x = src.m_Rotate.m_AX.m_V.m128_f32[VX]; pDst->rx_y = src.m_Rotate.m_AX.m_V.m128_f32[VY]; pDst->rx_z = src.m_Rotate.m_AX.m_V.m128_f32[VZ];
        //pDst->ry_x = src.m_Rotate.m_AY.m_V.m128_f32[VX]; pDst->ry_y = src.m_Rotate.m_AY.m_V.m128_f32[VY]; pDst->ry_z = src.m_Rotate.m_AY.m_V.m128_f32[VZ];
        //pDst->rz_x = src.m_Rotate.m_AZ.m_V.m128_f32[VX]; pDst->rz_y = src.m_Rotate.m_AZ.m_V.m128_f32[VY]; pDst->rz_z = src.m_Rotate.m_AZ.m_V.m128_f32[VZ];
        //pDst->rw_x = src.m_Translate.m_V.m128_f32[VX];   pDst->rw_y = src.m_Translate.m_V.m128_f32[VY];   pDst->rw_z = src.m_Translate.m_V.m128_f32[VZ];
        //#endif
    #else
    pDst->rx_x = src.m_Rotate.m_A[AX].m_V[VX]; pDst->rx_y = src.m_Rotate.m_A[AX].m_V[VY]; pDst->rx_z = src.m_Rotate.m_A[AX].m_V[VZ];
    pDst->ry_x = src.m_Rotate.m_A[AY].m_V[VX]; pDst->ry_y = src.m_Rotate.m_A[AY].m_V[VY]; pDst->ry_z = src.m_Rotate.m_A[AY].m_V[VZ];
    pDst->rz_x = src.m_Rotate.m_A[AZ].m_V[VX]; pDst->rz_y = src.m_Rotate.m_A[AZ].m_V[VY]; pDst->rz_z = src.m_Rotate.m_A[AZ].m_V[VZ];
    pDst->rw_x = src.m_Translate.m_V[VX];      pDst->rw_y = src.m_Translate.m_V[VY];      pDst->rw_z = src.m_Translate.m_V[VZ];
    #endif
}

/// Store HMat43 into a Float43
NOALIAS __forceinline void StoreUncached( const Maths::HMat43& src, MathsStorage::Float43* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _SPU_ )
    pDst->rx_x = spu_extract( src.m_AX, VX );
    pDst->rx_y = spu_extract( src.m_AX, VY );
    pDst->rx_z = spu_extract( src.m_AX, VZ );
    pDst->ry_x = spu_extract( src.m_AY, VX );
    pDst->ry_y = spu_extract( src.m_AY, VY );
    pDst->ry_z = spu_extract( src.m_AY, VZ );
    pDst->rz_x = spu_extract( src.m_AZ, VX );
    pDst->rz_y = spu_extract( src.m_AZ, VY );
    pDst->rz_z = spu_extract( src.m_AZ, VZ );
    pDst->rw_x = spu_extract( src.m_AW, VX );
    pDst->rw_y = spu_extract( src.m_AW, VY );
    pDst->rw_z = spu_extract( src.m_AW, VZ );
    #elif defined( _PPU_ )
    vec_ste( vec_splat( src.m_AX, VX ), 0x0, &pDst->rx_x );
    vec_ste( vec_splat( src.m_AX, VY ), 0x0, &pDst->rx_y );
    vec_ste( vec_splat( src.m_AX, VZ ), 0x0, &pDst->rx_z );
    vec_ste( vec_splat( src.m_AY, VX ), 0x0, &pDst->ry_x );
    vec_ste( vec_splat( src.m_AY, VY ), 0x0, &pDst->ry_y );
    vec_ste( vec_splat( src.m_AY, VZ ), 0x0, &pDst->ry_z );
    vec_ste( vec_splat( src.m_AZ, VX ), 0x0, &pDst->rz_x );
    vec_ste( vec_splat( src.m_AZ, VY ), 0x0, &pDst->rz_y );
    vec_ste( vec_splat( src.m_AZ, VZ ), 0x0, &pDst->rz_z );
    vec_ste( vec_splat( src.m_AW, VX ), 0x0, &pDst->rw_x );
    vec_ste( vec_splat( src.m_AW, VY ), 0x0, &pDst->rw_y );
    vec_ste( vec_splat( src.m_AW, VZ ), 0x0, &pDst->rw_z );
    #elif defined( _VMX128_ )
    // NOTE: Less optimal than the above function but won't crash when used to store to Write-Combined or Uncached memory. Don't use for stores to cached memory!
    ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CX, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CX, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CX, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CX, VW ), pDst, 0x0C ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CY, VX ), pDst, 0x10 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CY, VY ), pDst, 0x14 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CY, VZ ), pDst, 0x18 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CY, VW ), pDst, 0x1C ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CZ, VX ), pDst, 0x20 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CZ, VY ), pDst, 0x24 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CZ, VZ ), pDst, 0x28 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_CZ, VW ), pDst, 0x2C ); ReadWriteBarrier();
    #elif defined( _SSE_ )
        //#if defined( __ICL )
        const __m128 __ = _mm_setzero_ps();
        _mm_store_ss( &pDst->rx_x, src.m_Rotate.m_AX.m_V );
        _mm_store_ss( &pDst->rx_y, _mm_movehl_ps( __, _mm_unpacklo_ps( src.m_Rotate.m_AX.m_V, __ ) ) );
        _mm_store_ss( &pDst->rx_z, _mm_movehl_ps( __, src.m_Rotate.m_AX.m_V ) );
        _mm_store_ss( &pDst->ry_x, src.m_Rotate.m_AY.m_V );
        _mm_store_ss( &pDst->ry_y, _mm_movehl_ps( __, _mm_unpacklo_ps( src.m_Rotate.m_AY.m_V, __ ) ) );
        _mm_store_ss( &pDst->ry_z, _mm_movehl_ps( __, src.m_Rotate.m_AY.m_V ) );
        _mm_store_ss( &pDst->rz_x, src.m_Rotate.m_AZ.m_V );
        _mm_store_ss( &pDst->rz_y, _mm_movehl_ps( __, _mm_unpacklo_ps( src.m_Rotate.m_AZ.m_V, __ ) ) );
        _mm_store_ss( &pDst->rz_z, _mm_movehl_ps( __, src.m_Rotate.m_AZ.m_V ) );
        _mm_store_ss( &pDst->rw_x, src.m_Translate.m_V );
        _mm_store_ss( &pDst->rw_y, _mm_movehl_ps( __, _mm_unpacklo_ps( src.m_Translate.m_V, __ ) ) );
        _mm_store_ss( &pDst->rw_z, _mm_movehl_ps( __, src.m_Translate.m_V ) );
        //#else
        //pDst->rx_x = src.m_Rotate.m_AX.m_V.m128_f32[VX]; pDst->rx_y = src.m_Rotate.m_AX.m_V.m128_f32[VY]; pDst->rx_z = src.m_Rotate.m_AX.m_V.m128_f32[VZ];
        //pDst->ry_x = src.m_Rotate.m_AY.m_V.m128_f32[VX]; pDst->ry_y = src.m_Rotate.m_AY.m_V.m128_f32[VY]; pDst->ry_z = src.m_Rotate.m_AY.m_V.m128_f32[VZ];
        //pDst->rz_x = src.m_Rotate.m_AZ.m_V.m128_f32[VX]; pDst->rz_y = src.m_Rotate.m_AZ.m_V.m128_f32[VY]; pDst->rz_z = src.m_Rotate.m_AZ.m_V.m128_f32[VZ];
        //pDst->rw_x = src.m_Translate.m_V.m128_f32[VX];   pDst->rw_y = src.m_Translate.m_V.m128_f32[VY];   pDst->rw_z = src.m_Translate.m_V.m128_f32[VZ];
        //#endif
    #else
    pDst->rx_x = src.m_Rotate.m_A[AX].m_V[VX]; pDst->rx_y = src.m_Rotate.m_A[AX].m_V[VY]; pDst->rx_z = src.m_Rotate.m_A[AX].m_V[VZ];
    pDst->ry_x = src.m_Rotate.m_A[AY].m_V[VX]; pDst->ry_y = src.m_Rotate.m_A[AY].m_V[VY]; pDst->ry_z = src.m_Rotate.m_A[AY].m_V[VZ];
    pDst->rz_x = src.m_Rotate.m_A[AZ].m_V[VX]; pDst->rz_y = src.m_Rotate.m_A[AZ].m_V[VY]; pDst->rz_z = src.m_Rotate.m_A[AZ].m_V[VZ];
    pDst->rw_x = src.m_Translate.m_V[VX];      pDst->rw_y = src.m_Translate.m_V[VY];      pDst->rw_z = src.m_Translate.m_V[VZ];
    #endif
}




/// Load HMat44 from a Float44
template <> NOALIAS __forceinline const Maths::HMat44 Load( MathsStorage::Float44 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _SPU_ )
    HMat44 result( UNDEFINED );
    //result.m_AX = (vector float){ pSrc->rx_x, pSrc->rx_y, pSrc->rx_z, pSrc->rx_w };
    //result.m_AY = (vector float){ pSrc->ry_x, pSrc->ry_y, pSrc->ry_z, pSrc->ry_w };
    //result.m_AZ = (vector float){ pSrc->rz_x, pSrc->rz_y, pSrc->rz_z, pSrc->rz_w };
    //result.m_AW = (vector float){ pSrc->rw_x, pSrc->rw_y, pSrc->rw_z, pSrc->rw_w };
    result.m_AX = spu_load_unaligned( &pSrc->rx_x );
    result.m_AY = spu_load_unaligned( &pSrc->ry_x );
    result.m_AZ = spu_load_unaligned( &pSrc->rz_x );
    result.m_AW = spu_load_unaligned( &pSrc->rw_x );
    return result;
    #elif defined( _PPU_ )
    HMat44 result( UNDEFINED );
    result.m_AX = vec_load_unaligned( &pSrc->rx_x );
    result.m_AY = vec_load_unaligned( &pSrc->ry_x );
    result.m_AZ = vec_load_unaligned( &pSrc->rz_x );
    result.m_AW = vec_load_unaligned( &pSrc->rw_x );
    return result;
    #elif defined( _VMX128_ )
    HMat44 result( UNDEFINED );
    result.m_CX = __loadunalignedvector( &pSrc->cx_x );
    result.m_CY = __loadunalignedvector( &pSrc->cy_x );
    result.m_CZ = __loadunalignedvector( &pSrc->cz_x );
    result.m_CW = __loadunalignedvector( &pSrc->cw_x );
    return result;
    #else
    return HMat44( HVec4( pSrc->rx_x, pSrc->rx_y, pSrc->rx_z, pSrc->rx_w ),
                   HVec4( pSrc->ry_x, pSrc->ry_y, pSrc->ry_z, pSrc->ry_w ),
                   HVec4( pSrc->rz_x, pSrc->rz_y, pSrc->rz_z, pSrc->rz_w ),
                   HVec4( pSrc->rw_x, pSrc->rw_y, pSrc->rw_z, pSrc->rw_w ) );
    #endif
}

/// Store HMat44 into a Float44
template <> NOALIAS __forceinline void Store( const Maths::HMat44& src, MathsStorage::Float44* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _SPU_ )
    pDst->rx_x = spu_extract( src.m_AX, VX );
    pDst->rx_y = spu_extract( src.m_AX, VY );
    pDst->rx_z = spu_extract( src.m_AX, VZ );
    pDst->rx_w = spu_extract( src.m_AX, VW );
    pDst->ry_x = spu_extract( src.m_AY, VX );
    pDst->ry_y = spu_extract( src.m_AY, VY );
    pDst->ry_z = spu_extract( src.m_AY, VZ );
    pDst->ry_w = spu_extract( src.m_AY, VW );
    pDst->rz_x = spu_extract( src.m_AZ, VX );
    pDst->rz_y = spu_extract( src.m_AZ, VY );
    pDst->rz_z = spu_extract( src.m_AZ, VZ );
    pDst->rz_w = spu_extract( src.m_AZ, VW );
    pDst->rw_x = spu_extract( src.m_AW, VX );
    pDst->rw_y = spu_extract( src.m_AW, VY );
    pDst->rw_z = spu_extract( src.m_AW, VZ );
    pDst->rw_w = spu_extract( src.m_AW, VW );
    #elif defined( _PPU_ )
    vec_ste( vec_splat( src.m_AX, VX ), 0x0, &pDst->rx_x );
    vec_ste( vec_splat( src.m_AX, VY ), 0x0, &pDst->rx_y );
    vec_ste( vec_splat( src.m_AX, VZ ), 0x0, &pDst->rx_z );
    vec_ste( vec_splat( src.m_AX, VW ), 0x0, &pDst->rx_w );
    vec_ste( vec_splat( src.m_AY, VX ), 0x0, &pDst->ry_x );
    vec_ste( vec_splat( src.m_AY, VY ), 0x0, &pDst->ry_y );
    vec_ste( vec_splat( src.m_AY, VZ ), 0x0, &pDst->ry_z );
    vec_ste( vec_splat( src.m_AY, VW ), 0x0, &pDst->ry_w );
    vec_ste( vec_splat( src.m_AZ, VX ), 0x0, &pDst->rz_x );
    vec_ste( vec_splat( src.m_AZ, VY ), 0x0, &pDst->rz_y );
    vec_ste( vec_splat( src.m_AZ, VZ ), 0x0, &pDst->rz_z );
    vec_ste( vec_splat( src.m_AZ, VW ), 0x0, &pDst->rz_w );
    vec_ste( vec_splat( src.m_AW, VX ), 0x0, &pDst->rw_x );
    vec_ste( vec_splat( src.m_AW, VY ), 0x0, &pDst->rw_y );
    vec_ste( vec_splat( src.m_AW, VZ ), 0x0, &pDst->rw_z );
    vec_ste( vec_splat( src.m_AW, VW ), 0x0, &pDst->rw_w );
    #elif defined( _VMX128_ )
    ReadWriteBarrier();
    __storeunalignedvector( src.m_CX, &pDst->cx_x ); ReadWriteBarrier();
    __storeunalignedvector( src.m_CY, &pDst->cy_x ); ReadWriteBarrier();
    __storeunalignedvector( src.m_CZ, &pDst->cz_x ); ReadWriteBarrier();
    __storeunalignedvector( src.m_CW, &pDst->cw_x ); ReadWriteBarrier();
    #elif defined( _SSE_ )
    //pDst->rx_x = src.m_AX.m_V.m128_f32[VX]; pDst->rx_y = src.m_AX.m_V.m128_f32[VY]; pDst->rx_z = src.m_AX.m_V.m128_f32[VZ]; pDst->rx_w = src.m_AX.m_V.m128_f32[VW];
    //pDst->ry_x = src.m_AY.m_V.m128_f32[VX]; pDst->ry_y = src.m_AY.m_V.m128_f32[VY]; pDst->ry_z = src.m_AY.m_V.m128_f32[VZ]; pDst->ry_w = src.m_AY.m_V.m128_f32[VW];
    //pDst->rz_x = src.m_AZ.m_V.m128_f32[VX]; pDst->rz_y = src.m_AZ.m_V.m128_f32[VY]; pDst->rz_z = src.m_AZ.m_V.m128_f32[VZ]; pDst->rz_w = src.m_AZ.m_V.m128_f32[VW];
    //pDst->rw_x = src.m_AW.m_V.m128_f32[VX]; pDst->rw_y = src.m_AW.m_V.m128_f32[VY]; pDst->rw_z = src.m_AW.m_V.m128_f32[VZ]; pDst->rw_w = src.m_AW.m_V.m128_f32[VW];
    _mm_storeu_ps( &pDst->rx_x, src.m_AX.m_V );
    _mm_storeu_ps( &pDst->ry_x, src.m_AY.m_V );
    _mm_storeu_ps( &pDst->rz_x, src.m_AZ.m_V );
    _mm_storeu_ps( &pDst->rw_x, src.m_AW.m_V );
    #else
    pDst->rx_x = src.m_A[AX].m_V[VX]; pDst->rx_y = src.m_A[AX].m_V[VY]; pDst->rx_z = src.m_A[AX].m_V[VZ]; pDst->rx_w = src.m_A[AX].m_V[VW];
    pDst->ry_x = src.m_A[AY].m_V[VX]; pDst->ry_y = src.m_A[AY].m_V[VY]; pDst->ry_z = src.m_A[AY].m_V[VZ]; pDst->ry_w = src.m_A[AY].m_V[VW];
    pDst->rz_x = src.m_A[AZ].m_V[VX]; pDst->rz_y = src.m_A[AZ].m_V[VY]; pDst->rz_z = src.m_A[AZ].m_V[VZ]; pDst->rz_w = src.m_A[AZ].m_V[VW];
    pDst->rw_x = src.m_A[AW].m_V[VX]; pDst->rw_y = src.m_A[AW].m_V[VY]; pDst->rw_z = src.m_A[AW].m_V[VZ]; pDst->rw_w = src.m_A[AW].m_V[VW];    
    #endif
}

/// Load HMat44 from a MatrixRow44
template <> NOALIAS __forceinline const Maths::HMat44 Load( MathsStorage::MatrixRow44 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    HMat44 result( Load<HMat44>((MathsStorage::Float44*)pSrc ));
    return result.Transposed();
    #else
    return Load<HMat44>((MathsStorage::Float44*)pSrc );
    #endif
}

/// Store HMat44 into a MatrixRow44
template <> NOALIAS __forceinline void Store( const Maths::HMat44& src, MathsStorage::MatrixRow44* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    HMat44 store(src);
    store.Transpose();
    Store(store, (MathsStorage::Float44*)pDst);
    #else
    Store(src, (MathsStorage::Float44*)pDst);
    #endif
}

/// Load HMat44 from a MatrixCol44
template <> NOALIAS __forceinline const Maths::HMat44 Load( MathsStorage::MatrixCol44 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    return Load<HMat44>((MathsStorage::Float44*)pSrc );
    #else
    HMat44 result( Load<HMat44>((MathsStorage::Float44*)pSrc ));
    return result.Transposed();
    #endif
}

/// Store HMat44 into a MatrixCol44
template <> NOALIAS __forceinline void Store( const Maths::HMat44& src, MathsStorage::MatrixCol44* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    Store(src, (MathsStorage::Float44*)pDst);
    #else
    HMat44 store(src);
    store.Transpose();
    Store(store, (MathsStorage::Float44*)pDst); 
    #endif
}

/// Load HMat44 from a Float44A16
template <> NOALIAS __forceinline const Maths::HMat44 Load( MathsStorage::Float44A16 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _SPU_ )
    HMat44 result( UNDEFINED );
    result.m_AX = *((vector float*)&pSrc->rx_x);
    result.m_AY = *((vector float*)&pSrc->ry_x);
    result.m_AZ = *((vector float*)&pSrc->rz_x);
    result.m_AW = *((vector float*)&pSrc->rw_x);
    return result;
    #elif defined( _PPU_ )
    HMat44 result( UNDEFINED );
    result.m_AX = vec_lvx( 0x0, &pSrc->rx_x );
    result.m_AY = vec_lvx( 0x0, &pSrc->ry_x );
    result.m_AZ = vec_lvx( 0x0, &pSrc->rz_x );
    result.m_AW = vec_lvx( 0x0, &pSrc->rw_x );
    return result;
    #elif defined( _VMX128_ )
    HMat44 result( UNDEFINED );
    result.m_CX = __lvx( &pSrc->cx_x, 0 );
    result.m_CY = __lvx( &pSrc->cy_x, 0 );
    result.m_CZ = __lvx( &pSrc->cz_x, 0 );
    result.m_CW = __lvx( &pSrc->cw_x, 0 );
    return result;
    #elif defined( _SSE_ )
    HMat44 result( UNDEFINED );
    result.m_AX = Maths::HVec4(_mm_load_ps( (const F32*)&pSrc->rx_x ));
    result.m_AY = Maths::HVec4(_mm_load_ps( (const F32*)&pSrc->ry_x ));
    result.m_AZ = Maths::HVec4(_mm_load_ps( (const F32*)&pSrc->rz_x ));
    result.m_AW = Maths::HVec4(_mm_load_ps( (const F32*)&pSrc->rw_x ));
    return result;
    #else
    return HMat44( HVec4( pSrc->rx_x, pSrc->rx_y, pSrc->rx_z, pSrc->rx_w ),
                   HVec4( pSrc->ry_x, pSrc->ry_y, pSrc->ry_z, pSrc->ry_w ),
                   HVec4( pSrc->rz_x, pSrc->rz_y, pSrc->rz_z, pSrc->rz_w ),
                   HVec4( pSrc->rw_x, pSrc->rw_y, pSrc->rw_z, pSrc->rw_w ) );
    #endif
}

/// Store HMat44 into a Float44A16
template <> NOALIAS __forceinline void Store( const Maths::HMat44& src, MathsStorage::Float44A16* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _SPU_ )
    *((vector float*)&pDst->rx_x) = src.m_AX;
    *((vector float*)&pDst->ry_x) = src.m_AY;
    *((vector float*)&pDst->rz_x) = src.m_AZ;
    *((vector float*)&pDst->rw_x) = src.m_AW;
    #elif defined( _PPU_ )
    vec_stvx( src.m_AX, 0x0, &pDst->rx_x );
    vec_stvx( src.m_AY, 0x0, &pDst->ry_x );
    vec_stvx( src.m_AZ, 0x0, &pDst->rz_x );
    vec_stvx( src.m_AW, 0x0, &pDst->rw_x );
    #elif defined( _VMX128_ )
    ReadWriteBarrier();
    __stvx( src.m_CX, pDst, 0x00 ); ReadWriteBarrier();
    __stvx( src.m_CY, pDst, 0x10 ); ReadWriteBarrier();
    __stvx( src.m_CZ, pDst, 0x20 ); ReadWriteBarrier();
    __stvx( src.m_CW, pDst, 0x30 ); ReadWriteBarrier();
    #elif defined( _SSE_ )
    _mm_store_ps( ((F32*)pDst), src.m_AX );
    _mm_store_ps( ((F32*)pDst)+4, src.m_AY );
    _mm_store_ps( ((F32*)pDst)+8, src.m_AZ );
    _mm_store_ps( ((F32*)pDst)+12, src.m_AW );
    #else
    pDst->rx_x = src.m_A[AX].m_V[VX]; pDst->rx_y = src.m_A[AX].m_V[VY]; pDst->rx_z = src.m_A[AX].m_V[VZ]; pDst->rx_w = src.m_A[AX].m_V[VW];
    pDst->ry_x = src.m_A[AY].m_V[VX]; pDst->ry_y = src.m_A[AY].m_V[VY]; pDst->ry_z = src.m_A[AY].m_V[VZ]; pDst->ry_w = src.m_A[AY].m_V[VW];
    pDst->rz_x = src.m_A[AZ].m_V[VX]; pDst->rz_y = src.m_A[AZ].m_V[VY]; pDst->rz_z = src.m_A[AZ].m_V[VZ]; pDst->rz_w = src.m_A[AZ].m_V[VW];
    pDst->rw_x = src.m_A[AW].m_V[VX]; pDst->rw_y = src.m_A[AW].m_V[VY]; pDst->rw_z = src.m_A[AW].m_V[VZ]; pDst->rw_w = src.m_A[AW].m_V[VW];    
    #endif
}


/// Load HMat44 from a MatrixRow44A16
template <> NOALIAS __forceinline const Maths::HMat44 Load( MathsStorage::MatrixRow44A16 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    HMat44 result( Load<HMat44>((MathsStorage::Float44A16*)pSrc ));
    return result.Transposed();
    #else
    return Load<HMat44>((MathsStorage::Float44A16*)pSrc );
    #endif
}

/// Store HMat44 into a MatrixRow44A16
template <> NOALIAS __forceinline void Store( const Maths::HMat44& src, MathsStorage::MatrixRow44A16* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    HMat44 store(src);
    store.Transpose();
    Store(store, (MathsStorage::Float44A16*)pDst);
    #else
    Store(src, (MathsStorage::Float44A16*)pDst);
    #endif
}

/// Load HMat44 from a MatrixCol44A16
template <> NOALIAS __forceinline const Maths::HMat44 Load( MathsStorage::MatrixCol44A16 const * const __restrict pSrc )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    return Load<HMat44>((MathsStorage::Float44A16*)pSrc );
    #else
    HMat44 result( Load<HMat44>((MathsStorage::Float44A16*)pSrc ));
    return result.Transposed();
    #endif
}

/// Store HMat44 into a MatrixCol44A16
template <> NOALIAS __forceinline void Store( const Maths::HMat44& src, MathsStorage::MatrixCol44A16* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    Store(src, (MathsStorage::Float44A16*)pDst);    
    #else
    HMat44 store(src);
    store.Transpose();
    Store(store, (MathsStorage::Float44A16*)pDst);
    #endif
}



//
// *** SI UNIT VERTEX/VECTOR/MATRIX LOAD & STORE ***
//

/// Load C_SiScalar from an F32
template < typename T >
NOALIAS __forceinline const Maths::C_SiScalar<T> LoadScalarSI( F32 const * const __restrict pSrc )
{
    using namespace Maths;
    
    #if defined( _VMX128_ )
    
    C_SiScalar<T> result( UNDEFINED );
    result.m_Scalar.m_F = __vspltw( __lvlx( pSrc, 0x0 ), VX );
    return result;
    
    #elif defined ( _SSE_ )
    
    C_SiScalar<T> result( UNDEFINED );
    const __m128 scalar = _mm_load_ss( (const F32*)pSrc );
    result.m_Scalar.m_F = _mm_shuffle_ps( scalar, scalar, _MM_SHUFFLE(VX,VX,VX,VX) );
    return result;
    
    #else
    
    return C_SiScalar<T>( Load<HF32>( pSrc ) );
    
    #endif
}

/// Load C_SiVertex from a Float3
template < typename T >
NOALIAS __forceinline const Maths::C_SiVertex<T> LoadVertexSI( MathsStorage::Float3 const * const __restrict pSrc )
{
    using namespace Maths;
    
    #if defined( _VMX128_ )
    
    C_SiVertex<T> result( UNDEFINED );
    result.m_Vertex.m_V = __vrlimi( __loadunalignedvector( pSrc ), __vzero(), VMX_MASK_INSERT_W, 0 );
    return result;
    
    #elif defined ( _SSE_ )
    
    C_SiVertex<T> result( UNDEFINED );
    result.m_Vertex.m_V = _mm_and_ps( _mm_loadu_ps( (const F32*)pSrc ), SIMD_Mask<1,1,1,0>() );
    return result;
    
    #else
    
    return C_SiVertex<T>( Load<HVec3>( pSrc ) );
    
    #endif
}

/// Load C_SiVector from a Float3
template < typename T >
NOALIAS __forceinline const Maths::C_SiVector<T> LoadVectorSI( MathsStorage::Float3 const * const __restrict pSrc )
{
    using namespace Maths;
    
    #if defined( _VMX128_ )
    
    C_SiVector<T> result( UNDEFINED );
    result.m_Vector.m_V = __vrlimi( __loadunalignedvector( pSrc ), __vzero(), VMX_MASK_INSERT_W, 0 );
    return result;
    
    #elif defined ( _SSE_ )
    
    C_SiVector<T> result( UNDEFINED );
    result.m_Vector.m_V = _mm_and_ps( _mm_loadu_ps( (const F32*)pSrc ), SIMD_Mask<1,1,1,0>() );
    return result;
    
    #else
    
    return C_SiVector<T>( Load<HVec3>( pSrc ) );
    
    #endif
}

/// Load C_SiQuaternion from a Float4
template < typename T >
NOALIAS __forceinline const Maths::C_SiQuaternion<T> LoadQuaternionSI( MathsStorage::Float4 const * const __restrict pSrc )
{
    using namespace Maths;
    
    #if defined( _VMX128_ )
    
    C_SiQuaternion<T> result( UNDEFINED );
    result.m_Quaternion.m_V = __loadunalignedvector( pSrc );
    return result;
    
    #elif defined ( _SSE_ )
    
    C_SiQuaternion<T> result( UNDEFINED );
    result.m_Quaternion.m_V.m_V = _mm_loadu_ps( (const F32*)pSrc );
    return result;
    
    #else
    
    return C_SiQuaternion<T>( C_Quaternion( Load<HVec4>( pSrc ) ) );
    
    #endif
}

/// Load C_SiRotMatrix from a Matrix33
template < typename T, bool OrthoNorm >
NOALIAS __forceinline const Maths::C_SiRotMatrix<T,OrthoNorm> LoadMatrixSI( MathsStorage::Matrix33 const * const __restrict pSrc )
{
    using namespace Maths;
    
    #if defined( _VMX128_ )
    
    C_SiRotMatrix<T,OrthoNorm> result( UNDEFINED );
    result.m_Matrix.m_CX = __vrlimi( __loadunalignedvector( &pSrc->_11 ), __vzero(), VMX_MASK_INSERT_W, 0 );
    result.m_Matrix.m_CY = __vrlimi( __loadunalignedvector( &pSrc->_21 ), __vzero(), VMX_MASK_INSERT_W, 0 );
    result.m_Matrix.m_CZ = __vrlimi( __loadunalignedvector( &pSrc->_31 ), __vzero(), VMX_MASK_INSERT_W, 0 );
    return result;
    
    #elif defined ( _SSE_ )
    
    C_SiRotMatrix<T,OrthoNorm> result( UNDEFINED );
    result.m_Matrix.m_AX.m_V = _mm_and_ps( _mm_loadu_ps( (const F32*)&pSrc->_11 ), SIMD_Mask<1,1,1,0>() );
    result.m_Matrix.m_AY.m_V = _mm_and_ps( _mm_loadu_ps( (const F32*)&pSrc->_21 ), SIMD_Mask<1,1,1,0>() );
    result.m_Matrix.m_AZ.m_V = _mm_and_ps( _mm_loadu_ps( (const F32*)&pSrc->_31 ), SIMD_Mask<1,1,1,0>() );
    return result;
    
    #else
    
    return C_SiRotMatrix<T,OrthoNorm>( Load<HMat33>( pSrc ) );
    
    #endif
}






/// Store C_SiScalar into an F32
template < typename T >
NOALIAS __forceinline void StoreScalarSI( const Maths::C_SiScalar<T>& src, F32* const __restrict pDst )
{
    using namespace Maths;
    
    #if defined( _VMX128_ )
    
    __stvewx( src.m_Scalar.m_F, pDst, 0x00 ); ReadWriteBarrier();
    
    #elif defined ( _SSE_ )
    
    _mm_store_ss( pDst, src.m_Scalar.m_F );
    
    #else
    
    Store( src.Get(), pDst );
    
    #endif
}

/// Store C_SiVertex into a Float3
template < typename T >
NOALIAS __forceinline void StoreVertexSI( const Maths::C_SiVertex<T>& src, MathsStorage::Float3* const __restrict pDst )
{
    using namespace Maths;
    
    #if defined( _VMX128_ )
    
    #if 0 // Yes, fail
    __stvewx( __vspltw( src.m_Vertex.m_V, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Vertex.m_V, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Vertex.m_V, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    #else // Let's make it as easy as possible for the stupid compiler...
    const __vector4 xyz( src.m_Vertex.m_V );
    const __vector4 x( __vspltw( xyz, VX ) );
    const __vector4 y( __vspltw( xyz, VY ) );
    const __vector4 z( __vspltw( xyz, VZ ) );
    __stvewx( x, pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( y, pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( z, pDst, 0x08 ); ReadWriteBarrier();
    #endif
    
    #elif defined ( _SSE_ )
    
    _mm_store_ss( &pDst->x, src.m_Vertex.m_V );
    _mm_store_ss( &pDst->y, _mm_shuffle_ps( src.m_Vertex.m_V, src.m_Vertex.m_V, _MM_SHUFFLE(VY,VY,VY,VY) ) );
    _mm_store_ss( &pDst->z, _mm_shuffle_ps( src.m_Vertex.m_V, src.m_Vertex.m_V, _MM_SHUFFLE(VZ,VZ,VZ,VZ) ) );
    
    #else
    
    pDst->x = src.Get().X();
    pDst->y = src.Get().Y();
    pDst->z = src.Get().Z();
    
    #endif
}

/// Store C_SiVector into a Float3
template < typename T >
NOALIAS __forceinline void StoreVectorSI( const Maths::C_SiVector<T>& src, MathsStorage::Float3* const __restrict pDst )
{
    using namespace Maths;
    
    #if defined( _VMX128_ )
    
    #if 0 // Yes, the compiler does bongo up even this simple bit of code!
    __stvewx( __vspltw( src.m_Vector.m_V, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Vector.m_V, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Vector.m_V, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    #else // Let's make it as easy as possible for the stupid compiler...
    const __vector4 xyz( src.m_Vector.m_V );
    const __vector4 x( __vspltw( xyz, VX ) );
    const __vector4 y( __vspltw( xyz, VY ) );
    const __vector4 z( __vspltw( xyz, VZ ) );
    __stvewx( x, pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( y, pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( z, pDst, 0x08 ); ReadWriteBarrier();
    #endif
    
    #elif defined ( _SSE_ )
    
    _mm_store_ss( &pDst->x, src.m_Vector.m_V );
    _mm_store_ss( &pDst->y, _mm_shuffle_ps( src.m_Vector.m_V, src.m_Vector.m_V, _MM_SHUFFLE(VY,VY,VY,VY) ) );
    _mm_store_ss( &pDst->z, _mm_shuffle_ps( src.m_Vector.m_V, src.m_Vector.m_V, _MM_SHUFFLE(VZ,VZ,VZ,VZ) ) );
    
    #else
    
    pDst->x = src.Get().X();
    pDst->y = src.Get().Y();
    pDst->z = src.Get().Z();
    
    #endif
}

/// Store C_SiQuaternion into a Float4
template < typename T >
NOALIAS __forceinline void StoreQuaternionSI( const Maths::C_SiQuaternion<T>& src, MathsStorage::Float4* const __restrict pDst )
{
    using namespace Maths;
    
    #if defined( _VMX128_ )
    
    #if 0 // Yes, the compiler does blip up even this simple bit of code!
    __stvewx( __vspltw( src.m_Quaternion.m_V, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Quaternion.m_V, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Quaternion.m_V, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Quaternion.m_V, VW ), pDst, 0x0C ); ReadWriteBarrier();
    #else // Let's make it as easy as possible for the stupid compiler...
    const __vector4 xyzw( src.m_Quaternion.m_V );
    const __vector4 x( __vspltw( xyzw, VX ) );
    const __vector4 y( __vspltw( xyzw, VY ) );
    const __vector4 z( __vspltw( xyzw, VZ ) );
    const __vector4 w( __vspltw( xyzw, VW ) );
    __stvewx( x, pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( y, pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( z, pDst, 0x08 ); ReadWriteBarrier();
    __stvewx( w, pDst, 0x0C ); ReadWriteBarrier();
    #endif
    
    #elif defined ( _SSE_ )
    
    _mm_store_ss( &pDst->x, src.m_Quaternion.m_V );
    _mm_store_ss( &pDst->y, _mm_shuffle_ps( src.m_Quaternion.m_V, src.m_Quaternion.m_V, _MM_SHUFFLE(VY,VY,VY,VY) ) );
    _mm_store_ss( &pDst->z, _mm_shuffle_ps( src.m_Quaternion.m_V, src.m_Quaternion.m_V, _MM_SHUFFLE(VZ,VZ,VZ,VZ) ) );
    _mm_store_ss( &pDst->w, _mm_shuffle_ps( src.m_Quaternion.m_V, src.m_Quaternion.m_V, _MM_SHUFFLE(VW,VW,VW,VW) ) );
    
    #else
    
    const HVec4 xyzw( src.Get() );
    pDst->x = xyzw.X();
    pDst->y = xyzw.Y();
    pDst->z = xyzw.Z();
    pDst->w = xyzw.W();
    
    #endif
}

/// Store C_SiQuaternion into a Matrix33
template < typename T, bool OrthoNorm >
NOALIAS __forceinline void StoreMatrixSI( const Maths::C_SiRotMatrix<T,OrthoNorm>& src, MathsStorage::Matrix33* const __restrict pDst )
{
    using namespace Maths;
    
    #if defined( _VMX128_ )
    
    #if 0 // Yes, the compiler does spang up even this simple bit of code!
    __stvewx( __vspltw( src.m_Matrix.m_CX, VX ), &pDst->_11, 0x0 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Matrix.m_CX, VY ), &pDst->_12, 0x0 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Matrix.m_CX, VZ ), &pDst->_13, 0x0 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Matrix.m_CY, VX ), &pDst->_21, 0x0 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Matrix.m_CY, VY ), &pDst->_22, 0x0 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Matrix.m_CY, VZ ), &pDst->_23, 0x0 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Matrix.m_CZ, VX ), &pDst->_31, 0x0 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Matrix.m_CZ, VY ), &pDst->_32, 0x0 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_Matrix.m_CZ, VZ ), &pDst->_33, 0x0 ); ReadWriteBarrier();
    #else // Let's make it as easy as possible for the stupid compiler...
    const __vector4 cx( src.m_Matrix.m_CX );
    const __vector4 cy( src.m_Matrix.m_CY );
    const __vector4 cz( src.m_Matrix.m_CZ );
    const __vector4 cx_x( __vspltw( cx, VX ) );
    const __vector4 cx_y( __vspltw( cx, VY ) );
    const __vector4 cx_z( __vspltw( cx, VZ ) );
    const __vector4 cy_x( __vspltw( cy, VX ) );
    const __vector4 cy_y( __vspltw( cy, VY ) );
    const __vector4 cy_z( __vspltw( cy, VZ ) );
    const __vector4 cz_x( __vspltw( cz, VX ) );
    const __vector4 cz_y( __vspltw( cz, VY ) );
    const __vector4 cz_z( __vspltw( cz, VZ ) );
    __stvewx( cx_x, &pDst->_11, 0x0 ); ReadWriteBarrier();
    __stvewx( cx_y, &pDst->_12, 0x0 ); ReadWriteBarrier();
    __stvewx( cx_z, &pDst->_13, 0x0 ); ReadWriteBarrier();
    __stvewx( cy_x, &pDst->_21, 0x0 ); ReadWriteBarrier();
    __stvewx( cy_y, &pDst->_22, 0x0 ); ReadWriteBarrier();
    __stvewx( cy_z, &pDst->_23, 0x0 ); ReadWriteBarrier();
    __stvewx( cz_x, &pDst->_31, 0x0 ); ReadWriteBarrier();
    __stvewx( cz_y, &pDst->_32, 0x0 ); ReadWriteBarrier();
    __stvewx( cz_z, &pDst->_33, 0x0 ); ReadWriteBarrier();
    #endif
    
    #elif defined ( _SSE_ )
    
    _mm_store_ss( &pDst->_11, src.m_Matrix.m_AX );
    _mm_store_ss( &pDst->_12, _mm_shuffle_ps( src.m_Matrix.m_AX, src.m_Matrix.m_AX, _MM_SHUFFLE(VY,VY,VY,VY) ) );
    _mm_store_ss( &pDst->_13, _mm_shuffle_ps( src.m_Matrix.m_AX, src.m_Matrix.m_AX, _MM_SHUFFLE(VZ,VZ,VZ,VZ) ) );
    _mm_store_ss( &pDst->_21, src.m_Matrix.m_AY );
    _mm_store_ss( &pDst->_22, _mm_shuffle_ps( src.m_Matrix.m_AY, src.m_Matrix.m_AY, _MM_SHUFFLE(VY,VY,VY,VY) ) );
    _mm_store_ss( &pDst->_23, _mm_shuffle_ps( src.m_Matrix.m_AY, src.m_Matrix.m_AY, _MM_SHUFFLE(VZ,VZ,VZ,VZ) ) );
    _mm_store_ss( &pDst->_31, src.m_Matrix.m_AZ );
    _mm_store_ss( &pDst->_32, _mm_shuffle_ps( src.m_Matrix.m_AZ, src.m_Matrix.m_AZ, _MM_SHUFFLE(VY,VY,VY,VY) ) );
    _mm_store_ss( &pDst->_33, _mm_shuffle_ps( src.m_Matrix.m_AZ, src.m_Matrix.m_AZ, _MM_SHUFFLE(VZ,VZ,VZ,VZ) ) );
    
    #else
    
    const HVec3 ax( src.Get().AX() );
    const HVec3 ay( src.Get().AY() );
    const HVec3 az( src.Get().AZ() );
    pDst->_11 = ax.X();
    pDst->_12 = ax.Y();
    pDst->_13 = ax.Z();
    pDst->_21 = ay.X();
    pDst->_22 = ay.Y();
    pDst->_23 = ay.Z();
    pDst->_31 = az.X();
    pDst->_32 = az.Y();
    pDst->_33 = az.Z();
    
    #endif
}






}; // namespace Maths


#endif // #if defined( INCLUDE_THIS )




