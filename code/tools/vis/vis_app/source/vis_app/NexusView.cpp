// NexusView.cpp : implementation of the NexusView class
//

#include <vis_app/stdafx.h>
#include <vis_app/Nexus.h>
#include <vis_app/NexusSystem.h>
#include <vis_app/NexusRender.h>

#include <vis_app/NexusDoc.h>

#include "../../resource.h"
#include <vis_app/NexusProgressDialog.h>
#include <mfc_extras/BaseThread.h>

#include "..\..\include\vis_app\nexusview.h"
#include <vis_app/Nexus.h>
#include <vis_app/MainFrm.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Nexus;

NexusView* g_pView = 0;

// NexusView
#include "../../resource.h"

IMPLEMENT_DYNCREATE(NexusView, CView)

BEGIN_MESSAGE_MAP(NexusView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
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
	ON_UPDATE_COMMAND_UI(ID_SHADING_SMOOTHSHADEALL, OnUpdateShadingSmoothshadeall)
	ON_UPDATE_COMMAND_UI(ID_SHADING_WIREFRAMEALL, OnUpdateShadingWireframeall)
	ON_UPDATE_COMMAND_UI(ID_SHADING_FLATSHADEALL, OnUpdateShadingFlatshadeall)
	ON_COMMAND(ID_SHADING_POINTSALL, OnShadingPointsall)
	ON_UPDATE_COMMAND_UI(ID_SHADING_BACKFACECULLING, OnUpdateShadingBackfaceculling)
	ON_COMMAND(ID_SHADING_TEXTURING, OnShadingTexturing)
	ON_UPDATE_COMMAND_UI(ID_SHADING_TEXTURING, OnUpdateShadingTexturing)
	ON_COMMAND(ID_SHADING_WIREFRAMEALL, OnShadingWireframeall)
	ON_COMMAND(ID_SHADING_SMOOTHSHADEALL, OnShadingSmoothshadeall)
	ON_COMMAND(ID_SHADING_FLATSHADEALL, OnShadingFlatshadeall)
	ON_UPDATE_COMMAND_UI(ID_SHADING_POINTSALL, OnUpdateShadingPointsall)
	ON_COMMAND(ID_SHADING_BACKFACECULLING, OnShadingBackfaceculling)
	ON_UPDATE_COMMAND_UI(ID_RENDERING_VERTEXNORMALS, OnUpdateRenderingVertexnormals)
	ON_COMMAND(ID_RENDERING_VERTEXNORMALS, OnRenderingVertexnormals)
	ON_MESSAGE(UPDATE_RENDER_TIME, UpdateTime)
	ON_COMMAND(ID_RENDERING_AXES, OnRenderingAxes)
	ON_UPDATE_COMMAND_UI(ID_RENDERING_AXES, OnUpdateRenderingAxes)
	ON_COMMAND(ID_LIGHTING_DEFAULTLIGHTING, OnLightingDefaultlighting)
	ON_UPDATE_COMMAND_UI(ID_LIGHTING_DEFAULTLIGHTING, OnUpdateLightingDefaultlighting)
	ON_COMMAND(ID_LIGHTMAPTYPE_FULLCOLOUR, OnLightmaptypeFullcolour)
	ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_FULLCOLOUR, OnUpdateLightmaptypeFullcolour)
	ON_COMMAND(ID_LIGHTMAPTYPE_MULTIPLIED, OnLightmaptypeMultiplied)
	ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_MULTIPLIED, OnUpdateLightmaptypeMultiplied)
	ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_3, OnUpdateLightmaptype3)
	ON_COMMAND(ID_LIGHTMAPTYPE_3, OnLightmaptype3)
	ON_COMMAND(ID_LIGHTMAPTYPE_2, OnLightmaptype2)
	ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_2, OnUpdateLightmaptype2)
	ON_COMMAND(ID_SHOW_NULLS, OnShowNulls)
	ON_UPDATE_COMMAND_UI(ID_SHOW_NULLS, OnUpdateShowNulls)
	ON_COMMAND(ID_SHOW_MESHES, OnShowMeshes)
	ON_UPDATE_COMMAND_UI(ID_SHOW_MESHES, OnUpdateShowMeshes)
	ON_COMMAND(ID_SHOW_SPLINES, OnShowSplines)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SPLINES, OnUpdateShowSplines)
	ON_COMMAND(ID_SHOW_INSTANCES, OnShowInstances)
	ON_UPDATE_COMMAND_UI(ID_SHOW_INSTANCES, OnUpdateShowInstances)
	ON_COMMAND(ID_SHOW_POINTLIGHTS, OnShowPointlights)
	ON_UPDATE_COMMAND_UI(ID_SHOW_POINTLIGHTS, OnUpdateShowPointlights)
	ON_COMMAND(ID_SHOW_SPOTLIGHTS, OnShowSpotlights)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SPOTLIGHTS, OnUpdateShowSpotlights)
	ON_COMMAND(ID_SHOW_DIRECTIONALLIGHTS, OnShowDirectionallights)
	ON_UPDATE_COMMAND_UI(ID_SHOW_DIRECTIONALLIGHTS, OnUpdateShowDirectionallights)
	ON_COMMAND(ID_SHOW_JOINTS, OnShowJoints)
	ON_UPDATE_COMMAND_UI(ID_SHOW_JOINTS, OnUpdateShowJoints)
	ON_COMMAND(ID_VIEW_SHOWGRID, OnViewShowgrid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWGRID, OnUpdateViewShowgrid)
	ON_COMMAND(ID_SHADING_SHOWWEIGHTING, OnShadingShowweighting)
	ON_UPDATE_COMMAND_UI(ID_SHADING_SHOWWEIGHTING, OnUpdateShadingShowweighting)
	ON_COMMAND(ID_SHADING_FIXEDFUNCTIONSKINNING, OnShadingFixedfunctionskinning)
	ON_UPDATE_COMMAND_UI(ID_SHADING_FIXEDFUNCTIONSKINNING, OnUpdateShadingFixedfunctionskinning)
	ON_COMMAND(ID_SHADING_SHADERSKINNING, OnShadingShaderskinning)
	ON_UPDATE_COMMAND_UI(ID_SHADING_SHADERSKINNING, OnUpdateShadingShaderskinning)
