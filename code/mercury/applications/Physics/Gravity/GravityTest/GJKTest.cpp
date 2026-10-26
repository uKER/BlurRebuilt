#include "stdafx.h"

#include "SectionPhysics.h"
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/GJK.inl"
#include "TriangleSetting.h"
#include "gravity/sdk/shapedescriptors.h"
#include "gravity/narrowphase/PersistentContactManifold.h"
#include <Core/Maths/Hvec/New/IncludePhysics.h>
#include <Core/Maths/Hvec/New/IncludePhysics.inl>


namespace Samples
{
	void C_SectionPhysics::GJKOBBsTest()
{
	using namespace Maths;
	using namespace Collision;

	static Collision::C_PersistentContactManifold manifold;
	manifold.SetContactBreakingThreshold(0.02f);

	//Test GJK OBB OBB intersection
	C_OBB boxA;
	//boxA.c=HVec3(3.f, 5.f, -2.f);
	boxA.c=HVec3(0.f, 0.f, 0.f);
	boxA.u=HMat33(UNDEFINED);
	boxA.u.SetIdentity();
	//boxA.e=HVec3(1.f, 2.f, 3.f);
	boxA.e=HVec3(1.f, 1.f, 1.f);
	boxA.margin = 0.02f;

	C_OBB boxB;
	//boxB.c=HVec3(-2.f, 4.f, 3.f);
	boxB.c=HVec3(1.f, 0.f, 0.f);
	boxB.u=HMat33(UNDEFINED);
	boxB.u.SetIdentity();
//	boxB.e=HVec3(2.f, 1.f, 2.f);
	boxB.e=HVec3(1.f, 1.f, 1.f);
	boxB.margin = 0.02f;

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;
	
	rot+=0.002f;

	if(m_input.IsButtonDn(E_Button_A))
	{
		rot-=0.02f;
	}

	if(m_input.IsButtonDn(E_Button_B))
	{
		rot+=0.02f;
	}

	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);
	boxA.u=matra;

	const Maths::RotY rotyb(rot);
	HMat33 matrb(rotyb);
	boxB.u=matrb;

	const HVec3 zero(Maths::Constants<HVec3>::Zero());
	HF32 dis;
	HVec3 closestA;
	HVec3 closestB;

	EPA epa;
	HVec3 contact;
	HVec3 normal;
	HF32 penetration;
	

	
	//s=GJKCPair<C_OBB, C_OBB>(boxA, boxB, dis, closestA, closestB);
	bool s =GJKCPair<C_OBB, C_OBB>(boxA, boxB, contact, normal, penetration, epa);

	
	

	//HF32 s=GJKCPair(boxA, boxB, dis, closestA, closestB);
	HMat43 matA(boxA.u, boxA.c);
	HVec3 minA(-boxA.e);
	HVec3 maxA(boxA.e);

	HMat43 matB(boxB.u, boxB.c);
	HVec3 minB(-boxB.e);
	HVec3 maxB(boxB.e);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, matA, Maths::RGBA(255, 0, 0));
	if(s)
	{
		Collision::C_ManifoldContact manifoldContact;
		manifoldContact.m_LocalContactA = boxA.u * (contact - boxA.c);
		const HVec3 bWorld = contact + normal*penetration;
		manifoldContact.m_LocalContactB = boxB.u * (bWorld - boxB.c);
		manifoldContact.m_Normal = normal;
		manifoldContact.m_ProjDist = (contact - bWorld)|manifoldContact.m_Normal;


		S32 index = manifold.GetEntryIndex(manifoldContact);
		if(index == -1 )//the contact doesn't exist in the manifold
		{
			manifold.AddManifoldContact(manifoldContact);
		}
		else
		{
			manifold.m_Manifold[index] = manifoldContact;
		}
			
		manifold.RefreshManifoldContacts(matA, matB);

		if(manifold.m_NumContacts == 4)
		{
			int bob=0;
		}

		for(U32 i=0; i< manifold.m_NumContacts; ++i)
		{

			const Collision::C_ManifoldContact& p=manifold.m_Manifold[i];
			const HVec3 a = boxA.c + p.m_LocalContactA*boxA.u;
			const HVec3 b = boxB.c + p.m_LocalContactB*boxB.u;
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(a, 0.05f, RGBA(0, 255, 0, 0));
			/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(b, 0.05f, RGBA(0, 0, 255, 0));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a, b, RGBA(0, 0, 255, 0));*/
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minB, maxB, matB, Maths::RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.05f, RGBA(0, 255, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(contact, "c");
		/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact, contact + normal*penetration, RGBA(0, 255, 0, 255));*/
	}
	else
	{
	
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minB, maxB, matB, Maths::RGBA(0, 0, 255));

	/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, Maths::RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.05f, Maths::RGBA(64, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.05f, Maths::RGBA(64, 0, 255));*/
	}

}


