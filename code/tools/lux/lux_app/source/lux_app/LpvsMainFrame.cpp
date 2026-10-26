// LpvsMainFrm.cpp : implementation of the LpvsMainFrame class
//

#include "stdafx.h"
#include <system/debug/log/debuglog.h>
#include <lux_app/LpvsApp.h>

#include <lux_app/LpvsFileProgressDialog.h>
#include <lux_app/LpvsProjectPreferencesDialog.h>
#include <lux_app/LpvsMainFrame.h>
#include <lux_app/LpvsRenderView.h>
#include "..\..\include\lux_app\lpvsmainframe.h"
#include "lux_app/AppSettings.h"
#include "lux_app/LpvsAppDoc.h"
#include "lux_app/PickerDlg.h"
#include "lux_app/LayerDlg.h"
#include "lux_app/LMapProgDlg.h"
#include <system\debuglog.h>
#include "..\..\Lux\Include\Lux\Parameterization\LMPolyAttributes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


using namespace LPVS;

LpvsMainFrame *gpMainFrm;

void UpdateMainLpvsView()
{
	gpMainFrm->GetActiveDocument()->UpdateAllViews(NULL);
}

// LpvsMainFrame

IMPLEMENT_DYNCREATE(LpvsMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(LpvsMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_COMMAND(ID_LIGHTING_LIGHT, OnLightingLight)
	ON_WM_KEYDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_KEYDOWN()
ON_COMMAND(ID_PROJECT_PROJECTSETTINGS, OnProjectProjectsettings)
ON_UPDATE_COMMAND_UI(ID_VIEW_REALTIMELIGHTING, OnUpdateViewRealtimelighting)
ON_COMMAND(ID_VIEW_REALTIMELIGHTING, OnViewRealtimelighting)

ON_COMMAND_EX(IDR_TOOLBARX, OnBarCheck)
ON_UPDATE_COMMAND_UI(IDR_TOOLBARX, OnUpdateControlBarMenu)

ON_COMMAND_EX(ID_VIEW_TOOLBOX, OnBarCheck)
ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBOX, OnUpdateControlBarMenu)

ON_COMMAND_EX(ID_VIEW_DEBUGOUTPUT, OnBarCheck)
ON_UPDATE_COMMAND_UI(ID_VIEW_DEBUGOUTPUT, OnUpdateControlBarMenu)

ON_COMMAND(ID_VIEW_BOUNCEPATHS, OnViewBouncePaths)
ON_UPDATE_COMMAND_UI(ID_VIEW_BOUNCEPATHS, OnUpdateViewBouncePaths)
ON_COMMAND(ID_LIGHTING_DEBUG, OnLightingDebug)
ON_COMMAND(ID_LIGHTMAPTYPE_FULLCOLOUR, OnLightmaptypeFullcolour)
ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_FULLCOLOUR, OnUpdateLightmaptypeFullcolour)
ON_COMMAND(ID_LIGHTMAPTYPE_2, OnLightmaptype2)
ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_2, OnUpdateLightmaptype2)
ON_COMMAND(ID_LIGHTMAPTYPE_3, OnLightmaptype3)
ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_3, OnUpdateLightmaptype3)
ON_COMMAND(ID_LIGHTMAPTYPE_MULTIPLIED, OnLightmaptypeMultiplied)
ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_MULTIPLIED, OnUpdateLightmaptypeMultiplied)
ON_COMMAND(ID_LIGHTMAPTYPE_DEBUGUNWRAPPING, OnLightmaptypeDebugUnwrapping)
ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_DEBUGUNWRAPPING, OnUpdateLightmaptypeDebugUnwrapping)
ON_COMMAND(ID_LIGHTMAPTYPE_BUMPY, OnLightmaptypeBumpy)
ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_VERTEXSH, OnUpdateLightmaptypeVertexSH)
ON_COMMAND(ID_LIGHTMAPTYPE_VERTEXSH, OnLightmaptypeVertexSH)
ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_BUMPY, OnUpdateLightmaptypeBumpy)
ON_COMMAND(ID_LIGHTING_USENETWORKRENDERING, OnLightingUsenetworkrendering)
ON_UPDATE_COMMAND_UI(ID_LIGHTING_USENETWORKRENDERING, OnUpdateLightingUsenetworkrendering)
ON_COMMAND(ID_VIEW_SHOWSAMPLEPOINTS, OnViewShowsamplepoints)
ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWSAMPLEPOINTS, OnUpdateViewShowsamplepoints)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/// subscriber that intercepts messages for the lpvsapp GUI
class LpvsAppSubscriber : public Debug::SubscriberCommon
{
	virtual void LogString(const Util::C_String &string)
	{
		char buffer[512];

		// detect and remove trailing carriage return
		strcpy(buffer, string.Chars());
		int iLen = (int)strlen(buffer);
		if (buffer[iLen-1] == '\n') buffer[iLen-1] = 0;

		// add to list box and scroll down
		int i = gpMainFrm->m_wndInBarListBox.AddString( buffer );
		gpMainFrm->m_wndInBarListBox.SetCurSel( i );
	}
};




