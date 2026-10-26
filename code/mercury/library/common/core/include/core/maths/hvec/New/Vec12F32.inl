#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VEC12F32_INL
#define SOFT_MATHS_HVEC_NEW_VEC12F32_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VEC12F32_INL
#define MATHS_HVEC_NEW_VEC12F32_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vec12F32
/// @{

/// Load Vec12F32 from F32 ptr
template <> NOALIAS __forceinline const Vec12F32 Load( F32 const * const __restrict pSrc )
{
    Vec12F32 result;
    result.m_V0 = Load<Vec4F32>( pSrc );
    result.m_V1 = Load<Vec4F32>( pSrc + 4 );
    result.m_V2 = Load<Vec4F32>( pSrc + 8 );
    return result;
}

/// Store Vec12F32 into F32 ptr
NOALIAS __forceinline void Store( const Vec12F32& data, F32 * const __restrict pDst )
{
    Store( data.m_V0, pDst );
    Store( data.m_V1, pDst + 4 );
    Store( data.m_V2, pDst + 8 );
}


/// Load Vec12F32 from Float9
template <> NOALIAS __forceinline const Vec12F32 Load( MathsStorage::Float9 const * const __restrict pSrc )
{
    Vec12F32 result;
    result.m_V0 = Load<Vec4F32>( (MathsStorage::Float4*)&pSrc->v0 );
    result.m_V1 = Load<Vec4F32>( (MathsStorage::Float4*)&pSrc->v4 );
    result.m_V2 = Load<Vec4F32>( (MathsStorage::Float1*)&pSrc->v8 );
    return result;
}

/// Store Vec12F32 into Float9
NOALIAS __forceinline void Store( const Vec12F32& data, MathsStorage::Float9 * const __restrict pDst )
{
    Store( data.m_V0, (MathsStorage::Float4*)&pDst->v0 );
    Store( data.m_V1, (MathsStorage::Float4*)&pDst->v4 );
    Store( data.m_V2, (MathsStorage::Float1*)&pDst->v8 );
}


/// Load Vec12F32 from Float10
template <> NOALIAS __forceinline const Vec12F32 Load( MathsStorage::Float10 const * const __restrict pSrc )
{
    Vec12F32 result;
    result.m_V0 = Load<Vec4F32>( (MathsStorage::Float4*)&pSrc->v0 );
    result.m_V1 = Load<Vec4F32>( (MathsStorage::Float4*)&pSrc->v4 );
    result.m_V2 = Load<Vec4F32>( (MathsStorage::Float2*)&pSrc->v8 );
    return result;
}

/// Store Vec12F32 into Float10
NOALIAS __forceinline void Store( const Vec12F32& data, MathsStorage::Float10 * const __restrict pDst )
{
    Store( data.m_V0, (MathsStorage::Float4*)&pDst->v0 );
    Store( data.m_V1, (MathsStorage::Float4*)&pDst->v4 );
    Store( data.m_V2, (MathsStorage::Float2*)&pDst->v8 );
}


/// Load Vec12F32 from Float11
template <> NOALIAS __forceinline const Vec12F32 Load( MathsStorage::Float11 const * const __restrict pSrc )
{
    Vec12F32 result;
    result.m_V0 = Load<Vec4F32>( (MathsStorage::Float4*)&pSrc->v0 );
    result.m_V1 = Load<Vec4F32>( (MathsStorage::Float4*)&pSrc->v4 );
    result.m_V2 = Load<Vec4F32>( (MathsStorage::Float3*)&pSrc->v8 );
    return result;
}

/// Store Vec12F32 into Float11
NOALIAS __forceinline void Store( const Vec12F32& data, MathsStorage::Float11 * const __restrict pDst )
{
    Store( data.m_V0, (MathsStorage::Float4*)&pDst->v0 );
    Store( data.m_V1, (MathsStorage::Float4*)&pDst->v4 );
    Store( data.m_V2, (MathsStorage::Float3*)&pDst->v8 );
}


/// Load Vec12F32 from Float12
template <> NOALIAS __forceinline const Vec12F32 Load( MathsStorage::Float12 const * const __restrict pSrc )
{
    Vec12F32 result;
    result.m_V0 = Load<Vec4F32>( (MathsStorage::Float4*)&pSrc->v0 );
    result.m_V1 = Load<Vec4F32>( (MathsStorage::Float4*)&pSrc->v4 );
    result.m_V2 = Load<Vec4F32>( (MathsStorage::Float4*)&pSrc->v8 );
    return result;
}

/// Store Vec12F32 into Float12
NOALIAS __forceinline void Store( const Vec12F32& data, MathsStorage::Float12 * const __restrict pDst )
{
    Store( data.m_V0, (MathsStorage::Float4*)&pDst->v0 );
    Store( data.m_V1, (MathsStorage::Float4*)&pDst->v4 );
    Store( data.m_V2, (MathsStorage::Float4*)&pDst->v8 );
}


/// Load Vec12F32 from StorageVec9F32
template <> NOALIAS __forceinline const Vec12F32 Load( StorageVec9F32 const * const __restrict pSrc )
{
    Vec12F32 result;
    result.m_V0 = Load<Vec4F32>( (StorageVec4F32*)&pSrc->m_Data.v0 );
    result.m_V1 = Load<Vec4F32>( (StorageVec4F32*)&pSrc->m_Data.v4 );
    result.m_V2 = Load<Vec4F32>( (StorageVec1F32*)&pSrc->m_Data.v8 );
    return result;
}

/// Store Vec12F32 into StorageVec9F32
NOALIAS __forceinline void Store( const Vec12F32& data, StorageVec9F32 * const __restrict pDst )
{
    Store( data.m_V0, (StorageVec4F32*)&pDst->m_Data.v0 );
    Store( data.m_V1, (StorageVec4F32*)&pDst->m_Data.v4 );
    Store( data.m_V2, (StorageVec1F32*)&pDst->m_Data.v8 );
}


/// Load Vec12F32 from StorageVec10F32
template <> NOALIAS __forceinline const Vec12F32 Load( StorageVec10F32 const * const __restrict pSrc )
{
    Vec12F32 result;
    result.m_V0 = Load<Vec4F32>( (StorageVec4F32*)&pSrc->m_Data.v0 );
    result.m_V1 = Load<Vec4F32>( (StorageVec4F32*)&pSrc->m_Data.v4 );
    result.m_V2 = Load<Vec4F32>( (StorageVec2F32*)&pSrc->m_Data.v8 );
    return result;
}

/// Store Vec12F32 into StorageVec10F32
NOALIAS __forceinline void Store( const Vec12F32& data, StorageVec10F32 * const __restrict pDst )
{
    Store( data.m_V0, (StorageVec4F32*)&pDst->m_Data.v0 );
    Store( data.m_V1, (StorageVec4F32*)&pDst->m_Data.v4 );
    Store( data.m_V2, (StorageVec2F32*)&pDst->m_Data.v8 );
}


