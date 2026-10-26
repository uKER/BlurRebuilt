#include "stdafx.h" 
#include <wx/wx.h>
#include "wx/aui/aui.h"



 AftkRender::C_Application *m_pAftkRenderApp = 0;
 AftkRender::C_SceneRenderer *m_pSceneRenderer = 0;
 Aftk::C_Scene *m_pScene = 0;


class RenderCanvas : public wxControl
{
public:
	RenderCanvas(wxFrame *pParent, wxWindowID id)
		: wxControl(pParent,id,wxDefaultPosition, wxSize(300,300), wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxNO_BORDER)
	{ }
	~RenderCanvas()
	{
	int kk = 0;
	}

	void OnSize(wxSizeEvent& e);
	void OnPaint(wxPaintEvent& e);
	void OnEraseBackground(wxEraseEvent &e);
	void OnRenderTimer(wxTimerEvent& e);

	void Render();
	AftkRender::C_ViewContext *pContext;
private:
	AftkRender::C_Application* m_pApp;

	DECLARE_CLASS(RenderCanvas)
	DECLARE_EVENT_TABLE()
};

const long ID_RENDERTIMER = wxNewId();

IMPLEMENT_CLASS(RenderCanvas, wxControl)
BEGIN_EVENT_TABLE(RenderCanvas, wxControl)
	EVT_SIZE(RenderCanvas::OnSize)
	//EVT_PAINT(RenderCanvas::OnPaint)
	EVT_ERASE_BACKGROUND( RenderCanvas::OnEraseBackground )
	EVT_TIMER( ID_RENDERTIMER, RenderCanvas::OnRenderTimer )
END_EVENT_TABLE()

void RenderCanvas::OnSize(wxSizeEvent &e)
{
	int w, h;
	GetSize(&w, &h);
	if (m_pAftkRenderApp)
		m_pAftkRenderApp->Resize(w,h, pContext);

	Render();
}

void RenderCanvas::OnPaint(wxPaintEvent &e)
{
}

void RenderCanvas::OnEraseBackground(wxEraseEvent &e)
{
	Render();
}
void RenderCanvas::OnRenderTimer(wxTimerEvent &e)
{
	//m_pApp->UpdateTimer(e);
}

void RenderCanvas::Render()
{
	if (m_pSceneRenderer)
	{
		m_pAftkRenderApp->BeginRendering(pContext);

		Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(127, 127, 127, 255));
		m_pSceneRenderer->Render();
		m_pAftkRenderApp->EndRendering(pContext);
	}
}




