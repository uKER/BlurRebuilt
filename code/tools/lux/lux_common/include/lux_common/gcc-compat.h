#ifndef _GCC_COMPAT_
#define _GCC_COMPAT_

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifdef GCC

#define _aligned_malloc(X,Y)	malloc(X)
#define _aligned_free(X)		free(X)

#define _finite(X)				finite(X)

#endif

#endif
