#include "stdafx.h"
#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/PrimitiveTest.inl"


//using namespace Maths;

namespace Collision
{

/*Maths::HVec3 Sphere::Support(const ConvexHull& hull, const Maths::HVec3 PCREF dir)
{
	const Sphere& s=(Sphere&) hull;
	Maths::HVec3 temp(s.radius*(dir/dir.Length()));
	return s.c+temp; //Maths::HVec3(s.c.X()+temp.X(),s.c.Y()+temp.Y(), s.c.Z()+temp.Z()); 
}
*/

bool IntersectSpherePlane(C_Sphere& s, Maths::HVec3 PCREF n, Maths::HF32 PCREF d)
{
	using namespace Maths;
	//Insure n is a unit vector
	assert(n.Length()==1.f+ (1e-3f));
	HF32 dist=(s.Centre()|n)-d;
	return ((HF32)Abs(dist)).LE(s.Radius());

}
};
