#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_SCALAR_H
#define SOFT_MATHS_HVEC_NEW_CLASS_SCALAR_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_SCALAR_H
#define MATHS_HVEC_NEW_CLASS_SCALAR_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Scalar_Classes
/// @{

/// VF32: Vector register float class. All SIMD implementations have the same value splatted into all elements.
class ALIGN16 PASSINREG DARWIN64_ABI VF32
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline VF32() {}                                                                    ///< VF32 Default Constructor (Undefined!)
    NOALIAS __forceinline VF32( const VF32& copy ) : m_V( copy.m_V ) {}                                ///< VF32 Copy Constructor
    NOALIAS __forceinline const VF32& operator = ( const VF32& copy ) { m_V = copy.m_V; return copy; } ///< VF32 Copy Assignment
    #endif
    
    #if defined( _SIMD_ )
    V128    m_V; ///< VF32 storage
    #else
    F32     m_V; ///< VF32 storage
    #endif
};

/// StorageVF32: Templated Storage Type
typedef Storage< VF32, F32 > StorageVF32;

/// VF64: Vector register double class. All SIMD implementations have the same value splatted into all elements.
class ALIGN16 DARWIN64_ABI VF64
{
public:
    
    NOALIAS __forceinline VF64() {}                                    ///< VF64 Default Constructor (Undefined!)
    NOALIAS __forceinline VF64( const VF64& copy );                    ///< VF64 Copy Constructor
    NOALIAS __forceinline const VF64& operator = ( const VF64& copy ); ///< VF64 Copy Assignment
    
    #if defined( _SPU_ )
    vector double   m_V; ///< VF64 storage
    #elif defined( _PPU_ )
    F64             m_V; ///< VF64 storage. No PPU vector double support!
    F64             pad; ///< Padding to ensure PPU & SPU structures are the same size!
    #elif defined( _VMX128_ )
    F64             m_V; ///< VF64 storage. No 360 vector double support!
    #elif defined( _SSE_ )
    __m128d         m_V; ///< VF64 storage
    #else
    F64             m_V; ///< VF64 storage
    #endif
};

/// VS8: Vector register S8 class. The same value is splatted into all elements.
class ALIGN16 PASSINREG DARWIN64_ABI VS8
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline VS8() {}                                                                   ///< VS8 Default Constructor (Undefined!)
    NOALIAS __forceinline VS8( const VS8& copy ) : m_V( copy.m_V ) {}                                ///< VS8 Copy Constructor
    NOALIAS __forceinline const VS8& operator = ( const VS8& copy ) { m_V = copy.m_V; return copy; } ///< VS8 Copy Assignment
    #endif
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector signed char      m_V; ///< vector S8 storage
    #elif defined( _VMX128_ )
    __vector4               m_V; ///< vector S8 storage
    #elif defined( _SSE_ )
    __m128i                 m_V; ///< vector S8 storage
    #else
    S8                      m_V; ///< vector S8 storage
    #endif
};

/// VU8: Vector register U8 class. The same value is splatted into all elements.
class ALIGN16 PASSINREG DARWIN64_ABI VU8
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline VU8() {}                                                                   ///< VU8 Default Constructor (Undefined!)
    NOALIAS __forceinline VU8( const VU8& copy ) : m_V( copy.m_V ) {}                                ///< VU8 Copy Constructor
    NOALIAS __forceinline const VU8& operator = ( const VU8& copy ) { m_V = copy.m_V; return copy; } ///< VU8 Copy Assignment
    #endif
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector unsigned char    m_V; ///< vector U8 storage
    #elif defined( _VMX128_ )
    __vector4               m_V; ///< vector U8 storage
    #elif defined( _SSE_ )
    __m128i                 m_V; ///< vector U8 storage
    #else
    U8                      m_V; ///< vector U8 storage
    #endif
};

/// VS16: Vector register S16 class. The same value is splatted into all elements.
class ALIGN16 PASSINREG DARWIN64_ABI VS16
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline VS16() {}                                                                    ///< VS16 Default Constructor (Undefined!)
    NOALIAS __forceinline VS16( const VS16& copy ) : m_V( copy.m_V ) {}                                ///< VS16 Copy Constructor
    NOALIAS __forceinline const VS16& operator = ( const VS16& copy ) { m_V = copy.m_V; return copy; } ///< VS16 Copy Assignment
    #endif
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector signed short     m_V; ///< vector S16 storage
    #elif defined( _VMX128_ )
    __vector4               m_V; ///< vector S16 storage
    #elif defined( _SSE_ )
    __m128i                 m_V; ///< vector S16 storage
    #else
    S16                     m_V; ///< vector S16 storage
    #endif
};

