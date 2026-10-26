#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_STORAGE_H
#define SOFT_MATHS_STORAGE_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_STORAGE_H
#define MATHS_STORAGE_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

#include <core/core.h>
#include <core/maths/constants.h>

#if defined( COMPILER_GNU )
#define STOREALIGN16 __attribute__((aligned(16)))
#elif defined( COMPILER_VC )
#define STOREALIGN16 __declspec(align(16))
#else
#define STOREALIGN16
#endif

#if defined( COMPILER_GNU )
#define STOREALIGN128 __attribute__((aligned(128)))
#elif defined( COMPILER_VC )
#define STOREALIGN128 __declspec(align(128))
#else
#define STOREALIGN128
#endif

namespace MathsStorage
{

#if defined( COMPILER_VC )
#pragma warning( push )
#pragma warning( disable: 4201 ) // warning C4201: nonstandard extension used : nameless struct/union
#pragma warning( disable: 4324 ) // warning C4324: structure was padded due to __declspec(align())
#endif

// short vectors

struct Short2
{
	Short2() : x(0), y(0) {}
	Short2(S16 _x, S16 _y) : x(_x), y(_y) {}
	
	union 
	{
		struct
		{
			S16 x;
			S16 y;
		};
		S16 v[2];
	};
};

struct Short3
{
	Short3() : x(0), y(0), z(0) {}
	Short3(S16 _x, S16 _y, S16 _z) : x(_x), y(_y), z(_z) {}
	
	union 
	{
		struct
		{
			S16 x;
			S16 y;
			S16 z;
		};
		S16 v[3];
	};
};

struct Short4
{
	Short4() : x(0), y(0), z(0), w(0) {}
	Short4(S16 _x, S16 _y, S16 _z, S16 _w) : x(_x), y(_y), z(_z), w(_w) {}
	
	union 
	{
		struct
		{
			S16 x;
			S16 y;
			S16 z;
			S16 w;
		};
		S16 v[4];
	};
};

/// Normalised array of 4 S16's
/// In conversion to floats, the range of values maps from ( -32767, +32767 ) to ( -1.0f, +1.0f )
struct ShortN4
{
	ShortN4() : x(0), y(0), z(0), w(0) {}
	ShortN4( S16 _x, S16 _y, S16 _z, S16 _w ) : x(_x), y(_y), z(_z), w(_w) {}
	
	union 
	{
		struct
		{
			S16 x;
			S16 y;
			S16 z;
			S16 w;
		};
		S16 v[4];
	};
};

// unsigned short vectors

struct UShort2
{
	UShort2() : x(0), y(0) {}
	UShort2(U16 _x, U16 _y) : x(_x), y(_y) {}
	
	union 
	{
		struct
		{
			U16 x;
			U16 y;
		};
		U16 v[2];
	};
};

struct UShort3
{
	UShort3() : x(0), y(0), z(0) {}
	UShort3(U16 _x, U16 _y, U16 _z) : x(_x), y(_y), z(_z) {}
	
	union 
	{
		struct
		{
			U16 x;
			U16 y;
			U16 z;
		};
		U16 v[3];
	};
};

struct UShort4
{
	UShort4() : x(0), y(0), z(0), w(0) {}
	UShort4(U16 _x, U16 _y, U16 _z, U16 _w) : x(_x), y(_y), z(_z), w(_w) {}
	
	union 
	{
		struct
		{
			U16 x;
			U16 y;
			U16 z;
			U16 w;
		};
		U16 v[4];
	};
};

// int vectors

struct Int2
{
	Int2() : x(0), y(0) {}
	Int2(S32 _x, S32 _y) : x(_x), y(_y) {}
	
	union 
	{
		struct
		{
			S32 x;
			S32 y;
		};
		S32 v[2];
	};
};

struct Int3
{
	Int3() : x(0), y(0),z(0) {}
	Int3(S32 _x, S32 _y, S32 _z) : x(_x), y(_y), z(_z) {}
	
	union 
	{
		struct
		{
			S32 x;
			S32 y;
			S32 z;
		};
		S32 v[3];
	};
};

struct Int4
{
	Int4() : x(0), y(0), z(0), w(0) {}
	Int4(S32 _x, S32 _y, S32 _z, S32 _w) : x(_x), y(_y), z(_z), w(_w) {}
	
	union 
	{
		struct
		{
			S32 x;
			S32 y;
			S32 z;
			S32 w;
		};
		S32 v[4];
	};
};

// unsigned int vectors

struct UInt2
{
	UInt2() : x(0), y(0) {}
	UInt2(U32 _x, U32 _y) : x(_x), y(_y) {}
	
