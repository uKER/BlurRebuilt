#include "Display/Spu/Display.h"
#include "Display/Spu/Table/VertexTable.h"
#include "Display/Spu/Util/Transform.h"
#include "Core/../../Source/Maths/HVec/SIMD.cpp.inl"



namespace Display
{
namespace Spu
{



extern const vec_uchar16 s_AaBb;
extern const vec_uchar16 s_BbAa;
extern const vec_uchar16 s_CcDd;
extern const vec_uchar16 s_DdCc;
extern const vec_uchar16 s_ABcd;

void TransformVertexTable(const Maths::HMat44 &matrix, C_VertexTable * pVertexTable)
{
	const U8 * pVertices = (const U8 *) pVertexTable->m_pBuffer;
	const U32 offset0 = pVertexTable->m_Stride * 0;
	const U32 offset1 = pVertexTable->m_Stride * 1;
	const U32 offset2 = pVertexTable->m_Stride * 2;
	const U32 offset3 = pVertexTable->m_Stride * 3;
	const U32 offset4 = pVertexTable->m_Stride * 4;
	const U32 offset5 = pVertexTable->m_Stride * 5;
	const U32 offset6 = pVertexTable->m_Stride * 6;
	const U32 offset7 = pVertexTable->m_Stride * 7;

	Maths::HVec3 vAdd(0.0f, 5.0f, 0.0f);	

	for (U32 i = 0; i < pVertexTable->m_Count; i += 8)
	{
		// load positions
		Maths::HVec3 v0 = Maths::Load<Maths::HVec3>( (MathsStorage::Float3 *) (pVertices + offset0) );
		Maths::HVec3 v1 = Maths::Load<Maths::HVec3>( (MathsStorage::Float3 *) (pVertices + offset1) );
		Maths::HVec3 v2 = Maths::Load<Maths::HVec3>( (MathsStorage::Float3 *) (pVertices + offset2) );
		Maths::HVec3 v3 = Maths::Load<Maths::HVec3>( (MathsStorage::Float3 *) (pVertices + offset3) );
		Maths::HVec3 v4 = Maths::Load<Maths::HVec3>( (MathsStorage::Float3 *) (pVertices + offset4) );
		Maths::HVec3 v5 = Maths::Load<Maths::HVec3>( (MathsStorage::Float3 *) (pVertices + offset5) );
		Maths::HVec3 v6 = Maths::Load<Maths::HVec3>( (MathsStorage::Float3 *) (pVertices + offset6) );
		Maths::HVec3 v7 = Maths::Load<Maths::HVec3>( (MathsStorage::Float3 *) (pVertices + offset7) );

		// add something to them
		v0 = v0 * matrix;
		v1 = v1 * matrix;
		v2 = v2 * matrix;
		v3 = v3 * matrix;
		v4 = v4 * matrix;
		v5 = v5 * matrix;
		v6 = v6 * matrix;
		v7 = v7 * matrix;

		// store them back
		Maths::Store(v0, (MathsStorage::Float3 *) (pVertices + offset0));
		Maths::Store(v1, (MathsStorage::Float3 *) (pVertices + offset1));
		Maths::Store(v2, (MathsStorage::Float3 *) (pVertices + offset2));
		Maths::Store(v3, (MathsStorage::Float3 *) (pVertices + offset3));
		Maths::Store(v4, (MathsStorage::Float3 *) (pVertices + offset4));
		Maths::Store(v5, (MathsStorage::Float3 *) (pVertices + offset5));
		Maths::Store(v6, (MathsStorage::Float3 *) (pVertices + offset6));
		Maths::Store(v7, (MathsStorage::Float3 *) (pVertices + offset7));

		pVertices += pVertexTable->m_Stride * 8;
	}
}


/*
template <int Stride>
void TransformVertexTable(const MathsStorage::MatrixCol44 * pMatrix, C_VertexTable * pVertexTable)
{
    const qword sel_0FF0 = si_fsm(si_from_uint(0x6));
    const qword sel_00FF = si_fsm(si_from_uint(0x3));

    //Build matrix
    qword trans0 = si_lqd(si_from_ptr(pMatrix), 0x00);
    qword trans1 = si_lqd(si_from_ptr(pMatrix), 0x10);
    qword trans2 = si_lqd(si_from_ptr(pMatrix), 0x20);
    qword trans3 = si_lqd(si_from_ptr(pMatrix), 0x30);

    trans1 = si_rotqbyi(trans1, 4);
    trans2 = si_rotqbyi(trans2, 8);
    trans3 = si_rotqbyi(trans3, 12);

	qword temp1 = si_selb(trans0, trans2, sel_0FF0);		// temp1 = x1 y3 z3 w1
	qword temp2 = si_selb(trans2, trans0, sel_0FF0);		// temp2 = x3 y1 z1 w3
	qword temp3 = si_selb(trans1, trans3, sel_0FF0);		// temp3 = x2 y4 z4 w2
	qword temp4 = si_selb(trans3, trans1, sel_0FF0);		// temp4 = x4 y2 z2 w4
	qword temp5 = si_shufb(temp1, temp3, (qword)s_AaBb);	// temp5 = x1 x2 y3 y4
	qword temp6 = si_shufb(temp2, temp4, (qword)s_BbAa);	// temp6 = y1 y2 x3 x4
	qword temp7 = si_shufb(temp1, temp3, (qword)s_DdCc);	// temp7 = w1 w2 z3 z4
	qword temp8 = si_shufb(temp2, temp4, (qword)s_CcDd);	// temp8 = z1 z2 w3 w4
	qword m0 = si_shufb(temp5, temp6, (qword)s_ABcd);	    // m0  = x1 x2 x3 x4
	qword m1 = si_selb(temp6, temp5, sel_00FF);		// m1  = y1 y2 y3 y4
	qword m2 = si_selb(temp8, temp7, sel_00FF);		// m2  = z1 z2 z3 z4
	qword m3 = si_selb(temp7, temp8, sel_00FF);		// m3  = w1 w2 w3 w4

    //Loop Preamble
    uint32_t counter = ((pVertexTable->m_Count + 3) >> 2) + 1;
    qword pV = si_from_ptr(pVertexTable->m_pBuffer); // in vertices

    const qword pTv0 = si_from_ptr(pVertexTable->m_pBuffer); // out vertices
    const qword pTv1 = si_ai(si_from_ptr(pVertexTable->m_pBuffer), Stride);
    const qword pTv2 = si_ai(si_from_ptr(pVertexTable->m_pBuffer), Stride * 2);
    const qword pTv3 = si_ai(si_from_ptr(pVertexTable->m_pBuffer), Stride * 3);
    const qword pTv4 = si_ai(si_from_ptr(pVertexTable->m_pBuffer), Stride * 4);
    const qword pTv5 = si_ai(si_from_ptr(pVertexTable->m_pBuffer), Stride * 5);
    const qword pTv6 = si_ai(si_from_ptr(pVertexTable->m_pBuffer), Stride * 6);
    const qword pTv7 = si_ai(si_from_ptr(pVertexTable->m_pBuffer), Stride * 7);
    qword pTvOffset = (qword)spu_splats(0);
    qword pTvOffsetInc = (qword)spu_splats(0);
    const qword pTvOffsetIncBase = (qword)spu_splats(Stride * 8);

    qword v23 = (qword)spu_splats(0);
    qword v33 = (qword)spu_splats(0);
    qword v43 = (qword)spu_splats(0);
    qword v53 = (qword)spu_splats(0);
    qword v63 = (qword)spu_splats(0);
    qword v73 = (qword)spu_splats(0);

    qword v2tmp2 = (qword)spu_splats(0);
    qword v3tmp2 = (qword)spu_splats(0);
    qword v4tmp2 = (qword)spu_splats(0);
    qword v5tmp2 = (qword)spu_splats(0);
    qword v6tmp2 = (qword)spu_splats(0);
    qword v7tmp2 = (qword)spu_splats(0);

    qword v0out = (qword)spu_splats(0);
    qword v1out = (qword)spu_splats(0);

    do
    {
        qword v2out = si_fma(v23, m3, v2tmp2);
            qword v00 = si_lqd(pV, 0x00);
        qword v3out = si_fma(v33, m3, v3tmp2);
            qword v10 = si_lqd(pV, Stride);
        qword v4out = si_fma(v43, m3, v4tmp2);
            qword v20 = si_lqd(pV, Stride * 2);
        qword v5out = si_fma(v53, m3, v5tmp2);
            qword v30 = si_lqd(pV, Stride * 3);
        qword v6out = si_fma(v63, m3, v6tmp2);
            qword v40 = si_lqd(pV, Stride * 4);
        qword v7out = si_fma(v73, m3, v7tmp2);
            qword v50 = si_lqd(pV, Stride * 5);
        counter--;
            qword v60 = si_lqd(pV, Stride * 6);
            qword v70 = si_lqd(pV, Stride * 7);
        pV = si_ai(pV, 0x80);
            si_stqx(v0out, pTv0, pTvOffset);
            si_stqx(v1out, pTv1, pTvOffset);
            si_stqx(v2out, pTv2, pTvOffset);
            si_stqx(v3out, pTv3, pTvOffset);
            si_stqx(v4out, pTv4, pTvOffset);
            si_stqx(v5out, pTv5, pTvOffset);
            si_stqx(v6out, pTv6, pTvOffset);
            si_stqx(v7out, pTv7, pTvOffset);
        pTvOffset = si_a(pTvOffset, pTvOffsetInc);
        pTvOffsetInc = pTvOffsetIncBase;

        qword v0tmp0 = si_fm(v00, m0);
            
        qword v1tmp0 = si_fm(v10, m0);
            
        qword v2tmp0 = si_fm(v20, m0);
            qword v01 = si_rotqbyi(v00, 4);
        qword v3tmp0 = si_fm(v30, m0);
            qword v11 = si_rotqbyi(v10, 4);
        qword v4tmp0 = si_fm(v40, m0);
            qword v21 = si_rotqbyi(v20, 4);
        qword v5tmp0 = si_fm(v50, m0);
            qword v31 = si_rotqbyi(v30, 4);
        qword v6tmp0 = si_fm(v60, m0);
            qword v41 = si_rotqbyi(v40, 4);
        qword v7tmp0 = si_fm(v70, m0);
            qword v51 = si_rotqbyi(v50, 4);
        qword v0tmp1 = si_fma(v01, m1, v0tmp0);
            qword v61 = si_rotqbyi(v60, 4);
        qword v1tmp1 = si_fma(v11, m1, v1tmp0);
            qword v71 = si_rotqbyi(v70, 4);
        qword v2tmp1 = si_fma(v21, m1, v2tmp0);
            qword v02 = si_rotqbyi(v00, 8);
        qword v3tmp1 = si_fma(v31, m1, v3tmp0);
            qword v12 = si_rotqbyi(v10, 8);
        qword v4tmp1 = si_fma(v41, m1, v4tmp0);
            qword v22 = si_rotqbyi(v20, 8);
        qword v5tmp1 = si_fma(v51, m1, v5tmp0);
            qword v32 = si_rotqbyi(v30, 8);
        qword v6tmp1 = si_fma(v61, m1, v6tmp0);
            qword v42 = si_rotqbyi(v40, 8);
        qword v7tmp1 = si_fma(v71, m1, v7tmp0);
            qword v52 = si_rotqbyi(v50, 8);
        qword v0tmp2 = si_fma(v02, m2, v0tmp1);
            qword v62 = si_rotqbyi(v60, 8);
        qword v1tmp2 = si_fma(v12, m2, v1tmp1);
            qword v72 = si_rotqbyi(v70, 8);
        v2tmp2 = si_fma(v22, m2, v2tmp1);
            qword v03 = si_rotqbyi(v00, 12);
        v3tmp2 = si_fma(v32, m2, v3tmp1);
            qword v13 = si_rotqbyi(v10, 12);
        v4tmp2 = si_fma(v42, m2, v4tmp1);
            v23 = si_rotqbyi(v20, 12);    
        v5tmp2 = si_fma(v52, m2, v5tmp1);
            v33 = si_rotqbyi(v30, 12);
        v6tmp2 = si_fma(v62, m2, v6tmp1);
            v43 = si_rotqbyi(v40, 12);
        v7tmp2 = si_fma(v72, m2, v7tmp1);
            v53 = si_rotqbyi(v50, 12);
        v0out = si_fma(v03, m3, v0tmp2);
            v63 = si_rotqbyi(v60, 12);
        v1out = si_fma(v13, m3, v1tmp2);
            v73 = si_rotqbyi(v70, 12);
    } while (counter > 0);
}
*/


}; // namespace Spu
}; // namespace Display
