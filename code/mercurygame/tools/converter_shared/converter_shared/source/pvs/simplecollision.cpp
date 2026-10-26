#include "stdafx.h"
#include "converter_shared/pvs/simplecollision.h"

namespace SimpleCollision
{

BoolVR GetContact(const RayCast& rayCast, const Triangle& triangle, Contact& contact)
{
	const Point3R& v0 = triangle.vertex0;
	const Point3R& v1 = triangle.vertex1;
	const Point3R& v2 = triangle.vertex2;

	// reject by triangle normal
	FloatVR a1 = triangle.normal | (v0 - rayCast.start);
	FloatVR a2 = triangle.normal | (v0 - rayCast.end);
	BoolVR reject1 = (a1 * a2 > Maths::Constants<FloatVR>::Zero());

	// reject by edges
	Vector3R l0 = (rayCast.start - v0) ^ rayCast.direction;
	FloatVR b1 = (rayCast.start - v1) | l0;
	FloatVR b2 = (rayCast.start - v2) | l0;
	BoolVR reject2 = (b1 * b2 > Maths::Constants<FloatVR>::Zero()); // plane containing the line and one vertex must intersect the opposite edge

	Vector3R l1 = (rayCast.start - v1) ^ rayCast.direction;
	FloatVR c1 = (rayCast.start - v0) | l1;
	FloatVR c2 = (rayCast.start - v2) | l1;
	BoolVR reject3 = (c1 * c2 > Maths::Constants<FloatVR>::Zero()); // checking two edges is enough

	BoolVR collision = !(reject1 | reject2 | reject3);

	if (collision)
	{
		FloatVR f = triangle.normal | rayCast.direction;
		FloatVR d = triangle.normal | (v0 - rayCast.start);

		FloatVR distance = Maths::Select((Abs(f) > Maths::Constants<FloatVR>::Epsilon()),
											Abs(d / f),
											Maths::Constants<FloatVR>::Zero());

		if (distance < contact.distance)
		{
			contact.collision = Maths::Constants<BoolVR>::True();
			contact.normal = triangle.normal;
			contact.position = rayCast.start + rayCast.direction * distance;
			contact.distance = distance;

			return Maths::Constants<BoolVR>::True();
		}
	}

	return Maths::Constants<BoolVR>::False();
}

}