
#ifndef VECMAT_COMPILED
#define VECMAT_COMPILED
#ifndef GCC
#pragma once
#endif

#include <core\maths\storage.h>

// 
// Hardware Vector Classes
// Designed for optimal hardware acceleration
// 
//namespace VecMat
//{
    // Forward declarations of all classes
    //class HVecBase;
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
    class Plane;
    class Quaternion;
    class Line;
    class LineSphereIntersect;
    class Sphere;
    
    typedef enum { VX = 0, VY = 1, VZ = 2, VW = 3 } VecElement;
    typedef enum { MX = 0, MY = 1, MZ = 2, MW = 3 } MatAxis;
    typedef enum { QX = 0, QY = 1, QZ = 2, QA = 3 } QuatElement;
    typedef enum { SX = 0, SY = 1, SZ = 2, SR = 3 } SphereElement;
    
	enum EulerOrder
	{
		EULER_XYZ,
		EULER_XZY,
		EULER_YXZ,
		EULER_YZX,
		EULER_ZXY,
		EULER_ZYX
	};

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
    static const U32 c_iSignMask = 0x7FFFFFFF;
    
    static const F32 c_fSet      = (F32&)c_iSet;
    static const F32 c_fClear    = (F32&)c_iClear;
    static const F32 c_fSignMask = (F32&)c_iSignMask;
    
    //                            { X              Y              Z              W             }
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
    static ConstV4F32( c_Zero )   = { 0.0f, 0.0f, 0.0f, 0.0f };
    static ConstV4F32( c_Half )   = { 0.5f, 0.5f, 0.5f, 0.5f };
    static ConstV4F32( c_One )    = { 1.0f, 1.0f, 1.0f, 1.0f };
    static ConstV4F32( c_Three )  = { 3.0f, 3.0f, 3.0f, 3.0f };
    
    static ConstV4F32( c_SignMask ) = { c_fSignMask, c_fSignMask, c_fSignMask, c_fSignMask };
    
    static ConstV4F32( c_Epsilon ) = { FLT_EPSILON, FLT_EPSILON, FLT_EPSILON, FLT_EPSILON };
    
    static ConstV4F32( c_P1M1ZZ ) = { +1, -1, 0, 0 }; // Plus 1, Minus 1, Zero, Zero
    
    //
    // SSE & SSE2 Helper Functions
    //
    
    #ifdef _SSE_
    
    // Shuffle : 1 parameter
    template < int Ax, int Ay, int Az, int Aw >
    inline const V128 Shuffle( const V128 A )
    {
        return _mm_shuffle_ps( A, A, _MM_SHUFFLE( Aw, Az, Ay, Ax ) );
    }
    
    // Shuffle : 2 parameters
    template < int Ax, int Ay, int Bz, int Bw >
    inline const V128 Shuffle( const V128 A, const V128 B )
    {
        return _mm_shuffle_ps( A, B, _MM_SHUFFLE( Bw, Bz, Ay, Ax ) );
    }
    
    // Shuffle : 3 parameters
    template < int Ax, int By, int Cz, int Cw >
    inline const V128 Shuffle( const V128 A, const V128 B, const V128 C )
    {
        return Shuffle< 0, 2, Cz, Cw >( Shuffle< Ax, Ax, By, By >( A, B ), C );
    }
    
    // Shuffle : 4 parameters
    template < int Ax, int By, int Cz, int Dw >
    inline const V128 Shuffle( const V128 A, const V128 B, const V128 C, const V128& D )
    {
        return Shuffle< 0, 2, 0, 2 >( Shuffle< Ax, Ax, By, By >( A, B ), Shuffle< Cz, Cz, Dw, Dw >( C, D ) );
    }
    
    // Replicate X into XYZW
    inline const V128 ReplicateX( const V128 src )
    {
        return Shuffle<VX,VX,VX,VX>( src );
    }
    
    // Replicate Y into XYZW
    inline const V128 ReplicateY( const V128 src )
    {
        return Shuffle<VY,VY,VY,VY>( src );
    }
    
    // Replicate Z into XYZW
    inline const V128 ReplicateZ( const V128 src )
    {
        return Shuffle<VZ,VZ,VZ,VZ>( src );
    }
    
    // Replicate W into XYZW
    inline const V128 ReplicateW( const V128 src )
    {
        return Shuffle<VW,VW,VW,VW>( src );
    }
    
    // Approx Reciprocal : 11 bit accuracy
    inline const V128 ApproxRecipSS( const V128 x ) // Scalar: X only
    {
        // Needs check for div-zero!
        return _mm_rcp_ss( x );
    }
    inline const V128 ApproxRecipPS( const V128 x ) // Packed: XYZW
    {
        // Needs check for div-zero!
        return _mm_rcp_ps( x );
    }
    
    // Approx Reciprocal + Newton-Raphson Iteration : 22 bit accuracy
    inline const V128 RecipSS( const V128 x ) // Scalar: X only
    {
        // Needs check for div-zero!
        const V128 rx = _mm_rcp_ss( x );
        return _mm_sub_ss( _mm_add_ss( rx, rx ), _mm_mul_ss( x, _mm_mul_ss( rx, rx ) ) );
    }
    inline const V128 RecipPS( const V128 x ) // Packed: XYZW
    {
        // Needs check for div-zero!
        const V128 rx = _mm_rcp_ps( x );
        return _mm_sub_ps( _mm_add_ps( rx, rx ), _mm_mul_ps( x, _mm_mul_ps( rx, rx ) ) );
    }
    
    // Approx Reciprocal Sqrt : 11 bit accuracy
    inline const V128 ApproxRecipSqrtSS( const V128 x ) // Scalar: X only
    {
        const V128 result = _mm_rsqrt_ss( x );
        return _mm_and_ps( _mm_cmpgt_ss( x, c_Epsilon ), result );
    }
    inline const V128 ApproxRecipSqrtPS( const V128 x ) // Packed: XYZW
    {
        const V128 result = _mm_rsqrt_ps( x );
        return _mm_and_ps( _mm_cmpgt_ps( x, c_Epsilon ), result );
    }
    
    // Approx Reciprocal Sqrt + Newton-Raphson Iteration : 22 bit accuracy
    inline const V128 RecipSqrtSS( const V128 x ) // Scalar: X only
    {
        const V128 rsx = _mm_rsqrt_ss( x );
        const V128 result = _mm_mul_ss( _mm_mul_ss( c_Half, rsx ), _mm_sub_ss( c_Three, _mm_mul_ss( x, _mm_mul_ss( rsx, rsx ) ) ) );
        return _mm_and_ps( _mm_cmpgt_ss( x, c_Epsilon ), result );
    }
    inline const V128 RecipSqrtPS( const V128 x ) // Packed: XYZW
    {
        const V128 rsx = _mm_rsqrt_ps( x );
        const V128 result = _mm_mul_ps( _mm_mul_ps( c_Half, rsx ), _mm_sub_ps( c_Three, _mm_mul_ps( x, _mm_mul_ps( rsx, rsx ) ) ) );
        return _mm_and_ps( _mm_cmpgt_ps( x, c_Epsilon ), result );
    }
    
    #endif
    
    
    // RGBA Class
    class RGBA
    {
    public:
        
        RGBA()
        {
        }
        
        RGBA( U8 r, U8 g, U8 b, U8 a )
        {
            colour = ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;
        }
        
        RGBA( U8 r, U8 g, U8 b )
        {
            colour = ( 0xFF << 24 ) | ( r << 16 ) | ( g << 8 ) | b;
        }
        
        RGBA( U32 rgba )
        {
            colour = rgba;
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

		#ifdef VEC_MAT_PRINT
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
    
    typedef struct { F32 array[2]; } Array2F32;
    typedef struct { F32 array[3]; } Array3F32;
    typedef struct { F32 array[4]; } Array4F32;
    
    
    //
    // Hardware 1 Element F32 Vector X Component Class
    //
    
    ALIGN16 class HVecX
    {
    friend class HVecY;
    friend class HVecZ;
    friend class HVecW;
    friend class HVec2;
    friend class HVec3;
    friend class HVec4;
    
    public:
        
        HVecX() { }
        
        #ifdef _SSE_
        
        // Explicit construction from unknown data due to potential for data loss!
        explicit HVecX( const V128& src )   { m_X = src; }
        
        operator const V128 () const        { return m_X; }
        
        #endif
        
        const F32 X() const
        {
            #ifdef _SSE_
            F32 x;
            _mm_store_ss( &x, m_X );
            return x;
            #else
            return m_X;
            #endif
        }
        
        void X( const F32 x )
        {
            #ifdef _SSE_
            m_X = _mm_load_ss( &x );
            #else
            m_X = x;
            #endif
        }
        
        HVecX( const F32 x )                { X( x ); }
        HVecX( const HVecX& src )           { m_X = src.m_X; }
        
        explicit HVecX( const HVecY& src );
        explicit HVecX( const HVecZ& src );
        explicit HVecX( const HVecW& src );
        
        operator const F32 () const         { return X(); }
        
        const HVecX& operator = ( const HVecX& src )
        {
            m_X = src.m_X;
            return *this;
        }
        
        const HVecX operator + ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return HVecX( _mm_add_ss( m_X, src.m_X ) );
            #else
            return HVecX( m_X + src.m_X );
            #endif
        }
        
        const HVecX operator - ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return HVecX( _mm_sub_ss( m_X, src.m_X ) );
            #else
            return HVecX( m_X - src.m_X );
            #endif
        }
        
        const HVecX operator * ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return HVecX( _mm_mul_ss( m_X, src.m_X ) );
            #else
            return HVecX( m_X * src.m_X );
            #endif
        }
        
        const HVecX operator / ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return HVecX( _mm_div_ss( m_X, src.m_X ) );
            #else
            return HVecX( m_X / src.m_X );
            #endif
        }
        
        const HVecX operator + ( const F32 src ) const      { return *this + HVecX( src ); }
        const HVecX operator - ( const F32 src ) const      { return *this - HVecX( src ); }
        const HVecX operator * ( const F32 src ) const      { return *this * HVecX( src ); }
        const HVecX operator / ( const F32 src ) const      { return *this / HVecX( src ); }
        
        // Min
        const HVecX operator << ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return HVecX( _mm_min_ss( m_X, src.m_X ) );
            #else
            return HVecX( min( m_X, src.m_X ) );
            #endif
        }
        
        // Max
        const HVecX operator >> ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return HVecX( _mm_max_ss( m_X, src.m_X ) );
            #else
            return HVecX( max( m_X, src.m_X ) );
            #endif
        }
        
        const bool operator == ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return _mm_movemask_ps( _mm_cmpeq_ps( m_X, src.m_X ) ) & 1;
            #else
            return m_X == src.m_X;
            #endif
        }
        
        const bool operator != ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return _mm_movemask_ps( _mm_cmpneq_ps( m_X, src.m_X ) ) & 1;
            #else
            return m_X != src.m_X;
            #endif
        }
        
        const bool operator <= ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return _mm_movemask_ps( _mm_cmple_ps( m_X, src.m_X ) ) & 1;
            #else
            return m_X <= src.m_X;
            #endif
        }
        
        const bool operator < ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return _mm_movemask_ps( _mm_cmplt_ps( m_X, src.m_X ) ) & 1;
            #else
            return m_X < src.m_X;
            #endif
        }
        
        const bool operator >= ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return _mm_movemask_ps( _mm_cmpge_ps( m_X, src.m_X ) ) & 1;
            #else
            return m_X >= src.m_X;
            #endif
        }
        
        const bool operator > ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return _mm_movemask_ps( _mm_cmpgt_ps( m_X, src.m_X ) ) & 1;
            #else
            return m_X > src.m_X;
            #endif
        }
        
        const bool operator == ( const F32 src ) const { return *this == HVecX( src ); }
        const bool operator != ( const F32 src ) const { return *this != HVecX( src ); }
        const bool operator <= ( const F32 src ) const { return *this <= HVecX( src ); }
        const bool operator <  ( const F32 src ) const { return *this <  HVecX( src ); }
        const bool operator >= ( const F32 src ) const { return *this >= HVecX( src ); }
        const bool operator >  ( const F32 src ) const { return *this >  HVecX( src ); }
        
        // Address-of: Illegal - return type is void
        void operator & () const {};
        
        const HVecX Abs() const
        {
            #ifdef _SSE_
            return HVecX( _mm_and_ps( c_SignMask, m_X ) );
            #else
            return HVecX( fabsf( m_X ) );
            #endif
        }
        
        const HVecX Sqrt() const
        {
            #ifdef _SSE_
            return HVecX( _mm_sqrt_ss( m_X ) );
            #else
            return HVecX( sqrt( m_X ) );
            #endif
        }

		#ifdef VEC_MAT_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s HVecX( %+8.3f )\n", text, X() );
            OutputDebugString( buffer );
        }
		#endif
       
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
    friend class HVecX;
    friend class HVecZ;
    friend class HVecW;
    friend class HVec2;
    friend class HVec3;
    friend class HVec4;
    
    public:
        
        HVecY() { }
        
        #ifdef _SSE_
        
        // Explicit construction from unknown data due to potential for data loss!
        explicit HVecY( const V128& src )   { m_Y = src; }
        
        operator const V128 () const        { return m_Y; }
        
        #endif
        
        const F32 Y() const
        {
            #ifdef _SSE_
            F32 y;
            _mm_store_ss( &y, ReplicateY( m_Y ) );
            return y;
            #else
            return m_Y;
            #endif
        }
        
        void Y( const F32 y )
        {
            #ifdef _SSE_
            m_Y = Shuffle<1,0,1,1>( _mm_load_ss( &y ) );
            #else
            m_Y = y;
            #endif
        }
        
        HVecY( const F32 y )                { Y( y ); }
        HVecY( const HVecY& src )           { m_Y = src.m_Y; }
        
        explicit HVecY( const HVecX& src );
        explicit HVecY( const HVecZ& src );
        explicit HVecY( const HVecW& src );
        
        operator const F32 () const         { return Y(); }
        
        const HVecY& operator = ( const HVecY& src )
        {
            m_Y = src.m_Y;
            return *this;
        }
        
        // Address-of: Illegal - return type is void
        void operator & () const {};
        
		#ifdef VEC_MAT_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s HVecY( %+8.3f )\n", text, Y() );
            OutputDebugString( buffer );
        }
		#endif
        
       
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
    friend class HVecX;
    friend class HVecY;
    friend class HVecW;
    friend class HVec2;
    friend class HVec3;
    friend class HVec4;
    
    public:
        
        HVecZ() { }
        
        #ifdef _SSE_
        
        // Explicit construction from unknown data due to potential for data loss!
        explicit HVecZ( const V128& src )   { m_Z = src; }
        
        operator const V128 () const        { return m_Z; }
        
        #endif
        
        const F32 Z() const
        {
            #ifdef _SSE_
            F32 z;
            _mm_store_ss( &z, ReplicateZ( m_Z ) );
            return z;
            #else
            return m_Z;
            #endif
        }
        
        void Z( const F32 z )
        {
            #ifdef _SSE_
            m_Z = Shuffle<1,1,0,1>( _mm_load_ss( &z ) );
            #else
            m_Z = z;
            #endif
        }
        
        HVecZ( const F32 z )                { Z( z ); }
        HVecZ( const HVecZ& src )           { m_Z = src.m_Z; }
        
        explicit HVecZ( const HVecX& src );
        explicit HVecZ( const HVecY& src );
        explicit HVecZ( const HVecW& src );
        
        operator const F32 () const         { return Z(); }
        
        const HVecZ& operator = ( const HVecZ& src )
        {
            m_Z = src.m_Z;
            return *this;
        }
        
        // Address-of: Illegal - return type is void
        void operator & () const {};

		#ifdef VEC_MAT_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s HVecZ( %+8.3f )\n", text, Z() );
            OutputDebugString( buffer );
        }
		#endif
        
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
    friend class HVecX;
    friend class HVecY;
    friend class HVecZ;
    friend class HVec2;
    friend class HVec3;
    friend class HVec4;
    
    public:
        
        HVecW() { }
        
        #ifdef _SSE_
        
        // Explicit construction from unknown data due to potential for data loss!
        explicit HVecW( const V128& src )   { m_W = src; }
        
        operator const V128 () const        { return m_W; }
        
        #endif
        
        const F32 W() const
        {
            #ifdef _SSE_
            F32 w;
            _mm_store_ss( &w, ReplicateW( m_W ) );
            return w;
            #else
            return m_W;
            #endif
        }
        
        void W( const F32 w )
        {
            #ifdef _SSE_
            m_W = Shuffle<1,1,1,0>( _mm_load_ss( &w ) );
            #else
            m_W = w;
            #endif
        }
        
        HVecW( const F32 w )                { W( w ); }
        HVecW( const HVecW& src )           { m_W = src.m_W; }
        