/// Load Vec12F32 from StorageVec11F32
template <> NOALIAS __forceinline const Vec12F32 Load( StorageVec11F32 const * const __restrict pSrc )
{
    Vec12F32 result;
    result.m_V0 = Load<Vec4F32>( (StorageVec4F32*)&pSrc->m_Data.v0 );
    result.m_V1 = Load<Vec4F32>( (StorageVec4F32*)&pSrc->m_Data.v4 );
    result.m_V2 = Load<Vec4F32>( (StorageVec3F32*)&pSrc->m_Data.v8 );
    return result;
}

/// Store Vec12F32 into StorageVec11F32
NOALIAS __forceinline void Store( const Vec12F32& data, StorageVec11F32 * const __restrict pDst )
{
    Store( data.m_V0, (StorageVec4F32*)&pDst->m_Data.v0 );
    Store( data.m_V1, (StorageVec4F32*)&pDst->m_Data.v4 );
    Store( data.m_V2, (StorageVec3F32*)&pDst->m_Data.v8 );
}


/// Load Vec12F32 from StorageVec12F32
template <> NOALIAS __forceinline const Vec12F32 Load( StorageVec12F32 const * const __restrict pSrc )
{
    Vec12F32 result;
    result.m_V0 = Load<Vec4F32>( (StorageVec4F32*)&pSrc->m_Data.v0 );
    result.m_V1 = Load<Vec4F32>( (StorageVec4F32*)&pSrc->m_Data.v4 );
    result.m_V2 = Load<Vec4F32>( (StorageVec4F32*)&pSrc->m_Data.v8 );
    return result;
}

/// Store Vec12F32 into StorageVec12F32
NOALIAS __forceinline void Store( const Vec12F32& data, StorageVec12F32 * const __restrict pDst )
{
    Store( data.m_V0, (StorageVec4F32*)&pDst->m_Data.v0 );
    Store( data.m_V1, (StorageVec4F32*)&pDst->m_Data.v4 );
    Store( data.m_V2, (StorageVec4F32*)&pDst->m_Data.v8 );
}


/// Make Vec12F32 from a VF32 with the same value broadcast into all elements
NOALIAS __forceinline const Vec12F32 MakeVec12F32( const VF32 VECREF e0 )
{
    Vec12F32 result;
    result.m_V0 = result.m_V1 = result.m_V2 = MakeVec4F32( e0 );
    return result;
}

/// Make Vec12F32 from 9 VF32's
NOALIAS __forceinline const Vec12F32 MakeVec12F32( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                   const VF32 VECREF e4, const VF32 VECREF e5, const VF32 VECREF e6, const VF32 VECREF e7,
                                                   const VF32 VECREF e8 )
{
    Vec12F32 result;
    result.m_V0 = MakeVec4F32( e0, e1, e2, e3 );
    result.m_V1 = MakeVec4F32( e4, e5, e6, e7 );
    result.m_V2 = MakeVec4F32( e8 );
    return result;
}

/// Make Vec12F32 from 10 VF32's
NOALIAS __forceinline const Vec12F32 MakeVec12F32( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                   const VF32 VECREF e4, const VF32 VECREF e5, const VF32 VECREF e6, const VF32 VECREF e7,
                                                   const VF32 VECREF e8, const VF32 VECREF e9 )
{
    Vec12F32 result;
    result.m_V0 = MakeVec4F32( e0, e1, e2, e3 );
    result.m_V1 = MakeVec4F32( e4, e5, e6, e7 );
    result.m_V2 = MakeVec4F32( e8, e9 );
    return result;
}

/// Make Vec12F32 from 11 VF32's
NOALIAS __forceinline const Vec12F32 MakeVec12F32( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                   const VF32 VECREF e4, const VF32 VECREF e5, const VF32 VECREF e6, const VF32 VECREF e7,
                                                   const VF32 VECREF e8, const VF32 VECREF e9, const VF32 VECREF e10 )
{
    Vec12F32 result;
    result.m_V0 = MakeVec4F32( e0, e1, e2, e3 );
    result.m_V1 = MakeVec4F32( e4, e5, e6, e7 );
    result.m_V2 = MakeVec4F32( e8, e9, e10 );
    return result;
}

/// Make Vec12F32 from 12 VF32's
NOALIAS __forceinline const Vec12F32 MakeVec12F32( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2,  const VF32 VECREF e3,
                                                   const VF32 VECREF e4, const VF32 VECREF e5, const VF32 VECREF e6,  const VF32 VECREF e7,
                                                   const VF32 VECREF e8, const VF32 VECREF e9, const VF32 VECREF e10, const VF32 VECREF e11 )
{
    Vec12F32 result;
    result.m_V0 = MakeVec4F32( e0, e1, e2, e3 );
    result.m_V1 = MakeVec4F32( e4, e5, e6, e7 );
    result.m_V2 = MakeVec4F32( e8, e9, e10, e11 );
    return result;
}


/// Make Vec12F32 from 3 Vec4F32's
NOALIAS __forceinline const Vec12F32 MakeVec12F32( const Vec4F32 VECREF v0123, const Vec4F32 VECREF v4567, const Vec4F32 VECREF v89AB )
{
    Vec12F32 result;
    result.m_V0 = v0123;
    result.m_V1 = v4567;
    result.m_V2 = v89AB;
    return result;
}

/// Split Vec12F32 into 3 Vec4F32's
NOALIAS __forceinline void Split( const Vec12F32& src, Vec4F32* const __restrict pV0123, Vec4F32* const __restrict pV4567, Vec4F32* const __restrict pV89AB )
{
    *pV0123 = src.m_V0;
    *pV4567 = src.m_V1;
    *pV89AB = src.m_V2;
}


/// Vec12F32 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vec12F32& v )
{
    BIZ_STATIC_ASSERT( I < 12 );
    switch ( I )
    {
        case 0  : return X( v.m_V0 );
        case 1  : return Y( v.m_V0 );
        case 2  : return Z( v.m_V0 );
        case 3  : return W( v.m_V0 );
        case 4  : return X( v.m_V1 );
        case 5  : return Y( v.m_V1 );
        case 6  : return Z( v.m_V1 );
        case 7  : return W( v.m_V1 );
        case 8  : return X( v.m_V2 );
        case 9  : return Y( v.m_V2 );
        case 10 : return Z( v.m_V2 );
        case 11 : return W( v.m_V2 );
        default : return Const<VF32>::Zero();
    }
}

/// Vec12F32 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vec12F32* const __restrict pDst, const VF32 VECREF ei )
{
    BIZ_STATIC_ASSERT( I < 12 );
    switch ( I )
    {
        case 0  : SetX( &pDst->m_V0, ei ); break;
        case 1  : SetY( &pDst->m_V0, ei ); break;
        case 2  : SetZ( &pDst->m_V0, ei ); break;
        case 3  : SetW( &pDst->m_V0, ei ); break;
        case 4  : SetX( &pDst->m_V1, ei ); break;
        case 5  : SetY( &pDst->m_V1, ei ); break;
        case 6  : SetZ( &pDst->m_V1, ei ); break;
        case 7  : SetW( &pDst->m_V1, ei ); break;
        case 8  : SetX( &pDst->m_V2, ei ); break;
        case 9  : SetY( &pDst->m_V2, ei ); break;
        case 10 : SetZ( &pDst->m_V2, ei ); break;
        case 11 : SetW( &pDst->m_V2, ei ); break;
        default : break;
    }
}


