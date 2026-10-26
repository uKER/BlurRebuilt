#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_SIMD_H
#define SOFT_MATHS_HVEC_SIMD_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_SIMD_H
#define MATHS_HVEC_SIMD_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

//Uncomment to turn off force-inline stack allocations on PS3
#if defined PLATFORM_PS3 || defined PLATFORM_SPU
//#define NO_INLINE_STACK_ALLOC_PS3
#endif

extern void *(*bizalloca)(unsigned int);

namespace Maths
{
//#if !defined( PCREF )
//#if defined( PLATFORM_PC ) && !defined( __INTEL_COMPILER )
//#define PCREF &
//#else
//#define PCREF
//#endif
//#endif

//#if !defined( PASSINREG )
//#if defined( PLATFORM_XENON ) && ( _XDK_VER >= 5632 ) && !defined( CFG_ALLOW_OPTIMIZE_OFF ) && ( defined( CFG_SPEED ) || defined( CFG_GOLD ) )
//#define PASSINREG __declspec( passinreg )
//#define PASSINREG_ENABLED
//#else
//#define PASSINREG
//#endif
//#endif // #if !defined( PASSINREG )


#if 0 && defined( COMPILER_VC ) && defined( USER_JDENTON )
    #define SYS_DEPRECATED __declspec( deprecated )
#elif defined( COMPILER_GNU ) && defined( USER_JDENTON )
    //#define SYS_DEPRECATED __attribute__( ( deprecated ) )
    #define SYS_DEPRECATED
#else
    #define SYS_DEPRECATED
#endif

//
// SIMD Vector Element Indices
//

enum VectorElement      { VX = 0, VY = 1, VZ = 2, VW = 3 };
enum MatrixElement      { MX = 0, MY = 1, MZ = 2, MW = 3 }; // MatrixElement == MatrixRow
enum MatrixAxis         { AX = 0, AY = 1, AZ = 2, AW = 3 }; // MatrixAxis == MatrixRow
enum MatrixRow          { RX = 0, RY = 1, RZ = 2, RW = 3 };
enum MatrixColumn       { CX = 0, CY = 1, CZ = 2, CW = 3 };
enum QuaternionElement  { QX = 0, QY = 1, QZ = 2, QA = 3 };
enum SphereElement      { SX = 0, SY = 1, SZ = 2, SR = 3 };


// Enum for permuting float elements from concatenation of 2 vectors
enum PermuteElement
{
    V0X = 0, V0Y = 1, V0Z = 2, V0W = 3, // X, Y, Z & W components from 1st vector
    V1X = 4, V1Y = 5, V1Z = 6, V1W = 7, // X, Y, Z & W components from 2nd vector
};




#if defined( _VMX128_ ) || defined( _PPU_ ) || defined (_SPU_)

// VMX Condition Register Bit Masks & Shifts
#define VMX_CR6_ALL_TRUE_BIT    7
#define VMX_CR6_ALL_FALSE_BIT   5
#define VMX_CR6_ALL_TRUE        ( 1 << VMX_CR6_ALL_TRUE_BIT )
#define VMX_CR6_ALL_FALSE       ( 1 << VMX_CR6_ALL_FALSE_BIT )

enum VMX_MASK_INSERT
{
    VMX_MASK_INSERT_X       = 0x8,
    VMX_MASK_INSERT_Y       = 0x4,
    VMX_MASK_INSERT_Z       = 0x2,
    VMX_MASK_INSERT_W       = 0x1,
    
    VMX_MASK_INSERT_XY      = VMX_MASK_INSERT_X | VMX_MASK_INSERT_Y,
    VMX_MASK_INSERT_XZ      = VMX_MASK_INSERT_X | VMX_MASK_INSERT_Z,
    VMX_MASK_INSERT_XW      = VMX_MASK_INSERT_X | VMX_MASK_INSERT_W,
    VMX_MASK_INSERT_YZ      = VMX_MASK_INSERT_Y | VMX_MASK_INSERT_Z,
    VMX_MASK_INSERT_YW      = VMX_MASK_INSERT_Y | VMX_MASK_INSERT_W,
    VMX_MASK_INSERT_ZW      = VMX_MASK_INSERT_Z | VMX_MASK_INSERT_W,
    
    VMX_MASK_INSERT_XYZ     = VMX_MASK_INSERT_X | VMX_MASK_INSERT_Y | VMX_MASK_INSERT_Z,
    VMX_MASK_INSERT_XYW     = VMX_MASK_INSERT_X | VMX_MASK_INSERT_Y | VMX_MASK_INSERT_W,
    VMX_MASK_INSERT_XZW     = VMX_MASK_INSERT_X | VMX_MASK_INSERT_Z | VMX_MASK_INSERT_W,
    VMX_MASK_INSERT_YZW     = VMX_MASK_INSERT_Y | VMX_MASK_INSERT_Z | VMX_MASK_INSERT_W,
    
    VMX_MASK_INSERT_XYZW    = VMX_MASK_INSERT_X | VMX_MASK_INSERT_Y | VMX_MASK_INSERT_Z | VMX_MASK_INSERT_W
};

enum VMX_ROTATE_LEFT
{
    VMX_ROTATE_NONE   = 0,
    
    VMX_ROTATE_X_TO_X = 0,
    VMX_ROTATE_Y_TO_X = 1,
    VMX_ROTATE_Z_TO_X = 2,
    VMX_ROTATE_W_TO_X = 3,
    
    VMX_ROTATE_X_TO_Y = 3,
    VMX_ROTATE_Y_TO_Y = 0,
    VMX_ROTATE_Z_TO_Y = 1,
    VMX_ROTATE_W_TO_Y = 2,
    
    VMX_ROTATE_X_TO_Z = 2,
    VMX_ROTATE_Y_TO_Z = 3,
    VMX_ROTATE_Z_TO_Z = 0,
    VMX_ROTATE_W_TO_Z = 1,
    
    VMX_ROTATE_X_TO_W = 1,
    VMX_ROTATE_Y_TO_W = 2,
    VMX_ROTATE_Z_TO_W = 3,
    VMX_ROTATE_W_TO_W = 0
};

#endif








//
// Temporary Stack Buffer Allocator: Data aligned and padded to cache line size ( 128 bytes ).
// Buffer is cache controlled on allocation & deallocation to prevent memory traffic pollution.
// Note: Memory is allocated using _alloca() and will therefore only be released on function exit!
//

template < int BUFFERSIZE = 4096 >
class C_StackAllocTemplate
{
public:
    
    C_StackAllocTemplate( const U32 size );
    ~C_StackAllocTemplate();
    
    char*           Data() const;
    
protected:
    
    U32   m_Size;
    #if defined( _DEBUG )// || defined( COMPILER_GNU )
    char* m_pMemory;
    #endif
    char* m_pData;
    
	#if defined( _DEBUG ) || defined( NO_INLINE_STACK_ALLOC_PS3 )
	char m_buffer[BUFFERSIZE];
	#endif
};

typedef C_StackAllocTemplate<4096> C_StackAlloc;


//
// Float & Integer Vector Types
//

#if defined( _SPU_ )

typedef vector float V128;

struct V64
{
    F32 m_X, m_Y;
};

#elif defined( _PPU_ )

typedef vector float V128;

struct V64
{
    F32 m_X, m_Y;
};

#elif defined( _VMX128_ )

typedef PASSINREG __vector4 V128;
typedef PASSINREG __int64   V64;

#elif defined( _SSE_ )

typedef ::__m128 V128;
typedef ::__m64  V64;

#else

struct V128
{
    //V128( const F32 x, const F32 y, const F32 z, const F32 w );
    
    F32 m_X, m_Y, m_Z, m_W;

