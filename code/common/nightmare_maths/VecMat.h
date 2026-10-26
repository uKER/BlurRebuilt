#pragma once

// 
// Hardware Vector Classes
// Designed for optimal hardware acceleration
// 

//
// Notes on implementation
//
// 1) Unions cannot be used due to the crippling impact they have on performance.
//    All compilers aggressively store back to memory after modification to a
//    union member. A union member will not be kept in a temporary register for
//    subsequent reuse, instead it will have to be reloaded - in case it is
//    modified via a different union member.
//
// 2) Arrays of elements cannot be used, the elements must be stored individually.
//    This is due to an issue with the MSVC compiler: on initialisation, only the
//    first array element will be initialised by the proper constructor. The rest
//    of the array elements are copied from the first in dword chunks. This causes
//    code bloat, load-hit-store stalls and forces the object into memory even if
//    it's a temporary and could have stayed in a register for it's entire lifetime.
//
// 3) There's more... (when I can remember them all!)
//
//
//
//



//#pragma warning( once : 4714 ) // Enable warning once only: __forceinline failure
//#pragma warning( default : 4714 ) // Enable warning: __forceinline failure
#pragma warning( error : 4714 ) // Set warning to error: __forceinline failure

#ifdef WIN32
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE inline
#endif

class BinaryFileBlock;

void Matrix44InverseCramersRuleFP( const F32* mat, F32* dst );

//
// Float & SIMD Initialisation & Exception Control
//

//void FP_Init();
//void FP_UnmaskExceptions();
//void FP_MaskExceptions();


typedef enum { VX = 0, VY = 1, VZ = 2, VW = 3 } VecElement;
typedef enum { MX = 0, MY = 1, MZ = 2, MW = 3 } MatAxis;
typedef enum { QX = 0, QY = 1, QZ = 2, QA = 3 } QuatElement;
typedef enum { SX = 0, SY = 1, SZ = 2, SR = 3 } SphereElement;

typedef enum
{
    E_SignBits0 =     0x0,
    E_SignBitsX =     0x1,
    E_SignBitsY =     0x2,
    E_SignBitsZ =     0x4,
    E_SignBitsW =     0x8,
    E_SignBitsXY =    ( E_SignBitsX | E_SignBitsY ),
    E_SignBitsXZ =    ( E_SignBitsX | E_SignBitsZ ),
    E_SignBitsXW =    ( E_SignBitsX | E_SignBitsW ),
    E_SignBitsYZ =    ( E_SignBitsY | E_SignBitsZ ),
    E_SignBitsYW =    ( E_SignBitsY | E_SignBitsW ),
    E_SignBitsZW =    ( E_SignBitsZ | E_SignBitsW ),
    E_SignBitsXYZ =   ( E_SignBitsX | E_SignBitsY | E_SignBitsZ ),
    E_SignBitsXYW =   ( E_SignBitsX | E_SignBitsY | E_SignBitsW ),
    E_SignBitsXZW =   ( E_SignBitsX | E_SignBitsZ | E_SignBitsW ),
    E_SignBitsYZW =   ( E_SignBitsY | E_SignBitsZ | E_SignBitsW ),
    E_SignBitsXYZW =  ( E_SignBitsX | E_SignBitsY | E_SignBitsZ | E_SignBitsW )
} E_SignBits;

namespace VecMat
{
    // Forward declarations of all classes
    class HVecX;
    class HVecY;
    class HVecZ;
    class HVecW;
    class HVec2;
    class HVec3;
    class HVec4;
    class HMat33;
    class HMat43;
    class HMat44;
    class C_Quaternion;
    
    //
    // Constants
    //
    
    #ifdef _SSE_
    #define ConstV4F32(x) const V128 x
    #define ConstV3F32(x) const V128 x
    #define ConstV2F32(x) const V128 x
    #else
    #define ConstV4F32(x) const F32 x[4]
    #define ConstV3F32(x) const F32 x[3]
    #define ConstV2F32(x) const F32 x[2]
    #endif
    
    static const U32 c_iSet      = 0xFFFFFFFF;
    static const U32 c_iClear    = 0x00000000;
    static const U32 c_iAbsMask  = 0x7FFFFFFF;
    static const U32 c_iSignMask = 0x80000000;
    
    static const F32 c_fSet      = (F32&)c_iSet;
    static const F32 c_fClear    = (F32&)c_iClear;
    static const F32 c_fAbsMask  = (F32&)c_iAbsMask;
    static const F32 c_fSignMask = (F32&)c_iSignMask;
    
    #ifdef _SSE_
    
    #define c_Zero _mm_setzero_ps()
    
    #else
    
    static ConstV4F32( c_Zero ) = { 0.0f, 0.0f, 0.0f, 0.0f };
    
    #endif
    
    //                               { X         Y         Z         W        }
    static ConstV4F32( c_MaskX )   = { c_fSet,   c_fClear, c_fClear, c_fClear };
    static ConstV4F32( c_MaskY )   = { c_fClear, c_fSet,   c_fClear, c_fClear };
    static ConstV4F32( c_MaskZ )   = { c_fClear, c_fClear, c_fSet,   c_fClear };
    static ConstV4F32( c_MaskW )   = { c_fClear, c_fClear, c_fClear, c_fSet   };
                                                                       
    static ConstV4F32( c_MaskXY )  = { c_fSet,   c_fSet,   c_fClear, c_fClear };
    static ConstV4F32( c_MaskXZ )  = { c_fSet,   c_fClear, c_fSet,   c_fClear };
    static ConstV4F32( c_MaskXW )  = { c_fSet,   c_fClear, c_fClear, c_fSet   };
    static ConstV4F32( c_MaskYZ )  = { c_fClear, c_fSet,   c_fSet,   c_fClear };
    static ConstV4F32( c_MaskYW )  = { c_fClear, c_fSet,   c_fClear, c_fSet   };
    static ConstV4F32( c_MaskZW )  = { c_fClear, c_fClear, c_fSet,   c_fSet   };
                                                                       
    static ConstV4F32( c_MaskYZW ) = { c_fClear, c_fSet,   c_fSet,   c_fSet   };
    static ConstV4F32( c_MaskXZW ) = { c_fSet,   c_fClear, c_fSet,   c_fSet   };
    static ConstV4F32( c_MaskXYW ) = { c_fSet,   c_fSet,   c_fClear, c_fSet   };
    static ConstV4F32( c_MaskXYZ ) = { c_fSet,   c_fSet,   c_fSet,   c_fClear };
    
    static ConstV4F32( c_UnitX )  = { 1.0f, 0.0f, 0.0f, 0.0f };
    static ConstV4F32( c_UnitY )  = { 0.0f, 1.0f, 0.0f, 0.0f };
    static ConstV4F32( c_UnitZ )  = { 0.0f, 0.0f, 1.0f, 0.0f };
    static ConstV4F32( c_UnitW )  = { 0.0f, 0.0f, 0.0f, 1.0f };
    //static ConstV4F32( c_Zero )   = { 0.0f, 0.0f, 0.0f, 0.0f };
    static ConstV4F32( c_Half )   = { 0.5f, 0.5f, 0.5f, 0.5f };
    static ConstV4F32( c_One )    = { 1.0f, 1.0f, 1.0f, 1.0f };
    static ConstV4F32( c_Three )  = { 3.0f, 3.0f, 3.0f, 3.0f };
    
    static ConstV4F32( c_AbsMask )  = { c_fAbsMask,  c_fAbsMask,  c_fAbsMask,  c_fAbsMask };
    static ConstV4F32( c_SignMask ) = { c_fSignMask, c_fSignMask, c_fSignMask, c_fSignMask };
    
    static ConstV4F32( c_Epsilon ) = { FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON };
    
    static ConstV4F32( c_FltMaxPos ) = { +FLT_MAX, +FLT_MAX, +FLT_MAX, +FLT_MAX };
    static ConstV4F32( c_FltMinPos ) = { +FLT_MIN, +FLT_MIN, +FLT_MIN, +FLT_MIN };
    static ConstV4F32( c_FltMaxNeg ) = { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };
    static ConstV4F32( c_FltMinNeg ) = { -FLT_MIN, -FLT_MIN, -FLT_MIN, -FLT_MIN };
    
    static ConstV4F32( c_P1M1ZZ ) = { +1, -1, 0, 0 }; // Plus 1, Minus 1, Zero, Zero
    
    //
    // SSE & SSE2 Helper Functions
    //
    
    #ifdef _SSE_
    
    // Shuffle : 1 parameter
    template < int Ax, int Ay, int Az, int Aw >
    FORCE_INLINE const V128 Shuffle( const V128 A )
    {
        return _mm_shuffle_ps( A, A, _MM_SHUFFLE( Aw, Az, Ay, Ax ) );
    }
    
    // Shuffle : 2 parameters
    template < int Ax, int Ay, int Bz, int Bw >
    FORCE_INLINE const V128 Shuffle( const V128 A, const V128 B )
    {
        return _mm_shuffle_ps( A, B, _MM_SHUFFLE( Bw, Bz, Ay, Ax ) );
    }
    
    // Shuffle : 3 parameters
    template < int Ax, int By, int Cz, int Cw >
    FORCE_INLINE const V128 Shuffle( const V128 A, const V128 B, const V128 C )
    {
        return Shuffle< 0, 2, Cz, Cw >( Shuffle< Ax, Ax, By, By >( A, B ), C );
    }
    
    // Shuffle : 4 parameters
    template < int Ax, int By, int Cz, int Dw >
    FORCE_INLINE const V128 Shuffle( const V128 A, const V128 B, const V128 C, const V128& D )
    {
        return Shuffle< 0, 2, 0, 2 >( Shuffle< Ax, Ax, By, By >( A, B ), Shuffle< Cz, Cz, Dw, Dw >( C, D ) );
    }
    
    // Replicate X into XYZW
    FORCE_INLINE const V128 ReplicateX( const V128 src )
    {
        return Shuffle<VX,VX,VX,VX>( src );
    }
    
    // Replicate Y into XYZW
    FORCE_INLINE const V128 ReplicateY( const V128 src )
    {
        return Shuffle<VY,VY,VY,VY>( src );
    }
    
    // Replicate Z into XYZW
    FORCE_INLINE const V128 ReplicateZ( const V128 src )
    {
        return Shuffle<VZ,VZ,VZ,VZ>( src );
    }
    
    // Replicate W into XYZW
    FORCE_INLINE const V128 ReplicateW( const V128 src )
    {
        return Shuffle<VW,VW,VW,VW>( src );
    }
    
    // Returns safe value of x for Square Root
    //FORCE_INLINE const V128 SafeSqrtSS( const V128 x )
    //{
    //    return _mm_max_ss( x, _mm_setzero_ps() );
    //}
    
    // Returns safe value of x for Square Root
    //FORCE_INLINE const V128 SafeSqrtPS( const V128 x )
    //{
    //    return _mm_max_ps( x, _mm_setzero_ps() );
    //}
    
    // Returns safe value of x for Reciprocal Square Root
    //FORCE_INLINE const V128 SafeRcpSqrtSS( const V128 x )
    //{
    //    return _mm_max_ss( x, c_FltMinPos );
    //}
    
    // Returns safe value of x for Reciprocal Square Root
    //FORCE_INLINE const V128 SafeRcpSqrtPS( const V128 x )
    //{
    //    return _mm_max_ps( x, c_FltMinPos );
    //}
    
    // Approx Reciprocal : 11 bit accuracy
    FORCE_INLINE const V128 ApproxRcpSS( const V128 x ) // Scalar: X only
    {
        //const V128 rx = _mm_rcp_ss( x );
        //
        //const V128 test = _mm_or_ps( _mm_cmplt_ss( x, c_FltMinNeg ),
        //                             _mm_cmpgt_ss( x, c_FltMinPos ) );
        //
        //return _mm_and_ps( rx, test );
        return _mm_rcp_ss( x );
    }
    FORCE_INLINE const V128 ApproxRcpPS( const V128 x ) // Packed: XYZW
    {
        //const V128 rx = _mm_rcp_ps( x );
        //
        //const V128 test = _mm_or_ps( _mm_cmplt_ps( x, c_FltMinNeg ),
        //                             _mm_cmpgt_ps( x, c_FltMinPos ) );
        //
        //return _mm_and_ps( rx, test );
        return _mm_rcp_ps( x );
    }
    
    // Approx Reciprocal + Newton-Raphson Iteration : 22 bit accuracy
    FORCE_INLINE const V128 RcpSS( const V128 x ) // Scalar: X only
    {
        const V128 rx = ApproxRcpSS( x );
        return _mm_sub_ss( _mm_add_ss( rx, rx ), _mm_mul_ss( x, _mm_mul_ss( rx, rx ) ) );
    }
    FORCE_INLINE const V128 RcpPS( const V128 x ) // Packed: XYZW
    {
        const V128 rx = ApproxRcpPS( x );
        return _mm_sub_ps( _mm_add_ps( rx, rx ), _mm_mul_ps( x, _mm_mul_ps( rx, rx ) ) );
    }
    
    // Approx Reciprocal Sqrt : 11 bit accuracy
    FORCE_INLINE const V128 ApproxRcpSqrtSS( const V128 x ) // Scalar: X only
    {
        //const V128 safe_x = SafeRcpSqrtSS( x );
        //return _mm_rsqrt_ss( safe_x );
        return _mm_rsqrt_ss( x );
    }
    FORCE_INLINE const V128 ApproxRcpSqrtPS( const V128 x ) // Packed: XYZW
    {
        //const V128 safe_x = SafeRcpSqrtPS( x );
        //return _mm_rsqrt_ps( safe_x );
        return _mm_rsqrt_ps( x );
    }
    
    // Approx Reciprocal Sqrt + Newton-Raphson Iteration : 22 bit accuracy
    FORCE_INLINE const V128 RcpSqrtSS( const V128 x ) // Scalar: X only
    {
        const V128 rsx = ApproxRcpSqrtSS( x );
        return _mm_mul_ss( _mm_mul_ss( c_Half, rsx ), _mm_sub_ss( c_Three, _mm_mul_ss( x, _mm_mul_ss( rsx, rsx ) ) ) );
    }
    FORCE_INLINE const V128 RcpSqrtPS( const V128 x ) // Packed: XYZW
    {
        const V128 rsx = ApproxRcpSqrtPS( x );
        return _mm_mul_ps( _mm_mul_ps( c_Half, rsx ), _mm_sub_ps( c_Three, _mm_mul_ps( x, _mm_mul_ps( rsx, rsx ) ) ) );
    }
    
    #endif
    
    
    // RGBA Class
    class RGBA
    {
    public:
        
        RGBA() :
            colour( 0 )
        {
        }
        
        RGBA( U8 r, U8 g, U8 b, U8 a ) :
            colour( ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b )
        {
        }
        
        RGBA( U8 r, U8 g, U8 b ) :
            colour( ( 0xFF << 24 ) | ( r << 16 ) | ( g << 8 ) | b )
        {
        }
        
        RGBA( U32 rgba ) :
            colour( rgba )
        {
        }
        
        const U8 A() const { return ( colour >> 24 ) & 0xFF; }
        const U8 R() const { return ( colour >> 16 ) & 0xFF; }
        const U8 G() const { return ( colour >> 8 ) & 0xFF; }
        const U8 B() const { return colour & 0xFF; }
        
        void A( const U8 a ) { colour = ( colour & 0x00FFFFFF ) | ( a << 24 ); }
        void R( const U8 r ) { colour = ( colour & 0xFF00FFFF ) | ( r << 16 ); }
        void G( const U8 g ) { colour = ( colour & 0xFFFF00FF ) | ( g << 8 ); }
        void B( const U8 b ) { colour = ( colour & 0xFFFFFF00 ) | b; }
        
        const U32 Colour() const { return colour; }
        
        operator const U32 () const { return colour; }
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* name = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s RGBA( %02X, %02X, %02X, %02X )\n", name, R(), G(), B(), A() );
            OutputDebugString( buffer );
        }
        #endif
        
    protected:
        
        U32 colour;
    };
    
    //
    // Float Arrays
    //
    
    struct Array2F32
    {
        Array2F32() {}
        Array2F32( const F32 x, const F32 y )
        {
            m_V[VX] = x;
            m_V[VY] = y;
        }
        F32 m_V[2];
    };
    
    struct Array3F32
    {
        Array3F32() {}
        Array3F32( const F32 x, const F32 y, const F32 z )
        {
            m_V[VX] = x;
            m_V[VY] = y;
            m_V[VZ] = z;
        }
        F32 m_V[3];
    };
    
    struct Array4F32
    {
        Array4F32() {}
        Array4F32( const F32 x, const F32 y, const F32 z, const F32 w )
        {
            m_V[VX] = x;
            m_V[VY] = y;
            m_V[VZ] = z;
            m_V[VW] = w;
        }
        F32 m_V[4];
    };
    
    
    //
    // Vector Element Accessor Templates
    //
    
    //template < class VectorType, unsigned Index >
    //class VectorAccessor
    //{
    //public:
    //    
    //    typedef typename VectorType::DataType ElementType;
    //    
    //    VectorAccessor( VectorType& vector ) : m_Vector( vector ) {}
    //    
    //    void operator = ( const ElementType& value ) const
    //    {
    //        switch ( Index )
    //        {
    //            case 0 : m_Vector.X( value ); break;
    //            case 1 : m_Vector.Y( value ); break;
    //            case 2 : m_Vector.Z( value ); break;
    //            case 3 : m_Vector.W( value ); break;
    //        }
    //    }
    //    
    //    operator const ElementType () const
    //    {
    //        switch ( Index )
    //        {
    //            case 0  : return m_Vector.X();
    //            case 1  : return m_Vector.Y();
    //            case 2  : return m_Vector.Z();
    //            case 3  : return m_Vector.W();
    //            default : return 0.0f;
    //        }
    //    }
    //    
    //private:
    //    
    //    VectorType&         m_Vector;
    //};
    //
    //template < class VectorType, unsigned Index >
    //class VectorConstAccessor
    //{
    //public:
    //    
    //    typedef typename VectorType::DataType ElementType;
    //    
    //    VectorConstAccessor( const VectorType& vector ) : m_Vector( vector ) {}
    //    
    //    operator const ElementType () const
    //    {
    //        switch ( Index )
    //        {
    //            case 0  : return m_Vector.X();
    //            case 1  : return m_Vector.Y();
    //            case 2  : return m_Vector.Z();
    //            case 3  : return m_Vector.W();
    //            default : return 0.0f;
    //        }
    //    }
    //    
    //private:
    //    
    //    const VectorType&   m_Vector;
    //};
    
    
    template < class VectorType >
    class VectorAccessor
    {
    public:
        
        typedef typename VectorType::DataType ElementType;
        
        FORCE_INLINE VectorAccessor( VectorType& vector, const unsigned index ) :
            m_Vector( vector ),
            m_Index( index )
        {
        }
        
        FORCE_INLINE void operator = ( const ElementType& value ) const
        {
            switch ( m_Index )
            {
                case 0 : m_Vector.X( value ); break;
                case 1 : m_Vector.Y( value ); break;
                case 2 : m_Vector.Z( value ); break;
                case 3 : m_Vector.W( value ); break;
            }
        }
        
        FORCE_INLINE operator const ElementType () const
        {
            switch ( m_Index )
            {
                case 0  : return m_Vector.X();
                case 1  : return m_Vector.Y();
                case 2  : return m_Vector.Z();
                case 3  : return m_Vector.W();
                default : return 0.0f;
            }
        }
        
    private:
        
        VectorType&     m_Vector;
        const unsigned  m_Index;
    };
    
    template < class VectorType >
    class VectorConstAccessor
    {
    public:
        
        typedef typename VectorType::DataType ElementType;
        
        FORCE_INLINE VectorConstAccessor( const VectorType& vector, const unsigned index ) :
            m_Vector( vector ),
            m_Index( index )
        {
        }
        
        FORCE_INLINE operator const ElementType () const
        {
            switch ( m_Index )
            {
                case 0  : return m_Vector.X();
                case 1  : return m_Vector.Y();
                case 2  : return m_Vector.Z();
                case 3  : return m_Vector.W();
                default : return 0.0f;
            }
        }
        
    private:
        
        const VectorType&   m_Vector;
        const unsigned      m_Index;
    };
    
        
    /// 4xS32 Integer Vector Class
    ALIGN16 class V4_S32
    {
    public:
        
        V4_S32() :
            #ifdef _SSE_
            xyzw( _mm_setzero_ps() )
            #else
            x( 0 ), y( 0 ), z( 0 ), w( 0 )
            #endif
        {
        }
        
        V4_S32( const V4_S32& copy ) :
		    #ifdef _SSE_
            xyzw( copy.xyzw )
		    #else
			x( copy.x ), y( copy.y ), z( copy.z ), w( copy.w )
		    #endif
        {
        }
        
    public:
        
        #ifdef _SSE_
        union
        {
            struct
            {
                S32 x;
                S32 y;
                S32 z;
                S32 w;
            };
            struct
            {
                V64 xy;
                V64 zw;
            };
            struct
            {
                V128 xyzw;
            };
        };
        #else
        S32 x;
        S32 y;
        S32 z;
        S32 w;
        #endif
    };
    
    
    ///
    /// Hardware Scalar F32 Class
    /// Single float stored in X component of hardware 4 element vector register.
    /// Y, Z & W components are set to zero.
    
    class HF32
    {
    friend class HVecX;
    friend class HVecY;
    friend class HVecZ;
    friend class HVecW;
    friend class HVec2;
    friend class HVec3;
    friend class HVec4;
    
    public:
        
        /// Default Constructor
        FORCE_INLINE HF32() :
            #ifdef _SSE_
            m_F( _mm_setzero_ps() )
            #else
            m_F( 0.0f )
            #endif
        {
        }
        
        /// Copy Constructor
        FORCE_INLINE HF32( const HF32& copy ) :
            m_F( copy.m_F )
        {
        }
        
        /// Copy Assignment
        FORCE_INLINE const HF32& operator = ( const HF32& copy )
        {
            m_F = copy.m_F;
            return *this;
        }
        
        /// F32 Constructor
        FORCE_INLINE HF32( const F32 f ) :
            #ifdef _SSE_
            m_F( _mm_set_ss( f ) )
            #else
            m_F( f )
            #endif
        {
        }
        
        /// double Constructor
        FORCE_INLINE HF32( const double f ) :
            #ifdef _SSE_
            m_F( _mm_set_ss( (float)f ) )
            #else
            m_F( (float)f )
            #endif
        {
        }

        /// F32 Assignment
        FORCE_INLINE const HF32& operator = ( const F32 f )
        {
            #ifdef _SSE_
            m_F = _mm_set_ss( f );
            #else
            m_F = f;
            #endif
            return *this;
        }
        
        /// F32 Cast
        FORCE_INLINE operator const F32() const
        {
            #ifdef _SSE_
            F32 f;
            _mm_store_ss( &f, m_F );
            return f;
            #else
            return m_F;
            #endif
        }
        
        /// S32 Constructor
        FORCE_INLINE HF32( const S32 i ) :
            #ifdef _SSE_
            m_F( _mm_cvtsi32_ss( _mm_setzero_ps(), i ) )
            #else
            m_F( (F32)i )
            #endif
        {
        }
        
        /// S32 Assignment
        FORCE_INLINE const HF32& operator = ( const S32 i )
        {
            #ifdef _SSE_
            m_F = _mm_cvtsi32_ss( _mm_setzero_ps(), i );
            #else
            m_F = (F32)i;
            #endif
        }
        
        /// S32 Cast
        //operator const S32() const
        FORCE_INLINE const S32 F2I() const
        {
            #ifdef _SSE_
            return _mm_cvtss_si32( m_F );
            #else
            return (S32)m_F;
            #endif
        }
        
        /// U32 Constructor
        FORCE_INLINE HF32( const U32 i ) :
            #ifdef _SSE_
            m_F( _mm_cvtsi32_ss( _mm_setzero_ps(), i ) )
            #else
            m_F( (F32)i )
            #endif
        {
        }
        
        /// U32 Assignment
        FORCE_INLINE const HF32& operator = ( const U32 i )
        {
            #ifdef _SSE_
            m_F = _mm_cvtsi32_ss( _mm_setzero_ps(), i );
            #else
            m_F = (F32)i;
            #endif
        }
        
        #ifdef _SSE_
        
        /// V128 Constructor: Explicit construction from unknown data due to potential for data loss!
        FORCE_INLINE explicit HF32( const V128& src ) :
            m_F( src )
        {
        }
        
        /// V128 Cast
        FORCE_INLINE operator const V128 () const
        {
            return m_F;
        }
        
        #endif
        
        explicit HF32( const HVecX& x );
        explicit HF32( const HVecY& y );
        explicit HF32( const HVecZ& z );
        explicit HF32( const HVecW& w );
        
        /// Abs
        FORCE_INLINE const HF32 Abs() const
        {
            #ifdef _SSE_
            return HF32( _mm_and_ps( c_AbsMask, m_F ) );
            #else
            return HF32( fabs( m_F ) );
            #endif
        }
        
        /// Square Root
        FORCE_INLINE const HF32 Sqrt() const
        {
            #ifdef _SSE_
            return HF32( _mm_sqrt_ss( m_F ) );
            #else
            return HF32( sqrt( m_F ) );
            #endif
        }
        
        /// Reciprocal
        FORCE_INLINE const HF32 Rcp() const
        {
            #ifdef _SSE_
            return HF32( RcpSS( m_F ) );
            #else
            return HF32( 1.0f / m_F );
            #endif
        }
        
        /// Reciprocal
        FORCE_INLINE const HF32 Reciprocal() const
        {
            #ifdef _SSE_
            return HF32( RcpSS( m_F ) );
            #else
            return HF32( 1.0f / m_F );
            #endif
        }
        
        /// Reciprocal Square Root
        FORCE_INLINE const HF32 RcpSqrt() const
        {
            #ifdef _SSE_
            return HF32( RcpSqrtSS( m_F ) );
            #else
            return HF32( 1.0f / sqrt( m_F ) );
            #endif
        }
        
        /// Reciprocal Square Root
        FORCE_INLINE const HF32 ReciprocalSqrt() const
        {
            #ifdef _SSE_
            return HF32( RcpSqrtSS( m_F ) );
            #else
            return HF32( 1.0f / sqrt( m_F ) );
            #endif
        }
        
        /// Basic Arithmetic +, -, *, /
        
        FORCE_INLINE const HF32 operator - () const
        {
            #ifdef _SSE_
            return HF32( _mm_sub_ss( _mm_setzero_ps(), m_F ) );
            #else
            return HF32( -m_F );
            #endif
        }
        
        FORCE_INLINE const HF32 operator + ( const HF32& src ) const
        {
            #ifdef _SSE_
            return HF32( _mm_add_ss( m_F, src.m_F ) );
            #else
            return HF32( m_F + src.m_F );
            #endif
        }
        
        FORCE_INLINE const HF32 operator - ( const HF32& src ) const
        {
            #ifdef _SSE_
            return HF32( _mm_sub_ss( m_F, src.m_F ) );
            #else
            return HF32( m_F - src.m_F );
            #endif
        }
        
        FORCE_INLINE const HF32 operator * ( const HF32& src ) const
        {
            #ifdef _SSE_
            return HF32( _mm_mul_ss( m_F, src.m_F ) );
            #else
            return HF32( m_F * src.m_F );
            #endif
        }
        
        FORCE_INLINE const HF32 operator / ( const HF32& src ) const
        {
            #ifdef _SSE_
            //return HF32( _mm_div_ss( m_F, src.m_F ) );
            return HF32( _mm_mul_ss( m_F, src.Rcp() ) );
            #else
            return HF32( m_F / src.m_F );
            #endif
        }
        
        FORCE_INLINE const HF32    operator + ( const F32 src ) const      { return operator + ( HF32( src ) ); }
        FORCE_INLINE const HF32    operator - ( const F32 src ) const      { return operator - ( HF32( src ) ); }
        FORCE_INLINE const HF32    operator * ( const F32 src ) const      { return operator * ( HF32( src ) ); }
        FORCE_INLINE const HF32    operator / ( const F32 src ) const      { return operator / ( HF32( src ) ); }
        
        FORCE_INLINE const HF32    operator + ( const U32 src ) const      { return operator + ( HF32( src ) ); }
        FORCE_INLINE const HF32    operator - ( const U32 src ) const      { return operator - ( HF32( src ) ); }
        FORCE_INLINE const HF32    operator * ( const U32 src ) const      { return operator * ( HF32( src ) ); }
        FORCE_INLINE const HF32    operator / ( const U32 src ) const      { return operator / ( HF32( src ) ); }
        
        FORCE_INLINE const HF32    operator + ( const S32 src ) const      { return operator + ( HF32( src ) ); }
        FORCE_INLINE const HF32    operator - ( const S32 src ) const      { return operator - ( HF32( src ) ); }
        FORCE_INLINE const HF32    operator * ( const S32 src ) const      { return operator * ( HF32( src ) ); }
        FORCE_INLINE const HF32    operator / ( const S32 src ) const      { return operator / ( HF32( src ) ); }
        
        /// Basic Arithmetic +=, -=, *=, /=
        
        FORCE_INLINE const HF32& operator += ( const HF32& src )
        {
            #ifdef _SSE_
            m_F = _mm_add_ss( m_F, src.m_F );
            #else
            m_F = m_F + src.m_F;
            #endif
            return *this;
        }
        
        FORCE_INLINE const HF32& operator -= ( const HF32& src )
        {
            #ifdef _SSE_
            m_F = _mm_sub_ss( m_F, src.m_F );
            #else
            m_F = m_F - src.m_F;
            #endif
            return *this;
        }
        
        FORCE_INLINE const HF32& operator *= ( const HF32& src )
        {
            #ifdef _SSE_
            m_F = _mm_mul_ss( m_F, src.m_F );
            #else
            m_F = m_F * src.m_F;
            #endif
            return *this;
        }
        
        FORCE_INLINE const HF32& operator /= ( const HF32& src )
        {
            #ifdef _SSE_
            //m_F = _mm_div_ss( m_F, src.m_F );
            m_F = _mm_mul_ss( m_F, src.Rcp() );
            #else
            m_F = m_F / src.m_F;
            #endif
            return *this;
        }
        
        FORCE_INLINE const HF32&   operator += ( const F32 src )   { return operator += ( HF32( src ) ); }
        FORCE_INLINE const HF32&   operator -= ( const F32 src )   { return operator -= ( HF32( src ) ); }
        FORCE_INLINE const HF32&   operator *= ( const F32 src )   { return operator *= ( HF32( src ) ); }
        FORCE_INLINE const HF32&   operator /= ( const F32 src )   { return operator /= ( HF32( src ) ); }
        
        FORCE_INLINE const HF32&   operator += ( const S32 src )   { return operator += ( HF32( src ) ); }
        FORCE_INLINE const HF32&   operator -= ( const S32 src )   { return operator -= ( HF32( src ) ); }
        FORCE_INLINE const HF32&   operator *= ( const S32 src )   { return operator *= ( HF32( src ) ); }
        FORCE_INLINE const HF32&   operator /= ( const S32 src )   { return operator /= ( HF32( src ) ); }
        
        FORCE_INLINE const HF32&   operator += ( const U32 src )   { return operator += ( HF32( src ) ); }
        FORCE_INLINE const HF32&   operator -= ( const U32 src )   { return operator -= ( HF32( src ) ); }
        FORCE_INLINE const HF32&   operator *= ( const U32 src )   { return operator *= ( HF32( src ) ); }
        FORCE_INLINE const HF32&   operator /= ( const U32 src )   { return operator /= ( HF32( src ) ); }
        
        /// Min & Max
        
        FORCE_INLINE const HF32 Min( const HF32& src ) const
        {
            #ifdef _SSE_
            return HF32( _mm_min_ss( m_F, src.m_F ) );
            #else
            return HF32( Maths::Min( m_F, src.m_F ) );
            #endif
        }
        
        FORCE_INLINE const HF32 Max( const HF32& src ) const
        {
            #ifdef _SSE_
            return HF32( _mm_max_ss( m_F, src.m_F ) );
            #else
            return HF32( Maths::Max( m_F, src.m_F ) );
            #endif
        }
        
        FORCE_INLINE const HF32 Min( const F32 src ) const   { return Min( HF32( src ) ); }
        FORCE_INLINE const HF32 Max( const F32 src ) const   { return Max( HF32( src ) ); }
        
        FORCE_INLINE const HF32& MinEq( const HF32& src )
        {
            #ifdef _SSE_
            m_F = _mm_min_ss( m_F, src.m_F );
            #else
            m_F = Maths::Min( m_F, src.m_F );
            #endif
            return *this;
        }
        
        FORCE_INLINE const HF32& MaxEq( const HF32& src )
        {
            #ifdef _SSE_
            m_F = _mm_max_ss( m_F, src.m_F );
            #else
            m_F = Maths::Max( m_F, src.m_F );
            #endif
            return *this;
        }
        
        FORCE_INLINE const HF32& MinEq( const F32 src )  { return MinEq( HF32( src ) ); }
        FORCE_INLINE const HF32& MaxEq( const F32 src )  { return MaxEq( HF32( src ) ); }
        
        /// Comparison operators
        
        FORCE_INLINE const S32 operator == ( const HF32& src ) const
        {
            #ifdef _SSE_
            //return _mm_movemask_ps( _mm_cmpeq_ps( m_F, src.m_F ) ) & 1;
            // This version does not modify the vector register in the comparison op
            return _mm_ucomieq_ss( m_F, src.m_F );
            #else
            return m_F == src.m_F;
            #endif
        }
        
        FORCE_INLINE const S32 operator != ( const HF32& src ) const
        {
            #ifdef _SSE_
            //return _mm_movemask_ps( _mm_cmpneq_ps( m_F, src.m_F ) ) & 1;
            // This version does not modify the vector register in the comparison op
            return _mm_ucomineq_ss( m_F, src.m_F );
            #else
            return m_F != src.m_F;
            #endif
        }
        
        FORCE_INLINE const S32 operator <= ( const HF32& src ) const
        {
            #ifdef _SSE_
            //return _mm_movemask_ps( _mm_cmple_ps( m_F, src.m_F ) ) & 1;
            // This version does not modify the vector register in the comparison op
            return _mm_ucomile_ss( m_F, src.m_F );
            #else
            return m_F <= src.m_F;
            #endif
        }
        
        FORCE_INLINE const S32 operator < ( const HF32& src ) const
        {
            #ifdef _SSE_
            //return _mm_movemask_ps( _mm_cmplt_ps( m_F, src.m_F ) ) & 1;
            // This version does not modify the vector register in the comparison op
            return _mm_ucomilt_ss( m_F, src.m_F );
            #else
            return m_F < src.m_F;
            #endif
        }
        
        FORCE_INLINE const S32 operator >= ( const HF32& src ) const
        {
            #ifdef _SSE_
            //return _mm_movemask_ps( _mm_cmpge_ps( m_F, src.m_F ) ) & 1;
            // This version does not modify the vector register in the comparison op
            return _mm_ucomige_ss( m_F, src.m_F );
            #else
            return m_F >= src.m_F;
            #endif
        }
        
        FORCE_INLINE const S32 operator > ( const HF32& src ) const
        {
            #ifdef _SSE_
            //return _mm_movemask_ps( _mm_cmpgt_ps( m_F, src.m_F ) ) & 1;
            // This version does not modify the vector register in the comparison op
            return _mm_ucomigt_ss( m_F, src.m_F );
            #else
            return m_F > src.m_F;
            #endif
        }
        
        FORCE_INLINE const S32     operator == ( const F32 src ) const     { return *this == HF32( src ); }
        FORCE_INLINE const S32     operator != ( const F32 src ) const     { return *this != HF32( src ); }
        FORCE_INLINE const S32     operator <= ( const F32 src ) const     { return *this <= HF32( src ); }
        FORCE_INLINE const S32     operator <  ( const F32 src ) const     { return *this <  HF32( src ); }
        FORCE_INLINE const S32     operator >= ( const F32 src ) const     { return *this >= HF32( src ); }
        FORCE_INLINE const S32     operator >  ( const F32 src ) const     { return *this >  HF32( src ); }
        
        FORCE_INLINE const S32 IsNonZero() const
        {
            #ifdef _SSE_
            return _mm_comineq_ss( m_F, _mm_setzero_ps() );
            #else
            return m_F != 0.0f;
            #endif
        }
        
        FORCE_INLINE const S32 IsZero() const
        {
            #ifdef _SSE_
            return 1 ^ _mm_comineq_ss( m_F, _mm_setzero_ps() );
            #else
            return m_F == 0.0f;
            #endif
        }
        
#ifdef WIN32
        FORCE_INLINE const S32 Finite() const
        {
            #ifdef _SSE_
            return 1 ^ ( _mm_comilt_ss( m_F, HF32( -FLT_MAX ) ) |
                         _mm_comigt_ss( m_F, HF32( +FLT_MAX ) ) );
            #else
            return _finite( m_F );
            #endif
        }
#endif
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s HF32( %+8.3f )\n", text, (F32)*this );
            OutputDebugString( buffer );
        }
        #endif
        
    private:
        
        #ifdef _SSE_
        V128    m_F;
        #else
        F32     m_F;
        #endif
    };
    
    FORCE_INLINE const HF32 operator + ( const F32 lhs, const HF32& rhs )   { return HF32( lhs ) + rhs; }
    FORCE_INLINE const HF32 operator - ( const F32 lhs, const HF32& rhs )   { return HF32( lhs ) - rhs; }
    FORCE_INLINE const HF32 operator * ( const F32 lhs, const HF32& rhs )   { return HF32( lhs ) * rhs; }
    FORCE_INLINE const HF32 operator / ( const F32 lhs, const HF32& rhs )   { return HF32( lhs ) / rhs; }
    
    FORCE_INLINE const HF32 operator + ( const S32 lhs, const HF32& rhs )   { return HF32( lhs ) + rhs; }
    FORCE_INLINE const HF32 operator - ( const S32 lhs, const HF32& rhs )   { return HF32( lhs ) - rhs; }
    FORCE_INLINE const HF32 operator * ( const S32 lhs, const HF32& rhs )   { return HF32( lhs ) * rhs; }
    FORCE_INLINE const HF32 operator / ( const S32 lhs, const HF32& rhs )   { return HF32( lhs ) / rhs; }
    
    FORCE_INLINE const HF32 operator + ( const U32 lhs, const HF32& rhs )   { return HF32( lhs ) + rhs; }
    FORCE_INLINE const HF32 operator - ( const U32 lhs, const HF32& rhs )   { return HF32( lhs ) - rhs; }
    FORCE_INLINE const HF32 operator * ( const U32 lhs, const HF32& rhs )   { return HF32( lhs ) * rhs; }
    FORCE_INLINE const HF32 operator / ( const U32 lhs, const HF32& rhs )   { return HF32( lhs ) / rhs; }
    
    FORCE_INLINE const HF32 HMin( const HF32& a, const HF32& b )     { return a.Min( b ); }
    FORCE_INLINE const HF32 HMin( const F32& a, const HF32& b )      { return HF32( a ).Min( b ); }
    FORCE_INLINE const HF32 HMin( const HF32& a, const F32& b )      { return a.Min( HF32( b ) ); }
    
    FORCE_INLINE const HF32 HMax( const HF32& a, const HF32& b )     { return a.Max( b ); }
    FORCE_INLINE const HF32 HMax( const F32& a, const HF32& b )      { return HF32( a ).Max( b ); }
    FORCE_INLINE const HF32 HMax( const HF32& a, const F32& b )      { return a.Max( HF32( b ) ); }
    
    FORCE_INLINE const HF32 HAbs( const HF32& src )                  { return src.Abs(); }
    FORCE_INLINE const HF32 HSqrt( const HF32& src )                 { return src.Sqrt(); }
    FORCE_INLINE const HF32 HTcp( const HF32& src )                  { return src.Rcp(); }
    FORCE_INLINE const HF32 HTcpsqrt( const HF32& src )              { return src.RcpSqrt(); }
    FORCE_INLINE const HF32 HRcp( const HF32& src )                  { return src.Rcp(); }
    FORCE_INLINE const HF32 HRcpSqrt( const HF32& src )              { return src.RcpSqrt(); }
    FORCE_INLINE const HF32 HReciprocal( const HF32& src )           { return src.Rcp(); }
    FORCE_INLINE const HF32 HReciprocalSqrt( const HF32& src )       { return src.RcpSqrt(); }
    
    FORCE_INLINE const S32 HIsNonZero( const HF32& src )             { return src.IsNonZero(); }
    FORCE_INLINE const S32 HIsZero( const HF32& src )                { return src.IsZero(); }