	union 
	{
		struct
		{
			U32 x;
			U32 y;
		};
		U32 v[2];
	};
};

struct UInt3
{
	UInt3() : x(0), y(0), z(0) {}
	UInt3(U32 _x, U32 _y, U32 _z) : x(_x), y(_y), z(_z) {}
	
	union 
	{
		struct
		{
			U32 x;
			U32 y;
			U32 z;
		};
		U32 v[3];
	};
};

struct UInt4
{
	UInt4() : x(0), y(0), z(0), w(0) {}
	UInt4(U32 _x, U32 _y, U32 _z, U32 _w) : x(_x), y(_y), z(_z), w(_w) {}
	
	union 
	{
		struct
		{
			U32 x;
			U32 y;
			U32 z;
			U32 w;
		};
		U32 v[4];
	};
};

// float vectors

struct Float1
{
	union 
	{
		struct
		{
			F32 v0;
		};
		F32 v[1];
	};
};

struct Float2
{
	Float2( const Maths::_UNDEFINED /*undefined*/ ) {}
	Float2() : x(0.f), y(0.f) {}
	Float2(F32 _x, F32 _y) : x(_x), y(_y) {}

	/// Write to stream
	template < typename T_Stream >
	FORCE_INLINE void Serialise( T_Stream& stream )
	{
		stream << x;
		stream << y;
	}

	/// Read from stream
	template < typename T_Stream >
	FORCE_INLINE void Deserialise( T_Stream& stream )
	{
		stream >> x;
		stream >> y;
	}
	
	union 
	{
		struct
		{
			F32 x;
			F32 y;
		};
		F32 v[2];
	};
};

struct Float3
{
	Float3( const Maths::_UNDEFINED /*undefined*/ ) {}
	Float3() : x(0.f), y(0.f), z(0.f) {}
	Float3(F32 _x, F32 _y, F32 _z) : x(_x), y(_y), z(_z) {}

	/// Write to stream
	template < typename T_Stream >
	FORCE_INLINE void Serialise( T_Stream& stream )
	{
		stream << x;
		stream << y;
		stream << z;
	}

	/// Read from stream
	template < typename T_Stream >
	FORCE_INLINE void Deserialise( T_Stream& stream )
	{
		stream >> x;
		stream >> y;
		stream >> z;
	}

	union 
	{
		struct
		{
			F32 x;
			F32 y;
			F32 z;
		};
		F32 v[3];
	};
};

struct Float4
{
	Float4( const Maths::_UNDEFINED /*undefined*/ ) {}
	Float4() : x(0.f), y(0.f), z(0.f), w(0.f) {}
	Float4(F32 _x, F32 _y, F32 _z, F32 _w) : x(_x), y(_y), z(_z), w(_w) {}

	/// Write to stream
	template < typename T_Stream >
	FORCE_INLINE void Serialise( T_Stream& stream )
	{
		stream << x;
		stream << y;
		stream << z;
		stream << w;
	}

	/// Read from stream
	template < typename T_Stream >
	FORCE_INLINE void Deserialise( T_Stream& stream )
	{
		stream >> x;
		stream >> y;
		stream >> z;
		stream >> w;
	}