/// Vec12F32 Write to Stream
template < typename T_Stream >
void Serialise( const Vec12F32& src, T_Stream& stream )
{
    StorageVec12F32 storage;
    Store( src, &storage );
    stream << storage.m_Data.v0;
    stream << storage.m_Data.v1;
    stream << storage.m_Data.v2;
    stream << storage.m_Data.v3;
    stream << storage.m_Data.v4;
    stream << storage.m_Data.v5;
    stream << storage.m_Data.v6;
    stream << storage.m_Data.v7;
    stream << storage.m_Data.v8;
    stream << storage.m_Data.v9;
    stream << storage.m_Data.v10;
    stream << storage.m_Data.v11;
}

/// Vec12F32 Read from Stream
template < typename T_Stream >
void Deserialise( T_Stream& stream, Vec12F32* const __restrict pDst )
{
    StorageVec12F32 storage;
    stream >> storage.m_Data.v0;
    stream >> storage.m_Data.v1;
    stream >> storage.m_Data.v2;
    stream >> storage.m_Data.v3;
    stream >> storage.m_Data.v4;
    stream >> storage.m_Data.v5;
    stream >> storage.m_Data.v6;
    stream >> storage.m_Data.v7;
    stream >> storage.m_Data.v8;
    stream >> storage.m_Data.v9;
    stream >> storage.m_Data.v10;
    stream >> storage.m_Data.v11;
    *pDst = Load<Vec4F32>( &storage );
}



/// Vec12F32 Permute : Indices select 12 result elements [I0,I11] from [ 0:v0, 11:v11 ]
//template < unsigned int I0, unsigned int I1, unsigned int I2,  unsigned int I3,
//           unsigned int I4, unsigned int I5, unsigned int I6,  unsigned int I7,
//           unsigned int I8, unsigned int I9, unsigned int I10, unsigned int I11 >
//NOALIAS __forceinline const Vec12F32 Permute( const Vec12F32& v )
//{
//    StaticAssert( I0  < 12, I0_MUST_BE_LESS_THAN_12 );
//    StaticAssert( I1  < 12, I1_MUST_BE_LESS_THAN_12 );
//    StaticAssert( I2  < 12, I2_MUST_BE_LESS_THAN_12 );
//    StaticAssert( I3  < 12, I3_MUST_BE_LESS_THAN_12 );
//    StaticAssert( I4  < 12, I4_MUST_BE_LESS_THAN_12 );
//    StaticAssert( I5  < 12, I5_MUST_BE_LESS_THAN_12 );
//    StaticAssert( I6  < 12, I6_MUST_BE_LESS_THAN_12 );
//    StaticAssert( I7  < 12, I7_MUST_BE_LESS_THAN_12 );
//    StaticAssert( I8  < 12, I7_MUST_BE_LESS_THAN_12 );
//    StaticAssert( I9  < 12, I7_MUST_BE_LESS_THAN_12 );
//    StaticAssert( I10 < 12, I7_MUST_BE_LESS_THAN_12 );
//    StaticAssert( I11 < 12, I7_MUST_BE_LESS_THAN_12 );
//    
//    Vec12F32 result;
//    result.m_V0 = Permute<I0,I1,I2,I3>( v.m_V0, v.m_V1 );
//    result.m_V1 = Permute<I4,I5,I6,I7>( v.m_V0, v.m_V1 );
//    result.m_V2 = ?;
//    return result;
//}



/// Vec12F32 Abs of each element
NOALIAS __forceinline const Vec12F32 Abs( const Vec12F32& v )
{
    Vec12F32 result;
    result.m_V0 = Abs( v.m_V0 );
    result.m_V1 = Abs( v.m_V1 );
    result.m_V2 = Abs( v.m_V2 );
    return result;
}

/// Vec12F32 Sign of each element : For each x in v: returns -1.0f if x < 0.0f, +1.0f if x > 0.0f, or 0.0f if x == 0.0f 
NOALIAS __forceinline const Vec12F32 Sign( const Vec12F32& v )
{
    Vec12F32 result;
    result.m_V0 = Sign( v.m_V0 );
    result.m_V1 = Sign( v.m_V1 );
    result.m_V2 = Sign( v.m_V2 );
    return result;
}

/// Vec12F32 CopySign of each element : Returns a Vec12F32 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vec12F32 CopySign( const Vec12F32& magnitude, const Vec12F32& sign )
{
    Vec12F32 result;
    result.m_V0 = CopySign( magnitude.m_V0, sign.m_V0 );
    result.m_V1 = CopySign( magnitude.m_V1, sign.m_V1 );
    result.m_V2 = CopySign( magnitude.m_V2, sign.m_V2 );
    return result;
}


/// Vec12F32 Negation : Indices select elements to negate [I0,I11]
template < bool I0, bool I1, bool I2,  bool I3,
           bool I4, bool I5, bool I6,  bool I7,
           bool I8, bool I9, bool I10, bool I11 >
NOALIAS __forceinline const Vec12F32 Negate( const Vec12F32& v )
{
    Vec12F32 result;
    result.m_V0 = Negate<I0,I1,I2, I3 >( v.m_V0 );
    result.m_V1 = Negate<I4,I5,I6, I7 >( v.m_V1 );
    result.m_V2 = Negate<I8,I9,I10,I11>( v.m_V2 );
    return result;
}


/// Vec12F32 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vec12F32 Rcp( const Vec12F32& v )
{
	Vec12F32 result;
	result.m_V0 = Rcp( v.m_V0 );
	result.m_V1 = Rcp( v.m_V1 );
	result.m_V2 = Rcp( v.m_V2 );
	return result;
}

/// Vec12F32 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vec12F32 Rcp24( const Vec12F32& v )
{
	Vec12F32 result;
	result.m_V0 = Rcp24( v.m_V0 );
	result.m_V1 = Rcp24( v.m_V1 );
	result.m_V2 = Rcp24( v.m_V2 );
	return result;
}

/// Vec12F32 Division : vector / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vec12F32 Div( const Vec12F32& x, const Vec12F32& y )
{
	Vec12F32 result;
	result.m_V0 = Div( x.m_V0, y.m_V0 );
	result.m_V1 = Div( x.m_V1, y.m_V1 );
	result.m_V2 = Div( x.m_V2, y.m_V2 );
	return result;
}

/// Vec12F32 Division : vector / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vec12F32 Div24( const Vec12F32& x, const Vec12F32& y )
{
	Vec12F32 result;
	result.m_V0 = Div24( x.m_V0, y.m_V0 );
	result.m_V1 = Div24( x.m_V1, y.m_V1 );
	result.m_V2 = Div24( x.m_V2, y.m_V2 );
	return result;
}

/// Vec12F32 Division : scalar / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vec12F32 Div( const VF32 VECREF x, const Vec12F32& y )
{
	Vec12F32 result;
	result.m_V0 = Div( x, y.m_V0 );
	result.m_V1 = Div( x, y.m_V1 );
	result.m_V2 = Div( x, y.m_V2 );
	return result;
}

