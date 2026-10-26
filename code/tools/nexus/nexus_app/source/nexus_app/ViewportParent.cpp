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

Nexus::C_ViewportParent* Nexus::g_pViewportParent = 0;

using namespace Nexus;

// NexusView
#include "../../resource.h"

IMPLEMENT_DYNCREATE(C_ViewportParent, CView)

BEGIN_MESSAGE_MAP(C_ViewportParent, CView)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
END_MESSAGE_MAP()

C_ViewportParent::C_ViewportParent()
{
	g_pViewportParent = this;

	m_initialPaint = true;

	memset(&m_viewports, 0, sizeof(m_viewports));
}

int C_ViewportParent::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pDisplayManager = new C_DisplayManager();
	if (!m_pDisplayManager)
		return -1;

	m_viewports[0] = new C_Viewport();
	if (!m_viewports[0])
		return -1;

	RECT clientRect;
	GetClientRect(&clientRect);
	if (!m_viewports[0]->Create(
		lpCreateStruct->lpszClass, 
		0,
		lpCreateStruct->style,
		clientRect,
		this,
		IDD_VIEWPORT_CHILD_0))
		return -1;

	return 0;
}

C_ViewportParent::~C_ViewportParent()
{
	for (U32 i = 0; i < NEXUS_NUM_VIEWPORTS; i++)
		if (m_viewports[i])
			delete m_viewports[i];

	if (m_pDisplayManager)
	{
		delete m_pDisplayManager;
	}
}

C_DisplayManager* C_ViewportParent::GetDisplayManagerPtr()
{
	return m_pDisplayManager;
}

C_Viewport* C_ViewportParent::GetViewportPtr(U32 index)
{
	return m_viewports[index];
}

BOOL C_ViewportParent::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

void C_ViewportParent::OnDraw(CDC* pDC)
{
	CView::OnDraw(pDC);

	for (U32 i = 0; i < NEXUS_NUM_VIEWPORTS; i++)
		if (m_viewports[i])
			m_viewports[i]->OnDraw(pDC);

	LARGE_INTEGER startTime = g_pNexusSystem->GetFrameStartTime();
	if (startTime.QuadPart == 0)
	{
		//First call of render code.
		if (!QueryPerformanceCounter(&startTime))
		{
			memset(&startTime, 0, sizeof(LARGE_INTEGER));
			//TODO: Handle this.
		}
		g_pNexusSystem->SetFrameStartTime(startTime);
	}
	else
	{
		LARGE_INTEGER endTime;
		if (!QueryPerformanceCounter(&endTime))
		{
			memset(&endTime, 0, sizeof(LARGE_INTEGER));
			//TODO: Handle this.
		}

		//Advance time.
		U64 ticks = (endTime.QuadPart - startTime.QuadPart);
		startTime = endTime;

		//Frequency is the number of ticks/second
		//Therefore ticks represents ticks / freq of a second
		F32 dtf = (F32)ticks / (F32)g_pNexusSystem->GetTimerFrequency().QuadPart;

		F32 frameRate = g_pNexusSystem->GetCorePtr()->GetAftkScenePtr()->GetFrameRate();
		g_pNexusSystem->UpdateTime(dtf*frameRate);

		g_pNexusSystem->SetFrameStartTime(endTime);
	}
}

#ifdef _DEBUG
void C_ViewportParent::AssertValid() const
{
	CView::AssertValid();
}

void C_ViewportParent::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif

void C_ViewportParent::OnInitialUpdate()
{
	CView::OnInitialUpdate();
}

void C_ViewportParent::OnFinalRelease()
{
	// TODO: Add your specialized code here and/or call the base class
	CView::OnFinalRelease();
}

void C_ViewportParent::OnPaint()
{
	CView::OnPaint();

	for (U32 i = 0; i < NEXUS_NUM_VIEWPORTS; i++)
		if (m_viewports[i])
			m_viewports[i]->OnPaint();
}

void C_ViewportParent::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	for (U32 i = 0; i < NEXUS_NUM_VIEWPORTS; i++)
		if (m_viewports[i])
			m_viewports[i]->SetWindowPos(0, 0, 0, cx, cy, 0);
}

void C_ViewportParent::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	CView::OnUpdate(pSender, lHint, pHint);
}

BOOL C_ViewportParent::OnEraseBkgnd(CDC* pDC)
{
	return true;
}

void C_ViewportParent::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	g_pNexusSystem->OnKeyDown(nChar, nRepCnt, nFlags);
}

void C_ViewportParent::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	g_pNexusSystem->OnKeyUp(nChar, nRepCnt, nFlags);
}