/*
	ON_COMMAND(ID_SHADING_SHOWUV0, OnShadingShowuv0)
	ON_UPDATE_COMMAND_UI(ID_SHADING_SHOWUV0, OnUpdateShadingShowuv0)
	ON_COMMAND(ID_SHADING_SHOWUV1, OnShadingShowuv1)
	ON_UPDATE_COMMAND_UI(ID_SHADING_SHOWUV1, OnUpdateShadingShowuv1)
	ON_COMMAND(ID_VIEW_SCENEBOUNDINGBOX, OnViewSceneboundingbox)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SCENEBOUNDINGBOX, OnUpdateViewSceneboundingbox)
	ON_COMMAND(ID_RENDERING_VERTEXTANGENTSPACE, OnRenderingVertextangentspace)
	ON_UPDATE_COMMAND_UI(ID_RENDERING_VERTEXTANGENTSPACE, OnUpdateRenderingVertextangentspace)
	ON_COMMAND(ID_TASKS_OUTPUTSCENEBOUNDS, OnTasksOutputscenebounds)
	ON_COMMAND(ID_TASKSREPAIRSCENE, OnTasksrepairscene)
	ON_COMMAND(ID_TASKS_CALCULATEVERTEXTANGENTSPACEFORSCENE, OnTasksCalculatevertextangentspaceforscene)
	ON_COMMAND(ID_TASKS_WELDMESHCOMPONENTSFORSCENE, OnTasksWeldmeshcomponentsforscene)
	ON_COMMAND(ID_TASKS_WELDMESHPOLYGONVERTICESFORSCENE, OnTasksWeldmeshpolygonverticesforscene)
	ON_UPDATE_COMMAND_UI(ID_TASKS_WELDMESHPOLYGONVERTICESFORSCENE, OnUpdateTasksWeldmeshpolygonverticesforscene)
	ON_UPDATE_COMMAND_UI(ID_TASKS_WELDMESHCOMPONENTSFORSCENE, OnUpdateTasksWeldmeshcomponentsforscene)
	ON_UPDATE_COMMAND_UI(ID_TASKS_CALCULATEVERTEXTANGENTSPACEFORSCENE, OnUpdateTasksCalculatevertextangentspaceforscene)
	ON_UPDATE_COMMAND_UI(ID_TASKSREPAIRSCENE, OnUpdateTasksrepairscene)
	ON_UPDATE_COMMAND_UI(ID_TASKS_OUTPUTSCENEBOUNDS, OnUpdateTasksOutputscenebounds)
*/
//	ON_MESSAGE(WM_DROPFILES,OnDropFiles)// Message Handler for Drang and Drop
END_MESSAGE_MAP()