#ifdef WIN32
    FORCE_INLINE const S32 HFinite( const HF32& src )                { return src.Finite(); }
#endif
    
    //
    // Hardware 1 Element F32 Vector X Component Class
    //
    
    ALIGN16 class HVecX
    {
    friend class HF32;
    friend class HVecY;
    friend class HVecZ;
    friend class HVecW;
    friend class HVec2;
    friend class HVec3;
    friend class HVec4;
    
    public:
        
        /// Default Constructor
        HVecX() :
            #ifdef _SSE_
            m_X( _mm_setzero_ps() )
            #else
            m_X( 0.0f )
            #endif
        {
        }
        
        /// Copy Constructor
        HVecX( const HVecX& copy ) :
            m_X( copy.m_X )
        {
        }
        
        /// Copy Assignment
        const HVecX& operator = ( const HVecX& copy )
        {
            m_X = copy.m_X;
            return *this;
        }
        
        #ifdef _SSE_
        
        /// V128 Constructor: Explicit construction from unknown data due to potential for data loss!
        explicit HVecX( const V128& src ) :
            m_X( src )
        {
        }
        
        /// V128 Cast
        operator const V128 () const
        {
            return m_X;
        }
        
        #endif
        
        /// HF32 Constructor
        HVecX( const HF32& x ) :
            m_X( x.m_F )
        {
        }
        
        /// HF32 Assignment
        const HVecX& operator = ( const HF32& x )
        {
            m_X = x.m_F;
            return *this;
        }
        
        /// HF32 now has an explicit constructor from HVecX!
        /// HF32 Cast
        //operator const HF32 () const
        //{
        //    return HF32( m_X );
        //}
        
        /// Accessor: Set HF32
        void X( const HF32& x )
        {
            m_X = x.m_F;
        }
        
        /// Accessor: Get HF32
        const HF32 X() const
        {
            return HF32( m_X );
        }
        
        explicit HVecX( const HVecY& src );
        explicit HVecX( const HVecZ& src );
        explicit HVecX( const HVecW& src );
        
        /// Scalar Multiplication
        const HVecX operator * ( const HF32& src ) const
        {
            #ifdef _SSE_
            return HVecX( _mm_mul_ss( m_X, src.m_F ) );
            #else
            return HVecX( m_X * src );
            #endif
        }
        
        const HF32 Sum() const                  { return X(); }
        const HF32 Length() const               { return X(); }
        const HF32 Length2() const              { const HF32 length( X() ); return length * length; }
        const HF32 LengthSquared() const        { return Length2(); }
        const HF32 RcpLength() const            { return X().Rcp(); }    
        const HF32 ReciprocalLength() const     { return RcpLength(); }    
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s HVecX( %+8.3f )\n", text, (F32)X() );
            OutputDebugString( buffer );
        }
        #endif
        
        // Support for Ed's MVector class
        
        typedef F32 DataType;
        const static int SIZE = 1;
	    typedef HVecX& type1; ///< This type is used by MVector to form a constructor.
	    typedef void /*Ut::NullType*/ type2; ///< What on earth is this used for???
        
        #ifdef _SSE_
        template < unsigned Index > const VectorAccessor<HVecX>         At()        { return VectorAccessor<HVecX>( *this, Index ); }
        template < unsigned Index > const VectorConstAccessor<HVecX>    At() const  { return VectorConstAccessor<HVecX>( *this, Index ); }
        #else
        template < unsigned Index > F32&                                At()        { return m_X; }
        template < unsigned Index > const F32                           At() const  { return m_X; }
        #endif
        
        typedef void CrossProductType;
        
    private:
        
        #ifdef _SSE_
        V128    m_X;
        #else
        F32     m_X;
        #endif
    };
    
    //
    // Hardware 1 Element F32 Vector Y Component Class
    //
    
    ALIGN16 class HVecY
    {
    friend class HF32;
    friend class HVecX;
    friend class HVecZ;
    friend class HVecW;
    friend class HVec2;
    friend class HVec3;
    friend class HVec4;
    
    public:
        
        /// Default Constructor
        HVecY() :
            #ifdef _SSE_
            m_Y( _mm_setzero_ps() )
            #else
            m_Y( 0.0f )
            #endif
        {
        }
        
        /// Copy Constructor
        HVecY( const HVecY& copy ) :
            m_Y( copy.m_Y )
        {
        }
        
        /// Copy Assignment
        const HVecY& operator = ( const HVecY& copy )
        {
            m_Y = copy.m_Y;
            return *this;
        }
        
        #ifdef _SSE_
        
        /// V128 Constructor: Explicit construction from unknown data due to potential for data loss!
        explicit HVecY( const V128& src ) :
            m_Y( src )
        {
        }
        
        /// V128 Cast
        operator const V128 () const
        {
            return m_Y;
        }
        
        #endif
        
        /// HF32 Constructor
        HVecY( const HF32& y ) :
            #ifdef _SSE_
            m_Y( Shuffle<1,0,1,1>( y.m_F ) )
            #else
            m_Y( y.m_F )
            #endif
        {
        }
        
        /// HF32 Assignment
        const HVecY& operator = ( const HF32& y )
        {
            #ifdef _SSE_
            m_Y = Shuffle<1,0,1,1>( y.m_F );
            #else
            m_Y = y.m_F;
            #endif
            return *this;
        }
        
        /// HF32 now has an explicit constructor from HVecY!
        /// HF32 Cast
        //operator const HF32 () const
        //{
        //    #ifdef _SSE_
        //    return HF32( Shuffle<1,0,0,0>( m_Y ) );
        //    #else
        //    return HF32( m_Y );
        //    #endif
        //}
        
        /// Accessor: Set HF32
        void Y( const HF32& y )
        {
            #ifdef _SSE_
            m_Y = Shuffle<1,0,1,1>( y.m_F );
            #else
            m_Y = y.m_F;
            #endif
        }
        
        /// Accessor: Get HF32
        const HF32 Y() const
        {
            #ifdef _SSE_
            return HF32( Shuffle<1,0,0,0>( m_Y ) );
            #else
            return HF32( m_Y );
            #endif
        }
        
        explicit HVecY( const HVecX& src );
        explicit HVecY( const HVecZ& src );
        explicit HVecY( const HVecW& src );
        
        /// Scalar Multiplication
        const HVecY operator * ( const HF32& src ) const
        {
            #ifdef _SSE_
            return HVecY( _mm_mul_ps( m_Y, src.m_F ) );
            #else
            return HVecY( m_Y * src );
            #endif
        }
        
        const HF32 Sum() const                  { return Y(); }
        const HF32 Length() const               { return Y(); }
        const HF32 Length2() const              { const HF32 length( Y() ); return length * length; }
        const HF32 LengthSquared() const        { return Length2(); }
        const HF32 RcpLength() const            { return Y().Rcp(); }    
        const HF32 ReciprocalLength() const     { return RcpLength(); }    
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s HVecY( %+8.3f )\n", text, (F32)Y() );
            OutputDebugString( buffer );
        }
        #endif
        
        // Support for Ed's MVector class
        
        typedef F32 DataType;
        const static int SIZE = 1;
	    typedef HVecY& type1; ///< This type is used by MVector to form a constructor.
	    typedef void /*Ut::NullType*/ type2; ///< What on earth is this used for???
        
        #ifdef _SSE_
        template < unsigned Index > const VectorAccessor<HVecY>         At()        { return VectorAccessor<HVecY>( *this, Index ); }
        template < unsigned Index > const VectorConstAccessor<HVecY>    At() const  { return VectorConstAccessor<HVecY>( *this, Index ); }
        #else
        template < unsigned Index > F32&                                At()        { return m_Y; }
        template < unsigned Index > const F32                           At() const  { return m_Y; }
        #endif
        
        typedef void CrossProductType;
        
    private:
        
        #ifdef _SSE_
        V128    m_Y;
        #else
        F32     m_Y;
        #endif
    };
    
    
    //
    // Hardware 1 Element F32 Vector Z Component Class
    //
    
    ALIGN16 class HVecZ
    {
    friend class HF32;
    friend class HVecX;
    friend class HVecY;
    friend class HVecW;
    friend class HVec2;
    friend class HVec3;
    friend class HVec4;
    
    public:
        
        /// Default Constructor
        HVecZ() :
            #ifdef _SSE_
            m_Z( _mm_setzero_ps() )
            #else
            m_Z( 0.0f )
            #endif
        {
        }
        
        /// Copy Constructor
        HVecZ( const HVecZ& copy ) :
            m_Z( copy.m_Z )
        {
        }
        
        /// Copy Assignment
        const HVecZ& operator = ( const HVecZ& copy )
        {
            m_Z = copy.m_Z;
            return *this;
        }
        
        #ifdef _SSE_
        
        /// V128 Constructor: Explicit construction from unknown data due to potential for data loss!
        explicit HVecZ( const V128& src ) :
            m_Z( src )
        {
        }
        
        /// V128 Cast
        operator const V128 () const
        {
            return m_Z;
        }
        
        #endif
        
        /// HF32 Constructor
        HVecZ( const HF32& z ) :
            #ifdef _SSE_
            m_Z( Shuffle<1,1,0,1>( z.m_F ) )
            #else
            m_Z( z.m_F )
            #endif
        {
        }
        
        /// HF32 Assignment
        const HVecZ& operator = ( const HF32& z )
        {
            #ifdef _SSE_
            m_Z = Shuffle<1,1,0,1>( z.m_F );
            #else
            m_Z = z.m_F;
            #endif
            return *this;
        }
        
        /// HF32 now has an explicit constructor from HVecZ!
        /// HF32 Cast
        //operator const HF32 () const
        //{
        //    #ifdef _SSE_
        //    return HF32( Shuffle<2,0,0,0>( m_Z ) );
        //    #else
        //    return HF32( m_Z );
        //    #endif
        //}
        
        /// Accessor: Set HF32
        void Z( const HF32& z )
        {
            #ifdef _SSE_
            m_Z = Shuffle<1,1,0,1>( z.m_F );
            #else
            m_Z = z.m_F;
            #endif
        }
        
        /// Accessor: Get HF32
        const HF32 Z() const
        {
            #ifdef _SSE_
            return HF32( Shuffle<2,0,0,0>( m_Z ) );
            #else
            return HF32( m_Z );
            #endif
        }
        
        explicit HVecZ( const HVecX& src );
        explicit HVecZ( const HVecY& src );
        explicit HVecZ( const HVecW& src );
        
        /// Scalar Multiplication
        const HVecZ operator * ( const HF32& src ) const
        {
            #ifdef _SSE_
            return HVecZ( _mm_mul_ps( m_Z, src.m_F ) );
            #else
            return HVecZ( m_Z * src );
            #endif
        }
        
        const HF32 Sum() const                  { return Z(); }
        const HF32 Length() const               { return Z(); }
        const HF32 Length2() const              { const HF32 length( Z() ); return length * length; }
        const HF32 LengthSquared() const        { return Length2(); }
        const HF32 RcpLength() const            { return Z().Rcp(); }    
        const HF32 ReciprocalLength() const     { return RcpLength(); }    
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s HVecZ( %+8.3f )\n", text, (F32)Z() );
            OutputDebugString( buffer );
        }
        #endif
        
        // Support for Ed's MVector class
        
        typedef F32 DataType;
        const static int SIZE = 1;
	    typedef HVecZ& type1; ///< This type is used by MVector to form a constructor.
	    typedef void /*Ut::NullType*/ type2; ///< What on earth is this used for???
        
        #ifdef _SSE_
        template < unsigned Index > const VectorAccessor<HVecZ>         At()        { return VectorAccessor<HVecZ>( *this, Index ); }
        template < unsigned Index > const VectorConstAccessor<HVecZ>    At() const  { return VectorConstAccessor<HVecZ>( *this, Index ); }
        #else
        template < unsigned Index > F32&                                At()        { return m_Z; }
        template < unsigned Index > const F32                           At() const  { return m_Z; }
        #endif
        
        typedef void CrossProductType;
        
    private:
        
        #ifdef _SSE_
        V128    m_Z;
        #else
        F32     m_Z;
        #endif
    };
    
    
    //
    // Hardware 1 Element F32 Vector W Component Class
    //
    
    ALIGN16 class HVecW
    {
    friend class HF32;
    friend class HVecX;
    friend class HVecY;
    friend class HVecZ;
    friend class HVec2;
    friend class HVec3;
    friend class HVec4;
    
    public:
        
        /// Default Constructor
        HVecW() :
            #ifdef _SSE_
            m_W( _mm_setzero_ps() )
            #else
            m_W( 0.0f )
            #endif
        {
        }
        
        /// Copy Constructor
        HVecW( const HVecW& copy ) :
            m_W( copy.m_W )
        {
        }
        
        /// Copy Assignment
        const HVecW& operator = ( const HVecW& copy )
        {
            m_W = copy.m_W;
            return *this;
        }
        
        #ifdef _SSE_
        
        /// V128 Constructor: Explicit construction from unknown data due to potential for data loss!
        explicit HVecW( const V128& src ) :
            m_W( src )
        {
        }
        
        /// V128 Cast
        operator const V128 () const
        {
            return m_W;
        }
        
        #endif
        
        /// HF32 Constructor
        HVecW( const HF32& w ) :
            #ifdef _SSE_
            m_W( Shuffle<1,1,1,0>( w.m_F ) )
            #else
            m_W( w.m_F )
            #endif
        {
        }
        
        /// HF32 Assignment
        const HVecW& operator = ( const HF32& w )
        {
            #ifdef _SSE_
            m_W = Shuffle<1,1,1,0>( w.m_F );
            #else
            m_W = w.m_F;
            #endif
            return *this;
        }
        
        /// HF32 now has an explicit constructor from HVecW!
        /// HF32 Cast
        //operator const HF32 () const
        //{
        //    #ifdef _SSE_
        //    return HF32( Shuffle<3,0,0,0>( m_W ) );
        //    #else
        //    return HF32( m_W );
        //    #endif
        //}
        
        /// Accessor: Set HF32
        void W( const HF32& w )
        {
            #ifdef _SSE_
            m_W = Shuffle<1,1,1,0>( w.m_F );
            #else
            m_W = w.m_F;
            #endif
        }
        
        /// Accessor: Get HF32
        const HF32 W() const
        {
            #ifdef _SSE_
            return HF32( Shuffle<3,0,0,0>( m_W ) );
            #else
            return HF32( m_W );
            #endif
        }
        
        explicit HVecW( const HVecX& src );
        explicit HVecW( const HVecY& src );
        explicit HVecW( const HVecZ& src );
        
        /// Scalar Multiplication
        const HVecW operator * ( const HF32& src ) const
        {
            #ifdef _SSE_
            return HVecW( _mm_mul_ps( m_W, src.m_F ) );
            #else
            return HVecW( m_W * src );
            #endif
        }
        
        const HF32 Sum() const                  { return W(); }
        const HF32 Length() const               { return W(); }
        const HF32 Length2() const              { const HF32 length( W() ); return length * length; }
        const HF32 LengthSquared() const        { return Length2(); }
        const HF32 RcpLength() const            { return W().Rcp(); }    
        const HF32 ReciprocalLength() const     { return RcpLength(); }    
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s HVecW( %+8.3f )\n", text, (F32)W() );
            OutputDebugString( buffer );
        }
        #endif
        
        // Support for Ed's MVector class
        
        typedef F32 DataType;
        const static int SIZE = 1;
	    typedef HVecW& type1; ///< This type is used by MVector to form a constructor.
	    typedef void /*Ut::NullType*/ type2; ///< What on earth is this used for???
        
        #ifdef _SSE_
        template < unsigned Index > const VectorAccessor<HVecW>         At()        { return VectorAccessor<HVecW>( *this, Index ); }
        template < unsigned Index > const VectorConstAccessor<HVecW>    At() const  { return VectorConstAccessor<HVecW>( *this, Index ); }
        #else
        template < unsigned Index > F32&                                At()        { return m_W; }
        template < unsigned Index > const F32                           At() const  { return m_W; }
        #endif
        
        typedef void CrossProductType;
        
    private:
        
        #ifdef _SSE_
        V128    m_W;
        #else
        F32     m_W;
        #endif
    };
    
    
    /// HF32 Constructor from HVecX
    FORCE_INLINE HF32::HF32( const HVecX& x ) :
        m_F( x.m_X )
    {
    }
    
    /// HF32 Constructor from HVecY
    FORCE_INLINE HF32::HF32( const HVecY& y ) :
        #ifdef _SSE_
        m_F( Shuffle<1,0,0,0>( y.m_Y ) )
        #else
        m_F( y.m_Y )
        #endif
    {
    }
    
    /// HF32 Constructor from HVecZ
    FORCE_INLINE HF32::HF32( const HVecZ& z ) :
        #ifdef _SSE_
        m_F( Shuffle<2,0,0,0>( z.m_Z ) )
        #else
        m_F( z.m_Z )
        #endif
    {
    }
    
    /// HF32 Constructor from HVecW
    FORCE_INLINE HF32::HF32( const HVecW& w ) :
        #ifdef _SSE_
        m_F( Shuffle<3,0,0,0>( w.m_W ) )
        #else
        m_F( w.m_W )
        #endif
    {
    }
    
    
    
    //
    // Hardware 2 Element F32 Vector Class
    //
    
    ALIGN16 class HVec2
    {
    friend class HVec3;
    friend class HVec4;
    
    public:
        
        /// Default Constructor
        HVec2()
            #ifdef _SSE_
            : m_V( _mm_setzero_ps() )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            #endif
        }
        
        #ifdef _SSE_
        
        /// Copy Constructor
        HVec2( const HVec2& copy ) :
            m_V( copy )
        {
        }
        
        /// Copy Assignment
        const HVec2& operator = ( const HVec2& copy )
        {
            m_V = copy;
            return *this;
        }
        
        /// V128 Constructor: Explicit construction from unknown data due to potential for data loss!
        explicit HVec2( const V128& src ) :
            m_V( src )
        {
        }
        
        /// V128 Cast
        operator const V128 () const
        {
            return m_V;
        }
        
        #endif
        
        /// Accessor: Set X
        void X( const HF32& x )
        {
            #ifdef _SSE_
            m_V = Shuffle<0,2,2,3>( Shuffle<0,0,1,1>( x, m_V ), m_V );
            #else
            m_V[VX] = x;
            #endif
        }
        
        /// Accessor: Set Y
        void Y( const HF32& y )
        {
            #ifdef _SSE_
            m_V = Shuffle<0,2,2,3>( Shuffle<0,0,0,0>( m_V, y ), m_V );
            #else
            m_V[VY] = y;
            #endif
        }
        
        /// Accessor: Get X
        const HF32 X() const
        {
            #ifdef _SSE_
            return HF32( Shuffle<0,3,3,3>( m_V ) );
            #else
            return m_V[VX];
            #endif
        }
        
        /// Accessor: Get Y
        const HF32 Y() const
        {
            #ifdef _SSE_
            return HF32( Shuffle<1,3,3,3>( m_V ) );
            #else
            return m_V[VY];
            #endif
        }
        
        /// Accessor: Set Z - Required for templated accessors - should never be used!
        FORCE_INLINE void Z( const HF32& z )
        {
            #ifdef _DEBUG
            #ifdef SHOOTER_VECMAT_DEBUG_BREAK
            DebugBreak();
            #endif
            #endif
        }
        
        /// Accessor: Get Z - Required for templated accessors - should never be used!
        FORCE_INLINE const HF32 Z() const
        {
            #ifdef _DEBUG
            #ifdef SHOOTER_VECMAT_DEBUG_BREAK
            DebugBreak();
            #endif
            #endif
            return 0.0f;
        }
        
        /// Accessor: Set W - Required for templated accessors - should never be used!
        FORCE_INLINE void W( const HF32& w )
        {
            #ifdef _DEBUG
            #ifdef SHOOTER_VECMAT_DEBUG_BREAK
            DebugBreak();
            #endif
            #endif
        }
        
        /// Accessor: Get W - Required for templated accessors - should never be used!
        FORCE_INLINE const HF32 W() const
        {
            #ifdef _DEBUG
            #ifdef SHOOTER_VECMAT_DEBUG_BREAK
            DebugBreak();
            #endif
            #endif
            return 0.0f;
        }
        
        /// F32 Array Constructor
        HVec2( const F32* array )
            #ifdef _SSE_
            : m_V( _mm_and_ps( c_MaskXY, _mm_loadu_ps( array ) ) )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = array[VX];
            m_V[VY] = array[VY];
            #endif
        }
        
        /// HF32 Pair Constructor
        HVec2( const HF32& x, const HF32& y )
            #ifdef _SSE_
            : m_V( _mm_add_ps( HVecX( x ), HVecY( y ) ) )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = x;
            m_V[VY] = y;
            #endif
        }
        
        /// HF32 Broadcast Constructor
        explicit HVec2( const HF32& xy )
            #ifdef _SSE_
            : m_V( Shuffle<0,0,1,1>( xy ) )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = xy;
            m_V[VY] = xy;
            #endif
        }
        
        /// HVecX Constructor
        HVec2( const HVecX& x )
            #ifdef _SSE_
            : m_V( x )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = x.m_X;
            m_V[VY] = 0.0f;
            #endif
        }
        
        /// HVecX Assignment
        const HVec2& operator = ( const HVecX& x )
        {
            #ifdef _SSE_
            m_V = x;
            #else
            m_V[VX] = x.m_X;
            m_V[VY] = 0.0f;
            #endif
            return *this;
        }
        
        /// HVecX Cast
        operator const HVecX () const
        {
            #ifdef _SSE_
            return HVecX( _mm_and_ps( c_MaskX, m_V ) );
            #else
            return (HVecX)m_V[VX];
            #endif
        }
        
        /// HVecY Constructor
        HVec2( const HVecY& y )
            #ifdef _SSE_
            : m_V( y )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = 0.0f;
            m_V[VY] = y.m_Y;
            #endif
        }
        
        /// HVecY Assignment
        const HVec2& operator = ( const HVecY& y )
        {
            #ifdef _SSE_
            m_V = y;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = y.m_Y;
            #endif
            return *this;
        }
        
        /// HVecY Cast
        operator const HVecY () const
        {
            #ifdef _SSE_
            return HVecY( _mm_and_ps( c_MaskY, m_V ) );
            #else
            return (HVecY)m_V[VY];
            #endif
        }
        
        // Explicit construction due to data loss!
        explicit HVec2( const HVec3& src );
        explicit HVec2( const HVec4& src );
        
        /// Unary Negation
        const HVec2 operator - () const
        {
            #ifdef _SSE_
            return HVec2( _mm_sub_ps( _mm_setzero_ps(), m_V ) );
            #else
            return HVec2( -m_V[VX],
                          -m_V[VY] );
            #endif
        }
        
        /// Addition
        const HVec2 operator + ( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_add_ps( m_V, src ) );
            #else
            return HVec2( m_V[VX] + src.m_V[VX],
                          m_V[VY] + src.m_V[VY] );
            #endif
        }
        
        /// Addition
        void operator += ( const HVec2& src )
        {
            #ifdef _SSE_
            m_V = _mm_add_ps( m_V, src );
            #else
            m_V[VX] += src.m_V[VX];
            m_V[VY] += src.m_V[VY];
            #endif
        }
        
        /// Subtraction
        const HVec2 operator - ( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_sub_ps( m_V, src ) );
            #else
            return HVec2( m_V[VX] - src.m_V[VX],
                          m_V[VY] - src.m_V[VY] );
            #endif
        }
        
        /// Subtraction
        void operator -= ( const HVec2& src )
        {
            #ifdef _SSE_
            m_V = _mm_sub_ps( m_V, src );
            #else
            m_V[VX] -= src.m_V[VX];
            m_V[VY] -= src.m_V[VY];
            #endif
        }
        
        /// Multiplication
        const HVec2 operator * ( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_mul_ps( m_V, src ) );
            #else
            return HVec2( m_V[VX] * src.m_V[VX],
                          m_V[VY] * src.m_V[VY] );
            #endif
        }
        
        /// Multiplication
        void operator *= ( const HVec2& src )
        {
            #ifdef _SSE_
            m_V = _mm_mul_ps( m_V, src );
            #else
            m_V[VX] *= src.m_V[VX];
            m_V[VY] *= src.m_V[VY];
            #endif
        }
        
        /// HF32 Multiplication
        const HVec2 operator * ( const HF32& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_mul_ps( m_V, ReplicateX( src ) ) );
            #else
            return HVec2( m_V[VX] * src,
                          m_V[VY] * src );
            #endif
        }
        
        /// HF32 Multiplication
        void operator *= ( const HF32& src )
        {
            #ifdef _SSE_
            m_V = _mm_mul_ps( m_V, ReplicateX( src ) );
            #else
            m_V[VX] *= src;
            m_V[VY] *= src;
            #endif
        }
        
        /// Division
        const HVec2 operator / ( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_mul_ps( m_V, RcpPS( src ) ) );
            #else
            return HVec2( m_V[VX] / src.m_V[VX],
                          m_V[VY] / src.m_V[VY] );
            #endif
        }
        
        /// Division
        void operator /= ( const HVec2& src )
        {
            #ifdef _SSE_
            m_V = _mm_mul_ps( m_V, RcpPS( src ) );
            #else
            m_V[VX] /= src.m_V[VX];
            m_V[VY] /= src.m_V[VY];
            #endif
        }
        
        /// HF32 Division
        const HVec2 operator / ( const HF32& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_mul_ps( m_V, HVec2( HF32( RcpSS( src ) ) ) ) );
            #else
            return HVec2( m_V[VX] / src,
                          m_V[VY] / src );
            #endif
        }
        
        /// HF32 Division
        void operator /= ( const HF32& src )
        {
            #ifdef _SSE_
            m_V = _mm_mul_ps( m_V, HVec2( HF32( RcpSS( src ) ) ) );
            #else
            m_V[VX] /= src;
            m_V[VY] /= src;
            #endif
        }
        
        /// Min
        const HVec2 Min( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_min_ps( m_V, src ) );
            #else
			return HVec2( GMin( m_V[VX], src.m_V[VX] ),
						  GMin( m_V[VY], src.m_V[VY] ) );
            #endif
        }
        
        /// Min operator
        const HVec2 operator << ( const HVec2& src ) const
        {
            return Min( src );
        }
        
        /// Min Update
        void MinEq( const HVec2& src )
        {
            #ifdef _SSE_
            m_V = _mm_min_ps( m_V, src );
            #else
			m_V[VX] = GMin( m_V[VX], src.m_V[VX] );
			m_V[VY] = GMin( m_V[VY], src.m_V[VY] );
            #endif
        }
        
        /// Min Update Operator
        void operator <<= ( const HVec2& src )
        {
            Min( src );
        }
        
        /// Max
        const HVec2 Max( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_max_ps( m_V, src ) );
            #else
			return HVec2( GMax( m_V[VX], src.m_V[VX] ),
						  GMax( m_V[VY], src.m_V[VY] ) );
            #endif
        }
        
        /// Max Operator
        const HVec2 operator >> ( const HVec2& src ) const
        {
            return Max( src );
        }
        
        /// Max Update
        void MaxEq( const HVec2& src )
        {
            #ifdef _SSE_
            m_V = _mm_max_ps( m_V, src );
            #else
			m_V[VX] = GMax( m_V[VX], src.m_V[VX] );
			m_V[VY] = GMax( m_V[VY], src.m_V[VY] );
            #endif
        }
        
        /// Max Update Operator
        void operator >>= ( const HVec2& src )
        {
            Max( src );
        }
        
        /// Lerp
        const HVec2 Lerp( const HVec2& dst, const HF32& f ) const
        {
            const HVec2& src = *this;
            return src + ( ( dst - src ) * f );
        }
        
        /// Horizontal sum of all elements
        const HF32 Sum() const
        {
            #ifdef _SSE_
            const V128 sSum = _mm_add_ss( m_V, ReplicateY( m_V ) );
            return HF32( _mm_and_ps( c_MaskX, sSum ) );
            #else
            return HF32( m_V[VX] + m_V[VY] );
            #endif
        }
        
        /// Dot product
        const HF32 operator | ( const HVec2& rhs ) const
        {
            const HVec2& lhs = *this;
            return ( lhs * rhs ).Sum();
        }
        
        /// Cross product
        const HF32 operator ^ ( const HVec2& rhs ) const
        {
            const HVec2& lhs = *this;
            return lhs.X() * rhs.Y() - lhs.Y() * rhs.X();
        }
        
        /// Vector Length Squared
        const HF32 Length2() const
        {
            const HVec2& src = *this;
            return ( src * src ).Sum();
        }    
        
        /// Vector Length Squared
        const HF32 LengthSquared() const
        {
            return Length2();
        }    
        
        /// Vector Length
        const HF32 Length() const
        {
            return Length2().Sqrt();
        }    
        
        /// Vector Reciprocal of Length
        const HF32 RcpLength() const
        {
            return Length2().RcpSqrt();
        }    
        
        /// Vector Reciprocal of Length
        const HF32 ReciprocalLength() const
        {
            return RcpLength();
        }    
        
        /// Vector Normalise
        const HVec2 Normalised() const
        {
            return operator * ( RcpLength() );
        }
        
        /// Vector Normalise
        void Normalise()
        {
            operator *= ( RcpLength() );
        }
        
        /// Vector Normalise - safe version with additional parameter to use for zero length vectors!
        const HVec2 SafeNormalised( const HVec2& zeroLengthResult = HVec2() ) const
        {
            const HF32 length2( Length2() );
            if ( length2.IsNonZero() )
            {
                return operator * ( HReciprocalSqrt( length2 ) );
            }
            else
            {
                return zeroLengthResult;
            }
        }
        
        /// Vector Normalise - safe version with additional parameter to use for zero length vectors!
        void SafeNormalise( const HVec2& zeroLengthResult = HVec2() )
        {
            const HF32 length2( Length2() );
            if ( length2.IsNonZero() )
            {
                operator *= ( HReciprocalSqrt( length2 ) );
            }
            else
            {
                operator = ( zeroLengthResult );
            }
        }
        
        //
        // Comparison Operators
        //
        
        const HVec2 operator == ( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_cmpeq_ps( m_V, src ) );
            #else
            return HVec2( m_V[VX] == src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] == src.m_V[VY] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec2 operator != ( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_cmpneq_ps( m_V, src ) );
            #else
            return HVec2( m_V[VX] != src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] != src.m_V[VY] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec2 operator <= ( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_cmple_ps( m_V, src ) );
            #else
            return HVec2( m_V[VX] <= src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] <= src.m_V[VY] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec2 operator < ( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_cmplt_ps( m_V, src ) );
            #else
            return HVec2( m_V[VX] < src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] < src.m_V[VY] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec2 operator >= ( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_cmpge_ps( m_V, src ) );
            #else
            return HVec2( m_V[VX] >= src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] >= src.m_V[VY] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec2 operator > ( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_cmpgt_ps( m_V, src ) );
            #else
            return HVec2( m_V[VX] > src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] > src.m_V[VY] ? c_fSet : c_fClear );
            #endif
        }
        
        /// Get sign bits of each element - used for testing results of comparisons
        const E_SignBits GetSignBits() const
        {
            #ifdef _SSE_
            return (E_SignBits)( _mm_movemask_ps( m_V ) & 0x3 );
            #else
            return (E_SignBits)( ( ( (U32&)m_V[VX] >> 31 ) & 0x1 ) |
                                 ( ( (U32&)m_V[VY] >> 30 ) & 0x2 ) );
            #endif
        }
        
        /// Swizzle HVec2:XY into a HVec3:XZ with zero Y
        const HVec3 XYtoXZ() const;
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s HVec2( %+8.3f, %+8.3f )\n", text, (F32)X(), (F32)Y() );
            OutputDebugString( buffer );
        }
        #endif
        
        // Support for Ed's MVector class
        
        typedef F32 DataType;
        const static int SIZE = 2;
	    typedef HVec2& type1; ///< This type is used by MVector to form a constructor.
	    typedef void /*Ut::NullType*/ type2; ///< What on earth is this used for???
        
        #ifdef _SSE_
        template < unsigned Index > const VectorAccessor<HVec2>         At()        { return VectorAccessor<HVec2>( *this, Index ); }
        template < unsigned Index > const VectorConstAccessor<HVec2>    At() const  { return VectorConstAccessor<HVec2>( *this, Index ); }
        #else
        template < unsigned Index > F32&                                At()        { return m_V[ Index ]; }
        template < unsigned Index > const F32                           At() const  { return m_V[ Index ]; }
        #endif
        
        #ifdef _SSE_
        FORCE_INLINE const VectorAccessor<HVec2>       operator [] ( const U32 index )         { return VectorAccessor<HVec2>( *this, index ); }
        FORCE_INLINE const VectorConstAccessor<HVec2>  operator [] ( const U32 index ) const   { return VectorConstAccessor<HVec2>( *this, index ); }
        #else
        FORCE_INLINE F32&                              operator [] ( const U32 index )         { return m_V[ index ]; }
        FORCE_INLINE const F32                         operator [] ( const U32 index ) const   { return m_V[ index ]; }
        #endif
        
        typedef HF32 CrossProductType;
        
    private:
        
        #ifdef _SSE_
        V128    m_V;
        #else
        F32     m_V[2];
        #endif
    };
    
    
    //
    // Hardware 3 Element F32 Vector Class
    //
    
    ALIGN16 class HVec3
    {
    friend class HVec2;
    friend class HVec4;
    friend class AxisX;
    friend class AxisY;
    friend class AxisZ;
    friend class HMat33;
    friend class HMat43;
    friend class HMat44;
    friend class C_Quaternion;
    
    public:
        
        /// Default Constructor
        FORCE_INLINE HVec3()
            #ifdef _SSE_
            : m_V( _mm_setzero_ps() )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            m_V[VZ] = 0.0f;
            #endif
        }
        
        #ifdef _SSE_
        
        /// Copy Constructor
        FORCE_INLINE HVec3( const HVec3& copy ) :
            m_V( copy )
        {
            DebugValidate();
        }
        
        /// Copy Assignment
        FORCE_INLINE const HVec3& operator = ( const HVec3& copy )
        {
            m_V = copy;
            DebugValidate();
            return *this;
        }
        
        /// V128 Constructor: Explicit construction from unknown data due to potential for data loss!
        FORCE_INLINE explicit HVec3( const V128& src ) :
            m_V( src )
        {
        }
        
        /// V128 Cast
        FORCE_INLINE operator const V128 () const
        {
            return m_V;
        }
        
        #endif
        
        /// Accessor: Set X
        FORCE_INLINE void X( const HF32& x )
        {
            #ifdef _SSE_
            m_V = Shuffle<0,2,2,3>( Shuffle<0,0,1,1>( x, m_V ), m_V );
            #else
            m_V[VX] = x;
            #endif
        }
        
        /// Accessor: Set Y
        FORCE_INLINE void Y( const HF32& y )
        {
            #ifdef _SSE_
            m_V = Shuffle<0,2,2,3>( Shuffle<0,0,0,0>( m_V, y ), m_V );
            #else
            m_V[VY] = y;
            #endif
        }
        
        /// Accessor: Set Z
        FORCE_INLINE void Z( const HF32& z )
        {
            #ifdef _SSE_
            m_V = Shuffle<0,1,0,2>( m_V, Shuffle<0,0,3,3>( z, m_V ) );
            #else
            m_V[VZ] = z;
            #endif
        }
        
        /// Accessor: Get X
        FORCE_INLINE const HF32 X() const
        {
            #ifdef _SSE_
            return HF32( Shuffle<0,3,3,3>( m_V ) );
            #else
            return m_V[VX];
            #endif
        }
        
        /// Accessor: Get Y
        FORCE_INLINE const HF32 Y() const
        {
            #ifdef _SSE_
            return HF32( Shuffle<1,3,3,3>( m_V ) );
            #else
            return m_V[VY];
            #endif
        }
        
        /// Accessor: Get Z
        FORCE_INLINE const HF32 Z() const
        {
            #ifdef _SSE_
            return HF32( Shuffle<2,3,3,3>( m_V ) );
            #else
            return m_V[VZ];
            #endif
        }
        
        /// Accessor: Set W - Required for templated accessors - should never be used!
        FORCE_INLINE void W( const HF32& w )
        {
            #ifdef _DEBUG
            #ifdef SHOOTER_VECMAT_DEBUG_BREAK
            DebugBreak();
            #endif
            #endif
        }
        
        /// Accessor: Get W - Required for templated accessors - should never be used!
        FORCE_INLINE const HF32 W() const
        {
            #ifdef _DEBUG
            #ifdef SHOOTER_VECMAT_DEBUG_BREAK
            DebugBreak();
            #endif
            #endif
            return 0.0f;
        }
        
        #ifdef _SSE_
        /// Accessor: Get W
        FORCE_INLINE const HF32 TestW() const
        {
            return HF32( Shuffle<3,3,3,3>( m_V ) );
        }
        #endif
        
        /// HF32 Triple Constructor
        FORCE_INLINE HVec3( const HF32& x, const HF32& y, const HF32& z )
            #ifdef _SSE_
            : m_V( _mm_add_ps( _mm_add_ps( HVecX(x), HVecY(y) ), HVecZ(z) ) )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = x;
            m_V[VY] = y;
            m_V[VZ] = z;
            #endif
        }
        
        /// HVecX + HVecY + HVecZ Triple Constructor
        FORCE_INLINE HVec3( const HVecX& x, const HVecY& y, const HVecZ& z )
            #ifdef _SSE_
            : m_V( _mm_add_ps( _mm_add_ps( x, y ), z ) )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = (HF32)x;
            m_V[VY] = (HF32)y;
            m_V[VZ] = (HF32)z;
            #endif
        }
        
        /// HF32 Broadcast Constructor
        FORCE_INLINE explicit HVec3( const HF32& xyz )
            #ifdef _SSE_
            : m_V( Shuffle<0,0,0,1>( xyz ) )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = xyz;
            m_V[VY] = xyz;
            m_V[VZ] = xyz;
            #endif
        }
        
        /// F32 Array Constructor
        FORCE_INLINE HVec3( const F32* array )
            #ifdef _SSE_
            : m_V( _mm_and_ps( c_MaskXYZ, _mm_loadu_ps( array ) ) )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = array[VX];
            m_V[VY] = array[VY];
            m_V[VZ] = array[VZ];
            #endif
        }
        
        /// Array3F32 Constructor
        FORCE_INLINE HVec3( const Array3F32& array )
            #ifdef _SSE_
            : m_V( _mm_and_ps( c_MaskXYZ, _mm_loadu_ps( array.m_V ) ) )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = array.m_V[VX];
            m_V[VY] = array.m_V[VY];
            m_V[VZ] = array.m_V[VZ];
            #endif
        }
        
        /// Array3F32 Assignment
        FORCE_INLINE const HVec3& operator = ( const Array3F32& array )
        {
            #ifdef _SSE_
            m_V = _mm_and_ps( c_MaskXYZ, _mm_loadu_ps( array.m_V ) );
            #else
            m_V[VX] = array.m_V[VX];
            m_V[VY] = array.m_V[VY];
            m_V[VZ] = array.m_V[VZ];
            #endif
            return *this;
        }
        
        /// Array3F32 Cast
        FORCE_INLINE operator const Array3F32() const
        {
            #ifdef _SSE_
            Array4F32 temp;
            _mm_storeu_ps( temp.m_V, m_V );
            return Array3F32( temp.m_V[VX], temp.m_V[VY], temp.m_V[VZ] );
            #else
            return Array3F32( m_V[VX], m_V[VY], m_V[VZ] );
            #endif
        }
        
        /// HVecX Constructor
        FORCE_INLINE HVec3( const HVecX& src )
            #ifdef _SSE_
            : m_V( src )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = src.m_X;
            m_V[VY] = 0.0f;
            m_V[VZ] = 0.0f;
            #endif
        }
        
        /// HVecY Constructor
        FORCE_INLINE HVec3( const HVecY& src )
            #ifdef _SSE_
            : m_V( src )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = 0.0f;
            m_V[VY] = src.m_Y;
            m_V[VZ] = 0.0f;
            #endif
        }
        
        /// HVecZ Constructor
        FORCE_INLINE HVec3( const HVecZ& src )
            #ifdef _SSE_
            : m_V( src )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            m_V[VZ] = src.m_Z;
            #endif
        }
        
        /// HVec2 Constructor
        FORCE_INLINE HVec3( const HVec2& src )
            #ifdef _SSE_
            : m_V( src )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = src.m_V[VX];
            m_V[VY] = src.m_V[VY];
            m_V[VZ] = 0.0f;
            #endif
        }
        
        // Explicit construction due to data loss!
        explicit HVec3( const HVec4& src );
        
        /// V4_S32 Integer Array Cast
        FORCE_INLINE operator const V4_S32() const
        {
            V4_S32 result;
			#ifdef _SSE_
            //result.xy = _mm_cvtps_pi32( m_V ); // Current rounding mode
            //result.zw = _mm_cvtps_pi32( _mm_movehl_ps( m_V, m_V ) );
            result.xy = _mm_cvttps_pi32( m_V ); // Truncate
            result.zw = _mm_cvttps_pi32( _mm_movehl_ps( m_V, m_V ) );
            _mm_empty();
            #else
            result.x = (S32)m_V[VX];
            result.y = (S32)m_V[VY];
            result.z = (S32)m_V[VZ];
            result.w = 0;
            #endif
            return result;
        }
        
        /// HVecX Cast
        FORCE_INLINE operator const HVecX () const
        {
            #ifdef _SSE_
            return HVecX( _mm_and_ps( c_MaskX, m_V ) );
            #else
            return (HVecX)m_V[VX];
            #endif
        }
        
        /// HVecY Cast
        FORCE_INLINE operator const HVecY () const
        {
            #ifdef _SSE_
            return HVecY( _mm_and_ps( c_MaskY, m_V ) );
            #else
            return (HVecY)m_V[VY];
            #endif
        }
        
        /// HVecZ Cast
        FORCE_INLINE operator const HVecZ () const
        {
            #ifdef _SSE_
            return HVecZ( _mm_and_ps( c_MaskZ, m_V ) );
            #else
            return (HVecZ)m_V[VZ];
            #endif
        }
        
        /// HVecX Assignment
        FORCE_INLINE const HVec3& operator = ( const HVecX& src )
        {
            #ifdef _SSE_
            m_V = src;
            #else
            m_V[VX] = src.m_X;
            m_V[VY] = 0.0f;
            m_V[VZ] = 0.0f;
            #endif
            return *this;
        }
        
        /// HVecY Assignment
        FORCE_INLINE const HVec3& operator = ( const HVecY& src )
        {
            #ifdef _SSE_
            m_V = src;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = src.m_Y;
            m_V[VZ] = 0.0f;
            #endif
            return *this;
        }
        
        /// HVecZ Assignment
        FORCE_INLINE const HVec3& operator = ( const HVecZ& src )
        {
            #ifdef _SSE_
            m_V = src;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            m_V[VZ] = src.m_Z;
            #endif
            return *this;
        }
        
        /// HVec2 Assignment
        FORCE_INLINE const HVec3& operator = ( const HVec2& src )
        {
            #ifdef _SSE_
            m_V = src;
            #else
            m_V[VX] = src.m_V[VX];
            m_V[VY] = src.m_V[VY];
            m_V[VZ] = 0.0f;
            #endif
            return *this;
        }
        
        /// F32 Array Assignment
        FORCE_INLINE const HVec3& operator = ( const F32* array )
        {
            #ifdef _SSE_
            m_V = _mm_and_ps( c_MaskXYZ, _mm_loadu_ps( array ) );
            #else
            m_V[VX] = array[VX];
            m_V[VY] = array[VY];
            m_V[VZ] = array[VZ];
            #endif
            return *this;
        }
        
        //
        // Clear Elements
        //
        
        FORCE_INLINE const HVec3 ZeroX() const
        {
            #ifdef _SSE_
            return HVec3( _mm_and_ps( c_MaskYZ, m_V ) );
            #else
            return HVec3( 0.0f, m_V[VY], m_V[VZ] );
            #endif
        }
        
        FORCE_INLINE const HVec3 ZeroY() const
        {
            #ifdef _SSE_
            return HVec3( _mm_and_ps( c_MaskXZ, m_V ) );
            #else
            return HVec3( m_V[VX], 0.0f, m_V[VZ] );
            #endif
        }
        
        FORCE_INLINE const HVec3 ZeroZ() const
        {
            #ifdef _SSE_
            return HVec3( _mm_and_ps( c_MaskXY, m_V ) );
            #else
            return HVec3( m_V[VX], m_V[VY], 0.0f );
            #endif
        }
        
        FORCE_INLINE const HVec3 ZeroXY() const
        {
            #ifdef _SSE_
            return HVec3( _mm_and_ps( c_MaskZ, m_V ) );
            #else
            return HVec3( 0.0f, 0.0f, m_V[VZ] );
            #endif
        }
        
        FORCE_INLINE const HVec3 ZeroXZ() const
        {
            #ifdef _SSE_
            return HVec3( _mm_and_ps( c_MaskY, m_V ) );
            #else
            return HVec3( 0.0f, m_V[VY], 0.0f );
            #endif
        }
        
        FORCE_INLINE const HVec3 ZeroYZ() const
        {
            #ifdef _SSE_
            return HVec3( _mm_and_ps( c_MaskX, m_V ) );
            #else
            return HVec3( m_V[VX], 0.0f, 0.0f );
            #endif
        }
        
        //
        // Unary Operators
        //
        
        /// Unary Negation
        FORCE_INLINE const HVec3 operator - () const
        {
            #ifdef _SSE_
            return HVec3( _mm_sub_ps( _mm_setzero_ps(), m_V ) );
            #else
            return HVec3( -m_V[VX], -m_V[VY], -m_V[VZ] );
            #endif
        }
        
        //
        // Finite Test
        //
