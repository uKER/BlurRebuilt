#ifndef DISPLAY_VIEW_VIEW_FRUSTM_INL
#define DISPLAY_VIEW_VIEW_FRUSTM_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE Maths::HVec3 C_ViewFrustum::GetPoint(const U32 i) const
{
	Maths::HVec3 point(Maths::Load<Maths::HVec3>(&m_Points[i]));
	return point;
}

INLINE Maths::C_Plane C_ViewFrustum::GetPlane(const U32 i) const
{
	Maths::C_Plane plane(Maths::Load<Maths::HVec4>(&m_ClipPlanes[i]));
	return plane;
}

INLINE U32 C_ViewFrustum::GetPlaneCount() const
{
	return m_PlaneCount;
}
	
template<int PlaneCount>
const S32 C_ViewFrustum::SphereCullT(const Maths::C_Sphere& sphere) const
{
#ifndef PLATFORM_SPU
	BIZ_STATIC_ASSERT(false);
#endif
	return E_CullResult_FullyVisible;
}

template<>
INLINE const S32 C_ViewFrustum::SphereCullT<16>(const Maths::C_Sphere& sphere) const
{
	BIZ_STATIC_ASSERT(m_MaxPlaneCount >= 16);

	Maths::HVec4 r(-sphere.Radius());
	Maths::HVec4 vCentre(sphere.Centre());
	vCentre.W(Maths::Constants<Maths::HF32>::One());
	Maths::HVec4 vPlanes0(Maths::Load<Maths::HVec4>(&m_ClipPlanes[0]));
	Maths::HVec4 vPlanes1(Maths::Load<Maths::HVec4>(&m_ClipPlanes[1]));
	Maths::HVec4 vPlanes2(Maths::Load<Maths::HVec4>(&m_ClipPlanes[2]));
	Maths::HVec4 vPlanes3(Maths::Load<Maths::HVec4>(&m_ClipPlanes[3]));
	Maths::HVec4 vPlanes4(Maths::Load<Maths::HVec4>(&m_ClipPlanes[4]));
	Maths::HVec4 vPlanes5(Maths::Load<Maths::HVec4>(&m_ClipPlanes[5]));
	Maths::HVec4 vPlanes6(Maths::Load<Maths::HVec4>(&m_ClipPlanes[6]));
	Maths::HVec4 vPlanes7(Maths::Load<Maths::HVec4>(&m_ClipPlanes[7]));
	Maths::HVec4 vPlanes8(Maths::Load<Maths::HVec4>(&m_ClipPlanes[8]));
	Maths::HVec4 vPlanes9(Maths::Load<Maths::HVec4>(&m_ClipPlanes[9]));
	Maths::HVec4 vPlanes10(Maths::Load<Maths::HVec4>(&m_ClipPlanes[10]));
	Maths::HVec4 vPlanes11(Maths::Load<Maths::HVec4>(&m_ClipPlanes[11]));
	Maths::HVec4 vPlanes12(Maths::Load<Maths::HVec4>(&m_ClipPlanes[12]));
	Maths::HVec4 vPlanes13(Maths::Load<Maths::HVec4>(&m_ClipPlanes[13]));
	Maths::HVec4 vPlanes14(Maths::Load<Maths::HVec4>(&m_ClipPlanes[14]));
	Maths::HVec4 vPlanes15(Maths::Load<Maths::HVec4>(&m_ClipPlanes[15]));

	Maths::HF32 d1_0 = vPlanes0 | vCentre;
	Maths::HF32 d1_1 = vPlanes1 | vCentre;
	Maths::HF32 d1_2 = vPlanes2 | vCentre;
	Maths::HF32 d1_3 = vPlanes3 | vCentre;
	Maths::HF32 d1_4 = vPlanes4 | vCentre;
	Maths::HF32 d1_5 = vPlanes5 | vCentre;
	Maths::HF32 d1_6 = vPlanes6 | vCentre;
	Maths::HF32 d1_7 = vPlanes7 | vCentre;
	Maths::HF32 d1_8 = vPlanes8 | vCentre;
	Maths::HF32 d1_9 = vPlanes9 | vCentre;
	Maths::HF32 d1_10 = vPlanes10 | vCentre;
	Maths::HF32 d1_11 = vPlanes11 | vCentre;
	Maths::HF32 d1_12 = vPlanes12 | vCentre;
	Maths::HF32 d1_13 = vPlanes13 | vCentre;
	Maths::HF32 d1_14 = vPlanes14 | vCentre;
	Maths::HF32 d1_15 = vPlanes15 | vCentre;

	Maths::HVec4 hd1_0 = Maths::HVec4(d1_0, d1_1, d1_2, d1_3);
	Maths::HVec4 hd1_4 = Maths::HVec4(d1_4, d1_5, d1_6, d1_7);
	Maths::HVec4 hd1_8 = Maths::HVec4(d1_8, d1_9, d1_10, d1_11);
	Maths::HVec4 hd1_12 = Maths::HVec4(d1_12, d1_13, d1_14, d1_15);

	Maths::Vector4_Bool r_0 = hd1_0 < r;
	Maths::Vector4_Bool r_1 = hd1_4 < r;
	Maths::Vector4_Bool r_2 = hd1_8 < r;
	Maths::Vector4_Bool r_3 = hd1_12 < r;

	Maths::Vector4_Bool r_4 = r_0 | r_1;
	Maths::Vector4_Bool r_5 = r_2 | r_3;

	Maths::Vector4_Bool r_6 = r_4 | r_5;

	return AllFalse(r_6);
}

