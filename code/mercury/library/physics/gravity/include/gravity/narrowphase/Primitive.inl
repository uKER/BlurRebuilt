//#include "Primitive.h"
#ifndef GRAVITY_PRIMITIVE_INL
#define GRAVITY_PRIMITIVE_INL

//#include "Primitive.h"
#include "gravity/narrowphase/PrimitiveTest.h"

namespace Collision
{

NOALIAS inline Maths::HVec3 C_AABB::Support(const Maths::HVec3 PCREF dir)const 
{
	using namespace Maths;

	const HVec3 zero(Constants<HVec3>::Zero());
	const Vector3_Bool b(dir < zero);
	const HVec3 v = Select(b, -m_Radius, m_Radius);
	return m_Centre + v;
}

NOALIAS inline Maths::HVec3 C_AABB::SupportMargin(const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF margin)const 
{
	//const AABB& ab=(const AABB&) hull;
	//const HVec3 r(ab.radius);
	/*using namespace Maths;
	const Maths::HVec3 zero(Maths::Constants<HVec3>::Zero());
	const Maths::HVec3 marginRadius(radius - Maths::HVec3(_margin));
	const Maths::Vector3_Bool b(dir < zero);
	const HVec3 v = Select(b, -marginRadius, marginRadius);
	return c + v;*/
	using namespace Maths;

	const HVec3 zero(Constants<HVec3>::Zero());
	const Vector3_Bool b(dir < zero);
	const HVec3 marginExtents(m_Radius - HVec3(margin));
	const HVec3 v = Select(b, -marginExtents, marginExtents);
	return m_Centre + v;
}

NOALIAS inline Maths::HVec3 C_OBB::Support(const Maths::HVec3 PCREF dir)const 
{
	using namespace Maths;
	const HVec3 ld2(m_Orientation * dir);
	const Vector3_Bool b2(ld2 < Constants<HVec3>::Zero());
	const HVec3 v(Select(b2, -m_Extent, m_Extent));
	return  m_Centre+( v * m_Orientation);
}

NOALIAS inline Maths::HVec3 C_OBB::SupportMargin(const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF margin) const
{
	using namespace Maths;
	const HVec3 zero(Constants<HVec3>::Zero());
	const HVec3 ld2(m_Orientation * dir);
	const Vector3_Bool b2(ld2 < zero);
	const HVec3 marginExtents(m_Extent - HVec3(margin));
	BIZ_ASSERT(marginExtents.All_GE(zero));
	const HVec3 v(Select(b2, -marginExtents, marginExtents));

	return  m_Centre+( v * m_Orientation);
}

NOALIAS inline Maths::HVec3 C_Capsule::Support(const Maths::HVec3 PCREF dir)const
{
	using namespace Maths;

	const HF32 d1(m_Start|dir);
	const HF32 d2(m_End|dir);

	const Scalar_Bool b(d1.GT(d2));
	const HVec3 extrem(Select(b, m_Start, m_End));
	const C_Normal n(dir);

	return extrem+n*m_Radius;
}

NOALIAS inline Maths::HVec3 C_Capsule::SupportMargin(const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF _margin)const
{
	using namespace Maths;

	const HF32 d1(m_Start|dir);
	const HF32 d2(m_End|dir);

	const Scalar_Bool b(d1.GT(d2));
	const HVec3 extrem=Select(b, m_Start, m_End);

	//The margin is the radius, therefore we just need to return one of the end point in the segment
	return extrem;
}

NOALIAS inline Maths::HVec3 C_Sphere::Support(const Maths::HVec3 PCREF dir) const 
{
	const Maths::C_Normal nDir(dir);
	return m_Centre + m_Radius*nDir;

	//Maths::HVec3 temp(m_Radius*(dir/dir.Length()));
	//return m_Centre+temp; //Maths::HVec3(s.c.X()+temp.X(),s.c.Y()+temp.Y(), s.c.Z()+temp.Z()); 
}

NOALIAS inline Maths::HVec3 C_Sphere::SupportMargin(const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF margin) const 
{
	//The margin is the radius of the sphere, therefore we just need to return one point
	return m_Centre;
}

NOALIAS inline Maths::HVec3 C_Cylinder::Support(const Maths::HVec3 PCREF dir)const
{
	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HF32 d(dir|m_CentreAxis);
	const HVec3 w(dir - m_CentreAxis*d ); // the component of dir orthogonal to u
	const C_Normal normal(w);
	const Scalar_Bool b(d.GT(zero));
	const HF32 t=Select(b, one, -one );
	
	return m_Centre+m_CentreAxis*t*m_halfHeight+normal*m_Radius;
	
}

NOALIAS inline Maths::HVec3 C_Cylinder::SupportMargin(const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF margin)const
{
	using namespace Maths;
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HF32 d(dir|m_CentreAxis);
	const HVec3 w(dir - m_CentreAxis*d ); // the component of dir orthogonal to u
	const C_Normal normal(w);
	const Scalar_Bool b(d.GT(zero));
	const HF32 t=Select(b, one, -one );
	
	//return m_Centre+m_CentreAxis*t*m_halfHeight+normal*m_Radius;
	return m_Centre+m_CentreAxis*t*(m_halfHeight-margin)+normal*(m_Radius-margin);
	
}


NOALIAS inline Maths::HVec3 C_Cone::Support(const Maths::HVec3 PCREF dir)const
{
	using namespace Maths;
	//DebugBreak();

	const HF32 projDist = dir|m_MidPointAxis;
	const HVec3 w = dir -  projDist*m_MidPointAxis;
	const HF32 wLength = w.Length();

	const HF32 dirLength = dir.Length();
	const HF32 condi = dirLength * m_SinTheta;


	Maths::Scalar_Bool con1(projDist.GE(condi));
	Maths::Scalar_Bool con2(wLength.LT(Constants<HF32>::Epsilon()));

	
	const HVec3 v = (w/wLength) * m_BaseRadius;

	const HVec3 s = m_MidPointAxis * m_Height * Constants<HF32>::Half();

	return Select(con1, m_Centre + s, Select(con2, m_Centre - s, m_Centre - s + v));
}


NOALIAS inline Maths::HVec3 C_Cone::SupportMargin(const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF margin) const
{
	//using namespace Maths;
	////DebugBreak();

	//const HF32 projDist(dir|m_MidPointAxis);
	//const HVec3 w(dir -  projDist*m_MidPointAxis);
	//const HF32 wLength(w.Length());

	////const C_Normal w(dir - projDist*m_MidPointAxis);

	//const HF32 dirLength(dir.Length());
	//const HF32 condi( dirLength * m_SinTheta);

	//const Maths::Scalar_Bool con1(projDist.GE(condi));
	//const Maths::Scalar_Bool con2(wLength.LT(Constants<HF32>::Epsilon()));//w==0

	//
	//const HF32 marginBaseRadius(m_BaseRadius - margin);
	//const HVec3 v = (w/wLength) * marginBaseRadius;
	//
	//const HF32 marginHalfHeight(m_Height*Constants<HF32>::Half() - margin);

	//const HVec3 s(m_MidPointAxis * marginHalfHeight);
	//const HVec3 top(m_Centre + s);
	//const HVec3 bottom(m_Centre - s);

	//return(Select(con1, top, Select(con2, bottom, bottom + v)));

	using namespace Maths;

	const HF32 projDist(dir|m_MidPointAxis);
	const C_Normal w(dir - projDist*m_MidPointAxis);

	const HF32 dirLength(dir.Length());
	const HF32 condi( dirLength * m_SinTheta);

	const Maths::Scalar_Bool con(projDist.GE(condi));
	
	const HF32 marginBaseRadius(m_BaseRadius - margin);
	const HVec3 v(w * marginBaseRadius);
	
	const HF32 marginHalfHeight(m_Height*Constants<HF32>::Half() - margin);

	const HVec3 s(m_MidPointAxis * marginHalfHeight);
	const HVec3 top(m_Centre + s);
	const HVec3 bottom(m_Centre - s);

	return Select(con, top, bottom + v);
}


//NOALIAS inline void C_Plane::ProjectQuat(C_CRAxisBox&  axisBox, Maths::HVec3& aVex, Maths::HVec3& bVex, Maths::HVec3& cVex, Maths::HVec3& dVex)
//{
//	using namespace Maths;
//
//	const HVec3 center = axisBox.centre;
//	const HF32 cx = center.X();
//	const HF32 cy = center.Y();
//	const HF32 cz = center.Z();
//	const HVec3 extents = axisBox.extents;
//	const HF32 x = extents.X();
//	const HF32 y = extents.Y();
//	const HF32 z = extents.Z();
//
//	const HVec3 p1 (cx+x, cy+y, cz+z);
//	const HVec3 p2 (cx+x, cy+y, cz-z);
//	const HVec3 p3 (cx-x, cy+y, cz-z);
//	const HVec3 p4 (cx-x, cy+y, cz+z);
//	const HVec3 p5 (cx+x, cy-y, cz+z);
//	const HVec3 p6 (cx+x, cy-y, cz-z);
//	const HVec3 p7 (cx-x, cy-y, cz-z);
//	const HVec3 p8 (cx-x, cy-y, cz+z);
//
//	const HVec3 proj1 = p1 - Distance(p1)*m_n;
//	const HVec3 proj2 = p2 - Distance(p2)*m_n;
//	const HVec3 proj3 = p3 - Distance(p3)*m_n;
//	const HVec3 proj4 = p4 - Distance(p4)*m_n;
//	const HVec3 proj5 = p5 - Distance(p5)*m_n;
//	const HVec3 proj6 = p6 - Distance(p6)*m_n;
//	const HVec3 proj7 = p7 - Distance(p7)*m_n;
//	const HVec3 proj8 = p8 - Distance(p8)*m_n;
//
//
//	const HVec3 tmax1 = Max(proj1, proj2);
//	const HVec3 tmin1 = Min(proj1, proj2);
//	const HVec3 tmax2 = Max(proj3, proj4);
//	const HVec3 tmin2 = Min(proj3, proj4);
//	const HVec3 tmax3 = Max(proj5, proj6);
//	const HVec3 tmin3 = Min(proj5, proj6);
//	const HVec3 tmax4 = Max(proj7, proj8);
//	const HVec3 tmin4 = Min(proj7, proj8);
//
//	const HVec3 qmax1 = Max(tmax1, tmax2);
//	const HVec3 qmin1 = Min(tmin1, tmin2);
//	const HVec3 qmax2 = Max(tmax3, tmax4);
//	const HVec3 qmin2 = Min(tmin3, tmin4);
//
//	const HVec3 max = Max(qmax1, qmax2);
//	const HVec3 min = Min(qmin1, qmin2);
//
//	const HVec3 mid = (max + min)*Constants<HF32>::Half();
//
//	const HVec3 dif = max - min;
//	const HF32 length =dif.Length();
//	const HVec3 n = dif/length;
//
//	const HVec3 dir = m_n ^ n;
//
//	const C_Normal d(dir);
//
//	const HVec3 v1 =  d*length;
//	const HVec3 v2 =  - d*length;
//
//	const HVec3 vmin = min - mid;
//	const HVec3 vmax = max - mid;
//
//	aVex = mid + vmax + v1;
//	bVex = mid + vmax + v2;
//	cVex = mid + vmin + v2;
//	dVex = mid + vmin + v1;
//
//#ifndef PLATFORM_SPU
//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle
//#endif
//
//}

inline bool IntersectTriangleRay(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, Maths::HVec3& res)
{
	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HVec3 qp(p-q);
	const HVec3 ab(b-a);
	const HVec3 ac(c-a);
	const HVec3 ap(p-a);



	const HVec3 normal(ab^ac);

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(a, n, 0.25f, 0.2f, RGBA(0, 0, 255, 255));


	const HF32 d(qp|normal);
	const HF32 recipD(d.Reciprocal());

	if(d.EQ(zero))
		return false;

	const HF32 tTemp(ap|normal);
	const HF32 t(tTemp*recipD);

	if(t.LT(zero))
		return false;

	const HVec3 e(qp^ap);
	const HF32 vTemp(ac|e);
	const HF32 v(vTemp*recipD);

	if(v.LT(zero) | v.GT(one))
		return false;

	const HF32 wTemp(-(ab|e));
	const HF32 w(wTemp*recipD);

	if(w.LT(zero) | w.GT(one))
		return false;

	
	res=p-qp*t;

	return true;
}

inline bool IntersectTriangleRay(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, Maths::HF32& v, Maths::HF32& w)
{
	using namespace Maths;

	const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const HF32 one(Maths::Constants<Maths::HF32>::One());
	const HVec3 qp(p-q);
	const HVec3 ab(b-a);
	const HVec3 ac(c-a);
	const HVec3 ap(p-a);



	const HVec3 normal(ab^ac);

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(a, n, 0.25f, 0.2f, RGBA(0, 0, 255, 255));


	const HF32 d(qp|normal);
	const HF32 recipD(d.Reciprocal());

	if(d.EQ(zero))
		return false;

	const HF32 tTemp(ap|normal);
	const HF32 t(tTemp*recipD);

	if(t.LT(zero))
		return false;

	const HVec3 e(qp^ap);
	const HF32 vTemp(ac|e);
	v=(vTemp*recipD);

	if(v.LT(zero) | v.GT(one))
		return false;

	const HF32 wTemp(-(ab|e));
	w=(wTemp*recipD);

	if(w.LT(zero) | w.GT(one))
		return false;

	return true;
}

inline bool IntersectTriangleSegment(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, Maths::HVec3& res)
{	
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, b,c, RGBA(255, 0, 0));
	
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(p, "p");
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(q, "q");
	

