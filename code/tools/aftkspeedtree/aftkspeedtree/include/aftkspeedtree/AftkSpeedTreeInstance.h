#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_SPEEDTREE_INSTANCE_H
#define AFTK_SPEEDTREE_INSTANCE_H

namespace AftkSpeedTree
{

class TreeData;

class InstanceProcessor
{
public:
	InstanceProcessor(TreeData& treeData) : m_pTreeData(&treeData) {}

	void Process(Aftk::C_Scene& aftk);

private:
	TreeData*				m_pTreeData;
	void ProcessNode(Aftk::C_SceneInstanceNode* pInstanceNode);
};

}

#endif
