// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_PLATFORM_CORE_TYPES_H
#define BD_PLATFORM_CORE_TYPES_H

#include <bdPlatform/bdPlatformConfig/bdPlatformConfig.h>

//
// Native types
//

/// Native int type, size defined by platform.
typedef int				bdInt;

/// Native unsigned int type, size defined by platform.
typedef unsigned int	bdUInt;

/// Native long type, size defined by platform.
typedef BD_LONG			bdLong;

/// Native unsigned long type, size defined by platform.
typedef BD_ULONG		bdULong;

/// Native platform word size type, size defined by platform.
typedef BD_WORD			bdWord;

/// Native platform unsigned word size type, size defined by platform.
typedef BD_UWORD		bdUWord;

/// Native platform boolean type, size defined by platform.
typedef bool			bdBool;

//
// Fixed width types
//

/// Default platform char type.
typedef BD_PLATFORM_CHAR bdNChar8;

/// bdInt16 (16 bits), defines a small signed integer type.
typedef short			bdInt16;

/// bdUInt16 (16 bits), defines a small unsigned integer type.
typedef unsigned short	bdUInt16;

/// bdInt32 (32 bits), defines a signed integer type.
typedef int				bdInt32;

/// bdUInt32 (32 bits), defines an unsigned integer type.
typedef unsigned int	bdUInt32;

/// bdInt64 (64 bits), defines a large signed integer type.
typedef BD_INT_64		bdInt64;

/// bdUInt64 (64 bits), defines a large unsigned integer type.
typedef BD_UINT_64		bdUInt64;

/// bdLong64 (64 bits), defines a large signed long type.
typedef BD_LONG_64		bdLong64;

///bdULong64 (64 bits), defines a large unsigned long type.
typedef BD_ULONG_64		bdULong64;

/// bdFloat32 (32 bits), defines a generic floating point type.
/// This should be used as the default floating point type.
typedef float			bdFloat32;

/// bdFloat64 (64 bits), a larger floating point type than bdFloat32.
/// Should only be used when bdFloat32 is not big enough.
typedef double			bdFloat64;

//
// Misc types
//

/// bdByte8 (8 bits), defines a generic signed byte type.
typedef BD_CHAR8 bdByte8;

/// bdByte8 (8 bits), defines a generic unsigned byte type.
typedef BD_UCHAR8 bdUByte8;

typedef bdUInt16 bdSeqNumber;

#define BD_INITIAL_SEQ_NUMBER (0)

/// The maximum valid sequence number.
#define BD_MAX_SEQ_NUMBER  (bdSeqNumber(0xffff))

enum bdPriority
{
	BD_PRIORITY_VERY_LOW,
	BD_PRIORITY_LOW,
	BD_PRIORITY_NORMAL,
	BD_PRIORITY_HIGH,
	BD_PRIORITY_VERY_HIGH,
	BD_NUM_PRIORITIES
};

typedef void* bdFileHandle;

//
// Defines
//

/// Defines the null value used in all code.
#define BD_NULL (0)

//
// Minimum and maximum fixed width integer values
//

#define BD_UBYTE8_MAX (255U)
#define BD_UBYTE8_MIN (0)

#define BD_BYTE8_MAX (127)
#define BD_BYTE8_MIN (-BD_BYTE8_MAX - 1)

#define BD_INT16_MAX (32767)
#define BD_INT16_MIN (-BD_INT16_MAX - 1) // (-32768)

#define BD_UINT16_MAX (65535U)
#define BD_UINT16_MIN (0)

#define BD_INT32_MAX (2147483647L)
#define BD_INT32_MIN (-BD_INT32_MAX - 1) // (-2147483648)

#define BD_UINT32_MAX (4294967295UL)
#define BD_UINT32_MIN (0)

#define BD_INT64_MAX (9223372036854775807LL)
#define BD_INT64_MIN (-BD_INT64_MAX - 1) // (-9223372036854775808)

#define BD_UINT64_MAX (18446744073709551615ULL)
#define BD_UINT64_MIN (0)


//
// Max digits for integer and floating point values
//

/// The maximum number of digits an 8 bit integer number can have.
/// This is very conservative, it does not include leading zeros,
/// but does include a sign character and null terminator.
#define BD_MAX_CHAR8_DIGITS (8)

/// The maximum number of digits a 16 bit integer number can have.
/// This is very conservative, it does not include leading zeros,
/// but does include a sign character and null terminator.
#define BD_MAX_INT16_DIGITS (16)

/// The maximum number of digits a 32 bit integer number can have.
/// This is very conservative, it does not include leading zeros,
/// but does include a sign character and null terminator.
#define BD_MAX_INT32_DIGITS (32)

/// The maximum number of digits a 64 bit integer number can have.
/// This is very conservative, it does not include leading zeros,
/// but does include a sign character and null terminator.
#define BD_MAX_INT64_DIGITS (64)

/// The maximum number of digits a 32 bit floating point  number can have.
/// This is very conservative, it does not include leading zeros,
/// but does include a sign character and null terminator, and exponential
/// characters.
#define BD_MAX_FLOAT32_DIGITS (32 + 36 + 32)

/// The maximum number of digits a 64 bit floating point  number can have.
/// This is very conservative, it does not include leading zeros,
/// but does include a sign character and null terminator, and exponential
/// characters.
#define BD_MAX_FLOAT64_DIGITS ((32 + 36 + 32) * 2)

/// A macro to find the min of 2 values.
#define BD_MIN(x, y) (((x)<(y))?(x):(y))
/// A macro to find the max of 2 values.
#define BD_MAX(x, y) (((x)>(y))?(x):(y))
/// A macro to find the min of 3 values.
#define BD_MIN3(x, y, z) (BD_MIN((x),(BD_MIN((y),(z)))))
/// A macro to find the max of 3 values.
#define BD_MAX3(x, y, z) (BD_MAX((x),(BD_MAX((y),(z)))))
/// A macro to find the min of 4 values.
#define BD_MIN4(w, x, y, z) (BD_MIN((BD_MIN((w),(x))),(BD_MIN((y),(z)))))
/// A macro to find the max of 4 values.
#define BD_MAX4(w, x, y, z) (BD_MAX((BD_MAX((w),(x))),(BD_MAX((y),(z)))))
/// A macro to find the min of 6 values.
#define BD_MIN6(u, v, w, x, y, z) (BD_MIN((BD_MIN3((u),(v),(w))),(BD_MIN3((x),(y),(z)))))
/// A macro to find the max of 6 values.
#define BD_MAX6(u, v, w, x, y, z) (BD_MAX((BD_MAX3((u),(v),(w))),(BD_MAX3((x),(y),(z)))))
/// A macro to clamp the value \a v between a high of \a h and a low of \a l.
#define BD_CLAMP(v, l, h) ((v)>(h)?(h):((v)<(l)?(l):(v)))


#endif // BD_PLATFORM_CORE_TYPES_H