// NexusView construction/destruction

NexusView::NexusView()
{
	g_pView = this;
	memset(&m_keyboardState, 0, sizeof(m_keyboardState));
	memset(&m_mouseButtonState, 0, sizeof(m_mouseButtonState));

	m_mouseXOld = 0;
	m_mouseYOld = 0;

	m_initialPaint = true;
}

NexusView::~NexusView()
{
	m_nexusRender.Destroy();
}

BOOL NexusView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// NexusView drawing

void NexusView::OnDraw(CDC* /*pDC*/)
{
	Tu::Status status;
	NexusDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (m_nexusRender.IsCreated())
	{
		status = m_nexusRender.Draw();
		if (!status)
		{
			g_pMainFrame->OnCriticalError(status);
		}
	}
}


// NexusView printing

BOOL NexusView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void NexusView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void NexusView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// NexusView diagnostics

#ifdef _DEBUG
void NexusView::AssertValid() const
{
	CView::AssertValid();
}

void NexusView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

NexusDoc* NexusView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(NexusDoc)));
	return (NexusDoc*)m_pDocument;
}
#endif //_DEBUG


void NexusView::KeyboardEvent(UINT nChar, UINT nRepCnt, UINT nFlags, bool pressed)
{
	bool viewInvalidated = false;

	switch (nChar)
	{
		case 'C':
		case 'c':
		{
			m_nexusRender.CentreCamera();
			viewInvalidated = true;
			break;
		}

		case 'A':
		case 'a':
		{
			m_nexusRender.FrameAll();
			viewInvalidated = true;
			break;
		}

		case 'F':
		case 'f':
		{
			m_nexusRender.FrameSelected();
			viewInvalidated = true;
			break;
		}

		case 'R':
		case 'r':
		{
			m_nexusRender.ResetAnimation();
			viewInvalidated = true;
			break;
		}

		case VK_LEFT:
		{
			S32 t = (S32)(Nexus::g_pNexusSystem->GetScenePtr()->GetFrameRate()*0.25);
			m_nexusRender.FrameSkip(-t);
			viewInvalidated = true;
			break;
		}

		case VK_RIGHT:
		{
			S32 t = (S32)(Nexus::g_pNexusSystem->GetScenePtr()->GetFrameRate()*0.25);
			m_nexusRender.FrameSkip(t);
			viewInvalidated = true;
			break;
		}

		case ' ':
		{
			if (pressed)
			{
				m_nexusRender.ToggleAnimation();
				viewInvalidated = true;
			}
			break;
		}
	}

	if (viewInvalidated)
	{
		GetDocument()->UpdateAllViews(0);
	}
}

