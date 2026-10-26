/*   SCE CONFIDENTIAL                                       */
/*   PLAYSTATION(R)3 Programmer Tool Runtime Library 091.006 */
/*   Copyright (C) 2006 Sony Computer Entertainment Inc.    */
/*   All Rights Reserved.                                   */

#undef CELL_GCM_SET_ALPHA_FUNC_AND_REF
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_ALPHA_FUNC_AND_REF(cmd, af, ref) do { \
	CELL_GCM_UNUSED(af); \
	CELL_GCM_UNUSED(ref); \
	} while(false)
#else
#define CELL_GCM_SET_ALPHA_FUNC_AND_REF(cmd, af, ref) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_ALPHA_FUNCTION, 2); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(af); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(ref); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_SET_BLEND_COLOR
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_BLEND_COLOR(cmd, color) do { \
	CELL_GCM_UNUSED(color); \
	} while(false)
#else
#define CELL_GCM_SET_BLEND_COLOR(cmd, color) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_BLEND_COLOR, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(color); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_BLEND_COLOR_2
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_BLEND_COLOR_2(cmd, color) do { \
	CELL_GCM_UNUSED(color); \
	} while(false)
#else
#define CELL_GCM_SET_BLEND_COLOR_2(cmd, color) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_FLOATING_POINT_BLEND_COLOR, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(color); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_BLEND_EQUATION
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_BLEND_EQUATION(cmd, color, alpha) do { \
	CELL_GCM_UNUSED(color); \
	CELL_GCM_UNUSED(alpha); \
	} while(false)
#else
#define CELL_GCM_SET_BLEND_EQUATION(cmd, color, alpha) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_BLEND_EQUATION, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((color) | ((alpha) << 16)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_BLEND_FUNC_SRC_AND_DST
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_BLEND_FUNC_SRC_AND_DST(cmd, sfcolor, sfalpha, dfcolor, dfalpha) do { \
	CELL_GCM_UNUSED(sfcolor); \
	CELL_GCM_UNUSED(sfalpha); \
	CELL_GCM_UNUSED(dfcolor); \
	CELL_GCM_UNUSED(dfalpha); \
	} while(false)
#else
#define CELL_GCM_SET_BLEND_FUNC_SRC_AND_DST(cmd, sfcolor, sfalpha, dfcolor, dfalpha) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_BLEND_SRC_FACTOR, 2); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((sfcolor) | ((sfalpha) << 16)); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP((dfcolor) | ((dfalpha) << 16)); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_SET_CLEAR_SURFACE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_CLEAR_SURFACE(cmd, mask) do { \
	CELL_GCM_UNUSED(mask); \
	} while(false)
#else
#define CELL_GCM_SET_CLEAR_SURFACE(cmd, mask) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_CLEAR, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(mask); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_NOP
#if CELL_GCM_MEASURE
#define CELL_GCM_NOP(cmd) do { } while(false)
#else
#define CELL_GCM_NOP(cmd) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_NO_OPERATION, 1); \
	(cmd)[1] = 0; \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_CLEAR_COLOR
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_CLEAR_COLOR(cmd, color) do { \
	CELL_GCM_UNUSED(color); \
	} while(false)
#else
#define CELL_GCM_SET_CLEAR_COLOR(cmd, color) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_CLEAR_COLOR, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(color); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_CLEAR_DEPTH_STENCIL
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_CLEAR_DEPTH_STENCIL(cmd, depthStencil) do { \
	CELL_GCM_UNUSED(depthStencil); \
	} while(false)
#else
#define CELL_GCM_SET_CLEAR_DEPTH_STENCIL(cmd, depthStencil) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_CLEAR_DEPTH_STENCIL, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(depthStencil); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_COLOR_MASK
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_COLOR_MASK(cmd, mask) do { \
	CELL_GCM_UNUSED(mask); \
	} while(false)
#else
#define CELL_GCM_SET_COLOR_MASK(cmd, mask) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_COLOR_MASK, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(mask); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_COLOR_MASK_MRT
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_COLOR_MASK_MRT(cmd, mask) do { \
	CELL_GCM_UNUSED(mask); \
	} while(false)
#else
#define CELL_GCM_SET_COLOR_MASK_MRT(cmd, mask) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_MRT_COLOR_MASK, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(mask); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_CULL_FACE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_CULL_FACE(cmd, cfm) do { \
	CELL_GCM_UNUSED(cfm); \
	} while(false)
#else
#define CELL_GCM_SET_CULL_FACE(cmd, cfm) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_CULL_FACE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(cfm); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_DEPTH_BOUNDS_MIN_AND_MAX
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_DEPTH_BOUNDS_MIN_AND_MAX(cmd, zmin, zmax) do { \
	CELL_GCM_UNUSED(zmin); \
	CELL_GCM_UNUSED(zmax); \
	} while(false)
#else
#define CELL_GCM_SET_DEPTH_BOUNDS_MIN_AND_MAX(cmd, zmin, zmax) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DEPTH_BOUNDS_MIN, 2); \
	*(float *) &(cmd)[1] = CELL_GCM_ENDIAN_SWAP_F(zmin); \
	*(float *) &(cmd)[2] = CELL_GCM_ENDIAN_SWAP_F(zmax); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_SET_DEPTH_FUNC
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_DEPTH_FUNC(cmd, zf) do { \
	CELL_GCM_UNUSED(zf); \
	} while(false)
#else
#define CELL_GCM_SET_DEPTH_FUNC(cmd, zf) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DEPTH_FUNCTION, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(zf); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_DEPTH_MASK
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_DEPTH_MASK(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_DEPTH_MASK(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DEPTH_MASK, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_FRONT_FACE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_FRONT_FACE(cmd, dir) do { \
	CELL_GCM_UNUSED(dir); \
	} while(false)
#else
#define CELL_GCM_SET_FRONT_FACE(cmd, dir) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_FRONT_FACE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(dir); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_LINE_WIDTH
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_LINE_WIDTH(cmd, width) do { \
	CELL_GCM_UNUSED(width); \
	} while(false)
#else
#define CELL_GCM_SET_LINE_WIDTH(cmd, width) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_LINE_WIDTH, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(width); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_LINE_SMOOTH_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_LINE_SMOOTH_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_LINE_SMOOTH_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_LINE_SMOOTH_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_LINE_STIPPLE_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_LINE_STIPPLE_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_LINE_STIPPLE_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_LINE_STIPPLE_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_LINE_STIPPLE_PATTERN
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_LINE_STIPPLE_PATTERN(cmd, pattern, factor) do { \
	CELL_GCM_UNUSED(pattern); \
	CELL_GCM_UNUSED(factor); \
	} while(false)
#else
#define CELL_GCM_SET_LINE_STIPPLE_PATTERN(cmd, pattern, factor) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_LINE_STIPPLE_PATTERN, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((factor) | ((pattern) << 16)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_LOGIC_OP
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_LOGIC_OP(cmd, op) do { \
	CELL_GCM_UNUSED(op); \
	} while(false)
#else
#define CELL_GCM_SET_LOGIC_OP(cmd, op) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_LOGIC_OP, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(op); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_POINT_SIZE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_POINT_SIZE(cmd, size) do { \
	CELL_GCM_UNUSED(size); \
	} while(false)
#else
#define CELL_GCM_SET_POINT_SIZE(cmd, size) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_POINT_SIZE, 1); \
	*(float *) &(cmd)[1] = CELL_GCM_ENDIAN_SWAP_F(size); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_POLYGON_OFFSET_SCALE_FACTOR_AND_BIAS
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_POLYGON_OFFSET_SCALE_FACTOR_AND_BIAS(cmd, factor, bias) do { \
	CELL_GCM_UNUSED(factor); \
	CELL_GCM_UNUSED(bias); \
	} while(false)
#else
#define CELL_GCM_SET_POLYGON_OFFSET_SCALE_FACTOR_AND_BIAS(cmd, factor, bias) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_POLYGON_OFFSET_FACTOR, 2); \
	*(float *) &(cmd)[1] = CELL_GCM_ENDIAN_SWAP_F(factor); \
	*(float *) &(cmd)[2] = CELL_GCM_ENDIAN_SWAP_F(bias); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_SET_POLY_SMOOTH_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_POLY_SMOOTH_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_POLY_SMOOTH_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_POLYGON_SMOOTH_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_POLYGON_STIPPLE_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_POLYGON_STIPPLE_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_POLYGON_STIPPLE_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_POLYGON_STIPPLE_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_POLYGON_STIPPLE_PATTERN
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_POLYGON_STIPPLE_PATTERN(cmd, pattern) do { \
	CELL_GCM_UNUSED(pattern); \
	} while(false)
#else
#define CELL_GCM_SET_POLYGON_STIPPLE_PATTERN(cmd, pattern) do { \
	uint32_t d0,d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15,d16,d17,d18,d19,d20,d21,d22,d23,d24,d25,d26,d27,d28,d29,d30,d31; \
	d0 = *((uint32_t*__restrict)(pattern)+0); \
	d1 = *((uint32_t*__restrict)(pattern)+1); \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_POLYGON_STIPPLE_PATTERN, 32); \
	d2 = *((uint32_t*__restrict)(pattern)+2); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(d0); \
	d3 = *((uint32_t*__restrict)(pattern)+3); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(d1); \
	d4 = *((uint32_t*__restrict)(pattern)+4); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(d2); \
	d5 = *((uint32_t*__restrict)(pattern)+5); \
	(cmd)[4] = CELL_GCM_ENDIAN_SWAP(d3); \
	d6 = *((uint32_t*__restrict)(pattern)+6); \
	(cmd)[5] = CELL_GCM_ENDIAN_SWAP(d4); \
	d7 = *((uint32_t*__restrict)(pattern)+7); \
	(cmd)[6] = CELL_GCM_ENDIAN_SWAP(d5); \
	d8 = *((uint32_t*__restrict)(pattern)+8); \
	(cmd)[7] = CELL_GCM_ENDIAN_SWAP(d6); \
	d9 = *((uint32_t*__restrict)(pattern)+9); \
	(cmd)[8] = CELL_GCM_ENDIAN_SWAP(d7); \
	d10 = *((uint32_t*__restrict)(pattern)+10); \
	(cmd)[9] = CELL_GCM_ENDIAN_SWAP(d8); \
	d11 = *((uint32_t*__restrict)(pattern)+11); \
	(cmd)[10] = CELL_GCM_ENDIAN_SWAP(d9); \
	d12 = *((uint32_t*__restrict)(pattern)+12); \
	(cmd)[11] = CELL_GCM_ENDIAN_SWAP(d10); \
	d13 = *((uint32_t*__restrict)(pattern)+13); \
	(cmd)[12] = CELL_GCM_ENDIAN_SWAP(d11); \
	d14 = *((uint32_t*__restrict)(pattern)+14); \
	(cmd)[13] = CELL_GCM_ENDIAN_SWAP(d12); \
	d15 = *((uint32_t*__restrict)(pattern)+15); \
	(cmd)[14] = CELL_GCM_ENDIAN_SWAP(d13); \
	d16 = *((uint32_t*__restrict)(pattern)+16); \
	(cmd)[15] = CELL_GCM_ENDIAN_SWAP(d14); \
	d17 = *((uint32_t*__restrict)(pattern)+17); \
	(cmd)[16] = CELL_GCM_ENDIAN_SWAP(d15); \
	d18 = *((uint32_t*__restrict)(pattern)+18); \
	(cmd)[17] = CELL_GCM_ENDIAN_SWAP(d16); \
	d19 = *((uint32_t*__restrict)(pattern)+19); \
	(cmd)[18] = CELL_GCM_ENDIAN_SWAP(d17); \
	d20 = *((uint32_t*__restrict)(pattern)+20); \
	(cmd)[19] = CELL_GCM_ENDIAN_SWAP(d18); \
	d21 = *((uint32_t*__restrict)(pattern)+21); \
	(cmd)[20] = CELL_GCM_ENDIAN_SWAP(d19); \
	d22 = *((uint32_t*__restrict)(pattern)+22); \
	(cmd)[21] = CELL_GCM_ENDIAN_SWAP(d20); \
	d23 = *((uint32_t*__restrict)(pattern)+23); \
	(cmd)[22] = CELL_GCM_ENDIAN_SWAP(d21); \
	d24 = *((uint32_t*__restrict)(pattern)+24); \
	(cmd)[23] = CELL_GCM_ENDIAN_SWAP(d22); \
	d25 = *((uint32_t*__restrict)(pattern)+25); \
	(cmd)[24] = CELL_GCM_ENDIAN_SWAP(d23); \
	d26 = *((uint32_t*__restrict)(pattern)+26); \
	(cmd)[25] = CELL_GCM_ENDIAN_SWAP(d24); \
	d27 = *((uint32_t*__restrict)(pattern)+27); \
	(cmd)[26] = CELL_GCM_ENDIAN_SWAP(d25); \
	d28 = *((uint32_t*__restrict)(pattern)+28); \
	(cmd)[27] = CELL_GCM_ENDIAN_SWAP(d26); \
	d29 = *((uint32_t*__restrict)(pattern)+29); \
	(cmd)[28] = CELL_GCM_ENDIAN_SWAP(d27); \
	d30 = *((uint32_t*__restrict)(pattern)+30); \
	(cmd)[29] = CELL_GCM_ENDIAN_SWAP(d28); \
	d31 = *((uint32_t*__restrict)(pattern)+31); \
	(cmd)[30] = CELL_GCM_ENDIAN_SWAP(d29); \
	(cmd)[31] = CELL_GCM_ENDIAN_SWAP(d30); \
	(cmd)[32] = CELL_GCM_ENDIAN_SWAP(d31); \
	(cmd) += 33; \
	} while(false)