	using namespace Maths;

	const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const HF32 one(Maths::Constants<Maths::HF32>::One());
	const HVec3 qp(p-q);
	const HVec3 ab(b-a);
	const HVec3 ac(c-a);
	const HVec3 ap(p-a);



	const HVec3 normal(ab^ac);

	//const C_Normal n(normal);

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(a, n, 0.25f, 0.2f, RGBA(0, 0, 255, 255));


	const HF32 d(qp|normal);
	const HF32 recipD(d.Reciprocal());

	if(d.EQ(zero))
		return false;

	HF32 tTemp(ap|normal);
	/*if(Abs(tTemp).LT(Constants<HF32>::Epsilon()))
		tTemp = zero;*/

	const HF32 t(tTemp*recipD);

	if(t.LT(zero) | t.GT(one))
		return false;

	const HVec3 e(qp^ap);
	const HF32 vTemp(ac|e);
	const HF32 v(vTemp*recipD);

	if(v.LT(zero) | v.GT(one))
		return false;

	const HF32 wTemp(-(ab|e));
	const HF32 w(wTemp*recipD);

	if(w.LT(zero) | w.GT(one))
		return false;

	if((v+w).GT(one))
		return false;

	//Scalar_Bool conD(d.GT(zero));
	////const Maths::HF32 dAbs(d.Abs());
	////Scalar_Bool conD(d.Abs().GT(zero));
	//Scalar_Bool conT(t.GE(zero) & t.LE(d));
	//Scalar_Bool conV(v.GE(zero) & v.LE(d));
	//Scalar_Bool conW(w.GE(zero) &(v+w).LE(d));

