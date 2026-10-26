namespace EditorLib
{
namespace CollisionHelpers
{

//Moller Trumbore triangle-intersection.
inline bool IntersectTriangleRay(
	const Maths::HVec3& p0, 
	const Maths::HVec3& p1, 
	const Maths::HVec3& p2, 
	const Maths::HVec3& origin, 
	const Maths::HVec3& dir, 
	Maths::HVec3& res)
{
	F32 u, v, t;

	Maths::HVec3 p0p1(p1-p0), p0p2(p2-p0);

	Maths::HVec3 pVec = dir ^ p0p2;

	F32 det = p0p1 | pVec;
	if (fabsf(det) < Maths::Constants<F32>::Epsilon())
		return false;

	F32 invdet = 1.0f/det;

	Maths::HVec3 tVec(origin - p0);
	u = (tVec|pVec)*invdet;

	if (u<0.f || u>1.f)
		return false;

	Maths::HVec3 qVec = tVec ^ p0p1;
	v = (dir|qVec)*invdet;
	if (v<0.f || u+v>1.f)
		return false;

	t = (p0p2|qVec)*invdet;

	if (t<0.f)
		return false;

	res	= origin + dir * t;
	return true;
}

}

}