void NexusView::MouseMoveEvent(UINT nFlags, CPoint point)
{
	bool viewInvalidated = false;
	S32 mouseX = point.x - m_mouseXOld;
	S32 mouseY = point.y - m_mouseYOld;

	m_mouseXOld = point.x;
	m_mouseYOld = point.y;

	switch (m_viewportState)
	{
		case NEXUS_VIEWPORT_STATE_VIEW_ROTATE:
		{
			SetCursor(LoadCursor(NULL, (LPCSTR)32649));
			F32 mouseXF = F32(mouseX);
			mouseXF *= 0.01f;
			F32 mouseYF = F32(mouseY);
			mouseYF *= 0.01f;

			m_nexusRender.CameraPtr()->RotatePitch(mouseYF);
			m_nexusRender.CameraPtr()->RotateYaw(mouseXF);
			viewInvalidated = true;
			break;
		}

		case NEXUS_VIEWPORT_STATE_VIEW_DOLLY:
		{
			if (m_nexusRender.CameraPtr()->Type() == SRD::SRD_CAMERA_TYPE_TARGET)
			{
				SRD::TargetCamera* pTargetCamera = (SRD::TargetCamera*)m_nexusRender.CameraPtr();
				SetCursor(LoadCursor(NULL, IDC_SIZENS));

				F32 mouseYF = F32(mouseY);
				mouseYF *= 0.5f;
				
				F32 distancePx = -mouseYF;

				const SRD::D3DWindow* pD3DWindow = m_nexusRender.D3DWindowPtr();

				F32 widthWorld = (F32)pD3DWindow->Width();
				widthWorld *= pTargetCamera->ZDistance();

				F32 halfWidthPx = (F32)pD3DWindow->Width()*0.5f;
				F32 cz = halfWidthPx / tan(pTargetCamera->Fov());

				F32 distanceWorld = distancePx / cz;
				distanceWorld *= pTargetCamera->ZDistance()*4;

				pTargetCamera->MoveInZ(distanceWorld);
				viewInvalidated = true;
				m_nexusRender.OnCameraZoom();
			}
			break;
		}

		case NEXUS_VIEWPORT_STATE_VIEW_TRANSLATE:
		{
			if (m_nexusRender.CameraPtr()->Type() == SRD::SRD_CAMERA_TYPE_TARGET)
			{
				SRD::TargetCamera* pTargetCamera = (SRD::TargetCamera*)m_nexusRender.CameraPtr();

				SetCursor(LoadCursor(NULL, IDC_SIZEALL));
				F32 mouseXF = F32(mouseX);
				mouseXF *= 0.5f;
				F32 mouseYF = F32(mouseY);
				mouseYF *= 0.5f;
				
				const SRD::D3DWindow* pD3DWindow = m_nexusRender.D3DWindowPtr();

				F32 dxWorld = mouseXF / (F32)pD3DWindow->Width();
				dxWorld *= pTargetCamera->ZDistance();

				F32 dyWorld = mouseYF / (F32)pD3DWindow->Height();
				dyWorld *= pTargetCamera->ZDistance();

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

	if (viewInvalidated)
	{
		GetDocument()->UpdateAllViews(NULL);
	}
}

void NexusView::MouseButtonEvent(
	int buttonId, 
	NexusMouseButtonActionEnum action,
	UINT nFlags,
	CPoint point)
{
	switch (action)
	{
		case NEXUS_MOUSE_BUTTON_ACTION_DOWN:
			m_mouseButtonState[buttonId] = true;
			MouseButtonStateChanged(buttonId);
			break;

		case NEXUS_MOUSE_BUTTON_ACTION_UP:
			m_mouseButtonState[buttonId] = false;
			MouseButtonStateChanged(buttonId);
			break;

		case NEXUS_MOUSE_BUTTON_ACTION_DOUBLE_CLICK:
			break;

		default:
			break;
	};
}

void NexusView::MouseButtonStateChanged(int buttonId)
{
	if ((buttonId == NEXUS_MOUSE_BUTTON_LEFT)
		&&
		(m_mouseButtonState[buttonId]))
	{
		if (GetKeyState(VK_MENU)&0x8000)
		{
			m_viewportState = NEXUS_VIEWPORT_STATE_VIEW_ROTATE;
		}
		else
		{
			m_nexusRender.Pick();
		}
	}
	else if ((buttonId == NEXUS_MOUSE_BUTTON_MIDDLE)
		&&
		(m_mouseButtonState[buttonId]))
	{
		if (GetKeyState(VK_MENU)&0x8000)
		{
			if (m_viewportState != NEXUS_VIEWPORT_STATE_VIEW_TRANSLATE)
			{
				m_viewportState = NEXUS_VIEWPORT_STATE_VIEW_TRANSLATE;
			}
		}
	}
	else if ((buttonId == NEXUS_MOUSE_BUTTON_RIGHT)
		&&
		(m_mouseButtonState[buttonId]))
	{
		if (GetKeyState(VK_MENU)&0x8000)
		{
			if (m_viewportState != NEXUS_VIEWPORT_STATE_VIEW_DOLLY)
			{
				m_viewportState = NEXUS_VIEWPORT_STATE_VIEW_DOLLY;
			}
		}
	}
	else
	{
		if (m_viewportState != NEXUS_VIEWPORT_STATE_NO_ACTION)
		{
			m_viewportState = NEXUS_VIEWPORT_STATE_NO_ACTION;
		}
	}
}

// NexusView message handlers

void NexusView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
}

void NexusView::OnFinalRelease()
{
	// TODO: Add your specialized code here and/or call the base class
	CView::OnFinalRelease();
}

bool gbDisablePainting = false;

void NexusView::OnPaint()
{
	//Painting is permitted.
	RECT r;
	GetClientRect(&r);
	if (m_initialPaint == true)
	{
		m_nexusRender.Resize(r.right - r.left, r.bottom - r.top);
		m_initialPaint = false;
	}

	m_nexusRender.SetViewportRectangle(r);

	CView::OnPaint();
}

void NexusView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	m_nexusRender.Resize(cx, cy);
}

void NexusView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	RECT r;
	GetClientRect(&r);
	m_nexusRender.SetViewportRectangle(r);
	m_nexusRender.Draw();
	m_nexusRender.OnCameraZoom();
	
	CView::OnUpdate(pSender, lHint, pHint);
}

