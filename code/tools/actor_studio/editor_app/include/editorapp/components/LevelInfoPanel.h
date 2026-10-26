#ifndef EDITOR_LEVEL_INFO_PANEL_H
#define EDITOR_LEVEL_INFO_PANEL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <sigslot/sigslot.h>
#include <EditorApp/Components/scene_graph/SceneGraphEnums.h>

class wxNotebook;

namespace Editor
{
	class C_LevelPackPanel;
	class C_EditorSceneGraphPanel;

	class C_LevelInfoPanel : public wxPanel
	{
	public:
		struct C_Flags
		{
		};

		C_LevelInfoPanel(
			wxWindow *parent, 
			wxWindowID = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(200,400),
			long style = wxTAB_TRAVERSAL|wxNO_BORDER);

		virtual ~C_LevelInfoPanel();

		virtual void Shutdown();

	private:

		wxNotebook*				 m_pTabs;
		C_EditorSceneGraphPanel* m_pSceneGraphPanel;
		C_LevelPackPanel*		 m_pLevelPackPanel;
	};
}

#endif