#endif

#undef CELL_GCM_SET_FRONT_POLYGON_MODE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_FRONT_POLYGON_MODE(cmd, mode) do { \
	CELL_GCM_UNUSED(mode); \
	} while(false)
#else
#define CELL_GCM_SET_FRONT_POLYGON_MODE(cmd, mode) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_FRONT_POLYGON_MODE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(mode); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_BACK_POLYGON_MODE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_BACK_POLYGON_MODE(cmd, mode) do { \
	CELL_GCM_UNUSED(mode); \
	} while(false)
#else
#define CELL_GCM_SET_BACK_POLYGON_MODE(cmd, mode) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_BACK_POLYGON_MODE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(mode); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_SCISSOR_LEFT_WIDTH_AND_TOP_HEIGHT
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_SCISSOR_LEFT_WIDTH_AND_TOP_HEIGHT(cmd, x, w, y, h) do { \
	CELL_GCM_UNUSED(x); \
	CELL_GCM_UNUSED(w); \
	CELL_GCM_UNUSED(y); \
	CELL_GCM_UNUSED(h); \
	} while(false)
#else
#define CELL_GCM_SET_SCISSOR_LEFT_WIDTH_AND_TOP_HEIGHT(cmd, x, w, y, h) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_SCISSOR_X, 2); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((x) | ((w) << 16)); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP((y) | ((h) << 16)); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_SET_SHADE_MODE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_SHADE_MODE(cmd, sm) do { \
	CELL_GCM_UNUSED(sm); \
	} while(false)
#else
#define CELL_GCM_SET_SHADE_MODE(cmd, sm) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_SHADE_MODEL, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(sm); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TWO_SIDE_LIGHT_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TWO_SIDE_LIGHT_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_TWO_SIDE_LIGHT_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_TWO_SIDE_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_STENCIL_FUNC_AND_REF_AND_MASK
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_STENCIL_FUNC_AND_REF_AND_MASK(cmd, func, ref, mask) do { \
	CELL_GCM_UNUSED(func); \
	CELL_GCM_UNUSED(ref); \
	CELL_GCM_UNUSED(mask); \
	} while(false)
#else
#define CELL_GCM_SET_STENCIL_FUNC_AND_REF_AND_MASK(cmd, func, ref, mask) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_FRONT_STENCIL_FUNCTION, 3); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(func); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(ref); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(mask); \
	(cmd) += 4; \
	} while(false)
#endif

#undef CELL_GCM_SET_BACK_STENCIL_FUNC_AND_REF_AND_MASK
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_BACK_STENCIL_FUNC_AND_REF_AND_MASK(cmd, func, ref, mask) do { \
	CELL_GCM_UNUSED(func); \
	CELL_GCM_UNUSED(ref); \
	CELL_GCM_UNUSED(mask); \
	} while(false)
#else
#define CELL_GCM_SET_BACK_STENCIL_FUNC_AND_REF_AND_MASK(cmd, func, ref, mask) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_BACK_STENCIL_FUNCTION, 3); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(func); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(ref); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(mask); \
	(cmd) += 4; \
	} while(false)
#endif

#undef CELL_GCM_SET_STENCIL_MASK
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_STENCIL_MASK(cmd, sm) do { \
	CELL_GCM_UNUSED(sm); \
	} while(false)
#else
#define CELL_GCM_SET_STENCIL_MASK(cmd, sm) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_FRONT_STENCIL_MASK, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(sm); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_BACK_STENCIL_MASK
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_BACK_STENCIL_MASK(cmd, sm) do { \
	CELL_GCM_UNUSED(sm); \
	} while(false)
#else
#define CELL_GCM_SET_BACK_STENCIL_MASK(cmd, sm) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_BACK_STENCIL_MASK, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(sm); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_STENCIL_OP_FAIL_AND_DEPTHFAIL_AND_DEPTHPASS
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_STENCIL_OP_FAIL_AND_DEPTHFAIL_AND_DEPTHPASS(cmd, sfail, dfail, dpass) do { \
	CELL_GCM_UNUSED(sfail); \
	CELL_GCM_UNUSED(dfail); \
	CELL_GCM_UNUSED(dpass); \
	} while(false)
#else
#define CELL_GCM_SET_STENCIL_OP_FAIL_AND_DEPTHFAIL_AND_DEPTHPASS(cmd, sfail, dfail, dpass) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_FRONT_STENCIL_FAIL_OP, 3); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(sfail); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(dfail); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(dpass); \
	(cmd) += 4; \
	} while(false)
#endif

#undef CELL_GCM_SET_BACK_STENCIL_OP_FAIL_AND_DEPTHFAIL_AND_DEPTHPASS
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_BACK_STENCIL_OP_FAIL_AND_DEPTHFAIL_AND_DEPTHPASS(cmd, sfail, dfail, dpass) do { \
	CELL_GCM_UNUSED(sfail); \
	CELL_GCM_UNUSED(dfail); \
	CELL_GCM_UNUSED(dpass); \
	} while(false)
#else
#define CELL_GCM_SET_BACK_STENCIL_OP_FAIL_AND_DEPTHFAIL_AND_DEPTHPASS(cmd, sfail, dfail, dpass) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_BACK_STENCIL_FAIL_OP, 3); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(sfail); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(dfail); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(dpass); \
	(cmd) += 4; \
	} while(false)
#endif

#undef CELL_GCM_SET_VIEWPORT_LEFT_WIDTH_AND_TOP_HEIGHT
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VIEWPORT_LEFT_WIDTH_AND_TOP_HEIGHT(cmd, x, w, y, h) do { \
	CELL_GCM_UNUSED(x); \
	CELL_GCM_UNUSED(w); \
	CELL_GCM_UNUSED(y); \
	CELL_GCM_UNUSED(h); \
	} while(false)
#else
#define CELL_GCM_SET_VIEWPORT_LEFT_WIDTH_AND_TOP_HEIGHT(cmd, x, w, y, h) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VIEWPORT_X, 2); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((x) | ((w) << 16)); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP((y) | ((h) << 16)); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_SET_CLIP_MIN_AND_MAX
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_CLIP_MIN_AND_MAX(cmd, min, max) do { \
	CELL_GCM_UNUSED(min); \
	CELL_GCM_UNUSED(max); \
	} while(false)
#else
#define CELL_GCM_SET_CLIP_MIN_AND_MAX(cmd, min, max) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DEPTH_RANGE_MIN, 2); \
	*(float *) &(cmd)[1] = CELL_GCM_ENDIAN_SWAP_F(min); \
	*(float *) &(cmd)[2] = CELL_GCM_ENDIAN_SWAP_F(max); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_SET_VIEWPORT_OFFSET_AND_SCALE_BYVAL
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VIEWPORT_OFFSET_AND_SCALE_BYVAL(cmd, offset0, offset1, offset2, offset3, scale0, scale1, scale2, scale3) do { \
	CELL_GCM_UNUSED(offset0); \
	CELL_GCM_UNUSED(offset1); \
	CELL_GCM_UNUSED(offset2); \
	CELL_GCM_UNUSED(offset3); \
	CELL_GCM_UNUSED(scale0); \
	CELL_GCM_UNUSED(scale1); \
	CELL_GCM_UNUSED(scale2); \
	CELL_GCM_UNUSED(scale3); \
	} while(false)
