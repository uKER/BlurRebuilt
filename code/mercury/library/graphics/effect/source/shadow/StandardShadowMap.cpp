#include "stdafx.h"
#include "Effect/Shadow/StandardShadowMap.h"

namespace Effect
{

void DirectionalLightShadowMapSetup(C_ShadowMapView& view, const C_ShadowMapSetup& setup)
{
	// setup view
	view.m_ViewSetup.m_ViewSetup3D.m_FieldOfView = 0.0f;
	view.m_ViewSetup.m_ViewSetup3D.m_AspectRatio = setup.m_iWidth / (F32) setup.m_iHeight;
	view.m_ViewSetup.m_ViewSetup3D.m_PerspDist = 0.0f;
	view.m_ViewSetup.m_ViewSetup3D.m_NearClip = setup.m_fNearClip;
	view.m_ViewSetup.m_ViewSetup3D.m_FarClip = setup.m_fFarClip;
	view.m_ViewSetup.m_ViewSetup3D.m_ViewExtents = setup.m_vViewExtents;
	view.m_ViewSetup.m_ViewSetup3D.m_CameraToWorld = ShadowMatrixCamera(setup.m_vPosition, setup.m_vDirection);

	Display::Device()->SetupView(&view.m_View, view.m_ViewSetup, setup.m_iWidth, setup.m_iHeight);

	// setup an orthogonal projection matrix
	F32 fNearClip = setup.m_fNearClip;
	F32 fFarClip = setup.m_fFarClip;
	if (Display::Device()->GetDepthFlip())
	{
		fNearClip = setup.m_fFarClip;
		fFarClip = setup.m_fNearClip;
	}
	
	Maths::HMat44	mView		= (Maths::HMat44)view.m_View.m_ViewSettings3D.m_WorldToCamera;
	Maths::HMat44	mProj		= ShadowMatrixOrtho(setup.m_vViewExtents, fNearClip, fFarClip);
	Maths::HMat44	mViewProj	= mView * mProj;
	Maths::HMat44	mRound		= ShadowMatrixRound(mViewProj, (F32)setup.m_iWidth, (F32)setup.m_iHeight);
					mProj		= mProj * mRound;
					mViewProj	= mViewProj * mRound;

	view.m_View.m_ViewSettings3D.m_CameraToDevice = mProj;
	view.m_View.m_ViewSettings3D.m_WorldToDevice = mViewProj;

	Maths::Store(view.m_View.m_ViewSettings3D.m_CameraToDevice, &view.m_View.m_ViewSettings3D.m_ProjMatrix);
	Maths::Store(view.m_View.m_ViewSettings3D.m_WorldToDevice, &view.m_View.m_ViewSettings3D.m_ViewProjMatrix);

	const Maths::HF32 vWindowWidth = view.m_ViewSetup.m_WindowCoords.Size().X() / view.m_ViewSetup.GetDeviceCoords().Size().X();
	view.m_View.m_ViewFrustum.Setup(view.m_View.m_ViewSettings3D.m_CameraToDevice, view.m_View.m_ViewSettings3D.m_WorldToDevice, vWindowWidth);
}

void PointLightShadowMapSetup(C_ShadowMapView& view, const C_ShadowMapSetup& setup)
{
	// setup view
	view.m_ViewSetup.m_ViewSetup3D.m_FieldOfView = 0.0f;
	view.m_ViewSetup.m_ViewSetup3D.m_AspectRatio = setup.m_iWidth / (F32) setup.m_iHeight;
	view.m_ViewSetup.m_ViewSetup3D.m_PerspDist = setup.m_fPerspDist;
	view.m_ViewSetup.m_ViewSetup3D.m_NearClip = setup.m_fNearClip;
	view.m_ViewSetup.m_ViewSetup3D.m_FarClip = setup.m_fFarClip;
	view.m_ViewSetup.m_ViewSetup3D.m_ViewExtents = setup.m_vViewExtents;
	view.m_ViewSetup.m_ViewSetup3D.m_CameraToWorld = ShadowMatrixCamera(setup.m_vPosition, setup.m_vDirection);

	Display::Device()->SetupView(&view.m_View, view.m_ViewSetup, setup.m_iWidth, setup.m_iHeight);
}

}; // namespace Effect
