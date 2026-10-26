/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.4.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

/*////////////////////////////////////////////////////////////////////////*/

/*	Edge Zlib SPU Version*/
/*	This file is the optimized asm version of the SPU code whose*/
/*	C implementation is in "edgezlib_inffast.c"*/

/*////////////////////////////////////////////////////////////////////////*/




/*////////////////////////////////////////////////////////////////////////*/



	

/*////////////////////////////////////////////////////////////////////////*/



	

/*////////////////////////////////////////////////////////////////////////*/



	

/*////////////////////////////////////////////////////////////////////////*/



	

/*////////////////////////////////////////////////////////////////////////*/



	.equ strm, 2
.equ start, 3
.equ endianSwap, 7
.equ pCurrOut, 29
.equ pEndOut, 25
.equ pRealEndOut, 25
.equ pRealEndIn, 24
.equ pEndIn, 24
.equ pBeg, 3
.equ vOut0, 44
.equ vOut1, 45
.equ vFromM0, 36
.equ vFromM1, 38
.equ vFromL0, 36
.equ vFromL1, 39
.equ input128A, 35
.equ input128B, 36
.equ maskIn128, 37
.equ maskOut128M, 30
.equ maskOut128L, 41
.equ maskOut128MA, 30
.equ maskOut128LA, 41
.equ maskOut128MB, 36
.equ maskOut128LB, 39
.equ allFs, 6
.equ backwardsDist, 30
.equ pFrom, 35
.equ len, 40
.equ availInRot, 27
.equ newAvailIn, 3
.equ nextOutInsrtMsk, 9
.equ strmNextOutQw, 8
.equ stDistCodeQwd, 45
.equ availInInstMsk, 13
.equ availOutInstMsk, 16
.equ strmOutQword, 29
.equ strmNextInQw, 6
.equ holdInsertMsk, 7
.equ strmAvailInQw, 8
.equ strmAvailOutQw, 3
.equ stateHoldQword, 2
.equ stateNmUseBtsQw, 2
.equ stateBitsQword, 37
.equ nextInInsertMsk, 7
.equ stateModeQword, 6
.equ modeInsertMsk, 4
.equ stateBitsRot, 36
.equ stateRot, 26
.equ stateDistBits, 13
.equ stDistBitsQwd, 13
.equ lenCodeRot, 19
.equ newAvailOut, 4
.equ availOut, 25
.equ strmOutRot, 42
.equ availOutQword, 25
.equ distBitsRot, 39
.equ stLenCodeQwd, 37
.equ lenBitsRot, 38
.equ extraLenB, 30
.equ extraDistB, 35
.equ oneUpDistOp, 38
.equ stateQword, 12
.equ oneUpDistBits, 13
.equ oneUpLenOp1, 35
.equ stateLenBits, 11
.equ stLenBitsQwd, 11
.equ oneUpLenBits, 11
.equ newTotBits_3, 26
.equ oneUpXtraBts2, 37
.equ strmInQword, 24
.equ availIn, 27
.equ lcodeindex0, 35
.equ newTotBits_1, 26
.equ oneUpNmUsedBits, 6
.equ newTotBits_2, 26
.equ newTotBits_4, 26
.equ insertWord, 17
.equ numExtraBits2, 35
.equ one, 8
.equ sixteen, 9
.equ picOffset, 4
.equ picLabel, 4
.equ picLoc, 5
.equ pInputQword, 32
.equ pInputQw16_1, 30
.equ pInputQw16_2, 33
.equ pInputQw16_3, 33
.equ pInputQw16_4, 30
.equ pStartIn, 24
.equ _vecCurr_0, 31
.equ _vecNext_0, 34
.equ vecCurr_1, 40
.equ vecNext_1, 30
.equ vecCurr_2, 38
.equ vecNext_2, 33
.equ vecCurr_3, 41
.equ vecNext_3, 33
.equ vecCurr_4, 36
.equ vecNext_4, 30
.equ _mask128_0, 37
.equ mask128_1, 28
.equ mask128_2, 28
.equ mask128_3, 28
.equ mask128_4, 28
.equ _bitShift_0, 35
.equ bitShift_1, 33
.equ bitShift_2, 35
.equ bitShift_3, 35
.equ bitShift_4, 33
.equ holdShift_1, 30
.equ holdShift_2, 37
.equ holdShift_3, 38
.equ holdShift_4, 36
.equ bHold1Rfrsh, 40
.equ bHold2Rfrsh, 35
.equ bHold3Rfrsh, 35
.equ bHold4Rfrsh, 37
.equ numUdBtsInptQw, 26
.equ numUdBtsInHold, 33
.equ pState, 12
.equ fromQword, 36
.equ toQword, 37
.equ insB0, 39
.equ fromRot, 30
.equ lmask, 11
.equ lcode, 19
.equ lcodeByteOffs1, 30
.equ lcodeQword1, 35
.equ pThisLcode1, 34
.equ lcodeIndex1, 30
.equ lcodeByteOffs0, 35
.equ lcodeQword0, 35
.equ pThisLcode0, 40
.equ maskFFFF, 20
.equ dmask, 13
.equ dcode, 32
.equ dcodeByteOffs0, 30
.equ dcodeQword_0, 30
.equ pThisDcode_0, 35
.equ dcodeIndex0, 30
.equ dcodeByteOffs1, 30
.equ dcodeQword_1, 30
.equ pThisDcode_1, 35
.equ lengthBits, 41
.equ startBits, 36
.equ isDistanceBase, 39
.equ isNot2ndLvlDist, 37
.equ distOp, 37
.equ distVal, 30
.equ distBits, 36
.equ dcodeIndex1, 30
.equ bMoreInput, 42
.equ bMoreOutput, 43
.equ insertByteMask, 38
.equ storeByteOutQw, 36
.equ bitMask2, 37
.equ extraDistA, 28
.equ saveHold, 6
.equ saveHoldMask, 6
.equ offset4, 5
.equ offset12, 14
.equ offset24, 23
.equ offset28, 12
.equ offset40, 16
.equ offset44, 17
.equ offset52, 18
.equ offset56, 15
.equ offset60, 10
.equ offset76, 19
.equ offset84, 11
.equ offset88, 13
.equ isLengthBase, 41
.equ lengthOp, 38
.equ isEndOfBlock, 3
.equ kModeType, 7
.equ rotLenOp0, 38
.equ rotLenVal0, 40
.equ rotLenBits0, 34
.equ rotLenOp1, 38
.equ rotLenVal1, 34
.equ rotLenBits1, 30
.equ rotDistOp0, 37
.equ rotDistVal0, 35
.equ rotDistBits0, 36
.equ rotDistOp1, 37
.equ rotDistVal1, 35
.equ rotDistBits1, 36
.equ notScndLvlLen, 34
.equ extraBytes_1, 28
.equ extraBytes_2, 28
.equ extraBytes_3, 28
.equ extraBytes_4, 28
.equ hold128, 28
.equ hold128tmp1, 39
.equ hold128tmp2, 30
.equ someMask1, 35
.equ someMask3, 38
.equ pStoreByteQw, 30
.equ maskOutRot1, 30
.equ maskOutRot2, 39
.equ maskOutRotA, 41
.equ maskOutRotB, 43
.equ outNybble, 41
.equ fromNybble, 37
.equ inputRot, 42
.equ falseStoreAddr, 21
.equ isOver16Bytes, 36
.equ sizeThisLoop, 38
.equ kModeBad, 22
.equ pErrorString, 3
.equ outStartOffset, 3
.equ pUsedIn, 39
.equ endInMinusUsed, 3
.equ endOutMinusCurr, 4
.equ newNumUsedBits, 11
.equ eightMinusNmBts, 11
.equ sumNumUsedBits, 37
.equ rndUpUsedBits0, 39
.equ rndUpUsedBits2, 3
.equ rndUpUsedBits3, 3
.equ extraUsedBytes0, 39
.equ extraUsedBytes2, 3
.equ extraUsedBytes3, 3
.equ strmMsgQword, 6
.equ insrtStrmMsg, 4
.equ bNoBits, 27
.equ maxOutputDist, 36
.equ windowBackDist, 36
.equ whave, 17
.equ write, 31
.equ wsize, 16
.equ isWrapArndWind, 35
.equ bIsTooFar, 35
.equ bSomeFromWindow, 37
.equ bCopyFromWindow, 37
.equ lengthVal, 34
.equ numExtraBits1, 30
.equ oneUpXtraBts1, 35
.equ bitMask1, 35
.equ extraDistC, 35
.equ availInQw, 28
.equ stHoldQw, 40
.equ stHoldRot, 38
.equ oneUpStartBits, 43
.equ startBitsMask, 35
.equ prefSlotMask, 34
.equ startBitsMsk128, 34
.equ alignedHold128, 31
.equ rotHold128, 31
.equ startHold, 28
.equ stateWriteQw, 35
.equ stateWsizeQw, 43
.equ stateWsizeRot, 16
.equ stateWhaveQw, 17
.equ stateWhaveRot, 39
.equ stateWindowQw, 18
.equ stateWindowRot, 44
.equ window, 18
.equ nmStrtBytsInQw, 31
.equ _pInputData, 33
.equ _inputDataByts, 41
.equ _inputDataBits, 41
.equ _numUdBtsInptQw, 41
.equ _pInputQword, 33
.equ isOverlapping, 30
.equ windOffset, 35
.equ bSomeFromWind, 37
.equ bSomeFrmWndEnd, 38
.equ bSomeFrmWndStrt, 39
.equ inByte3, 37
.equ inByte4, 37
.equ inByte5, 36
.equ inByteQw3, 38
.equ inByteQw4, 38
.equ inByteQw5, 36
.equ inByteQw6, 37
.equ outByteQw3, 38
.equ outByteQw4, 38
.equ outByteQw5, 38
.equ outByteQw6, 37
.equ insByteMask3, 39
.equ insByteMask4, 39
.equ insByteMask5, 41
.equ insByteMask6, 39
.equ byteRotVal3, 37
.equ byteRotVal4, 37
.equ byteRotVal5, 38
.equ byteRotVal6, 36
.equ bytesToCopy3, 36
.equ bytesToCopy4, 36
.equ bytesToCopy5, 37
.equ bytesToCopy6, 38
.equ endWindDist, 37
.equ windOffset2, 35
.equ windOffset3, 35
.equ pCurrOutCopy1, 38
.equ pInputQword_, 27
.equ lengthBits_, 35
.equ distBits_, 35
.equ distOp_, 36
.equ vecCurr_3_, 35
.equ byteRotVal3_, 41
.equ inByteQw3_, 42
.equ byteRotVal5_, 42
.equ inByteQw5_, 43
.equ byteRotVal6_, 41
.equ inByteQw6_, 42
.equ byteRotVal4_, 41
.equ inByteQw4_, 42
.equ vFromM1_, 35
.equ vOut1_, 36
.equ vOut0_, 30
.equ maskOutRot1_, 41
.equ vOut1__, 42
.equ outNybble_, 43
.equ inputRot_, 45
.equ maskOutRot2_, 46
.equ maskOutRotB_, 47
.equ maskOutRotA_, 43
.equ vOut1___, 39
.equ vOut0__, 41
.equ fromRot_, 38
.equ toQword_, 36
.equ stateModeQword_, 4
.equ strmMsgQword_, 3
.equ stateModeQword__, 3
.equ strmAvailInQw_, 3
.section .text.align64
.align 6
.type _edgeZlib_inflate_fast_spu_asm, @function
.global _edgeZlib_inflate_fast_spu_asm