template<>
INLINE const S32 C_ViewFrustum::SphereCullT<12>(const Maths::C_Sphere& sphere) const
{
	BIZ_STATIC_ASSERT(m_MaxPlaneCount >= 12);

	Maths::HVec4 r(-sphere.Radius());
	Maths::HVec4 vCentre(sphere.Centre());
	vCentre.W(Maths::Constants<Maths::HF32>::One());
	Maths::HVec4 vPlanes0(Maths::Load<Maths::HVec4>(&m_ClipPlanes[0]));
	Maths::HVec4 vPlanes1(Maths::Load<Maths::HVec4>(&m_ClipPlanes[1]));
	Maths::HVec4 vPlanes2(Maths::Load<Maths::HVec4>(&m_ClipPlanes[2]));
	Maths::HVec4 vPlanes3(Maths::Load<Maths::HVec4>(&m_ClipPlanes[3]));
	Maths::HVec4 vPlanes4(Maths::Load<Maths::HVec4>(&m_ClipPlanes[4]));
	Maths::HVec4 vPlanes5(Maths::Load<Maths::HVec4>(&m_ClipPlanes[5]));
	Maths::HVec4 vPlanes6(Maths::Load<Maths::HVec4>(&m_ClipPlanes[6]));
	Maths::HVec4 vPlanes7(Maths::Load<Maths::HVec4>(&m_ClipPlanes[7]));
	Maths::HVec4 vPlanes8(Maths::Load<Maths::HVec4>(&m_ClipPlanes[8]));
	Maths::HVec4 vPlanes9(Maths::Load<Maths::HVec4>(&m_ClipPlanes[9]));
	Maths::HVec4 vPlanes10(Maths::Load<Maths::HVec4>(&m_ClipPlanes[10]));
	Maths::HVec4 vPlanes11(Maths::Load<Maths::HVec4>(&m_ClipPlanes[11]));

	Maths::HF32 d1_0 = vPlanes0 | vCentre;
	Maths::HF32 d1_1 = vPlanes1 | vCentre;
	Maths::HF32 d1_2 = vPlanes2 | vCentre;
	Maths::HF32 d1_3 = vPlanes3 | vCentre;
	Maths::HF32 d1_4 = vPlanes4 | vCentre;
	Maths::HF32 d1_5 = vPlanes5 | vCentre;
	Maths::HF32 d1_6 = vPlanes6 | vCentre;
	Maths::HF32 d1_7 = vPlanes7 | vCentre;
	Maths::HF32 d1_8 = vPlanes8 | vCentre;
	Maths::HF32 d1_9 = vPlanes9 | vCentre;
	Maths::HF32 d1_10 = vPlanes10 | vCentre;
	Maths::HF32 d1_11 = vPlanes11 | vCentre;

	Maths::HVec4 hd1_0 = Maths::HVec4(d1_0, d1_1, d1_2, d1_3);
	Maths::HVec4 hd1_4 = Maths::HVec4(d1_4, d1_5, d1_6, d1_7);
	Maths::HVec4 hd1_8 = Maths::HVec4(d1_8, d1_9, d1_10, d1_11);

	Maths::Vector4_Bool r_0 = hd1_0 < r;
	Maths::Vector4_Bool r_1 = hd1_4 < r;
	Maths::Vector4_Bool r_2 = hd1_8 < r;

	Maths::Vector4_Bool r_4 = r_0 | r_1;
	Maths::Vector4_Bool r_5 = r_2 | r_4;

	return AllFalse(r_5);
}