LpvsMainFrame::LpvsMainFrame()
{
	Aftk::C_Global::Create();

	Reset();

	::memset( &m_dataFrameWP, 0, sizeof(WINDOWPLACEMENT) );
	m_dataFrameWP.length = sizeof(WINDOWPLACEMENT);
	m_dataFrameWP.showCmd = SW_HIDE;

	gpMainFrm = this;
}

LpvsMainFrame::~LpvsMainFrame()
{
	Aftk::C_Global::Destroy();
}

void LpvsMainFrame::Reset()
{
	memset(&m_keyboardState, 0, sizeof(m_keyboardState));
	memset(&m_mouseButtonState, 0, sizeof(m_mouseButtonState));

	m_viewportState = LPVS_VIEWPORT_STATE_NO_ACTION;

	m_mouseXOld = 0;
	m_mouseYOld = 0;
}

void LpvsMainFrame::KeyboardEvent(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	bool viewInvalidated = false;

	F32 cameraSpeed = 0.85f;

	switch (nChar)
	{
		case 'W':
		case 'w':
		{
			if (LPVSCORE.GetCameraPtr()->GetType() == SRD::SR_CAMERA_TYPE_TARGET)
			{
				SRD::TargetCamera* pTargetCamera = (SRD::TargetCamera*)LPVSCORE.GetCameraPtr();
				pTargetCamera->MoveInZ(-5);
			}
			viewInvalidated = true;
			break;
		}

		case 'S':
		case 's':
		{
			if (LPVSCORE.GetCameraPtr()->GetType() == SRD::SR_CAMERA_TYPE_TARGET)
			{
				SRD::TargetCamera* pTargetCamera = (SRD::TargetCamera*)LPVSCORE.GetCameraPtr();
				pTargetCamera->MoveInZ(5);
			}
			viewInvalidated = true;
			break;
		}

		case 'A':
		case 'a':
		{
			LPVSCORE.GetCameraPtr()->MoveRight(-5);
			viewInvalidated = true;
			break;
		}

		case 'D':
		case 'd':
		{
			LPVSCORE.GetCameraPtr()->MoveRight(5);
			viewInvalidated = true;
			break;
		}

		case 'Q':
		case 'q':
		{
			//LPVSCORE.GetCameraPtr()->MoveUp(cameraSpeed);
			LPVSCORE.GetCameraPtr()->RotatePitch(-0.5f);
			viewInvalidated = true;
			break;
		}

		case 'Z':
		case 'z':
		{
			//LPVSCORE.GetCameraPtr()->MoveUp(-cameraSpeed);
			LPVSCORE.GetCameraPtr()->RotatePitch(0.5f);
			viewInvalidated = true;
			break;
		}

		case 'C':
		case 'c':
		{
			LPVSCORE.CentreCamera();
			viewInvalidated = true;
			break;
		}

		case 'F':
		case 'f':
		{
			LPVSCORE.FrameAll();
			viewInvalidated = true;
			break;
		}
	}

	if (viewInvalidated)
	{
		this->GetActiveDocument()->UpdateAllViews(NULL);
	}
}