void NexusView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	m_keyboardState[nChar] = true;
	KeyboardEvent(nChar, nRepCnt, nFlags, true);

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void NexusView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	m_keyboardState[nChar] = false;
	KeyboardEvent(nChar, nRepCnt, nFlags, false);

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}

void NexusView::OnMouseMove(UINT nFlags, CPoint point)
{
	MouseMoveEvent(nFlags, point);

	CView::OnMouseMove(nFlags, point);
}

BOOL NexusView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void NexusView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnLButtonDblClk(nFlags, point);
}

void NexusView::OnLButtonDown(UINT nFlags, CPoint point)
{
	MouseButtonEvent(NEXUS_MOUSE_BUTTON_LEFT, NEXUS_MOUSE_BUTTON_ACTION_DOWN, nFlags, point);

	CView::OnLButtonDown(nFlags, point);
}

void NexusView::OnLButtonUp(UINT nFlags, CPoint point)
{
	MouseButtonEvent(NEXUS_MOUSE_BUTTON_LEFT, NEXUS_MOUSE_BUTTON_ACTION_UP, nFlags, point);

	CView::OnLButtonUp(nFlags, point);
}

void NexusView::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	CView::OnMButtonDblClk(nFlags, point);
}

void NexusView::OnMButtonDown(UINT nFlags, CPoint point)
{
	MouseButtonEvent(NEXUS_MOUSE_BUTTON_MIDDLE, NEXUS_MOUSE_BUTTON_ACTION_DOWN, nFlags, point);

	CView::OnMButtonDown(nFlags, point);
}

void NexusView::OnMButtonUp(UINT nFlags, CPoint point)
{
	MouseButtonEvent(NEXUS_MOUSE_BUTTON_MIDDLE, NEXUS_MOUSE_BUTTON_ACTION_UP, nFlags, point);

	CView::OnMButtonUp(nFlags, point);
}

void NexusView::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	CView::OnRButtonDblClk(nFlags, point);
}

void NexusView::OnRButtonDown(UINT nFlags, CPoint point)
{
	MouseButtonEvent(NEXUS_MOUSE_BUTTON_RIGHT, NEXUS_MOUSE_BUTTON_ACTION_DOWN, nFlags, point);
/*
	m_nexusRender.UpdateTime(1.0f);
	if (m_nexusRender.Time() > 200)
	{
		m_nexusRender.SetTime(80);
	}

	NexusDoc* pNexusDoc = GetDocument();
	pNexusDoc->UpdateAllViews(0);
*/
	CView::OnRButtonDown(nFlags, point);
}

