#include "stdafx.h"
#include "Dynamic/Light/Util.h"
#include "Dynamic/Node.h"

namespace DynamicEffect
{
namespace Light
{

bool GetNodeFromDirection(C_Node* pNode, const Maths::HVec3& vWorldDirection)
{
	const Display::C_ViewSettings3D& viewSettings3d = Display::Device()->GetViewSettings3D();
	const Maths::HMat43 mView = viewSettings3d.GetWorldToCamera();
	const Maths::HMat43 mCamera = viewSettings3d.GetCameraToWorld();
	const Maths::HF32 vFarClip(viewSettings3d.GetFarClip() - 0.001f);
	const Maths::HVec3 vViewDirection = vWorldDirection * Maths::HMat33(mView);

	Maths::HVec3 vWorldPosition(vViewDirection);
	vWorldPosition = vWorldPosition * vFarClip;
	vWorldPosition = vWorldPosition * mCamera;

	Maths::HMat33 mWorldRotation = Maths::HMat33(Maths::AxisZ(Maths::C_Normal(vWorldDirection)));
	Maths::C_Quaternion qWorldRotation(mWorldRotation);

	pNode->SetPosition(vWorldPosition);
	pNode->SetRotation(qWorldRotation);

	return (bool) Maths::LT(vViewDirection.Z(), Maths::Constants<Maths::HF32>::Epsilon());
}

}; // namespace Light
}; // namespace DynamicEffect
