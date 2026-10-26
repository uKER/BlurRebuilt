/*
	Twilight Prophecy SDK
*/
#ifndef PRMATH_BOX_HPP
#define PRMATH_BOX_HPP


#include "minmax.hpp"


namespace prmath
{
	template <typename T>
	struct Box
	{

		// members

		float3 vmin;
		float3 vmax;

		// constructors

		Box()
		: vmin(+10000000000.0f, +10000000000.0f, +10000000000.0f),
		  vmax(-10000000000.0f, -10000000000.0f, -10000000000.0f)
		{
		}

		Box(const float3& umin, 
			const float3& umax)
		: vmin(umin), vmax(umax)
		{
		}

		bool operator==(const Box &other)
		{
			return  (vmin.x == other.vmin.x) &&
					(vmin.y == other.vmin.y) &&
					(vmin.z == other.vmin.z) &&
					(vmax.x == other.vmax.x) &&
					(vmax.y == other.vmax.y) &&
					(vmax.z == other.vmax.z);
		}

		Box(const Box& a, const Box& b)
		{
			vmin.x = a.vmin.x < b.vmin.x ? a.vmin.x : b.vmin.x;
			vmin.y = a.vmin.y < b.vmin.y ? a.vmin.y : b.vmin.y;
			vmin.z = a.vmin.z < b.vmin.z ? a.vmin.z : b.vmin.z;
			vmax.x = a.vmax.x > b.vmax.x ? a.vmax.x : b.vmax.x;
			vmax.y = a.vmax.y > b.vmax.y ? a.vmax.y : b.vmax.y;
			vmax.z = a.vmax.z > b.vmax.z ? a.vmax.z : b.vmax.z;
		}

		// methods

		void UpdateBox(const float3& umin, const float3& umax)
		{
			if ( umin.x < vmin.x) vmin.x = umin.x;
			if ( umin.y < vmin.y) vmin.y = umin.y;
			if ( umin.z < vmin.z) vmin.z = umin.z;
			if ( umax.x > vmax.x) vmax.x = umax.x;
			if ( umax.y > vmax.y) vmax.y = umax.y;
			if ( umax.z > vmax.z) vmax.z = umax.z;
		}

		void UpdateBox(const float3& v)
		{
			if ( v.x < vmin.x ) vmin.x = v.x; if ( v.x > vmax.x ) vmax.x = v.x;
			if ( v.y < vmin.y ) vmin.y = v.y; if ( v.y > vmax.y ) vmax.y = v.y;
			if ( v.z < vmin.z ) vmin.z = v.z; if ( v.z > vmax.z ) vmax.z = v.z;
		}

		float3 GetCenter() const
		{
			return (vmin + vmax) * static_cast<T>(0.5);
		}

		float3 GetSize() const
		{
			return vmax - vmin;
		}

		bool IsInside(const float3& point) const
		{
			if ( point.x < vmin.x || point.y < vmin.y || point.z < vmin.z ) return false;
			if ( point.x > vmax.x || point.y > vmax.y || point.z > vmax.z ) return false;
			return true;
		}
	};

	template <typename T>
	bool Intersect(const Box<T>& box1, const Box<T>& box2)
	{
		int j;

		for (j=0; j<3; j++)
			if (!(box1.vmin[j]<box2.vmax[j] &&
				  box1.vmax[j]>box2.vmin[j])) return false;

		return true;
	}


} // namespace prmath


	#ifndef PRMATH_NOTYPENAME

	typedef prmath::Box<F32>		box3f;

	#endif // PRMATH_NOTYPENAME


#endif