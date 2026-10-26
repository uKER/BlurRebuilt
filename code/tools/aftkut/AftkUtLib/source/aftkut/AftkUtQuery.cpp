#include "stdafx.h"
#include <AftkUt/AftkUtQuery.h>
#include <AftkUt/AftkUtNodeFlagParser.h>

namespace AftkUt
{
namespace Query
{

//Naming conventions.
static const char* kBaseInstanceConvention				= "baseinstance";
static const char* kBaseInstanceParentConvention		= "InstanceReferences";
static const char* kBaseInstanceLodParentConvention		= "InstanceReferenceLODs";
static const char* kDoNotExportConvention				= "donotexport";
//ActorStudio actor.
static const char* kAsActorConvention					= "as_actor";
//ActorStudio dynamic obj
static const char* kAsDynamic							= "as_dynamic";

static const char* kCollisionPrimitive					= "collisionPrimitive";
static const char* kIsDecalProjectionMesh				= "projectionMesh";
static const char* kIsDecalMesh							= "decalMesh";
static const char* kIsFoliageBillboard					= "biz_grassMesh";

static const char* kCrowdConvention						= "crowd";
static const char* kCrowdPlaceholderConvention			= "Placeholder";

bool IsInstance(const Aftk::C_SceneNode* pSceneNode)
{
	if (!pSceneNode)
		return false;

	return (
		pSceneNode &&
		(pSceneNode->HasType(Aftk::E_ObjType_SceneInstance) || 
		 pSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance)));
}

bool HasBaseInstanceNodeFlags(const Aftk::C_SceneNode* pSceneNode)
{
	bool status;
	AftkUt::C_NodeFlagParser nodeParser(*pSceneNode, &status);
	if (status)
	{
		if (nodeParser.GetFlagGroupCategory() == "System" && nodeParser.GetFlagGroupName() == "BaseInstance")
			return true;
	}

	return false;
}

bool IsBaseInstance(const Aftk::C_SceneNode* pSceneNode)
{
	if (IsInstance(pSceneNode))
		//Instances cannot be base instances.
		return false;

	//Derived from base instance node?
	for (const Aftk::C_SceneNode* pParent = pSceneNode->GetParentPtr(); 
		pParent != 0; pParent = pParent->GetParentPtr())
	{
		if (!stricmp(pParent->GetName().c_str(), kBaseInstanceParentConvention) ||
			!stricmp(pParent->GetName().c_str(), kBaseInstanceLodParentConvention))
			return true;
	}
	
	for (U32 i = 0; i < pSceneNode->GetNumDynamicPlugs(); ++i)
	{
		const Aftk::C_DynamicPlug* pPlug = pSceneNode->GetDynamicPlugPtr(i);
		assert(pPlug);
		if (!stricmp(pPlug->GetName().c_str(), kBaseInstanceConvention))
			return true;
	}

	if (HasBaseInstanceNodeFlags(pSceneNode))
		return true;

	for (const Aftk::C_SceneNode* pParent = pSceneNode->GetParentPtr();
		pParent != 0; pParent = pParent->GetParentPtr())
	{
		if (HasBaseInstanceNodeFlags(pParent))
			return true;
	}

	return false;
}

bool IsBaseCrowdInstance(const Aftk::C_SceneNode* pSceneNode)
{
	if (pSceneNode && AftkUt::CrowdHelper::IsBaseCrowdInstance(pSceneNode))
		return true;

	return false;
}

bool IsCrowdSkeletonNode(const Aftk::C_SceneNode* pSceneNode)
{
	if (pSceneNode && AftkUt::CrowdHelper::IsCrowdSkeleton(pSceneNode))
		return true;

	return false;
}

bool IsCrowdInstance(const Aftk::C_SceneNode* pSceneNode)
{
	if (pSceneNode)
	{
		if (pSceneNode->HasType(Aftk::E_ObjType_SceneInstance)
			|| pSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance)
			|| pSceneNode->HasType(Aftk::E_ObjType_SceneMeshBonedInstance))
		{
			Aftk::C_SceneNode* pSourceNode = NULL;// = pSceneNode->GetSourceNodePtr();

			if (pSceneNode->HasType(Aftk::E_ObjType_SceneInstance))
				pSourceNode = ((Aftk::C_SceneInstanceNode*)pSceneNode)->GetSourceNodePtr();
			else if (pSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
				pSourceNode = ((Aftk::C_SceneMeshRemappedInstanceNode*)pSceneNode)->GetSourceNodePtr();
			else if (pSceneNode->HasType(Aftk::E_ObjType_SceneMeshBonedInstance))
				pSourceNode = ((Aftk::C_SceneMeshBonedInstanceNode*)pSceneNode)->GetSourceNodePtr();

			if (pSourceNode)
			{
				if (IsBaseCrowdInstance(pSourceNode))
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool IsFlagged(const Aftk::C_SceneNode* pSceneNode)
{
	return GetConventionFlags(pSceneNode).data != 0;
}

C_Flags GetConventionFlags(const Aftk::C_SceneNode* pSceneNode)
{
	C_Flags result;
	if (!pSceneNode)
		return result;

	bool bStatus;
	AftkUt::C_NodeFlagParser parser(*pSceneNode, &bStatus);
	if (!stricmp(parser.GetFlagGroupName().c_str(), kBaseInstanceConvention))
		result.baseInstance = true;

	if(bStatus)
	{
		if(_stricmp(parser.GetFlagGroupCategory().c_str(), kCrowdConvention) == 0)
		{
			if (_stricmp(parser.GetFlagGroupName().c_str(), kCrowdPlaceholderConvention) == 0)
				result.asCrowd = true;
		}
	}

	for (U32 i = 0; i < pSceneNode->GetNumDynamicPlugs(); ++i)
	{
		const Aftk::C_DynamicPlug* pPlug = pSceneNode->GetDynamicPlugPtr(i);
		assert(pPlug);
		if (!stricmp(pPlug->GetName().c_str(), kDoNotExportConvention))
			result.doNotExport = true;

		if (!stricmp(pPlug->GetName().c_str(), kAsActorConvention))
			result.asActor = true;

		if (!stricmp(pPlug->GetName().c_str(), kAsDynamic))
		{
			bool b;
			if (pPlug->GetValue(b))
				result.asDynamic = b;
		}

		if (!stricmp(pPlug->GetName().c_str(), kCollisionPrimitive))
			result.collisionPrimitive = true;

		if (!stricmp(pPlug->GetName().c_str(), kIsDecalProjectionMesh))
			result.isConstructionNode = true;

		if (!stricmp(pPlug->GetName().c_str(), kIsDecalMesh))
			result.decal = true;

		if (!stricmp(pPlug->GetName().c_str(), kIsFoliageBillboard))
			result.isBillboard = true;
	}

	if (IsBaseInstance(pSceneNode))
	{
		result.baseInstance = true;
	}

	return result;
}

Aftk::C_SceneNode* GetTopLevelSourceNodePtr(const Aftk::C_SceneNode* pInstance)
{
	Aftk::C_SceneNode* pSource = 0;
	if (pInstance->HasType(Aftk::E_ObjType_SceneInstance))
	{
		pSource = ((Aftk::C_SceneInstanceNode*)pInstance)->GetSourceNodePtr();
	}
	else if (pInstance->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
	{
		pSource = ((Aftk::C_SceneMeshRemappedInstanceNode*)pInstance)->GetSourceNodePtr();
	}

	if (IsInstance(pSource))
		return GetTopLevelSourceNodePtr(pSource);

	return pSource;
}


}
}
