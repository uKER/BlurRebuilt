/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.3.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <spu_printf.h>

#include "edge/geom/edgegeom.h"

#ifdef EDGE_GEOM_DEBUG
extern void edgeGeomValidateProcessingOrder(EdgeGeomSpuContext *ctx, uint32_t currentOp);
#else
static inline void edgeGeomValidateProcessingOrder(EdgeGeomSpuContext *ctx, uint32_t currentOp){}
#endif

/* from edgegeom_masks.cpp */
extern const vec_uchar16 s_000C000000000000;
extern const vec_uchar16 s_0G0G0G0G0G0G0G0G;
extern const vec_uchar16 s_0A0C0E0G;
extern const vec_uchar16 s_0B0D0F0H;
extern const vec_uchar16 s_AACCEEGGIIKKMMOO;
extern const vec_uchar16 s_ABCD;
extern const vec_uchar16 s_AIBJCKDLEMFNGOHP;
extern const vec_uchar16 s_BBBBFFFFJJJJNNNN;
extern const vec_uchar16 s_BCDD;
extern const vec_uchar16 s_GGGGGGGGGGGGGGGG;

/* These MUST be here for software index decompression */
static vec_uint4 indexTriangleMaskTable[16] __attribute__((__aligned__(16))) =
{
    {0x06070a0b, 0x10110607, 0x10111213, 0x00000000},    // ACa ACa 0000 
    {0x06070a0b, 0x10111011, 0x0a0b1213, 0x00000000},    // ACa CBa 0001 
    {0x06070a0b, 0x10110a0b, 0x06071213, 0x00000000},    // ACa BAa 0010
    {0x06070a0b, 0x10111213, 0x14151617, 0x00000000},    // ACa abc 0011
    {0x0a0b0809, 0x10110a0b, 0x10111213, 0x00000000},    // CBa ACa 0100
    {0x0a0b0809, 0x10111011, 0x08091213, 0x00000000},    // CBa CBa 0101 
    {0x0a0b0809, 0x10110809, 0x0a0b1213, 0x00000000},    // CBa BAa 0110
    {0x0a0b0809, 0x10111213, 0x14151617, 0x00000000},    // CBa abc 0111
    {0x08090607, 0x10110809, 0x10111213, 0x00000000},    // BAa ACa 1000
    {0x08090607, 0x10111011, 0x06071213, 0x00000000},    // BAa CBa 1001 
    {0x08090607, 0x10110607, 0x08091213, 0x00000000},    // BAa BAa 1010 
    {0x08090607, 0x10111213, 0x14151617, 0x00000000},    // BAa abc 1011 
    {0x10111213, 0x14151011, 0x14151617, 0x00000000},    // abc ACa 1100               
    {0x10111213, 0x14151415, 0x12131617, 0x00000000},    // abc CBa 1101                
    {0x10111213, 0x14151213, 0x10111617, 0x00000000},    // abc BAa 1110                
    {0x10111213, 0x14151617, 0x18191a1b, 0x00000000},    // abc abc 1111                
};

static vec_uint4 indexInputMaskTable[16] __attribute__((__aligned__(16))) =
{
    {0x04050607, 0x08090a0b, 0x0c0d0e0f, 0x10111213},    // ACa ACa 0000   
    {0x04050607, 0x08090a0b, 0x0c0d0e0f, 0x10111213},    // ACa CBa 0001   
    {0x04050607, 0x08090a0b, 0x0c0d0e0f, 0x10111213},    // ACa BAa 0010   
    {0x08090a0b, 0x0c0d0e0f, 0x10111213, 0x14151617},    // ACa abc 0011   
    {0x04050607, 0x08090a0b, 0x0c0d0e0f, 0x10111213},    // CBa ACa 0100   
    {0x04050607, 0x08090a0b, 0x0c0d0e0f, 0x10111213},    // CBa CBa 0101   
    {0x04050607, 0x08090a0b, 0x0c0d0e0f, 0x10111213},    // CBa BAa 0110   
    {0x08090a0b, 0x0c0d0e0f, 0x10111213, 0x14151617},    // CBa abc 0111   
    {0x04050607, 0x08090a0b, 0x0c0d0e0f, 0x10111213},    // BAa ACa 1000   
    {0x04050607, 0x08090a0b, 0x0c0d0e0f, 0x10111213},    // BAa CBa 1001   
    {0x04050607, 0x08090a0b, 0x0c0d0e0f, 0x10111213},    // BAa BAa 1010   
    {0x08090a0b, 0x0c0d0e0f, 0x10111213, 0x14151617},    // BAa abc 1011   
    {0x08090a0b, 0x0c0d0e0f, 0x10111213, 0x14151617},    // abc ACa 1100   
    {0x08090a0b, 0x0c0d0e0f, 0x10111213, 0x14151617},    // abc CBa 1101   
    {0x08090a0b, 0x0c0d0e0f, 0x10111213, 0x14151617},    // abc BAa 1110   
    {0x0c0d0e0f, 0x10111213, 0x14151617, 0x18191a1b},    // abc abc 1111   
};

/**
 *	Decompresses software indexes
 * 
 * 	There are 4 main loops here, each doing 1 step in the decompression.  There is
 *  additional documentation inside the LibEdge-Overview document.  
 * 
 * 	The first loop decompress an n-bit stream into halfwords.  There are separate
 *  implementations for n <=8 and n > 8, due to the ability of n-bit values to straddle
 *  3 bytes, requiring additional work (9 is included in this group, even though it 
 *  can't straddle three bytes).  
 * 
 * 	For the n <= 8 case:
 * 		1) Load 3 qw
 * 		2) Align such that the first compressed index is aligned to the left of a quadword
 * 		3) Shuffle n-bit values into appropriate half words
 * 		4) Rotate into correct spot in half word
 * 		5) mask off bits n and higher
 * 		6) store
 * 	For the n > 8 case:
 * 		1) Load 6 qw
 * 		2) Align first 3 to front of quadword.  Align second 3 to front of quadword.
 * 		3) Shuffle portions of n-bit values into upper and lower splits.
 * 		4) Shift/rotate each split into place
 * 		5) Select to combine upper and lower bits.
 * 		6) Mask off garbage upper bits.
 * 		7) Store
 * 
 * 	The second loop applies the delta values.  For delta compression, each index decompressed
 * 	from the n-bit stream will have a base value subtracted from it, and will then be added 
 *  half-word-wise to the previous qw of indices.
 * 		1) Load 4 qw
 * 		2) Subtract base offset
 * 		3) Add previous qw of indices
 * 		4) Store
 * 		
 * 	Some additional info before describing the third loop: 
 * 		Actual construction of triangles is done in the 4th loop, using the output of the 3rd
 * 		loop as input.  Triangles are built by reusing previous triangles' indices, together
 * 		with a "new" index, or if none of the old indices can be reused, 3 new indices.  The
 * 		output of the 3rd loop is the stream from which these new indices are selected.
 * 
 * 		The "new index" stream as generated by the Edge Geometry tools is highly sequential; 
 * 		it is, in general, an incrementing series of indices, occasionally interrupted by an
 * 		index seen earlier in the list {1, 2, 3, 4, 1, 5, 6, 7, 3 . . . }.  The stream that
 * 		we've output from the second loop is an ordered list of the out of sequence entries,
 * 		in this case, {1,3}, for the second appearances of those indices.  
 * 
 * 	The third loop decompresses a 1-bit stream which specifies whether the next index in the
 * 	stream should be drawn from the incrementing sequence, or from the out of sequence list
 * 	output by the second loop.  
 * 		1) Load 1-bit control stream and out of sequence indexes
 * 		2) Align control stream
 * 		3) Build masks according to control bits
 * 		4) Shuffle sequence and index streams to produce output
 * 		5) Update sequence qw
 * 		6) Shuffle sequence and next set of indices to produce next output.
 * 		7) Update sequence
 * 		8) Store final output buffers
 * 
 *  The fourth loop builds the final triangle ordering, using the output of the third loop
 * 	as the input of "new" indexes.  A 2-bit pattern specifies which triangle ordering should
 *  be used (ACa, CBa, BAa, or abc, where A,B,C are from the previous triangle and a,b,c are
 *  the next indexes in the input stream).  This loop decompresses 8 triangles at a time.
 * 
 * 		1) Load control bits for 8 triangles (1 halfword)
 * 		2) Load enough indices for 8 triangles of all new indices
 * 		3) Rotate control bits to be used as offsets into lookup tables (4 bit indices)
 * 		4) Load triangle format masks and input stream update masks
 * 		5) Shuffle inputs to make first triangle.
 * 		6) Shuffle inputs to remove used indices.
 * 		7) Repeat 5 and 6 twice.  
 * 		8) Repeat 5 a third time (inputs needn't be realigned, since they'll be reloaded next 
 * 		   time through)
 * 		9) Shuffle triangles together to form final output stream.
 * 		10)Store outputs.
 * 
 * 	At this point we have our final index list, and can free the space previously taken up
 * 	by the block of index data.
 */