.extern _edgeZlibEndianSwap_0F0E0D0C_0B0A0908_07060504_03020100
.extern _edgeZlibFalseStore
.extern _edgeZlibInvalidDistanceCodeString
.extern _edgeZlibInvalidLiteralLengthCodeString
.extern _edgeZlibInvalidDistanceTooFarBackString

.set kMaxUsedHoldBits ,	112		/*When we pass this number of bits used, refresh our hold128 qword*/

.set kOpCodeRot ,		13
.set kBitsCodeRot ,		14
.set kValCodeRot ,		16

_edgeZlib_inflate_fast_spu_asm:
	ori				strm,			$3,				0;
	shlqbyi			start,			$4,				0;
	ila				picLabel,		_PicLabel;
	brsl			picLoc,			_PicLabel;
_PicLabel:
	il				allFs,			-1;
	lqr				endianSwap,		_edgeZlibEndianSwap_0F0E0D0C_0B0A0908_07060504_03020100;
	il				one,			1;
	hbrr			_____Hint1,		_TestConditionAndMainLoop;
	il				sixteen,		16;
	sf				picOffset,		picLabel,		picLoc;
	il				offset4,		4;
	il				offset60,		60;
	il				offset84,		84;
	il				offset28,		28;
	il				offset88,		88;
	il				offset12,		12;
	il				offset56,		56;
	il				offset40,		40;
	il				offset44,		44;
	il				offset52,		52;
	il				offset76,		76;
	ila				maskFFFF,		0xFFFF;
	ila				falseStoreAddr,	_edgeZlibFalseStore;
	il				kModeBad,		0x1B;
	il				offset24,		0x18;
	lqd				strmInQword,	0(strm);
	a				falseStoreAddr,	falseStoreAddr,	picOffset;
	lqd				availOutQword,	16(strm);
	a				stateRot,		strm,			offset28;
	lqx				stateQword,		strm,			offset28;
	a				availInRot,		strm,			offset4;
	lqx				availInQw,		strm,			offset4;
	nop $15;
	lqx				strmOutQword,	strm,			offset12;
	ai				pStoreByteQw,	falseStoreAddr,	0;					/*StoreByte qword will go to a false address*/
	rotqby			pStartIn,		strmInQword,	strm;
	nop $26;
	rotqby			availOut,		availOutQword,	strm;
	andi			nmStrtBytsInQw,	pStartIn,		0xF;
	lnop;
	andi			pInputQword,	pStartIn,		0xFFFFFFF0;
	rotqby			pState,			stateQword,		stateRot;
	shli			numUdBtsInptQw,	nmStrtBytsInQw,	3;