template<>
INLINE const S32 C_ViewFrustum::SphereCullT<10>(const Maths::C_Sphere& sphere) const
{
	BIZ_STATIC_ASSERT(m_MaxPlaneCount >= 10);

	Maths::HVec4 r(-sphere.Radius());
	Maths::HVec4 vCentre(sphere.Centre());
	vCentre.W(Maths::Constants<Maths::HF32>::One());
	Maths::HVec4 vPlanes0(Maths::Load<Maths::HVec4>(&m_ClipPlanes[0]));
	Maths::HVec4 vPlanes1(Maths::Load<Maths::HVec4>(&m_ClipPlanes[1]));
	Maths::HVec4 vPlanes2(Maths::Load<Maths::HVec4>(&m_ClipPlanes[2]));
	Maths::HVec4 vPlanes3(Maths::Load<Maths::HVec4>(&m_ClipPlanes[3]));
	Maths::HVec4 vPlanes4(Maths::Load<Maths::HVec4>(&m_ClipPlanes[4]));
	Maths::HVec4 vPlanes5(Maths::Load<Maths::HVec4>(&m_ClipPlanes[5]));
	Maths::HVec4 vPlanes6(Maths::Load<Maths::HVec4>(&m_ClipPlanes[6]));
	Maths::HVec4 vPlanes7(Maths::Load<Maths::HVec4>(&m_ClipPlanes[7]));
	Maths::HVec4 vPlanes8(Maths::Load<Maths::HVec4>(&m_ClipPlanes[8]));
	Maths::HVec4 vPlanes9(Maths::Load<Maths::HVec4>(&m_ClipPlanes[9]));

	Maths::HF32 d1_0 = vPlanes0 | vCentre;
	Maths::HF32 d1_1 = vPlanes1 | vCentre;
	Maths::HF32 d1_2 = vPlanes2 | vCentre;
	Maths::HF32 d1_3 = vPlanes3 | vCentre;
	Maths::HF32 d1_4 = vPlanes4 | vCentre;
	Maths::HF32 d1_5 = vPlanes5 | vCentre;
	Maths::HF32 d1_6 = vPlanes6 | vCentre;
	Maths::HF32 d1_7 = vPlanes7 | vCentre;
	Maths::HF32 d1_8 = vPlanes8 | vCentre;
	Maths::HF32 d1_9 = vPlanes9 | vCentre;

	Maths::HVec4 hd1_0 = Maths::HVec4(d1_0, d1_1, d1_2, d1_3);
	Maths::HVec4 hd1_4 = Maths::HVec4(d1_4, d1_5, d1_6, d1_7);
	Maths::HVec4 hd1_8 = Maths::HVec4(d1_8, d1_9, d1_8, d1_9);

	Maths::Vector4_Bool r_0 = hd1_0 < r;
	Maths::Vector4_Bool r_1 = hd1_4 < r;
	Maths::Vector4_Bool r_2 = hd1_8 < r;

	Maths::Vector4_Bool r_4 = r_0 | r_1;
	Maths::Vector4_Bool r_5 = r_2 | r_4;

	return AllFalse(r_5);
}

template<>
INLINE const S32 C_ViewFrustum::SphereCullT<9>(const Maths::C_Sphere& sphere) const
{
	BIZ_STATIC_ASSERT(m_MaxPlaneCount >= 9);

	Maths::HVec4 r(-sphere.Radius());
	Maths::HVec4 vCentre(sphere.Centre());
	vCentre.W(Maths::Constants<Maths::HF32>::One());
	Maths::HVec4 vPlanes0(Maths::Load<Maths::HVec4>(&m_ClipPlanes[0]));
	Maths::HVec4 vPlanes1(Maths::Load<Maths::HVec4>(&m_ClipPlanes[1]));
	Maths::HVec4 vPlanes2(Maths::Load<Maths::HVec4>(&m_ClipPlanes[2]));
	Maths::HVec4 vPlanes3(Maths::Load<Maths::HVec4>(&m_ClipPlanes[3]));
	Maths::HVec4 vPlanes4(Maths::Load<Maths::HVec4>(&m_ClipPlanes[4]));
	Maths::HVec4 vPlanes5(Maths::Load<Maths::HVec4>(&m_ClipPlanes[5]));
	Maths::HVec4 vPlanes6(Maths::Load<Maths::HVec4>(&m_ClipPlanes[6]));
	Maths::HVec4 vPlanes7(Maths::Load<Maths::HVec4>(&m_ClipPlanes[7]));
	Maths::HVec4 vPlanes8(Maths::Load<Maths::HVec4>(&m_ClipPlanes[8]));

	Maths::HF32 d1_0 = vPlanes0 | vCentre;
	Maths::HF32 d1_1 = vPlanes1 | vCentre;
	Maths::HF32 d1_2 = vPlanes2 | vCentre;
	Maths::HF32 d1_3 = vPlanes3 | vCentre;
	Maths::HF32 d1_4 = vPlanes4 | vCentre;
	Maths::HF32 d1_5 = vPlanes5 | vCentre;
	Maths::HF32 d1_6 = vPlanes6 | vCentre;
	Maths::HF32 d1_7 = vPlanes7 | vCentre;
	Maths::HF32 d1_8 = vPlanes8 | vCentre;

	Maths::HVec4 hd1_0 = Maths::HVec4(d1_0, d1_1, d1_2, d1_3);
	Maths::HVec4 hd1_4 = Maths::HVec4(d1_4, d1_5, d1_6, d1_7);
	Maths::HVec4 hd1_8 = Maths::HVec4(d1_8);

	Maths::Vector4_Bool r_0 = hd1_0 < r;
	Maths::Vector4_Bool r_1 = hd1_4 < r;
	Maths::Vector4_Bool r_2 = hd1_8 < r;

	Maths::Vector4_Bool r_4 = r_0 | r_1;
	Maths::Vector4_Bool r_5 = r_2 | r_4;

	return AllFalse(r_5);
}