static inline void decompressSoftwareIndexes(EdgeGeomSpuContext *ctx, const void *indexes)
{
    const qword s_DDDDDDDDDDDDDDDD = (qword)spu_splats(0x03030303);
    const qword s_ABCD_qw = (qword)s_ABCD;
    const qword s_BCDD_qw = (qword)s_BCDD;

    qword AAAB = (qword)spu_splats(0x0000AAAB);
    unsigned int numTriangles = si_to_uint(si_and(si_mpy(si_from_ushort(ctx->spuConfigInfo.numIndexes), AAAB), si_fsmh(si_from_int(0x55))));

    qword pInput = si_from_ptr(indexes);
    qword startData = si_lqd(pInput, 0x00);

    unsigned int num1Bits = si_to_uint(si_and(si_shlqbyi(startData, 2), (qword)spu_splats(0x0000FFFF))) << 3;
    unsigned int bytesBefore2BitStream = 8 + ((num1Bits + 7) >> 3);
    unsigned int num2BitStreamBytes = (((numTriangles + numTriangles) + 7) >> 3);
    unsigned int bytesBeforeNBitStream = bytesBefore2BitStream + num2BitStreamBytes;

	unsigned int qwToWriteTo = (bytesBeforeNBitStream + 0xf) & ~0xf;
    qword pDecompressedNBits = si_a(pInput, si_from_uint(qwToWriteTo));
    qword pEndOfDecompressedSequence;
    qword pDecompressedSequence;

    /* BEGIN - DECOMPRESS N-BIT STREAM */
    {
        qword hwN = si_shufb(startData, startData, (qword)s_0G0G0G0G0G0G0G0G);
        qword  bN = si_shufb(startData, startData, (qword)s_GGGGGGGGGGGGGGGG);

        unsigned int n = si_to_uint(si_and(hwN, (qword)spu_splats(0x0000ffff)));

        if (n > 0) 
        {
            //okay, we skip 4*n bytes every loop iteration.  So, starting point of last iter = (4*n)*(numLoops-1)
            qword numIndexes = si_rotmi(startData, -16);
            qword numLoops = si_rotmi(si_ai(numIndexes, 31), -5);
    
            qword lastLoop = si_ai(numLoops, -1);
    
            qword bytesPerLoop = si_shlhi(hwN, 2);
            qword pInputStreamOffset = si_and(si_mpy(bytesPerLoop, lastLoop), (qword)spu_splats(0x0000FFFF));
    
            qword pInputStream = si_a(pInput, si_from_uint(bytesBeforeNBitStream));
    
            qword offsetMask = (qword)spu_splats(0x0f0f0f0f);
    
            //we need a pointer to the last outputs we're going to write
			qword pOutputStream0 = pDecompressedNBits;
            qword pOutputStream1 = si_ai(pOutputStream0, 0x10);
            qword pOutputStream2 = si_ai(pOutputStream0, 0x20);
            qword pOutputStream3 = si_ai(pOutputStream0, 0x30);
    
           //and we need the output offset
            qword pOutputStreamOffset = si_shli(lastLoop, 6);
    
            qword outputMaskShiftAmount = si_sfhi(hwN, 1);
            qword outputMask = si_roth(si_rotmah((qword)spu_splats(0x80008000), outputMaskShiftAmount), hwN);
    
            const qword multiplier = (qword){0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00, 0x08};
    
            qword shiftAmountIncHi = si_mpyhh(hwN, multiplier);
            qword shiftAmountIncLo = si_mpy(hwN, multiplier);
            qword shiftAmountInc = si_rotqmbyi(si_or(shiftAmountIncLo, si_shli(shiftAmountIncHi, 16)), -2);
    
            qword mods = si_andhi(shiftAmountInc, 7);
            qword bytePositions = si_rothmi(shiftAmountInc, -3);
    
            //predeclare
            qword s_offset = (qword)spu_splats(0);
    
            qword inputQuad0 = (qword)spu_splats(0);
            qword inputQuad1 = (qword)spu_splats(0);
            qword inputQuad2 = (qword)spu_splats(0);

            qword inputBuffer0 = (qword)spu_splats(0);
            qword inputBuffer1 = (qword)spu_splats(0);
            qword expanded0, expanded1, expanded2, expanded3;
            qword output0, output1, output2, output3;

            qword tmpAmount =  si_sfi(si_shli(si_from_ushort(n), 2), 0);
    
            if (n <= 8) 
            {
                qword pInputStreamOffsetInc = tmpAmount; 
    
                qword rollAmount = si_a(pInputStreamOffsetInc, pInputStreamOffsetInc);
    
                pInputStreamOffset = si_sf(rollAmount, pInputStreamOffset);
                qword pInputStream0 = si_a(pInputStream, rollAmount);
                qword pInputStream1 = si_ai(pInputStream0, 0x10);
                qword pInputStream2 = si_ai(pInputStream0, 0x20);
    
                qword offset = si_andi(si_a((qword)pInputStream0, (qword)pInputStreamOffset), 0xf);
                offset = si_shufb(offset, offset, (qword)s_DDDDDDDDDDDDDDDD);
    
                qword startingPos = si_sfhi((qword)hwN, 8);
                qword leftByte = si_andhi(si_cgth((qword)mods,startingPos), 1);
    
                qword s_mask_lower = si_a((qword)bytePositions, leftByte);
                qword s_mask_upper = si_shlqbyi(si_ahi(s_mask_lower, -1), 1);
                qword s_mask_mask = si_cgthi(s_mask_lower, 0);
                s_mask_upper = si_and(s_mask_mask, s_mask_upper);
    
                qword s_mask = si_or(s_mask_lower, s_mask_upper);
                qword s_mask0 = s_mask;
                qword s_mask1 = si_a(s_mask0, (qword)bN);
                qword s_mask2 = si_a(s_mask1, (qword)bN);
                qword s_mask3 = si_a(s_mask2, (qword)bN);
    
                qword baseShiftAmount = si_sfhi((qword)hwN, 8);
    
                qword shiftAmount = si_sfhi(si_andhi(si_sfh((qword)shiftAmountInc, baseShiftAmount), 7), 0);
    
                qword offsetInc = si_shlhi(si_sfhi((qword)hwN, 16), 2);
    
                offsetInc = si_or(offsetInc, si_shlhi(offsetInc, 8));
    
                qword inPosition0 = (qword)spu_splats(0);
                qword inPosition1 = (qword)spu_splats(0);
                qword inPosition2 = (qword)spu_splats(0);
                qword inPosition3 = (qword)spu_splats(0);
    
                qword pOutputStreamOffsetInc = (qword){ 0x00, 0x00, 0x00, 0x00, 
                                                        0x00, 0x00, 0x00, 0x00, 
                                                        0xFF, 0xFF, 0xFF, 0xC0,
                                                        0xFF, 0xFF, 0xFF, 0xC0};

                do
                {
                        inputBuffer0 = si_shufb(inputQuad0, inputQuad1, s_offset);
                        inputBuffer1 = si_shufb(inputQuad1, inputQuad2, s_offset);
                    
                    inputQuad0 = si_lqx(pInputStream0, pInputStreamOffset);
                    inputQuad1 = si_lqx(pInputStream1, pInputStreamOffset);
                    inputQuad2 = si_lqx(pInputStream2, pInputStreamOffset);
    
                            output0 = si_and(inPosition0, outputMask);
                            output1 = si_and(inPosition1, outputMask);
                            output2 = si_and(inPosition2, outputMask);
                            output3 = si_and(inPosition3, outputMask);
    
                            si_stqx(output0, pOutputStream0, pOutputStreamOffset);
                            si_stqx(output1, pOutputStream1, pOutputStreamOffset);
                            si_stqx(output2, pOutputStream2, pOutputStreamOffset);
                            si_stqx(output3, pOutputStream3, pOutputStreamOffset);
            
                    pInputStreamOffset = si_a(pInputStreamOffset, pInputStreamOffsetInc);
    
                        expanded0 = si_shufb(inputBuffer0, inputBuffer1, s_mask0);
                        expanded1 = si_shufb(inputBuffer0, inputBuffer1, s_mask1);
                        expanded2 = si_shufb(inputBuffer0, inputBuffer1, s_mask2);
                        expanded3 = si_shufb(inputBuffer0, inputBuffer1, s_mask3);
    
                        inPosition0 = si_rothm(expanded0, shiftAmount);
                        inPosition1 = si_rothm(expanded1, shiftAmount);
                        inPosition2 = si_rothm(expanded2, shiftAmount);
                        inPosition3 = si_rothm(expanded3, shiftAmount);
    
                    s_offset = si_a(offset, s_ABCD_qw);
                    offset = si_and(si_ah(offset, offsetInc), offsetMask);
    
                        pOutputStreamOffset = si_a(pOutputStreamOffset, pOutputStreamOffsetInc);
                        pOutputStreamOffsetInc = si_shufb(pOutputStreamOffsetInc, pOutputStreamOffsetInc, s_BCDD_qw);
    
                } while (si_to_int(pInputStreamOffset) > -1);
    
            }
            else
            {
                
                qword rollAmount = si_a(tmpAmount, tmpAmount);
    
                pInputStreamOffset = si_sf(rollAmount, pInputStreamOffset);
                qword pInputStream0 = si_a(pInputStream, rollAmount);
                qword pInputStream1 = si_ai(pInputStream0, 0x10);
                qword pInputStream2 = si_ai(pInputStream0, 0x20);
    
                qword pOutputStreamOffsetInc = (qword){ 0x00, 0x00, 0x00, 0x00, 
                                                        0x00, 0x00, 0x00, 0x00, 
                                                        0xFF, 0xFF, 0xFF, 0xC0,
                                                        0xFF, 0xFF, 0xFF, 0xC0};
    
                qword pInputStreamOffsetInc = si_sfi(si_a(si_from_ushort(n), si_from_ushort(n)), 0);
                pInputStreamOffset = si_sf(pInputStreamOffsetInc, (qword)pInputStreamOffset);
    
                qword offset = si_andi(si_a((qword)pInputStream0, (qword)pInputStreamOffset), 0xf);
                offset = si_shufb(offset, offset, (qword)s_DDDDDDDDDDDDDDDD);
    
                const qword s_DDDDHHHHLLLLPPPP = (qword) si_a((qword)s_BBBBFFFFJJJJNNNN, (qword)spu_splats(0x02020202));
                const qword s_AAAA = (qword)spu_splats(0x10203);
    
                qword baseShiftAmount = si_ahi((qword)hwN, -8);
    
                qword shiftAmount = si_andhi(si_ah(baseShiftAmount, (qword)shiftAmountInc), 7);
    
                qword startingPos = si_sfhi((qword)hwN, 16);
                qword noShift = si_cgth((qword)mods, startingPos);
                qword shufShiftAmount = si_sfhi(si_shli(si_ahi(noShift,1), 3), 0);
                qword shufShiftAmountBDFH = si_shufb(shufShiftAmount, shufShiftAmount, (qword)s_0B0D0F0H);
                qword shufShiftAmountACEG = si_shufb(shufShiftAmount, shufShiftAmount, (qword)s_0A0C0E0G);
    
                qword bigShiftAmount1_int = si_ai(si_shufb(shiftAmount, shiftAmount, (qword)s_0B0D0F0H), -8);
                bigShiftAmount1_int = si_a(bigShiftAmount1_int, si_andi(si_ceqi(bigShiftAmount1_int, -8), 8));
    
                qword bigShiftAmount0 = si_shufb(shiftAmount, shiftAmount, (qword)s_0A0C0E0G);
                qword bigShiftAmount1 = bigShiftAmount1_int;
    
                qword selector = si_fsmh(si_from_int(0x55));
    
                qword offsetInc = si_shlhi(si_sfhi((qword)hwN, 16), 1);
                offsetInc = si_or(offsetInc, si_shlhi(offsetInc, 8));
    
                qword byteBDFH = si_shufb((qword)bytePositions, (qword)bytePositions, s_DDDDHHHHLLLLPPPP);
                qword byteACEG = si_shufb((qword)bytePositions, (qword)bytePositions, (qword)s_BBBBFFFFJJJJNNNN);
    
                qword s_mask00 = si_rotm(si_a(byteACEG, s_AAAA), shufShiftAmountACEG);
                qword s_mask01 = si_rotmi(si_rotm(si_a(byteBDFH, s_AAAA),shufShiftAmountBDFH), -8);
                qword s_mask10 = si_a(s_mask00,(qword)bN);
                qword s_mask11 = si_a(s_mask01,(qword)bN);
    
                qword inputQuad3, inputQuad4, inputQuad5;
                qword inputBuffer2 = (qword)spu_splats(0);
                qword inputBuffer3 = (qword)spu_splats(0);
                qword splitInput00;
                qword splitInput01;
                qword splitInput10;
                qword splitInput11;
                qword splitInput20 = (qword)spu_splats(0);
                qword splitInput21 = (qword)spu_splats(0);
                qword splitInput30 = (qword)spu_splats(0);
                qword splitInput31 = (qword)spu_splats(0);
                qword expanded00 = (qword)spu_splats(0);
                qword expanded01 = (qword)spu_splats(0);
                qword expanded10 = (qword)spu_splats(0);
                qword expanded11 = (qword)spu_splats(0);
                qword expanded20;
                qword expanded21;
                qword expanded30;
                qword expanded31;
    
                do
                {
                            expanded20 = si_shl((qword)splitInput20, (qword)bigShiftAmount0);
                            expanded21 = si_rot((qword)splitInput21, (qword)bigShiftAmount1);
                            expanded30 = si_shl((qword)splitInput30, (qword)bigShiftAmount0);
                            expanded31 = si_rot((qword)splitInput31, (qword)bigShiftAmount1);
    
                    inputQuad0 = (vec_char16)si_lqx((qword)pInputStream0, (qword)pInputStreamOffset);
                    inputQuad1 = (vec_char16)si_lqx((qword)pInputStream1, (qword)pInputStreamOffset);
                    inputQuad2 = (vec_char16)si_lqx((qword)pInputStream2, (qword)pInputStreamOffset);
    
                            expanded0 = si_selb((qword)expanded00, (qword)expanded01, (qword)selector);
                            expanded1 = si_selb((qword)expanded10, (qword)expanded11, (qword)selector);
                            expanded2 = si_selb((qword)expanded20, (qword)expanded21, (qword)selector);
                            expanded3 = si_selb((qword)expanded30, (qword)expanded31, (qword)selector);
    
                        splitInput00 = si_shufb((qword)inputBuffer0, (qword)inputBuffer0, (qword)s_mask00);
                        splitInput01 = si_shufb((qword)inputBuffer0, (qword)inputBuffer0, (qword)s_mask01);
                        splitInput10 = si_shufb((qword)inputBuffer0, (qword)inputBuffer1, (qword)s_mask10);
                        splitInput11 = si_shufb((qword)inputBuffer0, (qword)inputBuffer1, (qword)s_mask11);
                        splitInput20 = si_shufb((qword)inputBuffer2, (qword)inputBuffer2, (qword)s_mask00);
                        splitInput21 = si_shufb((qword)inputBuffer2, (qword)inputBuffer2, (qword)s_mask01);
                        splitInput30 = si_shufb((qword)inputBuffer2, (qword)inputBuffer3, (qword)s_mask10);
                        splitInput31 = si_shufb((qword)inputBuffer2, (qword)inputBuffer3, (qword)s_mask11);
    
                            output0 = si_and((qword)expanded0, (qword)outputMask);
                            output1 = si_and((qword)expanded1, (qword)outputMask);
                            output2 = si_and((qword)expanded2, (qword)outputMask);
                            output3 = si_and((qword)expanded3, (qword)outputMask);
    
                    pInputStreamOffset = si_a((qword)pInputStreamOffset, (qword)pInputStreamOffsetInc);
    
                    inputQuad3 = si_lqx((qword)pInputStream0, (qword)pInputStreamOffset);
                    inputQuad4 = si_lqx((qword)pInputStream1, (qword)pInputStreamOffset);
                    inputQuad5 = si_lqx((qword)pInputStream2, (qword)pInputStreamOffset);
    
                        expanded00 = si_shl((qword)splitInput00, (qword)bigShiftAmount0);
                        expanded01 = si_rot((qword)splitInput01, (qword)bigShiftAmount1);
                        expanded10 = si_shl((qword)splitInput10, (qword)bigShiftAmount0);
                        expanded11 = si_rot((qword)splitInput11, (qword)bigShiftAmount1);
    
                            si_stqx((qword)output0, (qword)pOutputStream2, (qword)pOutputStreamOffset);
                            si_stqx((qword)output1, (qword)pOutputStream3, (qword)pOutputStreamOffset);
                            si_stqx((qword)output2, (qword)pOutputStream0, (qword)pOutputStreamOffset);
                            si_stqx((qword)output3, (qword)pOutputStream1, (qword)pOutputStreamOffset);
    
                    s_offset = si_a((qword)offset, (qword)s_ABCD_qw);
                    offset = si_and(si_ah((qword)offset, (qword)offsetInc), (qword)offsetMask);
    
                    pInputStreamOffset = si_a((qword)pInputStreamOffset, (qword)pInputStreamOffsetInc);                
    
                            pOutputStreamOffset = si_a((qword)pOutputStreamOffset, (qword)pOutputStreamOffsetInc);
    
                    inputBuffer0 = si_shufb((qword)inputQuad0, (qword)inputQuad1, (qword)s_offset);
                    inputBuffer1 = si_shufb((qword)inputQuad1, (qword)inputQuad2, (qword)s_offset);
    
                    s_offset = si_a((qword)offset, (qword)s_ABCD_qw);
                    offset = si_and(si_ah((qword)offset, (qword)offsetInc), (qword)offsetMask);
    
                            pOutputStreamOffsetInc = si_shufb(pOutputStreamOffsetInc, pOutputStreamOffsetInc, s_BCDD_qw);
                    
                    inputBuffer2 = si_shufb((qword)inputQuad3, (qword)inputQuad4, (qword)s_offset);
                    inputBuffer3 = si_shufb((qword)inputQuad4, (qword)inputQuad5, (qword)s_offset);
    
                } while (si_to_int(pInputStreamOffset) > -1);
            }
        }
		else // n == 0
		{
			si_stqd((qword)spu_splats(0), pDecompressedNBits, 0x00);
		}
    }
    /* END - DECOMPRESS N-BIT STREAM */

    /* BEGIN - DELTA*/

    //preamble

    const qword s_BBBBBBBB = (qword)spu_splats(0x02030203);
    qword deltaOffset = si_shufb(startData, startData, s_BBBBBBBB);
    qword pInputDeltaOffsetInc = (qword)spu_splats(0x00000040);
    qword pOutputDeltaOffsetInc = (qword)spu_splats(0x0);
    //predeclarations

    unsigned int numIndexes = si_to_uint(si_rotmi(startData, -16));
    if (numIndexes > 0) 
    {
        unsigned int deltaCounter = ((numIndexes + 31) >> 5) + 1;
    
        qword pInputDeltaOffset = (qword)spu_splats(0);
        qword pOutputDeltaOffset = (qword)spu_splats(0);
    
        qword inputDelta0; 
        qword inputDelta1; 
        qword inputDelta2; 
        qword inputDelta3;
        qword offsetDelta0 = (qword)spu_splats(0); 
        qword offsetDelta1 = (qword)spu_splats(0); 
        qword offsetDelta2 = (qword)spu_splats(0); 
        qword offsetDelta3 = (qword)spu_splats(0);
        qword outputDelta0; 
        qword outputDelta1; 
        qword outputDelta2;
        qword outputDelta3 = (qword)spu_splats(0);
    
        qword pDeltaInputStream0 = pDecompressedNBits;
        qword pDeltaInputStream1 = si_ai(pDeltaInputStream0, 0x10);
        qword pDeltaInputStream2 = si_ai(pDeltaInputStream0, 0x20);
        qword pDeltaInputStream3 = si_ai(pDeltaInputStream0, 0x30);
    
        do
        {
    
            inputDelta0 = si_lqx((qword)pDeltaInputStream0, (qword)pInputDeltaOffset);
            inputDelta1 = si_lqx((qword)pDeltaInputStream1, (qword)pInputDeltaOffset);
            inputDelta2 = si_lqx((qword)pDeltaInputStream2, (qword)pInputDeltaOffset);
            inputDelta3 = si_lqx((qword)pDeltaInputStream3, (qword)pInputDeltaOffset);
    
                outputDelta0 = si_ah((qword)offsetDelta0, (qword)outputDelta3);
                outputDelta1 = si_ah((qword)offsetDelta1, (qword)outputDelta0);
                outputDelta2 = si_ah((qword)offsetDelta2, (qword)outputDelta1);
                outputDelta3 = si_ah((qword)offsetDelta3, (qword)outputDelta2);
    
                si_stqx((qword)outputDelta0, (qword)pDeltaInputStream0, (qword)pOutputDeltaOffset);
                si_stqx((qword)outputDelta1, (qword)pDeltaInputStream1, (qword)pOutputDeltaOffset);
                si_stqx((qword)outputDelta2, (qword)pDeltaInputStream2, (qword)pOutputDeltaOffset);
                si_stqx((qword)outputDelta3, (qword)pDeltaInputStream3, (qword)pOutputDeltaOffset);
    
            pInputDeltaOffset = si_a((qword)pInputDeltaOffset, (qword)pInputDeltaOffsetInc);
    
            offsetDelta0 = si_sfh((qword)deltaOffset, (qword)inputDelta0);
            offsetDelta1 = si_sfh((qword)deltaOffset, (qword)inputDelta1);
            offsetDelta2 = si_sfh((qword)deltaOffset, (qword)inputDelta2);
            offsetDelta3 = si_sfh((qword)deltaOffset, (qword)inputDelta3);
    
                pOutputDeltaOffset = si_a((qword)pOutputDeltaOffset, (qword)pOutputDeltaOffsetInc);
                pOutputDeltaOffsetInc = si_shlqbyi(pInputDeltaOffsetInc, 0);
    
        } while((--deltaCounter) != 0);
    }
    /* END - DELTA */

    /* BEGIN - DECOMPRESS 1-BIT STREAM */
    {
        qword pControlStreamOffset = (qword)spu_splats(0x6);
        qword pControlStream = si_ai(pInput, 2);

        //compute output location for sequence
        unsigned int bytesBeforeDecompressedSequence = ((numTriangles * 6) + 0xf) & ~0xf;

        qword pOutputStream = si_a(pInput, si_from_uint(bytesBeforeDecompressedSequence));
        pDecompressedSequence = pOutputStream;

        qword pIndexStream = pDecompressedNBits;

        unsigned int counter = ((num1Bits + 15) >> 4) + 1;

        qword controlBits, alignedControlBits;
        qword bitCounts0, bitCounts1;
        qword controlBitsBC;
        qword selector = (qword)spu_splats(0);
        qword maskedBits;
        qword indexContrib = (qword)spu_splats(0);
        qword sequenceInc;
        qword sequenceContrib = (qword)spu_splats(0);
        qword interleavedMask;
        qword s_mask0, s_mask1;
        qword bytesUsed0, bytesUsed1;
        qword fromSequence0, fromSequence1;
        qword fromSequence0BC, fromSequence1BC;
        qword offset, offsetBC;
        qword s_offset;

        qword sequence = (qword){0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07};

        const qword controlBitsMask = (qword){0x00, 0x00, 0x80, 0x80, 0xc0, 0xc0, 0xe0, 0xe0, 0xf0, 0xf0, 0xf8, 0xf8, 0xfc, 0xfc, 0xfe, 0xfe};

        const qword s_BBBBBBBB = (qword)spu_splats(0x02030203);
        const qword s_BBDDFFHHJJLLNNPP = si_orbi((qword)s_AACCEEGGIIKKMMOO, 0x01);
        const qword subtractor = (qword){0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07};

        const qword eights = (qword)spu_splats(0x08080808);
        const qword thirtyTwo = (qword)spu_splats(0x20);

        bytesUsed0 = (qword)spu_splats(0);
        fromSequence0 = (qword)spu_splats(0);
        bytesUsed1 = (qword)spu_splats(0);
        fromSequence1 = (qword)spu_splats(0);

        qword pOutputStreamInc = (qword)spu_splats(0);

        do
        {
                qword inputQuad0 = si_lqd(pIndexStream, 0x00);
                qword inputQuad1 = si_lqd(pIndexStream, 0x10);

                interleavedMask = si_selb(sequenceContrib, indexContrib, selector);
                interleavedMask = si_a(interleavedMask, interleavedMask);

                s_mask0 = si_shufb(interleavedMask, interleavedMask, (qword)s_AACCEEGGIIKKMMOO);
                s_mask1 = si_shufb(interleavedMask, interleavedMask, s_BBDDFFHHJJLLNNPP);

                bytesUsed0 = si_a(bytesUsed0, bytesUsed0);
                fromSequence0BC = si_shufb(fromSequence0, fromSequence0, s_BBBBBBBB);

                s_mask0 = si_ahi(s_mask0, 0x1);
                s_mask1 = si_ahi(s_mask1, 0x1);

                offset = si_andi(pIndexStream, 0xf);
                offsetBC = si_shufb(offset, offset, s_DDDDDDDDDDDDDDDD);
                s_offset = si_a(s_ABCD_qw, offsetBC);

                pIndexStream = si_a(pIndexStream, bytesUsed0);

                qword inputBuffer0 = si_shufb(inputQuad0, inputQuad1, s_offset);
    
                qword outputBuffer0 = si_shufb(inputBuffer0, sequence, s_mask0);
    
                sequence = si_a(sequence, fromSequence0BC);
                fromSequence1BC = si_shufb(fromSequence1, fromSequence1, s_BBBBBBBB);
    
                qword inputQuad2 = si_lqd(pIndexStream, 0x00);
                qword inputQuad3 = si_lqd(pIndexStream, 0x10);
    
                offset = si_andi(pIndexStream, 0xf);
                offsetBC = si_shufb(offset, offset, s_DDDDDDDDDDDDDDDD);
                s_offset = si_a(s_ABCD_qw, offsetBC);

            controlBits = si_lqx(pControlStream, pControlStreamOffset);
            alignedControlBits = si_rotqby(controlBits, pControlStreamOffset);

                pIndexStream = si_a(pIndexStream, bytesUsed1);
    
                qword inputBuffer1 = si_shufb(inputQuad2, inputQuad3, s_offset);
    
                qword outputBuffer1 = si_shufb(inputBuffer1, sequence, s_mask1);
    
                sequence = si_a(sequence, fromSequence1BC);
    
                counter--;

            pControlStreamOffset = si_ai(pControlStreamOffset, 0x2);

            bitCounts0 = si_cntb(alignedControlBits);
            controlBitsBC = si_shufb(alignedControlBits, alignedControlBits, s_BBBBBBBB);

            selector = si_fsmb(alignedControlBits);
            selector = si_shufb(selector, selector, (qword)s_AIBJCKDLEMFNGOHP);

            maskedBits = si_and(controlBitsBC, controlBitsMask);
            indexContrib = si_cntb(maskedBits);
            sequenceInc = si_sf(indexContrib, subtractor);
            sequenceContrib = si_ah(sequenceInc, eights);

            bytesUsed0 = si_shufb(bitCounts0, bitCounts0, (qword)s_000C000000000000);
            fromSequence0 = si_sfi(bytesUsed0, 8);

                si_stqd(outputBuffer0, pOutputStream, 0x00);
                si_stqd(outputBuffer1, pOutputStream, 0x10);
    
                pOutputStream = si_a(pOutputStream, pOutputStreamInc);
                pOutputStreamInc = thirtyTwo;

            bitCounts1 = si_andi(bitCounts0, 0xf);
            fromSequence1 = si_sfi(bitCounts1, 8);
            bytesUsed1 = si_a(bitCounts1, bitCounts1);

        } while(counter != 0);
        pEndOfDecompressedSequence = pOutputStream;
    }
    /* END - DECOMPRESS 1-BIT STREAM */


    /* BEGIN - RELOCATE 2-BIT TABLE */
    vec_char16 p2BitStream = (vec_char16)spu_add((vec_uint4)pInput, (vec_uint4)si_from_uint(bytesBefore2BitStream));


    {
        unsigned int copyCount = -0x10;
        qword pInputStream = si_ai(p2BitStream, -0x10);
        qword pOutputStream = pEndOfDecompressedSequence;
        qword pOutputStreamInc = (qword)spu_splats(0);
        qword pOutputStreamIncValue = (qword)spu_splats(0x10);

        qword s_offset = (qword)spu_splats(0);
        qword inputQuad0 = (qword)spu_splats(0);
        qword inputQuad1 = (qword)spu_splats(0);

        do
        {
                qword output = si_shufb(inputQuad0, inputQuad1, s_offset);
                si_stqd(output, pOutputStream, 0x00);
                pInputStream = si_ai(pInputStream, 0x10);
                pOutputStream = si_a(pOutputStream, pOutputStreamInc);
                pOutputStreamInc = pOutputStreamIncValue;

            qword offset = si_andi(pInputStream, 0xf);
            qword offsetBC = si_shufb(offset, offset, s_DDDDDDDDDDDDDDDD);
            s_offset = si_a(offsetBC, s_ABCD_qw);
            inputQuad0 = si_lqd(pInputStream, 0x00);
            inputQuad1 = si_lqd(pInputStream, 0x10);

            copyCount += 0x10;

        } while (copyCount < num2BitStreamBytes);
    }
   
    /* END - COPY 2-BIT TABLE */

    /* BEGIN - DECOMPRESS 2-BIT STREAM */

    {
        const qword pTriangleMasks = si_from_ptr(indexTriangleMaskTable);
        const qword pInputMasks = si_from_ptr(indexInputMaskTable);

        const qword six = (qword)spu_splats(6);

        const qword s_output0 = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x10, 0x11, 0x12, 0x13};
        const qword s_output1 = {0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
        const qword s_output2 = {0x08, 0x09, 0x0a, 0x0b, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b};

        qword pControlStream = (qword)si_ai(pEndOfDecompressedSequence, 0x2);
        qword pControlStreamOffset = (qword)spu_splats(-0x2);

        qword pInputStream = pDecompressedSequence;
        qword pOutputStream = pInput;

        unsigned int triangleCounter = ((numTriangles + 7) >> 3) + 1;

        qword controlHw, alignedControlHw;
        qword newBits, newBitsCountPerByte, newBitsCountPerByteShifted;
        qword newBitsCount = (qword)spu_splats(0);
        qword oldBitsCount;
        qword newBytes = (qword)spu_splats(0);
        qword oldBytes = (qword)spu_splats(0);
        qword byteInc = (qword)spu_splats(0);

        qword inputQuad0 = (qword)spu_splats(0);
        qword inputQuad1 = (qword)spu_splats(0);
        qword inputQuad2 = (qword)spu_splats(0);
        qword inputQuad3 = (qword)spu_splats(0);
        qword offset, offsetBC;
        qword s_offset = (qword)spu_splats(0);

        qword inputBuffer00 = (qword)spu_splats(0); 
        qword inputBuffer10 = (qword)spu_splats(0); 
        qword inputBuffer20 = (qword)spu_splats(0);
        qword inputBuffer01; 
        qword inputBuffer11; 
        qword inputBuffer21;
        qword inputBuffer02; 
        qword inputBuffer12;
        qword inputBuffer03;

        qword pMaskOffset0 = (qword)spu_splats(0); 
        qword pMaskOffset1 = (qword)spu_splats(0); 
        qword pMaskOffset2 = (qword)spu_splats(0); 
        qword pMaskOffset3 = (qword)spu_splats(0);
        qword triangleMask0 = (qword)spu_splats(0); 
        qword triangleMask1 = (qword)spu_splats(0); 
        qword triangleMask2; 
        qword triangleMask3;
        qword inputMask0 = (qword)spu_splats(0); 
        qword inputMask1 = (qword)spu_splats(0); 
        qword inputMask2; 
        qword inputMask3;

        qword triangle01; 
        qword triangle23; 
        qword triangle45; 
        qword triangle67 = (qword)spu_splats(0);
        qword output0; 
        qword output1; 
        qword output2;

        qword AAAA = (qword)spu_splats(0xAAAA);
        qword controlMask3 = (qword)spu_splats(0xf);
        qword controlMask2 = si_shli(controlMask3, 4);
        qword controlMask1 = si_shli(controlMask3, 8);
        qword controlMask0 = si_shli(controlMask3, 12);

        qword pOutputStreamInc = (qword)spu_splats(0);

        do
        {
            controlHw = si_lqx(pControlStream, pControlStreamOffset);
            alignedControlHw = si_rotqby(controlHw, pControlStreamOffset);
            qword alignedControlHwShifted = si_shli(alignedControlHw, 1);
            pControlStreamOffset = si_ai(pControlStreamOffset, 0x2);

                newBytes = si_mpy(newBitsCount, six);

                byteInc = si_a(newBytes, oldBytes);

                pInputStream = si_a(pInputStream, byteInc);

                inputBuffer00 = si_shufb(inputQuad0, inputQuad1, s_offset);
                inputBuffer10 = si_shufb(inputQuad1, inputQuad2, s_offset);
                inputBuffer20 = si_shufb(inputQuad2, inputQuad3, s_offset);

                triangleMask2 = si_lqx(pTriangleMasks, pMaskOffset2);
                inputMask2 = si_lqx(pInputMasks, pMaskOffset2);
    
                triangleMask3 = si_lqx(pTriangleMasks, pMaskOffset3);
                inputMask3 = si_lqx(pInputMasks, pMaskOffset3);

                triangleMask1 = si_lqx(pTriangleMasks, pMaskOffset1);
                inputMask1 = si_lqx(pInputMasks, pMaskOffset1);

                triangle01 = si_shufb(triangle67, inputBuffer00, triangleMask0);
                inputBuffer01 = si_shufb(inputBuffer00, inputBuffer10, inputMask0);
                inputBuffer11 = si_shufb(inputBuffer10, inputBuffer20, inputMask0);
                inputBuffer21 = si_shufb(inputBuffer20, inputBuffer20, inputMask0);
    
                triangle23 = si_shufb(triangle01, inputBuffer01, triangleMask1);   
                inputBuffer02 = si_shufb(inputBuffer01, inputBuffer11, inputMask1);
                inputBuffer12 = si_shufb(inputBuffer11, inputBuffer21, inputMask1);
    
                triangle45 = si_shufb(triangle23, inputBuffer02, triangleMask2);   
                inputBuffer03 = si_shufb(inputBuffer02, inputBuffer12, inputMask2);
    
                triangle67 = si_shufb(triangle45, inputBuffer03, triangleMask3);   
    
                output0 = si_shufb(triangle01, triangle23, s_output0);             
                output1 = si_shufb(triangle23, triangle45, s_output1);             
                output2 = si_shufb(triangle45, triangle67, s_output2);             

            inputQuad0 = si_lqd(pInputStream, 0x00);
            inputQuad1 = si_lqd(pInputStream, 0x10);
            inputQuad2 = si_lqd(pInputStream, 0x20);
            inputQuad3 = si_lqd(pInputStream, 0x30);

                si_stqd(output0, pOutputStream, 0x00);                          
                si_stqd(output1, pOutputStream, 0x10);
                si_stqd(output2, pOutputStream, 0x20);
    
                pOutputStream = si_a(pOutputStream, pOutputStreamInc);
                pOutputStreamInc = si_il(0x30);
    
                triangleCounter--;

            newBits = si_and(si_and(alignedControlHw, alignedControlHwShifted), AAAA);
            newBitsCountPerByte = si_cntb(newBits);
            newBitsCountPerByteShifted = si_rotmi(newBitsCountPerByte, -8);
            newBitsCount = si_a(si_andi(newBitsCountPerByte,0xff), newBitsCountPerByteShifted);
            oldBitsCount = si_sfi(newBitsCount, 8);

            oldBytes = si_a(oldBitsCount, oldBitsCount);

            offset = si_andi(pInputStream, 0xf);
            offsetBC = si_shufb(offset, offset, s_DDDDDDDDDDDDDDDD);
            s_offset = si_a(offsetBC, s_ABCD_qw);

            pMaskOffset0 = si_rotmi(si_and(alignedControlHw, controlMask0), -8);
            pMaskOffset1 = si_rotmi(si_and(alignedControlHw, controlMask1), -4);            
            pMaskOffset2 = si_and(alignedControlHw, controlMask2);
            pMaskOffset3 = si_shli(si_and(alignedControlHw, controlMask3), 4);

            triangleMask0 = si_lqx(pTriangleMasks, pMaskOffset0);
            inputMask0 = si_lqx(pInputMasks, pMaskOffset0);

        } while (triangleCounter != 0);


        ctx->freePtr = (void*)si_to_ptr(pOutputStream);
        ctx->freePtr = (void*)(((unsigned int)ctx->freePtr + 0xf) & ~0xf);
    }

    /* END - DECOMPRESS 2-BIT STREAM */
}