/*GetHold128?*/
	lqd				_vecCurr_0,		0(pInputQword);
	ai				_pInputData,	pStartIn,		-1;
	lqd				_vecNext_0,		16(pInputQword);
	sfi				_bitShift_0,	numUdBtsInptQw,	0x80;
	lnop;
	a				stateBitsRot,	pState,			offset60;
	lqx				stateBitsQword,	pState,			offset60;
	a				lenBitsRot,		pState,			offset84;
	lqx				stLenBitsQwd,	pState,			offset84;
	a				distBitsRot,	pState,			offset88;
	lqx				stDistBitsQwd,	pState,			offset88;
	nop $68;
	shufb			_vecCurr_0,		_vecCurr_0,		_vecCurr_0,		endianSwap;
	nop $74;
	shufb			_vecNext_0,		_vecNext_0,		_vecNext_0,		endianSwap;
	nop $26;
	lqx				stHoldQw,		pState,			offset56;
	andi			_inputDataByts,	_pInputData,	15;
	rotqby			startBits,		stateBitsQword,	stateBitsRot;
	nop $40;
	shlqbybi		_mask128_0,		allFs,			_bitShift_0;
	a				strmOutRot,		strm,			offset12;
	rotqbybi		_vecCurr_0,		_vecCurr_0,		_bitShift_0;
	shli			_inputDataBits,	_inputDataByts,	3;
	rotqbybi		_vecNext_0,		_vecNext_0,		_bitShift_0;
	shl				oneUpStartBits,	one,			startBits;
	rotqby			stateLenBits,	stLenBitsQwd,	lenBitsRot;
	a				stHoldRot,		pState,			offset56;
	shlqbi			_mask128_0,		_mask128_0,		_bitShift_0;
	a				stateWindowRot, pState,			offset52;
	rotqbi			_vecCurr_0,		_vecCurr_0,		_bitShift_0;
	sf				_numUdBtsInptQw,startBits,		_inputDataBits;
	rotqbi			_vecNext_0,		_vecNext_0,		_bitShift_0;
	ai				startBitsMask,	oneUpStartBits,	-1;
	rotqby			stateDistBits,	stDistBitsQwd,	distBitsRot;
	a				stateWhaveRot,	pState,			offset44;
	lqx				stateWsizeQw,	pState,			offset40;
	shl				oneUpLenBits,	one,			stateLenBits;
	lqx				stateWindowQw,	pState,			offset52;
	selb			alignedHold128,	_vecCurr_0,		_vecNext_0,		_mask128_0;
	fsmbi			prefSlotMask,	0xF000;
	shl				oneUpDistBits,	one,			stateDistBits;
	lqx				stLenCodeQwd,	pState,			offset76;
	a				stateWsizeRot,	pState,			offset40;
	rotqbyi			alignedHold128,	alignedHold128,	12;
	sf				outStartOffset,	availOut,		start;
	lqx				stateWhaveQw,	pState,			offset44;
	and				startBitsMsk128,startBitsMask,	prefSlotMask;
	lqd				stateWriteQw,	0x30(pState);
	ai				_numUdBtsInptQw,_numUdBtsInptQw,8;
	lqd				stDistCodeQwd,	0x50(pState);
	andi			_pInputQword,	_pInputData,	-16;
	rotqbi			rotHold128,		alignedHold128,	startBits;
	nop $70;
	rotqby			pCurrOut,		strmOutQword,	strmOutRot;
	a				lenCodeRot,		pState,			offset76;
	rotqby			availIn,		availInQw,		availInRot;
	ai				lmask,			oneUpLenBits,	-1;
	rotqby			startHold,		stHoldQw,		stHoldRot;
	ai				dmask,			oneUpDistBits,	-1;
	rotqbybi		rotHold128,		rotHold128,		startBits;
	a				pRealEndOut,	pCurrOut,		availOut;
	lnop;
	a				pRealEndIn,		pStartIn,		availIn;
	lnop;
	ceqi			bNoBits,		startBits,		0;
	rotqby			wsize,			stateWsizeQw,	stateWsizeRot;
	selb			hold128,		rotHold128,		startHold,		startBitsMsk128;
	rotqby			window,			stateWindowQw,	stateWindowRot;
	selb			numUdBtsInptQw,	_numUdBtsInptQw,numUdBtsInptQw,	bNoBits;
	rotqby			lcode,			stLenCodeQwd,	lenCodeRot;
	selb			pInputQword_,	_pInputQword,	pInputQword,	bNoBits;
	rotqby			write,			stateWriteQw,	pState;
	ai				pEndIn,			pRealEndIn,		-5;
	rotqby			whave,			stateWhaveQw,	stateWhaveRot;
	sf				pBeg,			outStartOffset,	pCurrOut;
	rotqby			dcode,			stDistCodeQwd,	pState;
	ai				pEndOut,		pRealEndOut,	-257;
	fsmbi			numUdBtsInHold,	0;
	ai				pCurrOut,		pCurrOut,		-1;					/*Decrement in order to nullify the increment at _TestConditionAndMainLoop*/
	_____Hint1:
	br				_TestConditionAndMainLoop;


.align 6
_mainLineCase:
	ai				pStoreByteQw,	pCurrOut,		0;					/*Store the byte to a genuine output address*/
	brnz			lengthOp,		_NotLiteral;
_StoreByte:																/*If we branch to here, pStoreByteQw is genuine*/
_TestConditionAndMainLoop:												/*If we branch to here, pStoreByteQw is a fake store address (pOut will have been decremented to nullify the following increment)*/
	and				lcodeindex0,	hold128,		lmask;
	lqd				storeByteOutQw,	0(pCurrOut);
	a				sumNumUsedBits,	numUdBtsInptQw,	numUdBtsInHold;
	hbrr			_____Hint3,		_mainLineCase;
	shli			lcodeByteOffs0,	lcodeindex0,	2;
	cbd				insertByteMask,	0(pCurrOut);
	ai				rndUpUsedBits0,	sumNumUsedBits,	7;
	lnop;
	ai				pCurrOut,		pCurrOut,		1;					/*Only wanted for StoreByte - other entry points have pre-nullified this increment*/
	lnop;
	rotmi			extraUsedBytes0,rndUpUsedBits0,	-3;
	lnop;
	a				pThisLcode0,	lcode,			lcodeByteOffs0;
	lqx				lcodeQword0,	lcode,			lcodeByteOffs0;
	nop $107;
	shufb			storeByteOutQw,	lengthVal,		storeByteOutQw,	insertByteMask;
	ai				rotLenBits0,	pThisLcode0,	kBitsCodeRot;
	lnop;
	ai				rotLenOp0,		pThisLcode0,	kOpCodeRot;
	lnop;
	ai				rotLenVal0,		pThisLcode0,	kValCodeRot;
	lnop;
	a				pUsedIn,		pInputQword_,	extraUsedBytes0;
	lnop;
	nop $68;
	rotqby			lengthBits,		lcodeQword0,	rotLenBits0;
	clgt			bMoreInput,		pEndIn,			pUsedIn;
	rotqby			lengthOp,		lcodeQword0,	rotLenOp0;
	clgt			bMoreOutput,	pEndOut,		pCurrOut;
	rotqby			lengthVal,		lcodeQword0,	rotLenVal0;
_DoLen:																	/*If we branch to here, pStoreByteQw is a fake store address*/
	andi			lengthBits_,		lengthBits,		0xFF;
	stqd			storeByteOutQw,	0(pStoreByteQw);						/*pStoreByteQw is either the StoreByte address, or a false store address*/
	andi			lengthOp,		lengthOp,		0xFF;
	brz				bMoreInput,		_EndOfMainLoop;
	sfi				holdShift_1,	lengthBits_,		0x80;
	brz				bMoreOutput,	_EndOfMainLoop;
	a				numUdBtsInHold,	numUdBtsInHold,	lengthBits_;
	lnop;
	shl				oneUpLenOp1,	one,			lengthOp;
	rotqbybi		hold128,		hold128,		holdShift_1;
	clgti			bHold1Rfrsh,	numUdBtsInHold,	kMaxUsedHoldBits;
	lnop;
	andi			isLengthBase,	lengthOp,		0x10;
	rotqbi			hold128,		hold128,		holdShift_1;
	ai				someMask1,		oneUpLenOp1,	-1;