//		explicit HVecW( const HVecX& src );
//		explicit HVecW( const HVecY& src );
//		explicit HVecW( const HVecZ& src );
        
        operator const F32 () const         { return W(); }
        
        const HVecW& operator = ( const HVecW& src )
        {
            m_W = src.m_W;
            return *this;
        }
        
        const HVecW operator + ( const HVecW& src ) const
        {
            #ifdef _SSE_
            return HVecW( _mm_add_ps( m_W, src.m_W ) );
            #else
            return HVecW( m_W + src.m_W );
            #endif
        }
        
        // Address-of: Illegal - return type is void
        void operator & () const {};
        
		#ifdef VEC_MAT_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s HVecW( %+8.3f )\n", text, W() );
            OutputDebugString( buffer );
        }
		#endif
        
        #ifdef _SSE_
        V128    m_W;
        #else
        F32     m_W;
        #endif
    };
    
    
    //
    // Hardware 2 Element F32 Vector Class
    //
    
    ALIGN16 class HVec2
    {
    friend class HVec3;
    friend class HVec4;
    
    public:
        
        FORCE_INLINE HVec2() { }
        
        #ifdef _SSE_
        
        // Explicit construction from unknown data due to potential for data loss!
        explicit HVec2( const V128& src )   { m_V = src; }
        
        operator const V128 () const        { return m_V; }
        
        HVec2( const HVec2& src )           { m_V = src.m_V; }
        
        const HVec2& operator = ( const HVec2& src )    { m_V = src.m_V; return *this; }
        
        #endif
        
        HVec2( const HVecX& src )
        {
            #ifdef _SSE_
            m_V = src.m_X;
            #else
            m_V[VX] = src.m_X;
            m_V[VY] = 0.0f;
            #endif
        }
        
        HVec2( const HVecY& src )
        {
            #ifdef _SSE_
            m_V = src.m_Y;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = src.m_Y;
            #endif
        }
        
        // Explicit construction due to data loss!
        explicit HVec2( const HVec3& src );
        explicit HVec2( const HVec4& src );
        
        const F32 X() const
        {
            #ifdef _SSE_
            F32 x;
            _mm_store_ss( &x, m_V );
            return x;
            #else
            return m_V[VX];
            #endif
        }
        
        const F32 Y() const
        {
            #ifdef _SSE_
            F32 y;
            _mm_store_ss( &y, ReplicateY( m_V ) );
            return y;
            #else
            return m_V[VY];
            #endif
        }
        
        void X( const F32 x )
        {
            #ifdef _SSE_
            const V128 vx = _mm_load_ss( &x );
            m_V = Shuffle<0,2,2,3>( Shuffle<0,0,1,1>( vx, m_V ), m_V );
            #else
            m_V[VX] = x;
            #endif
        }
        
        void Y( const F32 y )
        {
            #ifdef _SSE_
            const V128 vy = _mm_load_ss( &y );
            m_V = Shuffle<0,2,2,3>( Shuffle<0,0,0,0>( m_V, vy ), m_V );
            #else
            m_V[VY] = y;
            #endif
        }
        
        FORCE_INLINE HVec2( const F32* array )
        {
            #ifdef _SSE_
            m_V = _mm_and_ps( c_MaskXY, _mm_loadu_ps( array ) );
            #else
            m_V[VX] = array[VX];
            m_V[VY] = array[VY];
            #endif
        }
        
        FORCE_INLINE HVec2( F32 x, F32 y )
        {
            #ifdef _SSE_
            m_V = _mm_set_ps( 0.0f, 0.0f, y, x );
            #else
            m_V[VX] = x;
            m_V[VY] = y;
            #endif
        }
        
        explicit HVec2( const F32 xy )
        {
            #ifdef _SSE_
            m_V = Shuffle<0,0,1,1>( _mm_load_ss( &xy ) );
            #else
            m_V[VX] = xy;
            m_V[VY] = xy;
            #endif
        }
        
        operator const HVecX () const
        {
            #ifdef _SSE_
            return HVecX( _mm_and_ps( c_MaskX, m_V ) );
            #else
            return HVecX( m_V[VX] );
            #endif
        }
        
        operator const HVecY () const
        {
            #ifdef _SSE_
            return HVecY( _mm_and_ps( c_MaskY, m_V ) );
            #else
            return HVecY( m_V[VY] );
            #endif
        }
        
        const HVec2& operator = ( const HVecX& src )
        {
            #ifdef _SSE_
            m_V = src.m_X;
            #else
            m_V[VX] = src.m_X;
            m_V[VY] = 0.0f;
            #endif
            return *this;
        }
        
		bool operator == ( const HVec2 &b)
		{
			return x == b.x && y == b.y;
		}

        const HVec2& operator = ( const HVecY& src )
        {
            #ifdef _SSE_
            m_V = src.m_Y;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = src.m_Y;
            #endif
            return *this;
        }
        
        const HVec2 operator - () const
        {
            #ifdef _SSE_
            return HVec2( _mm_sub_ps( _mm_setzero_ps(), m_V ) );
            #else
            return HVec2( -m_V[VX],
                          -m_V[VY] );
            #endif
        }
        
        FORCE_INLINE const HVec2 operator + ( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_add_ps( m_V, src.m_V ) );
            #else
            return HVec2( m_V[VX] + src.m_V[VX],
                          m_V[VY] + src.m_V[VY] );
            #endif
        }
        
        FORCE_INLINE const HVec2 operator - ( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_sub_ps( m_V, src.m_V ) );
            #else
            return HVec2( m_V[VX] - src.m_V[VX],
                          m_V[VY] - src.m_V[VY] );
            #endif
        }
        
        FORCE_INLINE const HVec2 operator * ( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_mul_ps( m_V, src.m_V ) );
            #else
            return HVec2( m_V[VX] * src.m_V[VX],
                          m_V[VY] * src.m_V[VY] );
            #endif
        }
        
        const HVec2 operator * ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_mul_ps( m_V, ReplicateX( src.m_X ) ) );
            #else
            return HVec2( m_V[VX] * src,
                          m_V[VY] * src );
            #endif
        }
        
        const HVec2 operator / ( const HVec2& src ) const
        {
            #ifdef _SSE_
            return HVec2( _mm_mul_ps( m_V, RecipPS( src.m_V ) ) );
            #else
            return HVec2( m_V[VX] / src.m_V[VX],
                          m_V[VY] / src.m_V[VY] );
            #endif
        }

		FORCE_INLINE const void Normalize()
		{
			float f = 1.0f/sqrtf( m_V[0]*m_V[0]+m_V[1]*m_V[1] );
			m_V[0] *= f;
			m_V[1] *= f;
		}

        // Lerp
        const HVec2 Lerp( const HVec2& dst, const HVecX& f ) const
        {
            const HVec2& src = *this;
            return src + ( ( dst - src ) * f );
        }
        
        // Dot product
        const F32 operator | ( const HVec2& src ) const
        {
            return m_V[VX] * src.m_V[VX] +
                   m_V[VY] * src.m_V[VY];
		}

        // Dot product
        const HVec2 operator * ( const float scale ) const
        {
			HVec2 out;

			out.m_V[VX] = m_V[VX] * scale;
			out.m_V[VY] = m_V[VY] * scale;

			return out;
		}

        // Address-of: Illegal - return type is void
        //void operator & () const {};
        
		#ifdef VEC_MAT_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s HVec2( %+8.3f, %+8.3f )\n", text, X(), Y() );
            OutputDebugString( buffer );
        }
		#endif
        
        #ifndef _SSE_
        const F32& operator [] ( const U32 index ) const
        {
            return m_V[index];
        }

		F32& operator [] ( const U32 index )
        {
            return m_V[index];
        }

        #endif
        
        #ifdef _SSE_
        V128    m_V;
        #else
		union
		{
			struct
			{
				float x,y;
			};
			F32     m_V[2];
		};
        #endif
    };
    
    
    //
    // Hardware 3 Element F32 Vector Class
    //
    
	namespace UtMaths
	{
		template<class T> class VectorXYZ;

		template<class T> class MVector;
		template<class T> class MPoint;
		template<class T> class MNormal;
	}
	typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >	UtMVectorXYZ;
	typedef UtMaths::MPoint<UtMaths::VectorXYZ<F32> >	UtMPointXYZ;
	typedef UtMaths::MNormal<UtMaths::VectorXYZ<F32> >	UtMNormalXYZ;
		
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
    friend class Quaternion;
    
    public:
		HVec3(const UtMVectorXYZ& p);

		const HVec3& operator = (const UtMVectorXYZ& p);

		HVec3(const UtMPointXYZ& p);

		const HVec3& operator = (const UtMPointXYZ& p);

		HVec3(const UtMNormalXYZ& p);

		const HVec3& operator = (const UtMNormalXYZ& p);

        HVec3() { }
        
        #ifdef _SSE_
        
        // Explicit construction from unknown data due to potential for data loss!
        explicit HVec3( const V128& src )   { m_V = src; }
        
        operator const V128 () const        { return m_V; }
        
        HVec3( const HVec3& src )           { m_V = src.m_V; DebugValidate(); }
        
        const HVec3& operator = ( const HVec3& src )    { m_V = src.m_V; DebugValidate(); return *this; }
        
        #endif

		operator MathsStorage::Float3() const
		{
			MathsStorage::Float3 rtn;
			rtn.x = x, rtn.y = y, rtn.z = z;
			return rtn;
		}

		HVec3(const MathsStorage::Float3 &a)
		{
			x = a.x, y = a.y, z = a.z;
		}

		HVec3( const HVecX& src )
        {
            #ifdef _SSE_
            m_V = src.m_X;
            #else
            m_V[VX] = src.m_X;
            m_V[VY] = 0.0f;
            m_V[VZ] = 0.0f;
            #endif
        }
        
        HVec3( const HVecY& src )
        {
            #ifdef _SSE_
            m_V = src.m_Y;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = src.m_Y;
            m_V[VZ] = 0.0f;
            #endif
        }
        
        HVec3( const HVecZ& src )
        {
            #ifdef _SSE_
            m_V = src.m_Z;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            m_V[VZ] = src.m_Z;
            #endif
        }
        
        HVec3( const HVec2& src )
        {
            #ifdef _SSE_
            m_V = src.m_V;
            #else
            m_V[VX] = src.m_V[VX];
            m_V[VY] = src.m_V[VY];
            m_V[VZ] = 0.0f;
            #endif
        }
        
        // Explicit construction due to data loss!
        explicit HVec3( const HVec4& src );
        
		// Removed Havok

        ///* explicit */ HVec3( const hkVector4& src )
        //{
        //    #ifdef _SSE_
        //    m_V = _mm_set_ps( 0.0f, src.getSimdAt(2), src.getSimdAt(1), src.getSimdAt(0) );
        //    #else
        //    m_V[VX] = src.getSimdAt(0);
        //    m_V[VY] = src.getSimdAt(1);
        //    m_V[VZ] = src.getSimdAt(2);
        //    #endif
        //}
        //
        //operator const hkVector4() const
        //{
        //    return hkVector4( X(), Y(), Z(), 0.0f );
        //}
        
        const F32 X() const
        {
            #ifdef _SSE_
            F32 x;
            _mm_store_ss( &x, m_V );
            return x;
            #else
            return m_V[VX];
            #endif
        }
        
        const F32 Y() const
        {
            #ifdef _SSE_
            F32 y;
            _mm_store_ss( &y, ReplicateY( m_V ) );
            return y;
            #else
            return m_V[VY];
            #endif
        }
        
        const F32 Z() const
        {
            #ifdef _SSE_
            F32 z;
            _mm_store_ss( &z, ReplicateZ( m_V ) );
            return z;
            #else
            return m_V[VZ];
            #endif
        }
        
        #ifdef _SSE_
        const F32 W() const
        {
            F32 w;
            _mm_store_ss( &w, ReplicateW( m_V ) );
            return w;
        }
        #endif
        
        void X( const F32 x )
        {
            #ifdef _SSE_
            const V128 vx = _mm_load_ss( &x );
            m_V = Shuffle<0,2,2,3>( Shuffle<0,0,1,1>( vx, m_V ), m_V );
            #else
            m_V[VX] = x;
            #endif
        }
        
        void Y( const F32 y )
        {
            #ifdef _SSE_
            const V128 vy = _mm_load_ss( &y );
            m_V = Shuffle<0,2,2,3>( Shuffle<0,0,0,0>( m_V, vy ), m_V );
            #else
            m_V[VY] = y;
            #endif
        }
        
        void Z( const F32 z )
        {
            #ifdef _SSE_
            const V128 vz = _mm_load_ss( &z );
            m_V = Shuffle<0,1,0,2>( m_V, Shuffle<0,0,3,3>( vz, m_V ) );
            #else
            m_V[VZ] = z;
            #endif
        }
        
        HVec3( const F32* array )
        {
            #ifdef _SSE_
            m_V = _mm_and_ps( c_MaskXYZ, _mm_loadu_ps( array ) );
            #else
            m_V[VX] = array[VX];
            m_V[VY] = array[VY];
            m_V[VZ] = array[VZ];
            #endif
        }
        
        HVec3( F32 x, F32 y, F32 z )
        {
            #ifdef _SSE_
            m_V = _mm_set_ps( 0.0f, z, y, x );
            #else
            m_V[VX] = x;
            m_V[VY] = y;
            m_V[VZ] = z;
            #endif
        }
        
        explicit HVec3( const F32 xyz )
        {
            #ifdef _SSE_
            m_V = Shuffle<0,0,0,1>( _mm_load_ss( &xyz ) );
            #else
            m_V[VX] = xyz;
            m_V[VY] = xyz;
            m_V[VZ] = xyz;
            #endif
        }
        
        operator const HVecX () const
        {
            #ifdef _SSE_
            return HVecX( _mm_and_ps( c_MaskX, m_V ) );
            #else
            return HVecX( m_V[VX] );
            #endif
        }
        
        operator const HVecY () const
        {
            #ifdef _SSE_
            return HVecY( _mm_and_ps( c_MaskY, m_V ) );
            #else
            return HVecY( m_V[VY] );
            #endif
        }
        
        operator const HVecZ () const
        {
            #ifdef _SSE_
            return HVecZ( _mm_and_ps( c_MaskZ, m_V ) );
            #else
            return HVecZ( m_V[VZ] );
            #endif
        }
        
        const HVec3& operator = ( const HVecX& src )
        {
            #ifdef _SSE_
            m_V = src.m_X;
            #else
            m_V[VX] = src.m_X;
            m_V[VY] = 0.0f;
            m_V[VZ] = 0.0f;
            #endif
            return *this;
        }
        
        const HVec3& operator = ( const HVecY& src )
        {
            #ifdef _SSE_
            m_V = src.m_Y;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = src.m_Y;
            m_V[VZ] = 0.0f;
            #endif
            return *this;
        }
        
        const HVec3& operator = ( const HVecZ& src )
        {
            #ifdef _SSE_
            m_V = src.m_Z;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            m_V[VZ] = src.m_Z;
            #endif
            return *this;
        }
        
        const HVec3& operator = ( const HVec2& src )
        {
            #ifdef _SSE_
            m_V = src.m_V;
            #else
            m_V[VX] = src.m_V[VX];
            m_V[VY] = src.m_V[VY];
            m_V[VZ] = 0.0f;
            #endif
            return *this;
        }
        
        const HVec3& operator = ( const F32* array )
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
        // Unary Operators
        //
        
        // Unary Negation
        FORCE_INLINE const HVec3 operator - () const
        {
            #ifdef _SSE_
            return HVec3( _mm_sub_ps( _mm_setzero_ps(), m_V ) );
            #else
            return HVec3( -m_V[VX], -m_V[VY], -m_V[VZ] );
            #endif
        }
        
        //
        // Binary HVec3 Operators
        //
        
        // Binary Addition
        FORCE_INLINE const HVec3 operator + ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_add_ps( m_V, src.m_V ) );
            #else
            return HVec3( m_V[VX] + src.m_V[VX], m_V[VY] + src.m_V[VY], m_V[VZ] + src.m_V[VZ] );
            #endif
        }
        FORCE_INLINE HVec3& operator += ( const HVec3& src )
        {
            #ifdef _SSE_
            m_V = _mm_add_ps( m_V, src.m_V );
            #else
            m_V[VX] += src.m_V[VX];
            m_V[VY] += src.m_V[VY];
            m_V[VZ] += src.m_V[VZ];
            #endif
            return *this;
        }
        
        // Binary Subtraction
        FORCE_INLINE const HVec3 operator - ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_sub_ps( m_V, src.m_V ) );
            #else
            return HVec3( m_V[VX] - src.m_V[VX], m_V[VY] - src.m_V[VY], m_V[VZ] - src.m_V[VZ] );
            #endif
        }
        FORCE_INLINE HVec3& operator -= ( const HVec3& src )
        {
            #ifdef _SSE_
            m_V = ( _mm_sub_ps( m_V, src.m_V ) );
            #else
            m_V[VX] -= src.m_V[VX];
            m_V[VY] -= src.m_V[VY];
            m_V[VZ] -= src.m_V[VZ];
            #endif
            return *this;
        }
        
        // Binary Multiplication
        FORCE_INLINE const HVec3 operator * ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, src.m_V ) );
            #else
            return HVec3( m_V[VX] * src.m_V[VX], m_V[VY] * src.m_V[VY], m_V[VZ] * src.m_V[VZ] );
            #endif
        }
        FORCE_INLINE HVec3& operator *= ( const HVec3& src )
        {
            #ifdef _SSE_
            m_V = _mm_mul_ps( m_V, src.m_V );
            #else
            m_V[VX] *= src.m_V[VX];
            m_V[VY] *= src.m_V[VY];
            m_V[VZ] *= src.m_V[VZ];
            #endif
            return *this;
        }
        
        // Binary Division
        FORCE_INLINE const HVec3 operator / ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, RecipPS( src.m_V ) ) );
            #else
            return HVec3( m_V[VX] / src.m_V[VX], m_V[VY] / src.m_V[VY], m_V[VZ] / src.m_V[VZ] );
            #endif
        }
        FORCE_INLINE HVec3& operator /= ( const HVec3& src )
        {
            #ifdef _SSE_
            m_V = _mm_mul_ps( m_V, RecipPS( src.m_V ) );
            #else
            m_V[VX] /= src.m_V[VX];
            m_V[VY] /= src.m_V[VY];
            m_V[VZ] /= src.m_V[VZ];
            #endif
            return *this;
        }
        
        // Min
        FORCE_INLINE const HVec3 operator << ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_min_ps( m_V, src.m_V ) );
            #else
            return HVec3( min( m_V[VX], src.m_V[VX] ),
                          min( m_V[VY], src.m_V[VY] ),
                          min( m_V[VZ], src.m_V[VZ] ) );
            #endif
        }
        
        // Max
        FORCE_INLINE const HVec3 operator >> ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_max_ps( m_V, src.m_V ) );
            #else
            return HVec3( max( m_V[VX], src.m_V[VX] ),
                          max( m_V[VY], src.m_V[VY] ),
                          max( m_V[VZ], src.m_V[VZ] ) );
            #endif
        }
        
        //
        // Binary F32 Operators
        //
        
        const HVec3 operator + ( const F32 src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_add_ps( m_V, HVec3( src ).m_V ) );
            #else
            return HVec3( m_V[VX] + src, m_V[VY] + src, m_V[VZ] + src );
            #endif
        }
        HVec3& operator += ( const F32 src )
        {
            #ifdef _SSE_
            m_V = _mm_add_ps( m_V, HVec3( src ).m_V );
            #else
            m_V[VX] += src;
            m_V[VY] += src;
            m_V[VZ] += src;
            #endif
            return *this;
        }
        
        const HVec3 operator - ( const F32 src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_sub_ps( m_V, HVec3( src ).m_V ) );
            #else
            return HVec3( m_V[VX] - src, m_V[VY] - src, m_V[VZ] - src );
            #endif
        }
        HVec3& operator -= ( const F32 src )
        {
            #ifdef _SSE_
            m_V = _mm_sub_ps( m_V, HVec3( src ).m_V );
            #else
            m_V[VX] -= src;
            m_V[VY] -= src;
            m_V[VZ] -= src;
            #endif
            return *this;
        }
        
        const HVec3 operator * ( const F32 src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, HVec3( src ).m_V ) );
            #else
            return HVec3( m_V[VX] * src, m_V[VY] * src, m_V[VZ] * src );
            #endif
        }
        HVec3& operator *= ( const F32 src )
        {
            #ifdef _SSE_
            m_V = _mm_mul_ps( m_V, HVec3( src ).m_V );
            #else
            m_V[VX] *= src;
            m_V[VY] *= src;
            m_V[VZ] *= src;
            #endif
            return *this;
        }
        
        const HVec3 operator / ( const F32 src ) const
        {
            #ifdef _SSE_
            //return HVec3( _mm_mul_ps( m_V, (HVec3)RecipSS( HVecX( src ) ) ) );
            return HVec3( _mm_mul_ps( m_V, ReplicateX( RecipSS( HVecX( src ) ) ) ) );
            #else
            return HVec3( m_V[VX] / src, m_V[VY] / src, m_V[VZ] / src );
            #endif
        }
        HVec3& operator /= ( const F32 src )
        {
            #ifdef _SSE_
            //m_V = _mm_mul_ps( m_V, (HVec3)RecipSS( HVecX( src ) ) );
            m_V = _mm_mul_ps( m_V, ReplicateX( RecipSS( HVecX( src ) ) ) );
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
        
        const HVec3 operator * ( const HVecX& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, ReplicateX( src.m_X ) ) );
            #else
            return HVec3( m_V[VX] * src.m_X, m_V[VY] * src.m_X, m_V[VZ] * src.m_X );
            #endif
        }
        HVec3& operator *= ( const HVecX& src )
        {
            #ifdef _SSE_
            m_V = _mm_mul_ps( m_V, ReplicateX( src.m_X ) );
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
        
        const HVec3 operator * ( const HVecY& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, ReplicateY( src.m_Y ) ) );
            #else
            return HVec3( m_V[VX] * src.m_Y, m_V[VY] * src.m_Y, m_V[VZ] * src.m_Y );
            #endif
        }
        
        //
        // Binary HVecZ Operators
        //
        
        const HVec3 operator * ( const HVecZ& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, ReplicateZ( src.m_Z ) ) );
            #else
            return HVec3( m_V[VX] * src.m_Z, m_V[VY] * src.m_Z, m_V[VZ] * src.m_Z );
            #endif
        }
        
        //
        // Binary HVecW Operators
        //
        
        const HVec3 operator * ( const HVecW& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_mul_ps( m_V, ReplicateW( src.m_W ) ) );
            #else
            return HVec3( m_V[VX] * src.m_W, m_V[VY] * src.m_W, m_V[VZ] * src.m_W );
            #endif
        }
        
        //
        // Comparison Operators
        //
        
        const HVec3 operator == ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_cmpeq_ps( m_V, src.m_V ) );
            #else
            return HVec3( m_V[VX] == src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] == src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] == src.m_V[VZ] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec3 operator != ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_cmpneq_ps( m_V, src.m_V ) );
            #else
            return HVec3( m_V[VX] != src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] != src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] != src.m_V[VZ] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec3 operator <= ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_cmple_ps( m_V, src.m_V ) );
            #else
            return HVec3( m_V[VX] <= src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] <= src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] <= src.m_V[VZ] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec3 operator < ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_cmplt_ps( m_V, src.m_V ) );
            #else
            return HVec3( m_V[VX] < src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] < src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] < src.m_V[VZ] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec3 operator >= ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_cmpge_ps( m_V, src.m_V ) );
            #else
            return HVec3( m_V[VX] >= src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] >= src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] >= src.m_V[VZ] ? c_fSet : c_fClear );
            #endif
        }
        
        const HVec3 operator > ( const HVec3& src ) const
        {
            #ifdef _SSE_
            return HVec3( _mm_cmpgt_ps( m_V, src.m_V ) );
            #else
            return HVec3( m_V[VX] > src.m_V[VX] ? c_fSet : c_fClear,
                          m_V[VY] > src.m_V[VY] ? c_fSet : c_fClear,
                          m_V[VZ] > src.m_V[VZ] ? c_fSet : c_fClear );
            #endif
        }
        
        //
        // Other Binary Operators & Functions
        //
        
        const U32 SignMask() const
        {
            #ifdef _SSE_
            return _mm_movemask_ps( m_V ) & 0x7;
            #else
            return ( ( (U32&)m_V[VX] >> 31 ) & 0x1 ) |
                   ( ( (U32&)m_V[VY] >> 30 ) & 0x2 ) |
                   ( ( (U32&)m_V[VZ] >> 29 ) & 0x4 );
            #endif
        }
        
        // Horizontal sum of all elements
        const HVecX Sum() const
        {
            #ifdef _SSE_
            const V128 sSum = _mm_add_ss( m_V, _mm_add_ss( ReplicateY( m_V ), ReplicateZ( m_V ) ) );
            return HVecX( _mm_and_ps( c_MaskX, sSum ) );
            #else
            return HVecX( m_V[VX] + m_V[VY] + m_V[VZ] );
            #endif
        }
        
        // Dot product
        const HVecX operator | ( const HVec3& src ) const
        {
            //#ifdef _SSE_
            //const V128 dotXYZ = _mm_mul_ps( m_V, src.m_V );
            //const V128 sSum = _mm_add_ss( _mm_add_ss( dotXYZ, ReplicateY( dotXYZ ) ), ReplicateZ( dotXYZ ) );
            //return HVecX( _mm_and_ps( c_MaskX, sSum ) );
            //#else
            //return HVecX( m_V[VX] * src.m_V[VX] + m_V[VY] * src.m_V[VY] + m_V[VZ] * src.m_V[VZ] );
            //#endif
            return ( *this * src ).Sum();
        }
        
        // Cross Product
        const HVec3 operator ^ ( const HVec3& r ) const
        {
            const HVec3& l = *this;
            #ifdef _SSE_
            const V128 l_yzx = Shuffle<VY,VZ,VX,VW>( l.m_V );
            const V128 l_zxy = Shuffle<VZ,VX,VY,VW>( l.m_V );
            const V128 r_yzx = Shuffle<VY,VZ,VX,VW>( r.m_V );
            const V128 r_zxy = Shuffle<VZ,VX,VY,VW>( r.m_V );
            return HVec3( _mm_sub_ps( _mm_mul_ps( l_yzx, r_zxy ), _mm_mul_ps( l_zxy, r_yzx ) ) );
            #else
            return HVec3( l.m_V[VY] * r.m_V[VZ] - l.m_V[VZ] * r.m_V[VY],
                          l.m_V[VZ] * r.m_V[VX] - l.m_V[VX] * r.m_V[VZ],
                          l.m_V[VX] * r.m_V[VY] - l.m_V[VY] * r.m_V[VX] );
            #endif
        }
        
        // Combine with HVecW to form a HVec4
        const HVec4 operator + ( const HVecW& src ) const;
        
        // Vector * Matrix Multiplication
        const HVec3 operator * ( const HMat33& m ) const;
        const HVec3 operator * ( const HMat43& m ) const;
        const HVec3 operator * ( const HMat44& m ) const;

		const HVec3 TransformNormal(const HMat43& m) const;
        
        // Vector * Quaternion Multiplication
        const HVec3 operator * ( const Quaternion& q ) const;
        
        // Vector Length
        const HVecX Length() const
        {
            return ( *this * *this ).Sum().Sqrt();
        }

		// Vector length squared
		FORCE_INLINE const F32 LengthSquared() const
		{
			return ( *this * *this ).Sum();
		}
        
        // Vertex Distance
        const HVecX Distance( const HVec3& v ) const
        {
            return ( *this - v ).Length();
        }
        
        // Lerp
        const HVec3 Lerp( const HVec3& dst, const HVecX& f ) const
        {
            const HVec3& src = *this;
            return src + ( ( dst - src ) * f );
        }

		void Normalize()
		{
			*this = this->Normalised();
		}

		// returns the weighted luminance of an RGB vector
		float Luma() const
		{
			return (0.2125f * x) + (0.7154f * y) + (0.0721f * z);
		}

        // Vector Normalise
        const HVec3 Normalised() const
        {
            #ifdef _SSE_
            const HVec3& v = *this;
            const V128 recipLength = ReplicateX( RecipSqrtSS( ( v * v ).Sum() ) );
            return HVec3( _mm_mul_ps( m_V, recipLength ) );
            #else
            const F32 recipLength = 1.0f / Length();
#ifndef GCC
            if ( _finite( recipLength ) )
#else
			if (1)
#endif
            {
                return HVec3( m_V[VX] * recipLength, m_V[VY] * recipLength, m_V[VZ] * recipLength );
            }
            else
            {
                return HVec3( c_Zero );
            }
            #endif
        }

        // Parallel & perpendicular
        void ParaPerp( const HVec3& vector, HVec3& para, HVec3& perp )
        {
            const HVec3 normal = vector.Normalised();
            const F32 dp = *this | normal;
            para = normal * dp;
            perp = *this - para;
        }
