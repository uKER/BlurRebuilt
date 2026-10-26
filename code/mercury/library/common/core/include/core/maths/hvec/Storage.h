#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_STORAGE_H
#define SOFT_MATHS_HVEC_STORAGE_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_STORAGE_H
#define MATHS_HVEC_STORAGE_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{


//template < typename T > class T_Vector3
//{
//public:
//    
//    typedef T Type;
//    
//    F32     m_X, m_Y, m_Z;
//}
//
//template < typename T > class T_Vector4
//{
//public:
//    
//    typedef T Type;
//    
//    F32     m_X, m_Y, m_Z, m_W;
//}
//
//template < typename T > class T_Matrix33
//{
//public:
//    
//    typedef T Type;
//    
//    #if defined( _VMX128_ )
//    F32     m_CX_VX, m_CX_VY, m_CX_VZ;
//    F32     m_CY_VX, m_CY_VY, m_CY_VZ;
//    F32     m_CZ_VX, m_CZ_VY, m_CZ_VZ;
//    #else
//    F32     m_RX_VX, m_RX_VY, m_RX_VZ;
//    F32     m_RY_VX, m_RY_VY, m_RY_VZ;
//    F32     m_RZ_VX, m_RZ_VY, m_RZ_VZ;
//    #endif
//}

//#ifndef __LINT__

/// Default template for loading from a storage type
template < typename T_Data, typename T_Storage >
NOALIAS __forceinline const T_Data Load( T_Storage const * const __restrict pStorage )
{
    // If this assert is triggered then the template specialisation for
    // loading a T_Vector from T_Storage needs to be implemented!
    #ifndef __GNUC__
    BIZ_STATIC_ASSERT( false );
    #endif
}

/// Default template for storing into a storage type
template < typename T_Data, typename T_Storage >
NOALIAS __forceinline void Store( const T_Data& data, T_Storage * const __restrict pStorage )
{
    // If this assert is triggered then the template specialisation for
    // storing a T_Vector into T_Storage needs to be implemented!
    #ifndef __GNUC__
    BIZ_STATIC_ASSERT( false );
    #endif
}

//#endif


template <> NOALIAS __forceinline void Store( const F32& src, F32* const __restrict pDst ) {*pDst = src;}
template <> NOALIAS __forceinline void Store( const F64& src, F32* const __restrict pDst ) {*pDst = (float)src;}

class HF32;
class HVec2;
class HVec3;
class HVec4;
class C_RGBA_F32;
class C_Quaternion;
class HMat33;
class HMat43;
class HMat44;
class C_Vertex;
class C_Vector;
class C_Normal;


/// Load HF32 from an F32
template <>
NOALIAS __forceinline const Maths::HF32 Load( F32 const * const __restrict pSrc );

/// Store HF32 into an F32
template <>
NOALIAS __forceinline void Store( const Maths::HF32& src, F32* const __restrict pDst );


/// Load HVec2 from a Float2
template <>
NOALIAS __forceinline const Maths::HVec2 Load( MathsStorage::Float2 const * const __restrict pSrc );

/// Store HVec2 into a Float2
template <>
NOALIAS __forceinline void Store( const Maths::HVec2& src, MathsStorage::Float2* const __restrict pDst );

/// Load HVec3 from a Float3
template <>
NOALIAS __forceinline const Maths::HVec3 Load( MathsStorage::Float3 const * const __restrict pSrc );

/// Store HVec3 into a Float3
template <>
NOALIAS __forceinline void Store( const Maths::HVec3& src, MathsStorage::Float3* const __restrict pDst );

/// Load HVec3 from a Float3A16
template <>
NOALIAS __forceinline const Maths::HVec3 Load( MathsStorage::Float3A16 const * const __restrict pSrc );

/// Store HVec3 into a Float3A16
template <>
NOALIAS __forceinline void Store( const Maths::HVec3& src, MathsStorage::Float3A16* const __restrict pDst );

/// Load HVec3 from a Float4A16
template <>
NOALIAS __forceinline const Maths::HVec3 Load( MathsStorage::Float4A16 const * const __restrict pSrc );

/// Store HVec3 into a Float4A16
template <>
NOALIAS __forceinline void Store( const Maths::HVec3& src, MathsStorage::Float4A16* const __restrict pDst );

/// Load HVec3 from a HenDN3
template <>
NOALIAS __forceinline const Maths::HVec3 Load( MathsStorage::HenDN3 const * const __restrict pSrc );

/// Store HVec3 into a HenDN3
template <>
NOALIAS __forceinline void Store( const Maths::HVec3& src, MathsStorage::HenDN3* const __restrict pDst );

/// Load HVec4 from a Float4
template <>
NOALIAS __forceinline const Maths::HVec4 Load( MathsStorage::Float4 const * const __restrict pSrc );

/// Store HVec4 into a Float4
template <>
NOALIAS __forceinline void Store( const Maths::HVec4& src, MathsStorage::Float4* const __restrict pDst );

