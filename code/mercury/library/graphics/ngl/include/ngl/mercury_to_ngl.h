#ifndef __FX_EXPORT_MERCURY_TO_NGL_H__
#define __FX_EXPORT_MERCURY_TO_NGL_H__

typedef Util::C_String apkString;
#define TL_INLINE inline
#define NGL_EXTERN extern
#define NGL_API
#define tl_restrict
namespace math
{
	typedef Maths::HMat43 RotTranMat43;
	typedef Maths::HVec4 Vector4;
	typedef Maths::HVec3 Dir3;
	typedef Maths::HMat44 Mat44;
	typedef Maths::HVec4 Vector4Arg;
	typedef Maths::HMat44 Mat44Arg;
};

// Flags for the memory allocation functions.
enum
{
	TL_MEMALLOC_NGL          = 0x01000000,
	TL_MEMALLOC_NAL          = 0x02000000,
	TL_MEMALLOC_AEPS         = 0x03000000,
	TL_MEMALLOC_NVL          = 0x04000000,
	TL_MEMALLOC_PHYSICS      = 0x05000000,
	TL_MEMALLOC_LIBMASK      = 0xFF000000,

	TL_MEMALLOC_TEMP         = 0x00000001,
	TL_MEMALLOC_CANFAIL      = 0x00000002,

	TL_MEMALLOC_PHYSICAL     = 0x00010000, // GC, Xenon and Xbox only
	TL_MEMALLOC_WRITECOMBINE = 0x00020000, // Xenon and Xbox only
	TL_MEMALLOC_FIFO         = 0x00040000, // GC, Revolution only


#if TL_PLAT_PS3
	TL_MEMALLOC_RSXMAIN      = 0x00080000, // PS3 only, rsx-mapped main memory
	TL_MEMALLOC_RSXLOCAL     = 0x00100000, // PS3 only, rsx-mapped local memory
#else
	TL_MEMALLOC_RSXMAIN      = 0,
	TL_MEMALLOC_RSXLOCAL     = 0,
#endif

#if TL_PLAT_REV
	TL_MEMALLOC_MEM1		 = 0x00080000, // Revolution only
	TL_MEMALLOC_MEM2		 = 0x00100000, // Revolution only
#else
	TL_MEMALLOC_MEM1		 = 0,
	TL_MEMALLOC_MEM2		 = 0,
#endif
};

#endif