#ifndef GCC
        const S32 Finite() const
        {
            return _finite( X() ) &&
                   _finite( Y() ) &&
                   _finite( Z() );
        }
#endif
        
		#ifdef VEC_MAT_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            #ifdef _SSE_
            if ( W() != 0.0f )
            {
                sprintf( buffer, "%s HVec3( %+8.3f, %+8.3f, %+8.3f ) [ %+18.15f ]\n", text, X(), Y(), Z(), W() );
            }
            else
            {
                sprintf( buffer, "%s HVec3( %+8.3f, %+8.3f, %+8.3f )\n", text, X(), Y(), Z() );
            }
            #else
            sprintf( buffer, "%s HVec3( %+8.3f, %+8.3f, %+8.3f )\n", text, X(), Y(), Z() );
            #endif
            OutputDebugString( buffer );
        }
		#endif
        
        void DebugValidate() const;
        
        #ifndef _SSE_
        const F32& operator [] ( const U32 index ) const
        {
            return m_V[index];
        }
        #endif
        
        F32 &operator [] ( const U32 index )
        {
            return m_V[index];
        }

        #ifdef _SSE_
        V128    m_V;
        #else
		union
		{
			struct
			{
				float	x,y,z;
			};
			F32     m_V[3];
		};
        #endif
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
    friend class Plane;
    friend class Quaternion;
    
    public:
        
        HVec4()                             { }
        
        #ifdef _SSE_
        
        HVec4( const V128& src )                        { m_V = src; }
        
        operator const V128 () const                    { return m_V; }
        
        const HVec4& operator = ( const V128& src )     { m_V = src; return *this; }
        
        HVec4( const HVec4& src )                       { m_V = src.m_V; }
        
        const HVec4& operator = ( const HVec4& src )    { m_V = src.m_V; return *this; }
        
        #endif
        
        HVec4( const HVecX& src )
        {
            #ifdef _SSE_
            m_V = src.m_X;
            #else
            m_V[VX] = src.m_X;
            m_V[VY] = 0.0f;
            m_V[VZ] = 0.0f;
            m_V[VW] = 0.0f;
            #endif
        }
        
        HVec4( const HVecY& src )
        {
            #ifdef _SSE_
            m_V = src.m_Y;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = src.m_Y;
            m_V[VZ] = 0.0f;
            m_V[VW] = 0.0f;
            #endif
        }
        
        HVec4( const HVecZ& src )
        {
            #ifdef _SSE_
            m_V = src.m_Z;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            m_V[VZ] = src.m_Z;
            m_V[VW] = 0.0f;
            #endif
        }
        
        HVec4( const HVecW& src )
        {
            #ifdef _SSE_
            m_V = src.m_W;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            m_V[VZ] = 0.0f;
            m_V[VW] = src.m_W;
            #endif
        }
        
        HVec4( const HVec2& src )
        {
            #ifdef _SSE_
            m_V = src.m_V;
            #else
            m_V[VX] = src.m_V[VX];
            m_V[VY] = src.m_V[VY];
            m_V[VZ] = 0.0f;
            m_V[VW] = 0.0f;
            #endif
        }
        
        HVec4( const HVec3& src )
        {
            #ifdef _SSE_
            m_V = src.m_V;
            #else
            m_V[VX] = src.m_V[VX];
            m_V[VY] = src.m_V[VY];
            m_V[VZ] = src.m_V[VZ];
            m_V[VW] = 0.0f;
            #endif
        }

		// Removed Havok
        ///* explicit */ HVec4( const hkVector4& src )
        //{
        //    #ifdef _SSE_
        //    m_V = _mm_set_ps( src.getSimdAt(3), src.getSimdAt(2), src.getSimdAt(1), src.getSimdAt(0) );
        //    #else
        //    m_V[VX] = src.getSimdAt(0);
        //    m_V[VY] = src.getSimdAt(1);
        //    m_V[VZ] = src.getSimdAt(2);
        //    m_V[VW] = src.getSimdAt(3);
        //    #endif
        //}

		// Removed Havok
        //operator const hkVector4() const
        //{
        //    return hkVector4( X(), Y(), Z(), W() );
        //}
        
        const F32 X() const
        {
            #ifdef _SSE_
            F32 x;
            _mm_store_ss( &x, m_V );
            return x;
            #else
            return m_V[VX];
            #endif
        }
        
        const F32 Y() const
        {
            #ifdef _SSE_
            F32 y;
            _mm_store_ss( &y, ReplicateY( m_V ) );
            return y;
            #else
            return m_V[VY];
            #endif
        }
        
        const F32 Z() const
        {
            #ifdef _SSE_
            F32 z;
            _mm_store_ss( &z, ReplicateZ( m_V ) );
            return z;
            #else
            return m_V[VZ];
            #endif
        }
        
        const F32 W() const
        {
            #ifdef _SSE_
            F32 w;
            _mm_store_ss( &w, ReplicateW( m_V ) );
            return w;
            #else
            return m_V[VW];
            #endif
        }
        
        void X( const F32 x )
        {
            #ifdef _SSE_
            const V128 vx = _mm_load_ss( &x );
            m_V = Shuffle<0,2,2,3>( Shuffle<0,0,1,1>( vx, m_V ), m_V );
            #else
            m_V[VX] = x;
            #endif
        }
        
        void Y( const F32 y )
        {
            #ifdef _SSE_
            const V128 vy = _mm_load_ss( &y );
            m_V = Shuffle<0,2,2,3>( Shuffle<0,0,0,0>( m_V, vy ), m_V );
            #else
            m_V[VY] = y;
            #endif
        }
        
        void Z( const F32 z )
        {
            #ifdef _SSE_
            const V128 vz = _mm_load_ss( &z );
            m_V = Shuffle<0,1,0,2>( m_V, Shuffle<0,0,3,3>( vz, m_V ) );
            #else
            m_V[VZ] = z;
            #endif
        }
        
        void W( const F32 w )
        {
            #ifdef _SSE_
            const V128 vw = _mm_load_ss( &w );
            m_V = Shuffle<0,1,0,2>( m_V, Shuffle<2,2,0,0>( m_V, vw ) );
            #else
            m_V[VW] = w;
            #endif
        }
        
        HVec4( const F32* array )
        {
            #ifdef _SSE_
            m_V = _mm_loadu_ps( array );
            #else
            m_V[VX] = array[VX];
            m_V[VY] = array[VY];
            m_V[VZ] = array[VZ];
            m_V[VW] = array[VW];
            #endif
        }
        
        HVec4( const F32 x, const F32 y, const F32 z, const F32 w )
        {
            #ifdef _SSE_
            m_V = _mm_set_ps( w, z, y, x );
            #else
            m_V[VX] = x;
            m_V[VY] = y;
            m_V[VZ] = z;
            m_V[VW] = w;
            #endif
        }
        
        explicit HVec4( const F32 xyzw )
        {
            #ifdef _SSE_
            m_V = ReplicateX( _mm_load_ss( &xyzw ) );
            #else
            m_V[VX] = xyzw;
            m_V[VY] = xyzw;
            m_V[VZ] = xyzw;
            m_V[VW] = xyzw;
            #endif
        }
        
        operator const HVecX () const
        {
            #ifdef _SSE_
            return HVecX( _mm_and_ps( c_MaskX, m_V ) );
            #else
            return HVecX( m_V[VX] );
            #endif
        }
        
        operator const HVecY () const
        {
            #ifdef _SSE_
            return HVecY( _mm_and_ps( c_MaskY, m_V ) );
            #else
            return HVecY( m_V[VY] );
            #endif
        }
        
        operator const HVecZ () const
        {
            #ifdef _SSE_
            return HVecZ( _mm_and_ps( c_MaskZ, m_V ) );
            #else
            return HVecZ( m_V[VZ] );
            #endif
        }
        
        operator const HVecW () const
        {
            #ifdef _SSE_
            return HVecW( _mm_and_ps( c_MaskW, m_V ) );
            #else
            return HVecW( m_V[VW] );
            #endif
        }
        
        const HVec4& operator = ( const HVecX& src )
        {
            #ifdef _SSE_
            m_V = src.m_X;
            #else
            m_V[VX] = src.m_X;
            m_V[VY] = 0.0f;
            m_V[VZ] = 0.0f;
            m_V[VW] = 0.0f;
            #endif
            return *this;
        }
        
        const HVec4& operator = ( const HVecY& src )
        {
            #ifdef _SSE_
            m_V = src.m_Y;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = src.m_Y;
            m_V[VZ] = 0.0f;
            m_V[VW] = 0.0f;
            #endif
            return *this;
        }
        
        const HVec4& operator = ( const HVecZ& src )
        {
            #ifdef _SSE_
            m_V = src.m_Z;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            m_V[VZ] = src.m_Z;
            m_V[VW] = 0.0f;
            #endif
            return *this;
        }
        
        const HVec4& operator = ( const HVecW& src )
        {
            #ifdef _SSE_
            m_V = src.m_W;
            #else
            m_V[VX] = 0.0f;
            m_V[VY] = 0.0f;
            m_V[VZ] = 0.0f;
            m_V[VW] = src.m_W;
            #endif
            return *this;
        }
        
        const HVec4& operator = ( const HVec2& src )
        {
            #ifdef _SSE_
            m_V = src.m_V;
            #else
            m_V[VX] = src.m_V[VX];
            m_V[VY] = src.m_V[VY];
            m_V[VZ] = 0.0f;
            m_V[VW] = 0.0f;
            #endif
            return *this;
        }
        
        const HVec4& operator = ( const HVec3& src )
        {
            #ifdef _SSE_
            m_V = src.m_V;
            #else
            m_V[VX] = src.m_V[VX];
            m_V[VY] = src.m_V[VY];
            m_V[VZ] = src.m_V[VZ];
            m_V[VW] = 0.0f;
            #endif
            return *this;
        }
        
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
        
        const HVec4 operator + ( const HVec4& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_add_ps( m_V, src.m_V ) );
            #else
            return HVec4( m_V[VX] + src.m_V[VX],
                          m_V[VY] + src.m_V[VY],
                          m_V[VZ] + src.m_V[VZ],
                          m_V[VW] + src.m_V[VW] );
            #endif
        }
        
        const HVec4 operator - ( const HVec4& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_sub_ps( m_V, src.m_V ) );
            #else
            return HVec4( m_V[VX] - src.m_V[VX],
                          m_V[VY] - src.m_V[VY],
                          m_V[VZ] - src.m_V[VZ],
                          m_V[VW] - src.m_V[VW] );
            #endif
        }
        
        const HVec4 operator * ( const HVec4& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_mul_ps( m_V, src.m_V ) );
            #else
            return HVec4( m_V[VX] * src.m_V[VX],
                          m_V[VY] * src.m_V[VY],
                          m_V[VZ] * src.m_V[VZ],
                          m_V[VW] * src.m_V[VW] );
            #endif
        }
        
        const HVec4 operator * ( const F32 src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_mul_ps( m_V, HVec4( src ).m_V ) );
            #else
            return HVec4( m_V[VX] * src,
                          m_V[VY] * src,
                          m_V[VZ] * src,
                          m_V[VW] * src );
            #endif
        }
        
        const HVec4 operator / ( const HVec4& src ) const
        {
            #ifdef _SSE_
            return HVec4( _mm_mul_ps( m_V, RecipPS( src.m_V ) ) );
            #else
            return HVec4( m_V[VX] / src.m_V[VX],
                          m_V[VY] / src.m_V[VY],
                          m_V[VZ] / src.m_V[VZ],
                          m_V[VW] / src.m_V[VW] );
            #endif
        }
        
        // Dot product
        const F32 operator | ( const HVec4& src ) const
        {
            #ifdef _SSE_
            
            const V128 dotXYZW = _mm_mul_ps( m_V, src.m_V );
            
            const V128 dotX = dotXYZW;
            const V128 dotY = ReplicateY( dotXYZW );
            const V128 dotZ = ReplicateZ( dotXYZW );
            const V128 dotW = ReplicateW( dotXYZW );
            
            const V128 sSum = _mm_add_ss( _mm_add_ss( dotX, dotY ), _mm_add_ss( dotZ, dotW ) );
            
            F32 dot;
            _mm_store_ss( &dot, sSum );
            return dot;
            
            #else
            
            return m_V[VX] * src.m_V[VX] +
                   m_V[VY] * src.m_V[VY] +
                   m_V[VZ] * src.m_V[VZ] +
                   m_V[VW] * src.m_V[VW];
            
            #endif
        }
        
        // Vector * Matrix Multiplication
        const HVec4 operator * ( const HMat44& m ) const;
        
        // Horizontal sum of all elements
        const F32 Sum() const
        {
            #ifdef _SSE_
            
            const V128 sSum = _mm_add_ss( _mm_add_ss( m_V,
                                                      ReplicateY( m_V ) ),
                                          _mm_add_ss( ReplicateZ( m_V ),
                                                      ReplicateW( m_V ) ) );
            
            F32 sum;
            _mm_store_ss( &sum, sSum );
            return sum;
            
            #else
            
            return m_V[VX] +
                   m_V[VY] +
                   m_V[VZ] +
                   m_V[VW];
            
            #endif
        }
        
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
        
        const U32 SignMask() const
        {
            #ifdef _SSE_
            return _mm_movemask_ps( m_V );
            #else
            return ( ( (U32&)m_V[VX] >> 31 ) & 0x1 ) |
                   ( ( (U32&)m_V[VY] >> 30 ) & 0x2 ) |
                   ( ( (U32&)m_V[VZ] >> 29 ) & 0x4 );
            #endif
        }
        
        // Vector Length
        FORCE_INLINE const F32 Length() const
        {
            #ifdef _SSE_
            
            const V128 dotXYZW = _mm_mul_ps( m_V, m_V );
            
            const V128 dotX = dotXYZW;
            const V128 dotY = ReplicateY( dotXYZW );
            const V128 dotZ = ReplicateZ( dotXYZW );
            const V128 dotW = ReplicateW( dotXYZW );
            
            const V128 sSum = _mm_add_ss( _mm_add_ss( dotX, dotY ), _mm_add_ss( dotZ, dotW ) );
            
            const V128 sLength = _mm_sqrt_ss( sSum );
            
            F32 length;
            _mm_store_ss( &length, sLength );
            return length;
            
            #else
            
            return sqrt( m_V[VX] * m_V[VX] +
                         m_V[VY] * m_V[VY] +
                         m_V[VZ] * m_V[VZ] +
                         m_V[VW] * m_V[VW] );
            
            #endif
        }
        
        // Vector Normalise
        const HVec4 Normalised() const
        {
            #ifdef _SSE_
            
            const V128 dotXYZW = _mm_mul_ps( m_V, m_V );
            
            const V128 dotX = dotXYZW;
            const V128 dotY = ReplicateY( dotXYZW );
            const V128 dotZ = ReplicateZ( dotXYZW );
            const V128 dotW = ReplicateW( dotXYZW );
            
            const V128 sSum2 = _mm_add_ss( _mm_add_ss( dotX, dotY ), _mm_add_ss( dotZ, dotW ) );
            
            const V128 rsqrtSum2 = ReplicateX( RecipSqrtSS( sSum2 ) );
            
            return HVec4( _mm_mul_ps( m_V, rsqrtSum2 ) );
            
            #else
            
            const F32 recipLength = 1.0f / Length();
            
            return HVec4( m_V[VX] * recipLength,
                          m_V[VY] * recipLength,
                          m_V[VZ] * recipLength,
                          m_V[VW] * recipLength );
            
            #endif
        }
        
		inline void Normalize()
		{
			*this = this->Normalised();
		}