#ifdef WIN32
        const S32 Finite() const
        {
            return _finite( X() ) &&
                   _finite( Y() ) &&
                   _finite( Z() );
        }
#endif
        
        //
        // Binary HVec3 Operators
        //
        
        /// Addition
        FORCE_INLINE const HVec3 operator + ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_add_ps( m_V, src ) );
            #else
            return HVec3( m_V[VX] + src.m_V[VX], m_V[VY] + src.m_V[VY], m_V[VZ] + src.m_V[VZ] );
            #endif
        }
        
        FORCE_INLINE HVec3& operator += ( const HVec3& src )
        {
            #ifdef _SSE_
            m_V = _mm_add_ps( m_V, src );
            #else
            m_V[VX] += src.m_V[VX];
            m_V[VY] += src.m_V[VY];
            m_V[VZ] += src.m_V[VZ];
            #endif
            return *this;
        }
        
        /// Subtraction
        FORCE_INLINE const HVec3 operator - ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_sub_ps( m_V, src ) );
            #else
            return HVec3( m_V[VX] - src.m_V[VX], m_V[VY] - src.m_V[VY], m_V[VZ] - src.m_V[VZ] );
            #endif
        }
        
        FORCE_INLINE HVec3& operator -= ( const HVec3& src )
        {
            #ifdef _SSE_
            m_V = ( _mm_sub_ps( m_V, src ) );
            #else
            m_V[VX] -= src.m_V[VX];
            m_V[VY] -= src.m_V[VY];
            m_V[VZ] -= src.m_V[VZ];
            #endif
            return *this;
        }
        
        /// Multiplication
        FORCE_INLINE const HVec3 operator * ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, src ) );
            #else
            return HVec3( m_V[VX] * src.m_V[VX], m_V[VY] * src.m_V[VY], m_V[VZ] * src.m_V[VZ] );
            #endif
        }
        
        FORCE_INLINE HVec3& operator *= ( const HVec3& src )
        {
            #ifdef _SSE_
            m_V = _mm_mul_ps( m_V, src );
            #else
            m_V[VX] *= src.m_V[VX];
            m_V[VY] *= src.m_V[VY];
            m_V[VZ] *= src.m_V[VZ];
            #endif
            return *this;
        }
        
        /// Division
        FORCE_INLINE const HVec3 operator / ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, RcpPS( src ) ) );
            #else
            return HVec3( m_V[VX] / src.m_V[VX], m_V[VY] / src.m_V[VY], m_V[VZ] / src.m_V[VZ] );
            #endif
        }
        
        FORCE_INLINE HVec3& operator /= ( const HVec3& src )
        {
            #ifdef _SSE_
            m_V = _mm_mul_ps( m_V, RcpPS( src ) );
            #else
            m_V[VX] /= src.m_V[VX];
            m_V[VY] /= src.m_V[VY];
            m_V[VZ] /= src.m_V[VZ];
            #endif
            return *this;
        }
        
        /// Min
        FORCE_INLINE const HVec3 Min( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_min_ps( m_V, src ) );
            #else
			return HVec3( GMin( m_V[VX], src.m_V[VX] ),
					  	  GMin( m_V[VY], src.m_V[VY] ),
						  GMin( m_V[VZ], src.m_V[VZ] ) );
            #endif
        }
        
        FORCE_INLINE const HVec3 operator << ( const HVec3& src ) const
        {
            return Min( src );
        }
        
        FORCE_INLINE void MinEq( const HVec3& src )
        {
            #ifdef _SSE_
            m_V = _mm_min_ps( m_V, src );
            #else
			m_V[VX] = GMin( m_V[VX], src.m_V[VX] );
			m_V[VY] = GMin( m_V[VY], src.m_V[VY] );
			m_V[VZ] = GMin( m_V[VZ], src.m_V[VZ] );
            #endif
        }
        
        FORCE_INLINE void operator <<= ( const HVec3& src )
        {
            Min( src );
        }
        
        /// Max
        FORCE_INLINE const HVec3 Max( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_max_ps( m_V, src ) );
            #else
			return HVec3( GMax( m_V[VX], src.m_V[VX] ),
						  GMax( m_V[VY], src.m_V[VY] ),
						  GMax( m_V[VZ], src.m_V[VZ] ) );
            #endif
        }
        
        FORCE_INLINE const HVec3 operator >> ( const HVec3& src ) const
        {
            return Max( src );
        }
        
        FORCE_INLINE void MaxEq( const HVec3& src )
        {
            #ifdef _SSE_
            m_V = _mm_max_ps( m_V, src );
            #else
			m_V[VX] = GMax( m_V[VX], src.m_V[VX] );
			m_V[VY] = GMax( m_V[VY], src.m_V[VY] );
			m_V[VZ] = GMax( m_V[VZ], src.m_V[VZ] );
            #endif
        }
        
        FORCE_INLINE void operator >>= ( const HVec3& src )
        {
            Max( src );
        }
        
        //
        // Binary HF32 Operators
        //
        
        FORCE_INLINE const HVec3 operator + ( const HF32& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_add_ps( m_V, HVec3( src ) ) );
            #else
            return HVec3( m_V[VX] + src, m_V[VY] + src, m_V[VZ] + src );
            #endif
        }
        
        FORCE_INLINE HVec3& operator += ( const HF32& src )
        {
            #ifdef _SSE_
            m_V = _mm_add_ps( m_V, HVec3( src ) );
            #else
            m_V[VX] += src;
            m_V[VY] += src;
            m_V[VZ] += src;
            #endif
            return *this;
        }
        
        FORCE_INLINE const HVec3 operator - ( const HF32& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_sub_ps( m_V, HVec3( src ) ) );
            #else
            return HVec3( m_V[VX] - src, m_V[VY] - src, m_V[VZ] - src );
            #endif
        }
        
        FORCE_INLINE HVec3& operator -= ( const HF32& src )
        {
            #ifdef _SSE_
            m_V = _mm_sub_ps( m_V, HVec3( src ) );
            #else
            m_V[VX] -= src;
            m_V[VY] -= src;
            m_V[VZ] -= src;
            #endif
            return *this;
        }
        
        FORCE_INLINE const HVec3 operator * ( const HF32& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, HVec3( src ) ) );
            #else
            return HVec3( m_V[VX] * src, m_V[VY] * src, m_V[VZ] * src );
            #endif
        }
        
        FORCE_INLINE HVec3& operator *= ( const HF32& src )
        {
            #ifdef _SSE_
            m_V = _mm_mul_ps( m_V, HVec3( src ) );
            #else
            m_V[VX] *= src;
            m_V[VY] *= src;
            m_V[VZ] *= src;
            #endif
            return *this;
        }
        
        FORCE_INLINE const HVec3 operator / ( const HF32& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, HVec3( (HF32)RcpSS( src ) ) ) );
            #else
            return HVec3( m_V[VX] / src, m_V[VY] / src, m_V[VZ] / src );
            #endif
        }
        
        FORCE_INLINE HVec3& operator /= ( const HF32& src )
        {
            #ifdef _SSE_
            m_V = _mm_mul_ps( m_V, HVec3( (HF32)RcpSS( src ) ) );
            #else
            m_V[VX] /= src;
            m_V[VY] /= src;
            m_V[VZ] /= src;
            #endif
            return *this;
        }
        
        //
        // Binary HVecX Operators
        //
        
        FORCE_INLINE const HVec3 operator * ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, ReplicateX( src ) ) );
            #else
            return HVec3( m_V[VX] * src.m_X, m_V[VY] * src.m_X, m_V[VZ] * src.m_X );
            #endif
        }
        
        FORCE_INLINE HVec3& operator *= ( const HVecX& src )
        {
            #ifdef _SSE_
            m_V = _mm_mul_ps( m_V, ReplicateX( src ) );
            #else
            m_V[VX] *= src.m_X;
            m_V[VY] *= src.m_X;
            m_V[VZ] *= src.m_X;
            #endif
            return *this;
        }
        
        //
        // Binary HVecY Operators
        //
        
        FORCE_INLINE const HVec3 operator * ( const HVecY& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, ReplicateY( src ) ) );
            #else
            return HVec3( m_V[VX] * src.m_Y, m_V[VY] * src.m_Y, m_V[VZ] * src.m_Y );
            #endif
        }
        
        //
        // Binary HVecZ Operators
        //
        
        FORCE_INLINE const HVec3 operator * ( const HVecZ& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, ReplicateZ( src ) ) );
            #else
            return HVec3( m_V[VX] * src.m_Z, m_V[VY] * src.m_Z, m_V[VZ] * src.m_Z );
            #endif
        }
        
        //
        // Binary HVecW Operators
        //
        
        FORCE_INLINE const HVec3 operator * ( const HVecW& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, ReplicateW( src ) ) );
            #else
            return HVec3( m_V[VX] * src.m_W, m_V[VY] * src.m_W, m_V[VZ] * src.m_W );
            #endif
        }
        
        //
        // Comparison Operators
        //
        
        FORCE_INLINE const HVec3 operator == ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_cmpeq_ps( m_V, src ) );
            #else
            return HVec3( m_V[VX] == src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] == src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] == src.m_V[VZ] ? c_fSet : c_fClear );
            #endif
        }
        
        FORCE_INLINE const HVec3 operator != ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_cmpneq_ps( m_V, src ) );
            #else
            return HVec3( m_V[VX] != src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] != src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] != src.m_V[VZ] ? c_fSet : c_fClear );
            #endif
        }
        
        FORCE_INLINE const HVec3 operator <= ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_cmple_ps( m_V, src ) );
            #else
            return HVec3( m_V[VX] <= src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] <= src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] <= src.m_V[VZ] ? c_fSet : c_fClear );
            #endif
        }
        
        FORCE_INLINE const HVec3 operator < ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_cmplt_ps( m_V, src ) );
            #else
            return HVec3( m_V[VX] < src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] < src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] < src.m_V[VZ] ? c_fSet : c_fClear );
            #endif
        }
        
        FORCE_INLINE const HVec3 operator >= ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_cmpge_ps( m_V, src ) );
            #else
            return HVec3( m_V[VX] >= src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] >= src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] >= src.m_V[VZ] ? c_fSet : c_fClear );
            #endif
        }
        
        FORCE_INLINE const HVec3 operator > ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_cmpgt_ps( m_V, src ) );
            #else
            return HVec3( m_V[VX] > src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] > src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] > src.m_V[VZ] ? c_fSet : c_fClear );
            #endif
        }
        
        /// Bitwise AND
        FORCE_INLINE const HVec3 AND( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_and_ps( m_V, src.m_V ) );
            #else
            const U32 x = (U32&)m_V[VX] & (U32&)src.m_V[VX];
            const U32 y = (U32&)m_V[VY] & (U32&)src.m_V[VY];
            const U32 z = (U32&)m_V[VZ] & (U32&)src.m_V[VZ];
            return HVec3( (F32&)x, (F32&)y, (F32&)z );
            #endif
        }
        
        /// Bitwise AND-NOT
        FORCE_INLINE const HVec3 ANDNOT( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_andnot_ps( m_V, src.m_V ) );
            #else
            const U32 x = ~(U32&)m_V[VX] & (U32&)src.m_V[VX];
            const U32 y = ~(U32&)m_V[VY] & (U32&)src.m_V[VY];
            const U32 z = ~(U32&)m_V[VZ] & (U32&)src.m_V[VZ];
            return HVec3( (F32&)x, (F32&)y, (F32&)z );
            #endif
        }
        
        /// Bitwise OR
        FORCE_INLINE const HVec3 OR( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_or_ps( m_V, src.m_V ) );
            #else
            const U32 x = (U32&)m_V[VX] | (U32&)src.m_V[VX];
            const U32 y = (U32&)m_V[VY] | (U32&)src.m_V[VY];
            const U32 z = (U32&)m_V[VZ] | (U32&)src.m_V[VZ];
            return HVec3( (F32&)x, (F32&)y, (F32&)z );
            #endif
        }
        
        /// Bitwise OR
        FORCE_INLINE const HVec3 XOR( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_xor_ps( m_V, src.m_V ) );
            #else
            const U32 x = (U32&)m_V[VX] ^ (U32&)src.m_V[VX];
            const U32 y = (U32&)m_V[VY] ^ (U32&)src.m_V[VY];
            const U32 z = (U32&)m_V[VZ] ^ (U32&)src.m_V[VZ];
            return HVec3( (F32&)x, (F32&)y, (F32&)z );
            #endif
        }
        
        /// Get sign bits of each element - used for testing results of comparisons
        FORCE_INLINE const E_SignBits GetSignBits() const
        {
            #ifdef _SSE_
            return (E_SignBits)( _mm_movemask_ps( m_V ) & 0x7 );
            #else
            return (E_SignBits)( ( ( (U32&)m_V[VX] >> 31 ) & 0x1 ) |
                                 ( ( (U32&)m_V[VY] >> 30 ) & 0x2 ) |
                                 ( ( (U32&)m_V[VZ] >> 29 ) & 0x4 ) );
            #endif
        }
        
        /// Swizzle
        template < unsigned SX, unsigned SY, unsigned SZ >
        FORCE_INLINE const HVec3 Swizzle() const
        {
            #ifdef _SSE_
            return HVec3( _mm_shuffle_ps( m_V, m_V, _MM_SHUFFLE( SX, SY, SZ, VW ) ) );
            #else
            return HVec3( operator [] ( SX ),
                          operator [] ( SY ),
                          operator [] ( SZ ) );
            #endif
        }
        
        /// Horizontal sum of all elements
        FORCE_INLINE const HF32 Sum() const
        {
            #ifdef _SSE_
            const V128 sSum = _mm_add_ss( m_V, _mm_add_ss( ReplicateY( m_V ), ReplicateZ( m_V ) ) );
            return HF32( _mm_and_ps( c_MaskX, sSum ) );
            #else
            return HF32( m_V[VX] + m_V[VY] + m_V[VZ] );
            #endif
        }
        
        /// Dot product
        FORCE_INLINE const HF32 operator | ( const HVec3& r ) const
        {
            const HVec3& l = *this;
            return ( l * r ).Sum();
        }
        
        /// Cross Product
        FORCE_INLINE const HVec3 operator ^ ( const HVec3& r ) const
        {
            const HVec3& l = *this;
            #ifdef _SSE_
            const V128 l_yzx = Shuffle<VY,VZ,VX,VW>( l );
            const V128 l_zxy = Shuffle<VZ,VX,VY,VW>( l );
            const V128 r_yzx = Shuffle<VY,VZ,VX,VW>( r );
            const V128 r_zxy = Shuffle<VZ,VX,VY,VW>( r );
            return HVec3( _mm_sub_ps( _mm_mul_ps( l_yzx, r_zxy ), _mm_mul_ps( l_zxy, r_yzx ) ) );
            #else
            return HVec3( l.m_V[VY] * r.m_V[VZ] - l.m_V[VZ] * r.m_V[VY],
                          l.m_V[VZ] * r.m_V[VX] - l.m_V[VX] * r.m_V[VZ],
                          l.m_V[VX] * r.m_V[VY] - l.m_V[VY] * r.m_V[VX] );
            #endif
        }
        
        /// Combine with HVecW to form a HVec4
        //const HVec4 operator + ( const HVecW& src ) const;
        const HVec4 operator || ( const HVecW& src ) const;
        
        /// Vector * Matrix Multiplication
        const HVec3 operator * ( const HMat33& m ) const;
        const HVec3 operator * ( const HMat43& m ) const;
        const HVec3 operator * ( const HMat44& m ) const;
        
        /// Vector * C_Quaternion Multiplication
        const HVec3 operator * ( const C_Quaternion& q ) const;
        
        /// Vector Length Squared
        FORCE_INLINE const HF32 Length2() const
        {
            const HVec3& src = *this;
            return ( src * src ).Sum();
        }    
        
        /// Vector Length Squared
        FORCE_INLINE const HF32 LengthSquared() const
        {
            return Length2();
        }    
        
        /// Vector Length
        FORCE_INLINE const HF32 Length() const
        {
            return Length2().Sqrt();
        }    
        
        /// Vector Reciprocal of Length
        FORCE_INLINE const HF32 RcpLength() const
        {
            return Length2().RcpSqrt();
        }    
        
        /// Vector Reciprocal of Length
        FORCE_INLINE const HF32 ReciprocalLength() const
        {
            return RcpLength();
        }    
        
        /// Vertex Distance
        FORCE_INLINE const HF32 Distance( const HVec3& v ) const
        {
            const HVec3& src = *this;
            return ( src - v ).Length();
        }
        
        /// Lerp
        FORCE_INLINE const HVec3 Lerp( const HVec3& dst, const HF32& f ) const
        {
            const HVec3& src = *this;
            return src + ( ( dst - src ) * f );
        }
        
        /// Vector Normalise
        const HVec3 Normalised() const
        {
            return operator * ( RcpLength() );
        }
        
        /// Vector Normalise
        void Normalise()
        {
            operator *= ( RcpLength() );
        }
        
        /// Vector Normalise - safe version with additional parameter to use for zero length vectors!
        const HVec3 SafeNormalised( const HVec3& zeroLengthResult = HVec3() ) const
        {
            const HF32 length2( Length2() );
            if ( length2.IsNonZero() )
            {
                return operator * ( HReciprocalSqrt( length2 ) );
            }
            else
            {
                return zeroLengthResult;
            }
        }
        
        /// Vector Normalise - safe version with additional parameter to use for zero length vectors!
        void SafeNormalise( const HVec3& zeroLengthResult = HVec3() )
        {
            const HF32 length2( Length2() );
            if ( length2.IsNonZero() )
            {
                operator *= ( HReciprocalSqrt( length2 ) );
            }
            else
            {
                operator = ( zeroLengthResult );
            }
        }
        
        /// Get Vector of Magnitude
        FORCE_INLINE const HVec3 Magnitude( const HF32& mag ) const
        {
            return operator * ( mag * RcpLength() );
        }
        
        /// Get components of this vector parallel & perpendicular to input vector
        FORCE_INLINE void ParaPerp( const HVec3& vector, HVec3& para, HVec3& perp ) const
        {
            const HVec3 normal = vector.Normalised();
            const HF32 dp = *this | normal;
            para = normal * dp;
            perp = *this - para;
        }
        
        /// Get axes perpendicular to vector.
        void PerpendicularAxes( HVec3& u, HVec3& v ) const;
        
        /// Swizzle HVec3:XZ into a HVec2:XY
        FORCE_INLINE const HVec2 SwizzleXZtoXY() const
        {
            #ifdef _SSE_
            return HVec2( Shuffle<VX,VZ,VW,VW>( m_V ) );
            #else
            return HVec2( m_V[VX], m_V[VZ] );
            #endif
        }
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            #ifdef _SSE_
            if ( TestW() != 0.0f )
            {
                sprintf( buffer, "%s HVec3( %+8.3f, %+8.3f, %+8.3f ) [ %+18.15f !!! ]\n", text, (F32)X(), (F32)Y(), (F32)Z(), (F32)TestW() );
            }
            else
            {
                sprintf( buffer, "%s HVec3( %+8.3f, %+8.3f, %+8.3f )\n", text, (F32)X(), (F32)Y(), (F32)Z() );
            }
            #else
            sprintf( buffer, "%s HVec3( %+8.3f, %+8.3f, %+8.3f )\n", text, (F32)X(), (F32)Y(), (F32)Z() );
            #endif
            OutputDebugString( buffer );
        }
        #endif
        
        void DebugValidate() const;
        
    private:
        
        #ifdef _SSE_
        V128    m_V;
        #else
        F32     m_V[3];
        #endif
        
    public:
        
        // Support for Ed's MVector class
        
        typedef F32 DataType;
        const static int SIZE = 3;
	    typedef HVec3& type1; ///< This type is used by MVector to form a constructor.
	    typedef void /*Ut::NullType*/ type2; ///< What on earth is this used for???
        
        #ifdef _SSE_
        template < unsigned Index > const VectorAccessor<HVec3>         At()        { return VectorAccessor<HVec3>( *this, Index ); }
        template < unsigned Index > const VectorConstAccessor<HVec3>    At() const  { return VectorConstAccessor<HVec3>( *this, Index ); }
        #else
        template < unsigned Index > F32&                                At()        { return m_V[ Index ]; }
        template < unsigned Index > const F32                           At() const  { return m_V[ Index ]; }
        #endif
        
        #ifdef _SSE_
        FORCE_INLINE const VectorAccessor<HVec3>       operator [] ( const U32 index )         { return VectorAccessor<HVec3>( *this, index ); }
        FORCE_INLINE const VectorConstAccessor<HVec3>  operator [] ( const U32 index ) const   { return VectorConstAccessor<HVec3>( *this, index ); }
        #else
        FORCE_INLINE F32&                              operator [] ( const U32 index )         { return m_V[ index ]; }
        FORCE_INLINE const F32                         operator [] ( const U32 index ) const   { return m_V[ index ]; }
        #endif
        
        typedef HVec3 CrossProductType;
    };
    
    //
    // Hardware 4 Element F32 Vector Class
    //
    
    ALIGN16 class HVec4
    {
    friend class HVec2;
    friend class HVec3;
    friend class HMat33;
    friend class HMat43;
    friend class HMat44;
    friend class C_Quaternion;
    friend class C_Plane;
    
    public:
        
        /// Default Constructor
        HVec4()
            #ifdef _SSE_
            : m_V( _mm_setzero_ps() )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            m_V[VZ] = 0.0f;
            m_V[VW] = 0.0f;
            #endif
        }
        
        #ifdef _SSE_
        
        /// Copy Constructor
        HVec4( const HVec4& copy ) :
            m_V( copy )
        {
        }
        
        /// Copy Assignment
        const HVec4& operator = ( const HVec4& copy )
        {
            m_V = copy;
            return *this;
        }
        
        /// V128 Constructor
        HVec4( const V128& src ) :
            m_V( src )
        {
        }
        
        /// V128 Assignment
        const HVec4& operator = ( const V128& copy )
        {
            m_V = copy;
            return *this;
        }
        
        /// V128 Cast
        operator const V128 () const
        {
            return m_V;
        }
        
        #endif
        
        /// Accessor: Set X
        void X( const HF32& x )
        {
            #ifdef _SSE_
            m_V = Shuffle<0,2,2,3>( Shuffle<0,0,1,1>( x, m_V ), m_V );
            #else
            m_V[VX] = x;
            #endif
        }
        
        /// Accessor: Set Y
        void Y( const HF32& y )
        {
            #ifdef _SSE_
            m_V = Shuffle<0,2,2,3>( Shuffle<0,0,0,0>( m_V, y ), m_V );
            #else
            m_V[VY] = y;
            #endif
        }
        
        /// Accessor: Set Z
        void Z( const HF32& z )
        {
            #ifdef _SSE_
            m_V = Shuffle<0,1,0,2>( m_V, Shuffle<0,0,3,3>( z, m_V ) );
            #else
            m_V[VZ] = z;
            #endif
        }
        
        /// Accessor: Set W
        void W( const HF32& w )
        {
            #ifdef _SSE_
            m_V = Shuffle<0,1,0,2>( m_V, Shuffle<2,2,0,0>( m_V, w ) );
            #else
            m_V[VW] = w;
            #endif
        }
        
        /// Accessor: Get X
        const HF32 X() const
        {
            #ifdef _SSE_
            return HF32( _mm_and_ps( c_MaskX, m_V ) );
            #else
            return m_V[VX];
            #endif
        }
        
        /// Accessor: Get Y
        const HF32 Y() const
        {
            #ifdef _SSE_
            return HF32( _mm_and_ps( c_MaskX, ReplicateY( m_V ) ) );
            #else
            return m_V[VY];
            #endif
        }
        
        /// Accessor: Get Z
        const HF32 Z() const
        {
            #ifdef _SSE_
            return HF32( _mm_and_ps( c_MaskX, ReplicateZ( m_V ) ) );
            #else
            return m_V[VZ];
            #endif
        }
        
        /// Accessor: Get W
        const HF32 W() const
        {
            #ifdef _SSE_
            return HF32( _mm_and_ps( c_MaskX, ReplicateW( m_V ) ) );
            #else
            return m_V[VW];
            #endif
        }
        
        /// HF32 Quad Constructor
        HVec4( const HF32& x, const HF32& y, const HF32& z, const HF32& w )
            #ifdef _SSE_
            : m_V( _mm_add_ps( _mm_add_ps( HVecX(x), HVecY(y) ), _mm_add_ps( HVecZ(z), HVecW(w) ) ) )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = x;
            m_V[VY] = y;
            m_V[VZ] = z;
            m_V[VW] = w;
            #endif
        }
        
        /// HF32 Broadcast Constructor
        explicit HVec4( const HF32& xyzw )
            #ifdef _SSE_
            : m_V( ReplicateX( xyzw ) )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = xyzw;
            m_V[VY] = xyzw;
            m_V[VZ] = xyzw;
            m_V[VW] = xyzw;
            #endif
        }
        
        /// F32 Array Constructor
        HVec4( const F32* array )
            #ifdef _SSE_
            : m_V( _mm_loadu_ps( array ) )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = array[VX];
            m_V[VY] = array[VY];
            m_V[VZ] = array[VZ];
            m_V[VW] = array[VW];
            #endif
        }
        
        /// HVecX Constructor
        HVec4( const HVecX& x )
            #ifdef _SSE_
            : m_V( x )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = x.m_X;
            m_V[VY] = 0.0f;
            m_V[VZ] = 0.0f;
            m_V[VW] = 0.0f;
            #endif
        }
        
        /// HVecY Constructor
        HVec4( const HVecY& y )
            #ifdef _SSE_
            : m_V( y )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = 0.0f;
            m_V[VY] = y.m_Y;
            m_V[VZ] = 0.0f;
            m_V[VW] = 0.0f;
            #endif
        }
        
        /// HVecZ Constructor
        HVec4( const HVecZ& z )
            #ifdef _SSE_
            : m_V( z )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            m_V[VZ] = z.m_Z;
            m_V[VW] = 0.0f;
            #endif
        }
        
        /// HVecW Constructor
        HVec4( const HVecW& w )
            #ifdef _SSE_
            : m_V( w )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            m_V[VZ] = 0.0f;
            m_V[VW] = w.m_W;
            #endif
        }
        
        /// HVec2 Constructor
        HVec4( const HVec2& xy )
            #ifdef _SSE_
            : m_V( xy )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = xy.m_V[VX];
            m_V[VY] = xy.m_V[VY];
            m_V[VZ] = 0.0f;
            m_V[VW] = 0.0f;
            #endif
        }
        
        /// HVec3 Constructor
        HVec4( const HVec3& xyz )
            #ifdef _SSE_
            : m_V( xyz )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = xyz.m_V[VX];
            m_V[VY] = xyz.m_V[VY];
            m_V[VZ] = xyz.m_V[VZ];
            m_V[VW] = 0.0f;
            #endif
        }
        
        /// HVecX Cast
        operator const HVecX () const
        {
            #ifdef _SSE_
            return HVecX( _mm_and_ps( c_MaskX, m_V ) );
            #else
            return (HVecX)m_V[VX];
            #endif
        }
        
        /// HVecY Cast
        operator const HVecY () const
        {
            #ifdef _SSE_
            return HVecY( _mm_and_ps( c_MaskY, m_V ) );
            #else
            return (HVecY)m_V[VY];
            #endif
        }
        
        /// HVecZ Cast
        operator const HVecZ () const
        {
            #ifdef _SSE_
            return HVecZ( _mm_and_ps( c_MaskZ, m_V ) );
            #else
            return (HVecZ)m_V[VZ];
            #endif
        }
        
        /// HVecW Cast
        operator const HVecW () const
        {
            #ifdef _SSE_
            return HVecW( _mm_and_ps( c_MaskW, m_V ) );
            #else
            return (HVecW)m_V[VW];
            #endif
        }
        
        /// HVecX Assignment
        const HVec4& operator = ( const HVecX& x )
        {
            #ifdef _SSE_
            m_V = x;
            #else
            m_V[VX] = x.m_X;
            m_V[VY] = 0.0f;
            m_V[VZ] = 0.0f;
            m_V[VW] = 0.0f;
            #endif
            return *this;
        }
        
        /// HVecY Assignment
        const HVec4& operator = ( const HVecY& y )
        {
            #ifdef _SSE_
            m_V = y;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = y.m_Y;
            m_V[VZ] = 0.0f;
            m_V[VW] = 0.0f;
            #endif
            return *this;
        }
        
        /// HVecZ Assignment
        const HVec4& operator = ( const HVecZ& z )
        {
            #ifdef _SSE_
            m_V = z;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            m_V[VZ] = z.m_Z;
            m_V[VW] = 0.0f;
            #endif
            return *this;
        }
        
        /// HVecW Assignment
        const HVec4& operator = ( const HVecW& w )
        {
            #ifdef _SSE_
            m_V = w;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            m_V[VZ] = 0.0f;
            m_V[VW] = w.m_W;
            #endif
            return *this;
        }
        
        /// HVec2 Assignment
        const HVec4& operator = ( const HVec2& xy )
        {
            #ifdef _SSE_
            m_V = xy;
            #else
            m_V[VX] = xy.m_V[VX];
            m_V[VY] = xy.m_V[VY];
            m_V[VZ] = 0.0f;
            m_V[VW] = 0.0f;
            #endif
            return *this;
        }
        
        /// HVec3 Assignment
        const HVec4& operator = ( const HVec3& xyz )
        {
            #ifdef _SSE_
            m_V = xyz;
            #else
            m_V[VX] = xyz.m_V[VX];
            m_V[VY] = xyz.m_V[VY];
            m_V[VZ] = xyz.m_V[VZ];
            m_V[VW] = 0.0f;
            #endif
            return *this;
        }
        
        /// F32 Array Assignment
        const HVec4& operator = ( const F32* array )
        {
            #ifdef _SSE_
            m_V = _mm_loadu_ps( array );
            #else
            m_V[VX] = array[VX];
            m_V[VY] = array[VY];
            m_V[VZ] = array[VZ];
            m_V[VW] = array[VW];
            #endif
            return *this;
        }
        
        /// Unary Negation
        const HVec4 operator - () const
        {
            #ifdef _SSE_
            return HVec4( _mm_sub_ps( _mm_setzero_ps(), m_V ) );
            #else
            return HVec4( -m_V[VX],
                          -m_V[VY],
                          -m_V[VZ],
                          -m_V[VW] );
            #endif
        }
        
        /// Addition
        const HVec4 operator + ( const HVec4& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_add_ps( m_V, src ) );
            #else
            return HVec4( m_V[VX] + src.m_V[VX],
                          m_V[VY] + src.m_V[VY],
                          m_V[VZ] + src.m_V[VZ],
                          m_V[VW] + src.m_V[VW] );
            #endif
        }
        
        /// Subtraction
        const HVec4 operator - ( const HVec4& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_sub_ps( m_V, src ) );
            #else
            return HVec4( m_V[VX] - src.m_V[VX],
                          m_V[VY] - src.m_V[VY],
                          m_V[VZ] - src.m_V[VZ],
                          m_V[VW] - src.m_V[VW] );
            #endif
        }
        
        /// Multiplication
        const HVec4 operator * ( const HVec4& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_mul_ps( m_V, src ) );
            #else
            return HVec4( m_V[VX] * src.m_V[VX],
                          m_V[VY] * src.m_V[VY],
                          m_V[VZ] * src.m_V[VZ],
                          m_V[VW] * src.m_V[VW] );
            #endif
        }
        
        /// Division
        const HVec4 operator / ( const HVec4& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_mul_ps( m_V, RcpPS( src ) ) );
            #else
            return HVec4( m_V[VX] / src.m_V[VX],
                          m_V[VY] / src.m_V[VY],
                          m_V[VZ] / src.m_V[VZ],
                          m_V[VW] / src.m_V[VW] );
            #endif
        }
        
        /// HF32 Multiplication
        const HVec4 operator * ( const HF32& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_mul_ps( m_V, HVec4( src ) ) );
            #else
            return HVec4( m_V[VX] * src,
                          m_V[VY] * src,
                          m_V[VZ] * src,
                          m_V[VW] * src );
            #endif
        }
        
        void operator *= ( const HF32& src )
        {
            #ifdef _SSE_
            m_V = _mm_mul_ps( m_V, HVec4( src ) );
            #else
            m_V[VX] *= src;
            m_V[VY] *= src;
            m_V[VZ] *= src;
            m_V[VW] *= src;
            #endif
        }
        
        /// HF32 Division
        const HVec4 operator / ( const HF32& src ) const
        {
            #ifdef _SSE_
            return operator * ( HF32( RcpSS( src ) ) );
            #else
            return operator * ( HF32( 1.0f ) / src );
            #endif
        }
        
        void operator /= ( const HF32& src )
        {
            #ifdef _SSE_
            operator *= ( HF32( RcpSS( src ) ) );
            #else
            operator *= ( HF32( 1.0f ) / src );
            #endif
        }
        
        /// Horizontal sum of all elements
        const HF32 Sum() const
        {
            #ifdef _SSE_
            const V128 sum = _mm_add_ss( _mm_add_ss( m_V,               ReplicateY( m_V ) ),
                                         _mm_add_ss( ReplicateZ( m_V ), ReplicateW( m_V ) ) );
            return HF32( _mm_and_ps( c_MaskX, sum ) );
            #else
            return HF32( m_V[VX] + m_V[VY] + m_V[VZ] + m_V[VW] );
            #endif
        }
        
        /// Dot product
        const HF32 operator | ( const HVec4& r ) const
        {
            const HVec4& l = *this;
            return ( l * r ).Sum();
        }
        
        /// Vector Length Squared
        const HF32 Length2() const
        {
            const HVec4& src = *this;
            return ( src * src ).Sum();
        }
        
        /// Vector Length Squared
        const HF32 LengthSquared() const
        {
            return Length2();
        }
        
        /// Vector Length
        const HF32 Length() const
        {
            return Length2().Sqrt();
        }
        
        /// Vector Reciprocal of Length
        const HF32 RcpLength() const
        {
            return Length2().RcpSqrt();
        }    
        
        /// Vector Reciprocal of Length
        const HF32 ReciprocalLength() const
        {
            return RcpLength();
        }    
        
        /// Vector Normalise
        const HVec4 Normalised() const
        {
            return operator * ( RcpLength() );
        }
        
        /// Vector Normalise
        void Normalise()
        {
            operator *= ( RcpLength() );
        }
        
        /// Vector Normalise - safe version with additional parameter to use for zero length vectors!
        const HVec4 SafeNormalised( const HVec4& zeroLengthResult = HVec4() ) const
        {
            const HF32 length2( Length2() );
            if ( length2.IsNonZero() )
            {
                return operator * ( HReciprocalSqrt( length2 ) );
            }
            else
            {
                return zeroLengthResult;
            }
        }
        
        /// Vector Normalise - safe version with additional parameter to use for zero length vectors!
        void SafeNormalise( const HVec4& zeroLengthResult = HVec4() )
        {
            const HF32 length2( Length2() );
            if ( length2.IsNonZero() )
            {
                operator *= ( HReciprocalSqrt( length2 ) );
            }
            else
            {
                operator = ( zeroLengthResult );
            }
        }
        
        // Vector * Matrix Multiplication
        const HVec4 operator * ( const HMat44& m ) const;
        
        //
        // Comparison Operators
        //
        
        const HVec4 operator == ( const HVec4& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_cmpeq_ps( m_V, src.m_V ) );
            #else
            return HVec4( m_V[VX] == src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] == src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] == src.m_V[VZ] ? c_fSet : c_fClear,
                          m_V[VW] == src.m_V[VW] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec4 operator != ( const HVec4& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_cmpneq_ps( m_V, src.m_V ) );
            #else
            return HVec4( m_V[VX] != src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] != src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] != src.m_V[VZ] ? c_fSet : c_fClear,
                          m_V[VW] != src.m_V[VW] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec4 operator <= ( const HVec4& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_cmple_ps( m_V, src.m_V ) );
            #else
            return HVec4( m_V[VX] <= src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] <= src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] <= src.m_V[VZ] ? c_fSet : c_fClear,
                          m_V[VW] <= src.m_V[VW] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec4 operator < ( const HVec4& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_cmplt_ps( m_V, src.m_V ) );
            #else
            return HVec4( m_V[VX] < src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] < src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] < src.m_V[VZ] ? c_fSet : c_fClear,
                          m_V[VW] < src.m_V[VW] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec4 operator >= ( const HVec4& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_cmpge_ps( m_V, src.m_V ) );
            #else
            return HVec4( m_V[VX] >= src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] >= src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] >= src.m_V[VZ] ? c_fSet : c_fClear,
                          m_V[VW] >= src.m_V[VW] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec4 operator > ( const HVec4& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_cmpgt_ps( m_V, src.m_V ) );
            #else
            return HVec4( m_V[VX] > src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] > src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] > src.m_V[VZ] ? c_fSet : c_fClear,
                          m_V[VW] > src.m_V[VW] ? c_fSet : c_fClear );
            #endif
        }
        
        const E_SignBits GetSignBits() const
        {
            #ifdef _SSE_
            return (E_SignBits)( _mm_movemask_ps( m_V ) );
            #else
            return (E_SignBits)( ( ( (U32&)m_V[VX] >> 31 ) & 0x1 ) |
                                 ( ( (U32&)m_V[VY] >> 30 ) & 0x2 ) |
                                 ( ( (U32&)m_V[VZ] >> 29 ) & 0x4 ) |
                                 ( ( (U32&)m_V[VW] >> 28 ) & 0x8 ) );
            #endif
        }