	//const HF32 recipD(d.Reciprocal());
	
	//res=p-qp*t*recipD;
	res=p-qp*t;

	return true;
	
	//return (conD & conT & conV & conW);
}


NOALIAS FORCE_INLINE Maths::Scalar_Bool IntersectTriangleSegmentSB(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, 
																   const Maths::C_Normal PCREF _normal, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, Maths::HVec3& res)
{	
	//using namespace Maths;

	//const HF32 zero;//(-Maths::Constants<HF32>::Epsilon2());
	//const HVec3 qp(p-q);
	//const HVec3 ab(b-a);
	//const HVec3 ac(c-a);
	//const HVec3 ap(p-a);

	//const HVec3 normal(ab^ac);
	//
	//const HF32 td(qp|_normal);
	//const HF32 d(td);// + Maths::Constants<Maths::HF32>::Epsilon2());
	//const HF32 t(ap|_normal);
	//const HVec3 e(qp^ap);
	//const HF32 v(ac|e);
	//const HF32 w(-(ab|e));

	//Scalar_Bool conD(d.Abs().GT(zero));//Maths::Constants<Maths::HF32>::Epsilon2()));
	//Scalar_Bool conT(t.GT(zero) & t.LE(d));
	//Scalar_Bool conV(v.GT(zero) & v.LE(d));
	//Scalar_Bool conW(w.GT(zero) & (v+w).LE(d));

	//const HF32 recipD(d.Reciprocal());
	//res=p-qp*t*recipD;
	//return (conD & conT & conV & conW);


	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());//(-Maths::Constants<HF32>::Epsilon2());
	const HF32 one(Constants<HF32>::One());
	const HVec3 qp(p-q);
	const HVec3 ab(b-a);
	const HVec3 ac(c-a);
	const HVec3 ap(p-a);

	const HVec3 normal(ab^ac);
	
	const HF32 td(qp|normal);
	const HF32 d(td);// + Maths::Constants<Maths::HF32>::Epsilon2());
	const HF32 recipD(d.Reciprocal());
	const HF32 t((ap|normal) * recipD);
	const HVec3 e(qp^ap);
	const HF32 v((ac|e) * recipD);
	const HF32 w((-(ab|e)) * recipD);

	Scalar_Bool conD(d.Abs().GT(zero));//Maths::Constants<Maths::HF32>::Epsilon2()));
	Scalar_Bool conT(t.GT(zero) & t.LT(one));
	Scalar_Bool conV(v.GT(zero) & v.LT(one));
	Scalar_Bool conW(w.GT(zero) & (v+w).LT(one));

	res=p-qp*t;
	return (conD & conT & conV & conW);
}