#else
#define CELL_GCM_SET_VIEWPORT_OFFSET_AND_SCALE_BYVAL(cmd, offset0, offset1, offset2, offset3, scale0, scale1, scale2, scale3) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VIEWPORT_TRANSLATE_X, 8); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(offset0); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(offset1); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(offset2); \
	(cmd)[4] = CELL_GCM_ENDIAN_SWAP(offset3); \
	(cmd)[5] = CELL_GCM_ENDIAN_SWAP(scale0); \
	(cmd)[6] = CELL_GCM_ENDIAN_SWAP(scale1); \
	(cmd)[7] = CELL_GCM_ENDIAN_SWAP(scale2); \
	(cmd)[8] = CELL_GCM_ENDIAN_SWAP(scale3); \
	(cmd) += 9; \
	} while(false)
#endif

#undef CELL_GCM_SET_VIEWPORT_OFFSET_AND_SCALE_TWICE_BYARR
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VIEWPORT_OFFSET_AND_SCALE_TWICE_BYARR(cmd, offset, scale) do { \
	CELL_GCM_UNUSED(offset); \
	CELL_GCM_UNUSED(scale); \
	} while(false)
#else
#define CELL_GCM_SET_VIEWPORT_OFFSET_AND_SCALE_TWICE_BYARR(cmd, offset, scale) do { \
	uint32_t o0,o1,o2,o3,s0,s1,s2,s3; \
	o0 = *((uint32_t*__restrict)offset+0); \
	o1 = *((uint32_t*__restrict)offset+1); \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VIEWPORT_TRANSLATE_X, 8); \
	o2 = *((uint32_t*__restrict)offset+2); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(o0); \
	o3 = *((uint32_t*__restrict)offset+3); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(o1); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(o2); \
	(cmd)[4] = CELL_GCM_ENDIAN_SWAP(o3); \
	(cmd)[10] = CELL_GCM_ENDIAN_SWAP(o0); \
	(cmd)[11] = CELL_GCM_ENDIAN_SWAP(o1); \
	(cmd)[12] = CELL_GCM_ENDIAN_SWAP(o2); \
	s0 = *((uint32_t*__restrict)scale+0); \
	(cmd)[13] = CELL_GCM_ENDIAN_SWAP(o3); \
	s1 = *((uint32_t*__restrict)scale+1); \
	(cmd)[9] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VIEWPORT_TRANSLATE_X, 8); \
	s2 = *((uint32_t*__restrict)scale+2); \
	(cmd)[5] = CELL_GCM_ENDIAN_SWAP(s0); \
	s3 = *((uint32_t*__restrict)scale+3); \
	(cmd)[6] = CELL_GCM_ENDIAN_SWAP(s1); \
	(cmd)[7] = CELL_GCM_ENDIAN_SWAP(s2); \
	(cmd)[8] = CELL_GCM_ENDIAN_SWAP(s3); \
	(cmd)[14] = CELL_GCM_ENDIAN_SWAP(s0); \
	(cmd)[15] = CELL_GCM_ENDIAN_SWAP(s1); \
	(cmd)[16] = CELL_GCM_ENDIAN_SWAP(s2); \
	(cmd)[17] = CELL_GCM_ENDIAN_SWAP(s3); \
	(cmd) += 18; \
	} while(false)
#endif

#undef CELL_GCM_SET_ALPHA_TEST_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_ALPHA_TEST_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_ALPHA_TEST_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_ALPHA_TEST_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_BLEND_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_BLEND_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_BLEND_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_BLEND_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_BLEND_ENABLE_MRT
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_BLEND_ENABLE_MRT(cmd, mrt1, mrt2, mrt3) do { \
	CELL_GCM_UNUSED(mrt1); \
	CELL_GCM_UNUSED(mrt2); \
	CELL_GCM_UNUSED(mrt3); \
	} while(false)
#else
#define CELL_GCM_SET_BLEND_ENABLE_MRT(cmd, mrt1, mrt2, mrt3) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_MRT_BLEND_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(((mrt1) << 1)|((mrt2) << 2)|((mrt3) << 3)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_LOGIC_OP_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_LOGIC_OP_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_LOGIC_OP_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_LOGIC_OP_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_CULL_FACE_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_CULL_FACE_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_CULL_FACE_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_CULL_FACE_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_DEPTH_BOUNDS_TEST_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_DEPTH_BOUNDS_TEST_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_DEPTH_BOUNDS_TEST_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DEPTH_BOUNDS_TEST_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_DEPTH_TEST_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_DEPTH_TEST_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_DEPTH_TEST_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DEPTH_TEST_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_DITHER_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_DITHER_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_DITHER_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DITHER_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_STENCIL_TEST_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_STENCIL_TEST_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_STENCIL_TEST_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_FRONT_STENCIL_TEST_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TWO_SIDED_STENCIL_TEST_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TWO_SIDED_STENCIL_TEST_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_TWO_SIDED_STENCIL_TEST_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_BACK_STENCIL_TEST_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_POLYGON_OFFSET_FILL_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_POLYGON_OFFSET_FILL_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_POLYGON_OFFSET_FILL_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_POLYGON_OFFSET_POLYGON_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_RESTART_INDEX_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_RESTART_INDEX_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_RESTART_INDEX_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_PRIMITIVE_RESTART_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_POINT_PARAMS_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_POINT_PARAMS_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_POINT_PARAMS_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_POINT_SIZE_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_POINT_SPRITE_CONTROL
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_POINT_SPRITE_CONTROL(cmd, enable, rmode, texcoordMask) do { \
	CELL_GCM_UNUSED(enable); \
	CELL_GCM_UNUSED(rmode); \
	CELL_GCM_UNUSED(texcoordMask); \
	} while(false)
#else
#define CELL_GCM_SET_POINT_SPRITE_CONTROL(cmd, enable, rmode, texcoordMask) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_POINT_SPRITE_CONTROL, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((enable) | ((rmode) << 1) | (texcoordMask)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_INVALIDATE_TEXTURE_CACHE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_INVALIDATE_TEXTURE_CACHE(cmd, value) do { \
	CELL_GCM_UNUSED(value); \
	} while(false)
#else
#define CELL_GCM_SET_INVALIDATE_TEXTURE_CACHE(cmd, value) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_TEXTURE_CACHE_INVALIDATE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(value); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TEXTURE_BORDER_COLOR
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TEXTURE_BORDER_COLOR(cmd, index, color) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(color); \
	} while(false)
#else
#define CELL_GCM_SET_TEXTURE_BORDER_COLOR(cmd, index, color) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_TEXTURE_BORDER_COLOR + 0x20 * (index), 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(color); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TEXTURE_FILTER
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TEXTURE_FILTER(cmd, index, bias, min, mag, conv) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(bias); \
	CELL_GCM_UNUSED(min); \
	CELL_GCM_UNUSED(mag); \
	CELL_GCM_UNUSED(conv); \
	} while(false)
#else
#define CELL_GCM_SET_TEXTURE_FILTER(cmd, index, bias, min, mag, conv) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_TEXTURE_FILTER + 0x20 * (index), 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((bias) | ((conv) << 13) | ((min) << 16) | ((mag) << 24)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TEXTURE_FILTER_RAW
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TEXTURE_FILTER_RAW(cmd, index, value) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(value); \
	} while(false)
#else
#define CELL_GCM_SET_TEXTURE_FILTER_RAW(cmd, index, value) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_TEXTURE_FILTER + 0x20 * (index), 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(value); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TEXTURE_ADDRESS
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TEXTURE_ADDRESS(cmd, index, wraps, wrapt, wrapr, unsignedRemap, zfunc, gamma) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(wraps); \
	CELL_GCM_UNUSED(wrapt); \
	CELL_GCM_UNUSED(wrapr); \
	CELL_GCM_UNUSED(unsignedRemap); \
	CELL_GCM_UNUSED(zfunc); \
	CELL_GCM_UNUSED(gamma); \
	} while(false)
#else
#define CELL_GCM_SET_TEXTURE_ADDRESS(cmd, index, wraps, wrapt, wrapr, unsignedRemap, zfunc, gamma) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_TEXTURE_CONTROL_1 + 0x20 * (index), 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((wraps) | ((wrapt) << 8) | ((unsignedRemap) << 12) | ((wrapr) << 16) | ((gamma) << 20) | ((zfunc) << 28)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TEXTURE_CONTROL
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TEXTURE_CONTROL(cmd, index, enable, minlod, maxlod, maxaniso) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(enable); \
	CELL_GCM_UNUSED(minlod); \
	CELL_GCM_UNUSED(maxlod); \
	CELL_GCM_UNUSED(maxaniso); \
	} while(false)
#else
#define CELL_GCM_SET_TEXTURE_CONTROL(cmd, index, enable, minlod, maxlod, maxaniso) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_TEXTURE_CONTROL_2 + 0x20 * (index), 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(((maxaniso) << 4) | ((maxlod) << 7) | ((minlod) << 19) | ((enable) << 31)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TEXTURE_CONTROL_3
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TEXTURE_CONTROL_3(cmd, index, value) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(value); \
	} while(false)
#else
#define CELL_GCM_SET_TEXTURE_CONTROL_3(cmd, index, value) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_TEXTURE_CONTROL_3 + (index) * 4, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(value); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TEXTURE_SIZE2
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TEXTURE_SIZE2(cmd, index, control3) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(control3); \
	} while(false)
#else
#define CELL_GCM_SET_TEXTURE_SIZE2(cmd, index, control3) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_TEXTURE_SIZE_2 + (index) * 4, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(control3); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_CYLINDRICAL_WRAP
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_CYLINDRICAL_WRAP(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_CYLINDRICAL_WRAP(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_CYLINDRICAL_WRAP_1, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_CYLINDRICAL_WRAP1
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_CYLINDRICAL_WRAP1(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_CYLINDRICAL_WRAP1(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_CYLINDRICAL_WRAP_2, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif


#undef CELL_GCM_SET_TEXTURE_OFFSET_AND_FORMAT
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TEXTURE_OFFSET_AND_FORMAT(cmd, index, offset, format) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(offset); \
	CELL_GCM_UNUSED(format); \
	} while(false)
#else
#define CELL_GCM_SET_TEXTURE_OFFSET_AND_FORMAT(cmd, index, offset, format) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_TEXTURE_OFFSET + (index) * 32, 2); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(offset); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(format); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_SET_TEXTURE_SWIZZLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TEXTURE_SWIZZLE(cmd, index, control1) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(control1); \
	} while(false)
#else
#define CELL_GCM_SET_TEXTURE_SWIZZLE(cmd, index, control1) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_TEXTURE_SWIZZLE + (index) * 32, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(control1); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TEXTURE_SIZE1
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TEXTURE_SIZE1(cmd, index, imagerect) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(imagerect); \
	} while(false)
#else
#define CELL_GCM_SET_TEXTURE_SIZE1(cmd, index, imagerect) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_TEXTURE_SIZE_1 + (index) * 32, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(imagerect); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_DRAW_MODE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_DRAW_MODE(cmd, mode) do { \
	CELL_GCM_UNUSED(mode); \
	} while(false)
