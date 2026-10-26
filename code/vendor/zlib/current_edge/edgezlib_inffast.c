/* inffast.c -- fast decoding
 * Copyright (C) 1995-2004 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

//////////////////////////////////////////////////////////////////////////
//
//	Edge Zlib SPU Version
//	This file shows a C version of the SPU code which is implemented in
//	assembler in "edgezlib_inffast.spu.s"
//
//////////////////////////////////////////////////////////////////////////

#include "edgezlib_zutil.h"
#include "edgezlib_inftrees.h"
#include "edgezlib_inflate.h"
#include "edgezlib_inffast.h"
#include <spu_intrinsics.h>

#include "edge_assert.h"

#undef PREDICT
#define PREDICT( cond, expected )	__builtin_expect( ( cond ), ( expected ) )

#ifndef ASMINF

#define INIT_INPUT_READER( _numUsedBitsInInputQword, _pInputQword, _hold128, _hold32, _numUsedBitsInHold128, _pInputData, _startHoldData, _numBitsInStartHold )	\
	do																												\
	{																												\
		/*_hold128 = the combination of whatever few bits [0,7] are in startHoldData, followed by data	*/			\
		/*			read from pInputData [121,128].														*/			\
		/*_numUsedBitsInHold128 = 0; because nothing has been used yet and we have a full cache line	*/			\
																													\
		/*Firstly, grab the hold128 located at pInputData												*/			\
		/*This may up off by 7 bits from our actual read point, but those bits may not be readable,		*/			\
		/* so we must use them from startHoldData instead.												*/			\
		_pInputQword				= (const unsigned char*)(((uint32_t)_pInputData) & ~0xF);						\
		_numUsedBitsInInputQword	= (((uint32_t)_pInputData) & 0xF) * 8;											\
		qword _alignedHold128		= (qword) GetHold128( _pInputQword, _numUsedBitsInInputQword );					\
																													\
		uint32_t _mask32			= (1 << _numBitsInStartHold) - 1;												\
		qword _prefSlotMask			= si_fsmbi( 0xF000 );															\
		qword _mask128				= si_from_uint( _mask32 );														\
		_mask128					= si_and( _mask128, _prefSlotMask );											\
																													\
		qword _rotHold128			= spu_rlqw( _alignedHold128, _numBitsInStartHold );								\
		_rotHold128					= spu_rlqwbytebc( _rotHold128, _numBitsInStartHold );/*in case numBits=8*/		\
		_hold128					= si_selb( _rotHold128, si_from_uint(_startHoldData), _mask128 );				\
		_hold32						= spu_extract( (vec_uint4)_hold128, 0 );										\
																													\
		_numUsedBitsInHold128		= 0;																			\
																													\
		/*Although we mustn't rely on reading backwards in order to access the preceding bits which		*/			\
		/*	we've already got in startHold, we should set up _pInputQword and _numUsedBitsInInputQword	*/			\
		/*	suitably as if we were going to read them.													*/			\
		/*																								*/			\
		/*_pInputQword = the qword containing the pInputByte used on the next call to GetHold128().		*/			\
		/*				ie. for init, this shouldn't be rounded until after the call to GetHold128().	*/			\
		/*_numUsedBitsInInputQword = _numBitsInStartHold + NumBytesLeftInCurrQword(_pInputByte);		*/			\
																													\
		if ( _numBitsInStartHold )																					\
		{																											\
			_pInputData--;																							\
			_pInputQword					= (const unsigned char*)(((uint32_t)_pInputData) & ~0xF);				\
			_numUsedBitsInInputQword		= ((((uint32_t)_pInputData) & 0xF) * 8) + (8-_numBitsInStartHold);		\
		}																											\
	} while ( 0 )

