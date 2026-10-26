#ifndef GRAPHICS_SPU_COMMON_H
#define GRAPHICS_SPU_COMMON_H

#if _MSC_VER > 1000
#pragma once
#endif

// Shuffle Masks
#define _VECTORMATH_CSHUF_A 0x10, 0x11, 0x12, 0x13
#define _VECTORMATH_CSHUF_B 0x14, 0x15, 0x16, 0x17
#define _VECTORMATH_CSHUF_C 0x18, 0x19, 0x1A, 0x1B
#define _VECTORMATH_CSHUF_D 0x1C, 0x1D, 0x1E, 0x1F
#define _VECTORMATH_CSHUF_X 0x00, 0x01, 0x02, 0x03
#define _VECTORMATH_CSHUF_Y 0x04, 0x05, 0x06, 0x07
#define _VECTORMATH_CSHUF_Z 0x08, 0x09, 0x0A, 0x0B
#define _VECTORMATH_CSHUF_W 0x0C, 0x0D, 0x0E, 0x0F
#define _VECTORMATH_CSHUF_ZERO 0x80, 0x80, 0x80, 0x80

#if defined(PLATFORM_SPU)

// specific intrinsic constants

static const qword shuffle_xxxx						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_X };
static const qword shuffle_yyyy						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_Y };
static const qword shuffle_zzzz						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_Z };
static const qword shuffle_wwww						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_W, _VECTORMATH_CSHUF_W, _VECTORMATH_CSHUF_W, _VECTORMATH_CSHUF_W };
static const qword shuffle_xa00						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_A, _VECTORMATH_CSHUF_ZERO, _VECTORMATH_CSHUF_ZERO };
static const qword shuffle_00xa						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_ZERO, _VECTORMATH_CSHUF_ZERO ,_VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_A };
static const qword shuffle_xya0						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_A, _VECTORMATH_CSHUF_ZERO };
static const qword shuffle_xzac						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_A, _VECTORMATH_CSHUF_C };
static const qword shuffle_ywbd						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_W, _VECTORMATH_CSHUF_B, _VECTORMATH_CSHUF_D };
static const qword shuffle_xyzd						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_D };
static const qword shuffle_xaxa						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_A, _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_A };
static const qword shuffle_xxaa						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_A, _VECTORMATH_CSHUF_A };
static const qword shuffle_xd00						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_D, _VECTORMATH_CSHUF_ZERO, _VECTORMATH_CSHUF_ZERO };
static const qword shuffle_xyxy						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_Y };
static const qword shuffle_zwzw						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_W, _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_W };
static const qword shuffle_xyza						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_A };
static const qword shuffle_xy00						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_ZERO, _VECTORMATH_CSHUF_ZERO };
static const qword shuffle_zw00						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_W, _VECTORMATH_CSHUF_ZERO, _VECTORMATH_CSHUF_ZERO };
static const qword shuffle_x0z0						= (qword)(vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_ZERO, _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_ZERO };

