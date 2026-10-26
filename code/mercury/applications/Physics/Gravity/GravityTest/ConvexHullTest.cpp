#include "stdafx.h"

#include "SectionPhysics.h"
#include "SegmentSetting.h"
#include "gravity/narrowphase/ConvexHull.h"
#include "gravity/narrowphase/primitivetest.inl"
#include "gravity/narrowphase/Shapes.h"
//#include "gravity/narrowphase/CylinderRay.h"
#include "gravity/narrowphase/ConvexHullRay.inl"
#include "gravity/narrowphase/Ray.h"
#include "gravity/narrowphase/primitive.inl"
#include <Core/Maths/Hvec/New/IncludePhysics.h>
#include <Core/Maths/Hvec/New/IncludePhysics.inl>


namespace Samples
{
void C_SectionPhysics::TestConvexHullRay()
{
	using namespace Maths;

	Collision::C_CollisionFilter filter;
	Collision::C_ConvexHullShape cShape(&filter);
	
	Collision::C_ConvexHull& hull=cShape.m_Hull;
	
	hull.m_Position=Maths::HVec3(-5, 4, -1);
	hull.SetNumberVertices(10);
	hull.m_pVertices[0]=HVec3(-2.f, -2.f, -2.f);
	hull.m_pVertices[1]=HVec3(2.f, -2.f, -2.f);
	hull.m_pVertices[2]=HVec3(-2.f, 2.f, -2.f);
	hull.m_pVertices[3]=HVec3(2.f, 2.f, -2.f);

	hull.m_pVertices[4]=HVec3(-2.f, -2.f, 2.f);
	hull.m_pVertices[5]=HVec3(2.f, -2.f, 2.f);
	hull.m_pVertices[6]=HVec3(-2.f, 2.f, 2.f);
	hull.m_pVertices[7]=HVec3(2.f, 2.f, 2.f);

	hull.m_pVertices[8]=HVec3(0.f, -4.f, 0.f);
	hull.m_pVertices[9]=HVec3(0.f, 4.f, 0.f);

	hull.SetNumberTriangles(16);
	hull.SetTriangles(0, 3, 1);
	hull.SetTriangles(0, 2, 3);
	hull.SetTriangles(4, 5, 7);
	hull.SetTriangles(4, 7, 6);

	hull.SetTriangles(4, 2, 0);
	hull.SetTriangles(4, 6, 2);
	hull.SetTriangles(5, 3, 7);
	hull.SetTriangles(5, 1, 3);

	hull.SetTriangles(6, 7, 9);
	hull.SetTriangles(6, 9, 2);
	hull.SetTriangles(7, 3, 9);
	hull.SetTriangles(3, 2, 9);

	hull.SetTriangles(4, 8, 5);
	hull.SetTriangles(8, 1, 5);
	hull.SetTriangles(1, 8, 0);
	hull.SetTriangles(4, 0, 8);

	Collision::C_Ray ray;
	ray.m_start=Maths::HVec3(HF32(segAX), HF32(segAY), HF32(segAZ));
	ray.m_end=Maths::HVec3(HF32(segBX), HF32(segBY), HF32(segBZ));

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

	hull.m_Orientation=matra;
	U32 addy = (U32)alloca(sizeof(Maths::HVec3)*hull.m_NumVertices + 16);
	addy = (addy + 15)& 0xfffffff0;
	Maths::HVec3* verts((Maths::HVec3*)addy);

	for(U32 b = 0; b < hull.m_NumVertices; ++b)
	{
		const Maths::HVec3 localV(&hull.m_pVertices[b]);
		verts[b] = hull.m_Position + (localV * (hull.m_High - hull.m_Low) + hull.m_Low) * matra;
	}

	//Loop through all the convex triangles and draw them!
	const Maths::RGBA colour(0, 0, 255);
	for(U32 b = 0; b < hull.m_NumTriangles; ++b)
	{
		Collision::C_ConvexTriangle& tri(hull.m_pTriangles[b]);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleLit(verts[tri.m_VertexA],
			verts[tri.m_VertexB], verts[tri.m_VertexC], colour);	
	}

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, RGBA(255, 0, 0, 255));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(ray.m_start, 0.05f, RGBA(0, 0, 255, 255));

	bool collide=Collision::CollideConvexHullRay(&cShape, ray);

	if(collide)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(ray.m_start, ray.m_end, RGBA(0, 0, 0));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(ray.m_end, 0.5f, RGBA(0, 255, 0, 255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(ray.m_end, ray.m_result.m_normal, 2.f, 0.5f, RGBA(0, 255, 0, 0));
	}

}

};
