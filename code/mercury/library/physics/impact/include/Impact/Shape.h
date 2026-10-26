#ifndef __PHYSICS_SHAPE_H__
#define __PHYSICS_SHAPE_H__

namespace Impact
{

class Body;
class Shape;

enum E_ShapeType
{
	E_ShapeType_ConvexHull,
	E_ShapeType_TriMesh,
	E_ShapeType_Sphere,
	E_ShapeType_CompoundShape,
};

/// a base-shape containing an Instance function
class BaseShape
{
public:
	/// get instance
	virtual Shape *Instance() = 0;
};


/// instance of a shape, containing run-time information such as location
class Shape
{
public:
							/// virtual destructor
	virtual					~Shape() {}

							/// update this shape's position/orientation
	void					UpdateOrientation(const Maths::HVec3 &vPos, const Maths::C_Quaternion &qRotation);

	Maths::C_AxisBox		GetAABB(Body *pBody);

	E_ShapeType				m_type;
	Maths::HVec3			m_vPos;
	Maths::C_Quaternion		m_qRotation;
	bool					m_bDirty;
};

};

#endif
