#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_VECTOR_H
#define SOFT_MATHS_HVEC_NEW_CLASS_VECTOR_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_VECTOR_H
#define MATHS_HVEC_NEW_CLASS_VECTOR_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vector_Classes
/// @{

/// Vec4F32: Vector register 4 element vector float class.
class ALIGN16 PASSINREG DARWIN64_ABI Vec4F32
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vec4F32() {}                                       ///< Vec4F32 Default Constructor (Undefined!)
    NOALIAS __forceinline Vec4F32( const Vec4F32& copy );                    ///< Vec4F32 Copy Constructor
    NOALIAS __forceinline const Vec4F32& operator = ( const Vec4F32& copy ); ///< Vec4F32 Copy Assignment
    #endif
    
    #if defined( _SIMD_ )
    V128    m_V;    ///< Vec4F32 storage
    #else
    F32     m_V[4]; ///< Vec4F32 storage
    #endif
};

// StorageVec[1,2,3,4]F32: Templated Storage Types
typedef Storage< Vec4F32, MathsStorage::Float1 > StorageVec1F32; ///< 1 element float storage for Vec4F32 Load & Store
typedef Storage< Vec4F32, MathsStorage::Float2 > StorageVec2F32; ///< 2 element float storage for Vec4F32 Load & Store
typedef Storage< Vec4F32, MathsStorage::Float3 > StorageVec3F32; ///< 3 element float storage for Vec4F32 Load & Store
typedef Storage< Vec4F32, MathsStorage::Float4 > StorageVec4F32; ///< 4 element float storage for Vec4F32 Load & Store

typedef Storage< Vec4F32, MathsStorage::_UByte4 >  StorageVec4F32_U8;    ///< 4 element U8 storage for Vec4F32 Load/Store.
typedef Storage< Vec4F32, MathsStorage::_UByteN4 > StorageVec4F32_U8N;   ///< 4 element U8 storage for Vec4F32 Load/Store. Normalised: 255 -> 1.0
typedef Storage< Vec4F32, MathsStorage::Short4 >   StorageVec4F32_S16;   ///< 4 element S16 storage for Vec4F32 Load/Store.
typedef Storage< Vec4F32, MathsStorage::ShortN4 >  StorageVec4F32_S16N;  ///< 4 element S16 storage for Vec4F32 Load/Store. Normalised: -32767 -> -1.0, 32767 -> 1.0
typedef Storage< Vec4F32, MathsStorage::_HenD3 >   StorageVec4F32_HenD3; ///< 3 element SHHDX3 storage for Vec4F32 Load/Store.
typedef Storage< Vec4F32, MathsStorage::Vec4F16 >  StorageVec4F32_F16;   ///< 4 element F16 storage for Vec4F32 Load/Store.


/// Vec8F32: Vector register 8 element vector float class.
class ALIGN16 DARWIN64_ABI Vec8F32
{
public:
    
    NOALIAS __forceinline Vec8F32() {}                                                   ///< Vec8F32 Default Constructor (Undefined!)
    NOALIAS __forceinline Vec8F32( const Vec8F32& copy );                          ///< Vec8F32 Copy Constructor
    NOALIAS __forceinline const Vec8F32& operator = ( const Vec8F32& copy ); ///< Vec8F32 Copy Assignment
    
    Vec4F32 m_V0; ///< Vec8F32 storage
    Vec4F32 m_V1; ///< Vec8F32 storage
};


// StorageVec[5,6,7,8]F32: Templated Storage Types
typedef Storage< Vec8F32, MathsStorage::Float5 > StorageVec5F32; ///< 5 element float storage for Vec8F32 Load & Store
typedef Storage< Vec8F32, MathsStorage::Float6 > StorageVec6F32; ///< 6 element float storage for Vec8F32 Load & Store
typedef Storage< Vec8F32, MathsStorage::Float7 > StorageVec7F32; ///< 7 element float storage for Vec8F32 Load & Store
typedef Storage< Vec8F32, MathsStorage::Float8 > StorageVec8F32; ///< 8 element float storage for Vec8F32 Load & Store


/// Vec12F32: Vector register 12 element vector float class.
class ALIGN16 DARWIN64_ABI Vec12F32
{
public:
    
    NOALIAS __forceinline Vec12F32() {}                                         ///< Vec12F32 Default Constructor (Undefined!)
    NOALIAS __forceinline Vec12F32( const Vec12F32& copy );                     ///< Vec12F32 Copy Constructor
    NOALIAS __forceinline const Vec12F32& operator = ( const Vec12F32& copy );  ///< Vec12F32 Copy Assignment
    
