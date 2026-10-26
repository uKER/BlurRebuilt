/* SCE CONFIDENTIAL
PLAYSTATION(R)3 Programmer Tool Runtime Library 084.001
* Copyright (C) 2005 Sony Computer Entertainment Inc. 
* All Rights Reserved.
*/
/* stdbool.h standard header */
#ifndef _STDBOOL
#define _STDBOOL
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

_C_STD_BEGIN

 #define __bool_true_false_are_defined  1

 #ifndef __cplusplus
                /* TYPES */

 #if 199901L > __STDC_VERSION__ && !defined(__GNUC__) 
 /* C99 requires _Bool and as of version 3, GCC always has it.  For
    anything else, we provide a typedef. */
typedef unsigned int _Bool;
 #endif 

                /* MACROS */
 #define bool   _Bool
 #define false  0
 #define true   1
 #endif /* __cplusplus */

_C_STD_END
#endif /* _STDBOOL */

/*
 * Copyright (c) 1992-2003 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V4.02:0216 */