	#if defined (PLATFORM_PS3) || defined (PLATFORM_SPU)
	//operator vector float() const			{ return (vector float)(m_X,m_Y,m_Z,m_W); }
	operator vector float() const			{ return spu_insert( m_X, spu_insert( m_Y, spu_insert( m_Z, spu_promote( m_W, VW ), VZ ), VY ), VX ); }
	void operator=(const vector float src)	{ (vector float&) *this = src; }
	#endif
};

// TODO - provide SIMD_**** wrapper for V128 type, or cast to __v*** emu by default ??
//
//
/*
__forceinline V128 operator*= (V128& a, const V128 b) {
	V128 ret; 
	a.m_X*= b.m_X;
	a.m_Y*= b.m_Y;
	a.m_Z*= b.m_Z;
	a.m_W*= b.m_W;
	return	a;
};
__forceinline V128 operator+= (V128& a, const V128 b) {
	
	a.m_X+= b.m_X;
	a.m_Y+= b.m_Y;
	a.m_Z+= b.m_Z;
	a.m_W+= b.m_W;
	return	a;
};
__forceinline V128 operator* (const V128 a, const V128 b) {
	V128 ret; 
	ret.m_X = a.m_X * b.m_X;
	ret.m_Y = a.m_Y * b.m_Y;
	ret.m_Z = a.m_Z * b.m_Z;
	ret.m_W = a.m_W * b.m_W;
	return	ret;
};
__forceinline V128 operator+ (const V128 a, const V128 b) {
	V128 ret; 
	ret.m_X = a.m_X + b.m_X;
	ret.m_Y = a.m_Y + b.m_Y;
	ret.m_Z = a.m_Z + b.m_Z;
	ret.m_W = a.m_W + b.m_W;
	return	ret;
};
__forceinline V128 operator- (const V128 a, const V128 b) {
	V128 ret; 
	ret.m_X = a.m_X - b.m_X;
	ret.m_Y = a.m_Y - b.m_Y;
	ret.m_Z = a.m_Z - b.m_Z;
	ret.m_W = a.m_W - b.m_W;
	return	ret;
};
*/
struct V64
{
    F32 m_X, m_Y;
};

#endif


//#if defined( PLATFORM_PC ) && !defined( __INTEL_COMPILER )
//typedef const V128& V128_Param;
//#else
//typedef const V128 V128_Param;
//#endif



//
// Integer, Scalar & Vector Selector Classes
// TODO: Move scalar selector classes into scalar.h/.inl/.cpp and
// move vector selector classes into vector.h/.inl/.cpp
//


class C_SelectorBool
{
NOALIAS __forceinline friend const U32 Select( const C_SelectorBool& test, const U32 result_if_true, const U32 result_if_false );

template < typename T >
NOALIAS __forceinline friend T* Select( const C_SelectorBool& test, T* result_if_true, T* result_if_false );

template < typename T >
NOALIAS __forceinline friend T* Select( const C_SelectorBool& test, T const * result_if_true, T const * result_if_false );

public:
    
    NOALIAS __forceinline C_SelectorBool( const S32 initialState );
    NOALIAS __forceinline C_SelectorBool( const U32 initialState );
    NOALIAS __forceinline C_SelectorBool( const bool initialState );
    
private:
    
    U32     m_Mask; // true == 0xFFFFFFFF, false == 0x00000000
};







class Scalar_Bool
{
public:
    
	// Default constructor
	NOALIAS __forceinline Scalar_Bool() {}
    
    /// Undefined Constructor
    NOALIAS __forceinline Scalar_Bool( const _UNDEFINED undefined ) __attribute__(( always_inline ));
    
    /// Copy Constructor
    NOALIAS __forceinline Scalar_Bool( const Scalar_Bool& copy ) __attribute__(( always_inline ));
    
    #if defined( _SPU_ )
    /// Constructor: Initial state set from vector unsigned int
    NOALIAS __forceinline explicit Scalar_Bool( const vector unsigned int src ) __attribute__(( always_inline ));
    #elif defined( _PPU_ )
    /// Constructor: Initial state set from vector bool int
    NOALIAS __forceinline explicit Scalar_Bool( const vector bool int src ) __attribute__(( always_inline ));
    #elif defined( _SIMD_ )
    /// Constructor: Initial state set from V128
    NOALIAS __forceinline explicit Scalar_Bool( const V128 PCREF src ) __attribute__(( always_inline ));
    #endif
    
    /// Constructor: Initial state set from bool
    NOALIAS __forceinline Scalar_Bool( const bool initialState ) __attribute__(( always_inline ));
    
    /// Constructor: Initial state set from S32
    NOALIAS __forceinline explicit Scalar_Bool( const S32 initialState ) __attribute__(( always_inline ));
    
    /// Constructor: Initial state set from U32
    NOALIAS __forceinline explicit Scalar_Bool( const U32 initialState ) __attribute__(( always_inline ));
    
    /// Copy Assignment
    NOALIAS __forceinline const Scalar_Bool& operator = ( const Scalar_Bool& copy ) __attribute__(( always_inline ));
    
    NOALIAS __forceinline operator const bool () const __restrict __attribute__(( const, always_inline ));
    
    #if defined( _SPU_ )
    vector unsigned int     m_V;
    #elif defined( _PPU_ )
    vector bool int         m_V;
    #elif defined( _SIMD_ )
    V128                    m_V;
    #else
    U32                     m_F;
    #endif

    #if defined( _SIMD_ )
    #if defined( _PPU_ )
      NOALIAS __forceinline const V128            GetV128() const __restrict                  { return (vector float)m_V; }
      NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) __restrict  { m_V = (vector bool int)src; }
    #elif defined( _SPU_ )
      NOALIAS __forceinline const V128            GetV128() const __restrict                  { return (vector float)m_V; }
      NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) __restrict  { m_V = (vector unsigned int)src; }
    #else
      NOALIAS __forceinline const V128            GetV128() const __restrict                  { return m_V; }
      NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) __restrict  { m_V = src; }
      #endif
    #else
      //NOALIAS __forceinline const V128            GetV128() const __restrict                  { const V128 v = { m_V[0], m_V[1], m_V[2], 0.0f }; return v; }
      //NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) __restrict  { *this = HVec3(src); }
    NOALIAS __forceinline const V128            GetV128() const __restrict                  { V128  v;  v.m_X = v.m_Y = v.m_Z = v.m_W = (F32&) m_F; return v; }
    NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) __restrict  { m_F = (U32&) src;}
    #endif


};