#ifdef WIN32
        const S32 Finite() const
        {
            return _finite( X() ) &&
                   _finite( Y() ) &&
                   _finite( Z() ) &&
                   _finite( W() );
        }
#endif
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s HVec4( %+8.3f, %+8.3f, %+8.3f, %+8.3f )\n", text, (F32)X(), (F32)Y(), (F32)Z(), (F32)W() );
            OutputDebugString( buffer );
        }
        #endif
        
        // Support for Ed's MVector class
        
        typedef F32 DataType;
        const static int SIZE = 4;
	    typedef HVec4& type1; ///< This type is used by MVector to form a constructor.
	    typedef void /*Ut::NullType*/ type2; ///< What on earth is this used for???
        
        #ifdef _SSE_
        template < unsigned Index > const VectorAccessor<HVec4>         At()        { return VectorAccessor<HVec4>( *this, Index ); }
        template < unsigned Index > const VectorConstAccessor<HVec4>    At() const  { return VectorConstAccessor<HVec4>( *this, Index ); }
        #else
        template < unsigned Index > F32&                                At()        { return m_V[ Index ]; }
        template < unsigned Index > const F32                           At() const  { return m_V[ Index ]; }
        #endif
        
        #ifdef _SSE_
        FORCE_INLINE const VectorAccessor<HVec4>       operator [] ( const U32 index )         { return VectorAccessor<HVec4>( *this, index ); }
        FORCE_INLINE const VectorConstAccessor<HVec4>  operator [] ( const U32 index ) const   { return VectorConstAccessor<HVec4>( *this, index ); }
        #else
        FORCE_INLINE F32&                              operator [] ( const U32 index )         { return m_V[ index ]; }
        FORCE_INLINE const F32                         operator [] ( const U32 index ) const   { return m_V[ index ]; }
        #endif
        
        typedef void CrossProductType;
        
    private:
        
        #ifdef _SSE_
        V128    m_V;
        #else
        F32     m_V[4];
        #endif
    };
    
    
    
    
    
    
    //
    // Scalar, Vector & Vertex Base Types
    //

    class C_Scalar;
    class C_Vertex;
    class C_Vector;
    class C_Normal;

    class C_Scalar
    {
    public:
        
        /// Default Constructor
        C_Scalar() : m_F( 0.0f )
        {
        }
        
        /// Copy Constructor
        C_Scalar( const C_Scalar& src ) : m_F( src.m_F )
        {
        }
        
        /// HF32 Constructor
        C_Scalar( const HF32& src ) : m_F( src )
        {
        }
        
        /// HF32 Cast
        operator const HF32 () const
        {
            return m_F;
        }
        
        const bool operator <  ( const HF32& src ) const    { return m_F < src; }
        const bool operator >  ( const HF32& src ) const    { return m_F > src; }
        const bool operator <= ( const HF32& src ) const    { return m_F <= src; }
        const bool operator >= ( const HF32& src ) const    { return m_F >= src; }
        const bool operator == ( const HF32& src ) const    { return m_F == src; }
        const bool operator != ( const HF32& src ) const    { return m_F != src; }
        
    protected:
        
        F32     m_F;
    };


    class C_Origin
    {
    public:
        
        /// Addition: Origin + Vector --> Vertex
        const C_Vertex operator + ( const C_Vector& src ) const;
        
        /// Subtraction: Origin - Vertex --> Vector
        const C_Vector operator - ( const C_Vertex& src ) const;
    };


    class C_Vertex
    {
    friend class C_Origin;
    friend class C_Vector;
    friend class C_Normal;
    public:
        
        /// Default Constructor
        C_Vertex()                                                                  {}
        
        /// Copy Constructor
        C_Vertex( const C_Vertex& src ) : m_V( src.m_V )                            {}
        
        /// Copy Assignment
        const C_Vertex& operator = ( const C_Vertex& src )                          { m_V = src.m_V; return *this; }
        
        /// Origin Constructor
        C_Vertex( const C_Origin& src ) : m_V( c_Zero )                             {}
        
        /// Origin Assignment
        const C_Vertex& operator = ( const C_Origin& src )                          { m_V = HVec3( c_Zero ); return *this; }
        
        /// Explicit Vector Constructor
        explicit C_Vertex( const C_Vector& src );
        
        /// HVec3 Constructor
        C_Vertex( const HVec3& src ) : m_V( src )                                   {}
        
        /// HVec3 Cast
        operator const HVec3 () const                                               { return m_V; }
        
        /// HVec4 Constructor
        explicit C_Vertex( const HVec4& src ) : m_V( HVec3( src ) )                 {}
        
        /// HF32 XYZ Constructor
        C_Vertex( const HF32& x, const HF32& y, const HF32& z ) : m_V( x, y, z )    {}
        
        /// F32 Array Constructor
        C_Vertex( const F32* xyz ) : m_V( xyz )                                     {}
        
        /// HF32 XYZ Broadcast Constructor
        explicit C_Vertex( const HF32& xyz ) : m_V( xyz )                           {}
        
        /// Accessor: X
        const HF32 X() const                                                        { return m_V.X(); }
        
        /// Accessor: Y
        const HF32 Y() const                                                        { return m_V.Y(); }
        
        /// Accessor: Z
        const HF32 Z() const                                                        { return m_V.Z(); }
        
        /// Addition: Vertex + Vertex --> Vertex
        const C_Vertex operator + ( const C_Vertex& src ) const                     { return C_Vertex( m_V + src.m_V ); }
        
        /// Addition: Vertex + Vector --> Vertex
        const C_Vertex operator + ( const C_Vector& src ) const;
        
        /// Addition: Vertex + Normal --> Vertex
        const C_Vertex operator + ( const C_Normal& src ) const;
        
        /// Addition: Vertex += Vertex
        C_Vertex& operator += ( const C_Vertex& src )                               { m_V += src.m_V; return *this; }
        
        /// Subtraction: Vertex - Vertex --> Vector
        const C_Vector operator - ( const C_Vertex& src ) const;
        
        /// Subtraction: Vertex - Vector --> Vertex
        const C_Vertex operator - ( const C_Vector& src ) const;
        
        /// Subtraction: Vertex - Normal --> Vertex
        const C_Vertex operator - ( const C_Normal& src ) const;
        
        /// Subtraction: Vertex - Origin --> Vector
        const C_Vector operator - ( const C_Origin& src ) const;
        
        /// Multiplication: Vertex * HF32 --> Vertex
        const C_Vertex operator * ( const HF32& scalar ) const                      { return C_Vertex( m_V * scalar ); }
        
        /// Multiplication: Vertex * HMat43 --> Vertex
        const C_Vertex operator * ( const HMat43& matrix ) const                    { return C_Vertex( m_V * matrix ); }
        
        /// Multiplication: Vertex *= HF32
        C_Vertex& operator *= ( const HF32& scalar )                                { m_V *= scalar; return *this; }
        
        /// Minimum: Vertex min Vertex --> Vertex
        const C_Vertex Min( const C_Vertex& src ) const                             { return C_Vertex( m_V.Min( src.m_V ) ); }
        
        /// Minimum: Vertex max Vertex --> Vertex
        const C_Vertex Max( const C_Vertex& src ) const                             { return C_Vertex( m_V.Max( src.m_V ) ); }
        
        /// Distance
        const HF32 Distance( const C_Vertex& src ) const                            { return m_V.Distance( src.m_V ); }
        
        /// Lerp
        const C_Vertex Lerp( const C_Vertex& src, const HF32& t ) const             { return C_Vertex( m_V.Lerp( src.m_V, t ) ); }
        
        /// Comparison - Less Than
        const E_SignBits operator < ( const C_Vertex& src ) const                   { return ( m_V < src.m_V ).GetSignBits(); }
        
        /// Comparison - Less Equal
        const E_SignBits operator <= ( const C_Vertex& src ) const                  { return ( m_V <= src.m_V ).GetSignBits(); }
        
        /// Comparison - Greater Than
        const E_SignBits operator > ( const C_Vertex& src ) const                   { return ( m_V > src.m_V ).GetSignBits(); }
        
        /// Comparison - Greater Equal
        const E_SignBits operator >= ( const C_Vertex& src ) const                  { return ( m_V >= src.m_V ).GetSignBits(); }
        
        /// Comparison - Equal
        const E_SignBits operator == ( const C_Vertex& src ) const                  { return ( m_V == src.m_V ).GetSignBits(); }
        
        /// Comparison - Not Equal
        const E_SignBits operator != ( const C_Vertex& src ) const                  { return ( m_V != src.m_V ).GetSignBits(); }
#ifdef WIN32
        /// Finite test
        const S32 Finite() const                                                    { return m_V.Finite(); }
#endif
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s C_Vertex( %+11.6f, %+11.6f, %+11.6f )\n", text, (F32)m_V.X(), (F32)m_V.Y(), (F32)m_V.Z() );
            OutputDebugString( buffer );
        }
        #endif
        
    protected:
        
        HVec3   m_V;
    };


    class C_Vector
    {
    friend class C_Origin;
    friend class C_Vertex;
    friend class C_Normal;
    public:
        
        /// Default Constructor
        C_Vector()                                                                  {}
        
        /// Copy Constructor
        C_Vector( const C_Vector& src ) : m_V( src.m_V )                            {}
        
        /// Copy Assignment
        const C_Vector& operator = ( const C_Vector& src )                          { m_V = src.m_V; return *this; }
        
        /// Explicit Vertex Constructor
        explicit C_Vector( const C_Vertex& src ) : m_V( src.m_V )                   {}
        
        /// Explicit Normal Constructor
        explicit C_Vector( const C_Normal& src );
        
        /// HVec3 Constructor
        C_Vector( const HVec3& src ) : m_V( src )                                   {}
        
        /// HVec3 Cast
        operator const HVec3 () const                                               { return m_V; }
        
        /// HVec4 Constructor
        explicit C_Vector( const HVec4& src ) : m_V( HVec3( src ) )                 {}
        
        /// HF32 XYZ Constructor
        C_Vector( const HF32& x, const HF32& y, const HF32& z ) : m_V( x, y, z )    {}
        
        /// F32 Array Constructor
        C_Vector( const F32* xyz ) : m_V( xyz )                                     {}
        
        /// HF32 XYZ Broadcast Constructor
        explicit C_Vector( const HF32& xyz ) : m_V( xyz )                           {}
        
        /// Accessor: X
        const HF32 X() const                                                        { return m_V.X(); }
        
        /// Accessor: Y
        const HF32 Y() const                                                        { return m_V.Y(); }
        
        /// Accessor: Z
        const HF32 Z() const                                                        { return m_V.Z(); }
        
        /// Negation: Vector --> Vector
        const C_Vector operator - () const                                          { return C_Vector( -m_V ); }
        
        /// Addition: Vector + Vector --> Vector
        const C_Vector operator + ( const C_Vector& src ) const                     { return C_Vector( m_V + src.m_V ); }
        
        /// Addition: Vector + Vertex --> Vertex
        const C_Vertex operator + ( const C_Vertex& src ) const                     { return C_Vertex( m_V + src.m_V ); }
        
        /// Addition: Vector + Origin --> Vertex
        const C_Vertex operator + ( const C_Origin& src ) const                     { return C_Vertex( m_V ); }
        
        /// Subtraction: Vector - Vector --> Vector
        const C_Vector operator - ( const C_Vector& src ) const                     { return C_Vector( m_V - src.m_V ); }
        
        /// Multiplication: Vector * HF32 --> Vertex
        const C_Vector operator * ( const HF32& scalar ) const                      { return C_Vector( m_V * scalar ); }
        
        /// Multiplication: Vector * HMat33 --> Vector
        const C_Vector operator * ( const HMat33& matrix ) const                    { return C_Vector( m_V * matrix ); }
        
        /// Dot Product: Vector | Vector --> HF32
        const HF32 operator | ( const C_Vector& src ) const                         { return m_V | src.m_V; }
        
        /// Dot Product: Vector | Normal --> HF32
        const HF32 operator | ( const C_Normal& src ) const;
        
        /// Cross Product: Vector ^ Vector --> Vector
        const C_Vector operator ^ ( const C_Vector& src ) const                     { return C_Vector( m_V ^ src.m_V ); }
        
        /// Cross Product: Vector ^ Normal --> Vector
        const C_Vector operator ^ ( const C_Normal& src ) const;
        
        /// Normalised
        const C_Normal Normalised() const;
        
        /// Normalised
        const C_Normal SafeNormalised( const C_Normal& zeroLengthResult ) const;
        
        /// Vector Length Squared
        const HF32 LengthSquared() const                                            { return m_V.LengthSquared(); }
        
        /// Vector Length
        const HF32 Length() const                                                   { return m_V.Length(); }
        
        /// Get components of this vector parallel & perpendicular to input normal
        void ParaPerp( const C_Normal& normal, C_Vector& para, C_Vector& perp ) const;
        
        /// Comparison - Less Than
        const E_SignBits operator < ( const C_Vector& src ) const                   { return ( m_V < src.m_V ).GetSignBits(); }
        
        /// Comparison - Less Equal
        const E_SignBits operator <= ( const C_Vector& src ) const                  { return ( m_V <= src.m_V ).GetSignBits(); }
        
        /// Comparison - Greater Than
        const E_SignBits operator > ( const C_Vector& src ) const                   { return ( m_V > src.m_V ).GetSignBits(); }
        
        /// Comparison - Greater Equal
        const E_SignBits operator >= ( const C_Vector& src ) const                  { return ( m_V >= src.m_V ).GetSignBits(); }
        
        /// Comparison - Equal
        const E_SignBits operator == ( const C_Vector& src ) const                  { return ( m_V == src.m_V ).GetSignBits(); }
        
        /// Comparison - Not Equal
        const E_SignBits operator != ( const C_Vector& src ) const                  { return ( m_V != src.m_V ).GetSignBits(); }
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s C_Vector( %+11.6f, %+11.6f, %+11.6f )\n", text, (F32)m_V.X(), (F32)m_V.Y(), (F32)m_V.Z() );
            OutputDebugString( buffer );
        }
        #endif
        
        // Support for Ed's MVector class
        
        //typedef F32 DataType;
        //const static int SIZE = 3;
	    //typedef C_Vector& type1; ///< This type is used by MVector to form a constructor.
	    //typedef Ut::NullType type2; ///< What on earth is this used for???
        
    protected:
        
        HVec3   m_V;
    };


    class C_Normal
    {
    friend class C_Vertex;
    friend class C_Vector;
    friend class C_Plane;
    public:
        
        /// Default Constructor
        C_Normal() : m_V( c_UnitZ )                                                 {}
        
        /// Copy Constructor
        C_Normal( const C_Normal& src ) : m_V( src.m_V )                            {}
        
        /// Copy Assignment
        const C_Normal& operator = ( const C_Normal& src )                          { m_V = src.m_V; return *this; }
        
        /// Vector Constructor
        C_Normal( const C_Vector& src ) : m_V( src.m_V.Normalised() )               {}
        
        /// Vector Assignment
        const C_Normal& operator = ( const C_Vector& src )                          { m_V = src.Normalised().m_V; return *this; }
        
        /// HVec3 Constructor
        explicit C_Normal( const HVec3& src ) : m_V( src.Normalised() )             {}
        
        /// HVec3 Cast
        operator const HVec3 () const                                               { return m_V; }
        
        /// HVec4 Constructor
        explicit C_Normal( const HVec4& src ) : m_V( HVec3( src ).Normalised() )    {}
        
        /// HF32 XYZ Constructor
        C_Normal( const HF32& x, const HF32& y, const HF32& z ) : m_V( HVec3( x, y, z ).Normalised() ) {}
        
        /// Accessor: X
        const HF32 X() const                                                        { return m_V.X(); }
        
        /// Accessor: Y
        const HF32 Y() const                                                        { return m_V.Y(); }
        
        /// Accessor: Z
        const HF32 Z() const                                                        { return m_V.Z(); }
        
        /// Negation: Normal --> Normal
        const C_Normal operator - () const                                          { return C_Normal( -m_V ); }
        
        /// Multiplication: Normal * HF32 -> Vector
        const C_Vector operator * ( const HF32& scalar ) const                      { return C_Vector( m_V * scalar ); }
        
        /// Multiplication: Normal * HMat33 --> Vector
        const C_Vector operator * ( const HMat33& matrix ) const                    { return C_Vector( m_V * matrix ); }
        
        /// Dot Product: Normal | Vector --> HF32
        const HF32 operator | ( const C_Vector& src ) const                         { return m_V | src.m_V; }
        
        /// Dot Product: Normal | Normal --> HF32
        const HF32 operator | ( const C_Normal& src ) const                         { return m_V | src.m_V; }
        
        /// Cross Product: Normal ^ Normal --> Vector
        const C_Vector operator ^ ( const C_Normal& src ) const                     { return C_Vector( m_V ^ src.m_V ); }
        
        /// Cross Product: Normal ^ Vector --> Vector
        const C_Vector operator ^ ( const C_Vector& src ) const                     { return C_Vector( m_V ^ src.m_V ); }
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s C_Normal( %+11.6f, %+11.6f, %+11.6f )\n", text, (F32)m_V.X(), (F32)m_V.Y(), (F32)m_V.Z() );
            OutputDebugString( buffer );
        }
        #endif
        
        // Support for Ed's MVector class
        
        //typedef F32 DataType;
        //const static int SIZE = 3;
	    //typedef C_Normal& type1; ///< This type is used by MVector to form a constructor.
	    //typedef Ut::NullType type2; ///< What on earth is this used for???
        
    protected:
        
        HVec3   m_V;
    };
    
    //
    // Origin Class Functions
    //

    /// Addition: Origin + Vector --> Vertex
    FORCE_INLINE const C_Vertex C_Origin::operator + ( const C_Vector& src ) const
    {
        return C_Vertex( src.m_V );
    }

    /// Subtraction: Origin - Vertex --> Vector
    FORCE_INLINE const C_Vector C_Origin::operator - ( const C_Vertex& src ) const
    {
        return C_Vector( src.m_V );
    }

    //
    // Vertex Class Functions
    //

    /// Explicit Vertex( Vector ) Constructor
    FORCE_INLINE C_Vertex::C_Vertex( const C_Vector& src ) :
        m_V( src.m_V )
    {
    }

    /// Vertex Addition: Vertex + Vector --> Vertex
    FORCE_INLINE const C_Vertex C_Vertex::operator + ( const C_Vector& src ) const
    {
        return C_Vertex( m_V + src.m_V );
    }

    /// Vertex Addition: Vertex + Normal --> Vertex
    FORCE_INLINE const C_Vertex C_Vertex::operator + ( const C_Normal& src ) const
    {
        return C_Vertex( m_V + src.m_V );
    }

    /// Vertex Subtraction: Vertex - Vertex --> Vector
    FORCE_INLINE const C_Vector C_Vertex::operator - ( const C_Vertex& src ) const
    {
        return C_Vector( m_V - src.m_V );
    }

    /// Vertex Subtraction: Vertex - Vector --> Vertex
    FORCE_INLINE const C_Vertex C_Vertex::operator - ( const C_Vector& src ) const
    {
        return C_Vertex( m_V - src.m_V );
    }

    /// Vertex Subtraction: Vertex - Normal --> Vertex
    FORCE_INLINE const C_Vertex C_Vertex::operator - ( const C_Normal& src ) const
    {
        return C_Vertex( m_V - src.m_V );
    }

    /// Vertex Subtraction: Vertex - Origin --> Vector
    FORCE_INLINE const C_Vector C_Vertex::operator - ( const C_Origin& src ) const
    {
        return C_Vector( m_V );
    }

    //
    // Vector Class Functions
    //

    /// Explicit Normal Constructor
    FORCE_INLINE C_Vector::C_Vector( const C_Normal& src ) :
        m_V( src.m_V )
    {
    }

    /// Normalised
    FORCE_INLINE const C_Normal C_Vector::Normalised() const
    {
        return C_Normal( m_V ); // C_Normal constructor from HVec3 performs the normalise op!
    }
    
    /// Normalised
    FORCE_INLINE const C_Normal C_Vector::SafeNormalised( const C_Normal& zeroLengthResult ) const
    {
        const HF32 length2( LengthSquared() );
        if ( length2.IsNonZero() )
        {
            return (C_Normal&)operator * ( HReciprocalSqrt( length2 ) );
        }
        else
        {
            return zeroLengthResult;
        }
    }
    
    /// Dot Product: Vector | Normal --> HF32
    FORCE_INLINE const HF32 C_Vector::operator | ( const C_Normal& src ) const
    {
        return m_V | src.m_V;
    }
    
    /// Cross Product: Vector ^ Normal --> Vector
    FORCE_INLINE const C_Vector C_Vector::operator ^ ( const C_Normal& src ) const
    {
        return C_Vector( m_V ^ src.m_V );
    }
    
    /// Get components of this vector parallel & perpendicular to input normal
    FORCE_INLINE void C_Vector::ParaPerp( const C_Normal& normal, C_Vector& para, C_Vector& perp ) const
    {
        const HF32 dp( operator | ( normal ) );
        para = normal * dp;
        perp = operator - ( para );
    }
    
    
    
    
    
    //
    // Rotation classes for matrix construction
    // (For use only as temporaries - do not use for storage)
    //
    
    class RotX
    {
    public:
        RotX( const F32 a ) : m_AngleR( a ) {}
        operator const F32 () const { return m_AngleR; }
        F32 m_AngleR;
    };
    
    class RotY
    {
    public:
        RotY( const F32 a ) : m_AngleR( a ) {}
        operator const F32 () const { return m_AngleR; }
        F32 m_AngleR;
    };
    
    class RotZ
    {
    public:
        RotZ( const F32 a ) : m_AngleR( a ) {}
        operator const F32 () const { return m_AngleR; }
        F32 m_AngleR;
    };
    
    //
    // Axis classes for matrix construction
    //
    
    class AxisX : public HVec3
    {
    public:
        AxisX( const HVec3& ax )
            #ifdef _SSE_
            : HVec3( ax )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = ax.m_V[VX];
            m_V[VY] = ax.m_V[VY];
            m_V[VZ] = ax.m_V[VZ];
            #endif
        }
    };
    
    class AxisY : public HVec3
    {
    public:
        AxisY( const HVec3& ay )
            #ifdef _SSE_
            : HVec3( ay )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = ay.m_V[VX];
            m_V[VY] = ay.m_V[VY];
            m_V[VZ] = ay.m_V[VZ];
            #endif
        }
    };
    
    class AxisZ : public HVec3
    {
    public:
        AxisZ( const HVec3& az )
            #ifdef _SSE_
            : HVec3( az )
            #endif
        {
            #ifndef _SSE_
            m_V[VX] = az.m_V[VX];
            m_V[VY] = az.m_V[VY];
            m_V[VZ] = az.m_V[VZ];
            #endif
        }
    };
    
    
    
    class C_MatrixScale
    {
    public:
        
        C_MatrixScale() {}
        C_MatrixScale( const HVec3& scale ) : m_Scale( scale ) {}
        C_MatrixScale( const HF32& sx, const HF32& sy, const HF32& sz ) : m_Scale( HVec3( sx, sy, sz ) ) {}
        void operator = ( const HVec3& scale ) { m_Scale = scale; }
        operator const HVec3 () const { return m_Scale; }
        const HF32 X() const { return m_Scale.X(); }
        const HF32 Y() const { return m_Scale.Y(); }
        const HF32 Z() const { return m_Scale.Z(); }
        HVec3 m_Scale;
    };
    
    class C_MatrixRotateRPH
    {
    public:
        
        C_MatrixRotateRPH() {}
        C_MatrixRotateRPH( const HVec3& rotate ) : m_Rotate( rotate ) {}
        C_MatrixRotateRPH( const HF32& r, const HF32& p, const HF32& h ) : m_Rotate( HVec3( r, p, h ) ) {}
        void operator = ( const HVec3& rotate ) { m_Rotate = rotate; }
        operator const HVec3 () const { return m_Rotate; }
        const RotZ Roll() const    { return RotZ( m_Rotate.X() ); }
        const RotX Pitch() const   { return RotX( m_Rotate.Y() ); }
        const RotY Heading() const { return RotY( m_Rotate.Z() ); }
        HVec3 m_Rotate;
    };

	class testKen
	{
	};
    
    class C_MatrixRotatePHR
    {
    public:
        
        C_MatrixRotatePHR() {}
        C_MatrixRotatePHR( const HVec3& rotate ) : m_Rotate( rotate ) {}
        C_MatrixRotatePHR( const HF32& p, const HF32& h, const HF32& r ) : m_Rotate( HVec3( p, h, r ) ) {}
        void operator = ( const HVec3& rotate ) { m_Rotate = rotate; }
        operator const HVec3 () const { return m_Rotate; }
        const RotX Pitch() const   { return RotX( m_Rotate.X() ); }
        const RotY Heading() const { return RotY( m_Rotate.Y() ); }
        const RotZ Roll() const    { return RotZ( m_Rotate.Z() ); }
        HVec3 m_Rotate;
    };
    
    class C_MatrixTranslate
    {
    public:
        
        C_MatrixTranslate() {}
        C_MatrixTranslate( const HVec3& translate ) : m_Translate( translate ) {}
        C_MatrixTranslate( const HF32& tx, const HF32& ty, const HF32& tz ) : m_Translate( HVec3( tx, ty, tz ) ) {}
        void operator = ( const HVec3& translate ) { m_Translate = translate; }
        operator const HVec3 () const { return m_Translate; }
        HVec3 m_Translate;
    };
    
    class C_MatrixSRT
    {
    public:
        
        C_MatrixSRT() {}
        
        C_MatrixSRT( const C_MatrixScale& scale, const C_MatrixRotateRPH& rotate, const C_MatrixTranslate& translate ) :
            m_Scale( scale ),
            m_Rotate( rotate ),
            m_Translate( translate )
        {
        }
        
        const C_MatrixScale         Scale() const       { return m_Scale; }
        const C_MatrixRotateRPH     Rotate() const      { return m_Rotate; }
        const C_MatrixTranslate     Translate() const   { return m_Translate; }
        
    private:
        
        C_MatrixScale               m_Scale;
        C_MatrixRotateRPH           m_Rotate;
        C_MatrixTranslate           m_Translate;
    };
    
    //
    // Hardware 3 Row * 3 Column Matrix Class
    //
    
    ALIGN16 class HMat33
    {
    friend class HVec3;
    friend class HVec4;
    friend class HMat43;
    friend class HMat44;
    friend class C_Quaternion;
    
    public:
        
        /// Default Constructor: Initialised to Identity
        FORCE_INLINE HMat33()
            #ifdef _SSE_
            : m_AX( c_UnitX ),
              m_AY( c_UnitY ),
              m_AZ( c_UnitZ )
            #endif
        {
            #ifndef _SSE_
            m_A[MX] = HVec3( c_UnitX );
            m_A[MY] = HVec3( c_UnitY );
            m_A[MZ] = HVec3( c_UnitZ );
            #endif
        }
        
        /// Copy Constructor
        HMat33( const HMat33& src )
            #ifdef _SSE_
            : m_AX( src.m_AX ),
              m_AY( src.m_AY ),
              m_AZ( src.m_AZ )
            #endif
        {
            #ifndef _SSE_
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
            #endif
        }
        
        /// Copy Assignment
        const HMat33& operator = ( const HMat33& copy )
        {
            #ifdef _SSE_
            m_AX = copy.m_AX;
            m_AY = copy.m_AY;
            m_AZ = copy.m_AZ;
            #else
            m_A[MX] = copy.m_A[MX];
            m_A[MY] = copy.m_A[MY];
            m_A[MZ] = copy.m_A[MZ];
            #endif
            return *this;
        }
        
        /// Accessor: Get Axis X
        const HVec3& AX() const
        {
            #ifdef _SSE_
            return m_AX;
            #else
            return m_A[MX];
            #endif
        }
        
        /// Accessor: Get Axis Y
        const HVec3& AY() const
        {
            #ifdef _SSE_
            return m_AY;
            #else
            return m_A[MY];
            #endif
        }
        
        /// Accessor: Get Axis Z
        const HVec3& AZ() const
        {
            #ifdef _SSE_
            return m_AZ;
            #else
            return m_A[MZ];
            #endif
        }
        
        /// Accessor: Set Axis X
        void AX( const HVec3& ax )
        {
            #ifdef _SSE_
            m_AX = ax;
            #else
            m_A[MX] = ax;
            #endif
        }
        
        /// Accessor: Set Axis Y
        void AY( const HVec3& ay )
        {
            #ifdef _SSE_
            m_AY = ay;
            #else
            m_A[MY] = ay;
            #endif
        }
        
        /// Accessor: Set Axis Z
        void AZ( const HVec3& az )
        {
            #ifdef _SSE_
            m_AZ = az;
            #else
            m_A[MZ] = az;
            #endif
        }
        
        #ifdef _SSE_
        const HVec3&    operator [] ( const U32 i ) const   { return (&m_AX)[i]; }
        HVec3&          operator [] ( const U32 i )         { return (&m_AX)[i]; }
        #else
        const HVec3&    operator [] ( const U32 i ) const   { return m_A[i]; }
        HVec3&          operator [] ( const U32 i )         { return m_A[i]; }
        #endif
        
    private:
        
        /// Private Accessor: Set Axes X, Y & Z
        #ifdef _SSE_
        void SetAXYZ( const V128& ax, const V128& ay, const V128& az )
        {
            m_AX.m_V = ax;
            m_AY.m_V = ay;
            m_AZ.m_V = az;
        }
	    #else
        void SetAXYZ( const HVec3& ax, const HVec3& ay, const HVec3& az )
        {
            m_A[MX] = ax;
            m_A[MY] = ay;
            m_A[MZ] = az;
        }
	    #endif
        
    public:
        
        // Explicit construction due to data loss!
        explicit HMat33( const HMat43& src );
        explicit HMat33( const HMat44& src );
        
        #ifdef _SSE_
        HMat33( const V128& ax, const V128& ay, const V128& az ) :
            m_AX( ax ),
            m_AY( ay ),
            m_AZ( az )
        {
        }
        #endif
        
        HMat33( const HVec3& ax, const HVec3& ay, const HVec3& az )
            #ifdef _SSE_
            : m_AX( ax ),
              m_AY( ay ),
              m_AZ( az )
            #endif
        {
            #ifndef _SSE_
            m_A[MX] = ax;
            m_A[MY] = ay;
            m_A[MZ] = az;
            #endif
        }
        
        // Rotation about X axis - clockwise when viewed back down the X axis
        HMat33( const RotX& alpha )
        {
            const F32 cx = cos( alpha );
            const F32 sx = sin( alpha );
            ConstV3F32( ay ) = { 0.0f,  cx, sx };
            ConstV3F32( az ) = { 0.0f, -sx, cx };
            SetAXYZ( c_UnitX, ay, az );
        }
        
        // Rotation about Y axis - clockwise when viewed back down the Y axis
        HMat33( const RotY& beta )
        {
            const F32 cy = cos( beta );
            const F32 sy = sin( beta );
            ConstV3F32( ax ) = { cy, 0.0f, -sy };
            ConstV3F32( az ) = { sy, 0.0f,  cy };
            SetAXYZ( ax, c_UnitY, az );
        }
        
        // Rotation about Z axis - clockwise when viewed back down the Z axis
        HMat33( const RotZ& gamma )
        {
            const F32 cz = cos( gamma );
            const F32 sz = sin( gamma );
            ConstV3F32( ax ) = {  cz, sz, 0.0f };
            ConstV3F32( ay ) = { -sz, cz, 0.0f };
            SetAXYZ( ax, ay, c_UnitZ );
        }
        
        /// Constructor - Scale matrix
        HMat33( const C_MatrixScale& scale )
        {
            SetAXYZ( HVecX( scale.X() ), HVecY( scale.Y() ), HVecZ( scale.Z() ) );
        }
        
        /// Constructor - Rotation matrix from euler angles - Roll-Pitch-Heading
        HMat33( const C_MatrixRotateRPH& euler )
        {
            // Matrix RPH =
            //            [col][X]                               [col][Y]        [col][Z]
            // [X][row](  cos(r)*cos(h) + sin(r)*sin(p)*sin(h),  sin(r)*cos(p), -cos(r)*sin(h) + sin(r)*sin(p)*cos(h) );
            // [Y][row]( -sin(r)*cos(h) + cos(r)*sin(p)*sin(h),  cos(r)*cos(p),  sin(r)*sin(h) + cos(r)*sin(p)*cos(h) );
            // [Z][row](  cos(p)*sin(h),                        -sin(p),         cos(p)*cos(h) );
            
            const F32 r = euler.Roll();
            const F32 p = euler.Pitch();
            const F32 h = euler.Heading();
            
            const HF32 cr( cos( r ) );
            const HF32 sr( sin( r ) );
            const HF32 cp( cos( p ) );
            const HF32 sp( sin( p ) );
            const HF32 ch( cos( h ) );
            const HF32 sh( sin( h ) );
            
            const HVec3 ax(  cr*ch + sr*sp*sh,  sr*cp, -cr*sh + sr*sp*ch );
            const HVec3 ay( -sr*ch + cr*sp*sh,  cr*cp,  sr*sh + cr*sp*ch );
            const HVec3 az(  cp*sh,            -sp,     cp*ch );
            
            SetAXYZ( ax, ay, az );
        }
        
        /// Conversion to Euler angles - Roll-Pitch-Heading
        const C_MatrixRotateRPH GetRotationRPH() const
        {
            // m[Z][Y] = -sin(p) --> p = asin( -m[Z][Y]
            
            #ifdef _SSE_
            const F32 sinPitch( -m_AZ.Y() );
            #else
            const F32 sinPitch( -m_A[MZ].Y() );
            #endif
            
            const HF32 p( asin( sinPitch ) );
            HF32 h;
            HF32 r;
            
            if ( fabs( sinPitch ) < 0.999f )
            {
                // m[X][Y] / m[Y][Y} = sin(r)cos(p) / cos(r)cos(p)
                //                   = sin(r) / cos(r)
                //                   = tan(r)
                // -->             r = atan( m[X][Y] / m[Y][Y] )
                
                // m[Z][X] / m[Z][Z] = cos(p)sin(h) / cos(p)cos(h)
                //                   = sin(h) / cos(h)
                //                   = tan(h)
                // -->             h = atan( m[Z][X] / m[Z][Z] )
                
                #ifdef _SSE_
                const F32 tanRollSin( m_AX.Y() );
                const F32 tanRollCos( m_AY.Y() );
                const F32 tanHeadingSin( m_AZ.X() );
                const F32 tanHeadingCos( m_AZ.Z() );
                #else
                const F32 tanRollSin( m_A[MX].Y() );
                const F32 tanRollCos( m_A[MY].Y() );
                const F32 tanHeadingSin( m_A[MZ].X() );
                const F32 tanHeadingCos( m_A[MZ].Z() );
                #endif
                
                h = (float)atan2( tanHeadingSin, tanHeadingCos );
                r = (float)atan2( tanRollSin, tanRollCos );
            }
            else
            {
                // sin(p) ~= 1 therefore cos(p) ~= 0
                // m[Y][X] / m[Y][Z] = -sin(r)cos(h) + cos(r)sin(p)sin(h) / sin(r)sin(h) + cos(r)sin(p)cos(h)
                //                   = sin(h)cos(r) - cos(h)sin(r)        / cos(h)cos(r) + sin(h)sin(r)
                // using    sin(x-y) = sin(x)cos(y) - cos(x)sin(y)
                // and      cos(x-y) = cos(x)cos(y) + sin(x)sin(y)
                //                   = sin(h-r)                           / cos(h-r)
                // [ at poles h equivalent to r therefore assume r = 0 ]
                //                   = sin(h) / cos(h)
                //                 h = atan( sin(h) / cos(h) )
                //                 h = atan( m[Y][X] / m[Y][Z]
                
                #ifdef _SSE_
                const F32 tanHeadingSin( m_AY.X() );
                const F32 tanHeadingCos( m_AY.Z() );
                #else
                const F32 tanHeadingSin( m_A[MY].X() );
                const F32 tanHeadingCos( m_A[MY].Z() );
                #endif
                
                h = (float)atan2( tanHeadingSin, tanHeadingCos );
                r = 0.0f;
            }
            
            return C_MatrixRotateRPH( r, p, h );
        }
        
        /// Constructor - Rotation matrix from euler angles - Pitch-Heading-Roll
        HMat33( const C_MatrixRotatePHR& euler )
        {
            // Matrix PHR =
            //           [col][X]                            [col][Y]                            [col][Z]
            // [X][row]( cos(h)*cos(r),                      cos(h)*sin(r),                      -sin(h)        )
            // [Y][row]( sin(p)*sin(h)*cos(r)-cos(p)*sin(r), sin(p)*sin(h)*sin(r)+cos(p)*cos(r),  sin(p)*cos(h) )
            // [Z][row]( cos(p)*sin(h)*cos(r)+sin(p)*sin(r), cos(p)*sin(h)*sin(r)-sin(p)*cos(r),  cos(p)*cos(h) )
            
            const F32 p = euler.Pitch();
            const F32 h = euler.Heading();
            const F32 r = euler.Roll();
            
            const HF32 cp( cos( p ) );
            const HF32 sp( sin( p ) );
            const HF32 ch( cos( h ) );
            const HF32 sh( sin( h ) );
            const HF32 cr( cos( r ) );
            const HF32 sr( sin( r ) );
            
            const HVec3 ax( ch*cr,          ch*sr,          -sh    );
            const HVec3 ay( sp*sh*cr-cp*sr, sp*sh*sr+cp*cr,  sp*ch );
            const HVec3 az( cp*sh*cr+sp*sr, cp*sh*sr-sp*cr,  cp*ch );
            
            SetAXYZ( ax, ay, az );
        }
        
        /// Conversion to Euler angles - Pitch-Heading-Roll
        const C_MatrixRotatePHR GetRotationPHR() const
        {
            // m[X][Z] = -sin(h) --> h = asin( -m[X][Z]
            
            #ifdef _SSE_
            const F32 sinHeading( -m_AX.Z() );
            #else
            const F32 sinHeading( -m_A[MX].Z() );
            #endif
            
            const HF32 h( asin( sinHeading ) );
            HF32 p;
            HF32 r;
            
            if ( fabs( sinHeading ) < 0.999f )
            {
                // m[Y][Z] / m[Z][Z} = sin(p)*cos(h) / cos(p)*cos(h)
                //                   = sin(p) / cos(p)
                //                   = tan(p)
                // -->             p = atan( m[Y][Z] / m[Z][Z] )
                
                // m[X][Y] / m[X][X] = cos(h)*sin(r) / cos(h)*cos(r)
                //                   = sin(r) / cos(r)
                //                   = tan(r)
                // -->             r = atan( m[X][Y] / m[X][X] )
                
                #ifdef _SSE_
                const F32 tanPitchSin( m_AY.Z() );
                const F32 tanPitchCos( m_AZ.Z() );
                const F32 tanRollSin( m_AX.Y() );
                const F32 tanRollCos( m_AX.X() );
                #else
                const F32 tanPitchSin( m_A[MY].Z() );
                const F32 tanPitchCos( m_A[MZ].Z() );
                const F32 tanRollSin( m_A[MX].Y() );
                const F32 tanRollCos( m_A[MX].X() );
                #endif
                
                p = (float)atan2( tanPitchSin, tanPitchCos );
                r = (float)atan2( tanRollSin, tanRollCos );
            }
            else
            {
                // sin(h) ~= 1 therefore cos(h) ~= 0
                // m[Y][X] / m[Z][X] = sin(p)sin(h)cos(r) - cos(p)sin(r) / cos(p)sin(h)cos(r) + sin(p)sin(r)
                //                   = sin(p)cos(r) - cos(p)sin(r)       / cos(p)cos(r) + sin(p)sin(r)
                // using    sin(x-y) = sin(x)cos(y) - cos(x)sin(y)
                // and      cos(x-y) = cos(x)cos(y) + sin(x)sin(y)
                //                   = sin(p-r)                          / cos(p-r)
                // [ at poles p equivalent to r therefore assume r = 0 ]
                //                   = sin(p) / cos(p)
                //                 p = atan( sin(p) / cos(p) )
                //                 p = atan( m[Y][X] / m[Z][X]
                
                #ifdef _SSE_
                const F32 tanPitchSin( m_AY.X() );
                const F32 tanPitchCos( m_AZ.X() );
                #else
                const F32 tanPitchSin( m_A[MY].X() );
                const F32 tanPitchCos( m_A[MZ].X() );
                #endif
                
                p = (float)atan2( tanPitchSin, tanPitchCos );
                r = 0.0f;
            }
            
            return C_MatrixRotatePHR( p, h, r );
        }
        
        // Cross products for deriving third matrix axis from 2 input axes
        // const HVec3 ax = ay ^ az;
        // const HVec3 ay = az ^ ax;
        // const HVec3 az = ax ^ ay;
        
        // Initialise with primary axis Z
        HMat33( const AxisZ& az )
        {
            const F32 cos45deg = ::sqrt( 2.0f ) / 2.0f;
            const HVec3 ay( c_UnitY );
            
            if ( abs( ay | az ) < cos45deg )
            {
                const HVec3 ax = ay ^ az;
                SetAXYZ( ax.Normalised(), ( az ^ ax ).Normalised(), az.Normalised() );
            }
            else
            {
                const HVec3 ax( c_UnitX );
                const HVec3 ay = az ^ ax;
                SetAXYZ( ( ay ^ az ).Normalised(), ay.Normalised(), az.Normalised() );
            }
        }
        
        // Initialise with primary axis X, secondary axis Y
        HMat33( const AxisX& ax, const AxisY& ay )
        {
            const HVec3 az = ax ^ ay;
            SetAXYZ( ax.Normalised(), ( az ^ ax ).Normalised(), az.Normalised() );
        }
        
        // Initialise with primary axis X, secondary axis Z
        HMat33( const AxisX& ax, const AxisZ& az )
        {
            const HVec3 ay = az ^ ax;
            SetAXYZ( ax.Normalised(), ay.Normalised(), ( ax ^ ay ).Normalised() );
        }
        
        // Initialise with primary axis Y, secondary axis X
        HMat33( const AxisY& ay, const AxisX& ax )
        {
            const HVec3 az = ax ^ ay;
            SetAXYZ( ( ay ^ az ).Normalised(), ay.Normalised(), az.Normalised() );
        }
        
        // Initialise with primary axis Y, secondary axis Z
        HMat33( const AxisY& ay, const AxisZ& az )
        {
            const HVec3 ax = ay ^ az;
            SetAXYZ( ax.Normalised(), ay.Normalised(), ( ax ^ ay ).Normalised() );
        }
        
        // Initialise with primary axis Z, secondary axis X
        HMat33( const AxisZ& az, const AxisX& ax )
        {
            const HVec3 ay = az ^ ax;
            SetAXYZ( ( ay ^ az ).Normalised(), ay.Normalised(), az.Normalised() );
        }
        
        // Initialise with primary axis Z, secondary axis Y
        HMat33( const AxisZ& az, const AxisY& ay )
        {
            const HVec3 ax = ay ^ az;
            SetAXYZ( ax.Normalised(), ( az ^ ax ).Normalised(), az.Normalised() );
        }
        
        void SetIdentity()
        {
            SetAXYZ( c_UnitX, c_UnitY, c_UnitZ );
        }
        
        const HMat33 Transposed() const
        {
            #ifdef _SSE_
            
            const V128 zero = _mm_setzero_ps();
            
            const V128 tmp0 = _mm_shuffle_ps( m_AX.m_V, m_AY.m_V, 0x44 );
            const V128 tmp1 = _mm_shuffle_ps( m_AZ.m_V, zero,     0x44 );
            const V128 tmp2 = _mm_shuffle_ps( m_AX.m_V, m_AY.m_V, 0xEE );
            const V128 tmp3 = _mm_shuffle_ps( m_AZ.m_V, zero,     0xEE );
            
            const V128 ax = _mm_shuffle_ps( tmp0, tmp1, 0x88 );
            const V128 ay = _mm_shuffle_ps( tmp0, tmp1, 0xDD );
            const V128 az = _mm_shuffle_ps( tmp2, tmp3, 0x88 );
            
            return HMat33( ax, ay, az );
            
            #else
            
            const HVec3 ax( m_A[MX][VX], m_A[MY][VX], m_A[MZ][VX] );
            const HVec3 ay( m_A[MX][VY], m_A[MY][VY], m_A[MZ][VY] );
            const HVec3 az( m_A[MX][VZ], m_A[MY][VZ], m_A[MZ][VZ] );
            
            return HMat33( ax, ay, az );
            
            #endif
        }
        
        void Transpose()
        {
            #ifdef _SSE_
            
            const V128 zero = _mm_setzero_ps();
            
            const V128 tmp0 = _mm_shuffle_ps( m_AX.m_V, m_AY.m_V, 0x44 );
            const V128 tmp1 = _mm_shuffle_ps( m_AZ.m_V, zero,     0x44 );
            const V128 tmp2 = _mm_shuffle_ps( m_AX.m_V, m_AY.m_V, 0xEE );
            const V128 tmp3 = _mm_shuffle_ps( m_AZ.m_V, zero,     0xEE );
            
            m_AX.m_V = _mm_shuffle_ps( tmp0, tmp1, 0x88 );
            m_AY.m_V = _mm_shuffle_ps( tmp0, tmp1, 0xDD );
            m_AZ.m_V = _mm_shuffle_ps( tmp2, tmp3, 0x88 );
            
            #else
            
            const HVec3 ax( m_A[MX][VX], m_A[MY][VX], m_A[MZ][VX] );
            const HVec3 ay( m_A[MX][VY], m_A[MY][VY], m_A[MZ][VY] );
            const HVec3 az( m_A[MX][VZ], m_A[MY][VZ], m_A[MZ][VZ] );
            
            m_A[MX] = ax;
            m_A[MY] = ay;
            m_A[MZ] = az;
            
            #endif
        }
        
        // Matrix Multiply
        const HMat33 operator * ( const HMat33& m ) const
        {
            #ifdef _SSE_
            return HMat33( m_AX * m,
                           m_AY * m,
                           m_AZ * m );
            #else
            return HMat33( m_A[MX] * m,
                           m_A[MY] * m,
                           m_A[MZ] * m );
            #endif
        }
        
        // Combine with HVec3 position to form a HMat43
        //const HMat43 operator + ( const HVec3& pos ) const;
        const HMat43 operator || ( const HVec3& pos ) const;
        
        // Inverse of Rotation.
        // Not valid for matrices with scaling/skewing etc!
        const HMat33 InverseR() const
        {
            return Transposed();
        }
        
        // Matrix Determinant
        const HF32 Determinant() const
        {
            #ifdef _SSE_
            
            const V128 vDet = _mm_mul_ps( m_AX, _mm_sub_ps( _mm_mul_ps( Shuffle<VY,VX,VX,VW>( m_AY ), Shuffle<VZ,VZ,VY,VW>( m_AZ ) ),
                                                            _mm_mul_ps( Shuffle<VZ,VZ,VY,VW>( m_AY ), Shuffle<VY,VX,VX,VW>( m_AZ ) ) ) );
            
            const V128 sDet = _mm_add_ss( _mm_sub_ss( vDet, ReplicateY( vDet ) ), ReplicateZ( vDet ) );
            
            return HF32( _mm_and_ps( c_MaskX, sDet ) );
            
            #else
            
            return HF32( m_A[MX][VX] * ( m_A[MY][VY] * m_A[MZ][VZ] - m_A[MY][VZ] * m_A[MZ][VY] ) -
                         m_A[MX][VY] * ( m_A[MY][VX] * m_A[MZ][VZ] - m_A[MY][VZ] * m_A[MZ][VX] ) +
                         m_A[MX][VZ] * ( m_A[MY][VX] * m_A[MZ][VY] - m_A[MY][VY] * m_A[MZ][VX] ) );
            
            #endif
        }
        
        // Full Matrix Inverse - This will FAIL if determinant is zero!
        const HMat33 Inverse() const
        {
            const HF32 det = Determinant();
            
            //if ( det != 0.0f )
            if ( HIsNonZero( det ) )
            {
                const HF32 reciprocalDet = HReciprocal( det );
                
                #ifdef _SSE_
                
                const HVec3 ax( _mm_sub_ps( _mm_mul_ps( Shuffle<VY,VZ,VY,VW>( m_AY, m_AX, m_AX ),
                                                        Shuffle<VZ,VY,VZ,VW>( m_AZ, m_AZ, m_AY ) ),
                                            _mm_mul_ps( Shuffle<VZ,VY,VZ,VW>( m_AY, m_AX, m_AX ),
                                                        Shuffle<VY,VZ,VY,VW>( m_AZ, m_AZ, m_AY ) ) ) );
                
                const HVec3 ay( _mm_sub_ps( _mm_mul_ps( Shuffle<VZ,VX,VZ,VW>( m_AY, m_AX, m_AX ),
                                                        Shuffle<VX,VZ,VX,VW>( m_AZ, m_AZ, m_AY ) ),
                                            _mm_mul_ps( Shuffle<VX,VZ,VX,VW>( m_AY, m_AX, m_AX ),
                                                        Shuffle<VZ,VX,VZ,VW>( m_AZ, m_AZ, m_AY ) ) ) );
                
                const HVec3 az( _mm_sub_ps( _mm_mul_ps( Shuffle<VX,VY,VX,VW>( m_AY, m_AX, m_AX ),
                                                        Shuffle<VY,VX,VY,VW>( m_AZ, m_AZ, m_AY ) ),
                                            _mm_mul_ps( Shuffle<VY,VX,VY,VW>( m_AY, m_AX, m_AX ),
                                                        Shuffle<VX,VY,VX,VW>( m_AZ, m_AZ, m_AY ) ) ) );
                
                #else
                
                const HVec3 ax( m_A[MY][VY] * m_A[MZ][VZ] - m_A[MY][VZ] * m_A[MZ][VY],
                                m_A[MX][VZ] * m_A[MZ][VY] - m_A[MX][VY] * m_A[MZ][VZ],
                                m_A[MX][VY] * m_A[MY][VZ] - m_A[MX][VZ] * m_A[MY][VY] );
                                                                           
                const HVec3 ay( m_A[MY][VZ] * m_A[MZ][VX] - m_A[MY][VX] * m_A[MZ][VZ],
                                m_A[MX][VX] * m_A[MZ][VZ] - m_A[MX][VZ] * m_A[MZ][VX],
                                m_A[MX][VZ] * m_A[MY][VX] - m_A[MX][VX] * m_A[MY][VZ] );
                                                                           
                const HVec3 az( m_A[MY][VX] * m_A[MZ][VY] - m_A[MY][VY] * m_A[MZ][VX],
                                m_A[MX][VY] * m_A[MZ][VX] - m_A[MX][VX] * m_A[MZ][VY],
                                m_A[MX][VX] * m_A[MY][VY] - m_A[MX][VY] * m_A[MY][VX] );
                
                #endif
                
                return HMat33( ax * reciprocalDet,
                               ay * reciprocalDet,
                               az * reciprocalDet );
            }
            else
            {
                // Failed to calculate an inverse!
                #ifdef _DEBUG
                #ifdef SHOOTER_VECMAT_DEBUG_PRINT
                Print( "HMat33::Inverse(): Failed to calculate inverse for matrix:" );
                #endif
                #endif
                return HMat33( c_Zero, c_Zero, c_Zero );
            }
        }
        
        const HMat33 NormalisedZY() const
        {
            #ifdef _SSE_
            const HVec3 az( m_AZ.Normalised() );
            const HVec3 ax( ( m_AY ^ az ).Normalised() );
            const HVec3 ay( az ^ ax );
            #else
            const HVec3 az( m_A[MZ].Normalised() );
            const HVec3 ax( ( m_A[MY] ^ az ).Normalised() );
            const HVec3 ay( az ^ ax );
            #endif
            return HMat33( ax, ay, az );
        }
        
        void Align( F32 a, const HVec3& vNormal );
        
        #if ( DIRECT3D_VERSION >= 0x0900 )
        
        // Convert to D3DMATRIX
        operator const D3DMATRIX () const;
        operator const D3DXMATRIX () const;
        
        #endif
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            if ( text != "" )
            {
                OutputDebugString( text );
                OutputDebugString( "\n" );
            }
            AX().Print( "AX" );
            AY().Print( "AY" );
            AZ().Print( "AZ" );
            OutputDebugString( "\n" );
        }
        #endif
        
    private:
        
        #ifdef _SSE_
        HVec3 m_AX;
        HVec3 m_AY;
        HVec3 m_AZ;
        #else
        HVec3 m_A[3];
        #endif
    };
    
    
    //
    // Hardware 4 Row * 3 Column Matrix Class
    //
    
    ALIGN16 class HMat43
    {
    friend class HVec3;
    friend class HVec4;
    friend class HMat33;
    friend class HMat44;
    
    public:
        
        // Matrices are initialised to Identity by default constructor!
        FORCE_INLINE HMat43()
        {
            m_A[MX] = HVec3( c_UnitX );
            m_A[MY] = HVec3( c_UnitY );
            m_A[MZ] = HVec3( c_UnitZ );
            m_A[MW] = HVec3( c_Zero );
        }
        
        const HVec3& AX() const             { return m_A[MX]; }
        const HVec3& AY() const             { return m_A[MY]; }
        const HVec3& AZ() const             { return m_A[MZ]; }
        const HVec3& AW() const             { return m_A[MW]; }
        
        void AX( const HVec3& ax )          { m_A[MX] = ax; }
        void AY( const HVec3& ay )          { m_A[MY] = ay; }
        void AZ( const HVec3& az )          { m_A[MZ] = az; }
        void AW( const HVec3& aw )          { m_A[MW] = aw; }
        
        const HVec3&    operator [] ( const U32 i ) const   { return m_A[i]; }
        HVec3&          operator [] ( const U32 i )         { return m_A[i]; }
        
        explicit HMat43( const HMat33& src )
        {
            m_A[MX] = src.AX();
            m_A[MY] = src.AY();
            m_A[MZ] = src.AZ();
            m_A[MW] = HVec3( c_Zero );
        }
        
        HMat43( const HMat43& src )
        {
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
            m_A[MW] = src.m_A[MW];
        }
        
        // Explicit construction due to data loss!
        explicit HMat43( const HMat44& src );
        
        const HMat43& operator = ( const HMat33& src )
        {
            m_A[MX] = src.AX();
            m_A[MY] = src.AY();
            m_A[MZ] = src.AZ();
            m_A[MW] = HVec3( c_Zero );
            return *this;
        }
        
        const HMat43& operator = ( const HMat43& copy )
        {
            m_A[MX] = copy.m_A[MX];
            m_A[MY] = copy.m_A[MY];
            m_A[MZ] = copy.m_A[MZ];
            m_A[MW] = copy.m_A[MW];
            return *this;
        }
        
        #ifdef _SSE_
        HMat43( const V128& ax, const V128& ay, const V128& az, const V128& aw )
        {
            m_A[MX] = HVec3( ax );
            m_A[MY] = HVec3( ay );
            m_A[MZ] = HVec3( az );
            m_A[MW] = HVec3( aw );
        }
        #endif
        
        HMat43( const HVec3& ax, const HVec3& ay, const HVec3& az, const HVec3& aw )
        {
            m_A[MX] = ax;
            m_A[MY] = ay;
            m_A[MZ] = az;
            m_A[MW] = aw;
        }
        
        // Rotation about X axis - clockwise when viewed back down the X axis
        HMat43( const RotX& rx, const HVec3& aw = HVec3( c_Zero ) )
        {
            const F32 alpha = rx;
            const F32 cx = cos( alpha );
            const F32 sx = sin( alpha );
            ConstV3F32( ay ) = { 0.0f,  cx, sx };
            ConstV3F32( az ) = { 0.0f, -sx, cx };
            m_A[MX] = HVec3( c_UnitX );
            m_A[MY] = HVec3( ay );
            m_A[MZ] = HVec3( az );
            m_A[MW] = aw;
        }
        
        // Rotation about Y axis - clockwise when viewed back down the Y axis
        HMat43( const RotY& ry, const HVec3& aw = HVec3( c_Zero ) )
        {
            const F32 beta = ry;
            const F32 cy = cos( beta );
            const F32 sy = sin( beta );
            ConstV3F32( ax ) = { cy, 0.0f, -sy };
            ConstV3F32( az ) = { sy, 0.0f,  cy };
            m_A[MX] = HVec3( ax );
            m_A[MY] = HVec3( c_UnitY );
            m_A[MZ] = HVec3( az );
            m_A[MW] = aw;
        }
        
        // Rotation about Z axis - clockwise when viewed back down the Z axis
        HMat43( const RotZ& rz, const HVec3& aw = HVec3( c_Zero ) )
        {
            const F32 gamma = rz;
            const F32 cz = cos( gamma );
            const F32 sz = sin( gamma );
            ConstV3F32( ax ) = {  cz, sz, 0.0f };
            ConstV3F32( ay ) = { -sz, cz, 0.0f };
            m_A[MX] = HVec3( ax );
            m_A[MY] = HVec3( ay );
            m_A[MZ] = HVec3( c_UnitZ );
            m_A[MW] = aw;
        }
        
        // Cross products for deriving third matrix axis from 2 input axes
        // const HVec3 ax = ay ^ az;
        // const HVec3 ay = az ^ ax;
        // const HVec3 az = ax ^ ay;
        
        // Initialise with primary axis X, secondary axis Y
        HMat43( const AxisX& ax, const AxisY& ay, const HVec3& aw = HVec3( c_Zero ) )
        {
            const HVec3 az = ax ^ ay;
            m_A[MX] = ax.Normalised();
            m_A[MY] = ( az ^ ax ).Normalised();
            m_A[MZ] = az.Normalised();
            m_A[MW] = aw;
        }
        
        // Initialise with primary axis X, secondary axis Z
        HMat43( const AxisX& ax, const AxisZ& az, const HVec3& aw = HVec3( c_Zero ) )
        {
            const HVec3 ay = az ^ ax;
            m_A[MX] = ax.Normalised();
            m_A[MY] = ay.Normalised();
            m_A[MZ] = ( ax ^ ay ).Normalised();
            m_A[MW] = aw;
        }
        
        // Initialise with primary axis Y, secondary axis X
        HMat43( const AxisY& ay, const AxisX& ax, const HVec3& aw = HVec3( c_Zero ) )
        {
            const HVec3 az = ax ^ ay;
            m_A[MX] = ( ay ^ az ).Normalised();
            m_A[MY] = ay.Normalised();
            m_A[MZ] = az.Normalised();
            m_A[MW] = aw;
        }
        
        // Initialise with primary axis Y, secondary axis Z
        HMat43( const AxisY& ay, const AxisZ& az, const HVec3& aw = HVec3( c_Zero ) )
        {
            const HVec3 ax = ay ^ az;
            m_A[MX] = ax.Normalised();
            m_A[MY] = ay.Normalised();
            m_A[MZ] = ( ax ^ ay ).Normalised();
            m_A[MW] = aw;
        }
        
        // Initialise with primary axis Z, secondary axis X
        HMat43( const AxisZ& az, const AxisX& ax, const HVec3& aw = HVec3( c_Zero ) )
        {
            const HVec3 ay = az ^ ax;
            m_A[MX] = ( ay ^ az ).Normalised();
            m_A[MY] = ay.Normalised();
            m_A[MZ] = az.Normalised();
            m_A[MW] = aw;
        }
        
        /// Initialise with primary axis Z, secondary axis Y
        HMat43( const AxisZ& az, const AxisY& ay, const HVec3& aw = HVec3( c_Zero ) )
        {
            const HVec3 ax = ay ^ az;
            m_A[MX] = ax.Normalised();
            m_A[MY] = ( az ^ ax ).Normalised();
            m_A[MZ] = az.Normalised();
            m_A[MW] = aw;
        }
        
        HMat43( const C_MatrixSRT& srt )
        {
            const HMat33 scaleRotate( HMat33( srt.Scale() ) * HMat33( srt.Rotate() ) );
            #ifdef _SSE_
            m_A[MX] = scaleRotate.m_AX;
            m_A[MY] = scaleRotate.m_AY;
            m_A[MZ] = scaleRotate.m_AZ;
            #else
            m_A[MX] = scaleRotate.m_A[MX];
            m_A[MY] = scaleRotate.m_A[MY];
            m_A[MZ] = scaleRotate.m_A[MZ];
            #endif
            m_A[MW] = srt.Translate();
        }
        
        void SetIdentity()
        {
            m_A[MX] = HVec3( c_UnitX );
            m_A[MY] = HVec3( c_UnitY );
            m_A[MZ] = HVec3( c_UnitZ );
            m_A[MW] = HVec3( c_Zero );
        }
        
        const HMat43 Transposed() const
        {
            #ifdef _SSE_
            
            const V128 tmp0 = _mm_shuffle_ps( m_A[MX].m_V, m_A[MY].m_V, 0x44 );
            const V128 tmp1 = _mm_shuffle_ps( m_A[MZ].m_V, m_A[MW].m_V, 0x44 );
            const V128 tmp2 = _mm_shuffle_ps( m_A[MX].m_V, m_A[MY].m_V, 0xEE );
            const V128 tmp3 = _mm_shuffle_ps( m_A[MZ].m_V, m_A[MW].m_V, 0xEE );
            
            const V128 ax = _mm_and_ps( c_MaskXYZ, _mm_shuffle_ps( tmp0, tmp1, 0x88 ) );
            const V128 ay = _mm_and_ps( c_MaskXYZ, _mm_shuffle_ps( tmp0, tmp1, 0xDD ) );
            const V128 az = _mm_and_ps( c_MaskXYZ, _mm_shuffle_ps( tmp2, tmp3, 0x88 ) );
            
            return HMat43( ax, ay, az, _mm_setzero_ps() );
            
            #else
            
            const HMat43& m = *this;
            
            const HVec3 ax( m[MX][VX], m[MY][VX], m[MZ][VX] );
            const HVec3 ay( m[MX][VY], m[MY][VY], m[MZ][VY] );
            const HVec3 az( m[MX][VZ], m[MY][VZ], m[MZ][VZ] );
            
            return HMat43( ax, ay, az, HVec3( c_Zero ) );
            
            #endif
        }
        
        // Matrix Multiply
        const HMat43 operator * ( const HMat43& m ) const
        {
            const HMat33 rm( m );
            
            return HMat43( m_A[MX] * rm,
                           m_A[MY] * rm,
                           m_A[MZ] * rm,
                           m_A[MW] * m );
        }
        
        // Inverse of Rotation & Translation
        // Not valid for matrices with scaling/skewing etc!
        const HMat43 InverseRT() const
        {
            const HMat33 invRot = HMat33( *this ).Transposed();
            
            return HMat43( invRot.AX(),
                           invRot.AY(),
                           invRot.AZ(),
                           -m_A[MW] * invRot );
        }
        
        const C_MatrixSRT GetSRT() const
        {
            const HMat33 scaleRotate( *this );
            
            const C_MatrixRotateRPH rotate( scaleRotate.NormalisedZY().GetRotationRPH() );
            
            const HMat33 scaleMatrix( scaleRotate * HMat33( rotate ).Transposed() );
            
            #ifdef _SSE_
            const C_MatrixScale scale( scaleMatrix.m_AX.Length(),
                                       scaleMatrix.m_AY.Length(),
                                       scaleMatrix.m_AZ.Length() );
            #else
            const C_MatrixScale scale( scaleMatrix.m_A[MX].Length(),
                                       scaleMatrix.m_A[MY].Length(),
                                       scaleMatrix.m_A[MZ].Length() );
            #endif
            
            const C_MatrixTranslate translate( m_A[MW] );
            
            return C_MatrixSRT( scale, rotate, translate );
        }
        
        #if ( DIRECT3D_VERSION >= 0x0900 )
        
        // Convert to D3DMATRIX
        operator const D3DMATRIX () const;
        operator const D3DXMATRIX () const;
        
        #endif
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            if ( text != "" )
            {
                OutputDebugString( text );
                OutputDebugString( "\n" );
            }
            m_A[MX].Print( "AX" );
            m_A[MY].Print( "AY" );
            m_A[MZ].Print( "AZ" );
            m_A[MW].Print( "AW" );
            OutputDebugString( "\n" );
        }
        #endif
        
    private:
        
        //#ifdef _SSE_
        //HVec3 m_AX;
        //HVec3 m_AY;
        //HVec3 m_AZ;
        //HVec3 m_AW;
        //#else
        HVec3 m_A[4];
        //#endif
    };
    
    
    //
    // Hardware 4 Row * 4 Column Matrix Class
    //
    
    ALIGN16 class HMat44
    {
    friend class HVec3;
    friend class HVec4;
    friend class HMat33;
    friend class HMat43;
    
    public:
        
        /// Default Constructor: Initialise to Identity
        FORCE_INLINE HMat44()
            #ifdef _SSE_
            : m_AX( c_UnitX ),
              m_AY( c_UnitY ),
              m_AZ( c_UnitZ ),
              m_AW( c_UnitW )
            #endif
        {
            #ifndef _SSE_
            m_A[MX] = c_UnitX;
            m_A[MY] = c_UnitY;
            m_A[MZ] = c_UnitZ;
            m_A[MW] = c_UnitW;
            #endif
        }
        
        /// Accessors
        
        #ifdef _SSE_
        FORCE_INLINE const HVec4&  AX() const              { return m_AX; }
        FORCE_INLINE const HVec4&  AY() const              { return m_AY; }
        FORCE_INLINE const HVec4&  AZ() const              { return m_AZ; }
        FORCE_INLINE const HVec4&  AW() const              { return m_AW; }
        #else
        FORCE_INLINE const HVec4&  AX() const              { return m_A[MX]; }
        FORCE_INLINE const HVec4&  AY() const              { return m_A[MY]; }
        FORCE_INLINE const HVec4&  AZ() const              { return m_A[MZ]; }
        FORCE_INLINE const HVec4&  AW() const              { return m_A[MW]; }
        #endif
        
        #ifdef _SSE_
        FORCE_INLINE void          AX( const HVec4& ax )   { m_AX = ax; }
        FORCE_INLINE void          AY( const HVec4& ay )   { m_AY = ay; }
        FORCE_INLINE void          AZ( const HVec4& az )   { m_AZ = az; }
        FORCE_INLINE void          AW( const HVec4& aw )   { m_AW = aw; }
        #else
        FORCE_INLINE void          AX( const HVec4& ax )   { m_A[MX] = ax; }
        FORCE_INLINE void          AY( const HVec4& ay )   { m_A[MY] = ay; }
        FORCE_INLINE void          AZ( const HVec4& az )   { m_A[MZ] = az; }
        FORCE_INLINE void          AW( const HVec4& aw )   { m_A[MW] = aw; }
        #endif
        
        #ifdef _SSE_
        FORCE_INLINE const HVec4&   operator [] ( const U32 i ) const   { return (&m_AX)[i]; }
        FORCE_INLINE HVec4&         operator [] ( const U32 i )         { return (&m_AX)[i]; }
        #else
        FORCE_INLINE const HVec4&   operator [] ( const U32 i ) const   { return m_A[i]; }
        FORCE_INLINE HVec4&         operator [] ( const U32 i )         { return m_A[i]; }
        #endif
        
    private:
        
        /// Private Accessor: Set Axes X, Y, Z & W
        #ifdef _SSE_
        FORCE_INLINE void SetAXYZW( const V128& ax, const V128& ay, const V128& az, const V128& aw )
        {
            m_AX.m_V = ax;
            m_AY.m_V = ay;
            m_AZ.m_V = az;
            m_AW.m_V = aw;
        }
	    #else
        FORCE_INLINE void SetAXYZW( const HVec4& ax, const HVec4& ay, const HVec4& az, const HVec4& aw )
        {
            m_A[MX] = ax;
            m_A[MY] = ay;
            m_A[MZ] = az;
            m_A[MW] = aw;
        }
	    #endif
        
    public:
        
        /// Explicit Constructor from HMat33
        FORCE_INLINE explicit HMat44( const HMat33& src )
            #ifdef _SSE_
            : m_AX( src.m_AX ),
              m_AY( src.m_AY ),
              m_AZ( src.m_AZ ),
              m_AW( c_UnitW )
            #endif
        {
            #ifndef _SSE_
            m_A[MX] = src.AX();
            m_A[MY] = src.AY();
            m_A[MZ] = src.AZ();
            m_A[MW] = c_UnitW;
            #endif
        }
        
        /// Explicit Constructor from HMat43
        FORCE_INLINE explicit HMat44( const HMat43& src )
            #ifdef _SSE_
            : m_AX( src.m_A[MX] ),
              m_AY( src.m_A[MY] ),
              m_AZ( src.m_A[MZ] ),
              m_AW( src.m_A[MW] || HVecW( 1.0f ) )
            #endif
        {
            #ifndef _SSE_
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
            m_A[MW] = src.m_A[MW] || HVecW( 1.0f );
            #endif
        }
        
        /// Copy Constructor
        FORCE_INLINE HMat44( const HMat44& src )
            #ifdef _SSE_
            : m_AX( src.m_AX ),
              m_AY( src.m_AY ),
              m_AZ( src.m_AZ ),
              m_AW( src.m_AW )
            #endif
        {
            #ifndef _SSE_
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
            m_A[MW] = src.m_A[MW];
            #endif
        }
        
        FORCE_INLINE const HMat44& operator = ( const HMat33& src )
        {
            #ifdef _SSE_
            m_AX = src.m_AX;
            m_AY = src.m_AY;
            m_AZ = src.m_AZ;
            m_AW = c_UnitW;
            #else
            m_A[MX] = src.AX();
            m_A[MY] = src.AY();
            m_A[MZ] = src.AZ();
            m_A[MW] = c_UnitW;
            #endif
            return *this;
        }
        
        FORCE_INLINE const HMat44& operator = ( const HMat43& src )
        {
            #ifdef _SSE_
            m_AX = src.m_A[MX];
            m_AY = src.m_A[MY];
            m_AZ = src.m_A[MZ];
            m_AW = src.m_A[MW] || HVecW( 1.0f );
            #else
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
            m_A[MW] = src.m_A[MW] || HVecW( 1.0f );
            #endif
            return *this;
        }
        
        FORCE_INLINE const HMat44& operator = ( const HMat44& copy )
        {
            #ifdef _SSE_
            m_AX = copy.m_AX;
            m_AY = copy.m_AY;
            m_AZ = copy.m_AZ;
            m_AW = copy.m_AW;
            #else
            m_A[MX] = copy.m_A[MX];
            m_A[MY] = copy.m_A[MY];
            m_A[MZ] = copy.m_A[MZ];
            m_A[MW] = copy.m_A[MW];
            #endif
            return *this;
        }
        
        #ifdef _SSE_
        FORCE_INLINE HMat44( const V128& ax, const V128& ay, const V128& az, const V128& aw ) :
            m_AX( ax ),
            m_AY( ay ),
            m_AZ( az ),
            m_AW( aw )
        {
        }
        #endif
        
        FORCE_INLINE HMat44( const HVec4& ax, const HVec4& ay, const HVec4& az, const HVec4& aw )
            #ifdef _SSE_
            : m_AX( ax ),
              m_AY( ay ),
              m_AZ( az ),
              m_AW( aw )
            #endif
        {
            #ifndef _SSE_
            m_A[MX] = ax;
            m_A[MY] = ay;
            m_A[MZ] = az;
            m_A[MW] = aw;
            #endif
        }
        
        // Rotation about X axis - clockwise when viewed back down the X axis
        FORCE_INLINE HMat44( const RotX& rx, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const F32 alpha = rx;
            const F32 cx = cos( alpha );
            const F32 sx = sin( alpha );
            ConstV4F32( ay ) = { 0.0f,  cx, sx, 0.0f };
            ConstV4F32( az ) = { 0.0f, -sx, cx, 0.0f };
            SetAXYZW( c_UnitX, ay, az, aw );
        }
        
        // Rotation about Y axis - clockwise when viewed back down the Y axis
        FORCE_INLINE HMat44( const RotY& ry, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const F32 beta = ry;
            const F32 cy = cos( beta );
            const F32 sy = sin( beta );
            ConstV4F32( ax ) = { cy, 0.0f, -sy, 0.0f };
            ConstV4F32( az ) = { sy, 0.0f,  cy, 0.0f };
            SetAXYZW( ax, c_UnitY, az, aw );
        }
        
        // Rotation about Z axis - clockwise when viewed back down the Z axis
        FORCE_INLINE HMat44( const RotZ& rz, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const F32 gamma = rz;
            const F32 cz = cos( gamma );
            const F32 sz = sin( gamma );
            ConstV4F32( ax ) = {  cz, sz, 0.0f, 0.0f };
            ConstV4F32( ay ) = { -sz, cz, 0.0f, 0.0f };
            SetAXYZW( ax, ay, c_UnitZ, aw );
        }
        
        // Cross products for deriving third matrix axis from 2 input axes
        // const HVec3 ax = ay ^ az;
        // const HVec3 ay = az ^ ax;
        // const HVec3 az = ax ^ ay;
        
        // Initialise with primary axis X, secondary axis Y
        FORCE_INLINE HMat44( const AxisX& ax, const AxisY& ay, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const HVec3 az = ax ^ ay;
            SetAXYZW( ax.Normalised(), ( az ^ ax ).Normalised(), az.Normalised(), aw );
        }
        
        // Initialise with primary axis X, secondary axis Z
        FORCE_INLINE HMat44( const AxisX& ax, const AxisZ& az, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const HVec3 ay = az ^ ax;
            SetAXYZW( ax.Normalised(), ay.Normalised(), ( ax ^ ay ).Normalised(), aw );
        }
        
        // Initialise with primary axis Y, secondary axis X
        FORCE_INLINE HMat44( const AxisY& ay, const AxisX& ax, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const HVec3 az = ax ^ ay;
            SetAXYZW( ( ay ^ az ).Normalised(), ay.Normalised(), az.Normalised(), aw );
        }
        
        // Initialise with primary axis Y, secondary axis Z
        FORCE_INLINE HMat44( const AxisY& ay, const AxisZ& az, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const HVec3 ax = ay ^ az;
            SetAXYZW( ax.Normalised(), ay.Normalised(), ( ax ^ ay ).Normalised(), aw );
        }
        
        // Initialise with primary axis Z, secondary axis X
        FORCE_INLINE HMat44( const AxisZ& az, const AxisX& ax, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const HVec3 ay = az ^ ax;
            SetAXYZW( ( ay ^ az ).Normalised(), ay.Normalised(), az.Normalised(), aw );
        }
        
        // Initialise with primary axis Z, secondary axis Y
        FORCE_INLINE HMat44( const AxisZ& az, const AxisY& ay, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const HVec3 ax = ay ^ az;
            SetAXYZW( ax.Normalised(), ( az ^ ax ).Normalised(), az.Normalised(), aw );
        }
        
        FORCE_INLINE void SetIdentity()
        {
            SetAXYZW( c_UnitX, c_UnitY, c_UnitZ, c_UnitW );
        }
        
        FORCE_INLINE const HMat44 Transposed() const
        {
            #ifdef _SSE_
            
            const V128 tmp0 = _mm_shuffle_ps( m_AX.m_V, m_AY.m_V, 0x44 );
            const V128 tmp1 = _mm_shuffle_ps( m_AZ.m_V, m_AW.m_V, 0x44 );
            const V128 tmp2 = _mm_shuffle_ps( m_AX.m_V, m_AY.m_V, 0xEE );
            const V128 tmp3 = _mm_shuffle_ps( m_AZ.m_V, m_AW.m_V, 0xEE );
            
            const V128 ax = _mm_shuffle_ps( tmp0, tmp1, 0x88 );
            const V128 ay = _mm_shuffle_ps( tmp0, tmp1, 0xDD );
            const V128 az = _mm_shuffle_ps( tmp2, tmp3, 0x88 );
            const V128 aw = _mm_shuffle_ps( tmp2, tmp3, 0xDD );
            
            return HMat44( ax, ay, az, aw );
            
            #else
            
            const HMat44& m = *this;
            
            const HVec4 ax( m[MX][VX], m[MY][VX], m[MZ][VX], m[MW][VX] );
            const HVec4 ay( m[MX][VY], m[MY][VY], m[MZ][VY], m[MW][VY] );
            const HVec4 az( m[MX][VZ], m[MY][VZ], m[MZ][VZ], m[MW][VZ] );
            const HVec4 aw( m[MX][VW], m[MY][VW], m[MZ][VW], m[MW][VW] );
            
            return HMat44( ax, ay, az, aw );
            
            #endif
        }
        
        FORCE_INLINE void Transpose()
        {
            #ifdef _SSE_
            
            const V128 tmp0 = _mm_shuffle_ps( m_AX.m_V, m_AY.m_V, 0x44 );
            const V128 tmp1 = _mm_shuffle_ps( m_AZ.m_V, m_AW.m_V, 0x44 );
            const V128 tmp2 = _mm_shuffle_ps( m_AX.m_V, m_AY.m_V, 0xEE );
            const V128 tmp3 = _mm_shuffle_ps( m_AZ.m_V, m_AW.m_V, 0xEE );
            
            m_AX.m_V = _mm_shuffle_ps( tmp0, tmp1, 0x88 );
            m_AY.m_V = _mm_shuffle_ps( tmp0, tmp1, 0xDD );
            m_AZ.m_V = _mm_shuffle_ps( tmp2, tmp3, 0x88 );
            m_AW.m_V = _mm_shuffle_ps( tmp2, tmp3, 0xDD );
            
            #else
            
            const HMat44& m = *this;
            
            const HVec4 ax( m[MX][VX], m[MY][VX], m[MZ][VX], m[MW][VX] );
            const HVec4 ay( m[MX][VY], m[MY][VY], m[MZ][VY], m[MW][VY] );
            const HVec4 az( m[MX][VZ], m[MY][VZ], m[MZ][VZ], m[MW][VZ] );
            const HVec4 aw( m[MX][VW], m[MY][VW], m[MZ][VW], m[MW][VW] );
            
            m_A[MX] = ax;
            m_A[MY] = ay;
            m_A[MZ] = az;
            m_A[MW] = aw;
            
            #endif
        }
        
        // Matrix Multiply
        /*FORCE_INLINE*/ const HMat44 operator * ( const HMat44& m ) const
        {
            #ifdef _SSE_
            return HMat44( m_AX * m,
                           m_AY * m,
                           m_AZ * m,
                           m_AW * m );
            #else
            return HMat44( m_A[MX] * m,
                           m_A[MY] * m,
                           m_A[MZ] * m,
                           m_A[MW] * m );
            #endif
        }
        
        /// Full 4x4 Matrix Inverse
        
        #ifdef _SSE_
        const HMat44 Matrix44InverseCramersRuleSIMD() const;
        #endif
        
        FORCE_INLINE const HMat44 Inverse() const
        {
            #ifdef _SSE_
            return Matrix44InverseCramersRuleSIMD();
            #else
            HMat44 result;
            Matrix44InverseCramersRuleFP( (F32*)m_A, (F32*)result.m_A );
            return result;
            #endif
        }
        
        // Inverse of Rotation & Translation
        // Not valid for matrices with scaling/skewing etc!
        FORCE_INLINE const HMat44 InverseRT() const
        {
            //const HMat33 invRot = HMat33( *this ).Transposed();
            const HMat33 invRot = ( (HMat33&) *this ).Transposed();
            
            #ifdef _SSE_
            return HMat44( invRot.AX(),
                           invRot.AY(),
                           invRot.AZ(),
                           //HVec4( c_UnitW ) - HVec3( m_AW ) * invRot );
                           ( HVec3( -m_AW ) * invRot ) || HVecW( c_UnitW ) );
            #else
            return HMat44( invRot.AX(),
                           invRot.AY(),
                           invRot.AZ(),
                           //HVec4( c_UnitW ) - HVec3( m_A[MW] ) * invRot );
                           ( HVec3( -m_A[MW] ) * invRot ) || HVecW( 1.0f ) );
            #endif
        }
        
        #if ( DIRECT3D_VERSION >= 0x0900 )
        
        // Convert to D3DMATRIX
        operator const D3DMATRIX () const
        {
            //return *((D3DMATRIX*)this);
            
            D3DMATRIX result;
            
            #ifdef _SSE_
            
            _mm_storeu_ps( &result._11, m_AX );
            _mm_storeu_ps( &result._21, m_AY );
            _mm_storeu_ps( &result._31, m_AZ );
            _mm_storeu_ps( &result._41, m_AW );
            
            #else
            
            const HMat44& m = *this;
            
            result._11 = m[MX][VX];
            result._12 = m[MX][VY];
            result._13 = m[MX][VZ];
            result._14 = m[MX][VW];
            
            result._21 = m[MY][VX];
            result._22 = m[MY][VY];
            result._23 = m[MY][VZ];
            result._24 = m[MY][VW];
            
            result._31 = m[MZ][VX];
            result._32 = m[MZ][VY];
            result._33 = m[MZ][VZ];
            result._34 = m[MZ][VW];
            
            result._41 = m[MW][VX];
            result._42 = m[MW][VY];
            result._43 = m[MW][VZ];
            result._44 = m[MW][VW];
            
            #endif
            
            return result;
        }
        
        // Convert to D3DXMATRIX
        operator const D3DXMATRIX () const
        {
            //return *((D3DXMATRIX*)this);
            
            D3DXMATRIX result;
            
            #ifdef _SSE_
            
            _mm_storeu_ps( &result._11, m_AX );
            _mm_storeu_ps( &result._21, m_AY );
            _mm_storeu_ps( &result._31, m_AZ );
            _mm_storeu_ps( &result._41, m_AW );
            
            #else
            
            const HMat44& m = *this;
            
            result._11 = m[MX][VX];
            result._12 = m[MX][VY];
            result._13 = m[MX][VZ];
            result._14 = m[MX][VW];
            
            result._21 = m[MY][VX];
            result._22 = m[MY][VY];
            result._23 = m[MY][VZ];
            result._24 = m[MY][VW];
            
            result._31 = m[MZ][VX];
            result._32 = m[MZ][VY];
            result._33 = m[MZ][VZ];
            result._34 = m[MZ][VW];
            
            result._41 = m[MW][VX];
            result._42 = m[MW][VY];
            result._43 = m[MW][VZ];
            result._44 = m[MW][VW];
            
            #endif
            
            return result;
        }
        
        #endif
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            if ( text != "" )
            {
                OutputDebugString( text );
                OutputDebugString( "\n" );
            }
            AX().Print( "AX" );
            AY().Print( "AY" );
            AZ().Print( "AZ" );
            AW().Print( "AW" );
            OutputDebugString( "\n" );
        }
        #endif
        
    private:
        
        #ifdef _SSE_
        HVec4 m_AX;
        HVec4 m_AY;
        HVec4 m_AZ;
        HVec4 m_AW;
        #else
        HVec4 m_A[4];
        #endif
    };
    
    
    //
    // C_Quaternion Class
    //
    
    FORCE_INLINE const HVec4 InitQuaternion( const HVec3& axis, const HF32& theta )
    {
        const HF32 halfTheta( theta * 0.5f );
        const HVec3 imag( axis.Normalised() * sin( halfTheta ) );
        const HVecW real( HVecW( cos( halfTheta ) ) );
        return imag || real;
    }
    
    FORCE_INLINE const HVec4 InitQuaternion( const HMat33& matrix )
    {
        #ifdef _SSE_
        
        const HF32 one( 1.0f );
        const HF32 half( 0.5f );
        const HF32 quarter( 0.25f );
        
        const V128 ax( matrix.AX() );
        const V128 ay( matrix.AY() );
        const V128 az( matrix.AZ() );
        
	    //const V128 vec_xyz( az.Y() - ay.Z(),
	    //                    ax.Z() - az.X(),
	    //                    ay.X() - ax.Y() );
        const V128 zy_xz_yx( Shuffle<VY,VZ,VX,VW>( az, ax, ay ) );
        const V128 yz_zx_xy( Shuffle<VZ,VX,VY,VW>( ay, az, ax ) );
	    const HVec3 axis_xyz( HVec3( _mm_sub_ps( zy_xz_yx, yz_zx_xy ) ).Normalised() );
        
	    //const F32 trace = ax.X() + ay.Y() + az.Z() + 1.0f;
	    const V128 trace( _mm_add_ss( one,
	                      _mm_add_ss( ax,
	                      _mm_add_ss( Shuffle<VY,VW,VW,VW>( ay ),
	                                  Shuffle<VZ,VW,VW,VW>( az ) ) ) ) );
        
        const HF32 cos_half_alpha( _mm_mul_ss( _mm_sqrt_ss( trace ), half ) );
        
        const HF32 sin_half_alpha( _mm_sqrt_ss( _mm_sub_ss( one, _mm_mul_ss( trace, quarter ) ) ) );
        
	    return ( axis_xyz * sin_half_alpha ) || HVecW( cos_half_alpha );
        
        #else
        
        const HVec3 ax = matrix.AX();
        const HVec3 ay = matrix.AY();
        const HVec3 az = matrix.AZ();
        
	    const HVec3 vec_xyz( az.Y() - ay.Z(),
	                         ax.Z() - az.X(),
	                         ay.X() - ax.Y() );

	    const F32 trace = ax.X() + ay.Y() + az.Z() + 1.0f;
        
		const F32 cos_half_alpha = sqrtf( GMax( trace, 0.0f ) ) * 0.5f;
		const F32 sin_half_alpha = sqrtf( GMax( 1.0f - ( trace * 0.25f ), 0.0f ) );
		
		// S.PENSON
		//const F32 cos_half_alpha = (trace < 0.00001f) ? 0.0f : sqrtf( trace ) * 0.5f;
		//const F32 sin_half_alpha = (trace > 3.99999f) ? 0.0f : sqrtf( 1.0f - ( trace * 0.25f ) );
        
        const HVec3 axis_xyz = vec_xyz.Normalised();
        //const HVec3 axis_xyz = vec_xyz.SafeNormalised();
        
		// S.PENSON
		//const F32 vec_xyz_len = vec_xyz.Length();
		//const HVec3 axis_xyz = (vec_xyz_len > 0.000001f) ? vec_xyz * (1.0f/vec_xyz_len) : vec_xyz;
        
	    return ( axis_xyz * sin_half_alpha ) || HVecW( cos_half_alpha );
	    
	    #endif
    }
    
    ALIGN16 class C_Quaternion
    {
    public:
        
        C_Quaternion() :
            m_V( c_UnitW ) // Initialise to the Multiplicative Identity.
        {
        }
        
        #ifdef _SSE_
        explicit C_Quaternion( const V128& src ) :
            m_V( src )
        {
        }
        #endif
        
        explicit C_Quaternion( const HVec4& src ) :
            m_V( src )
        {
        }
        
        C_Quaternion( const C_Quaternion& src ) :
            m_V( src.m_V )
        {
        }
        
        C_Quaternion( const F32 vx, const F32 vy, const F32 vz, const F32 a ) :
            m_V( HVec4( vx, vy, vz, a ) )
        {
        }
        
        C_Quaternion( const HVec3& axis, const HF32& theta ) :
            m_V( InitQuaternion( axis, theta ) )
        {
        }
        
        C_Quaternion( const HVec3& imag, const HVecW& real ) :
            m_V( imag || real )
        {
        }
        
        explicit C_Quaternion( const HVec3& imag ) :
            m_V( imag )
        {
        }
        
        explicit C_Quaternion( const HF32& real ) :
            m_V( HVecW( real ) )
        {
        }
        
        explicit C_Quaternion( const HMat33& matrix ) :
            m_V( InitQuaternion( matrix ) )
        {
        }
        
        /// Returns the Imaginary part as a HVec3
        const HVec3 Imag() const
        {
            return (HVec3)m_V;
        }
        
        /// Returns the Real part as a HVecW
        const HVecW Real() const
        {
            return (HVecW)m_V.W();
        }
        
        /// Addition
        const C_Quaternion operator + ( const C_Quaternion& src ) const
        {
            return C_Quaternion( m_V + src.m_V );
        }
        
        /// Subtraction
        const C_Quaternion operator - ( const C_Quaternion& src ) const
        {
            return C_Quaternion( m_V - src.m_V );
        }
        
        /// Unary Negation
        const C_Quaternion operator - () const
        {
            return C_Quaternion( -m_V );
        }
        
        /// Conjugate (Multiplicative Inverse) - Assumes Unit C_Quaternion!
        const C_Quaternion operator ! () const
        {
            return C_Quaternion( -Imag() || Real() );
        }
        
        /// Scalar multiplication
        const C_Quaternion operator * ( const HF32& f ) const
        {
            return C_Quaternion( m_V * f );
        }
        
        /// Multiplication
        //const C_Quaternion operator * ( const C_Quaternion& r ) const
        const C_Quaternion operator * ( const C_Quaternion& l ) const
        {
            // q1q2 = ( a1*a2 - v1|v2, a1*v2 + a2*v1 + v1^v2 )
            
            //const C_Quaternion& l = *this;
            const C_Quaternion& r = *this;
            const HVec3 vl( l.Imag() );
            const HVec3 vr( r.Imag() );
            
            // Scalar * Vector components a1*v2 + a2*v1 +
            // Vector cross product component v1^v2
            const HVec3 imag( ( vr * l.Real() ) + ( vl * r.Real() ) + ( vl ^ vr ) );
            
            #ifdef _SSE_
            
            // Scalar multiplication component a1*a2 -
            // Vector dot product component v1|v2
            const HVec4 dotXYZA( l.m_V * r.m_V );
            const V128 dotX( dotXYZA );
            const V128 dotY( ReplicateY( dotXYZA ) );
            const V128 dotZ( ReplicateZ( dotXYZA ) );
            const V128 dotA( ReplicateW( dotXYZA ) );
            
            const V128 real( _mm_andnot_ps( c_MaskXYZ, ReplicateX( _mm_sub_ss( dotA, _mm_add_ss( _mm_add_ss( dotX, dotY ), dotZ ) ) ) ) );
            
            // Combine real & imaginary parts
            const V128 q( _mm_add_ps( imag, real ) );
            
            return C_Quaternion( q );
            
            #else
            
            // Scalar multiplication component a1*a2 -
            // Vector dot product component v1|v2
            const F32 real( ( l.m_V[QA] * r.m_V[QA] ) - ( vl | vr ) );
            
            return C_Quaternion( imag.m_V[VX], imag.m_V[VY], imag.m_V[VZ], real );
            
            #endif
        }
        
        /// Division: q1 / q2 = q1 * q2-1
        const C_Quaternion operator / ( const C_Quaternion& r ) const
        {
            const C_Quaternion& l = *this;
            return l * !r;
        }
        
        /// Returns Unit C_Quaternion
        const C_Quaternion Normalised() const
        {
            return C_Quaternion( m_V.Normalised() );
        }
        
        /// Lerp: Linear Interpolation
        const C_Quaternion Lerp( const C_Quaternion& dst, const HF32& t ) const
        {
            const C_Quaternion& src = *this;
            return C_Quaternion( ( src * ( HF32( 1.0f ) - t ) ) + ( dst * t ) );
        }
        
        /// Slerp: Spherical Linear Interpolation
        const C_Quaternion Slerp( const C_Quaternion& q, const F32 t ) const
        {
            const C_Quaternion& src = *this;
            C_Quaternion dst( q );
            
            F32 cosHalfTheta( src.m_V | dst.m_V );
            
            if ( cosHalfTheta < 0.0f )
            {
                cosHalfTheta = fabs( cosHalfTheta );
                dst = -q;
            }
            
            // Need check for small angles!
            if ( ( 1.0f - cosHalfTheta ) >= FLT_EPSILON )
            {
                const F32 halfTheta( acos( cosHalfTheta ) );
                const HF32 sinHalfTheta( sin( halfTheta ) );
                const HF32 reciprocalSinHalfTheta( HF32( 1.0f ) / sinHalfTheta );
                
                const HF32 scale0( reciprocalSinHalfTheta * (float)sin( ( 1.0f - t ) * halfTheta ) );
                const HF32 scale1( reciprocalSinHalfTheta * (float)sin( t * halfTheta ) );
                
                return C_Quaternion( ( src * scale0 ) + ( dst * scale1 ) );
            }
            else
            {
                return src.Lerp( dst, t );
            }
        }
        
        operator const HVec4() const { return m_V; }
        
        operator const HMat33 () const
        {
            #ifdef _SSE_
            
            const HVec4 xyza2( m_V + m_V );
            
            const HVec3 ax( HVec3( c_UnitX ) +
                            HVec3( Shuffle<VY,VX,VX,0>( m_V ) ) * HVec3( Shuffle<VY,VY,VZ,0>( xyza2 ) ) * HVec3( Shuffle<1,0,0,3>( c_P1M1ZZ ) ) +
                            HVec3( Shuffle<VZ,VW,VW,0>( m_V ) ) * HVec3( Shuffle<VZ,VZ,VY,0>( xyza2 ) ) * HVec3( Shuffle<1,0,1,3>( c_P1M1ZZ ) ) );
            
            const HVec3 ay( HVec3( c_UnitY ) +
                            HVec3( Shuffle<VX,VX,VY,0>( m_V ) ) * HVec3( Shuffle<VY,VX,VZ,0>( xyza2 ) ) * HVec3( Shuffle<0,1,0,3>( c_P1M1ZZ ) ) +
                            HVec3( Shuffle<VW,VZ,VW,0>( m_V ) ) * HVec3( Shuffle<VZ,VZ,VX,0>( xyza2 ) ) * HVec3( Shuffle<1,1,0,3>( c_P1M1ZZ ) ) );
            
            const HVec3 az( HVec3( c_UnitZ ) +
                            HVec3( Shuffle<VX,VY,VX,0>( m_V ) ) * HVec3( Shuffle<VZ,VZ,VX,0>( xyza2 ) ) * HVec3( Shuffle<0,0,1,3>( c_P1M1ZZ ) ) +
                            HVec3( Shuffle<VW,VW,VY,0>( m_V ) ) * HVec3( Shuffle<VY,VX,VY,0>( xyza2 ) ) * HVec3( Shuffle<0,1,1,3>( c_P1M1ZZ ) ) );
            
            #else
            
            // calculate coefficients
            
            const F32 x2 = m_V[QX] * 2.0f;      // quat->x + quat->x;
            const F32 y2 = m_V[QY] * 2.0f;      // quat->y + quat->y; 
            const F32 z2 = m_V[QZ] * 2.0f;      // quat->z + quat->z;
            
            const F32 xx2 = m_V[QX] * x2;       // quat->x * x2;
            const F32 xy2 = m_V[QX] * y2;       // quat->x * y2;
            const F32 xz2 = m_V[QX] * z2;       // quat->x * z2;
            
            const F32 yy2 = m_V[QY] * y2;       // quat->y * y2;
            const F32 yz2 = m_V[QY] * z2;       // quat->y * z2;
            const F32 zz2 = m_V[QZ] * z2;       // quat->z * z2;
            
            const F32 wx2 = m_V[QA] * x2;       // quat->w * x2;
            const F32 wy2 = m_V[QA] * y2;       // quat->w * y2;
            const F32 wz2 = m_V[QA] * z2;       // quat->w * z2;
            
            const HVec3 ax( 1.0f - yy2 - zz2,        xy2 + wz2,        xz2 - wy2 );
            const HVec3 ay(        xy2 - wz2, 1.0f - xx2 - zz2,        yz2 + wx2 );
            const HVec3 az(        xz2 + wy2,        yz2 - wx2, 1.0f - xx2 - yy2 );
            
            #endif
            
            return HMat33( ax, ay, az );
        }
        
#ifdef WIN32
        const S32 Finite() const
        {
            return m_V.Finite();
        }
#endif
        
        #ifdef SHOOTER_VECMAT_DEBUG_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            HVec4 v( m_V );
            sprintf( buffer, "%s C_Quaternion( Imag( %+11.6f, %+11.6f, %+11.6f ), Real( %+11.6f ) )\n", text, (F32)v.X(), (F32)v.Y(), (F32)v.Z(), (F32)v.W() );
            OutputDebugString( buffer );
        }
        #endif
        
    private:
        
        // The quaternion is stored with the real(scalar) part in the W component and
        // the imaginary(vector) part in the X, Y & Z components.
        
        HVec4   m_V;
    };
    
    
    //
    // HVecX Class Member Functions
    //
    
    // Explicit
    FORCE_INLINE HVecX::HVecX( const HVecY& src )
    {
        #ifdef _SSE_
        m_X = Shuffle<VY,0,0,0>( src.m_Y );
        #else
        m_X = src.m_Y;
        #endif
    }
    
    // Explicit
    FORCE_INLINE HVecX::HVecX( const HVecZ& src )
    {
        #ifdef _SSE_
        m_X = Shuffle<VZ,0,0,0>( src.m_Z );
        #else
        m_X = src.m_Z;
        #endif
    }
    
    // Explicit
    FORCE_INLINE HVecX::HVecX( const HVecW& src )
    {
        #ifdef _SSE_
        m_X = Shuffle<VW,0,0,0>( src.m_W );
        #else
        m_X = src.m_W;
        #endif
    }
    
    
    //
    // HVecY Class Member Functions
    //
    
    // Explicit
    FORCE_INLINE HVecY::HVecY( const HVecX& src )
    {
        #ifdef _SSE_
        m_Y = Shuffle<1,VX,1,1>( src.m_X );
        #else
        m_Y = src.m_X;
        #endif
    }
    
    // Explicit
    FORCE_INLINE HVecY::HVecY( const HVecZ& src )
    {
        #ifdef _SSE_
        m_Y = Shuffle<1,VZ,1,1>( src.m_Z );
        #else
        m_Y = src.m_Z;
        #endif
    }
    
    // Explicit
    FORCE_INLINE HVecY::HVecY( const HVecW& src )
    {
        #ifdef _SSE_
        m_Y = Shuffle<1,VW,1,1>( src.m_W );
        #else
        m_Y = src.m_W;
        #endif
    }
    
    
    //
    // HVecZ Class Member Functions
    //
    
    // Explicit
    FORCE_INLINE HVecZ::HVecZ( const HVecX& src )
    {
        #ifdef _SSE_
        m_Z = Shuffle<2,2,VX,2>( src.m_X );
        #else
        m_Z = src.m_X;
        #endif
    }
    
    // Explicit
    FORCE_INLINE HVecZ::HVecZ( const HVecY& src )
    {
        #ifdef _SSE_
        m_Z = Shuffle<2,2,VY,2>( src.m_Y );
        #else
        m_Z = src.m_Y;
        #endif
    }
    
    // Explicit
    FORCE_INLINE HVecZ::HVecZ( const HVecW& src )
    {
        #ifdef _SSE_
        m_Z = Shuffle<2,2,VW,2>( src.m_W );
        #else
        m_Z = src.m_W;
        #endif
    }
    
    
    //
    // HVecW Class Member Functions
    //
    
    // Explicit
    FORCE_INLINE HVecW::HVecW( const HVecX& src )
    {
        #ifdef _SSE_
        m_W = Shuffle<3,3,3,VX>( src.m_X );
        #else
        m_W = src.m_X;
        #endif
    }
    
    // Explicit
    FORCE_INLINE HVecW::HVecW( const HVecY& src )
    {
        #ifdef _SSE_
        m_W = Shuffle<3,3,3,VY>( src.m_Y );
        #else
        m_W = src.m_Y;
        #endif
    }
    
    // Explicit
    FORCE_INLINE HVecW::HVecW( const HVecZ& src )
    {
        #ifdef _SSE_
        m_W = Shuffle<3,3,3,VZ>( src.m_Z );
        #else
        m_W = src.m_Z;
        #endif
    }
    
    
    //
    // HVec2 Class Member Functions
    //
    
    // Explicit
    FORCE_INLINE HVec2::HVec2( const HVec3& src )
    {
        #ifdef _SSE_
        m_V = _mm_and_ps( c_MaskXY, src.m_V );
        #else
        m_V[VX] = src.m_V[VX];
        m_V[VY] = src.m_V[VY];
        #endif
    }
    
    // Explicit
    FORCE_INLINE HVec2::HVec2( const HVec4& src )
    {
        #ifdef _SSE_
        m_V = _mm_and_ps( c_MaskXY, src.m_V );
        #else
        m_V[VX] = src.m_V[VX];
        m_V[VY] = src.m_V[VY];
        #endif
    }
    
    // Swizzle HVec2:XY into a HVec3:XZ with zero Y
    FORCE_INLINE const HVec3 HVec2::XYtoXZ() const
    {
        #ifdef _SSE_
        return HVec3( _mm_and_ps( c_MaskXZ, Shuffle<VX,0,VY,0>( m_V ) ) );
        #else
        return HVec3( m_V[VX], 0.0f, m_V[VY] );
        #endif
    }
    
    //
    // HVec3 Class Member Functions
    //
    
    // Explicit
    FORCE_INLINE HVec3::HVec3( const HVec4& src )
    {
        #ifdef _SSE_
        m_V = _mm_and_ps( c_MaskXYZ, src.m_V );
        #else
        m_V[VX] = src.m_V[VX];
        m_V[VY] = src.m_V[VY];
        m_V[VZ] = src.m_V[VZ];
        #endif
    }
    
    FORCE_INLINE const HVec3 HVec3::operator * ( const HMat33& m ) const
    {
        #ifdef _SSE_
        
        const V128 x_r0 = _mm_mul_ps( ReplicateX( m_V ), m.m_AX.m_V );
        const V128 y_r1 = _mm_mul_ps( ReplicateY( m_V ), m.m_AY.m_V );
        const V128 z_r2 = _mm_mul_ps( ReplicateZ( m_V ), m.m_AZ.m_V );
        
        const V128 result = _mm_add_ps( _mm_add_ps( x_r0, y_r1 ), z_r2 );
        
        return HVec3( result );
        
        #else
        
        const HVec3& v = *this;
        
        const F32 x = v[VX] * m[MX][VX] + v[VY] * m[MY][VX] + v[VZ] * m[MZ][VX];
        const F32 y = v[VX] * m[MX][VY] + v[VY] * m[MY][VY] + v[VZ] * m[MZ][VY];
        const F32 z = v[VX] * m[MX][VZ] + v[VY] * m[MY][VZ] + v[VZ] * m[MZ][VZ];
        
        return HVec3( x, y, z );
        
        #endif
    }
    
    FORCE_INLINE const HVec3 HVec3::operator * ( const HMat43& m ) const
    {
        #ifdef _SSE_
        
        const V128 x_r0 = _mm_mul_ps( ReplicateX( m_V ), m.m_A[MX].m_V );
        const V128 y_r1 = _mm_mul_ps( ReplicateY( m_V ), m.m_A[MY].m_V );
        const V128 z_r2 = _mm_mul_ps( ReplicateZ( m_V ), m.m_A[MZ].m_V );
        const V128 w_r3 = m.m_A[MW].m_V;
        
        const V128 result = _mm_add_ps( _mm_add_ps( x_r0, y_r1 ), _mm_add_ps( z_r2, w_r3 ) );
        
        return HVec3( result );
        
        #else
        
        const HVec3& v = *this;
        
        const F32 x = v[VX] * m[MX][VX] + v[VY] * m[MY][VX] + v[VZ] * m[MZ][VX] + m[MW][VX];
        const F32 y = v[VX] * m[MX][VY] + v[VY] * m[MY][VY] + v[VZ] * m[MZ][VY] + m[MW][VY];
        const F32 z = v[VX] * m[MX][VZ] + v[VY] * m[MY][VZ] + v[VZ] * m[MZ][VZ] + m[MW][VZ];
        
        return HVec3( x, y, z );
        
        #endif
    }
    
    FORCE_INLINE const HVec3 HVec3::operator * ( const HMat44& m ) const
    {
        #ifdef _SSE_
        
        const V128 x_r0 = _mm_mul_ps( ReplicateX( m_V ), m.m_AX.m_V );
        const V128 y_r1 = _mm_mul_ps( ReplicateY( m_V ), m.m_AY.m_V );
        const V128 z_r2 = _mm_mul_ps( ReplicateZ( m_V ), m.m_AZ.m_V );
        const V128 w_r3 = m.m_AW.m_V;
        
        const V128 result = _mm_add_ps( _mm_add_ps( x_r0, y_r1 ), _mm_add_ps( z_r2, w_r3 ) );
        
        return HVec3( _mm_and_ps( c_MaskXYZ, result ) );
        
        #else
        
        const HVec3& v = *this;
        
        const F32 x = v[VX] * m[MX][VX] + v[VY] * m[MY][VX] + v[VZ] * m[MZ][VX] + m[MW][VX];
        const F32 y = v[VX] * m[MX][VY] + v[VY] * m[MY][VY] + v[VZ] * m[MZ][VY] + m[MW][VY];
        const F32 z = v[VX] * m[MX][VZ] + v[VY] * m[MY][VZ] + v[VZ] * m[MZ][VZ] + m[MW][VZ];
        
        return HVec3( x, y, z );
        
        #endif
    }
    
    // Vector * C_Quaternion Multiplication
    FORCE_INLINE const HVec3 HVec3::operator * ( const C_Quaternion& q ) const
    {
        return ( q * C_Quaternion( *this ) * !q ).Imag();
    }
    
    // Combine HVec3 with HVecW to form a HVec4
    //FORCE_INLINE const HVec4 HVec3::operator + ( const HVecW& src ) const
    FORCE_INLINE const HVec4 HVec3::operator || ( const HVecW& src ) const
    {
        DebugValidate();
        
        #ifdef _SSE_
        return HVec4( _mm_add_ps( m_V, src.m_W ) );
        #else
        return HVec4( m_V[VX],
                      m_V[VY],
                      m_V[VZ],
                      src.m_W );
        #endif
    }
    
    FORCE_INLINE void HVec3::DebugValidate() const
    {
        #ifdef _DEBUG
        #ifdef _SSE_
        if ( HVec4( m_V ).W() != 0.0f )
        {
			#ifdef SHOOTER_VECMAT_DEBUG_PRINT
            Print( "HVec3::DebugValidate(): Invalid HVec3!" );
			#endif
        }
        #endif
        #endif
    }
    
    // Get axes perpendicular to vector
    FORCE_INLINE void HVec3::PerpendicularAxes( HVec3& u, HVec3& v ) const
    {
        const AxisZ vectorZ( *this );
        const HMat33 orientation( vectorZ );
        u = orientation.AX();
        v = orientation.AY();
    }
    
    //
    // HVec4 Class Member Functions
    //
    
    FORCE_INLINE const HVec4 HVec4::operator * ( const HMat44& m ) const
    {
        #ifdef _SSE_
        
        const V128 x_r0 = _mm_mul_ps( ReplicateX( m_V ), m.m_AX.m_V );
        const V128 y_r1 = _mm_mul_ps( ReplicateY( m_V ), m.m_AY.m_V );
        const V128 z_r2 = _mm_mul_ps( ReplicateZ( m_V ), m.m_AZ.m_V );
        const V128 w_r3 = _mm_mul_ps( ReplicateW( m_V ), m.m_AW.m_V );
        
        const V128 result = _mm_add_ps( _mm_add_ps( x_r0, y_r1 ), _mm_add_ps( z_r2, w_r3 ) );
        
        return HVec4( result );
        
        #else
        
        const HVec4& v = *this;
        
        const F32 x = v[VX] * m[MX][VX] + v[VY] * m[MY][VX] + v[VZ] * m[MZ][VX] + v[VW] * m[MW][VX];
        const F32 y = v[VX] * m[MX][VY] + v[VY] * m[MY][VY] + v[VZ] * m[MZ][VY] + v[VW] * m[MW][VY];
        const F32 z = v[VX] * m[MX][VZ] + v[VY] * m[MY][VZ] + v[VZ] * m[MZ][VZ] + v[VW] * m[MW][VZ];
        const F32 w = v[VX] * m[MX][VW] + v[VY] * m[MY][VW] + v[VZ] * m[MZ][VW] + v[VW] * m[MW][VW];
        
        return HVec4( x, y, z, w );
        
        #endif
    }
    
    //
    // HMat33 Class Member Functions
    //
    
    // Explicit 
    FORCE_INLINE HMat33::HMat33( const HMat43& src )
    {
        #ifdef _SSE_
        m_AX = src.m_A[MX];
        m_AY = src.m_A[MY];
        m_AZ = src.m_A[MZ];
        #else
        m_A[MX] = src.m_A[MX];
        m_A[MY] = src.m_A[MY];
        m_A[MZ] = src.m_A[MZ];
        #endif
    }
    
    // Explicit 
    FORCE_INLINE HMat33::HMat33( const HMat44& src )
    {
        #ifdef _SSE_
        
        m_AX.m_V = _mm_and_ps( c_MaskXYZ, src.m_AX.m_V );
        m_AY.m_V = _mm_and_ps( c_MaskXYZ, src.m_AY.m_V );
        m_AZ.m_V = _mm_and_ps( c_MaskXYZ, src.m_AZ.m_V );
        
        #else
        
        m_A[MX].m_V[VX] = src.m_A[MX].m_V[VX];
        m_A[MX].m_V[VY] = src.m_A[MX].m_V[VY];
        m_A[MX].m_V[VZ] = src.m_A[MX].m_V[VZ];
        
        m_A[MY].m_V[VX] = src.m_A[MY].m_V[VX];
        m_A[MY].m_V[VY] = src.m_A[MY].m_V[VY];
        m_A[MY].m_V[VZ] = src.m_A[MY].m_V[VZ];
        
        m_A[MZ].m_V[VX] = src.m_A[MZ].m_V[VX];
        m_A[MZ].m_V[VY] = src.m_A[MZ].m_V[VY];
        m_A[MZ].m_V[VZ] = src.m_A[MZ].m_V[VZ];
        
        #endif
    }
    
    // Combine with HVec3 position to form a HMat43
    //FORCE_INLINE const HMat43 HMat33::operator + ( const HVec3& pos ) const
    FORCE_INLINE const HMat43 HMat33::operator || ( const HVec3& pos ) const
    {
        return HMat43( AX(), AY(), AZ(), pos );
    }
    
    #if ( DIRECT3D_VERSION >= 0x0900 )
    
    FORCE_INLINE HMat33::operator const D3DMATRIX () const
    {
        return (D3DMATRIX)HMat44( *this );
    }
    
    FORCE_INLINE HMat33::operator const D3DXMATRIX () const
    {
        return (D3DXMATRIX)HMat44( *this );
    }
    
    #endif
    
    
    //
    // HMat43 Class Member Functions
    //
    
    // Explicit 
    FORCE_INLINE HMat43::HMat43( const HMat44& src )
    {
        #ifdef _SSE_
        
        m_A[MX].m_V = _mm_and_ps( c_MaskXYZ, src.m_AX.m_V );
        m_A[MY].m_V = _mm_and_ps( c_MaskXYZ, src.m_AY.m_V );
        m_A[MZ].m_V = _mm_and_ps( c_MaskXYZ, src.m_AZ.m_V );
        m_A[MW].m_V = _mm_and_ps( c_MaskXYZ, src.m_AW.m_V );
        
        #else
        
        m_A[MX].m_V[VX] = src.m_A[MX].m_V[VX];
        m_A[MX].m_V[VY] = src.m_A[MX].m_V[VY];
        m_A[MX].m_V[VZ] = src.m_A[MX].m_V[VZ];
        
        m_A[MY].m_V[VX] = src.m_A[MY].m_V[VX];
        m_A[MY].m_V[VY] = src.m_A[MY].m_V[VY];
        m_A[MY].m_V[VZ] = src.m_A[MY].m_V[VZ];
        
        m_A[MZ].m_V[VX] = src.m_A[MZ].m_V[VX];
        m_A[MZ].m_V[VY] = src.m_A[MZ].m_V[VY];
        m_A[MZ].m_V[VZ] = src.m_A[MZ].m_V[VZ];
        
        m_A[MW].m_V[VX] = src.m_A[MW].m_V[VX];
        m_A[MW].m_V[VY] = src.m_A[MW].m_V[VY];
        m_A[MW].m_V[VZ] = src.m_A[MW].m_V[VZ];
        
        #endif
    }
    
    #if ( DIRECT3D_VERSION >= 0x0900 )
    
    FORCE_INLINE HMat43::operator const D3DMATRIX () const
    {
        return (D3DMATRIX)HMat44( *this );
    }
    
    FORCE_INLINE HMat43::operator const D3DXMATRIX () const
    {
        return (D3DXMATRIX)HMat44( *this );
    }
    
    #endif
    
    //
    // HMat44 Class Member Functions
    //
    
    
    
};



