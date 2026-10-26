#ifndef DMAN_APP_H
#define DMAN_APP_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Debug
{
	class TextSubscriber;
}

namespace DMan
{

class C_ShortcutHandler;
static const std::bad_alloc outOfMemEx;

class C_App : public wxApp
{
public:
	virtual bool		OnInit();
	virtual int			OnExit();
	virtual void		OnInitCmdLine(wxCmdLineParser& parser);
	virtual bool		OnCmdLineParsed(wxCmdLineParser& parser);
	virtual	void		OnTimer(wxTimerEvent& e);

	C_Frame*			GetFrame();

	Xt::DomDb*			GetDb();
	Xt::DomPack*		GetCurrentPack();

	C_XtTypeDatabase&	GetXtTypeDatabase();
	C_DbManager&		GetDbManager();

	DialogueManConfig&  GetConfig();

	void				SetSelection(const C_Selection& selection);
	void				GetSelection(C_Selection& selection);

	void						 SetRecordsInCurrentFolder(const Btl::Vector<Xt::DomRecord*>& records);
	Btl::Vector<Xt::DomRecord*>& GetRecordsInCurrentFolder();

	bool				OpenDb();
	void				CloseDb();
	bool				SaveDb();

	void				SorceryToTemp();
	void				TempToSorcery();
	bool				InMemorySavingMode();

	void				AddForDeferredUpdate(XtShared::C_XtObjectImpl* pImpl);

	void				RemoveForDeferredUpdate(XtShared::C_XtObjectImpl* pImpl);

	bool				IsDeferredUpdatePending();

	Debug::Logger&		GetLogger() const;

	C_GlobalSignals&	GetGlobalSignals();

	void				IdleTick();

	void				EnableViews(bool);

	virtual int			OnRun();

private:
	class C_Slots : public sigslot::has_slots<>
	{
	public:
		void Connect();
		void Disconnect();

		void OnQuitSlot();

		FRIEND_CLASS C_App;
	};

	struct C_Flags
	{
		bool pendingSync : 1;
	};

	class C_DummyToolsApp : public Tools::C_ToolsApplication
	{
	public:
		C_DummyToolsApp(const char *szDmpPath)							
		: C_ToolsApplication(kEditorName, kVendorName, "tools.dman", szDmpPath)
		{
		}

		S32 Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress)
		{
			return 0;
		}
	};

	static void StaticOnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent, 
			C_App* pThis);

	bool				InitialiseUI();
	void				UninitialiseUI();
	void				OnOpenDb();
	void				OnCloseDb();

	wxTimer				m_dirtyTimer;
	C_Slots				m_slots;
	C_Flags				m_flags;
	DialogueManConfig	m_DialogueManConfig;
	C_Frame*			m_pFrame;
	ULONG_PTR			m_gdiplusToken;

	//Db
	C_XtTypeDatabase*	m_pXtTypeDatabase;
	Xt::DomDb*			m_pDb;
	Xt::DomPack*		m_pCurrentPack;
	Xt::DomMonitor		m_dbMonitor;

	C_DummyToolsApp*	m_pDummyToolsApp;

	Debug::TextSubscriber*	m_pLogTextSubscriber;
	wxTextCtrl*			m_pLogText;

	//Any state necessary for undo.
	C_NonXtState		m_nonXtState;

	C_DbManager*		m_pDbManager;

	typedef Btl::OpenHashSet<XtShared::C_XtObjectImpl*> XtObjectImplSet;
	XtObjectImplSet			m_deferredUpdates;

	mutable C_GlobalSignals*	
						m_pGlobalSignals;

	int					m_viewsEnabled;
	C_ShortcutHandler*	m_pShortcutHandler;

	Btl::Vector<Xt::DomRecord*> m_recordsInCurrentFolder;

	//Memory Saving Mode
	wxArrayString		m_packsToKeep;
	bool				m_inMemorySavingMode;
};

DECLARE_APP(C_App);

} //namespace DMan

#endif //DMAN_APP_H
