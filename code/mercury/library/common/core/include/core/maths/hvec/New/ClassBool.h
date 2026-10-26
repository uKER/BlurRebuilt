#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_BOOL_H
#define SOFT_MATHS_HVEC_NEW_CLASS_BOOL_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_BOOL_H
#define MATHS_HVEC_NEW_CLASS_BOOL_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Boolean_Classes
/// @{

/// VBool: Vector register bool class: bool result from a scalar comparison.
/// All SIMD implementations have the same value splatted into all elements.
class ALIGN16 PASSINREG DARWIN64_ABI VBool
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline VBool() {}
    NOALIAS __forceinline VBool( const VBool& copy ) : m_V( copy.m_V ) {}
    NOALIAS __forceinline const VBool& operator = ( const VBool& copy ) { m_V = copy.m_V; return copy; }
    #endif
    
    NOALIAS __forceinline operator const bool () const;
    
    #if defined( _SPU_ )
    vector unsigned int     m_V;
    #elif defined( _PPU_ )
    vector bool int         m_V;
    #elif defined( _VMX128_ )
    __vector4               m_V;
    #elif defined( _SSE_ )
    __m128                  m_V;
    #else
    bool                    m_V;
    #endif
};

/// Vec4Bool: Vector register 4 element bool class : bool results from a vector inequality comparison [ <, <=, >, >= ].
class ALIGN16 PASSINREG DARWIN64_ABI Vec4Bool
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vec4Bool() {}
    NOALIAS __forceinline Vec4Bool( const Vec4Bool& copy );                     ///< Vec4Bool Copy Constructor
    NOALIAS __forceinline const Vec4Bool& operator = ( const Vec4Bool& copy );  ///< Vec4Bool Copy Assignment
    #endif
    
    #if defined( _SPU_ )
    vector unsigned int     m_V;
    #elif defined( _PPU_ )
    vector bool int         m_V;
    #elif defined( _VMX128_ )
    __vector4               m_V;
    #elif defined( _SSE_ )
    __m128                  m_V;
    #else
    bool                    m_V[4];
    #endif
};

/// Vec8Bool: Vector register 8 element bool class : bool results from a vector inequality comparison [ <, <=, >, >= ].
class ALIGN16 PASSINREG DARWIN64_ABI Vec8Bool
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vec8Bool() {}
    NOALIAS __forceinline Vec8Bool( const Vec8Bool& copy );                     ///< Vec8Bool Copy Constructor
    NOALIAS __forceinline const Vec8Bool& operator = ( const Vec8Bool& copy );  ///< Vec8Bool Copy Assignment
    #endif
    
    #if defined( _SPU_ )
    vector unsigned short   m_V;
    #elif defined( _PPU_ )
    vector bool short       m_V;
    #elif defined( _VMX128_ )
    __vector4               m_V;
    #elif defined( _SSE_ )
    __m128i                 m_V;
    #else
    bool                    m_V[8];
    #endif
};

/// Vec16Bool: Vector register 16 element bool class : bool results from a vector inequality comparison [ <, <=, >, >= ].
class ALIGN16 PASSINREG DARWIN64_ABI Vec16Bool
{
public:
    
    #if !defined( PASSINREG_ENABLED )
    NOALIAS __forceinline Vec16Bool() {}
    NOALIAS __forceinline Vec16Bool( const Vec16Bool& copy );                    ///< Vec16Bool Copy Constructor
    NOALIAS __forceinline const Vec16Bool& operator = ( const Vec16Bool& copy ); ///< Vec16Bool Copy Assignment
    #endif
    
    #if defined( _SPU_ )
    vector unsigned char    m_V;
    #elif defined( _PPU_ )
    vector bool char        m_V;
    #elif defined( _VMX128_ )
    __vector4               m_V;
    #elif defined( _SSE_ )
    __m128i                 m_V;
    #else
    bool                    m_V[16];
    #endif
};

/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