#ifndef _SSE_
using VecMat::c_Zero;
#endif

using VecMat::c_UnitX;
using VecMat::c_UnitY;
using VecMat::c_UnitZ;
using VecMat::c_UnitW;

using VecMat::Array2F32;
using VecMat::Array3F32;
using VecMat::Array4F32;

using VecMat::RGBA;

using VecMat::V4_S32;

using VecMat::HF32;
using VecMat::HMin;
using VecMat::HMax;
using VecMat::HAbs;
using VecMat::HSqrt;


using VecMat::HVecX;
using VecMat::HVecY;
using VecMat::HVecZ;
using VecMat::HVecW;

using VecMat::HVec2;
using VecMat::HVec3;
using VecMat::HVec4;

using VecMat::C_Scalar;
using VecMat::C_Origin;
using VecMat::C_Vertex;
using VecMat::C_Vector;
using VecMat::C_Normal;

using VecMat::RotX;
using VecMat::RotY;
using VecMat::RotZ;

using VecMat::AxisX;
using VecMat::AxisY;
using VecMat::AxisZ;

using VecMat::C_MatrixScale;
using VecMat::C_MatrixRotateRPH;
using VecMat::C_MatrixTranslate;
using VecMat::C_MatrixSRT;

using VecMat::HMat33;
using VecMat::HMat43;
using VecMat::HMat44;
using VecMat::C_Quaternion;




