/*
	Twilight Prophecy SDK
	A multi-platform development system for virtual reality and multimedia.

	Copyright (C) 1997-2003 Twilight 3D Finland Oy Ltd.
*/
#ifndef PRMATH_PLANE_HPP
#define PRMATH_PLANE_HPP


namespace prmath
{

	template <typename T>
	struct Plane
	{

		// members

		float3		normal;
		T			dist;
		
		// constructors

		Plane() 
		{
		}

		Plane(const float3& n, const T& d)
		: normal(n), dist(d)
		{
		}

		Plane(const float3& n, const float3& point)
		: normal(n), dist(DotProduct(n,point))
		{
		}

		Plane(const float3& v1, const float3& v2, const float3& v3)
		: normal(Normalize(CrossProduct(v2-v1,v3-v1))), dist(DotProduct(v1,normal))
		{
		}

		Plane(const Plane& plane)
		: normal(plane.normal), dist(plane.dist)
		{
		}

		// methods		

		T GetOffset(const float3& point) const
		{
			return DotProduct(normal,point) - dist;
		}

		bool IsFront(const float3& point) const
		{
			return GetOffset(point) > 0;
		}

		bool IsBack(const float3& point) const
		{
			return GetOffset(point) < 0;
		}

		void Normalize()
		{
			float denom = 1.f / 
				normal.Length();
			normal *= denom;
			dist *= denom;
		}
	};

} // namespace prmath


	#ifndef PRMATH_NOTYPENAME

	typedef prmath::Plane<float>	plane3f;
	typedef prmath::Plane<double>	plane3d;

	#endif // PRMATH_NOTYPENAME


#endif