/*void InitInputReader( const unsigned char* pInputData, uint32_t startHoldData, uint32_t numBitsInStartHold );
void InitInputReader( const unsigned char* pInputData, uint32_t startHoldData, uint32_t numBitsInStartHold )
{
	//hold128 = the combination of whatever few bits [0,7] are in startHoldData, followed by data
	//			read from pInputData [121,128].
	//numUsedBitsInHold128 = 0; because nothing has been used yet and we have a full cache line

	//Firstly, grab the hold128 located at pInputData
	//This may up off by 7 bits from our actual read point, but those bits may not be readable,
	// so we must use them from startHoldData instead.
	pInputQword				= (const unsigned char*)(((uint32_t)pInputData) & ~0xF);
	numUsedBitsInInputQword	= (((uint32_t)pInputData) & 0xF) * 8;
	qword alignedHold128	= (qword) GetHold128( pInputQword, numUsedBitsInInputQword );

	uint32_t mask32 = (1 << numBitsInStartHold) - 1;
	qword prefSlotMask		= si_fsmbi( 0xF000 );
	qword mask128			= si_from_uint( mask32 );
	mask128					= si_and( mask128, prefSlotMask );

	qword rotHold128		= spu_rlqw( alignedHold128, numBitsInStartHold );
	hold128					= si_selb( rotHold128, si_from_uint(startHoldData), mask128 );

	numUsedBitsInHold128	= 0;

	//Although we mustn't rely on reading backwards in order to access the preceding bits which
	//	we've already got in startHold, we should set up pInputQword and numUsedBitsInInputQword
	//	suitably as if we were going to read them.
	//
	//pInputQword = the qword containing the pInputByte used on the next call to GetHold128().
	//				ie. for init, this shouldn't be rounded until after the call to GetHold128().
	//numUsedBitsInInputQword = numBitsInStartHold + NumBytesLeftInCurrQword(pInputByte);

	if ( numBitsInStartHold )
	{
		pInputData--;
		pInputQword						= (const unsigned char*)(((uint32_t)pInputData) & ~0xF);
		numUsedBitsInInputQword			= ((((uint32_t)pInputData) & 0xF) * 8) + (8-numBitsInStartHold);
	}
}*/


#define USE_BITS( _numBitsToUse, _hold32, _numUsedBitsInHold128, _hold128 )						\
	do																												\
	{																												\
		_numUsedBitsInHold128	= _numUsedBitsInHold128 + _numBitsToUse;											\
		uint32_t _holdShift		= 128-_numBitsToUse;																\
		vec_uchar16 _hold128X	= spu_rlqwbytebc( (vec_uchar16) _hold128, _holdShift );								\
		_hold128				= (qword) spu_rlqw( _hold128X, _holdShift );										\
		_hold32					= spu_extract( (vec_uint4)_hold128, 0 );											\
	} while ( 0 )


//Make sure we've got at least 15 bits in hold32
#define UPDATE_BITS_15( _hold32, _numUsedBitsInHold128, _hold128, _numUsedBitsInInputQword, _pInputQword )	\
	do																												\
	{																												\
		const uint32_t _kMaxUsedHoldBits = 112;																		\
		if ( PREDICT( _numUsedBitsInHold128 > _kMaxUsedHoldBits, 0 ) )												\
		{																											\
			uint32_t _newTotBits		= _numUsedBitsInHold128 + _numUsedBitsInInputQword;							\
			_pInputQword				+= (_newTotBits & 0x80) >> 3;												\
			_numUsedBitsInInputQword	= _newTotBits & 0x7F;														\
			_hold128					= (qword) GetHold128( _pInputQword, _numUsedBitsInInputQword );				\
			_numUsedBitsInHold128		= 0;																		\
			_hold32					= spu_extract( (vec_uint4)_hold128, 0 );										\
		}																											\
	} while ( 0 )

static inline vec_uchar16 GetHold128( const void* pIn, uint32_t numBits ) __attribute__((always_inline));
vec_uchar16 GetHold128( const void* pIn, uint32_t numUsedBitsInInputQword )
{
	uint32_t bitShift = 128 - numUsedBitsInInputQword;
#ifdef _DEBUG
	EDGE_ASSERT( bitShift <= 128 );
	EDGE_ASSERT( bitShift > 0 );
	EDGE_ASSERT( (((uint32_t)(pIn)) & 0xF) == 0 );
#endif

	vec_uchar16 vecCurr = ((vec_uchar16*) pIn)[0];
	vec_uchar16 vecNext = ((vec_uchar16*) pIn)[1];

	const vec_uchar16 endianSwap	= (vec_uchar16)(vec_uint4){ 0x0F0E0D0C, 0x0B0A0908, 0x07060504, 0x03020100 };
	vec_uchar16 mask128				= (vec_uchar16)(vec_uint4){ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };

	//Endian swap the data
	vecCurr = spu_shuffle( vecCurr, vecCurr, endianSwap );
	vecNext = spu_shuffle( vecNext, vecNext, endianSwap );

	//rotate the bytes
	vecCurr = spu_rlqwbytebc( vecCurr, bitShift );
	vecNext = spu_rlqwbytebc( vecNext, bitShift );
	mask128 = spu_slqwbytebc( mask128, bitShift );

	//rotate some bits
	vecCurr = spu_rlqw( vecCurr, bitShift );
	vecNext = spu_rlqw( vecNext, bitShift );
	mask128 = spu_slqw( mask128, bitShift );

	//combine the two qwords to make our 128 bits of data
	vec_uchar16 hold128 = spu_sel( vecCurr, vecNext, mask128 );

	//hold128 is rotated so that the bottom bits which we will be interested in are ready in the prefered slot
	hold128 = spu_rlqwbyte( hold128, 12 );

	return hold128;
}