#else
#define CELL_GCM_SET_DRAW_MODE(cmd, mode) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DRAW_MODE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(mode); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_DRAW_ARRAYS
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_DRAW_ARRAYS(cmd, first, count) do { \
	CELL_GCM_UNUSED(first); \
	CELL_GCM_UNUSED(count); \
	} while(false)
#else
#define CELL_GCM_SET_DRAW_ARRAYS(cmd, first, count) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DRAW_ARRAYS, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((first) | ((count)<<24)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_INDEX_ARRAY_OFFSET_AND_FORMAT
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_INDEX_ARRAY_OFFSET_AND_FORMAT(cmd, location, offset, type) do { \
	CELL_GCM_UNUSED(location); \
	CELL_GCM_UNUSED(offset); \
	CELL_GCM_UNUSED(type); \
	} while(false)
#else
#define CELL_GCM_SET_INDEX_ARRAY_OFFSET_AND_FORMAT(cmd, location, offset, type) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_INDEX_BUFFER_ADDRESS, 2); \
	(cmd)[1] = offset; \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP((location) | ((type) << 4)); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_SET_DRAW_INDEX_ARRAY
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_DRAW_INDEX_ARRAY(cmd, first, count) do { \
	CELL_GCM_UNUSED(first); \
	CELL_GCM_UNUSED(count); \
	} while(false)
#else
#define CELL_GCM_SET_DRAW_INDEX_ARRAY(cmd, first, count) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DRAW_ELEMENTS, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((first) | ((count) << 24)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_INVALIDATE_VERTEX_CACHE_FILE
#if CELL_GCM_MEASURE
#define CELL_GCM_INVALIDATE_VERTEX_CACHE_FILE(cmd) do { } while(false)
#else
#define CELL_GCM_INVALIDATE_VERTEX_CACHE_FILE(cmd) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_INVALIDATE_PRE_TRANSFORM_CACHE, 1); \
	(cmd)[1] = 0; \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_INVALIDATE_VERTEX_FILE
#if CELL_GCM_MEASURE
#define CELL_GCM_INVALIDATE_VERTEX_FILE(cmd) do { } while(false)
#else
#define CELL_GCM_INVALIDATE_VERTEX_FILE(cmd) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_INVALIDATE_POST_TRANSFORM_CACHE, 1); \
	(cmd)[1] = 0; \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_RESTART_INDEX
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_RESTART_INDEX(cmd, index) do { \
	CELL_GCM_UNUSED(index); \
	} while(false)
#else
#define CELL_GCM_SET_RESTART_INDEX(cmd, index) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_PRIMITIVE_RESTART_INDEX, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(index); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_VERTEX_DATA_4F_U
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VERTEX_DATA_4F_U(cmd, index, v0, v1, v2, v3) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(v0); \
	CELL_GCM_UNUSED(v1); \
	CELL_GCM_UNUSED(v2); \
	CELL_GCM_UNUSED(v3); \
	} while(false)
#else
#define CELL_GCM_SET_VERTEX_DATA_4F_U(cmd, index, v0, v1, v2, v3) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_ATTRIB_4F + (index) * 16, 4); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(v0); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(v1); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(v2); \
	(cmd)[4] = CELL_GCM_ENDIAN_SWAP(v3); \
	(cmd) += 5; \
	} while(false)
#endif

#undef CELL_GCM_SET_VERTEX_DATA_ARRAY_OFFSET
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VERTEX_DATA_ARRAY_OFFSET(cmd, index, location, offset) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(location); \
	CELL_GCM_UNUSED(offset); \
	} while(false)
#else
#define CELL_GCM_SET_VERTEX_DATA_ARRAY_OFFSET(cmd, index, location, offset) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_ATTRIB_ADDRESS + (index) * 4, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(((location) << 31) | (offset)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_VERTEX_DATA_ARRAY_FORMAT
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VERTEX_DATA_ARRAY_FORMAT(cmd, index, frequency, stride, size, type) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(frequency); \
	CELL_GCM_UNUSED(stride); \
	CELL_GCM_UNUSED(size); \
	CELL_GCM_UNUSED(type); \
	} while(false)
#else
#define CELL_GCM_SET_VERTEX_DATA_ARRAY_FORMAT(cmd, index, frequency, stride, size, type) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_ATTRIB_FORMAT + (index) * 4, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(((frequency) << 16) | ((stride) << 8) | ((size) << 4) | (type)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_FREQUENCY_DIVIDER_OPERATION
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_FREQUENCY_DIVIDER_OPERATION(cmd, operation) do { \
	CELL_GCM_UNUSED(operation); \
	} while(false)
#else
#define CELL_GCM_SET_FREQUENCY_DIVIDER_OPERATION(cmd, operation) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_FREQUENCY_CONTROL, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(operation); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TRANSFORM_BRANCH_BITS
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TRANSFORM_BRANCH_BITS(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_TRANSFORM_BRANCH_BITS(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_TRANSFORM_BRANCH_BITS, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_VERTEX_ATTRIB_INPUT_MASK
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VERTEX_ATTRIB_INPUT_MASK(cmd, mask) do { \
	CELL_GCM_UNUSED(mask); \
	} while(false)
#else
#define CELL_GCM_SET_VERTEX_ATTRIB_INPUT_MASK(cmd, mask) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_ATTRIBUTE_MASK, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(mask); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_VERTEX_ATTRIB_OUTPUT_MASK
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VERTEX_ATTRIB_OUTPUT_MASK(cmd, mask) do { \
	CELL_GCM_UNUSED(mask); \
	} while(false)
#else
#define CELL_GCM_SET_VERTEX_ATTRIB_OUTPUT_MASK(cmd, mask) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_RESULT_MASK, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(mask); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TRANSFORM_PROGRAM_LOAD_AND_START
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TRANSFORM_PROGRAM_LOAD_AND_START(cmd, load, start) do { \
	CELL_GCM_UNUSED(load); \
	CELL_GCM_UNUSED(start); \
	} while(false)
#else
#define CELL_GCM_SET_TRANSFORM_PROGRAM_LOAD_AND_START(cmd, load, start) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_INSTR_LOAD_SLOT, 2); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(load); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(start); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_SET_TRANSFORM_TIMEOUT
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TRANSFORM_TIMEOUT(cmd, count, registerCount) do { \
	CELL_GCM_UNUSED(count); \
	CELL_GCM_UNUSED(registerCount); \
	} while(false)
#else
#define CELL_GCM_SET_TRANSFORM_TIMEOUT(cmd, count, registerCount) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_LIMITS, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((count) | ((registerCount) << 16)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TRANSFORM_PROGRAM
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TRANSFORM_PROGRAM(cmd, index, d0, d1, d2, d3) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(d0); \
	CELL_GCM_UNUSED(d1); \
	CELL_GCM_UNUSED(d2); \
	CELL_GCM_UNUSED(d3); \
	} while(false)
#else
#define CELL_GCM_SET_TRANSFORM_PROGRAM(cmd, index, d0, d1, d2, d3) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_INSTRUCTION + (index) * 16, 4); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(d0); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(d1); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(d2); \
	(cmd)[4] = CELL_GCM_ENDIAN_SWAP(d3); \
	(cmd) += 5; \
	} while(false)
#endif

#undef CELL_GCM_SET_SHADER_PROGRAM
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_SHADER_PROGRAM(cmd, location, offset) do { \
	CELL_GCM_UNUSED(location); \
	CELL_GCM_UNUSED(offset); \
	} while(false)
#else
#define CELL_GCM_SET_SHADER_PROGRAM(cmd, location, offset) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_FRAGMENT_PROGRAM_ADDRESS, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((location) | (offset)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TEX_COORD_CONTROL
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TEX_COORD_CONTROL(cmd, index, hwTexCtrl) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(hwTexCtrl); \
	} while(false)
#else
#define CELL_GCM_SET_TEX_COORD_CONTROL(cmd, index, hwTexCtrl) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_TEXCOORD_CONTROL + (index) * 4, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(hwTexCtrl); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_SHADER_CONTROL
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_SHADER_CONTROL(cmd, value) do { \
	CELL_GCM_UNUSED(value); \
	} while(false)
#else
#define CELL_GCM_SET_SHADER_CONTROL(cmd, value) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_FRAGMENT_PROGRAM_CONTROL, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(value); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_ZMIN_MAX_CONTROL
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_ZMIN_MAX_CONTROL(cmd, cullNearFarEnable, zclampEnable, cullIgnoreW) do { \
	CELL_GCM_UNUSED(cullNearFarEnable); \
	CELL_GCM_UNUSED(zclampEnable); \
	CELL_GCM_UNUSED(cullIgnoreW); \
	} while(false)
#else
#define CELL_GCM_SET_ZMIN_MAX_CONTROL(cmd, cullNearFarEnable, zclampEnable, cullIgnoreW) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DEPTH_CLAMP, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((cullNearFarEnable) | ((zclampEnable) << 4) | ((cullIgnoreW)<<8)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_TRANSFORM_CONSTANT_LOAD_AND_LOAD_4U
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TRANSFORM_CONSTANT_LOAD_AND_LOAD_4U(cmd, loadAt, d0, d1, d2, d3) do { \
	CELL_GCM_UNUSED(loadAt); \
	CELL_GCM_UNUSED(d0); \
	CELL_GCM_UNUSED(d1); \
	CELL_GCM_UNUSED(d2); \
	CELL_GCM_UNUSED(d3); \
	} while(false)
#else
#define CELL_GCM_SET_TRANSFORM_CONSTANT_LOAD_AND_LOAD_4U(cmd, loadAt, d0, d1, d2, d3) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_CONST_LOAD_SLOT, 5); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(loadAt); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(d0); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(d1); \
	(cmd)[4] = CELL_GCM_ENDIAN_SWAP(d2); \
	(cmd)[5] = CELL_GCM_ENDIAN_SWAP(d3); \
	(cmd) += 6; \
	} while(false)
#endif

#undef CELL_GCM_SET_TRANSFORM_CONSTANT_LOAD_AND_LOAD_16U_XYZ0
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TRANSFORM_CONSTANT_LOAD_AND_LOAD_16U_XYZ0(cmd, loadAt, data) do { \
	CELL_GCM_UNUSED(loadAt); \
	CELL_GCM_UNUSED(data); \
	} while(false)
