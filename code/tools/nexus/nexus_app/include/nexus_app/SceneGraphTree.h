#ifndef NEXUS_SCENE_GRAPH_TREE_H
#define NEXUS_SCENE_GRAPH_TREE_H

namespace Nexus
{
	class SceneGraphTree : public CTreeCtrlEx
	{
	public:
		bool Create(CWnd* pParentWnd);

		void Destroy();

		void ClearSelection();

		void Clear();

		void Update();

		// This is used to test for duplicate TVN_SELCHANGED notifications from CTreeCtrlEx
		BOOL OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);

		void ToggleSelection(const Aftk::C_SceneNode* pAftkSceneNode);

	private:
		DECLARE_MESSAGE_MAP();
		HTREEITEM Update(const Aftk::C_SceneNode* pAftkSceneNode, HTREEITEM hParent);

		//This table maps from an HTREEITEM to an Aftk unique node ID.
		std::map<HTREEITEM, U32>		m_treeCtrlAftkMapping;
		std::map<U32, HTREEITEM>		m_aftkTreeCtrlMapping;
	};
};

#endif
