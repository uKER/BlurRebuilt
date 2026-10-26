#include "stdafx.h"
#include <AftkSpeedTree/AftkSpeedTreeLodProcessor.h>
#include <AftkSpeedTree/AftkSpeedTreeData.h>

using namespace AftkSpeedTree;

//#pragma optimize("", off)

void LodProcessor::Process()
{
	//Process base mesh first.
	const TreeMeshSet& treeMeshSet = m_pTreeData->GetTreeMeshSet();
	for (TreeMeshSet::ConstIterator it = treeMeshSet.Begin(); it != treeMeshSet.End(); ++it)
	{
		GenerateBaseInstance(it->GetKey());
	}
	
	//Now process instances.
	const TreeInstanceSet& treeInstanceSet = m_pTreeData->GetTreeInstanceSet();
	{
		Ut::Vector<Aftk::C_SceneInstanceNode*> toProcess;
		for (TreeInstanceSet::ConstIterator it = treeInstanceSet.Begin(); it != treeInstanceSet.End(); ++it)
			toProcess.PushBack(it->GetKey());

		for (U32 i = 0; i < (U32)toProcess.Size(); i++)
			GenerateInstance(toProcess[i]);
	}

	//Add to our list of billboards to generate.
	for (U32 i = 0; i < (U32)m_newBillboards.Size(); i++)
	{
		Aftk::C_SceneInstanceNode* pInstanceNode = 
			m_newBillboards[i];

		Aftk::C_SceneNode* pTopLevel = pInstanceNode->GetTopLevelSourceNodePtr();
		Aftk::C_SceneMeshNode* pMesh;
		if (pTopLevel->GetType() == Aftk::E_ObjType_SceneMesh)
			pMesh = (Aftk::C_SceneMeshNode*)pTopLevel;
		else
		{
			assert(!"Encountered top-level node other than mesh!");
			pMesh = 0;
		}

		assert(pInstanceNode->GetParentPtr()->GetType() == Aftk::E_ObjType_SceneLodGroup);

		m_pTreeData->AddTreeInstanceInfo(
			pInstanceNode->GetWorldMatrix(), 
			pInstanceNode,
			pMesh,
			//Our parents are lod groups.
			(Aftk::C_SceneLodGroupNode*)pInstanceNode->GetParentPtr());
	}
}

static unsigned long LowestPow2(const unsigned long nSize)
{
	if (nSize < 1) return 1;
	unsigned long x = 1 << 31;
	for (; !(x & nSize); x >>= 1);
	return x;
}

