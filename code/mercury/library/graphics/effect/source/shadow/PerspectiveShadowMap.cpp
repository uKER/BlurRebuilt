#include "stdafx.h"
#include "Effect/Shadow/PerspectiveShadowMap.h"

namespace Effect
{

Debug::Float g_debugTrapezoidalShadowMapDelta			("Graphics.Effect.Shadow.Trapezoidal.Delta",				0.52f,		Debug::Range(0.0f, 1.0f));
Debug::Float g_debugTrapezoidalShadowMapXi				("Graphics.Effect.Shadow.Trapezoidal.Xi",				-0.60f,		Debug::Range(-1.0f, 1.0f));
Debug::Float g_debugTrapezoidalShadowMapLightNearOffset	("Graphics.Effect.Shadow.Trapezoidal.LightNearOffset",	-10.0f,		Debug::Range(-10.0f, 10.0f));
Debug::Float g_debugTrapezoidalShadowMapLightFarOffset	("Graphics.Effect.Shadow.Trapezoidal.LightFarOffset",	0.0f,		Debug::Range(0.0f, 200.0f));

void TrapezoidalShadowMapSetup(C_ShadowMapView& view, const F32 fNearSplit, const F32 fFarSplit, const C_ShadowMapSetup& setup)
{
	const Maths::HF32	vZero		= Maths::Constants<Maths::HF32>::Zero();
	const Maths::HF32	vQuarter	= Maths::Constants<Maths::HF32>::Quarter();
	const Maths::HF32	vHalf		= Maths::Constants<Maths::HF32>::Half();
	const Maths::HF32	vOne		= Maths::Constants<Maths::HF32>::One();
	const Maths::HF32	vTwo		= Maths::Constants<Maths::HF32>::Two();

	const Display::C_ViewSettings3D& viewSettings3D = setup.m_pView->GetViewSettings3D();

	const Maths::HMat43	mEyeView	= viewSettings3D.GetWorldToCamera();
	const Maths::HMat44	mEyeProj	= viewSettings3D.GetCameraToDevice();

	// get light basis
	const Maths::HVec3	vAZ			= Maths::Normalise(setup.m_vDirection * Maths::HMat33(mEyeView));
	const Maths::HVec3	vAX			= Maths::Normalise(Maths::HVec3(vZero, vZero, -vOne) ^ vAZ);
	const Maths::HVec3	vAY			= Maths::Normalise(vAZ ^ vAX);

	Maths::HMat33	mLightBasis(vAX, vAY, vAZ);
					mLightBasis.Transpose();

	// get view frustum
	Maths::HVec3 vFrustumPoints[8];
	ShadowFrustum(viewSettings3D.GetViewExtents(), viewSettings3D.GetPerspDist(), fNearSplit, fFarSplit, vFrustumPoints);

	// rotate into light space
	Maths::HVec3 vMin(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Maths::HVec3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for(U32 pointNo = 0; pointNo < 8; ++pointNo)
	{
		vFrustumPoints[pointNo] = vFrustumPoints[pointNo] * mLightBasis;

		vMin = Maths::Min(vMin, vFrustumPoints[pointNo]);
		vMax = Maths::Max(vMax, vFrustumPoints[pointNo]);
	}

	const Maths::HVec3 vCameraPosition = setup.m_vPosition * mEyeView * mLightBasis;
	Maths::HF32 vLightMinZ = Maths::Min(vMin.Z(), vCameraPosition.Z() + g_debugTrapezoidalShadowMapLightNearOffset);
	Maths::HF32 vLightMaxZ = Maths::Max(vMax.Z(), vCameraPosition.Z() + g_debugTrapezoidalShadowMapLightFarOffset);

	const Maths::HVec3 vLightTranslate(vZero, vZero, vOne - vLightMinZ);
	if (vMin.Z() < vZero)
	{
		const Maths::HMat43 mLightTranslate(Maths::HMat33(), vLightTranslate);
		vLightMaxZ = -vMin.Z() + vMax.Z() + vOne;
		vLightMinZ = vOne;
		for (U32 pointNo = 0; pointNo < 8; ++pointNo)
		{
			vFrustumPoints[pointNo] = vFrustumPoints[pointNo] * mLightTranslate;
			vMin.Z( Maths::Min(vMin.Z(), vFrustumPoints[pointNo].Z()) );
			vMax.Z( Maths::Min(vMax.Z(), vFrustumPoints[pointNo].Z()) );
		}
	}
	const Maths::HF32 vMinZ = vMin.Z();
	const Maths::HF32 vMaxZ = vMax.Z();

	// build light space matrices
	const Maths::HVec4 vLightProjAX(vTwo * (vMax.X() - vOne).Rcp(), vZero, vZero, vZero);
	const Maths::HVec4 vLightProjAY(vZero, vTwo * (vMax.Y() - vMin.Y()).Rcp(), vZero, vZero);
	const Maths::HVec4 vLightProjAZ(vZero, vZero, vOne * (vLightMaxZ - vLightMinZ).Rcp(), vZero);
	const Maths::HVec4 vLightProjAW((vOne + vMax.X()) * (vOne - vMax.X()).Rcp(), (vMax.Y() + vMin.Y()) * (vMin.Y() - vMax.Y()).Rcp(), vLightMinZ * (vLightMinZ - vLightMaxZ).Rcp(), vOne);
	
	const Maths::HMat43 mLightView(mLightBasis, vLightTranslate);
	const Maths::HMat44 mLightProj(vLightProjAX, vLightProjAY, vLightProjAZ, vLightProjAW);;

	for (U32 pointNo = 0; pointNo < 8; ++pointNo)
	{
		vFrustumPoints[pointNo] = (Maths::HVec3) Maths::HVec4(vFrustumPoints[pointNo]) * mLightProj;
	}

	const Maths::HVec2 vFarCentre	= Maths::HVec2(vFrustumPoints[0] + vFrustumPoints[1] + vFrustumPoints[2] + vFrustumPoints[3]) * vQuarter;
	const Maths::HVec2 vNearCentre	= Maths::HVec2(vFrustumPoints[4] + vFrustumPoints[5] + vFrustumPoints[6] + vFrustumPoints[7]) * vQuarter;
	const Maths::HVec2 vCentreOrig	= (vNearCentre + vFarCentre) * vHalf;

	const Maths::HVec4 vTranslateCentreAW(-vCentreOrig.X(), -vCentreOrig.Y(), vZero, vOne);
	Maths::HMat44 mTranslateCentre;
	mTranslateCentre.AW(vTranslateCentreAW);

	const Maths::HVec2 vLength				= vFarCentre - vCentreOrig;
	const Maths::HF32 vHalfCentreLength		= vLength.Length();
	const Maths::HF32 vHalfCentreRcpLength	= vHalfCentreLength.Rcp();

	const Maths::HVec2 vCosSinTheta			= vLength * vHalfCentreRcpLength;

	Maths::HMat44 mRotCentre;
	mRotCentre.AX( Maths::HVec4(vCosSinTheta.X(), -vCosSinTheta.Y(), vZero, vZero) );
	mRotCentre.AY( Maths::HVec4(vCosSinTheta.Y(), vCosSinTheta.X(), vZero, vZero) );

	Maths::HMat44 mTrapezoidSpace = mTranslateCentre * mRotCentre;
	vMin = Maths::HVec3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	vMax = Maths::HVec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (U32 pointNo = 0; pointNo < 8; ++pointNo)
	{
		vFrustumPoints[pointNo] = vFrustumPoints[pointNo] * Maths::HMat43(mTrapezoidSpace);
		vMin = Maths::Min(vMin, vFrustumPoints[pointNo]);		
		vMax = Maths::Max(vMax, vFrustumPoints[pointNo]);
	}

	const Maths::HVec4 vXScale = Maths::HVec4(Maths::Max( vMax.X().Abs(), vMin.X().Abs() ).Rcp(), vZero, vZero, vZero);
	const Maths::HVec4 vYScale = Maths::HVec4(vZero, Maths::Max( vMax.Y().Abs(), vMin.Y().Abs() ).Rcp(), vZero, vZero);
	Maths::HMat44 mScaleCentre;
	mScaleCentre.AX(vXScale);
	mScaleCentre.AY(vYScale);

	mTrapezoidSpace = mTrapezoidSpace * mScaleCentre;

	vMin.X(vMin.X() * vXScale.X());
	vMin.Y(vMin.Y() * vYScale.Y());
	vMax.X(vMax.X() * vXScale.X());
	vMax.Y(vMax.Y() * vYScale.Y());

	const Maths::HF32 vLambda		= vMax.X() - vMin.X();
	const Maths::HF32 vDeltaProj	= g_debugTrapezoidalShadowMapDelta * vLambda;
	const Maths::HF32 vXi			= (F32) g_debugTrapezoidalShadowMapXi;
	const Maths::HF32 vEta			= vLambda * vDeltaProj * (vOne + vXi) * (vLambda * (vOne - vXi) - vTwo * vDeltaProj).Rcp();

	const Maths::HVec2 vProjectionQ(vMax.X() + vEta, vZero);
	Maths::HF32 vMaxSlope = -FLT_MAX;
	Maths::HF32 vMinSlope = FLT_MAX;
	
	for (U32 pointNo = 0; pointNo < 8; ++pointNo)
	{
		const Maths::HVec2	v		= Maths::HVec2( vFrustumPoints[pointNo].X() * vXScale.X(), vFrustumPoints[pointNo].Y() * vYScale.Y() );
		const Maths::HF32	vXDist	= v.X() - vProjectionQ.X();
		
		const Maths::Scalar_Bool select0	= vXDist.Abs().LT(Maths::Constants<Maths::HF32>::Epsilon());
		const Maths::Scalar_Bool select1	= v.Y().Abs().LT(Maths::Constants<Maths::HF32>::Epsilon());
		
		if ( !(select0 || select1) )
		{
			vMaxSlope = Maths::Max(vMaxSlope, v.Y() * vXDist.Rcp());
			vMinSlope = Maths::Min(vMinSlope, v.Y() * vXDist.Rcp());
		}
	}

	const Maths::HF32 vXNear	= vEta;
	const Maths::HF32 vXFar		= vLambda + vEta;

	const Maths::HVec4 vQTranslateAX( -vOne, vZero, vZero, vZero );
	const Maths::HVec4 vQTranslateAW( vProjectionQ.X(), vZero, vZero, vOne );
	Maths::HMat44 mQTranslate;
	mQTranslate.AX(vQTranslateAX);
	mQTranslate.AW(vQTranslateAW);

	mTrapezoidSpace = mTrapezoidSpace * mQTranslate;

	const Maths::HF32 vShearAmt = (vMinSlope.Abs() - vMaxSlope) * vHalf;
	vMaxSlope = vMaxSlope + vShearAmt;

	const Maths::HVec4 vTrapezoidShearAX(vOne, vShearAmt, vZero, vZero);
	Maths::HMat44 mTrapezoidShear;
	mTrapezoidShear.AX(vTrapezoidShearAX);

	mTrapezoidSpace = mTrapezoidSpace * mTrapezoidShear;

	const Maths::HF32 vZAspect = (vMaxZ - vMinZ) * (vMax.Y() - vMin.Y()).Rcp();

	const Maths::HVec4 vTrapezoidProjAX(vXFar * (vXFar - vXNear).Rcp(), vZero, vZero, vOne);
	const Maths::HVec4 vTrapezoidProjAY(vZero, vMaxSlope.Rcp(), vZero, vZero);
	const Maths::HVec4 vTrapezoidProjAZ(vZero, vZero, (vZAspect * vMaxSlope).Rcp(), vZero);
	const Maths::HVec4 vTrapezoidProjAW(-vXNear * vXFar * (vXFar - vXNear).Rcp(), vZero, vZero, vZero);
	const Maths::HMat44 mTrapezoidProj(vTrapezoidProjAX, vTrapezoidProjAY, vTrapezoidProjAZ, vTrapezoidProjAW);

	mTrapezoidSpace = mTrapezoidSpace * mTrapezoidProj;	

	const Maths::HVec4 vBiasedScaleAX(vTwo, vZero, vZero, vZero);
	const Maths::HVec4 vBiasedScaleAW(-vOne, vZero, vZero, vOne);
	Maths::HMat44 mBiasedScale;
	mBiasedScale.AX(vBiasedScaleAX);
	mBiasedScale.AW(vBiasedScaleAW);

	mTrapezoidSpace = mTrapezoidSpace * mBiasedScale;

	// finalise matrices
	const Maths::HMat43 mView	= mEyeView * mLightView;
	const Maths::HMat43 mCamera	= mView.InverseRotTrans();
	const Maths::HMat44	mProj	= mLightProj * mTrapezoidSpace;

	// setup view
	view.m_ViewSetup.m_ViewSetup3D.m_FieldOfView = 0.0f;
	view.m_ViewSetup.m_ViewSetup3D.m_AspectRatio = setup.m_iWidth / (F32) setup.m_iHeight;
	view.m_ViewSetup.m_ViewSetup3D.m_PerspDist = 0.0f;
	view.m_ViewSetup.m_ViewSetup3D.m_NearClip = setup.m_fNearClip;
	view.m_ViewSetup.m_ViewSetup3D.m_FarClip = setup.m_fFarClip;
	view.m_ViewSetup.m_ViewSetup3D.m_ViewExtents = setup.m_vViewExtents;
	view.m_ViewSetup.m_ViewSetup3D.m_CameraToWorld = mCamera;

	Display::Device()->SetupView(&view.m_View, view.m_ViewSetup, setup.m_iWidth, setup.m_iHeight);

	// override matrices
	view.m_View.m_ViewSettings3D.m_WorldToCamera	= mView;
	view.m_View.m_ViewSettings3D.m_CameraToDevice	= mProj;
	view.m_View.m_ViewSettings3D.m_CameraToWorld	= mCamera;
	view.m_View.m_ViewSettings3D.m_WorldToDevice	= Maths::HMat44(mView) * mProj;

	Maths::Store(Maths::HMat44(mView), &view.m_View.m_ViewSettings3D.m_ViewMatrix);
	Maths::Store(mProj, &view.m_View.m_ViewSettings3D.m_ProjMatrix);
	Maths::Store(Maths::HMat44(mView) * mProj, &view.m_View.m_ViewSettings3D.m_ViewProjMatrix);
	Maths::Store(Maths::HVec4(mCamera.AW()), &view.m_View.m_ViewSettings3D.m_ViewPosition);

	const Maths::HF32 vWindowWidth = view.m_ViewSetup.m_WindowCoords.Size().X() / view.m_ViewSetup.GetDeviceCoords().Size().X();
	view.m_View.m_ViewFrustum.Setup(view.m_View.m_ViewSettings3D.m_CameraToDevice, view.m_View.m_ViewSettings3D.m_WorldToDevice, vWindowWidth);
}

}; // namespace Effect