/// VU16: Vector register U16 class. The same value is splatted into all elements.
class ALIGN16 PASSINREG DARWIN64_ABI VU16
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline VU16() {}                                                                    ///< VU16 Default Constructor (Undefined!)
    NOALIAS __forceinline VU16( const VU16& copy ) : m_V( copy.m_V ) {}                                ///< VU16 Copy Constructor
    NOALIAS __forceinline const VU16& operator = ( const VU16& copy ) { m_V = copy.m_V; return copy; } ///< VU16 Copy Assignment
    #endif
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector unsigned short   m_V; ///< vector U16 storage
    #elif defined( _VMX128_ )
    __vector4               m_V; ///< vector U16 storage
    #elif defined( _SSE_ )
    __m128i                 m_V; ///< vector U16 storage
    #else
    U16                     m_V; ///< vector U16 storage
    #endif
};

/// VS32: Vector register S32 class. The same value is splatted into all elements.
class ALIGN16 PASSINREG DARWIN64_ABI VS32
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline VS32() {}                                                                    ///< VS32 Default Constructor (Undefined!)
    NOALIAS __forceinline VS32( const VS32& copy ) : m_V( copy.m_V ) {}                                ///< VS32 Copy Constructor
    NOALIAS __forceinline const VS32& operator = ( const VS32& copy ) { m_V = copy.m_V; return copy; } ///< VS32 Copy Assignment
    #endif
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector signed int       m_V; ///< vector S32 storage
    #elif defined( _VMX128_ )
    __vector4               m_V; ///< vector S32 storage
    #elif defined( _SSE_ )
    __m128i                 m_V; ///< vector S32 storage
    #else
    S32                     m_V; ///< vector S32 storage
    #endif
};

/// VU32: Vector register U32 class. The same value is splatted into all elements.
class ALIGN16 PASSINREG DARWIN64_ABI VU32
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline VU32() {}                                                                    ///< VU32 Default Constructor (Undefined!)
    NOALIAS __forceinline VU32( const VU32& copy ) : m_V( copy.m_V ) {}                                ///< VU32 Copy Constructor
    NOALIAS __forceinline const VU32& operator = ( const VU32& copy ) { m_V = copy.m_V; return copy; } ///< VU32 Copy Assignment
    #endif
    
    #if defined( _SPU_ ) || defined( _PPU_ )
    vector unsigned int     m_V; ///< vector U32 storage
    #elif defined( _VMX128_ )
    __vector4               m_V; ///< vector U32 storage
    #elif defined( _SSE_ )
    __m128i                 m_V; ///< vector U32 storage
    #else
    U32                     m_V; ///< vector U32 storage
    #endif
};

/// VS64: Vector register S64 class. The same value is splatted into all elements.
//class ALIGN16 DARWIN64_ABI VS64
//{
//public:
//    
//    #if !defined( PASSINREG_ENABLED )
//    NOALIAS __forceinline VS64() {}                                                                    ///< VS64 Default Constructor (Undefined!)
//    NOALIAS __forceinline VS64( const VS64& copy ) : m_V( copy.m_V ) {}                                ///< VS64 Copy Constructor
//    NOALIAS __forceinline const VS64& operator = ( const VS64& copy ) { m_V = copy.m_V; return copy; } ///< VS64 Copy Assignment
//    #endif
//    
//    #if defined( _SPU_ )
//    vector signed long long     m_V; ///< vector S64 storage
//    #elif defined( _PPU_ )
//    S64                         m_V; ///< vector S64 storage. No PPU vector S64 support!
//    S64                         pad; ///< Padding to ensure PPU & SPU structures are the same size!
//    #elif defined( _VMX128_ )
//    S64                         m_V; ///< vector S64 storage. No 360 vector S64 support!
//    #elif defined( _SSE_ )
//    __m128i                     m_V; ///< vector S64 storage
//    #else
//    S64                         m_V; ///< vector S64 storage
//    #endif
//};

/// VU64: Vector register U64 class. The same value is splatted into all elements.
//class ALIGN16 DARWIN64_ABI VU64
//{
//public:
//    
//    #if !defined( PASSINREG_ENABLED )
//    NOALIAS __forceinline VU64() {}                                                                    ///< VU64 Default Constructor (Undefined!)
//    NOALIAS __forceinline VU64( const VU64& copy ) : m_V( copy.m_V ) {}                                ///< VU64 Copy Constructor
//    NOALIAS __forceinline const VU64& operator = ( const VU64& copy ) { m_V = copy.m_V; return copy; } ///< VU64 Copy Assignment
//    #endif
//    
//    #if defined( _SPU_ )
//    vector signed long long     m_V; ///< vector U64 storage
//    #elif defined( _PPU_ )
//    U64                         m_V; ///< vector U64 storage. No PPU vector S64 support!
//    U64                         pad; ///< Padding to ensure PPU & SPU structures are the same size!
//    #elif defined( _VMX128_ )
//    U64                         m_V; ///< vector U64 storage. No 360 vector S64 support!
//    #elif defined( _SSE_ )
//    __m128i                     m_V; ///< vector U64 storage
//    #else
//    U64                         m_V; ///< vector U64 storage
//    #endif
//};

/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