/// Load HVec4 from a Float4A16
template <>
NOALIAS __forceinline const Maths::HVec4 Load( MathsStorage::Float4A16 const * const __restrict pSrc );

/// Store HVec4 into a Float4A16
template <>
NOALIAS __forceinline void Store( const Maths::HVec4& src, MathsStorage::Float4A16* const __restrict pDst );


/// Load C_RGBA_F32 from a Float4
template <>
NOALIAS __forceinline const Maths::C_RGBA_F32 Load( MathsStorage::Float4 const * const __restrict pSrc );

/// Store C_RGBA_F32 into a Float4
template <>
NOALIAS __forceinline void Store( const Maths::C_RGBA_F32& src, MathsStorage::Float4* const __restrict pDst );

/// Load C_RGBA_F32 from a Float4A16
template <>
NOALIAS __forceinline const Maths::C_RGBA_F32 Load( MathsStorage::Float4 const * const __restrict pSrc );

/// Store C_RGBA_F32 into a Float4A16
template <>
NOALIAS __forceinline void Store( const Maths::C_RGBA_F32& src, MathsStorage::Float4A16* const __restrict pDst );

/// Load C_Quaternion from a Float4
template <>
NOALIAS __forceinline const Maths::C_Quaternion Load( MathsStorage::Float4 const * const __restrict pSrc );

/// Store C_Quaternion into a Float4
template <>
NOALIAS __forceinline void Store( const Maths::C_Quaternion& src, MathsStorage::Float4* const __restrict pDst );

/// Load C_Quaternion from a Float4A16
template <>
NOALIAS __forceinline const Maths::C_Quaternion Load( MathsStorage::Float4 const * const __restrict pSrc );

/// Store C_Quaternion into a Float4A16
template <>
NOALIAS __forceinline void Store( const Maths::C_Quaternion& src, MathsStorage::Float4A16* const __restrict pDst );


/// Load HMat33 from a Float33
template <>
NOALIAS __forceinline const Maths::HMat33 Load( MathsStorage::Float33 const * const __restrict pSrc );

/// Store HMat33 into a Float33
template <>
NOALIAS __forceinline void Store( const Maths::HMat33& src, MathsStorage::Float33* const __restrict pDst );


/// Load HMat43 from a Float43
template <>
NOALIAS __forceinline const Maths::HMat43 Load( MathsStorage::Float43 const * const __restrict pSrc );

/// Store HMat43 into a Float43
/// NOTE: This will crash if used to store to Write-Combined or Uncached memory! Use StoreUncached instead!
template <>
NOALIAS __forceinline void Store( const Maths::HMat43& src, MathsStorage::Float43* const __restrict pDst );

/// Store HMat43 into a Float43
// NOTE: Specialised version of above function for storing to Write-Combined or Uncached memory. Don't use for stores to cached memory!
NOALIAS __forceinline void StoreUncached( const Maths::HMat43& src, MathsStorage::Float43* const __restrict pDst );


/// Load HMat44 from a Float44
template <>
NOALIAS __forceinline const Maths::HMat44 Load( MathsStorage::Float44 const * const __restrict pSrc );

/// Store HMat44 into Float44
template <>
NOALIAS __forceinline void Store( const Maths::HMat44& src, MathsStorage::Float44* const __restrict pDst );

/// Load HMat33 from a Matrix33
template <>
NOALIAS __forceinline const Maths::HMat33 Load( MathsStorage::Matrix33 const * const __restrict pSrc );

/// Store HMat33 into a Matrix33
template <>
NOALIAS __forceinline void Store( const Maths::HMat33& src, MathsStorage::Matrix33* const __restrict pDst );


/// Load HMat44 from a MatrixRow44
template <>
NOALIAS __forceinline const Maths::HMat44 Load( MathsStorage::MatrixRow44 const * const __restrict pSrc );

/// Store HMat44 into MatrixRow44
template <>
NOALIAS __forceinline void Store( const Maths::HMat44& src, MathsStorage::MatrixRow44* const __restrict pDst );


/// Load HMat44 from a MatrixCol44
template <>
NOALIAS __forceinline const Maths::HMat44 Load( MathsStorage::MatrixCol44 const * const __restrict pSrc );

/// Store HMat44 into MatrixCol44
template <>
NOALIAS __forceinline void Store( const Maths::HMat44& src, MathsStorage::MatrixCol44* const __restrict pDst );


/// Load HMat44 from a Float44A16
template <>
NOALIAS __forceinline const Maths::HMat44 Load( MathsStorage::Float44A16 const * const __restrict pSrc );

/// Store HMat44 into Float44A16
template <>
NOALIAS __forceinline void Store( const Maths::HMat44& src, MathsStorage::Float44A16* const __restrict pDst );

/// Load HMat44 from a MatrixRow44A16
template <>
NOALIAS __forceinline const Maths::HMat44 Load( MathsStorage::MatrixRow44A16 const * const __restrict pSrc );

/// Store HMat44 into MatrixRow44A16
template <>
NOALIAS __forceinline void Store( const Maths::HMat44& src, MathsStorage::MatrixRow44A16* const __restrict pDst );

