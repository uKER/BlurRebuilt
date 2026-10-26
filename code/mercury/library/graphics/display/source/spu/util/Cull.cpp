#include "Display/Spu/Display.h"
#include "Display/Spu/Device/DeviceTypes.h"
#include "Display/Spu/Table/IndexTable.h"
#include "Display/Spu/Table/PositionTable.h"
#include "Display/Spu/Table/VertexTable.h"
#include "Display/Spu/Util/View.h"
#include "Display/Spu/Util/Cull.h"



namespace Display
{
namespace Spu
{



extern const vec_uchar16 s_0A0B0c0d;
extern const vec_uchar16 s_AAAA;
extern const vec_uchar16 s_AaBb;
extern const vec_uchar16 s_AaCc;
extern const vec_uchar16 s_ABCD;
extern const vec_uchar16 s_ABcd;
extern const vec_uchar16 s_BbAa;
extern const vec_uchar16 s_BBBB;
extern const vec_uchar16 s_BbDd;
extern const vec_uchar16 s_BCDD;
extern const vec_uchar16 s_CcAa;
extern const vec_uchar16 s_CCCC;
extern const vec_uchar16 s_CcCc;
extern const vec_uchar16 s_cdAB;
extern const vec_uchar16 s_DDDD;
extern const vec_uchar16 s_DdBb;

const qword s_ABCDEFGHd000MNOP = {0x00, 0x01, 0x02, 0x03,  0x04, 0x05, 0x06, 0x07, 0x13, 0x80, 0x80, 0x80, 0x0c, 0x0d, 0x0e, 0x0f};
const qword s_ABa0 = {0x00, 0x01, 0x02, 0x03,  0x04, 0x05, 0x06, 0x07,  0x10, 0x11, 0x12, 0x13,  0x80, 0x80, 0x80, 0x80};
const qword s_CDa0 = {0x08, 0x09, 0x0a, 0x0b,  0x0c, 0x0d, 0x0e, 0x0f,  0x10, 0x11, 0x12, 0x13,  0x80, 0x80, 0x80, 0x80};
const qword s_CDab = {0x08, 0x09, 0x0a, 0x0b, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
const qword s_FBDbdf00 = {0x0a, 0x0b,  0x02, 0x03,  0x06, 0x07,  0x12, 0x13,  0x16, 0x17,  0x1a, 0x1b,  0x80, 0x80,  0x80, 0x80};

/*
 *	Transforms 4 vertices at a time into screen space, precomputes off-screen test, w=0 test, 
 *  quantizations (dependent on multisample type) for each vertex (for degenerate test) and 
 *  stuffs results into Z and W components.  Flips X and Y coordinates for vertices behind 
 *  the inversion point.
 * 
 *  The loop:
 * 	1) Load 4 vertices 
 * 	2) Transpose vertices
 * 	3) Multiple by local to projection matrix
 * 	4) Compute 1/W
 * 	5) Homogenize X, Y, Z
 * 	6) Flip sign of X, Y if W is negative
 * 	7) Compare w against zero (which invalidates some tests)
 * 	8) Scale and offset X, Y, Z into viewport
 * 	9) Convert X, Y to signed ints
 *  10)Mask of any necessary bits to quantize according to sample type
 * 	11)Combine quantized X and Y into a word.
 * 	12)Untranspose transformed X, Y, Z
 * 	13)Compare against frustum
 * 	14)Collect frustum test results into bit pattern
 * 	15)Move frustum test bits into Z component
 * 	16)Move quantized X and Y with W==0 as high bit into W
 * 	17)Store results
 */
void TransformPositionTable(const MathsStorage::MatrixCol44 &matWorld, const Display::Spu::C_View * pView, Display::Spu::C_PositionTable * pPositionTable)
{
	//constants, shuffles
	const qword fOne = (qword)spu_splats(1.0f);
    const qword floatZero = (qword)spu_splats(0.0f);

	//distribute viewport scale and offset values
	vec_char16 pViewportScales = (vec_char16)si_from_ptr(pView->m_ViewportScales);
	vec_char16 pViewportOffsets = (vec_char16)si_from_ptr(pView->m_ViewportOffsets);
	vec_float4 vpScale = (vec_float4)si_lqd(pViewportScales, 0x00);
	vec_float4 vpOffset = (vec_float4)si_lqd(pViewportOffsets, 0x00);
	qword viewportScaleX = (qword)spu_shuffle(vpScale, vpScale, s_AAAA);
	qword viewportScaleY = (qword)spu_shuffle(vpScale, vpScale, s_BBBB);
	qword viewportScaleZ = (qword)spu_shuffle(vpScale, vpScale, s_CCCC);
	qword viewportOffsetX = (qword)spu_shuffle(vpOffset, vpOffset, s_AAAA);
	qword viewportOffsetY = (qword)spu_shuffle(vpOffset, vpOffset, s_BBBB);
	qword viewportOffsetZ = (qword)spu_shuffle(vpOffset, vpOffset, s_CCCC);

    uint8_t vpSampleFlavor = pView->m_MultisampleType;

    vec_char16 pScissorArea = (vec_char16)si_from_ptr(pView->m_ScissorRect);
    vec_ushort8 scissorArea = (vec_ushort8)si_lqd(pScissorArea,0x00);
    vec_ushort8 scissorAreaSum = spu_add(scissorArea, spu_rlmaskqwbyte(scissorArea, -4));
    //spread, convert, broadcast broadcast broadcast broadcast
    qword scissorAreaW = si_shufb((qword)scissorArea, (qword)scissorAreaSum, (qword)s_0A0B0c0d);
    qword scissorAreaF = si_cuflt(scissorAreaW, 0);

    const vec_float4 depthRange0 = (vec_float4)spu_shuffle(scissorArea, scissorArea, s_CCCC);
    const vec_float4 depthRange1 = (vec_float4)spu_shuffle(scissorArea, scissorArea, s_DDDD);

    const vec_uint4 flipDepthRange = spu_cmpgt(depthRange0, depthRange1);
    const qword frustumMaxZz = (qword)spu_sel(depthRange1, depthRange0, flipDepthRange);
    const qword frustumMinZz = (qword)spu_sel(depthRange0, depthRange1, flipDepthRange);

    const qword frustumMax = si_shufb(scissorAreaF, frustumMaxZz, s_CDa0);
    const qword frustumMin = si_shufb(scissorAreaF, frustumMinZz, s_ABa0);

	//load and distribute local to world and view projection matrices
	vec_char16 pLocalToWorldMatrix = (vec_char16)si_from_ptr(&(matWorld));
	vec_float4 localToWorld0 = (vec_float4)si_lqd(pLocalToWorldMatrix, 0x00);
	vec_float4 localToWorld1 = (vec_float4)si_lqd(pLocalToWorldMatrix, 0x10);
	vec_float4 localToWorld2 = (vec_float4)si_lqd(pLocalToWorldMatrix, 0x20);
	vec_float4 localToWorld3 =  {0.0f, 0.0f, 0.0f, 1.0f};

	vec_char16 pViewProjectionMatrix = (vec_char16)si_from_ptr(&(pView->m_ViewProjMatrix));
	vec_float4 viewProjMat0 = (vec_float4)si_lqd(pViewProjectionMatrix, 0x00);
	vec_float4 viewProjMat1 = (vec_float4)si_lqd(pViewProjectionMatrix, 0x10);
	vec_float4 viewProjMat2 = (vec_float4)si_lqd(pViewProjectionMatrix, 0x20);
	vec_float4 viewProjMat3 = (vec_float4)si_lqd(pViewProjectionMatrix, 0x30);

	//compute LocalToProj matrix from LocalToWorld and ViewportProjection matrices
	vec_float4 vp00 = spu_shuffle(viewProjMat0, viewProjMat0, s_AAAA);
	vec_float4 vp10 = spu_shuffle(viewProjMat1, viewProjMat1, s_AAAA);
	vec_float4 vp20 = spu_shuffle(viewProjMat2, viewProjMat2, s_AAAA);
	vec_float4 vp30 = spu_shuffle(viewProjMat3, viewProjMat3, s_AAAA);

	vec_float4 localToProjContrib00 = spu_mul(vp00, localToWorld0);
	vec_float4 localToProjContrib10 = spu_mul(vp10, localToWorld0);
	vec_float4 localToProjContrib20 = spu_mul(vp20, localToWorld0);
	vec_float4 localToProjContrib30 = spu_mul(vp30, localToWorld0);

	vec_float4 vp01 = spu_shuffle(viewProjMat0, viewProjMat0, s_BBBB);
	vec_float4 vp11 = spu_shuffle(viewProjMat1, viewProjMat1, s_BBBB);
	vec_float4 vp21 = spu_shuffle(viewProjMat2, viewProjMat2, s_BBBB);
	vec_float4 vp31 = spu_shuffle(viewProjMat3, viewProjMat3, s_BBBB);

	vec_float4 localToProjContrib01 = spu_madd(vp01, localToWorld1, localToProjContrib00);
	vec_float4 localToProjContrib11 = spu_madd(vp11, localToWorld1, localToProjContrib10);
	vec_float4 localToProjContrib21 = spu_madd(vp21, localToWorld1, localToProjContrib20);
	vec_float4 localToProjContrib31 = spu_madd(vp31, localToWorld1, localToProjContrib30);

	vec_float4 vp02 = spu_shuffle(viewProjMat0, viewProjMat0, s_CCCC);
	vec_float4 vp12 = spu_shuffle(viewProjMat1, viewProjMat1, s_CCCC);
	vec_float4 vp22 = spu_shuffle(viewProjMat2, viewProjMat2, s_CCCC);
	vec_float4 vp32 = spu_shuffle(viewProjMat3, viewProjMat3, s_CCCC);

	vec_float4 localToProjContrib02 = spu_madd(vp02, localToWorld2, localToProjContrib01);
	vec_float4 localToProjContrib12 = spu_madd(vp12, localToWorld2, localToProjContrib11);
	vec_float4 localToProjContrib22 = spu_madd(vp22, localToWorld2, localToProjContrib21);
	vec_float4 localToProjContrib32 = spu_madd(vp32, localToWorld2, localToProjContrib31);

	vec_float4 vp03 = spu_shuffle(viewProjMat0, viewProjMat0, s_DDDD);
	vec_float4 vp13 = spu_shuffle(viewProjMat1, viewProjMat1, s_DDDD);
	vec_float4 vp23 = spu_shuffle(viewProjMat2, viewProjMat2, s_DDDD);
	vec_float4 vp33 = spu_shuffle(viewProjMat3, viewProjMat3, s_DDDD);

	vec_float4 localToProj0 = spu_madd(vp03, localToWorld3, localToProjContrib02);
	vec_float4 localToProj1 = spu_madd(vp13, localToWorld3, localToProjContrib12);
	vec_float4 localToProj2 = spu_madd(vp23, localToWorld3, localToProjContrib22);
	vec_float4 localToProj3 = spu_madd(vp33, localToWorld3, localToProjContrib32);

	//distribute localToProj
	qword localToProj00 = (qword)spu_shuffle(localToProj0, localToProj0, s_AAAA);
	qword localToProj10 = (qword)spu_shuffle(localToProj1, localToProj1, s_AAAA);
	qword localToProj20 = (qword)spu_shuffle(localToProj2, localToProj2, s_AAAA);
	qword localToProj30 = (qword)spu_shuffle(localToProj3, localToProj3, s_AAAA);

	qword localToProj01 = (qword)spu_shuffle(localToProj0, localToProj0, s_BBBB);
	qword localToProj11 = (qword)spu_shuffle(localToProj1, localToProj1, s_BBBB);
	qword localToProj21 = (qword)spu_shuffle(localToProj2, localToProj2, s_BBBB);
	qword localToProj31 = (qword)spu_shuffle(localToProj3, localToProj3, s_BBBB);

	qword localToProj02 = (qword)spu_shuffle(localToProj0, localToProj0, s_CCCC);
	qword localToProj12 = (qword)spu_shuffle(localToProj1, localToProj1, s_CCCC);
	qword localToProj22 = (qword)spu_shuffle(localToProj2, localToProj2, s_CCCC);
	qword localToProj32 = (qword)spu_shuffle(localToProj3, localToProj3, s_CCCC);

	qword localToProj03 = (qword)spu_shuffle(localToProj0, localToProj0, s_DDDD);
	qword localToProj13 = (qword)spu_shuffle(localToProj1, localToProj1, s_DDDD);
	qword localToProj23 = (qword)spu_shuffle(localToProj2, localToProj2, s_DDDD);
	qword localToProj33 = (qword)spu_shuffle(localToProj3, localToProj3, s_DDDD);

	unsigned int numIterations = (pPositionTable->m_Count + 3) >> 2;

	int adjustVertexPointer = numIterations << 6;
	int negativeAdjustVertexPointer = -adjustVertexPointer;

	qword outputOffset = si_from_int(negativeAdjustVertexPointer);
	qword outputOffsetInc = (qword)
	{ 
		0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x40
	};

	vec_uint4 pTransVerts = (vec_uint4)si_from_ptr(pPositionTable->m_pBuffer);
	qword pTransVerts0 = (qword)spu_add(pTransVerts,adjustVertexPointer);
	qword pTransVerts1 = si_ai(pTransVerts0, 0x10);
	qword pTransVerts2 = si_ai(pTransVerts0, 0x20);
	qword pTransVerts3 = si_ai(pTransVerts0, 0x30);

	adjustVertexPointer += 0x80;
	negativeAdjustVertexPointer = -adjustVertexPointer;

	qword pVerts0 = (qword)spu_add((vec_uint4)si_from_ptr(pPositionTable->m_pBuffer), adjustVertexPointer);
	qword pVerts1 = si_ai(pVerts0, 0x10);
	qword pVerts2 = si_ai(pVerts0, 0x20);
	qword pVerts3 = si_ai(pVerts0, 0x30);

	qword inputOffset = si_from_int(negativeAdjustVertexPointer - 0x40);
	qword inputOffsetInc = si_from_int(0x40);

    const qword signMask = (qword)spu_splats(0x80000000);
    
    const qword s_AbCdEfGh = {0x00, 0x01, 0x12, 0x13, 0x04, 0x05, 0x16, 0x17, 0x08, 0x09, 0x1a, 0x1b, 0x0c, 0x0d, 0x1e, 0x1f};

    unsigned int sampleFlavor = 0;
    switch (vpSampleFlavor) 
    {
    case CELL_GCM_SURFACE_CENTER_1:				sampleFlavor = 0; break;
    case CELL_GCM_SURFACE_DIAGONAL_CENTERED_2:	sampleFlavor = 1; break;
    case CELL_GCM_SURFACE_SQUARE_CENTERED_4:	sampleFlavor = 1; break;
    case CELL_GCM_SURFACE_SQUARE_ROTATED_4:		sampleFlavor = 2; break;
    }

    const qword sampleFlavorBC = (qword)spu_splats(sampleFlavor);
    qword sampleMask = (qword)spu_splats(-1);
    sampleMask = si_selb(sampleMask, (qword)spu_splats(-4), si_ceqi(sampleFlavorBC, 0));
    sampleMask = si_selb(sampleMask, (qword)spu_splats(-2), si_ceqi(sampleFlavorBC, 1));

	// predeclare variables
    qword resW = (qword)spu_splats(0);
    qword zzzz = (qword)spu_splats(0);
    qword mMultYw = (qword)spu_splats(0);
    qword z2w2z0w0 = (qword)spu_splats(0);
    qword zzzzScaled = (qword)spu_splats(0);
    qword resWcopy = (qword)spu_splats(0);
    qword z3w3z1w1 = (qword)spu_splats(0);
    qword mMultYx = (qword)spu_splats(0);
    qword yyyy = (qword)spu_splats(0);

    qword mMultXx = (qword)spu_splats(0);
    qword mMultYy = (qword)spu_splats(0);
    qword mMultXy = (qword)spu_splats(0);
    qword xyz0 = (qword)spu_splats(0);
    qword xxxxScaled = (qword)spu_splats(0);
    qword xxxxUnitSigned = (qword)spu_splats(0);
    qword xyz2Trans = (qword)spu_splats(0);
    qword x0y0x2y2 = (qword)spu_splats(0);
    qword mMultYz = (qword)spu_splats(0);
    qword mMultXz = (qword)spu_splats(0);
    qword xyz3Trans = (qword)spu_splats(0);
    qword x1y1x3y3 = (qword)spu_splats(0);
    qword yyyyScaled = (qword)spu_splats(0);
    qword yyyyUnitSigned = (qword)spu_splats(0);
    qword tmp0 = (qword)spu_splats(0);
    qword zzzzUnit = (qword)spu_splats(0);
    qword xyz2 = (qword)spu_splats(0);
    qword over2 = (qword)spu_splats(0);
    qword xyz3 = (qword)spu_splats(0);
    qword over3 = (qword)spu_splats(0);
    qword xyz0Trans = (qword)spu_splats(0);
    qword recipEstW = (qword)spu_splats(0);
    qword xyz1Trans = (qword)spu_splats(0);
    qword under2 = (qword)spu_splats(0);
    qword xyz1 = (qword)spu_splats(0);
    qword under3 = (qword)spu_splats(0);
    qword over0 = (qword)spu_splats(0);
    qword frustumTest2Tmp0 = (qword)spu_splats(0);                
    qword under1 = (qword)spu_splats(0);
    qword yyxx23 = (qword)spu_splats(0);
    qword under0 = (qword)spu_splats(0);
    qword frustumTest3Tmp0 = (qword)spu_splats(0);
    qword over1 = (qword)spu_splats(0);
    qword zzzz23 = (qword)spu_splats(0);
    qword tmp1 = (qword)spu_splats(0);
    qword xxyy01 = (qword)spu_splats(0);
    qword xxxxAsSigned = (qword)spu_splats(0);
    qword frustumTest0Tmp0 = (qword)spu_splats(0);
    qword yyyyAsSigned = (qword)spu_splats(0);
    qword frustumTest2 = (qword)spu_splats(0);
    qword signW = (qword)spu_splats(0);
    qword frustumTest3 = (qword)spu_splats(0);
    qword resY = (qword)spu_splats(0);
    qword xxxx = (qword)spu_splats(0);
    qword resX = (qword)spu_splats(0);
    qword frustumTest1Tmp0 = (qword)spu_splats(0);
    qword recipW = (qword)spu_splats(0);
    qword xyz2Out = (qword)spu_splats(0);
    qword resZ = (qword)spu_splats(0);
    qword frustumTest0 = (qword)spu_splats(0);
    qword mMultXw = (qword)spu_splats(0);
    qword xyz3Out = (qword)spu_splats(0);
    qword xxxxQuantized = (qword)spu_splats(0);
    qword frustumTest1 = (qword)spu_splats(0);
    qword xyz0Out = (qword)spu_splats(0);
    qword xxxxUnit = (qword)spu_splats(0);
    qword yyyyUnit = (qword)spu_splats(0);
    qword xyz1Out = (qword)spu_splats(0);
    qword yyyyQuantized = (qword)spu_splats(0);
    qword xxxxShifted = (qword)spu_splats(0);
    qword zzzz01 = (qword)spu_splats(0);
    qword xyxyxyxy = (qword)spu_splats(0);
    qword wSafe = (qword)spu_splats(0);

	/*  Loop iteration delimited by tabbing
		For example:
		1.  First iteration
			2.  Second Iteration
				3.  Third Iteration
					4.  Fourth Iteration
	*/

	do
	{
            resW = si_fma(zzzz, localToProj32, mMultYw);			        
                    z2w2z0w0 = si_shufb(zzzzScaled, resWcopy, (qword)s_CcAa);
        inputOffset = si_a(inputOffset, inputOffsetInc);
                    z3w3z1w1 = si_shufb(zzzzScaled, resWcopy, (qword)s_DdBb);
            mMultYx = si_fma(yyyy, localToProj01, mMultXx);

            mMultYy = si_fma(yyyy, localToProj11, mMultXy);
        xyz0 = si_lqx(pVerts0, inputOffset);
                xxxxScaled = si_fma(xxxxUnitSigned, viewportScaleX, viewportOffsetX);
                    xyz2Trans = si_shufb(z2w2z0w0, x0y0x2y2, (qword)s_cdAB);
            mMultYz = si_fma(yyyy, localToProj21, mMultXz);
                    xyz3Trans = si_shufb(z3w3z1w1, x1y1x3y3, (qword)s_cdAB);
                yyyyScaled = si_fma(yyyyUnitSigned, viewportScaleY, viewportOffsetY);
            tmp0 = si_frest(resW);
                zzzzScaled = si_fma(zzzzUnit, viewportScaleZ, viewportOffsetZ);
        xyz2 = si_lqx(pVerts2, inputOffset);
                    over2 = si_fcgt(xyz2Trans, frustumMax);
        xyz3 = si_lqx(pVerts3, inputOffset);
                    over3 = si_fcgt(xyz3Trans, frustumMax);
                    xyz0Trans = si_shufb(x0y0x2y2, z2w2z0w0, (qword)s_ABcd);
            recipEstW = si_fi(resW, tmp0);					
                    xyz1Trans = si_shufb(x1y1x3y3, z3w3z1w1, (qword)s_ABcd);
                    under2 = si_fcgt(frustumMin, xyz2Trans);
        xyz1 = si_lqx(pVerts1, inputOffset);
                    under3 = si_fcgt(frustumMin, xyz3Trans);
                x1y1x3y3 = si_shufb(xxxxScaled, yyyyScaled, (qword)s_BbDd);
                    over0 = si_fcgt(xyz0Trans, frustumMax);
                    frustumTest2Tmp0 = si_shufb(under2, over2, s_FBDbdf00);
                    under1 = si_fcgt(frustumMin, xyz1Trans);				
        yyxx23 = si_shufb(xyz2, xyz3, (qword)s_BbAa);
                    under0 = si_fcgt(frustumMin, xyz0Trans);
                    frustumTest3Tmp0 = si_shufb(under3, over3, s_FBDbdf00);
                    over1 = si_fcgt(xyz1Trans, frustumMax);
        zzzz23 = si_shufb(xyz2, xyz3, (qword)s_CcCc);
            tmp1 = si_fnms(recipEstW, resW, fOne);				
        xxyy01 = si_shufb(xyz0, xyz1, (qword)s_AaBb);
                xxxxAsSigned = si_cflts(xxxxScaled, 3);				
                    frustumTest0Tmp0 = si_shufb(under0, over0, s_FBDbdf00);
                yyyyAsSigned = si_cflts(yyyyScaled, 3);
                    frustumTest2 = si_gbh(frustumTest2Tmp0);
            signW = si_and(resW, signMask);					
                    frustumTest3 = si_gbh(frustumTest3Tmp0);
            resY = si_fma(zzzz, localToProj12, mMultYy);
        xxxx = si_shufb(xxyy01, yyxx23, (qword)s_ABcd);
            resX = si_fma(zzzz, localToProj02, mMultYx);
                    frustumTest1Tmp0 = si_shufb(under1, over1, s_FBDbdf00);
            recipW = si_fma(tmp1, recipEstW, recipEstW);
                    xyz2Out = si_shufb(xyz2Trans, frustumTest2, s_ABCDEFGHd000MNOP);
            resZ = si_fma(zzzz, localToProj22, mMultYz);
                    frustumTest0 = si_gbh(frustumTest0Tmp0);
        mMultXw = si_fma(xxxx, localToProj30, localToProj33);		
                    xyz3Out = si_shufb(xyz3Trans, frustumTest3, s_ABCDEFGHd000MNOP);
                xxxxQuantized = si_and(xxxxAsSigned, sampleMask);			
                    frustumTest1 = si_gbh(frustumTest1Tmp0);
        mMultXz = si_fma(xxxx, localToProj20, localToProj23);
                    si_stqx(xyz2Out, pTransVerts2, outputOffset);
        mMultXx = si_fma(xxxx, localToProj00, localToProj03);
                    xyz0Out = si_shufb(xyz0Trans, frustumTest0, s_ABCDEFGHd000MNOP);
            xxxxUnit = si_fm(resX, recipW);
                    si_stqx(xyz3Out, pTransVerts3, outputOffset);
            yyyyUnit = si_fm(resY, recipW);
                    xyz1Out = si_shufb(xyz1Trans, frustumTest1, s_ABCDEFGHd000MNOP);
            zzzzUnit = si_fm(resZ, recipW);
        yyyy = si_shufb(yyxx23, xxyy01, (qword)s_cdAB);
        mMultXy = si_fma(xxxx, localToProj10, localToProj13);
                    si_stqx(xyz0Out, pTransVerts0, outputOffset);
                yyyyQuantized = si_and(yyyyAsSigned, sampleMask);
                xxxxShifted = si_shlqbyi(xxxxQuantized, 2);
			
                    si_stqx(xyz1Out, pTransVerts1, outputOffset);
        mMultYw = si_fma(yyyy, localToProj31, mMultXw);			

        outputOffset = si_a(outputOffset, outputOffsetInc);		
        zzzz01 = si_shufb(xyz0, xyz1, (qword)s_CcCc);

                xyxyxyxy = si_shufb(xxxxShifted, yyyyQuantized, s_AbCdEfGh);
            xxxxUnitSigned = si_xor(xxxxUnit, signW);				

            yyyyUnitSigned = si_xor(yyyyUnit, signW);				
        zzzz = si_shufb(zzzz01, zzzz23, (qword)s_ABcd);
        
                x0y0x2y2 = si_shufb(xxxxScaled, yyyyScaled, (qword)s_AaCc);
                resWcopy = si_selb(xyxyxyxy, wSafe, signMask);
        outputOffsetInc = si_shufb(outputOffsetInc, outputOffsetInc, (qword)s_BCDD);
            wSafe = si_fcgt(resW, floatZero);					
	} while (si_to_int(inputOffset) != 0);
}

static qword cullMaskTable00[16] __attribute__((__aligned__(16))) =
{
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},   // 0000
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},   // 0001
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x12, 0x13, 0x14, 0x15},   // 0010
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x80, 0x80, 0x80, 0x80},   // 0011
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15},   // 0100
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x80, 0x80, 0x80, 0x80},   // 0101
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x80, 0x80, 0x80, 0x80},   // 0110
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 0111
    {0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15},   // 1000
    {0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x80, 0x80, 0x80, 0x80},   // 1001
    {0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x80, 0x80, 0x80, 0x80},   // 1010
    {0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 1011
    {0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x80, 0x80, 0x80, 0x80},   // 1100
    {0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 1101
    {0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 1110
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80}    // 1111
};

static qword cullMaskTable01[16] __attribute__((__aligned__(16))) =
{
    {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 0000
    {0x10, 0x11, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 0001
    {0x16, 0x17, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 0010
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 0011
    {0x16, 0x17, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 0100
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 0101
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 0110
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 0111
    {0x16, 0x17, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 1000
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 1001
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 1010
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 1011
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 1100
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 1101
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // 1110
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80}    // 1111
};

static qword cullPreviousTable[16] __attribute__((__aligned__(16))) =
{
    {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f},//0
    {0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e},//1
    {0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d},//2
    {0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c},//3
    {0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b},//4
    {0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a},//5
    {0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19},//6
    {0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18},//7
    {0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17},//8
    {0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16},//9
    {0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15},//10
    {0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14},//11
    {0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13},//12
    {0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12},//13
    {0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11},//14
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10}//15
};

const qword s_Aa00 = {0x00, 0x01, 0x02, 0x03, 0x10, 0x11, 0x12, 0x13, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
const qword s_Cc00 = {0x08, 0x09, 0x0a, 0x0b, 0x18, 0x19, 0x1a, 0x1b, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
const qword s_ABab = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
const qword s_ACac = {0x00, 0x01, 0x02, 0x03, 0x08, 0x09, 0x0a, 0x0b, 0x10, 0x11, 0x12, 0x13, 0x18, 0x19, 0x1a, 0x1b};
const qword s_BAba = {0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x14, 0x15, 0x16, 0x17, 0x10, 0x11, 0x12, 0x13};
const qword s_Dd00 = {0x0c, 0x0d, 0x0e, 0x0f, 0x1c, 0x1d, 0x1e, 0x1f, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};

/*
 * 	Culls triangles.  Performs offscreen, front/back-face(optional), degenerate,
 * 	and no-pixel (separate for 4xRGMS) triangle tests.  Does not cull triangles which
 *  are on screen and cross near plane.  
 * 
 *  Operates on 4 triangles/12 indices at a time.
 * 
 *  Offscreen test uses bitwise-and of flags preprocessed in transform loop, 
 * 	stored in Z component of each vertex (see above).  Near plane test also uses
 *  preprocess test results.
 * 
 *  In-a-line degenerate test culls triangle if all 3 vertices have same quantized X coordinate
 *  or same quantized Y coordinate.
 * 
 *  Cross product is used for back/front-face tests (to get direction of normal) and zero-area tests.
 *  Backface test is not used if w = 0.
 * 
 *  4XRGMS pixel center test (optional) computes tight triangle bounding box, then quantizes min/max corners 
 *  and computes quantized edge lengths.  Min X % 4 and Min Y % 4 are combined into 4-bit index into lookup
 *  table of max-edge length to hit pixel-center.  Values in table are compared against quantized BB
 *  edge lengths to determine if a pixel is hit.  This table will not cull 100% of no-pixel triangles,
 *  but gets very many and is fast.
 * 
 *  Test results are combined to determine which of the 4 triangles will be culled.  The resulting bit pattern
 *  is used as an index into tables of shuffle masks used to construct new index list.
 * 
 */
void CullIndexTable(Display::Spu::C_IndexTable * pIndexTable, const Display::Spu::C_PositionTable * pPositionTable, Display::Spu::E_CullMode cullMode, Display::Spu::C_View * pView)
{
	const qword s_00Aa = si_rotqbyi(s_Aa00, 8);
	const qword s_00Cc = si_rotqbyi(s_Cc00, 8);

	//const qword s_DDDDDDDDDDDDDDDD = (qword)spu_splats(0x03030303);
	const qword s_00Dd = si_rotqbyi(s_Dd00, 8);

	const qword negativeOne = (qword)spu_splats(0xffffffff);
	const qword fZero = (qword)spu_splats(0.0f);

	const uint8_t vpSampleFlavor = pView->m_MultisampleType;
	const bool useRgms = vpSampleFlavor == CELL_GCM_SURFACE_SQUARE_ROTATED_4;

	const qword nearMask = (qword)spu_splats(0x80000000);

    qword flipBit = (qword)spu_splats(0x80000000);
    qword useFace = (qword)spu_splats(0);
    
	const qword pVertexes = si_from_ptr(pPositionTable->m_pBuffer);

	const unsigned int numTriangles = pIndexTable->m_Count / 3;
	unsigned int iterCount = ((numTriangles + 3) >> 2);
    qword iterCountBC = si_shufb(si_from_uint(iterCount), si_from_uint(iterCount), (qword)s_AAAA);

    const int modTriangles = numTriangles & 0x3;

    const qword inLoopMaskBase = si_from_uint(0x0);
    const qword lastLoopMask0Base = si_from_int((modTriangles == 0) ? 0 : (0xf >> modTriangles));

    const qword pIndexStream0 = si_from_uint((U32) pIndexTable->m_pBuffer);
    const qword pIndexStream1 = si_ai(pIndexStream0, 0x10);
   
    qword pOut = si_shufb(pIndexStream0, pIndexStream0, (qword)s_AAAA);

    qword pIndexStreamOffset = (qword)spu_splats(0);
	qword pIndexStreamOffsetInc = si_from_int(0x18);

	qword localNumVisibleIndexes = (qword)spu_splats(0x0);

	const qword pMasks00 = si_from_ptr(cullMaskTable00);
    const qword pMasks01 = si_from_ptr(cullMaskTable01);

    const qword pPrevMask = si_from_ptr(cullPreviousTable);
    qword prevIndex = (qword)spu_splats(0);
    qword prevRotAmount = (qword)spu_splats(0);

    qword numBytes = (qword)spu_splats(0);
    qword prevIndexTmp = (qword)spu_splats(0);

	qword dummyBuffer[3];
	const qword pDummy = si_shufb(si_from_ptr(dummyBuffer), (qword)s_AAAA, (qword)s_AAAA);
	const qword endOfBuffer = si_a(pOut, si_shufb(si_from_int( (numTriangles*6 + 0xF) & ~0xF ), (qword)s_AAAA, (qword)s_AAAA));
	const qword endOfBufferOffset = (qword)(vec_int4){-17, -33, 0, 0};
	const qword endOfBuffer0 = si_a(endOfBuffer, endOfBufferOffset);
	const qword endOfBuffer1 = si_shlqbyi(endOfBuffer0, 4);

    switch (cullMode) 
    {
	case Display::Spu::E_CullMode_Ccw:
        flipBit = si_xor(flipBit,(qword)spu_splats(0x80000000));
		// fall through
	case Display::Spu::E_CullMode_Cw:
        useFace = (qword)spu_splats(-1);
        // fall through
	case Display::Spu::E_CullMode_None:
		if (!useRgms)
        {
            /*  Load triangle indexes */
            qword inputQuad0 = si_lqx(pIndexStream0, pIndexStreamOffset);
            qword inputQuad1 = si_lqx(pIndexStream1, pIndexStreamOffset);

            qword indexQuad0 = (qword)spu_splats(0);
            qword indexQuad1 = (qword)spu_splats(0);

            qword s_input = (qword)s_ABCD;
            const qword s_inputMask = {     0x08, 0x08, 0x08, 0x08,
                                            0x08, 0x08, 0x08, 0x08,
                                            0x18, 0x18, 0x18, 0x18,
                                            0x18, 0x18, 0x18, 0x18};
            qword firstLoopMask =   {   0x0, 0x0, 0x0, 0xf,
                                        0x0, 0x0, 0x0, 0xf,
                                        0x0, 0x0, 0x0, 0x0,
                                        0x0, 0x0, 0x0, 0x0};

            iterCountBC = si_ai(iterCountBC, 2);

            qword cullBits0Tmp1 = (qword)spu_splats(0xf);
            qword e01A = (qword)spu_splats(0);
            qword e01B = (qword)spu_splats(0);
            qword e23A = (qword)spu_splats(0);
            qword e23B = (qword)spu_splats(0);
            qword e00 = (qword)spu_splats(0);
            qword e10 = (qword)spu_splats(0);
            qword e01 = (qword)spu_splats(0);
            qword e11 = (qword)spu_splats(0);
            qword e20 = (qword)spu_splats(0);
            qword e30 = (qword)spu_splats(0);
            qword e21 = (qword)spu_splats(0);
            qword e31 = (qword)spu_splats(0);
            qword doesNotIntersectNear0 = (qword)spu_splats(0);
            qword wwww2 = (qword)spu_splats(0);
            qword wwww1 = (qword)spu_splats(0);
            qword wwww0 = (qword)spu_splats(0);
            qword cullFrustum0 = (qword)spu_splats(0);
            qword inLine0 = (qword)spu_splats(0);

            qword indexQuad0Copy1 = (qword)spu_splats(0);
            qword indexQuad1Copy1 = (qword)spu_splats(0);
            qword indexQuad0Copy0 = (qword)spu_splats(0);
            qword indexQuad1Copy0 = (qword)spu_splats(0);

            qword cullFrustum0Tmp1 = (qword)spu_splats(0);
            qword crossesNear0 = (qword)spu_splats(0);
            qword inLine0tmp0 = (qword)spu_splats(0);

            qword cullBits0Tmp0 = (qword)spu_splats(0);
            qword inLine0cmp0 = (qword)spu_splats(0);
            qword inLine0cmp1 = (qword)spu_splats(0);

            do 
            {
                pIndexStreamOffset = si_a(pIndexStreamOffset, pIndexStreamOffsetInc);

                indexQuad0Copy1 = indexQuad0Copy0;
                indexQuad1Copy1 = indexQuad1Copy0;

                indexQuad0Copy0 = indexQuad0;
                indexQuad1Copy0 = indexQuad1;

                indexQuad0 = si_shufb(inputQuad0, inputQuad1, s_input);
                indexQuad1 = si_shufb(inputQuad1, inputQuad1, s_input);

                        firstLoopMask = si_shlqbyi(firstLoopMask, 4);

                        cullBits0Tmp1 = si_or(firstLoopMask, cullBits0Tmp0);
        
                        qword cullBits0 = si_shli(cullBits0Tmp1,4);
        
                        qword s_mask00 = si_lqx(pMasks00, cullBits0);
                        qword s_mask01 = si_lqx(pMasks01, cullBits0);
        
                        qword keptIndexes0 = si_shufb(indexQuad0Copy1, indexQuad1Copy1, s_mask00);
                        qword keptIndexes1 = si_shufb(indexQuad1Copy1, indexQuad1Copy1, s_mask01);
        
                        qword bitCount0 = si_shufb(si_cntb(cullBits0), (qword)s_AAAA, (qword)s_AAAA);
        
                        /*  now combine w/ previous */
                        qword s_prevBase = si_lqx(pPrevMask, prevIndex);
                        qword sel_prev = si_rotqmby(negativeOne, prevRotAmount);
        
                        qword s_prev = si_selb((qword)s_ABCD, s_prevBase, sel_prev);
        
                        pOut = si_a(pOut, numBytes);
        
                        qword prevOut = si_lqd(pOut, 0x00);
        
                        qword out0 = si_shufb(prevOut, keptIndexes0, s_prev);
                        qword out1 = si_shufb(keptIndexes0, keptIndexes1, s_prevBase);
                        qword out2 = si_shufb(keptIndexes1, keptIndexes1, s_prevBase);
        
                        /*  Load triangle indexes */
                        inputQuad0 = si_lqx(pIndexStream0, pIndexStreamOffset);
                        inputQuad1 = si_lqx(pIndexStream1, pIndexStreamOffset);
        
						qword pSafeOut2 = si_selb(pOut, pDummy, si_cgt(pOut, endOfBuffer0));
						qword pSafeOut3 = si_rotqbyi(pSafeOut2, 4);

                        si_stqd(out0, pOut, 0x00);
                        si_stqd(out1, pSafeOut2, 0x10);
                        si_stqd(out2, pSafeOut3, 0x20);
        
                        qword numBytesTmp = si_sfi(bitCount0, 4);
                        numBytes = si_mpyui(numBytesTmp, 6);
        
                        localNumVisibleIndexes = si_a(localNumVisibleIndexes, numBytesTmp);
        
                        prevIndexTmp = si_andi(si_a(prevIndexTmp, numBytes), 0xf);
                        prevRotAmount = si_sfi(prevIndexTmp, 0);
                        prevIndex = si_shli(prevIndexTmp,4);

                    inLine0tmp0 = si_and(inLine0cmp0, inLine0cmp1);

                    cullFrustum0 = si_clgti(cullFrustum0Tmp1, 0);
                    doesNotIntersectNear0 = si_clgt(nearMask, crossesNear0);
                    inLine0 = si_clgti(inLine0tmp0, 0);

                    e01A = si_shufb(e00, e10, (qword)s_ABab);
                    e01B = si_shufb(e01, e11, s_BAba);

                    e23A = si_shufb(e20, e30, (qword)s_ABab);
                    e23B = si_shufb(e21, e31, s_BAba);

                    qword mul01 = si_fm(e01A, e01B);
                    qword mul23 = si_fm(e23A, e23B);

                    qword mul01shift = si_shlqbyi(mul01, 4);
                    qword mul23shift = si_shlqbyi(mul23, 4);

                    qword cross01 = si_fs(mul01, mul01shift);
                    qword cross23 = si_fs(mul23, mul23shift);

                    qword cross0 = si_shufb(cross01, cross23, s_ACac);

                    qword zeroArea0 = si_fcmeq(cross0, fZero);

                    qword cross0flipped = si_xor(cross0, flipBit);

                    qword isBackFacing0 = si_fcgt(cross0flipped, fZero);

                    qword cullBackFace0Tmp0 = si_and(isBackFacing0, doesNotIntersectNear0);


                    qword wSafe0Bit = si_and(wwww2, si_and(wwww1, wwww0));
                    qword wSafe0Word = si_rotmai(wSafe0Bit, -31);

                    qword cullBackFace0 = si_and(cullBackFace0Tmp0, wSafe0Word);

                    qword cull0Tmp0 = si_fnms(cullBackFace0, useFace, cullFrustum0);
                    qword cull0Tmp1 = si_or(zeroArea0, inLine0);
                    qword cull0Tmp2 = si_or(cull0Tmp0, cull0Tmp1);
                    qword cull0 = si_gb(cull0Tmp2);

                    iterCountBC = si_ai(iterCountBC, -1);

                    qword isLastLoop = si_ceqi(iterCountBC, 1);

                    qword lastLoopMask0 = si_selb(inLoopMaskBase, lastLoopMask0Base, isLastLoop);

                    cullBits0Tmp0 = si_or(lastLoopMask0, cull0);
                    

                s_input = si_xor(s_input, s_inputMask);
                
                /*  Rotate into pointer offsets */
                qword i1 = si_shlqbii(indexQuad0, 4);
                qword i3 = si_shlqbyi(i1, 4);
                qword i5 = si_shlqbyi(i1, 8);
                qword i7 = si_shlqbyi(i1, 12);
        
                qword i9 = si_shlqbii(indexQuad1, 4);
                qword i11 = si_shlqbyi(i9, 4);
        
                qword i0 = si_rotmi(i1, -16);
                qword i2 = si_rotmi(i3, -16);
                qword i4 = si_shlqbyi(i1, 6);
                qword i6 = si_shlqbyi(i1, 10);
        
                qword i8 = si_rotqmbyi(i9, -2);
                qword i10 = si_shlqbyi(i9, 2);

                /*  Load Vertexes */
                qword v0 = si_lqx(pVertexes, i0);
                qword v1 = si_lqx(pVertexes, i1);
                qword v2 = si_lqx(pVertexes, i2);
                qword v3 = si_lqx(pVertexes, i3);
                qword v4 = si_lqx(pVertexes, i4);
                qword v5 = si_lqx(pVertexes, i5);
                qword v6 = si_lqx(pVertexes, i6);
                qword v7 = si_lqx(pVertexes, i7);
                qword v8 = si_lqx(pVertexes, i8);
                qword v9 = si_lqx(pVertexes, i9);
                qword v10 = si_lqx(pVertexes, i10);
                qword v11 = si_lqx(pVertexes, i11);

                /*  Frustum Test */
                qword zzzz0Tmp0 = si_shufb(v0, v3, s_Cc00);
                qword zzzz0Tmp1 = si_shufb(v6, v9, s_00Cc);
                qword zzzz0 = si_or(zzzz0Tmp0, zzzz0Tmp1);

                qword zzzz1Tmp0 = si_shufb(v1, v4, s_Cc00);
                qword zzzz1Tmp1 = si_shufb(v7, v10, s_00Cc);
                qword zzzz1 = si_or(zzzz1Tmp0, zzzz1Tmp1);

                qword zzzz2Tmp0 = si_shufb(v2, v5, s_Cc00);
                qword zzzz2Tmp1 = si_shufb(v8, v11, s_00Cc);
                qword zzzz2 = si_or(zzzz2Tmp0, zzzz2Tmp1);

                qword cullFrustum0Tmp0 = si_and(zzzz0, zzzz1);
                cullFrustum0Tmp1 = si_and(cullFrustum0Tmp0, zzzz2);
        
                qword crossesNear0Tmp0 = si_or(zzzz0, zzzz1);
                crossesNear0 = si_or(crossesNear0Tmp0, zzzz2);

                e00 = si_fs(v1, v0);
                e01 = si_fs(v2, v0);
        
                e10 = si_fs(v4, v3);
                e11 = si_fs(v5, v3);
        
                e20 = si_fs(v7, v6);
                e21 = si_fs(v8, v6);
        
                e30 = si_fs(v10, v9);
                e31 = si_fs(v11, v9);
       
                qword wwww0tmp0 = si_shufb(v0, v3, s_Dd00);
                qword wwww0tmp1 = si_shufb(v6, v9, s_00Dd);
                wwww0 = si_or(wwww0tmp0, wwww0tmp1);

                qword wwww1tmp0 = si_shufb(v1, v4, s_Dd00);
                qword wwww1tmp1 = si_shufb(v7, v10, s_00Dd);
                wwww1 = si_or(wwww1tmp0, wwww1tmp1);

                qword wwww2tmp0 = si_shufb(v2, v5, s_Dd00);
                qword wwww2tmp1 = si_shufb(v8, v11, s_00Dd);
                wwww2 = si_or(wwww2tmp0, wwww2tmp1);

                inLine0cmp0 = si_ceqh(wwww0, wwww1);
                inLine0cmp1 = si_ceqh(wwww0, wwww2);

            } while(si_to_uint(iterCountBC) != 0);
        }
		else
		{

            const qword rgmsX = {   1, 0, 3, 2,
                                    3, 1, 1, 0,
                                    0, 3, 2, 1,
                                    1, 1, 0, 2};

            const qword rgmsY = {   2, 0, 1, 1,
                                    1, 3, 2, 0,
                                    0, 1, 1, 3,
                                    3, 1, 0, 2};

            const qword sel_Rgms = (qword)spu_splats(0xC);
            const qword sel_RgmsShuf = si_fsmbi(0x1111);
            const qword s_0000 = (qword)spu_splats(0x80808080);

            /*  Load triangle indexes */
            qword inputQuad0 = si_lqx(pIndexStream0, pIndexStreamOffset);
            qword inputQuad1 = si_lqx(pIndexStream1, pIndexStreamOffset);
    
            qword indexQuad0 = (qword)spu_splats(0);
            qword indexQuad1 = (qword)spu_splats(0);
    
            qword s_input = (qword)s_ABCD;
            const qword s_inputMask = {     0x08, 0x08, 0x08, 0x08,
                                            0x08, 0x08, 0x08, 0x08,
                                            0x18, 0x18, 0x18, 0x18,
                                            0x18, 0x18, 0x18, 0x18};
            qword firstLoopMask = { 0x0, 0x0, 0x0, 0xf,
                                    0x0, 0x0, 0x0, 0xf,
                                    0x0, 0x0, 0x0, 0x0,
                                    0x0, 0x0, 0x0, 0x0};

            iterCountBC = si_ai(iterCountBC, 2);

            qword indexQuad0Copy0 = (qword)spu_splats(0);
            qword indexQuad1Copy0 = (qword)spu_splats(0);

            qword e00 = (qword)spu_splats(0);
            qword e10 = (qword)spu_splats(0);
            qword e01 = (qword)spu_splats(0);
            qword e11 = (qword)spu_splats(0);
            qword e20 = (qword)spu_splats(0);
            qword e30 = (qword)spu_splats(0);
            qword e21 = (qword)spu_splats(0);
            qword e31 = (qword)spu_splats(0);
            qword doesNotIntersectNear0 = (qword)spu_splats(0);
            qword wwww0 = (qword)spu_splats(0);
            qword wwww1 = (qword)spu_splats(0);
            qword wwww2 = (qword)spu_splats(0);
            qword cullFrustum0 = (qword)spu_splats(0);

            qword v0 = (qword)spu_splats(0);
            qword v1 = (qword)spu_splats(0);
            qword v2 = (qword)spu_splats(0);
            qword v3 = (qword)spu_splats(0);
            qword v4 = (qword)spu_splats(0);
            qword v5 = (qword)spu_splats(0);
            qword v6 = (qword)spu_splats(0);
            qword v7 = (qword)spu_splats(0);
            qword v8 = (qword)spu_splats(0);
            qword v9 = (qword)spu_splats(0);
            qword v10 = (qword)spu_splats(0);
            qword v11 = (qword)spu_splats(0);

            qword cullFace0 = (qword)spu_splats(0);
            qword zeroAreaNoPixel0 = (qword)spu_splats(0);
            qword inLine0 = (qword)spu_splats(0);
            qword cullFrustum0Tmp1 = (qword)spu_splats(0);

            qword wwww0tmp0 = (qword)spu_splats(0);
            qword wwww0tmp1 = (qword)spu_splats(0);
            qword wwww1tmp0 = (qword)spu_splats(0);
            qword wwww1tmp1 = (qword)spu_splats(0);
            qword wwww2tmp0 = (qword)spu_splats(0);
            qword wwww2tmp1 = (qword)spu_splats(0);

            qword minY0 = (qword)spu_splats(0);
            qword maxY0 = (qword)spu_splats(0);
            qword minX0 = (qword)spu_splats(0);
            qword maxX0 = (qword)spu_splats(0);

            qword cullFace0Gathered = (qword)spu_splats(0);
            qword cullFrustum0Gathered = (qword)spu_splats(0);
            qword zeroAreaNoPixel0Gathered = (qword)spu_splats(0);
            qword inLine0Gathered = (qword)spu_splats(0);

            qword tmp41 = (qword)spu_splats(0);
            qword yyyy2 = (qword)spu_splats(0);
            qword yyyy0 = (qword)spu_splats(0);
            qword yyyy1 = (qword)spu_splats(0);
            qword cmp12 = (qword)spu_splats(0);
            qword tmp40 = (qword)spu_splats(0);
            qword cmp11 = (qword)spu_splats(0);
            qword tmp31 = (qword)spu_splats(0);
            qword xxxx2 = (qword)spu_splats(0);
            qword xxxx0 = (qword)spu_splats(0);
            qword xxxx1 = (qword)spu_splats(0);
            qword cmp02 = (qword)spu_splats(0);
            qword tmp30 = (qword)spu_splats(0);
            qword cmp01 = (qword)spu_splats(0);
            qword cmp10 = (qword)spu_splats(0);
            qword cmp00 = (qword)spu_splats(0);
            qword inLine0tmp0 = (qword)spu_splats(0);
            qword cullFrustum0Tmp0 = (qword)spu_splats(0);
            qword zzzz2 = (qword)spu_splats(0);

            const qword sel_00FF = si_fsm(si_from_uint(0x3));

            do 
            {
                pIndexStreamOffset = si_a(pIndexStreamOffset, pIndexStreamOffsetInc);

                        qword inLine0cmp0 = si_ceqh(wwww0, wwww1);
                        qword inLine0cmp1 = si_ceqh(wwww0, wwww2);
                        inLine0tmp0 = si_and(inLine0cmp0, inLine0cmp1);

                        inLine0 = si_clgti(inLine0tmp0, 0);
                        cullFrustum0 = si_clgti(cullFrustum0Tmp1, 0);

                        cullFrustum0Gathered = si_gb(cullFrustum0);
                        inLine0Gathered = si_gb(inLine0);


                        qword cull0GatheredTmp0 = si_shufb(cullFace0Gathered, cullFrustum0Gathered, s_Aa00);
                        qword cull0GatheredTmp1 = si_shufb(zeroAreaNoPixel0Gathered, inLine0Gathered, s_00Aa);
                        qword cull0Gathered = si_shufb(cull0GatheredTmp0, cull0GatheredTmp1, (qword)s_ABcd);
                        qword cull0 = si_orx(cull0Gathered);
                        
                        
                        iterCountBC = si_ai(iterCountBC, -1);
                        
                        qword isLastLoop = si_ceqi(iterCountBC, 0);
                        
                        qword lastLoopMask0 = si_selb(inLoopMaskBase, lastLoopMask0Base, isLastLoop);
                        
                        qword cullBits0Tmp0 = si_or(lastLoopMask0, cull0);
                        qword cullBits0Tmp1 = si_or(firstLoopMask, cullBits0Tmp0);
                        qword cullBits0 = si_shli(cullBits0Tmp1,4);
                        firstLoopMask = si_shlqbyi(firstLoopMask, 4);
                        
                        qword s_mask00 = si_lqx(pMasks00, cullBits0);
                        qword s_mask01 = si_lqx(pMasks01, cullBits0);
                        
                        qword keptIndexes0 = si_shufb(indexQuad0Copy0, indexQuad1Copy0, s_mask00);
                        qword keptIndexes1 = si_shufb(indexQuad1Copy0, indexQuad1Copy0, s_mask01);

                indexQuad0Copy0 = indexQuad0;
                indexQuad1Copy0 = indexQuad1;

                indexQuad0 = si_shufb(inputQuad0, inputQuad1, s_input);
                indexQuad1 = si_shufb(inputQuad1, inputQuad1, s_input);

                        //qword bitCount0 = si_shufb(si_cntb(cullBits0), (qword)s_AAAA, (qword)s_AAAA);
						qword bitCount0 = si_cntb(cullBits0);

                        /*  now combine w/ previous */
                        qword s_prevBase = si_lqx(pPrevMask, prevIndex);
                        qword sel_prev = si_rotqmby(negativeOne, prevRotAmount);
                        
                        qword s_prev = si_selb((qword)s_ABCD, s_prevBase, sel_prev);
                        
                        pOut = si_a(pOut, numBytes);
                        
                        qword prevOut = si_lqd(pOut, 0x00);
                        
                        qword out0 = si_shufb(prevOut, keptIndexes0, s_prev);
                        qword out1 = si_shufb(keptIndexes0, keptIndexes1, s_prevBase);
                        qword out2 = si_shufb(keptIndexes1, keptIndexes1, s_prevBase);
                        
                        /*  Load triangle indexes */
                        inputQuad0 = si_lqx(pIndexStream0, pIndexStreamOffset);
                        inputQuad1 = si_lqx(pIndexStream1, pIndexStreamOffset);

						qword pSafeOut2 = si_selb(pOut, pDummy, si_cgt(pOut, endOfBuffer0));
						qword pSafeOut3 = si_selb(pOut, pDummy, si_cgt(pOut, endOfBuffer1));

                        si_stqd(out0, pOut, 0x00);
                        si_stqd(out1, pSafeOut2, 0x10);
                        si_stqd(out2, pSafeOut3, 0x20);

                        qword numBytesTmp = si_sfi(bitCount0, 4);
                        numBytes = si_mpyui(numBytesTmp, 6);
                        
                        localNumVisibleIndexes = si_a(localNumVisibleIndexes, numBytesTmp);
                        
                        prevIndexTmp = si_andi(si_a(prevIndexTmp, numBytes), 0xf);
                        prevRotAmount = si_sfi(prevIndexTmp, 0);
                        prevIndex = si_shli(prevIndexTmp,4);

                    tmp30 = si_selb(xxxx1, xxxx0, cmp00);
                    tmp31 = si_selb(xxxx0, xxxx1, cmp00);

                    tmp40 = si_selb(yyyy1, yyyy0, cmp10);
                    tmp41 = si_selb(yyyy0, yyyy1, cmp10);

                    cmp01 = si_fcgt(tmp30, xxxx2);
                    cmp02 = si_fcgt(tmp31, xxxx2);
                    cmp11 = si_fcgt(tmp40, yyyy2);
                    cmp12 = si_fcgt(tmp41, yyyy2);

                    minY0 = si_selb(tmp41, yyyy2, cmp12);
                    maxY0 = si_selb(yyyy2, tmp40, cmp11);
                    minX0 = si_selb(tmp31, xxxx2, cmp02);
                    maxX0 = si_selb(xxxx2, tmp30, cmp01);

                    wwww0 = si_shufb(wwww0tmp0, wwww0tmp1, (qword)s_ABcd);
                    
                    wwww1 = si_shufb(wwww1tmp0, wwww1tmp1, (qword)s_ABcd);
                    
                    wwww2 = si_shufb(wwww2tmp0, wwww2tmp1, (qword)s_ABcd);
                    
                    
                    qword minX0Quant = si_cflts(minX0, 2);
                    qword minY0Quant = si_cflts(minY0, 2);
                    
                    qword maxX0Quant = si_cflts(maxX0, 2);
                    qword maxY0Quant = si_cflts(maxY0, 2);
                    
                    qword dxQuant = si_sf(minX0Quant, maxX0Quant);
                    qword dyQuant = si_sf(minY0Quant, maxY0Quant);
    
                    qword yCoord = si_shlqbyi(minY0Quant, 2);
                    
                    qword s_Rgms = si_selb(s_0000, si_andi(si_selb(minX0Quant, yCoord, sel_Rgms), 0xf), sel_RgmsShuf);
                    
                    qword xValues = si_shufb(rgmsX, rgmsX, s_Rgms);
                    qword yValues = si_shufb(rgmsY, rgmsY, s_Rgms);
                    
                    qword xMiss = si_cgt(xValues, dxQuant);
                    qword yMiss = si_cgt(yValues, dyQuant);
                    qword noPixel0 = si_and(xMiss, yMiss);
                    
                    qword e01A = si_shufb(e00, e10, (qword)s_ABab);
                    qword e01B = si_shufb(e01, e11, s_BAba);
                    
                    qword e23A = si_shufb(e20, e30, (qword)s_ABab);
                    qword e23B = si_shufb(e21, e31, s_BAba);
                    
                    qword mul01 = si_fm(e01A, e01B);
                    qword mul23 = si_fm(e23A, e23B);
                    
                    qword mul01shift = si_shlqbyi(mul01, 4);
                    qword mul23shift = si_shlqbyi(mul23, 4);
                    
                    qword cross01 = si_fs(mul01, mul01shift);
                    qword cross23 = si_fs(mul23, mul23shift);
                    
                    qword cross0 = si_shufb(cross01, cross23, s_ACac);
                    
                    qword zeroArea0 = si_fcmeq(cross0, fZero);
                    
                    zeroAreaNoPixel0 = si_or(zeroArea0, noPixel0);
                    
                    qword cross0flipped = si_xor(cross0, flipBit);
                    
                    qword isBackFacing0 = si_fcgt(cross0flipped, fZero);
                    
                    qword cullBackFace0Tmp0 = si_and(isBackFacing0, doesNotIntersectNear0);
                    
                    
                    qword wSafe0Bit = si_and(wwww2, si_and(wwww1, wwww0));
                    qword wSafe0Word = si_rotmai(wSafe0Bit, -31);
                    
                    qword cullBackFace0 = si_and(cullBackFace0Tmp0, wSafe0Word);
                    
                    cullFace0 = si_and(cullBackFace0, useFace);

                    cullFrustum0Tmp1 = si_and(cullFrustum0Tmp0, zzzz2);
                    
                    zeroAreaNoPixel0Gathered = si_gb(zeroAreaNoPixel0);
                    cullFace0Gathered = si_gb(cullFace0);

                s_input = si_xor(s_input, s_inputMask);
                
                qword i1 = si_shlqbii(indexQuad0, 4);
                qword i3 = si_shlqbyi(i1, 4);
                qword i5 = si_shlqbyi(i1, 8);
                qword i7 = si_shlqbyi(i1, 12);
                
                qword i9 = si_shlqbii(indexQuad1, 4);
                qword i11 = si_shlqbyi(i9, 4);
                
                qword i0 = si_rotmi(i1, -16);
                qword i2 = si_shlqbyi(i1, 2);
                qword i4 = si_shlqbyi(i1, 6);
                qword i6 = si_shlqbyi(i1, 10);
                
                qword i8 = si_rotqmbyi(i9, -2);
                qword i10 = si_shlqbyi(i9, 2);
                
                /*  Load Vertexes */
                v0 = si_lqx(pVertexes, i0);
                v1 = si_lqx(pVertexes, i1);
                v2 = si_lqx(pVertexes, i2);
                v3 = si_lqx(pVertexes, i3);
                v4 = si_lqx(pVertexes, i4);
                v5 = si_lqx(pVertexes, i5);
                v6 = si_lqx(pVertexes, i6);
                v7 = si_lqx(pVertexes, i7);
                v8 = si_lqx(pVertexes, i8);
                v9 = si_lqx(pVertexes, i9);
                v10 = si_lqx(pVertexes, i10);
                v11 = si_lqx(pVertexes, i11);
                
                wwww0tmp0 = si_shufb(v0, v3, s_Dd00);
                wwww0tmp1 = si_shufb(v6, v9, s_00Dd);
                wwww1tmp0 = si_shufb(v1, v4, s_Dd00);
                wwww1tmp1 = si_shufb(v7, v10, s_00Dd);
                wwww2tmp0 = si_shufb(v2, v5, s_Dd00);
                wwww2tmp1 = si_shufb(v8, v11, s_00Dd);
                
                qword zzzz0Tmp0 = si_shufb(v0, v3, s_Cc00);
                qword zzzz0Tmp1 = si_shufb(v6, v9, s_00Cc);
                qword zzzz0 = si_or(zzzz0Tmp0, zzzz0Tmp1);
                
                qword zzzz1Tmp0 = si_shufb(v1, v4, (qword)s_Cc00);
                qword zzzz1Tmp1 = si_shufb(v7, v10, s_00Cc);
                qword zzzz1 = si_or(zzzz1Tmp0, zzzz1Tmp1);
                
                qword zzzz2Tmp0 = si_shufb(v2, v5, s_Cc00);
                qword zzzz2Tmp1 = si_shufb(v8, v11, s_00Cc);
                zzzz2 = si_or(zzzz2Tmp0, zzzz2Tmp1);
                
                cullFrustum0Tmp0 = si_and(zzzz0, zzzz1);
                
                qword crossesNear0Tmp0 = si_or(zzzz0, zzzz1);
                qword crossesNear0 = si_or(crossesNear0Tmp0, zzzz2);
                
                doesNotIntersectNear0 = si_clgt(nearMask, crossesNear0);
                
                qword tmp00 = si_shufb(v0, v3, (qword)s_AaBb);
                qword tmp01 = si_shufb(v6, v9, (qword)s_BbAa);
                xxxx0 = si_selb(tmp00, tmp01, sel_00FF);
                yyyy0 = si_shufb(tmp00, tmp01, (qword)s_CDab);
                
                qword tmp10 = si_shufb(v1, v4, (qword)s_AaBb);
                qword tmp11 = si_shufb(v7, v10, (qword)s_BbAa);
                xxxx1 = si_selb(tmp10, tmp11, sel_00FF);
                yyyy1 = si_shufb(tmp10, tmp11, (qword)s_CDab);
                
                qword tmp20 = si_shufb(v2, v5, (qword)s_AaBb);
                qword tmp21 = si_shufb(v8, v11, (qword)s_BbAa);
                xxxx2 = si_selb(tmp20, tmp21, sel_00FF);
                yyyy2 = si_shufb(tmp20, tmp21, (qword)s_CDab);
                
                cmp00 = si_fcgt(xxxx0, xxxx1);
                cmp10 = si_fcgt(yyyy0, yyyy1);

                e00 = si_fs(v1, v0);
                e01 = si_fs(v2, v0);
                
                e10 = si_fs(v4, v3);
                e11 = si_fs(v5, v3);
                
                e20 = si_fs(v7, v6);
                e21 = si_fs(v8, v6);
                
                e30 = si_fs(v10, v9);
                e31 = si_fs(v11, v9);
                
            } while(si_to_uint(iterCountBC) != 0);
        }
        break;
	default:
		break;
    }

	pIndexTable->m_Count = si_to_uint(localNumVisibleIndexes) * 3;
}



void Cull(Display::Spu::C_IndexTable * pIndexTable, Display::Spu::C_PositionTable * pPositionTable, Display::Spu::E_CullMode cullMode, Display::Spu::C_View * pView, const MathsStorage::MatrixCol44 &matWorld)
{
	TransformPositionTable(matWorld, pView, pPositionTable);
	CullIndexTable(pIndexTable, pPositionTable, cullMode, pView);
}



}; // namespace Spu
}; // namespace Display
