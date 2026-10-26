// NexusView.cpp : implementation of the NexusView class
//

#include <nexus_app/stdafx.h>
#include <nexus_app/Nexus.h>
#include <nexus_app/NexusSystem.h>
#include <nexus_app/DisplayManager.h>

#include <nexus_app/NexusDoc.h>

#include "../../resource.h"
#include <nexus_app/NexusProgressDialog.h>
#include <mfc_extras/BaseThread.h>

#include <nexus_app/ViewportDisplay.h>
#include <nexus_app/Nexus.h>
#include <nexus_app/MainFrm.h>
#include <nexus_app/CalculateTangentsAndBinormalsDialog.h>
#include <nexus_app/MeshWeldingOptions.h>
#include <nexus_app/DockingPanelRight.h>
#include <nexus_app/SceneGraphTree.h>
#include <nexus_app/ViewportParent.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Nexus;

// NexusView
#include "../../resource.h"

IMPLEMENT_DYNCREATE(C_Viewport, CWnd)

BEGIN_MESSAGE_MAP(C_Viewport, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// NexusView construction/destruction

C_Viewport::C_Viewport()
{
	m_mouseXOld = 0;
	m_mouseYOld = 0;

	m_initialPaint = true;
}

int C_Viewport::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

C_Viewport::~C_Viewport()
{
}

BOOL C_Viewport::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CWnd::PreCreateWindow(cs);
}

// NexusView drawing

void C_Viewport::OnDraw(CDC* /*pDC*/)
{
	ASSERT_VALID(g_pDocument);
	if (!g_pDocument)
		return;

	if (m_display.IsCreated())
	{
		if (!m_display.Draw())
			g_pMainFrame->OnCriticalError(Tu::Status(Tu::Status::FAILURE, "Failure during render update."));
	}
}

#ifdef _DEBUG
void C_Viewport::AssertValid() const
{
	CWnd::AssertValid();
}

void C_Viewport::Dump(CDumpContext& dc) const
{
	CWnd::Dump(dc);
}

#endif //_DEBUG

void C_Viewport::MouseMoveEvent(UINT nFlags, CPoint point)
{
	bool viewInvalidated = false;
	S32 mouseX = point.x - m_mouseXOld;
	S32 mouseY = point.y - m_mouseYOld;

	m_mouseXOld = point.x;
	m_mouseYOld = point.y;

	switch (g_pNexusSystem->GetViewportState())
	{
		case E_ViewportState_ViewRotate:
		{
			SetCursor(LoadCursor(NULL, (LPCSTR)32649));
			F32 mouseXF = F32(mouseX);
			mouseXF *= 0.01f;
			F32 mouseYF = F32(mouseY);
			mouseYF *= 0.01f;

			m_display.GetCameraPtr()->RotatePitch(mouseYF);
			m_display.GetCameraPtr()->RotateYaw(mouseXF);
			viewInvalidated = true;
			break;
		}

		case E_ViewportState_ViewDolly:
		{
			if (m_display.GetCameraPtr()->GetType() == NexusRender::Camera::CAMERA_TYPE_TARGET)
			{
				NexusRender::TargetCamera* pTargetCamera = (NexusRender::TargetCamera*)m_display.GetCameraPtr();
				SetCursor(LoadCursor(NULL, IDC_SIZENS));

				F32 mouseXF = F32(mouseX);
				mouseXF *= 0.5f;
				F32 mouseYF = F32(mouseY);
				mouseYF *= 0.5f;
				
				F32 distancePx = -mouseXF + -mouseYF;
				ZoomCamera(distancePx);
				viewInvalidated = true;
			}
			break;
		}

		case E_ViewportState_ViewTranslate:
		{
			if (m_display.GetCameraPtr()->GetType() == NexusRender::Camera::CAMERA_TYPE_TARGET)
			{
				NexusRender::TargetCamera* pTargetCamera = 
					(NexusRender::TargetCamera*)m_display.GetCameraPtr();

				SetCursor(LoadCursor(NULL, IDC_SIZEALL));
				F32 mouseXF = F32(mouseX);
				mouseXF *= 0.5f;
				F32 mouseYF = F32(mouseY);
				mouseYF *= 0.5f;
				
				const NexusRender::C_D3DWindow* pD3DWindow = 
					g_pViewportParent->GetDisplayManagerPtr()->GetD3DWindowPtr();

				F32 dxWorld = mouseXF / (F32)pD3DWindow->GetWidth();
				dxWorld *= pTargetCamera->GetZDistance();

				F32 dyWorld = mouseYF / (F32)pD3DWindow->GetHeight();
				dyWorld *= pTargetCamera->GetZDistance();

				pTargetCamera->MoveRight(-dxWorld);
				pTargetCamera->MoveUp(dyWorld);
				viewInvalidated = true;
			}
			break;
		}

		default:
			break;
	}

	if (viewInvalidated)
		g_pDocument->UpdateAllViews(NULL);

}

