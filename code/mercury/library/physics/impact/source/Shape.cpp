#include "stdafx.h"

#include "Impact\ContactUID.h"
#include "Impact\CollisionUtil.h"
#include "Impact\ConvexBase.h"
#include "Impact\PersistentData.h"
#include "Impact\I_TriMesh.h"
#include "Impact\ConvexHull.h"
#include "Impact\Sphere.h"
#include "Impact\CompoundShape.h"

using namespace Maths;

namespace Impact
{

/// update this shape's position/orientation
void Shape::UpdateOrientation(const Maths::HVec3 &vPos, const Maths::C_Quaternion &qRotation)
{
	m_bDirty = true;
	m_vPos = vPos;
	m_qRotation = qRotation;
}

/// call the GetAABB function on the correct shape type
Maths::C_AxisBox Shape::GetAABB(Body *pBody)
{
	switch (m_type)
	{
	case E_ShapeType_TriMesh:		return ((I_TriMesh*)this)->GetAABB(pBody);
	case E_ShapeType_ConvexHull:	return ((ConvexHullInstance*)this)->GetAABB(pBody);
	case E_ShapeType_Sphere:		return ((SphereInstance*)this)->GetAABB(pBody);
	case E_ShapeType_CompoundShape:	return ((CompoundShapeInstance*)this)->GetAABB(pBody);
	}

	assert(0);
	return C_AxisBox();
}


}; // namespace Impact