template<>
INLINE const S32 C_ViewFrustum::SphereCullT<8>(const Maths::C_Sphere& sphere) const
{
	BIZ_STATIC_ASSERT(m_MaxPlaneCount >= 8);

	Maths::HVec4 r(-sphere.Radius());
	Maths::HVec4 vCentre(sphere.Centre());
	vCentre.W(Maths::Constants<Maths::HF32>::One());
	Maths::HVec4 vPlanes0(Maths::Load<Maths::HVec4>(&m_ClipPlanes[0]));
	Maths::HVec4 vPlanes1(Maths::Load<Maths::HVec4>(&m_ClipPlanes[1]));
	Maths::HVec4 vPlanes2(Maths::Load<Maths::HVec4>(&m_ClipPlanes[2]));
	Maths::HVec4 vPlanes3(Maths::Load<Maths::HVec4>(&m_ClipPlanes[3]));
	Maths::HVec4 vPlanes4(Maths::Load<Maths::HVec4>(&m_ClipPlanes[4]));
	Maths::HVec4 vPlanes5(Maths::Load<Maths::HVec4>(&m_ClipPlanes[5]));
	Maths::HVec4 vPlanes6(Maths::Load<Maths::HVec4>(&m_ClipPlanes[6]));
	Maths::HVec4 vPlanes7(Maths::Load<Maths::HVec4>(&m_ClipPlanes[7]));

	Maths::HF32 d1_0 = vPlanes0 | vCentre;
	Maths::HF32 d1_1 = vPlanes1 | vCentre;
	Maths::HF32 d1_2 = vPlanes2 | vCentre;
	Maths::HF32 d1_3 = vPlanes3 | vCentre;
	Maths::HF32 d1_4 = vPlanes4 | vCentre;
	Maths::HF32 d1_5 = vPlanes5 | vCentre;
	Maths::HF32 d1_6 = vPlanes6 | vCentre;
	Maths::HF32 d1_7 = vPlanes7 | vCentre;

	Maths::HVec4 hd1_0 = Maths::HVec4(d1_0, d1_1, d1_2, d1_3);
	Maths::HVec4 hd1_4 = Maths::HVec4(d1_4, d1_5, d1_6, d1_7);

	Maths::Vector4_Bool r_0 = hd1_0 < r;
	Maths::Vector4_Bool r_1 = hd1_4 < r;

	Maths::Vector4_Bool r_2 = r_0 | r_1;

	return AllFalse(r_2);
}

template<>
INLINE const S32 C_ViewFrustum::SphereCullT<6>(const Maths::C_Sphere& sphere) const
{
	BIZ_STATIC_ASSERT(m_MaxPlaneCount >= 6);

	Maths::HVec4 r(-sphere.Radius());
	Maths::HVec4 vCentre(sphere.Centre());
	vCentre.W(Maths::Constants<Maths::HF32>::One());
	Maths::HVec4 vPlanes0(Maths::Load<Maths::HVec4>(&m_ClipPlanes[0]));
	Maths::HVec4 vPlanes1(Maths::Load<Maths::HVec4>(&m_ClipPlanes[1]));
	Maths::HVec4 vPlanes2(Maths::Load<Maths::HVec4>(&m_ClipPlanes[2]));
	Maths::HVec4 vPlanes3(Maths::Load<Maths::HVec4>(&m_ClipPlanes[3]));
	Maths::HVec4 vPlanes4(Maths::Load<Maths::HVec4>(&m_ClipPlanes[4]));
	Maths::HVec4 vPlanes5(Maths::Load<Maths::HVec4>(&m_ClipPlanes[5]));

	Maths::HF32 d1_0 = vPlanes0 | vCentre;
	Maths::HF32 d1_1 = vPlanes1 | vCentre;
	Maths::HF32 d1_2 = vPlanes2 | vCentre;
	Maths::HF32 d1_3 = vPlanes3 | vCentre;
	Maths::HF32 d1_4 = vPlanes4 | vCentre;
	Maths::HF32 d1_5 = vPlanes5 | vCentre;

	Maths::HVec4 hd1_0 = Maths::HVec4(d1_0, d1_1, d1_2, d1_3);
	Maths::HVec4 hd1_4 = Maths::HVec4(d1_4, d1_5, d1_4, d1_5);

	Maths::Vector4_Bool r_0 = hd1_0 < r;
	Maths::Vector4_Bool r_1 = hd1_4 < r;

	Maths::Vector4_Bool r_2 = r_0 | r_1;

	return AllFalse(r_2);
}

