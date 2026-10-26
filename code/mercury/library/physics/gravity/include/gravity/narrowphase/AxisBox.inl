#ifndef GRAVITY_AXIS_BOX_INL
#define GRAVITY_AXIS_BOX_INL


namespace Collision
{

	NOALIAS FORCE_INLINE Maths::Scalar_Bool C_AxisBoxMinMax::Contains(const Maths::HVec3 PCREF point) const __restrict
	{
		//return point.All_GE(m_min) && point.All_LE(m_max);
		const Maths::Vector3 p(point);
		const Maths::Vector3 _min(m_min);
		const Maths::Vector3 _max(m_max);
		const Maths::VBool vResult(Maths::AllTrue3(((p >= _min) & (p < _max))));
		return (Maths::Scalar_Bool&) vResult;
	}

	//NOALIAS __forceinline bool C_CRAxisBox::Intersects(const C_CRAxisBox& other) const __restrict
	//{
	//	/*Maths::HF32 absx = (centre.X() - other.centre.X()).Abs();
	//	Maths::HF32 absy = (centre.Y() - other.centre.Y()).Abs();
	//	Maths::HF32 absz = (centre.Z() - other.centre.Z()).Abs();

	//	return absx.LE(extents.X() + other.extents.X()) &
	//		absy.LE(extents.Y() + other.extents.Y()) &&
	//		absz.LE(extents.Z() + other.extents.Z());*/

	//	const Maths::HVec3 absDif(Maths::Abs(centre - other.centre));
	//	const Maths::HVec3 extSum((Maths::HVec3)(extents + other.extents));
	//	return absDif.All_LE(extSum) != 0;
	//}

	NOALIAS __forceinline bool C_CRAxisBox::IntersectsInAxisBox(const C_CRAxisBox& other,
		const C_AxisBoxMinMax& enclosingBox) const __restrict
	{
		//const Maths::VF32 half(Maths::Const<Maths::VF32>::Half());
		const Maths::Vector3 _min(enclosingBox.m_min);
		const Maths::Vector3 _max(enclosingBox.m_max);
		const Maths::Vector3 p(ClosestPoint(other.centre));
		//const Maths::Vector3 closestA(ClosestPoint(other.centre));
		//const Maths::Vector3 closestB(other.ClosestPoint(centre));
		////DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, Maths::RGBA(255, 0, 0));
		//const Maths::Vector3 p((closestA + closestB)*half);
		const Maths::Vector3 absDif(Maths::Abs(centre - other.centre));
		const Maths::Vector3 extSum((Maths::HVec3)(extents + other.extents));
		return Maths::AllTrue3((absDif <= extSum) & ((p >= _min) & (p < _max)));
	}

	NOALIAS __forceinline bool C_CRAxisBox::IntersectsSphereInBox(const Maths::HVec3 PCREF c, const Maths::HF32 PCREF radius,
			const C_AxisBoxMinMax& enclosingBox) const
	{
		using namespace Maths;
		const Vector3 sphereCentre(c);
		const Vector3 p(ClosestPoint(c));
		const VF32 rad(radius);
		const VF32 radSq(rad * rad);
		const Maths::Vector3 _min(enclosingBox.m_min);
		const Maths::Vector3 _max(enclosingBox.m_max);
		const Maths::Vector3 dif(p - sphereCentre);
		const Maths::VF32 dis(dif | dif);
		return dis <= radSq & Maths::AllTrue3((p >= _min) & (p < _max));
	}


	NOALIAS __forceinline bool C_AxisBoxMinMax::Contains(const C_CRAxisBox& innerBox) const __restrict 
	{
		using namespace Maths;
		const Vector3 c_min((HVec3)(innerBox.centre - m_min));
		const Vector3 c_max((HVec3)(m_max - innerBox.centre));
		const Vector3 extents(innerBox.extents);

		//return (c_min.All_GE(innerBox.extents) & c_max.All_GE(innerBox.extents)) != 0;
		return Maths::AllTrue3(c_min >= extents & c_max >= extents);
	}

	NOALIAS __forceinline bool C_AxisBoxMinMax::Intersects(const C_CRAxisBox& other) const __restrict
	{
		using namespace Maths;
		/*const HVec3 otherMax(other.centre + other.extents); 
		const HVec3 otherMin(other.centre - other.extents);
		return (otherMax.Any_LT(m_min) || m_max.Any_LT(otherMin)) == 0;*/

		const Vector3 otherMax((HVec3)(other.centre + other.extents)); 
		const Vector3 otherMin((HVec3)(other.centre - other.extents));
		const Vector3 _min(m_min);
		const Vector3 _max(m_max);
		return AllTrue3(otherMax >= _min & otherMin <= _max);
	}

	NOALIAS __forceinline bool C_AxisBoxMinMax::IntersectSphere(const Maths::HVec3 PCREF c, const Maths::HF32 PCREF radius) const
	{
		const Maths::HVec3 closest(Maths::Max(m_min, Maths::Min(m_max, c)));
		const Maths::HF32 radSq(radius * radius);
		const Maths::HVec3 dif(c - closest);
		const Maths::HF32 dis(dif | dif);
		return dis.LE(radSq);
	}

}

#endif