static const qword shuffle_bytes_xxxx				= (qword)(vec_uchar16) { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const qword shuffle_bytes_yyyy				= (qword)(vec_uchar16) { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
static const qword shuffle_bytes_zzzz				= (qword)(vec_uchar16) { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };
static const qword shuffle_bytes_wwww				= (qword)(vec_uchar16) { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };

static const qword shuffle_splat_byte_colours		= (qword)(vec_uchar16) { 0x03, 0x03, 0x03, 0x03, 0x07, 0x07, 0x07, 0x07, 0x0B, 0x0B, 0x0B, 0x0B, 0x0F, 0x0F, 0x0F, 0x0F };

static const qword shuffle_short_to_colour_rg0		= (qword)(vec_uchar16) { 0x01, 0x11, 0x80, 0x80, 0x03, 0x13, 0x80, 0x80, 0x05, 0x15, 0x80, 0x80, 0x07, 0x17, 0x80, 0x80};
static const qword shuffle_short_to_colour_rg1		= (qword)(vec_uchar16) { 0x09, 0x19, 0x80, 0x80, 0x0B, 0x1b, 0x80, 0x80, 0x0D, 0x1D, 0x80, 0x80, 0x0F, 0x1F, 0x80, 0x80};
static const qword shuffle_short_to_colour_ba0		= (qword)(vec_uchar16) { 0x80, 0x80, 0x01, 0x11, 0x80, 0x80, 0x03, 0x13, 0x80, 0x80, 0x05, 0x15, 0x80, 0x80, 0x07, 0x17};
static const qword shuffle_short_to_colour_ba1		= (qword)(vec_uchar16) { 0x80, 0x80, 0x09, 0x19, 0x80, 0x80, 0x0B, 0x1B, 0x80, 0x80, 0x0D, 0x1D, 0x80, 0x80, 0x0F, 0x1F};
static const qword shuffle_int_to_colour_rg			= (qword)(vec_uchar16) { 0x03, 0x13, 0x80, 0x80, 0x07, 0x17, 0x80, 0x80, 0x0B, 0x1B, 0x80, 0x80, 0x0F, 0x1F, 0x80, 0x80};
static const qword shuffle_int_to_colour_ba			= (qword)(vec_uchar16) { 0x80, 0x80, 0x03, 0x13, 0x80, 0x80, 0x07, 0x17, 0x80, 0x80, 0x0B, 0x1B, 0x80, 0x80, 0x0F, 0x1F};

static const qword shuffle_combine_focus_buffer0	= (qword)(vec_uchar16)	{ 0x13, 0x03, 0x17, 0x07, 0x1B, 0x0B, 0x1F, 0x0F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 };
static const qword shuffle_combine_focus_buffer1	= (qword)(vec_uchar16)	{ 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x13, 0x03, 0x17, 0x07, 0x1B, 0x0B, 0x1F, 0x0F };

static const qword shuffle_short_to_int0			= (qword)(vec_uchar16) {0x80, 0x80, 0x00, 0x01, 0x80, 0x80, 0x02, 0x03, 0x80, 0x80, 0x04, 0x05, 0x80, 0x80, 0x06, 0x07 };
static const qword shuffle_short_to_int1			= (qword)(vec_uchar16) {0x80, 0x80, 0x08, 0x09, 0x80, 0x80, 0x0A, 0x0B, 0x80, 0x80, 0x0C, 0x0D, 0x80, 0x80, 0x0E, 0x0F };

static const qword shuffle_int_to_short				= (qword)(vec_uchar16) {0x02, 0x03, 0x06, 0x07, 0x0A, 0x0B, 0x0E, 0x0F, 0x12, 0x13, 0x16, 0x17, 0x1a, 0x1b, 0x1e, 0x1f};

static const qword shuffle_char_to_int0				= (qword)(vec_uchar16) {0x80, 0x80, 0x80, 0, 0x80, 0x80, 0x80,1, 0x80, 0x80, 0x80,2, 0x80, 0x80, 0x80,3 };
static const qword shuffle_char_to_int1				= (qword)(vec_uchar16) {0x80, 0x80, 0x80,4, 0x80, 0x80, 0x80,5, 0x80, 0x80, 0x80,6, 0x80, 0x80, 0x80,7 };
static const qword shuffle_char_to_int2				= (qword)(vec_uchar16) {0x80, 0x80, 0x80,8, 0x80, 0x80, 0x80,9, 0x80, 0x80, 0x80,10, 0x80, 0x80, 0x80,11 };
static const qword shuffle_char_to_int3				= (qword)(vec_uchar16) {0x80, 0x80, 0x80,12, 0x80, 0x80, 0x80,13, 0x80, 0x80, 0x80,14, 0x80, 0x80, 0x80,15 };

static const qword shuffle_stencil_short_to_int0	= (qword)(vec_uchar16) { 0x00, 0x01, 0x00, 0x01, 0x02, 0x03, 0x02, 0x03, 0x04, 0x05, 0x04, 0x05, 0x06, 0x07, 0x06, 0x07 };
static const qword shuffle_stencil_short_to_int1	= (qword)(vec_uchar16) { 0x08, 0x09, 0x08, 0x09, 0x0A, 0x0B, 0x0A, 0x0B, 0x0C, 0x0D, 0x0C, 0x0D, 0x0E, 0x0F, 0x0E, 0x0F };
static const qword shuffle_z_from_rgba				= (qword)(vec_uchar16) { 0x01, 0x02, 0x03, 0x00, 0x05, 0x06, 0x07, 0x04, 0x09, 0x0A, 0x0B, 0x08, 0x0D, 0x0E, 0x0F, 0x0C };
static const qword shuffle_z_from_rgba_no_stencil	= (qword)(vec_uchar16) { 0x01, 0x02, 0x03, 0x80, 0x05, 0x06, 0x07, 0x80, 0x09, 0x0A, 0x0B, 0x80, 0x0D, 0x0E, 0x0F, 0x80 };
static const qword shuffle_z_to_short				= (qword)(vec_uchar16) { 0x00, 0x01, 0x04, 0x05, 0x08, 0x09, 0x0C, 0x0D, 0x10, 0x11, 0x14, 0x15, 0x18, 0x19, 0x1C, 0x1D };
static const qword shuffle_z_to_int0				= (qword)(vec_uchar16) { 0x00, 0x01, 0x80, 0x80, 0x02, 0x03, 0x80, 0x80, 0x04, 0x05, 0x80, 0x80, 0x06, 0x07, 0x80, 0x80 };
static const qword shuffle_z_to_int1				= (qword)(vec_uchar16) { 0x08, 0x09, 0x80, 0x80, 0x0A, 0x0B, 0x80, 0x80, 0x0C, 0x0D, 0x80, 0x80, 0x0E, 0x0F, 0x80, 0x80 };
static const qword shuffle_stencils_0				= (qword)(vec_uchar16) { 0x03, 0x07, 0x0B, 0x0F, 0x13, 0x17, 0x1B, 0x1F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 };
static const qword shuffle_stencils_1				= (qword)(vec_uchar16) { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x03, 0x07, 0x0B, 0x0F, 0x13, 0x17, 0x1B, 0x1F };

static const qword shuffle_pair_0					= (qword)(vec_uchar16) { 0x00, 0x01, 0x02, 0x03, 0x08, 0x09, 0x0A, 0x0B, 0x10, 0x11, 0x12, 0x13, 0x18, 0x19, 0x1A, 0x1B };
static const qword shuffle_pair_1					= (qword)(vec_uchar16) { 0x04, 0x05, 0x06, 0x07, 0x0C, 0x0D, 0x0E, 0x0F, 0x14, 0x15, 0x16, 0x17, 0x1C, 0x1D, 0x1E, 0x1F };

static const qword shuffle_prev						= (qword)(vec_uchar16) { 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B };
static const qword shuffle_next						= (qword)(vec_uchar16) { 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13};
static const qword shuffle_previous0				= (qword)(vec_uchar16) { 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B };
static const qword shuffle_previous1				= (qword)(vec_uchar16) { 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 };
static const qword shuffle_previous2				= (qword)(vec_uchar16) { 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13 };
static const qword shuffle_next0					= (qword)(vec_uchar16) { 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13 };
static const qword shuffle_next1					= (qword)(vec_uchar16) { 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 };
static const qword shuffle_next2					= (qword)(vec_uchar16) { 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B };

// static mask values
static const qword mask_z_bits						= (qword)(vec_uint4) { 0xFFFFFF00, 0xFFFFFF00, 0xFFFFFF00, 0xFFFFFF00 };
static const qword mask_stencil_bits				= (qword)(vec_uint4) { 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF };
static const qword mask_top_bit_remove				= (qword)(vec_uint4) { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF };

static const qword pack_low_halves					= (qword)(vec_uchar16) { 0, 1, 4, 5, 8, 9, 12, 13, 16, 17, 20, 21, 24, 25, 28, 29 };

static const qword zero_float						= (qword)(vec_float4){ 0.0f, 0.0f, 0.0f, 0.0f };
static const qword half_float						= (qword)(vec_float4){ 0.5f, 0.5f, 0.5f, 0.5f };
static const qword one_float						= (qword)(vec_float4){ 1.0f, 1.0f, 1.0f, 1.0f };
static const qword zero_u32							= (qword)(vec_uint4){ 0, 0, 0, 0 };
static const qword max_s16							= (qword)(vec_uint4){ 32767, 32767, 32767, 32767 };

// general intrinsic constants

static const vec_uchar16 SHUFFLE_XXXX	= (vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_X };
static const vec_uchar16 SHUFFLE_YYYY	= (vec_uchar16) { _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_Y };
static const vec_uchar16 SHUFFLE_ZZZZ	= (vec_uchar16) { _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_Z };
static const vec_uchar16 SHUFFLE_WWWW	= (vec_uchar16) { _VECTORMATH_CSHUF_W, _VECTORMATH_CSHUF_W, _VECTORMATH_CSHUF_W, _VECTORMATH_CSHUF_W };
static const vec_uchar16 SHUFFLE_XA00	= (vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_A, _VECTORMATH_CSHUF_ZERO, _VECTORMATH_CSHUF_ZERO };
static const vec_uchar16 SHUFFLE_00XA	= (vec_uchar16) { _VECTORMATH_CSHUF_ZERO, _VECTORMATH_CSHUF_ZERO ,_VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_A };
static const vec_uchar16 SHUFFLE_XYA0	= (vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_A, _VECTORMATH_CSHUF_ZERO };
static const vec_uchar16 SHUFFLE_XZAC	= (vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_A, _VECTORMATH_CSHUF_C };
static const vec_uchar16 SHUFFLE_YWBD	= (vec_uchar16) { _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_W, _VECTORMATH_CSHUF_B, _VECTORMATH_CSHUF_D };
static const vec_uchar16 SHUFFLE_XYZD	= (vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_D };
static const vec_uchar16 SHUFFLE_XAXA	= (vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_A, _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_A };
static const vec_uchar16 SHUFFLE_XXAA	= (vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_A, _VECTORMATH_CSHUF_A };
static const vec_uchar16 SHUFFLE_XD00	= (vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_D, _VECTORMATH_CSHUF_ZERO, _VECTORMATH_CSHUF_ZERO };
static const vec_uchar16 SHUFFLE_XYXY	= (vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_Y };
static const vec_uchar16 SHUFFLE_ZWZW	= (vec_uchar16) { _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_W, _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_W };
static const vec_uchar16 SHUFFLE_XYZA	= (vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_A };

static const vec_uchar16 SHUFFLE_XY00	= (vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_Y, _VECTORMATH_CSHUF_ZERO, _VECTORMATH_CSHUF_ZERO };
static const vec_uchar16 SHUFFLE_ZW00	= (vec_uchar16) { _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_W, _VECTORMATH_CSHUF_ZERO, _VECTORMATH_CSHUF_ZERO };
static const vec_uchar16 SHUFFLE_X0Z0	= (vec_uchar16) { _VECTORMATH_CSHUF_X, _VECTORMATH_CSHUF_ZERO, _VECTORMATH_CSHUF_Z, _VECTORMATH_CSHUF_ZERO };

static const vec_uchar16 SHUFFLE_SPLAT_BYTE_COLOURS		= { 0x03, 0x03, 0x03, 0x03, 0x07, 0x07, 0x07, 0x07, 0x0B, 0x0B, 0x0B, 0x0B, 0x0F, 0x0F, 0x0F, 0x0F };

static const vec_uchar16 SHUFFLE_SHORT_TO_COLOUR_RG0	= { 0x01, 0x11, 0x80, 0x80, 0x03, 0x13, 0x80, 0x80, 0x05, 0x15, 0x80, 0x80, 0x07, 0x17, 0x80, 0x80};
static const vec_uchar16 SHUFFLE_SHORT_TO_COLOUR_RG1	= { 0x09, 0x19, 0x80, 0x80, 0x0B, 0x1b, 0x80, 0x80, 0x0D, 0x1D, 0x80, 0x80, 0x0F, 0x1F, 0x80, 0x80};
static const vec_uchar16 SHUFFLE_SHORT_TO_COLOUR_BA0	= { 0x80, 0x80, 0x01, 0x11, 0x80, 0x80, 0x03, 0x13, 0x80, 0x80, 0x05, 0x15, 0x80, 0x80, 0x07, 0x17};
static const vec_uchar16 SHUFFLE_SHORT_TO_COLOUR_BA1	= { 0x80, 0x80, 0x09, 0x19, 0x80, 0x80, 0x0B, 0x1B, 0x80, 0x80, 0x0D, 0x1D, 0x80, 0x80, 0x0F, 0x1F};
static const vec_uchar16 SHUFFLE_INT_TO_COLOUR_RG		= { 0x03, 0x13, 0x80, 0x80, 0x07, 0x17, 0x80, 0x80, 0x0B, 0x1B, 0x80, 0x80, 0x0F, 0x1F, 0x80, 0x80};
static const vec_uchar16 SHUFFLE_INT_TO_COLOUR_BA		= { 0x80, 0x80, 0x03, 0x13, 0x80, 0x80, 0x07, 0x17, 0x80, 0x80, 0x0B, 0x1B, 0x80, 0x80, 0x0F, 0x1F};

static const vec_uchar16 SHUFFLE_COMBINE_FOCUS_BUFFER0	= (vec_uchar16){ 0x13, 0x03, 0x17, 0x07, 0x1B, 0x0B, 0x1F, 0x0F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 };
static const vec_uchar16 SHUFFLE_COMBINE_FOCUS_BUFFER1	= (vec_uchar16){ 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x13, 0x03, 0x17, 0x07, 0x1B, 0x0B, 0x1F, 0x0F };
static const vec_uchar16 SHORT_TO_INT_SHUFFLE0			= {0x80, 0x80, 0x00, 0x01, 0x80, 0x80, 0x02, 0x03, 0x80, 0x80, 0x04, 0x05, 0x80, 0x80, 0x06, 0x07 };
static const vec_uchar16 SHORT_TO_INT_SHUFFLE1			= {0x80, 0x80, 0x08, 0x09, 0x80, 0x80, 0x0A, 0x0B, 0x80, 0x80, 0x0C, 0x0D, 0x80, 0x80, 0x0E, 0x0F };
static const vec_uchar16 SHUFFLE_SHORT_TO_INT0			= {0x80, 0x80, 0x00, 0x01, 0x80, 0x80, 0x02, 0x03, 0x80, 0x80, 0x04, 0x05, 0x80, 0x80, 0x06, 0x07 };
static const vec_uchar16 SHUFFLE_SHORT_TO_INT1			= {0x80, 0x80, 0x08, 0x09, 0x80, 0x80, 0x0A, 0x0B, 0x80, 0x80, 0x0C, 0x0D, 0x80, 0x80, 0x0E, 0x0F };

static const vec_uchar16 CHAR_TO_INT_SHUFFLE0			= {0x80, 0x80, 0x80, 0, 0x80, 0x80, 0x80,1, 0x80, 0x80, 0x80,2, 0x80, 0x80, 0x80,3 };
static const vec_uchar16 CHAR_TO_INT_SHUFFLE1			= {0x80, 0x80, 0x80,4, 0x80, 0x80, 0x80,5, 0x80, 0x80, 0x80,6, 0x80, 0x80, 0x80,7 };
static const vec_uchar16 CHAR_TO_INT_SHUFFLE2			= {0x80, 0x80, 0x80,8, 0x80, 0x80, 0x80,9, 0x80, 0x80, 0x80,10, 0x80, 0x80, 0x80,11 };
static const vec_uchar16 CHAR_TO_INT_SHUFFLE3			= {0x80, 0x80, 0x80,12, 0x80, 0x80, 0x80,13, 0x80, 0x80, 0x80,14, 0x80, 0x80, 0x80,15 };

static const vec_uchar16 SHUFFLE_INT_TO_SHORT			= {0x02, 0x03, 0x06, 0x07, 0x0A, 0x0B, 0x0E, 0x0F, 0x12, 0x13, 0x16, 0x17, 0x1a, 0x1b, 0x1e, 0x1f};

static const vec_uchar16 SHUFFLE_STENCIL_SHORT_TO_INT0	= { 0x00, 0x01, 0x00, 0x01, 0x02, 0x03, 0x02, 0x03, 0x04, 0x05, 0x04, 0x05, 0x06, 0x07, 0x06, 0x07 };
static const vec_uchar16 SHUFFLE_STENCIL_SHORT_TO_INT1	= { 0x08, 0x09, 0x08, 0x09, 0x0A, 0x0B, 0x0A, 0x0B, 0x0C, 0x0D, 0x0C, 0x0D, 0x0E, 0x0F, 0x0E, 0x0F };
static const vec_uchar16 SHUFFLE_Z_FROM_RGBA			= { 0x01, 0x02, 0x03, 0x00, 0x05, 0x06, 0x07, 0x04, 0x09, 0x0A, 0x0B, 0x08, 0x0D, 0x0E, 0x0F, 0x0C };
static const vec_uchar16 SHUFFLE_Z_FROM_RGBA_NO_STENCIL	= { 0x01, 0x02, 0x03, 0x80, 0x05, 0x06, 0x07, 0x80, 0x09, 0x0A, 0x0B, 0x80, 0x0D, 0x0E, 0x0F, 0x80 };
static const vec_uchar16 SHUFFLE_Z_TOSHORT				= { 0x00, 0x01, 0x04, 0x05, 0x08, 0x09, 0x0C, 0x0D, 0x10, 0x11, 0x14, 0x15, 0x18, 0x19, 0x1C, 0x1D };
static const vec_uchar16 SHUFFLE_Z_TOINT0				= { 0x00, 0x01, 0x80, 0x80, 0x02, 0x03, 0x80, 0x80, 0x04, 0x05, 0x80, 0x80, 0x06, 0x07, 0x80, 0x80 };
static const vec_uchar16 SHUFFLE_Z_TOINT1				= { 0x08, 0x09, 0x80, 0x80, 0x0A, 0x0B, 0x80, 0x80, 0x0C, 0x0D, 0x80, 0x80, 0x0E, 0x0F, 0x80, 0x80 };
static const vec_uchar16 SHUFFLE_STENCILS_0				= { 0x03, 0x07, 0x0B, 0x0F, 0x13, 0x17, 0x1B, 0x1F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 };
static const vec_uchar16 SHUFFLE_STENCILS_1				= { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x03, 0x07, 0x0B, 0x0F, 0x13, 0x17, 0x1B, 0x1F };

static const vec_uchar16 SHUFFLE_PAIR0		= { 0x00, 0x01, 0x02, 0x03, 0x08, 0x09, 0x0A, 0x0B, 0x10, 0x11, 0x12, 0x13, 0x18, 0x19, 0x1A, 0x1B };
static const vec_uchar16 SHUFFLE_PAIR1		= { 0x04, 0x05, 0x06, 0x07, 0x0C, 0x0D, 0x0E, 0x0F, 0x14, 0x15, 0x16, 0x17, 0x1C, 0x1D, 0x1E, 0x1F };

static const vec_uchar16 SHUFFLE_PREV		= { 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B };
static const vec_uchar16 SHUFFLE_NEXT		= { 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13};
static const vec_uchar16 SHUFFLE_PREVIOUS0	= { 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B };
static const vec_uchar16 SHUFFLE_PREVIOUS1	= { 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 };
static const vec_uchar16 SHUFFLE_PREVIOUS2	= { 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13 };
static const vec_uchar16 SHUFFLE_NEXT0		= { 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13 };
static const vec_uchar16 SHUFFLE_NEXT1		= { 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17 };
static const vec_uchar16 SHUFFLE_NEXT2		= { 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B };

// static mask values
static const vec_uint4 MASK_Z_BITS			= { 0xFFFFFF00, 0xFFFFFF00, 0xFFFFFF00, 0xFFFFFF00 };
static const vec_uint4 MASK_STENCIL_BITS	= { 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF };
static const vec_uint4 MASK_TOP_BIT_REMOVE	= { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF };

// static constants
static const vec_float4 zerof	= (vec_float4) { 0.0f, 0.0f, 0.0f, 0.0f };
static const vec_float4 halff	= (vec_float4) { 0.5f, 0.5f, 0.5f, 0.5f };
static const vec_float4 onef	= (vec_float4) { 1.0f, 1.0f, 1.0f, 1.0f };
static const vec_uint4 maxu8	= { 32767, 32767, 32767, 32767 };
static const vec_ushort8 zero8	= { 0, 0, 0, 0, 0, 0, 0, 0 };

static vec_uchar16 PACK_LOW_HALVES = (vec_uchar16) { 0, 1, 4, 5, 8, 9, 12, 13, 16, 17, 20, 21, 24, 25, 28, 29 };

static inline const qword si_splat(const F32 f) __attribute__((pure, always_inline, const));
static inline const qword si_splat(const F32 f)
{
	const qword shuf_xxxx			= si_ila(0x010203);	/* [STOVE] Should save off 4 cycles from a splat =), 2 cycle odd pipe for constant formation rather than 6 cycle lqd (ODD) from ls */
	const qword q					= si_from_float(f);	/* Do this once just incase it generate unaligned loads */
	return si_shufb(q, q, shuf_xxxx);
}

namespace Spu
{

INLINE void* MatchAlignment16(void* pAddress, const U32 address)
{
	U8* pAlignedAddress = (U8*)si_to_ptr((qword)spu_sel((vec_uint4)si_from_ptr(pAddress), (vec_uint4)si_from_ptr((void*)address), spu_splats(15u)));
	if(pAlignedAddress < pAddress)
		pAlignedAddress += 16;
	return pAlignedAddress;
}

INLINE void* MatchAlignment128(void* pAddress, const U32 address)
{
	U8* pAlignedAddress = (U8*)si_to_ptr((qword)spu_sel((vec_uint4)si_from_ptr(pAddress), (vec_uint4)si_from_ptr((void*)address), spu_splats(127u)));
	if(pAlignedAddress < pAddress)
		pAlignedAddress += 128;
	return pAlignedAddress;
}

INLINE void WriteByte(const U32 address, const U8 value)
{
	vec_uchar16 storage = spu_splats(value);
	U32* pAligned = (U32*)MatchAlignment16((U32*)&storage, address);
	cellDmaSmallPutf(pAligned, address, sizeof(U8), 0, 0, 0);
	cellDmaWaitTagStatusAll(1 << 0);
}

INLINE void WriteWord(const U32 address, const U32 value)
{
	vec_uint4 storage = spu_splats(value);
	U32* pAligned = (U32*)MatchAlignment16((U32*)&storage, address);
	cellDmaSmallPutf(pAligned, address, sizeof(U32), 0, 0, 0);
	cellDmaWaitTagStatusAll(1 << 0);
}

INLINE void HardSync(const U32 address, const U32 tag)
{
	vec_uint4 buffer;
	void* pAlignedAddress = (void*)si_to_ptr((qword)spu_sel((vec_uint4)si_from_ptr(&buffer), (vec_uint4)si_from_ptr((void*)address), spu_splats(15u)));
	cellDmaSmallGetf(pAlignedAddress, address, 1, tag, 0, 0);
	cellDmaWaitTagStatusAll(1 << tag);
}

}; // namespace Spu

//! Get view space Z value.
//@param in 32bit Z buffer value.
//@param near Near plane.
//@param far Far plane.an
//@return View space Z value.
static inline const qword get_view_z_value(const qword z, const qword far_minus_near, const qword far) __attribute__((pure, const, always_inline));
static inline const qword get_view_z_value(const qword z, const qword far_minus_near, const qword far)
{
	qword val 			= si_andi(z, -0x0100);										// EVEN 2, mask z bits
	qword z_val_c		= si_cuflt(val, 0x20);										// EVEN 7, convert to floats
	qword view_z		= si_fma(z_val_c, far_minus_near, far);						// EVEN 6, (z*(far-near)) + far
	qword view_z_inv	= si_frest(view_z);											// ODD  4, 1/view_z
	qword result		= si_fi(view_z, view_z_inv);								// EVEN, 7
	return result;
}

//! Get view space Z value.
//@param in 4 32bit Z buffer values.
//@param near Near plane.
//@param far Far plane.
//@return View space Z value.
static inline const qword get_view_z_value4(const qword z, const qword far_minus_near, const qword far) __attribute__((pure, const, always_inline));
static inline const qword get_view_z_value4(const qword z, const qword far_minus_near, const qword far)
{
	qword shf_xxxx		= si_ila(0x010203);											// ODD, 2, load shuffle mask.
	qword val 			= si_andi(z, -0x0100);										// EVEN 2, mask z bits.
	qword farz			= si_shufb(far, far, shf_xxxx);								// ODD 4, swizzle far.xxxx.
	qword z_val			= si_cuflt(val, 0x20);										// EVEN 7, convert to floats.
	qword nearz			= si_shufb(far_minus_near, far_minus_near, shf_xxxx);		// ODD 4, swizzle near.xxxx.
	qword view_z		= si_fma(z_val, nearz, farz);								// EVEN 6, (near*far) + z.
	qword view_z_inv	= si_frest(view_z);											// ODD  4, 1/view_z.
	qword result		= si_fi(view_z, view_z_inv);								// EVEN, 7
	return result;
}

//! Get view space Z value from lower 16 bits of input.
//@param in 32 bit value, linear Z in lower 16 bits
//@param far Far plane.
//@return View space Z value.
static inline const qword get_view_z_value_linear_16(const qword z, const qword far) __attribute__((pure, const, always_inline));
static inline const qword get_view_z_value_linear_16(const qword z, const qword far)
{
	qword shuf			= si_ilhu(0x8080);
		  shuf			= si_iohl(shuf, 0x0300);
	qword val 			= si_shufb(z, z, shuf);										// EVEN 2, mask z bits
	qword z_val_c		= si_cuflt(val, 0x10);										// EVEN 7, convert to floats
	qword result		= si_fm(z_val_c, far);										// EVEN ?, multiply by far plane
	return result;
}

//! Get view space Z valuefrom lower 16 bits of input.
//@param in 4 32bit values, linear Z in lower 16 bits.
//@param far Far plane.
//@return View space Z value.
static inline const qword get_view_z_value4_linear_16(const qword z, const qword far) __attribute__((pure, const, always_inline));
static inline const qword get_view_z_value4_linear_16(const qword z, const qword far)
{
	const vector unsigned char _shuf = 
	 { 0x80, 0x80, 0x03, 0x00, 0x80, 0x80, 0x07, 0x04, 0x80, 0x80, 0x0b, 0x08, 0x80, 0x80, 0x0f, 0x0c };
	qword shf_xxxx		= si_ila(0x010203);											// ODD, 2, load shuffle pattern.
	qword shuf			= (const qword)_shuf;										// ODD, 6, load shuffle pattern.
	qword val 			= si_shufb(z, z, shuf);										// EVEN 2, mask z bits
	qword farz			= si_shufb(far, far, shf_xxxx);								// ODD 4, swizzle far.xxxx.
	qword z_val			= si_cuflt(val, 0x10);										// EVEN 7, convert to floats.
	qword result		= si_fm(z_val, farz);										// EVEN ?, multiply by far plane
	return result;
}

#endif // PLATFORM_SPU

#endif // GRAPHICS_SPU_COMMON_H
