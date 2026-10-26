#include "stdafx.h"
#include "Effect/Shadow/ShadowMapView.h"

namespace Effect
{

void ShadowFrustum(const Maths::C_Rect& viewExtents, const F32 fPerspDist, const F32 fNearClip, const F32 fFarClip, Maths::HVec3* vPoints)
{
	const Maths::HVec3	vRect0(viewExtents.Min().X(), viewExtents.Min().Y(), fPerspDist);
	const Maths::HVec3	vRect1(viewExtents.Min().X(), viewExtents.Max().Y(), fPerspDist);
	const Maths::HVec3	vRect2(viewExtents.Max().X(), viewExtents.Max().Y(), fPerspDist);
	const Maths::HVec3	vRect3(viewExtents.Max().X(), viewExtents.Min().Y(), fPerspDist);
	
	vPoints[0] = vRect0 * fNearClip / fPerspDist;
	vPoints[1] = vRect1 * fNearClip / fPerspDist;
	vPoints[2] = vRect2 * fNearClip / fPerspDist;
	vPoints[3] = vRect3 * fNearClip / fPerspDist;
	vPoints[4] = vRect0 * fFarClip / fPerspDist;
	vPoints[5] = vRect1 * fFarClip / fPerspDist;
	vPoints[6] = vRect2 * fFarClip / fPerspDist;
	vPoints[7] = vRect3 * fFarClip / fPerspDist;
}

Maths::C_AxisBox ShadowFrustumAABB(const Maths::HVec3* vPoints)
{
	Maths::HVec3 vMin = vPoints[0];
	Maths::HVec3 vMax = vPoints[0];
	for (U32 i = 1; i < 8; ++i)
	{
		vMin = Maths::Min(vMin, vPoints[i]);
		vMax = Maths::Max(vMax, vPoints[i]);
	}
	return Maths::C_AxisBox(vMin, vMax);
}

Maths::C_AxisBox ShadowClipAABB(const Maths::C_AxisBox& box, const Maths::HMat44& mViewProj)
{
	Maths::HVec3 vPoints[8];
	box.GetVertexArray(vPoints);

	Maths::HVec4 vClipPoints[8];
	for (U32 i = 0; i < 8; ++i)
	{
		vClipPoints[i] = Maths::HVec4(vPoints[i]);
		vClipPoints[i].W( Maths::Constants<Maths::HF32>::One() );
		vClipPoints[i] = vClipPoints[i] * mViewProj;
		vClipPoints[i] = vClipPoints[i] * vClipPoints[i].W().Rcp();
	}

	Maths::HVec3 vMin = Maths::HVec3(vClipPoints[0]);
	Maths::HVec3 vMax = Maths::HVec3(vClipPoints[0]);
	for (U32 i = 1; i < 8; ++i)
	{
		vMin = Maths::Min(vMin, Maths::HVec3(vClipPoints[i]));
		vMax = Maths::Max(vMax, Maths::HVec3(vClipPoints[i]));
	}

	vMin.Z( Maths::Constants<Maths::HF32>::Zero() );

	return Maths::C_AxisBox(vMin, vMax);
}

Maths::HMat43 ShadowMatrixCamera(const Maths::HVec3& vPosition, const Maths::HVec3& vDirection)
{
	const Maths::HVec3	vUp = Maths::HVec3(0.0f, 1.0f, 0.0f);
	const Maths::HVec3	vAZ	= Maths::Normalise(vDirection);
	const Maths::HVec3	vAX	= Maths::Normalise(vUp ^ vAZ);
	const Maths::HVec3	vAY	= Maths::Normalise(vAZ ^ vAX);
	return Maths::HMat43(vAX, vAY, vAZ, vPosition);
}

Maths::HMat43 ShadowMatrixLookAt(const Maths::HVec3& vEye, const Maths::HVec3& vAt)
{
	const Maths::HVec3	vUp		= Maths::HVec3(0.0f, 1.0f, 0.0f);
	const Maths::HVec3	vAZ		= Maths::Normalise(vAt - vEye);
	const Maths::HVec3	vAX		= Maths::Normalise(vUp ^ vAZ);
	const Maths::HVec3	vAY		= Maths::Normalise(vAZ ^ vAX);
	const Maths::HMat33	mRot	= Maths::HMat33(vAX, vAY, vAZ);
	const Maths::HF32	vTX		= -(vAX | vEye);
	const Maths::HF32	vTY		= -(vAY | vEye);
	const Maths::HF32	vTZ		= -(vAZ | vEye);
	const Maths::HVec3	vT		= Maths::HVec3(vTX, vTY, vTZ);
	return Maths::HMat43(Maths::Transpose(mRot), vT);
}

Maths::HMat44 ShadowMatrixOrtho(const F32 fWidth, const F32 fHeight, const F32 fNear, const F32 fFar)
{
	const MathsStorage::MatrixRow44	matrix =	{	
													2.0f / fWidth,	0.0f,			0.0f,					0.0f,
													0.0f,			2.0f / fHeight,	0.0f,					0.0f,
													0.0f,			0.0f,			1.0f / (fFar - fNear),	0.0f,
													0.0f,			0.0f,			fNear / (fNear - fFar),	1.0f,
												};
	return Maths::Load<Maths::HMat44>(&matrix);
}

Maths::HMat44 ShadowMatrixOrtho(const Maths::C_Rect& vViewExtents, const F32 fNear, const F32 fFar)
{
	const F32 fWidth	= vViewExtents.Max().X() - vViewExtents.Min().X();
	const F32 fHeight	= vViewExtents.Max().Y() - vViewExtents.Min().Y();
	const F32 fX		= (vViewExtents.Min().X() + vViewExtents.Max().X()) / (vViewExtents.Min().X() - vViewExtents.Max().X());
	const F32 fY		= (vViewExtents.Min().Y() + vViewExtents.Max().Y()) / (vViewExtents.Min().Y() - vViewExtents.Max().Y());
	const MathsStorage::MatrixRow44	matrix =	{	
													2.0f / fWidth,	0.0f,			0.0f,					0.0f,
													0.0f,			2.0f / fHeight,	0.0f,					0.0f,
													0.0f,			0.0f,			1.0f / (fFar - fNear),	0.0f,
													fX,				fY,				fNear / (fNear - fFar),	1.0f,
												};
	return Maths::Load<Maths::HMat44>(&matrix);
}

Maths::HMat44 ShadowMatrixPerspective(const F32 fWidth, const F32 fHeight, const F32 fNear, const F32 fFar)
{
	const MathsStorage::MatrixRow44	matrix =	{	
													2.0f * fNear / fWidth,	0.0f,					0.0f,							0.0f,
													0.0f,					2.0f * fNear / fHeight,	0.0f,							0.0f,
													0.0f,					0.0f,					fFar / (fFar - fNear),			1.0f,
													0.0f,					0.0f,					fNear * fFar / (fNear - fFar),	0.0f,
												};
	return Maths::Load<Maths::HMat44>(&matrix);
}

Maths::HMat44 ShadowMatrixPerspectiveFieldOfView(const F32 fFieldOfView, const F32 fAspectRatio, const F32 fNear, const F32 fFar)
{
	const F32 fHeight = 1.0f / tanf(fFieldOfView * 0.5f);
	const F32 fWidth = fHeight / fAspectRatio;
	const MathsStorage::MatrixRow44	matrix =	{	
													fWidth,	0.0f,		0.0f,							0.0f,
													0.0f,	fHeight,	0.0f,							0.0f,
													0.0f,	0.0f,		fFar / (fFar - fNear),			1.0f,
													0.0f,	0.0f,		fNear * fFar / (fNear - fFar),	0.0f,
												};
	return Maths::Load<Maths::HMat44>(&matrix);
}

Maths::HMat44 ShadowMatrixRound(const Maths::HMat44& mViewProj, const F32 fWidth, const F32 fHeight)
{
	const Maths::HF32 vOne( Maths::Constants<Maths::HF32>::One() );
	const Maths::HF32 vZero( Maths::Constants<Maths::HF32>::Zero() );

	Maths::HVec4	vOrigin	= Maths::HVec4(vZero, vZero, vZero, vOne);
					vOrigin	= vOrigin * mViewProj;
					vOrigin = vOrigin * vOrigin.W().Rcp();

	Maths::HF32		vXO		= vOrigin.X() * fWidth * 0.5f;
	Maths::HF32		vYO		= vOrigin.Y() * fHeight * 0.5f;
	Maths::HF32		vX		= Maths::Floor(vXO);
	Maths::HF32		vY		= Maths::Floor(vYO);
					vX		= vX - vXO;
					vY		= vY - vYO;
					vX		= vX * (2.0f / fWidth);
					vY		= vY * (2.0f / fHeight);

	Maths::HVec4	vAX		= Maths::HVec4(vOne, vZero, vZero, vZero);
	Maths::HVec4	vAY		= Maths::HVec4(vZero, vOne, vZero, vZero);
	Maths::HVec4	vAZ		= Maths::HVec4(vZero, vZero, vOne, vZero);
	Maths::HVec4	vAW		= Maths::HVec4(vX, vY, vZero, vOne);
	
	return Maths::HMat44(vAX, vAY, vAZ, vAW);
}

Maths::HMat44 ShadowMatrixCrop(const Maths::HVec3& vMin, const Maths::HVec3& vMax)
{
	const Maths::HF32	vTwo	= Maths::Constants<Maths::HF32>::Two();
	const Maths::HF32	vOne	= Maths::Constants<Maths::HF32>::One();
	const Maths::HF32	vHalf	= Maths::Constants<Maths::HF32>::Half();
	const Maths::HF32	vZero	= Maths::Constants<Maths::HF32>::Zero();

	const Maths::HF32	vScaleX		= vTwo * (vMax.X() - vMin.X()).Rcp();
	const Maths::HF32	vScaleY		= vTwo * (vMax.Y() - vMin.Y()).Rcp();
	const Maths::HF32	vScaleZ		= vOne * (vMax.Z() - vMin.Z()).Rcp();
	const Maths::HF32	vOffsetX	= -vHalf * (vMax.X() + vMin.X()) * vScaleX;
	const Maths::HF32	vOffsetY	= -vHalf * (vMax.Y() + vMin.Y()) * vScaleY;
	const Maths::HF32	vOffsetZ	= -vMin.Z() * vScaleZ;

	const Maths::HVec4	vAX	= Maths::HVec4(vScaleX, vZero, vZero, vZero);
	const Maths::HVec4	vAY	= Maths::HVec4(vZero, vScaleY, vZero, vZero);
	const Maths::HVec4	vAZ	= Maths::HVec4(vZero, vZero, vScaleZ, vZero);
	const Maths::HVec4	vAW	= Maths::HVec4(vOffsetX, vOffsetY, vOffsetZ, vOne);

	return Maths::HMat44(vAX, vAY, vAZ, vAW);
}

Maths::HMat44 ShadowMatrixCrop(const Maths::HMat44& mViewProj, const Maths::HVec3* vPoints)
{
	const Maths::C_AxisBox worldBox = ShadowFrustumAABB(vPoints);
	const Maths::C_AxisBox clipBox = ShadowClipAABB(worldBox, mViewProj);

	return ShadowMatrixCrop(clipBox.Min(), clipBox.Max());
}

Maths::HMat44 ShadowMatrixCropRect(const Maths::HVec3& vMin, const Maths::HVec3& vMax)
{
	const Maths::HF32	vTwo	= Maths::Constants<Maths::HF32>::Two();
	const Maths::HF32	vOne	= Maths::Constants<Maths::HF32>::One();
	const Maths::HF32	vHalf	= Maths::Constants<Maths::HF32>::Half();
	const Maths::HF32	vZero	= Maths::Constants<Maths::HF32>::Zero();

	const Maths::HF32	vScaleX		= vTwo * (vMax.X() - vMin.X()).Rcp();
	const Maths::HF32	vScaleY		= vTwo * (vMax.Y() - vMin.Y()).Rcp();
	const Maths::HF32	vOffsetX	= -vHalf * (vMax.X() + vMin.X()) * vScaleX;
	const Maths::HF32	vOffsetY	= -vHalf * (vMax.Y() + vMin.Y()) * vScaleY;

	const Maths::HVec4	vAX	= Maths::HVec4(vScaleX, vZero, vZero, vZero);
	const Maths::HVec4	vAY	= Maths::HVec4(vZero, vScaleY, vZero, vZero);
	const Maths::HVec4	vAZ	= Maths::HVec4(vZero, vZero, vOne, vZero);
	const Maths::HVec4	vAW	= Maths::HVec4(vOffsetX, vOffsetY, vZero, vOne);

	return Maths::HMat44(vAX, vAY, vAZ, vAW);
}

Maths::HMat44 ShadowMatrixCropRect(const Maths::HMat44& mViewProj, const Maths::HVec3* vPoints)
{
	const Maths::C_AxisBox worldBox = ShadowFrustumAABB(vPoints);
	const Maths::C_AxisBox clipBox = ShadowClipAABB(worldBox, mViewProj);

	return ShadowMatrixCropRect(clipBox.Min(), clipBox.Max());
}

}; // namespace Effect
