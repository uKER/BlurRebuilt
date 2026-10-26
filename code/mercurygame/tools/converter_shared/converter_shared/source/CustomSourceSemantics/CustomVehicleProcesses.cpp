#include "stdafx.h"

#include <converter_shared/CustomSourceSemantics/CustomVehicleProcesses.h>
#include <AftkUt/AftkUtNodeFlagParser.h>
#include <converter_shared/helpers/AftkHelpers.h>

namespace Vehicle
{

namespace Maintenance
{

/* 
===============================================================
					class C_ModelsToElements

NOTES: Converts models incorrectly defined in Art to elements
===============================================================
*/

/*
===============
NAME: IsInteresting

NOTE: 
===============
*/
bool C_ModelsToElements::IsInteresting(Aftk::C_SceneNode* p_pSceneNode)
{
	if (p_pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh || 
		p_pSceneNode->GetType() == Aftk::E_ObjType_SceneNode)
		return true;

	return false;
}

/*
===============
NAME: Visit

NOTE: 
===============
*/

static bool HasBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& rkAttributeName)
{
	assert(pPolygon);
	const Aftk::C_SceneMeshNode* pSceneMeshNode = pPolygon->GetMeshNodePtr();
	const Aftk::C_Scene* pAftkScene = pSceneMeshNode->GetScenePtr();
	const S16 iBlindDataMappingIndex = pPolygon->GetBlindDataMappingIndex();
	const Aftk::C_BlindDataResource* pBlindDataResource = pSceneMeshNode->GetPolygonBlindDataMappingPtr(iBlindDataMappingIndex);

	if(pBlindDataResource)
	{
		const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindDataResource->GetContent();

		for (U32 i=0; i<aftkBlindDataNode.GetNumAttributes(); ++i) 
		{
			const Aftk::C_GenericAttribute& aftkAttribute = aftkBlindDataNode.GetAttribute(i); 
			if (!stricmp(aftkAttribute.GetName().c_str(), rkAttributeName.Chars()))
				return true;
		}
	}  

	return false;
}

/// Add an attribute to the polygons blind data.
static void AddBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& rkAttributeName)
{
	// Make sure this attribute doesn't already exist in polygon's blind data.
	if(HasBlindDataAttribute(pPolygon, rkAttributeName))
		return;

	Aftk::C_SceneMeshNode* pSceneMeshNode = pPolygon->GetMeshNodePtr();
	Aftk::C_Scene* pAftkScene = pSceneMeshNode->GetScenePtr();
	S16 iBlindDataMappingIndex = pPolygon->GetBlindDataMappingIndex();
	const Aftk::C_BlindDataResource* pBlindDataResource = pSceneMeshNode->GetPolygonBlindDataMappingPtr(iBlindDataMappingIndex);
	if(pBlindDataResource)
	{
		const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindDataResource->GetContent();
		Aftk::C_BlindDataNode newBlindDataNode(aftkBlindDataNode);	// Replicate the blind data node.
	
		// Add additional generic attribute from argument.
		Aftk::C_GenericAttribute newAttribute;
		newAttribute.SetName(rkAttributeName.Chars());
		newBlindDataNode.AddAttribute(newAttribute);

		// Add data to the node and set the index on polygon.
 		const Aftk::C_BlindDataResource* pResource = pAftkScene->AddBlindDataNode(newBlindDataNode);
 		S16 iBlindDataMapping = pSceneMeshNode->AddPolygonBlindDataMapping(pResource);
 		pPolygon->SetBlindDataMappingIndex(iBlindDataMapping);
 	}
	else
	{
		Aftk::C_BlindDataNode newBlindDataNode;	// No duplication on create.
	
		// Add additional generic attribute from argument.
		Aftk::C_GenericAttribute newAttribute;
		newAttribute.SetName(rkAttributeName.Chars());
		newBlindDataNode.AddAttribute(newAttribute);

		// Add data to the node and set the index on polygon.
		const Aftk::C_BlindDataResource* pResource = pAftkScene->AddBlindDataNode(newBlindDataNode);
		S16 iBlindDataMapping = pSceneMeshNode->AddPolygonBlindDataMapping(pResource);
		pPolygon->SetBlindDataMappingIndex(iBlindDataMapping);
	}

	return;
}

