#ifndef NEXUS_DOCKING_PANEL_BOTTOM_H
#define NEXUS_DOCKING_PANEL_BOTTOM_H

namespace Nexus
{
	class CommandView;

	class DockingPanelBottom : public CExtControlBar
	{
	public:
		bool Create(CWnd* pMainFrame);

		void Destroy();

		void Clear();

		void Update();

		CommandView* GetCommandViewPtr();

		const CommandView* GetCommandViewPtr() const;

	private:
		CExtTabPageContainerWnd	m_tabPageContainerCtrl;

		CommandView m_commandViewDlg;
	};
}

#endif