#else
#define CELL_GCM_SET_TRANSFORM_CONSTANT_LOAD_AND_LOAD_16U_XYZ0(cmd, loadAt, data) do { \
	uint32_t d0,d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11; \
	d0 = *((uint32_t*__restrict)(data)+0); \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_CONST_LOAD_SLOT, 17); \
	d1 = *((uint32_t*__restrict)(data)+1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(loadAt); \
	d2 = *((uint32_t*__restrict)(data)+2); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(d0); \
	d3 = *((uint32_t*__restrict)(data)+3); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(d1); \
	d4 = *((uint32_t*__restrict)(data)+4); \
	(cmd)[4] = CELL_GCM_ENDIAN_SWAP(d2); \
	d5 = *((uint32_t*__restrict)(data)+5); \
	(cmd)[5] = 0; \
	d6 = *((uint32_t*__restrict)(data)+6); \
	(cmd)[6] = CELL_GCM_ENDIAN_SWAP(d3); \
	d7 = *((uint32_t*__restrict)(data)+7); \
	(cmd)[7] = CELL_GCM_ENDIAN_SWAP(d4); \
	d8 = *((uint32_t*__restrict)(data)+8); \
	(cmd)[8] = CELL_GCM_ENDIAN_SWAP(d5); \
	d9 = *((uint32_t*__restrict)(data)+9); \
	(cmd)[9] = 0; \
	d10 = *((uint32_t*__restrict)(data)+10); \
	(cmd)[10] = CELL_GCM_ENDIAN_SWAP(d6); \
	d11 = *((uint32_t*__restrict)(data)+11); \
	(cmd)[11] = CELL_GCM_ENDIAN_SWAP(d7); \
	(cmd)[12] = CELL_GCM_ENDIAN_SWAP(d8); \
	(cmd)[13] = 0; \
	(cmd)[14] = CELL_GCM_ENDIAN_SWAP(d9); \
	(cmd)[15] = CELL_GCM_ENDIAN_SWAP(d10); \
	(cmd)[16] = CELL_GCM_ENDIAN_SWAP(d11); \
	(cmd)[17] = 0; \
	(cmd) += 18; \
	} while(false)
#endif

#undef CELL_GCM_SET_TRANSFORM_CONSTANT_LOAD_AND_LOAD_16U_XYZW
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TRANSFORM_CONSTANT_LOAD_AND_LOAD_16U_XYZW(cmd, loadAt, data) do { \
	CELL_GCM_UNUSED(loadAt); \
	CELL_GCM_UNUSED(data); \
	} while(false)
#else
#define CELL_GCM_SET_TRANSFORM_CONSTANT_LOAD_AND_LOAD_16U_XYZW(cmd, loadAt, data) do { \
	uint32_t d0,d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15; \
	d0 = *((uint32_t*__restrict)(data)+0); \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_CONST_LOAD_SLOT, 17); \
	d1 = *((uint32_t*__restrict)(data)+1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(loadAt); \
	d2 = *((uint32_t*__restrict)(data)+2); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(d0); \
	d3 = *((uint32_t*__restrict)(data)+3); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(d1); \
	d4 = *((uint32_t*__restrict)(data)+4); \
	(cmd)[4] = CELL_GCM_ENDIAN_SWAP(d2); \
	d5 = *((uint32_t*__restrict)(data)+5); \
	(cmd)[5] = CELL_GCM_ENDIAN_SWAP(d3); \
	d6 = *((uint32_t*__restrict)(data)+6); \
	(cmd)[6] = CELL_GCM_ENDIAN_SWAP(d4); \
	d7 = *((uint32_t*__restrict)(data)+7); \
	(cmd)[7] = CELL_GCM_ENDIAN_SWAP(d5); \
	d8 = *((uint32_t*__restrict)(data)+8); \
	(cmd)[8] = CELL_GCM_ENDIAN_SWAP(d6); \
	d9 = *((uint32_t*__restrict)(data)+9); \
	(cmd)[9] = CELL_GCM_ENDIAN_SWAP(d7); \
	d10 = *((uint32_t*__restrict)(data)+10); \
	(cmd)[10] = CELL_GCM_ENDIAN_SWAP(d8); \
	d11 = *((uint32_t*__restrict)(data)+11); \
	(cmd)[11] = CELL_GCM_ENDIAN_SWAP(d9); \
	d12 = *((uint32_t*__restrict)(data)+12); \
	(cmd)[12] = CELL_GCM_ENDIAN_SWAP(d10); \
	d13 = *((uint32_t*__restrict)(data)+13); \
	(cmd)[13] = CELL_GCM_ENDIAN_SWAP(d11); \
	d14 = *((uint32_t*__restrict)(data)+14); \
	(cmd)[14] = CELL_GCM_ENDIAN_SWAP(d12); \
	d15 = *((uint32_t*__restrict)(data)+15); \
	(cmd)[15] = CELL_GCM_ENDIAN_SWAP(d13); \
	(cmd)[16] = CELL_GCM_ENDIAN_SWAP(d14); \
	(cmd)[17] = CELL_GCM_ENDIAN_SWAP(d15); \
	(cmd) += 18; \
	} while(false)
#endif

#undef CELL_GCM_SET_TRANSFORM_CONSTANT_LOAD_AND_LOAD_32U
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_TRANSFORM_CONSTANT_LOAD_AND_LOAD_32U(cmd, loadAt, data) do { \
	CELL_GCM_UNUSED(loadAt); \
	CELL_GCM_UNUSED(data); \
	} while(false)
#else
#define CELL_GCM_SET_TRANSFORM_CONSTANT_LOAD_AND_LOAD_32U(cmd, loadAt, data) do { \
	uint32_t d0,d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15,d16,d17,d18,d19,d20,d21,d22,d23,d24,d25,d26,d27,d28,d29,d30,d31; \
	d0 = *((uint32_t*__restrict)(data)+0); \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_CONST_LOAD_SLOT, 33); \
	d1 = *((uint32_t*__restrict)(data)+1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(loadAt); \
	d2 = *((uint32_t*__restrict)(data)+2); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(d0); \
	d3 = *((uint32_t*__restrict)(data)+3); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(d1); \
	d4 = *((uint32_t*__restrict)(data)+4); \
	(cmd)[4] = CELL_GCM_ENDIAN_SWAP(d2); \
	d5 = *((uint32_t*__restrict)(data)+5); \
	(cmd)[5] = CELL_GCM_ENDIAN_SWAP(d3); \
	d6 = *((uint32_t*__restrict)(data)+6); \
	(cmd)[6] = CELL_GCM_ENDIAN_SWAP(d4); \
	d7 = *((uint32_t*__restrict)(data)+7); \
	(cmd)[7] = CELL_GCM_ENDIAN_SWAP(d5); \
	d8 = *((uint32_t*__restrict)(data)+8); \
	(cmd)[8] = CELL_GCM_ENDIAN_SWAP(d6); \
	d9 = *((uint32_t*__restrict)(data)+9); \
	(cmd)[9] = CELL_GCM_ENDIAN_SWAP(d7); \
	d10 = *((uint32_t*__restrict)(data)+10); \
	(cmd)[10] = CELL_GCM_ENDIAN_SWAP(d8); \
	d11 = *((uint32_t*__restrict)(data)+11); \
	(cmd)[11] = CELL_GCM_ENDIAN_SWAP(d9); \
	d12 = *((uint32_t*__restrict)(data)+12); \
	(cmd)[12] = CELL_GCM_ENDIAN_SWAP(d10); \
	d13 = *((uint32_t*__restrict)(data)+13); \
	(cmd)[13] = CELL_GCM_ENDIAN_SWAP(d11); \
	d14 = *((uint32_t*__restrict)(data)+14); \
	(cmd)[14] = CELL_GCM_ENDIAN_SWAP(d12); \
	d15 = *((uint32_t*__restrict)(data)+15); \
	(cmd)[15] = CELL_GCM_ENDIAN_SWAP(d13); \
	d16 = *((uint32_t*__restrict)(data)+16); \
	(cmd)[16] = CELL_GCM_ENDIAN_SWAP(d14); \
	d17 = *((uint32_t*__restrict)(data)+17); \
	(cmd)[17] = CELL_GCM_ENDIAN_SWAP(d15); \
	d18 = *((uint32_t*__restrict)(data)+18); \
	(cmd)[18] = CELL_GCM_ENDIAN_SWAP(d16); \
	d19 = *((uint32_t*__restrict)(data)+19); \
	(cmd)[19] = CELL_GCM_ENDIAN_SWAP(d17); \
	d20 = *((uint32_t*__restrict)(data)+20); \
	(cmd)[20] = CELL_GCM_ENDIAN_SWAP(d18); \
	d21 = *((uint32_t*__restrict)(data)+21); \
	(cmd)[21] = CELL_GCM_ENDIAN_SWAP(d19); \
	d22 = *((uint32_t*__restrict)(data)+22); \
	(cmd)[22] = CELL_GCM_ENDIAN_SWAP(d20); \
	d23 = *((uint32_t*__restrict)(data)+23); \
	(cmd)[23] = CELL_GCM_ENDIAN_SWAP(d21); \
	d24 = *((uint32_t*__restrict)(data)+24); \
	(cmd)[24] = CELL_GCM_ENDIAN_SWAP(d22); \
	d25 = *((uint32_t*__restrict)(data)+25); \
	(cmd)[25] = CELL_GCM_ENDIAN_SWAP(d23); \
	d26 = *((uint32_t*__restrict)(data)+26); \
	(cmd)[26] = CELL_GCM_ENDIAN_SWAP(d24); \
	d27 = *((uint32_t*__restrict)(data)+27); \
	(cmd)[27] = CELL_GCM_ENDIAN_SWAP(d25); \
	d28 = *((uint32_t*__restrict)(data)+28); \
	(cmd)[28] = CELL_GCM_ENDIAN_SWAP(d26); \
	d29 = *((uint32_t*__restrict)(data)+29); \
	(cmd)[29] = CELL_GCM_ENDIAN_SWAP(d27); \
	d30 = *((uint32_t*__restrict)(data)+30); \
	(cmd)[30] = CELL_GCM_ENDIAN_SWAP(d28); \
	d31 = *((uint32_t*__restrict)(data)+31); \
	(cmd)[31] = CELL_GCM_ENDIAN_SWAP(d29); \
	(cmd)[32] = CELL_GCM_ENDIAN_SWAP(d30); \
	(cmd)[33] = CELL_GCM_ENDIAN_SWAP(d31); \
	(cmd) += 34; \
	} while(false)
#endif