void AddNoPVSActorProperty(Aftk::C_SceneNode *pNode, bool bInvisible, bool bNoPVS, bool bNoLight, bool bNoShadows, bool bDoubleSided)
{
	/*
	AftkUt::C_NodeFlagParser actorParser(*pNode, NULL, false);

	actorParser.SetParameterValue("DoNotPVS", "1", AftkUt::E_ParamType_Bool);

	*/
	if (pNode->GetType() == Aftk::E_ObjType_SceneMesh)
	{
		Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)pNode;

		U32 iPolygonCount = pMesh->GetNumPolygons(0);
		for (U32 iCurrentPoly=0; iCurrentPoly<iPolygonCount; ++iCurrentPoly)
		{
			Aftk::C_Polygon *pCurrentPolygon = &pMesh->GetPolygon(iCurrentPoly);
			const S16 iBlindDataIndex = pCurrentPolygon->GetBlindDataMappingIndex();

			// Add any additional flags here:
			if (bInvisible)
				AddBlindDataAttribute(pCurrentPolygon, "polySet_invisible");

			if (bNoPVS)
				AddBlindDataAttribute(pCurrentPolygon, "polySet_doNotPVS");

			if (bNoLight)
				AddBlindDataAttribute(pCurrentPolygon, "polySet_doNotLight");

			if (bNoShadows)
				AddBlindDataAttribute(pCurrentPolygon, "polySet_doNotCastShadows");

			if (bDoubleSided)
				AddBlindDataAttribute(pCurrentPolygon, "polySet_doubleSided");
		}
	}

	for (U32 i=0; i<pNode->GetNumChildren(); i++)
	{
		AddNoPVSActorProperty(pNode->GetChildPtr(i), bInvisible, bNoPVS, bNoLight, bNoShadows, false);
	}
};



Aftk::C_Status C_ModelsToElements::Visit(Aftk::C_SceneNode* p_pNode, C_NullMemoryManager& memoryManager)
{
	Util::C_String name = p_pNode->GetName().c_str();
	name.ConvertToLower();
	if (name.Length() < 2)
		return Aftk::C_Status::E_Status_Success;

	if (name.ISearch("_bone") != -1 ||
		name == "m_collisionmesh")
		AddNoPVSActorProperty(p_pNode, true, true, true, true, false);

	if (name == "m_shadowcaster")
		AddNoPVSActorProperty(p_pNode, false, true, true, true, false);

	if (name.ISearch("window_") != -1 ||
		name.ISearch("HeadLightCover") != -1 ||
		name.ISearch("wiper_front") != -1 )
		AddNoPVSActorProperty(p_pNode, false, false, false, true, false);

	if (name.ISearch("_alloyblur") != -1)
		AddNoPVSActorProperty(p_pNode, false, false, false, true, true);

	if (name == "m_collisionmesh" || 
		name == "m_shadowcaster" || 
		//name == "m_exterior" || 
		//name == "m_interior" || 
		name == "m_alloy" || 
		//name == "m_wheellod" || 
		//name == "m_door" || 
		//name == "m_boot" || 
		//name == "m_wheels" || 
		//name.ISearch("m_wheel_") != -1 ||
		//name.ISearch("m_bumper_") != -1 ||
		//name.ISearch("m_door_") != -1 ||
		//name.ISearch("m_bonnet_") != -1 ||
		//name.ISearch("m_wing_") != -1 ||
		name.ISearch("_bone") != -1)
		return Aftk::C_Status::E_Status_Success;

	/*if (name.Search("m_signal") != -1 || name.Search("e_signal") != -1 ||
		name.Search("m_running") != -1 || name.Search("e_running") != -1)
	{
		name.NonConstChars()[0] = 'x';
		p_pNode->SetName(name.Chars());
	}
	else */if (name.Chars()[0] == 'm' && name.Chars()[1] == '_')
	{
		name.NonConstChars()[0] = 'e';
		p_pNode->SetName(name.Chars());
	}

	return Aftk::C_Status::E_Status_Success;
}

} // namespace Maintenance

struct LodInfo
{
	const char *pszName;
	int iDistance;
	bool bMirrorView;
	const char *pszLayer;
};

#define NEAR_LOD_DISTANCE			10
#define MEDIUM_LOD_DISTANCE			20
#define FAR_LOD_DISTANCE			50