//
// *************************
// *** Support Functions ***
// *************************
//


///// Returns the reciprocal of a HF32
//FORCE_INLINE const HF32 Rcp( const HF32& src )
//{
//    return src.Rcp();
//}
//
///// Returns the reciprocal of a HF32
//FORCE_INLINE const HF32 Reciprocal( const HF32& src )
//{
//    return src.Rcp();
//}
//
///// Returns the reciprocal sqrt of a HF32
//FORCE_INLINE const HF32 RcpSqrt( const HF32& src )
//{
//    return src.RcpSqrt();
//}
//
///// Returns the reciprocal sqrt of a HF32
//FORCE_INLINE const HF32 ReciprocalSqrt( const HF32& src )
//{
//    return src.RcpSqrt();
//}

/// Returns the SignBits of a N dimensional Vector packed into N bits of an int
template < class Vector >
FORCE_INLINE const E_SignBits GetSignBits( const Vector& src )
{
    return src.GetSignBits();
}

/// Returns the average of the elements of a Vector
template < class Vector >
FORCE_INLINE const HF32 Average( const Vector& src )
{
    return src.Sum() * ( 1.0f / Vector::SIZE );
}

/// Returns the length of a Vector
template < class Vector >
FORCE_INLINE const HF32 Length( const Vector& src )
{
    return src.Length();
}

