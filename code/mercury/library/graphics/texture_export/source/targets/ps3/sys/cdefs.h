/*   SCE CONFIDENTIAL                                       */
/*   PLAYSTATION(R)3 Programmer Tool Runtime Library 091.006 */
/*   Copyright (C) 2006 Sony Computer Entertainment Inc.    */
/*   All Rights Reserved.                                   */

#ifndef __SYS_SYS_CDEFS_H__
#define __SYS_SYS_CDEFS_H__

#if defined(__cplusplus)
#define							CDECL_BEGIN   extern "C" {
#define							CDECL_END }
#define NAMESPACE_LV2_BEGIN	namespace lv2 {
#define NAMESPACE_LV2_END	}
#else /* defined(__cplusplus) */
#define							CDECL_BEGIN
#define							CDECL_END
#define NAMESPACE_LV2_BEGIN
#define NAMESPACE_LV2_END
#endif /* defined(__cplusplus) */

#if !defined(__LV2_KERNEL__) && defined(__cplusplus)
#define __C_STD_BEGIN	namespace std {
#define __C_STD_END	}
#define __CSTD		::std::
#define __STD_USING
#else /* !defined(__LV2_KERNEL__) && defined(__cplusplus) */
#define __C_STD_BEGIN
#define __C_STD_END
#define __CSTD
#undef __STD_USING
#endif /* !defined(__LV2_KERNEL__) && defined(__cplusplus) */

#ifdef __LV2_KERNEL__
#define branch_predict_true(x) __builtin_expect(!!(x), 1)
#define branch_predict_false(x) __builtin_expect(!!(x), 0)
#endif

#endif // __SYS_SYS_CDEFS_H__