void NexusView::OnRButtonUp(UINT nFlags, CPoint point)
{
	MouseButtonEvent(NEXUS_MOUSE_BUTTON_RIGHT, NEXUS_MOUSE_BUTTON_ACTION_UP, nFlags, point);

	CView::OnRButtonUp(nFlags, point);
}

BOOL NexusView::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void NexusView::OnUpdateShadingSmoothshadeall(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.GlobalShadingMode() == NEXUS_SHADING_TYPE_SMOOTH)
	{
		pCmdUI->SetCheck( true );
	}
	else
	{
		pCmdUI->SetCheck( false);
	}
}

void NexusView::OnUpdateShadingWireframeall(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.GlobalShadingMode() == NEXUS_SHADING_TYPE_WIRE_FRAME)
	{
		pCmdUI->SetCheck( true );
	}
	else
	{
		pCmdUI->SetCheck( false );
	}
}

void NexusView::OnUpdateShadingFlatshadeall(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.GlobalShadingMode() == NEXUS_SHADING_TYPE_FLAT)
	{
		pCmdUI->SetCheck( true );
	}
	else
	{
		pCmdUI->SetCheck( false );
	}
}

void NexusView::OnShadingPointsall()
{
	m_nexusRender.SetGlobalShadingMode(NEXUS_SHADING_TYPE_POINTS);
	GetDocument()->UpdateAllViews(0);
}

void NexusView::OnUpdateShadingBackfaceculling(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.Settings().BackFaceCulling())
	{
		pCmdUI->SetCheck( true );
	}
	else
	{
		pCmdUI->SetCheck( false );
	}
}

void NexusView::OnUpdateShadingTexturing(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.Settings().UseTextures())
	{
		pCmdUI->SetCheck( true );
	}
	else
	{
		pCmdUI->SetCheck( false );
	}
}

void NexusView::OnShadingWireframeall()
{
	m_nexusRender.SetGlobalShadingMode(NEXUS_SHADING_TYPE_WIRE_FRAME);
	GetDocument()->UpdateAllViews(0);
}

void NexusView::OnShadingSmoothshadeall()
{
	m_nexusRender.SetGlobalShadingMode(NEXUS_SHADING_TYPE_SMOOTH);
	GetDocument()->UpdateAllViews(0);
}

void NexusView::OnShadingFlatshadeall()
{
	m_nexusRender.SetGlobalShadingMode(NEXUS_SHADING_TYPE_FLAT);
	GetDocument()->UpdateAllViews(0);
}

void NexusView::OnShadingTexturing()
{
	m_nexusRender.Settings().SetUseTextures((!m_nexusRender.Settings().UseTextures()));
	if (m_nexusRender.Settings().UseTextures())
	{
		if (!m_nexusRender.TexturesLoaded())
		{
			m_nexusRender.OnLoadTextures();
		}
	}
		
	GetDocument()->UpdateAllViews(0);
}

void NexusView::OnUpdateShadingPointsall(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.GlobalShadingMode() == NEXUS_SHADING_TYPE_POINTS)
	{
		pCmdUI->SetCheck( true );
	}
	else
	{
		pCmdUI->SetCheck( false );
	}
}

void NexusView::OnShadingBackfaceculling()
{
	m_nexusRender.Settings().SetBackFaceCulling(!m_nexusRender.Settings().BackFaceCulling());
	GetDocument()->UpdateAllViews(0);
}

void NexusView::OnUpdateRenderingVertexnormals(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(0);
	return;
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().RenderNormals())
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnRenderingVertexnormals()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			m_nexusRender.Settings().SetRenderNormals(!m_nexusRender.Settings().RenderNormals());
			m_nexusRender.ISceneRendererPtr()->OnWidgetRenderingChanged();
			GetDocument()->UpdateAllViews(0);
		}
	}
}

// Display next cell in animation sequence 
LRESULT NexusView::UpdateTime(WPARAM wParam, LPARAM lParam)
{
	NexusDoc* pNexusDoc = GetDocument();

	pNexusDoc->UpdateAllViews(0);
    return 0;
}