void LpvsMainFrame::MouseMoveEvent(UINT nFlags, CPoint point)
{
	bool viewInvalidated = false;
	{
		S32 mouseX = point.x - m_mouseXOld;
		S32 mouseY = point.y - m_mouseYOld;

		m_mouseXOld = point.x;
		m_mouseYOld = point.y;

		switch (m_viewportState)
		{
			case LPVS_VIEWPORT_STATE_VIEW_ROTATE:
			{
				SetCursor(LoadCursor(NULL, (LPCSTR)32649));
				F32 mouseXF = F32(mouseX);
				mouseXF *= 0.01f;
				F32 mouseYF = F32(mouseY);
				mouseYF *= 0.01f;

				LPVSCORE.GetCameraPtr()->RotatePitch(mouseYF);
				LPVSCORE.GetCameraPtr()->RotateYaw(mouseXF);
				viewInvalidated = true;
				break;
			}

			case LPVS_VIEWPORT_STATE_VIEW_DOLLY:
			{
				if (LPVSCORE.GetCameraPtr()->GetType() == SRD::SR_CAMERA_TYPE_TARGET)
				{
					SRD::TargetCamera* pTargetCamera = (SRD::TargetCamera*)LPVSCORE.GetCameraPtr();
					SetCursor(LoadCursor(NULL, IDC_SIZENS));

					F32 mouseYF = F32(mouseY);
					mouseYF *= 0.5f;
					
					F32 distancePx = -mouseYF;

					const SRD::D3dWindow* pD3dWindow = LPVSCORE.GetD3dWindowPtr();

					F32 widthWorld = (F32)pD3dWindow->GetWidth();
					widthWorld *= pTargetCamera->GetZDistance();

					F32 halfWidthPx = (F32)pD3dWindow->GetWidth()*0.5f;
					F32 cz = halfWidthPx / tan(pTargetCamera->GetFov());

					F32 distanceWorld = distancePx / cz;
					distanceWorld *= pTargetCamera->GetZDistance()*4;

					pTargetCamera->MoveInZ(distanceWorld);
					viewInvalidated = true;
				}
				break;
			}

			case LPVS_VIEWPORT_STATE_VIEW_TRANSLATE:
			{
				if (LPVSCORE.GetCameraPtr()->GetType() == SRD::SR_CAMERA_TYPE_TARGET)
				{
					SRD::TargetCamera* pTargetCamera = (SRD::TargetCamera*)LPVSCORE.GetCameraPtr();
	
					SetCursor(LoadCursor(NULL, IDC_SIZEALL));
					F32 mouseXF = F32(mouseX);
					mouseXF *= 0.5f;
					F32 mouseYF = F32(mouseY);
					mouseYF *= 0.5f;
					
					const SRD::D3dWindow* pD3dWindow = LPVSCORE.GetD3dWindowPtr();

					F32 dxWorld = mouseXF / (F32)pD3dWindow->GetWidth();
					dxWorld *= pTargetCamera->GetZDistance();

					F32 dyWorld = mouseYF / (F32)pD3dWindow->GetHeight();
					dyWorld *= pTargetCamera->GetZDistance();

					pTargetCamera->MoveRight(-dxWorld);
					pTargetCamera->MoveUp(dyWorld);
					viewInvalidated = true;
				}
				break;
			}

			default:
			{
			}
		}
	}

	if (viewInvalidated)
	{
		this->GetActiveDocument()->UpdateAllViews(NULL);
	}
}

void LpvsMainFrame::MouseButtonEvent(int buttonId, LpvsGuiMouseButtonActionEnum action, UINT nFlags, CPoint point)
{
	switch (action)
	{
	case LPVS_MOUSE_BUTTON_ACTION_DOWN:
		m_mouseButtonState[buttonId] = true;
		OnMouseButtonStateChanged(buttonId);
		break;
	case LPVS_MOUSE_BUTTON_ACTION_UP:
		m_mouseButtonState[buttonId] = false;
		OnMouseButtonStateChanged(buttonId);
		break;
	case LPVS_MOUSE_BUTTON_ACTION_DOUBLE_CLICK:
		break;
	default:
		break;
	};
}
static SD::RayTraceResult result;

