#ifndef EDITOR_REFERENCE_DIALOG_H
#define EDITOR_REFERENCE_DIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

#include "sigslot/SigSlot.h"

class wxTreeListCtrl;

namespace Editor
{
	class C_ReferenceData : public wxTreeItemData
	{
	public:
		C_ReferenceData(const EditorLib::C_AftkRef *pAftkRef);

		~C_ReferenceData ();

		const EditorLib::C_AftkRef*	GetAftkRef() const;

	private:
		const EditorLib::C_AftkRef*	m_pAftkRef;
	};


	class C_ReferencePanel : public wxPanel
	{
	public:
		C_ReferencePanel(wxWindow *parent,  wxWindowID id = wxID_ANY,
							const wxPoint& pos = wxDefaultPosition,
							const wxSize& size = wxSize(500, 300),
							long style = wxTAB_TRAVERSAL|wxNO_BORDER);

		~C_ReferencePanel();
		void Shutdown();

		void OnNodeActivate(wxTreeEvent& e);
		void OnNodeSelect(wxTreeEvent& e);
		void OnAddReference(wxCommandEvent& e);
		void OnRefreshReference(wxCommandEvent& e);
		void OnLoadAllReference(wxCommandEvent& e);
		void OnUnloadAllReference(wxCommandEvent& e);
		void OnGetLatestReferences(wxCommandEvent& e);
		void OnKeyDown(wxTreeEvent& e);
		void OnExtRefCbClicked(wxCommandEvent& e);
		void OnIdle(wxIdleEvent& e);

		void Populate();

		void AddRefNode(const char* pPath);
		void RemoveRefNode(const char* pPath);

		void AddRefNode(const EditorLib::C_AftkRef *pRef);
		void RemoveRefNode(const EditorLib::C_AftkRef *pRef);

	private:

		class C_Slots : public sigslot::has_slots<>
		{
		public: 
			C_Slots(C_ReferencePanel *pDialog);
			void Connect();
			void Disconnect();

			void OnLevelAftkRefAddedSlot(const Btl::String& originalFilePath);
			void OnLevelAftkRefRemovedSlot(const Btl::String& originalFilePath);
			void OnAftkRefLoadedSlot(
					const Btl::String& originalFilePath, 
					const Aftk::C_Scene& aftkScene, 
					const EditorLib::C_SceneNode& sceneNode, 
					const EditorLib::AftkDgMap& aftkDgMap);
			void OnAftkRefUnloadedSlot(const Btl::String& originalFilePath);

			void OnLevelEventSlot(const EditorLib::I_Level* level, EditorLib::E_LevelEvent e);
			void OnLevelOpeningSlot(const EditorLib::I_Level& level);
			void OnLevelOpenedSlot(const EditorLib::I_Level& level);
			void OnLevelClosingSlot(const EditorLib::I_Level& level);
			void OnLevelClosedSlot(const EditorLib::I_Level& level);

		private:
			C_ReferencePanel* m_pDialog;
		};

		struct C_Flags
		{
			bool m_connected	: 1;
			bool m_pendingSync	: 1;
			bool m_syncOnIdle	: 1;
		};

		void SetActiveLevel(const EditorLib::I_Level* level);

		C_Slots		m_Slots;

		wxTextCtrl	*m_pScenePathCtrl;
		wxCheckBox	*m_pScenePathIsExtRefCb;
		wxTextCtrl	*m_pCachePathCtrl;
		wxTextCtrl	*m_pVisIdCtrl;

		wxTreeListCtrl	*m_pReferenceListCtrl;
		wxTreeItemId	m_levelRoot;
		wxTreeItemId	m_globalRoot;
		wxTreeItemId	m_visualisationRoot;

		const EditorLib::I_Level* m_Level;

		mutable C_Flags	m_flags;

		typedef Btl::OpenHashMap<Btl::String, wxTreeItemId, Btl::HashKeyOpBtlString> StringTreeMap;
		StringTreeMap m_VisualisationMap;
		StringTreeMap m_LevelMap;
		StringTreeMap m_GlobalMap;

		DECLARE_CLASS(C_ReferencePanel)
	};

}
#endif