#include "stdafx.h"
#include "Lux/Common.h"
#include "Lux/Lights/Light.h"
#include "Lux/Lights/DirectionalLight.h"
#include "Lux/LightingModelProperties.h"
#include "lux_common/LpvsUtility.h"
#include "Aftkut/AftkUtNodeFlagParser.h"
#include "Tools_Common/StringProc.h"
#include <algorithm>

using namespace LUX;

Debug::Logger &LUX::g_Log = Debug::GetLogger("tools.lux");

// returns true it's an abnormal floating point number
bool CheckBuggeredFloat(float f)
{
	int i = *(int*)&f;
	return (i&0x7f000000) == 0x7f000000;
}

// returns true if a given node is a barrier instance
bool IsNodeABarrier(Aftk::C_SceneNode *pNode)
{
	// has to be a boned instance
	if (pNode->GetType() != Aftk::E_ObjType_SceneMeshBonedInstance)
		return false;

	Aftk::C_SceneMeshBonedInstanceNode *pInstance = (Aftk::C_SceneMeshBonedInstanceNode*)pNode;

	// get source node name
	Aftk::C_SceneNode *pSourceNode = pInstance->GetSourceNodePtr();
	std::string name = pSourceNode->GetName().c_str();
	Tu::ToLower( name );
	if (name.find("barrier") != std::string::npos)
	{
		return true;
	}

	return false;
}

// returns whether this node has "light using vertex colours" flag set
NodeLightingMode GetNodeLightingModeX(const LightingModelProperties *pProp, Aftk::C_SceneNode *pNode)
{
	bool isActor;
	AftkUt::C_NodeFlagParser parser(*pNode, &isActor);

	bool bNotSet = true;
	std::string text;

	// does this node have actor data attached?
	if (isActor)
	{
		std::string result;

		std::string strClass, strName;
		strClass = parser.GetFlagGroupCategory();
		std::transform(strClass.begin(), strClass.end(), strClass.begin(), tolower);

		strName = parser.GetFlagGroupName();
		std::transform(strName.begin(), strName.end(), strName.begin(), tolower);
//		if ((strClass == "system") && (strName == "vertexlighting" || strName == "instance"))
		{
			// yes it has system properties

			// get the SH lighting value
			if (parser.GetParameterValue("UseSHLighting", result) &&  result != "")
			{
				if (atof(result.c_str()) != 0)
					return LMode_SH; // it was set to true
				bNotSet = false;
			}

			// get the vertex lighting value
			if (parser.GetParameterValue("UseVertexLighting", result) && result != "")
			{
				if (atof(result.c_str()) != 0)
					return LMode_VtxColour; // it was set to true
				bNotSet = false;
			}

			// get the lightmap lighting value
			if (parser.GetParameterValue("UseLightmapLighting", result) && result != "")
			{
				if (atof(result.c_str()) != 0)
					return LMode_Lightmap; // it was set to true
				bNotSet = false;
			}

			// get the lightmap lighting value
			if (parser.GetParameterValue("DoNotLight", result) && result != "")
			{
				if (atof(result.c_str()) != 0)
					return LMode_DoNotLight; // it was set to true
				bNotSet = false;
			}
		}
	}
	if (bNotSet)
	{
		// nothing set. if it's an instance, check it's source node
		if (pNode->HasType( Aftk::E_ObjType_SceneMeshRemappedInstance ))
		{
			Aftk::C_SceneMeshRemappedInstanceNode *pInstance = (Aftk::C_SceneMeshRemappedInstanceNode*) pNode;
			NodeLightingMode mode = GetNodeLightingModeX(pProp, pInstance->GetSourceNodePtr());
			if (mode != LMode_NotSet)
				return mode;

			if (pProp->m_bInstancesDefaultToSHLighting)
			{
				// make barriers default to LMode_Lightmap
				if (IsNodeABarrier(pNode))
					return LMode_Lightmap;

				// make instances default to LMode_SH
				return LMode_SH;
			}
		}

		// if we have "InstancesDefaultToSHLighting" set, light base instances using SH by default as well
		if (pNode->GetNumInstancesOfThisNode() > 0 && pProp->m_bInstancesDefaultToSHLighting)
			return LMode_SH;
	}
	else
		return LMode_Lightmap;	// if someone has applied actor properties but not ticked any boxes, assume they mean to use lightmaps

	return LMode_NotSet;
}

