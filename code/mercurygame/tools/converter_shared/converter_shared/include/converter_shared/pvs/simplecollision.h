#ifndef SIMPLE_COLLISION
#define SIMPLE_COLLISION

#ifdef _MSC_VER
#pragma once
#endif

namespace SimpleCollision
{

typedef Maths::C_Vertex Point3R;
typedef Maths::C_Vector Vector3R;
typedef Maths::C_Normal Normal3R;
typedef Maths::Scalar_Bool BoolVR;
typedef Maths::HF32 FloatVR;

inline Maths::C_Normal Normalised(const Maths::C_Vector& vector)
{
	return vector.Normalised().Normal();
}

struct RayCast
{
	Point3R start;
	Point3R end;
	Normal3R direction;
	FloatVR maxDistance;

	RayCast() {}
	RayCast(const Point3R& p, const Normal3R& n, float d) :
		start(p), 
		end(p + n * d),
		direction(n),
		maxDistance(d)
		{}
};

struct Triangle
{
	Point3R vertex0;
	Point3R vertex1;
	Point3R vertex2;
	Normal3R normal;

	Triangle() {}
	Triangle(const Point3R& v0, const Point3R& v1, const Point3R& v2) :
		vertex0(v0),
		vertex1(v1),
		vertex2(v2),
		normal(Normalised((v0 - v1) ^ (v2 - v1)))
		{}
};

struct Contact
{
	Contact() : collision(false) {}

	BoolVR collision;
	FloatVR distance;
	Point3R position;
	Normal3R normal;
};

BoolVR GetContact(const RayCast& rayCast, const Triangle& triangle, Contact& contact);

}

#endif