INLINE const S32 C_ViewFrustum::SphereCull(const Maths::C_Sphere& sphere) const
{
	//return SphereCullT<16>(sphere);

	const Maths::C_Vertex centre( sphere.Centre() );
    const Maths::HF32     radius( sphere.Radius() );
    
    U32 result = 0;
    U32 planeFlag = 1;
    
    for( U32 i=0; i<m_PlaneCount; i++ )
    {
		Maths::C_Plane plane(Maths::Load<Maths::HVec4>(&m_ClipPlanes[i]));
        const Maths::HF32 dist( plane.Distance( centre ) );
        if( dist > radius )
        {
            // completely infront of plane
            // visible, no clipping required
        }
        else
        {
            if( dist < -radius )
            {
                // completely behind plane, not visible
                return 0;
            }
            else
            {
                // spans plane, clipping required
                result |= planeFlag;
            }
        }        
        
        planeFlag = planeFlag << 1;
    }
    
    if( !result )
    {
        result = result | E_CullResult_FullyVisible;
    }
    
    return result | E_CullResult_Visible;
}

// cull C_AxisBox
INLINE const S32 C_ViewFrustum::CullAxisBox(const Maths::C_AxisBox& axisBox) const
{
	Maths::HVec3 vBoxCentre = axisBox.Centre();
	Maths::HVec3 vSize = axisBox.Max() - vBoxCentre;
	Maths::HVec4 vCentre(vBoxCentre);
	vCentre.W(Maths::Constants<Maths::HF32>::One());

	return CullAxisBox(vCentre, vSize);
}

// cull C_AxisBox, override far clip
INLINE const S32 C_ViewFrustum::CullAxisBox(const Maths::C_AxisBox& axisBox, const Maths::HF32& fFarClip) const
{
	Maths::HVec3 vBoxCentre = axisBox.Centre();
	Maths::HVec3 vBoxSize = axisBox.Max() - vBoxCentre;
	Maths::HVec4 vCentre(vBoxCentre);
	vCentre.W(Maths::Constants<Maths::HF32>::One());
	Maths::HVec4 vSize(vBoxSize);
	vSize.W(fFarClip);

	return CullAxisBox(vCentre, vSize);
}

// centre of object, plus half size. w component of vSizeCull is culling distance
INLINE const S32 C_ViewFrustum::CullAxisBox(const Maths::HVec4& vCentre, const Maths::HVec4& vSizeCull) const
{
	Maths::HVec3 vSize(vSizeCull);

	Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&m_Forward));

	Maths::HVec4 vPlanes0(Maths::Load<Maths::HVec4>(&m_ClipPlanes[0]));
	Maths::HVec4 vPlanes1(Maths::Load<Maths::HVec4>(&m_ClipPlanes[1]));
	Maths::HVec4 vPlanes2(Maths::Load<Maths::HVec4>(&m_ClipPlanes[2]));
	Maths::HVec4 vPlanes3(Maths::Load<Maths::HVec4>(&m_ClipPlanes[3]));
	Maths::HVec4 vPlanes4(Maths::Load<Maths::HVec4>(&m_ClipPlanes[4]));
	Maths::HVec4 vPlanes5(Maths::Load<Maths::HVec4>(&m_ClipPlanes[5]));

	Maths::HVec3 vPlanesAbs0(Maths::Load<Maths::HVec3>(&m_ClipPlanesAbs[0]));
	Maths::HVec3 vPlanesAbs1(Maths::Load<Maths::HVec3>(&m_ClipPlanesAbs[1]));
	Maths::HVec3 vPlanesAbs2(Maths::Load<Maths::HVec3>(&m_ClipPlanesAbs[2]));
	Maths::HVec3 vPlanesAbs3(Maths::Load<Maths::HVec3>(&m_ClipPlanesAbs[3]));
	Maths::HVec3 vPlanesAbs4(Maths::Load<Maths::HVec3>(&m_ClipPlanesAbs[4]));
	Maths::HVec3 vPlanesAbs5(Maths::Load<Maths::HVec3>(&m_ClipPlanesAbs[5]));

	Maths::HF32 d = (vCentre | forward);
	Maths::HF32 s = vSizeCull.W();

	Maths::HF32 d1_0 = vPlanes0 | vCentre;
	Maths::HF32 d2_0 = vPlanesAbs0 | vSize;

	Maths::HF32 d1_1 = vPlanes1 | vCentre;
	Maths::HF32 d2_1 = vPlanesAbs1 | vSize;

	Maths::HF32 d1_2 = vPlanes2 | vCentre;
	Maths::HF32 d2_2 = vPlanesAbs2 | vSize;

	Maths::HF32 d1_3 = vPlanes3 | vCentre;
	Maths::HF32 d2_3 = vPlanesAbs3 | vSize;

	Maths::HF32 d1_4 = vPlanes4 | vCentre;
	Maths::HF32 d2_4 = vPlanesAbs4 | vSize;

	Maths::HF32 d1_5 = vPlanes5 | vCentre;
	Maths::HF32 d2_5 = vPlanesAbs5 | vSize;
 
	Maths::HVec4 hd1_0 = Maths::HVec4(d1_0, d1_1, d1_2, d1_3);
	Maths::HVec4 hd2_0 = Maths::HVec4(d2_0, d2_1, d2_2, d2_3);

	Maths::HVec4 hd1_4 = Maths::HVec4(d1_4, d1_5, d1_4, s);
	Maths::HVec4 hd2_4 = Maths::HVec4(d2_4, d2_5, d2_4, d);

	Maths::Vector4_Bool r_0 = hd1_0 < hd2_0;
	Maths::Vector4_Bool r_1 = hd1_4 < hd2_4;

	Maths::Vector4_Bool r_2 = r_0 | r_1;

	return S32(AllFalse(r_2));
};