// here when we clicked on the viewport - calculate the new selection
void LpvsMainFrame::Pick()
{
	SRD::Camera *pCamera = LPVSCORE.GetCameraPtr();
	SD::SpatialHierarchy *pTree = LPVSCORE.GetSpatialHierarchy();

	if (!pCamera || !pTree) return;

    // Get the pick ray from the mouse position
	CView *pView = GetActiveView();
	if (!pView) return;
	RECT rect;
	pView->GetWindowRect(&rect);
	int iWidth = (rect.right-rect.left);
	int iHeight = (rect.bottom-rect.top);

	D3DXMATRIX m;
	FLOAT	   fAspect = (FLOAT)iWidth / (FLOAT)iHeight;

    POINT ptCursor;
    GetCursorPos( &ptCursor );
	pView->ScreenToClient( &ptCursor );

    // Compute the vector of the pick ray in screen space
	D3DXMATRIX matProj, matView, matWorld;
	pCamera->GetProjectionMatrix(matProj);
	pCamera->GetViewMatrix(matView);
	pCamera->GetWorldMatrix(matWorld);

    D3DXVECTOR3 v;
	v.x =  ( ( ( 2.0f * ptCursor.x ) / iWidth  ) - 1 ) / matProj._11;
    v.y = -( ( ( 2.0f * ptCursor.y ) / iHeight ) - 1 ) / matProj._22;
    v.z =  1.0f;

    // Get the inverse of the composite view and world matrix
	m = /*matWorld * */ matView;
    D3DXMatrixInverse( &m, NULL, &m );

    // Transform the screen space pick ray into 3D space
	float3 vOrigin = float3( m._41, m._42, m._43);
	float3 vDirection =	float3( v.x*m._11 + v.y*m._21 + v.z*m._31,
								v.x*m._12 + v.y*m._22 + v.z*m._32,
								v.x*m._13 + v.y*m._23 + v.z*m._33);

	SD::Ray pickRay(vOrigin,vDirection,true);

	// 
	pTree->RayTrace(pickRay,result);

	LPVSCORE.SetSelection(&result);

 	GetActiveDocument()->UpdateAllViews(NULL);

	if (result.bHit)
	{
		LMPolyAttributes *pAttr = ((LMPolyAttributes*)result.pMeshNode->GetPolygon(result.iPolygon).GetAttachDataPtr());

		// update picker dialog
		m_pPickerDlg->m_iPolygon = 	result.iPolygon;
		if (pAttr)
			m_pPickerDlg->m_iChart	 =  pAttr->iChart;
		if (result.pMeshNode)
			m_pPickerDlg->m_csNodeName = result.pMeshNode->GetName().c_str();
		m_pPickerDlg->UpdateData(false);
	}
}

void LpvsMainFrame::OnMouseButtonStateChanged(int buttonId)
{
	if ((buttonId == LPVS_MOUSE_BUTTON_LEFT)
		&&
		(m_mouseButtonState[buttonId]))
	{
		if (GetKeyState(VK_MENU)&0x8000)
		{
			m_viewportState = LPVS_VIEWPORT_STATE_VIEW_ROTATE;
		}
		else
		{
			Pick();
		}
	}
	else if ((buttonId == LPVS_MOUSE_BUTTON_MIDDLE)
		&&
		(m_mouseButtonState[buttonId]))
	{
		if (GetKeyState(VK_MENU)&0x8000)
		{
			if (m_viewportState != LPVS_VIEWPORT_STATE_VIEW_TRANSLATE)
			{
				m_viewportState = LPVS_VIEWPORT_STATE_VIEW_TRANSLATE;
			}
		}
	}
	else if ((buttonId == LPVS_MOUSE_BUTTON_RIGHT)
		&&
		(m_mouseButtonState[buttonId]))
	{
		if (GetKeyState(VK_MENU)&0x8000)
		{
			if (m_viewportState != LPVS_VIEWPORT_STATE_VIEW_DOLLY)
			{
				m_viewportState = LPVS_VIEWPORT_STATE_VIEW_DOLLY;
			}
		}
	}
	else
	{
		if (m_viewportState != LPVS_VIEWPORT_STATE_NO_ACTION)
		{
			m_viewportState = LPVS_VIEWPORT_STATE_NO_ACTION;
		}
	}
}

// associate an icon with a menu item
void LpvsMainFrame::SetCmdIcon( UINT nCmd, UINT nIconID ) 
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

BOOL LpvsMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// if we are pressing ALT and no other key then ignore the message
	if ((pMsg->message == WM_SYSKEYDOWN ||
		pMsg->message == WM_SYSKEYUP)
		&& pMsg->wParam == 18)
	{
		// pressing ALT on it's own is ignored - it's used for view manipulation
	}
	else
	{
		if( m_wndMenuBar.TranslateMainFrameMessage(pMsg) )
			return TRUE;
	}

	return CFrameWnd::PreTranslateMessage(pMsg);
}