/// Vec12F32 Division : scalar / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vec12F32 Div24( const VF32 VECREF x, const Vec12F32& y )
{
	Vec12F32 result;
	result.m_V0 = Div24( x, y.m_V0 );
	result.m_V1 = Div24( x, y.m_V1 );
	result.m_V2 = Div24( x, y.m_V2 );
	return result;
}

/// Vec12F32 Division : vector / scalar (faster but only 22 bit precision)
NOALIAS __forceinline const Vec12F32 Div( const Vec12F32& x, const VF32 VECREF y )
{
	Vec12F32 result;
	result.m_V0 = Div( x.m_V0, y );
	result.m_V1 = Div( x.m_V1, y );
	result.m_V2 = Div( x.m_V2, y );
	return result;
}

/// Vec12F32 Division : vector / scalar (slower but full 24 bit precision)
NOALIAS __forceinline const Vec12F32 Div24( const Vec12F32& x, const VF32 VECREF y )
{
	Vec12F32 result;
	result.m_V0 = Div24( x.m_V0, y );
	result.m_V1 = Div24( x.m_V1, y );
	result.m_V2 = Div24( x.m_V2, y );
	return result;
}

/// Vec12F32 Reciprocal Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vec12F32 RcpSqrt( const Vec12F32& v )
{
	Vec12F32 result;
	result.m_V0 = RcpSqrt( v.m_V0 );
	result.m_V1 = RcpSqrt( v.m_V1 );
	result.m_V2 = RcpSqrt( v.m_V2 );
	return result;
}

/// Vec12F32 Reciprocal Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vec12F32 RcpSqrt24( const Vec12F32& v )
{
	Vec12F32 result;
	result.m_V0 = RcpSqrt24( v.m_V0 );
	result.m_V1 = RcpSqrt24( v.m_V1 );
	result.m_V2 = RcpSqrt24( v.m_V2 );
	return result;
}

/// Vec12F32 Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vec12F32 Sqrt( const Vec12F32& v )
{
	Vec12F32 result;
	result.m_V0 = Sqrt( v.m_V0 );
	result.m_V1 = Sqrt( v.m_V1 );
	result.m_V2 = Sqrt( v.m_V2 );
	return result;
}

/// Vec12F32 Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vec12F32 Sqrt24( const Vec12F32& v )
{
	Vec12F32 result;
	result.m_V0 = Sqrt24( v.m_V0 );
	result.m_V1 = Sqrt24( v.m_V1 );
	result.m_V2 = Sqrt24( v.m_V2 );
	return result;
}

/// Vec12F32 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void RcpAndSqrt( const Vec12F32& v,
                                       Vec12F32* const __restrict pRcp,
                                       Vec12F32* const __restrict pSqrt,
                                       Vec12F32* const __restrict pRcpSqrt )
{
    RcpAndSqrt( v.m_V0, &pRcp->m_V0, &pSqrt->m_V0, &pRcpSqrt->m_V0 );
    RcpAndSqrt( v.m_V1, &pRcp->m_V1, &pSqrt->m_V1, &pRcpSqrt->m_V1 );
    RcpAndSqrt( v.m_V2, &pRcp->m_V2, &pSqrt->m_V2, &pRcpSqrt->m_V2 );
}

/// Vec12F32 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void RcpAndSqrt24( const Vec12F32& v,
                                         Vec12F32* const __restrict pRcp,
                                         Vec12F32* const __restrict pSqrt,
                                         Vec12F32* const __restrict pRcpSqrt )
{
    RcpAndSqrt24( v.m_V0, &pRcp->m_V0, &pSqrt->m_V0, &pRcpSqrt->m_V0 );
    RcpAndSqrt24( v.m_V1, &pRcp->m_V1, &pSqrt->m_V1, &pRcpSqrt->m_V1 );
    RcpAndSqrt24( v.m_V2, &pRcp->m_V2, &pSqrt->m_V2, &pRcpSqrt->m_V2 );
}

/// Vec12F32 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt( const Vec12F32& v,
                                           Vec12F32* const __restrict pSqrt,
                                           Vec12F32* const __restrict pRcpSqrt )
{
    SqrtAndRcpSqrt( v.m_V0, &pSqrt->m_V0, &pRcpSqrt->m_V0 );
    SqrtAndRcpSqrt( v.m_V1, &pSqrt->m_V1, &pRcpSqrt->m_V1 );
    SqrtAndRcpSqrt( v.m_V2, &pSqrt->m_V2, &pRcpSqrt->m_V2 );
}

/// Vec12F32 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt24( const Vec12F32& v,
                                             Vec12F32* const __restrict pSqrt,
                                             Vec12F32* const __restrict pRcpSqrt )
{
    SqrtAndRcpSqrt24( v.m_V0, &pSqrt->m_V0, &pRcpSqrt->m_V0 );
    SqrtAndRcpSqrt24( v.m_V1, &pSqrt->m_V1, &pRcpSqrt->m_V1 );
    SqrtAndRcpSqrt24( v.m_V2, &pSqrt->m_V2, &pRcpSqrt->m_V2 );
}



/// Vec12F32 Unary Positive Operator
NOALIAS __forceinline const Vec12F32 operator + ( const Vec12F32& x )
{
    return x;
}

/// Vec12F32 Unary Negation Operator
NOALIAS __forceinline const Vec12F32 operator - ( const Vec12F32& x )
{
    Vec12F32 result;
    result.m_V0 = -x.m_V0;
    result.m_V1 = -x.m_V1;
    result.m_V2 = -x.m_V2;
    return result;
}


/// Vec12F32 Addition Operator
NOALIAS __forceinline const Vec12F32 operator + ( const Vec12F32& a, const Vec12F32& b )
{
    Vec12F32 result;
    result.m_V0 = a.m_V0 + b.m_V0;
    result.m_V1 = a.m_V1 + b.m_V1;
    result.m_V2 = a.m_V2 + b.m_V2;
    return result;
}

/// Vec12F32 Subtraction Operator
NOALIAS __forceinline const Vec12F32 operator - ( const Vec12F32& a, const Vec12F32& b )
{
    Vec12F32 result;
    result.m_V0 = a.m_V0 - b.m_V0;
    result.m_V1 = a.m_V1 - b.m_V1;
    result.m_V2 = a.m_V2 - b.m_V2;
    return result;
}

/// Vec12F32 Multiplication Operator
NOALIAS __forceinline const Vec12F32 operator * ( const Vec12F32& a, const Vec12F32& b )
{
    Vec12F32 result;
    result.m_V0 = a.m_V0 * b.m_V0;
    result.m_V1 = a.m_V1 * b.m_V1;
    result.m_V2 = a.m_V2 * b.m_V2;
    return result;
}

/// Vec12F32 Division Operator
NOALIAS __forceinline const Vec12F32 operator / ( const Vec12F32& a, const Vec12F32& b )
{
    Vec12F32 result;
    result.m_V0 = a.m_V0 / b.m_V0;
    result.m_V1 = a.m_V1 / b.m_V1;
    result.m_V2 = a.m_V2 / b.m_V2;
    return result;
}


/// Vec12F32 Addition Update Operator
NOALIAS __forceinline void operator += ( Vec12F32& a, const Vec12F32& b )
{
    a.m_V0 += b.m_V0;
    a.m_V1 += b.m_V1;
    a.m_V2 += b.m_V2;
}