INLINE const S32 C_ViewFrustum::CullAxisBox(const Maths::HVec4& vCentre, const Maths::HVec3& vSize) const
{
	Maths::HVec4 vPlanes0(Maths::Load<Maths::HVec4>(&m_ClipPlanes[0]));
	Maths::HVec4 vPlanes1(Maths::Load<Maths::HVec4>(&m_ClipPlanes[1]));
	Maths::HVec4 vPlanes2(Maths::Load<Maths::HVec4>(&m_ClipPlanes[2]));
	Maths::HVec4 vPlanes3(Maths::Load<Maths::HVec4>(&m_ClipPlanes[3]));
	Maths::HVec4 vPlanes4(Maths::Load<Maths::HVec4>(&m_ClipPlanes[4]));
	Maths::HVec4 vPlanes5(Maths::Load<Maths::HVec4>(&m_ClipPlanes[5]));

	Maths::HVec3 vPlanesAbs0(Maths::Load<Maths::HVec3>(&m_ClipPlanesAbs[0]));
	Maths::HVec3 vPlanesAbs1(Maths::Load<Maths::HVec3>(&m_ClipPlanesAbs[1]));
	Maths::HVec3 vPlanesAbs2(Maths::Load<Maths::HVec3>(&m_ClipPlanesAbs[2]));
	Maths::HVec3 vPlanesAbs3(Maths::Load<Maths::HVec3>(&m_ClipPlanesAbs[3]));
	Maths::HVec3 vPlanesAbs4(Maths::Load<Maths::HVec3>(&m_ClipPlanesAbs[4]));
	Maths::HVec3 vPlanesAbs5(Maths::Load<Maths::HVec3>(&m_ClipPlanesAbs[5]));

	Maths::HF32 d1_0 = vPlanes0 | vCentre;
	Maths::HF32 d2_0 = vPlanesAbs0 | vSize;

	Maths::HF32 d1_1 = vPlanes1 | vCentre;
	Maths::HF32 d2_1 = vPlanesAbs1 | vSize;

	Maths::HF32 d1_2 = vPlanes2 | vCentre;
	Maths::HF32 d2_2 = vPlanesAbs2 | vSize;

	Maths::HF32 d1_3 = vPlanes3 | vCentre;
	Maths::HF32 d2_3 = vPlanesAbs3 | vSize;

	Maths::HF32 d1_4 = vPlanes4 | vCentre;
	Maths::HF32 d2_4 = vPlanesAbs4 | vSize;

	Maths::HF32 d1_5 = vPlanes5 | vCentre;
	Maths::HF32 d2_5 = vPlanesAbs5 | vSize;
 
	Maths::HVec4 hd1_0 = Maths::HVec4(d1_0, d1_1, d1_2, d1_3);
	Maths::HVec4 hd2_0 = Maths::HVec4(d2_0, d2_1, d2_2, d2_3);

	Maths::HVec4 hd1_4 = Maths::HVec4(d1_4, d1_5, d1_4, d1_5);
	Maths::HVec4 hd2_4 = Maths::HVec4(d2_4, d2_5, d2_4, d2_5);

	Maths::Vector4_Bool r_0 = hd1_0 < hd2_0;
	Maths::Vector4_Bool r_1 = hd1_4 < hd2_4;

	Maths::Vector4_Bool r_2 = r_0 | r_1;

	return AllFalse(r_2);
}