// sliding pane toolbar
void LpvsMainFrame::CreateSlidingPaneToolbar()
{
	// create the rollup toolbar
	m_wndResizableBar3.Create(
			_T("Toolbox"),
			this,
			ID_VIEW_TOOLBOX,
				WS_CHILD|WS_VISIBLE
					|CBRS_RIGHT|CBRS_GRIPPER|CBRS_TOOLTIPS
					|CBRS_FLYBY|CBRS_SIZE_DYNAMIC
					|CBRS_HIDE_INPLACE
			);

	LPCTSTR psz = AfxRegisterWndClass(
		CS_VREDRAW,
		(HCURSOR)::LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_BTNSHADOW),
		NULL);
	
	m_wndInBarDlg.Create(
		psz, "Hi", WS_CHILD | WS_VISIBLE, CRect(0, 0, 20, 20), &m_wndResizableBar3, 0
			);

	m_pLayerDlg = new CLayerDlg;
	m_wndInBarDlg.InsertPage("Layer Select", m_pLayerDlg, IDD_LAYERS);

	m_pPickerDlg = new CPickerDlg;
	m_wndInBarDlg.InsertPage("Polygon Info", m_pPickerDlg, IDD_TESTDIALOG);
}

// main initialization function
int LpvsMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    g_CmdManager->ProfileSetup( "Lux", GetSafeHwnd());

	// create fancy menus
	m_wndMenuBar.Create( "Menu", this, IDR_MAINFRAME );

	if(!m_wndToolBar.Create(_T("Toolbar"), this, IDR_TOOLBARX)
		||!m_wndToolBar.LoadToolBar(IDR_TOOLBARX))
	{
		TRACE0("Failed to create true-color toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	CreateListToolbar();

	CreateSlidingPaneToolbar();

	// Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.FrameEnableDocking(this);
	m_wndResizableBar1.EnableDocking(CBRS_ALIGN_ANY);
	m_wndResizableBar1.FrameEnableDocking(this);
	m_wndResizableBar1.DockControlBar( AFX_IDW_DOCKBAR_BOTTOM, 1, this, false );
	DockControlBar(&m_wndToolBar);

	m_wndResizableBar3.SetInitDesiredSizeVertical(CSize(200));
	m_wndResizableBar3.EnableDocking(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT);
	m_wndResizableBar3.FrameEnableDocking(this);
	m_wndResizableBar3.DockControlBar( AFX_IDW_DOCKBAR_RIGHT, 1, this, false );

	//Perform custom initialisations
	LPVS::LPVSCORE.SetCameraType(SRD::SR_CAMERA_TYPE_TARGET);

	// load recent file list etc.
	g_CmdManager->SerializeState( "Lux", "Bizarre Creations", "Lightmapper", false);

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

	// this disables the hiding of rarely used commands
	CExtPopupColorMenuWnd::g_bMenuExpanding = false;
	CExtPopupColorMenuWnd::g_bMenuHighlightRarely = false;

	// set our icons
	SetCmdIcon( ID_FILE_NEW,		IDI_NEW );
	SetCmdIcon( ID_FILE_OPEN,		IDI_OPEN);
	SetCmdIcon( ID_FILE_SAVE,		IDI_SAVE);
	SetCmdIcon( ID_LIGHTING_LIGHT,	IDI_RUN);

	// read view settings from registry
	m_appSettings.m_RendererSettings.iLightmapType =
		(SRD::SceneRendererSettings::LightMapType) theApp.GetProfileInt("viewsettings", "lightmaptype", SRD::SceneRendererSettings::LMType_FullColour);
	m_appSettings.m_RendererSettings.bRealtimeLighting = 
		(theApp.GetProfileInt("Viewsettings","realtimelighting",1) != 0);
	m_appSettings.m_RendererSettings.bViewLightPaths = (bool)
		(theApp.GetProfileInt("viewsettings","viewlightpaths",0) != 0);
	m_appSettings.m_RendererSettings.bViewSamplePoints = (bool)
		(theApp.GetProfileInt("viewsettings","showSH",0) != 0);

	Debug::AutoConfigureDebugLog();

	/// add our custom subscriber to the rootlogger
	Debug::GetRootLogger().AddSubscriber( new LpvsAppSubscriber() );

	return 0;
}

void LpvsMainFrame::OnClose()
{
	CFrameWnd::OnClose();
}

BOOL LpvsMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	return __super::OnCreateClient(lpcs,pContext);
}