/// Vec12F32 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Vec12F32& a, const Vec12F32& b )
{
    a.m_V0 -= b.m_V0;
    a.m_V1 -= b.m_V1;
    a.m_V2 -= b.m_V2;
}

/// Vec12F32 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vec12F32& a, const Vec12F32& b )
{
    a.m_V0 *= b.m_V0;
    a.m_V1 *= b.m_V1;
    a.m_V2 *= b.m_V2;
}

/// Vec12F32 Division Update Operator
NOALIAS __forceinline void operator /= ( Vec12F32& a, const Vec12F32& b )
{
    a.m_V0 /= b.m_V0;
    a.m_V1 /= b.m_V1;
    a.m_V2 /= b.m_V2;
}


/// Vec12F32 Scalar Multiplication: Vec12F32 * VF32
NOALIAS __forceinline const Vec12F32 operator * ( const Vec12F32& v, const VF32 VECREF s )
{
    Vec12F32 result;
    result.m_V0 = v.m_V0 * s;
    result.m_V1 = v.m_V1 * s;
    result.m_V2 = v.m_V2 * s;
    return result;
}

/// Vec12F32 Scalar Multiplication: VF32 * Vec12F32
NOALIAS __forceinline const Vec12F32 operator * ( const VF32 VECREF s, const Vec12F32& v )
{
    Vec12F32 result;
    result.m_V0 = s * v.m_V0;
    result.m_V1 = s * v.m_V1;
    result.m_V2 = s * v.m_V2;
    return result;
}

/// Vec12F32 Scalar Division: Vec12F32 / VF32
NOALIAS __forceinline const Vec12F32 operator / ( const Vec12F32& v, const VF32 VECREF s )
{
    Vec12F32 result;
    const VF32 rcp_s( Rcp( s ) );
    result.m_V0 = v.m_V0 * rcp_s;
    result.m_V1 = v.m_V1 * rcp_s;
    result.m_V2 = v.m_V2 * rcp_s;
    return result;
}

/// Vec12F32 Scalar Division: VF32 / Vec12F32
NOALIAS __forceinline const Vec12F32 operator / ( const VF32 VECREF s, const Vec12F32& v )
{
    Vec12F32 result;
    result.m_V0 = s / v.m_V0;
    result.m_V1 = s / v.m_V1;
    result.m_V2 = s / v.m_V2;
    return result;
}


/// Vec12F32 Scalar Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vec12F32& v, const VF32 VECREF s )
{
    v.m_V0 *= s;
    v.m_V1 *= s;
    v.m_V2 *= s;
}

/// Vec12F32 Scalar Division Update Operator
NOALIAS __forceinline void operator /= ( Vec12F32& v, const VF32 VECREF s )
{
    const VF32 rcp_s( Rcp( s ) );
    v.m_V0 *= rcp_s;
    v.m_V1 *= rcp_s;
    v.m_V2 *= rcp_s;
}


/// Vec12F32 MulAdd : a * b + c
NOALIAS __forceinline const Vec12F32 MulAdd( const Vec12F32& a, const Vec12F32& b, const Vec12F32& c )
{
    Vec12F32 result;
    result.m_V0 = MulAdd( a.m_V0, b.m_V0, c.m_V0 );
    result.m_V1 = MulAdd( a.m_V1, b.m_V1, c.m_V1 );
    result.m_V2 = MulAdd( a.m_V2, b.m_V2, c.m_V2 );
    return result;
}

/// Vec12F32 MulSub : a * b - c
NOALIAS __forceinline const Vec12F32 MulSub( const Vec12F32& a, const Vec12F32& b, const Vec12F32& c )
{
    Vec12F32 result;
    result.m_V0 = MulSub( a.m_V0, b.m_V0, c.m_V0 );
    result.m_V1 = MulSub( a.m_V1, b.m_V1, c.m_V1 );
    result.m_V2 = MulSub( a.m_V2, b.m_V2, c.m_V2 );
    return result;
}

/// Vec12F32 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vec12F32 NegMulAdd( const Vec12F32& a, const Vec12F32& b, const Vec12F32& c )
{
    Vec12F32 result;
    result.m_V0 = NegMulAdd( a.m_V0, b.m_V0, c.m_V0 );
    result.m_V1 = NegMulAdd( a.m_V1, b.m_V1, c.m_V1 );
    result.m_V2 = NegMulAdd( a.m_V2, b.m_V2, c.m_V2 );
    return result;
}

/// Vec12F32 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vec12F32 NegMulSub( const Vec12F32& a, const Vec12F32& b, const Vec12F32& c )
{
    Vec12F32 result;
    result.m_V0 = NegMulSub( a.m_V0, b.m_V0, c.m_V0 );
    result.m_V1 = NegMulSub( a.m_V1, b.m_V1, c.m_V1 );
    result.m_V2 = NegMulSub( a.m_V2, b.m_V2, c.m_V2 );
    return result;
}


/// Vec12F32 MulAdd : a * b + c : [ vector * scalar + vector ]
NOALIAS __forceinline const Vec12F32 MulAdd( const Vec12F32& a, const VF32 VECREF b, const Vec12F32& c )
{
    Vec12F32 result;
    result.m_V0 = MulAdd( a.m_V0, b, c.m_V0 );
    result.m_V1 = MulAdd( a.m_V1, b, c.m_V1 );
    result.m_V2 = MulAdd( a.m_V2, b, c.m_V2 );
    return result;
}

/// Vec12F32 MulSub : a * b - c : [ vector * scalar - vector ]
NOALIAS __forceinline const Vec12F32 MulSub( const Vec12F32& a, const VF32 VECREF b, const Vec12F32& c )
{
    Vec12F32 result;
    result.m_V0 = MulAdd( a.m_V0, b, c.m_V0 );
    result.m_V1 = MulAdd( a.m_V1, b, c.m_V1 );
    result.m_V2 = MulAdd( a.m_V2, b, c.m_V2 );
    return result;
}

/// Vec12F32 NegMulAdd : -( a * b + c ) : [ - ( vector * scalar + vector ) ]
NOALIAS __forceinline const Vec12F32 NegMulAdd( const Vec12F32& a, const VF32 VECREF b, const Vec12F32& c )
{
    Vec12F32 result;
    result.m_V0 = MulAdd( a.m_V0, b, c.m_V0 );
    result.m_V1 = MulAdd( a.m_V1, b, c.m_V1 );
    result.m_V2 = MulAdd( a.m_V2, b, c.m_V2 );
    return result;
}

/// Vec12F32 NegMulSub : -( a * b - c ) == c - a * b : [ - ( vector * scalar - vector ) ]
NOALIAS __forceinline const Vec12F32 NegMulSub( const Vec12F32& a, const VF32 VECREF b, const Vec12F32& c )
{
    Vec12F32 result;
    result.m_V0 = MulAdd( a.m_V0, b, c.m_V0 );
    result.m_V1 = MulAdd( a.m_V1, b, c.m_V1 );
    result.m_V2 = MulAdd( a.m_V2, b, c.m_V2 );
    return result;
}


