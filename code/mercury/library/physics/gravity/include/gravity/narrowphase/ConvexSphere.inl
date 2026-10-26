#ifndef CONVEXSPHERE_INL
#define CONVEXSPHERE_INL

namespace Collision
{
	//Called by the particle system collide with the dynamic objects
	bool CollideConvexSphere(const C_ConvexHull& convex, const C_Sphere& sphere, Maths::HVec3& contact, Maths::HF32& pDepth, Maths::C_Normal& normal)
	{
		/*using namespace Maths;
		const HVec3 dir(sphere.Centre() - convex.Centre());
		const HVec3 p = convex.Support(dir);
		const HVec3 v(sphere.Centre() - p);
		const HF32 vLength(v.Length());
		if(vLength.GT(sphere.Radius()))
			return false;

		contact = p;
		pDepth = sphere.Radius() - vLength;
		normal = (v/vLength);
		return true;*/
		using namespace Maths;

		Collision::C_SpuConvexHull spuConvex(convex, true);

		const HVec3 centre=convex.Centre();
		const HVec3 sCentre = sphere.Centre();
		const HMat33& mat(convex.Orientation());
		HVec3 contact1(UNDEFINED);
		HVec3 normal1(UNDEFINED);
		bool collide=false;
		HVec3 end(centre);

		const U32 numTriangles = spuConvex.GetNumTriangles();
		for(U32 i=0; i<numTriangles; ++i)
		{
			Collision::C_ConvexTriangle* tri = &spuConvex.GetTriangle(i);
			const HVec3 a(spuConvex.GetLocalVertex(tri->m_VertexA));
			const HVec3 b(spuConvex.GetLocalVertex(tri->m_VertexB));
			const HVec3 c(spuConvex.GetLocalVertex(tri->m_VertexC));

			//if(IntersectTriangleSegment(a, b, c, start, end, contact))
			const Maths::Scalar_Bool bRes(IntersectTriangleSegmentSB(a, b, c, sCentre, centre, contact1));
			{
				end=Maths::Select(bRes, contact1, end);
				normal1=Maths::Select(bRes, Maths::Load<Maths::HVec3>(&tri->m_Normal), normal1);
				collide = collide || bool(bRes);
			}
		}
		
		if(collide)
		{
			contact = end;
			normal = -normal1;
			const HVec3 v(sCentre - end);
			pDepth = sphere.Radius() - v.Length();
			return true;
		}
		return false;

	}
}

#endif