static inline unsigned char* StoreByte( unsigned char* pOut, unsigned char value ) __attribute__((always_inline));
unsigned char* StoreByte( unsigned char* pOut, unsigned char value )
{
    *(pOut)++ = value;
	return pOut;
}


//CopyBytes2 is for copying from output to output, and handles overlapping etc..
//CopyBytes is for copying from window to output (ie. necessarily not overlapping)
//This can be optimized more

static inline unsigned char* CopyBytes( unsigned char* pDest, const unsigned char* pSource, unsigned int numBytes ) __attribute__((always_inline));
static inline unsigned char* CopyBytes2( unsigned char* pOut, const unsigned char* pFrom, unsigned int backwardsDist, unsigned int numBytes ) __attribute__((always_inline));

unsigned char* CopyBytes( unsigned char* pDest, const unsigned char* pSource, unsigned int numBytes )
{
	for ( unsigned int i = 0; i < numBytes; ++i )
	{
		pDest[i] = pSource[i];
	}

	return &pDest[numBytes];
}

unsigned char* CopyBytes2( unsigned char* pOut, const unsigned char* pFrom, unsigned int backwardsDist, unsigned int numBytes )
{
	//const unsigned char* pFrom = pOut - backwardsDist;			/* where to copy match from */ /* copy direct from output */

	//Mainline case is non-overlapping and copying less than 16 bytes
	if ( PREDICT( (backwardsDist >= numBytes) && (numBytes <= 16), 1 ) )
	{
		//Load current and next input data
		vec_uchar16 vIn0		= ((vec_uchar16*)pFrom)[0];
		vec_uchar16 vIn1		= ((vec_uchar16*)pFrom)[1];
		vec_uchar16 maskIn128	= (vec_uchar16)(vec_uint4){ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
		vec_uchar16 maskOut128	= (vec_uchar16)(vec_uint4){ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };

		//rotate both qwords
		vIn0 = spu_rlqwbyte( vIn0, (uint32_t)pFrom );
		vIn1 = spu_rlqwbyte( vIn1, (uint32_t)pFrom );

		//create a select mask
		maskIn128 = spu_slqwbyte( maskIn128, 0xF & (uint32_t)pFrom );

		//create qword of input data
		vec_uchar16 input128A = spu_sel( vIn1, vIn0, maskIn128 );

		//Load current output data
		vec_uchar16 vOut0 = ((vec_uchar16*)pOut)[0];
		vec_uchar16 vOut1 = ((vec_uchar16*)pOut)[1];	//It's tempting to just store a qword, but potentially that would write ahead invalidly

		//Create insertion mask for output data
		//Need to take account of length
		uint32_t shiftA = (16-numBytes);
		vec_uchar16 maskOut128A = spu_rlmaskqwbyte( maskOut128, -shiftA );	//Shift right
		maskOut128A = spu_slqwbyte( maskOut128A, shiftA );		//Shift back to the left, so right bits have been zeroed
		vec_uchar16 maskOut0 = spu_rlmaskqwbyte( maskOut128A, -((uint32_t)pOut & 0xF) );
		vec_uchar16 maskOut1 = spu_slqwbyte( maskOut128A, 16 - ((uint32_t)pOut & 0xF) );

		//rotate input qword to output alignment
		input128A = spu_rlqwbyte( input128A, 16-(uint32_t)pOut );

		//mask input qword bytes into output qword 0
		vOut0 = spu_sel( vOut0, input128A, maskOut0 );

		//mask input qword bytes into output qword 1
		vOut1 = spu_sel( vOut1, input128A, maskOut1 );

		//Store output qwords
		((vec_uchar16*)pOut)[0] = vOut0;
		((vec_uchar16*)pOut)[1] = vOut1;

#ifdef _DEBUG
		vec_uchar16* pOutStored = (vec_uchar16*) pOut;
		do
		{	
			--numBytes;
			*(pOut)++ = *(pFrom)++;
		} while ( PREDICT( numBytes > 0, 1 ));

		vec_uchar16 vRealOut0 = pOutStored[0];
		vec_uchar16 vRealOut1 = pOutStored[1];
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut0, 0 ) == spu_extract( (vec_uint4)vOut0, 0 ) );
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut0, 1 ) == spu_extract( (vec_uint4)vOut0, 1 ) );
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut0, 2 ) == spu_extract( (vec_uint4)vOut0, 2 ) );
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut0, 3 ) == spu_extract( (vec_uint4)vOut0, 3 ) );
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut1, 0 ) == spu_extract( (vec_uint4)vOut1, 0 ) );
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut1, 1 ) == spu_extract( (vec_uint4)vOut1, 1 ) );
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut1, 2 ) == spu_extract( (vec_uint4)vOut1, 2 ) );
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut1, 3 ) == spu_extract( (vec_uint4)vOut1, 3 ) );
#else
		pOut += numBytes;