BOOL LpvsMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}


// LpvsMainFrame diagnostics

#ifdef _DEBUG
void LpvsMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void LpvsMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// LpvsMainFrame message handlers

void LpvsMainFrame::OnLightingLight()
{
	CLMapProgDlg lmapdlg;

	lmapdlg.DoModal();

	if (!lmapdlg.m_bCancel)
	{
		lmapdlg.WaitForThreadToFinish();

		LPVS::LPVSCORE.UpdateVertexBuffers();

		this->GetActiveDocument()->UpdateAllViews(NULL);

		// Ken, now writes scene after lighting
		if (!LPVS::LPVSCORE.WriteScene(0))
		{
			//Todo, print error here.
			return;
		}
	}
}

BOOL LpvsMainFrame::DestroyWindow()
{
	LPVSCORE.Destroy();

	CWinApp * pApp = ::AfxGetApp();

    CExtControlBar::ProfileBarStateSave(
        this,
        pApp->m_pszRegistryKey,
        pApp->m_pszProfileName,
        pApp->m_pszProfileName
        );

	// save view settings to registry
	pApp->WriteProfileInt("viewsettings", "lightmaptype",		m_appSettings.m_RendererSettings.iLightmapType);
	pApp->WriteProfileInt("Viewsettings","realtimelighting",	m_appSettings.m_RendererSettings.bRealtimeLighting);
	pApp->WriteProfileInt("viewsettings","viewlightpaths",		m_appSettings.m_RendererSettings.bViewLightPaths);
	pApp->WriteProfileInt("viewsettings","showSH",				m_appSettings.m_RendererSettings.bViewSamplePoints);

	g_CmdManager->SerializeState( "Lux", "Bizarre Creations", "Lightmapper", true);

	g_CmdManager->ProfileWndRemove( GetSafeHwnd() );

	return CFrameWnd::DestroyWindow();
}

void LpvsMainFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	KeyboardEvent(nChar, nRepCnt, nFlags);

	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void LpvsMainFrame::CreateListToolbar()
{
	if(	!m_wndResizableBar1.Create(
			_T("Debug Output"),
			this,
			ID_VIEW_DEBUGOUTPUT
			)
		)
	{
		return;
	}
	if( !m_wndInBarListBox.Create(
			WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL
				|LBS_HASSTRINGS|LBS_NOINTEGRALHEIGHT				
				,
			CRect(0,0,0,0),
			&m_wndResizableBar1,
			m_wndResizableBar1.GetDlgCtrlID()
			)
		)
	{
		return;
	}
	m_wndInBarListBox.SetFont(
		CFont::FromHandle(
			(HFONT)::GetStockObject(DEFAULT_GUI_FONT)
			)
		);
}

void LpvsMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// Set caption of main frame window
	SetWindowText("Lighting and PVS Application (LPVS)");
}

void LpvsMainFrame::OnProjectProjectsettings()
{
	LpvsProjectPreferencesDialog prefs;
	prefs.DoModal();
	this->GetActiveDocument()->UpdateAllViews(NULL);
}

// check the menu option for realtime lighting on/off
void LpvsMainFrame::OnUpdateViewRealtimelighting(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( m_appSettings.m_RendererSettings.bRealtimeLighting );
}

// turn realtime lighting on/off
void LpvsMainFrame::OnViewRealtimelighting()
{
	SRD::SceneRendererSettings *pSettings = &m_appSettings.m_RendererSettings;

	pSettings->bRealtimeLighting = !pSettings->bRealtimeLighting;

	if (LPVSCORE.GetSceneRenderer())
		LPVSCORE.GetSceneRenderer()->SetSettings(*pSettings);

	this->GetActiveDocument()->UpdateAllViews(NULL);
}

// turn viewing of lighting paths on and off
extern int gnCurLines;
void LpvsMainFrame::OnViewBouncePaths()
{
	SRD::SceneRendererSettings *pSettings = &m_appSettings.m_RendererSettings;

	pSettings->bViewLightPaths = !pSettings->bViewLightPaths;
	gnCurLines = 0;

	if (LPVSCORE.GetSceneRenderer())
		LPVSCORE.GetSceneRenderer()->SetSettings(*pSettings);

	this->GetActiveDocument()->UpdateAllViews(NULL);
}
// check the menu option for showing lighting paths on and off
void LpvsMainFrame::OnUpdateViewBouncePaths(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( m_appSettings.m_RendererSettings.bViewLightPaths );
}

