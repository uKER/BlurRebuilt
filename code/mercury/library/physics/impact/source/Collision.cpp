#include "stdafx.h"

#include "Impact\ContactUID.h"
#include "Impact\Body.h"
#include "Impact\Collision.h"
#include "Impact\CollisionUtil.h"
#include "Impact\GJK.h"
#include "Impact\SAT.h"
#include "Impact\ConvexHull.h"
#include "Impact\ConvexTri.h"
#include "Impact\PersistentData.h"
#include "Impact\CompoundShape.h"
#include "Impact\TriMeshCollision.h"
#include "Impact\SphereCollision.h"
#include "System\Memory\MemoryTracker.h"
#include <system/debug/I_DebugRender.h>

using namespace Maths;

namespace Impact
{

extern Debug::Logger &g_Log;

Debug::Bool g_bDebugRenderManifold("Impact.DebugRenderManifold", false);
Debug::Bool g_bUseNewCollision("Impact.UseNewCollision", true);

void DisplayContacts(ContactVector &contacts)
{
/*
	for (int i=0; i<contacts.Size(); i++)
	{
//		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(contacts[i].m_vPos, contacts[i].m_vNormal, contacts[i].m_fSeparation, .2f, RGBA(0,255,255));

		Util::C_String text;
		text.Printf("pos %5.2f %5.2f %5.2f nrm %5.2f %5.2f %5.2f sep %5.2f",  
			(float)contacts[i].m_vPos.X(), (float)contacts[i].m_vPos.Y(), (float)contacts[i].m_vPos.Z(),
			(float)contacts[i].m_vNormal.X(), (float)contacts[i].m_vNormal.Y(), (float)contacts[i].m_vNormal.Z(),
			contacts[i].m_fSeparation);

//		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(50, 180+25*i), text);

		LOGDEBUG( g_Log, "contact %s\n", text.Chars());
	}
*/
}

int CollideHull(ContactVector &contacts, ConvexHullInstance *pHull[2], I_PersistentData **ppPersist)
{
	HullPersistentData *pPersist = GetPersist<HullPersistentData>(ppPersist);

	pHull[0]->ApplyTransform();
	pHull[1]->ApplyTransform();

	// use GJK to determine whether the bodies are intersecting
	GJK gjk(pHull[0], pHull[1]);
	bool bResult = gjk.Process(pPersist);
	if (!bResult)
		return 0;

	// use SAT to determine the penetration depth and contact points
	SAT sat(pHull[0], pHull[1]);
	SAT::Result satResult;
	sat.Process(satResult, pPersist);

	C_ClipPolygon incidentPoly, tmp;
	pHull[satResult.iReference^1]->GetPolygon(satResult.iPlane[satResult.iReference^1], incidentPoly);

	// get refplane and side-plane info
	int iRefPlane = satResult.iPlane[satResult.iReference];
	ConvexHullInstance *pRefHull = pHull[satResult.iReference];
	const C_Plane &refPlane = pRefHull->GetPlane(iRefPlane);
	C_ClipPolygon clippedPolygon;

	ClipPolyToPlanes(clippedPolygon, pRefHull, incidentPoly, iRefPlane);

	EmitContactPoints(contacts, clippedPolygon, refPlane, satResult.iReference);

	return contacts.Size();
}

// create a persistent collision object of the given type and call it's _Collide_ function
template<typename T>
int Collide(ContactVector &contacts, Body *pBody[2], Shape *pShape[2], I_PersistentData **ppPersist)
{
	if (!*ppPersist)
		*ppPersist = new T();

	return ((T*)(*ppPersist))->Collide(contacts, pBody, pShape);
}

/// main function for calculating contact points between two bodies
int Collide(ContactVector &contacts, Body *pBody1, Body *pBody2, I_PersistentData **ppPersist)
{
	Body *pBody[2] = { pBody1, pBody2 };
	Shape *pShape[2] = { pBody1->GetShape(), pBody2->GetShape() };
	return Collide(contacts, pBody, pShape, ppPersist);
}

/// main function for calculating contact points between two bodies
int Collide(ContactVector &contacts, Body *pBody[2], Shape *pShape[2], I_PersistentData **ppPersist)
{
	Body *pBodySwapped[2] = { pBody[1], pBody[0] };
	Shape *pShapeSwapped[2] = { pShape[1], pShape[0] };

	if (pShape[0]->m_type == E_ShapeType_ConvexHull &&
		pShape[1]->m_type == E_ShapeType_ConvexHull)
	{
		// convex hull vs convex hull
		return CollideHull(contacts, (ConvexHullInstance**)pShape, ppPersist);	// TODO: convert CollideHull to a class
		// TODO: this triggers a crashbug - obviously it's broken code but should be investigated really
//		for (int i=0; i<contacts.Size(); i++)
//			contacts[i].m_vNormal = -contacts[i].m_vNormal;
	}
	else if (pShape[0]->m_type == E_ShapeType_TriMesh && pShape[1]->m_type == E_ShapeType_ConvexHull)
	{
		// trimesh vs convex hull
		return Collide<TriMeshCollision>(contacts, pBody, pShape, ppPersist);
	}
	else if (pShape[1]->m_type == E_ShapeType_TriMesh && pShape[0]->m_type == E_ShapeType_ConvexHull)
	{
		// convex hull vs trimesh
		int i0 = contacts.Size();
		int rtn = Collide<TriMeshCollision>(contacts, pBodySwapped, pShapeSwapped, ppPersist);
		for (int i=i0; i<contacts.Size(); i++)
		{
			contacts[i].m_vNormal = -contacts[i].m_vNormal;
		}
		return rtn;
	}
	else if (pShape[0]->m_type == E_ShapeType_Sphere && pShape[1]->m_type == E_ShapeType_ConvexHull)
	{
		// sphere vs convex hull
		return Collide<SphereCollision>(contacts, pBody, pShape, ppPersist);
	}
	else if (pShape[1]->m_type == E_ShapeType_Sphere && pShape[0]->m_type == E_ShapeType_ConvexHull)
	{
		// convex hull vs sphere
		int rtn = Collide<SphereCollision>(contacts, pBodySwapped, pShapeSwapped, ppPersist);
		for (int i=0; i<contacts.Size(); i++)
		{
			contacts[i].m_vNormal = -contacts[i].m_vNormal;
		}
		return rtn;
	}
	else if (pShape[0]->m_type == E_ShapeType_CompoundShape)
	{
		return Collide<CompoundCollision>(contacts, pBody, pShape, ppPersist);
	}
	else if (pShape[1]->m_type == E_ShapeType_CompoundShape)
	{
		int i0 = contacts.Size();
		int rtn = Collide<CompoundCollision>(contacts, pBodySwapped, pShapeSwapped, ppPersist);
		for (int i=i0; i<contacts.Size(); i++)
		{
			contacts[i].m_vNormal = -contacts[i].m_vNormal;
		}
		return rtn;
	}

	return 0;
}

};