	union 
	{
		struct
		{
			F32 x;
			F32 y;
			F32 z;
			F32 w;
		};
		F32 v[4];
	};
};

struct Float5
{
	union 
	{
		struct { F32 v0, v1, v2, v3, v4; };
		F32 v[5];
	};
};

struct Float6
{
	union 
	{
		struct { F32 v0, v1, v2, v3, v4, v5; };
		F32 v[6];
	};
};

struct Float7
{
	union 
	{
		struct { F32 v0, v1, v2, v3, v4, v5, v6; };
		F32 v[7];
	};
};

struct Float8
{
	union 
	{
		struct { F32 v0, v1, v2, v3, v4, v5, v6, v7; };
		F32 v[8];
	};
};

struct Float9
{
	union 
	{
		struct { F32 v0, v1, v2, v3, v4, v5, v6, v7, v8; };
		F32 v[9];
	};
};

struct Float10
{
	union 
	{
		struct { F32 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9; };
		F32 v[10];
	};
};

struct Float11
{
	union 
	{
		struct { F32 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10; };
		F32 v[11];
	};
};

struct Float12
{
	union 
	{
		struct { F32 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11; };
		F32 v[12];
	};
};

struct Float13
{
	union 
	{
		struct { F32 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12; };
		F32 v[13];
	};
};

struct Float14
{
	union 
	{
		struct { F32 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13; };
		F32 v[14];
	};
};

struct Float15
{
	union 
	{
		struct { F32 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14; };
		F32 v[15];
	};
};

struct Float16
{
	union 
	{
		struct { F32 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15; };
		F32 v[16];
	};
};






// 3x3 floats, used for temporary storage of matrix. transposition different on vmx/sse
struct Float33
{
	union 
	{
        #if defined( _VMX128_ )
        struct { F32 cx_x, cx_y, cx_z; // Columns
                 F32 cy_x, cy_y, cy_z;
                 F32 cz_x, cz_y, cz_z; };
        struct { struct { F32 x, y, z; } cx; // Columns
                 struct { F32 x, y, z; } cy;
                 struct { F32 x, y, z; } cz; };
        #else
        struct { F32 rx_x, rx_y, rx_z; // Rows
                 F32 ry_x, ry_y, ry_z;
                 F32 rz_x, rz_y, rz_z; };
        struct { struct { F32 x, y, z; } rx; // Rows
                 struct { F32 x, y, z; } ry;
                 struct { F32 x, y, z; } rz; };
        #endif
	    F32 v[9];
	};
};

// 4x3 floats, temporary storage of matrix. transposition different on vmx/sse
struct Float43
{
	union 
	{
	    struct
	    {
	        #if defined( _VMX128_ )
		    F32 cx_x, cx_y, cx_z, cx_w; // Columns
		    F32 cy_x, cy_y, cy_z, cy_w;
		    F32 cz_x, cz_y, cz_z, cz_w;
		    #else
		    F32 rx_x, rx_y, rx_z; // Rows
		    F32 ry_x, ry_y, ry_z;
		    F32 rz_x, rz_y, rz_z;
			F32 rw_x, rw_y, rw_z;
		    #endif
	    };
	    struct
	    {
	        #if defined( _VMX128_ )
		    struct { F32 x, y, z, w; } cx; // Columns
		    struct { F32 x, y, z, w; } cy;
		    struct { F32 x, y, z, w; } cz;
		    #else
		    struct { F32 x, y, z; } rx; // Rows
		    struct { F32 x, y, z; } ry;
		    struct { F32 x, y, z; } rz;
			struct { F32 x, y, z; } rw;
		    #endif
	    };
	    F32 v[12];
	};

	/// Write to stream
	template < typename T_Stream >
	FORCE_INLINE void Serialise( T_Stream& stream )
	{
		for (int i = 0; i < lengthof(v); ++i)
			stream << v[i];
	}

	/// Read from stream
	template < typename T_Stream >
	FORCE_INLINE void Deserialise( T_Stream& stream )
	{
		for (int i = 0; i < lengthof(v); ++i)
			stream >> v[i];
	}
};

// 4x4 floats, temporary storage of matrix. transposition different on vmx/sse
struct Float44
{
	union 
	{
	    struct
	    {
	        #if defined( _VMX128_ )
		    F32 cx_x, cx_y, cx_z, cx_w; // Columns
		    F32 cy_x, cy_y, cy_z, cy_w;
		    F32 cz_x, cz_y, cz_z, cz_w;
			F32 cw_x, cw_y, cw_z, cw_w;
		    #else
		    F32 rx_x, rx_y, rx_z, rx_w; // Rows
		    F32 ry_x, ry_y, ry_z, ry_w;
		    F32 rz_x, rz_y, rz_z, rz_w;
			F32 rw_x, rw_y, rw_z, rw_w;
		    #endif
	    };
	    struct
	    {
	        #if defined( _VMX128_ )
		    struct { F32 x, y, z, w; } cx; // Columns
		    struct { F32 x, y, z, w; } cy;
		    struct { F32 x, y, z, w; } cz;
		    struct { F32 x, y, z, w; } cw;
		    #else
		    struct { F32 x, y, z, w; } rx; // Rows
		    struct { F32 x, y, z, w; } ry;
		    struct { F32 x, y, z, w; } rz;
			struct { F32 x, y, z, w; } rw;
		    #endif
	    };
	    F32 v[16];
	};
};


// 3x3 matrix, row-major
struct MatrixRow33
{
	union 
	{
	    struct { F32 _11, _12, _13;
		         F32 _21, _22, _23;
		         F32 _31, _32, _33; };
        
        struct { struct { F32 x, y, z; } rx; // Rows
                 struct { F32 x, y, z; } ry;
                 struct { F32 x, y, z; } rz; };
	    
	    F32 v[9];
	};

	/// Write to stream
	template < typename T_Stream >
	FORCE_INLINE void Serialise( T_Stream& stream )
	{
		for(U32 i=0; i<9; ++i)
		{
			stream << v[i];
		}
	}

	/// Read from stream
	template < typename T_Stream >
	FORCE_INLINE void Deserialise( T_Stream& stream )
	{
		for(U32 i=0; i<9; ++i)
		{
			stream >> v[i];
		}
	}
};

// 3x3 matrix, column-major
struct MatrixCol33
{
	union 
	{
	    struct { F32 _11, _21, _31;
		         F32 _12, _22, _32;
		         F32 _13, _23, _33; };
	    
