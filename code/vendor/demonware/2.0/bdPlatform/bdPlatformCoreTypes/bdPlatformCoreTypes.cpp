// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>


// Check the size of the fixed width types
#include <bdPlatform/bdPlatformError/bdPlatformError.h>

BD_COMPILE_ASSERT( sizeof(bdNChar8)		== 1, bdNChar8_is_unexpected_size);
BD_COMPILE_ASSERT( sizeof(bdByte8)		== 1, bdByte8_is_unexpected_size);
BD_COMPILE_ASSERT( sizeof(bdUByte8)		== 1, bdUByte8_is_unexpected_size);
BD_COMPILE_ASSERT( sizeof(bdInt16)		== 2, bdInt16_is_unexpected_size);
BD_COMPILE_ASSERT( sizeof(bdUInt16)		== 2, bdUInt16_is_unexpected_size);
BD_COMPILE_ASSERT( sizeof(bdInt32)		== 4, bdInt32_is_unexpected_size);
BD_COMPILE_ASSERT( sizeof(bdUInt32)		== 4, bdUInt32_is_unexpected_size);
BD_COMPILE_ASSERT( sizeof(bdInt64)		== 8, bdInt64_is_unexpected_size);
BD_COMPILE_ASSERT( sizeof(bdUInt64)		== 8, bdUInt64_is_unexpected_size);
BD_COMPILE_ASSERT( sizeof(bdLong64)		== 8, bdLong64_is_unexpected_size);
BD_COMPILE_ASSERT( sizeof(bdULong64)	== 8, bdULong64_is_unexpected_size);
BD_COMPILE_ASSERT( sizeof(bdFloat32)	== 4, bdFloat32_is_unexpected_size);
BD_COMPILE_ASSERT( sizeof(bdFloat64)	== 8, bdFloat64_is_unexpected_size);