#undef CELL_GCM_SET_SHADER_PACKER
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_SHADER_PACKER(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_SHADER_PACKER(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_FRAGMENT_PROGRAM_CONTROL_2, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_USER_CLIP_PLANE_CONTROL
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_USER_CLIP_PLANE_CONTROL(cmd, plane0, plane1, plane2, plane3, plane4, plane5) do { \
	CELL_GCM_UNUSED(plane0); \
	CELL_GCM_UNUSED(plane1); \
	CELL_GCM_UNUSED(plane2); \
	CELL_GCM_UNUSED(plane3); \
	CELL_GCM_UNUSED(plane4); \
	CELL_GCM_UNUSED(plane5); \
	} while(false)
#else
#define CELL_GCM_SET_USER_CLIP_PLANE_CONTROL(cmd, plane0, plane1, plane2, plane3, plane4, plane5) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_CLIP_PLANE_CONTROL, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((plane0) | ((plane1) << 4) | ((plane2) << 8) | ((plane3) << 12) | ((plane4) << 16) | ((plane5) << 20)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_RENDER_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_RENDER_ENABLE(cmd, mode, offset) do { \
	CELL_GCM_UNUSED(mode); \
	CELL_GCM_UNUSED(offset); \
	} while(false)
#else
#define CELL_GCM_SET_RENDER_ENABLE(cmd, mode, offset) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_RENDER_CONTROL, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((offset) | ((mode) << 24)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_ZPASS_PIXEL_COUNT_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_ZPASS_PIXEL_COUNT_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_ZPASS_PIXEL_COUNT_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_PIXEL_COUNT_MODE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_CLEAR_REPORT_VALUE
#if CELL_GCM_MEASURE
#define CELL_GCM_CLEAR_REPORT_VALUE(cmd, type) do { \
	CELL_GCM_UNUSED(type); \
	} while(false)
#else
#define CELL_GCM_CLEAR_REPORT_VALUE(cmd, type) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_RESET_REPORT, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(type); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_GET_REPORT
#if CELL_GCM_MEASURE
#define CELL_GCM_GET_REPORT(cmd, type, offset) do { \
	CELL_GCM_UNUSED(type); \
	CELL_GCM_UNUSED(offset); \
	} while(false)
#else
#define CELL_GCM_GET_REPORT(cmd, type, offset) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_WRITE_REPORT, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((offset) | ((type) << 24)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_ZCULL_STATS_ENABLE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_ZCULL_STATS_ENABLE(cmd, enable) do { \
	CELL_GCM_UNUSED(enable); \
	} while(false)
#else
#define CELL_GCM_SET_ZCULL_STATS_ENABLE(cmd, enable) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DEPTH_CULL_STATS_ENABLE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(enable); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_ZCULL_CONTROL0
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_ZCULL_CONTROL0(cmd, zCullDir, zCullFormat) do { \
	CELL_GCM_UNUSED(zCullDir); \
	CELL_GCM_UNUSED(zCullFormat); \
	} while(false)
#else
#define CELL_GCM_SET_ZCULL_CONTROL0(cmd, zCullDir, zCullFormat) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DEPTH_CULL_DEPTH_CONTROL, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((zCullDir) | (zCullFormat<<4)); \
	(cmd) += 2; \
	} while(false)
#endif


#undef CELL_GCM_SET_ZCULL_CONTROL1
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_ZCULL_CONTROL1(cmd, moveForwardLimit, pushBackLimit) do { \
	CELL_GCM_UNUSED(moveForwardLimit); \
	CELL_GCM_UNUSED(pushBackForwardLimit); \
	} while(false)
#else
#define CELL_GCM_SET_ZCULL_CONTROL1(cmd, moveForwardLimit, pushBackLimit) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DEPTH_CULL_FEEDBACK_CONTROL, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((moveForwardLimit<<16) | (pushBackLimit)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_SCULL_CONTROL
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_SCULL_CONTROL(cmd, sFunc, sRef, sMask) do { \
	CELL_GCM_UNUSED(sFunc); \
	CELL_GCM_UNUSED(sRef); \
	CELL_GCM_UNUSED(sMask); \
	} while(false)
#else
#define CELL_GCM_SET_SCULL_CONTROL(cmd, sFunc, sRef, sMask) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DEPTH_CULL_STENCIL_CONTROL, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((sFunc) | (sRef<<16) | (sMask<<24)); \
	(cmd) += 2; \
	} while(false)
#endif


#undef CELL_GCM_INVALIDATE_ZCULL
#if CELL_GCM_MEASURE
#define CELL_GCM_INVALIDATE_ZCULL(cmd) do { \
	CELL_GCM_UNUSED(cmd); \
	} while(false)
#else
#define CELL_GCM_INVALIDATE_ZCULL(cmd) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_INVALIDATE_DEPTH_CULL, 1); \
	(cmd)[1] = 0; \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_SEMAPHORE_OFFSET
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_SEMAPHORE_OFFSET(cmd, offset) do { \
	CELL_GCM_UNUSED(offset); \
	} while(false)
#else
#define CELL_GCM_SET_SEMAPHORE_OFFSET(cmd, offset) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_SYNC_SEMAPHORE_OFFSET, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(offset); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_TEXTURE_READ_SEMAPHORE_RELEASE
#if CELL_GCM_MEASURE
#define CELL_GCM_TEXTURE_READ_SEMAPHORE_RELEASE(cmd, value) do { \
	CELL_GCM_UNUSED(value); \
	} while(false)
#else
#define CELL_GCM_TEXTURE_READ_SEMAPHORE_RELEASE(cmd, value) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_SIGNAL_TEXTURE_READ_SEMAPHORE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(value); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_BACK_END_WRITE_SEMAPHORE_RELEASE
#if CELL_GCM_MEASURE
#define CELL_GCM_BACK_END_WRITE_SEMAPHORE_RELEASE(cmd, value) do { \
	CELL_GCM_UNUSED(value); \
	} while(false)
#else
#define CELL_GCM_BACK_END_WRITE_SEMAPHORE_RELEASE(cmd, value) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_SIGNAL_BACKEND_WRITE_SEMAPHORE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(value); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SEMAPHORE_OFFSET
#if CELL_GCM_MEASURE
#define CELL_GCM_SEMAPHORE_OFFSET(cmd, offset) do { \
	CELL_GCM_UNUSED(offset); \
	} while(false)
#else
#define CELL_GCM_SEMAPHORE_OFFSET(cmd, offset) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_SEMAPHORE_OFFSET, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(offset); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SEMAPHORE_ACQUIRE
#if CELL_GCM_MEASURE
#define CELL_GCM_SEMAPHORE_ACQUIRE(cmd, value) do { \
	CELL_GCM_UNUSED(value); \
	} while(false)
#else
#define CELL_GCM_SEMAPHORE_ACQUIRE(cmd, value) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_WAIT_ON_SEMAPHORE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(value); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SEMAPHORE_RELEASE
#if CELL_GCM_MEASURE
#define CELL_GCM_SEMAPHORE_RELEASE(cmd, value) do { \
	CELL_GCM_UNUSED(value); \
	} while(false)
#else
#define CELL_GCM_SEMAPHORE_RELEASE(cmd, value) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_SIGNAL_SEMAPHORE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(value); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_ANTI_ALIASING_CONTROL
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_ANTI_ALIASING_CONTROL(cmd, enable, alphaToCoverage, alphaToOne, sampleMask) do { \
	CELL_GCM_UNUSED(enable); \
	CELL_GCM_UNUSED(alphaToCoverage); \
	CELL_GCM_UNUSED(alphaToOne); \
	CELL_GCM_UNUSED(sampleMask); \
	} while(false)
#else
#define CELL_GCM_SET_ANTI_ALIASING_CONTROL(cmd, enable, alphaToCoverage, alphaToOne, sampleMask) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_MULTISAMPLE_CONTROL, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((enable) | ((alphaToCoverage) << 4) | ((alphaToOne) << 8) | ((sampleMask) << 16)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_CONTEXT_DMA_COLOR_A
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_CONTEXT_DMA_COLOR_A(cmd, context) do { \
	CELL_GCM_UNUSED(context); \
	} while(false)
#else
#define CELL_GCM_SET_CONTEXT_DMA_COLOR_A(cmd, context) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_COLOR_BUFFER_CONTEXT_0, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(context); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_CONTEXT_DMA_COLOR_B
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_CONTEXT_DMA_COLOR_B(cmd, context) do { \
	CELL_GCM_UNUSED(context); \
	} while(false)
#else
#define CELL_GCM_SET_CONTEXT_DMA_COLOR_B(cmd, context) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_COLOR_BUFFER_CONTEXT_1, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(context); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_CONTEXT_DMA_COLOR_C
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_CONTEXT_DMA_COLOR_C(cmd, context) do { \
	CELL_GCM_UNUSED(context); \
	} while(false)
#else
#define CELL_GCM_SET_CONTEXT_DMA_COLOR_C(cmd, context) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_COLOR_BUFFER_CONTEXT_2, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(context); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_CONTEXT_DMA_COLOR_D
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_CONTEXT_DMA_COLOR_D(cmd, context) do { \
	CELL_GCM_UNUSED(context); \
	} while(false)
#else
#define CELL_GCM_SET_CONTEXT_DMA_COLOR_D(cmd, context) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_COLOR_BUFFER_CONTEXT_3, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(context); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_CONTEXT_DMA_COLOR_C_AND_D
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_CONTEXT_DMA_COLOR_C_AND_D(cmd, contextC, contextD) do { \
	CELL_GCM_UNUSED(contextC); \
	CELL_GCM_UNUSED(contextD); \
	} while(false)
#else
#define CELL_GCM_SET_CONTEXT_DMA_COLOR_C_AND_D(cmd, contextC, contextD) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_COLOR_BUFFER_CONTEXT_2, 2); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(contextC); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(contextD); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_SET_SURFACE_FORMAT_AND_PITCH_A_AND_COLOR_AOFFSET_AND_ZETA_OFFSET_AND_COLOR_BOFFSET_AND_PITCH_B
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_SURFACE_FORMAT_AND_PITCH_A_AND_COLOR_AOFFSET_AND_ZETA_OFFSET_AND_COLOR_BOFFSET_AND_PITCH_B(cmd, colorFormat, depthFormat, antialias, type, width, height, pitchA, offsetA, offsetZ, offsetB, pitchB) do { \
	CELL_GCM_UNUSED(colorFormat); \
	CELL_GCM_UNUSED(depthFormat); \
	CELL_GCM_UNUSED(antialias); \
	CELL_GCM_UNUSED(type); \
	CELL_GCM_UNUSED(width); \
	CELL_GCM_UNUSED(height); \
	CELL_GCM_UNUSED(pitchA); \
	CELL_GCM_UNUSED(offsetA); \
	CELL_GCM_UNUSED(offsetZ); \
	CELL_GCM_UNUSED(offsetB); \
	CELL_GCM_UNUSED(pitchB); \
	} while(false)