NOALIAS FORCE_INLINE Maths::Scalar_Bool IntersectTriangleSegmentSB(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, Maths::HVec3& res)
{	
	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HVec3 qp(p-q);
	const HVec3 ab(b-a);
	const HVec3 ac(c-a);
	const HVec3 ap(p-a);
	const HVec3 normal(ab^ac);
	const HVec3 e(qp^ap);
	const HF32 d(qp|normal);
	const HF32 recipD(d.Reciprocal());
	const HF32 t((ap|normal) * recipD);
	
	const HF32 v((ac|e)*recipD);
	const HF32 w((-(ab|e))* recipD);
	res=p-qp*t;
	Scalar_Bool conD(d.NE(zero));//Maths::Constants<Maths::HF32>::Epsilon2()));
	Scalar_Bool conT(t.GT(zero) & t.LT(one));
	Scalar_Bool conV(v.GT(zero) & v.LT(one));
	Scalar_Bool conW(w.GT(zero) & (v+w).LT(one));
	return (conD & conT & conV & conW);
}


///////////////////////////////////////////////////////////////////

NOALIAS FORCE_INLINE Maths::Scalar_Bool TriangleSurfaceSweptSphere(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::C_Normal PCREF norm, 
																	const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::C_Normal PCREF direction, const Maths::HF32 PCREF radius, 
																	Maths::HVec3& resultPosition, Maths::HF32& resultDistance, Maths::C_Normal& resultNormal)
{
	using namespace Maths;

	HVec3 result(UNDEFINED);

	if (IntersectTriangleSegmentSB(a + norm * radius, b + norm * radius, c + norm * radius, p, q, result))
	{
		resultPosition = result - norm * radius;
		resultDistance = (direction | (result - p));
		resultNormal = norm;
		return Constants<Scalar_Bool>::True();
	}

	return Constants<Scalar_Bool>::False();
}