void C_SectionPhysics::GJKAABBsTest()
{
	using namespace Maths;
	using namespace Collision;
	C_AABB boxA;
	boxA.c=HVec3(4.f,8.f, 2.f);
	boxA.radius=HVec3(3.f, 4.f, 2.f);
	
	

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;
	//rot+=0.01f;

	if(m_input.IsButtonDn(E_Button_A))
	{
		rot-=0.02f;
	}

	if(m_input.IsButtonDn(E_Button_B))
	{
		rot+=0.02f;
	}

	const Maths::RotX rotx(rot);
	HMat44 mata(rotx);
	boxA.c=boxA.c*mata;
	//boxA[0]=boxA.c;
	boxA.wStart=boxA.c;


	C_AABB boxB;
	boxB.c=HVec3(2.f, -3.f, -4.f);
	boxB.radius=HVec3(2.f, 3.f, 2.f);
	

	const Maths::RotY roty(rot);
	HMat33 matb(roty);
	boxB.c=boxB.c*matb;
	//boxB[0]=boxB.c;
	boxB.wStart=boxB.c;

	const HVec3 rb(boxB.radius);
	const HVec3 cb(boxB.c);
	

	HF32 dis;
	HVec3 closestA;
	HVec3 closestB;
	

	HF32 s;
	for(int i=0; i<1000; i++)
	{
		//optimize("",off);
		s=GJKCPair<C_AABB, C_AABB>(boxA, boxB, dis, closestA, closestB);
		//optimize("",on);
	}

	HMat33 matA(Maths::UNDEFINED);
	matA.SetIdentity();

	HMat33 matB(Maths::UNDEFINED);
	matB.SetIdentity();

	HMat43 tranA(matA, boxA.c);
	HMat43 tranB(matB, boxB.c);

	HVec3 minA(-boxA.radius);
	HVec3 maxA(boxA.radius);

	HVec3 minB(-boxB.radius);
	HVec3 maxB(boxB.radius);

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minA, maxA, tranA, RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minB, maxB, tranB, RGBA(255, 0, 0));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, Maths::RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.05f, Maths::RGBA(64, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.05f, Maths::RGBA(64, 0, 255));
	
}
// ---

	void C_SectionPhysics::GJKCapsuleCapsule()
{
	using namespace Maths;
//	const HF32 half(Maths::Constants<HF32>::Half());
	Collision::C_Capsule a;
	a.m_Start=HVec3(-8.f, 4.f, -2.f);
	a.m_End=HVec3(-6.f, 3.f, 2.f);
	a.m_Radius=2.f;
	a.wStart=a.m_Start;

	//HVec3 mida((a.start+a.end)*half);
	HVec3 na(a.m_End-a.m_Start);
	Collision::C_Capsule b;
	b.m_Start=HVec3(4.f, 6.f, -5.5f);
	b.m_End=HVec3(-3.f, -5.f, 1.f);
	b.m_Radius=3.f;
	b.wStart=b.m_Start;
	//HVec3 midb((b.start+b.end)*half);
	HVec3 nb(b.m_End-b.m_Start);

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	/*if(m_input.IsButtonDn(E_Button_A))
	{
		rot-=0.02f;
	}
	

	if(m_input.IsButtonDn(E_Button_B))
	{
		rot+=0.02f;
	}
	*/
	
	rot+=0.02f;



	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);

	a.m_Start=a.m_Start*matra;
	a.m_End=a.m_End*matra;
	
	/*const Maths::RotZ rotzb(rot);
	HMat33 matrb(rotzb);
	b.start=b.start*matrb;
	b.end=b.end*matrb;
	*/

	HVec3 closestA(Maths::UNDEFINED);
	HVec3 closestB(Maths::UNDEFINED);
	HVec3 contact(Maths::UNDEFINED);
	C_Normal normal(Maths::UNDEFINED);
	HF32 pDepth(Maths::UNDEFINED);

	F32 s;

	for(int i=0; i<1000; ++i)
	{
		//a.wStart=a.start;
		//b.wStart=b.start;
		s=Collision::GJKCPair<Collision::C_Capsule, Collision::C_Capsule>(a, b, pDepth, closestA, closestB);
	}

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a.m_Start, a.m_End, RGBA(255, 0, 0));


	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(a.m_Start+(a.m_End-a.m_Start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, a.m_Radius, RGBA(255, 0, 0));
	}

	
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(a.m_Start, a.m_Radius, RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(a.m_End,a.m_Radius, RGBA(255, 0, 0));

	
	if(s==0)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(b.m_Start, b.m_End, RGBA(0, 0, 255));
		for(int i=0; i<20; i++)
		{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(b.m_Start+(b.m_End-b.m_Start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, nb, b.m_Radius, RGBA(0, 0, 255));
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(b.m_Start, b.m_Radius, RGBA(0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(b.m_End, b.m_Radius, RGBA(0, 0, 255));


		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.5f,  RGBA(255, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.5f,  RGBA(0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, RGBA(0,255,0));
	}
	else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(b.m_Start, b.m_End, RGBA(0, 255, 0));
		for(int i=0; i<20; i++)
		{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(b.m_Start+(b.m_End-b.m_Start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, nb, b.m_Radius, RGBA(0, 255, 0));
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(b.m_Start, b.m_Radius, RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(b.m_End, b.m_Radius, RGBA(0, 255, 0));


		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.5f,  RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.5f,  RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, RGBA(0,255,0));

	}
}


void C_SectionPhysics::GJKAABBCapsule()
{
//	const HF32 half(Maths::Constants<HF32>::Half());
	using namespace Maths;
	static Collision::C_Capsule a;
	a.m_Start=HVec3(-8.f, 4.f, -2.f);
	a.m_End=HVec3(-6.f, 3.f, 2.f);
	a.m_Radius=2.f;
	//a[0]=a.start;
	static bool iniA=false;
	if(!iniA)
	{
		a.wStart=a.m_Start;
		iniA = true;
	}
	HVec3 na(a.m_End-a.m_Start);

	
	static Collision::C_AABB box;
	box.c=HVec3(4.f,8.f, 2.f);
	box.radius=HVec3(3.f, 4.f, 2.f);
	//box[0]=box.c;
	box.wStart=box.c;

	static bool iniB=false;
	if(!iniB)
	{
		box.wStart=box.c;
		iniB = true;
	}


	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	/*if(m_input.IsButtonDn(E_Button_A))
	{
		rot-=0.02f;
	}
	

	if(m_input.IsButtonDn(E_Button_B))
	{
		rot+=0.02f;
	}
	*/
	
	rot+=0.02f;



	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);

	a.m_Start=a.m_Start*matra;
	a.m_End=a.m_End*matra;
	
	/*const Maths::RotZ rotzb(rot);
	HMat33 matrb(rotzb);
	b.start=b.start*matrb;
	b.end=b.end*matrb;
	*/

	HVec3 closestA(Maths::UNDEFINED);
	HVec3 closestB(Maths::UNDEFINED);
	HVec3 contact(Maths::UNDEFINED);
	C_Normal normal(Maths::UNDEFINED);

	HF32 pDepth(Maths::UNDEFINED);

	F32 s;

	for(int i=0; i<1000; ++i)
	{
		s=Collision::GJKCPair<Collision::C_Capsule, Collision::C_AABB>(a, box, pDepth, closestA, closestB);
	}

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(a.m_Start, a.m_End, RGBA(255, 0, 0));


	HF32 base(20.f);
	for(int i=0; i<20; i++)
	{
		HF32 ratio(((HF32)i)/base);
		HVec3 e(a.m_Start+(a.m_End-a.m_Start)*ratio);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(e, na, a.m_Radius, RGBA(255, 0, 0));
	}

	
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(a.m_Start, a.m_Radius, RGBA(255, 0, 0));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(a.m_End,a.m_Radius, RGBA(255, 0, 0));

	const HVec3 _min(- box.radius);
	const HVec3 _max(box.radius);
	HMat33 u(Maths::UNDEFINED);
	u.SetIdentity();
	HMat43 mat(u, box.c);
	if(s==0)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_min, _max, mat, RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, RGBA(0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.5f, RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.5f, RGBA(255, 0, 0));
	}
	else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(_min, _max, mat, RGBA(0, 0, 255));

	}
}

void C_SectionPhysics::GJKConvexTriangle()
{
		
	using namespace Maths;
	
	static Collision::C_ConvexHullShape* pHull = NULL;
	static Collision::EPA epa;
	//static Collision::EPA21 epa2;
	static bool bInitialised = false;
	

	if(bInitialised == false)
	{
		bInitialised = true;
		Gravity::C_ConvexHullDesc desc;
		//TODO - enter in the points that make up the convex shape!!!!
		//Try a 10-point shape here...
		desc.AddPoint(HVec3(-0.2f, -0.2f, -0.2f));
		desc.AddPoint(HVec3(0.2f, -0.2f, -0.2f));
		desc.AddPoint(HVec3(-0.2f, 0.2f, -0.2f));
		desc.AddPoint(HVec3(0.2f, 0.2f, -0.2f));

		desc.AddPoint(HVec3(-0.2f, -0.2f, 0.2f));
		desc.AddPoint(HVec3(0.2f, -0.2f, 0.2f));
		desc.AddPoint(HVec3(-0.2f, 0.2f, 0.2f));
		desc.AddPoint(HVec3(0.2f, 0.2f, 0.2f));

		desc.AddPoint(HVec3(0.f, -0.4f, 0.f));
		desc.AddPoint(HVec3(0.f, 0.4f, 0.f));

		desc.AddTriangle(Gravity::C_TriangleIndices(0, 3, 1));
		desc.AddTriangle(Gravity::C_TriangleIndices(0, 2, 3));
		desc.AddTriangle(Gravity::C_TriangleIndices(4, 5, 7));
		desc.AddTriangle(Gravity::C_TriangleIndices(4, 7, 6));

		desc.AddTriangle(Gravity::C_TriangleIndices(4, 2, 0));
		desc.AddTriangle(Gravity::C_TriangleIndices(4, 6, 2));
		desc.AddTriangle(Gravity::C_TriangleIndices(5, 3, 7));
		desc.AddTriangle(Gravity::C_TriangleIndices(5, 1, 3));

		desc.AddTriangle(Gravity::C_TriangleIndices(6, 7, 9));
		desc.AddTriangle(Gravity::C_TriangleIndices(6, 9, 2));
		desc.AddTriangle(Gravity::C_TriangleIndices(7, 3, 9));
		desc.AddTriangle(Gravity::C_TriangleIndices(3, 2, 9));

		desc.AddTriangle(Gravity::C_TriangleIndices(4, 8, 5));
		desc.AddTriangle(Gravity::C_TriangleIndices(8, 1, 5));
		desc.AddTriangle(Gravity::C_TriangleIndices(1, 8, 0));
		desc.AddTriangle(Gravity::C_TriangleIndices(4, 0, 8));

		desc.m_Margin = 0.2f;
		desc.m_Epsilon = 0.0005f;

		pHull = (Collision::C_ConvexHullShape*)desc.CreateShape(&m_TestPhysics.m_Filter);
	}	

	HVec3 position(0.f, 1.5f, 0.5f);
	HMat33 mat;
	mat.SetIdentity();
	pHull->Transform(HMat43(mat, position));
	Collision::C_ConvexHull& convex= pHull->m_Hull;
	
	static bool iniA=false;
	//if(!iniA)
	{
		convex.wStart=convex.Centre();
		//iniA = true;
	}

	static HF32 rot=0.f;
	if(rot.GE(Maths::Constants<HF32>::Pi()*Maths::Constants<HF32>::Two()))
		rot=0.f;

	if(m_input.IsButtonDn(E_Button_A))
	{
		rot-=0.02f;
	}
	

	if(m_input.IsButtonDn(E_Button_B))
	{
		rot+=0.02f;
	}
	const Maths::RotX rotxa(rot);
	HMat33 matra(rotxa);
	pHull->Transform(HMat43(matra, position));
	

	F32 s;

	Collision::C_Triangle tri;
	tri.m_VertexA=HVec3(HF32(triAX), HF32(triAY), HF32(triAZ));
	tri.m_VertexB=HVec3(HF32(triBX), HF32(triBY), HF32(triBZ));
	tri.m_VertexC=HVec3(HF32(triCX), HF32(triCY), HF32(triCZ));

	
	
	tri.WarmStart(tri.m_VertexA);
	Maths::HVec3 contact(Maths::UNDEFINED);
	Maths::HVec3 norm(Maths::UNDEFINED);
	Maths::HF32 penetration(Maths::UNDEFINED);
	tri.margin =0.2f;

	Collision::C_CollisionResult<4, 16> results;
	//results.m_pEpa = &epa;
	results.m_NumContactPatches = 1;

	const Maths::HVec3 ab(tri.m_VertexB - tri.m_VertexA);
	const Maths::HVec3 ac(tri.m_VertexC - tri.m_VertexA);
	C_Normal n(ab^ac);
	results.m_ContactPatches[0].m_Normal = n;

	//check whether the plane which the triangle is in sperate the convex hull
	/*const HVec3 support1 = convex.Support(n);
	const HVec3 support2 = convex.Support(-n);
	const HF32 dist1 = (support1 - tri.m_VertexA)|n;
	const HF32 dist2 = (support2 - tri.m_VertexA)|n;*/
	HVec3 closestA;
	HVec3 closestB;
	HF32 dist;
	
	//s=Collision::GJKCPair<Collision::C_ConvexHull, Collision::C_Triangle>(convex, tri, dist, closestA, closestB);
	//s=Collision::GJKCPair<Collision::C_ConvexHull, Collision::C_Triangle>(convex, tri , contact, norm, penetration, epa);
	s=Collision::GJKCPair<Collision::C_ConvexHull, Collision::C_Triangle>(convex, tri , contact, norm, penetration, epa);

	

	for(U32 i=0; i<convex.m_NumTriangles; ++i)
	{
		Collision::C_ConvexTriangle tri = convex.m_pTriangles[i];
		const HVec3 _a(&convex.m_pVertices[tri.m_VertexA]);
		const HVec3 _b(&convex.m_pVertices[tri.m_VertexB]);
		const HVec3 _c(&convex.m_pVertices[tri.m_VertexC]);

		const HVec3 a(_a * (convex.m_High - convex.m_Low) + convex.m_Low);
		const HVec3 b(_b * (convex.m_High - convex.m_Low) + convex.m_Low);
		const HVec3 c(_c * (convex.m_High - convex.m_Low) + convex.m_Low);
		const HVec3 a1 = a*convex.m_Orientation + convex.m_Position;
		const HVec3 b1 = b*convex.m_Orientation + convex.m_Position;
		const HVec3 c1 = c*convex.m_Orientation + convex.m_Position;
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(a1, b1, c1, RGBA(0, 255, 0, 255));
	}


	
	if(s)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexA, tri.m_VertexB, RGBA(0, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexA, tri.m_VertexC, RGBA(0, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexB, tri.m_VertexC, RGBA(0, 0, 0));

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(tri.m_VertexA, n, 0.5f, 0.025f, RGBA(0, 0, 0));

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.025f, RGBA(255, 0, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(contact, contact + norm *penetration, RGBA(255, 0, 0, 255));
		
		/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.05f, RGBA(256, 0, 0,255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.05f, RGBA(256, 0, 0,255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, RGBA(0, 0, 0,0 ));*/
	}
	else
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexA, tri.m_VertexB, RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexA, tri.m_VertexC, RGBA(0, 255, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(tri.m_VertexB, tri.m_VertexC, RGBA(0, 255, 0));

	}
}

}
