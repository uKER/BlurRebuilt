#include "stdafx.h"
#include "gravity/cloths/SoftBody.h"
#include "gravity/narrowphase/ConeRay.h"

namespace Collision
{
	bool SphereSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		Collision::C_SphereShape* pSphere = (Collision::C_SphereShape*)pShape;
		Maths::HVec3 closest(Maths::UNDEFINED);
		const Maths::HVec3 centre(pSphere->m_Sphere.Centre());
		const Maths::HF32 radius(pSphere->m_Sphere.Radius());
		const Maths::HVec3 vertA(tree.m_pParticles[tri.m_VertexIndexA].m_CurPosition);
		const Maths::HVec3 vertB(tree.m_pParticles[tri.m_VertexIndexB].m_CurPosition);
		const Maths::HVec3 vertC(tree.m_pParticles[tri.m_VertexIndexC].m_CurPosition);
		Collision::ClosestPtPointTriangle(centre, vertA, vertB, vertC, closest);
		const Maths::HVec3 dif(closest - centre);
		const Maths::HF32 dis(dif | dif);
		if(dis.LE(radius * radius))
		{
			Maths::HF32 d(dis.Sqrt());
			const Maths::HF32 penetration = radius - d;
			//const Maths::C_Normal resultNormal(dif/d, Maths::C_Normal::UNIT_LENGTH_VECTOR);
			const Maths::C_Normal resultNormal(dif);
			//const Maths::C_Normal resultNormal(Maths::Select((dif | tri.m_Normal).GE(zero), tri.m_Normal, -tri.m_Normal));
			const Maths::HVec3 contact = closest;

			//tree.AddTriangleContact(pClothNode, pNode, contact, resultNormal, penetration, tree.m_Thickness, index);
			contacts.AddContact(pClothNode, pNode, contact, resultNormal, tri.m_Normal, penetration,  tree.m_Thickness, index, 
				C_ParticleContact::E_TriangleConstraint);
			return true;
		}
		return false;
	}

	bool CapsuleSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts)
	{
		Collision::C_CapsuleShape* pCapsuleShape = (Collision::C_CapsuleShape*)pShape;
		Collision::C_Capsule& capsule = pCapsuleShape->m_Capsule;
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 radSq(capsule.Radius() * capsule.Radius());

		/*
		 * (1)Closest pt segment - triangle
		 * (2) Closest pt start - triangle
		 * (3) Closest pt end - triangle
		 * (4) Intersect pt segment, triangle
		 */

		//Intersect segment with triangle!

		const Maths::HVec3 vertA(tree.m_pParticles[tri.m_VertexIndexA].m_CurPosition);
		const Maths::HVec3 vertB(tree.m_pParticles[tri.m_VertexIndexB].m_CurPosition);
		const Maths::HVec3 vertC(tree.m_pParticles[tri.m_VertexIndexC].m_CurPosition);

		const Maths::C_Normal direction(pCapsuleShape->m_PosRot.AY());

		const Maths::HVec3 startA(capsule.Start() + direction * capsule.Radius());
		const Maths::HVec3 endA(capsule.End() - direction * capsule.Radius());

		const Maths::HF32 penStart(startA | tri.m_Normal);
		const Maths::HF32 penEnd(endA | tri.m_Normal);

		const Maths::HVec3 start(Maths::Select(penStart.GE(zero), startA, endA));
		const Maths::HVec3 end(Maths::Select(penStart.GE(zero), endA, startA));
		Maths::HVec3 hitPoint(Maths::UNDEFINED);
		Maths::HVec3 closestSegment(Maths::UNDEFINED);


		bool bHit(false);
		if(Collision::IntersectTriangleSegment(vertA, vertB, vertC, startA, endA, hitPoint))
		{
			const Maths::Scalar_Bool bSide(Maths::Abs(penStart).GE(Maths::Abs(penEnd)));
			const Maths::HVec3 bottomSide(Maths::Select(bSide, startA, endA));
			const Maths::HF32 penetration(Maths::Select(bSide, penStart, penEnd));
			const Maths::C_Normal norm0(Maths::Select(penetration.LE(zero), -tri.m_Normal, tri.m_Normal));
			//tree.AddTriangleContact(pClothNode, pNode, hitPoint,norm0, Maths::Abs(penetration), tree.m_Thickness, index);
			contacts.AddContact(pClothNode, pNode, hitPoint,norm0, tri.m_Normal, Maths::Abs(penetration),  tree.m_Thickness, index, 
				C_ParticleContact::E_TriangleConstraint);
			bHit = true;
		}

		{
			//Now test closest pt point triangle!!!
			Collision::ClosestPtPointTriangle(capsule.Start(), vertA, vertB, vertC, hitPoint);
			Collision::ClosestPtPointSegment(hitPoint, capsule.Start(), capsule.End(), closestSegment);

			const Maths::HVec3 dif(closestSegment - hitPoint);
			const Maths::HF32 dis(dif | dif);
			if(dis.LE(radSq))
			{
				//hit!
				const Maths::HF32 distance(dis.Sqrt());
				const Maths::HF32 penetration(capsule.Radius() - distance);
				const Maths::C_Normal norm0(-dif/distance, Maths::C_Normal::UNIT_LENGTH_VECTOR);
				//tree.AddTriangleContact(pClothNode, pNode, hitPoint, norm0, penetration, tree.m_Thickness, index);
				contacts.AddContact(pClothNode, pNode, hitPoint, norm0, tri.m_Normal, Maths::Abs(penetration), tree.m_Thickness, index, 
					C_ParticleContact::E_TriangleConstraint);
				bHit = true;
			}
		}

		{
			//Now test closest pt point triangle!!!
			Collision::ClosestPtPointTriangle(capsule.End(), vertA, vertB, vertC, hitPoint);
			Collision::ClosestPtPointSegment(hitPoint, capsule.Start(), capsule.End(), closestSegment);

			const Maths::HVec3 dif(closestSegment - hitPoint);
			const Maths::HF32 dis(dif | dif);
			if(dis.LE(radSq))
			{
				//hit!
				const Maths::HF32 distance(dis.Sqrt());
				const Maths::HF32 penetration(capsule.Radius() - distance);
				const Maths::C_Normal norm0(-dif/distance, Maths::C_Normal::UNIT_LENGTH_VECTOR);
				//tree.AddTriangleContact(pClothNode, pNode, hitPoint, norm0, penetration, tree.m_Thickness, index);
				contacts.AddContact(pClothNode, pNode, hitPoint, norm0, tri.m_Normal, Maths::Abs(penetration), tree.m_Thickness, index, 
					C_ParticleContact::E_TriangleConstraint);
				bHit = true;
			}
		}

		return bHit;

	}


	void GetNormalAndPenetration(const Collision::C_OBB& obb, const Maths::HVec3 PCREF contact, const Maths::HVec3 PCREF a, 
		const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c,
		Maths::C_Normal& normal, Maths::HF32& penetration)
	{
		using namespace Maths;
		const HF32 zero(Constants<HF32>::Zero());
		const HVec3 e(obb.Extent());
		const HVec3 pen(e - Abs(contact));
		//Select which one is the smaller abs value!
		const HVec3 absPen(Abs(pen));
		const Scalar_Bool bX((absPen.X().LT(absPen.Y())) & (absPen.X().LT(absPen.Z())));
		const Scalar_Bool bY(absPen.Y().LT(absPen.Z()));

		const C_Normal norm(C_Normal(Select(bX, Constants<HVec3>::UnitAxisPositiveX(),
			Maths::Select(bY, Constants<HVec3>::UnitAxisPositiveY(), Constants<HVec3>::UnitAxisPositiveZ())), C_Normal::UNIT_LENGTH_VECTOR));
		const HF32 pp(norm | pen);
		const HF32 ss(norm | contact);
		normal = Maths::Select(ss.GE(zero), norm, -norm);
		const HF32 nA(normal | a);
		const HF32 nB(normal | b);
		const HF32 nC(normal | c);
		const HF32 extents(e | Abs(normal.m_V));


		const HF32 deepest(Min(nA, Min(nB, nC)));
		penetration = extents - deepest;
	}

	bool BoxSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts)
	{
		using namespace Maths;
		//Collide box-triangle!
		const Maths::HF32 _zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		Collision::C_BoxShape* pBox = (Collision::C_BoxShape*)pShape;
		Collision::C_OBB& obb = pBox->m_Box;

		//return bHit;

		const Maths::HVec3 a(tree.m_pParticles[tri.m_VertexIndexA].m_CurPosition);
		const Maths::HVec3 b(tree.m_pParticles[tri.m_VertexIndexB].m_CurPosition);
		const Maths::HVec3 c(tree.m_pParticles[tri.m_VertexIndexC].m_CurPosition);

		const HVec3 ca(obb.Centre());
		const HVec3 e(obb.Extent());
		const HVec3 ne(-e);
		
		const HVec3 na(a-ca);
		const HVec3 nb(b-ca);
		const HVec3 nc(c-ca);

		const HMat33& u = obb.Orientation();
		//translate the triangle from world space to obb's local space
		const HVec3 v0(u*na); 
		const HVec3 v1(u*nb);
		const HVec3 v2(u*nc);

		const Maths::C_Normal norm(u * tri.m_Normal);


		//const Maths::HVec3 oa(obb.u * (tree.m_Particles[tri.m_VertexIndexA].m_PrePosition - obb.c));
		//const Maths::HVec3 ob(obb.u * (tree.m_Particles[tri.m_VertexIndexB].m_PrePosition - obb.c));
		//const Maths::HVec3 oc(obb.u * (tree.m_Particles[tri.m_VertexIndexC].m_PrePosition - obb.c));

		//Maths::HVec3 minContacts(Maths::UNDEFINED), maxContacts(Maths::UNDEFINED);
		//Maths::HVec3 normal(Maths::UNDEFINED);

		const HVec3 max012(Max(v0, Max(v1, v2)));
		const HVec3 min012(Min(v0, Min(v1, v2)));
		if((max012.Any_LT(-e) | min012.Any_GT(e)))
			return false;


		//Compute edge vectors for triangle
		const HVec3 f0(v1-v0);
		const HVec3 f1(v2-v1);
		const HVec3 f2(v0-v2);

		
		const HF32 v0Y(v0.Y());
		const HF32 v0Z(v0.Z());

		const HF32 v2Y(v2.Y());
		const HF32 v2Z(v2.Z());
		

		const HF32 f0Y(f0.Y());
		const HF32 f0Z(f0.Z());
		

		const HF32 absf0Y(f0Y.Abs());
		const HF32 absf0Z(f0Z.Abs());
		//const HVec3 absF0(Abs(f0));

		HF32 p0(Maths::UNDEFINED), p1(Maths::UNDEFINED), ra(Maths::UNDEFINED), max(Maths::UNDEFINED), min(Maths::UNDEFINED);
		//Test axis a00=u0 Xf0, all triangle vertexs project onto axis a00
		
		Maths::HVec3 contactPoint(Maths::UNDEFINED);
		Maths::HF32 penetrationDepth(Maths::UNDEFINED);
		//Maths::HF32 zero(Maths::Constants<HF32>::Zero());

		//Test axis a00=u0 X f0, all triangle vertexs project onto axis a00
		
		
		p0=(v0Z*f0Y - v0Y*f0Z);
		p1=(v2Z*f0Y - v2Y*f0Z);
		ra=(e.Y()*absf0Z + e.Z()*absf0Y);
		max=Max(p0, p1);
		min=Min(p0, p1);
		if(max.LT(-ra) | min.GT(ra))
			return false;
		
		
		const HF32 f0X(f0.X());
		const HF32 v0X(v0.X());
		const HF32 v2X(v2.X());
		const HF32 absf0X(f0X.Abs());
		//Test axis a10=u1 X f0, all triangle vertex project onto axis a10

		p0=(v0X*f0.Z()-v0Z*f0.X());
		p1=(v2X*f0.Z()-v2Z*f0.X());
		ra=(e.X()*absf0Z+e.Z()*absf0X);
		max=(Max(p0, p1));
		min=(Min(p0, p1));
		if(max.LT(-ra) | min.GT(ra))
			return false;	

		//Test axis a20=u2 X f0, all triangle vertexs project onto axis a20
		p0=(v0Y*f0.X()-v0X*f0.Y());
		p1=(v2Y*f0.X()-v2X*f0.Y());
		
		ra=(e.X()*absf0Y+e.Y()*absf0X);
		max=(Max(p0, p1));
		min=(Min(p0, p1));
		if(max.LT(-ra) | min.GT(ra))
			return false;


		const HF32 v1Y(v1.Y());
		const HF32 v1Z(v1.Z());

		const HF32 f1X(f1.X());
		const HF32 f1Y(f1.Y());
		const HF32 f1Z(f1.Z());
		const HF32 absf1X(f1X.Abs());
		const HF32 absf1Y(f1Y.Abs());
		const HF32 absf1Z(f1Z.Abs());
		//Test axis a01=u0 X f1, all triangle vertexs project onto axis a01

		p0=(v0Z*f1Y-v0Y*f1Z);
		p1=(v1Z*f1Y-v1Y*f1Z);
		ra=(e.Y()*absf1Z + e.Z()*absf1Y);
		max=(Max(p0, p1));
		min=(Min(p0, p1));
		if(max.LT(-ra) | min.GT(ra))
			return false;
		

		const HF32 v1X(v1.X());
		//Test axis a11=u1 X f1, all triangle vertexs project onto axis a11
		
		p0=(v0X*f1Z-v0Z*f1X);
		p1=(v1X*f1Z-v1Z*f1X);
		ra=(e.X()*absf1Z+e.Z()*absf1X);
		max=(Max(p0, p1));
		min=(Min(p0, p1));
		if(max.LT(-ra) | min.GT(ra))
			return false;


		//Test axis a21=u2 X f1, all triangle vertexs project onto axis a21
		
		p0=(v0Y*f1X-v0X*f1Y);
		p1=(v1Y*f1X-v1X*f1Y);
		ra=(e.X()*absf1Y+e.Y()*absf1X);
		max=(Max(p0, p1));
		min=(Min(p0, p1));
		if(max.LT(-ra) | min.GT(ra))
			return false;
		
		
		const HF32 f2X(f2.X());
		const HF32 f2Y(f2.Y());
		const HF32 f2Z(f2.Z());
		const HF32 absf2X(f2X.Abs());
		const HF32 absf2Y(f2Y.Abs());
		const HF32 absf2Z(f2Z.Abs());

		//Test axis a02=u0 X f2, all triangle vertexs project onto axis a02
		p0=(v0Z*f2Y-v0Y*f2Z);
		p1=(v1Z*f2Y-v1Y*f2Z);
		ra=(e.Y()*absf2Z+ e.Z()*absf2Y);
		max=(Max(p0, p1));
		min=(Min(p0, p1));
		if(max.LT(-ra) | min.GT(ra))
			return false;
		
		
		
		//Test axis a12=u1 X f2, all triangle vertexs project onto axis a12
		p0=(v0X*f2Z-v0Z*f2X);
		p1=(v1X*f2Z-v1Z*f2X);
		ra=(e.X()*absf2Z+e.Z()*absf2X);
		max=(Max(p0, p1));
		min=(Min(p0, p1));
		if(max.LT(-ra) | min.GT(ra))
			return false;
		

		//Test axis a21=u2 X f2, all triangle vertexs project onto axis a22
		p0=(v0Y*f2X-v0X*f2Y);
		p1=(v1Y*f2X-v1X*f2Y);
		ra=(e.X()*absf2Y+e.Y()*absf2X);
		max=(Max(p0, p1));
		min=(Min(p0, p1));
		if(max.LT(-ra) | min.GT(ra))
			return false;
				
		//U32 vector_b(max012.AllLE(-e) | min012.All_GE(e));
		//Vector3_Bool vecb((max012 <= -e) || (min012 >= e));

		//Test triangle normal and AABB intersect
		
		//const HVec3 n(f0^f1);
		const HF32 d(norm|v0);
		const C_Normal absn(Maths::Abs(norm.m_V), Maths::C_Normal::UNIT_LENGTH_VECTOR);
		const HF32 r(e|absn);
		//const HF32 s=(n|aabb.c)-d;
		if(Abs(d).GT(r))
			return false;


		const HF32 x(e.X());
		const HF32 y(e.Y());
		const HF32 z(e.Z());


		const Maths::HVec3 verts[8] = 
		{
			-e, HVec3(x, -y, -z), HVec3(-x, y, -z), HVec3(x, y, -z),
			HVec3(-x, -y, z), HVec3(x, -y, z), HVec3(-x, y, z), e
		};


		const Maths::HF32 half(Maths::Constants<HF32>::Half());
		const Maths::HVec3 zero(Maths::Constants<HVec3>::Zero());

		
		HVec3 contacts0(Maths::UNDEFINED);
		HVec3 contacts1(Maths::UNDEFINED);
		HF32 penetration(Maths::UNDEFINED);
		F32 pDepth;

		C_Normal nn(Maths::UNDEFINED);
		HF32 pp(Maths::UNDEFINED);

		for(U32 i = 0; i < 7; ++i)
		{
			const U32 right(i | 1);
			const U32 up(i | 2);
			const U32 back(i | 4);
			const Maths::HF32 dA(verts[i] | norm);
			Maths::Vector3_Bool bAbove(dA.GE(d));
			
			if(i != right)
			{
				//Select the start and end points of these...
				const Maths::HVec3 pA(Select(bAbove, verts[i], verts[right]));
				const Maths::HVec3 pB(Select(bAbove, verts[right], verts[i]));
				if(IntersectTriangleBoxSegment(v0, v1, v2, pA, pB, 
					norm, contacts0, pDepth))
				{
					const HMat33& u(obb.Orientation());
					const Maths::C_Normal vectorDir(pA - pB);
					const Maths::HVec3 contact(contacts0 * u + obb.Centre());
					//const Maths::HF32 _d(norm | v0);
					const Maths::HF32 contactSide(contacts0 | norm);
					const Maths::C_Normal _norm(Maths::Select(contactSide.GT(_zero), norm, -norm));
					const Maths::HF32 dotScale(vectorDir | _norm);
					const Maths::C_Normal norm0(_norm * u, Maths::C_Normal::UNIT_LENGTH_VECTOR);
					const Maths::HF32 d(_norm | v0);					
					const Maths::HF32 pen0(((pA | _norm) - d) * dotScale);
					const Maths::HF32 pen1(((pB | _norm) - d) * dotScale);

					//penetration = Maths::Max(pen0, pen1);
					penetration = Maths::Min(Maths::Abs(pen0), Maths::Abs(pen1));

					Maths::HF32 newPen(Maths::UNDEFINED);
					Maths::C_Normal newNorm(Maths::UNDEFINED);

					GetNormalAndPenetration(obb, contacts0, v0, v1, v2, newNorm, newPen);
					newNorm = Maths::C_Normal(newNorm * u, Maths::C_Normal::UNIT_LENGTH_VECTOR);
					
					const Maths::Scalar_Bool bWhichNorm(newPen.LT(penetration));
					penetration = Maths::Select(bWhichNorm, newPen, penetration);
					const Maths::C_Normal finalNormal(Maths::Select(bWhichNorm, newNorm, norm0));

					//assert(penetration < 0.3f);
					//tree.AddTriangleContact(pClothNode, pNode, contact, finalNormal, tri.m_Normal, penetration, tree.m_Thickness, index);
					contacts.AddContact(pClothNode, pNode, contact, finalNormal, tri.m_Normal, penetration, tree.m_Thickness, index, 
						C_ParticleContact::E_TriangleConstraint);
				}
			}

			if(i != up)
			{
				//Select the start and end points of these...
				const Maths::HVec3 pA(Select(bAbove, verts[i], verts[up]));
				const Maths::HVec3 pB(Select(bAbove, verts[up], verts[i]));
				if(IntersectTriangleBoxSegment(v0, v1, v2, pA, pB, 
					norm, contacts0, pDepth))
				{
					const HMat33& u(obb.Orientation());
					const Maths::C_Normal vectorDir(pA - pB);
					const Maths::HVec3 contact(contacts0 * u + obb.Centre());
					//const Maths::HF32 _d(norm | v0);
					const Maths::HF32 contactSide(contacts0 | norm);
					const Maths::C_Normal _norm(Maths::Select(contactSide.GT(_zero), norm, -norm));
					const Maths::HF32 dotScale(vectorDir | _norm);
					const Maths::C_Normal norm0(_norm * u, Maths::C_Normal::UNIT_LENGTH_VECTOR);
					const Maths::HF32 d(_norm | v0);					
					const Maths::HF32 pen0(((pA | _norm) - d) * dotScale);
					const Maths::HF32 pen1(((pB | _norm) - d) * dotScale);

					//penetration = Maths::Max(pen0, pen1);
					penetration = Maths::Min(Maths::Abs(pen0), Maths::Abs(pen1));

					Maths::HF32 newPen(Maths::UNDEFINED);
					Maths::C_Normal newNorm(Maths::UNDEFINED);

					GetNormalAndPenetration(obb, contacts0, v0, v1, v2, newNorm, newPen);
					newNorm = Maths::C_Normal(newNorm * u, Maths::C_Normal::UNIT_LENGTH_VECTOR);
					
					const Maths::Scalar_Bool bWhichNorm(newPen.LT(penetration));
					penetration = Maths::Select(bWhichNorm, newPen, penetration);
					const Maths::C_Normal finalNormal(Maths::Select(bWhichNorm, newNorm, norm0));

					//assert(penetration < 0.3f);
					//tree.AddTriangleContact(pClothNode, pNode, contact,  finalNormal, tri.m_Normal, penetration,  tree.m_Thickness, index);
					contacts.AddContact(pClothNode, pNode, contact, finalNormal, tri.m_Normal, penetration, tree.m_Thickness, index, 
						C_ParticleContact::E_TriangleConstraint);
				}
			}

			if(i != back)
			{
				//Select the start and end points of these...
				const Maths::HVec3 pA(Select(bAbove, verts[i], verts[back]));
				const Maths::HVec3 pB(Select(bAbove, verts[back], verts[i]));
				if(IntersectTriangleBoxSegment(v0, v1, v2, pA, pB, 
					norm, contacts0, pDepth))
				{
					const HMat33& u(obb.Orientation());
					const Maths::C_Normal vectorDir(pA - pB);
					const Maths::HVec3 contact(contacts0 * u + obb.Centre());
					//const Maths::HF32 _d(norm | v0);
					const Maths::HF32 contactSide(contacts0 | norm);
					//const Maths::HF32 contactSide(-_d);
					const Maths::C_Normal _norm(Maths::Select(contactSide.GT(_zero), norm, -norm));
					const Maths::HF32 dotScale(vectorDir | _norm);
					const Maths::C_Normal norm0(_norm * u, Maths::C_Normal::UNIT_LENGTH_VECTOR);
					const Maths::HF32 d(_norm | v0);					
					const Maths::HF32 pen0(((pA | _norm) - d) * dotScale);
					const Maths::HF32 pen1(((pB | _norm) - d) * dotScale);

					//penetration = Maths::Max(pen0, pen1);
					penetration = Maths::Min(Maths::Abs(pen0), Maths::Abs(pen1));
					//assert(penetration < 0.3f);
					Maths::HF32 newPen(Maths::UNDEFINED);
					Maths::C_Normal newNorm(Maths::UNDEFINED);

					GetNormalAndPenetration(obb, contacts0, v0, v1, v2, newNorm, newPen);
					newNorm = Maths::C_Normal(newNorm * u, Maths::C_Normal::UNIT_LENGTH_VECTOR);
					
					const Maths::Scalar_Bool bWhichNorm(newPen.LT(penetration));
					penetration = Maths::Select(bWhichNorm, newPen, penetration);
					const Maths::C_Normal finalNormal(Maths::Select(bWhichNorm, newNorm, norm0));

					//assert(penetration < 0.3f);
					//tree.AddTriangleContact(pClothNode, pNode, contact, finalNormal, tri.m_Normal, penetration,  tree.m_Thickness, index);
					contacts.AddContact(pClothNode, pNode, contact, finalNormal, tri.m_Normal, penetration, tree.m_Thickness, index, 
						C_ParticleContact::E_TriangleConstraint);
				}
			}
		}		

		return true;
		
	}

	bool CylinderSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts)
	{
		using namespace Maths;

		Collision::C_CylinderShape* pCylinderShape = (Collision::C_CylinderShape*)pShape;
		Collision::C_Cylinder& cylinder = pCylinderShape->m_Cylinder;
		const HF32 zero(Maths::Constants<Maths::HF32>::Zero());

		//Intersect segment with triangle!
		const HVec3 vertA(tree.m_pParticles[tri.m_VertexIndexA].m_CurPosition);
		const HVec3 vertB(tree.m_pParticles[tri.m_VertexIndexB].m_CurPosition);
		const HVec3 vertC(tree.m_pParticles[tri.m_VertexIndexC].m_CurPosition);

		
		const HVec3 ab(vertB-vertA);
		const HVec3 ac(vertC-vertA);
		const C_Normal normal(ab^ac);
		const HF32 d(normal|vertA); //the plane sign distance

		const HVec3 center(cylinder.Centre());
		const C_Normal cAxis(cylinder.CentreAxis(),C_Normal::UNIT_LENGTH_VECTOR);

		const C_Normal dir(Normalise(normal^cAxis)); //construct a plane which include the center of cylinder and perpendicular to the triangle's normal
	
		const HF32 t(normal|cAxis);
		const C_Normal w(Normalise(C_Vector(normal) - cAxis*t)); //the component of normal orthogonal to cAxis

		const Maths::HVec3 dir1(cAxis * cylinder.HalfHeight());
		const Maths::HVec3 comp1(w*cylinder.Radius());
	

		//construct a rectange
		const HVec3 point1(center + dir1 + comp1);
		const HVec3 point2(center + dir1 - comp1);
		const HVec3 point3(center - dir1 - comp1);
		const HVec3 point4(center - dir1 + comp1);


		const HF32 dist1((normal|point1) - d);
		const HF32 dist2((normal|point2) - d);
		const HF32 dist3((normal|point3) - d);
		const HF32 dist4((normal|point4) - d);


		Scalar_Bool con1(dist1.GT(zero) & dist2.GT(zero) & dist3.GT(zero) & dist4.GT(zero));
		Scalar_Bool con2(dist1.LT(zero) & dist2.LT(zero) & dist3.LT(zero) & dist4.LT(zero));

		if(con1|con2)
			return false; //no intersection
 
		HVec3 contactA(Maths::UNDEFINED);
		HVec3 contactB(Maths::UNDEFINED);
		HF32 penA(Maths::UNDEFINED);
		HF32 penB(Maths::UNDEFINED);
		bool collideA = false, collideB = false;
		if(Collision::TomasCollideTriangleTriangle(vertA, vertB, vertC, point1, point2, point3, normal, dir, contactA, contactB, penA, penB))
		{
			const HVec3 v(contactA - contactB);
			const HF32 sqV(v|v);
			contacts.AddContact(pClothNode, pNode, contactA, normal, tri.m_Normal, penA, tree.m_Thickness, index, C_ParticleContact::E_TriangleConstraint);
			if(sqV.GT(zero))
				contacts.AddContact(pClothNode, pNode, contactB, normal, tri.m_Normal, penB, tree.m_Thickness, index, C_ParticleContact::E_TriangleConstraint);
			collideA = true;
		}

		if(Collision::TomasCollideTriangleTriangle(vertA, vertB, vertC, point1, point3, point4, normal, dir, contactA, contactB, penA, penB))
		{
			const HVec3 v(contactA - contactB);
			const HF32 sqV(v|v);
			contacts.AddContact(pClothNode, pNode, contactA, normal, tri.m_Normal, penA, tree.m_Thickness, index, C_ParticleContact::E_TriangleConstraint);
			if(sqV.GT(zero))
				contacts.AddContact(pClothNode, pNode, contactB, normal, tri.m_Normal, penB, tree.m_Thickness, index, C_ParticleContact::E_TriangleConstraint);
			collideB = true;
		}

		return collideA|collideB;
	}


	bool ConeSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts)
	{
		using namespace Maths;
		Collision::C_ConeShape* pConeShape = (Collision::C_ConeShape*)pShape;
		Collision::C_Cone& cone = pConeShape->m_Cone;
		const HF32 zero(Maths::Constants<Maths::HF32>::Zero());

		//Intersect segment with triangle!
		const HVec3 vertA(tree.m_pParticles[tri.m_VertexIndexA].m_CurPosition);
		const HVec3 vertB(tree.m_pParticles[tri.m_VertexIndexB].m_CurPosition);
		const HVec3 vertC(tree.m_pParticles[tri.m_VertexIndexC].m_CurPosition);
		const HVec3 ab(vertB - vertA);
		const HVec3 ac(vertC - vertA);
		const C_Normal norm(ab ^ ac);

		const HVec3 support1 = cone.Support(-norm) ;
		const HVec3 support2 = cone.Support(norm);

		const HF32 dist1 = norm |(support1 - vertA);
		const HF32 dist2 = norm |(support2 - vertB);

		//No intersection
		if((dist1*dist2).GT(zero))
			return false;

		const HVec3 midPoint = (support1 + support2)*Constants<HF32>::Half();
		const HVec3 dir(cone.Centre() - midPoint);
		const HVec3 support3 = cone.Support(dir);

		bool result = false;

		HVec3 contact2(UNDEFINED);

		if(IntersectTriangleSegmentSB(vertA, vertB, vertC, support1, support2, contact2))
		{
			const HF32 sum(dist1 + dist2);
			
			if(sum < zero)
				return false;

			contacts.AddContact(pClothNode, pNode, contact2, norm, tri.m_Normal, zero, tree.m_Thickness, index, C_ParticleContact::E_TriangleConstraint);
			result = true;

			const HVec3 projA = support1 + norm *dist1;

			if(Collision::PointInTriangle(projA, vertA, vertB, vertC))
			{
				const HF32 pDepth((support1 - projA).Length());
				contacts.AddContact(pClothNode, pNode, support1, norm, tri.m_Normal, pDepth, tree.m_Thickness, index, C_ParticleContact::E_TriangleConstraint);
			}
			else
			{

				HVec3 cP2(UNDEFINED);
				//ClosestPtPointTriangleSegment(contact2, a, b, c, cP2);
				ClosestPtSegmentTriangleSegment(support1, support2, vertA, vertB, vertC, cP2);

				const HVec3 end = cP2 - norm*(cone.BaseRadius() + cone.Height())*Constants<HF32>::Three();

				HVec3 contact(UNDEFINED);
				if(Collision::CollideConeRay(&cone, end, cP2, contact))
				{
					const HF32 pDepth((cP2 - contact).Length());
					contacts.AddContact(pClothNode, pNode, contact, norm, tri.m_Normal, pDepth, tree.m_Thickness, index, C_ParticleContact::E_TriangleConstraint);
				}
			}
		}

		
		HVec3 contact3(UNDEFINED);
		if(IntersectTriangleSegmentSB(vertA, vertB , vertC, support1, support3, contact3))
		{
			result =true;
			contacts.AddContact(pClothNode, pNode, contact3, norm, tri.m_Normal, zero, tree.m_Thickness, index, C_ParticleContact::E_TriangleConstraint);

			const HVec3 projA = support1 + norm *dist1;
			if(Collision::PointInTriangle(projA, vertA, vertB, vertC))
			{
				const HF32 pDepth((support1 - projA).Length());
				contacts.AddContact(pClothNode, pNode, support1, norm, tri.m_Normal, pDepth, tree.m_Thickness, index, C_ParticleContact::E_TriangleConstraint);
			}
			else
			{
				HVec3 cP2(UNDEFINED);
				//ClosestPtPointTriangleSegment(contact3, a, b, c, cP2);
				ClosestPtSegmentTriangleSegment(support1, support3, vertA, vertB, vertC, cP2);

				const HVec3 end = cP2 - norm*(cone.BaseRadius() + cone.Height())*Constants<HF32>::Three();

				//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(end, cP2, RGBA(255, 255, 0));
				HVec3 contact(UNDEFINED);
				if(Collision::CollideConeRay(&cone, end, cP2, contact))
				{
					const HF32 pDepth((cP2 - contact).Length());
					contacts.AddContact(pClothNode, pNode, contact, norm, tri.m_Normal, pDepth, tree.m_Thickness, index, C_ParticleContact::E_TriangleConstraint);
				}
			}
		}

		HVec3 contact4(UNDEFINED);
		if(IntersectTriangleSegmentSB(vertA, vertB, vertC, support2, support3, contact4))
		{
			
			contacts.AddContact(pClothNode, pNode, contact3, norm, tri.m_Normal, zero, tree.m_Thickness, index, C_ParticleContact::E_TriangleConstraint);
			result=true;
			
			const HF32 dist3 = norm |(support3 - vertA);
			const HVec3 projC = support3 + norm *dist3;
			if(Collision::PointInTriangle(projC, vertA, vertB, vertC))
			{
				const HF32 pDepth((support3 - projC).Length());
				contacts.AddContact(pClothNode, pNode, support3, norm, tri.m_Normal, pDepth, tree.m_Thickness, index, C_ParticleContact::E_TriangleConstraint);
			}
			else
			{
				HVec3 cP2(UNDEFINED);
				ClosestPtSegmentTriangleSegment(support2, support3, vertA, vertB, vertC, cP2);

				const HVec3 end = cP2 - norm*(cone.BaseRadius() + cone.Height())*Constants<HF32>::Three();

				HVec3 contact(UNDEFINED);
				if(Collision::CollideConeRay(&cone, end, cP2, contact))
				{
					const HF32 pDepth((cP2 - contact).Length());
					contacts.AddContact(pClothNode, pNode, contact, norm, tri.m_Normal, pDepth, tree.m_Thickness, index, C_ParticleContact::E_TriangleConstraint);
				}
			}
		}

		return result;
	}

	bool ConvexHullSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts)
	{
		return false;
	}

	bool CompositeSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts)
	{
		return false;
	}


}