void DebugCallback(SD::RayTraceResult &result);
void LpvsMainFrame::OnLightingDebug()
{
	DebugCallback(result);
}

void LpvsMainFrame::SetLightmapMode(int type)
{
	SRD::SceneRendererSettings *pSettings = &m_appSettings.m_RendererSettings;
	pSettings->iLightmapType = (SRD::SceneRendererSettings::LightMapType)type;
	if (LPVSCORE.GetSceneRenderer())
		LPVSCORE.GetSceneRenderer()->SetSettings(*pSettings);

	this->GetActiveDocument()->UpdateAllViews(NULL);
}
int LpvsMainFrame::GetLightmapMode()
{
	SRD::SceneRendererSettings *pSettings = &m_appSettings.m_RendererSettings;
	return pSettings->iLightmapType;
}

void LpvsMainFrame::OnLightmaptypeFullcolour()
{
	SetLightmapMode(SRD::SceneRendererSettings::LMType_FullColour);
}

void LpvsMainFrame::OnUpdateLightmaptypeFullcolour(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( GetLightmapMode() == SRD::SceneRendererSettings::LMType_FullColour );
}

void LpvsMainFrame::OnLightmaptype2()
{
	SetLightmapMode(SRD::SceneRendererSettings::LMType_DXT1);
}

void LpvsMainFrame::OnUpdateLightmaptype2(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( GetLightmapMode() == SRD::SceneRendererSettings::LMType_DXT1 );
}

void LpvsMainFrame::OnLightmaptype3()
{
	SetLightmapMode(SRD::SceneRendererSettings::LMType_A8L8);
}

void LpvsMainFrame::OnUpdateLightmaptype3(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( GetLightmapMode() == SRD::SceneRendererSettings::LMType_A8L8 );
}

void LpvsMainFrame::OnLightmaptypeMultiplied()
{
	SetLightmapMode(SRD::SceneRendererSettings::LMType_Multiplied);
}

void LpvsMainFrame::OnUpdateLightmaptypeMultiplied(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( GetLightmapMode() == SRD::SceneRendererSettings::LMType_Multiplied );
}

void LpvsMainFrame::OnLightmaptypeDebugUnwrapping()
{
	SetLightmapMode(SRD::SceneRendererSettings::LMType_DebugUnwrapping);
}

void LpvsMainFrame::OnUpdateLightmaptypeDebugUnwrapping(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( GetLightmapMode() == SRD::SceneRendererSettings::LMType_DebugUnwrapping );
}

void LpvsMainFrame::OnLightmaptypeBumpy()
{
	SetLightmapMode(SRD::SceneRendererSettings::LMType_Bumpy);
}

void LpvsMainFrame::OnUpdateLightmaptypeVertexSH(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( GetLightmapMode() == SRD::SceneRendererSettings::LMType_VertexSH );
}

void LpvsMainFrame::OnLightmaptypeVertexSH()
{
	SetLightmapMode(SRD::SceneRendererSettings::LMType_VertexSH);
}

void LpvsMainFrame::OnUpdateLightmaptypeBumpy(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( GetLightmapMode() == SRD::SceneRendererSettings::LMType_Bumpy );
}

extern bool g_useNetworkClients;
void LpvsMainFrame::OnLightingUsenetworkrendering()
{
	g_useNetworkClients = !g_useNetworkClients;
}

void LpvsMainFrame::OnUpdateLightingUsenetworkrendering(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( g_useNetworkClients  );
}

void LpvsMainFrame::OnViewShowsamplepoints()
{
	SRD::SceneRendererSettings *pSettings = &m_appSettings.m_RendererSettings;

	pSettings->bViewSamplePoints = !pSettings->bViewSamplePoints;
	gnCurLines = 0;

	if (LPVSCORE.GetSceneRenderer())
		LPVSCORE.GetSceneRenderer()->SetSettings(*pSettings);

	this->GetActiveDocument()->UpdateAllViews(NULL);
}

void LpvsMainFrame::OnUpdateViewShowsamplepoints(CCmdUI *pCmdUI)
{
	SRD::SceneRendererSettings *pSettings = &m_appSettings.m_RendererSettings;

	pCmdUI->SetCheck( pSettings->bViewSamplePoints );
}