#else
#define CELL_GCM_SET_SURFACE_FORMAT_AND_PITCH_A_AND_COLOR_AOFFSET_AND_ZETA_OFFSET_AND_COLOR_BOFFSET_AND_PITCH_B(cmd, colorFormat, depthFormat, antialias, type, width, height, pitchA, offsetA, offsetZ, offsetB, pitchB) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_RENDER_TARGET_FORMAT, 6); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((colorFormat) | ((depthFormat) << 5) | ((type) << 8) | ((antialias) << 12) | ((width) << 16) | ((height) << 24)); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(pitchA); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(offsetA); \
	(cmd)[4] = CELL_GCM_ENDIAN_SWAP(offsetZ); \
	(cmd)[5] = CELL_GCM_ENDIAN_SWAP(offsetB); \
	(cmd)[6] = CELL_GCM_ENDIAN_SWAP(pitchB); \
	(cmd) += 7; \
	} while(false)
#endif

#undef CELL_GCM_SET_SURFACE_PITCH_Z
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_SURFACE_PITCH_Z(cmd, pitch) do { \
	CELL_GCM_UNUSED(pitch); \
	} while(false)
#else
#define CELL_GCM_SET_SURFACE_PITCH_Z(cmd, pitch) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DEPTH_BUFFER_PITCH, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(pitch); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_SURFACE_PITCH_C_AND_PITCH_D_AND_COLOR_COFFSET_AND_COLOR_DOFFSET
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_SURFACE_PITCH_C_AND_PITCH_D_AND_COLOR_COFFSET_AND_COLOR_DOFFSET(cmd, pitchC, pitchD, offsetC, offsetD) do { \
	CELL_GCM_UNUSED(pitchC); \
	CELL_GCM_UNUSED(pitchD); \
	CELL_GCM_UNUSED(offsetC); \
	CELL_GCM_UNUSED(offsetD); \
	} while(false)
#else
#define CELL_GCM_SET_SURFACE_PITCH_C_AND_PITCH_D_AND_COLOR_COFFSET_AND_COLOR_DOFFSET(cmd, pitchC, pitchD, offsetC, offsetD) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_COLOR_BUFFER_PITCH_2, 4); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(pitchC); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(pitchD); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(offsetC); \
	(cmd)[4] = CELL_GCM_ENDIAN_SWAP(offsetD); \
	(cmd) += 5; \
	} while(false)
#endif

#undef CELL_GCM_SET_SURFACE_COLOR_TARGET
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_SURFACE_COLOR_TARGET(cmd, mask) do { \
	CELL_GCM_UNUSED(mask); \
	} while(false)
#else
#define CELL_GCM_SET_SURFACE_COLOR_TARGET(cmd, mask) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_DRAW_BUFFER_MASK, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(mask); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_WINDOW_OFFSET
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_WINDOW_OFFSET(cmd, x, y) do { \
	CELL_GCM_UNUSED(x); \
	CELL_GCM_UNUSED(y); \
	} while(false)
#else
#define CELL_GCM_SET_WINDOW_OFFSET(cmd, x, y) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_RENDER_TARGET_ORIGIN, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((x) | ((y) << 16)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_SURFACE_CLIP_HORIZONTAL_AND_VERTICAL
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_SURFACE_CLIP_HORIZONTAL_AND_VERTICAL(cmd, x, w, y, h) do { \
	CELL_GCM_UNUSED(x); \
	CELL_GCM_UNUSED(w); \
	CELL_GCM_UNUSED(y); \
	CELL_GCM_UNUSED(h); \
	} while(false)
#else
#define CELL_GCM_SET_SURFACE_CLIP_HORIZONTAL_AND_VERTICAL(cmd, x, w, y, h) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_RENDER_TARGET_LEFT_WIDTH, 2); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((x) | ((w) << 16)); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP((y) | ((h) << 16)); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_SET_SHADER_WINDOW
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_SHADER_WINDOW(cmd, height, origin, pixelCenters) do { \
	CELL_GCM_UNUSED(height); \
	CELL_GCM_UNUSED(origin); \
	CELL_GCM_UNUSED(pixelCenters); \
	} while(false)
#else
#define CELL_GCM_SET_SHADER_WINDOW(cmd, height, origin, pixelCenters) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_RENDER_TARGET_AUX_HEIGHT, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((height) | ((origin) << 12) | ((pixelCenters) << 16)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_REFERENCE
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_REFERENCE(cmd, ref) do { \
	CELL_GCM_UNUSED(ref); \
	} while(false)
#else
#define CELL_GCM_SET_REFERENCE(cmd, ref) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_REFERENCE, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(ref); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SURFACE_2D_DMA_IMAGE_DESTIN
#if CELL_GCM_MEASURE
#define CELL_GCM_SURFACE_2D_DMA_IMAGE_DESTIN(cmd, context) do { \
	CELL_GCM_UNUSED(context); \
	} while(false)
#else
#define CELL_GCM_SURFACE_2D_DMA_IMAGE_DESTIN(cmd, context) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_SURFACE_2D_DESTINATION_CONTEXT, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(context); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SURFACE_2D_SET_OFFSET_DESTIN
#if CELL_GCM_MEASURE
#define CELL_GCM_SURFACE_2D_SET_OFFSET_DESTIN(cmd, dst) do { \
	CELL_GCM_UNUSED(dst); \
	} while(false)
#else
#define CELL_GCM_SURFACE_2D_SET_OFFSET_DESTIN(cmd, dst) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_SURFACE_2D_DESTINATION_ADDRESS, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(dst); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SURFACE_2D_SET_COLOR_FORMAT_AND_PITCHES
#if CELL_GCM_MEASURE
#define CELL_GCM_SURFACE_2D_SET_COLOR_FORMAT_AND_PITCHES(cmd, format, srcPitch, dstPitch) do { \
	CELL_GCM_UNUSED(format); \
	CELL_GCM_UNUSED(srcPitch); \
	CELL_GCM_UNUSED(dstPitch); \
	} while(false)
#else
#define CELL_GCM_SURFACE_2D_SET_COLOR_FORMAT_AND_PITCHES(cmd, format, srcPitch, dstPitch) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_SURFACE_2D_FORMAT, 2); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(format); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP((srcPitch) | ((dstPitch) << 16)); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_BLIT_POINT_AND_SIZE_OUT_AND_SIZE_IN
#if CELL_GCM_MEASURE
#define CELL_GCM_BLIT_POINT_AND_SIZE_OUT_AND_SIZE_IN(cmd, pointX, pointY, outSizeX, outSizeY, inSizeX, inSizeY) do { \
	CELL_GCM_UNUSED(pointX); \
	CELL_GCM_UNUSED(pointY); \
	CELL_GCM_UNUSED(outSizeX); \
	CELL_GCM_UNUSED(outSizeY); \
	CELL_GCM_UNUSED(inSizeX); \
	CELL_GCM_UNUSED(inSizeY); \
	} while(false)
#else
#define CELL_GCM_BLIT_POINT_AND_SIZE_OUT_AND_SIZE_IN(cmd, pointX, pointY, outSizeX, outSizeY, inSizeX, inSizeY) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_BLIT_POINT, 3); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(((pointY) << 16) | (pointX)); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(((outSizeY) << 16) | (outSizeX)); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(((inSizeY) << 16) | (inSizeX)); \
	(cmd) += 4; \
	} while(false)
#endif

#undef CELL_GCM_STRETCHBLIT_SET_CONTEXT_DMA_IMAGE
#if CELL_GCM_MEASURE
#define CELL_GCM_STRETCHBLIT_SET_CONTEXT_DMA_IMAGE(cmd, context) do { \
	CELL_GCM_UNUSED(context); \
	} while(false)
#else
#define CELL_GCM_STRETCHBLIT_SET_CONTEXT_DMA_IMAGE(cmd, context) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_STRETCH_BLIT_SOURCE_CONTEXT, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(context); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_STRETCHBLIT_SET_CONTEXT_SURFACE
#if CELL_GCM_MEASURE
#define CELL_GCM_STRETCHBLIT_SET_CONTEXT_SURFACE(cmd, context) do { \
	CELL_GCM_UNUSED(context); \
	} while(false)
#else
#define CELL_GCM_STRETCHBLIT_SET_CONTEXT_SURFACE(cmd, context) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_STRETCH_BLIT_DESTINATION_CONTEXT, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(context); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_STRETCHBLIT_SET_CONVERSION_AND_FORMAT_AND_OPERATION_AND_POINT_AND_SIZE_AND_OUT_POINT_AND_OUT_SIZE_AND_DS_DX_AND_DT_DY
#if CELL_GCM_MEASURE
#define CELL_GCM_STRETCHBLIT_SET_CONVERSION_AND_FORMAT_AND_OPERATION_AND_POINT_AND_SIZE_AND_OUT_POINT_AND_OUT_SIZE_AND_DS_DX_AND_DT_DY(cmd, conv, fmt, op, inX, inY, inW, inH, outX, outY, outW, outH, dsdx, dtdy) do { \
	CELL_GCM_UNUSED(conv); \
	CELL_GCM_UNUSED(fmt); \
	CELL_GCM_UNUSED(op); \
	CELL_GCM_UNUSED(inX); \
	CELL_GCM_UNUSED(inY); \
	CELL_GCM_UNUSED(inW); \
	CELL_GCM_UNUSED(inH); \
	CELL_GCM_UNUSED(outX); \
	CELL_GCM_UNUSED(outY); \
	CELL_GCM_UNUSED(outW); \
	CELL_GCM_UNUSED(outH); \
	CELL_GCM_UNUSED(dsdx); \
	CELL_GCM_UNUSED(dtdy); \
	} while(false)
#else
#define CELL_GCM_STRETCHBLIT_SET_CONVERSION_AND_FORMAT_AND_OPERATION_AND_POINT_AND_SIZE_AND_OUT_POINT_AND_OUT_SIZE_AND_DS_DX_AND_DT_DY(cmd, conv, fmt, op, inX, inY, inW, inH, outX, outY, outW, outH, dsdx, dtdy) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_STRETCH_BLIT_CONVERSION, 9); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(conv); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(fmt); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(op); \
	(cmd)[4] = CELL_GCM_ENDIAN_SWAP(((inY) << 16) | (inX)); \
	(cmd)[5] = CELL_GCM_ENDIAN_SWAP(((inH) << 16) | (inW)); \
	(cmd)[6] = CELL_GCM_ENDIAN_SWAP(((outY) << 16) | (outX)); \
	(cmd)[7] = CELL_GCM_ENDIAN_SWAP(((outH) << 16) | (outW)); \
	(cmd)[8] = CELL_GCM_ENDIAN_SWAP(dsdx); \
	(cmd)[9] = CELL_GCM_ENDIAN_SWAP(dtdy); \
	(cmd) += 10; \
	} while(false)