    Vec4F32 m_V0; ///< Vec12F32 storage
    Vec4F32 m_V1; ///< Vec12F32 storage
    Vec4F32 m_V2; ///< Vec12F32 storage
};


// StorageVec[9,10,11,12]F32: Templated Storage Types
typedef Storage< Vec12F32, MathsStorage::Float9  > StorageVec9F32;  ///< 9  element float storage for Vec12F32 Load & Store
typedef Storage< Vec12F32, MathsStorage::Float10 > StorageVec10F32; ///< 10 element float storage for Vec12F32 Load & Store
typedef Storage< Vec12F32, MathsStorage::Float11 > StorageVec11F32; ///< 11 element float storage for Vec12F32 Load & Store
typedef Storage< Vec12F32, MathsStorage::Float12 > StorageVec12F32; ///< 12 element float storage for Vec12F32 Load & Store


/// Vec16F32: Vector register 16 element vector float class.
class ALIGN16 DARWIN64_ABI Vec16F32
{
public:
    
    NOALIAS __forceinline Vec16F32() {}                                         ///< Vec16F32 Default Constructor (Undefined!)
    NOALIAS __forceinline Vec16F32( const Vec16F32& copy );                     ///< Vec16F32 Copy Constructor
    NOALIAS __forceinline const Vec16F32& operator = ( const Vec16F32& copy );  ///< Vec16F32 Copy Assignment
    
    Vec4F32 m_V0; ///< Vec16F32 storage
    Vec4F32 m_V1; ///< Vec16F32 storage
    Vec4F32 m_V2; ///< Vec16F32 storage
    Vec4F32 m_V3; ///< Vec16F32 storage
};


// StorageVec[13,14,15,16]F32: Templated Storage Types
typedef Storage< Vec16F32, MathsStorage::Float13 > StorageVec13F32; ///< 13 element float storage for Vec16F32 Load & Store
typedef Storage< Vec16F32, MathsStorage::Float14 > StorageVec14F32; ///< 14 element float storage for Vec16F32 Load & Store
typedef Storage< Vec16F32, MathsStorage::Float15 > StorageVec15F32; ///< 15 element float storage for Vec16F32 Load & Store
typedef Storage< Vec16F32, MathsStorage::Float16 > StorageVec16F32; ///< 16 element float storage for Vec16F32 Load & Store


/// Point3: 3D Point implemented with Vec4F32
/// Note: For matrix multiplication, this class has implicit W=1.0f
/// (This value is NOT stored in the W element of the underlying Vec4F32!)
class ALIGN16 PASSINREG DARWIN64_ABI Point3
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Point3() {}                                                                      ///< Point3 Default Constructor (Undefined!)
    NOALIAS __forceinline Point3( const Point3& copy ) : m_V( copy.m_V ) {}                                ///< Point3 Copy Constructor
    NOALIAS __forceinline const Point3& operator = ( const Point3& copy ) { m_V = copy.m_V; return copy; } ///< Point3 Copy Assignment
    #endif
    
    Vec4F32 m_V; ///< Point3 storage
};

typedef Storage< Point3, MathsStorage::Float3 > StoragePoint3; ///< Point3 3 element float storage

/// HPoint3: Homogeneous 3D Point implemented with Vec4F32
class ALIGN16 PASSINREG DARWIN64_ABI HPoint3
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline HPoint3() {}                                                                       ///< HPoint3 Default Constructor (Undefined!)
    NOALIAS __forceinline HPoint3( const HPoint3& copy ) : m_V( copy.m_V ) {}                                ///< HPoint3 Copy Constructor
    NOALIAS __forceinline const HPoint3& operator = ( const HPoint3& copy ) { m_V = copy.m_V; return copy; } ///< HPoint3 Copy Assignment
    #endif
    
    Vec4F32 m_V; ///< HPoint3 storage
};

typedef Storage< HPoint3, MathsStorage::Float4 > StorageHPoint3; ///< HPoint3 4 element float storage

/// Vector2: 2D Vector implemented with Vec4F32
class ALIGN16 PASSINREG DARWIN64_ABI Vector2
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vector2() {}                                                                       ///< Vector2 Default Constructor (Undefined!)
    NOALIAS __forceinline Vector2( const Vector2& copy ) : m_V( copy.m_V ) {}                                ///< Vector2 Copy Constructor
    NOALIAS __forceinline const Vector2& operator = ( const Vector2& copy ) { m_V = copy.m_V; return copy; } ///< Vector2 Copy Assignment
    #endif
    
    Vec4F32 m_V; ///< Vector2 storage
};