void NexusView::OnRenderingAxes()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			m_nexusRender.Settings().SetRenderAxes(!m_nexusRender.Settings().RenderAxes());
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateRenderingAxes(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().RenderAxes())
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnLightingDefaultlighting()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			m_nexusRender.Settings().SetUseRealTimeLighting(!m_nexusRender.Settings().UseRealTimeLighting());
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateLightingDefaultlighting(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().UseRealTimeLighting())
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnLightmaptypeFullcolour()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			m_nexusRender.Settings().SetLightmapMode(0);
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateLightmaptypeFullcolour(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().LightmapMode() == 1)
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnLightmaptypeMultiplied()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			m_nexusRender.Settings().SetLightmapMode(3);
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateLightmaptypeMultiplied(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().LightmapMode() == 4)
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnUpdateLightmaptype3(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().LightmapMode() == 3)
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnLightmaptype3()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			m_nexusRender.Settings().SetLightmapMode(2);
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnLightmaptype2()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			m_nexusRender.Settings().SetLightmapMode(1);
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateLightmaptype2(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().LightmapMode() == 2)
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnShowNulls()
{
}

void NexusView::OnUpdateShowNulls(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);
}

void NexusView::OnShowMeshes()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			m_nexusRender.Settings().SetRenderMeshes(!m_nexusRender.Settings().RenderMeshes());
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateShowMeshes(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().RenderMeshes())
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnShowSplines()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			bool inverted = !m_nexusRender.Settings().RenderSplines();
			m_nexusRender.Settings().SetRenderSplines(inverted);
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateShowSplines(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().RenderSplines())
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnShowInstances()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			m_nexusRender.Settings().SetRenderInstances(!m_nexusRender.Settings().RenderInstances());
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateShowInstances(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().RenderInstances())
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnShowPointlights()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			m_nexusRender.Settings().SetRenderPointLights(!m_nexusRender.Settings().RenderPointLights());
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateShowPointlights(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().RenderPointLights())
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnShowSpotlights()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			m_nexusRender.Settings().SetRenderSpotLights(!m_nexusRender.Settings().RenderSpotLights());
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateShowSpotlights(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().RenderSpotLights())
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnShowDirectionallights()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			m_nexusRender.Settings().SetRenderDirectionalLights(!m_nexusRender.Settings().RenderDirectionalLights());
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateShowDirectionallights(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().RenderDirectionalLights())
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnShowJoints()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			m_nexusRender.Settings().SetRenderJoints(!m_nexusRender.Settings().RenderJoints());
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateShowJoints(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().RenderJoints())
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnViewShowgrid()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			m_nexusRender.Settings().SetShowGrid(!m_nexusRender.Settings().ShowGrid());
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateViewShowgrid(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().ShowGrid())
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnShadingShowweighting()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			if (m_nexusRender.Settings().ComponentMode() == 1)
			{
				m_nexusRender.Settings().SetComponentMode(0);
			}
			else
			{
				m_nexusRender.Settings().SetComponentMode(1);
			}
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateShadingShowweighting(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().ComponentMode() ==1)
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnDebugOutputscenegraph()
{
	// TODO: Add your command handler code here
	Nexus::g_pNexusSystem->GetScenePtr()->OutputSceneGraph();
}

