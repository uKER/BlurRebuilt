#ifndef RD_BOUNDS_H
#define RD_BOUNDS_H
#ifdef _MSC_VER
#pragma once
#endif

#include "maths.h"

namespace RenderingData
{
	struct ALIGN16 Bounds
	{
		Point3 aabbMin;
		Point3 aabbMax;
		float distanceMin;
		float distanceMax;

		// Set the bounds as large as possible.
		void SetUniversal();

		// Set the bounds to the minimum values (such that they have no intersection with another bounds object).
		void SetEmpty();

		// Set the axis-aligned bounding box to the minimum values.
		void SetAABBEmpty();

		// Set the axis-aligned bounding box to the size of the triangle.
		void SetAABBTriangle(const Point3& p0, const Point3& p1, const Point3& p2);

		// Extend the axis-aligned bounding box to include the point p.
		void SetUnion(const Point3& p);

		// Extend to include another Bounds object.
		void SetUnion(const Bounds& b);

		// Reduce to the overlap with another bounds object.
		void SetIntersection(const Bounds& b);

		// Scale bounding box.
		void Scale(const F32 s);

		// Get the width, height and length of the bounding box.
		Vector3 GetSize() const;

		// Get the centre of the bounding box.
		Point3 GetCentre() const;

		bool Contains(const Bounds& b) const;
		bool Intersects(const Bounds& b) const;
	};

	bool operator == (const Bounds& b1, const Bounds& b2);

	#include "noninterface/bounds.inl"
}

#endif