void edgeGeomDecompressIndexes(EdgeGeomSpuContext *ctx, const void *indexes)
{
	edgeGeomValidateProcessingOrder(ctx, EDGE_GEOM_OPERATION_DECOMPRESS_INDEXES);

    if (ctx->spuConfigInfo.numIndexes == 0)
        return;

    switch((ctx->spuConfigInfo.indexesFlavorAndSkinningFlavor >> 4) & 0xF)
    {
//    case EDGE_GEOM_INDEXES_U16_TRIANGLE_LIST_CW:
//    case EDGE_GEOM_INDEXES_U16_TRIANGLE_LIST_CCW:
    case EDGE_GEOM_INDEXES_SW_TRIANGLE_LIST_CW:
        //winding order doesn't matter, hooray!
    case EDGE_GEOM_INDEXES_SW_TRIANGLE_LIST_CCW:
        decompressSoftwareIndexes(ctx, indexes);
        break;
    default:
        break;
    }

    ctx->indexesLs = (uint32_t)indexes;
}

void edgeGeomDecompressVertexesByFlavor(EdgeGeomSpuContext *ctx, const void *vertexesA, const void *vertexesB,
	const void *fixedOffsetsA, const void *fixedOffsetsB)
{
    if(ctx->spuConfigInfo.numVertexes == 0)
		return;
	ctx->vertexesLs = (uint32_t)vertexesA;

	// This sets up some global decompression structures which are
	// independent of the input flavor.  To reduce code bloat, it's
	// done once outside the loop instead of being duplicated inside
	// each case block.
	//
	// It also assumes that only a single input stream is used.  See
	// the multistream-sample for a demonstration of how to use
	// multiple streams in a pre-compiled input flavor.
	EDGE_GEOM_DECOMPRESS_INIT_GLOBAL_COMMON(ctx, vertexesA, ctx->spuConfigInfo.numVertexes,
		fixedOffsetsA);

	switch(ctx->spuConfigInfo.inputFlavor)
	{
	case EDGE_GEOM_INPUT_FLAVOR_F32c3:
		{
			// This flavor is special-cased for performance
			EDGE_GEOM_DECOMPRESS_SPECIALCASE_F32_3(ctx, vertexesA, ctx->spuConfigInfo.numVertexes);
		}
		break;
	case EDGE_GEOM_INPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N:
		{
			EDGE_GEOM_DECOMPRESS_INIT_GLOBAL(20);
			
			EDGE_GEOM_DECOMPRESS_INIT_F32(0, EDGE_GEOM_ATTRIBUTE_USAGE_POSITION, 0, 3);
			EDGE_GEOM_DECOMPRESS_INIT_X11Y11Z10N(1, EDGE_GEOM_ATTRIBUTE_USAGE_NORMAL, 12, 3);
			EDGE_GEOM_DECOMPRESS_INIT_X11Y11Z10N(2, EDGE_GEOM_ATTRIBUTE_USAGE_TANGENT, 16, 3);
			
			EDGE_GEOM_DECOMPRESS_LOAD_COMMON();
			do
			{
				EDGE_GEOM_DECOMPRESS_LOOP_START();
				
				EDGE_GEOM_DECOMPRESS_LOOP_F32(0);
				EDGE_GEOM_DECOMPRESS_LOOP_X11Y11Z10N(1);
				EDGE_GEOM_DECOMPRESS_LOOP_X11Y11Z10N(2);
				
				EDGE_GEOM_DECOMPRESS_LOOP_END();
			} while (!EDGE_GEOM_DECOMPRESS_LOOP_DONE());
			
			EDGE_GEOM_DECOMPRESS_FINALIZE_F32(0);
			EDGE_GEOM_DECOMPRESS_FINALIZE_X11Y11Z10N(1);
			EDGE_GEOM_DECOMPRESS_FINALIZE_X11Y11Z10N(2);
		}
		break;
	case EDGE_GEOM_INPUT_FLAVOR_F32c3_X11Y11Z10N_F16c4:
		{
			EDGE_GEOM_DECOMPRESS_INIT_GLOBAL(24);
			
			EDGE_GEOM_DECOMPRESS_INIT_F32(0, EDGE_GEOM_ATTRIBUTE_USAGE_POSITION, 0, 3);
			EDGE_GEOM_DECOMPRESS_INIT_X11Y11Z10N(1, EDGE_GEOM_ATTRIBUTE_USAGE_NORMAL, 12, 3);
			EDGE_GEOM_DECOMPRESS_INIT_F16(2, EDGE_GEOM_ATTRIBUTE_USAGE_TANGENT, 16, 4);
			
			EDGE_GEOM_DECOMPRESS_LOAD_COMMON();
			do
			{
				EDGE_GEOM_DECOMPRESS_LOOP_START();
				
				EDGE_GEOM_DECOMPRESS_LOOP_F32(0);
				EDGE_GEOM_DECOMPRESS_LOOP_X11Y11Z10N(1);
				EDGE_GEOM_DECOMPRESS_LOOP_F16(2);
				
				EDGE_GEOM_DECOMPRESS_LOOP_END();
			} while (!EDGE_GEOM_DECOMPRESS_LOOP_DONE());
			
			EDGE_GEOM_DECOMPRESS_FINALIZE_F32(0);
			EDGE_GEOM_DECOMPRESS_FINALIZE_X11Y11Z10N(1);
			EDGE_GEOM_DECOMPRESS_FINALIZE_F16(2);
		}
		break;
	case EDGE_GEOM_INPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_F16c2:
		{
			EDGE_GEOM_DECOMPRESS_INIT_GLOBAL(24);
			
			EDGE_GEOM_DECOMPRESS_INIT_F32(0, EDGE_GEOM_ATTRIBUTE_USAGE_POSITION, 0, 3);
			EDGE_GEOM_DECOMPRESS_INIT_X11Y11Z10N(1, EDGE_GEOM_ATTRIBUTE_USAGE_NORMAL, 12, 3);
			EDGE_GEOM_DECOMPRESS_INIT_X11Y11Z10N(2, EDGE_GEOM_ATTRIBUTE_USAGE_TANGENT, 16, 3);
			EDGE_GEOM_DECOMPRESS_INIT_F16(3, EDGE_GEOM_ATTRIBUTE_USAGE_GENERIC, 20, 2);
			
			EDGE_GEOM_DECOMPRESS_LOAD_COMMON();
			do
			{
				EDGE_GEOM_DECOMPRESS_LOOP_START();
				
				EDGE_GEOM_DECOMPRESS_LOOP_F32(0);
				EDGE_GEOM_DECOMPRESS_LOOP_X11Y11Z10N(1);
				EDGE_GEOM_DECOMPRESS_LOOP_X11Y11Z10N(2);
				EDGE_GEOM_DECOMPRESS_LOOP_F16(3);
				
				EDGE_GEOM_DECOMPRESS_LOOP_END();
			} while (!EDGE_GEOM_DECOMPRESS_LOOP_DONE());
			
			EDGE_GEOM_DECOMPRESS_FINALIZE_F32(0);
			EDGE_GEOM_DECOMPRESS_FINALIZE_X11Y11Z10N(1);
			EDGE_GEOM_DECOMPRESS_FINALIZE_X11Y11Z10N(2);
			EDGE_GEOM_DECOMPRESS_FINALIZE_F16(3);
		}
		break;
	case EDGE_GEOM_INPUT_FLAVOR_F32c3_X11Y11Z10N_F16c4_F16c2:
		{
			EDGE_GEOM_DECOMPRESS_INIT_GLOBAL(28);
			
			EDGE_GEOM_DECOMPRESS_INIT_F32(0, EDGE_GEOM_ATTRIBUTE_USAGE_POSITION, 0, 3);
			EDGE_GEOM_DECOMPRESS_INIT_X11Y11Z10N(1, EDGE_GEOM_ATTRIBUTE_USAGE_NORMAL, 12, 3);
			EDGE_GEOM_DECOMPRESS_INIT_F16(2, EDGE_GEOM_ATTRIBUTE_USAGE_TANGENT, 16, 4);
			EDGE_GEOM_DECOMPRESS_INIT_F16(3, EDGE_GEOM_ATTRIBUTE_USAGE_GENERIC, 24, 2);
			
			EDGE_GEOM_DECOMPRESS_LOAD_COMMON();
			do
			{
				EDGE_GEOM_DECOMPRESS_LOOP_START();
				
				EDGE_GEOM_DECOMPRESS_LOOP_F32(0);
				EDGE_GEOM_DECOMPRESS_LOOP_X11Y11Z10N(1);
				EDGE_GEOM_DECOMPRESS_LOOP_F16(2);
				EDGE_GEOM_DECOMPRESS_LOOP_F16(3);
				
				EDGE_GEOM_DECOMPRESS_LOOP_END();
			} while (!EDGE_GEOM_DECOMPRESS_LOOP_DONE());
			
			EDGE_GEOM_DECOMPRESS_FINALIZE_F32(0);
			EDGE_GEOM_DECOMPRESS_FINALIZE_X11Y11Z10N(1);
			EDGE_GEOM_DECOMPRESS_FINALIZE_F16(2);
			EDGE_GEOM_DECOMPRESS_FINALIZE_F16(3);
		}
		break;
	case EDGE_GEOM_INPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N:
		{
			EDGE_GEOM_DECOMPRESS_INIT_GLOBAL(24);
			
			EDGE_GEOM_DECOMPRESS_INIT_F32(0, EDGE_GEOM_ATTRIBUTE_USAGE_POSITION, 0, 3);
			EDGE_GEOM_DECOMPRESS_INIT_X11Y11Z10N(1, EDGE_GEOM_ATTRIBUTE_USAGE_NORMAL, 12, 3);
			EDGE_GEOM_DECOMPRESS_INIT_X11Y11Z10N(2, EDGE_GEOM_ATTRIBUTE_USAGE_TANGENT, 16, 3);
			EDGE_GEOM_DECOMPRESS_INIT_X11Y11Z10N(3, EDGE_GEOM_ATTRIBUTE_USAGE_BINORMAL, 20, 3);
			
			EDGE_GEOM_DECOMPRESS_LOAD_COMMON();
			do
			{
				EDGE_GEOM_DECOMPRESS_LOOP_START();
				
				EDGE_GEOM_DECOMPRESS_LOOP_F32(0);
				EDGE_GEOM_DECOMPRESS_LOOP_X11Y11Z10N(1);
				EDGE_GEOM_DECOMPRESS_LOOP_X11Y11Z10N(2);
				EDGE_GEOM_DECOMPRESS_LOOP_X11Y11Z10N(3);
				
				EDGE_GEOM_DECOMPRESS_LOOP_END();
			} while (!EDGE_GEOM_DECOMPRESS_LOOP_DONE());
	
			EDGE_GEOM_DECOMPRESS_FINALIZE_F32(0);
			EDGE_GEOM_DECOMPRESS_FINALIZE_X11Y11Z10N(1);
			EDGE_GEOM_DECOMPRESS_FINALIZE_X11Y11Z10N(2);
			EDGE_GEOM_DECOMPRESS_FINALIZE_X11Y11Z10N(3);
		}
		break;
	case EDGE_GEOM_INPUT_FLAVOR_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N_F16c2:
		{
			EDGE_GEOM_DECOMPRESS_INIT_GLOBAL(28);
			
			EDGE_GEOM_DECOMPRESS_INIT_F32(0, EDGE_GEOM_ATTRIBUTE_USAGE_POSITION, 0, 3);
			EDGE_GEOM_DECOMPRESS_INIT_X11Y11Z10N(1, EDGE_GEOM_ATTRIBUTE_USAGE_NORMAL, 12, 3);
			EDGE_GEOM_DECOMPRESS_INIT_X11Y11Z10N(2, EDGE_GEOM_ATTRIBUTE_USAGE_TANGENT, 16, 3);
			EDGE_GEOM_DECOMPRESS_INIT_X11Y11Z10N(3, EDGE_GEOM_ATTRIBUTE_USAGE_BINORMAL, 20, 3);
			EDGE_GEOM_DECOMPRESS_INIT_F16(4, EDGE_GEOM_ATTRIBUTE_USAGE_GENERIC, 24, 2);
			
			EDGE_GEOM_DECOMPRESS_LOAD_COMMON();
			do
			{
				EDGE_GEOM_DECOMPRESS_LOOP_START();
				
				EDGE_GEOM_DECOMPRESS_LOOP_F32(0);
				EDGE_GEOM_DECOMPRESS_LOOP_X11Y11Z10N(1);
				EDGE_GEOM_DECOMPRESS_LOOP_X11Y11Z10N(2);
				EDGE_GEOM_DECOMPRESS_LOOP_X11Y11Z10N(3);
				EDGE_GEOM_DECOMPRESS_LOOP_F16(4);
				
				EDGE_GEOM_DECOMPRESS_LOOP_END();
			} while (!EDGE_GEOM_DECOMPRESS_LOOP_DONE());
			
			EDGE_GEOM_DECOMPRESS_FINALIZE_F32(0);
			EDGE_GEOM_DECOMPRESS_FINALIZE_X11Y11Z10N(1);
			EDGE_GEOM_DECOMPRESS_FINALIZE_X11Y11Z10N(2);
			EDGE_GEOM_DECOMPRESS_FINALIZE_X11Y11Z10N(3);
			EDGE_GEOM_DECOMPRESS_FINALIZE_F16(4);
		}
		break;
	case EDGE_GEOM_INPUT_FLAVOR_F32c3_F16c2:
		{
			EDGE_GEOM_DECOMPRESS_INIT_GLOBAL(16);
			
			EDGE_GEOM_DECOMPRESS_INIT_F32(0, EDGE_GEOM_ATTRIBUTE_USAGE_POSITION, 0, 3);
			EDGE_GEOM_DECOMPRESS_INIT_F16(1, EDGE_GEOM_ATTRIBUTE_USAGE_GENERIC, 12, 2);
			
			EDGE_GEOM_DECOMPRESS_LOAD_COMMON();
			do
			{
				EDGE_GEOM_DECOMPRESS_LOOP_START();
				
				EDGE_GEOM_DECOMPRESS_LOOP_F32(0);
				EDGE_GEOM_DECOMPRESS_LOOP_F16(1);
				
				EDGE_GEOM_DECOMPRESS_LOOP_END();
			} while (!EDGE_GEOM_DECOMPRESS_LOOP_DONE());
			
			EDGE_GEOM_DECOMPRESS_FINALIZE_F32(0);
			EDGE_GEOM_DECOMPRESS_FINALIZE_F16(1);
		}
		break;
	case EDGE_GEOM_INPUT_FLAVOR_FIXED_UNITVEC_UNITVEC:
		{
			EDGE_GEOM_DECOMPRESS_INIT_GLOBAL(12);
			
			EDGE_GEOM_DECOMPRESS_INIT_FIXED_POINT(0, EDGE_GEOM_ATTRIBUTE_USAGE_POSITION, 0, 3, 4, 12, 4, 12, 4, 12, 0, 0);
			EDGE_GEOM_DECOMPRESS_INIT_UNIT_VECTOR(1, EDGE_GEOM_ATTRIBUTE_USAGE_NORMAL, 6, 3);
			EDGE_GEOM_DECOMPRESS_INIT_UNIT_VECTOR(2, EDGE_GEOM_ATTRIBUTE_USAGE_TANGENT, 9, 3);
			
			EDGE_GEOM_DECOMPRESS_LOAD_COMMON();
			do
			{
				EDGE_GEOM_DECOMPRESS_LOOP_START();
				
				EDGE_GEOM_DECOMPRESS_LOOP_FIXED_POINT(0);
				EDGE_GEOM_DECOMPRESS_LOOP_UNIT_VECTOR(1);
				EDGE_GEOM_DECOMPRESS_LOOP_UNIT_VECTOR(2);
				
				EDGE_GEOM_DECOMPRESS_LOOP_END();
			} while (!EDGE_GEOM_DECOMPRESS_LOOP_DONE());
			
			EDGE_GEOM_DECOMPRESS_FINALIZE_FIXED_POINT(0);
			EDGE_GEOM_DECOMPRESS_FINALIZE_UNIT_VECTOR(1);
			EDGE_GEOM_DECOMPRESS_FINALIZE_UNIT_VECTOR(2);

		}
		break;
	case EDGE_GEOM_INPUT_FLAVOR_FIXED_UNITVEC_UNITVEC_F16c2:
		{
			EDGE_GEOM_DECOMPRESS_INIT_GLOBAL(16);
			
			EDGE_GEOM_DECOMPRESS_INIT_FIXED_POINT(0, EDGE_GEOM_ATTRIBUTE_USAGE_POSITION, 0, 3, 4, 12, 4, 12, 4, 12, 0, 0);
			EDGE_GEOM_DECOMPRESS_INIT_UNIT_VECTOR(1, EDGE_GEOM_ATTRIBUTE_USAGE_NORMAL, 6, 3);
			EDGE_GEOM_DECOMPRESS_INIT_UNIT_VECTOR(2, EDGE_GEOM_ATTRIBUTE_USAGE_TANGENT, 9, 3);
			EDGE_GEOM_DECOMPRESS_INIT_F16(3, EDGE_GEOM_ATTRIBUTE_USAGE_GENERIC, 12, 2);
			
			EDGE_GEOM_DECOMPRESS_LOAD_COMMON();
			do
			{
				EDGE_GEOM_DECOMPRESS_LOOP_START();
				
				EDGE_GEOM_DECOMPRESS_LOOP_FIXED_POINT(0);

                EDGE_GEOM_DECOMPRESS_LOOP_UNIT_VECTOR(1);
				EDGE_GEOM_DECOMPRESS_LOOP_UNIT_VECTOR(2);
				EDGE_GEOM_DECOMPRESS_LOOP_F16(3);
				
				EDGE_GEOM_DECOMPRESS_LOOP_END();
			} while (!EDGE_GEOM_DECOMPRESS_LOOP_DONE());
			
			EDGE_GEOM_DECOMPRESS_FINALIZE_FIXED_POINT(0);
			EDGE_GEOM_DECOMPRESS_FINALIZE_UNIT_VECTOR(1);
			EDGE_GEOM_DECOMPRESS_FINALIZE_UNIT_VECTOR(2);
			EDGE_GEOM_DECOMPRESS_FINALIZE_F16(3);
		}
		break;
	case EDGE_GEOM_INPUT_FLAVOR_FIXED_UNITVEC_UNITVEC_UNITVEC:
		{
			EDGE_GEOM_DECOMPRESS_INIT_GLOBAL(15);
			
			EDGE_GEOM_DECOMPRESS_INIT_FIXED_POINT(0, EDGE_GEOM_ATTRIBUTE_USAGE_POSITION, 0, 3, 4, 12, 4, 12, 4, 12, 0, 0);
			EDGE_GEOM_DECOMPRESS_INIT_UNIT_VECTOR(1, EDGE_GEOM_ATTRIBUTE_USAGE_NORMAL, 6, 3);
			EDGE_GEOM_DECOMPRESS_INIT_UNIT_VECTOR(2, EDGE_GEOM_ATTRIBUTE_USAGE_TANGENT, 9, 3);
			EDGE_GEOM_DECOMPRESS_INIT_UNIT_VECTOR(3, EDGE_GEOM_ATTRIBUTE_USAGE_BINORMAL, 12, 3);
			
			EDGE_GEOM_DECOMPRESS_LOAD_COMMON();
			do
			{
				EDGE_GEOM_DECOMPRESS_LOOP_START();
				
				EDGE_GEOM_DECOMPRESS_LOOP_FIXED_POINT(0);
				EDGE_GEOM_DECOMPRESS_LOOP_UNIT_VECTOR(1);
				EDGE_GEOM_DECOMPRESS_LOOP_UNIT_VECTOR(2);
				EDGE_GEOM_DECOMPRESS_LOOP_UNIT_VECTOR(3);
				
				EDGE_GEOM_DECOMPRESS_LOOP_END();
			} while (!EDGE_GEOM_DECOMPRESS_LOOP_DONE());
			
			EDGE_GEOM_DECOMPRESS_FINALIZE_FIXED_POINT(0);
			EDGE_GEOM_DECOMPRESS_FINALIZE_UNIT_VECTOR(1);
			EDGE_GEOM_DECOMPRESS_FINALIZE_UNIT_VECTOR(2);
			EDGE_GEOM_DECOMPRESS_FINALIZE_UNIT_VECTOR(3);
		}
		break;
	case EDGE_GEOM_INPUT_FLAVOR_FIXED_UNITVEC_UNITVEC_UNITVEC_F16c2:
		{
			EDGE_GEOM_DECOMPRESS_INIT_GLOBAL(19);
			
			EDGE_GEOM_DECOMPRESS_INIT_FIXED_POINT(0, EDGE_GEOM_ATTRIBUTE_USAGE_POSITION, 0, 3, 4, 12, 4, 12, 4, 12, 0, 0);
			EDGE_GEOM_DECOMPRESS_INIT_UNIT_VECTOR(1, EDGE_GEOM_ATTRIBUTE_USAGE_NORMAL, 6, 3);
			EDGE_GEOM_DECOMPRESS_INIT_UNIT_VECTOR(2, EDGE_GEOM_ATTRIBUTE_USAGE_TANGENT, 9, 3);
			EDGE_GEOM_DECOMPRESS_INIT_UNIT_VECTOR(3, EDGE_GEOM_ATTRIBUTE_USAGE_BINORMAL, 12, 3);
			EDGE_GEOM_DECOMPRESS_INIT_F16(4, EDGE_GEOM_ATTRIBUTE_USAGE_GENERIC, 15, 2);
			
			EDGE_GEOM_DECOMPRESS_LOAD_COMMON();
			do
			{
				EDGE_GEOM_DECOMPRESS_LOOP_START();
				
				EDGE_GEOM_DECOMPRESS_LOOP_FIXED_POINT(0);
				EDGE_GEOM_DECOMPRESS_LOOP_UNIT_VECTOR(1);
				EDGE_GEOM_DECOMPRESS_LOOP_UNIT_VECTOR(2);
				EDGE_GEOM_DECOMPRESS_LOOP_UNIT_VECTOR(3);
				EDGE_GEOM_DECOMPRESS_LOOP_F16(4);
		
				EDGE_GEOM_DECOMPRESS_LOOP_END();
			} while (!EDGE_GEOM_DECOMPRESS_LOOP_DONE());
			
			EDGE_GEOM_DECOMPRESS_FINALIZE_FIXED_POINT(0);
			EDGE_GEOM_DECOMPRESS_FINALIZE_UNIT_VECTOR(1);
			EDGE_GEOM_DECOMPRESS_FINALIZE_UNIT_VECTOR(2);
			EDGE_GEOM_DECOMPRESS_FINALIZE_UNIT_VECTOR(3);
			EDGE_GEOM_DECOMPRESS_FINALIZE_F16(4);
		}
		break;
	default:
		if(ctx->decompressInputCallback != 0)
		{
			(*ctx->decompressInputCallback)(ctx, vertexesA, vertexesB, ctx->spuConfigInfo.numVertexes,
				fixedOffsetsA, fixedOffsetsB, ctx->spuConfigInfo.inputFlavor);
		}
		else
		{
			spu_printf("EDGE error: unknown input flavor %d for vertex stream", ctx->spuConfigInfo.inputFlavor);
			si_stopd(si_from_uint(0), si_from_uint(0), si_from_uint(0));
		}
		break;
	}

	// Only allow the input vertexes to be overwritten if we're not using the fast path
	if (ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH)
	{
		// We want to set the free pointer to the end of the vertex
		// stream.  Generally we don't know the input vertex stride,
		// but in the fast path it happens to be the same as the output
		// stride!
		uint32_t streamSize = (edgeGeomGetOutputFlavorStride(ctx, ctx->spuConfigInfo.outputFlavor) * ctx->spuConfigInfo.numVertexes + 0xF) & ~0xF;
		ctx->freePtr = (void*)((uint32_t)vertexesA + streamSize);
	}
	else
	{
		ctx->freePtr = (void*)(vertexesB ? ((vertexesA < vertexesB) ? vertexesA : vertexesB) : vertexesA);
	}
}