void NexusView::OnShadingFixedfunctionskinning()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			if (m_nexusRender.Settings().SkinningMode() == 1)
			{
				m_nexusRender.Settings().SetSkinningMode(0);
			}
			else
			{
				m_nexusRender.Settings().SetSkinningMode(1);
			}
			m_nexusRender.ISceneRendererPtr()->OnSkinningModeChanged();
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateShadingFixedfunctionskinning(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().SkinningMode() == 1)
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnShadingShaderskinning()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			if (m_nexusRender.Settings().SkinningMode() == 2)
			{
				m_nexusRender.Settings().SetSkinningMode(0);
			}
			else
			{
				m_nexusRender.Settings().SetSkinningMode(2);
			}
			m_nexusRender.ISceneRendererPtr()->OnSkinningModeChanged();
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateShadingShaderskinning(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(FALSE);
	if (!m_nexusRender.ISceneRendererPtr())
	{
		return;
	}
	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().SkinningMode() == 2)
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}
/*
void NexusView::OnShadingShowuv0()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			if (m_nexusRender.Settings().ComponentMode() == 2)
			{
				m_nexusRender.Settings().SetComponentMode(0);
			}
			else
			{
				m_nexusRender.Settings().SetComponentMode(2);
			}
			GetDocument()->UpdateAllViews(0);
		}
	}

}

void NexusView::OnUpdateShadingShowuv0(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);
	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().ComponentMode() == 2)
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnShadingShowuv1()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			if (m_nexusRender.Settings().ComponentMode() == 3)
			{
				m_nexusRender.Settings().SetComponentMode(0);
			}
			else
			{
				m_nexusRender.Settings().SetComponentMode(3);
			}
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateShadingShowuv1(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().ComponentMode() == 3)
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnViewSceneboundingbox()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			if (m_nexusRender.Settings().BoundingBoxMode() == 1)
			{
				m_nexusRender.Settings().SetBoundingBoxMode(0);
			}
			else
			{
				m_nexusRender.Settings().SetBoundingBoxMode(1);
			}
			GetDocument()->UpdateAllViews(0);
		}
	}

}

void NexusView::OnUpdateViewSceneboundingbox(CCmdUI *pCmdUI)
{
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().BoundingBoxMode() == 1)
		{
			pCmdUI->SetCheck( true );
		}
		else
		{
			pCmdUI->SetCheck( false );
		}
	}
}

void NexusView::OnRenderingVertextangentspace()
{
	if (m_nexusRender.ISceneRendererPtr())
	{
		if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
		{
			SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
			if (m_nexusRender.Settings().TangentSpaceMode() == -1)
			{
				m_nexusRender.Settings().SetTangentSpaceMode(0);
			}
			else
			{
				m_nexusRender.Settings().SetTangentSpaceMode(-1);
			}
			m_nexusRender.ISceneRendererPtr()->OnWidgetRenderingChanged();
			GetDocument()->UpdateAllViews(0);
		}
	}
}

void NexusView::OnUpdateRenderingVertextangentspace(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(0);
	return;
	if (!m_nexusRender.ISceneRendererPtr())
	{
		pCmdUI->Enable(0);
		return;
	}

	if (!m_nexusRender.ISceneRendererPtr()->IsBuilt())
	{
		pCmdUI->Enable(0);
		return;
	}
	pCmdUI->Enable(1);

	if (m_nexusRender.ISceneRendererPtr()->Type() == SRD::SCENE_RENDERER_TYPE_GENERIC)
	{
		SRD::GSceneRenderer* pSceneRenderer = (SRD::GSceneRenderer*)m_nexusRender.ISceneRendererPtr();
		if (m_nexusRender.Settings().TangentSpaceMode() == -1)
		{
			pCmdUI->SetCheck( false );
		}
		else
		{
			pCmdUI->SetCheck( true );
		}
	}
}

void NexusView::OnTasksOutputscenebounds()
{
	Nexus::g_pNexusSystem->OutputSceneBounds();
}

void NexusView::OnTasksrepairscene()
{
	// TODO: Add your command handler code here
}

void NexusView::OnTasksCalculatevertextangentspaceforscene()
{
	// TODO: Add your command handler code here
}

void NexusView::OnTasksWeldmeshcomponentsforscene()
{
	// TODO: Add your command handler code here
}

void NexusView::OnTasksWeldmeshpolygonverticesforscene()
{
	// TODO: Add your command handler code here
}

void NexusView::OnUpdateTasksWeldmeshpolygonverticesforscene(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(0);
}

void NexusView::OnUpdateTasksWeldmeshcomponentsforscene(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(0);
}

void NexusView::OnUpdateTasksCalculatevertextangentspaceforscene(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(0);
}

void NexusView::OnUpdateTasksrepairscene(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(0);
}

void NexusView::OnUpdateTasksOutputscenebounds(CCmdUI *pCmdUI)
{
	SRD::ISceneRenderer* pSceneRenderer = m_nexusRender.ISceneRendererPtr();
	if (pSceneRenderer)
	{
		if (pSceneRenderer->IsBuilt())
		{
			pCmdUI->Enable(1);
			return;
		}
	}
	pCmdUI->Enable(0);
}
*/