#endif
	}
/*	else if ( PREDICT( (backwardsDist >= numBytes) && (numBytes <= 64), 1 ) )
	{
		//Load current and next input data
		vec_uchar16 vIn0		= ((vec_uchar16*)pFrom)[0];
		vec_uchar16 vIn1		= ((vec_uchar16*)pFrom)[1];
		vec_uchar16 vIn2		= ((vec_uchar16*)pFrom)[2];
		vec_uchar16 vIn3		= ((vec_uchar16*)pFrom)[3];
		vec_uchar16 vIn4		= ((vec_uchar16*)pFrom)[4];
		vec_uchar16 maskIn128	= (vec_uchar16)(vec_uint4){ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
		vec_uchar16 maskOut128	= (vec_uchar16)(vec_uint4){ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };

		//rotate qwords
		vIn0 = spu_rlqwbyte( vIn0, (uint32_t)pFrom );
		vIn1 = spu_rlqwbyte( vIn1, (uint32_t)pFrom );
		vIn2 = spu_rlqwbyte( vIn2, (uint32_t)pFrom );
		vIn3 = spu_rlqwbyte( vIn3, (uint32_t)pFrom );
		vIn4 = spu_rlqwbyte( vIn4, (uint32_t)pFrom );

		//create a select mask
		maskIn128 = spu_slqwbyte( maskIn128, 0xF & (uint32_t)pFrom );

		//create 2 qwords of input data
		vec_uchar16 input128A = spu_sel( vIn1, vIn0, maskIn128 );
		vec_uchar16 input128B = spu_sel( vIn2, vIn1, maskIn128 );
		vec_uchar16 input128C = spu_sel( vIn3, vIn2, maskIn128 );
		vec_uchar16 input128D = spu_sel( vIn4, vIn3, maskIn128 );

		//Load current output data
		vec_uchar16 vOut0 = ((vec_uchar16*)pOut)[0];
		vec_uchar16 vOut1 = ((vec_uchar16*)pOut)[1];	//It's tempting to just store a qword, but potentially that would write ahead invalidly
		vec_uchar16 vOut2 = ((vec_uchar16*)pOut)[2];
		vec_uchar16 vOut3 = ((vec_uchar16*)pOut)[3];
		vec_uchar16 vOut4 = ((vec_uchar16*)pOut)[4];

		//Create insertion mask for output data
		//Need to take account of length
		uint32_t shiftA = (16-numBytes);
		uint32_t shiftB = (32-numBytes);
		uint32_t shiftC = (48-numBytes);
		uint32_t shiftD = (64-numBytes);
		if (numBytes <= 16)	//I could be more intelligent here, but the compiler folds them anyway for now
		{
			shiftB = 16;
			shiftC = 16;
			shiftD = 16;
		}
		else if (numBytes <= 32)
		{
			shiftA = 0;
			shiftC = 16;
			shiftD = 16;
		}
		else if (numBytes <= 48)
		{
			shiftA = 0;
			shiftB = 0;
			shiftD = 16;
		}
		else
		{
			shiftA = 0;
			shiftB = 0;
			shiftC = 0;
		}
		vec_uchar16 maskOut128A = spu_rlmaskqwbyte( maskOut128, -shiftA );	//Shift right
		vec_uchar16 maskOut128B = spu_rlmaskqwbyte( maskOut128, -shiftB );	//Shift right
		vec_uchar16 maskOut128C = spu_rlmaskqwbyte( maskOut128, -shiftC );	//Shift right
		vec_uchar16 maskOut128D = spu_rlmaskqwbyte( maskOut128, -shiftD );	//Shift right
		maskOut128A = spu_slqwbyte( maskOut128A, shiftA );		//Shift back to the left, so right bits have been zeroed
		maskOut128B = spu_slqwbyte( maskOut128B, shiftB );		//Shift back to the left, so right bits have been zeroed
		maskOut128C = spu_slqwbyte( maskOut128C, shiftC );		//Shift back to the left, so right bits have been zeroed
		maskOut128D = spu_slqwbyte( maskOut128D, shiftD );		//Shift back to the left, so right bits have been zeroed
		vec_uchar16 maskOut0 = spu_rlmaskqwbyte( maskOut128A, -((uint32_t)pOut & 0xF) );
		vec_uchar16 maskOut1 = spu_slqwbyte( maskOut128A, 16 - ((uint32_t)pOut & 0xF) );
		vec_uchar16 maskOut1_ = spu_rlmaskqwbyte( maskOut128B, -((uint32_t)pOut & 0xF) );	//combine?
		vec_uchar16 maskOut2 = spu_slqwbyte( maskOut128B, 16 - ((uint32_t)pOut & 0xF) );
		vec_uchar16 maskOut2_ = spu_rlmaskqwbyte( maskOut128C, -((uint32_t)pOut & 0xF) );	//combine?
		vec_uchar16 maskOut3 = spu_slqwbyte( maskOut128C, 16 - ((uint32_t)pOut & 0xF) );
		vec_uchar16 maskOut3_ = spu_rlmaskqwbyte( maskOut128D, -((uint32_t)pOut & 0xF) );	//combine?
		vec_uchar16 maskOut4 = spu_slqwbyte( maskOut128D, 16 - ((uint32_t)pOut & 0xF) );


		//rotate input qwords to output alignment
		input128A = spu_rlqwbyte( input128A, 16-(uint32_t)pOut );
		input128B = spu_rlqwbyte( input128B, 16-(uint32_t)pOut );
		input128C = spu_rlqwbyte( input128C, 16-(uint32_t)pOut );
		input128D = spu_rlqwbyte( input128D, 16-(uint32_t)pOut );
		
		vOut1 = spu_sel( vOut1, input128A, maskOut1 );	//mask input qwordA bytes into output qword 1
		vOut2 = spu_sel( vOut2, input128B, maskOut2 );	//mask input qwordB bytes into output qword 2
		vOut3 = spu_sel( vOut3, input128C, maskOut3 );	//mask input qwordC bytes into output qword 3
		vOut4 = spu_sel( vOut4, input128D, maskOut4 );	//mask input qwordD bytes into output qword 4
		vOut0 = spu_sel( vOut0, input128A, maskOut0 );	//mask input qwordA bytes into output qword 0
		vOut1 = spu_sel( vOut1, input128B, maskOut1_ );	//mask input qwordB bytes into output qword 1
		vOut2 = spu_sel( vOut2, input128C, maskOut2_ );	//mask input qwordC bytes into output qword 2
		vOut3 = spu_sel( vOut3, input128D, maskOut3_ );	//mask input qwordD bytes into output qword 3

		//Store output qwords
		((vec_uchar16*)pOut)[0] = vOut0;
		((vec_uchar16*)pOut)[1] = vOut1;
		((vec_uchar16*)pOut)[2] = vOut2;
		((vec_uchar16*)pOut)[3] = vOut3;
		((vec_uchar16*)pOut)[4] = vOut4;

#ifdef _DEBUG
		vec_uchar16* pOutStored = (vec_uchar16*) pOut;
		do
		{	
			--numBytes;
			*(pOut)++ = *(pFrom)++;
		} while ( PREDICT( numBytes > 0, 1 ));

		vec_uchar16 vRealOut0 = pOutStored[0];
		vec_uchar16 vRealOut1 = pOutStored[1];
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut0, 0 ) == spu_extract( (vec_uint4)vOut0, 0 ) );
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut0, 1 ) == spu_extract( (vec_uint4)vOut0, 1 ) );
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut0, 2 ) == spu_extract( (vec_uint4)vOut0, 2 ) );
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut0, 3 ) == spu_extract( (vec_uint4)vOut0, 3 ) );
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut1, 0 ) == spu_extract( (vec_uint4)vOut1, 0 ) );
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut1, 1 ) == spu_extract( (vec_uint4)vOut1, 1 ) );
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut1, 2 ) == spu_extract( (vec_uint4)vOut1, 2 ) );
		EDGE_ASSERT( spu_extract( (vec_uint4)vRealOut1, 3 ) == spu_extract( (vec_uint4)vOut1, 3 ) );
#else
		pOut += numBytes;
#endif
	}
*/	else if ( PREDICT( backwardsDist >= numBytes, 1 ) )
	{
		vec_uchar16 maskIn128	= (vec_uchar16)(vec_uint4){ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };

		//create a select mask
		maskIn128 = spu_slqwbyte( maskIn128, 0xF & (uint32_t)pFrom );

		do
		{
			uint32_t copySizeThisLoop = (numBytes > 16) ? 16 : numBytes;

			//Load current and next input data
			vec_uchar16 vIn0		= ((vec_uchar16*)pFrom)[0];
			vec_uchar16 vIn1		= ((vec_uchar16*)pFrom)[1];

			vec_uchar16 maskOut128	= (vec_uchar16)(vec_uint4){ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };

			//rotate both qwords
			vIn0 = spu_rlqwbyte( vIn0, (uint32_t)pFrom );
			vIn1 = spu_rlqwbyte( vIn1, (uint32_t)pFrom );

			//create qword of input data
			vec_uchar16 input128A = spu_sel( vIn1, vIn0, maskIn128 );

			//Load current output data
			vec_uchar16 vOut0 = ((vec_uchar16*)pOut)[0];
			vec_uchar16 vOut1 = ((vec_uchar16*)pOut)[1];	//It's tempting to just store a qword, but potentially that would write ahead invalidly

			//Create insertion mask for output data
			//Need to take account of length
			uint32_t shiftA = (16-copySizeThisLoop);
			vec_uchar16 maskOut128A = spu_rlmaskqwbyte( maskOut128, -shiftA );	//Shift right
			maskOut128A = spu_slqwbyte( maskOut128A, shiftA );		//Shift back to the left, so right bits have been zeroed
			vec_uchar16 maskOut0 = spu_rlmaskqwbyte( maskOut128A, -((uint32_t)pOut & 0xF) );
			vec_uchar16 maskOut1 = spu_slqwbyte( maskOut128A, 16 - ((uint32_t)pOut & 0xF) );

			//rotate input qword to output alignment
			input128A = spu_rlqwbyte( input128A, 16-(uint32_t)pOut );

			//mask input qword bytes into output qword 0
			vOut0 = spu_sel( vOut0, input128A, maskOut0 );

			//mask input qword bytes into output qword 1
			vOut1 = spu_sel( vOut1, input128A, maskOut1 );

			//Store output qwords
			((vec_uchar16*)pOut)[0] = vOut0;
			((vec_uchar16*)pOut)[1] = vOut1;

			pFrom += copySizeThisLoop;
			pOut += copySizeThisLoop;
			numBytes -= copySizeThisLoop;
		} while ( PREDICT( numBytes > 0, 1 ));
	}
	else
	{
		//In the failure case we have overlapping buffer so must do it one byte at a time.
		unsigned char val = *pFrom;
		do
		{	
			--numBytes;
			++pFrom;
			*pOut = val;
			++pOut;
			val = *pFrom;
		} while ( PREDICT( numBytes > 0, 1 ));
	}

	return pOut;
}