/// Vec12F32 MulAdd : a * b + c : [ scalar * vector + vector ]
NOALIAS __forceinline const Vec12F32 MulAdd( const VF32 VECREF a, const Vec12F32& b, const Vec12F32& c )
{
    Vec12F32 result;
    result.m_V0 = MulAdd( a, b.m_V0, c.m_V0 );
    result.m_V1 = MulAdd( a, b.m_V1, c.m_V1 );
    result.m_V2 = MulAdd( a, b.m_V2, c.m_V2 );
    return result;
}

/// Vec12F32 MulSub : a * b - c : [ scalar * vector - vector ]
NOALIAS __forceinline const Vec12F32 MulSub( const VF32 VECREF a, const Vec12F32& b, const Vec12F32& c )
{
    Vec12F32 result;
    result.m_V0 = MulSub( a, b.m_V0, c.m_V0 );
    result.m_V1 = MulSub( a, b.m_V1, c.m_V1 );
    result.m_V2 = MulSub( a, b.m_V2, c.m_V2 );
    return result;
}

/// Vec12F32 NegMulAdd : -( a * b + c ) : [ - ( scalar * vector + vector ) ]
NOALIAS __forceinline const Vec12F32 NegMulAdd( const VF32 VECREF a, const Vec12F32& b, const Vec12F32& c )
{
    Vec12F32 result;
    result.m_V0 = NegMulAdd( a, b.m_V0, c.m_V0 );
    result.m_V1 = NegMulAdd( a, b.m_V1, c.m_V1 );
    result.m_V2 = NegMulAdd( a, b.m_V2, c.m_V2 );
    return result;
}

/// Vec12F32 NegMulSub : -( a * b - c ) == c - a * b : [ - ( scalar * vector - vector ) ]
NOALIAS __forceinline const Vec12F32 NegMulSub( const VF32 VECREF a, const Vec12F32& b, const Vec12F32& c )
{
    Vec12F32 result;
    result.m_V0 = NegMulSub( a, b.m_V0, c.m_V0 );
    result.m_V1 = NegMulSub( a, b.m_V1, c.m_V1 );
    result.m_V2 = NegMulSub( a, b.m_V2, c.m_V2 );
    return result;
}


/// Vec12F32 Lerp : src + ( ( dst - src ) * t )
NOALIAS __forceinline const Vec12F32 Lerp( const Vec12F32& src, const Vec12F32& dst, const VF32 VECREF t )
{
    Vec12F32 result;
    result.m_V0 = Lerp( src.m_V0, dst.m_V0, t );
    result.m_V1 = Lerp( src.m_V1, dst.m_V1, t );
    result.m_V2 = Lerp( src.m_V2, dst.m_V2, t );
    return result;
}


/// Vec12F32 Min
NOALIAS __forceinline const Vec12F32 Min( const Vec12F32& x, const Vec12F32& y )
{
    Vec12F32 result;
    result.m_V0 = Min( x.m_V0, y.m_V0 );
    result.m_V1 = Min( x.m_V1, y.m_V1 );
    result.m_V2 = Min( x.m_V2, y.m_V2 );
    return result;
}

/// Vec12F32 Max
NOALIAS __forceinline const Vec12F32 Max( const Vec12F32& x, const Vec12F32& y )
{
    Vec12F32 result;
    result.m_V0 = Max( x.m_V0, y.m_V0 );
    result.m_V1 = Max( x.m_V1, y.m_V1 );
    result.m_V2 = Max( x.m_V2, y.m_V2 );
    return result;
}

/// Vec12F32 Min
NOALIAS __forceinline const Vec12F32 Min( const Vec12F32& x, const VF32 VECREF y )
{
    Vec12F32 result;
    result.m_V0 = Min( x.m_V0, y );
    result.m_V1 = Min( x.m_V1, y );
    result.m_V2 = Min( x.m_V2, y );
    return result;
}

/// Vec12F32 Max
NOALIAS __forceinline const Vec12F32 Max( const Vec12F32& x, const VF32 VECREF y )
{
    Vec12F32 result;
    result.m_V0 = Max( x.m_V0, y );
    result.m_V1 = Max( x.m_V1, y );
    result.m_V2 = Max( x.m_V2, y );
    return result;
}

/// Vec12F32 Min Update
DEPRECATED NOALIAS __forceinline const Vec12F32& MinEq( Vec12F32* const __restrict pX, const Vec12F32& y )
{
    Min( &pX->m_V0, y.m_V0 );
    Min( &pX->m_V1, y.m_V1 );
    Min( &pX->m_V2, y.m_V2 );
    return *pX;
}

/// Vec12F32 Max Update
DEPRECATED NOALIAS __forceinline const Vec12F32& MaxEq( Vec12F32* const __restrict pX, const Vec12F32& y )
{
    Max( &pX->m_V0, y.m_V0 );
    Max( &pX->m_V1, y.m_V1 );
    Max( &pX->m_V2, y.m_V2 );
    return *pX;
}

/// Vec12F32 Min
NOALIAS __forceinline void Min( Vec12F32* const __restrict pX, const Vec12F32& y )
{
    Min( &pX->m_V0, y.m_V0 );
    Min( &pX->m_V1, y.m_V1 );
    Min( &pX->m_V2, y.m_V2 );
}

/// Vec12F32 Max
NOALIAS __forceinline void Max( Vec12F32* const __restrict pX, const Vec12F32& y )
{
    Max( &pX->m_V0, y.m_V0 );
    Max( &pX->m_V1, y.m_V1 );
    Max( &pX->m_V2, y.m_V2 );
}


/// Vec12F32 Clamp
NOALIAS __forceinline const Vec12F32 Clamp( const Vec12F32& x, const Vec12F32& lo, const Vec12F32& hi )
{
    Vec12F32 result;
    result.m_V0 = Clamp( x.m_V0, lo.m_V0, hi.m_V0 );
    result.m_V1 = Clamp( x.m_V1, lo.m_V1, hi.m_V1 );
    result.m_V2 = Clamp( x.m_V2, lo.m_V2, hi.m_V2 );
    return result;
}

/// Vec12F32 Clamp Update
DEPRECATED NOALIAS __forceinline const Vec12F32& ClampEq( Vec12F32* const __restrict pX, const Vec12F32& lo, const Vec12F32& hi )
{
    Clamp( &pX->m_V0, lo.m_V0, hi.m_V0 );
    Clamp( &pX->m_V1, lo.m_V1, hi.m_V1 );
    Clamp( &pX->m_V2, lo.m_V2, hi.m_V2 );
    return *pX;
}

/// Vec12F32 Clamp
NOALIAS __forceinline void Clamp( Vec12F32* const __restrict pX, const Vec12F32& lo, const Vec12F32& hi )
{
    Clamp( &pX->m_V0, lo.m_V0, hi.m_V0 );
    Clamp( &pX->m_V1, lo.m_V1, hi.m_V1 );
    Clamp( &pX->m_V2, lo.m_V2, hi.m_V2 );
}


/// Vec12F32 Round to integer towards minus infinity
NOALIAS __forceinline const Vec12F32 Floor( const Vec12F32& x )
{
    Vec12F32 result;
    result.m_V0 = Floor( x.m_V0 );
    result.m_V1 = Floor( x.m_V1 );
    result.m_V2 = Floor( x.m_V2 );
    return result;
}

