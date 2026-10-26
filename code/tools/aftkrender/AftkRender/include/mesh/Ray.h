#ifndef AFTK_RENDER_RAY_H
#define AFTK_RENDER_RAY_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AftkRender
{
	class C_MeshSceneGraphNode;

	struct MeshIntersection
	{
		S32 cluster;
		S32 polygon;
		S32 batch;
		Maths::HVec3 intersection;

		MeshIntersection() : cluster(-1), polygon(-1), batch(-1) { }
	};

	struct TriangleIntersection
	{
		Maths::HVec3 intersection;
		F32 distance;
		bool hit;

		TriangleIntersection() : distance(0.f), hit(false) { }
	};

	struct PlaneIntersection
	{
		F32 distance;
		bool hit;

		PlaneIntersection() : hit(false) { }
	};

	class C_Ray
	{
	public:
		C_Ray(const Maths::HVec3& origin, const Maths::HVec3& direction);

		C_Ray(const C_Ray& rhs);

		void IntersectTriangle(	TriangleIntersection& result, 
								const Maths::HVec3& p0, 
								const Maths::HVec3& p1, 
								const Maths::HVec3& p2) const;

		void IntersectPlane(PlaneIntersection& result, 
							const Maths::HVec3& p0, 
							const Maths::HVec3& p1, 
							const Maths::HVec3& p2) const;

	public:
		Maths::HVec3	m_Origin;
		Maths::HVec3	m_Direction;
	};


	void IntersectMesh(const C_Ray& ray, MeshIntersection& result, const C_MeshSceneGraphNode *pCluster);

	inline C_Ray::C_Ray(const Maths::HVec3& origin, const Maths::HVec3& direction)
	: m_Origin(origin)
	, m_Direction(direction)
	{
	}

	inline C_Ray::C_Ray(const C_Ray& ray)
	: m_Origin(ray.m_Origin)
	, m_Direction(ray.m_Direction)
	{
	}

}

#endif