NodeLightingMode GetNodeLightingMode(const LightingModelProperties *pProp, Aftk::C_SceneNode *pNode)
{
	NodeLightingMode result = GetNodeLightingModeX(pProp, pNode);

	// if we're not LMode_Lightmap, but some instances of us are, then we have to be LMode_Lightmap as well
	if (result != LMode_Lightmap)
	{
		for (int i=0; i < (int)pNode->GetNumInstancesOfThisNode(); i++)
		{
			if (GetNodeLightingMode(pProp, pNode->GetInstanceOfThisNode(i)) == LMode_Lightmap)
			{
				result = LMode_Lightmap;
				break;
			}
		}
	}

	if (result == LMode_NotSet)
	{
		// the default behaviour is defined by the default bake target from the lighting model properties
		switch (pProp->GetDefaultBakeTarget())
		{
		case LUX_MODEL_BAKE_TARGET_VERTEX:
			return LMode_VtxColour;
		case LUX_MODEL_BAKE_TARGET_LIGHTMAP:
			return LMode_Lightmap;
		default:
			assert(0);
			return LMode_Lightmap;
		}
		return LMode_Lightmap;
	}
	else
		return result;
}

// check whether this aftk scene node is an instance of a speed tree
bool LUX::IsNodeASpeedTreeInstance(Aftk::C_SceneNode *pNode)
{
	// is it an instance?
	Aftk::E_ObjType type = pNode->GetType();
	if (type == Aftk::E_ObjType_SceneInstance)
	{
		// get the reference geometry node
		pNode = ((Aftk::C_SceneInstanceNode*)pNode)->GetSourceNodePtr();
		if (!pNode)
			return false;
		type = pNode->GetType();
	}
	else if (pNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance) )
	{
		pNode = ((Aftk::C_SceneMeshRemappedInstanceNode*)pNode)->GetSourceNodePtr();
		if (!pNode)
			return false;
		type = pNode->GetType();
	}

	// check that it is a meshnode
	if (type != Aftk::E_ObjType_SceneMesh)
		return false;

	if (pNode->GetName().find("SpeedTreeMesh:") != 0)
		return false;

	return true;
}

bool CheckCarNodeNeedsLighting(Aftk::C_SceneNode *pNode)
{
	// don't light damage parts for cars for Chad
	if (strstr(pNode->GetName().c_str(), "scratch") ||
		strstr(pNode->GetName().c_str(), "Scratch"))
		return false;

	// check parent
	Aftk::C_SceneNode *pParent = pNode->GetParentPtr();
	if (pParent)
		return CheckCarNodeNeedsLighting(pParent);

	return true;
}

// check for "base instance" actor properties
bool NodeHasBaseInstanceDoNotExportActorProperties(Aftk::C_SceneNode *pNode)
{
	bool bNotSet = true;
	std::string text;
	// does this node have blind data?
	if (AftkUt::C_NodeFlagParser::IsFlagged(*pNode))
	{
		AftkUt::C_NodeFlagParser parser(*pNode);
		std::string result;

		std::string strClass, strName;
		strClass = parser.GetFlagGroupCategory();
		std::transform(strClass.begin(), strClass.end(), strClass.begin(), tolower);

		strName = parser.GetFlagGroupName();
		std::transform(strName.begin(), strName.end(), strName.begin(), tolower);
		if ((strClass == "system") && (strName == "baseinstance"))
		{
			// yes it has base instance properties

			// get the 'export' parameter
			if (parser.GetParameterValue("ExportBase", result) && result != "")
			{
				if (atoi(result.c_str()) == 0)
					return true;
			}
		}
	}

	return false;
}