_____Hint3:
	brz				bHold1Rfrsh,	_mainLineCase;

/*This is the lesser likely case - the case where USE_BITS has to pull more data*/
/*_GetHold_1:*/
	a				newTotBits_1,	numUdBtsInptQw,	numUdBtsInHold;
	hbrr			_____Hint4,		_StoreByte;
	nop $127;
	lnop;
	rotmi			extraBytes_1,	newTotBits_1,	-3;					/*Should really mask this off to just given 0x0 or 0x10, but we can do that later*/
	lnop;
	andi			numUdBtsInptQw,	newTotBits_1,	0x7F;
	lnop;
	ai				pInputQw16_1,	pInputQword_,	16;
	lnop;
	sfi				bitShift_1,		numUdBtsInptQw,	0x80;
	lqx				vecCurr_1,		extraBytes_1,	pInputQword_;
	nop $57;
	lqx				vecNext_1,		extraBytes_1,	pInputQw16_1;
	a				pInputQword_,	pInputQword_,	extraBytes_1;
	shlqbybi		mask128_1,		allFs,			bitShift_1;
	shufb			vecCurr_1,		vecCurr_1,		vecCurr_1,		endianSwap;
	shufb			vecNext_1,		vecNext_1,		vecNext_1,		endianSwap;
	andi			pInputQword_,	pInputQword_,	0xFFFFFFF0;			/*and now it's later,... pInputQword should remain qword aligned*/
	shlqbi			mask128_1,		mask128_1,		bitShift_1;
	rotqbybi		vecCurr_1,		vecCurr_1,		bitShift_1;
	rotqbybi		vecNext_1,		vecNext_1,		bitShift_1;
	rotqbi			vecCurr_1,		vecCurr_1,		bitShift_1;
	rotqbi			vecNext_1,		vecNext_1,		bitShift_1;
	selb			hold128,		vecCurr_1,		vecNext_1,		mask128_1;
	lnop;
	il				numUdBtsInHold,	0;
	rotqbyi			hold128,		hold128,		12;
	ai				pStoreByteQw,	pCurrOut,		0;					/*Store the byte to a genuine output address*/
_____Hint4:
	brz				lengthOp,		_StoreByte;

_NotLiteral:
	andi			numExtraBits1,	lengthOp,		0xF;
	lnop;
	and				len,			lengthVal,		maskFFFF;
	brz				isLengthBase,	_NotLengthBase;
/*_LengthBase*/
	nop $29;
	brnz			numExtraBits1,	_XtraBits;
_ReturnFromXtraBits:
	and				dcodeIndex1,	hold128,		dmask;
	lnop;
	nop $127;
	lnop;
	shli			dcodeByteOffs1,	dcodeIndex1,	2;
	lnop;
	nop $127;
	lnop;
	nop $127;
	lnop;
	a				pThisDcode_1,	dcode,			dcodeByteOffs1;
	lqx				dcodeQword_1,	dcode,			dcodeByteOffs1;
	nop $127;
	lnop;
	ai				rotDistBits1,	pThisDcode_1,	kBitsCodeRot;
	lnop;
	ai				rotDistOp1,		pThisDcode_1,	kOpCodeRot;
	lnop;
	ai				rotDistVal1,	pThisDcode_1,	kValCodeRot;
	lnop;
	nop $127;
	lnop;
	nop $65;
	rotqby			distBits,		dcodeQword_1,	rotDistBits1;
	nop $66;
	rotqby			distOp,			dcodeQword_1,	rotDistOp1;
	nop $64;
	rotqby			distVal,		dcodeQword_1,	rotDistVal1;
_DoDist:
	andi			distBits_,		distBits,		0xFF;
	lnop;
	andi			distOp_,			distOp,			0xFF;
	lnop;
	sfi				holdShift_2,	distBits_,		0x80;
	lnop;
	shl				oneUpDistOp,	one,			distOp_;
	lnop;
	a				numUdBtsInHold,	numUdBtsInHold,	distBits_;
	rotqbybi		hold128,		hold128,		holdShift_2;
	and				distVal,		distVal,		maskFFFF;
	lnop;
	clgti			bHold2Rfrsh,	numUdBtsInHold,	kMaxUsedHoldBits;
	lnop;
	ai				someMask3,		oneUpDistOp,	-1;
	lnop;
	andi			isDistanceBase,	distOp_,			0x10;
	rotqbi			hold128,		hold128,		holdShift_2;
	andi			isNot2ndLvlDist,distOp_,			0x40;
	brnz			bHold2Rfrsh,	_GetHold_3;
_ReturnFromGetHold_3:
	andi			numExtraBits2,	distOp_,			0xF;
	brz				isDistanceBase,	_NotDistanceBase;
/*_DistanceBase:*/
	sf				maxOutputDist,	pBeg,			pCurrOut;
	lnop;
	shl				oneUpXtraBts2,	one,			numExtraBits2;
	lnop;
	sfi				holdShift_3,	numExtraBits2,	0x80;
	lnop;
	nop $127;
	lnop;
	ai				bitMask2,		oneUpXtraBts2,	-1;
	rotqbybi		hold128tmp1,	hold128,		holdShift_3;
	a				numUdBtsInHold,	numUdBtsInHold,	numExtraBits2;
	lnop;
	and				extraDistA,		bitMask2,		hold128;
	lnop;
	a				backwardsDist,	extraDistA,		distVal;
	lnop;
	clgti			bHold3Rfrsh,	numUdBtsInHold,	kMaxUsedHoldBits;
	rotqbi			hold128,		hold128tmp1,	holdShift_3;
	clgt			bCopyFromWindow,backwardsDist,	maxOutputDist;
	brnz			bHold3Rfrsh,	_GetHold_2;
_ReturnFromGetHold_2:
	sf				pFrom,			backwardsDist,	pCurrOut;		/*If not form window, pFrom = pCurrOut - backwardsDist*/
	brnz			bCopyFromWindow,_CopyFromWindow;

/*_NotCopyFromWindow:*/
_DoMainByteCopy:
	clgt			isOverlapping,	len,			backwardsDist;
	hbrr			_____Hint2,		_TestConditionAndMainLoop;
	clgti			isOver16Bytes,	len,			16;
	lnop;
	nop $29;
	brnz			isOverlapping,	_ByteMemcpyLoop;
	nop $35;
	brnz			isOver16Bytes,	_NonOverlapMemcpyLoop;

