/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.4.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_PRINTF_H__
#define __EDGE_PRINTF_H__

//////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) | defined(__PPU__)

	#include <stdio.h>
	#define EDGE_PRINTF		printf

#elif defined(__SPU__)

	#include <spu_printf.h>
	#define EDGE_PRINTF		spu_printf

#else

	#error EDGE_PRINTF not defined

#endif

//////////////////////////////////////////////////////////////////////////

#endif	//__EDGE_PRINTF_H__