#endif

#undef CELL_GCM_STRETCHBLIT_IMAGE_IN_SIZE_AND_FORMAT_AND_OFFSET_AND_POINT
#if CELL_GCM_MEASURE
#define CELL_GCM_STRETCHBLIT_IMAGE_IN_SIZE_AND_FORMAT_AND_OFFSET_AND_POINT(cmd, w, h, pitch, origin, interpolator, offset, u, v) do { \
	CELL_GCM_UNUSED(w); \
	CELL_GCM_UNUSED(h); \
	CELL_GCM_UNUSED(pitch); \
	CELL_GCM_UNUSED(origin); \
	CELL_GCM_UNUSED(interpolator); \
	CELL_GCM_UNUSED(offset); \
	CELL_GCM_UNUSED(u); \
	CELL_GCM_UNUSED(v); \
	} while(false)
#else
#define CELL_GCM_STRETCHBLIT_IMAGE_IN_SIZE_AND_FORMAT_AND_OFFSET_AND_POINT(cmd, w, h, pitch, origin, interpolator, offset, u, v) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_STRETCH_BLIT_IN_SIZE, 4); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(((h) << 16) | (w)); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP((pitch) | ((origin) << 16) | ((interpolator) << 24)); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(offset); \
	(cmd)[4] = CELL_GCM_ENDIAN_SWAP(((v) << 16) | (u)); \
	(cmd) += 5; \
	} while(false)
#endif

#undef CELL_GCM_HOST2VID_SET_CONTEXT_DMA_BUFFER_IN_AND_OUT
#if CELL_GCM_MEASURE
#define CELL_GCM_HOST2VID_SET_CONTEXT_DMA_BUFFER_IN_AND_OUT(cmd, srcContext, dstContext) do { \
	CELL_GCM_UNUSED(srcContext); \
	CELL_GCM_UNUSED(dstContext); \
	} while(false)
#else
#define CELL_GCM_HOST2VID_SET_CONTEXT_DMA_BUFFER_IN_AND_OUT(cmd, srcContext, dstContext) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_HOST_TO_VID_SOURCE_CONTEXT, 2); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(srcContext); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(dstContext); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_HOST2VID_OFFSET_IN_AND_OUT_AND_PITCH_IN_AND_OUT_AND_LINE_LENGTH_AND_LINE_COUNT_AND_FORMAT_AND_NOTIFY
#if CELL_GCM_MEASURE
#define CELL_GCM_HOST2VID_OFFSET_IN_AND_OUT_AND_PITCH_IN_AND_OUT_AND_LINE_LENGTH_AND_LINE_COUNT_AND_FORMAT_AND_NOTIFY(cmd, inOffset, outOffset, inPitch, outPitch, lineLength, lineCount, inFormat, outFormat, notify) do { \
	CELL_GCM_UNUSED(inOffset); \
	CELL_GCM_UNUSED(outOffset); \
	CELL_GCM_UNUSED(inPitch); \
	CELL_GCM_UNUSED(outPitch); \
	CELL_GCM_UNUSED(lineLength); \
	CELL_GCM_UNUSED(lineCount); \
	CELL_GCM_UNUSED(inFormat); \
	CELL_GCM_UNUSED(outFormat); \
	CELL_GCM_UNUSED(notify); \
	} while(false)
#else
#define CELL_GCM_HOST2VID_OFFSET_IN_AND_OUT_AND_PITCH_IN_AND_OUT_AND_LINE_LENGTH_AND_LINE_COUNT_AND_FORMAT_AND_NOTIFY(cmd, inOffset, outOffset, inPitch, outPitch, lineLength, lineCount, inFormat, outFormat, notify) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_HOST_TO_VID_SOURCE_OFFSET, 8); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(inOffset); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(outOffset); \
	(cmd)[3] = CELL_GCM_ENDIAN_SWAP(inPitch); \
	(cmd)[4] = CELL_GCM_ENDIAN_SWAP(outPitch); \
	(cmd)[5] = CELL_GCM_ENDIAN_SWAP(lineLength); \
	(cmd)[6] = CELL_GCM_ENDIAN_SWAP(lineCount); \
	(cmd)[7] = CELL_GCM_ENDIAN_SWAP(((outFormat) << 8) | (inFormat)); \
	(cmd)[8] = CELL_GCM_ENDIAN_SWAP(notify); \
	(cmd) += 9; \
	} while(false)
#endif

#undef CELL_GCM_HOST2VID_OFFSET_OUT
#if CELL_GCM_MEASURE
#define CELL_GCM_HOST2VID_OFFSET_OUT(cmd, offset) do { \
	CELL_GCM_UNUSED(offset); \
	} while(false)
#else
#define CELL_GCM_HOST2VID_OFFSET_OUT(cmd, offset) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_HOST_TO_VID_DESTINATION_OFFSET, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(offset); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SURFACE_SWIZ_SET_CONTEXT_DMA_IMAGE
#if CELL_GCM_MEASURE
#define CELL_GCM_SURFACE_SWIZ_SET_CONTEXT_DMA_IMAGE(cmd, context) do { \
	CELL_GCM_UNUSED(context); \
	} while(false)
#else
#define CELL_GCM_SURFACE_SWIZ_SET_CONTEXT_DMA_IMAGE(cmd, context) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_SURFACE_SWIZ_CONTEXT, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(context); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SURFACE_SWIZ_SET_FORMAT_AND_OFFSET
#if CELL_GCM_MEASURE
#define CELL_GCM_SURFACE_SWIZ_SET_FORMAT_AND_OFFSET(cmd, format, log2Width, log2Height, offset) do { \
	CELL_GCM_UNUSED(format); \
	CELL_GCM_UNUSED(log2Width); \
	CELL_GCM_UNUSED(log2Height); \
	CELL_GCM_UNUSED(offset); \
	} while(false)
#else
#define CELL_GCM_SURFACE_SWIZ_SET_FORMAT_AND_OFFSET(cmd, format, log2Width, log2Height, offset) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_SURFACE_SWIZ_FORMAT, 2); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((format) | ((log2Width) << 16) | ((log2Height) << 24));  \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(offset); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_SET_VERTEX_TEXTURE_OFFSET_AND_FORMAT
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VERTEX_TEXTURE_OFFSET_AND_FORMAT(cmd, index, offset, format) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(offset); \
	CELL_GCM_UNUSED(format); \
	} while(false)
#else
#define CELL_GCM_SET_VERTEX_TEXTURE_OFFSET_AND_FORMAT(cmd, index, offset, format) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_TEXTURE_ADDRESS + (index) * 32, 2); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(offset); \
	(cmd)[2] = CELL_GCM_ENDIAN_SWAP(format); \
	(cmd) += 3; \
	} while(false)
#endif

#undef CELL_GCM_SET_VERTEX_TEXTURE_CONTROL3
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VERTEX_TEXTURE_CONTROL3(cmd, index, control3) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(control3); \
	} while(false)
#else
#define CELL_GCM_SET_VERTEX_TEXTURE_CONTROL3(cmd, index, control3) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_TEXTURE_SWIZZLE + (index) * 32, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(control3); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_VERTEX_TEXTURE_IMAGE_RECT
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VERTEX_TEXTURE_IMAGE_RECT(cmd, index, imagerect) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(imagerect); \
	} while(false)
#else
#define CELL_GCM_SET_VERTEX_TEXTURE_IMAGE_RECT(cmd, index, imagerect) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_TEXTURE_SIZE + (index) * 32, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(imagerect); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_VERTEX_TEXTURE_ADDRESS
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VERTEX_TEXTURE_ADDRESS(cmd, index, wrapu, wrapv) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(wrapu); \
	CELL_GCM_UNUSED(wrapv); \
	} while(false)
#else
#define CELL_GCM_SET_VERTEX_TEXTURE_ADDRESS(cmd, index, wrapu, wrapv) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_TEXTURE_CONTROL_1 + (index) * 32, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP((wrapu) | ((wrapv) << 8)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_VERTEX_TEXTURE_FILTER
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VERTEX_TEXTURE_FILTER(cmd, index, bias) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(bias); \
	} while(false)
#else
#define CELL_GCM_SET_VERTEX_TEXTURE_FILTER(cmd, index, bias) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_TEXTURE_FILTER + (index) * 32, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(bias); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_VERTEX_TEXTURE_CONTROL0
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VERTEX_TEXTURE_CONTROL0(cmd, index, enable, minLod, maxLod) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(enable); \
	CELL_GCM_UNUSED(minLod); \
	CELL_GCM_UNUSED(maxLod); \
	} while(false)
#else
#define CELL_GCM_SET_VERTEX_TEXTURE_CONTROL0(cmd, index, enable, minLod, maxLod) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_TEXTURE_CONTROL_2 + (index) * 32, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(((maxLod) << 7) | ((minLod) << 19) | ((enable) << 31)); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_SET_VERTEX_TEXTURE_BORDER_COLOR
#if CELL_GCM_MEASURE
#define CELL_GCM_SET_VERTEX_TEXTURE_BORDER_COLOR(cmd, index, color) do { \
	CELL_GCM_UNUSED(index); \
	CELL_GCM_UNUSED(color); \
	} while(false)
#else
#define CELL_GCM_SET_VERTEX_TEXTURE_BORDER_COLOR(cmd, index, color) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_VERTEX_PROGRAM_TEXTURE_BORDER_COLOR + (index) * 32, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(color); \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_WAIT_FOR_IDLE
#if CELL_GCM_MEASURE
#define CELL_GCM_WAIT_FOR_IDLE(cmd) do { \
	} while(false)
#else
#define CELL_GCM_WAIT_FOR_IDLE(cmd) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_WAIT_FOR_IDLE, 1); \
	(cmd)[1] = 0; \
	(cmd) += 2; \
	} while(false)
#endif

#undef CELL_GCM_PM_TRIGGER
#if CELL_GCM_MEASURE
#define CELL_GCM_PM_TRIGGER(cmd) do { \
	} while(false)
#else
#define CELL_GCM_PM_TRIGGER(cmd) do { \
	(cmd)[0] = CELL_GCM_METHOD(CELL_GCM_METHOD_ADDR_PM_TRIGGER, 1); \
	(cmd)[1] = CELL_GCM_ENDIAN_SWAP(1); \
	(cmd) += 2; \
	} while(false)
#endif