NOALIAS FORCE_INLINE Maths::Scalar_Bool EdgeSweptSphere(const C_Edge& edge, 
														const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::C_Normal PCREF direction, const Maths::HF32 PCREF radius, 
														Maths::HVec3& resultPosition, Maths::HF32& resultDistance, Maths::C_Normal& resultNormal)
{

	using namespace Maths;

	C_Normal lineDirection = edge.direction;
	HF32 radiusSquared = radius * radius;

	// solve quadratic for distance
	HVec3 v = edge.vertA - p;

	HF32 d1 = (lineDirection | direction);
	HF32 d2 = (v | lineDirection);
	HF32 d3 = (v | direction);
	HF32 d4 = (v | v);

	HF32 qa = Constants<HF32>::One() - (d1 * d1);
	HF32 qb = Constants<HF32>::Two() * (-d3 + d2 * d1);
	HF32 qc = d4 - d2 * d2 - radiusSquared;

	HF32 x = qb * qb - Constants<HF32>::Four() * qa * qc;

	// no roots - no intersection, or parallel lines
	if (x <= Constants<HF32>::Zero()) 
		return Constants<Scalar_Bool>::False();

	HF32 sx = Sqrt(x);
	HF32 distance = resultDistance;

	// check both roots for minimum
	HF32 r1 = ((-qb + sx) / (Constants<HF32>::Two() * qa));
	distance = Maths::Select(((r1 > Constants<HF32>::Zero()) & (r1 < distance)),
						r1,
						distance);

	HF32 r2 = ((-qb - sx) / (Constants<HF32>::Two() * qa));
	distance = Maths::Select(((r2 > Constants<HF32>::Zero()) & (r2 < distance)),
						r2,
						distance);

	// find normal and intersection point
	if (distance < resultDistance)
	{
		HVec3 centre = p + direction * distance;
		HF32 l = ((centre - edge.vertA) | lineDirection);

		// check whether point is on the line segment
		if (l.GT(Constants<HF32>::Zero()) & (l.LT(edge.edgeLength)))
		{
			resultPosition = edge.vertA + lineDirection * l;
			resultNormal = Maths::C_Normal((centre - resultPosition) / radius, C_Normal::UNIT_LENGTH_VECTOR);
			resultDistance = distance;

			return Constants<Scalar_Bool>::True();
		}
	}

	return Constants<Scalar_Bool>::False();
}

NOALIAS FORCE_INLINE Maths::Scalar_Bool VertexSweptSphere(const Maths::HVec3 PCREF a, 
													const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::C_Normal PCREF direction, const Maths::HF32 PCREF radius, 
													Maths::HVec3& resultPosition, Maths::HF32& resultDistance, Maths::C_Normal& resultNormal)
{
	using namespace Maths;

	HVec3 v = p - a;

	HF32 qb = Constants<HF32>::Two() * (direction | v);
	HF32 qc = (v | v) - radius * radius;

	HF32 x = qb * qb - Maths::Constants<Maths::HF32>::Four() * qc;
	if (x <= Constants<HF32>::Zero()) 
		return Constants<Scalar_Bool>::False();

	HF32 sx = Sqrt(x);
	HF32 distance = resultDistance;

	HF32 r1 = ((-qb + sx) / Constants<HF32>::Two());
	distance = Maths::Select(((r1 > Constants<HF32>::Zero()) & (r1 < distance)),
						r1,
						distance);

	HF32 r2 = ((-qb - sx) / Constants<HF32>::Two());
	distance = Maths::Select(((r2 > Constants<HF32>::Zero()) & (r2 < distance)),
						r2,
						distance);

	// find normal and intersection point
	if (distance < resultDistance)
	{
		HVec3 centre = p + direction * distance;
		resultPosition = a;
		resultNormal = C_Normal((centre - a) / radius, C_Normal::UNIT_LENGTH_VECTOR);
		resultDistance = distance;

		return Constants<Scalar_Bool>::True();
	}

	return Constants<Scalar_Bool>::False();
}