// centre of object, plus half size. x component of vClip is near, y component is far clip
FORCE_INLINE const S32 C_ViewFrustum::CullAxisBox(const Maths::HVec4& vCentre, const Maths::HVec4& vSize, const Maths::HVec4& vClip) const
{
	Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&m_Forward));
	Maths::HVec4 forwardAbs(Maths::Load<Maths::HVec4>(&m_ForwardAbs));

	Maths::HVec4 vPlanes0(Maths::Load<Maths::HVec4>(&m_ClipPlanes[0]));
	Maths::HVec4 vPlanes1(Maths::Load<Maths::HVec4>(&m_ClipPlanes[1]));
	Maths::HVec4 vPlanes2(Maths::Load<Maths::HVec4>(&m_ClipPlanes[2]));
	Maths::HVec4 vPlanes3(Maths::Load<Maths::HVec4>(&m_ClipPlanes[3]));
	Maths::HVec4 vPlanes4(Maths::Load<Maths::HVec4>(&m_ClipPlanes[4]));
	Maths::HVec4 vPlanes5(Maths::Load<Maths::HVec4>(&m_ClipPlanes[5]));

	Maths::HVec4 vPlanesAbs0(Maths::Load<Maths::HVec4>(&m_ClipPlanesAbs[0]));
	Maths::HVec4 vPlanesAbs1(Maths::Load<Maths::HVec4>(&m_ClipPlanesAbs[1]));
	Maths::HVec4 vPlanesAbs2(Maths::Load<Maths::HVec4>(&m_ClipPlanesAbs[2]));
	Maths::HVec4 vPlanesAbs3(Maths::Load<Maths::HVec4>(&m_ClipPlanesAbs[3]));
	Maths::HVec4 vPlanesAbs4(Maths::Load<Maths::HVec4>(&m_ClipPlanesAbs[4]));
	Maths::HVec4 vPlanesAbs5(Maths::Load<Maths::HVec4>(&m_ClipPlanesAbs[5]));

	Maths::HF32 distance = vCentre | forward;
	Maths::HF32 sizeZ = vSize | forwardAbs;
	Maths::HF32 nearClip = vClip.X() - sizeZ;
	Maths::HF32 farClip = vClip.Y() + sizeZ;

	Maths::HF32 d1_0 = vPlanes0 | vCentre;
	Maths::HF32 d2_0 = vPlanesAbs0 | vSize;

	Maths::HF32 d1_1 = vPlanes1 | vCentre;
	Maths::HF32 d2_1 = vPlanesAbs1 | vSize;

	Maths::HF32 d1_2 = vPlanes2 | vCentre;
	Maths::HF32 d2_2 = vPlanesAbs2 | vSize;

	Maths::HF32 d1_3 = vPlanes3 | vCentre;
	Maths::HF32 d2_3 = vPlanesAbs3 | vSize;

	Maths::HF32 d1_4 = vPlanes4 | vCentre;
	Maths::HF32 d2_4 = vPlanesAbs4 | vSize;

	Maths::HF32 d1_5 = vPlanes5 | vCentre;
	Maths::HF32 d2_5 = vPlanesAbs5 | vSize;
 
	Maths::HVec4 hd1_0 = Maths::HVec4(d1_0, d1_1, d1_2, d1_3);
	Maths::HVec4 hd2_0 = Maths::HVec4(d2_0, d2_1, d2_2, d2_3);

	Maths::HVec4 hd1_4 = Maths::HVec4(d1_4, d1_5, distance, farClip);
	Maths::HVec4 hd2_4 = Maths::HVec4(d2_4, d2_5, nearClip, distance);

	Maths::Vector4_Bool r_0 = hd1_0 < hd2_0;
	Maths::Vector4_Bool r_1 = hd1_4 < hd2_4;

	Maths::Vector4_Bool r_2 = r_0 | r_1;

	return AllFalse(r_2);
};

INLINE const U32 C_ViewFrustum::CalculateAxisBoxPredication4(const Maths::HVec4& vCentre, const Maths::HVec3& vSize) const
{
	return 0;
}

INLINE const U32 C_ViewFrustum::CalculateAxisBoxPredication3(const Maths::HVec4& vCentre, const Maths::HVec3& vSize) const
{
	return 0;
}

INLINE const U32 C_ViewFrustum::CalculateAxisBoxPredication2(const Maths::HVec4& vCentre, const Maths::HVec3& vSize) const
{
	return 0;
}

// cull C_Sphere
INLINE const S32 C_ViewFrustum::CullSphere(const Maths::C_Sphere& sphere) const
{
	Maths::HF32 fRadius(sphere.Radius());
	Maths::HVec4 vCentre(sphere.Centre());
	vCentre.W(Maths::Constants<Maths::HF32>::One());

	return CullSphere(vCentre, fRadius);
}

