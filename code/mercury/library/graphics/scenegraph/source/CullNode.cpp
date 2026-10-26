#include "stdafx.h"
#include "SceneGraph/CullNode.h"

#include "SceneGraph/View.h"

#include "RenderingData/RenderingData.h"

namespace SceneGraph
{

// C_BoundsCull

C_BoundsCull::C_BoundsCull() : m_CullFlags(0)
{

}

// data driven cull
S32 C_BoundsCull::Cull(const C_View& view) const
{
	S32 result = 0;
	
	if(m_CullFlags & E_BoundsCull_FrustumSphere)
		result = CullSphere(view);
	else if(m_CullFlags & E_BoundsCull_FrustumAxisBox)
		result = CullAxisBox(view);


	return result;
}

S32 C_BoundsCull::CullSphere(const C_View& view) const
{
	Maths::HVec4 vCentre(Maths::Load<Maths::HVec4>(&centre));
	Maths::HF32	radius(size.w);

	S32 result = view.GetViewFrustum().CullSphere(vCentre, radius);
	return result;
}

S32 C_BoundsCull::CullSphereDistance(const C_View& view, const F32 drawDistanceScale) const
{
	Maths::HVec4 vCentre(Maths::Load<Maths::HVec4>(&centre));
	Maths::HF32 farClip(view.GetViewData().farClip);
	Maths::HF32	radius(size.w);
	Maths::HF32	farDistance(clip.y);
	Maths::HF32 scale(drawDistanceScale);

	farDistance = Maths::Min(farDistance * scale, farClip + radius);
	S32 result = view.GetViewFrustum().CullSphere(vCentre, radius, farDistance);
	return result;
}

// cull bounds with no distance culling
S32 C_BoundsCull::CullAxisBox(const C_View& view) const
{
	Maths::HVec4 vCentre(Maths::Load<Maths::HVec4>(&centre));
	Maths::HVec3 vSize(Maths::Load<Maths::HVec4>(&size));

	S32 result = view.GetViewFrustum().CullAxisBox(vCentre, vSize);
	return result;
}

// cull bounds with distance culling
S32 C_BoundsCull::CullAxisBoxDistance(const C_View& view) const
{
	Maths::HVec4 vCentre(Maths::Load<Maths::HVec4>(&centre));
	Maths::HVec4 vSize(Maths::Load<Maths::HVec4>(&size));
	Maths::HVec4 vClip(Maths::Load<Maths::HVec4>(&clip));

	S32 result(view.GetViewFrustum().CullAxisBox(vCentre, vSize, vClip));
	return result;
}

} // namespace SceneGraph

