#include "stdafx.h"
#include <camera2/export/AftkToCameraScene.h>
#include <camera2/export/AftkToAnimData.h>

#pragma warning (push,3)
#include <aftk/Aftk.h>
#pragma warning (pop)
#include <camera2/tv/CameraScene.h>
#include <camera2/tv/AnimData.h>
#include <camera2/tv/TvZone.h>

namespace Cam2
{

static C_ZoneSpline AftkToZoneSpline(Aftk::C_SceneSplineNode *node)
{
	S32 max = node->GetNumCVs();
	Aftk::Matrix44 matrix = node->GetWorldMatrix();

	C_ZoneSpline spline;

	S32 i = 0;
	F32 distance = 0.f;
	
	Aftk::Position3 lastPos(0.f, 0.f, 0.f);
	
	for (i = 0; i < max; i++)
	{
		Aftk::Position3 vect = Aftk::ApplyMatrix(matrix,node->GetCV(i));
		
		if (i > 0)
			distance += Magnitude(vect - lastPos);

		C_ZoneSpline::Cv cv;
		cv.v = Maths::HVec3(vect.x, vect.y, vect.z);
		cv.alpha = distance;
		spline.m_cvs.PushBack(cv);

		lastPos = vect;
	}
	
	if (distance > 1e-6)
	{
		for (i = 0; i < spline.m_cvs.Size(); i++)
		{
			spline.m_cvs[i].alpha /= distance;
		}
	}	

	return spline;
}

// Export a 3D camera zone:
static C_TvZone AftkToTvZone(Aftk::C_SceneMeshNode* mesh)
{
	C_TvZone zone;

	S32 numVert = mesh->GetNumVertexPositions();
	Aftk::Matrix44 matrix = mesh->GetWorldMatrix();

	// Export vertices:
	S32 i = 0;
	for (i = 0; i < numVert; i++)
	{
		Aftk::Position3 pt = mesh->GetVertexPosition(i);
		pt = Aftk::ApplyMatrix(matrix, pt);
		
		zone.m_point.PushBack(Maths::HVec3(pt.x, pt.y, pt.z));
	}

	S32 numPoly = mesh->GetNumPolygons(0);
	for (i = 0; i < numPoly; i++)
	{
		Aftk::C_Polygon& poly = mesh->GetPolygon(0, i);

		// Write out 4 vertices (or 3 + first one again if the polygon is a triangle)
		S32 npv = poly.GetNumPolygonVertices();
		
		S32 n = 0;
		for (n = 0; n < 4; n++)
		{
			S32 r = 0;
			if (n < npv)
				r = n; // Vertex to read
			
			Aftk::C_PolygonVertex& vertex = poly.GetPolygonVertex(r);
			S32 index = vertex.GetPositionIndex();
			zone.m_face.PushBack(index);
		}
	}

	return zone;
}

static void AddNode(Aftk::C_Scene* scene, Aftk::C_SceneNode* node, S32 whichType, C_CameraScene& outScene, S32 cameraStart = 0, S32 cameraEnd = 0)
{
	Aftk::C_SceneNode* source = node;
	// If it's an instance, find the source node:
	if (node->GetType() == Aftk::E_ObjType_SceneInstance)
	{
		source = ((Aftk::C_SceneInstanceNode *)node)->GetSourceNodePtr();
	}

	S32 type = source->GetType();

	if (whichType == 0 && type == Aftk::E_ObjType_SceneCamera)
	{
		C_AnimData* animData = new C_AnimData;
		if (animData)
		{
			AftkToAnimData(scene, source, node, animData);
			outScene.m_cams.PushBack(animData);
		}
	}

	if (whichType == 1 && type == Aftk::E_ObjType_SceneMesh)
	{
		C_TvZone* zone = new C_TvZone;
		if (zone)
		{
			*zone = AftkToTvZone((Aftk::C_SceneMeshNode*)source);
			// Fill in which cameras from the camera list are associated with this zone:
			zone->m_cameraStart = cameraStart;
			zone->m_cameraCount = cameraEnd - cameraStart;
			outScene.m_zones.PushBack(zone);
		}
	}

	if (whichType == 2 && type == Aftk::E_ObjType_SceneSpline)
	{
		C_ZoneSpline spline = AftkToZoneSpline((Aftk::C_SceneSplineNode*)node);

		if (outScene.m_zones.Size() >= 1)
			outScene.m_zones.Back()->m_spline = spline;
	}
	
	// Recursively add items in children
	S32 max = node->GetNumChildren();
	S32 i = 0;
	for (i = 0; i < max; i++)
	{
		AddNode(scene, node->GetChildPtr(i), whichType, outScene, cameraStart, cameraEnd);
	}
}

void AftkToCameraScene(Aftk::C_Scene* scene, Aftk::C_SceneNode* topNode, C_CameraScene& outScene)
{
	if (topNode == 0)
		return;
	
	S32 i = 0;
	for (i = 0; i < (S32)topNode->GetNumChildren(); i++)
	{
		Aftk::C_SceneNode* group = topNode->GetChildPtr(i);

		S32 cameraStart = outScene.m_cams.Size(); // Marker for camera number at the start of the group
		AddNode(scene, group, 0, outScene); // Cameras
		S32 cameraEnd = outScene.m_cams.Size(); // Marker for camera number at the end of the group
		
		AddNode(scene, group, 1, outScene, cameraStart, cameraEnd); // Zones
		AddNode(scene, group, 2, outScene, cameraStart, cameraEnd); // Zone splines
	}
}

} // end of namespace