/// Returns the squared length of a Vector
template < class Vector >
FORCE_INLINE const HF32 Length2( const Vector& src )
{
    return src.Length2();
}

/// Returns the squared length of a Vector
template < class Vector >
FORCE_INLINE const HF32 LengthSquared( const Vector& src )
{
    return src.Length2();
}

/// Returns the reciprocal of the length of a Vector
template < class Vector >
FORCE_INLINE const HF32 RcpLength( const Vector& src )
{
    return src.ReciprocalLength();
}

/// Returns the reciprocal of the length of a Vector
template < class Vector >
FORCE_INLINE const HF32 ReciprocalLength( const Vector& src )
{
    return src.ReciprocalLength();
}

/// Returns a normalised vector
template < class Vector >
FORCE_INLINE const Vector Normalise( const Vector& src )
{
    return src * ReciprocalLength( src );
}

#ifndef GCC
// JAN: commented this out - ReciprocalSqrt not defined!

/// Returns a normalised vector - safe version with additional parameter to use for zero length vectors!
template < class Vector >
FORCE_INLINE const Vector SafeNormalise( const Vector& src, const Vector& zeroLengthResult )
{
    const HF32 length2( src.Length2() );
    if ( length2.IsNonZero() )
    {
		return src * ReciprocalSqrt( length2 );
    }
    else
    {
        return zeroLengthResult;
    }
}
#endif

