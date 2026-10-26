#ifndef EDITOR_APP_H
#define EDITOR_APP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <sigslot/sigslot.h>
#include "Config.h"

namespace Editor
{
	class C_App;
	class C_EventLoop;
	class C_Frame;
	class C_LevelInfoPanel;
	class C_WxScarabForm;
	class C_SelectionToolPanel;
	class C_ActorToolPanel;
	class C_ShortcutHandler;
	class C_ReferencePanel;
	class C_SorceryHypergraphPanel;
	class C_HypergraphPanel;
	class C_AnimationPanel;
	class C_GroupEditPanel;
	class C_VerificationPanel;
	class C_DopeSheetPanel;
	class C_LayerEditPanel;
	class C_ScriptLogPanel;
	class C_SorceryPanel;
	class C_DynamicObjectPanel;

	class C_SceneNodePropertyPanel;
	class C_CameraPropertyPanel;
	class C_ManipulatorPropertyPanel;
	class C_LayerPropertyPanel;

	class C_App : public wxApp
	{
	public:
		struct C_Flags 
		{
			bool postLevelLoadPending : 1;
			bool isDirty : 1;
		};

		virtual bool	OnInit();
		virtual int		OnExit();
	    virtual void	OnInitCmdLine(wxCmdLineParser& parser);
		virtual bool	OnCmdLineParsed(wxCmdLineParser& parser);

		bool InitialiseUI();
		bool UninitialiseUI();
		const EditorConfig& GetConfig() const;
		void UpdateTitleBar(const EditorLib::I_Level* pLevel);

		void OnTimer(wxTimerEvent& e);
		void OnWindowCreate(wxWindowCreateEvent& e);

		C_Frame*					GetFrame();
		C_SceneNodePropertyPanel*	GetSceneNodePropertyPanel();
		C_CameraPropertyPanel*		GetCameraPropertyPanel();
		C_ManipulatorPropertyPanel*	GetManipulatorPropertyPanel();
		C_ShortcutHandler*			GetShortcutHandler();


		XtId GetMainLevelXtRecordId();
		void SetAutosaveInterval(U32 interval);

		void OnDirty();

		virtual int MainLoop();

		//Hack - need to fix event handling.
		wxPanel* GetScriptLogPanel() { return (wxPanel*)m_pScriptLogPanel; }

	private:
			class C_Slots : public sigslot::has_slots<>
		{
		public:
			void Connect();
			void Disconnect();

			void OnLevelEventSlot(const EditorLib::I_Level* level, EditorLib::E_LevelEvent e);
			void OnLevelOpeningSlot(const EditorLib::I_Level& level);
			void OnLevelClosingSlot(const EditorLib::I_Level& level);
			void OnLevelOpenedSlot(const EditorLib::I_Level& level);
			void OnLevelClosedSlot(const EditorLib::I_Level& level);
			void OnLevelNameChangedSlot(const EditorLib::I_Level& level);

			void OnCoreDirtyViewsSlot(EditorLib::E_DirtyEventType e);

			void OnQuitSlot();

			FRIEND_CLASS C_App;
		};

		class C_DummyToolsApp : public Tools::C_ToolsApplication
		{
		public:
			C_DummyToolsApp(const char *szDmpPath)							
			: C_ToolsApplication(kEditorName, kVendorName, "tools.editor", szDmpPath)
			{
			}

			S32 Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress)
			{
				return 0;
			}
		};

		bool InitialiseManipulatorContexts();

	private:
		C_Slots						m_slots;

		C_DummyToolsApp				*m_pDummyToolsApp;
		EditorConfig				m_EditorConfig;
		
		C_Frame*					m_pFrame;
		C_ScriptLogPanel*			m_pScriptLogPanel;
		C_LevelInfoPanel*			m_pLevelInfoPanel;
		C_ReferencePanel*			m_pReferencePanel;
		C_SorceryHypergraphPanel*	m_pSorceryHypergraphPanel;
		C_HypergraphPanel*			m_pHypergraphPanel;
		C_AnimationPanel*			m_pAnimationPanel;
		C_DopeSheetPanel*			m_pDopeSheetPanel;
		C_LayerEditPanel*			m_pLayerEditPanel;
		C_GroupEditPanel*			m_pGroupEditPanel;
		C_VerificationPanel*		m_pVerificationPanel;
		C_SceneNodePropertyPanel*	m_pSceneNodePropertyPanel;
		C_CameraPropertyPanel*		m_pCameraPropertyPanel;
		C_ManipulatorPropertyPanel*	m_pManipulatorPropertyPanel;
		C_LayerPropertyPanel*		m_pLayerPropertyPanel;
		C_SorceryPanel*				m_pSorceryPanel;
		C_DynamicObjectPanel*		m_pDynamicObjectPanel;
		
		wxToolBar*					m_pStandardToolBar;
		wxToolBar*					m_pManipulatorToolBar;
		C_SelectionToolPanel*		m_pSelectionToolPanel;
		wxToolBar*					m_pSnapToolBar;
		wxToolBar*					m_pDisplayToolBar;
		C_ActorToolPanel*			m_pActorTypeToolPanel;

		C_WxScarabForm*				m_pXtDynamicForm;
		C_ShortcutHandler*			m_pShortcutHandler;

		wxTimer						m_Timer;
		wxTimer						m_DirtyTimer;
		wxTimer						m_AutosaveTimer;
		wxTimer						m_NetworkTimer;
		ULONG_PTR					m_gdiplusToken;
		XtId						m_mainLevelId;

		C_Flags						m_flags;
		Btl::Vector<I_View*>		m_views;
	};

	DECLARE_APP(C_App);
}


#endif