NOALIAS FORCE_INLINE Maths::Scalar_Bool TriangleSweptSphere(const C_RayCastTriangle& triangle,
															const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::C_Normal PCREF direction, const Maths::HF32 PCREF radius, 
															Maths::HVec3& resultPosition, Maths::HF32& resultDistance, Maths::C_Normal& resultNormal)
{
	using namespace Maths;

	Scalar_Bool collision = Constants<Scalar_Bool>::False();

	////Create the triangle!
	//// surface
	//collision |= TriangleSurfaceSweptSphere(triangle.vertA, triangle.vertB, triangle.vertC, triangle.faceNormal, p, q, direction, radius, resultPosition, resultDistance, resultNormal);
	//

	//// edges

	//collision |= EdgeSweptSphere(triangle.m_EdgeA, p, q, direction, radius, resultPosition, resultDistance, resultNormal);
	//collision |= EdgeSweptSphere(triangle.m_EdgeB, p, q, direction, radius, resultPosition, resultDistance, resultNormal);
	//collision |= EdgeSweptSphere(triangle.m_EdgeC, p, q, direction, radius, resultPosition, resultDistance, resultNormal);

	//// vertices
	//collision |= VertexSweptSphere(triangle.vertA, p, q, direction, radius, resultPosition, resultDistance, resultNormal);
	//collision |= VertexSweptSphere(triangle.vertB, p, q, direction, radius, resultPosition, resultDistance, resultNormal);
	//collision |= VertexSweptSphere(triangle.vertC, p, q, direction, radius, resultPosition, resultDistance, resultNormal);

	return collision;
}

////////////////////////////////////////////////////////////////////////////

inline bool IntersectTriangleSegmentLite(const C_RayCastTriangle& triangle, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::C_Normal PCREF direction, const Maths::HF32 PCREF radius, Maths::HVec3& res, Maths::HF32& distance, Maths::C_Normal& resultNormal) 
{
	Maths::HVec3 result(Maths::UNDEFINED);
	if(IntersectTriangleSegmentSB(triangle.vertA + triangle.faceNormal * radius, 
		triangle.vertB + triangle.faceNormal * radius, triangle.vertC + triangle.faceNormal * radius, 
		triangle.faceNormal, p, q, result))
	{
		// Change to capsule trace - changed distance calculation
		res = result - triangle.faceNormal * radius;
		distance = (direction | (result - p));
		resultNormal = triangle.faceNormal;
		return true;
	}
	return false;
}


inline Maths::Scalar_Bool IntersectTriangleSegmentSBCar(const C_RayCastTriangle& triangle, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::C_Normal PCREF direction, const Maths::HF32 PCREF radius, Maths::HVec3& res, Maths::HF32& distance, Maths::C_Normal& resultNormal) 
{
	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());
	const Scalar_Bool bFalse(false);

	//const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon2());
	const HF32 validNormal(Constants<HF32>::Half());
	const HF32 radSq(radius * radius);

	HVec3 result(Maths::UNDEFINED);
	//bool bHit = bFalse;

	
	if(IntersectTriangleSegmentSB(triangle.vertA + triangle.faceNormal * radius, 
		triangle.vertB + triangle.faceNormal * radius, triangle.vertC + triangle.faceNormal * radius, 
		p, q, result))
	{
		// Change to capsule trace - changed distance calculation
		distance = (direction | (result - p));
		if(distance > zero)
		{
			res = result - triangle.faceNormal * radius;
			resultNormal = triangle.faceNormal;
			return !bFalse;
		}
	}

	HVec3 closestLine(UNDEFINED), closestTri(UNDEFINED);
	HF32 sqDist(UNDEFINED);
	Collision::ClosestPtSegmentTriangle(p, q, triangle.vertA, triangle.vertB, triangle.vertC, closestLine, closestTri, sqDist);

	if(sqDist < radSq)
	{
		//OK - definitely hit, but need to solve such that the distance from the line and point of contact
		//is exactly radius!
		HVec3 closestLine2(UNDEFINED);
		Collision::ClosestPtPointSegment(closestTri, p, q + direction * SharedPhysics::Constants::Twenty(), closestLine2);
		const HVec3 resRes(closestLine2 - closestTri);
		const HF32 dd(resRes | resRes);
		const HF32 scaleSq(radSq - dd);
		const HF32 scale(scaleSq.Sqrt());


		const HVec3 rayEnd = closestLine2 - direction * scale;
		const HF32 dis((rayEnd - p).Length());
		if(dis > zero)
		{
			res = closestTri;
			distance = dis;
			resultNormal = Maths::C_Normal(rayEnd - res);
			return !bFalse;
		}
	}
	return bFalse;


	//return TriangleSweptSphere(triangle, p, q, direction, radius, res, distance, resultNormal);
}