/// Returns the dot product of two vectors
template < class Vector >
FORCE_INLINE const HF32 DotProduct( const Vector& lhs, const Vector& rhs )
{
    return lhs | rhs;
}

/// Returns the cross product of two vectors
template < class Vector >
FORCE_INLINE const typename Vector::CrossProductType CrossProduct( const Vector& lhs, const Vector& rhs )
{
    return lhs ^ rhs;
}

/// Swizzle
template < unsigned SX, unsigned SY, unsigned SZ, class Vector3 >
FORCE_INLINE const Vector3 Swizzle( const Vector3& src )
{
    return src.Swizzle<SX,SY,SZ>();
}

/// Horizontal sum of all elements
template < class Vector >
FORCE_INLINE const HF32 Sum( const Vector& src )
{
    return src.Sum();
}

/// Box Half Surface Area : Vector represents dimensions of box
template < class Vector3 >
FORCE_INLINE const HF32 BoxHalfSurfaceArea( const Vector3& dimensionsXYZ )
{
    using namespace VecMat;
    const Vector3 dimensionsYZX( Swizzle<VY,VZ,VX>( dimensionsXYZ ) );
    // This generates: warning LNK4221: no public symbols found; archive member will be inaccessible. Why?!?
    //const HF32 halfSurfaceArea( Sum( xyz * yzx ) );
    return ( dimensionsXYZ * dimensionsYZX ).Sum();
}

/// Box Surface Area : Vector represents dimensions of box
template < class Vector3 >
FORCE_INLINE const HF32 BoxSurfaceArea( const Vector3& dimensionsXYZ )
{
    const HF32 halfSurfaceArea( BoxHalfSurfaceArea( dimensionsXYZ ) );
    return halfSurfaceArea + halfSurfaceArea;
}

/// Returns the bitwise AND of the two inputs: lhs & rhs
template < class Vector >
FORCE_INLINE const Vector AND( const Vector& lhs, const Vector& rhs )
{
    return lhs.AND( rhs );
}

/// Returns the bitwise AND-NOT of the two inputs: ~lhs & rhs
template < class Vector >
FORCE_INLINE const Vector ANDNOT( const Vector& lhs, const Vector& rhs )
{
    return lhs.ANDNOT( rhs );
}

/// Returns the bitwise OR of the two inputs: lhs | rhs
template < class Vector >
FORCE_INLINE const Vector OR( const Vector& lhs, const Vector& rhs )
{
    return lhs.OR( rhs );
}

/// Returns the bitwise XOR of the two inputs: lhs ^ rhs
template < class Vector >
FORCE_INLINE const Vector XOR( const Vector& lhs, const Vector& rhs )
{
    return lhs.XOR( rhs );
}


/// Returns the transpose of the matrix
template < class Matrix >
const Matrix Transpose( const Matrix& src )
{
    return src.Transposed();
}

/// Returns the inverse of the matrix
template < class Matrix >
const Matrix Inverse( const Matrix& src )
{
    return src.Inverse();
}

/// Returns the transpose of the inverse of a matrix ( for transforming normals ).
/// INFO: http://www.gignews.com/realtime020100.htm
template < class Matrix >
const Matrix TransposeInverse( const Matrix& src )
{
    return Transpose( Inverse( src ) );
}

