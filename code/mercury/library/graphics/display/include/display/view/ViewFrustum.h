#ifndef DISPLAY_VIEW_VIEW_FRUSTM_H
#define DISPLAY_VIEW_VIEW_FRUSTM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Utils/Matrix.h"
#include "Display/View/ViewSetup.h"

namespace Display
{

struct C_TileSetup;

//! Cull result.

enum E_CullResult
{
	E_CullResult_Visible		= 0x00010000,
	E_CullResult_FullyVisible	= 0x00020000,
};

//! View frustum.

class C_ViewFrustum
{
public:

	C_ViewFrustum();
	~C_ViewFrustum();

	// setup the view frustum using explicit planes
	void Setup(const U32 planeCount, const Maths::C_Plane* pPlanes);
	
	// deprecate this
	const S32 SphereCull(const Maths::C_Sphere& sphere) const;

	template<int PlaneCount> const S32 SphereCullT(const Maths::C_Sphere& sphere) const;
#ifdef COMPILER_VC
	template<> const S32 SphereCullT<16>(const Maths::C_Sphere& sphere) const;
	template<> const S32 SphereCullT<12>(const Maths::C_Sphere& sphere) const;
	template<> const S32 SphereCullT<10>(const Maths::C_Sphere& sphere) const;
	template<> const S32 SphereCullT<9>(const Maths::C_Sphere& sphere) const;
	template<> const S32 SphereCullT<8>(const Maths::C_Sphere& sphere) const;
	template<> const S32 SphereCullT<6>(const Maths::C_Sphere& sphere) const;
#endif

	void Setup(const Maths::HMat44& cameraToDevice, const Maths::HMat44& worldToDevice, const Maths::HF32& windowWidth);

	void SetupTilingPlanes(const Maths::HMat44& worldToDevice, const C_TileSetup& tileSetup);

	const S32 CullAxisBox(const Maths::C_AxisBox& axisBox) const;
	const S32 CullAxisBox(const Maths::C_AxisBox& axisBox, const Maths::HF32& fFarClip) const;
	const S32 CullAxisBox(const Maths::HVec4& vCentre, const Maths::HVec4& vSizeCull) const;
	const S32 CullAxisBox(const Maths::HVec4& vCentre, const Maths::HVec3& vSize) const;
	const S32 CullAxisBox(const Maths::HVec4& vCentre, const Maths::HVec4& vSize, const Maths::HVec4& vClip) const;

	const U32 CalculateAxisBoxPredication2(const Maths::HVec4& vCentre, const Maths::HVec3& vSize) const;
	const U32 CalculateAxisBoxPredication3(const Maths::HVec4& vCentre, const Maths::HVec3& vSize) const;
	const U32 CalculateAxisBoxPredication4(const Maths::HVec4& vCentre, const Maths::HVec3& vSize) const;

	const S32 CullSphere(const Maths::C_Sphere& sphere) const;
	const S32 CullSphere(const Maths::C_Sphere& sphere, const Maths::HF32& fFarClip) const;
	const S32 CullSphere(const Maths::HVec4& vCentre, const Maths::HF32& fRadius) const;
	const S32 CullSphere(const Maths::HVec4& vCentre, const Maths::HF32& fRadius, const Maths::HF32& fFarClip) const;

	const U32 CalculateSpherePredication3(const Maths::HVec4& vCentre, const Maths::HF32& fRadius) const;
	const U32 CalculateSpherePredication4(const Maths::HVec4& vCentre, const Maths::HF32& fRadius) const;


	Maths::HVec3		GetPoint(const U32 i) const;
	Maths::C_Plane		GetPlane(const U32 i) const;
	U32					GetPlaneCount() const;

//private:

	void SetupClipPlaneAbs();

	static const U32			m_MaxPlaneCount = 16;
	U32							m_PlaneCount;
	F32							m_ForwardMultiplier;

	MathsStorage::Float4A16		m_Forward;
	MathsStorage::Float4A16		m_ForwardAbs;
	MathsStorage::Float4A16		m_ClipPlanes[m_MaxPlaneCount];
	MathsStorage::Float4A16		m_ClipPlanesAbs[m_MaxPlaneCount];


	MathsStorage::Float4A16		m_Points[8];
};

}; // namespace Display

#include "Display/View/ViewFrustum.inl"

#endif // DISPLAY_VIEW_VIEW_FRUSTM_H