/*_MemcpyUpTo16b:*/
	ai				maskOutRot1,	len,			-16;
	lqd				vFromM0,		0(pFrom);
	andi			fromNybble,		pFrom,			15;
	lqd				vFromM1,		16(pFrom);
	sfi				maskOutRot2,	len,			16;
	lnop;
	andi			outNybble,		pCurrOut,		0xF;
	rotqmby			maskOut128M,	allFs,			maskOutRot1;
	sfi				inputRot,		pCurrOut,		16;
	shlqby			maskIn128,		allFs,			fromNybble;
	sfi				maskOutRotB,	outNybble,		16;
	lqd				vOut0,			0(pCurrOut);
	sfi				maskOutRotA,	outNybble,		0;
	rotqby			vFromM0,		vFromM0,		pFrom;
	lqd				vOut1,			16(pCurrOut);
	rotqby			vFromM1_,		vFromM1,		pFrom;
	ori				pCurrOutCopy1,	pCurrOut,		0;					/*Take a copy so we can modify pCurrOut early*/
	shlqby			maskOut128M,	maskOut128M,	maskOutRot2;
	a				pCurrOut,		pCurrOut,		len;
	selb			input128A,		vFromM1_,		vFromM0,		maskIn128;
	shlqby			maskOut128MB,	maskOut128M,	maskOutRotB;
	rotqby			input128A,		input128A,		inputRot;
	nop $70;
	rotqmby			maskOut128MA,	maskOut128M,	maskOutRotA;
	selb			vOut1_,			vOut1,			input128A,		maskOut128MB;
	selb			vOut0_,			vOut0,			input128A,		maskOut128MA;
	nop $73;
	stqd			vOut1_,			16(pCurrOutCopy1);
	ai				pCurrOut,		pCurrOut,		-1;					/*Decrement in order to nullify the increment at _TestConditionAndMainLoop*/
	stqd			vOut0_,			0(pCurrOutCopy1);
	ai				pStoreByteQw,	falseStoreAddr,	0;					/*StoreByte qword will go to a false address*/
_____Hint2:
	br				_TestConditionAndMainLoop;

_EndOfMainLoop:
	sf				endInMinusUsed,	pUsedIn,		pEndIn;
	hbr				_____Hint_EndOfMainLoop_Exit,	$lr;
	sf				endOutMinusCurr,pCurrOut,		pEndOut;
	lqd				strmNextInQw,	0(strm);
	ai				newAvailIn,		endInMinusUsed,	5;
	cwd				nextInInsertMsk,0(strm);
	ai				newAvailOut,	endOutMinusCurr,257;
	cwx				nextOutInsrtMsk,strm,			offset12;
	sfi				eightMinusNmBts,sumNumUsedBits,	8;
	cwx				availInInstMsk,	strm,			offset4;
	nop $1;
	cwd				availOutInstMsk,16(strm);
	andi			newNumUsedBits,	eightMinusNmBts,7;
	shufb			strmNextInQw,	pUsedIn,		strmNextInQw,	nextInInsertMsk;
	nop $26;
	cwx				holdInsertMsk,	pState,			offset56;
	nop $7;
	stqd			strmNextInQw,	0(strm);
	shl				oneUpNmUsedBits,one,			newNumUsedBits;
	lqx				strmNextOutQw,	strm,			offset12;
	nop $21;
	cwx				insertWord,		pState,			offset60;
	nop $45;
	shufb			strmNextOutQw,	pCurrOut,		strmNextOutQw,	nextOutInsrtMsk;
	nop $23;
	stqx			strmNextOutQw,	strm,			offset12;
	ai				saveHoldMask,	oneUpNmUsedBits,-1;
	lqx				strmAvailInQw,	strm,			offset4;
	nop $23;
	shufb			strmAvailInQw_,	newAvailIn,		strmAvailInQw,	availInInstMsk;
	and				saveHold,		hold128,		saveHoldMask;
	stqx			strmAvailInQw_,	strm,			offset4;
	nop $1;
	lqd				strmAvailOutQw,	16(strm);
	nop $22;
	shufb			strmAvailOutQw,	newAvailOut,	strmAvailOutQw,	availOutInstMsk;
	nop $4;
	stqd			strmAvailOutQw,	16(strm);
	nop $26;
	lqx				stateHoldQword,	pState,			offset56;
	nop $14;
	shufb			stateHoldQword,	saveHold,		stateHoldQword,	holdInsertMsk;
	nop $28;
	stqx			stateHoldQword,	pState,			offset56;
	nop $21;
	lqx				stateNmUseBtsQw,pState,			offset60;
	nop $29;
	shufb			stateNmUseBtsQw,newNumUsedBits,	stateNmUseBtsQw,insertWord;
	nop $23;
	stqx			stateNmUseBtsQw,pState,			offset60;
	nop $127;
_____Hint_EndOfMainLoop_Exit:
	bi				$lr;													/*return*/



_XtraBits:		/*numExtraBits1 > 0 ?*/
	shl				oneUpXtraBts1,	one,			numExtraBits1;
	hbrr			_____Hint_ReturnFromXtraBits, _ReturnFromXtraBits;
	sfi				holdShift_4,	numExtraBits1,	0x80;
	lnop;
	a				numUdBtsInHold,	numUdBtsInHold,	numExtraBits1;
	rotqbybi		hold128tmp2,	hold128,		holdShift_4;
	ai				bitMask1,		oneUpXtraBts1,	-1;
	lnop;
	clgti			bHold4Rfrsh,	numUdBtsInHold,	kMaxUsedHoldBits;
	lnop;
	and				extraDistC,		bitMask1,		hold128;
	rotqbi			hold128,		hold128tmp2,	holdShift_4;
	nop $36;
	brnz			bHold4Rfrsh,	_GetHold_4;
_ReturnFromGetHold_4:
	a				len,			len,			extraDistC;
_____Hint_ReturnFromXtraBits:
	br				_ReturnFromXtraBits;


_CopyFromWindow:
	sf				windowBackDist,	maxOutputDist,	backwardsDist;
	clgt			bIsTooFar,		windowBackDist,	whave;
	brnz			bIsTooFar,		_InvalidDistance_TooFarBack;
	brnz			write,			_WriteIsNonZero;
/*_WriteIsZero:*/
	sf				windOffset,		windowBackDist,	wsize;
	clgt			bSomeFromWind,	len,			windowBackDist;
	a				pFrom,			window,			windOffset;
	brz				bSomeFromWind,	_DoMainByteCopy;
/*_SomeFromWindow:*/
	sf				len,			windowBackDist,	len;
	brz				windowBackDist,	_SetFromAndDoMainByteCopy;
	ai				byteRotVal3,	pFrom,			13;
	lqd				inByteQw3,		0(pFrom);
	ai				bytesToCopy3,	windowBackDist,	0;
	hbrr			_____Hint_ByteMemcpyLoopBack3, _ByteMemcpyLoopBack3;
	nop $74;
	rotqby			inByte3,		inByteQw3,		byteRotVal3;
_ByteMemcpyLoopBack3:	/*Src and Dest do not overlap*/
	ai				pFrom,			pFrom,			1;
	lqd				outByteQw3,		0(pCurrOut);
	ai				bytesToCopy3,	bytesToCopy3,	-1;
	cbd				insByteMask3,	0(pCurrOut);
	ai				byteRotVal3_,	pFrom,			13;
	lqd				inByteQw3_,		0(pFrom);
	nop $113;
	shufb			outByteQw3,		inByte3,		outByteQw3,		insByteMask3;
	nop $82;
	rotqby			inByte3,		inByteQw3_,		byteRotVal3_;
	nop $66;
	stqd			outByteQw3,		0(pCurrOut);
	ai				pCurrOut,		pCurrOut,		1;
_____Hint_ByteMemcpyLoopBack3:
	brnz			bytesToCopy3,	_ByteMemcpyLoopBack3;

