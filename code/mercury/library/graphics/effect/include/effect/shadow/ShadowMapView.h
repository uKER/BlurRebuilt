#ifndef EFFECT_SHADOW_MAP_VIEW_H
#define EFFECT_SHADOW_MAP_VIEW_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Effect
{

class C_ShadowMapView
{
public:
	Display::C_View			m_View;
	Display::C_ViewSetup	m_ViewSetup;
};

void ShadowFrustum(const Maths::C_Rect& viewExtents, const F32 fPerspDist, const F32 fNearClip, const F32 fFarClip, Maths::HVec3* vPoints);

Maths::C_AxisBox ShadowFrustumAABB(const Maths::HVec3* vPoints);
Maths::C_AxisBox ShadowClipAABB(const Maths::C_AxisBox& box, const Maths::HMat44& mViewProj);

Maths::HMat43 ShadowMatrixCamera(const Maths::HVec3& vPosition, const Maths::HVec3& vDirection);
Maths::HMat43 ShadowMatrixLookAt(const Maths::HVec3& vEye, const Maths::HVec3& vAt);
Maths::HMat44 ShadowMatrixOrtho(const F32 fWidth, const F32 fHeight, const F32 fNear, const F32 fFar);
Maths::HMat44 ShadowMatrixOrtho(const Maths::C_Rect& vViewExtents, const F32 fNear, const F32 fFar);
Maths::HMat44 ShadowMatrixPerspective(const F32 fWidth, const F32 fHeight, const F32 fNear, const F32 fFar);
Maths::HMat44 ShadowMatrixPerspectiveFieldOfView(const F32 fFieldOfView, const F32 fAspectRatio, const F32 fNear, const F32 fFar);
Maths::HMat44 ShadowMatrixRound(const Maths::HMat44& mViewProj, const F32 fWidth, const F32 fHeight);
Maths::HMat44 ShadowMatrixCrop(const Maths::HVec3& vMin, const Maths::HVec3& vMax);
Maths::HMat44 ShadowMatrixCrop(const Maths::HMat44& mViewProj, const Maths::HVec3* vPoints);
Maths::HMat44 ShadowMatrixCropRect(const Maths::HVec3& vMin, const Maths::HVec3& vMax);
Maths::HMat44 ShadowMatrixCropRect(const Maths::HMat44& mViewProj, const Maths::HVec3* vPoints);

}; // namespace Effect

#endif // EFFECT_SHADOW_MAP_VIEW_H