        struct { struct { F32 x, y, z; } cx; // Columns
                 struct { F32 x, y, z; } cy;
                 struct { F32 x, y, z; } cz; };
	    
	    F32 v[9];
	};
};

// 4x3 matrix, row-major
struct MatrixRow43
{
	union 
	{
	    struct
	    {
			F32 _11, _12, _13, _14;
		    F32 _21, _22, _23, _24;
		    F32 _31, _32, _33, _34;
	    };
        struct { struct { F32 x, y, z, w; } rx; // Rows
                 struct { F32 x, y, z, w; } ry;
                 struct { F32 x, y, z, w; } rz; };
	    F32 v[12];
	};
};

// 4x4 matrix, row-major
struct MatrixRow44
{
	union 
	{
	    struct
	    {
			F32 _11, _12, _13, _14;
		    F32 _21, _22, _23, _24;
		    F32 _31, _32, _33, _34;
			F32 _41, _42, _43, _44;
	    };
        struct { struct { F32 x, y, z, w; } rx; // Rows
                 struct { F32 x, y, z, w; } ry;
                 struct { F32 x, y, z, w; } rz;
                 struct { F32 x, y, z, w; } rw; };
	    F32 v[16];
	};
};

// 4x4 matrix, column-major
struct MatrixCol44
{
	union 
	{
	    struct
	    {
			F32 _11, _21, _31, _41;
		    F32 _12, _22, _32, _42;
		    F32 _13, _23, _33, _43;
			F32 _14, _24, _34, _44;
	    };
        struct { struct { F32 x, y, z, w; } cx; // Columns
                 struct { F32 x, y, z, w; } cy;
                 struct { F32 x, y, z, w; } cz;
                 struct { F32 x, y, z, w; } cw; };
	    F32 v[16];
	};
};


typedef MatrixRow33 Matrix33;
typedef MatrixRow44	Matrix44;


struct Box3
{
	Float3 vmin, vmax;
};

// Fixed Point 3D  16bit vector
template < int T_Precision > // Number of binary digits after the point
struct Fixed3_S16
{
	union 
	{
		struct
		{
			S16 x;
			S16 y;
			S16 z;
		};
		S16 v[3];
	};
};

// Fixed Point 4D  16bit vector
template < int T_Precision > // Number of binary digits after the point
struct Fixed4_S16
{
	union 
	{
		struct
		{
			S16 x;
			S16 y;
			S16 z;
			S16 w;
		};
		S16 v[4];
	};
};


//aligned simple vector storage types for uncompressed float data. 

struct STOREALIGN16 Float2A16
{
	Float2A16() : x(0.f), y(0.f) {}
	Float2A16(F32 _x, F32 _y) : x(_x), y(_y) {}

	union 
	{
		struct
		{
			F32 x;
			F32 y;
		};
		F32 v[2];
	};

};

struct STOREALIGN16 Float3A16
{
	Float3A16() : x(0.f), y(0.f), z(0.f) {}
	Float3A16(F32 _x, F32 _y, F32 _z) : x(_x), y(_y), z(_z) {}

	union 
	{
		struct
		{
			F32 x;
			F32 y;
			F32 z;
		};
		F32 v[3];
	};
};

struct STOREALIGN16 Float4A16
{
	Float4A16() : x(0.f), y(0.f), z(0.f), w(0.f) {}
	Float4A16(F32 _x, F32 _y, F32 _z, F32 _w) : x(_x), y(_y), z(_z), w(_w) {}

