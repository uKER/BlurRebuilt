#ifndef AFTK_SPEEDTREE_CLEANUP_H
#define AFTK_SPEEDTREE_CLEANUP_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace AftkSpeedTree
{
	class TreeData;

	class Cleanup
	{
	public:
		Cleanup(TreeData& treeData) : m_pTreeData(&treeData) {}

		void PreProcess(Aftk::C_Scene& scene);

		void PostProcess(Aftk::C_Scene& scene);

	private:
		TreeData*				m_pTreeData;
	};
}

#endif