typedef Storage< Vector2, MathsStorage::Float2 > StorageVector2; ///< Vector2 2 element float storage

/// Vector3: 3D Vector implemented with Vec4F32
class ALIGN16 PASSINREG DARWIN64_ABI Vector3
{
/// Note: For matrix multiplication, this class has implicit W=0.0f
/// (This value is NOT stored in the W element of the underlying Vec4F32!)
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vector3() {}                                                                       ///< Vector3 Default Constructor (Undefined!)
    NOALIAS __forceinline Vector3( const Vector3& copy ) : m_V( copy.m_V ) {}                                ///< Vector3 Copy Constructor
    NOALIAS __forceinline const Vector3& operator = ( const Vector3& copy ) { m_V = copy.m_V; return copy; } ///< Vector3 Copy Assignment
    #endif
    
    Vec4F32 m_V; ///< Vector3 storage
};

typedef Storage< Vector3, MathsStorage::Float3 > StorageVector3; ///< Vector3 3 element float storage

/// Vector3NL: 3D Vector implemented with Vec4F32, stored as Normal & Length
class ALIGN16 PASSINREG DARWIN64_ABI Vector3NL
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vector3NL() {}                                                                         ///< Vector3NL Default Constructor (Undefined!)
    NOALIAS __forceinline Vector3NL( const Vector3NL& copy ) : m_V( copy.m_V ) {}                                ///< Vector3NL Copy Constructor
    NOALIAS __forceinline const Vector3NL& operator = ( const Vector3NL& copy ) { m_V = copy.m_V; return copy; } ///< Vector3NL Copy Assignment
    #endif
    
    Vec4F32 m_V; ///< Vector3 (x,y,z), Length (w)
};

/// Vector4: 4D Vector implemented with Vec4F32
class ALIGN16 PASSINREG DARWIN64_ABI Vector4
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vector4() {}                                                                       ///< Vector4 Default Constructor (Undefined!)
    NOALIAS __forceinline Vector4( const Vector4& copy ) : m_V( copy.m_V ) {}                                ///< Vector4 Copy Constructor
    NOALIAS __forceinline const Vector4& operator = ( const Vector4& copy ) { m_V = copy.m_V; return copy; } ///< Vector4 Copy Assignment
    #endif
    
    Vec4F32 m_V; ///< Vector4 storage
};

typedef Storage< Vector4, MathsStorage::Float4 > StorageVector4; ///< Vector4 4 element float storage


/// Vector5: 5D Vector
class ALIGN16 DARWIN64_ABI Vector5
{
public:
    
    NOALIAS __forceinline Vector5() {}                                       ///< Vector5 Default Constructor (Undefined!)
    NOALIAS __forceinline Vector5( const Vector5& copy );                    ///< Vector5 Copy Constructor
    NOALIAS __forceinline const Vector5& operator = ( const Vector5& copy ); ///< Vector5 Copy Assignment
    
    #if defined( _SIMD_ )
    Vec8F32 m_V;    ///< Vector5 storage
    #else
    F32     m_V[5]; ///< Vector5 storage
    #endif
};

typedef Storage< Vector5, MathsStorage::Float5 > StorageVector5; ///< Vector5 5 element float storage


/// Vector6: 6D Vector
class ALIGN16 DARWIN64_ABI Vector6
{
public:
    
    NOALIAS __forceinline Vector6() {}                                       ///< Vector6 Default Constructor (Undefined!)
    NOALIAS __forceinline Vector6( const Vector6& copy );                    ///< Vector6 Copy Constructor
    NOALIAS __forceinline const Vector6& operator = ( const Vector6& copy ); ///< Vector6 Copy Assignment
    
    #if defined( _SIMD_ )
    Vec8F32 m_V;    ///< Vector6 storage
    #else
    F32     m_V[6]; ///< Vector6 storage
    #endif
};

typedef Storage< Vector6, MathsStorage::Float6 > StorageVector6; ///< Vector6 6 element float storage


/// Vector7: 7D Vector
class ALIGN16 DARWIN64_ABI Vector7
{
public:
    
    NOALIAS __forceinline Vector7() {}                                       ///< Vector7 Default Constructor (Undefined!)
    NOALIAS __forceinline Vector7( const Vector7& copy );                    ///< Vector7 Copy Constructor
    NOALIAS __forceinline const Vector7& operator = ( const Vector7& copy ); ///< Vector7 Copy Assignment
    
