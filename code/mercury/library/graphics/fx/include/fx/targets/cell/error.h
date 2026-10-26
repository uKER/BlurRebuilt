/*   SCE CONFIDENTIAL
PLAYSTATION(R)3 Programmer Tool Runtime Library 091.006
*   Copyright (C) 2006 Sony Computer Entertainment Inc.
*   All Rights Reserved.
*/

#ifndef __CELL_CELL_ERROR_H__
#define __CELL_CELL_ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*J
 * CELL エラーコードの構造
 *
 *  0   1         3 4               15 16                   31
 * +---+-----------+------------------+-----------------------+
 * | E | Reserved  | F A C I L I T Y  |  E R R O R   C O D E  |
 * +---+-----------+------------------+-----------------------+
 *
 *  bit 0: エラーの有無(Error)
 *      1= エラー(bit 0〜31は以下の定義に従う)
 *      0= 成功(bit 0〜31も 0 にする)
 *
 *  bit 1-3: Reserved
 *      00= 常にゼロ固定
 *
 *  bit 4-15: ファシリティ(Facility)
 *      0x000= CELL_ERROR_FACILITY_NULL
 *      0x001= CELL_ERROR_FACILITY_SYSTEM_SERVICE
 *      0x002= CELL_ERROR_FACILITY_SYSTEM_UTILITY
 *         :
 *      0x800〜 予約領域
 *
 *  bit 16-31: エラーコード
 *      具体的な意味は、ファシリティコードごとに異なります。
 *      エラーコード部の値が同じでも、ファシリティコードが異なれば
 *      別個のエラーナンバーとして扱われることにご注意ください。
 */
/*E
 * CELL error code structure
 *
 *  0   1         3 4               15 16                   31
 * +---+-----------+------------------+-----------------------+
 * | E | Reserved  | F A C I L I T Y  |  E R R O R   C O D E  |
 * +---+-----------+------------------+-----------------------+
 *
 *  bit 0: Error
 *      1= Error
 *      0= Success
 *
 *  bit 1-3: Reserved
 *      00= always zero
 *
 *  bit 4-15: Facility
 *      0x000= CELL_ERROR_FACILITY_NULL
 *      0x001= CELL_ERROR_FACILITY_SYSTEM_SERVICE
 *      0x002= CELL_ERROR_FACILITY_SYSTEM_UTILITY
 *         :
 *      0x800- Reserved
 *
 *  bit 16-31: Error code
 *      Concrete meanings differ for every facility code.
 *      If facility codes differ, with the same value of an error code part
 *      must be treated as a different meanings.
 */

#ifndef __ASSEMBLER__
typedef int CellError;
#endif // __ASSEMBLER__

/*J
 * 汎用正常終了の値
 */
/*E
 * Generic succeeded code
 */
#define CELL_OK								0

/*J
 * エラーフラグの値
 */
/*E
 * Error flag and Critical Error flag
 */
#define CELL_ERROR_ERROR_FLAG				0x80000000

/*J
 * ユーティリティマクロ
 */
/*E
 * Utility macro
 */
#ifdef __cplusplus
	/* Static-cast for C++ */
	#define CELL_ERROR_CAST(err) (static_cast<int>(err))
#elif __ASSEMBLER__
	/* Cast for Assembler */
	#define CELL_ERROR_CAST(err) err
#else
	/* Cast for C */
	#define CELL_ERROR_CAST(err) ((int)(err))
#endif /* __cplusplus */

#define CELL_ERROR_IS_FAILURE(_err)			(((_err) & CELL_ERROR_ERROR_FLAG) == CELL_ERROR_ERROR_FLAG)
#define CELL_ERROR_IS_SUCCESS(_err)			(!((_err) & CELL_ERROR_ERROR_FLAG))
#define CELL_ERROR_GET_FACILITY(_err)		(((_err) >> 16) & 0xFFF)
#define CELL_ERROR_MAKE_ERROR(_fac, _sts)	CELL_ERROR_CAST(CELL_ERROR_ERROR_FLAG | ((_fac)<<16) | (_sts))
#define CELL_ERROR_CHECK_ERROR(_err)		/* do nothing */

/*J
 * ファシリティコードの定義
 */
/*E
 * Definition of facility codes
 */

#define CELL_ERROR_FACILITY_NULL				0x000
#define CELL_ERROR_FACILITY_SYSTEM_SERVICE		0x001
#define CELL_ERROR_FACILITY_SYSTEM_UTILITY		0x002
#define CELL_ERROR_FACILITY_SYSTEM_MIDDLEWARE	0x003

#define CELL_ERROR_FACILITY_USB					0x011
#define CELL_ERROR_FACILITY_HID					0x012
#define CELL_ERROR_FACILITY_NET					0x013

#define CELL_ERROR_FACILITY_MICCAM				0x014

#define CELL_ERROR_FACILITY_GFX					0x021
#define CELL_ERROR_FACILITY_PSGL				0x022

#define CELL_ERROR_FACILITY_SOUND				0x031

#define CELL_ERROR_FACILITY_SPU					0x041

#define CELL_ERROR_FACILITY_DEBUG				0x051
#define CELL_ERROR_FACILITY_PERFORMANCE			0x052
#define CELL_ERROR_FACILITY_MEMGLUE				0x053
#define CELL_ERROR_FACILITY_FONT				0x054

#define CELL_ERROR_FACILITY_CODEC				0x061

#define CELL_ERROR_FACILITY_HTTP				0x071
#define CELL_ERROR_FACILITY_FT2D				0x072
#define CELL_ERROR_FACILITY_VMATH				0x073
#define CELL_ERROR_FACILITY_SSL					0x074
#define CELL_ERROR_FACILITY_IME					0x075

#define CELL_ERROR_FACILITY_PSSG				0x081
#define CELL_ERROR_FACILITY_FW					0x082

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CELL_CELL_ERROR_H__ */

