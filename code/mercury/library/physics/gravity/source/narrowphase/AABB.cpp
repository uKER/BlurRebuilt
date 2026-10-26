#include <stdafx.h>
#include "gravity/narrowphase/Primitive.h"



namespace Collision
{



/*HVec3 AABB::Support(const ConvexHull& hull, const Maths::HVec3 PCREF dir)
{
	const AABB& ab=(const AABB&) hull;
	const HVec3 r(ab.radius);
	const HVec3 zero(Maths::Constants<HVec3>::Zero());
	const Vector3_Bool b(dir < zero);
	const HVec3 v = Select(b, -r, r);
	return ab.c + v;
}
*/

bool IntersectAABBSegment(const C_AABB& a, const Maths::HVec3 PCREF p0, const Maths::HVec3 PCREF p1)
{
	using namespace Maths;
	const HVec3 e(a.Radius());
	const HVec3 half(Maths::Constants<HVec3>::Half());
	const HVec3 m((p0+p1)*half);
	const HVec3 d(p1-m);
	const HVec3 mid( m -a.Centre()); //translate box and segment to origin
	const HVec3 absm(Abs(mid));
	const HVec3 absd(Abs(d));
	const HVec3 temp(e+absd);

	if(!(absm.All_LE(temp)))
		return false;

	const HVec3 epislon(Maths::Constants<HVec3>::Epsilon2());
	const HVec3 ed(absd+epislon);
	
	const HF32 c1((mid.Y()*d.Z()-mid.Z()*d.Y()).Abs());
	const HF32 c2((mid.Z()*d.X()-mid.X()*d.Z()).Abs());
	const HF32 c3((mid.X()*d.Y()-mid.Y()*d.X()).Abs());

	const HF32 d1(e.Y()*ed.Z() +e.Z()*ed.Y());
	const HF32 d2(e.X()*ed.Z() +e.Z()*ed.X());
	const HF32 d3(e.X()*ed.Y() +e.Y()*ed.X());

	const HVec3 con(c1, c2, c3);
	const HVec3 don(d1, d2, d3);

	return (con.All_LE(don)==1);

	//return (absm.All_LE(temp) & con.All_LE(don))==1;
}


bool IntersectAABBs(const C_AABB& a, const C_AABB& b)
{
	using namespace Maths;
	const HVec3 t1(a.Centre() - b.Centre());
	const HVec3 t2(a.Radius() + b.Radius());
	return (Scalar_Bool)Abs(t1).All_GT(t2);
}

bool IntersectAABBs(const Maths::HVec3 PCREF aCenter, const Maths::HVec3 PCREF aExtents, const Maths::HVec3 PCREF bCenter, const Maths::HVec3 PCREF bExtents )
{
	using namespace Maths;
	const HVec3 t1(aCenter - bCenter);
	const HVec3 t2(aExtents + bExtents);
	return (Scalar_Bool)Abs(t1).All_GT(t2);
}

bool IntersectAABBPlane(const C_AABB& a, const Maths::HVec3 PCREF n, const Maths::HF32 PCREF d)
{
	using namespace Maths;
	const HVec3 radius(a.Radius());
	const HVec3 absn(Abs(n));
	const HF32 r =	radius.X()*absn.X() +
					radius.Y()*absn.Y() +
					radius.Z()*absn.Z();
	const HF32 s=(n|a.Centre())-d;
	return Abs(s).LE(r);
}


#define AXISTEST(v00, v01, v10, v11, a, b, fa, fb, ea, eb)			   \
	p0 = (a*v00) - (b*v01);			       	   \
	p2 = (a*v10) - (b*v11);			       	   \
	rad = (fa * ea) + (fb * eb);   \
	min = Maths::Min( p0, p2 ); \
	max = Maths::Max( p0, p2 ); \
	if(min.GT(rad) | max.LT(-rad)) return 0;							


bool IntersectAABBTriangle1(const C_AABB& aabb, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c)
{
	using namespace Maths;
	//const HVec3 ca(aabb.c);
	//const HVec3 e(aabb.radius);

	const HVec3 radius(aabb.Radius());
	const HVec3 centre(aabb.Centre());
	const HF32 ex(radius.X());
	const HF32 ey(radius.Y());
	const HF32 ez(radius.Z());
	
	//Translate triangle as conceptually moving AABB to origin
	const HVec3 v0(a-centre); 
	const HVec3 v1(b-centre);
	const HVec3 v2(c-centre);

	//Compute edge vectors for triangle
	const HVec3 f0(v1-v0);
	const HVec3 f1(v2-v1);
	const HVec3 f2(v0-v2);

	
	//const HF32 v0Y(v0.Y());
	//const HF32 v0Z(v0.Z());

	//const HF32 v2Y(v2.Y());
	//const HF32 v2Z(v2.Z());

	//const HF32 f0X(f0.X());
	//const HF32 f0Y(f0.Y());
	//const HF32 f0Z(f0.Z());
	const HF32 absf0X(f0.X().Abs());
	const HF32 absf0Y(f0.Y().Abs());
	const HF32 absf0Z(f0.Z().Abs());

	HF32 p0(Maths::UNDEFINED);
	HF32 p2(Maths::UNDEFINED);
	HF32 rad(Maths::UNDEFINED);
	HF32 min(Maths::UNDEFINED);
	HF32 max(Maths::UNDEFINED);

	const HF32 v0X(v0.X());
	const HF32 v2X(v2.X());

	//AXISTEST(v0Z, v0Y, v2Z, v2Y, f0Y, f0Z, absf0Y, absf0Z, ez, ey); // axis a00
	AXISTEST(v0.Z(), v0.Y(), v2.Z(), v2.Y(), f0.Y(), f0.Z(), absf0Y, absf0Z, ez, ey); // axis a00
	AXISTEST(v0.X(), v0.Z(), v2.X(), v2.Z(), f0.Z(), f0.X(), absf0Z, absf0X, ex, ez); // axis a10
	AXISTEST(v0.Y(), v0.X(), v2.Y(), v2.X(), f0.X(), f0.Y(), absf0Y, absf0X, ex, ey); //axis a20


	//const HF32 v1Y(v1.Y());
	//const HF32 v1Z(v1.Z());

	//const HF32 f1X(f1.X());
	//const HF32 f1Y(f1.Y());
	//const HF32 f1Z(f1.Z());
	const HF32 absf1X(f1.X().Abs());
	const HF32 absf1Y(f1.Y().Abs());
	const HF32 absf1Z(f1.Z().Abs());
	const HF32 v1X(v1.X());

	AXISTEST(v0.Z(), v0.Y(), v1.Z(), v1.Y(), f1.Y(), f1.Z(), absf1Z, absf1Y, ey, ez); // axis a01
	AXISTEST(v0.X(), v0.Z(), v1.X(), v1.Z(), f1.Z(), f1.X(), absf1Z, absf1X, ex, ez); //axis a11
	AXISTEST(v0.Y(), v0.X(), v1.Y(), v1.X(), f1.X(), f1.Y(), absf1Y, absf1X, ex, ey); //axis a21


	//const HF32 f2X(f2.X());
	//const HF32 f2Y(f2.Y());
	//const HF32 f2Z(f2.Z());
	const HF32 absf2X(f2.X().Abs());
	const HF32 absf2Y(f2.Y().Abs());
	const HF32 absf2Z(f2.Z().Abs());

	AXISTEST(v0.Z(), v0.Y(), v1.Z(), v1.Y(), f2.Y(), f2.Z(), absf2Z, absf2Y, ey, ez); //axis a02
	AXISTEST(v0.X(), v0.Z(), v1.X(), v1.Z(), f2.Z(), f2.X(), absf2Z, absf2X, ex, ez); //axis a12
	AXISTEST(v0.Y(), v0.X(), v1.Y(), v1.X(), f2.X(), f2.Y(), absf2Y, absf2X, ex, ey); //axis a22


	//Test axis u[0]=(1,0,0), u[1]={0,1,0}, u[2]={0, 0, 1}
	const HVec3 max012(Max(v0, Max(v1, v2)));
	const HVec3 min012(Min(v0, Min(v1, v2)));
	//if(!(max012.All_GT(-aabb.radius) & min012.All_LT(aabb.radius)))
		//return false;

	//Test triangle normal and AABB intersect
	
	const HVec3 n(f0^f1);
	const HF32 d(n|v0);

	const HVec3 absn(Abs(n));
	//const HF32 r=ex*Abs(n.X())+ey*Abs(n.Y())+ez*Abs(n.Z());
	const HF32 r(radius|absn);
	//const HF32 s=(n|aabb.c)-d;
	U32 b32((max012.All_LE(-radius)) | (min012.All_GE(radius)));
	return (b32 & U32(Abs(d)<=r))!=0;
	//return true;
	

}


bool IntersectAABBTriangle(const C_AABB& aabb, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c)
{
	using namespace Maths;
	const HVec3 ca(aabb.Centre());
	const HVec3 e(aabb.Radius());
	
	//Translate triangle as conceptually moving AABB to origin
	const HVec3 v0(a-ca); 
	const HVec3 v1(b-ca);
	const HVec3 v2(c-ca);

	//Compute edge vectors for triangle
	const HVec3 f0(v1-v0);
	const HVec3 f1(v2-v1);
	const HVec3 f2(v0-v2);

	
	const HF32 v0Y(v0.Y());
	const HF32 v0Z(v0.Z());

	const HF32 v2Y(v2.Y());
	const HF32 v2Z(v2.Z());
	

	const HF32 f0Y(f0.Y());
	const HF32 f0Z(f0.Z());
	

	const HF32 absf0Y(f0Y.Abs());
	const HF32 absf0Z(f0Z.Abs());
	//const HVec3 absF0(Abs(f0));

	HF32 p0(Maths::UNDEFINED), p1(Maths::UNDEFINED), ra(Maths::UNDEFINED), max(Maths::UNDEFINED), min(Maths::UNDEFINED);
	//Test axis a00=u0 X f0, all triangle vertexs project onto axis a00
	
	
	p0=(v0Z*f0Y - v0Y*f0Z);
	p1=(v2Z*f0Y - v2Y*f0Z);
	ra=(e.Y()*absf0Z + e.Z()*absf0Y);
	max=Max(p0, p1);
	min=Min(p0, p1);
	if(max < (-ra) || min > (ra))
		return false;
	
	
	
	const HF32 f0X(f0.X());
	const HF32 v0X(v0.X());
	const HF32 v2X(v2.X());
	const HF32 absf0X(f0X.Abs());
	//Test axis a10=u1 X f0, all triangle vertex project onto axis a10

	p0=(v0X*f0.Z()-v0Z*f0.X());
	p1=(v2X*f0.Z()-v2Z*f0.X());
	ra=(e.X()*absf0Z+e.Z()*absf0X);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) || min > (ra))
		return false;	
	
	//Test axis a20=u2 X f0, all triangle vertexs project onto axis a20
	p0=(v0Y*f0.X()-v0X*f0.Y());
	p1=(v2Y*f0.X()-v2X*f0.Y());
	
	ra=(e.X()*absf0Y+e.Y()*absf0X);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) || min > (ra))
		return false;


	const HF32 v1Y(v1.Y());
	const HF32 v1Z(v1.Z());

	const HF32 f1X(f1.X());
	const HF32 f1Y(f1.Y());
	const HF32 f1Z(f1.Z());
	const HF32 absf1X(f1X.Abs());
	const HF32 absf1Y(f1Y.Abs());
	const HF32 absf1Z(f1Z.Abs());
	//Test axis a01=u0 X f1, all triangle vertexs project onto axis a01

	p0=(v0Z*f1Y-v0Y*f1Z);
	p1=(v1Z*f1Y-v1Y*f1Z);
	ra=(e.Y()*absf1Z + e.Z()*absf1Y);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) || min > (ra))
		return false;
	

	const HF32 v1X(v1.X());
	//Test axis a11=u1 X f1, all triangle vertexs project onto axis a11
	
	p0=(v0X*f1Z-v0Z*f1X);
	p1=(v1X*f1Z-v1Z*f1X);
	ra=(e.X()*absf1Z+e.Z()*absf1X);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) || min > (ra))
		return false;
	
	
	

	//Test axis a21=u2 X f1, all triangle vertexs project onto axis a21
	
	p0=(v0Y*f1X-v0X*f1Y);
	p1=(v1Y*f1X-v1X*f1Y);
	ra=(e.X()*absf1Y+e.Y()*absf1X);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) || min > (ra))
		return false;
	
	
	const HF32 f2X(f2.X());
	const HF32 f2Y(f2.Y());
	const HF32 f2Z(f2.Z());
	const HF32 absf2X(f2X.Abs());
	const HF32 absf2Y(f2Y.Abs());
	const HF32 absf2Z(f2Z.Abs());

	//Test axis a02=u0 X f2, all triangle vertexs project onto axis a02
	p0=(v0Z*f2Y-v0Y*f2Z);
	p1=(v1Z*f2Y-v1Y*f2Z);
	ra=(e.Y()*absf2Z+ e.Z()*absf2Y);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) || min > (ra))
		return false;
	
	
	
	//Test axis a12=u1 X f2, all triangle vertexs project onto axis a12
	p0=(v0X*f2Z-v0Z*f2X);
	p1=(v1X*f2Z-v1Z*f2X);
	ra=(e.X()*absf2Z+e.Z()*absf2X);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) || min > (ra))
		return false;
	

	//Test axis a21=u2 X f2, all triangle vertexs project onto axis a22
	p0=(v0Y*f2X-v0X*f2Y);
	p1=(v1Y*f2X-v1X*f2Y);
	ra=(e.X()*absf2Y+e.Y()*absf2X);
	max=(Max(p0, p1));
	min=(Min(p0, p1));
	if(max < (-ra) | min > (ra))
		return false;
	


	const HVec3 max012(Max(v0, Max(v1, v2)));
	const HVec3 min012(Min(v0, Min(v1, v2)));
	//Test axis u[0]=(1,0,0), u[1]={0,1,0}, u[2]={0, 0, 1}
	if(!(max012.All_GT(-e) & min012.All_LT(e)))
		return false;
	//U32 vector_b(max012.AllLE(-e) | min012.All_GE(e));
	//Vector3_Bool vecb((max012 <= -e) || (min012 >= e));

	//Test triangle normal and AABB intersect
	
	const HVec3 n(f0^f1);
	const HF32 d(n|v0);
	const HVec3 absn(Abs(n));
	//const HF32 r=ex*Abs(n.X())+ey*Abs(n.Y())+ez*Abs(n.Z());
	const HF32 r(e|absn);
	//const HF32 s=(n|aabb.c)-d;
	return Abs(d).LE(r);	
}

bool IntersectAABBMinMax(const C_AABBMinMax& a, const C_AABBMinMax& b)
{
	using namespace Maths;
	return !(a.m_max.Any_LT(b.m_min) | a.m_min.Any_GT(b.m_max));
}


};
