#ifndef OBBTRIANGLE_INL
#define OBBTRIANGLE_INL

//#include "gravity/narrowphase/Shapes.h"
//#include "gravity/narrowphase/OptimisedAABBTree.h"
//#include "gravity/narrowphase/CollisionProxy.h"
//#include "gravity/narrowphase/OBBRay.inl"
#include "gravity/narrowphase/GJK.inl"

namespace Collision
{

inline void _ClosestPtPointSegment(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, Maths::HVec3& closest)
{
	using namespace Maths;

	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HF32 one(Maths::Constants<HF32>::One());
	const HVec3 ab=b-a;
	const HF32 _t=((p-a)| ab)/(ab|ab);//Dot(p-a, ab)/Dot(ab, ab);
	const HF32 t=Clamp(_t, zero, one);
	closest=a+(ab*t);
}

#if 0
template <int MaxPatches, int MaxContacts>
bool CollideOBBTriangle(const C_OBB& obb, const HVec3 PCREF a, const HVec3 PCREF b, const HVec3 PCREF c, C_CollisionResult<MaxPatches, MaxContacts>& results)
{
	const HVec3 ca(obb.c);
	const HVec3 e(obb.e);
	const HVec3 ne(-e);
	
	//const HMat33 u(obb.u.InverseRotation());

	const HVec3 na(a-ca);
	const HVec3 nb(b-ca);
	const HVec3 nc(c-ca);

	//translate the triangle from world space to obb's local space
	const HVec3 v0(obb.u*na); 
	const HVec3 v1(obb.u*nb);
	const HVec3 v2(obb.u*nc);

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

	HF32 p0, p1, ra, max, min;
	//Test axis a00=u0 Xf0, all triangle vertexs project onto axis a00
	
	Maths::HVec3 contactPoint(Maths::UNDEFINED);
	Maths::HF32 penetrationDepth(Maths::UNDEFINED);
	//Maths::HF32 zero(Maths::Constants<HF32>::Zero());

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
	const HF32 r(e|absn);
	//const HF32 s=(n|aabb.c)-d;
	if(Abs(d).GT(r))
		return false;

	const Maths::HF32 half(Maths::Constants<HF32>::Half());
	const Maths::HVec3 zero(Maths::Constants<HVec3>::Zero());
	U32 numContacts=0;

	Collision::C_ContactPatch<MaxContacts>& patch=results.m_ContactPatches[0];
	patch.m_Normal = C_Normal(n);
	Collision::C_ContactPoint* contacts=patch.m_ContactPoints;

	numContacts+=(IntersectSegmentBox(v0, v1, e, contacts[numContacts].m_ContactPoint, contacts[numContacts+1].m_ContactPoint)<<1);
	numContacts+=(IntersectSegmentBox(v0, v2, e, contacts[numContacts].m_ContactPoint, contacts[numContacts+1].m_ContactPoint)<<1);
	numContacts+=(IntersectSegmentBox(v1, v2, e, contacts[numContacts].m_ContactPoint, contacts[numContacts+1].m_ContactPoint)<<1);

	for(U32 i=0; i<numContacts; ++i)
	{
		const Maths::HVec3 point(contacts[i].m_ContactPoint);

		const Maths::HVec3 sign(point.X().Sign(), point.Y().Sign(), point.Z().Sign());
		const Maths::HVec3 pickS(e*sign);
		const Maths::HF32 x(pickS.X());
		const Maths::HF32 y(pickS.Y());
		const Maths::HF32 z(pickS.Z());
		const Maths::HVec3 pick1(-x, y, z);
		const Maths::HVec3 pick2(x, -y, z);
		const Maths::HVec3 pick3(x, y, -z);

		const Maths::HF32 s1(pick1|patch.m_Normal);
		const Maths::HF32 s2(pick2|patch.m_Normal);
		const Maths::HF32 s3(pick3|patch.m_Normal);

		const Maths::Scalar_Bool con1(s1.GT(s2) & s1.GT(s3));
		const Maths::Scalar_Bool con2(s2.GT(s3));

		const Maths::HVec3 pickE=Select(con1, pick1, Select(con2, pick2, pick3));

		Maths::HVec3 closest(Maths::UNDEFINED);
		_ClosestPtPointSegment(point, pickS, pickE, closest);

		const Maths::HVec3 dif(closest - point);
		contacts[i].m_PenetrationDepth=(dif|patch.m_Normal);

	}

	const Maths::HF32 x(e.X());
	const Maths::HF32 y(e.Y());
	const Maths::HF32 z(e.Z());


	const Maths::HVec3 verts[8] = 
	{
	 	 e,					HVec3( x, y, -z),	HVec3( x, -y, z),	HVec3(-x, y, z),
		 HVec3(x, -y, -z),	HVec3(-x, y, -z),	HVec3(-x, -y, z),	-e
	};

	numContacts+=(U32)IntersectTriangleBoxSegment(v0, v1, v2, n, verts[0], verts[1], patch.m_Normal, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth); 
	numContacts+=(U32)IntersectTriangleBoxSegment(v0, v1, v2, n, verts[0], verts[2], patch.m_Normal, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth); 
	numContacts+=(U32)IntersectTriangleBoxSegment(v0, v1, v2, n, verts[0], verts[3], patch.m_Normal, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth); 
	numContacts+=(U32)IntersectTriangleBoxSegment(v0, v1, v2, n, verts[1], verts[4], patch.m_Normal, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth); 
	numContacts+=(U32)IntersectTriangleBoxSegment(v0, v1, v2, n, verts[1], verts[5], patch.m_Normal, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth);
	numContacts+=(U32)IntersectTriangleBoxSegment(v0, v1, v2, n, verts[2], verts[4], patch.m_Normal, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth);
	numContacts+=(U32)IntersectTriangleBoxSegment(v0, v1, v2, n, verts[2], verts[6], patch.m_Normal, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth);
	numContacts+=(U32)IntersectTriangleBoxSegment(v0, v1, v2, n, verts[3], verts[5], patch.m_Normal, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth);
	numContacts+=(U32)IntersectTriangleBoxSegment(v0, v1, v2, n, verts[3], verts[6], patch.m_Normal, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth);
	numContacts+=(U32)IntersectTriangleBoxSegment(v0, v1, v2, n, verts[4], verts[7], patch.m_Normal, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth);
	numContacts+=(U32)IntersectTriangleBoxSegment(v0, v1, v2, n, verts[5], verts[7], patch.m_Normal, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth);
	numContacts+=(U32)IntersectTriangleBoxSegment(v0, v1, v2, n, verts[6], verts[7], patch.m_Normal, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth);

	patch.m_Normal = patch.m_Normal * obb.u;
	for(U32 i=0; i<numContacts; ++i)
	{
		const Maths::HVec3 point(contacts[i].m_ContactPoint);
		const Maths::HVec3 transPoint(obb.c + contacts[i].m_ContactPoint * obb.u);
		contacts[i].m_ContactPoint = transPoint;
	}


	//const Maths::C_Normal nNormal(-patch.m_Normal);

	return true;
}
#endif

NOALIAS inline U32 IntersectSegmentBox(const Maths::HVec3 PCREF p, const Maths::HVec3 PCREF q, const Maths::HVec3 PCREF extents, Maths::HVec3& minContacts, Maths::HVec3& maxContacts, const U32 triangleIndex, const U32 edgeIndex, U32& hashCodeA, U32& hashCodeB)
{
	using namespace Maths;

	const HVec3 d(q - p);
	const HVec3 ood(d.Reciprocal());
	const HVec3 epsilon(Maths::Constants<HVec3>::Epsilon2());
	const HVec3 _max(Maths::Constants<HVec3>::One());
	const HVec3 _min(Maths::Constants<HVec3>::Zero());
	
	//Check to see if any of the extents are outside of the box...
	const HVec3 absD(Abs(d));
	const Vector3_Bool bSmall(absD < epsilon);
	//const Vector3_Bool bOutside((bSmall) & ((p < -extents) | (p > extents)));
	const Vector3_Bool bOutside((bSmall) & ((Abs(p) > extents)));
	
	const HVec3 _t1(((-extents) - p) * ood);
	const HVec3 _t2((extents - p) * ood);
	const Vector3_Bool vb(_t1 < _t2);
	const HVec3 t11(Select(vb, _t1, _t2));
	const HVec3 t21(Select(vb, _t2, _t1));

	const HVec3 t1(Select(bSmall, _min, Max(t11, _min)));
	const HVec3 t2(Select(bSmall, _max, Min(t21, _max)));

	const HF32 t1x(t1.X());
	const HF32 t1y(t1.Y());
	const HF32 t1z(t1.Z());
	const HF32 t2x(t2.X());
	const HF32 t2y(t2.Y());
	const HF32 t2z(t2.Z());

	const HF32 tmin(Maths::Max(t1x, Maths::Max(t1y, t1z)));
	const HF32 tmax(Maths::Min(t2x, Maths::Max(t2y, t2z)));

	/*const Scalar_Bool sb1(t1y.GT(t1z));
	const HF32 tmin_tmp(Select(sb1, t1y, t1z));
	const Scalar_Bool sb2(tmin_tmp.GT(t1x));
	const HF32 tmin(Select(sb2, tmin_tmp, t1x));

	const Scalar_Bool sb3(t2y.LT(t2z));
	const HF32 tmax_tmp=Select(sb3, t2y, t2z);
	const Scalar_Bool sb4(tmax_tmp.LT(t2x));
	const HF32 tmax=Select(sb4, tmax_tmp, t2x);*/
	
	//Work out what the contact point is...
	minContacts = p + d * tmin;
	maxContacts = p + d * tmax;
	hashCodeA = ((triangleIndex << 4) | (edgeIndex << 1)) & 0x7fffffff;
	hashCodeB = ((triangleIndex << 4) | (edgeIndex << 1) | 1) & 0x7fffffff;
	const Scalar_Bool sb(X(bOutside) | Y(bOutside) | Z(bOutside));
	return ((tmin.LT(tmax)) & (!sb)) != 0;
}

inline Maths::HF32 _PenDepth(const Maths::HVec3 PCREF extents, const Maths::HVec3 PCREF point, const Maths::C_Normal PCREF normal)
{
	const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
	const Maths::HVec3 end(point + normal * extents * two);
	Maths::HVec3 minContact(Maths::UNDEFINED), maxContact(Maths::UNDEFINED);
	Collision::IntersectSegmentBox(point, end, extents, minContact, maxContact);
	return (maxContact - point) | normal;
}

template <int MaxContacts>
bool CollideOBBTriangle(const C_OBB& obb, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c, const Maths::HVec3 PCREF normal, C_ContactPatch<MaxContacts>& patch, const Collision::AABBTreeTriangle& triangle, const U32 triangleIndex, const U32 physicsMaterial, const U32 effectMaterial)
{
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, b, c, Maths::RGBA(0, 0, 255));
	//Draw the triangle!!!
	using namespace Maths;

