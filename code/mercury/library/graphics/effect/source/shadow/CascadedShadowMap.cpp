#include "stdafx.h"
#include "Effect/Shadow/CascadedShadowMap.h"

namespace Effect
{

F32 LogarithmicSplit(const U32 iSplit, const F32 fNear, const F32 fFar, const U32 nSplits)
{
	return fNear * powf(fFar / fNear, (F32) iSplit / (F32) nSplits);
}

F32 UniformSplit(const U32 iSplit, const F32 fNear, const F32 fFar, const U32 nSplits)
{
	return fNear + (fFar - fNear) * iSplit / (F32) nSplits;
}

F32 PracticalSplit(const U32 iSplit, const F32 fNear, const F32 fFar, const U32 nSplits, const F32 fLambda = 0.5f)
{
	const F32 fLogarithmic = LogarithmicSplit(iSplit, fNear, fFar, nSplits);
	const F32 fUniform = UniformSplit(iSplit, fNear, fFar, nSplits);
	return fLogarithmic * fLambda + fUniform * (1.0f - fLambda);
}

Maths::C_Sphere GetMinimalEnclosingSphere(const Maths::HVec3& vO, const Maths::HVec3& vA, const Maths::HVec3& vB, const Maths::HVec3& vC)
{
	const Maths::HVec3 va = vA - vO;
	const Maths::HVec3 vb = vB - vO;
	const Maths::HVec3 vc = vC - vO;

	const Maths::HVec3 vaCrossb = va ^ vb;
	const Maths::HVec3 vaCrossc = va ^ vc;

	const Maths::HF32 vDenominator0 = Maths::Constants<Maths::HF32>::Two() * vaCrossb.Length2();
	const Maths::HF32 vDenominator1 = Maths::Constants<Maths::HF32>::Two() * vaCrossc.Length2();

	const Maths::HVec3 vo0 = ((vaCrossb ^ va) * vb.Length2() + (vb ^ vaCrossb) * va.Length2()) * vDenominator0.Rcp();
	const Maths::HVec3 vo1 = ((vaCrossc ^ va) * vc.Length2() + (vc ^ vaCrossc) * va.Length2()) * vDenominator1.Rcp();

	const Maths::HF32 vRadius0 = vo0.Length();
	const Maths::HF32 vRadius1 = vo1.Length();
	const Maths::HVec3 vCentre0 = vo0 + vO;
	const Maths::HVec3 vCentre1 = vo1 + vO;

	const Maths::HVec3 vD = vCentre1 - vCentre0;
	const Maths::HF32 vd = Maths::Length(vD);

	const Maths::HF32 vRadius = 0.5f * (vd + vRadius0 + vRadius1);
	const Maths::HVec3 vCentre = vCentre0 + 0.5f * (vd + vRadius0 - vRadius1) * vD * vd.Rcp();

	return Maths::C_Sphere(vCentre, vRadius);
}

Maths::C_Sphere GetMinimalEnclosingSphere(const Maths::C_Rect& viewExtents, const F32 fPerspDist, const F32 fNearClip, const F32 fFarClip)
{
	const Maths::HVec2 vMin(viewExtents.Min() / fPerspDist);
	const Maths::HVec2 vMax(viewExtents.Max() / fPerspDist);

	const Maths::HVec3 v0(vMin.X(), vMin.Y(), Maths::Constants<Maths::HF32>::One());
	const Maths::HVec3 v1(vMin.X(), vMax.Y(), Maths::Constants<Maths::HF32>::One());
	const Maths::HVec3 v2(vMax.X(), vMax.Y(), Maths::Constants<Maths::HF32>::One());
	const Maths::HVec3 v3(vMax.X(), vMin.Y(), Maths::Constants<Maths::HF32>::One());

	const Maths::HVec3 vNear0(v0 * fNearClip);
	const Maths::HVec3 vNear1(v1 * fNearClip);
	const Maths::HVec3 vNear2(v2 * fNearClip);
	const Maths::HVec3 vNear3(v3 * fNearClip);
	const Maths::HVec3 vFar0(v0 * fFarClip);
	const Maths::HVec3 vFar1(v1 * fFarClip);
	const Maths::HVec3 vFar2(v2 * fFarClip);
	const Maths::HVec3 vFar3(v3 * fFarClip);

	return GetMinimalEnclosingSphere(vNear0, vNear2, vFar0, vFar2);
}

void ParallelSplitShadowMapSetup(C_ShadowMapView& view, const F32 fNearSplit, const F32 fFarSplit, const C_ShadowMapSetup& setup)
{
	// calculate minimal enclosing sphere of this part of the frustum
	const Display::C_ViewSettings3D& viewSettings3D = setup.m_pView->m_ViewSettings3D;

	const Maths::HF32	vZero	= Maths::Constants<Maths::HF32>::Zero();
	const Maths::HF32	vOne	= Maths::Constants<Maths::HF32>::One();

	// construct matrices
	F32 fNearClip = setup.m_fNearClip;
	F32 fFarClip = setup.m_fFarClip;
	if (Display::Device()->GetDepthFlip())
	{
		fNearClip = setup.m_fFarClip;
		fFarClip = setup.m_fNearClip;
	}

	Maths::HMat43	mCamera		= ShadowMatrixCamera(setup.m_vPosition, setup.m_vDirection);
	Maths::HMat43	mView		= mCamera.InverseRotTrans();
	Maths::HMat44	mProj		= ShadowMatrixOrtho(vOne, vOne, fNearClip, fFarClip);
	Maths::HMat44	mViewProj	= Maths::HMat44(mView) * mProj;
	
	// construct crop matrix and extract the view extents
	Maths::HVec3 vPoints[8];
	ShadowFrustum(viewSettings3D.GetViewExtents(), viewSettings3D.GetPerspDist(), fNearSplit, fFarSplit, vPoints);
	for (U32 pointNo = 0; pointNo < 8; ++pointNo)
	{
		vPoints[pointNo] = vPoints[pointNo] * viewSettings3D.GetCameraToWorld();
	}

	const Maths::C_AxisBox worldBox = ShadowFrustumAABB(vPoints);
	const Maths::C_AxisBox clipBox = ShadowClipAABB(worldBox, mViewProj);
	
	const Maths::HF32	vScaleX		= 2.0f * (clipBox.Max().X() - clipBox.Min().X()).Rcp();
	const Maths::HF32	vScaleY		= 2.0f * (clipBox.Max().Y() - clipBox.Min().Y()).Rcp();
	const Maths::HF32	vOffsetX	= -0.5f * (clipBox.Max().X() + clipBox.Min().X());
	const Maths::HF32	vOffsetY	= -0.5f * (clipBox.Max().Y() + clipBox.Min().Y());

	const Maths::HVec2	vViewExtentsMin((vOffsetX - 0.5f) * vScaleX, (vOffsetY - 0.5f) * vScaleY);
	const Maths::HVec2	vViewExtentsMax((vOffsetX + 0.5f) * vScaleX, (vOffsetY + 0.5f) * vScaleY);
	
	Maths::HMat44	mCrop		= ShadowMatrixCropRect(clipBox.Min(), clipBox.Max());
					mProj		= mProj * mCrop;
					mViewProj	= mViewProj * mCrop;

	Maths::HMat44	mRound		= ShadowMatrixRound(mViewProj, (F32)setup.m_iWidth, (F32)setup.m_iHeight);
					mProj		= mProj * mRound;
					mViewProj	= mViewProj * mRound;

	// setup view
	view.m_ViewSetup.m_ViewSetup3D.m_FieldOfView = 0.0f;
	view.m_ViewSetup.m_ViewSetup3D.m_AspectRatio = setup.m_iWidth / (F32) setup.m_iHeight;
	view.m_ViewSetup.m_ViewSetup3D.m_PerspDist = 0.0f;
	view.m_ViewSetup.m_ViewSetup3D.m_NearClip = setup.m_fNearClip;
	view.m_ViewSetup.m_ViewSetup3D.m_FarClip = setup.m_fFarClip;
	view.m_ViewSetup.m_ViewSetup3D.m_ViewExtents = Maths::C_Rect(vViewExtentsMin, vViewExtentsMax);
	view.m_ViewSetup.m_ViewSetup3D.m_CameraToWorld = mCamera;

	Display::Device()->SetupView(&view.m_View, view.m_ViewSetup, setup.m_iWidth, setup.m_iHeight);

	// override projection matrices
	view.m_View.m_ViewSettings3D.m_CameraToDevice = mProj;
	view.m_View.m_ViewSettings3D.m_WorldToDevice = mViewProj;

	Maths::Store(view.m_View.m_ViewSettings3D.m_CameraToDevice, &view.m_View.m_ViewSettings3D.m_ProjMatrix);
	Maths::Store(view.m_View.m_ViewSettings3D.m_WorldToDevice, &view.m_View.m_ViewSettings3D.m_ViewProjMatrix);

	const Maths::HF32 vWindowWidth = view.m_ViewSetup.m_WindowCoords.Size().X() / view.m_ViewSetup.GetDeviceCoords().Size().X();
	view.m_View.m_ViewFrustum.Setup(view.m_View.m_ViewSettings3D.m_CameraToDevice, view.m_View.m_ViewSettings3D.m_WorldToDevice, vWindowWidth);
}

void StableCascadedShadowMapSetup(C_ShadowMapView& view, const F32 fNearSplit, const F32 fFarSplit, const C_ShadowMapSetup& setup)
{
	// calculate minimal enclosing sphere of this part of the frustum
	const Display::C_ViewSettings3D& viewSettings3D = setup.m_pView->m_ViewSettings3D;

	const Maths::C_Rect		vFrustumExtents		= viewSettings3D.GetViewExtents();
	const F32				fFrustumPerspDist	= viewSettings3D.GetPerspDist();

	const Maths::C_Sphere	vSphere		= GetMinimalEnclosingSphere(vFrustumExtents, fFrustumPerspDist, fNearSplit, fFarSplit);
	const Maths::HVec3		vCentre		= vSphere.Centre() * viewSettings3D.GetCameraToWorld();
	const Maths::HF32		vRadius		= vSphere.Radius();

	const Maths::HVec2 vViewExtentsMin(-vRadius, -vRadius);
	const Maths::HVec2 vViewExtentsMax(+vRadius, +vRadius);
	const Maths::C_Rect vViewExtents(vViewExtentsMin, vViewExtentsMax);

	// construct matrices
	F32 fNearClip = setup.m_fNearClip;
	F32 fFarClip = setup.m_fFarClip;
	if (Display::Device()->GetDepthFlip())
	{
		fNearClip = setup.m_fFarClip;
		fFarClip = setup.m_fNearClip;
	}

	Maths::HMat43	mView		= ShadowMatrixLookAt(vCentre - setup.m_vDirection, vCentre);
	Maths::HMat44	mProj		= ShadowMatrixOrtho(vViewExtents, fNearClip, fFarClip);
	Maths::HMat43	mCamera		= mView.InverseRotTrans();
	Maths::HMat44	mViewProj	= Maths::HMat44(mView) * mProj;
	Maths::HMat44	mRound		= ShadowMatrixRound(mViewProj, (F32)setup.m_iWidth, (F32)setup.m_iHeight);
					mProj		= mProj * mRound;
					mViewProj	= mViewProj * mRound;
	
	// setup view
	view.m_ViewSetup.m_ViewSetup3D.m_FieldOfView = 0.0f;
	view.m_ViewSetup.m_ViewSetup3D.m_AspectRatio = setup.m_iWidth / (F32) setup.m_iHeight;
	view.m_ViewSetup.m_ViewSetup3D.m_PerspDist = 0.0f;
	view.m_ViewSetup.m_ViewSetup3D.m_NearClip = setup.m_fNearClip;
	view.m_ViewSetup.m_ViewSetup3D.m_FarClip = setup.m_fFarClip;
	view.m_ViewSetup.m_ViewSetup3D.m_ViewExtents = vViewExtents;
	view.m_ViewSetup.m_ViewSetup3D.m_CameraToWorld = mCamera;

	Display::Device()->SetupView(&view.m_View, view.m_ViewSetup, setup.m_iWidth, setup.m_iHeight);

	// override projection matrices
	view.m_View.m_ViewSettings3D.m_CameraToDevice = mProj;
	view.m_View.m_ViewSettings3D.m_WorldToDevice = mViewProj;

	Maths::Store(view.m_View.m_ViewSettings3D.m_CameraToDevice, &view.m_View.m_ViewSettings3D.m_ProjMatrix);
	Maths::Store(view.m_View.m_ViewSettings3D.m_WorldToDevice, &view.m_View.m_ViewSettings3D.m_ViewProjMatrix);

	const Maths::HF32 vWindowWidth = view.m_ViewSetup.m_WindowCoords.Size().X() / view.m_ViewSetup.GetDeviceCoords().Size().X();
	view.m_View.m_ViewFrustum.Setup(view.m_View.m_ViewSettings3D.m_CameraToDevice, view.m_View.m_ViewSettings3D.m_WorldToDevice, vWindowWidth);
}

void StableCascadedShadowMapSetup(C_ShadowMapView& view, const Maths::C_Rect& vFrustumExtents, const F32 fNearSplit, const F32 fFarSplit, const C_ShadowMapSetup& setup)
{
	// calculate minimal enclosing sphere of this part of the frustum
	const Display::C_ViewSettings3D& viewSettings3D = setup.m_pView->m_ViewSettings3D;

	const F32				fFrustumPerspDist	= viewSettings3D.GetPerspDist();

	const Maths::C_Sphere	vSphere		= GetMinimalEnclosingSphere(vFrustumExtents, fFrustumPerspDist, fNearSplit, fFarSplit);
	const Maths::HVec3		vCentre		= vSphere.Centre() * viewSettings3D.GetCameraToWorld();
	const Maths::HF32		vRadius		= vSphere.Radius();

	const Maths::HVec2 vViewExtentsMin(-vRadius, -vRadius);
	const Maths::HVec2 vViewExtentsMax(+vRadius, +vRadius);
	const Maths::C_Rect vViewExtents(vViewExtentsMin, vViewExtentsMax);

	// construct matrices
	F32 fNearClip = setup.m_fNearClip;
	F32 fFarClip = setup.m_fFarClip;
	if (Display::Device()->GetDepthFlip())
	{
		fNearClip = setup.m_fFarClip;
		fFarClip = setup.m_fNearClip;
	}

	Maths::HMat43	mView		= ShadowMatrixLookAt(vCentre - setup.m_vDirection, vCentre);
	Maths::HMat44	mProj		= ShadowMatrixOrtho(vViewExtents, fNearClip, fFarClip);
	Maths::HMat43	mCamera		= mView.InverseRotTrans();
	Maths::HMat44	mViewProj	= Maths::HMat44(mView) * mProj;
	Maths::HMat44	mRound		= ShadowMatrixRound(mViewProj, (F32)setup.m_iWidth, (F32)setup.m_iHeight);
					mProj		= mProj * mRound;
					mViewProj	= mViewProj * mRound;
	
	// setup view
	view.m_ViewSetup.m_ViewSetup3D.m_FieldOfView = 0.0f;
	view.m_ViewSetup.m_ViewSetup3D.m_AspectRatio = setup.m_iWidth / (F32) setup.m_iHeight;
	view.m_ViewSetup.m_ViewSetup3D.m_PerspDist = 0.0f;
	view.m_ViewSetup.m_ViewSetup3D.m_NearClip = setup.m_fNearClip;
	view.m_ViewSetup.m_ViewSetup3D.m_FarClip = setup.m_fFarClip;
	view.m_ViewSetup.m_ViewSetup3D.m_ViewExtents = vViewExtents;
	view.m_ViewSetup.m_ViewSetup3D.m_CameraToWorld = mCamera;

	Display::Device()->SetupView(&view.m_View, view.m_ViewSetup, setup.m_iWidth, setup.m_iHeight);

	// override projection matrices
	view.m_View.m_ViewSettings3D.m_CameraToDevice = mProj;
	view.m_View.m_ViewSettings3D.m_WorldToDevice = mViewProj;

	Maths::Store(view.m_View.m_ViewSettings3D.m_CameraToDevice, &view.m_View.m_ViewSettings3D.m_ProjMatrix);
	Maths::Store(view.m_View.m_ViewSettings3D.m_WorldToDevice, &view.m_View.m_ViewSettings3D.m_ViewProjMatrix);

	const Maths::HF32 vWindowWidth = view.m_ViewSetup.m_WindowCoords.Size().X() / view.m_ViewSetup.GetDeviceCoords().Size().X();
	view.m_View.m_ViewFrustum.Setup(view.m_View.m_ViewSettings3D.m_CameraToDevice, view.m_View.m_ViewSettings3D.m_WorldToDevice, vWindowWidth);
}

}; // namespace Effect