inline Maths::Scalar_Bool IntersectTriangleSegmentSB(const C_RayCastTriangle& triangle, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::C_Normal PCREF direction, const Maths::HF32 PCREF radius, Maths::HVec3& res, Maths::HF32& distance, Maths::C_Normal& resultNormal) 
{
	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());
	const Scalar_Bool bTrue(Maths::Constants<Maths::Scalar_Bool>::True());
	const Scalar_Bool bFalse(Maths::Constants<Maths::Scalar_Bool>::False());

	//const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon2());
	const HF32 validNormal(Constants<HF32>::Half());
	const HF32 radSq(radius * radius);

	HVec3 closestLine(UNDEFINED), closestTri(UNDEFINED);
	HF32 sqDist(UNDEFINED);

	HVec3 result = Collision::ClosestPtPointTriangle(p, triangle.vertA, triangle.vertB, triangle.vertC, triangle.faceNormal);

	//(1) - Is it a contact between the initial sphere and the surface, i.e. already in contact!
	const HVec3 sphereVec(result - p);
	const HF32 sphereDis(sphereVec | sphereVec);
	if(sphereDis.LE(radSq))
	{
		distance = sphereDis.Sqrt() - radius;
		res = p;
		resultNormal = triangle.faceNormal;
		return bTrue;
	}


	//2 - then did we hit the triangle surface instead? - I'm sure I can get rid of this check
	// but won't bother for now in case I break something!
	if(IntersectTriangleSegmentSB(triangle.vertA + triangle.faceNormal * radius, 
		triangle.vertB + triangle.faceNormal * radius, triangle.vertC + triangle.faceNormal * radius, 
		triangle.faceNormal, p, q, result))
	{
		// Change to capsule trace - changed distance calculation
		res = result - triangle.faceNormal * radius;
		distance = (direction | (result - p));
		resultNormal = triangle.faceNormal;
		return bTrue;
	}

	
	//Last bit - we didn't actually hit the triangle, so we're going to have to work out where the
	//hit point was!
	Collision::ClosestPtSegmentTriangle(p, q, triangle.vertA, triangle.vertB, triangle.vertC, triangle.faceNormal, closestLine, closestTri, sqDist);

	if(sqDist < radSq)
	{
		HVec3 closestLine2(UNDEFINED);
		Collision::ClosestPtPointSegment(closestTri, p, q + direction * SharedPhysics::Constants::Twenty(), closestLine2);
		const HVec3 resRes(closestLine2 - closestTri);
		const HF32 dd(resRes | resRes);
		const HF32 scaleSq(radSq - dd);
		const HF32 scale(scaleSq.Sqrt());


		const HVec3 rayEnd = closestLine2 - direction * scale;
		res = closestTri;
		distance = (rayEnd - p).Length();
		resultNormal = Maths::C_Normal(rayEnd - res);
		return bTrue;
	}
	return bFalse;


	//return TriangleSweptSphere(triangle, p, q, direction, radius, res, distance, resultNormal);
}

NOALIAS FORCE_INLINE bool PlaneSweptSphere(const Maths::C_Normal PCREF normal, const Maths::HF32 PCREF d,
											const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HF32 PCREF radius,
											Maths::HVec3& contact, Maths::HF32& tValue)
{
	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());
	//const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon());
	//const Maths::HF32 dr(d + radius);
	const HF32 dp((p | normal) - d);
	if(Abs(dp) <= radius)
	{
		tValue = zero;
		contact = p - normal * dp;
		return true;
	}
	//const Maths::HF32 dq((q | normal) - d);
	const HVec3 v(q - p);
	const HF32 dv(v | normal);
	if((dv * dp) >= zero)
	{
		return false;
	}

	const HF32 r(Select(dp.GE(zero), radius, -radius));
	tValue = (r - dp)/dv;
	contact = p + tValue * v - r * normal;
	return tValue.GE(zero) & tValue.LE(Constants<HF32>::One());
}


