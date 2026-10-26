#include "stdafx.h"
#include "Render/View/ViewRender.h"
#include "System/Debug/I_DebugRender.h"

namespace Render
{

void RenderViewFrustum(const Display::C_View* pView, const U32 iColour)
{
	const Maths::HMat44 mWorldToDevice = pView->GetViewSettings3D().GetWorldToDevice();
	const Maths::HMat44 mDeviceToWorld = mWorldToDevice.Inverse();

	Maths::HVec4 vNear0(-1.0f, -1.0f, 0.0f, 1.0f);
	Maths::HVec4 vNear1(-1.0f, +1.0f, 0.0f, 1.0f);
	Maths::HVec4 vNear2(+1.0f, +1.0f, 0.0f, 1.0f);
	Maths::HVec4 vNear3(+1.0f, -1.0f, 0.0f, 1.0f);
	Maths::HVec4 vFar0(-1.0f, -1.0f, 1.0f, 1.0f);
	Maths::HVec4 vFar1(-1.0f, +1.0f, 1.0f, 1.0f);
	Maths::HVec4 vFar2(+1.0f, +1.0f, 1.0f, 1.0f);
	Maths::HVec4 vFar3(+1.0f, -1.0f, 1.0f, 1.0f);

	vNear0 = vNear0 * mDeviceToWorld;
	vNear1 = vNear1 * mDeviceToWorld;
	vNear2 = vNear2 * mDeviceToWorld;
	vNear3 = vNear3 * mDeviceToWorld;
	vFar0 = vFar0 * mDeviceToWorld;
	vFar1 = vFar1 * mDeviceToWorld;
	vFar2 = vFar2 * mDeviceToWorld;
	vFar3 = vFar3 * mDeviceToWorld;

	vNear0 = vNear0 * vNear0.W().Rcp();
	vNear1 = vNear1 * vNear1.W().Rcp();
	vNear2 = vNear2 * vNear2.W().Rcp();
	vNear3 = vNear3 * vNear3.W().Rcp();
	vFar0 = vFar0 * vFar0.W().Rcp();
	vFar1 = vFar1 * vFar1.W().Rcp();
	vFar2 = vFar2 * vFar2.W().Rcp();
	vFar3 = vFar3 * vFar3.W().Rcp();

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(Maths::HVec3(vNear0), Maths::HVec3(vNear1), Maths::RGBA(iColour));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(Maths::HVec3(vNear1), Maths::HVec3(vNear2), Maths::RGBA(iColour));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(Maths::HVec3(vNear2), Maths::HVec3(vNear3), Maths::RGBA(iColour));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(Maths::HVec3(vNear3), Maths::HVec3(vNear0), Maths::RGBA(iColour));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(Maths::HVec3(vFar0), Maths::HVec3(vFar1), Maths::RGBA(iColour));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(Maths::HVec3(vFar1), Maths::HVec3(vFar2), Maths::RGBA(iColour));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(Maths::HVec3(vFar2), Maths::HVec3(vFar3), Maths::RGBA(iColour));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(Maths::HVec3(vFar3), Maths::HVec3(vFar0), Maths::RGBA(iColour));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(Maths::HVec3(vNear0), Maths::HVec3(vFar0), Maths::RGBA(iColour));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(Maths::HVec3(vNear1), Maths::HVec3(vFar1), Maths::RGBA(iColour));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(Maths::HVec3(vNear2), Maths::HVec3(vFar2), Maths::RGBA(iColour));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(Maths::HVec3(vNear3), Maths::HVec3(vFar3), Maths::RGBA(iColour));
}

void RenderViewPosition(const Display::C_View* pView, const U32 iColour)
{
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(pView->GetViewSettings3D().GetCameraToWorld().AW(), 0.1f, Maths::RGBA(iColour));
}

}; // namespace Render