LodInfo g_LODTable[] =
{
	{ "_interior",				MEDIUM_LOD_DISTANCE,	false },
	
	{ "_headlightcover_",		NEAR_LOD_DISTANCE,		false },
	{ "_brakelight_",			FAR_LOD_DISTANCE,		false },
	{ "_reverselight_",			MEDIUM_LOD_DISTANCE,	false },
	{ "_rearlight_",			FAR_LOD_DISTANCE,		false },
	{ "_headlight_",			FAR_LOD_DISTANCE,		false },
	{ "_foglight_",				MEDIUM_LOD_DISTANCE,	false },
	{ "_signallight_",			FAR_LOD_DISTANCE,		false },
	{ "_runninglight_",			FAR_LOD_DISTANCE,		false },
	
	{ "_wiper_",				MEDIUM_LOD_DISTANCE,	false },	
	{ "_exhaust_",				NEAR_LOD_DISTANCE,		false },
	{ "_mirror_",				MEDIUM_LOD_DISTANCE,	false },

	{ "_steeringwheel",			NEAR_LOD_DISTANCE,		false },
	{ "_mirror_cent",			NEAR_LOD_DISTANCE,		false },
	{ "_handbrake",				NEAR_LOD_DISTANCE,		false },
	{ "_gearstick",				NEAR_LOD_DISTANCE,		false },
	{ "_windowinterior_",		NEAR_LOD_DISTANCE,		false },

	{ "_brakedisc_",			NEAR_LOD_DISTANCE,		false },
	{ "_brakecaliper_",			NEAR_LOD_DISTANCE,		false, "damage_layer"},
	{ "_alloyrim_",				MEDIUM_LOD_DISTANCE,	false},
	{ "_alloy_",				FAR_LOD_DISTANCE,		false},
	{ "_window_",				FAR_LOD_DISTANCE,		true },

	{ "_bonnet",				-1,						true, "damage_layer" },
	{ "_bumper_back",			-1,						true, "damage_layer" },
	{ "_alloyblur",				FAR_LOD_DISTANCE,		true, "damage_layer" },
	{ "_licenseplate",			FAR_LOD_DISTANCE,		false, "damage_layer" },	

	{ "chassis",				NEAR_LOD_DISTANCE,		false },

	{ 0 },
};

// if the name of this object, or the name of one of it's parents matches something in the table,
// then add in a LOD node using that info from the table
LodInfo *GetLODTableEntry(Aftk::C_SceneNode *pNode)
{
	while (pNode)
	{
		Util::C_String name(pNode->GetName().c_str());
		for (LodInfo *p = g_LODTable; p->pszName; p++)
		{
			if (name.ISearch(p->pszName) != -1)
				return p;
		}

		pNode = pNode->GetParentPtr();
	}

	return NULL;
}

Aftk::C_LayerNode *GetLayer(Aftk::C_Scene *pScene, const char *pszName)
{
	for (int i=0; i<static_cast<int>(pScene->GetNumLayers()); i++)
	{
		if (!stricmp(pScene->GetLayerPtr(i)->GetName().c_str(), pszName))
			return pScene->GetLayerPtr(i);
	}

	// create a new layer
	Aftk::C_LayerNode *pLayer = new Aftk::C_LayerNode(pScene);
	pLayer->SetName(pszName);
	return pLayer;
}

bool C_InsertLODNodes::IsInteresting(Aftk::C_SceneNode* p_pSceneNode)
{
	if (p_pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh || 
		p_pSceneNode->GetType() == Aftk::E_ObjType_SceneNode)
		return true;
	return false;
}

Aftk::C_Status C_InsertLODNodes::Visit(Aftk::C_SceneNode *p_pNode, C_NullMemoryManager &memoryManager)
{
	LodInfo *p = GetLODTableEntry(p_pNode);
	if (p)
	{
		m_pTargetNodes->PushBack(p_pNode);
	}
	else
	{
		Aftk::C_LayerNode *pLayer = GetLayer(p_pNode->GetScenePtr(), "Default");
		pLayer->AddMember(p_pNode);	
	}

	return Aftk::C_Status::E_Status_Success;
}

