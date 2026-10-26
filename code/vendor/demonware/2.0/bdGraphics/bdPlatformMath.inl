// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <stdlib.h>
#include <math.h>

inline 
bdInt BD_CALL bdAbsi(const bdInt x)
{
	return ::abs(x);
}

inline 
bdFloat32 BD_CALL bdFabsf32(const bdFloat32 x)
{
	return ::fabsf(x);
}

inline 
bdFloat32 BD_CALL bdPowf32(const bdFloat32 x,
                           const bdFloat32 y)
{
	return ::powf(x,y);
}

inline 
bdFloat32 BD_CALL bdSqrtf32(const bdFloat32 x)
{
	return ::sqrtf(x);
}

inline 
bdFloat32 BD_CALL bdSinf32(const bdFloat32 x)
{
	return ::sinf(x);
}

inline 
bdFloat32 BD_CALL bdCosf32(const bdFloat32 x)
{
	return ::cosf(x);
}

inline 
bdFloat32 BD_CALL bdTanf32(const bdFloat32 x)
{
	return ::tanf(x);
}

inline 
bdFloat32 BD_CALL bdAtanf32(const bdFloat32 x)
{
	return ::atanf(x);
}