void C_Viewport::MouseButtonEvent(
	int buttonId, 
	E_MouseButtonAction action,
	UINT nFlags,
	CPoint point)
{
	g_pNexusSystem->MouseButtonEvent(
		0,
		buttonId,
		action,
		nFlags,
		point);
}

void C_Viewport::MouseButtonStateChanged(int buttonId)
{
	g_pNexusSystem->MouseButtonStateChanged(
		0,
		buttonId);
}

void C_Viewport::OnFinalRelease()
{
	// TODO: Add your specialized code here and/or call the base class
	CWnd::OnFinalRelease();
}

void C_Viewport::OnPaint()
{
	CWnd::OnPaint();

	if (!m_display.IsCreated())
		return;

	RECT r;
	GetClientRect(&r);
	if (m_initialPaint == true)
	{
		//Painting is permitted.
		if ((r.right - r.left != 0) && (r.bottom - r.top != 0))
		{
			m_display.Resize(r.right - r.left, r.bottom - r.top);
			m_initialPaint = false;
			m_display.Draw();
			return;
		}
	}

	if ((r.right - r.left != 0) && (r.bottom - r.top != 0))
		m_display.Draw();
}

void C_Viewport::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	m_display.Resize(cx, cy);
}

void C_Viewport::OnMouseMove(UINT nFlags, CPoint point)
{
	MouseMoveEvent(nFlags, point);

	CWnd::OnMouseMove(nFlags, point);
}

void C_Viewport::ZoomCamera(F32 distancePx)
{
	NexusRender::TargetCamera* pTargetCamera = (NexusRender::TargetCamera*)m_display.GetCameraPtr();
	const NexusRender::C_D3DWindow* pD3DWindow = 
		g_pViewportParent->GetDisplayManagerPtr()->GetD3DWindowPtr();
	F32 widthWorld = (F32)pD3DWindow->GetWidth();
	widthWorld *= pTargetCamera->GetZDistance();

	F32 halfWidthPx = (F32)pD3DWindow->GetWidth()*0.5f;
	F32 cz = halfWidthPx / tan(pTargetCamera->GetFov());

	F32 movement = pTargetCamera->GetZDistance()*4;
	if (movement < 20)
		movement = 20;
	F32 distanceWorld = distancePx / cz;
	distanceWorld *= movement;
	pTargetCamera->MoveInZ(distanceWorld);
	m_display.OnCameraZoom();
}

BOOL C_Viewport::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (m_display.GetCameraPtr()->GetType() == NexusRender::Camera::CAMERA_TYPE_TARGET)
	{
		NexusRender::TargetCamera* pTargetCamera = (NexusRender::TargetCamera*)m_display.GetCameraPtr();
		F32 distancePx = (F32)-zDelta;
		distancePx *= 0.25f;
		ZoomCamera(distancePx);
	}
	g_pDocument->UpdateAllViews(0);
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void C_Viewport::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CWnd::OnLButtonDblClk(nFlags, point);
}

void C_Viewport::OnLButtonDown(UINT nFlags, CPoint point)
{
	MouseButtonEvent(
		E_MouseButton_Left, 
		E_MouseButtonAction_Down, 
		nFlags, 
		point);

	CWnd::OnLButtonDown(nFlags, point);
}

void C_Viewport::OnLButtonUp(UINT nFlags, CPoint point)
{
	MouseButtonEvent(
		E_MouseButton_Left, 
		E_MouseButtonAction_Up, 
		nFlags, 
		point);

	CWnd::OnLButtonUp(nFlags, point);
}

void C_Viewport::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	CWnd::OnMButtonDblClk(nFlags, point);
}

void C_Viewport::OnMButtonDown(UINT nFlags, CPoint point)
{
	MouseButtonEvent(
		E_MouseButton_Middle, 
		E_MouseButtonAction_Down, 
		nFlags, 
		point);

	CWnd::OnMButtonDown(nFlags, point);
}

