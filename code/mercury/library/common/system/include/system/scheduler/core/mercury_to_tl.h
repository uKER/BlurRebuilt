#ifndef __MERCURY_TO_TL_H__
#define __MERCURY_TO_TL_H__

#if defined( PLATFORM_SPU )
#define TL_PLAT_PS3_SPU 1
#elif defined( PLATFORM_PS3 )
#define TL_PLAT_PS3 1
#elif defined( PLATFORM_PC )
#define TL_PLAT_PC 1
#endif

#endif
