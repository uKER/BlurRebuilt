// MainFrm.cpp : implementation of the MainFrame class
//

#include <vis_app/stdafx.h>
#include <vis_app/Nexus.h>

#include <vis_app/MainFrm.h>

#include "../../resource.h"
#include "vis_core/vis.h"
#include "vis_app/NexusSystem.h"
#include "vis_app/NexusDoc.h"
#include "..\..\include\vis_app\mainfrm.h"
#include "vis_app/NexusRender.h"
#include "Tools_Common\MemoryDbg.h"
#include "vis_app/CellVisDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void MemMirrorOn(void);

MainFrame* g_pMainFrame = 0;
// MainFrame

IMPLEMENT_DYNCREATE(MainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(MainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND_EX(IDR_TOOLBARX, OnBarCheck)
	ON_UPDATE_COMMAND_UI(IDR_TOOLBARX, OnUpdateControlBarMenu)

	ON_COMMAND_EX(ID_VIEW_OUTPUT_TOOLBAR, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUT_TOOLBAR, OnUpdateControlBarMenu)

	ON_WM_INITMENU()
	ON_WM_ACTIVATE()
	ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
	ON_COMMAND(ID_DEBUG_CALCULATEVIS, OnDebugCalculatevis)
	ON_COMMAND(ID_DEBUG_SHOWCELL, OnDebugShowcell)
	ON_COMMAND(ID_DEBUG_LOADDATABASE, OnDebugLoaddatabase)
	ON_COMMAND(ID_DEBUG_SHOWUSAGE, OnDebugShowusage)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// MainFrame construction/destruction

MainFrame::MainFrame()
{
	memset(&m_dataFrameWP, 0, sizeof(WINDOWPLACEMENT));
	m_dataFrameWP.length = sizeof(WINDOWPLACEMENT);
	m_dataFrameWP.showCmd = SW_HIDE;
	g_pMainFrame = this;
	m_pVis = NULL;
	MemMirrorOn();
}

MainFrame::~MainFrame()
{
	if (m_pVis)
		delete m_pVis;
}

// associate an icon with a menu item
void MainFrame::SetCmdIcon( UINT nCmd, UINT nIconID ) 
{
    HICON hNewIcon = NULL;
    HINSTANCE hInstResource =
        AfxFindResourceHandle(
            MAKEINTRESOURCE( nIconID ),
            RT_GROUP_ICON
        );
    ASSERT( hInstResource != NULL );
    if( hInstResource != NULL ) {
        hNewIcon =
            (HICON)::LoadImage(
                hInstResource,
                MAKEINTRESOURCE( nIconID ),
                IMAGE_ICON,
                16, // icon’s width, in pixels 
                16, // icon’s height, in pixels
                0
            );
        ASSERT( hNewIcon != NULL );
    }
    if( hNewIcon != NULL ){
        g_CmdManager->CmdSetIcon( 
            g_CmdManager->ProfileNameFromWnd( GetSafeHwnd() ), 
            nCmd,  
            hNewIcon, 
            true //(false - use hIcon as is, true - store cloned hIcon)
        );
        
        // If a new icon is of the same size as the previous you should invalidate your toolbar, 
        // otherwise, you need to recalculate the layout of toolbar’s parent frame:
        if(m_wndToolBar .IsVisible() ){
            m_wndToolBar.GetParentFrame()->RecalcLayout();
            m_wndToolBar.Invalidate();
            m_wndToolBar.UpdateWindow();
        }
    }
}


int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
#ifdef _DEBUG
//	MemMirrorOn();
#endif

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	g_CmdManager->ProfileSetup( "Vis", GetSafeHwnd());

	if (Nexus::g_pNexusRender)
	{
		SRD::SceneRendererSettings& settings = Nexus::g_pNexusRender->Settings();

		CWinApp * pApp = ::AfxGetApp();

#pragma warning(push)
#pragma warning(disable:4800)
		S32 temp;


		temp = pApp->GetProfileInt(
			"lighting",
			"userealtimelighting", 
			settings.UseRealTimeLighting());
		settings.SetUseRealTimeLighting((bool)temp);

		temp = pApp->GetProfileInt(
			"shading",
			"usetextures", 
			settings.UseTextures());
		settings.SetUseTextures((bool)temp);
		temp = pApp->GetProfileInt(
			"shading",
			"componentMode", 
			settings.ComponentMode());
		settings.SetComponentMode((U32)temp);

		temp = pApp->GetProfileInt(
			"view", 
			"userealtimelighting",		
			settings.ShowGrid());
		settings.SetShowGrid((bool)temp);

		temp = pApp->GetProfileInt(
			"view", 
			"rendernormals",		
			settings.RenderNormals());
		settings.SetRenderNormals((bool)temp);

		temp = pApp->GetProfileInt(
			"view", 
			"tangentspacemode",		
			settings.TangentSpaceMode());
		settings.SetTangentSpaceMode(temp);

		temp = pApp->GetProfileInt(
			"view", 
			"renderaxes",		
			settings.RenderAxes());
		settings.SetRenderAxes((bool)temp);
		temp = pApp->GetProfileInt(
			"view", 
			"rendersplines",		
			settings.RenderSplines());
		settings.SetRenderSplines((bool)temp);
		temp = pApp->GetProfileInt(
			"view", 
			"renderjoints",		
			settings.RenderJoints());
		settings.SetRenderJoints((bool)temp);
		temp = pApp->GetProfileInt(
			"view", 
			"renderpointlights",		
			settings.RenderPointLights());
		settings.SetRenderPointLights((bool)temp);
		temp = pApp->GetProfileInt(
			"view", 
			"renderdirectionallights",		
			settings.RenderDirectionalLights());
		settings.SetRenderDirectionalLights((bool)temp);
		temp = pApp->GetProfileInt(
			"view", 
			"renderspotlights",		
			settings.RenderSpotLights());
		settings.SetRenderSpotLights((bool)temp);
		temp = pApp->GetProfileInt(
			"view", 
			"renderinstances",		
			settings.RenderInstances());
		settings.SetRenderInstances((bool)temp);
		temp = pApp->GetProfileInt(
			"view", 
			"rendermeshes",		
			settings.RenderMeshes());
		settings.SetRenderMeshes((bool)temp);
		temp = pApp->GetProfileInt(
			"view", 
			"lightmapmode",		
			settings.LightmapMode());
		settings.SetLightmapMode((U32)temp);
		temp = pApp->GetProfileInt(
			"view", 
			"skinningmode",		
			settings.SkinningMode());
		settings.SetSkinningMode((U32)temp);
		temp = pApp->GetProfileInt(
			"view", 
			"boundingboxmode",		
			settings.BoundingBoxMode());
		settings.SetBoundingBoxMode((U32)temp);
#pragma warning(pop)
	}
	
	if (!m_wndMenuBar.Create( "Menu", this, IDR_MAINFRAME ))
	{
		TRACE0("Failed to create menubar\n");
		return -1; 
	}

/*	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
*/

	if( !m_wndToolBar.Create(
			_T( "Toolbar" ),
			this,
			IDR_TOOLBARX
		)
		|| !m_wndToolBar.LoadToolBar( IDR_TOOLBARX )
		)
	{
		TRACE0( "Failed to create toolbar" );
		return -1;
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	CreateBottomToolBar();

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.FrameEnableDocking(this);
	m_wndBottomToolbar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndBottomToolbar.FrameEnableDocking(this);
	m_wndBottomToolbar.DockControlBar( AFX_IDW_DOCKBAR_BOTTOM, 1, this, false );
	DockControlBar(&m_wndToolBar);

	// this disables the hiding of rarely used commands
	CExtPopupColorMenuWnd::g_bMenuExpanding = false;
	CExtPopupColorMenuWnd::g_bMenuHighlightRarely = false;

	SetCmdIcon( ID_DEBUG_CALCULATEVIS,	IDR_MAINFRAME);

	// load recent file list etc.
	g_CmdManager->SerializeState( "Vis", "Bizarre Creations", "Vis", false);

	// load toolbar positions
	if (!CExtControlBar::ProfileBarStateLoad(
			this,
			theApp.m_pszRegistryKey,
			theApp.m_pszProfileName,
			theApp.m_pszProfileName,
			&m_dataFrameWP
			))
	{
	}
//	pNexusDoc->SetMenuPtr(&m_wndMenuBar);

	Nexus::g_pNexusSystem->SetLogCallback(OutputLogText);
	return 0;
}

void MainFrame::CreateBottomToolBar()
{
	if(	!m_wndBottomToolbar.Create(
			_T("Output window"),
			this,
			ID_VIEW_OUTPUT_TOOLBAR
			)
		)
	{
		return;
	}

	if( !m_wndInBottomBarListBox.Create(
			ES_MULTILINE | WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL
				|LBS_HASSTRINGS|LBS_NOINTEGRALHEIGHT				
				,
			CRect(0,0,0,0),
			&m_wndBottomToolbar,
			m_wndBottomToolbar.GetDlgCtrlID()
			)
		)
	{
		return;
	}

	m_wndInBottomBarListBox.SetFont(
		CFont::FromHandle(
			(HFONT)::GetStockObject(DEFAULT_GUI_FONT)
			)
	);

//	m_wndInBottomBarListBox.AppendString("Hello");
	m_wndInBottomBarListBox.AllowSelection(true);
	

//	m_wndInBottomBarTabContainer.
}

BOOL MainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// MainFrame diagnostics

#ifdef _DEBUG
void MainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void MainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG

// print debug output
void MainFrame::OutputLogText(const std::string& text)
{
	static std::string buffer;
	for (U32 i = 0; i < text.length(); i++)
	{
		if (text[i] == '\n')
		{
			g_pMainFrame->m_wndInBottomBarListBox.AppendString(buffer.c_str());
			buffer.clear();
		}
		else
		{
			buffer += text[i];
		}
	}
}

void MainFrame::ClearLogText()
{
	g_pMainFrame->m_wndInBottomBarListBox.SetWindowText("");
}

void MainFrame::OnCriticalError(Tu::Status status)
{
	MessageBox(status.ErrorString().c_str(), "Nexus Critical Error.", MB_OK);
	PostMessage(WM_CLOSE, 0, 0);
}

void MainFrame::OnError(Tu::Status status)
{
	MessageBox(status.ErrorString().c_str(), "Nexus Error", MB_OK);
}

void MainFrame::OnMessage(Tu::Status status)
{
	MessageBox(status.ErrorString().c_str(), "Nexus Message", MB_OK);
}
// MainFrame message handlers
BOOL MainFrame::PreTranslateMessage(MSG* pMsg)
{
    if( m_wndMenuBar.TranslateMainFrameMessage(pMsg) )
	{
       return TRUE;
	}

	return CFrameWnd::PreTranslateMessage(pMsg);
}

void MainFrame::OnInitMenu(CMenu* pMenu)
{
	CFrameWnd::OnInitMenu(pMenu);
}

void MainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);
}

void MainFrame::OnFileImport()
{
	// TODO: Add your command handler code here
	
}

void MainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	CFrameWnd::OnUpdateFrameTitle(bAddToTitle);
}

BOOL MainFrame::DestroyWindow()
{
	CWinApp * pApp = ::AfxGetApp();

    CExtControlBar::ProfileBarStateSave(
        this,
        pApp->m_pszRegistryKey,
        pApp->m_pszProfileName,
        pApp->m_pszProfileName
        );

	if (Nexus::g_pNexusRender)
	{
		SRD::SceneRendererSettings& settings = Nexus::g_pNexusRender->Settings();

		pApp->WriteProfileInt(
			"lighting", 
			"userealtimelighting",		
			settings.UseRealTimeLighting());
		
		pApp->WriteProfileInt(
			"shading", 
			"usetextures",		
			settings.UseTextures());
		pApp->WriteProfileInt(
			"shading", 
			"componentMode",		
			settings.ComponentMode());

		pApp->WriteProfileInt(
			"view", 
			"userealtimelighting",		
			settings.ShowGrid());
		pApp->WriteProfileInt(
			"view", 
			"rendernormals",		
			settings.RenderNormals());
		pApp->WriteProfileInt(
			"view", 
			"tangentspacemode",		
			settings.TangentSpaceMode());
		pApp->WriteProfileInt(
			"view", 
			"renderaxes",		
			settings.RenderAxes());
		pApp->WriteProfileInt(
			"view", 
			"rendersplines",		
			settings.RenderSplines());
		pApp->WriteProfileInt(
			"view", 
			"renderjoints",		
			settings.RenderJoints());
		pApp->WriteProfileInt(
			"view", 
			"renderpointlights",		
			settings.RenderPointLights());
		pApp->WriteProfileInt(
			"view", 
			"renderdirectionallights",		
			settings.RenderDirectionalLights());
		pApp->WriteProfileInt(
			"view", 
			"renderspotlights",		
			settings.RenderSpotLights());
		pApp->WriteProfileInt(
			"view", 
			"renderinstances",		
			settings.RenderInstances());
		pApp->WriteProfileInt(
			"view", 
			"rendermeshes",		
			settings.RenderMeshes());
		pApp->WriteProfileInt(
			"view", 
			"lightmapmode",		
			settings.LightmapMode());
		pApp->WriteProfileInt(
			"view", 
			"skinningmode",		
			settings.SkinningMode());
		pApp->WriteProfileInt(
			"view", 
			"boundingboxmode",		
			settings.BoundingBoxMode());
	}

	g_CmdManager->SerializeState( "Vis", "Bizarre Creations", "Vis", true);

	g_CmdManager->ProfileWndRemove( GetSafeHwnd() );

	return CFrameWnd::DestroyWindow();
}

BOOL MainFrame::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle , const RECT& rect , CWnd* pParentWnd , LPCTSTR lpszMenuName , DWORD dwExStyle , CCreateContext* pContext)
{
	return CFrameWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, lpszMenuName, dwExStyle, pContext);
}

extern int giDebugCell;
extern bool gbDebugMode;
extern bool gbDisablePainting;

void MainFrame::OnDebugCalculatevis()
{
	gbDebugMode = true;

	if (!m_pVis)
	{
		m_pVis = new Vis;
		m_pVis->SetGUIMode(true);
		m_pVis->SetLogCallback(OutputLogText);
		m_pVis->SetScene( ((NexusDoc*)GetActiveDocument())->GetNexusSystemPtr()->GetScenePtr() );
	}

	m_pVis->LoadXML( "D:/Subversion/Tools/vis/PVSinput.dat" );

	m_pVis->Process();

	m_pVis->Save( "d:/subversion/tools/vis/output.bin" );
}

int giDisplayCell = 0;

void MainFrame::OnDebugShowcell()
{
	CCellVisDlg dlg;

	dlg.m_iCell = giDisplayCell;

	dlg.DoModal();

	giDisplayCell = dlg.m_iCell;

	gbDisablePainting = true;
	m_pVis->DisplayCell(dlg.m_iCell);

	Nexus::g_pNexusRender->OnUpdateScene();
	GetActiveDocument()->UpdateAllViews(0);
	gbDisablePainting = false;
}

void MainFrame::OnDebugLoaddatabase()
{
	if (!m_pVis)
	{
		m_pVis = new Vis;
		m_pVis->SetGUIMode(true);
		m_pVis->SetLogCallback(OutputLogText);
		m_pVis->SetScene( ((NexusDoc*)GetActiveDocument())->GetNexusSystemPtr()->GetScenePtr() );
	}

	static char strFilter[] = { "PVS Database (*.dat)|*.dat|All Files (*.*)|*.*||" };

	CFileDialog openFile(
		true,
		"DAT",
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		strFilter,
		0);

	if( openFile.DoModal() == IDOK )
	{
		std::string path( openFile.GetPathName().GetBuffer() );
		m_pVis->LoadDatabase(path);
	}
}

void MainFrame::OnDebugShowusage()
{
	gbDisablePainting = true;
	m_pVis->DisplayUsage();

	Nexus::g_pNexusRender->OnUpdateScene();
	GetActiveDocument()->UpdateAllViews(0);
	gbDisablePainting = false;
}