#ifndef GCC
        const S32 Finite() const
        {
            return _finite( X() ) &&
                   _finite( Y() ) &&
                   _finite( Z() ) &&
                   _finite( W() );
        }
#endif
		#ifdef VEC_MAT_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            sprintf( buffer, "%s HVec4( %+8.3f, %+8.3f, %+8.3f, %+8.3f )\n", text, X(), Y(), Z(), W() );
            OutputDebugString( buffer );
        }
		#endif
       
        #ifndef _SSE_
        const F32 operator [] ( const U32 index ) const
        {
            return m_V[index];
        }
        #endif

        F32 &operator [] ( const U32 index )
        {
            return m_V[index];
        }
        
        #ifdef _SSE_
        V128    m_V;
        #else
		union
		{
			struct
			{
				float x,y,z,w;
			};
			F32     m_V[4];
		};
        #endif
    };
    
    //
    // Rotation classes for matrix construction
    //
    
    class RotX
    {
    public:
        RotX( const F32 a ) : m_AngleR( a ) { }
        operator const F32 () const { return m_AngleR; }
    private:
        const F32 m_AngleR; // Rotation angle about X axis in radians
    };
    
    class RotY
    {
    public:
        RotY( const F32 a ) : m_AngleR( a ) { }
        operator const F32 () const { return m_AngleR; }
    private:
        const F32 m_AngleR; // Rotation angle about Y axis in radians
    };
    
    class RotZ
    {
    public:
        RotZ( const F32 a ) : m_AngleR( a ) { }
        operator const F32 () const { return m_AngleR; }
    private:
        const F32 m_AngleR; // Rotation angle about Z axis in radians
    };
    
    //
    // Axis classes for matrix construction
    //
    
    class AxisX : public HVec3
    {
    public:
        AxisX( const HVec3& ax )
        {
            #ifdef _SSE_
            m_V = ax;
            #else
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
        {
            #ifdef _SSE_
            m_V = ay;
            #else
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
        {
            #ifdef _SSE_
            m_V = az;
            #else
            m_V[VX] = az.m_V[VX];
            m_V[VY] = az.m_V[VY];
            m_V[VZ] = az.m_V[VZ];
            #endif
        }
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
    
    public:
        
        // Matrices are initialised to Identity by default constructor!
        inline HMat33()
        {
            m_A[MX] = HVec3( c_UnitX );
            m_A[MY] = HVec3( c_UnitY );
            m_A[MZ] = HVec3( c_UnitZ );
        }
        
        HVec3 AX() const                    { return m_A[MX]; }
        HVec3 AY() const                    { return m_A[MY]; }
        HVec3 AZ() const                    { return m_A[MZ]; }
        
        void AX( const HVec3& ax )          { m_A[MX] = ax; }
        void AY( const HVec3& ay )          { m_A[MY] = ay; }
        void AZ( const HVec3& az )          { m_A[MZ] = az; }
        
        FORCE_INLINE const HVec3& operator [] ( const MatAxis i ) const
        {
            return m_A[i];
        }

		HVec3& operator [] ( const MatAxis i )
        {
            return m_A[i];
        }

        const HVec3& operator [] ( const U32 i ) const
        {
            return m_A[i];
        }

		HVec3& operator [] ( const U32 i )
        {
            return m_A[i];
        }
        
        HMat33( const HMat33& src )
        {
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
        }
        
        // Explicit construction due to data loss!
        explicit HMat33( const HMat43& src );
        explicit HMat33( const HMat44& src );
        
        const HMat33& operator = ( const HMat33& src )
        {
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
            return *this;
        }
        
        #ifdef _SSE_
        HMat33( const V128& ax, const V128& ay, const V128& az )
        {
            m_A[MX].m_V = ax;
            m_A[MY].m_V = ay;
            m_A[MZ].m_V = az;
        }
        #endif
        
        HMat33( const HVec3& ax, const HVec3& ay, const HVec3& az )
        {
            m_A[MX] = ax;
            m_A[MY] = ay;
            m_A[MZ] = az;
        }
        
        // Rotation about X axis - clockwise when viewed back down the X axis
        HMat33( const RotX& alpha )
        {
            const F32 cx = cos( alpha );
            const F32 sx = sin( alpha );
            ConstV3F32( ay ) = { 0.0f,  cx, sx };
            ConstV3F32( az ) = { 0.0f, -sx, cx };
            m_A[MX] = HVec3( c_UnitX );
            m_A[MY] = HVec3( ay );
            m_A[MZ] = HVec3( az );
        }

   
        // Rotation about Y axis - clockwise when viewed back down the Y axis
        HMat33( const RotY& beta )
        {
            const F32 cy = cos( beta );
            const F32 sy = sin( beta );
            ConstV3F32( ax ) = { cy, 0.0f, -sy };
            ConstV3F32( az ) = { sy, 0.0f,  cy };
            m_A[MX] = HVec3( ax );
            m_A[MY] = HVec3( c_UnitY );
            m_A[MZ] = HVec3( az );
        }
        
        // Rotation about Z axis - clockwise when viewed back down the Z axis
        HMat33( const RotZ& gamma )
        {
            const F32 cz = cos( gamma );
            const F32 sz = sin( gamma );
            ConstV3F32( ax ) = {  cz, sz, 0.0f };
            ConstV3F32( ay ) = { -sz, cz, 0.0f };
            m_A[MX] = HVec3( ax );
            m_A[MY] = HVec3( ay );
            m_A[MZ] = HVec3( c_UnitZ );
        }

		//Initialise matrix from Euler angles
		HMat33(const RotX& alpha, const RotY& beta, const RotZ& gamma, EulerOrder euler)
		{
			HMat33 rotX(alpha);
			HMat33 rotY(beta);
			HMat33 rotZ(gamma);

			switch(euler)
			{
				case EULER_XYZ: *this = rotZ * rotY * rotX; break;
				case EULER_XZY: *this = rotY * rotZ * rotX; break;
				case EULER_YXZ: *this = rotZ * rotX * rotY; break;
				case EULER_YZX: *this = rotX * rotZ * rotY; break;
				case EULER_ZXY: *this = rotY * rotX * rotZ; break;
				case EULER_ZYX: *this = rotX * rotY * rotZ; break;
			}
		}

		HMat33(F32 alpha, F32 beta, F32 gamma)
		{
			F32 sx,sy, cx,cy,sz,cz;
			sx = (F32) sinf (alpha);
			cx = (F32) cosf (alpha);

			sy = (F32) sinf (beta); 
			cy = (F32) cosf (beta);

			sz = (F32) sinf (gamma);
			cz = (F32) cosf (gamma);

			m_A[MX][VX] = cy *cz;
			m_A[MX][VY] = sx * sy * cz - cx * sz;
			m_A[MX][VZ] = cx * sy * cz - sx * sz;

			m_A[MY][VX] = cy * sz;
			m_A[MY][VY] = sx * sy * sz + cx * cz;
			m_A[MY][VZ] = cx * sy * sz - sx * cz;

			m_A[MZ][VX] = -sy;
			m_A[MZ][VY] = sx * cy;
			m_A[MZ][VZ] = cx * cy;
		}
        
        // Cross products for deriving third matrix axis from 2 input axes
        // const HVec3 ax = ay ^ az;
        // const HVec3 ay = az ^ ax;
        // const HVec3 az = ax ^ ay;
        
        // Initialise with primary axis Z
        HMat33( const AxisZ& az )
        {
            const F32 cos45deg = sqrt( 2.0f ) / 2.0f;
            const HVec3 ay( c_UnitY );
            
            if ( abs( ay | az ) < cos45deg )
            {
                const HVec3 ax = ay ^ az;
                m_A[MX] = ax.Normalised();
                m_A[MY] = ( az ^ ax ).Normalised();
                m_A[MZ] = az.Normalised();
            }
            else
            {
                const HVec3 ax( c_UnitX );
                const HVec3 ay = az ^ ax;
                m_A[MX] = ( ay ^ az ).Normalised();
                m_A[MY] = ay.Normalised();
                m_A[MZ] = az.Normalised();
            }
        }
        
        // Initialise with primary axis X, secondary axis Y
        HMat33( const AxisX& ax, const AxisY& ay )
        {
            const HVec3 az = ax ^ ay;
            m_A[MX] = ax.Normalised();
            m_A[MY] = ( az ^ ax ).Normalised();
            m_A[MZ] = az.Normalised();
        }
        
        // Initialise with primary axis X, secondary axis Z
        HMat33( const AxisX& ax, const AxisZ& az )
        {
            const HVec3 ay = az ^ ax;
            m_A[MX] = ax.Normalised();
            m_A[MY] = ay.Normalised();
            m_A[MZ] = ( ax ^ ay ).Normalised();
        }
        
        // Initialise with primary axis Y, secondary axis X
        HMat33( const AxisY& ay, const AxisX& ax )
        {
            const HVec3 az = ax ^ ay;
            m_A[MX] = ( ay ^ az ).Normalised();
            m_A[MY] = ay.Normalised();
            m_A[MZ] = az.Normalised();
        }
        
        // Initialise with primary axis Y, secondary axis Z
        HMat33( const AxisY& ay, const AxisZ& az )
        {
            const HVec3 ax = ay ^ az;
            m_A[MX] = ax.Normalised();
            m_A[MY] = ay.Normalised();
            m_A[MZ] = ( ax ^ ay ).Normalised();
        }
        
        // Initialise with primary axis Z, secondary axis X
        HMat33( const AxisZ& az, const AxisX& ax )
        {
            const HVec3 ay = az ^ ax;
            m_A[MX] = ( ay ^ az ).Normalised();
            m_A[MY] = ay.Normalised();
            m_A[MZ] = az.Normalised();
        }
        
        // Initialise with primary axis Z, secondary axis Y
        HMat33( const AxisZ& az, const AxisY& ay )
        {
            const HVec3 ax = ay ^ az;
            m_A[MX] = ax.Normalised();
            m_A[MY] = ( az ^ ax ).Normalised();
            m_A[MZ] = az.Normalised();
        }
        
        void SetIdentity()
        {
            m_A[MX] = HVec3( c_UnitX );
            m_A[MY] = HVec3( c_UnitY );
            m_A[MZ] = HVec3( c_UnitZ );
        }
        
        const HMat33 Transposed() const
        {
            #ifdef _SSE_
            
            const V128 zero = _mm_setzero_ps();
            
            const V128 tmp0 = _mm_shuffle_ps( m_A[MX].m_V, m_A[MY].m_V, 0x44 );
            const V128 tmp1 = _mm_shuffle_ps( m_A[MZ].m_V, zero,        0x44 );
            const V128 tmp2 = _mm_shuffle_ps( m_A[MX].m_V, m_A[MY].m_V, 0xEE );
            const V128 tmp3 = _mm_shuffle_ps( m_A[MZ].m_V, zero,        0xEE );
            
            const V128 ax = _mm_shuffle_ps( tmp0, tmp1, 0x88 );
            const V128 ay = _mm_shuffle_ps( tmp0, tmp1, 0xDD );
            const V128 az = _mm_shuffle_ps( tmp2, tmp3, 0x88 );
            
            return HMat33( ax, ay, az );
            
            #else
            
            const HMat33& m = *this;
            
            const HVec3 ax( m[MX][VX], m[MY][VX], m[MZ][VX] );
            const HVec3 ay( m[MX][VY], m[MY][VY], m[MZ][VY] );
            const HVec3 az( m[MX][VZ], m[MY][VZ], m[MZ][VZ] );
            
            return HMat33( ax, ay, az );
            
            #endif
        }
        
        // Matrix Multiply
        const HMat33 operator * ( const HMat33& m ) const
        {
            return HMat33( m_A[MX] * m,
                           m_A[MY] * m,
                           m_A[MZ] * m );
        }
        
        // Combine with HVec3 position to form a HMat43
        const HMat43 operator + ( const HVec3& pos ) const;
        
        // Inverse of Rotation.
        // Not valid for matrices with scaling/skewing etc!
        const HMat33 InverseR() const
        {
            return Transposed();
        }
        
        // Matrix Determinant
        const F32 Determinant() const
        {
            #ifdef _SSE_
            
            const V128 vDet = _mm_mul_ps( m_A[MX], _mm_sub_ps( _mm_mul_ps( Shuffle<VY,VX,VX,VW>( m_A[MY] ), Shuffle<VZ,VZ,VY,VW>( m_A[MZ] ) ),
                                                               _mm_mul_ps( Shuffle<VZ,VZ,VY,VW>( m_A[MY] ), Shuffle<VY,VX,VX,VW>( m_A[MZ] ) ) ) );
            
            const V128 sDet = _mm_add_ss( _mm_sub_ss( vDet, ReplicateY( vDet ) ), ReplicateZ( vDet ) );
            
            F32 det;
            _mm_store_ss( &det, sDet );
            return det;
            
            #else
            
            const HMat33& m = *this;
            
            return m[MX][VX] * ( m[MY][VY] * m[MZ][VZ] - m[MY][VZ] * m[MZ][VY] ) -
                   m[MX][VY] * ( m[MY][VX] * m[MZ][VZ] - m[MY][VZ] * m[MZ][VX] ) +
                   m[MX][VZ] * ( m[MY][VX] * m[MZ][VY] - m[MY][VY] * m[MZ][VX] );
            
            #endif
        }
        
        // Full Matrix Inverse - This will FAIL if determinant is zero!
        const HMat33 Inverse() const
        {
            const F32 det = Determinant();
            
            if ( det != 0.0f )
            {
                #ifdef _SSE_
                
                const HVec3 ax( _mm_sub_ps( _mm_mul_ps( Shuffle<VY,VZ,VY,VW>( m_A[MY], m_A[MX], m_A[MX] ),
                                                        Shuffle<VZ,VY,VZ,VW>( m_A[MZ], m_A[MZ], m_A[MY] ) ),
                                            _mm_mul_ps( Shuffle<VZ,VY,VZ,VW>( m_A[MY], m_A[MX], m_A[MX] ),
                                                        Shuffle<VY,VZ,VY,VW>( m_A[MZ], m_A[MZ], m_A[MY] ) ) ) );
                
                const HVec3 ay( _mm_sub_ps( _mm_mul_ps( Shuffle<VZ,VX,VZ,VW>( m_A[MY], m_A[MX], m_A[MX] ),
                                                        Shuffle<VX,VZ,VX,VW>( m_A[MZ], m_A[MZ], m_A[MY] ) ),
                                            _mm_mul_ps( Shuffle<VX,VZ,VX,VW>( m_A[MY], m_A[MX], m_A[MX] ),
                                                        Shuffle<VZ,VX,VZ,VW>( m_A[MZ], m_A[MZ], m_A[MY] ) ) ) );
                
                const HVec3 az( _mm_sub_ps( _mm_mul_ps( Shuffle<VX,VY,VX,VW>( m_A[MY], m_A[MX], m_A[MX] ),
                                                        Shuffle<VY,VX,VY,VW>( m_A[MZ], m_A[MZ], m_A[MY] ) ),
                                            _mm_mul_ps( Shuffle<VY,VX,VY,VW>( m_A[MY], m_A[MX], m_A[MX] ),
                                                        Shuffle<VX,VY,VX,VW>( m_A[MZ], m_A[MZ], m_A[MY] ) ) ) );
                
                #else
                
                const HMat33& m = *this;
                
                const HVec3 ax( m[MY][VY] * m[MZ][VZ] - m[MY][VZ] * m[MZ][VY],
                                m[MX][VZ] * m[MZ][VY] - m[MX][VY] * m[MZ][VZ],
                                m[MX][VY] * m[MY][VZ] - m[MX][VZ] * m[MY][VY] );
                
                const HVec3 ay( m[MY][VZ] * m[MZ][VX] - m[MY][VX] * m[MZ][VZ],
                                m[MX][VX] * m[MZ][VZ] - m[MX][VZ] * m[MZ][VX],
                                m[MX][VZ] * m[MY][VX] - m[MX][VX] * m[MY][VZ] );
                
                const HVec3 az( m[MY][VX] * m[MZ][VY] - m[MY][VY] * m[MZ][VX],
                                m[MX][VY] * m[MZ][VX] - m[MX][VX] * m[MZ][VY],
                                m[MX][VX] * m[MY][VY] - m[MX][VY] * m[MY][VX] );
                
                #endif
                
                const F32 reciprocalDet = 1.0f / det;
                
                return HMat33( ax * reciprocalDet,
                               ay * reciprocalDet,
                               az * reciprocalDet );
            }
            else
            {
				#ifdef VEC_MAT_PRINT
					// Failed to calculate an inverse!
					#ifdef _DEBUG
					Print( "HMat33::Inverse(): Failed to calculate inverse for matrix:" );
					#endif
				#endif
                return HMat33( c_Zero, c_Zero, c_Zero );
            }
        }
        
        // Convert to D3DMATRIX
		#ifdef __D3DX__H__
        operator const D3DMATRIX () const;
        operator const D3DXMATRIX () const;
		#endif

		// Removed Havok
        //operator const hkRotation() const
        //{
        //    hkRotation r;
        //    r.setCols( (hkVector4)m_A[MX], (hkVector4)m_A[MY], (hkVector4)m_A[MZ] );
        //    return r;
        //}
        
		#ifdef VEC_MAT_PRINT
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
            OutputDebugString( "\n" );
        }
		#endif
        
    private:
        
        HVec3 m_A[3];
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
        inline HMat43()
        {
            m_A[MX] = HVec3( c_UnitX );
            m_A[MY] = HVec3( c_UnitY );
            m_A[MZ] = HVec3( c_UnitZ );
            m_A[MW] = HVec3( c_Zero );
        }
        
        HVec3 AX() const                    { return m_A[MX]; }
        HVec3 AY() const                    { return m_A[MY]; }
        HVec3 AZ() const                    { return m_A[MZ]; }
        HVec3 AW() const                    { return m_A[MW]; }
        
        void AX( const HVec3& ax )          { m_A[MX] = ax; }
        void AY( const HVec3& ay )          { m_A[MY] = ay; }
        void AZ( const HVec3& az )          { m_A[MZ] = az; }
        void AW( const HVec3& aw )          { m_A[MW] = aw; }
        
        const HVec3& operator [] ( const MatAxis i ) const
        {
            return m_A[i];
        }
        
        HVec3& operator [] ( const MatAxis i )
        {
            return m_A[i];
        }

        const HVec3& operator [] ( const U32 i ) const 
        {
            return m_A[i];
        }

        HVec3& operator [] ( const U32 i )
        {
            return m_A[i];
        }

		explicit HMat43( const HMat33& src )
        {
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
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
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
            m_A[MW] = HVec3( c_Zero );
            return *this;
        }
        
        const HMat43& operator = ( const HMat43& src )
        {
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
            m_A[MW] = src.m_A[MW];
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
        
        // Initialise with primary axis Z, secondary axis Y
        HMat43( const AxisZ& az, const AxisY& ay, const HVec3& aw = HVec3( c_Zero ) )
        {
            const HVec3 ax = ay ^ az;
            m_A[MX] = ax.Normalised();
            m_A[MY] = ( az ^ ax ).Normalised();
            m_A[MZ] = az.Normalised();
            m_A[MW] = aw;
        }

		HMat43(F32 alpha, F32 beta, F32 gamma)
		{
			F32 sx,sy, cx,cy,sz,cz;
			sx = (F32) sinf (alpha);
			cx = (F32) cosf (alpha);

			sy = (F32) sinf (beta); 
			cy = (F32) cosf (beta);

			sz = (F32) sinf (gamma);
			cz = (F32) cosf (gamma);

			m_A[MX][VX] = cy *cz;
			m_A[MX][VY] = sx * sy * cz - cx * sz;
			m_A[MX][VZ] = cx * sy * cz - sx * sz;

			m_A[MY][VX] = cy * sz;
			m_A[MY][VY] = sx * sy * sz + cx * cz;
			m_A[MY][VZ] = cx * sy * sz - sx * cz;

			m_A[MZ][VX] = -sy;
			m_A[MZ][VY] = sx * cy;
			m_A[MZ][VZ] = cx * cy;

			m_A[MX][VW] = 0;
			m_A[MY][VW] = 0;
			m_A[MZ][VW] = 0;

			m_A[MW][VX] = 0;
			m_A[MW][VY] = 0;
			m_A[MW][VZ] = 0;
			m_A[MW][VW] = 1;
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
            
            return HMat43( ax, ay, az, c_Zero );
            
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
            
            return HMat43( invRot.m_A[MX],
                           invRot.m_A[MY],
                           invRot.m_A[MZ],
                           -m_A[MW] * invRot );
        }

		#ifdef __D3DX__H__
        // Convert to D3DMATRIX
        operator const D3DMATRIX () const;
        operator const D3DXMATRIX () const;
		#endif

		// Removed havok
        //operator const hkTransform() const
        //{
        //    hkRotation r;
        //    r.setCols( (hkVector4)m_A[MX], (hkVector4)m_A[MY], (hkVector4)m_A[MZ] );
        //    return hkTransform( r, (hkVector4)m_A[MW] );
        //}
        
		#ifdef VEC_MAT_PRINT
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
        
        HVec3 m_A[4];
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
        
        // Matrices are initialised to Identity by default constructor!
        HMat44()
        {
            m_A[MX] = c_UnitX;
            m_A[MY] = c_UnitY;
            m_A[MZ] = c_UnitZ;
            m_A[MW] = c_UnitW;
        }
        
        HVec4 AX() const                    { return m_A[MX]; }
        HVec4 AY() const                    { return m_A[MY]; }
        HVec4 AZ() const                    { return m_A[MZ]; }
        HVec4 AW() const                    { return m_A[MW]; }
        
        void AX( const HVec4& ax )          { m_A[MX] = ax; }
        void AY( const HVec4& ay )          { m_A[MY] = ay; }
        void AZ( const HVec4& az )          { m_A[MZ] = az; }
        void AW( const HVec4& aw )          { m_A[MW] = aw; }

		HMat44(F32 alpha, F32 beta, F32 gamma)
		{
/*
			F32 sx,sy, cx,cy,sz,cz;
			sx = (F32) sinf (alpha);
			cx = (F32) cosf (alpha);

			sy = (F32) sinf (beta); 
			cy = (F32) cosf (beta);

			sz = (F32) sinf (gamma);
			cz = (F32) cosf (gamma);

			m_A[MX][VX] = cy *cz;
			m_A[MX][VY] = sx * sy * cz - cx * sz;
			m_A[MX][VZ] = cx * sy * cz - sx * sz;

			m_A[MY][VX] = cy * sz;
			m_A[MY][VY] = sx * sy * sz + cx * cz;
			m_A[MY][VZ] = cx * sy * sz - sx * cz;

			m_A[MZ][VX] = -sy;
			m_A[MZ][VY] = sx * cy;
			m_A[MZ][VZ] = cx * cy;

			m_A[MX][VW] = 0;
			m_A[MY][VW] = 0;
			m_A[MZ][VW] = 0;

			m_A[MW][VX] = 0;
			m_A[MW][VY] = 0;
			m_A[MW][VZ] = 0;
			m_A[MW][VW] = 1;
*/

            F32 sp = sinf(alpha);
            F32 cp = cosf(alpha);
            F32 sy = sinf(beta);
            F32 cy = cosf(beta);
            F32 sr = sinf(gamma);
            F32 cr = cosf(gamma);
            F32 sysp = sy * sp; 
            F32 sycp = sy * cp; 

            m_A[0][0] = cr * cy;
            m_A[0][1] = sr * cy;
            m_A[0][2] = -sy;
            m_A[0][3] = 0;

            m_A[1][0] = cr * sysp - sr * cp;
			m_A[1][1] = sr * sysp + cr * cp;
            m_A[1][2] = cy * sp;
            m_A[1][3] = 0;

            m_A[2][0] = cr * sycp + sr * sp;
            m_A[2][1] = sr * sycp - cr * sp;
            m_A[2][2] = cy * cp;
            m_A[2][3] = 0;

            m_A[3][0] = 0;
            m_A[3][1] = 0;
            m_A[3][2] = 0;
            m_A[3][3] = 1;
		}
        
        const HVec4& operator [] ( const MatAxis i ) const
        {
            return m_A[i];
        }

        HVec4& operator [] ( int i )
        {
            return m_A[i];
        }
        
        explicit HMat44( const HMat33& src )
        {
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
            m_A[MW] = c_UnitW;
        }
        
        explicit HMat44( const HMat43& src )
        {
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
            m_A[MW] = src.m_A[MW] + HVecW( 1.0f );
        }
        
        HMat44( const HMat44& src )
        {
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
            m_A[MW] = src.m_A[MW];
        }
        
        const HMat44& operator = ( const HMat33& src )
        {
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
            m_A[MW] = c_UnitW;
            return *this;
        }
        
        const HMat44& operator = ( const HMat43& src )
        {
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
            m_A[MW] = src.m_A[MW] + HVecW( 1.0f );
            return *this;
        }
        
        const HMat44& operator = ( const HMat44& src )
        {
            m_A[MX] = src.m_A[MX];
            m_A[MY] = src.m_A[MY];
            m_A[MZ] = src.m_A[MZ];
            m_A[MW] = src.m_A[MW];
            return *this;
        }
        
        #ifdef _SSE_
        HMat44( const V128& ax, const V128& ay, const V128& az, const V128& aw )
        {
            m_A[MX] = ax;
            m_A[MY] = ay;
            m_A[MZ] = az;
            m_A[MW] = aw;
        }
        #endif
        
        HMat44( const HVec4& ax, const HVec4& ay, const HVec4& az, const HVec4& aw )
        {
            m_A[MX] = ax;
            m_A[MY] = ay;
            m_A[MZ] = az;
            m_A[MW] = aw;
        }
        
        // Rotation about X axis - clockwise when viewed back down the X axis
        HMat44( const RotX& rx, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const F32 alpha = rx;
            const F32 cx = cos( alpha );
            const F32 sx = sin( alpha );
            ConstV4F32( ay ) = { 0.0f,  cx, sx, 0.0f };
            ConstV4F32( az ) = { 0.0f, -sx, cx, 0.0f };
            m_A[MX] = c_UnitX;
            m_A[MY] = ay;
            m_A[MZ] = az;
            m_A[MW] = aw.m_V;
        }
        
        // Rotation about Y axis - clockwise when viewed back down the Y axis
        HMat44( const RotY& ry, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const F32 beta = ry;
            const F32 cy = cos( beta );
            const F32 sy = sin( beta );
            ConstV4F32( ax ) = { cy, 0.0f, -sy, 0.0f };
            ConstV4F32( az ) = { sy, 0.0f,  cy, 0.0f };
            m_A[MX] = ax;
            m_A[MY] = c_UnitY;
            m_A[MZ] = az;
            m_A[MW] = aw.m_V;
        }
        
        // Rotation about Z axis - clockwise when viewed back down the Z axis
        HMat44( const RotZ& rz, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const F32 gamma = rz;
            const F32 cz = cos( gamma );
            const F32 sz = sin( gamma );
            ConstV4F32( ax ) = {  cz, sz, 0.0f, 0.0f };
            ConstV4F32( ay ) = { -sz, cz, 0.0f, 0.0f };
            m_A[MX] = ax;
            m_A[MY] = ay;
            m_A[MZ] = c_UnitZ;
            m_A[MW] = aw.m_V;
        }
        
        // Cross products for deriving third matrix axis from 2 input axes
        // const HVec3 ax = ay ^ az;
        // const HVec3 ay = az ^ ax;
        // const HVec3 az = ax ^ ay;
        
        // Initialise with primary axis X, secondary axis Y
        HMat44( const AxisX& ax, const AxisY& ay, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const HVec3 az = ax ^ ay;
            m_A[MX] = ax.Normalised();
            m_A[MY] = ( az ^ ax ).Normalised();
            m_A[MZ] = az.Normalised();
            m_A[MW] = aw.m_V;
        }
        
        // Initialise with primary axis X, secondary axis Z
        HMat44( const AxisX& ax, const AxisZ& az, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const HVec3 ay = az ^ ax;
            m_A[MX] = ax.Normalised();
            m_A[MY] = ay.Normalised();
            m_A[MZ] = ( ax ^ ay ).Normalised();
            m_A[MW] = aw.m_V;
        }
        
        // Initialise with primary axis Y, secondary axis X
        HMat44( const AxisY& ay, const AxisX& ax, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const HVec3 az = ax ^ ay;
            m_A[MX] = ( ay ^ az ).Normalised();
            m_A[MY] = ay.Normalised();
            m_A[MZ] = az.Normalised();
            m_A[MW] = aw.m_V;
        }
        
        // Initialise with primary axis Y, secondary axis Z
        HMat44( const AxisY& ay, const AxisZ& az, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const HVec3 ax = ay ^ az;
            m_A[MX] = ax.Normalised();
            m_A[MY] = ay.Normalised();
            m_A[MZ] = ( ax ^ ay ).Normalised();
            m_A[MW] = aw.m_V;
        }
        
        // Initialise with primary axis Z, secondary axis X
        HMat44( const AxisZ& az, const AxisX& ax, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const HVec3 ay = az ^ ax;
            m_A[MX] = ( ay ^ az ).Normalised();
            m_A[MY] = ay.Normalised();
            m_A[MZ] = az.Normalised();
            m_A[MW] = aw.m_V;
        }
        
        // Initialise with primary axis Z, secondary axis Y
        HMat44( const AxisZ& az, const AxisY& ay, const HVec4& aw = HVec4( c_UnitW ) )
        {
            const HVec3 ax = ay ^ az;
            m_A[MX] = ax.Normalised();
            m_A[MY] = ( az ^ ax ).Normalised();
            m_A[MZ] = az.Normalised();
            m_A[MW] = aw.m_V;
        }

		//Initialise matrix from Euler angles
		HMat44(const RotX& alpha, const RotY& beta, const RotZ& gamma, EulerOrder euler)
		{
			HMat44 rotX(alpha);
			HMat44 rotY(beta);
			HMat44 rotZ(gamma);

			switch(euler)
			{
				case EULER_XYZ: *this = rotZ * rotY * rotX; break;
				case EULER_XZY: *this = rotY * rotZ * rotX; break;
				case EULER_YXZ: *this = rotZ * rotX * rotY; break;
				case EULER_YZX: *this = rotX * rotZ * rotY; break;
				case EULER_ZXY: *this = rotY * rotX * rotZ; break;
				case EULER_ZYX: *this = rotX * rotY * rotZ; break;
			}
		}
        
        void SetIdentity()
        {
            m_A[MX] = c_UnitX;
            m_A[MY] = c_UnitY;
            m_A[MZ] = c_UnitZ;
            m_A[MW] = c_UnitW;
        }
        
        const HMat44 Transposed() const
        {
            #ifdef _SSE_
            
            const V128 tmp0 = _mm_shuffle_ps( m_A[MX].m_V, m_A[MY].m_V, 0x44 );
            const V128 tmp1 = _mm_shuffle_ps( m_A[MZ].m_V, m_A[MW].m_V, 0x44 );
            const V128 tmp2 = _mm_shuffle_ps( m_A[MX].m_V, m_A[MY].m_V, 0xEE );
            const V128 tmp3 = _mm_shuffle_ps( m_A[MZ].m_V, m_A[MW].m_V, 0xEE );
            
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

		void GetEulerAngles(F32& alpha, F32& beta, F32& gamma)
		{
			//Code taken from fvector.cpp (editor) fvMatrixToEulerAngles
			float cy2;
			beta = (float) asin( - m_A[MZ][VX]);
			cy2 = 1.f - m_A[MZ][VX]* m_A[MZ][VX];

			if (cy2>0.0001f) {
				alpha = (float) atan2( m_A[MZ][VY], m_A[MZ][VZ]);
				gamma = (float) atan2( m_A[MY][VX], m_A[MX][VX]);
			} else {
				alpha = (float) atan2( m_A[MX][VY], m_A[MX][VZ] );
				gamma = 0.f;
			}

			alpha *= -1;
			beta *= -1;
			gamma *= -1;
		}

		const HMat44& GetSRT(HVec3&	scaling, HVec3& rotate, HVec3& trans) const 
		{
			HMat44 tmp;

			tmp =* this;

			scaling.X(tmp.AX().Length());
			scaling.Y(tmp.AY().Length());
			scaling.Z(tmp.AZ().Length());

			HMat44 tmp_mat;

			tmp_mat.m_A[MX][VX] = (tmp.m_A[MX][VX]);
			tmp_mat.m_A[MY][VX] = (tmp.m_A[MX][VY]);
			tmp_mat.m_A[MZ][VX] = (tmp.m_A[MX][VZ]);

			tmp_mat.m_A[MX][VY] = (tmp.m_A[MY][VX]);
			tmp_mat.m_A[MY][VY] = (tmp.m_A[MY][VY]);
			tmp_mat.m_A[MZ][VY] = (tmp.m_A[MY][VZ]);

			tmp_mat.m_A[MX][VZ] = (tmp.m_A[MZ][VX]);
			tmp_mat.m_A[MY][VZ] = (tmp.m_A[MZ][VY]);
			tmp_mat.m_A[MZ][VZ] = (tmp.m_A[MZ][VZ]);

			F32 alpha;
			F32 beta;
			F32 gamma;
			tmp_mat.GetEulerAngles(alpha, beta, gamma);
			rotate.X( -alpha );
			rotate.Y( -beta );
			rotate.Z( -gamma );
			
			//HMat44 pureRotateMat(RotX(rotate.X()), RotY(rotate.Y()), RotZ(rotate.Z()), EULER_XYZ);
			HMat44 pureRotateMat(alpha, beta, gamma);

			scaling.X( pureRotateMat.AX() | AX() );
			scaling.Y( pureRotateMat.AY() | AY() );
			scaling.Z( pureRotateMat.AZ() | AZ() );

			trans.X( AW().X() );
			trans.Y( AW().Y() );
			trans.Z( AW().Z() );

			return*	this;
		}
        
        // Matrix Multiply
        const HMat44 operator * ( const HMat44& m ) const
        {
            return HMat44( m_A[MX] * m,
                           m_A[MY] * m,
                           m_A[MZ] * m,
                           m_A[MW] * m );
        }
        
        // Inverse of Rotation & Translation
        // Not valid for matrices with scaling/skewing etc!
        const HMat44 InverseRT() const
        {
            const HMat33 invRot = HMat33( *this ).Transposed();
            
            return HMat44( invRot.m_A[MX],
                           invRot.m_A[MY],
                           invRot.m_A[MZ],
                           HVec4( c_UnitW ) - HVec3( m_A[MW] ) * invRot );
        }

		// calculate inverse of a matrix
		const HMat44 Inverse() const
		{
		#ifdef __D3DX9_H__
			D3DXMATRIX out;
			D3DXMatrixInverse(&out,NULL,(D3DXMATRIX*)this);
			return *(HMat44*)&out;
		#else
/*			bool affine = false;
			if ( affine )
			{
				F32 s = 1 / u.GetDeterminant();
				m44[0][0] = (u.m44[1][1]*u.m44[2][2] - u.m44[1][2]*u.m44[2][1]) * s;
				m44[0][1] = (u.m44[2][1]*u.m44[0][2] - u.m44[2][2]*u.m44[0][1]) * s;
				m44[0][2] = (u.m44[0][1]*u.m44[1][2] - u.m44[0][2]*u.m44[1][1]) * s;
				m44[0][3] =  u.m44[0][3];
				m44[1][0] = (u.m44[1][2]*u.m44[2][0] - u.m44[1][0]*u.m44[2][2]) * s;
				m44[1][1] = (u.m44[2][2]*u.m44[0][0] - u.m44[2][0]*u.m44[0][2]) * s;
				m44[1][2] = (u.m44[0][2]*u.m44[1][0] - u.m44[0][0]*u.m44[1][2]) * s;
				m44[1][3] =  u.m44[1][3];
				m44[2][0] = (u.m44[1][0]*u.m44[2][1] - u.m44[1][1]*u.m44[2][0]) * s;
				m44[2][1] = (u.m44[2][0]*u.m44[0][1] - u.m44[2][1]*u.m44[0][0]) * s;
				m44[2][2] = (u.m44[0][0]*u.m44[1][1] - u.m44[0][1]*u.m44[1][0]) * s;
				m44[2][3] =  u.m44[2][3];
				m44[3][0] =  -(m44[0][0]*u.m44[3][0] + m44[1][0]*u.m44[3][1] + m44[2][0]*u.m44[3][2]);
				m44[3][1] =  -(m44[0][1]*u.m44[3][0] + m44[1][1]*u.m44[3][1] + m44[2][1]*u.m44[3][2]);
				m44[3][2] =  -(m44[0][2]*u.m44[3][0] + m44[1][2]*u.m44[3][1] + m44[2][2]*u.m44[3][2]);
				m44[3][3] =  u.m44[3][3];
			}
			else
*/			{
				// full 4x4 matrix inverse using Cramer's Rule
				// original code: Intel Corp., adapted to prmath

				// JAN: this is the very reliable inverse matrix function that I used for the moving platforms in nicktoons

				// transpose matrix
				F32 src[16];
				src[0] = AX().X(); src[1] = AX().Y(); src[2] = AX().Z(); src[3] = AX().W();
				src[4] = AY().X(); src[5] = AY().Y(); src[6] = AY().Z(); src[7] = AY().W();
				src[8] = AZ().X(); src[9] = AZ().Y(); src[10] = AZ().Z(); src[11] = AZ().W();
				src[12] = AW().X(); src[13] = AW().Y(); src[14] = AW().Z(); src[15] = AW().W();

				// calculate pairs for first 8 elements (cofactors)
				F32 tmp[12]; // temp array for pairs
				tmp[0]  = src[10] * src[15];
				tmp[1]  = src[11] * src[14];
				tmp[2]  = src[9]  * src[15];
				tmp[3]  = src[11] * src[13];
				tmp[4]  = src[9]  * src[14];
				tmp[5]  = src[10] * src[13];
				tmp[6]  = src[8]  * src[15];
				tmp[7]  = src[11] * src[12];
				tmp[8]  = src[8]  * src[14];
				tmp[9]  = src[10] * src[12];
				tmp[10] = src[8]  * src[13];
				tmp[11] = src[9]  * src[12];

				F32 m16[16];

				// calculate first 8 elements (cofactors)
				m16[0] = (tmp[0] * src[5] + tmp[3] * src[6] + tmp[4]  * src[7]) - (tmp[1] * src[5] + tmp[2] * src[6] + tmp[5]  * src[7]);
				m16[1] = (tmp[1] * src[4] + tmp[6] * src[6] + tmp[9]  * src[7]) - (tmp[0] * src[4] + tmp[7] * src[6] + tmp[8]  * src[7]);
				m16[2] = (tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7]) - (tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7]);
				m16[3] = (tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6]) - (tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6]);
				m16[4] = (tmp[1] * src[1] + tmp[2] * src[2] + tmp[5]  * src[3]) - (tmp[0] * src[1] + tmp[3] * src[2] + tmp[4]  * src[3]);
				m16[5] = (tmp[0] * src[0] + tmp[7] * src[2] + tmp[8]  * src[3]) - (tmp[1] * src[0] + tmp[6] * src[2] + tmp[9]  * src[3]);
				m16[6] = (tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3]) - (tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3]);
				m16[7] = (tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2]) - (tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2]);

				// calculate pairs for second 8 elements (cofactors)
				tmp[0]  = src[2] * src[7];
				tmp[1]  = src[3] * src[6];
				tmp[2]  = src[1] * src[7];
				tmp[3]  = src[3] * src[5];
				tmp[4]  = src[1] * src[6];
				tmp[5]  = src[2] * src[5];
				tmp[6]  = src[0] * src[7];
				tmp[7]  = src[3] * src[4];
				tmp[8]  = src[0] * src[6];
				tmp[9]  = src[2] * src[4];
				tmp[10] = src[0] * src[5];
				tmp[11] = src[1] * src[4];
			
				// calculate second 8 elements (cofactors)
				m16[8]  = (tmp[0]  * src[13] + tmp[3]  * src[14] + tmp[4]  * src[15]) - (tmp[1]  * src[13] + tmp[2]  * src[14] + tmp[5]  * src[15]);
				m16[9]  = (tmp[1]  * src[12] + tmp[6]  * src[14] + tmp[9]  * src[15]) - (tmp[0]  * src[12] + tmp[7]  * src[14] + tmp[8]  * src[15]);
				m16[10] = (tmp[2]  * src[12] + tmp[7]  * src[13] + tmp[10] * src[15]) - (tmp[3]  * src[12] + tmp[6]  * src[13] + tmp[11] * src[15]);
				m16[11] = (tmp[5]  * src[12] + tmp[8]  * src[13] + tmp[11] * src[14]) - (tmp[4]  * src[12] + tmp[9]  * src[13] + tmp[10] * src[14]);
				m16[12] = (tmp[2]  * src[10] + tmp[5]  * src[11] + tmp[1]  * src[9]) - (tmp[4]  * src[11] + tmp[0]  * src[9]  + tmp[3]  * src[10]);
				m16[13] = (tmp[8]  * src[11] + tmp[0]  * src[8]  + tmp[7]  * src[10]) - (tmp[6]  * src[10] + tmp[9]  * src[11] + tmp[1]  * src[8]);
				m16[14] = (tmp[6]  * src[9]  + tmp[11] * src[11] + tmp[3]  * src[8]) - (tmp[10] * src[11] + tmp[2]  * src[8]  + tmp[7]  * src[9]);
				m16[15] = (tmp[10] * src[10] + tmp[4]  * src[8]  + tmp[9]  * src[9]) - (tmp[8]  * src[9]  + tmp[11] * src[10] + tmp[5]  * src[8]);

				// calculate determinant
				F32 det = src[0]*m16[0]+src[1]*m16[1]+src[2]*m16[2]+src[3]*m16[3];

				// calculate matrix inverse
				det = 1 / det;
				for ( int j=0; j<16; ++j )
					m16[j] *= det;

				HMat44 out;

				out[0][0] = m16[0]; out[1][0] = m16[1]; out[2][0] = m16[2]; out[3][0] = m16[3];
				out[0][1] = m16[4]; out[1][1] = m16[5]; out[2][1] = m16[6]; out[3][1] = m16[7];
				out[0][2] = m16[8]; out[1][2] = m16[9]; out[2][2] = m16[10]; out[3][2] = m16[11];
				out[0][3] = m16[12]; out[1][3] = m16[13]; out[2][3] = m16[14]; out[3][3] = m16[15];

				return out;
			}
