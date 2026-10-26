#ifdef _MSC_VER_
#pragma once
#endif

#ifndef ATTK_DEFINITIONS_H
#define ATTK_DEFINITIONS_H

#define ATTK_VERSION 100
#ifndef ATTK_ASSERT
	#if _DEBUG && _MSC_VER_
	#define ATTK_ASSERT(x) if (!(x)) _asm int 3
	#else
	#define ATTK_ASSERT(x) assert(x)
	#endif
#endif


#endif
