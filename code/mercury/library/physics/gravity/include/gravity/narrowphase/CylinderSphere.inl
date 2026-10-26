#ifndef CYLINDERSPHERE_INL
#define CYLINDERSPHERE_INL

namespace Collision
{
//Called by the particle system collide with the dynamic objects
bool inline CollideCylinderSphere(const C_Cylinder& cylinder, const C_Sphere& sphere, Maths::HVec3& contact, Maths::HF32& pDepth, Maths::C_Normal& normal)
{
	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());
	const HVec3 sCentre(sphere.Centre());
	const HVec3 cCentre(cylinder.Centre());
	const HVec3 axis(cylinder.CentreAxis());
	const HF32 cRadius(cylinder.Radius());
	const HF32 sRadius(sphere.Radius());
	const HF32 halfHeight(cylinder.HalfHeight());
	const HVec3 v(sCentre - cCentre);
	const HF32 vLength(v|axis);
	const HVec3 projP(cCentre + vLength * axis);
	const HVec3 u(sCentre - projP);
	const HF32 uLength(u.Length());
	const HVec3 dir(u/uLength);
	const HVec3 projC(projP + dir*cRadius);

	const HVec3 w(halfHeight*axis);
	const HVec3 top(cCentre + w);
	const HF32 distTop(top|axis);
	const HF32 sTopDist((projC|axis) - distTop);

	const HVec3 naxis(-axis);
	const HVec3 bottom(cCentre - w);
	const HF32 distBottom(bottom|naxis);
	const HF32 sBotDist((projC|naxis) - distBottom);

	const HF32 r1(sRadius + cRadius);

	/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(top, 0.5f, Maths::RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(top, "Top");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(bottom, 0.5f, RGBA(0, 255, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(bottom, "Bottom");

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(projP, 0.5f, RGBA(0, 255, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(projP, "P");

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(projC, 0.5f, RGBA(255, 255, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(projC, "C");*/

	if(uLength.GT(r1))
		return false;

	//if(sTopDist.LE(zero) && sBotDist.LE(zero) && uLength.LE(r1))
	if(sTopDist.LE(zero) && sBotDist.LE(zero))
	{
		contact = projC;
		normal = dir;
		const Scalar_Bool con(uLength.LT(cRadius));
		pDepth = Select(con, sRadius + uLength, sRadius - uLength);
		return true;
	}

	//const Scalar_Bool con1(sTopDist.GT(zero));
	//const Scalar_Bool con2(uLength.LE(cRadius));

	if(sTopDist.GT(zero) && sTopDist.LE(sRadius))
	//if(sTopDist.GT(zero) && sTopDist.LE(sRadius) && uLength.LE(r1))
	{
		const HVec3 w(sTopDist*axis);
		const HVec3 point(sCentre - w);

		const HVec3 m(point - top);
		const HF32 mLength(m.Length());
		if(mLength.LT(cRadius))
		{
			contact = point;
			normal = axis;
			pDepth = sRadius - sTopDist;
		}
		else
		{
			const HVec3 dir(m/mLength);
			contact = top + dir*cRadius;
			const HVec3 t(contact - sCentre);
			const HF32 tLength(t.Length());
			normal = t/tLength;
			pDepth = sRadius -tLength;

		}
		return true;
	}

	//if(sBotDist.GT(zero) && sBotDist.LE(sRadius) && uLength.LE(r1))
	if(sBotDist.GT(zero) && sBotDist.LE(sRadius))
	{
		const HVec3 w(sBotDist*axis);
		const HVec3 point(sCentre - w);

		const HVec3 m(point - bottom);
		const HF32 mLength(m.Length());
		if(mLength.LT(cRadius))
		{
			contact = point;
			normal = axis;
			pDepth = sRadius - sBotDist;
		}
		else
		{
			const HVec3 dir(m/mLength);
			contact = bottom + dir*cRadius;
			const HVec3 t(contact - sCentre);
			const HF32 tLength(t.Length());
			normal = t/tLength;
			pDepth = sRadius -tLength;

		}
		return true;
	}
	return false;
}

template <int MaxContacts>
bool CollideCylinderSphere(const C_Cylinder& cylinder, const C_Sphere& sphere, C_ContactPatch<MaxContacts>& patch, const U32 materialIndices)
{
	return false;
}
};




#endif