NOALIAS __forceinline const Scalar_Bool operator ~ ( const Scalar_Bool PCREF src ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_Bool operator & ( const Scalar_Bool PCREF src0, const Scalar_Bool PCREF src1 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_Bool operator | ( const Scalar_Bool PCREF src0, const Scalar_Bool PCREF src1 ) __attribute__(( always_inline ));
NOALIAS __forceinline const Scalar_Bool operator ^ ( const Scalar_Bool PCREF src0, const Scalar_Bool PCREF src1 ) __attribute__(( always_inline ));

NOALIAS __forceinline void operator &= ( Scalar_Bool& src0, const Scalar_Bool PCREF src1 ) __attribute__(( always_inline ));
NOALIAS __forceinline void operator |= ( Scalar_Bool& src0, const Scalar_Bool PCREF src1 ) __attribute__(( always_inline ));
    
NOALIAS __forceinline const Scalar_Bool operator ! ( const Scalar_Bool PCREF src ) __attribute__(( always_inline ));




//NOALIAS __forceinline const bool operator && ( const Scalar_Bool PCREF src0, const Scalar_Bool PCREF src1 ) __attribute__(( always_inline ));
//NOALIAS __forceinline const bool operator && ( const Scalar_Bool PCREF src0, const bool src1 ) __attribute__(( always_inline ));
//NOALIAS __forceinline const bool operator && ( const bool src0, const Scalar_Bool PCREF src1 ) __attribute__(( always_inline ));
                                                                                        
//NOALIAS __forceinline const bool operator || ( const Scalar_Bool PCREF src0, const Scalar_Bool PCREF src1 ) __attribute__(( always_inline ));
//NOALIAS __forceinline const bool operator || ( const Scalar_Bool PCREF src0, const bool src1 ) __attribute__(( always_inline ));
//NOALIAS __forceinline const bool operator || ( const bool src0, const Scalar_Bool PCREF src1 ) __attribute__(( always_inline ));



class Vector2_Bool
{
public:
    
    /// Undefined Constructor
    NOALIAS Vector2_Bool( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS Vector2_Bool( const Vector2_Bool& copy );
    
    /// Constructor from Scalar_Bool
    NOALIAS Vector2_Bool( const Scalar_Bool PCREF src );
    
    #if defined( _SPU_ )
    NOALIAS explicit Vector2_Bool( const vector unsigned int src );
    #elif defined( _PPU_ )
    NOALIAS explicit Vector2_Bool( const vector bool int src );
    #elif defined( _SIMD_ )
    NOALIAS explicit Vector2_Bool( const V128 PCREF src );
    #else
    NOALIAS explicit Vector2_Bool( const U32 x, const U32 y );
    #endif
    
    #if defined( _SPU_ )
    vector unsigned int     m_V;
    #elif defined( _PPU_ )
    vector bool int         m_V;
    #elif defined( _SIMD_ )
    V128                    m_V;
    #else
    U32                     m_X, m_Y;
    //bool                    m_X, m_Y;
    #endif
};

NOALIAS const Vector2_Bool operator ~ ( const Vector2_Bool PCREF src );
NOALIAS const Vector2_Bool operator & ( const Vector2_Bool PCREF src0, const Vector2_Bool PCREF src1 );
NOALIAS const Vector2_Bool operator | ( const Vector2_Bool PCREF src0, const Vector2_Bool PCREF src1 );
NOALIAS const Vector2_Bool operator ^ ( const Vector2_Bool PCREF src0, const Vector2_Bool PCREF src1 );

NOALIAS const Scalar_Bool X( const Vector2_Bool PCREF src );
NOALIAS const Scalar_Bool Y( const Vector2_Bool PCREF src );
    


class Vec4Bool;

class Vector3_Bool
{
public:
    
    /// Undefined Constructor
    NOALIAS Vector3_Bool( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS Vector3_Bool( const Vector3_Bool& copy );
    
    /// Constructor from Scalar_Bool
    NOALIAS Vector3_Bool( const Scalar_Bool PCREF src );
    
    
    /// Constructor from Vec4Bool
    NOALIAS __forceinline Vector3_Bool( const Vec4Bool& src ) __attribute__(( always_inline ));
    
    /// Assignment operator from Vec4Bool
    NOALIAS __forceinline const Vec4Bool& operator = ( const Vec4Bool& src ) __attribute__(( always_inline ));
    
    /// Cast to Vec4Bool
    NOALIAS __forceinline operator const Vec4Bool () const __attribute__(( always_inline ));
    
    
    #if defined( _SPU_ )
    NOALIAS explicit Vector3_Bool( const vector unsigned int src );
    #elif defined( _PPU_ )
    NOALIAS explicit Vector3_Bool( const vector bool int src );
    #elif defined( _SIMD_ )
    NOALIAS explicit Vector3_Bool( const V128 PCREF src );
    #else
    NOALIAS explicit Vector3_Bool( const U32 x, const U32 y, const U32 z );
    #endif
    
    #if defined( _SPU_ )
    vector unsigned int     m_V;
    #elif defined( _PPU_ )
    vector bool int         m_V;
    #elif defined( _SIMD_ )
    V128                    m_V;
    #else
    U32                     m_X, m_Y, m_Z;
    //bool                    m_X, m_Y, m_Z;
    #endif
};

NOALIAS const Vector3_Bool operator ~ ( const Vector3_Bool PCREF src );
NOALIAS const Vector3_Bool operator & ( const Vector3_Bool PCREF src0, const Vector3_Bool PCREF src1 );
NOALIAS const Vector3_Bool operator | ( const Vector3_Bool PCREF src0, const Vector3_Bool PCREF src1 );
NOALIAS const Vector3_Bool operator ^ ( const Vector3_Bool PCREF src0, const Vector3_Bool PCREF src1 );

NOALIAS const Scalar_Bool X( const Vector3_Bool PCREF src );
NOALIAS const Scalar_Bool Y( const Vector3_Bool PCREF src );
NOALIAS const Scalar_Bool Z( const Vector3_Bool PCREF src );

template < bool x, bool y, bool z >
NOALIAS const Vector3_Bool Create_Vector3_Bool();



class Vector4_Bool
{
public:
    
    /// Undefined Constructor
    NOALIAS Vector4_Bool( const _UNDEFINED undefined );
    
    /// Copy Constructor
    NOALIAS Vector4_Bool( const Vector4_Bool& copy );
    
    /// Constructor from Scalar_Bool
    NOALIAS Vector4_Bool( const Scalar_Bool PCREF src );
    
    
    /// Constructor from Vec4Bool
    NOALIAS __forceinline Vector4_Bool( const Vec4Bool& src ) __attribute__(( always_inline ));
    
    /// Assignment operator from Vec4Bool
    NOALIAS __forceinline const Vec4Bool& operator = ( const Vec4Bool& src ) __attribute__(( always_inline ));
    
    /// Cast to Vec4Bool
    NOALIAS __forceinline operator const Vec4Bool () const __attribute__(( always_inline ));
    
    
    #if defined( _SPU_ )
    NOALIAS explicit Vector4_Bool( const vector unsigned int src );
    #elif defined( _PPU_ )
    NOALIAS explicit Vector4_Bool( const vector bool int src );
    #elif defined( _SIMD_ )
    NOALIAS explicit Vector4_Bool( const V128 PCREF src );
    #else
    NOALIAS explicit Vector4_Bool( const U32 x, const U32 y, const U32 z, const U32 w );
    #endif
    
    #if defined( _SPU_ )
    vector unsigned int     m_V;
    #elif defined( _PPU_ )
    vector bool int         m_V;
    #elif defined( _SIMD_ )
    V128                    m_V;
    #else
    U32                     m_X, m_Y, m_Z, m_W;
    //bool                    m_X, m_Y, m_Z, m_W;
    #endif
};

NOALIAS const Vector4_Bool operator ~ ( const Vector4_Bool PCREF src );
NOALIAS const Vector4_Bool operator & ( const Vector4_Bool PCREF src0, const Vector4_Bool PCREF src1 );
NOALIAS const Vector4_Bool operator | ( const Vector4_Bool PCREF src0, const Vector4_Bool PCREF src1 );
NOALIAS const Vector4_Bool operator ^ ( const Vector4_Bool PCREF src0, const Vector4_Bool PCREF src1 );

NOALIAS const Scalar_Bool X( const Vector4_Bool PCREF src );
NOALIAS const Scalar_Bool Y( const Vector4_Bool PCREF src );
NOALIAS const Scalar_Bool Z( const Vector4_Bool PCREF src );
NOALIAS const Scalar_Bool W( const Vector4_Bool PCREF src );

NOALIAS const S32 AllTrue( const Vector4_Bool PCREF src );
NOALIAS const S32 AllFalse( const Vector4_Bool PCREF src );

NOALIAS const S32 AnyTrue( const Vector4_Bool PCREF src );
NOALIAS const S32 AnyFalse( const Vector4_Bool PCREF src );



typedef Scalar_Bool  C_SelectorScalar;
typedef Vector2_Bool C_SelectorVector2;
typedef Vector3_Bool C_SelectorVector3;
typedef Vector4_Bool C_SelectorVector4;




template < typename T_Ptr > class Vector_Ptr
{
public:
    
    NOALIAS __forceinline Vector_Ptr< T_Ptr >( const _UNDEFINED )
    {
    }
    
    NOALIAS __forceinline Vector_Ptr< T_Ptr >( T_Ptr* const & ptr )
        #if defined( _VMX128_ )
        : m_Ptr( __vspltw( __lvlx( &ptr, 0x0 ), 0 ) )
        #else
        : m_Ptr( ptr )
        #endif
    {
    }
    
public:
    
    #if defined( _VMX128_ )
    V128        m_Ptr;
    #else
    T_Ptr*      m_Ptr;
    #endif
};

template < typename T_Ptr > NOALIAS __forceinline const Vector_Ptr< T_Ptr > Select( const Scalar_Bool PCREF condition, const Vector_Ptr< T_Ptr > resultIfTrue, const Vector_Ptr< T_Ptr > resultIfFalse )
{
    #if defined( _VMX128_ )
    Vector_Ptr< T_Ptr > result( UNDEFINED );
    result.m_Ptr = __vsel( resultIfFalse.m_Ptr, resultIfTrue.m_Ptr, condition.m_V );
    return result;
    #else
    return condition ? resultIfTrue : resultIfFalse;
    #endif
}

template < typename T_Ptr > NOALIAS __forceinline void Store( const Vector_Ptr< T_Ptr > PCREF vecPtr, T_Ptr*& intPtr )
{
    #if defined( _VMX128_ )
    __stvewx( vecPtr.m_Ptr, &intPtr, 0x0 );
    #else
    intPtr = vecPtr.m_Ptr;
    #endif
}
    


//
// 4xF32 Float Array
//

//struct V4_F32
//{
//    /// Default Constructor
//    V4_F32();
//    
//    /// Copy Constructor
//    V4_F32( const V4_F32& copy );
//    
//    /// F32 XYZW Constructor
//    V4_F32( const F32& x, const F32& y, const F32& z, const F32& w );
//    
//    #if defined( _SIMD_ )
//    
//    /// V128 Constructor
//    V4_F32( const V128 PCREF src );
//    
//    union
//    {
//        struct { F32  m_X, m_Y, m_Z, m_W; };
//        struct { V64  m_XY,     m_ZW; };
//        struct { V128 m_XYZW; };
//    };
//    
//    #else
//    
//    F32 m_X, m_Y, m_Z, m_W;
//    
//    #endif
//};

//
// 4xS32 Integer Array
//

//struct V4_S32
//{
//    /// Default Constructor
//    V4_S32();
//    
//    /// Copy Constructor
//    V4_S32( const V4_S32& copy );
//    
//    /// S32 XYZW Constructor
//    V4_S32( const S32 x, const S32 y, const S32 z, const S32 w );
//    
//    #if defined( _SIMD_ )
//    
//    /// V128 Constructor
//    V4_S32( const V128 PCREF src );
//    
//    union
//    {
//        struct { S32  m_X, m_Y, m_Z, m_W; };
//        struct { V64  m_XY,     m_ZW; };
//        struct { V128 m_XYZW; };
//    };
//    
//    #else
//    
//    S32 m_X, m_Y, m_Z, m_W;
//    
//    #endif
//};

}; // end namespace Maths




//
// Constants
//

#ifndef MATHS_HVEC_CONST_DEFINES
#define MATHS_HVEC_CONST_DEFINES

#if defined( _SIMD_ )

#if defined( COMPILER_VC )

#define LocalConstV4F32(x) const Maths::V128 x
#define LocalConstV3F32(x) const Maths::V128 x
#define LocalConstV2F32(x) const Maths::V128 x

#define ConstV4F32(x) __declspec( selectany ) extern const Maths::V128 x
#define ConstV3F32(x) __declspec( selectany ) extern const Maths::V128 x
#define ConstV2F32(x) __declspec( selectany ) extern const Maths::V128 x

#elif defined( COMPILER_GNU )

#define LocalConstV4F32(x) const Maths::V128 x
#define LocalConstV3F32(x) const Maths::V128 x
#define LocalConstV2F32(x) const Maths::V128 x

#define ConstV4F32(x) extern const Maths::V128 __attribute__(( weak )) x
#define ConstV3F32(x) extern const Maths::V128 __attribute__(( weak )) x
#define ConstV2F32(x) extern const Maths::V128 __attribute__(( weak )) x

#else

#define LocalConstV4F32(x) const Maths::V128 x
#define LocalConstV3F32(x) const Maths::V128 x
#define LocalConstV2F32(x) const Maths::V128 x

#define ConstV4F32(x) const Maths::V128 x
#define ConstV3F32(x) const Maths::V128 x
#define ConstV2F32(x) const Maths::V128 x

#endif // #if defined( COMPILER_VC )

#else // #if defined( _SIMD_ )

#define LocalConstV4F32(x) const F32 x[4]
#define LocalConstV3F32(x) const F32 x[3]
#define LocalConstV2F32(x) const F32 x[2]

#define ConstV4F32(x) const F32 x[4]
#define ConstV3F32(x) const F32 x[3]
#define ConstV2F32(x) const F32 x[2]

#endif // #if defined( _SIMD_ )

#endif // #ifndef MATHS_HVEC_CONST_DEFINES



inline const F32 TypePun_IntToFloat( const U8 bits_x, const U8 bits_y, const U8 bits_z, const U8 bits_w )
{
    union I2F { struct { U8 x, y, z, w; }; F32 f; };
    I2F convert;
    convert.x = bits_x;
    convert.y = bits_y;
    convert.z = bits_z;
    convert.w = bits_w;
    return convert.f;
}

inline const F32 TypePun_IntToFloat( const U32 bits )
{
    typedef union { U32 i; F32 f; } I2F;
    I2F convert;
    convert.i = bits;
    return convert.f;
}

inline const U32 TypePun_FloatToInt( const F32 bits )
{
    typedef union { F32 f; U32 i; } F2I;
    F2I convert;
    convert.f = bits;
    return convert.i;
}

extern const F32 c_fSet;
extern const F32 c_fClear;
extern const F32 c_fAbsMask;
extern const F32 c_fSignMask;

#if defined( _VMX128_ )
#define c_VectorZero __vspltisw( 0 )
#elif defined( _SSE_ )
#define c_VectorZero _mm_setzero_ps()
#else
ConstV4F32( c_VectorZero ) = { 0.0f, 0.0f, 0.0f, 0.0f };
#endif


#if defined( _SPU_ ) || defined( _PPU_ )

#define c_VectorZero    Maths::Const< vector float >::Zero()
#define c_VectorQuarter Maths::Const< vector float >::Quarter()
#define c_VectorHalf    Maths::Const< vector float >::Half()
#define c_VectorOne     Maths::Const< vector float >::One()
#define c_VectorTwo     Maths::Const< vector float >::Two()
#define c_VectorThree   Maths::Const< vector float >::Three()
#define c_VectorFour    Maths::Const< vector float >::Four()

#elif defined( _VMX128_ )

NOALIAS static __forceinline const __vector4 VMX_ConstantOne( void )
{
    return __vupkd3d( __vzero(), VPACK_D3DCOLOR );
}

#define c_VectorQuarter __vcfux( __vspltisw(1), 2 )
#define c_VectorHalf    __vcfux( __vspltisw(1), 1 )
#define c_VectorOne     VMX_ConstantOne()
#define c_VectorTwo     __vcfux( __vspltisw(2), 0 )
#define c_VectorThree   __vupkd3d( __vzero(), VPACK_NORMSHORT4 )
#define c_VectorFour    __vcfux( __vspltisw(4), 0 )

#else

ConstV4F32( c_VectorQuarter ) = { 0.25f, 0.25f, 0.25f, 0.25f };
ConstV4F32( c_VectorHalf )    = { 0.5f,  0.5f,  0.5f,  0.5f  };
ConstV4F32( c_VectorOne )     = { 1.0f,  1.0f,  1.0f,  1.0f  };
ConstV4F32( c_VectorTwo )     = { 2.0f,  2.0f,  2.0f,  2.0f  };
ConstV4F32( c_VectorThree )   = { 3.0f,  3.0f,  3.0f,  3.0f  };
ConstV4F32( c_VectorFour )    = { 4.0f,  4.0f,  4.0f,  4.0f  };

#endif

#if defined( _SPU_ ) || defined( _PPU_ )

#define c_UnitVectorX Maths::Const< vector float >::UnitX()
#define c_UnitVectorY Maths::Const< vector float >::UnitY()
#define c_UnitVectorZ Maths::Const< vector float >::UnitZ()
#define c_UnitVectorW Maths::Const< vector float >::UnitW()

#elif defined( _VMX128_ )

#define c_UnitVectorX __vrlimi( __vzero(), c_VectorOne, VMX_MASK_INSERT_X, 0 )
#define c_UnitVectorY __vrlimi( __vzero(), c_VectorOne, VMX_MASK_INSERT_Y, 0 )
#define c_UnitVectorZ __vrlimi( __vzero(), c_VectorOne, VMX_MASK_INSERT_Z, 0 )
#define c_UnitVectorW __vupkd3d( __vzero(), VPACK_FLOAT16_2 )

#else

ConstV4F32( c_UnitVectorX ) = { 1.0f, 0.0f, 0.0f, 0.0f };
ConstV4F32( c_UnitVectorY ) = { 0.0f, 1.0f, 0.0f, 0.0f };
ConstV4F32( c_UnitVectorZ ) = { 0.0f, 0.0f, 1.0f, 0.0f };
ConstV4F32( c_UnitVectorW ) = { 0.0f, 0.0f, 0.0f, 1.0f };

#endif

#if defined( _PPU_ )

#define c_ScalarZero    c_VectorZero
#define c_ScalarQuarter c_VectorQuarter
#define c_ScalarHalf    c_VectorHalf
#define c_ScalarOne     c_VectorOne
#define c_ScalarTwo     c_VectorTwo
#define c_ScalarThree   c_VectorThree
#define c_ScalarFour    c_VectorFour
#define c_ScalarEpsilon c_VectorEpsilon

#elif defined( _VMX128_ )

#define c_ScalarQuarter c_VectorQuarter
#define c_ScalarHalf    c_VectorHalf
#define c_ScalarOne     c_VectorOne
#define c_ScalarTwo     c_VectorTwo
#define c_ScalarThree   c_VectorThree
#define c_ScalarFour    c_VectorFour
#define c_ScalarEpsilon c_VectorEpsilon

#elif defined( _SSE_ )

ConstV4F32( c_ScalarQuarter ) = { 0.25f, 0.0f, 0.0f, 0.0f };
ConstV4F32( c_ScalarHalf )    = { 0.5f,  0.0f, 0.0f, 0.0f };
ConstV4F32( c_ScalarOne )     = { 1.0f,  0.0f, 0.0f, 0.0f };
ConstV4F32( c_ScalarTwo )     = { 2.0f,  0.0f, 0.0f, 0.0f };
ConstV4F32( c_ScalarThree )   = { 3.0f,  0.0f, 0.0f, 0.0f };
ConstV4F32( c_ScalarFour )    = { 4.0f,  0.0f, 0.0f, 0.0f };
ConstV4F32( c_ScalarEpsilon ) = { FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON };

#else

extern const F32 c_ScalarQuarter;
extern const F32 c_ScalarHalf;
extern const F32 c_ScalarOne;
extern const F32 c_ScalarTwo;
extern const F32 c_ScalarThree;
extern const F32 c_ScalarFour;
extern const F32 c_ScalarEpsilon;

#endif



#if defined( _SPU_ ) || defined( _PPU_ )

//#define c_VectorAbsMask  Maths::Const< vector unsigned int >::AbsMask()
//#define c_VectorSignMask Maths::Const< vector unsigned int >::SignMask()

#elif defined( _VMX128_ )

//#define c_VectorAbsMask     __vsrw( __vspltisw( -1 ), __vspltisw( 1 ) )
//#define c_VectorSignMask    __vslw( __vspltisw( 1 ),  __vspltisw( -1 ) )

NOALIAS __forceinline const __vector4 VectorIntOne()        { return __vspltisw( 1 ); }
NOALIAS __forceinline const __vector4 VectorIntMinusOne()   { return __vspltisw( -1 ); }
NOALIAS __forceinline const __vector4 VectorAbsMask()       { return __vsrw( __vspltisw( -1 ), __vspltisw( 1 ) ); }
NOALIAS __forceinline const __vector4 VectorSignMask()      { return __vslw( __vspltisw( 1 ),  __vspltisw( -1 ) ); }

#define c_VectorAbsMask  VectorAbsMask()
#define c_VectorSignMask VectorSignMask()

#define c_Vector4_SignMask  c_VectorSignMask

//#define c_Vector4_SignMask  __vslw( __vspltisw( 1 ),  __vspltisw( -1 ) )
//#define c_Vector1_SignMask  __vrlimi( __vzero(), c_Vector4_SignMask, VMX_MASK_INSERT_X, 0 )
//#define c_Vector2_SignMask  __vrlimi( __vzero(), c_Vector4_SignMask, VMX_MASK_INSERT_XY, 0 )
//#define c_Vector3_SignMask  __vrlimi( __vzero(), c_Vector4_SignMask, VMX_MASK_INSERT_XYZ, 0 )

#define c_VectorX_SignMask  __vrlimi( __vzero(), c_Vector4_SignMask, VMX_MASK_INSERT_X, 0 )
#define c_VectorY_SignMask  __vrlimi( __vzero(), c_Vector4_SignMask, VMX_MASK_INSERT_Y, 0 )
#define c_VectorZ_SignMask  __vrlimi( __vzero(), c_Vector4_SignMask, VMX_MASK_INSERT_Z, 0 )
#define c_VectorW_SignMask  __vrlimi( __vzero(), c_Vector4_SignMask, VMX_MASK_INSERT_W, 0 )

#elif defined( __ICL )

__declspec(align(16)) static const F32 c_VectorAbsMask[4]  = { c_fAbsMask,  c_fAbsMask,  c_fAbsMask,  c_fAbsMask };
__declspec(align(16)) static const F32 c_VectorSignMask[4] = { c_fSignMask, c_fSignMask, c_fSignMask, c_fSignMask };

__declspec(align(16)) static const F32 c_Vector1_SignMask[4] = { c_fSignMask, 0.0f,        0.0f,        0.0f };
__declspec(align(16)) static const F32 c_Vector2_SignMask[4] = { c_fSignMask, c_fSignMask, 0.0f,        0.0f };
__declspec(align(16)) static const F32 c_Vector3_SignMask[4] = { c_fSignMask, c_fSignMask, c_fSignMask, 0.0f };
__declspec(align(16)) static const F32 c_Vector4_SignMask[4] = { c_fSignMask, c_fSignMask, c_fSignMask, c_fSignMask };

__declspec(align(16)) static const F32 c_VectorX_SignMask[4] = { c_fSignMask, 0.0f,        0.0f,        0.0f };
__declspec(align(16)) static const F32 c_VectorY_SignMask[4] = { 0.0f,        c_fSignMask, 0.0f,        0.0f };
__declspec(align(16)) static const F32 c_VectorZ_SignMask[4] = { 0.0f,        0.0f,        c_fSignMask, 0.0f };
__declspec(align(16)) static const F32 c_VectorW_SignMask[4] = { 0.0f,        0.0f,        0.0f,        c_fSignMask };

#else

ConstV4F32( c_VectorAbsMask )    = { c_fAbsMask,  c_fAbsMask,  c_fAbsMask,  c_fAbsMask };
ConstV4F32( c_VectorSignMask )   = { c_fSignMask, c_fSignMask, c_fSignMask, c_fSignMask };

ConstV4F32( c_Vector1_SignMask ) = { c_fSignMask, 0.0f,        0.0f,        0.0f };
ConstV4F32( c_Vector2_SignMask ) = { c_fSignMask, c_fSignMask, 0.0f,        0.0f };
ConstV4F32( c_Vector3_SignMask ) = { c_fSignMask, c_fSignMask, c_fSignMask, 0.0f };
ConstV4F32( c_Vector4_SignMask ) = { c_fSignMask, c_fSignMask, c_fSignMask, c_fSignMask };

ConstV4F32( c_VectorX_SignMask ) = { c_fSignMask, 0.0f,        0.0f,        0.0f };
ConstV4F32( c_VectorY_SignMask ) = { 0.0f,        c_fSignMask, 0.0f,        0.0f };
ConstV4F32( c_VectorZ_SignMask ) = { 0.0f,        0.0f,        c_fSignMask, 0.0f };
ConstV4F32( c_VectorW_SignMask ) = { 0.0f,        0.0f,        0.0f,        c_fSignMask };

#endif


#if defined( _SPU_ ) || defined( _PPU_ )

#define c_VectorEpsilon   Maths::Const< vector float >::Epsilon()
#define c_VectorFltMin    Maths::Const< vector float >::FltMin()
#define c_VectorFltMax    Maths::Const< vector float >::FltMax()
#define c_VectorFltMinNeg Maths::Const< vector float >::NegFltMin()
#define c_VectorFltMaxNeg Maths::Const< vector float >::NegFltMax()

#else

ConstV4F32( c_VectorEpsilon ) = { FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON };

ConstV4F32( c_VectorFltMax )    = { +FLT_MAX, +FLT_MAX, +FLT_MAX, +FLT_MAX };
ConstV4F32( c_VectorFltMin )    = { +FLT_MIN, +FLT_MIN, +FLT_MIN, +FLT_MIN };
ConstV4F32( c_VectorFltMaxNeg ) = { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };
ConstV4F32( c_VectorFltMinNeg ) = { -FLT_MIN, -FLT_MIN, -FLT_MIN, -FLT_MIN };

#endif



#if defined( _SPU_ ) || defined( _PPU_ )

#define c_ScalarAbsMask     c_VectorAbsMask
#define c_ScalarSignMask    c_VectorSignMask

#define c_ScalarFltMax      c_VectorFltMax
#define c_ScalarFltMin      c_VectorFltMin
#define c_ScalarFltMaxNeg   c_VectorFltMaxNeg
#define c_ScalarFltMinNeg   c_VectorFltMinNeg

#elif defined( _VMX128_ )

#define c_ScalarAbsMask     c_VectorAbsMask
#define c_ScalarSignMask    c_VectorSignMask

#define c_ScalarFltMax      c_VectorFltMax
#define c_ScalarFltMin      c_VectorFltMin
#define c_ScalarFltMaxNeg   c_VectorFltMaxNeg
#define c_ScalarFltMinNeg   c_VectorFltMinNeg

#elif defined( _SSE_ )

#define c_ScalarAbsMask     c_VectorX_AbsMask
#define c_ScalarSignMask    c_VectorX_SignMask

ConstV4F32( c_ScalarFltMax    ) = { +FLT_MAX, 0.0f, 0.0f, 0.0f };
ConstV4F32( c_ScalarFltMin    ) = { +FLT_MIN, 0.0f, 0.0f, 0.0f };
ConstV4F32( c_ScalarFltMaxNeg ) = { -FLT_MAX, 0.0f, 0.0f, 0.0f };
ConstV4F32( c_ScalarFltMinNeg ) = { -FLT_MIN, 0.0f, 0.0f, 0.0f };

#else

static const F32 c_ScalarAbsMask  = c_fAbsMask;
static const F32 c_ScalarSignMask = c_fSignMask;

static const F32 c_ScalarFltMax    = +FLT_MAX;
static const F32 c_ScalarFltMin    = +FLT_MIN;
static const F32 c_ScalarFltMaxNeg = -FLT_MAX;
static const F32 c_ScalarFltMinNeg = -FLT_MIN;

#endif



ConstV4F32( c_VectorP1M1ZZ ) = { +1.0f, -1.0f, 0.0f, 0.0f }; // Plus 1, Minus 1, Zero, Zero




namespace Maths
{

#if defined( _VMX128_ ) || defined( _SSE_ )

//
// SIMD Common Helper Functions : Identical implementations on VMX & SSE
//

/// Vector Mask Generator Template
template < bool x, bool y, bool z, bool w >
NOALIAS __forceinline const V128 SIMD_Mask();

/// Vector Masking Function Template
template < bool x, bool y, bool z, bool w >
NOALIAS __forceinline const V128 SIMD_Mask( const V128 PCREF src );

/// SIMD Scalar/Vector Zero
NOALIAS __forceinline const V128 SIMD_Zero();

/// SIMD Scalar F32 Constructor
NOALIAS __forceinline const V128 SIMD_Scalar( const F32& f );

/// SIMD Scalar F32 Constructor
NOALIAS __forceinline const V128 SIMD_Scalar( F32 const * ptr );

/// SIMD Scalar S32 Constructor
NOALIAS __forceinline const V128 SIMD_Scalar( const S32& i );

/// SIMD Scalar U32 Constructor
NOALIAS __forceinline const V128 SIMD_Scalar( const U32& i );

/// SIMD Scalar Vector2 X Constructor
NOALIAS __forceinline const V128 SIMD_Scalar_Vector2X( const V128 PCREF x );

/// SIMD Scalar Vector2 Y Constructor
NOALIAS __forceinline const V128 SIMD_Scalar_Vector2Y( const V128 PCREF y );

/// SIMD Scalar Vector3 X Constructor
NOALIAS __forceinline const V128 SIMD_Scalar_Vector3X( const V128 PCREF x );

/// SIMD Scalar Vector3 Y Constructor
NOALIAS __forceinline const V128 SIMD_Scalar_Vector3Y( const V128 PCREF y );

/// SIMD Scalar Vector3 Z Constructor
NOALIAS __forceinline const V128 SIMD_Scalar_Vector3Z( const V128 PCREF z );

/// SIMD Scalar Vector4 X Constructor
NOALIAS __forceinline const V128 SIMD_Scalar_Vector4X( const V128 PCREF x );

/// SIMD Scalar Vector4 Y Constructor
NOALIAS __forceinline const V128 SIMD_Scalar_Vector4Y( const V128 PCREF y );

/// SIMD Scalar Vector4 Z Constructor
NOALIAS __forceinline const V128 SIMD_Scalar_Vector4Z( const V128 PCREF z );

/// SIMD Scalar Vector4 W Constructor
NOALIAS __forceinline const V128 SIMD_Scalar_Vector4W( const V128 PCREF w );

/// SIMD Scalar VectorX Constructor
NOALIAS __forceinline const V128 SIMD_Scalar_VectorX( const V128 PCREF x );

/// SIMD Scalar VectorY Constructor
NOALIAS __forceinline const V128 SIMD_Scalar_VectorY( const V128 PCREF y );

/// SIMD Scalar VectorZ Constructor
NOALIAS __forceinline const V128 SIMD_Scalar_VectorZ( const V128 PCREF z );

/// SIMD Scalar VectorW Constructor
NOALIAS __forceinline const V128 SIMD_Scalar_VectorW( const V128 PCREF w );

/// SIMD Scalar Broadcast to XYZW
NOALIAS __forceinline const V128 SIMD_ScalarBroadcastToXYZW( const V128 PCREF src );

/// SIMD Scalar Cast to F32
NOALIAS __forceinline const F32 SIMD_ScalarToF32( const V128 PCREF src );

/// SIMD Scalar Cast to S32
NOALIAS __forceinline const S32 SIMD_ScalarToS32( const V128 PCREF src );

/// SIMD Scalar Abs
NOALIAS __forceinline const V128 SIMD_ScalarAbs( const V128 PCREF src );

/// SIMD Scalar Sqrt
NOALIAS __forceinline const V128 SIMD_ScalarSqrt( const V128 PCREF src );

/// SIMD Scalar Reciprocal
NOALIAS __forceinline const V128 SIMD_ScalarReciprocal( const V128 PCREF src );

/// SIMD Scalar Reciprocal Sqrt
NOALIAS __forceinline const V128 SIMD_ScalarReciprocalSqrt( const V128 PCREF src );

/// SIMD Scalar Negate
NOALIAS __forceinline const V128 SIMD_ScalarNegate( const V128 PCREF src );

/// SIMD Scalar Add
NOALIAS __forceinline const V128 SIMD_ScalarAdd( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Subtract
NOALIAS __forceinline const V128 SIMD_ScalarSubtract( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Multiply
NOALIAS __forceinline const V128 SIMD_ScalarMultiply( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Divide
NOALIAS __forceinline const V128 SIMD_ScalarDivide( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Min
NOALIAS __forceinline const V128 SIMD_ScalarMin( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Max
NOALIAS __forceinline const V128 SIMD_ScalarMax( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Compare EQ
NOALIAS __forceinline const S32 SIMD_ScalarCompareEQ( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Compare NE
NOALIAS __forceinline const S32 SIMD_ScalarCompareNE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Compare LE
NOALIAS __forceinline const S32 SIMD_ScalarCompareLE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Compare LT
NOALIAS __forceinline const S32 SIMD_ScalarCompareLT( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Compare GE
NOALIAS __forceinline const S32 SIMD_ScalarCompareGE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Compare GT
NOALIAS __forceinline const S32 SIMD_ScalarCompareGT( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Comparison Selector EQ
NOALIAS __forceinline const Scalar_Bool SIMD_ScalarSelectorEQ( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Comparison Selector NE
NOALIAS __forceinline const Scalar_Bool SIMD_ScalarSelectorNE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Comparison Selector LE
NOALIAS __forceinline const Scalar_Bool SIMD_ScalarSelectorLE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Comparison Selector LT
NOALIAS __forceinline const Scalar_Bool SIMD_ScalarSelectorLT( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Comparison Selector GE
NOALIAS __forceinline const Scalar_Bool SIMD_ScalarSelectorGE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Comparison Selector GT
NOALIAS __forceinline const Scalar_Bool SIMD_ScalarSelectorGT( const V128 PCREF a, const V128 PCREF b );

/// SIMD Scalar Selector Test : if ( selector == 0xFFFFFFFF ) returns 1 else returns 0
NOALIAS __forceinline const U32 SIMD_TestScalarSelector( const V128 PCREF src );


/// SIMD Scalar FP Round To Integer : Towards minus infinity
//const V128 SIMD_Scalar_RoundToIntMinusInf( const V128 PCREF src );

/// SIMD Scalar FP Round To Integer : Towards plus infinity
//const V128 SIMD_Scalar_RoundToIntPlusInf( const V128 PCREF src );

/// SIMD Scalar FP Round To Integer : Towards nearest
//const V128 SIMD_Scalar_RoundToIntNearest( const V128 PCREF src );

/// SIMD Scalar FP Round To Integer : Towards zero
//const V128 SIMD_Scalar_RoundToIntZero( const V128 PCREF src );













/// SIMD VectorX Constructor from float
NOALIAS __forceinline const V128 SIMD_VectorX_Float( const F32& f );

/// SIMD VectorX Constructor from Scalar
NOALIAS __forceinline const V128 SIMD_VectorX_Scalar( const V128 PCREF f );

/// SIMD VectorX Constructor from VectorY
NOALIAS __forceinline const V128 SIMD_VectorX_VectorY( const V128 PCREF y );

/// SIMD VectorX Constructor from VectorZ
NOALIAS __forceinline const V128 SIMD_VectorX_VectorZ( const V128 PCREF z );

/// SIMD VectorX Constructor from VectorW
NOALIAS __forceinline const V128 SIMD_VectorX_VectorW( const V128 PCREF w );

/// SIMD VectorX Constructor from Vector3
NOALIAS __forceinline const V128 SIMD_VectorX_Vector3( const V128 PCREF src );



/// SIMD VectorY Constructor from float
NOALIAS __forceinline const V128 SIMD_VectorY_Float( const F32& f );

/// SIMD VectorY Constructor from Scalar
NOALIAS __forceinline const V128 SIMD_VectorY_Scalar( const V128 PCREF f );

/// SIMD VectorY Constructor from VectorX
NOALIAS __forceinline const V128 SIMD_VectorY_VectorX( const V128 PCREF x );

/// SIMD VectorY Constructor from VectorZ
NOALIAS __forceinline const V128 SIMD_VectorY_VectorZ( const V128 PCREF z );

/// SIMD VectorY Constructor from VectorW
NOALIAS __forceinline const V128 SIMD_VectorY_VectorW( const V128 PCREF w );

/// SIMD VectorY Constructor from Vector3
NOALIAS __forceinline const V128 SIMD_VectorY_Vector3( const V128 PCREF src );


/// SIMD VectorZ Constructor from float
NOALIAS __forceinline const V128 SIMD_VectorZ_Float( const F32& f );

/// SIMD VectorZ Constructor from Scalar
NOALIAS __forceinline const V128 SIMD_VectorZ_Scalar( const V128 PCREF f );

/// SIMD VectorZ Constructor from VectorX
NOALIAS __forceinline const V128 SIMD_VectorZ_VectorX( const V128 PCREF x );

/// SIMD VectorZ Constructor from VectorY
NOALIAS __forceinline const V128 SIMD_VectorZ_VectorY( const V128 PCREF y );

/// SIMD VectorZ Constructor from VectorW
NOALIAS __forceinline const V128 SIMD_VectorZ_VectorW( const V128 PCREF w );

/// SIMD VectorZ Constructor from Vector3
NOALIAS __forceinline const V128 SIMD_VectorZ_Vector3( const V128 PCREF src );



/// SIMD VectorW Constructor from float
NOALIAS __forceinline const V128 SIMD_VectorW_Float( const F32& f );

/// SIMD VectorW Constructor from Scalar
NOALIAS __forceinline const V128 SIMD_VectorW_Scalar( const V128 PCREF f );

/// SIMD VectorW Constructor from VectorX
NOALIAS __forceinline const V128 SIMD_VectorW_VectorX( const V128 PCREF x );

/// SIMD VectorW Constructor from VectorY
NOALIAS __forceinline const V128 SIMD_VectorW_VectorY( const V128 PCREF y );

/// SIMD VectorW Constructor from VectorZ
NOALIAS __forceinline const V128 SIMD_VectorW_VectorZ( const V128 PCREF z );

/// SIMD VectorW Constructor from Vector4
NOALIAS __forceinline const V128 SIMD_VectorW_Vector4( const V128 PCREF src );




/// SIMD Vector2 Constructor
NOALIAS __forceinline const V128 SIMD_Vector2( const F32& x, const F32& y );

/// SIMD Vector3 Constructor
NOALIAS __forceinline const V128 SIMD_Vector3( const F32& x, const F32& y, const F32& z );

/// SIMD Vector4 Constructor
NOALIAS __forceinline const V128 SIMD_Vector4( const F32& x, const F32& y, const F32& z, const F32& w );

/// SIMD Vector2 Constructor: Source data all in X components!
NOALIAS __forceinline const V128 SIMD_Vector2( const V128 PCREF x, const V128 PCREF y );

/// SIMD Vector3 Constructor: Source data all in X components!
NOALIAS __forceinline const V128 SIMD_Vector3( const V128 PCREF x, const V128 PCREF y, const V128 PCREF z );

/// SIMD Vector4 Constructor: Source data all in X components!
NOALIAS __forceinline const V128 SIMD_Vector4( const V128 PCREF x, const V128 PCREF y, const V128 PCREF z, const V128 PCREF w );

/// SIMD Vector2 Scalar Constructor: Source data in HF32 format
NOALIAS __forceinline const V128 SIMD_Vector2_Scalar( const V128 PCREF xy );

/// SIMD Vector3 Scalar Constructor: Source data in HF32 format
NOALIAS __forceinline const V128 SIMD_Vector3_Scalar( const V128 PCREF xyz );

/// SIMD Vector4 Scalar Constructor: Source data in HF32 format
NOALIAS __forceinline const V128 SIMD_Vector4_Scalar( const V128 PCREF xyzw );

/// SIMD Vector3 Constructor from Vector4
NOALIAS __forceinline const V128 SIMD_Vector3_Vector4( const V128 PCREF vec4 );

/// SIMD Vector4 Constructor from Vector3 & VectorW
NOALIAS __forceinline const V128 SIMD_Vector4_Vector3W( const V128 PCREF vec3, const V128 PCREF vecW );

/// SIMD Vector4 Constructor from VectorW
NOALIAS __forceinline const V128 SIMD_Vector4_VectorW( const V128 PCREF vecW );





/// SIMD Vector Truncate To Int
//const V4_S32 SIMD_TruncateToInt( const V128 PCREF a );

/// SIMD Vector Unaligned Load
NOALIAS __forceinline const V128 SIMD_LoadUnaligned( const F32* ptr );

/// SIMD Vector Unaligned Store
NOALIAS __forceinline void SIMD_StoreUnaligned( const V128 PCREF a, F32* ptr );

/// SIMD Select: Selects fields from the inputs based on the selector mask: result = ( selector ? a : b )
NOALIAS __forceinline const V128 SIMD_Select( const V128 PCREF selector, const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector Permute
template < int ax, int ay, int az, int aw >
NOALIAS __forceinline const V128 SIMD_Permute( const V128 PCREF a );

/// SIMD Vector Permute : 2 Sources
template < int ax, int ay, int bz, int bw >
NOALIAS __forceinline const V128 SIMD_Permute( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector Permute : 3 Sources
template < int ax, int by, int cz >
NOALIAS __forceinline const V128 SIMD_Permute( const V128 PCREF a, const V128 PCREF b, const V128 PCREF c );

/// SIMD Vector Permute : 3 Sources
template < int ax, int by, int cz, int cw >
NOALIAS __forceinline const V128 SIMD_Permute( const V128 PCREF a, const V128 PCREF b, const V128 PCREF c );

/// SIMD Vector Permute : 4 Sources
template < int ax, int by, int cz, int dw >
NOALIAS __forceinline const V128 SIMD_Permute( const V128 PCREF a, const V128 PCREF b, const V128 PCREF c, const V128 PCREF d );

/// SIMD Vector Splat2: Replicate component I to XY - Z&W unchanged
template < int I >
NOALIAS __forceinline const V128 SIMD_Splat2( const V128 PCREF a );

/// SIMD Vector Splat3: Replicate component I to XYZ - W unchanged
template < int I >
NOALIAS __forceinline const V128 SIMD_Splat3( const V128 PCREF a );



/// SIMD Vector Set X : Returns src with X set to scalar HF32
NOALIAS __forceinline const V128 SIMD_SetX( const V128 PCREF src, const V128 PCREF scalar );

/// SIMD Vector Set Y : Returns src with Y set to scalar HF32
NOALIAS __forceinline const V128 SIMD_SetY( const V128 PCREF src, const V128 PCREF scalar );

/// SIMD Vector Set Z : Returns src with Z set to scalar HF32
NOALIAS __forceinline const V128 SIMD_SetZ( const V128 PCREF src, const V128 PCREF scalar );

/// SIMD Vector Set W : Returns src with W set to scalar HF32
NOALIAS __forceinline const V128 SIMD_SetW( const V128 PCREF src, const V128 PCREF scalar );

/// SIMD Vector Bitwise NOT
NOALIAS __forceinline const V128 SIMD_NOT( const V128 PCREF a );

/// SIMD Vector Bitwise AND
NOALIAS __forceinline const V128 SIMD_AND( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector Bitwise ANDNOT: a & ~b
NOALIAS __forceinline const V128 SIMD_ANDNOT( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector Bitwise OR
NOALIAS __forceinline const V128 SIMD_OR( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector Bitwise XOR
NOALIAS __forceinline const V128 SIMD_XOR( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Abs
NOALIAS __forceinline const V128 SIMD_Abs( const V128 PCREF src );

/// SIMD Vector FP Add
NOALIAS __forceinline const V128 SIMD_Add( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Subtract
NOALIAS __forceinline const V128 SIMD_Subtract( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Multiply
NOALIAS __forceinline const V128 SIMD_Multiply( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Multiply Add : A * B + C
NOALIAS __forceinline const V128 SIMD_MultiplyAdd( const V128 PCREF a, const V128 PCREF b, const V128 PCREF c );

/// SIMD Vector FP Subtract Multiply : A - B * C
NOALIAS __forceinline const V128 SIMD_SubtractMultiply( const V128 PCREF a, const V128 PCREF b, const V128 PCREF c );

/// SIMD Vector FP Dot Product 2 - Result is HF32
NOALIAS __forceinline const V128 SIMD_DotProduct2( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Dot Product 3 - Result is HF32
NOALIAS __forceinline const V128 SIMD_DotProduct3( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Dot Product 4 - Result is HF32
NOALIAS __forceinline const V128 SIMD_DotProduct4( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Sum of Elements 2 - Result is HF32
NOALIAS __forceinline const V128 SIMD_Sum2( const V128 PCREF a );

/// SIMD Vector FP Sum of Elements 3 - Result is HF32
NOALIAS __forceinline const V128 SIMD_Sum3( const V128 PCREF a );

/// SIMD Vector FP Sum of Elements 4 - Result is HF32
NOALIAS __forceinline const V128 SIMD_Sum4( const V128 PCREF a );

/// SIMD Vector FP Cross Product
NOALIAS __forceinline const V128 SIMD_CrossProduct( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Min
NOALIAS __forceinline const V128 SIMD_Min( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Max
NOALIAS __forceinline const V128 SIMD_Max( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Reciprocal X
NOALIAS __forceinline const V128 SIMD_Reciprocal1( const V128 PCREF a );

/// SIMD Vector FP Reciprocal XY
NOALIAS __forceinline const V128 SIMD_Reciprocal2( const V128 PCREF a );

/// SIMD Vector FP Reciprocal XYZ
NOALIAS __forceinline const V128 SIMD_Reciprocal3( const V128 PCREF a );

/// SIMD Vector FP Reciprocal XYZW
NOALIAS __forceinline const V128 SIMD_Reciprocal4( const V128 PCREF a );

/// SIMD Vector FP Reciprocal Square Root X
NOALIAS __forceinline const V128 SIMD_ReciprocalSqrt1( const V128 PCREF a );

/// SIMD Vector FP Reciprocal Square Root XY
NOALIAS __forceinline const V128 SIMD_ReciprocalSqrt2( const V128 PCREF a );

/// SIMD Vector FP Reciprocal Square Root XYZ
NOALIAS __forceinline const V128 SIMD_ReciprocalSqrt3( const V128 PCREF a );

/// SIMD Vector FP Reciprocal Square Root XYZW
NOALIAS __forceinline const V128 SIMD_ReciprocalSqrt4( const V128 PCREF a );

/// SIMD Vector FP Square Root X
NOALIAS __forceinline const V128 SIMD_Sqrt1( const V128 PCREF a );

/// SIMD Vector FP Square Root XY
NOALIAS __forceinline const V128 SIMD_Sqrt2( const V128 PCREF a );

/// SIMD Vector FP Square Root XYZ
NOALIAS __forceinline const V128 SIMD_Sqrt3( const V128 PCREF a );

/// SIMD Vector FP Square Root XYZW
NOALIAS __forceinline const V128 SIMD_Sqrt4( const V128 PCREF a );

/// SIMD Vector FP Normalise XY
NOALIAS __forceinline const V128 SIMD_Normalise2( const V128 PCREF a );

/// SIMD Vector FP Normalise XYZ
NOALIAS __forceinline const V128 SIMD_Normalise3( const V128 PCREF a );

/// SIMD Vector FP Normalise XYZW
NOALIAS __forceinline const V128 SIMD_Normalise4( const V128 PCREF a );


/// SIMD Vector FP Selector Compare ==
NOALIAS __forceinline const V128 SIMD_Vector_Selector_EQ( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Selector Compare !=
NOALIAS __forceinline const V128 SIMD_Vector_Selector_NE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Selector Compare >=
NOALIAS __forceinline const V128 SIMD_Vector_Selector_GE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Selector Compare >
NOALIAS __forceinline const V128 SIMD_Vector_Selector_GT( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Selector Compare <=
NOALIAS __forceinline const V128 SIMD_Vector_Selector_LE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector FP Selector Compare <
NOALIAS __forceinline const V128 SIMD_Vector_Selector_LT( const V128 PCREF a, const V128 PCREF b );


/// SIMD Vector3 FP Round To Integer : Towards minus infinity
//const V128 SIMD_Vector3_RoundToIntMinusInf( const V128 PCREF src );

/// SIMD Vector3 FP Round To Integer : Towards plus infinity
//const V128 SIMD_Vector3_RoundToIntPlusInf( const V128 PCREF src );

/// SIMD Vector3 FP Round To Integer : Towards nearest
//const V128 SIMD_Vector3_RoundToIntNearest( const V128 PCREF src );

/// SIMD Vector3 FP Round To Integer : Towards zero
//const V128 SIMD_Vector3_RoundToIntZero( const V128 PCREF src );



/// SIMD Vector3 FP Compare == ( true iff true for ALL 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_EQ( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare != ( true iff true for ALL 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_NE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare >= ( true iff true for ALL 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_GE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare > ( true iff true for ALL 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_GT( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare <= ( true iff true for ALL 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_LE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare < ( true iff true for ALL 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_LT( const V128 PCREF a, const V128 PCREF b );


/// SIMD Vector3 FP Compare == ( true if true for ANY of 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_EQ( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare != ( true if true for ANY of 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_NE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare >= ( true if true for ANY of 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_GE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare > ( true if true for ANY of 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_GT( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare <= ( true if true for ANY of 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_LE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare < ( true if true for ANY of 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_LT( const V128 PCREF a, const V128 PCREF b );



/// SIMD Vector3 FP Round To Integer : Towards minus infinity
//const V128 SIMD_Vector4_RoundToIntMinusInf( const V128 PCREF src );

/// SIMD Vector3 FP Round To Integer : Towards plus infinity
//const V128 SIMD_Vector4_RoundToIntPlusInf( const V128 PCREF src );

/// SIMD Vector3 FP Round To Integer : Towards nearest
//const V128 SIMD_Vector4_RoundToIntNearest( const V128 PCREF src );

/// SIMD Vector3 FP Round To Integer : Towards zero
//const V128 SIMD_Vector4_RoundToIntZero( const V128 PCREF src );



/// SIMD Vector3 FP Compare == ( true iff true for ALL 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_EQ( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare != ( true iff true for ALL 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_NE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare >= ( true iff true for ALL 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_GE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare > ( true iff true for ALL 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_GT( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare <= ( true iff true for ALL 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_LE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare < ( true iff true for ALL 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_LT( const V128 PCREF a, const V128 PCREF b );


/// SIMD Vector3 FP Compare == ( true if true for ANY of 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_EQ( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare != ( true if true for ANY of 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_NE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare >= ( true if true for ANY of 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_GE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare > ( true if true for ANY of 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_GT( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare <= ( true if true for ANY of 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_LE( const V128 PCREF a, const V128 PCREF b );

/// SIMD Vector3 FP Compare < ( true if true for ANY of 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_LT( const V128 PCREF a, const V128 PCREF b );

#endif

/////////////////////////////////////////////////////////////////////
//
//	SIMD fallbackS
//

// used for non optimized float fallback where wanting to keep one codepath for minimum development time
// as all important platforms use _SIMD_ this is fine :)
// [TODO: Move to simd_default.h - analogous to simd_ppu simd_spu etc. ]

#ifndef _SIMD_
inline
const V128	SIMD_Multiply(const V128 PCREF a, V128 b) 
{
	V128	tmp;
	tmp.m_X = a.m_X * b.m_X; tmp.m_Y =  a.m_Y*b.m_Y; tmp.m_Z =  a.m_Z * b.m_Z; tmp.m_W =  a.m_W * b.m_W;
	return	tmp;
}

#endif



/////////////////////////////////////////////////////////////////////
//
// Wrappers for SIMD functions available on all platforms.
//

#ifdef _SIMD_
NOALIAS __forceinline  void
SIMD_Transpose(
	V128&	dstAX,	
	V128&	dstAY,
	V128&	dstAZ,
	V128&	dstAW, 
	const V128 PCREF srcAX,
	const V128 PCREF srcAY,
	const V128 PCREF srcAZ,
	const V128 PCREF srcAW
		);

template < int I >
NOALIAS __forceinline  const V128 SIMD_Splat4( const V128 PCREF a );

NOALIAS __forceinline  const V128 SIMD_MergeHigh( const V128 PCREF a, const V128 PCREF b );

NOALIAS __forceinline  const V128 SIMD_MergeLow( const V128 PCREF a, const V128 PCREF b );

#endif




#if defined( _VMX128_ )

//
// VMX & VMX128 SIMD Scalar & Vector Maths Functions
//

//const V128 SIMD_RotateLeft1( const V128 src );
//const V128 SIMD_RotateLeft2( const V128 src );
//const V128 SIMD_RotateLeft3( const V128 src );

//const V128 SIMD_RotateRight1( const V128 src );
//const V128 SIMD_RotateRight2( const V128 src );
//const V128 SIMD_RotateRight3( const V128 src );

//const V128 SIMD_Rotate_X_to_Y( const V128 src );
//const V128 SIMD_Rotate_X_to_Z( const V128 src );
//const V128 SIMD_Rotate_X_to_W( const V128 src );

//const V128 SIMD_Rotate_Y_to_X( const V128 src );
//const V128 SIMD_Rotate_Y_to_Z( const V128 src );
//const V128 SIMD_Rotate_Y_to_W( const V128 src );

//const V128 SIMD_Rotate_Z_to_X( const V128 src );
//const V128 SIMD_Rotate_Z_to_Y( const V128 src );
//const V128 SIMD_Rotate_Z_to_W( const V128 src );

//const V128 SIMD_Rotate_W_to_X( const V128 src );
//const V128 SIMD_Rotate_W_to_Y( const V128 src );
//const V128 SIMD_Rotate_W_to_Z( const V128 src );

#endif // #if defined( _VMX128_ )






#if defined( _SSE_ )

//
// SSE & SSE2 SIMD Scalar & Vector Maths Functions
//

// Replicate X into XYZW
NOALIAS __forceinline const V128 SSE_ReplicateX( const V128 PCREF src );

// Replicate Y into XYZW
NOALIAS __forceinline const V128 SSE_ReplicateY( const V128 PCREF src );

// Replicate Z into XYZW
NOALIAS __forceinline const V128 SSE_ReplicateZ( const V128 PCREF src );

// Replicate W into XYZW
NOALIAS __forceinline const V128 SSE_ReplicateW( const V128 PCREF src );

/// Replicate X into XYZ, Preserve W
NOALIAS __forceinline const V128 SSE_ReplicateX_XYZ( const V128 PCREF src );

/// Replicate Y into XYZ, Preserve W
NOALIAS __forceinline const V128 SSE_ReplicateY_XYZ( const V128 PCREF src );

/// Replicate Z into XYZ, Preserve W
NOALIAS __forceinline const V128 SSE_ReplicateZ_XYZ( const V128 PCREF src );

/// Replicate X into XY, Preserve ZW
NOALIAS __forceinline const V128 SSE_ReplicateX_XY( const V128 PCREF src );

/// Replicate Y into XY, Preserve ZW
NOALIAS __forceinline const V128 SSE_ReplicateY_XY( const V128 PCREF src );

// Approx Reciprocal : 11 bit accuracy
NOALIAS __forceinline const V128 SSE_ApproxRcpSS( const V128 PCREF x );         // Scalar: X only
NOALIAS __forceinline const V128 SSE_ApproxRcpPS( const V128 PCREF xyzw );      // Packed: XYZW

// Approx Reciprocal + Newton-Raphson Iteration : 22 bit accuracy
NOALIAS __forceinline const V128 SSE_RcpSS( const V128 PCREF x );               // Scalar: X only
NOALIAS __forceinline const V128 SSE_RcpPS_XY( const V128 PCREF xyzw );         // Packed: XY
NOALIAS __forceinline const V128 SSE_RcpPS_XYZ( const V128 PCREF xyzw );        // Packed: XYZ
NOALIAS __forceinline const V128 SSE_RcpPS_XYZW( const V128 PCREF xyzw );       // Packed: XYZW

// Approx Reciprocal Sqrt : 11 bit accuracy
NOALIAS __forceinline const V128 SSE_ApproxRcpSqrtSS( const V128 PCREF x );     // Scalar: X only
NOALIAS __forceinline const V128 SSE_ApproxRcpSqrtPS( const V128 PCREF xyzw );  // Packed: XYZW

// Approx Reciprocal Sqrt + Newton-Raphson Iteration : 22 bit accuracy
NOALIAS __forceinline const V128 SSE_RcpSqrtSS( const V128 PCREF x );           // Scalar: X only
NOALIAS __forceinline const V128 SSE_RcpSqrtPS( const V128 PCREF xyzw );        // Packed: XYZW

#endif // #if defined( _SSE_ )



}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )



