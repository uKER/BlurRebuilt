// MainFrm.h : interface of the MainFrame class
//


#pragma once

class Vis;

class MainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	MainFrame();
	DECLARE_DYNCREATE(MainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~MainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;

#endif

	static void	OutputLogText(const std::string& text);

	void		OnCriticalError(Tu::Status status);
	void		OnError(Tu::Status status);
	void		OnMessage(Tu::Status status);

	void ClearLogText();

protected:  // control bar embedded members
	void CreateBottomToolBar();

	CExtMenuControlBar				m_wndMenuBar;

	CStatusBar						m_wndStatusBar;


	WINDOWPLACEMENT					m_dataFrameWP;

	CExtToolControlBar				m_wndToolBar;

	//Text output tool bar for the bottom of the screen
	CExtControlBar					m_wndBottomToolbar;			// toolbar
	CExtWFF< CHistoryEdit >			m_wndInBottomBarListBox;	// output text toolbar
	//CExtTabPageContainerWnd			m_wndInBottomBarTabContainer;

	//Main toolbar for the rhs
	//CExtWFF< CListBox >				m_wndInMainBarListBox;
//	CFixedWidthToolbar				m_wndMainTabbedToolbar;
	//CExtWRB< CWndSliderView >		m_wndInBarDlg;

	void SetCmdIcon( UINT nCmd, UINT nIconID );

	Vis *m_pVis;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnFileImport();
protected:
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
public:
	virtual BOOL DestroyWindow();
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle = WS_OVERLAPPEDWINDOW, const RECT& rect = rectDefault, CWnd* pParentWnd = NULL, LPCTSTR lpszMenuName = NULL, DWORD dwExStyle = 0, CCreateContext* pContext = NULL);
	afx_msg void OnDebugCalculatevis();
	afx_msg void OnDebugShowcell();
	afx_msg void OnDebugLoaddatabase();
	afx_msg void OnDebugShowusage();
};