_SetFromAndDoMainByteCopy:
	sf				pFrom,			backwardsDist,	pCurrOut;
	br				_DoMainByteCopy;


_NotLengthBase:
	and				extraLenB,		hold128,		someMask1;
	hbrr			_____Hint12,	_DoLen;
	a				lcodeIndex1,	len,			extraLenB;
	lnop;
	andi			notScndLvlLen,	lengthOp,		0x40;
	lnop;
	shli			lcodeByteOffs1,	lcodeIndex1,	2;
	brnz			notScndLvlLen,	_NotSecondLevelLengthCode;
/*_SecondLevelLengthCode:*/
	a				pThisLcode1,	lcode,			lcodeByteOffs1;
	lqx				lcodeQword1,	lcode,			lcodeByteOffs1;
	nop $127;
	lnop;
	ai				rotLenBits1,	pThisLcode1,	kBitsCodeRot;
	lnop;
	ai				rotLenOp1,		pThisLcode1,	kOpCodeRot;
	lnop;
	ai				rotLenVal1,		pThisLcode1,	kValCodeRot;
	lnop;
	il				bMoreInput,		0xFFFFFFFF;
	rotqby			lengthBits,		lcodeQword1,	rotLenBits1;
	il				bMoreOutput,	0xFFFFFFFF;
	rotqby			lengthOp,		lcodeQword1,	rotLenOp1;
	ai				pStoreByteQw,	falseStoreAddr,	0;					/*StoreByte qword will go to a false address*/
	rotqby			lengthVal,		lcodeQword1,	rotLenVal1;
	nop $127;
_____Hint12:
	br				_DoLen;


_NotDistanceBase:
	and				extraDistB,		someMask3,		hold128;
	lnop;
	a				dcodeIndex0,	distVal,		extraDistB;
	hbrr		_____Hint9,	_DoDist;
	nop $36;
	brnz			isNot2ndLvlDist,_InvalidDistance;
/*_SecondLevelDistanceCode:*/
	shli			dcodeByteOffs0,	dcodeIndex0,	2;
	lnop;
	a				pThisDcode_0,	dcode,			dcodeByteOffs0;
	lqx				dcodeQword_0,	dcode,			dcodeByteOffs0;
	ai				rotDistBits0,	pThisDcode_0,	kBitsCodeRot;
	lnop;
	ai				rotDistOp0,		pThisDcode_0,	kOpCodeRot;
	lnop;
	ai				rotDistVal0,	pThisDcode_0,	kValCodeRot;
	rotqby			distBits,		dcodeQword_0,	rotDistBits0;
	nop $66;
	rotqby			distOp,			dcodeQword_0,	rotDistOp0;
	nop $64;
	rotqby			distVal,		dcodeQword_0,	rotDistVal0;
	nop $127;
_____Hint9:
	br				_DoDist;


_NotSecondLevelLengthCode:
	andi			isEndOfBlock,	lengthOp,		0x20;
	hbrr			_____Hint13,	_EndOfMainLoop;
	a				sumNumUsedBits,	numUdBtsInptQw,	numUdBtsInHold;
	lqd				stateModeQword,	0(pState);
	il				kModeType,		11;
	brz				isEndOfBlock,	_InvalidLiteral;
/*_EndOfBlock:*/
	ai				rndUpUsedBits2,	sumNumUsedBits,	7;
	cwd				modeInsertMsk,	0(pState);
	rotmi			extraUsedBytes2,rndUpUsedBits2,	-3;
	shufb			stateModeQword_,	kModeType,		stateModeQword,	modeInsertMsk;
	nop $15;
	stqd			stateModeQword_,	0(pState);
	a				pUsedIn,		pInputQword_,	extraUsedBytes2;
_____Hint13:
	br				_EndOfMainLoop;


_WriteIsNonZero:
	clgt			isWrapArndWind,	windowBackDist,	write;
	brz				isWrapArndWind,	_ContiguousInWindow;
/*_WrapAroundWindow:*/
	sf				endWindDist,	write,			windowBackDist;
	lnop;
	a				windOffset2,	wsize,			write;
	lnop;
	clgt			bSomeFrmWndEnd,	len,			endWindDist;
	lnop;
	sf				windOffset2,	windowBackDist,	windOffset2;
	lnop;
	a				pFrom,			window,			windOffset2;
	brz				bSomeFrmWndEnd,	_DoMainByteCopy;
/*_SomeFromEndOfWindow:*/
	sf				len,			endWindDist,	len;
	lqd				inByteQw5,		0(pFrom);
	ai				byteRotVal5,	pFrom,			13;
	hbrr			_____Hint_ByteMemcpyLoopBack5, _ByteMemcpyLoopBack5;
	clgt			bSomeFrmWndStrt,len,			write;
	lnop;
	ai				bytesToCopy5,	endWindDist,	0;
	rotqby			inByte5,		inByteQw5,		byteRotVal5;
_ByteMemcpyLoopBack5:	/*Src and Dest do not overlap*/
	ai				pFrom,			pFrom,			1;
	lqd				outByteQw5,		0(pCurrOut);
	ai				bytesToCopy5,	bytesToCopy5,	-1;
	cbd				insByteMask5,	0(pCurrOut);
	ai				byteRotVal5_,	pFrom,			13;
	lqd				inByteQw5_,		0(pFrom);
	nop $114;
	shufb			outByteQw5,		inByte5,		outByteQw5,		insByteMask5;
	nop $84;
	rotqby			inByte5,		inByteQw5_,		byteRotVal5_;
	nop $66;
	stqd			outByteQw5,		0(pCurrOut);
	ai				pCurrOut,		pCurrOut,		1;
_____Hint_ByteMemcpyLoopBack5:
	brnz			bytesToCopy5,	_ByteMemcpyLoopBack5;

	ai				pFrom,			window,			0;
	brz				bSomeFrmWndStrt,_DoMainByteCopy;
/*_SomeFromStartOfWindow:*/
	sf				len,			write,			len;
	hbrr			_____Hint_ByteMemcpyLoopBack6, _ByteMemcpyLoopBack6;
	ai				byteRotVal6,	pFrom,			13;
	lqd				inByteQw6,		0(pFrom);
	ai				bytesToCopy6,	write,			0;
	rotqby			inByte5,		inByteQw6,		byteRotVal6;
_ByteMemcpyLoopBack6:	/*Src and Dest do not overlap*/
	ai				pFrom,			pFrom,			1;
	lqd				outByteQw6,		0(pCurrOut);
	ai				bytesToCopy6,	bytesToCopy6,	-1;
	cbd				insByteMask6,	0(pCurrOut);
	ai				byteRotVal6_,	pFrom,			13;
	lqd				inByteQw6_,		0(pFrom);
	nop $111;
	shufb			outByteQw6,		inByte5,		outByteQw6,		insByteMask6;
	nop $82;
	rotqby			inByte5,		inByteQw6_,		byteRotVal6_;
	nop $65;
	stqd			outByteQw6,		0(pCurrOut);
	ai				pCurrOut,		pCurrOut,		1;
_____Hint_ByteMemcpyLoopBack6:
	brnz			bytesToCopy6,	_ByteMemcpyLoopBack6;

	sf				pFrom,			backwardsDist,	pCurrOut;
	br				_DoMainByteCopy;