// cull C_Sphere, override far clip
INLINE const S32 C_ViewFrustum::CullSphere(const Maths::C_Sphere& sphere, const Maths::HF32& fFarClip) const
{
	Maths::HF32 fRadius(sphere.Radius());
	Maths::HVec4 vCentre(sphere.Centre());
	vCentre.W(Maths::Constants<Maths::HF32>::One());

	return CullSphere(vCentre, fRadius, fFarClip);
}

// cull sphere from centre and radius
INLINE const S32 C_ViewFrustum::CullSphere(const Maths::HVec4& vCentre, const Maths::HF32& fRadius) const
{
	Maths::HVec4 r(-fRadius);
	Maths::HVec4 vPlanes0(Maths::Load<Maths::HVec4>(&m_ClipPlanes[0]));
	Maths::HVec4 vPlanes1(Maths::Load<Maths::HVec4>(&m_ClipPlanes[1]));
	Maths::HVec4 vPlanes2(Maths::Load<Maths::HVec4>(&m_ClipPlanes[2]));
	Maths::HVec4 vPlanes3(Maths::Load<Maths::HVec4>(&m_ClipPlanes[3]));
	Maths::HVec4 vPlanes4(Maths::Load<Maths::HVec4>(&m_ClipPlanes[4]));
	Maths::HVec4 vPlanes5(Maths::Load<Maths::HVec4>(&m_ClipPlanes[5]));

	Maths::HF32 d1_0 = vPlanes0 | vCentre;
	Maths::HF32 d1_1 = vPlanes1 | vCentre;
	Maths::HF32 d1_2 = vPlanes2 | vCentre;
	Maths::HF32 d1_3 = vPlanes3 | vCentre;
	Maths::HF32 d1_4 = vPlanes4 | vCentre;
	Maths::HF32 d1_5 = vPlanes5 | vCentre;

	Maths::HVec4 hd1_0 = Maths::HVec4(d1_0, d1_1, d1_2, d1_3);
	Maths::HVec4 hd1_4 = Maths::HVec4(d1_4, d1_5, d1_4, d1_5);

	Maths::Vector4_Bool r_0 = hd1_0 < r;
	Maths::Vector4_Bool r_1 = hd1_4 < r;

	Maths::Vector4_Bool r_2 = r_0 | r_1;

	return AllFalse(r_2);
}

// cull sphere from centre and radius, override far clip
INLINE const S32 C_ViewFrustum::CullSphere(const Maths::HVec4& vCentre, const Maths::HF32& fRadius, const Maths::HF32& fFarClip) const
{
	Maths::HVec4 r0(-fRadius);
	Maths::HVec4 r1(-fRadius);
	Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&m_Forward));
	Maths::HVec4 vPlanes0(Maths::Load<Maths::HVec4>(&m_ClipPlanes[0]));
	Maths::HVec4 vPlanes1(Maths::Load<Maths::HVec4>(&m_ClipPlanes[1]));
	Maths::HVec4 vPlanes2(Maths::Load<Maths::HVec4>(&m_ClipPlanes[2]));
	Maths::HVec4 vPlanes3(Maths::Load<Maths::HVec4>(&m_ClipPlanes[3]));
	Maths::HVec4 vPlanes4(Maths::Load<Maths::HVec4>(&m_ClipPlanes[4]));
	Maths::HVec4 vPlanes5(Maths::Load<Maths::HVec4>(&m_ClipPlanes[5]));

	Maths::HF32 d = (vCentre | forward);;
	r1.W(d);

	Maths::HF32 d1_0 = vPlanes0 | vCentre;
	Maths::HF32 d1_1 = vPlanes1 | vCentre;
	Maths::HF32 d1_2 = vPlanes2 | vCentre;
	Maths::HF32 d1_3 = vPlanes3 | vCentre;
	Maths::HF32 d1_4 = vPlanes4 | vCentre;
	Maths::HF32 d1_5 = vPlanes5 | vCentre;

	Maths::HVec4 hd1_0 = Maths::HVec4(d1_0, d1_1, d1_2, d1_3);
	Maths::HVec4 hd1_4 = Maths::HVec4(d1_4, d1_5, d1_4, fFarClip);

	Maths::Vector4_Bool r_0 = hd1_0 < r0;
	Maths::Vector4_Bool r_1 = hd1_4 < r1;

	Maths::Vector4_Bool r_2 = r_0 | r_1;

	return AllFalse(r_2);
}

INLINE const U32 C_ViewFrustum::CalculateSpherePredication3(const Maths::HVec4& vCentre, const Maths::HF32& fRadius) const
{
	return 0;
}

INLINE const U32 C_ViewFrustum::CalculateSpherePredication4(const Maths::HVec4& vCentre, const Maths::HF32& fRadius) const
{
	return 0;
}

}; // namespace Display

#endif // DISPLAY_VIEW_VIEW_FRUSTM_INL
