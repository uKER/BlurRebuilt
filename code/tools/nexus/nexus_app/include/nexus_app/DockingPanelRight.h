#ifndef NEXUS_DOCKING_PANEL_RIGHT_H
#define NEXUS_DOCKING_PANEL_RIGHT_H

namespace Nexus
{
	class SceneGraphTree;
	class LayerGrid;

	//Inherit from dockable toolbar class
	class DockingPanelRight : public CExtControlBar
	{
	public:
		bool Create(CWnd* pMainFrame);

		void Destroy();

		void Clear();

		void Update();

		SceneGraphTree* GetSceneGraphTreePtr();

		const SceneGraphTree* GetSceneGraphTreePtr() const;

		LayerGrid* GetLayerGridPtr();

		const LayerGrid* GetLayerGridPtr() const;

	private:
		//Tab page container.
		CExtTabPageContainerWnd		m_tabPageContainerCtrl;

		CExtWFF< SceneGraphTree >*	m_pSceneGraphTreeCtrl;
		LayerGrid*					m_pLayerGridCtrl;
	};
}

#endif