_ContiguousInWindow:
	sf				windOffset3,	windowBackDist,	write;
	clgt			bSomeFromWindow,len,			windowBackDist;
	a				pFrom,			window,			windOffset3;
	brz				bSomeFromWindow,_DoMainByteCopy;
/*_SomeFromWindow:*/
	sf				len,			windowBackDist,	len;
	brz				windowBackDist,	_SetFromAndDoMainByteCopy;
	ai				byteRotVal4,	pFrom,			13;
	lqd				inByteQw4,		0(pFrom);
	ai				bytesToCopy4,	windowBackDist,	0;
	hbrr			_____Hint_ByteMemcpyLoopBack4, _ByteMemcpyLoopBack4;
	nop $74;
	rotqby			inByte4,		inByteQw4,		byteRotVal4;
_ByteMemcpyLoopBack4:	/*Src and Dest do not overlap*/
	ai				pFrom,			pFrom,			1;
	lqd				outByteQw4,		0(pCurrOut);
	ai				bytesToCopy4,	bytesToCopy4,	-1;
	cbd				insByteMask4,	0(pCurrOut);
	ai				byteRotVal4_,	pFrom,			13;
	lqd				inByteQw4_,		0(pFrom);
	nop $113;
	shufb			outByteQw4,		inByte4,		outByteQw4,		insByteMask4;
	nop $82;
	rotqby			inByte4,		inByteQw4_,		byteRotVal4_;
	nop $66;
	stqd			outByteQw4,		0(pCurrOut);
	ai				pCurrOut,		pCurrOut,		1;
_____Hint_ByteMemcpyLoopBack4:
	brnz			bytesToCopy4,	_ByteMemcpyLoopBack4;

	nop $127;
	br				_SetFromAndDoMainByteCopy;


_NonOverlapMemcpyLoop:
	andi			fromNybble,		pFrom,			0xF;
	hbrr			_____Hint5,		_NonOverlapMemcpyLoopBack;
	ai				pStoreByteQw,	falseStoreAddr,	0;					/*StoreByte qword will go to a false address (for when we branch to _TestConditionAndMainLoop later)*/
	lnop;
	nop $42;
	shlqby			maskIn128,		allFs,			fromNybble;

.align 6
_NonOverlapMemcpyLoopBack:
	selb			sizeThisLoop,	len,			sixteen,		isOver16Bytes;
	lqd				vFromL0,		0(pFrom);
	nop $34;
	lqd				vFromL1,		16(pFrom);
	ai				maskOutRot1_,	sizeThisLoop,	-16;
	lqd				vOut1__,			16(pCurrOut);
	andi			outNybble_,		pCurrOut,		0xF;
	lqd				vOut0,			0(pCurrOut);
	sfi				inputRot_,		pCurrOut,		16;
	rotqmby			maskOut128L,	allFs,			maskOutRot1_;
	sfi				maskOutRot2_,	sizeThisLoop,	16;
	rotqby			vFromL1,		vFromL1,		pFrom;
	sfi				maskOutRotB_,	outNybble_,		16;
	rotqby			vFromL0,		vFromL0,		pFrom;
	sfi				maskOutRotA_,	outNybble_,		0;
	lnop;
	nop $86;
	shlqby			maskOut128L,	maskOut128L,	maskOutRot2_;
	nop $127;
	lnop;
	selb			input128B,		vFromL1,		vFromL0,		maskIn128;
	lnop;
	nop $127;
	lnop;
	nop $87;
	shlqby			maskOut128LB,	maskOut128L,	maskOutRotB_;
	nop $80;
	rotqby			input128B,		input128B,		inputRot_;
	nop $83;
	rotqmby			maskOut128LA,	maskOut128L,	maskOutRotA_;
	nop $127;
	lnop;
	nop $127;
	lnop;
	selb			vOut1___,			vOut1__,			input128B,		maskOut128LB;
	lnop;
	selb			vOut0__,			vOut0,			input128B,		maskOut128LA;
	lnop;
	sf				len,			sizeThisLoop,	len;
	lnop;
	a				pFrom,			pFrom,			sizeThisLoop;
	stqd			vOut1___,			16(pCurrOut);
	clgti			isOver16Bytes,	len,			16;
	stqd			vOut0__,			0(pCurrOut);

	a				pCurrOut,		pCurrOut,		sizeThisLoop;
_____Hint5:
	brnz			len,			_NonOverlapMemcpyLoopBack;

	ai				pCurrOut,		pCurrOut,		-1;					/*Decrement in order to nullify the increment at _TestConditionAndMainLoop*/
	br				_TestConditionAndMainLoop;


.align 6
_ByteMemcpyLoop:
	ai				fromRot,		pFrom,			13;
	lqd				fromQword,		0(pFrom);
	nop $65;
	rotqby			fromQword,		fromQword,		fromRot;
	ai				pStoreByteQw,	falseStoreAddr,	0;					/*StoreByte qword will go to a false address  (for when we branch to _TestConditionAndMainLoop later)*/
	hbrr			_____Hint7,		_ByteMemcpyLoopBack;
_ByteMemcpyLoopBack:
	ai				pFrom,			pFrom,			1;
	lqd				toQword,		0(pCurrOut);
	ai				len,			len,			-1;
	lnop;
	ai				fromRot_,		pFrom,			13;
	cbd				insB0,			0(pCurrOut);
	nop $111;
	shufb			toQword_,		fromQword,		toQword,		insB0;
	nop $64;
	stqd			toQword_,		0(pCurrOut);
	ai				pCurrOut,		pCurrOut,		1;
	lqd				fromQword,		0(pFrom);
	nop $73;
	rotqby			fromQword,		fromQword,		fromRot_;
	nop $39;
_____Hint7:
	brnz			len,			_ByteMemcpyLoopBack;
	ai				pCurrOut,		pCurrOut,		-1;					/*Decrement in order to nullify the increment at _TestConditionAndMainLoop*/
	br				_TestConditionAndMainLoop;


.align 6
_GetHold_2:
	a				newTotBits_2,	numUdBtsInptQw,	numUdBtsInHold;
	hbrr			_____Hint_GetHold_2_Ret, _ReturnFromGetHold_2;
	nop $127;
	lnop;
	rotmi			extraBytes_2,	newTotBits_2,	-3;					/*Should really mask this off to just given 0x0 or 0x10, but we can do that later*/
	lnop;
	andi			numUdBtsInptQw,	newTotBits_2,	0x7F;
	lnop;
	ai				pInputQw16_2,	pInputQword_,	16;
	lnop;
	sfi				bitShift_2,		numUdBtsInptQw,	0x80;
	lqx				vecCurr_2,		extraBytes_2,	pInputQword_;
	nop $60;
	lqx				vecNext_2,		extraBytes_2,	pInputQw16_2;
	a				pInputQword_,	pInputQword_,	extraBytes_2;
	shlqbybi		mask128_2,		allFs,			bitShift_2;
	shufb			vecCurr_2,		vecCurr_2,		vecCurr_2,		endianSwap;
	shufb			vecNext_2,		vecNext_2,		vecNext_2,		endianSwap;
	andi			pInputQword_,	pInputQword_,	0xFFFFFFF0;			/*and now it's later,... pInputQword should remain qword aligned*/
	shlqbi			mask128_2,		mask128_2,		bitShift_2;
	rotqbybi		vecCurr_2,		vecCurr_2,		bitShift_2;
	rotqbybi		vecNext_2,		vecNext_2,		bitShift_2;
	rotqbi			vecCurr_2,		vecCurr_2,		bitShift_2;
	rotqbi			vecNext_2,		vecNext_2,		bitShift_2;
	selb			hold128,		vecCurr_2,		vecNext_2,		mask128_2;
	rotqbyi			hold128,		hold128,		12;
	il				numUdBtsInHold,	0;