/*
   Decode literal, length, and distance codes and write out the resulting
   literal and match bytes until either not enough input or output is
   available, an end-of-block is encountered, or a data error is encountered.
   When large enough input and output buffers are supplied to inflate(), for
   example, a 16K input buffer and a 64K output buffer, more than 95% of the
   inflate execution time is spent in this routine.

   Entry assumptions:

        state->mode == LEN
        strm->avail_in >= 6
        strm->avail_out >= 258
        start >= strm->avail_out
        state->bits < 8

   On return, state->mode is one of:

        LEN -- ran out of enough output space or enough available input
        TYPE -- reached end of block code, inflate() to interpret next block
        BAD -- error in block data

   Notes:

    - The maximum input bits used by a length/distance pair is 15 bits for the
      length code, 5 bits for the length extra, 15 bits for the distance code,
      and 13 bits for the distance extra.  This totals 48 bits, or six bytes.
      Therefore if strm->avail_in >= 6, then there is enough input to avoid
      checking for available input while decoding.

    - The maximum bytes that a single length/distance pair can output is 258
      bytes, which is the maximum length that can be coded.  inflate_fast()
      requires strm->avail_out >= 258 for each loop to avoid checking for
      output space.
 */
void _edgeZlib_inflate_fast( z_streamp strm, unsigned start )		/* inflate()'s starting value for strm->avail_out */
{
	struct inflate_state*	state;
	const unsigned char*	pStartIn;
	const unsigned char*	pEndIn;				/* while pUsedIn < pEndIn, enough input available */
	unsigned char*			pCurrOut;			/* local strm->next_out */
	unsigned char*			pBeg;				/* inflate()'s initial strm->next_out */
	unsigned char*			pEndOut;			/* while pCurrOut < pEndOut, enough space available */

	unsigned int			wsize;				/* window size or zero if not using window */
	unsigned int			whave;				/* valid bytes in the window */
	unsigned int			write;				/* window write index */
	const unsigned char*	window;				/* allocated sliding window, if wsize != 0 */

	unsigned int			startHold;			/* local strm->hold */
	unsigned int			hold32;				/* local strm->hold */
	unsigned int			startBits;			/* local strm->bits */

	const code*				lcode;				/* local strm->lencode */
	const code*				dcode;				/* local strm->distcode */
	unsigned int			lmask;				/* mask for first level of length codes */
	unsigned int			dmask;				/* mask for first level of distance codes */

	const unsigned char*	pInputQword;				// This points to the current qword of input data being used
	uint32_t				numUsedBitsInInputQword;	// This counts how many bits of the current qword have been used up

	qword					hold128;					// This keeps a suitably rotated cache of the current 128 bits of input data being used
														// At least 32 bits, and up to 128 bits are valid in this qword
														// The qword is pre-rotated so that the bits we'll be interested in are in the prefered slot
	uint32_t				numUsedBitsInHold128;		// This counts how many bits in the cache have been used up

	/* copy state to local variables */
	state		= (struct inflate_state*)strm->state;
	pStartIn	= strm->next_in;
	pEndIn		= pStartIn + (strm->avail_in - 5);
	pCurrOut	= strm->next_out;
	pBeg		= pCurrOut - (start - strm->avail_out);
	pEndOut		= pCurrOut + (strm->avail_out - 257);

	wsize		= state->wsize;
	whave		= state->whave;
	write		= state->write;
	window		= state->window;

	startHold	= state->hold;
	startBits	= state->bits;

	lcode		= state->lencode;
	dcode		= state->distcode;
	lmask		= (1U << state->lenbits) - 1;
	dmask		= (1U << state->distbits) - 1;

	INIT_INPUT_READER( numUsedBitsInInputQword, pInputQword, hold128, hold32, numUsedBitsInHold128, pStartIn, startHold, startBits );

	UPDATE_BITS_15( hold32, numUsedBitsInHold128, hold128, numUsedBitsInInputQword, pInputQword );

	/* decode literals and length/distances until end-of-block or not enough
		input data or output space */
	do
	{
		code thisLength = lcode[hold32 & lmask];									/* retrieved table entry */

	dolen:
		;
		unsigned char lengthBits	= thisLength.bits;
		unsigned int lengthOp		= thisLength.op;
		unsigned short lengthVal	= thisLength.val;
		USE_BITS( lengthBits, hold32, numUsedBitsInHold128, hold128 );
		UPDATE_BITS_15( hold32, numUsedBitsInHold128, hold128, numUsedBitsInInputQword, pInputQword );

		if ( PREDICT( lengthOp == 0, 1 ) )											/* literal */
		{
			Tracevv((stderr, lengthVal >= 0x20 && lengthVal < 0x7f ?
					"inflate:         literal '%c'\n" :
					"inflate:         literal 0x%02x\n", lengthVal));

			pCurrOut = StoreByte( pCurrOut, lengthVal );
		}
		else if ( PREDICT( lengthOp & 0x10, 1 ) )									/* length base */
		{
			unsigned int len = (unsigned int)(lengthVal);

			unsigned int numExtraBits1 = lengthOp & 0xF;							/* number of extra bits */

			if ( PREDICT( numExtraBits1, 0 ) )
			{
				unsigned int extraDistC		= (unsigned int)hold32 & ((1U << numExtraBits1) - 1);
				len		+= extraDistC;

				USE_BITS( numExtraBits1, hold32, numUsedBitsInHold128, hold128 );
				UPDATE_BITS_15( hold32, numUsedBitsInHold128, hold128, numUsedBitsInInputQword, pInputQword );
			}

			Tracevv((stderr, "inflate:         length %u\n", len));

			code thisDist = dcode[hold32 & dmask];

		dodist:
			;
			unsigned char distBits	= thisDist.bits;
			unsigned int distOp		= thisDist.op;
			unsigned short distVal	= thisDist.val;
			USE_BITS( distBits, hold32, numUsedBitsInHold128, hold128 );
			UPDATE_BITS_15( hold32, numUsedBitsInHold128, hold128, numUsedBitsInInputQword, pInputQword );

			if ( PREDICT( distOp & 0x10, 1 ) )										/* distance base */
			{
				unsigned int numExtraBits2 = distOp & 0xF;							/* number of extra bits */

				unsigned int extraDistA		= ((unsigned int)hold32 & ((1U << numExtraBits2) - 1));
				unsigned int backwardsDist	= distVal + extraDistA;
				USE_BITS( numExtraBits2, hold32, numUsedBitsInHold128, hold128 );
				UPDATE_BITS_15( hold32, numUsedBitsInHold128, hold128, numUsedBitsInInputQword, pInputQword );

				Tracevv((stderr, "inflate:         distance %u\n", backwardsDist));

				unsigned int maxOutputDist = (unsigned int)(pCurrOut - pBeg);		/* max distance in output */

				if ( PREDICT( backwardsDist > maxOutputDist, 0 ) )					/* see if copy from window */
				{
					unsigned int windowBackDist = backwardsDist - maxOutputDist;	/* distance back in window */

					if ( windowBackDist > whave )
					{
						strm->msg	= (const char*) "invalid distance too far back";
						state->mode	= BAD;
						break;
					}

					const unsigned char* pFrom = window;							/* where to copy match from */

					if ( write == 0 )												/* very common case */
					{
						pFrom += wsize - windowBackDist;

						if ( windowBackDist < len )									/* some from window */
						{
							len -= windowBackDist;

							pCurrOut = CopyBytes( pCurrOut, pFrom, windowBackDist );

							pFrom = pCurrOut - backwardsDist;						/* rest (if any) from output */
						}
					}
					else if ( write < windowBackDist )								/* wrap around window */
					{
						pFrom	+= (wsize + write - windowBackDist);
						unsigned int endWindDist	= windowBackDist - write;

						if ( endWindDist < len )										/* some from end of window */
						{
							len -= endWindDist;

							pCurrOut = CopyBytes( pCurrOut, pFrom, endWindDist );

							pFrom = window;

							if ( write < len )										/* some from start of window */
							{
								unsigned int startWindDist = write;
								len -= startWindDist;

								pCurrOut = CopyBytes( pCurrOut, pFrom, startWindDist );

								pFrom = pCurrOut - backwardsDist;					/* rest (if any) from output */
							}
						}
					}
					else
					{																/* contiguous in window */
						pFrom += write - windowBackDist;
						if ( windowBackDist < len )									/* some from window */
						{
							len -= windowBackDist;

							pCurrOut = CopyBytes( pCurrOut, pFrom, windowBackDist );

							pFrom = pCurrOut - backwardsDist;						/* rest (if any) from output */
						}
					}

					pCurrOut = CopyBytes2( pCurrOut, pFrom, pCurrOut - pFrom, len );
				}
				else
				{
					const unsigned char*	pFrom = pCurrOut - backwardsDist;		/* copy direct from output */

					pCurrOut = CopyBytes2( pCurrOut, pFrom, backwardsDist, len );
				}
			}
			else if ( (distOp & 0x40) == 0 )										/* 2nd level distance code */
			{
				unsigned int extraDistB = (hold32 & ((1U << distOp) - 1));
				thisDist = dcode[distVal + extraDistB];
				goto dodist;
			}
			else
			{
				strm->msg	= (const char*) "invalid distance code";
				state->mode	= BAD;
				break;
			}
		}
		else if ( (lengthOp & 0x40) == 0 )											/* 2nd level length code */
		{
			unsigned int extraLenB = (hold32 & ((1U << lengthOp) - 1));
			thisLength = lcode[lengthVal + extraLenB];
			goto dolen;
		}
		else if ( lengthOp & 0x20 )													/* end-of-block */
		{
			Tracevv((stderr, "inflate:         end of block\n"));
			state->mode = TYPE;
			break;
		}
		else
		{
			strm->msg	= (const char*) "invalid literal/length code";
			state->mode	= BAD;
			break;
		}

		//"zlib" is slightly conservative with regards to exiting this loop before running out of input data.
		//In fact, "zlib" could choose to stay in this loop just slightly longer dependent on how many bits (<32) have been pre-read into hold.
		//For "Edge Zlib" with 128-bit reads of input data, pUsedIn works out how much input data has actually been consumed.
		const unsigned char* pUsedIn = pInputQword + ((numUsedBitsInInputQword + numUsedBitsInHold128 + 7) >> 3);
		if ( PREDICT( pUsedIn >= pEndIn, 0 ) )
			break;
		if ( PREDICT( pCurrOut >= pEndOut, 0 ) )
			break;

	} while ( 1 );

	/* return unused bytes (on entry, hold < 8, so in won't go too far back) */

	const unsigned char* pCurrentIn	= pInputQword + ((numUsedBitsInInputQword + numUsedBitsInHold128 + 7) >> 3);

	/* update state and return */
	strm->next_in	= pCurrentIn;
	strm->next_out	= pCurrOut;
	strm->avail_in	= 5 + (pEndIn - pCurrentIn);
	strm->avail_out	= 257 + (pEndOut - pCurrOut);

	unsigned int numBitsInHold32	= (8 - (numUsedBitsInInputQword + numUsedBitsInHold128)) & 0x7;
	state->hold		= hold32 & ((1U << numBitsInHold32) - 1);
	state->bits		= numBitsInHold32;
}

/*
   inflate_fast() speedups that turned out slower (on a PowerPC G3 750CXe):
   - Using bit fields for code structure
   - Different op definition to avoid & for extra bits (do & for table bits)
   - Three separate decoding do-loops for direct, window, and write == 0
   - Special case for distance > 1 copies to do overlapped load and store copy
   - Explicit branch predictions (based on measured branch probabilities)
   - Deferring match copy and interspersed it with decoding subsequent codes
   - Swapping literal/length else
   - Swapping window/direct else
   - Larger unrolled copy loops (three is about right)
   - Moving len -= 3 statement into middle of loop
 */

#endif /* !ASMINF */
