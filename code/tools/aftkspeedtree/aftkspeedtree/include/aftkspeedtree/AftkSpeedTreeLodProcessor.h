#ifndef AFTK_SPEEDTREE_LOD_H
#define AFTK_SPEEDTREE_LOD_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace AftkSpeedTree
{
	class TreeData;

	class LodProcessor
	{
	public:
		LodProcessor(TreeData& treeData) : m_pTreeData(&treeData) {}

		void Process();

	private:
		void GenerateBaseInstance(Aftk::C_SceneMeshNode* pHighLod);

		void GenerateInstance(Aftk::C_SceneInstanceNode* pToReplace);

		TreeData*						m_pTreeData;
		Ut::Vector<Aftk::C_SceneInstanceNode*>	m_newBillboards;
	};
}

#endif