	union 
	{
		struct
		{
			F32 x;
			F32 y;
			F32 z;
			F32 w;
		};
		F32 v[4];
	};
};

// aligned 4x4 floats, temporary storage of matrix. transposition different on vmx/sse
struct STOREALIGN16 Float44A16
{
	union 
	{
	    struct
	    {
	        #if defined( _VMX128_ )
		    F32 cx_x, cx_y, cx_z, cx_w; // Columns
		    F32 cy_x, cy_y, cy_z, cy_w;
		    F32 cz_x, cz_y, cz_z, cz_w;
			F32 cw_x, cw_y, cw_z, cw_w;
		    #else
		    F32 rx_x, rx_y, rx_z, rx_w; // Rows
		    F32 ry_x, ry_y, ry_z, ry_w;
		    F32 rz_x, rz_y, rz_z, rz_w;
			F32 rw_x, rw_y, rw_z, rw_w;
		    #endif
	    };
	    F32 v[16];
	};
};

// aligned 4x4 matrix, row-major
struct STOREALIGN16 MatrixRow44A16
{
	union 
	{
        struct { struct { F32 x, y, z, w; } rx; // Rows
                 struct { F32 x, y, z, w; } ry;
                 struct { F32 x, y, z, w; } rz;
                 struct { F32 x, y, z, w; } rw; };
	    struct
	    {
			F32 _11, _12, _13, _14;
		    F32 _21, _22, _23, _24;
		    F32 _31, _32, _33, _34;
			F32 _41, _42, _43, _44;
	    };
	    F32 v[16];
	};
};

// aligned 4x4 matrix, column-major
struct STOREALIGN16 MatrixCol44A16
{
	union 
	{
        struct { struct { F32 x, y, z, w; } cx; // Columns
                 struct { F32 x, y, z, w; } cy;
                 struct { F32 x, y, z, w; } cz;
                 struct { F32 x, y, z, w; } cw; };
	    struct
	    {
			F32 _11, _21, _31, _41;
		    F32 _12, _22, _32, _42;
		    F32 _13, _23, _33, _43;
			F32 _14, _24, _34, _44;
	    };
	    F32 v[16];
	};
};

typedef MatrixRow44A16	Matrix44A16;

// 3D Vector; 11-11-10 bit normalized components packed into a 32 bit integer
// The normalized 3D Vector is packed into 32 bits as follows: a 10 bit signed, 
// normalized integer for the z component and 11 bit signed, normalized 
// integers for the x and y components.  The z component is stored in the 
// most significant bits and the x component in the least significant bits
// (Z10Y11X11): [32] zzzzzzzz zzyyyyyy yyyyyxxx xxxxxxxx [0]
typedef struct _HenDN3
{
    union
    {
        struct
        {
            S32  x   : 11;    // -1023/1023 to 1023/1023
            S32  y   : 11;    // -1023/1023 to 1023/1023
            S32  z   : 10;    // -511/511 to 511/511
        };
        U32 v;
    };

} HenDN3;

// 3D Vector; 11-11-10 bit components packed into a 32 bit integer
// The 3D Vector is packed into 32 bits as follows: a 10 bit signed, 
// integer for the z component and 11 bit signed integers for the 
// x and y components.  The z component is stored in the 
// most significant bits and the x component in the least significant bits
// (Z10Y11X11): [32] zzzzzzzz zzyyyyyy yyyyyxxx xxxxxxxx [0]
typedef struct _HenD3
{
    union
    {
        struct
        {
            S32  x   : 11;    // -1023 to 1023
            S32  y   : 11;    // -1023 to 1023
            S32  z   : 10;    // -511 to 511
        };
        U32 v;
    };

} HenD3;

// 3D Vector; 11-11-10 bit normalized components packed into a 32 bit integer
// The normalized 3D Vector is packed into 32 bits as follows: a 10 bit unsigned, 
// normalized integer for the z component and 11 bit unsigned, normalized 
// integers for the x and y components.  The z component is stored in the 
// most significant bits and the x component in the least significant bits
// (Z10Y11X11): [32] zzzzzzzz zzyyyyyy yyyyyxxx xxxxxxxx [0]
typedef struct _UHenDN3
{
    union
    {
        struct
        {
            U32  x  : 11;    // 0/2047 to 2047/2047
            U32  y  : 11;    // 0/2047 to 2047/2047
            U32  z  : 10;    // 0/1023 to 1023/1023
        };
        U32 v;
    };

} UHenDN3;

// 3D Vector; 11-11-10 bit components packed into a 32 bit integer
// The 3D Vector is packed into 32 bits as follows: a 10 bit unsigned
// integer for the z component and 11 bit unsigned integers 
// for the x and y components.  The z component is stored in the 
// most significant bits and the x component in the least significant bits
// (Z10Y11X11): [32] zzzzzzzz zzyyyyyy yyyyyxxx xxxxxxxx [0]
typedef struct _UHenD3
{
    union
    {
        struct
        {
            U32  x  : 11;    // 0 to 2047
            U32  y  : 11;    // 0 to 2047
            U32  z  : 10;    // 0 to 1023
        };
        U32 v;
    };

} UHenD3;

// 3D Vector; 10-11-11 bit normalized components packed into a 32 bit integer
// The normalized 3D Vector is packed into 32 bits as follows: a 10 bit signed, 
// normalized integer for the x component and 11 bit signed, normalized 
// integers for the y and z components.  The z component is stored in the 
// most significant bits and the x component in the least significant bits
// (Z11Y11X10): [32] zzzzzzzz zzzyyyyy yyyyyyxx xxxxxxxx [0]
typedef struct _DHenN3
{
    union
    {
        struct
        {
            S32  x   : 10;    // -511/511 to 511/511
            S32  y   : 11;    // -1023/1023 to 1023/1023
            S32  z   : 11;    // -1023/1023 to 1023/1023
        };
        U32 v;
    };

} DHenN3;

// 3D Vector; 10-11-11 bit components packed into a 32 bit integer
// The 3D Vector is packed into 32 bits as follows: a 10 bit signed, 
// integer for the x component and 11 bit signed integers for the 
// y and z components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (Z11Y11X10): [32] zzzzzzzz zzzyyyyy yyyyyyxx xxxxxxxx [0]
typedef struct _DHen3
{
    union
    {
        struct
        {
            S32  x   : 10;    // -511 to 511
            S32  y   : 11;    // -1023 to 1023
            S32  z   : 11;    // -1023 to 1023
        };
        U32 v;
    };

} DHen3;

// 3D Vector; 10-11-11 bit normalized components packed into a 32 bit integer
// The normalized 3D Vector is packed into 32 bits as follows: a 10 bit unsigned, 
// normalized integer for the x component and 11 bit unsigned, normalized 
// integers for the y and z components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (Z11Y11X10): [32] zzzzzzzz zzzyyyyy yyyyyyxx xxxxxxxx [0]
typedef struct _UDHenN3
{
    union
    {
        struct
        {
            U32  x  : 10;    // 0/1023 to 1023/1023
            U32  y  : 11;    // 0/2047 to 2047/2047
            U32  z  : 11;    // 0/2047 to 2047/2047
        };
        U32 v;
    };

} UDHenN3;

// 3D Vector; 10-11-11 bit components packed into a 32 bit integer
// The 3D Vector is packed into 32 bits as follows: a 10 bit unsigned, 
// integer for the x component and 11 bit unsigned integers 
// for the y and z components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (Z11Y11X10): [32] zzzzzzzz zzzyyyyy yyyyyyxx xxxxxxxx [0]
typedef struct _UDHen3
{
    union
    {
        struct
        {
            U32  x  : 10;    // 0 to 1023
            U32  y  : 11;    // 0 to 2047
            U32  z  : 11;    // 0 to 2047
        };
        U32 v;
    };

} UDHen3;


// 4D Vector; 10-10-10-2 bit normalized components packed into a 32 bit integer
// The normalized 4D Vector is packed into 32 bits as follows: a 2 bit unsigned, 
// normalized integer for the w component and 10 bit signed, normalized 
// integers for the z, y, and x components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (W2Z10Y10X10): [32] wwzzzzzz zzzzyyyy yyyyyyxx xxxxxxxx [0]
typedef struct _XDecN4
{
    union
    {
        struct
        {
            S32  x   : 10;    // -511/511 to 511/511
            S32  y   : 10;    // -511/511 to 511/511
            S32  z   : 10;    // -511/511 to 511/511
            U32 w   : 2;     //      0/3 to     3/3
        };
        U32 v;
    };

} XDecN4;

// 4D Vector; 10-10-10-2 bit components packed into a 32 bit integer
// The normalized 4D Vector is packed into 32 bits as follows: a 2 bit unsigned
// integer for the w component and 10 bit signed integers for the 
// z, y, and x components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (W2Z10Y10X10): [32] wwzzzzzz zzzzyyyy yyyyyyxx xxxxxxxx [0]
typedef struct _XDec4
{
    union
    {
        struct
        {
            S32  x   : 10;    // -511 to 511
            S32  y   : 10;    // -511 to 511
            S32  z   : 10;    // -511 to 511
            U32 w   : 2;     //    0 to   3
        };
        U32 v;
    };

} XDec4;

// 4D Vector; 10-10-10-2 bit normalized components packed into a 32 bit integer
// The normalized 4D Vector is packed into 32 bits as follows: a 2 bit signed, 
// normalized integer for the w component and 10 bit signed, normalized 
// integers for the z, y, and x components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (W2Z10Y10X10): [32] wwzzzzzz zzzzyyyy yyyyyyxx xxxxxxxx [0]
typedef struct _DecN4
{
    union
    {
        struct
        {
            S32  x   : 10;    // -511/511 to 511/511
            S32  y   : 10;    // -511/511 to 511/511
            S32  z   : 10;    // -511/511 to 511/511
            S32  w   : 2;     //     -1/1 to     1/1
        };
        U32 v;
    };

} DecN4;

// 4D Vector; 10-10-10-2 bit components packed into a 32 bit integer
// The 4D Vector is packed into 32 bits as follows: a 2 bit signed, 
// integer for the w component and 10 bit signed integers for the 
// z, y, and x components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (W2Z10Y10X10): [32] wwzzzzzz zzzzyyyy yyyyyyxx xxxxxxxx [0]
typedef struct _Dec4
{
    union
    {
        struct
        {
            S32  x   : 10;    // -511 to 511
            S32  y   : 10;    // -511 to 511
            S32  z   : 10;    // -511 to 511
            S32  w   : 2;     //   -1 to   1
        };
        U32 v;
    };

} Dec4;

// 4D Vector; 10-10-10-2 bit normalized components packed into a 32 bit integer
// The normalized 4D Vector is packed into 32 bits as follows: a 2 bit unsigned, 
// normalized integer for the w component and 10 bit unsigned, normalized 
// integers for the z, y, and x components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (W2Z10Y10X10): [32] wwzzzzzz zzzzyyyy yyyyyyxx xxxxxxxx [0]
typedef struct _UDecN4
{
    union
    {
        struct
        {
            U32  x  : 10;    // 0/1023 to 1023/1023
            U32  y  : 10;    // 0/1023 to 1023/1023
            U32  z  : 10;    // 0/1023 to 1023/1023
            U32  w  : 2;     //    0/3 to       3/3
        };
        U32 v;
    };

} UDecN4;

// 4D Vector; 10-10-10-2 bit components packed into a 32 bit integer
// The 4D Vector is packed into 32 bits as follows: a 2 bit unsigned, 
// integer for the w component and 10 bit unsigned integers 
// for the z, y, and x components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (W2Z10Y10X10): [32] wwzzzzzz zzzzyyyy yyyyyyxx xxxxxxxx [0]
typedef struct _UDec4
{
    union
    {
        struct
        {
            U32  x  : 10;    // 0 to 1023
            U32  y  : 10;    // 0 to 1023
            U32  z  : 10;    // 0 to 1023
            U32  w  : 2;     // 0 to    3
        };
        U32 v;
    };

} UDec4;

// 4D Vector; 20-20-20-4 bit normalized components packed into a 64 bit integer
// The normalized 4D Vector is packed into 64 bits as follows: a 4 bit unsigned, 
// normalized integer for the w component and 20 bit signed, normalized 
// integers for the z, y, and x components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (W4Z20Y20X20): [64] wwwwzzzz zzzzzzzz zzzzzzzz yyyyyyyy yyyyyyyy yyyyxxxx xxxxxxxx xxxxxxxx [0]
typedef struct _XIcoN4
{
    union
    {
        struct
        {
            S64  x   : 20;    // -524287/524287 to 524287/524287
            S64  y   : 20;    // -524287/524287 to 524287/524287
            S64  z   : 20;    // -524287/524287 to 524287/524287
            U64 w   : 4;     //           0/15 to         15/15
        };
        U64 v;
    };

} XIcoN4;

// 4D Vector; 20-20-20-4 bit components packed into a 64 bit integer
// The 4D Vector is packed into 64 bits as follows: a 4 bit unsigned
// integer for the w component and 20 bit signed integers for the 
// z, y, and x components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (W4Z20Y20X20): [64] wwwwzzzz zzzzzzzz zzzzzzzz yyyyyyyy yyyyyyyy yyyyxxxx xxxxxxxx xxxxxxxx [0]
typedef struct _XIco4
{
    union
    {
        struct
        {
            S64  x   : 20;    // -524287 to 524287
            S64  y   : 20;    // -524287 to 524287
            S64  z   : 20;    // -524287 to 524287
            U64 w   : 4;     //       0 to     15
        };
        U64 v;
    };

} XIco4;

// 4D Vector; 20-20-20-4 bit normalized components packed into a 64 bit integer
// The normalized 4D Vector is packed into 64 bits as follows: a 4 bit signed, 
// normalized integer for the w component and 20 bit signed, normalized 
// integers for the z, y, and x components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (W4Z20Y20X20): [64] wwwwzzzz zzzzzzzz zzzzzzzz yyyyyyyy yyyyyyyy yyyyxxxx xxxxxxxx xxxxxxxx [0]
typedef struct _IcoN4
{
    union
    {
        struct
        {
            S64  x   : 20;    // -524287/524287 to 524287/524287
            S64  y   : 20;    // -524287/524287 to 524287/524287
            S64  z   : 20;    // -524287/524287 to 524287/524287
            S64  w   : 4;     //           -7/7 to           7/7
        };
        U64 v;
    };

} IcoN4;

// 4D Vector; 20-20-20-4 bit components packed into a 64 bit integer
// The 4D Vector is packed into 64 bits as follows: a 4 bit signed, 
// integer for the w component and 20 bit signed integers for the 
// z, y, and x components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (W4Z20Y20X20): [64] wwwwzzzz zzzzzzzz zzzzzzzz yyyyyyyy yyyyyyyy yyyyxxxx xxxxxxxx xxxxxxxx [0]
typedef struct _Ico4
{
    union
    {
        struct
        {
            S64  x   : 20;    // -524287 to 524287
            S64  y   : 20;    // -524287 to 524287
            S64  z   : 20;    // -524287 to 524287
            S64  w   : 4;     //      -7 to      7
        };
        U64 v;
    };

} Ico4;

// 4D Vector; 20-20-20-4 bit normalized components packed into a 64 bit integer
// The normalized 4D Vector is packed into 64 bits as follows: a 4 bit unsigned, 
// normalized integer for the w component and 20 bit unsigned, normalized 
// integers for the z, y, and x components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (W4Z20Y20X20): [64] wwwwzzzz zzzzzzzz zzzzzzzz yyyyyyyy yyyyyyyy yyyyxxxx xxxxxxxx xxxxxxxx [0]
typedef struct _UIcoN4
{
    union
    {
        struct
        {
            U64  x  : 20;    // 0/1048575 to 1048575/1048575
            U64  y  : 20;    // 0/1048575 to 1048575/1048575
            U64  z  : 20;    // 0/1048575 to 1048575/1048575
            U64  w  : 4;     //      0/15 to           15/15
        };
        U64 v;
    };

} UIcoN4;

// 4D Vector; 20-20-20-4 bit components packed into a 64 bit integer
// The 4D Vector is packed into 64 bits as follows: a 4 bit unsigned 
// integer for the w component and 20 bit unsigned integers for the 
// z, y, and x components.  The w component is stored in the 
// most significant bits and the x component in the least significant bits
// (W4Z20Y20X20): [64] wwwwzzzz zzzzzzzz zzzzzzzz yyyyyyyy yyyyyyyy yyyyxxxx xxxxxxxx xxxxxxxx [0]
typedef struct _UIco4
{
    union
    {
        struct
        {
            U64  x  : 20;    // 0 to 1048575
            U64  y  : 20;    // 0 to 1048575
            U64  z  : 20;    // 0 to 1048575
            U64  w  : 4;     // 0 to      15
        };
        U64 v;
    };

} UIco4;

// ARGB Color; 8-8-8-8 bit unsigned normalized integer components packed into
// a 32 bit integer.  The normalized color is packed into 32 bits using 8 bit
// unsigned, normalized integers for the alpha, red, green, and blue components.
// The alpha component is stored in the most significant bits and the blue
// component in the least significant bits (A8R8G8B8):
// [32] aaaaaaaa rrrrrrrr gggggggg bbbbbbbb [0]
typedef struct _Color
{
    union
    {
        struct
        {
            U32 b    : 8;  // Blue:    0/255 to 255/255
            U32 g    : 8;  // Green:   0/255 to 255/255
            U32 r    : 8;  // Red:     0/255 to 255/255
            U32 a    : 8;  // Alpha:   0/255 to 255/255
        };
        U32 c;
    };

} Color;

// 4D Vector; 8 bit signed normalized integer components
typedef struct _ByteN4
{
    union
    {
        struct
        {
            S8 x;
            S8 y;
            S8 z;
            S8 w;
        };
        U32 v;
    };

} ByteN4;

// 4D Vector; 8 bit signed integer components
typedef struct _Byte4
{
    union
    {
        struct
        {
            S8 x;
            S8 y;
            S8 z;
            S8 w;
        };
        U32 v;
    };

} Byte4;

// 4D Vector; 8 bit unsigned normalized integer components
typedef struct _UByteN4
{
    union
    {
        struct
        {
            U8 x;
            U8 y;
            U8 z;
            U8 w;
        };
        U32 v;
    };

} UByteN4;

// 4D Vector; 8 bit unsigned integer components
typedef struct _UByte4
{
    union
    {
        struct
        {
            U8 x;
            U8 y;
            U8 z;
            U8 w;
        };
        U32 v;
    };

} UByte4;


/// F16 (half precision floating point)
struct F16
{
#if defined( PLATFORM_PC )
    // [Blur2PC] MSVC on x86 allocates bitfields from the low bit, so the console order below put the sign in bit 0
    // and decoded every half wrongly on PC (light sample positions came out as garbage, so cars got no SH light).
    // Retail Blur.exe 1.2 decodes IEEE halves with sign = bit 15, exponent = bits 10-14 (C_BSPPointTree::GetClosestSample
    // 0x0072a4d0); this is that layout.
    U16 mantissa : 10;
    U16 exponent : 5;
    U16 sign     : 1;
#else
    U16 sign     : 1;
    U16 exponent : 5;
    U16 mantissa : 10;
#endif
};

/// 4 Element Vector of F16 (half precision floating point) components
struct Vec4F16
{
    F16 x;
    F16 y;
    F16 z;
    F16 w;
};

#if defined( COMPILER_VC )
#pragma warning( pop )
#endif

} // namespace MathsStorage

#endif // #if defined( INCLUDE_THIS )