_____Hint_GetHold_2_Ret:
	br				_ReturnFromGetHold_2;


.align 6
_GetHold_3:
	a				newTotBits_3,	numUdBtsInptQw,	numUdBtsInHold;
	hbrr			_____Hint_GetHold_3_Ret, _ReturnFromGetHold_3;
	nop $127;
	lnop;
	rotmi			extraBytes_3,	newTotBits_3,	-3;					/*Should really mask this off to just given 0x0 or 0x10, but we can do that later*/
	lnop;
	andi			numUdBtsInptQw,	newTotBits_3,	0x7F;
	lnop;
	ai				pInputQw16_3,	pInputQword_,	16;
	lnop;
	sfi				bitShift_3,		numUdBtsInptQw,0x80;
	lqx				vecCurr_3,		extraBytes_3,	pInputQword_;
	nop $60;
	lqx				vecNext_3,		extraBytes_3,	pInputQw16_3;
	a				pInputQword_,	pInputQword_,	extraBytes_3;
	shlqbybi		mask128_3,		allFs,			bitShift_3;
	shufb			vecNext_3,		vecNext_3,		vecNext_3,		endianSwap;
	shufb			vecCurr_3,		vecCurr_3,		vecCurr_3,		endianSwap;
	andi			pInputQword_,	pInputQword_,	0xFFFFFFF0;			/*and now it's later,... pInputQword should remain qword aligned*/
	shlqbi			mask128_3,		mask128_3,		bitShift_3;
	rotqbybi		vecNext_3,		vecNext_3,		bitShift_3;
	rotqbybi		vecCurr_3,		vecCurr_3,		bitShift_3;
	rotqbi			vecNext_3,		vecNext_3,		bitShift_3;
	rotqbi			vecCurr_3_,		vecCurr_3,		bitShift_3;
	selb			hold128,		vecCurr_3_,		vecNext_3,		mask128_3;
	rotqbyi			hold128,		hold128,		12;
	il				numUdBtsInHold,	0;
_____Hint_GetHold_3_Ret:
	br				_ReturnFromGetHold_3;


.align 6
_GetHold_4:
	a				newTotBits_4,	numUdBtsInptQw,	numUdBtsInHold;
	hbrr			_____Hint_GetHold_4_Ret, _ReturnFromGetHold_4;
	nop $127;
	lnop;
	rotmi			extraBytes_4,	newTotBits_4,	-3;					/*Should really mask this off to just given 0x0 or 0x10, but we can do that later*/
	lnop;
	andi			numUdBtsInptQw,	newTotBits_4,	0x7F;
	lnop;
	ai				pInputQw16_4,	pInputQword_,	16;
	lnop;
	sfi				bitShift_4,		numUdBtsInptQw,0x80;
	lqx				vecCurr_4,		extraBytes_4,	pInputQword_;
	nop $57;
	lqx				vecNext_4,		extraBytes_4,	pInputQw16_4;
	a				pInputQword_,	pInputQword_,	extraBytes_4;
	shlqbybi		mask128_4,		allFs,			bitShift_4;
	shufb			vecCurr_4,		vecCurr_4,		vecCurr_4,		endianSwap;
	shufb			vecNext_4,		vecNext_4,		vecNext_4,		endianSwap;
	andi			pInputQword_,	pInputQword_,	0xFFFFFFF0;			/*and now it's later,... pInputQword should remain qword aligned*/
	shlqbi			mask128_4,		mask128_4,		bitShift_4;
	rotqbybi		vecCurr_4,		vecCurr_4,		bitShift_4;
	rotqbybi		vecNext_4,		vecNext_4,		bitShift_4;
	rotqbi			vecCurr_4,		vecCurr_4,		bitShift_4;
	rotqbi			vecNext_4,		vecNext_4,		bitShift_4;
	selb			hold128,		vecCurr_4,		vecNext_4,		mask128_4;
	rotqbyi			hold128,		hold128,		12;
	il				numUdBtsInHold, 0;
_____Hint_GetHold_4_Ret:
	br				_ReturnFromGetHold_4;


_InvalidDistance:
	ila				pErrorString,	_edgeZlibInvalidDistanceCodeString;
	br				_CommonHandleErrorCode;

_InvalidLiteral:
	ila				pErrorString,	_edgeZlibInvalidLiteralLengthCodeString;
	br				_CommonHandleErrorCode;

_InvalidDistance_TooFarBack:
	ila				pErrorString,	_edgeZlibInvalidDistanceTooFarBackString;
_CommonHandleErrorCode:
	lqx				strmMsgQword,	strm,			offset24;
	a				pErrorString,	pErrorString,	picOffset;
	cwx				insrtStrmMsg,	strm,			offset24;
	shufb			strmMsgQword_,	pErrorString,	strmMsgQword,	insrtStrmMsg;
	stqx			strmMsgQword_,	strm,			offset24;
	lqd				stateModeQword__,	0(pState);
	cwd				modeInsertMsk,	0(pState);
	shufb			stateModeQword__,	kModeBad,		stateModeQword__,	modeInsertMsk;
	stqd			stateModeQword__,	0(pState);
	a				sumNumUsedBits,	numUdBtsInptQw,	numUdBtsInHold;
	ai				rndUpUsedBits3,	sumNumUsedBits,	7;
	rotmi			extraUsedBytes3,rndUpUsedBits3,	-3;
	a				pUsedIn,		pInputQword_,	extraUsedBytes3;
	nop $127;
	br				_EndOfMainLoop;

	.size _edgeZlib_inflate_fast_spu_asm, .-_edgeZlib_inflate_fast_spu_asm



/*////////////////////////////////////////////////////////////////////////*/

.section .data
	.align 4

.global _edgeZlibFalseStore
_edgeZlibFalseStore:
	.word 0x00000000
	.word 0x00000000
	.word 0x00000000
	.word 0x00000000



/*////////////////////////////////////////////////////////////////////////*/

.section .rodata
	.align 4

.global _edgeZlibEndianSwap_0F0E0D0C_0B0A0908_07060504_03020100
_edgeZlibEndianSwap_0F0E0D0C_0B0A0908_07060504_03020100:
	.word 0x0F0E0D0C
	.word 0x0B0A0908
	.word 0x07060504
	.word 0x03020100



/*////////////////////////////////////////////////////////////////////////*/

.section .rodata
	.align 4

.global _edgeZlibInvalidDistanceCodeString
_edgeZlibInvalidDistanceCodeString:
	.string "invalid distance code"



/*////////////////////////////////////////////////////////////////////////*/

.section .rodata
	.align 4

.global _edgeZlibInvalidLiteralLengthCodeString
_edgeZlibInvalidLiteralLengthCodeString:
	.string "invalid literal/length code"



/*////////////////////////////////////////////////////////////////////////*/

.section .rodata
	.align 4

.global _edgeZlibInvalidDistanceTooFarBackString
_edgeZlibInvalidDistanceTooFarBackString:
	.string "invalid distance too far back"



/*////////////////////////////////////////////////////////////////////////*/