void C_InsertLODNodes::AddBlindDataAttribute(Aftk::C_Polygon* pPolygon, const Util::C_String& attributeName)
{
	Aftk::C_SceneMeshNode* pSceneMeshNode = pPolygon->GetMeshNodePtr();
	Aftk::C_Scene* pAftkScene = pSceneMeshNode->GetScenePtr();
	S16 blindDataMappingIndex = pPolygon->GetBlindDataMappingIndex();

	const Aftk::C_BlindDataResource* pBlindDataResource = pSceneMeshNode->GetPolygonBlindDataMappingPtr(blindDataMappingIndex);
	if(pBlindDataResource != NULL)
	{
		const Aftk::C_BlindDataNode& aftkBlindDataNode = pBlindDataResource->GetContent();

		Aftk::C_BlindDataNode newBlindDataNode(aftkBlindDataNode);

		Aftk::C_GenericAttribute attr;
		attr.SetName(attributeName.Chars());

		newBlindDataNode.AddAttribute(attr);
   
		const Aftk::C_BlindDataResource* pResource = pAftkScene->AddBlindDataNode(newBlindDataNode);
		S16 blindDataMapping = pSceneMeshNode->AddPolygonBlindDataMapping(pResource);
		pPolygon->SetBlindDataMappingIndex(blindDataMapping);
	}  
}
void C_InsertLODNodes::Apply(Btl::Vector<Aftk::C_SceneNode*> *pTargetNodes)
{
	for (int i=0; i<pTargetNodes->Size(); i++)
	{
		Aftk::C_SceneNode *pHighLod = pTargetNodes->At(i);
		Aftk::C_Scene* pScene = pHighLod->GetScenePtr();
		Aftk::C_SceneNode* pSPTNode = pHighLod->GetParentPtr();

		LodInfo *pLODInfo = GetLODTableEntry(pHighLod);
		assert(pLODInfo);

		if (pLODInfo->iDistance != -1)
		{
			//Create a new Lod group.
			Aftk::C_SceneLodGroupNode* pNewLodGroup = new Aftk::C_SceneLodGroupNode(pScene);

			//inherit layer membership
			for (U32 i=0; i<pSPTNode->GetNumLayers(); ++i)
			{
				Aftk::C_LayerNode *pLayerNode = pSPTNode->GetLayerPtr(i);
				pLayerNode->AddMember(pNewLodGroup);
			}

			// if this object is not supposed to appear in the mirror view, then put it in the 'excludefromcubemap' polyflag on
			if (!pLODInfo->bMirrorView && pHighLod->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)pHighLod;

				for (U32 i=0; i<pMesh->GetNumPolygons(0); i++)
				{
					Aftk::C_Polygon &poly = pMesh->GetPolygon(0, i);

					AddBlindDataAttribute(&poly, "polySet_excludeFromMirror");
				}
			}

			//Add to scene-graph.
			pHighLod->GetParentPtr()->AddChildPtr(pNewLodGroup);

			//Set a name to indicate the processing that's been done/
			char *psz = strdup(pHighLod->GetName().c_str());
			if ((psz[0] == 'm' || psz[0] == 'e' || psz[0] == 'M' || psz[0] == 'E') && psz[1] == '_')
				pNewLodGroup->SetName(std::string(psz+2) + "_LOD");
			else
				pNewLodGroup->SetName(pHighLod->GetName() + "_LOD");
			
			pNewLodGroup->SetWorldMatrix(pHighLod->GetWorldMatrix(), true);

			//Reset high lod local matrix to identity.
			Aftk::Matrix44 id;
			UtMaths::SetIdentity(&id);
			pHighLod->SetLocalMatrix(id, false);

			pScene->ExtractSceneNodeAndChildren(pHighLod);

			std::string baseName = pHighLod->GetName();

			//Setup high lod info.
			{
				pNewLodGroup->AddChildPtr(pHighLod);

				Aftk::C_GroupLodLevelInfo info;
				info.SetSceneNodePtr(pHighLod);

				F32 lodThreshold = (F32)pLODInfo->iDistance;

				info.SetThreshold(lodThreshold);
				pNewLodGroup->AddLodLevelInfo(info);
			}
		}

		if (pLODInfo->pszLayer)
		{
			Aftk::C_LayerNode *pLayer = GetLayer(pScene, pLODInfo->pszLayer);
			pLayer->AddMember(pHighLod);
		}	
		else
		{
			Aftk::C_LayerNode *pLayer = GetLayer(pScene, "Default");
			pLayer->AddMember(pHighLod);	
		}
	}
}


/* 
===============================================================
					class C_CreateInteriorGlass

NOTES: Create "interior" versions of windows
===============================================================
*/

/*
===============
NAME: CreateNewName

NOTE: Creates new name for mesh
===============
*/
Util::C_String C_CreateInteriorGlass::CreateNewName(Util::C_String p_CurrentName)
{
	p_CurrentName.ConvertToLower();

	Ut::Vector<Util::C_String> tokens;
	p_CurrentName.Tokenise(tokens, '_');
	if (tokens.Size() != 3)
		return Util::C_String("INVALID");

	return Util::C_String(tokens[0] + "_windowinterior_" + tokens[2]);
}