void LodProcessor::GenerateBaseInstance(Aftk::C_SceneMeshNode* pHighLod)
{
	Aftk::C_Scene* pScene = pHighLod->GetScenePtr();
	Aftk::C_SceneNode* pSPTNode = pHighLod->GetParentPtr();

	//Create a new Lod group.
	Aftk::C_SceneLodGroupNode* pNewLodGroup = new Aftk::C_SceneLodGroupNode(pScene);

	//inherit layer membership
	for (U32 i=0; i<pSPTNode->GetNumLayers(); ++i)
	{
		Aftk::C_LayerNode *pLayerNode = pSPTNode->GetLayerPtr(i);
		pLayerNode->AddMember(pNewLodGroup);
	}


	//copy object flags from SPT placeholder to LodGroup
	{
		Aftk::C_SceneNode *pSPTNode = pHighLod->GetParentPtr();
		if (pSPTNode && AftkUt::C_NodeFlagParser::IsFlagged(*pSPTNode))
		{
			AftkUt::C_NodeFlagParser parser(*pSPTNode);

			//apply flag overrides
			const SpeedTreeSetup& setup = m_pTreeData->GetSetup();
			for (S32 i=0; i<setup.m_flagOverrides.Size(); ++i)
				parser.SetParameterValue(setup.m_flagOverrides[i].name.Chars(), setup.m_flagOverrides[i].value.Chars(), AftkUt::E_ParamType_Unknown);

			parser.StoreFlags(*pNewLodGroup);
		}
	}

	//Add to scene-graph.
	pHighLod->GetParentPtr()->AddChildPtr(pNewLodGroup);

	//Set a name to indicate the processing that's been done/
	pNewLodGroup->SetName(pHighLod->GetName() + "_treelod_BASE");
	//Set our transformation in world-space, also preserving the world-space transformations of our
	//children.
	
	pNewLodGroup->SetWorldMatrix(pHighLod->GetWorldMatrix(), true);

	//Reset high lod local matrix to identity.
	Aftk::Matrix44 id;
	UtMaths::SetIdentity(&id);
	pHighLod->SetLocalMatrix(id, false);

	pScene->ExtractSceneNodeAndReparentChildren(pHighLod, true);

	std::string baseName = pHighLod->GetName();

	//Setup high lod info.
	{
		pNewLodGroup->AddChildPtr(pHighLod);

		pHighLod->SetName(baseName + "_high");
		Aftk::C_GroupLodLevelInfo info;
		info.SetSceneNodePtr(pHighLod);

		F32 lodThreshold = 0;
		F32 lodDistanceMax = 400.f;
		{
			bool status;
			AftkUt::C_NodeFlagParser parser(*pSPTNode, &status);
			if (status)
			{
				std::string lodSwitchDistance;
				if (parser.GetParameterValue("LODSwitchDistance", lodSwitchDistance))
				{
					lodThreshold = (F32)atof(lodSwitchDistance.c_str());
					LOGDEBUG( g_Log, "Found LOD Switch Distance of %.2f for node %s\n",
						lodThreshold, pSPTNode->GetName().c_str());
				}

				//clamp lod switch distance to maxdrawdistance if non zero
				F32 maxDrawDistance = +FLT_MAX;
				if (parser.GetParameterValue("MaxDrawDistance", maxDrawDistance) && maxDrawDistance > Maths::Constants<F32>::Epsilon())
					lodDistanceMax = maxDrawDistance;
			}
		}

		//Define our own threshold if none was specified.
		if (lodThreshold == 0)
		{
			const Aftk::Vector3F32& iExtentSize =
				pHighLod->GetWorldBounds().GetMax() - pHighLod->GetWorldBounds().GetCentre();

			F32 size = Maths::Sqrt(pHighLod->GetWorldBounds().GetObjectRadiusSquared());

			const F32 treeLodDistanceFactor = 30.f;
			//Clamp to 40m minimum.
			const F32 treeLodDistanceMin = 40.f;
			//Clamp to draw distance maximum.
			const F32 treeLodDistanceMax = lodDistanceMax;
			const F32 heightCutOff = 7.f;
			const F32 heightFactor = 12;
			const F32 lodOffset = 40;

			F32 height = 
				pHighLod->GetWorldBounds().GetMax().Y() - pHighLod->GetWorldBounds().GetMin().Y();
			F32 heightScale = height - heightCutOff;
			heightScale = Maths::Max(0.f, heightScale);
			heightScale = heightScale * 10;

			lodThreshold = size * treeLodDistanceFactor + heightScale + lodOffset;

			lodThreshold *= 0.80f;

			lodThreshold = Maths::Max(treeLodDistanceMin, lodThreshold);
			lodThreshold = Maths::Min(treeLodDistanceMax, lodThreshold);

			F32 f = (F32)LowestPow2((unsigned long)(lodThreshold*1.3f));

			LOGINFO( g_Log, "Automatically determined LOD Switch Distance of %.2f for node %s\n",
				f, pSPTNode->GetName().c_str());
		}

		lodThreshold = (F32)LowestPow2((unsigned long)(lodThreshold*1.3f));

		info.SetThreshold(lodThreshold);
		pNewLodGroup->AddLodLevelInfo(info);
	}
}

void LodProcessor::GenerateInstance(Aftk::C_SceneInstanceNode* pToReplace)
{
	Aftk::C_Scene* pScene = pToReplace->GetScenePtr();

	//By the time we get here we'll have already had our base-instance lod groups generated.

	if (!pToReplace->GetSourceNodePtr() || 
		!pToReplace->GetSourceNodePtr()->GetParentPtr() || 
		pToReplace->GetSourceNodePtr()->GetParentPtr()->GetType() != Aftk::E_ObjType_SceneLodGroup)
	{
		LOGDEBUG( g_Log, "\tLodProcessor: Instance %s is pointing at %s which is not a lod group.\n",
			pToReplace->GetName().c_str(), pToReplace->GetSourceNodePtr()->GetName().c_str());
		return;
	}

	Aftk::C_SceneLodGroupNode* pBaseInstanceLodGroup = 
		(Aftk::C_SceneLodGroupNode*)pToReplace->GetSourceNodePtr()->GetParentPtr();

	//Remove billboard placeholder.
	if (pToReplace->GetType() == Aftk::E_ObjType_SceneInstance)
	{
		//Remove our tracking of the to-be deleted instance.
		m_pTreeData->RemoveFromToDeleteArray(pToReplace);
	}

	Aftk::C_SceneLodGroupNode* pNewLodGroup = AftkUt::C_ReplaceWithLodGroup::ReplaceWithLodGroup(
		pToReplace,
		pBaseInstanceLodGroup);

	//copy node flags to child instances
	if (pNewLodGroup)
	{
		AftkUt::C_NodeFlagParser parser(*pNewLodGroup);
		for (U32 i=0; i<pNewLodGroup->GetNumLodLevels(); ++i)
		{
			Aftk::C_GroupLodLevelInfo& info = pNewLodGroup->GetLodLevelInfo(i);
			if (info.GetSceneNodePtr())
				parser.StoreFlags(*info.GetSceneNodePtr());
		}
	}
}