class MainGuiFrame : public wxFrame {
 public:
   MainGuiFrame(wxWindow* parent) 
	   : wxFrame(parent, -1, _("Test Application"), wxDefaultPosition, wxSize(800,600), wxDEFAULT_FRAME_STYLE)                              
   {
     m_mgr.SetManagedWindow(this);
	 m_mgr.SetFlags(wxAUI_MGR_ALLOW_FLOATING | wxAUI_MGR_TRANSPARENT_DRAG | wxAUI_MGR_TRANSPARENT_HINT);

	 m_pMenuBar = new wxMenuBar(0);
	 m_pMenuBar->Append(new wxMenu(), wxT("File"));
	 m_pStatusBar = this->CreateStatusBar(1, wxST_SIZEGRIP, wxID_ANY, wxStatusLineNameStr);
	 SetMenuBar(m_pMenuBar);
 
     wxTextCtrl* text1 = new wxTextCtrl(this, -1, _("Window1"),
                      wxDefaultPosition, wxSize(200,150),
                      wxNO_BORDER | wxTE_MULTILINE);

      wxTextCtrl* text2 = new wxTextCtrl(this, -1, _("Window1"),
                      wxDefaultPosition, wxSize(200,150),
                      wxNO_BORDER | wxTE_MULTILINE);

      wxAuiNotebook* pane2 = new wxAuiNotebook(this, -1);
	  pane2->AddPage(text2, "tabtest");


      //add the panes to the manager
     m_mgr.AddPane(text1, wxBOTTOM, wxT("Pane 1"));
     m_mgr.AddPane(pane2, wxBOTTOM, wxT("Pane 2"));

	 RenderCanvas *pViewport0 = new RenderCanvas(this, -1);
	 m_mgr.AddPane(pViewport0, wxAuiPaneInfo().
                  Name(wxT("test1")).Caption(wxT("Main View")).
                  Center().Layer(0).Position(0).
                  CloseButton(false).MaximizeButton(true));

	 RenderCanvas *pViewport1 = new RenderCanvas(this, -1);
	 m_mgr.AddPane(pViewport1, wxAuiPaneInfo().
                  Name(wxT("test2")).Caption(wxT("Perspective View")).
                  Right().Layer(0).Position(0).
                  CloseButton(true).MaximizeButton(true));

	 RenderCanvas *pViewport2 = new RenderCanvas(this, -1);
 	 m_mgr.AddPane(pViewport2, wxAuiPaneInfo().
                  Name(wxT("test3")).Caption(wxT("Another Perspective View")).
                  Right().Layer(0).Position(1).
                  CloseButton(true).MaximizeButton(true));

	 m_mgr.GetArtProvider()->SetMetric(wxAUI_DOCKART_SASH_SIZE, 2);


   Aftk::C_Global::Create();
	m_pAftkRenderApp = new AftkRender::C_Application();
	m_pAftkRenderApp->m_DeviceParams.m_Width = 1;
	m_pAftkRenderApp->m_DeviceParams.m_Height = 1;
	m_pAftkRenderApp->m_DeviceParams.m_hWnd = (HWND) pViewport0->GetHandle();
	m_pAftkRenderApp->m_DeviceParams.m_Multisample = Display::E_Multisample_2;

	if (!m_pAftkRenderApp->Create(pViewport0->GetSize().GetWidth(), pViewport0->GetSize().GetHeight()))
		int i=0;

	pViewport0->pContext = m_pAftkRenderApp->GetViewContext(0);

	Display::C_DeviceParams params = m_pAftkRenderApp->m_DeviceParams;
	params.m_hWnd = (HWND) pViewport1->GetHandle();
	pViewport1->pContext = m_pAftkRenderApp->AddViewContext(params);

	params.m_hWnd = (HWND) pViewport2->GetHandle();
	pViewport2->pContext = m_pAftkRenderApp->AddViewContext(params);

	m_pSceneRenderer = new AftkRender::C_SceneRenderer();
	m_pSceneRenderer->Create();
	m_pSceneRenderer->CreateVolatile();
	
	m_pScene = new Aftk::C_Scene();
	if (!m_pScene->Read("testassets/barrierskin.a3d"))
		int i=0;

	//Aftk::C_SceneNodeConstIterator it(m_pScene);
	//while (it)
	//{
	//	AftkRender::C_MeshResource res("hello");
	//	if (it->GetType() == Aftk::E_ObjType_SceneMesh)
	//	{
	//		AftkRender::C_MeshBuilder::Build(&res, (Aftk::C_SceneMeshNode*)&(*it));
	//		AftkRender::C_MeshNode mesh(&res);
	//		mesh.Build();
	//	}
	//	it++;
	//}

	m_pSceneRenderer->LoadFromScene(m_pScene);

     m_mgr.Update();
   }
 
   ~MainGuiFrame()
   {
		m_pSceneRenderer->Destroy();
		m_pSceneRenderer->DestroyVolatile();
		SAFE_DELETE_PTR(m_pSceneRenderer);
		SAFE_DELETE_PTR(m_pAftkRenderApp);
		SAFE_DELETE_PTR(m_pScene);
		Aftk::C_Global::Destroy();
     m_mgr.UnInit();
   }
 
 private:
     wxAuiManager m_mgr;

	wxMenuBar			*m_pMenuBar;
	wxStatusBar			*m_pStatusBar;
 };
 

 class TestApp : public wxApp {
 public:
	TestApp()
	: wxApp()
	{
	}
 
   bool OnInit()
   {
		MainGuiFrame* frame = new MainGuiFrame(NULL);

		SetTopWindow(frame);
		frame->Show();
		return true;                    
   }

   int OnExit()
   {
		return wxApp::OnExit();
   }
 };
 
  DECLARE_APP(TestApp);
  IMPLEMENT_APP(TestApp);