    #if defined( _SIMD_ )
    Vec8F32 m_V;    ///< Vector7 storage
    #else
    F32     m_V[7]; ///< Vector7 storage
    #endif
};

typedef Storage< Vector7, MathsStorage::Float7 > StorageVector7; ///< Vector7 7 element float storage


/// Vector8: 8D Vector
class ALIGN16 DARWIN64_ABI Vector8
{
public:
    
    NOALIAS __forceinline Vector8() {}                                       ///< Vector8 Default Constructor (Undefined!)
    NOALIAS __forceinline Vector8( const Vector8& copy );                    ///< Vector8 Copy Constructor
    NOALIAS __forceinline const Vector8& operator = ( const Vector8& copy ); ///< Vector8 Copy Assignment
    
    Vec8F32 m_V; ///< Vector8 storage
};

typedef Storage< Vector8, MathsStorage::Float8 > StorageVector8; ///< Vector8 8 element float storage


/// Normal3: 3D Normal implemented with Vec4F32
/// Note: For matrix multiplication, this class has implicit W=0.0f
/// (This value is NOT maintained in the W element of the underlying Vec4F32!)
/// Due to accumulated float errors, after a number of operations (e.g. rotations) this class will need to be re-normalised!
class ALIGN16 PASSINREG DARWIN64_ABI Normal3
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Normal3() {}                                                                       ///< Normal3 Default Constructor (Undefined!)
    NOALIAS __forceinline Normal3( const Normal3& copy ) : m_V( copy.m_V ) {}                                ///< Normal3 Copy Constructor
    NOALIAS __forceinline const Normal3& operator = ( const Normal3& copy ) { m_V = copy.m_V; return copy; } ///< Normal3 Copy Assignment
    #endif
    
    Vec4F32 m_V; ///< Normal3 storage
};

typedef Storage< Normal3, MathsStorage::Float3 > StorageNormal3; ///< Normal3 3 element float storage





/// Vec16S8: Vector register 16 element S8 class.
class ALIGN16 PASSINREG DARWIN64_ABI Vec16S8
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vec16S8() {}                                       ///< Vec16S8 Default Constructor (Undefined!)
    NOALIAS __forceinline Vec16S8( const Vec16S8& copy );                    ///< Vec16S8 Copy Constructor
    NOALIAS __forceinline const Vec16S8& operator = ( const Vec16S8& copy ); ///< Vec16S8 Copy Assignment
    #endif
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector signed char      m_V;     ///< Vec16S8 storage
    #elif defined( _VMX128_ )
    __vector4               m_V;     ///< Vec16S8 storage
    #elif defined( _SSE_ )
    __m128i                 m_V;     ///< Vec16S8 storage
    #else
    S8                      m_V[16]; ///< Vec16S8 storage
    #endif
};

/// Vec16U8: Vector register 16 element U8 class.
class ALIGN16 PASSINREG DARWIN64_ABI Vec16U8
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vec16U8() {}                                       ///< Vec16U8 Default Constructor (Undefined!)
    NOALIAS __forceinline Vec16U8( const Vec16U8& copy );                    ///< Vec16U8 Copy Constructor
    NOALIAS __forceinline const Vec16U8& operator = ( const Vec16U8& copy ); ///< Vec16U8 Copy Assignment
    #endif 
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector unsigned char    m_V;     ///< Vec16U8 storage
    #elif defined( _VMX128_ )
    __vector4               m_V;     ///< Vec16U8 storage
    #elif defined( _SSE_ )
    __m128i                 m_V;     ///< Vec16U8 storage
    #else
    U8                      m_V[16]; ///< Vec16U8 storage
    #endif
};

/// Vec8S16: Vector register 8 element S16 class.
class ALIGN16 PASSINREG DARWIN64_ABI Vec8S16
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vec8S16() {}                                       ///< Vec8S16 Default Constructor (Undefined!)
    NOALIAS __forceinline Vec8S16( const Vec8S16& copy );                    ///< Vec8S16 Copy Constructor
    NOALIAS __forceinline const Vec8S16& operator = ( const Vec8S16& copy ); ///< Vec8S16 Copy Assignment
    #endif
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector signed short     m_V;    ///< Vec8S16 storage
    #elif defined( _VMX128_ )
    __vector4               m_V;    ///< Vec8S16 storage
    #elif defined( _SSE_ )
    __m128i                 m_V;    ///< Vec8S16 storage
    #else
    S16                     m_V[8]; ///< Vec8S16 storage
    #endif
};