void C_Viewport::OnMButtonUp(UINT nFlags, CPoint point)
{
	MouseButtonEvent(
		E_MouseButton_Middle, 
		E_MouseButtonAction_Up, 
		nFlags, 
		point);

	CWnd::OnMButtonUp(nFlags, point);
}

void C_Viewport::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	CWnd::OnRButtonDblClk(nFlags, point);
}

void C_Viewport::OnRButtonDown(UINT nFlags, CPoint point)
{
	MouseButtonEvent(
		E_MouseButton_Right, 
		E_MouseButtonAction_Down,
		nFlags,
		point);

	CWnd::OnRButtonDown(nFlags, point);
}

void C_Viewport::OnRButtonUp(UINT nFlags, CPoint point)
{
	MouseButtonEvent(
		E_MouseButton_Right,
		E_MouseButtonAction_Up, 
		nFlags, 
		point);

	CWnd::OnRButtonUp(nFlags, point);
}

BOOL C_Viewport::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void C_Viewport::Pick()
{
	NexusRender::Camera* pCamera = m_display.GetCameraPtr();
	if (!pCamera)
		return;

    // Get the pick ray from the mouse position
	RECT r;
	GetClientRect(&r);
	int iWidth = (r.right-r.left);
	int iHeight = (r.bottom-r.top);

	D3DXMATRIX m;
	FLOAT fAspect = (FLOAT)iWidth / (FLOAT)iHeight;

    POINT ptCursor;
    GetCursorPos( &ptCursor );
	ScreenToClient( &ptCursor );

    // Compute the vector of the pick ray in screen space
	D3DXMATRIX matProj, matView;
	pCamera->GetProjectionMatrix(matProj);
	pCamera->GetViewMatrix(matView);

    D3DXVECTOR3 v;
	v.x =  ( ( ( 2.0f * ptCursor.x ) / iWidth  ) - 1 ) / matProj._11;
    v.y = -( ( ( 2.0f * ptCursor.y ) / iHeight ) - 1 ) / matProj._22;
    v.z =  1.0f;

    // Get the inverse of the composite view and world matrix
	m = matView;
    D3DXMatrixInverse( &m, NULL, &m );

    // Transform the screen space pick ray into 3D space
	Aftk::Position3 vOrigin ( m._41, m._42, m._43);
	Aftk::Normal3 vDirection;
	vDirection[0] = v.x*m._11 + v.y*m._21 + v.z*m._31;
	vDirection[1] = v.x*m._12 + v.y*m._22 + v.z*m._32;
	vDirection[2] = v.x*m._13 + v.y*m._23 + v.z*m._33;
	UtMaths::SafeNormalise(&(Aftk::Vector3F32&)vDirection);

	Aftk::C_Ray pickRay(
		vOrigin,
		vDirection,
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().backFaceCulling);

	Nexus::C_PickResult result;
	((NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr())->Pick(
		result,
		(NexusRender::E_SelectionMode)Nexus::g_pNexusSystem->GetState().selectionMode, 
		pickRay);
	
	if (g_pNexusSystem->GetSelectionState() == Nexus::E_SelectionState_Single)
	{
		g_pNexusSystem->GetCorePtr()->DeselectAll();
		SceneGraphTree* pTree = g_pMainFrame->GetDockingPanelRightPtr()->GetSceneGraphTreePtr();
		pTree->ClearSelection();
	}
	if (result.pSceneNode)
	{
		SceneGraphTree* pTree = g_pMainFrame->GetDockingPanelRightPtr()->GetSceneGraphTreePtr();

		//Toggle selection of our object in the scene graph if we're in object selection mode.
		if (Nexus::g_pNexusSystem->GetState().selectionMode == Nexus::E_SelectionMode_Object)
			pTree->ToggleSelection(result.pSceneNode);

		C_SelectedNodeInfo info;
		info.pSceneNode = result.pSceneNode;
		if (result.selectedComponent != -1)
			info.selectedComponents.Insert(
				C_ComponentSelection(
					result.lodLevel,
					result.selectedComponent));

		Nexus::g_pNexusSystem->GetCorePtr()->ToggleSelected(
			Nexus::g_pNexusSystem->GetState().selectionMode, 
			info);
		
	}
	Nexus::g_pNexusSystem->UpdateSceneNodeSelection();
	g_pDocument->UpdateAllViews(0);
}

C_ViewportDisplay* C_Viewport::GetDisplayPtr()
{
	return &m_display;
}