void edgeGeomBlendVertexesByFlavor(EdgeGeomSpuContext *ctx, const void *vertexDeltas, const void *fixedOffsets, float alpha)
{
	if(ctx->spuConfigInfo.numVertexes == 0)
		return;

	EDGE_GEOM_DECOMPRESS_INIT_GLOBAL_COMMON(ctx, vertexDeltas, ctx->spuConfigInfo.numVertexes, fixedOffsets);

	switch( ctx->spuConfigInfo.shapeFlavor )
	{
	case EDGE_GEOM_SHAPE_FLAVOR_FIXEDc3:
		{
			EDGE_GEOM_DECOMPRESS_INIT_BLEND_GLOBAL(6, alpha);
			
			EDGE_GEOM_DECOMPRESS_INIT_FIXED_POINT(0, EDGE_GEOM_ATTRIBUTE_USAGE_POSITION, 0, 3, 4, 12, 4, 12, 4, 12, 0, 0);
			
			EDGE_GEOM_DECOMPRESS_LOAD_COMMON();
			do
			{
				EDGE_GEOM_DECOMPRESS_LOOP_START();
				
				EDGE_GEOM_DECOMPRESS_LOOP_FIXED_POINT(0);
				
				EDGE_GEOM_DECOMPRESS_LOOP_END();
			} while (!EDGE_GEOM_DECOMPRESS_LOOP_DONE());
			
			EDGE_GEOM_DECOMPRESS_FINALIZE_FIXED_POINT(0);
		}
		break;
	case EDGE_GEOM_SHAPE_FLAVOR_FIXEDc3_F16c3:
		{
			EDGE_GEOM_DECOMPRESS_INIT_BLEND_GLOBAL(12, alpha);
			
			EDGE_GEOM_DECOMPRESS_INIT_FIXED_POINT(0, EDGE_GEOM_ATTRIBUTE_USAGE_POSITION, 0, 3, 4, 12, 4, 12, 4, 12, 0, 0);
			EDGE_GEOM_DECOMPRESS_INIT_F16(1, EDGE_GEOM_ATTRIBUTE_USAGE_NORMAL, 6, 3);
			
			EDGE_GEOM_DECOMPRESS_LOAD_COMMON();
			do
			{
				EDGE_GEOM_DECOMPRESS_LOOP_START();
				
				EDGE_GEOM_DECOMPRESS_LOOP_FIXED_POINT(0);
				EDGE_GEOM_DECOMPRESS_LOOP_F16(1);
				
				EDGE_GEOM_DECOMPRESS_LOOP_END();
			} while (!EDGE_GEOM_DECOMPRESS_LOOP_DONE());
			
			EDGE_GEOM_DECOMPRESS_FINALIZE_FIXED_POINT(0);
			EDGE_GEOM_DECOMPRESS_FINALIZE_F16(1);
		}
		break;
	case EDGE_GEOM_SHAPE_FLAVOR_F32c3_F32c3:
		{
			EDGE_GEOM_DECOMPRESS_INIT_BLEND_GLOBAL(24, alpha);
			
			EDGE_GEOM_DECOMPRESS_INIT_F32(0, EDGE_GEOM_ATTRIBUTE_USAGE_POSITION, 0, 3);
			EDGE_GEOM_DECOMPRESS_INIT_F32(1, EDGE_GEOM_ATTRIBUTE_USAGE_NORMAL, 12, 3);
			
			EDGE_GEOM_DECOMPRESS_LOAD_COMMON();
			do
			{
				EDGE_GEOM_DECOMPRESS_LOOP_START();
				
				EDGE_GEOM_DECOMPRESS_LOOP_F32(0);
				EDGE_GEOM_DECOMPRESS_LOOP_F32(1);
				
				EDGE_GEOM_DECOMPRESS_LOOP_END();
			} while (!EDGE_GEOM_DECOMPRESS_LOOP_DONE());
			
			EDGE_GEOM_DECOMPRESS_FINALIZE_F32(0);
			EDGE_GEOM_DECOMPRESS_FINALIZE_F32(1);
		}
		break;
	default:
		if(ctx->decompressBlendCallback != 0)
		{
			(*ctx->decompressBlendCallback)(ctx, vertexDeltas, ctx->spuConfigInfo.numVertexes,
											fixedOffsets, ctx->spuConfigInfo.shapeFlavor, alpha);
		}
		else
		{
			spu_printf("EDGE error: unknown flavor %d for blend shape stream", ctx->spuConfigInfo.shapeFlavor);
			si_stopd(si_from_int(0), si_from_int(0), si_from_int(0));
		}
		break;
	}
}