/// Vec12F32 Round to integer towards plus infinity
NOALIAS __forceinline const Vec12F32 Ceil ( const Vec12F32& x )
{
    Vec12F32 result;
    result.m_V0 = Ceil( x.m_V0 );
    result.m_V1 = Ceil( x.m_V1 );
    result.m_V2 = Ceil( x.m_V2 );
    return result;
}

/// Vec12F32 Round to integer towards nearest
NOALIAS __forceinline const Vec12F32 Round( const Vec12F32& x )
{
    Vec12F32 result;
    result.m_V0 = Round( x.m_V0 );
    result.m_V1 = Round( x.m_V1 );
    result.m_V2 = Round( x.m_V2 );
    return result;
}

/// Vec12F32 Round to integer towards zero
NOALIAS __forceinline const Vec12F32 Trunc( const Vec12F32& x )
{
    Vec12F32 result;
    result.m_V0 = Trunc( x.m_V0 );
    result.m_V1 = Trunc( x.m_V1 );
    result.m_V2 = Trunc( x.m_V2 );
    return result;
}



/// Vec12F32 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w
NOALIAS __forceinline const VBool operator == ( const Vec12F32& a, const Vec12F32& b )
{
    return ( a.m_V0 == b.m_V0 ) &
           ( a.m_V1 == b.m_V1 ) &
           ( a.m_V2 == b.m_V2 );
}

/// Vec12F32 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w
NOALIAS __forceinline const VBool operator != ( const Vec12F32& a, const Vec12F32& b )
{
    return ( a.m_V0 != b.m_V0 ) |
           ( a.m_V1 != b.m_V1 ) |
           ( a.m_V2 != b.m_V2 );
}

/// Vec12F32 Comparison a <= b : Returns Vec16Bool representing results of a.i <= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec16Bool operator <= ( const Vec12F32& a, const Vec12F32& b )
{
    return MakeVec16Bool( a.m_V0 <= b.m_V0,
                          a.m_V1 <= b.m_V1,
                          a.m_V2 <= b.m_V2,
                          Const<Vec4Bool>::False() );
}

/// Vec12F32 Comparison a >= b : Returns Vec16Bool representing results of a.i >= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec16Bool operator >= ( const Vec12F32& a, const Vec12F32& b )
{
    return MakeVec16Bool( a.m_V0 >= b.m_V0,
                          a.m_V1 >= b.m_V1,
                          a.m_V2 >= b.m_V2,
                          Const<Vec4Bool>::False() );
}

/// Vec12F32 Comparison a < b : Returns Vec16Bool representing results of a.i < b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec16Bool operator < ( const Vec12F32& a, const Vec12F32& b )
{
    return MakeVec16Bool( a.m_V0 < b.m_V0,
                          a.m_V1 < b.m_V1,
                          a.m_V2 < b.m_V2,
                          Const<Vec4Bool>::False() );
}

/// Vec12F32 Comparison a > b : Returns Vec16Bool representing results of a.i > b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec16Bool operator > ( const Vec12F32& a, const Vec12F32& b )
{
    return MakeVec16Bool( a.m_V0 > b.m_V0,
                          a.m_V1 > b.m_V1,
                          a.m_V2 > b.m_V2,
                          Const<Vec4Bool>::False() );
}

/// Vec12F32 Ordered Test : true if neither a nor b contains any elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vec12F32& a, const Vec12F32& b )
{
    return Ordered( a.m_V0, b.m_V0 ) &
           Ordered( a.m_V1, b.m_V1 ) &
           Ordered( a.m_V2, b.m_V2 );
}

/// Vec12F32 Unordered Test : true if a or b contains any elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vec12F32& a, const Vec12F32& b )
{
    return Unordered( a.m_V0, b.m_V0 ) |
           Unordered( a.m_V1, b.m_V1 ) |
           Unordered( a.m_V2, b.m_V2 );
}


/// Vec12F32 NaN Test : true if a contains any element that is a NaN
NOALIAS __forceinline const VBool IsNan( const Vec12F32& a )
{
    return IsNan( a.m_V0 ) |
           IsNan( a.m_V1 ) |
           IsNan( a.m_V2 );
}

/// Vec12F32 Finite Test : true if a contains no elements that are NaN's and for each element i [x,y,z,w], abs(x.i) <= FLT_MAX
NOALIAS __forceinline const VBool Finite( const Vec12F32& a )
{
    return Finite( a.m_V0 ) &
           Finite( a.m_V1 ) &
           Finite( a.m_V2 );
}


/// Vec12F32 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vec12F32 Select( const VBool PCREF condition, const Vec12F32& result_if_true, const Vec12F32& result_if_false )
{
    Vec12F32 result;
    result.m_V0 = Select( condition, result_if_true.m_V0, result_if_false.m_V0 );
    result.m_V1 = Select( condition, result_if_true.m_V1, result_if_false.m_V1 );
    result.m_V2 = Select( condition, result_if_true.m_V2, result_if_false.m_V2 );
    return result;
}

/// Vec12F32 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vec12F32 Select( const Vec16Bool PCREF condition, const Vec12F32& result_if_true, const Vec12F32& result_if_false )
{
    Vec12F32 result;
    Vec4Bool condition0, condition1, condition2, condition3;
    Split( condition, &condition0, &condition1, &condition2, &condition3 );
    result.m_V0 = Select( condition0, result_if_true.m_V0, result_if_false.m_V0 );
    result.m_V1 = Select( condition1, result_if_true.m_V1, result_if_false.m_V1 );
    result.m_V2 = Select( condition2, result_if_true.m_V2, result_if_false.m_V2 );
    return result;
}



/// Vec12F32 Bitwise NOT: ~a
NOALIAS __forceinline const Vec12F32 NOT( const Vec12F32& a )
{
    Vec12F32 result;
    result.m_V0 = NOT( a.m_V0 );
    result.m_V1 = NOT( a.m_V1 );
    result.m_V2 = NOT( a.m_V2 );
    return result;
}

/// Vec12F32 Bitwise OR: a | b
NOALIAS __forceinline const Vec12F32 OR( const Vec12F32& a, const Vec12F32& b )
{
    Vec12F32 result;
    result.m_V0 = OR( a.m_V0, b.m_V0 );
    result.m_V1 = OR( a.m_V1, b.m_V1 );
    result.m_V2 = OR( a.m_V2, b.m_V2 );
    return result;
}

/// Vec12F32 Bitwise AND: a & b
NOALIAS __forceinline const Vec12F32 AND( const Vec12F32& a, const Vec12F32& b )
{
    Vec12F32 result;
    result.m_V0 = AND( a.m_V0, b.m_V0 );
    result.m_V1 = AND( a.m_V1, b.m_V1 );
    result.m_V2 = AND( a.m_V2, b.m_V2 );
    return result;
}