	if(patch.m_NumContactPoints >= (MaxContacts - 1))
		return true;

	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
	const HVec3 ca(obb.Centre());
	const HVec3 e(obb.Extent());
	const HMat33& u(obb.Orientation());
	
	//Translate triangle as conceptually moving AABB to origin
	const HVec3 v0(u * (a-ca)); 
	const HVec3 v1(u * (b-ca));
	const HVec3 v2(u * (c-ca));

	const HVec3 max012(Max(v0, Max(v1, v2)));
	const HVec3 min012(Min(v0, Min(v1, v2)));
	//Test axis u[0]=(1,0,0), u[1]={0,1,0}, u[2]={0, 0, 1}
	if(Maths::AnyTrue3((max012 < (-e) | min012 > (e))))
		return false;

	//Compute edge vectors for triangle
	const HVec3 f0(v1-v0);
	const HVec3 f1(v2-v1);
	const HVec3 f2(v0-v2);
	

	const HVec3 absF0(Maths::Abs(f0));
	const HVec3 crossP00(v0 ^ f0);
	const HVec3 crossP01(v2 ^ f0);

	const HVec3 crossRA0(e.Y()*absF0.Z() + e.Z()*absF0.Y(),
			e.X()*absF0.Z()+e.Z()*absF0.X(),
			e.X()*absF0.Y()+e.Y()*absF0.X());