NOALIAS FORCE_INLINE bool IntersectSegmentCylinder(const Maths::HVec3 PCREF sa, const Maths::HVec3 PCREF sb,
												   const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q,
												   const Maths::HF32 PCREF radius, Maths::HF32& tValue)
{
	using namespace Maths;
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 one(Constants<HF32>::One());
	const HF32 epsilon(Constants<HF32>::Epsilon());
	const HVec3 d(q - p), m(sa - p), n(sb - sa);
	const HF32 md(m | d), nd(n | d), dd(d | d);
	if((md.LT(zero) & (md + nd).LT(zero)) | ((md.GT(dd) &  (md + nd).GT(dd))))
		return false;
	const HF32 nn(n | n), mn(m | n);
	const HF32 a(dd * nn - nd * nd);
	const HF32 k((m | m) - radius * radius);
	const HF32 c(dd * k - md * md);
	if(Maths::Abs(a) < epsilon)
	{
		if(c > zero)
			return false;
		tValue = Select(md.LT(zero), -mn/nn, Select(md.GT(dd), (nd - mn)/nn, zero));
		return true;
	}
	const HF32 b(dd * mn - nd * md);
	const HF32 discr(b * b - a * c);
	if(discr < zero)
		return false;
	tValue = (-b - Maths::Sqrt(discr))/a;
	const Scalar_Bool b1(tValue.GE(zero) & tValue.LE(one));
	const HF32 mdtnd(md + tValue * nd);
	const Scalar_Bool b2(mdtnd.GE(zero) & mdtnd.LE(dd));
	return b1 & b2;
}

NOALIAS FORCE_INLINE bool IntersectSegmentSphere(const Maths::HVec3 PCREF centre, const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HF32 PCREF r, 
												 Maths::HF32& tValue)
{
	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());
	const HVec3 v(q - p);
	const HVec3 m(p - centre);
	const HF32 a(v|v);
	const HF32 b(m|v);
	const HF32 c((m|m)-r*r);
	
	const HF32 discriminant(b*b - a*c);
	
	if(discriminant.LT(zero)) // not intersect
		return false;
	
	const HF32 sqrt(discriminant.Sqrt());
	const HF32 aRecip(a.Reciprocal());
	const HF32 t1((-b-sqrt)*aRecip);
	const HF32 t2((-b+sqrt)*aRecip);
	const Scalar_Bool con(t1.LT(zero));
	tValue = Select(con, t2, t1);
	return true;
}

NOALIAS FORCE_INLINE Maths::Scalar_Bool DetailedSweptSphereTriangle(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c,
													  const Maths::C_Normal PCREF triNormal, const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end,
													  const Maths::C_Normal PCREF direction, const Maths::HF32 PCREF radius, Maths::HVec3& contact, Maths::C_Normal& normal, Maths::HF32& distance)
{
	using namespace Maths;
	const HF32 zero(Constants<HF32>::Zero());
	const Scalar_Bool bFalse(false);


	const HF32 one(Constants<HF32>::One());
	//const Maths::HF32 d(a | triNormal);
	const HF32 validNormal(Constants<HF32>::Half());
	HVec3 intersect(UNDEFINED);
	if(IntersectTriangleSegmentSB(a, b, c, start, end + direction * radius, intersect))
	{
		contact = intersect;
		distance = (direction | (intersect - start)) - radius;
		normal = Maths::Select(Maths::Abs(triNormal | direction).GE(validNormal), triNormal, -direction);
		return !bFalse;
	}

	//Otherwise, we need to do tests against edges/points!
	HF32 tTemp(UNDEFINED), tResult(Constants<HF32>::Max()), t3(UNDEFINED);

	if(IntersectSegmentCylinder(start, end, a, b, radius, tTemp))
	{
		tResult = tTemp;
	}
	if(IntersectSegmentCylinder(start, end, a, c, radius, tTemp))
	{
		tResult = Min(tTemp, tResult);
	}
	if(IntersectSegmentCylinder(start, end, b, c, radius, tTemp))
	{
		tResult = Min(tTemp, tResult);
	}

	if(tResult.GE(zero) & tResult.LE(one))
	{
		//We hit an edge, so this is the point on the sphere. Need to find the closest point on the triangle edge to the point
		//of contact!

		const HVec3 dir((end - start) * tResult);
		const HVec3 sphereCentre(start + dir);
		ClosestPtPointTriangle(sphereCentre, a, b, c, contact);
		normal = Maths::C_Normal(sphereCentre - contact);
		distance = (dir | direction);
		return !bFalse;
	}
	//Final bit - ray-sphere intersection!

	if(IntersectSegmentSphere(a, start, end, radius, tTemp))
		tResult = tTemp;
	if(IntersectSegmentSphere(b, start, end, radius, tTemp))
		tResult = Maths::Min(tResult, tTemp);
	if(IntersectSegmentSphere(c, start, end, radius, tTemp))
		tResult = Maths::Min(tResult, tTemp);

	if(tResult.GE(zero) & tResult.LE(one))
	{
		//We hit an edge, so this is the point on the sphere. Need to find the closest point on the triangle edge to the point
		//of contact!

		const HVec3 dir((end - start) * tResult);
		const HVec3 sphereCentre(start + dir);
		ClosestPtPointTriangle(sphereCentre, a, b, c, contact);
		normal = C_Normal(sphereCentre - contact);
		distance = (dir | direction);
		return !bFalse;
	}
	return bFalse;
}
																

};

#endif 
