#include "stdafx.h"

#include <AftkSpeedTree/AftkSpeedTree.h>
#include <AftkSpeedTree/AftkSpeedTreeMesh.h>
#include <AftkSpeedTree/AftkSpeedTreeInstance.h>
#include <AftkSpeedTree/AftkSpeedTreeLodProcessor.h>
#include <AftkSpeedTree/AftkSpeedTreeCleanup.h>
#include <AftkSpeedTree/AftkSpeedTreeData.h>
#include <system/debug/log/debuglog.h>

using namespace AftkSpeedTree;

namespace AftkSpeedTree
{
	Debug::Logger &g_Log = Debug::GetLogger("tools.aftkspeedtree");
}

SpeedTree::SpeedTree()
: SpeedTreeSetup()
{
}

void SpeedTree::ProcessAftk(Aftk::C_Scene &aftk)
{
	g_Log.SetLevel(DebugLevel::Info);
	AftkSpeedTree::TreeData				treeData;

	Cleanup cleanup(treeData);
	//cleanup.PreProcess(aftk);

	AftkSpeedTree::MeshGenerator		meshProcessor(treeData);
	AftkSpeedTree::InstanceProcessor	instanceProcessor(treeData);
	AftkSpeedTree::LodProcessor			lodProcessor(treeData);
			
	// get settings from base class
	treeData.Setup(*this);

	// first generate all the scene mesh nodes for each speed tree node
	meshProcessor.Process(aftk);
	
	// process instances
	instanceProcessor.Process(aftk);

	lodProcessor.Process();

	//Remove billboard placeholders.
	const ToDeleteArray& toDelete = treeData.GetToDeleteArray();
	for (U32 i = 0; i < (U32)toDelete.Size(); i++)
	{
		Aftk::C_SceneNode* pSceneNode = toDelete[i];
		aftk.DeleteSceneNodeAndReparentChildren(pSceneNode, true);
	}

	//Perform final scene cleanp.
	cleanup.PostProcess(aftk);
}