	const Maths::HVec3 vMin0(Maths::Min(crossP00, crossP01));
	const Maths::HVec3 vMax0(Maths::Max(crossP00, crossP01));

	if(Maths::AnyTrue3((vMax0 < (-crossRA0)) | (vMin0 > crossRA0)))
		return false;

	const HVec3 absF1(Maths::Abs(f1));
	//Test axis a01=u0 X f1, all triangle vertexs project onto axis a01
	const HVec3 crossP10(v0^f1);
	const HVec3 crossP11(v1^f1);

	const HVec3 crossRA1(e.Y()*absF1.Z() + e.Z()*absF1.Y(),
			e.X()*absF1.Z()+e.Z()*absF1.X(),
			e.X()*absF1.Y()+e.Y()*absF1.X());


	const HVec3 vMax1(Max(crossP10, crossP11));
	const HVec3 vMin1(Min(crossP10, crossP11));

	if(Maths::AnyTrue3((vMax1 < (-crossRA1)) | (vMin1 > crossRA1)))
		return false;


	const HVec3 absF2(Maths::Abs(f2));
	//Test axis a01=u0 X f1, all triangle vertexs project onto axis a01
	const HVec3 crossP20(v0^f2);
	const HVec3 crossP21(v1^f2);


	const HVec3 vMax2(Max(crossP20, crossP21));
	const HVec3 vMin2(Min(crossP20, crossP21));

	const HVec3 crossRA2(e.Y()*absF2.Z() + e.Z()*absF2.Y(),
			e.X()*absF2.Z()+e.Z()*absF2.X(),
			e.X()*absF2.Y()+e.Y()*absF2.X());

	if(Maths::AnyTrue3((vMax2 < (-crossRA2)) | (vMin2 > crossRA2)))
		return false;
	
	//const Maths::C_Normal norm(u * (-patch.m_Normal));
	const Maths::C_Normal norm(u * (-normal));
	const HF32 d(norm|v0);
	const C_Normal absn(Abs(norm.m_V), C_Normal::UNIT_LENGTH_VECTOR);
	const HF32 r(e|absn);
	//const HF32 s=(n|aabb.c)-d;
	if(Abs(d).GT(r))
		return false;

	const HF32 x(e.X());
	const HF32 y(e.Y());
	const HF32 z(e.Z());
	const HF32 pen(r - Abs(d));

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(a, b, c, Maths::RGBA(0, 255, 0));

	/*const Maths::HVec3 verts[8] = 
	{
	 	 e,					HVec3( x, y, -z),	HVec3( x, -y, z),	HVec3(-x, y, z),
		 HVec3(x, -y, -z),	HVec3(-x, y, -z),	HVec3(-x, -y, z),	-e
	};*/

	//Re-organised, so that 
	const HVec3 verts[8] = 
	{
		-e, HVec3(x, -y, -z), HVec3(-x, y, -z), HVec3(x, y, -z),
		HVec3(-x, -y, z), HVec3(x, -y, z), HVec3(-x, y, z), e
	};


	//const HF32 half(Constants<HF32>::Half());
	const HVec3 zero(Constants<HVec3>::Zero());
	const HF32 _zero(Constants<HF32>::Zero());
	U32& numContacts=patch.m_NumContactPoints;
	U32 oldContacts=patch.m_NumContactPoints;

	//const Maths::C_Normal norm(n);
	//patch.m_Normal = norm;
	Collision::C_ContactPoint* contacts=patch.m_ContactPoints;
	if(triangle.ActiveEdge(0))
	{
		//assert(numContacts < MaxContacts);
		if(numContacts < (MaxContacts-1))
		{
			numContacts+=(IntersectSegmentBox(v0, v1, e, contacts[numContacts].m_ContactPoint, contacts[numContacts+1].m_ContactPoint, triangleIndex, 1, contacts[numContacts].m_HashValue, contacts[numContacts + 1].m_HashValue)<<1);
		}
	}
	if(triangle.ActiveEdge(1))
	{
		//assert(numContacts < MaxContacts);
		if(numContacts < (MaxContacts-1))
		{
			numContacts+=(IntersectSegmentBox(v0, v2, e, contacts[numContacts].m_ContactPoint, contacts[numContacts+1].m_ContactPoint, triangleIndex, 2, contacts[numContacts].m_HashValue, contacts[numContacts + 1].m_HashValue)<<1);
		}
	}
	if(triangle.ActiveEdge(2))
	{
		//assert(numContacts < MaxContacts);
		if(numContacts < (MaxContacts-1))
		{
			numContacts+=(IntersectSegmentBox(v1, v2, e, contacts[numContacts].m_ContactPoint, contacts[numContacts+1].m_ContactPoint, triangleIndex, 3, contacts[numContacts].m_HashValue, contacts[numContacts + 1].m_HashValue)<<1);
		}
	}

	for(U32 i=oldContacts; i<numContacts; ++i)
	{
		contacts[i].m_PenetrationDepth = 0.f;//_zero;//_PenDepth(obb.e, contacts[i].m_ContactPoint, -patch.m_Normal);
	}

	const U32 baseHashValue = 0x8000000 | ((triangleIndex << 6) & 0x7fffffff);

	for(U32 i = 0; i < 7; ++i)
	{
		const U32 right(i | 1);
		const U32 up(i | 2);
		const U32 back(i | 4);
		Vector3_Bool bAbove((verts[i] | norm).GE(d));
		
		if(i != right)
		{
			if(numContacts >= MaxContacts)
				break;
			BIZ_ASSERT(numContacts < MaxContacts);
			//Select the start and end points of these...
			const Maths::HVec3 pA(Select(bAbove, verts[i], verts[right]));
			const Maths::HVec3 pB(Select(bAbove, verts[right], verts[i]));
			contacts[numContacts].m_HashValue = baseHashValue | (i << 3) | right;
			numContacts += (U32)IntersectTriangleBoxSegment(v0, v1, v2, pA, pB, 
				norm, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth);
		}

		if(i != up)
		{
			if(numContacts >= MaxContacts)
				break;
			BIZ_ASSERT(numContacts < MaxContacts);
			//Select the start and end points of these...
			const Maths::HVec3 pA(Select(bAbove, verts[i], verts[up]));
			const Maths::HVec3 pB(Select(bAbove, verts[up], verts[i]));
			contacts[numContacts].m_HashValue = baseHashValue | (i << 3) | up;
			numContacts += (U32)IntersectTriangleBoxSegment(v0, v1, v2, pA, pB, 
				norm, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth);

		}

		if(i != back)
		{
			if(numContacts >= MaxContacts)
				break;
			BIZ_ASSERT(numContacts < MaxContacts);
			//Select the start and end points of these...
			const Maths::HVec3 pA(Select(bAbove, verts[i], verts[back]));
			const Maths::HVec3 pB(Select(bAbove, verts[back], verts[i]));
			contacts[numContacts].m_HashValue = baseHashValue | (i << 3) | back;
			numContacts += (U32)IntersectTriangleBoxSegment(v0, v1, v2, pA, pB, 
				norm, contacts[numContacts].m_ContactPoint, contacts[numContacts].m_PenetrationDepth);
		}
	}
		
	for(U32 i=oldContacts; i<numContacts; ++i)
	{
		const HVec3 point(contacts[i].m_ContactPoint);
		const HVec3 transPoint(obb.Centre() + contacts[i].m_ContactPoint * obb.Orientation());
		contacts[i].m_ContactPoint = transPoint;
		contacts[i].m_PhysicsMaterialIndices = physicsMaterial;
		contacts[i].m_EffectMaterialIndices = effectMaterial;
		//contacts[i].m_PenetrationDepth = pen;
	}

	//patch.m_Normal = patch.m_Normal * obb.u;
	//patch.m_NumContactPoints=numContacts;

	return (patch.m_NumContactPoints != oldContacts);
}

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideOBBTriangleMesh(C_BoxProxy& proxy, const MathsStorage::Fixed3_S16<14>* const __restrict verts, 
							const MathsStorage::HenDN3 * const __restrict normals, 
							const Triangle* const __restrict tris, const U32 numTris, 
							const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, 
							const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags,
							const Maths::HVec3& low, const Maths::HVec3& high)
{
	using namespace Maths;

	bool collide=false;
	const HF32 disEps(SharedPhysics::Constants::CloseNormal());
	const C_OBB& obb=proxy.m_Box;
	const HF32 zero(Constants<HF32>::Zero());
	const HVec3 pos(proxy.m_PosRot.Translation());
	const HMat33 orientation(proxy.m_PosRot.Rotation());
	MathsStorage::HenDN3 dNorm;
	const HVec3 range(high - low);
	
	for(U32 i=0; i<numTris; ++i)
	{
		Spu::MemBaseMarker marker;
		//const Triangle* tempTri=(const Triangle*)Spu::FetchAllocUnaligned(tris + i, sizeof(Triangle));
		const Triangle* tempTri = &tris[i];
		//((U32&)dNorm) = Spu::Get( (U32*)normals + tempTri->m_NormalIndex);
		dNorm = tempTri->m_SurfaceNormal;

		const HVec3 localVertA(&verts[tempTri->m_IndexA ]);
		const HVec3 localVertB(&verts[tempTri->m_IndexB ]);
		const HVec3 localVertC(&verts[tempTri->m_IndexC ]);

		const HVec3 a(localVertA*range + low);
		const HVec3 b(localVertB*range + low);
		const HVec3 c(localVertC*range + low);

		//const Maths::HVec3& normal(tris[i].m_Normal);
		HVec3 normal = -Load<HVec3, MathsStorage::HenDN3>(&dNorm);
		const HF32 dotA(a | normal);
		const HF32 dotCentre((obb.Centre() | normal) - dotA);
		if(dotCentre > zero)
			continue; 

		if(results.m_NumContactPatches >= MaxPatches)
			break;
		C_ContactPatch<MaxContacts>* pPatch = &results.m_ContactPatches[results.m_NumContactPatches];
		pPatch->m_NumContactPoints = 0;
		pPatch->m_Normal = Maths::C_Normal(normal*orientation, Maths::C_Normal::UNIT_LENGTH_VECTOR);
		bool incr = true;
		for(U32 j = 0; j < results.m_NumContactPatches; ++j)
		{
			HF32 dis(results.m_ContactPatches[j].m_Normal.m_V| pPatch->m_Normal);
			if(dis >= disEps)
			{
				pPatch = &results.m_ContactPatches[j];
				incr = false;
				break;
			}
		}
		U32 numContacts = pPatch->m_NumContactPoints;
		bool newCollider=CollideOBBTriangle(obb, a, b, c, normal, *pPatch, tris[i], triangleStartIndex + i, physicsMaterial, effectMaterial);

		for(U32 k = numContacts; k < pPatch->m_NumContactPoints; ++k)
		{
			C_ContactPoint& point(pPatch->m_ContactPoints[k]);
			point.m_ContactPoint = (point.m_ContactPoint * orientation) + pos;
		}

		results.m_NumContactPatches += (newCollider && incr);
		collide |= newCollider;
			
	}
	return collide;
}