/*
===============
NAME: CreateInteriorVersion

NOTE: Creates the interior version of the given window
===============
*/
bool C_CreateInteriorGlass::CreateInteriorVersion(Aftk::C_SceneMeshNode* p_pMeshNode)
{
	if (!p_pMeshNode)
		return false;

	Util::C_String currentName = p_pMeshNode->GetName().c_str();
	currentName.ConvertToLower();
	Util::C_String newName = CreateNewName(currentName);

	if (newName == "INVALID") 
		return false;

	Aftk::C_SceneMeshNode* pNewMesh = new Aftk::C_SceneMeshNode(*p_pMeshNode);

	if (!pNewMesh)
		return false;

	pNewMesh->SetName(newName.Chars());

	//Got cloned mesh, now need to invert the polygons and change the winding order
	for (U32 iPoly = 0; iPoly < pNewMesh->GetNumPolygons(0); iPoly++)
	{
		Aftk::C_Polygon& poly = pNewMesh->GetPolygon(0, iPoly);

		poly.ReverseWindingOrder();
	}

	for (U32 iVtx = 0; iVtx < pNewMesh->GetNumPolygonVertices(); iVtx++)
	{
		Aftk::C_PolygonVertex vtx = pNewMesh->GetPolygonVertex(iVtx);

		Aftk::Normal3 normal = pNewMesh->GetVertexNormal(iVtx);
		normal *= -1.0f;
		pNewMesh->SetVertexNormal(iVtx, normal);
	}

	p_pMeshNode->AddChildPtr((Aftk::C_SceneNode*)pNewMesh);

	return true;
}

/*
===============
NAME: CreateInteriorVersions

NOTE: Creates the interior versions of the windows
===============
*/
void C_CreateInteriorGlass::CreateInteriorVersions()
{
	for (S32 iWindow = 0; iWindow < m_pMeshNodes.Size(); iWindow++)
	{
		CreateInteriorVersion(m_pMeshNodes[iWindow]);
	}

	m_pMeshNodes.Clear();
}

/* 
===============================================================
					class C_FindWindows

NOTES: Find all windows on the vehicle
===============================================================
*/

/*
===============
NAME: IsInteresting

NOTE: 
===============
*/
bool C_FindWindows::IsInteresting(Aftk::C_SceneNode* p_pSceneNode)
{
	if (p_pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
		return true;

	return false;
}

/*
===============
NAME: Visit

NOTE: 
===============
*/
Aftk::C_Status C_FindWindows::Visit(Aftk::C_SceneNode* p_pNode, C_NullMemoryManager& memoryManager)
{
	if (!m_pCreator)
		return Aftk::C_Status::E_Status_Failure;

	Util::C_String name = p_pNode->GetName().c_str();
		
	if (name.ISearch("m_window_") != -1 || name.ISearch("e_window_") != -1)
		m_pCreator->m_pMeshNodes.PushBack((Aftk::C_SceneMeshNode*)p_pNode);

	return Aftk::C_Status::E_Status_Success;
}

/* 
===============================================================
					class C_TransformVertices

NOTES: Transform vertices to world space
===============================================================
*/

/*
===============
NAME: IsInteresting

NOTE: 
===============
*/
bool C_TransformVertices::IsInteresting(Aftk::C_SceneNode* p_pSceneNode)
{
// 	if (p_pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
// 		return true;
// 
// 	return false;
	return true;
}

/*
===============
NAME: Visit

NOTE: 
===============
*/

Aftk::C_Status C_TransformVertices::Visit(Aftk::C_SceneNode* p_pNode, C_NullMemoryManager& memoryManager)
{
	bool isWheel = false;
	
	Util::C_String name(p_pNode->GetName().c_str());
	if (name.ISearch("_wheel_") != -1 ||
		name.ISearch("_tyre_") != -1 ||
		name.ISearch("_brakedisc_") != -1 ||
		name.ISearch("_brakecaliper_") != -1 ||
		name.ISearch("_alloy") != -1)
		isWheel = true;
	
	const bool isPhysics = AftkHelpers::GetCollisionType(p_pNode) != AftkHelpers::E_CollisionType_None;
	
	if ((!isWheel && !isPhysics) && p_pNode->GetType() == Aftk::E_ObjType_SceneMesh)
	{
		Aftk::Matrix44 iMat;
		UtMaths::SetIdentity(&iMat);
		iMat.AX(Aftk::Vector4F32(-1.0f, 0.0f, 0.0f, 0.0f));
		iMat.AY(Aftk::Vector4F32(0.0f, 1.0f, 0.0f, 0.0f));
		iMat.AZ(Aftk::Vector4F32(0.0f, 0.0f, -1.0f, 0.0f));

		((Aftk::C_SceneMeshNode*)p_pNode)->ApplyMatrix(p_pNode->GetWorldMatrix(true) * iMat);
		p_pNode->SetWorldMatrix(iMat, true);
	}

	if (isWheel && !isPhysics && (p_pNode->GetType() == Aftk::E_ObjType_SceneMesh))
	{
		if ((name.ISearch("_front") != -1))
		{
			if (name.ISearch("_brakecaliper_") == -1)
			{
				GetLayer(p_pNode->GetScenePtr(), "fronttyre_layer")->AddMember(p_pNode);
				GetLayer(p_pNode->GetScenePtr(), "Default")->DeleteMember(p_pNode);
			}
			if ((name.ISearch("_tyre_") != -1))
			{
				//Front wheel
				m_pWheelPivot[0] = p_pNode->GetWorldMatrix(true)[3].y;
				m_pWheelPivot[1] = p_pNode->GetWorldMatrix(true)[3].z;
			}
		}
		
		if ((name.ISearch("_back") != -1) || (name.ISearch("_rear") != -1))
		{
			if (name.ISearch("_brakecaliper_") == -1)
			{
				GetLayer(p_pNode->GetScenePtr(), "backtyre_layer")->AddMember(p_pNode);
				GetLayer(p_pNode->GetScenePtr(), "Default")->DeleteMember(p_pNode);
			}

			if ((name.ISearch("_tyre_") != -1))
			{
				//Front wheel
				m_pWheelPivot[2] = p_pNode->GetWorldMatrix(true)[3].y;
				m_pWheelPivot[3] = p_pNode->GetWorldMatrix(true)[3].z;
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}


bool C_ReflectCar::IsInteresting(Aftk::C_SceneNode* p_pSceneNode)
{
	const bool isPhysics = AftkHelpers::GetCollisionType(p_pSceneNode) != AftkHelpers::E_CollisionType_None;

	if ((p_pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh) && !isPhysics)
	{
		Util::C_String name(p_pSceneNode->GetName().c_str());
		if (name.ISearch("_wheel_") != -1 ||
			name.ISearch("_tyre_") != -1 ||
			name.ISearch("_brakedisc_") != -1 ||
			name.ISearch("_brakecaliper_") != -1 ||
			name.ISearch("_alloy") != -1 ||
			name.ISearch("m_shadowcaster") != -1 ||
			name.ISearch("m_collisionmesh") != -1)
		{
			return false;
		}
		else
			return true;
	}
	else
		return false;
}

Aftk::C_Status C_ReflectCar::Visit(Aftk::C_SceneNode* p_pNode, C_NullMemoryManager& memoryManager)
{
	Aftk::C_SceneMeshNode* p_pMeshNode = (Aftk::C_SceneMeshNode*)p_pNode;
	m_pTargetNodes->PushBack(p_pMeshNode);

	return Aftk::C_Status::E_Status_Success;
}

void C_ReflectCar::Apply(Aftk::C_Scene* pScene, Btl::Vector<Aftk::C_SceneMeshNode*> *pTargetNodes, F32 frontWheelPosition[2], F32 backWheelPosition[2])
{
	Aftk::C_SceneNode* pNullNode = new Aftk::C_SceneNode(pScene);
	pScene->GetRootSceneNodePtr()->AddChildPtr(pNullNode);
	pNullNode->SetName("Reflect");

	Aftk::C_LayerNode *pFrontLayer = GetLayer(pScene, "reflectfront_layer");
	Aftk::C_LayerNode *pBackLayer = GetLayer(pScene, "reflectback_layer");
	Aftk::C_LayerNode *pDefaultLayer = GetLayer(pScene, "default");
	Aftk::C_LayerNode *pDamageLayer = GetLayer(pScene, "damage_layer");

	U32 numRotatedMesh = 8;

	//Front wheels
	for (U32 i = 0; i < (numRotatedMesh - 1) ; i++)
	{
		F32 angle = 2.f * Maths::Pi * float(i + 1) / float(numRotatedMesh);

		Aftk::Matrix44 iMatToPivot, iMatRotation, iMatFromPivot, iMat;
		UtMaths::SetIdentity(&iMatToPivot);
		UtMaths::SetIdentity(&iMatRotation);
		UtMaths::SetIdentity(&iMatFromPivot);


		iMatToPivot.AX(Aftk::Vector4F32(1.0f, 0.0f, 0.0f, 0.0f));	
		iMatToPivot.AY(Aftk::Vector4F32(0.0f, 1.0f, 0.0f, 0.f));
		iMatToPivot.AZ(Aftk::Vector4F32(0.0f, 0.0f, 1.0f, 0.f));
		iMatToPivot.At<3>() = Aftk::Vector4F32(0.f, -frontWheelPosition[0], -frontWheelPosition[1], 1.0f);

		iMatRotation.AX(Aftk::Vector4F32(1.0f, 0.0f, 0.0f, 0.0f));	
		iMatRotation.AY(Aftk::Vector4F32(0.0f, Maths::Cos(angle), -Maths::Sin(angle), 0.0f));
		iMatRotation.AZ(Aftk::Vector4F32(0.0f, Maths::Sin(angle), Maths::Cos(angle), 0.0f));
		iMatRotation.At<3>() = Aftk::Vector4F32(0.f, 0.0f, 0.f, 1.0f);

		iMatFromPivot.AX(Aftk::Vector4F32(1.0f, 0.0f, 0.0f, 0.0f));	
		iMatFromPivot.AY(Aftk::Vector4F32(0.0f, 1.0f, 0.0f, 0.f));
		iMatFromPivot.AZ(Aftk::Vector4F32(0.0f, 0.0f, 1.0f, 0.f));
		iMatFromPivot.At<3>() = Aftk::Vector4F32(0.f, frontWheelPosition[0], frontWheelPosition[1], 1.0f);

		
		iMat = iMatToPivot * iMatRotation * iMatFromPivot;
		
		for (S32 j = 0; j < pTargetNodes->Size(); j++)
		{
			Aftk::C_SceneMeshNode* pRotatedMesh = new Aftk::C_SceneMeshNode(*(*pTargetNodes)[j]);
			Util::C_String currentName = (*pTargetNodes)[j]->GetName().c_str();
			currentName.ConvertToLower();
			pRotatedMesh->SetName((currentName + "_reflectfront" + Util::C_String::Format("%i", i)).Chars());
			
			pRotatedMesh->ApplyMatrix(iMatFromPivot);
			pRotatedMesh->ApplyMatrix(iMatRotation);
			pRotatedMesh->ApplyMatrix(iMatToPivot);


			pNullNode->AddChildPtr(pRotatedMesh);

			pFrontLayer->AddMember(pRotatedMesh);
			pDefaultLayer->DeleteMember(pRotatedMesh);
			pDamageLayer->DeleteMember(pRotatedMesh);

			U32 iPolygonCount = pRotatedMesh->GetNumPolygons(0);
			for (U32 iCurrentPoly=0; iCurrentPoly<iPolygonCount; ++iCurrentPoly)
			{
				Aftk::C_Polygon *pCurrentPolygon = &pRotatedMesh->GetPolygon(iCurrentPoly);
				Maintenance::AddBlindDataAttribute(pCurrentPolygon, "polySet_doNotPVS");
			}
		}
	}

	//Front wheels
	for (U32 i = 0; i < (numRotatedMesh - 1) ; i++)
	{
		F32 angle = 2.f * Maths::Pi * float(i + 1) / float(numRotatedMesh);

		Aftk::Matrix44 iMatToPivot, iMatRotation, iMatFromPivot, iMat;
		UtMaths::SetIdentity(&iMatToPivot);
		UtMaths::SetIdentity(&iMatRotation);
		UtMaths::SetIdentity(&iMatFromPivot);

		iMatToPivot.AX(Aftk::Vector4F32(1.0f, 0.0f, 0.0f, 0.0f));	
		iMatToPivot.AY(Aftk::Vector4F32(0.0f, 1.0f, 0.0f, 0.f));
		iMatToPivot.AZ(Aftk::Vector4F32(0.0f, 0.0f, 1.0f, 0.f));
		iMatToPivot.At<3>() = Aftk::Vector4F32(0.f, -backWheelPosition[0], -backWheelPosition[1], 1.0f);

		iMatRotation.AX(Aftk::Vector4F32(1.0f, 0.0f, 0.0f, 0.0f));	
		iMatRotation.AY(Aftk::Vector4F32(0.0f, Maths::Cos(angle), -Maths::Sin(angle), 0.0f));
		iMatRotation.AZ(Aftk::Vector4F32(0.0f, Maths::Sin(angle), Maths::Cos(angle), 0.0f));
		iMatRotation.At<3>() = Aftk::Vector4F32(0.f, 0.0f, 0.f, 1.0f);

		iMatFromPivot.AX(Aftk::Vector4F32(1.0f, 0.0f, 0.0f, 0.0f));	
		iMatFromPivot.AY(Aftk::Vector4F32(0.0f, 1.0f, 0.0f, 0.f));
		iMatFromPivot.AZ(Aftk::Vector4F32(0.0f, 0.0f, 1.0f, 0.f));
		iMatFromPivot.At<3>() = Aftk::Vector4F32(0.f, backWheelPosition[0], backWheelPosition[1], 1.0f);


		iMat = iMatToPivot * iMatRotation * iMatFromPivot;

		for (S32 j = 0; j < pTargetNodes->Size(); j++)
		{
			Aftk::C_SceneMeshNode* pRotatedMesh = new Aftk::C_SceneMeshNode(*(*pTargetNodes)[j]);
			Util::C_String currentName = (*pTargetNodes)[j]->GetName().c_str();
			currentName.ConvertToLower();
			pRotatedMesh->SetName((currentName + "_reflectback" + Util::C_String::Format("%i", i)).Chars());

			pRotatedMesh->ApplyMatrix(iMatFromPivot);
			pRotatedMesh->ApplyMatrix(iMatRotation);
			pRotatedMesh->ApplyMatrix(iMatToPivot);


			pNullNode->AddChildPtr(pRotatedMesh);

			pBackLayer->AddMember(pRotatedMesh);
			pDefaultLayer->DeleteMember(pRotatedMesh);
			pDamageLayer->DeleteMember(pRotatedMesh);

			U32 iPolygonCount = pRotatedMesh->GetNumPolygons(0);
			for (U32 iCurrentPoly=0; iCurrentPoly<iPolygonCount; ++iCurrentPoly)
			{
				Aftk::C_Polygon *pCurrentPolygon = &pRotatedMesh->GetPolygon(iCurrentPoly);
				Maintenance::AddBlindDataAttribute(pCurrentPolygon, "polySet_doNotPVS");
			}
		}
	}
	
}


/* 
===============================================================
					class C_CustomVehicleProcesses

NOTES: 
===============================================================
*/

C_CustomVehicleProcesses C_CustomVehicleProcesses::s_Instance;

/*
===============
NAME: Process

NOTE: Perform custom processes on the vehicle scene
===============
*/
void C_CustomVehicleProcesses::Process(Aftk::C_Scene* p_pAftkScene)
{	
	m_pAftkScene = p_pAftkScene;

	if (!m_pAftkScene)
		return;

	//Perform model-element clean-up
	Maintenance::C_ModelsToElements mToE;
	Aftk::C_SceneNodeVisit::Traverse(m_pAftkScene, mToE);

	//Apply draw distances by appying LOD nodes
	Btl::Vector<Aftk::C_SceneNode*> targetNodes;
	C_InsertLODNodes insertLodNodes(&targetNodes);
	Aftk::C_SceneNodeVisit::Traverse(m_pAftkScene, insertLodNodes);
	insertLodNodes.Apply(&targetNodes);

	F32	wheelPosition[4];
	C_TransformVertices transformVerts (wheelPosition);
	Aftk::C_SceneNodeVisit::Traverse(m_pAftkScene, transformVerts);
	
	//Find all windows on the vehicle and create "interior" versions of them
	//C_CreateInteriorGlass createWindows;
	//C_FindWindows findWindows(&createWindows);
	//Aftk::C_SceneNodeVisit::Traverse(m_pAftkScene, findWindows);
	//createWindows.CreateInteriorVersions();
	//bool mofo = true;

	Btl::Vector<Aftk::C_SceneMeshNode*> newNodes;
	C_ReflectCar reflectCar(&newNodes);
	
	Aftk::C_SceneNodeVisit::Traverse(m_pAftkScene, reflectCar);
	F32 frontWheelPosition[2] = {wheelPosition[0], wheelPosition[1]};
	F32 rearWheelPosition[2] = {wheelPosition[2], wheelPosition[3]};
	reflectCar.Apply(m_pAftkScene, &newNodes, frontWheelPosition, rearWheelPosition);

}

} // namespace Vehicle
