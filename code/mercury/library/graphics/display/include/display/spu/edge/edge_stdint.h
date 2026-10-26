/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.3.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */
#ifndef EDGE_STDINT_H
#define EDGE_STDINT_H

/* 
 * Workaround for PC tools. MSVC is still not C99 compliant.
 */

#ifndef _MSC_VER
#include <stdint.h>
#else
typedef unsigned __int8		uint8_t;
typedef unsigned __int16	uint16_t;
typedef unsigned __int32	uint32_t;
typedef unsigned __int64	uint64_t;
typedef __int8				int8_t;
typedef __int16				int16_t;
typedef __int32				int32_t;
typedef __int64				int64_t;
typedef	unsigned __int32	uintptr_t;
typedef	__int32				intptr_t;
#endif

#endif // EDGE_STDINT_H