bool NodeIsChildOfNonExportedNode(Aftk::C_SceneNode *pNode)
{
	bool isChildOfNonExportedNode = false;
	while (pNode != NULL && !isChildOfNonExportedNode)
	{
		if (pNode->GetName() == "InstanceReferences")
		{
			isChildOfNonExportedNode = true;
			break;
		}

		// not interested if node is an instance
		if (pNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
			break;

		if (NodeHasBaseInstanceDoNotExportActorProperties(pNode))
		{
			isChildOfNonExportedNode = true;
			break;
		}
		pNode = pNode->GetParentPtr();
	}
	return isChildOfNonExportedNode;
}

bool LUX::DoesBaseInstanceNeedExporting(Aftk::C_SceneNode *pNode)
{
	// if this node is an instance, ignore any "donotexport" flags
	if (pNode->HasType( Aftk::E_ObjType_SceneMeshRemappedInstance ))
		return true;

	if (NodeHasBaseInstanceDoNotExportActorProperties(pNode))
		return false;

	if (NodeIsChildOfNonExportedNode(pNode))
		return false;

	return true;
}


// check whether a node needs processing by the lightmapper
extern char gpszOnlyLightObject[]; // this is in globalilluminationmodel.cpp
extern int giLightOnlyNode;
bool gbCarMode = false;
bool LUX::DoesNodeNeedProcessing(Aftk::C_SceneNode *pNode)
{
	// only interested in mesh nodes
	if (!pNode->HasType(Aftk::E_ObjType_ISceneMesh))
		return false;

	Aftk::I_SceneMeshNode *pMesh = (Aftk::I_SceneMeshNode*)pNode;
	if (pMesh->GetNumPolygons(0)==0)
		return false;

	// for cars only
	if (gbCarMode)
	{
		// skip stuff with 'glass' in the material name
		const Aftk::C_MaterialResource *pMatRes = 
			GetMaterialResourcePtr(&((Aftk::C_SceneMeshNode*)pNode)->GetPolygon(0));
		if (pMatRes)
		{
			const char *pszMatName = pMatRes->GetContent().GetName().c_str();
			if (strstr(pszMatName,"Glass"))
				return false;

			if (strstr(pszMatName,"Light_Overlay"))
				return false;
		}

		if (!CheckCarNodeNeedsLighting(pNode))
			return false;
	}

	// skip things in layers called 'sky'
	Aftk::C_LayerNode *pLayer = pNode->GetLayerPtr(0);
	if (pLayer)
	{
		char buffer[512];
		strcpy(buffer, pLayer->GetName().c_str());
		StrToLower(buffer);

		if (!stricmp(pLayer->GetName().c_str(), "Sky"))
			return false;

		if (!stricmp(pLayer->GetName().c_str(), "Occluders"))
			return false;

		if (strstr(buffer, "navmesh"))
			return false;

		if (!stricmp(pLayer->GetName().c_str(), "Noexport"))
			return false;

		if (!stricmp(buffer, "crowd_pvs_data"))
			return false;

		if (strstr(buffer, "rotoscope"))
			return false;
	}

	if (AftkUt::C_NodeFlagParser::IsFlagged(*pNode))
	{
		AftkUt::C_NodeFlagParser parser(*pNode);
		// ignore actor crowd stuff
		{
			if (parser.GetFlagGroupCategory() == "Crowd" && parser.GetFlagGroupName()!="Obstacle")
				return false;
		}

	    // ignore camera zones
		{
			if (parser.GetFlagGroupCategory() == "RouteItem" && parser.GetFlagGroupName()=="TVCameraZone")
				return false;
		}
	}

	// some handy code to make it only process one particular object - useful for debugging
	// has to be called "polySurface4" or we'll skip it
//	if (gpszOnlyLightObject[0]!=0 && stricmp(pNode->GetName().c_str(), gpszOnlyLightObject))
//		return false;

	// as above but with node id
//	if (giLightOnlyNode != -1  && pNode->GetUniqueId() != giLightOnlyNode)
//		return false;

	return true;
}

// set visibility ray using a start and end position
void Visibility::SetSegment(const float3 &vStart, const float3 &vEnd)
{
	ray.origin = vStart;
	ray.direction = vEnd - vStart;
	ray.length = Length(ray.direction);
	ray.direction *= 1.0f/ray.length;
	ray.length -= 0.0001f;
}

void Visibility::SetRay(const float3 &vStart, const float3 &vDir)
{
	ray.origin = vStart;
	ray.direction = vDir;
	ray.length = FLT_MAX;
	ray.direction = Normalize(ray.direction);
}

// test visibility
bool Visibility::Unoccluded(const SD::SpatialHierarchy *pScene)
{
//	return true;

	SD::RayTraceResult result;
	ray.bShadowRay = true;
	pScene->RayTrace(ray, result);

	return !result.bHit;
}

// calculate decay amount from squared distance
float GetDecay(Aftk::E_LightDecay type, float vDistSq)
{
	switch (type)
	{
	default:
		return 1.0f;
	case Aftk::E_LightDecay_Linear:
		return 1.0f/sqrtf(vDistSq);
	case Aftk::E_LightDecay_Quadratic:
		return 1.0f/vDistSq;
	case Aftk::E_LightDecay_Cubic:
		float f = sqrtf(vDistSq);
		return 1.0f/(f*f*f);
	}
}

// calculate max radius from intensity
float GetMaxRadius(const float3 &vIntensity)
{
	float fMin = 5;		// minimum brightness

	// get intensity
	float fIntensity = vIntensity.Luma();

	return sqrtf(fIntensity/fMin);
}

// calculate max radius from intensity
// fmin = minimum brightness cutoff point
float GetMaxRadius(const float3 &vIntensity, float fMin)
{
	// get intensity
	float fIntensity = fabsf(vIntensity.Luma());

	return sqrtf(fIntensity/fMin);
}

// debugging utility function
float3 PosToColour(float3 v)
{
	int x;
	float3 s;
	v.x += 100.0f;
	v.y += 100.0f;
	v.z += 100.0f;
	v = v*10;

	x = v.x;
	s.x = v.x -x;
	x = v.y;
	s.y = 0;//v.y -x;
	x = v.z;
	s.z = v.z -x;

	return s;
}

int VectorSortCallback(const void *a, const void *b)
{
	for (int i=0; i<3; i++)
	{
		int iSignA = (((int*)a)[i] & 0x80000000) >> 31;
		int iSignB = (((int*)b)[i] & 0x80000000) >> 31;

		if (iSignA != iSignB)
			return iSignA - iSignB;
	}

	for (int i=0; i<3; i++)
	{
		int iA = ((float*)a)[i] * 8;
		int iB = ((float*)b)[i] * 8;

		if (iA != iB)
			return iA - iB;
	}

	return 0;	
}

// sorting a list of samples by signs. this should improve coherency and
// therefore improve cache usage and probability of batching rays using SSE
void SortVectorsBySigns(float3 *pVectorList, int nVectors)
{
	qsort(pVectorList, nVectors, sizeof(float3), VectorSortCallback);
}

// find the UV set index that contains the unwrapped UVs
int	GetUVSetIndexFromMeshNode(Aftk::I_SceneMeshNode *pMesh)
{
	for (int i=0; i<(int)pMesh->GetNumTextureCoordinateSets(); i++)
	{
		if ( pMesh->GetTextureCoordinateSetName(i) == LIGHT_MAP_UV_SET_NAME )
		{
			return i;
		}
	}
	return -1;
}

// get the number of processors in the system
int GetNumProcessors()
{
#ifdef WIN32
	SYSTEM_INFO sysInfo;

	GetSystemInfo(&sysInfo);

	return sysInfo.dwNumberOfProcessors;
#else
	return 1;
#endif
}

// set 'denormals are zero' mode
void SetDAZ()
{
#ifdef WIN32
	try { _mm_setcsr(_mm_getcsr() | 0x8040); } catch (...) {};
#endif
}

// convert normal maya instances to 'remapped instances'
void ConvertNormalInstancesToRemappedInstances(Aftk::C_Scene *pScene, SD::SpatialHierarchy *pTree)
{
	Ut::Vector<Aftk::C_SceneInstanceNode*> instancelist;

	// generate a list of all instances to be converted
	for (Aftk::C_SceneNodeIterator it(pScene->GetRootSceneNodePtr()); it; it++)
	{
		Aftk::C_SceneNode* pAftkSceneNode = it;

		if (pAftkSceneNode->GetType() != Aftk::E_ObjType_SceneInstance)
			continue;

		// only convert instances that reference meshnodes
		Aftk::C_SceneInstanceNode *pInstance = (Aftk::C_SceneInstanceNode*)pAftkSceneNode;
		if (!pInstance->GetSourceNodePtr())
			continue;

		if (pInstance->GetSourceNodePtr()->GetType() != Aftk::E_ObjType_SceneMesh)
			continue;

		instancelist.PushBack(pInstance);
	}

	// convert the instances we found
	for (int i=0; i<instancelist.Size(); i++)
	{
		Aftk::C_SceneInstanceNode *pInstance = instancelist[i];

		Aftk::C_SceneMeshNode *pOrgNode = (Aftk::C_SceneMeshNode*)pInstance->GetSourceNodePtr();

		// convert instances into AftkSceneMeshRemappedNode class
		// create a new meshnode
		Aftk::C_SceneMeshRemappedInstanceNode *pNewNode = new Aftk::C_SceneMeshRemappedInstanceNode(pOrgNode->GetScenePtr());
		pNewNode->SetName(pInstance->GetName());

		// copy over the local matrix from the instance
		pNewNode->SetLocalMatrix(pInstance->GetLocalMatrix());

		Aftk::Matrix44 worldInstance = pInstance->GetWorldMatrix();

		// copy over blind data
		pNewNode->SetBlindDataResourcePtr(pInstance->GetBlindDataResourcePtr());

		for (U32 j = 0; j < pInstance->GetNumDynamicPlugs(); j++)
			pNewNode->AddPlugPtr((Aftk::C_DynamicPlug*)pInstance->GetDynamicPlugPtr(j)->Clone());

		pNewNode->Replace(pInstance);

		// copy over reference node data
		pNewNode->SetSourceNodePtr( pOrgNode );
	}
}

// add a blind data attribute to the existing blind data on a node
void AddBlindData(Aftk::C_SceneNode *pNode, Aftk::C_GenericAttribute &attr)
{
	Aftk::C_BlindDataNode newBlindData;

	// copy existing blind data into our new blind data (if it's there)
	const Aftk::C_BlindDataResource *pBlindDataResource = pNode->GetBlindDataResourcePtr();
	if (pBlindDataResource)
	{
		const Aftk::C_BlindDataNode *pBlindData = &pNode->GetBlindDataResourcePtr()->GetContent();
		if (pBlindData)
			newBlindData = *pBlindData;
	}

	// add our attribute to the blind data
	newBlindData.AddAttribute(attr);

	// add new blind data into the scene and the scene node
	const Aftk::C_BlindDataResource* pNewBlindDataResource = 
		pNode->GetScenePtr()->AddBlindDataNode(newBlindData);
	pNode->SetBlindDataResourcePtr(pNewBlindDataResource);
}


// check whether a node is a locator with SH data on
bool NodeIsLocatorWithSHData(Aftk::C_SceneNode *pNode)
{
    // get a node's blind data
	if (pNode->GetBlindDataResourcePtr())
	{
		const Aftk::C_BlindDataNode *pBlindData = &pNode->GetBlindDataResourcePtr()->GetContent();

		if (pBlindData)
		{
			for (int i=0; i<(int)pBlindData->GetNumAttributes(); i++)
			{
				if (pBlindData->GetAttribute(i).GetName() == "SH_R")
				{
					return true;
				}
			}
		}
	}

	return false;
}

// calculate bounding box of a meshnode or instance node
bool CalculateAftkNodeBoundingBox(Aftk::C_SceneNode *pNode, box3f &bbox, bool bWorldSpace)
{
	Aftk::Matrix44 mtx = pNode->GetWorldMatrix();

	// check that it is a meshnode
	if (!pNode->HasType( Aftk::E_ObjType_ISceneMesh ))
		return false;

	Aftk::I_SceneMeshNode *pMesh = (Aftk::I_SceneMeshNode*)pNode;
	for (int i=0; i<(int)pMesh->GetNumPolygonVertices(); i++)
	{
		Aftk::Position3 vPos;
		if (bWorldSpace)
			vPos = LPVS::ApplyMatrix(mtx, pMesh->GetEvaluatedPolygonVertexPosition( i ) );
		else
			vPos = pMesh->GetEvaluatedPolygonVertexPosition( i );
		bbox.UpdateBox( vPos );
	}

	return true;
}

// convert a remapped instance into a normal meshnode (for game engines that don't support instances)
Aftk::C_SceneMeshNode *ConvertRemappedInstanceToMesh(Aftk::C_SceneMeshRemappedInstanceNode *pInstance)
{
	Aftk::C_SceneMeshNode* pNewNode;

	Aftk::C_SceneMeshNode *pOrgNode = (Aftk::C_SceneMeshNode*)pInstance->GetSourceNodePtr();

	//We've got to take care here. If we're going to convert an instance into mesh then
	//we must special case the conversion of boned instances in order to make them appear in
	//the correct location.
	if (pInstance->GetType() == Aftk::E_ObjType_SceneMeshBonedInstance)
		pNewNode = new Aftk::C_SceneMeshNode((Aftk::C_SceneMeshBonedInstanceNode&)*pInstance);
	else
		pNewNode = new Aftk::C_SceneMeshNode(*pOrgNode);

	// copy over the local matrix from the instance
	pNewNode->SetLocalMatrix(pInstance->GetLocalMatrix());

	pNewNode->Replace(pInstance);

	return pNewNode;
}

// convert all remapped instances in a scene to meshnodes
void ConvertRemappedInstancesToMesh(Aftk::C_Scene *pAftkScene)
{
	// create a list of all instance nodes in the scene
	Ut::Vector<Aftk::C_SceneNode*> instancelist;

	// iterate all the scene's nodes
	for (Aftk::C_SceneNodeIterator it(pAftkScene->GetRootSceneNodePtr()); it; ++it)
	{
		Aftk::C_SceneNode* pAftkSceneNode = it;

		// only interested in instance nodes
		if (!pAftkSceneNode->HasType( Aftk::E_ObjType_SceneMeshRemappedInstance ))
			continue;

		Aftk::C_SceneMeshRemappedInstanceNode *pInstance = (Aftk::C_SceneMeshRemappedInstanceNode*)pAftkSceneNode;

		instancelist.PushBack( pInstance );
	}

	// convert all the instances that we found to mesh
	for (int iInst=0; iInst<instancelist.Size(); iInst++)
	{
		ConvertRemappedInstanceToMesh((Aftk::C_SceneMeshRemappedInstanceNode*)instancelist[iInst]);        
	}
}

// add a plug with custom data to this node
// data is always in the form of floats and is stored as a hex string on the plug
void SetLuxCustomDataToNode(Aftk::C_SceneNode *pNode, U8 *pData, int nc)
{
	Util::C_String text, tmp;
	for (int i=0; i<nc; i++)
	{
		tmp.Printf("%02x", pData[i]);
		text = text + tmp;
	}

	Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug();
	pNode->AddPlugPtr(pPlug);
	pPlug->SetName("luxCustom");
	pPlug->SetValue(std::string(text.Chars()));
}