/// Vec12F32 Bitwise XOR: a ^ b : a != b : (a & ~b) | (~a & b)
NOALIAS __forceinline const Vec12F32 XOR( const Vec12F32& a, const Vec12F32& b )
{
    Vec12F32 result;
    result.m_V0 = XOR( a.m_V0, b.m_V0 );
    result.m_V1 = XOR( a.m_V1, b.m_V1 );
    result.m_V2 = XOR( a.m_V2, b.m_V2 );
    return result;
}

/// Vec12F32 Bitwise NOR: ~(a | b)
NOALIAS __forceinline const Vec12F32 NOR( const Vec12F32& a, const Vec12F32& b )
{
    Vec12F32 result;
    result.m_V0 = NOR( a.m_V0, b.m_V0 );
    result.m_V1 = NOR( a.m_V1, b.m_V1 );
    result.m_V2 = NOR( a.m_V2, b.m_V2 );
    return result;
}

/// Vec12F32 Bitwise NAND: ~(a & b)
NOALIAS __forceinline const Vec12F32 NAND( const Vec12F32& a, const Vec12F32& b )
{
    Vec12F32 result;
    result.m_V0 = NAND( a.m_V0, b.m_V0 );
    result.m_V1 = NAND( a.m_V1, b.m_V1 );
    result.m_V2 = NAND( a.m_V2, b.m_V2 );
    return result;
}

/// Vec12F32 Bitwise EQV: ~(a ^ b) : a == b : (a & b) | (~a & ~b)
NOALIAS __forceinline const Vec12F32 EQV( const Vec12F32& a, const Vec12F32& b )
{
    Vec12F32 result;
    result.m_V0 = EQV( a.m_V0, b.m_V0 );
    result.m_V1 = EQV( a.m_V1, b.m_V1 );
    result.m_V2 = EQV( a.m_V2, b.m_V2 );
    return result;
}

/// Vec12F32 Bitwise ORC: a | ~b
NOALIAS __forceinline const Vec12F32 ORC( const Vec12F32& a, const Vec12F32& b )
{
    Vec12F32 result;
    result.m_V0 = ORC( a.m_V0, b.m_V0 );
    result.m_V1 = ORC( a.m_V1, b.m_V1 );
    result.m_V2 = ORC( a.m_V2, b.m_V2 );
    return result;
}

/// Vec12F32 Bitwise ANDC: a & ~b
NOALIAS __forceinline const Vec12F32 ANDC( const Vec12F32& a, const Vec12F32& b )
{
    Vec12F32 result;
    result.m_V0 = ANDC( a.m_V0, b.m_V0 );
    result.m_V1 = ANDC( a.m_V1, b.m_V1 );
    result.m_V2 = ANDC( a.m_V2, b.m_V2 );
    return result;
}


/// Vec12F32 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vec12F32 AND( const Vec12F32& a, const VBool PCREF b )
{
    Vec12F32 result;
    result.m_V0 = AND( a.m_V0, b );
    result.m_V1 = AND( a.m_V1, b );
    result.m_V2 = AND( a.m_V2, b );
    return result;
}

/// Vec12F32 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vec12F32 ANDC( const Vec12F32& a, const VBool PCREF b )
{
    Vec12F32 result;
    result.m_V0 = ANDC( a.m_V0, b );
    result.m_V1 = ANDC( a.m_V1, b );
    result.m_V2 = ANDC( a.m_V2, b );
    return result;
}


/// Vec12F32 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vec12F32 AND( const Vec12F32& a, const Vec16Bool PCREF b )
{
    Vec12F32 result;
    Vec4Bool b0, b1, b2, b3;
    Split( b, &b0, &b1, &b2, &b3 );
    result.m_V0 = AND( a.m_V0, b0 );
    result.m_V1 = AND( a.m_V1, b1 );
    result.m_V2 = AND( a.m_V2, b1 );
    return result;
}

/// Vec12F32 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vec12F32 ANDC( const Vec12F32& a, const Vec16Bool PCREF b )
{
    Vec12F32 result;
    Vec4Bool b0, b1, b2, b3;
    Split( b, &b0, &b1, &b2, &b3 );
    result.m_V0 = ANDC( a.m_V0, b0 );
    result.m_V1 = ANDC( a.m_V1, b1 );
    result.m_V2 = ANDC( a.m_V2, b1 );
    return result;
}



/// Vec12F32 Dot Product 9d
NOALIAS __forceinline const VF32 Dot9( const Vec12F32& a, const Vec12F32& b )
{
    //return Dot4( a.m_V0, b.m_V0 ) +
    //       Dot4( a.m_V1, b.m_V1 ) +
    //       X( a.m_V2 ) * X( b.m_V2 );
    return MulAdd( X( a.m_V2 ), X( b.m_V2 ), Dot4( a.m_V1, b.m_V1 ) ) + Dot4( a.m_V0, b.m_V0 );
}

/// Vec12F32 Dot Product 10d
NOALIAS __forceinline const VF32 Dot10( const Vec12F32& a, const Vec12F32& b )
{
    return Dot4( a.m_V0, b.m_V0 ) +
           Dot4( a.m_V1, b.m_V1 ) +
           Dot2( a.m_V2, b.m_V2 );
}

/// Vec12F32 Dot Product 11d
NOALIAS __forceinline const VF32 Dot11( const Vec12F32& a, const Vec12F32& b )
{
    return Dot4( a.m_V0, b.m_V0 ) +
           Dot4( a.m_V1, b.m_V1 ) +
           Dot3( a.m_V2, b.m_V2 );
}

/// Vec12F32 Dot Product 12d
NOALIAS __forceinline const VF32 Dot12( const Vec12F32& a, const Vec12F32& b )
{
    return Dot4( a.m_V0, b.m_V0 ) +
           Dot4( a.m_V1, b.m_V1 ) +
           Dot4( a.m_V2, b.m_V2 );
}


/// Vec12F32 Flush To Zero (9d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec12F32 FlushToZero9( const Vec12F32& v, const VF32 VECREF tolerance )
{
    return AND( v, Dot9( v, v ) > ( tolerance * tolerance ) );
}

/// Vec12F32 Flush To Zero (10d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec12F32 FlushToZero10( const Vec12F32& v, const VF32 VECREF tolerance )
{
    return AND( v, Dot10( v, v ) > ( tolerance * tolerance ) );
}

/// Vec12F32 Flush To Zero (11d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec12F32 FlushToZero11( const Vec12F32& v, const VF32 VECREF tolerance )
{
    return AND( v, Dot11( v, v ) > ( tolerance * tolerance ) );
}

/// Vec12F32 Flush To Zero (12d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec12F32 FlushToZero12( const Vec12F32& v, const VF32 VECREF tolerance )
{
    return AND( v, Dot12( v, v ) > ( tolerance * tolerance ) );
}


/// Vec12F32 Flush Elements To Zero: flushes elements to zero if less than or equal to tolerance
NOALIAS __forceinline const Vec12F32 FlushElementsToZero( const Vec12F32& value, const VF32 VECREF tolerance )
{
    Vec12F32 result;
    result.m_V0 = FlushElementsToZero( value.m_V0, tolerance );
    result.m_V1 = FlushElementsToZero( value.m_V1, tolerance );
    result.m_V2 = FlushElementsToZero( value.m_V2, tolerance );
    return result;
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )




