#ifndef _NEW_PRIMITIVE_H
#define _NEW_PRIMITIVE_H

//using namespace Maths;

#include "gravity/narrowphase/ConvexHull.h"
#include "AxisBox.h"

//#include "Auxiliary.h"
namespace Collision
{

class C_BoxProxy;
class C_CapsuleProxy;
class C_CylinderProxy;
class C_ConeProxy;

enum ConvexHullType
{
	CH_TYPE_AABB,			
	CH_TYPE_SPHERE,
	CH_TYPE_CONE,
	CH_TYPE_CYLINDER,
	CH_TYPE_MINKOWSKISUM	//support mapping is Sa-b(V)=Sa(V)-Sb(-V)
};


class C_Sphere : public C_Convex
{

public:
	C_Sphere() : m_Radius(Maths::UNDEFINED), C_Convex(E_CONVEX_SPHERE)
	{
	}

	C_Sphere(const Maths::HVec3 PCREF _c, const Maths::HF32 PCREF _r) : m_Radius(_r), C_Convex(E_CONVEX_SPHERE, _c)
	{
		Margin(_r);
		//WarmStart(_c);
	}

	C_Sphere(C_CRAxisBox& box) : C_Convex(E_CONVEX_SPHERE, box.centre), m_Radius(Maths::UNDEFINED)
	{
		//m_Centre = box.centre;
		m_Radius = box.extents.Length();
	}

	virtual ~C_Sphere()
	{
	}

	NOALIAS inline bool QuadraticShape() const
	{
		return true;
	}

	NOALIAS inline const Maths::HF32 Radius() const
	{
		return m_Radius;
	}

	NOALIAS inline void Radius(const Maths::HF32 PCREF r)
	{
		m_Radius = r;
		Margin(r);
	}

	NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF dir) const;

	NOALIAS inline Maths::HVec3 SupportMargin(const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF margin) const;

private:
	Maths::HF32 m_Radius;
};

//This class of AABB is for the GJK algorithms
class C_AABB : public C_Convex
{
public:
	C_AABB() : C_Convex(E_CONVEX_AABB), m_Radius(Maths::UNDEFINED)
	{}

	virtual ~C_AABB()
	{}
	

	NOALIAS inline Maths::HVec3 Radius() const
	{
		return m_Radius;
	}

	NOALIAS inline void Radius(const Maths::HVec3 PCREF radius)
	{
		m_Radius = radius;
	}

	//static Maths::HVec3 Support(const ConvexHull& hull, const Maths::HVec3 PCREF dir);
	NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF dir) const;

	NOALIAS inline Maths::HVec3 SupportMargin(const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF margin) const;


	NOALIAS inline C_AABB Transform(Maths::HMat43 PCREF mat) const __restrict
	{
		C_AABB result;
		result.m_Radius = AbsTransform( m_Radius, mat.Rotation() );
		result.m_Centre = m_Centre * mat;
		result.WarmStart(result.m_Centre);
		return result;
	}
private:
	Maths::HVec3 m_Radius;


};

class C_AABBMinMax
{
public:
	Maths::HVec3 m_min;
	Maths::HVec3 m_max;
};


class C_Plane
{
public:
	C_Plane() : m_n(Maths::UNDEFINED), m_d(Maths::UNDEFINED){}
	C_Plane(const Maths::C_Normal PCREF n, const Maths::HF32 PCREF d): m_n(n), m_d(d){}
	Maths::C_Normal m_n;
	Maths::HF32 m_d;
	Maths::HF32 Distance( const Maths::HVec3 PCREF vertex )
	{
		return (m_n | vertex) - m_d;
	}

	void ProjectQuat(C_CRAxisBox&  axisBox, Maths::HVec3& aVex, Maths::HVec3& bVex, Maths::HVec3& cVex, Maths::HVec3& dVex);
};

class C_FinitePlane
{
public:
	C_FinitePlane() : m_Normal(Maths::UNDEFINED), m_Height(Maths::UNDEFINED), m_Start(Maths::UNDEFINED), m_End(Maths::UNDEFINED)
	{
	}

	~C_FinitePlane()
	{
	}
	Maths::HVec3 m_Normal;
	Maths::HF32 m_Height;
	Maths::HVec3 m_Start;
	Maths::HVec3 m_End;
};

class C_OBB : public C_Convex
{
public:
	C_OBB(const C_OBB& obb): m_Orientation(obb.m_Orientation), m_Extent(obb.m_Extent), C_Convex(E_CONVEX_OBB, obb.m_Centre)
	{
		WarmStart(obb.m_Centre);
	}
	C_OBB(): C_Convex(E_CONVEX_OBB), m_Extent(Maths::UNDEFINED), m_Orientation(Maths::UNDEFINED)
	{
	}

	virtual ~C_OBB(){}
	


	//static Maths::HVec3 Support(const ConvexHull& hull, const Maths::HVec3 PCREF dir);

	NOALIAS inline Maths::HVec3 Extent() const
	{
		return m_Extent;
	}

	NOALIAS inline Maths::HMat33 Orientation() const
	{
		return m_Orientation;
	}


	NOALIAS inline void Extent(const Maths::HVec3 PCREF extent) 
	{
		m_Extent = extent;
	}

	NOALIAS inline void Orientation(const Maths::HMat33& orientation)
	{
		m_Orientation = orientation;
	}

	NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF dir) const;
	NOALIAS inline Maths::HVec3 SupportMargin(const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF margin) const;

	friend class C_BoxProxy;

private:
	Maths::HMat33 m_Orientation;//local x-, y-, z- axes
	Maths::HVec3  m_Extent;
};

class C_Capsule : public C_Convex
{
public:
	C_Capsule() : C_Convex(E_CONVEX_CAPSULE), m_Start(Maths::UNDEFINED), m_End(Maths::UNDEFINED),
		m_LocalStart(Maths::UNDEFINED), m_LocalEnd(Maths::UNDEFINED), m_Radius(Maths::UNDEFINED)
	{}
	virtual ~C_Capsule()
	{}


	NOALIAS inline bool QuadraticShape() const
	{
		return true;
	}

	NOALIAS inline const void LocalStart(const Maths::HVec3 PCREF start)
	{
		m_LocalStart=start;
	}

	NOALIAS inline const void LocalEnd(const Maths::HVec3 PCREF end)
	{
		m_LocalEnd=end;
	}

	NOALIAS inline const Maths::HVec3 LocalEnd() const
	{
		return m_LocalEnd;
	}

	NOALIAS inline const Maths::HVec3 LocalStart() const
	{
		return m_LocalStart;
	}

	NOALIAS inline const Maths::HVec3 Start() const
	{
		return m_Start;
	}

	NOALIAS inline const Maths::HVec3 End() const
	{
		return m_End;
	}

	NOALIAS inline const Maths::HF32 Radius() const
	{
		return m_Radius;
	}

	NOALIAS inline void Radius(const Maths::HF32 PCREF r)
	{
		m_Radius = r;
		Margin(r);
	}

	NOALIAS inline void End(const Maths::HVec3 PCREF end)
	{
		m_End = end;
	}

	NOALIAS inline void Start(const Maths::HVec3 PCREF start)
	{
		m_Start = start;
	}

	//static Maths::HVec3 Support(const ConvexHull& hull, const Maths::HVec3 PCREF dir);

	NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF dir) const;
	NOALIAS inline Maths::HVec3 SupportMargin(const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF margin) const;

	friend class C_CapsuleProxy;
private:
	Maths::HVec3 m_Start; //start point
	Maths::HVec3 m_End;	//end point
	Maths::HF32  m_Radius;		//radius
	Maths::HVec3 m_LocalStart;
	Maths::HVec3 m_LocalEnd;
};

class C_Cylinder: public C_Convex
{
public:
	C_Cylinder(): C_Convex(E_CONVEX_CYLINDER),  m_CentreAxis(Maths::UNDEFINED), m_halfHeight(Maths::UNDEFINED),
		m_Radius(Maths::UNDEFINED)
	{}
	virtual ~C_Cylinder()
	{}


	NOALIAS inline const void CentreAxis(const Maths::HVec3 PCREF centreAxis)
	{
		m_CentreAxis=centreAxis;
	}

	NOALIAS inline const Maths::HVec3 CentreAxis() const 
	{
		return m_CentreAxis;
	}

	NOALIAS inline const Maths::HVec3 LocalCentreAxis() const 
	{
		return Maths::Constants<Maths::HVec3>::UnitAxisPositiveY();
	}

	NOALIAS inline const void HalfHeight(const Maths::HF32 PCREF halfHeight)
	{
		m_halfHeight=halfHeight;
	}

	NOALIAS inline const Maths::HF32 HalfHeight() const
	{
		return m_halfHeight;
	}

	NOALIAS inline const Maths::HF32 Radius() const
	{
		return m_Radius;
	}

	NOALIAS inline void Radius(const Maths::HF32 PCREF r)
	{
		m_Radius = r;
		Margin(r);
	}

	NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF dir) const;
	NOALIAS inline Maths::HVec3 SupportMargin(const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF margin) const;

	friend class C_CylinderProxy;

private:
	Maths::C_Normal m_CentreAxis;
	Maths::HF32 m_halfHeight;	//half height
	Maths::HF32 m_Radius;		//radius
};


