#pragma once

namespace NexusRender
{
	__declspec(align(16)) class Plane
	{
	public:
		enum CollideResult
		{
			INTERSECTS,
			POSITIVE,
			NEGATIVE
		};

		Plane();
		Plane(const Maths::HVec4& plane);
		Plane(const Maths::HVec3& normal, F32 distance);

		~Plane();

		void SetPlaneEquation(const Maths::HVec4& plane);
		void SetNormal(const Maths::HVec3& normal);
		void SetDistance(F32 distance);

		const Maths::HVec3& Normal() const;
		Maths::HVec4 PlaneEquation() const;

		F32 DistanceFromPoint(const Maths::HVec3& point) const;

		F32 A() const;
		F32 B() const;
		F32 C() const;
		F32 D() const;

		F32 Distance() const;

		void A(F32 a);
		void B(F32 b);
		void C(F32 c);
		void D(F32 d);

		void Normalise();

		CollideResult Collide(const Maths::HVec3& point) const;
		CollideResult Collide(const Sphere& sphere) const;

		void* operator new (size_t size) 
		{
			return AllocAligned(size);
		}

		void operator delete (void *p)
		{	
			FreeAligned(p);	
		}

	private:
		Maths::HVec3	m_normal;
		F32				m_distance;
	};
}