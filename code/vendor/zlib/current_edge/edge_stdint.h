/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.4.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */
#ifndef EDGE_STDINT_H
#define EDGE_STDINT_H

/* 
 * Workaround for PC tools. MSVC is still not C99 compliant.
 */

#if defined(_MSC_VER)

	// gcm_tool.h simply doesn't compile under Visual Studio 2003 and earlier,
	// so we include the relevant typedefs directly. 
	typedef signed char		int8_t;
	typedef signed short	int16_t;
	typedef signed int		int32_t;
	typedef signed long long	int64_t;
	typedef unsigned char	uint8_t;
	typedef unsigned short	uint16_t;
	typedef unsigned int	uint32_t;
	typedef unsigned long long	uint64_t;

	typedef int32_t			intptr_t;
	typedef uint32_t		uintptr_t;

#else	//PPU, SPU, other?

	#include <stdint.h>

#endif

#endif // EDGE_STDINT_H
