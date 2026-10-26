#ifndef GRAPHICS_SPU_MATHS_H
#define GRAPHICS_SPU_MATHS_H

#if _MSC_VER > 1000
#pragma once
#endif

#if defined(PLATFORM_SPU)

FORCE_INLINE qword min_u8(const qword a, const qword b)
{
	return si_selb(a, b, si_clgtb(a, b));
}

FORCE_INLINE qword max_u8(const qword a, const qword b)
{
	return si_selb(b, a, si_clgtb(a, b));
}

FORCE_INLINE qword min4_u8(const qword a, const qword b, const qword c, const qword d)
{
	const qword e = min_u8(a, b);
	const qword f = min_u8(c, d);
	return min_u8(e, f);
}

FORCE_INLINE qword max4_u8(const qword a, const qword b, const qword c, const qword d)
{
	const qword e = max_u8(a, b);
	const qword f = max_u8(c, d);
	return max_u8(e, f);
}

#endif

#endif // GRAPHICS_SPU_MATHS_H