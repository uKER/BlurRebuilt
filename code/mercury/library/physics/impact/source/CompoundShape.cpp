#include "stdafx.h"

#include "Impact\Body.h"
#include "Impact\ContactUID.h"
#include "Impact\PersistentData.h"
#include "Impact\Collision.h"
#include "Impact\PersistentData.h"
#include "Impact\CollisionPair.h"
#include "Impact\Shape.h"
#include "Impact\CompoundShape.h"
#include "Impact\CollisionUtil.h"
#include <System/Debug/I_DebugRender.h>

using namespace Maths;

namespace Impact
{

/// CompoundShapeInstance constructor
CompoundShape::CompoundShape()
	: m_pBoundingShape(NULL)
{
}

/// destructor
CompoundShape::~CompoundShape()
{
}

/// get instance of shape
Shape *CompoundShape::Instance()
{
	CompoundShapeInstance *pInstance = new CompoundShapeInstance();
	for (int i=0; i<m_Shapes.Size(); i++)
		pInstance->AddShape(m_Shapes[i]);

	pInstance->SetBoundingShape(m_pBoundingShape);

	return pInstance;
}

/// add a sub-shape
void CompoundShape::AddShape(BaseShape *p)
{
	m_Shapes.PushBack(p);
}

/// add an bounding test shape
void CompoundShape::SetBoundingShape(BaseShape *p)
{
	m_pBoundingShape = p;
}

/// CompoundShapeInstance constructor
CompoundShapeInstance::CompoundShapeInstance()
	: m_pBoundingShape(NULL)
	, m_bGeneratedCachedBoundingBox(false)
{
	m_type = E_ShapeType_CompoundShape;
}

/// add a sub-shape
void CompoundShapeInstance::AddShape(BaseShape *p)
{
	m_Shapes.PushBack(p->Instance());
}

/// set the bounding shape
void CompoundShapeInstance::SetBoundingShape(BaseShape *p)
{
	if (p)
		m_pBoundingShape = p->Instance();
}

/// destructor
CompoundShapeInstance::~CompoundShapeInstance()
{
	for (int i=0; i<m_Shapes.Size(); i++)
		delete m_Shapes[i];
	delete m_pBoundingShape;
}

/// get bounding box
Maths::C_AxisBox CompoundShapeInstance::GetAABB(Body *pBody)
{
	if (!m_bGeneratedCachedBoundingBox)
	{
		m_bGeneratedCachedBoundingBox = true;

		C_AxisBox rtn;
		Body dummyBody;
		dummyBody.m_vPos = HVec3(0,0,0);
		dummyBody.m_rotation = C_Quaternion(1,0,0,0);
		for (int i=0; i<m_Shapes.Size(); i++)
			rtn.UpdateExtents(m_Shapes[i]->GetAABB(&dummyBody));

		m_CachedLocalBBox = rtn;
	}

	return TransformBBox(m_CachedLocalBBox, pBody->m_vPos, pBody->m_rotation);
}

// CompoundCollision constructor
CompoundCollision::CompoundCollision()
{
}

/// destructor
CompoundCollision::~CompoundCollision()
{
	for (int i=0; i<m_Shapes.Size(); i++)
		delete m_Shapes[i];
	delete m_pBoundingShape;
}

Debug::Float g_AntiStuckHack("physics.conephysics.antistuckconehack", 1.f, Debug::Range(0.f, 10.f));


/// calculate contact points between a body and a compound shape
int CompoundCollision::Collide(ContactVector &contacts, Body *p[2], Shape *pShape[2])
{
	CompoundShapeInstance *pCompoundShape = (CompoundShapeInstance*)pShape[0];

	// generate an empty array of collision classes, one for each shape
	int n = pCompoundShape->m_Shapes.Size();
	if (!m_Shapes.Size())
	{
		m_Shapes.Resize(pCompoundShape->m_Shapes.Size());
		for (int i=0; i<n; i++)
			m_Shapes[i] = NULL;

		m_pBoundingShape = NULL;
	}

	// propagate dirty flag to sub-shapes
	if (pShape[0]->m_bDirty)
	{
		for (int i=0; i<n; i++)
			pCompoundShape->m_Shapes[i]->UpdateOrientation(pShape[0]->m_vPos, pShape[0]->m_qRotation);

		if (pCompoundShape->m_pBoundingShape)
			pCompoundShape->m_pBoundingShape->UpdateOrientation(pShape[0]->m_vPos, pShape[0]->m_qRotation);
	}

	// perform bounding collision
	if (pCompoundShape->m_pBoundingShape)
	{
		Shape *pShapes[2] = { pCompoundShape->m_pBoundingShape, pShape[1] };
		ContactVector dummy;
		if (::Impact::Collide(dummy, p, pShapes, &m_pBoundingShape) == 0)
			return 0;
	}

	// perform collision
	for (int i=0; i<n; i++)
	{
		Shape *pShapes[2] = { pCompoundShape->m_Shapes[i], pShape[1] };
		::Impact::Collide(contacts, p, pShapes, &m_Shapes[i]);
	}

	// [Jan] HACK!!!! This is to prevent cones from getting stuck inside the car
	// if we detect lots of contacts, assume that the cone is stuck inside the car and apply an upwards impulse
	if (contacts.Size() > 7)
	{
		p[1]->m_vPos += HVec3(0.f,(float)g_AntiStuckHack,0.f);
	}
/*
	// if we have a lot of  contacts
	if (contacts.Size() > 4)
	{
		LOGDEBUG( g_Log, "Lot of contacts: %d\n", contacts.Size());
		for (int i=0; i<contacts.Size(); i++)
		{
			LOGDEBUG( g_Log, "pos %5.2f %5.2f %5.2f nrm %5.2f %5.2f %5.2f imp %5.2f sep %5.2f\n",
				(float)contacts[i].m_vPos.X(), (float)contacts[i].m_vPos.Y(), (float)contacts[i].m_vPos.Z(),
				(float)contacts[i].m_vNormal.X(), (float)contacts[i].m_vNormal.Y(), (float)contacts[i].m_vNormal.Z(),
				contacts[i].m_fAccNormalImpulse,
				contacts[i].m_fSeparation);

			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(contacts[i].m_vPos, contacts[i].m_vNormal, -contacts[i].m_fSeparation* 10.f, .2f, RGBA(0,255,255));
		}
	}
*/
	return contacts.Size();
}


}; // namespace Impact