#endif
		}

		void PostMultScale(float sx, float sy, float sz)
		{
			for ( int i=0; i<4; ++i )
			{
				m_A[i][0] *= sx;
				m_A[i][1] *= sy;
				m_A[i][2] *= sz;
			}
		}

		void PostMultTranslate(float tx, float ty, float tz)
		{
			for ( int i=0; i<4; ++i )
			{
				float e = m_A[i][3];
				m_A[i][0] += e * tx;
				m_A[i][1] += e * ty;
				m_A[i][2] += e * tz;
			}
		}


		#ifdef __D3DX9_H__
        // Convert to D3DMATRIX
        operator const D3DMATRIX () const
        {
            //return *((D3DMATRIX*)this);
            
            D3DMATRIX result;
            
            #ifdef _SSE_
            
            _mm_storeu_ps( &result._11, m_A[MX] );
            _mm_storeu_ps( &result._21, m_A[MY] );
            _mm_storeu_ps( &result._31, m_A[MZ] );
            _mm_storeu_ps( &result._41, m_A[MW] );
            
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
            
            _mm_storeu_ps( &result._11, m_A[MX] );
            _mm_storeu_ps( &result._21, m_A[MY] );
            _mm_storeu_ps( &result._31, m_A[MZ] );
            _mm_storeu_ps( &result._41, m_A[MW] );
            
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
        
		#ifdef VEC_MAT_PRINT
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

		/*
		* S = sin (a)
		* C = cos (a)
		* H = sqrt (X*X + Z*Z)
		*
		* [   v   ]  [  C  S        ]  [  1           ]  [ -Z/H  X/H    ]
		*            [ -S  C        ]  [     H  Y     ]  [     1        ]
		*            [        1     ]  [    -Y  H     ]  [ -X/H -Z/H    ]
		*            [           1  ]  [           1  ]  [           1  ]
		*
		*/
		void Align(float a, HVec3 &vNormal) 
		{
			static float dtor = PI / 180.0f;
			float s, c, h, hinv;

			HVec3 normal = vNormal;
			normal.Normalised();

			a *= dtor;
			s = (sin (a));
			c = (cos (a));
			h = (sqrt (normal.x*normal.x + normal.z*normal.z));

			if (h != 0) {
				hinv = 1 / h;

				m_A[0][0] = (-normal.z*c - s*normal.y*normal.x) * hinv;
				m_A[1][0] = (normal.z*s - c*normal.y*normal.x) * hinv;
				m_A[2][0] = -normal.x;
				m_A[3][0] = 0;

				m_A[0][1] = s*h;
				m_A[1][1] = c*h;
				m_A[2][1] = -normal.y;
				m_A[3][1] = 0;

				m_A[0][2] = (c*normal.x - s*normal.y*normal.z) * hinv;
				m_A[1][2] = (-s*normal.x - c*normal.y*normal.z) * hinv;
				m_A[2][2] = -normal.z;
				m_A[3][2] = 0;
			}
			else
			{
				m_A[0][0] = s;
				m_A[1][0] = c;
				m_A[2][0] = 0;
				m_A[3][0] = 0;

				m_A[0][1] = 0;
				m_A[1][1] = 0;
				m_A[2][1] = -1;
				m_A[3][1] = 0;

				m_A[0][2] = -c;
				m_A[1][2] = s;
				m_A[2][2] = 0;
				m_A[3][2] = 0;
			}

			m_A[0][3] = 0;
			m_A[1][3] = 0;
			m_A[2][3] = 0;
			m_A[3][3] = 1;
		}

        
    private:
        
        HVec4 m_A[4];
    };
    
    //
    // Plane Class
    //
    
    class Plane
    {
    public:
        
        Plane()
        {
            m_V = HVec4( c_UnitY );
        }
        
        #ifdef _SSE_
        explicit Plane( const V128& src )
        {
            m_V = HVec4( src );
        }
        #endif
        
        explicit Plane( const HVec4& src )
        {
            m_V = src;
        }
        
        Plane( const Plane& src )
        {
            m_V = src.m_V;
        }
        
        Plane( const HVec3& vector, const HVec3& position )
        {
            const HVec3 normal = vector.Normalised();
            m_V = (HVec4)normal - HVecW( normal | position );
        }
        
        Plane( const HVec3& A, const HVec3& B, const HVec3& C )
        {
            const HVec3 normal = ( ( B - A ) ^ ( C - A ) ).Normalised();
            m_V = (HVec4)normal - HVecW( normal | A );
        }
        
        operator const HVec4 () const
        {
            return m_V;
        }
        
        const HVec3 Normal() const
        {
            return HVec3( m_V );
        }
        
        const HVecX Distance( const HVec3& point ) const
        {
            return ( m_V * ( point + HVecW( 1.0f ) ) ).Sum();
        }
        
        const HVecX DistanceToOrigin() const
        {
            return (HVecX)HVecW( m_V );
        }
        
        const Plane operator + ( const F32 distance ) const
        {
            return Plane( m_V - HVecW( distance ) );
        }
        
        const Plane operator - ( const F32 distance ) const
        {
            return Plane( m_V + HVecW( distance ) );
        }
        
        const Plane operator * ( const HMat43& m ) const
        {
            const HVec3 vector = Normal();
            const HVec3 vertex = vector * -DistanceToOrigin();
            return Plane( vector * (HMat33)m, vertex * m );
        }
        
        const HVecX Intersect( const Line& line ) const;
        
        const bool CoPlanar( const Plane& src,
                             const F32 normalTolerance = cos( PI / 180.0f ),
                             const F32 offsetTolerance = 0.01f ) const
        {
            return ( ( Normal() | src.Normal() ) > normalTolerance ) &&
                   ( ( DistanceToOrigin() - src.DistanceToOrigin() ) < offsetTolerance );
        }
        
		#ifdef VEC_MAT_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            HVec4 v( m_V );
            sprintf( buffer, "%s Plane( Normal( %+11.6f, %+11.6f, %+11.6f ), Offset( %+11.6f ) )\n", text, v.X(), v.Y(), v.Z(), v.W() );
            OutputDebugString( buffer );
        }
		#endif
        
    private:
        
        HVec4   m_V;
    };
    
    
    //
    // Quaternion Class
    //
    
    ALIGN16 class Quaternion
    {
    public:
        
        Quaternion()
        {
            // Initialise to the Multiplicative Identity.
            m_V = c_UnitW;
        }
        
        #ifdef _SSE_
        explicit Quaternion( const V128& src )
        {
            m_V = src;
        }
        #endif
        
        explicit Quaternion( const HVec4& src )
        {
            m_V = src.m_V;
        }
        
        Quaternion( const Quaternion& src )
        {
            m_V = src.m_V;
        }
        
        Quaternion( const F32 vx, const F32 vy, const F32 vz, const F32 a )
        {
            m_V = HVec4( vx, vy, vz, a );
        }
        
        Quaternion( const HVec3& axis, const F32 theta )
        {
            const F32 halfTheta = theta * 0.5f;
            const HVec3 imag = axis.Normalised() * sin( halfTheta );
            const HVecW real = HVecW( cos( halfTheta ) );
            m_V = imag + real;
        }

		Quaternion(const HVec3& anglevec, EulerOrder euler)
		{
			F32 ex = 0;
			F32 ey = 0;
			F32 ez = 0;

			switch(euler)
			{
				case EULER_XYZ: ex = anglevec.x; ey = anglevec.y; ez = anglevec.z; break;
				case EULER_XZY: ex = anglevec.x; ey = anglevec.z; ez = anglevec.y; break;
				case EULER_YXZ: ex = anglevec.y; ey = anglevec.x; ez = anglevec.z; break;
				case EULER_YZX: ex = anglevec.y; ey = anglevec.z; ez = anglevec.x; break;
				case EULER_ZXY: ex = anglevec.z; ey = anglevec.x; ez = anglevec.y; break;
				case EULER_ZYX: ex = anglevec.z; ey = anglevec.y; ez = anglevec.x; break;
			}

			ex *= 0.5f;
			ey *= 0.5f;
			ez *= 0.5f;

			F32 cx = cosf(ex);
			F32 cy = cosf(ey);
			F32 cz = cosf(ez);
			F32 sx = sinf(ex);
			F32 sy = sinf(ey);
			F32 sz = sinf(ez);
 
			F32 cycz = cy * cz;
			F32 sysz = sy * sz;
			F32 sycz = sy * cz;
			F32 cysz = cy * sz;

			m_V[QX] = sx * cycz - cx * sysz;
			m_V[QY] = sx * cysz + cx * sycz;
			m_V[QZ] = cx * cysz - sx * sycz;
			m_V[QA] = cx * cycz + sx * sysz;
		}

		Quaternion( const HMat33& m )
		{
/*			F32 trace = rotm.AX().X() + rotm.AY().Y() + rotm.AY().Y() + 1.0f;
			if( trace > FLT_EPSILON ) 
			{
				F32 s = 0.5f / sqrtf(trace);
				m_V[QA] = 0.25f / s;
				m_V[QX] = ( rotm.AZ().Y() - rotm.AY().Z() ) * s;
				m_V[QY] = ( rotm.AX().Z() - rotm.AZ().X() ) * s;
				m_V[QZ] = ( rotm.AY().X() - rotm.AX().Y() ) * s;
			} 
			else 
			{
			    if ( rotm.AX().X() > rotm.AY().Y() && rotm.AX().X() > rotm.AZ().Z() ) 
				{
					F32 s = 2.0f * sqrtf( 1.0f + rotm.AX().X() - rotm.AY().Y() - rotm.AZ().Z());
					m_V[QX] = 0.25f * s;
					m_V[QY] = (rotm.AX().Y() + rotm.AY().X() ) / s;
					m_V[QZ] = (rotm.AX().Z() + rotm.AZ().X() ) / s;
					m_V[QA] = (rotm.AY().Z() - rotm.AZ().Y() ) / s;
				}
				else if (rotm.AY().Y() > rotm.AZ().Z()) 
				{
					F32 s = 2.0f * sqrtf( 1.0f + rotm.AY().Y() - rotm.AX().X() - rotm.AZ().Z());
					m_V[QX] = (rotm.AX().Y() + rotm.AY().X() ) / s;
					m_V[QY] = 0.25f * s;
					m_V[QZ] = (rotm.AY().Z() + rotm.AZ().Y() ) / s;
					m_V[QA] = (rotm.AX().Z() - rotm.AZ().X() ) / s;
				} 
				else
				{
					float s = 2.0f * sqrtf( 1.0f + rotm.AZ().Z() - rotm.AX().X() - rotm.AY().Y() );
					m_V[QX] = (rotm.AX().Z() + rotm.AZ().X() ) / s;
					m_V[QY] = (rotm.AY().Z() + rotm.AZ().Y() ) / s;
					m_V[QZ] = 0.25f * s;
					m_V[QA] = (rotm.AX().Y() - rotm.AY().X() ) / s;
				}
			}
			*/

			F32 t = m[0][0] + m[1][1] + m[2][2] + 1.0f;

			if ( t > 0 )
			{
				F32 s = 0.5f / sqrtf(t);
				m_V[3] = 0.25f / s;
				m_V[0] = (m[2][1] - m[1][2]) * s;
				m_V[1] = (m[0][2] - m[2][0]) * s;
				m_V[2] = (m[1][0] - m[0][1]) * s;
			} 
			else 
			{
				if ( m[0][0] > m[1][1] && m[0][0] > m[2][2] ) {
					F32 s = 2.0f * sqrtf( 1.0f + m[0][0] - m[1][1] - m[2][2]);
					m_V[0] = 0.25f * s;
					m_V[1] = (m[0][1] + m[1][0] ) / s;
					m_V[2] = (m[0][2] + m[2][0] ) / s;
					m_V[3] = (m[1][2] - m[2][1] ) / s;
				} 
				else if (m[1][1] > m[2][2])
				{
					F32 s = 2.0f * sqrtf( 1.0f + m[1][1] - m[0][0] - m[2][2]);
					m_V[0] = (m[0][1] + m[1][0] ) / s;
					m_V[1] = 0.25f * s;
					m_V[2] = (m[1][2] + m[2][1] ) / s;
					m_V[3] = (m[0][2] - m[2][0] ) / s;
				} else 
				{
					F32 s = 2.0f * sqrtf( 1.0f + m[2][2] - m[0][0] - m[1][1] );
					m_V[0] = (m[0][2] + m[2][0] ) / s;
					m_V[1] = (m[1][2] + m[2][1] ) / s;
					m_V[2] = 0.25f * s;
					m_V[3] = (m[0][1] - m[1][0] ) / s;
				}
			}
		}
        
        Quaternion( const HVec3& imag, const HVecW& real )
        {
            m_V = imag + real;
        }
        
        explicit Quaternion( const HVec3& imag )
        {
            m_V = imag;
        }
        
        explicit Quaternion( const HVecW& real )
        {
            m_V = real;
        }

		HMat33 GetMatrix33() const
		{
			HMat33 m;
			float d,s,xs,ys,zs,wx,wy,wz,xx,xy,xz,yy,yz,zz;

			d = m_V[QX]*m_V[QX] + m_V[QY]*m_V[QY] + m_V[QZ]*m_V[QZ] + m_V[QA]*m_V[QA];
			if (d == 0.0) 
			{
				s = 1.0f;
			}
			else 
			{
				s = 2.0f/d;
			}

			xs = m_V[QX] * s;   ys = m_V[QY] * s;  zs = m_V[QZ] * s;
			wx = m_V[QA] * xs;  wy = m_V[QA] * ys; wz = m_V[QA] * zs;
			xx = m_V[QX] * xs;  xy = m_V[QX] * ys; xz = m_V[QX] * zs;
			yy = m_V[QY] * ys;  yz = m_V[QY] * zs; zz = m_V[QZ] * zs;

			m[0][0] = (1.0f - (yy + zz));
			m[0][1] = (xy - wz);
			m[0][2] = (xz + wy);

			m[1][0] = (xy + wz);
			m[1][1] = (1.0f - (xx + zz));
			m[1][2] = (yz - wx);

			m[2][0] = (xz - wy);
			m[2][1] = (yz + wx);
			m[2][2] = (1.0f - (xx + yy));

			return m;
		}

		HMat44 GetMatrix44() const
		{
			HMat44 m;
			float d,s,xs,ys,zs,wx,wy,wz,xx,xy,xz,yy,yz,zz;

			d = m_V[QX]*m_V[QX] + m_V[QY]*m_V[QY] + m_V[QZ]*m_V[QZ] + m_V[QA]*m_V[QA];
			if (d == 0.0f) 
			{
				s = 1.0f;
			}
			else 
			{
				s = 2.0f/d;
			}

			xs = m_V[QX] * s;   ys = m_V[QY] * s;  zs = m_V[QZ] * s;
			wx = m_V[QA] * xs;  wy = m_V[QA] * ys; wz = m_V[QA] * zs;
			xx = m_V[QX] * xs;  xy = m_V[QX] * ys; xz = m_V[QX] * zs;
			yy = m_V[QY] * ys;  yz = m_V[QY] * zs; zz = m_V[QZ] * zs;

			m[0][0] = (1.0f - (yy + zz));
			m[0][1] = (xy - wz);
			m[0][2] = (xz + wy);
			m[0][3] = 0;

			m[1][0] = (xy + wz);
			m[1][1] = (1.0f - (xx + zz));
			m[1][2] = (yz - wx);
			m[1][3] = 0;

			m[2][0] = (xz - wy);
			m[2][1] = (yz + wx);
			m[2][2] = (1.0f - (xx + yy));
			m[2][3] = 0;

			m[3][0] = 0;
			m[3][1] = 0;
			m[3][2] = 0;
			m[3][3] = 1.0f;

			return m;
		}

        
        // Returns the Imaginary part as a HVec3
        const HVec3 Imag() const
        {
            return (HVec3)HVec4( m_V );
        }
        
        // Returns the Real part as a HVecW
        const HVecW Real() const
        {
            return (HVecW)HVec4( m_V );
        }
        
        // Addition
        const Quaternion operator + ( const Quaternion& src ) const
        {
            return Quaternion( HVec4( m_V ) + HVec4( src.m_V ) );
        }
        
        // Subtraction
        const Quaternion operator - ( const Quaternion& src ) const
        {
            return Quaternion( HVec4( m_V ) - HVec4( src.m_V ) );
        }
        
        // Unary Negation
        const Quaternion operator - () const
        {
            return Quaternion( -HVec4( m_V ) );
        }
        
        // Conjugate (Multiplicative Inverse) - Assumes Unit Quaternion!
        const Quaternion operator ! () const
        {
            return Quaternion( -Imag() + Real() );
        }
        
        // Scalar multiplication
        const Quaternion operator * ( const F32 f ) const
        {
            return Quaternion( HVec4( m_V ) * f );
        }
        
        // Multiplication
        //const Quaternion operator * ( const Quaternion& r ) const
        const Quaternion operator * ( const Quaternion& l ) const
        {
            // q1q2 = ( a1*a2 - v1|v2, a1*v2 + a2*v1 + v1^v2 )
            
            //const Quaternion& l = *this;
            const Quaternion& r = *this;
            const HVec3 vl = l.Imag();
            const HVec3 vr = r.Imag();
            
            // Scalar * Vector components a1*v2 + a2*v1 +
            // Vector cross product component v1^v2
            const HVec3 imag = ( vr * l.Real() ) + ( vl * r.Real() ) + ( vl ^ vr );
            
            #ifdef _SSE_
            
            // Scalar multiplication component a1*a2 -
            // Vector dot product component v1|v2
            const HVec4 dotXYZA = HVec4( l.m_V ) * HVec4( r.m_V );
            const V128 dotX = dotXYZA;
            const V128 dotY = ReplicateY( dotXYZA );
            const V128 dotZ = ReplicateZ( dotXYZA );
            const V128 dotA = ReplicateW( dotXYZA );
            
            const V128 real = _mm_andnot_ps( c_MaskXYZ, ReplicateX( _mm_sub_ss( dotA, _mm_add_ss( _mm_add_ss( dotX, dotY ), dotZ ) ) ) );
            
            // Combine real & imaginary parts
            const V128 q = _mm_add_ps( imag, real );
            
            return Quaternion( q );
            
            #else
            
            // Scalar multiplication component a1*a2 -
            // Vector dot product component v1|v2
            const F32 real = ( l.m_V[QA] * r.m_V[QA] ) - ( vl | vr );
            
            return Quaternion( imag.m_V[VX], imag.m_V[VY], imag.m_V[VZ], real );
            
            #endif
        }
        
        // Division: q1 / q2 = q1 * q2-1
        const Quaternion operator / ( const Quaternion& r ) const
        {
            const Quaternion& l = *this;
            return l * !r;
        }
        
        // Returns Unit Quaternion
        const Quaternion Normalised() const
        {
            return Quaternion( HVec4( m_V ).Normalised() );
        }
        
        // Lerp: Linear Interpolation
        const Quaternion Lerp( const Quaternion& dst, const F32 t ) const
        {
            const Quaternion& src = *this;
            return Quaternion( ( src * ( 1.0f - t ) ) + ( dst * t ) );
        }
        
        // Slerp: Spherical Linear Interpolation
        const Quaternion Slerp( const Quaternion& q, const F32 t ) const
        {
            const Quaternion& src = *this;
            Quaternion dst = q;
            
            F32 cosHalfTheta = HVec4( src.m_V ) | HVec4( dst.m_V );
            
            if ( cosHalfTheta < 0.0f )
            {
                cosHalfTheta = fabsf( cosHalfTheta );
                dst = -q;
            }
            
            // Need check for small angles!
            if ( ( 1.0f - cosHalfTheta ) >= FLT_EPSILON )
            {
                const F32 halfTheta = acos( cosHalfTheta );
                const F32 sinHalfTheta = sin( halfTheta );
                const F32 reciprocalSinHalfTheta = 1.0f / sinHalfTheta;
                
                const F32 scale0 = sin( ( 1.0f - t ) * halfTheta ) * reciprocalSinHalfTheta;
                const F32 scale1 = sin( t * halfTheta ) * reciprocalSinHalfTheta;
                
                return Quaternion( ( src * scale0 ) + ( dst * scale1 ) );
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
            
            const HVec4 xyza2 = HVec4( m_V ) + HVec4( m_V );
            
            const HVec3 ax = (HVec3)c_UnitX +
                             (HVec3)Shuffle<VY,VX,VX,0>( m_V ) * (HVec3)Shuffle<VY,VY,VZ,0>( xyza2 ) * (HVec3)Shuffle<1,0,0,3>( c_P1M1ZZ ) +
                             (HVec3)Shuffle<VZ,VW,VW,0>( m_V ) * (HVec3)Shuffle<VZ,VZ,VY,0>( xyza2 ) * (HVec3)Shuffle<1,0,1,3>( c_P1M1ZZ );
            
            const HVec3 ay = (HVec3)c_UnitY +
                             (HVec3)Shuffle<VX,VX,VY,0>( m_V ) * (HVec3)Shuffle<VY,VX,VZ,0>( xyza2 ) * (HVec3)Shuffle<0,1,0,3>( c_P1M1ZZ ) +
                             (HVec3)Shuffle<VW,VZ,VW,0>( m_V ) * (HVec3)Shuffle<VZ,VZ,VX,0>( xyza2 ) * (HVec3)Shuffle<1,1,0,3>( c_P1M1ZZ );
            
            const HVec3 az = (HVec3)c_UnitZ +
                             (HVec3)Shuffle<VX,VY,VX,0>( m_V ) * (HVec3)Shuffle<VZ,VZ,VX,0>( xyza2 ) * (HVec3)Shuffle<0,0,1,3>( c_P1M1ZZ ) +
                             (HVec3)Shuffle<VW,VW,VY,0>( m_V ) * (HVec3)Shuffle<VY,VX,VY,0>( xyza2 ) * (HVec3)Shuffle<0,1,1,3>( c_P1M1ZZ );
            
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
#ifndef GCC
        const S32 Finite() const
        {
            return m_V.Finite();
        }
#endif
        
		#ifdef VEC_MAT_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            HVec4 v( m_V );
            sprintf( buffer, "%s Quaternion( Imag( %+11.6f, %+11.6f, %+11.6f ), Real( %+11.6f ) )\n", text, v.X(), v.Y(), v.Z(), v.W() );
            OutputDebugString( buffer );
        }
		#endif
        
    private:
        
        // The quaternion is stored with the real(scalar) part in the W component and
        // the imaginary(vector) part in the X, Y & Z components.
        
        HVec4   m_V;
    };
    
    //
    // Line Class
    //
    
    ALIGN16 class Line
    {
    friend class Plane;
    public:
        
        Line( const HVec3& p0, const HVec3& p1 )
        {
            m_Q = p0;
            m_V = p1 - p0;
        }
        
        const HVec3 Q() const { return m_Q; }
        const HVec3 V() const { return m_V; }
        
        const HVec3 Point( const HVecX& t ) const
        {
            return m_Q + ( m_V * t );
        }
        
        const HVec3 operator () ( const HVecX& t ) const
        {
            return m_Q + ( m_V * t );
        }
        
		#ifdef VEC_MAT_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            
            sprintf( buffer, "%s Q", text );
            m_Q.Print( buffer );
            
            sprintf( buffer, "%s V", text );
            m_V.Print( buffer );
        }
		#endif
        
    private:
        
        HVec3   m_Q;
        HVec3   m_V;
    };
    
    //
    // Sphere Class
    //
    
    class LineSphereIntersect
    {
    public:
        
        LineSphereIntersect( const F32 t0, const F32 t1 )
        {
            m_T0 = t0;
            m_T1 = t1;
        }
        
        F32 m_T0;
        F32 m_T1;
    };
    
    ALIGN16 class Sphere
    {
    public:
        
        Sphere()
        {
            // Initialise to zero.
            m_V = c_Zero;
        }
        
        #ifdef _SSE_
        explicit Sphere( const V128& src )
        {
            m_V = src;
        }
        #endif
        
        Sphere( const Sphere& src )
        {
            m_V = src.m_V;
        }
        
        Sphere( const F32 vx, const F32 vy, const F32 vz, const F32 r )
        {
            m_V = HVec4( vx, vy, vz, r );
        }
        
        Sphere( const HVec3& centre, const HVecW& radius )
        {
            m_V = centre + radius;
        }
        
        // Returns the centre as a HVec3
        const HVec3 Centre() const
        {
            return (HVec3)HVec4( m_V );
        }
        
        // Returns the radius as a HVecW
        const HVecW Radius() const
        {
            return (HVecW)HVec4( m_V );
        }
        
        // Line-Sphere Intersection
        const LineSphereIntersect Intersect( const Line& line ) const
        {
            F32 t0 = -1.0f;
            F32 t1 = -1.0f;
            
            // Quadratic formula
            
            const HVec3 P = Centre();
            const HVec3 Q = line.Q();
            const HVec3 v = line.V();
            const F32 r = Radius();
            
            const HVec3 pq = Q - P;
            
            const F32 a = v | v;
            
            if ( fabs( a ) > FLT_EPSILON )
            {
                const F32 b = 2.0f * ( v | pq );
                const F32 c = ( pq | pq ) - ( r * r );
                
                const F32 square = ( b * b ) - ( 4.0f * a * c );
                
                if ( square >= 0.0f )
                {
                    const F32 root = sqrt( square );
                    
                    const F32 r2a = 0.5f / a;
                    
                    t0 = ( -b + root ) * r2a;
                    t1 = ( -b - root ) * r2a;
                }
            }
            
            return LineSphereIntersect( t0, t1 );
        }
        
        // Point inside sphere
        const bool Inside( const HVec3& point ) const
        {
            return ( point - Centre() ).Length() <= (HVecX)Radius();
        }
        
        // Distance from sphere
        const HVecX Distance( const HVec3& point ) const
        {
            return ( point - Centre() ).Length() - (HVecX)Radius();
        }
        
        // Sphere-Sphere Intersection
        bool Intersect( const Sphere& src ) const
        {
            const HVecX d = Distance( src.Centre() );
            return d < HVecX( Radius() + src.Radius() );
        }
        
        const Sphere operator * ( const HMat43& m ) const
        {
            return Sphere( Centre() * m, Radius() );
        }
        
		#ifdef VEC_MAT_PRINT
        void Print( char* text = "" ) const
        {
            char buffer[256];
            HVec4 v( m_V );
            sprintf( buffer, "%s Sphere( Centre( %+11.6f, %+11.6f, %+11.6f ), Radius( %+11.6f ) )\n", text, v.X(), v.Y(), v.Z(), v.W() );
            OutputDebugString( buffer );
        }
		#endif
        
    private:
        
        // The sphere is stored with the centre in the X, Y & Z components and the radius in the W component
        HVec4   m_V;
    };
    
    
    
    //
    // HVecX Class Member Functions
    //
    
    // Explicit
    inline HVecX::HVecX( const HVecY& src )
    {
        #ifdef _SSE_
        m_X = Shuffle<VY,0,0,0>( src.m_Y );
        #else
        m_X = src.m_Y;
        #endif
    }
    
    // Explicit
    inline HVecX::HVecX( const HVecZ& src )
    {
        #ifdef _SSE_
        m_X = Shuffle<VZ,0,0,0>( src.m_Z );
        #else
        m_X = src.m_Z;
        #endif
    }
    
    // Explicit
    inline HVecX::HVecX( const HVecW& src )
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
    inline HVecY::HVecY( const HVecX& src )
    {
        #ifdef _SSE_
        m_Y = Shuffle<1,VX,1,1>( src.m_X );
        #else
        m_Y = src.m_X;
        #endif
    }
    
    // Explicit
    inline HVecY::HVecY( const HVecZ& src )
    {
        #ifdef _SSE_
        m_Y = Shuffle<1,VZ,1,1>( src.m_Z );
        #else
        m_Y = src.m_Z;
        #endif
    }
    
    // Explicit
    inline HVecY::HVecY( const HVecW& src )
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
    inline HVecZ::HVecZ( const HVecX& src )
    {
        #ifdef _SSE_
        m_Z = Shuffle<2,2,VX,2>( src.m_X );
        #else
        m_Z = src.m_X;
        #endif
    }
    
    // Explicit
    inline HVecZ::HVecZ( const HVecY& src )
    {
        #ifdef _SSE_
        m_Z = Shuffle<2,2,VY,2>( src.m_Y );
        #else
        m_Z = src.m_Y;
        #endif
    }
    
    // Explicit
    inline HVecZ::HVecZ( const HVecW& src )
    {
        #ifdef _SSE_
        m_Z = Shuffle<2,2,VW,2>( src.m_W );
        #else
        m_Z = src.m_W;
        #endif
    }
    
   
    //
    // HVec2 Class Member Functions
    //
    
    // Explicit
    FORCE_INLINE  HVec2::HVec2( const HVec3& src )
    {
        #ifdef _SSE_
        m_V = _mm_and_ps( c_MaskXY, src.m_V );
        #else
        m_V[VX] = src.m_V[VX];
        m_V[VY] = src.m_V[VY];
        #endif
    }
    
    // Explicit
    FORCE_INLINE  HVec2::HVec2( const HVec4& src )
    {
        #ifdef _SSE_
        m_V = _mm_and_ps( c_MaskXY, src.m_V );
        #else
        m_V[VX] = src.m_V[VX];
        m_V[VY] = src.m_V[VY];
        #endif
    }
    
    //
    // HVec3 Class Member Functions
    //
    
    // Explicit
    inline HVec3::HVec3( const HVec4& src )
    {
        #ifdef _SSE_
        m_V = _mm_and_ps( c_MaskXYZ, src.m_V );
        #else
        m_V[VX] = src.m_V[VX];
        m_V[VY] = src.m_V[VY];
        m_V[VZ] = src.m_V[VZ];
        #endif
    }
    
    inline const HVec3 HVec3::operator * ( const HMat33& m ) const
    {
        #ifdef _SSE_
        
        const V128 x_r0 = _mm_mul_ps( ReplicateX( m_V ), m.m_A[MX].m_V );
        const V128 y_r1 = _mm_mul_ps( ReplicateY( m_V ), m.m_A[MY].m_V );
        const V128 z_r2 = _mm_mul_ps( ReplicateZ( m_V ), m.m_A[MZ].m_V );
        
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
    
    inline const HVec3 HVec3::operator * ( const HMat43& m ) const
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

	inline const HVec3 HVec3::TransformNormal( const HMat43& m ) const
    {
		//TODO: implement matrix inverse function so that we can multiply this normal by the 
		//inverse transpose of the transformation matrix m. Currently, the normal will not be
		//valid for scaling matrices.

		HVec3 dst;
		dst = (*this) * (HMat33)m;
		return dst.Normalised();
    }
    
    inline const HVec3 HVec3::operator * ( const HMat44& m ) const
    {
/*
#ifdef __D3DX9_H__
		D3DXVec3TransformCoord((D3DXVECTOR3*)this, (D3DXVECTOR3*)this, (D3DXMATRIX*)&m);
		return *this;
#else
*/
        #ifdef _SSE_
        
        const V128 x_r0 = _mm_mul_ps( ReplicateX( m_V ), m.m_A[MX].m_V );
        const V128 y_r1 = _mm_mul_ps( ReplicateY( m_V ), m.m_A[MY].m_V );
        const V128 z_r2 = _mm_mul_ps( ReplicateZ( m_V ), m.m_A[MZ].m_V );
        const V128 w_r3 = m.m_A[MW].m_V;
        
        const V128 result = _mm_add_ps( _mm_add_ps( x_r0, y_r1 ), _mm_add_ps( z_r2, w_r3 ) );
        
        return HVec3( _mm_and_ps( c_MaskXYZ, result ) );
        
        #else
        
        const HVec3& v = *this;
        
        const F32 x = v[VX] * m[MX][VX] + v[VY] * m[MY][VX] + v[VZ] * m[MZ][VX] + m[MW][VX];
        const F32 y = v[VX] * m[MX][VY] + v[VY] * m[MY][VY] + v[VZ] * m[MZ][VY] + m[MW][VY];
        const F32 z = v[VX] * m[MX][VZ] + v[VY] * m[MY][VZ] + v[VZ] * m[MZ][VZ] + m[MW][VZ];
        
        return HVec3( x, y, z );
        
        #endif
//#endif
    }
    
    // Vector * Quaternion Multiplication
    inline const HVec3 HVec3::operator * ( const Quaternion& q ) const
    {
        return ( q * Quaternion( *this ) * !q ).Imag();
    }
    
    // Combine HVec3 with HVecW to form a HVec4
    inline const HVec4 HVec3::operator + ( const HVecW& src ) const
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
    
	#ifdef VEC_MAT_PRINT
    inline void HVec3::DebugValidate() const
    {
        #ifdef _DEBUG
        #ifdef _SSE_
        if ( HVec4( m_V ).W() != 0.0f )
        {
            Print( "HVec3::DebugValidate(): Invalid HVec3!" );
        }
        #endif
        #endif
    }
	#endif
    
    //
    // HVec4 Class Member Functions
    //
    
    inline const HVec4 HVec4::operator * ( const HMat44& m ) const
    {
        #ifdef _SSE_
        
        const V128 x_r0 = _mm_mul_ps( ReplicateX( m_V ), m.m_A[MX].m_V );
        const V128 y_r1 = _mm_mul_ps( ReplicateY( m_V ), m.m_A[MY].m_V );
        const V128 z_r2 = _mm_mul_ps( ReplicateZ( m_V ), m.m_A[MZ].m_V );
        const V128 w_r3 = _mm_mul_ps( ReplicateW( m_V ), m.m_A[MW].m_V );
        
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
    inline HMat33::HMat33( const HMat43& src )
    {
        m_A[MX] = src.m_A[MX];
        m_A[MY] = src.m_A[MY];
        m_A[MZ] = src.m_A[MZ];
    }
    
    // Explicit 
    inline HMat33::HMat33( const HMat44& src )
    {
        #ifdef _SSE_
        
        m_A[MX].m_V = _mm_and_ps( c_MaskXYZ, src.m_A[MX].m_V );
        m_A[MY].m_V = _mm_and_ps( c_MaskXYZ, src.m_A[MY].m_V );
        m_A[MZ].m_V = _mm_and_ps( c_MaskXYZ, src.m_A[MZ].m_V );
        
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
    inline const HMat43 HMat33::operator + ( const HVec3& pos ) const
    {
        return HMat43( m_A[MX], m_A[MY], m_A[MZ], pos );
    }

	#ifdef __D3DX__H__
    inline HMat33::operator const D3DMATRIX () const
    {
        return (D3DMATRIX)HMat44( *this );
    }
    
    inline HMat33::operator const D3DXMATRIX () const
    {
        return (D3DXMATRIX)HMat44( *this );
    }
	#endif
    
    //
    // HMat43 Class Member Functions
    //
    
    // Explicit 
    inline HMat43::HMat43( const HMat44& src )
    {
        #ifdef _SSE_
        
        m_A[MX].m_V = _mm_and_ps( c_MaskXYZ, src.m_A[MX].m_V );
        m_A[MY].m_V = _mm_and_ps( c_MaskXYZ, src.m_A[MY].m_V );
        m_A[MZ].m_V = _mm_and_ps( c_MaskXYZ, src.m_A[MZ].m_V );
        m_A[MW].m_V = _mm_and_ps( c_MaskXYZ, src.m_A[MW].m_V );
        
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
    
	#ifdef __D3DX__H__
    inline HMat43::operator const D3DMATRIX () const
    {
        return (D3DMATRIX)HMat44( *this );
    }
    
    inline HMat43::operator const D3DXMATRIX () const
    {
        return (D3DXMATRIX)HMat44( *this );
    }
	#endif
    
    //
    // HMat44 Class Member Functions
    //
    
    
    
    //
    // Plane Class Member Functions
    //
    
    inline const HVecX Plane::Intersect( const Line& line ) const
    {
        const HVec3 N = Normal();
        const HVec3 P = N * -DistanceToOrigin();
        const HVecX denom = N | line.m_V;
        if ( denom.Abs() > FLT_EPSILON )
        {
            return ( N | ( P - line.m_Q ) ) / denom;
        }
        else
        {
            return -FLT_MAX;
        }
    }

	// Intersect two line segments
	inline bool Intersect(	const HVec2& vec1, 
							const HVec2& vec2,
							const HVec2& Bvec1,
							const HVec2& Bvec2)
	{
		typedef float T;

		// (Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
		T q = (vec2.x-vec1.x) * (Bvec2.y-Bvec1.y) - (vec2.y-vec1.y) * (Bvec2.x-Bvec1.x);

		if (q == 0) return false; // parallel line segments

		// ( (Ay-Cy)(Dx-Cx)-(Ax-Cx)(Dy-Cy) ) / q
		T r = ( (vec1.y-Bvec1.y) * (Bvec2.x-Bvec1.x) - (vec1.x-Bvec1.x) * (Bvec2.y-Bvec1.y) ) / q;

		if (r < 0.0f || r > 1.0f) return false;

		// ( (Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay) ) / q
		T s = ( (vec1.y-Bvec1.y) * (vec2.x-vec1.x) - (vec1.x-Bvec1.x) * (vec2.y-vec1.y) ) / q;

		if (s < 0.0f || s > 1.0f) return false;
		return true;
	}

#ifndef USE_PRMATH_FOR_FLOAT3

	// cross product
	inline const HVec3 CrossProduct( const HVec3 a, const HVec3 b)
	{
		return a ^ b;
	}

	// normalize anonymous vector
	inline const HVec3 Normalize( const HVec3 a)
	{
		return a.Normalised();
	}

	// return the average of a vector
	inline const float Average( const HVec3 a)
	{
		return (a.x + a.y + a.z) / 3.0f;
	}

	// get length
	inline const float Length( const HVec3 v )
	{
		return v.Length();
	}

	// dot product
	inline const HVecX DotProduct( const HVec3 a, const HVec3 b)
	{
		return a | b;
	}

	// dot product
	inline const HVecX DotProduct( const HVec2 a, const HVec2 b)
	{
		return a | b;
	}
#endif

//};

/*
using VecMat::RGBA;

using VecMat::HVecX;
using VecMat::HVecY;
using VecMat::HVecZ;
using VecMat::HVecW;

using VecMat::HVec2;
using VecMat::HVec3;
using VecMat::HVec4;

using VecMat::RotX;
using VecMat::RotY;
using VecMat::RotZ;

using VecMat::AxisX;
using VecMat::AxisY;
using VecMat::AxisZ;

using VecMat::HMat33;
using VecMat::HMat43;
using VecMat::HMat44;

using VecMat::Plane;
using VecMat::Quaternion;
using VecMat::Line;
using VecMat::Sphere;
using VecMat::LineSphereIntersect;
*/


typedef HVec2 float2;
#ifndef USE_PRMATH_FOR_FLOAT3
typedef HVec3 float3;
#endif
typedef HVec4 float4;
typedef HMat44 float4x4;


#endif