class C_Cone : public C_Convex
{
public:
	C_Cone() : C_Convex(E_CONVEX_CONE), m_CenterOfMass(Maths::UNDEFINED),m_OffSet(Maths::UNDEFINED),
	m_Rotation(Maths::UNDEFINED), m_LocalCenterAxis(Maths::UNDEFINED), m_MidPointAxis(Maths::UNDEFINED),
	m_BaseRadius(Maths::UNDEFINED), m_Height(Maths::UNDEFINED), m_SinTheta(Maths::UNDEFINED)
	{
	}

	virtual ~C_Cone()
	{
	}


	NOALIAS inline void SetUpCone(const Maths::HVec3 PCREF centerOfMass, const Maths::HVec3 PCREF offSet, const Maths::C_Normal PCREF localCenterAxis, const Maths::HF32 PCREF baseRadius, const Maths::HF32 PCREF height)
	{
		using namespace Maths;

		m_CenterOfMass = centerOfMass;
		m_OffSet = offSet;
		m_LocalCenterAxis = localCenterAxis;
		m_BaseRadius = baseRadius;
		m_Height = height;

		const HF32 sqBaseRadius = baseRadius * baseRadius;
		const HF32 sqHeight = height * height;
		const HF32 demo = Sqrt(sqBaseRadius + sqHeight);
		m_SinTheta = baseRadius * Rcp(demo);
	}

	NOALIAS inline void CentreOfMass(const Maths::HVec3 PCREF centre)
	{
		m_CenterOfMass = centre;
	}

	NOALIAS inline Maths::HVec3 CentreOfMass()
	{
		return m_CenterOfMass;
	}

	NOALIAS inline void MidPointAxis(const Maths::C_Normal PCREF midPointAxis)
	{
		m_MidPointAxis= midPointAxis;
	}

	NOALIAS inline void Rotation(const Maths::HMat33& rotation) 
	{
		m_Rotation = rotation;
	}

	NOALIAS inline Maths::C_Normal MidPointAxis() const
	{
		return m_MidPointAxis;
	}

	NOALIAS inline Maths::HVec3 Tip() const
	{
		return m_Centre + m_MidPointAxis*m_Height*Maths::Constants<Maths::HF32>::Half();
	}

	NOALIAS inline Maths::HVec3 Bottom() const
	{
		return m_Centre - m_MidPointAxis*m_Height*Maths::Constants<Maths::HF32>::Half();
	}

	NOALIAS inline Maths::HVec3 OffSet() const
	{
		return m_OffSet;
	}

	NOALIAS inline Maths::HF32 Height() const
	{
		return m_Height;
	}

	NOALIAS inline Maths::HF32 BaseRadius() const
	{
		return m_BaseRadius;
	}

	NOALIAS inline Maths::HF32 SinTheta() const
	{
		return m_SinTheta;
	}

	NOALIAS inline Maths::C_Normal LocalCenterAxis() const
	{
		return m_LocalCenterAxis;
	}

	NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF dir) const;
	NOALIAS inline Maths::HVec3 SupportMargin(const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF margin) const;

	friend class C_ConeProxy;

private:
	Maths::HVec3 m_CenterOfMass; //the position use to calculate inertia tensor
	Maths::HVec3 m_OffSet;
	Maths::HMat33 m_Rotation;
	Maths::C_Normal m_LocalCenterAxis;
	Maths::C_Normal m_MidPointAxis;
	Maths::HF32 m_BaseRadius;
	Maths::HF32 m_Height;
	Maths::HF32 m_SinTheta;
	//Maths::HVec3 m_CenterV;

};

enum E_Particle_Status
{
	E_Particle_Alive,
	E_Particle_Dead
};


class C_Edge
{
public:
	Maths::HVec3 vertA, vertB;
	Maths::C_Normal direction;
	Maths::HF32 edgeLength;

	C_Edge() : vertA(Maths::UNDEFINED), vertB(Maths::UNDEFINED), direction(Maths::UNDEFINED), 
		edgeLength(Maths::UNDEFINED)
	{
	}

	C_Edge(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b) : vertA(a), vertB(b),
		direction(Maths::UNDEFINED), edgeLength(Maths::UNDEFINED)
	{
		const Maths::HVec3 ba(b - a);
		edgeLength = Maths::Length(ba);
		direction = Maths::C_Normal(ba/edgeLength, Maths::C_Normal::UNIT_LENGTH_VECTOR);
	}
};

class C_RayCastTriangle
{
public:
	Maths::HVec3 vertA, vertB, vertC;
	//C_Edge m_EdgeA, m_EdgeB, m_EdgeC;
	Maths::C_Normal faceNormal;
	Maths::HF32 distance;

	C_RayCastTriangle(const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c,
		const Maths::C_Normal PCREF normal) : vertA(a), vertB(b), vertC(c), faceNormal(normal), distance(normal | a)
	{
	}
};

};

//#include "gravity/narrowphase/Primitive.inl"

#endif