/// Load HMat44 from a MatrixCol44A16
template <>
NOALIAS __forceinline const Maths::HMat44 Load( MathsStorage::MatrixCol44A16 const * const __restrict pSrc );

/// Store HMat44 into MatrixCol44A16
template <>
NOALIAS __forceinline void Store( const Maths::HMat44& src, MathsStorage::MatrixCol44A16* const __restrict pDst );




/// Load C_Vertex from a Float3
template <>
NOALIAS __forceinline const Maths::C_Vertex Load( MathsStorage::Float3 const * const __restrict pSrc );

/// Store C_Vertex into a Float3
template <>
NOALIAS __forceinline void Store( const Maths::C_Vertex& src, MathsStorage::Float3* const __restrict pDst );


/// Load C_Vector from a Float3
template <>
NOALIAS __forceinline const Maths::C_Vector Load( MathsStorage::Float3 const * const __restrict pSrc );

/// Store C_Vector into a Float3
template <>
NOALIAS __forceinline void Store( const Maths::C_Vector& src, MathsStorage::Float3* const __restrict pDst );


/// Load C_Normal from a Float3
template <>
NOALIAS __forceinline const Maths::C_Normal Load( MathsStorage::Float3 const * const __restrict pSrc );

/// Store C_Normal into a Float3
template <>
NOALIAS __forceinline void Store( const Maths::C_Normal& src, MathsStorage::Float3* const __restrict pDst );


/// Load V128 from a Float3A16
template <>
NOALIAS __forceinline const Maths::V128 Load( MathsStorage::Float3A16 const * const __restrict pSrc );

/// Store V128 into a Float3A16
template <>
NOALIAS __forceinline void Store( const Maths::V128& src, MathsStorage::Float3A16* const __restrict pDst );

/// Load V128 from a Float4A16
template <>
NOALIAS __forceinline const Maths::V128 Load( MathsStorage::Float4A16 const * const __restrict pSrc );

/// Store V128 into a Float4A16
template <>
NOALIAS __forceinline void Store( const Maths::V128& src, MathsStorage::Float4A16* const __restrict pDst );

/// Load V128 from a Float4
template <>
NOALIAS __forceinline const Maths::V128 Load( MathsStorage::Float4 const * const __restrict pSrc );

/// Store V128 into a Float4
template <>
NOALIAS __forceinline void Store( const Maths::V128& src, MathsStorage::Float4* const __restrict pDst );

/// Load V128 from a Float3
template <>
NOALIAS __forceinline const Maths::V128 Load( MathsStorage::Float3 const * const __restrict pSrc );

/// Store V128 into a Float3
template <>
NOALIAS __forceinline void Store( const Maths::V128& src, MathsStorage::Float3* const __restrict pDst );

/// Load V128 from a HenDN3
template <>
NOALIAS __forceinline const Maths::V128 Load( MathsStorage::HenDN3 const * const __restrict pSrc );

/// Store V128 into a HenDN3
template <>
NOALIAS __forceinline void Store( const Maths::V128& src, MathsStorage::HenDN3* const __restrict pDst );

// Load V128 into a UByteN4
template <>
NOALIAS __forceinline const Maths::V128 Load( MathsStorage::UByteN4 const * const __restrict pSrc );

// Store V128 into a UByteN4
template <>
NOALIAS __forceinline void Store( const Maths::V128& src, MathsStorage::UByteN4* const __restrict pDst );


//
// New templated storage class : has template parameters for both runtime & storage types.
// The Load() function doesn't need to be specialised by the user since the runtime type is known.
//

template < typename T_Runtime, typename T_Storage >
class Storage
{
public:
    
    //#if defined( PLATFORM_PC ) && defined( _DEBUG )
    NOALIAS __forceinline Storage() {}
    //#else
    //NOALIAS __forceinline Storage() : m_Data( UNDEFINED ) {}
    //#endif
    
    NOALIAS __forceinline Storage( const Storage<T_Runtime,T_Storage>& copy ) : m_Data( copy.m_Data ) {}
    NOALIAS __forceinline const Storage<T_Runtime,T_Storage>& operator = ( const Storage<T_Runtime,T_Storage>& copy ) { m_Data = copy.m_Data; return copy; }
    
    NOALIAS __forceinline Storage( const T_Storage& src ) : m_Data( src ) {}
    NOALIAS __forceinline Storage( const T_Runtime& src ) { Store( src, &m_Data ); }
    
    NOALIAS __forceinline const Storage<T_Runtime,T_Storage>& operator = ( const T_Storage& src ) { m_Data = src; return *this; }
    NOALIAS __forceinline const Storage<T_Runtime,T_Storage>& operator = ( const T_Runtime& src ) { Store( src, &m_Data ); return *this; }
    
    NOALIAS __forceinline operator const T_Storage () const { return m_Data; }
    NOALIAS __forceinline operator const T_Runtime () const { return Load<T_Runtime>( &m_Data ); }
    
    T_Storage m_Data;
};



}; // namespace Maths


#endif // #if defined( INCLUDE_THIS )