//template <int MaxPatches, int MaxContacts, class Triangle>
//bool CollideOBBTriangleMesh(C_Shape* shape, const MathsStorage::Float3* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, 
//							MaxContacts>& results, const U32 physicsMaterial, const U32 effectMaterial,
//							const U32 collisionFlags)
template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideOBBTriangleMesh(C_Shape* shape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, 
							const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, 
							MaxContacts>& results, const U32 physicsMaterial, const U32 effectMaterial,
							const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
{
	using namespace Maths;

	Spu::MemBaseMarker _marker;
	bool collide=false;
	const HF32 disEps(SharedPhysics::Constants::CloseNormal());
	//C_BoxShape* pBoxShape = (C_BoxShape*)Spu::FetchAlloc(shape, sizeof(C_BoxShape));
	C_BoxShape* pBoxShape = (C_BoxShape*)shape;
	C_OBB& obb=pBoxShape->m_Box;
	const HF32 zero(Constants<HF32>::Zero());
	MathsStorage::HenDN3 dNorm;

	const Maths::HVec3 range(high - low);
	for(U32 i=0; i<numTris; ++i)
	{
		Spu::MemBaseMarker marker;
		//Triangle* tri = (Triangle*)Spu::FetchAllocUnaligned(tris + i, sizeof(Triangle));
		const Triangle* tri = &tris[i];
		//((U32&)dNorm) = Spu::Get((U32*)(normals + tri->m_NormalIndex));
		dNorm = tri->m_SurfaceNormal;
		HVec3 normal = -Load<HVec3, MathsStorage::HenDN3>(&dNorm);
		/*if((normal | normal) == zero)
			continue;*/

		/*MathsStorage::Float3* f0 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexA, sizeof(MathsStorage::Float3));
		const HVec3 a=Load<HVec3, MathsStorage::Float3>(f0);*/
		
		const HVec3 localVertA(&verts[tri->m_IndexA ]);
		const HVec3 a(localVertA*range + low);

		const HF32 dotA(a | normal);
		const HF32 dotCentre((obb.Centre() | normal) - dotA);
		if(dotCentre > zero)
			continue;

		/*MathsStorage::Float3* f1 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexB, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f2 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexC, sizeof(MathsStorage::Float3));

		const HVec3 b=Load<HVec3, MathsStorage::Float3>(f1);
		const HVec3 c=Load<HVec3, MathsStorage::Float3>(f2);*/

		const HVec3 localVertB(&verts[tri->m_IndexB ]);
		const HVec3 b(localVertB*range + low);

		const HVec3 localVertC(&verts[tri->m_IndexC ]);
		const HVec3 c(localVertC*range + low);

		C_ContactPatch<MaxContacts>* pPatch(NULL);
		U32 incr = 0;

		if(results.m_NumContactPatches < MaxPatches)
		{
			pPatch= &results.m_ContactPatches[results.m_NumContactPatches];
			pPatch->m_NumContactPoints = 0;
			pPatch->m_Normal = Maths::C_Normal(normal, Maths::C_Normal::UNIT_LENGTH_VECTOR);
			incr = 1;
		}

		for(U32 j = 0; j < results.m_NumContactPatches; ++j)
		{
			HF32 dis(results.m_ContactPatches[j].m_Normal.m_V| normal);
			if(dis >= disEps)
			{
				pPatch = &results.m_ContactPatches[j];
				incr = 0;
				break;
			}
		}
		if(pPatch == NULL)
			continue;
		//results.m_NumContactPatches += incr;
		bool newCollider=CollideOBBTriangle(obb, a, b, c, normal, *pPatch, tris[i], triangleStartIndex + i, physicsMaterial, effectMaterial);
		if(newCollider && (incr > 0))
		{
			results.m_NumContactPatches++;
		}
		collide = collide || newCollider;

		//if(newCollider)
		//{
		//	U32 numContacts = pPatch->m_NumContactPoints;

		//	for(U32 i=0; i<numContacts; ++i)
		//	{
		//		C_ContactPoint& point(pPatch->m_ContactPoints[i]);
		//		/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(point.m_ContactPoint, 0.25f, Maths::RGBA(0,0, 255));
		//		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(point.m_ContactPoint, pPatch->m_Normal, point.m_PenetrationDepth, 0.025f, Maths::RGBA(255, 0, 0));*/
		//	}

		//	/*DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(a, 0.25f, RGBA(0, 255, 0));
		//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(b, 0.25f, RGBA(0, 255, 0));
		//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(c, 0.25f, RGBA(0, 255, 0));

		//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(a, b, c, RGBA(0, 255, 0));;*/
		//}
			
	}

	return collide;
}

template <class Triangle>
bool ContinuousOBBTriangleMesh(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, 
							   const U32 triangleStartIndex, const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, 
							   const Maths::HVec3& low, const Maths::HVec3& high)
{
	using namespace Maths;

	bool collide=false;
	//const HF32 disEps(SharedPhysics::Constants::CloseNormal());
	C_ContinuousShape* pContinuousShape = (C_ContinuousShape*)Spu::FetchAlloc(pShape, sizeof(C_ContinuousShape));
	C_BoxShape* pBoxShape = (C_BoxShape*)Spu::FetchAlloc(pContinuousShape->m_pContinuousPrimitive, sizeof(C_BoxShape));
	C_OBB& obb=pBoxShape->m_Box;
	const HF32 zero(Constants<HF32>::Zero());
	const HVec3 range(high - low);
	
	//const HVec3 bodyDt(results.m_pObjectB->m_VectorTravelled);
	MathsStorage::HenDN3 dNorm;
	for(U32 i=0; i<numTris; ++i)
	{
		Spu::MemBaseMarker marker;
		//Triangle* tri = (Triangle*)Spu::FetchAllocUnaligned(tris + i, sizeof(Triangle));
		const Triangle* tri = &tris[i];
		//((U32&)dNorm) = Spu::Get((U32*)(normals + tri->m_NormalIndex));
		dNorm = tri->m_SurfaceNormal;
		HVec3 normal = -Load<HVec3, MathsStorage::HenDN3>(&dNorm);
		assert((normal | normal) > zero);

		/*MathsStorage::Float3* f0 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexA, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f1 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexB, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f2 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexC, sizeof(MathsStorage::Float3));

		Collision::C_Triangle triangle;
		triangle.m_VertexA=Load<HVec3, MathsStorage::Float3>(f0);
		triangle.m_VertexB=Load<HVec3, MathsStorage::Float3>(f1);
		triangle.m_VertexC=Load<HVec3, MathsStorage::Float3>(f2);*/

		const HVec3 localA(&verts[tri->m_IndexA ]);
		const HVec3 localB(&verts[tri->m_IndexB ]);
		const HVec3 localC(&verts[tri->m_IndexC ]);

		Collision::C_Triangle triangle;
		triangle.m_VertexA = localA*range + low;
		triangle.m_VertexB = localB*range + low;
		triangle.m_VertexC = localC*range + low;


		lambda = zero;

		if(Collision::GJKRayCast<Collision::C_OBB, Collision::C_Triangle>(obb, triangle, r, maxLambda, lambda, nor, closestA, closestB))
		{
			nor = normal;
			maxLambda = lambda;
			collide = true;
		}
			
	}

	return collide;

}


};

#endif
