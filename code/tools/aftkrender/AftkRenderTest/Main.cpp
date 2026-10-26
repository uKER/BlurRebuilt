#include "Stdafx.h"


#include "Samples/System/PCApplication.h"
#include "samples/section/Section.h"
#include "render/render/GridRender.h"

Aftk::C_Scene *g_pScene = NULL;

//#define USE_AFTKRENDER_WINDOW

namespace Test
{
	class RenderSceneSection : public Samples::C_Section
	{
	public:
		RenderSceneSection(const Util::C_String& name)
		: C_Section(name)
		{
			m_pSceneRenderer = new AftkRender::C_SceneRenderer();
			m_pSceneRenderer->LoadFromScene(g_pScene);
		}
		~RenderSceneSection()
		{
			delete m_pSceneRenderer;
		}

		void OnUpdate(const Samples::C_Input& input, const Samples::C_Timer& timer)
		{
		}

		void OnPreRender(const Samples::C_Input& input, const Samples::C_Timer& timer)
		{
		}

		void OnRender(const Samples::C_Input& input, const Samples::C_Timer& timer)
		{
			m_pSceneRenderer->Update(timer.GetElapsedTime());

			Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(200, 200, 200, 255));

			Util::C_String str;
			str.Printf("frame rate: %.1f", timer.GetFrameRate());
			Display::FontRender()->Draw(str.Chars(), 30.f, 45.f, 20.f, 20.f, Maths::RGBA(0, 0, 0, 255));

			Render::DrawGrid();
			m_pSceneRenderer->Render();
		}

		void OnPostRender(const Samples::C_Input& input, const Samples::C_Timer& timer)
		{
			if (input.IsKeyDn(VK_INSERT))
				AftkRender::Utility::CaptureRenderTarget(Util::C_String("screengrab.png"));
		}

	private:
		AftkRender::C_SceneRenderer *m_pSceneRenderer;
	};


	class SceneRenderer : public Samples::C_PCApplication
	{
	public:
		SceneRenderer()
		{
		}
		~SceneRenderer()
		{
		}
		bool MyCreate()
		{
			AddSection(new Samples::C_SectionLauncher<RenderSceneSection>("TestScene"));
			return true;
		}
	};
};

Test::SceneRenderer* g_pSceneRenderer = NULL;


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (g_pSceneRenderer)
		return g_pSceneRenderer->MsgProc(hWnd, message, wParam, lParam);
	else
		return 0;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{

	Aftk::C_Global::Create();

#ifdef USE_AFTKRENDER_WINDOW

	AftkRender::C_Window *pWindow = new AftkRender::C_Window(hInstance);
	pWindow->Create(false, 640, 480);
	AftkRender::C_Application app;

	app.m_DeviceParams.m_hInstance = hInstance;
	app.m_DeviceParams.m_hWnd = pWindow->GetWindowHandle();
	app.Create(pWindow->GetWidth(), pWindow->GetHeight());

	g_pScene = new Aftk::C_Scene();
	Aftk::C_Status status = g_pScene->Read("TestAssets/uvissue.a3d");
	AftkRender::C_SceneRenderer renderer;
	renderer.LoadFromScene(g_pScene);


	Display::Device()->BeginScene();

	Display::Device()->SetColourDepthStencilSurface();
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(255, 0, 0, 255));
	app.OnRender();
	renderer.Render();
	Display::Device()->EndScene();
	Display::Device()->Present();




	app.Destroy();
	delete pWindow;

#else	//samples section renderer framework
	
	g_pSceneRenderer = new Test::SceneRenderer();
	g_pScene = new Aftk::C_Scene();
	Aftk::C_Status status = g_pScene->Read("testassets/barrierskin.a3d");

	g_pSceneRenderer->m_DeviceParams.m_Width		= 800;
	g_pSceneRenderer->m_DeviceParams.m_Height		= 600;
	g_pSceneRenderer->m_DeviceParams.m_pName		= "aa";
	g_pSceneRenderer->m_DeviceParams.m_hInstance	= hInstance;
	g_pSceneRenderer->m_DeviceParams.m_CmdShow		= nCmdShow;
	g_pSceneRenderer->m_DeviceParams.m_Multisample	= Display::E_Multisample_2;

	if (g_pSceneRenderer->Create())
	{
		IDirect3DSwapChain9 *pChain;
		HRESULT hr = Display::Device()->m_pd3dDevice->GetSwapChain(0, &pChain);
		int kk=0;

		g_pSceneRenderer->Run();
	}
	else
		LOGFATAL(Display::g_Log, "Failed to create the sample\n");

	if (g_pSceneRenderer)
		delete g_pSceneRenderer;

	if (g_pScene)
		delete g_pScene;
	
#endif

	return 0;
}