/// Vec8U16: Vector register 8 element U16 class.
class ALIGN16 PASSINREG DARWIN64_ABI Vec8U16
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vec8U16() {}                                       ///< Vec8U16 Default Constructor (Undefined!)
    NOALIAS __forceinline Vec8U16( const Vec8U16& copy );                    ///< Vec8U16 Copy Constructor
    NOALIAS __forceinline const Vec8U16& operator = ( const Vec8U16& copy ); ///< Vec8U16 Copy Assignment
    #endif
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector unsigned short   m_V;    ///< Vec8U16 storage
    #elif defined( _VMX128_ )
    __vector4               m_V;    ///< Vec8U16 storage
    #elif defined( _SSE_ )
    __m128i                 m_V;    ///< Vec8U16 storage
    #else
    U16                     m_V[8]; ///< Vec8U16 storage
    #endif
};

/// Vec4S32: Vector register 4 element S32 class.
class ALIGN16 PASSINREG DARWIN64_ABI Vec4S32
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vec4S32() {}                                       ///< Vec4S32 Default Constructor (Undefined!)
    NOALIAS __forceinline Vec4S32( const Vec4S32& copy );                    ///< Vec4S32 Copy Constructor
    NOALIAS __forceinline const Vec4S32& operator = ( const Vec4S32& copy ); ///< Vec4S32 Copy Assignment
    #endif
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector signed int       m_V;    ///< Vec4S32 storage
    #elif defined( _VMX128_ )
    __vector4               m_V;    ///< Vec4S32 storage
    #elif defined( _SSE_ )
    __m128i                 m_V;    ///< Vec4S32 storage
    #else
    S32                     m_V[4]; ///< Vec4S32 storage
    #endif
};

/// Vec4U32: Vector register 4 element U32 class.
class ALIGN16 PASSINREG DARWIN64_ABI Vec4U32
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vec4U32() {}                                       ///< Vec4U32 Default Constructor (Undefined!)
    NOALIAS __forceinline Vec4U32( const Vec4U32& copy );                    ///< Vec4U32 Copy Constructor
    NOALIAS __forceinline const Vec4U32& operator = ( const Vec4U32& copy ); ///< Vec4U32 Copy Assignment
    #endif
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector unsigned int     m_V;    ///< Vec4U32 storage
    #elif defined( _VMX128_ )
    __vector4               m_V;    ///< Vec4U32 storage
    #elif defined( _SSE_ )
    __m128i                 m_V;    ///< Vec4U32 storage
    #else
    U32                     m_V[4]; ///< Vec4U32 storage
    #endif
};

/// Vec2S64: Vector register 2 element S64 class.
//class ALIGN16 DARWIN64_ABI Vec2S64
//{
//public:
//    
//    #if !defined( PASSINREG_ENABLED )
//    NOALIAS __forceinline Vec2S64() {}                                       ///< Vec2S64 Default Constructor (Undefined!)
//    NOALIAS __forceinline Vec2S64( const Vec2S64& copy );                    ///< Vec2S64 Copy Constructor
//    NOALIAS __forceinline const Vec2S64& operator = ( const Vec2S64& copy ); ///< Vec2S64 Copy Assignment
//    #endif
//    
//    #if defined( _SPU_ )
//    vector signed long long     m_V;    ///< Vec2S64 storage
//    #elif defined( _SSE_ )
//    __m128i                     m_V;    ///< Vec2S64 storage
//    #else
//    S64                         m_V[2]; ///< Vec2S64 storage
//    #endif
//};

/// Vec2U64: Vector register 2 element U64 class.
//class ALIGN16 DARWIN64_ABI Vec2U64
//{
//public:
//    
//    #if !defined( PASSINREG_ENABLED )
//    NOALIAS __forceinline Vec2U64() {}                                       ///< Vec2U64 Default Constructor (Undefined!)
//    NOALIAS __forceinline Vec2U64( const Vec2U64& copy );                    ///< Vec2U64 Copy Constructor
//    NOALIAS __forceinline const Vec2U64& operator = ( const Vec2U64& copy ); ///< Vec2U64 Copy Assignment
//    #endif
//    
//    #if defined( _SPU_ )
//    vector unsigned long long   m_V;    ///< Vec2U64 storage
//    #elif defined( _SSE_ )
//    __m128i                     m_V;    ///< Vec2U64 storage
//    #else
//    U64                         m_V[2]; ///< Vec2U64 storage
//    #endif
//};

/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


