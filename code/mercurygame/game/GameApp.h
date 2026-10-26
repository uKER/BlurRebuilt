#ifndef GAMEAPP_H
#define GAMEAPP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtGame.h>
#include <Framework/Application/Application.h>

namespace Game
{
	class C_Game;
	class C_GameLayer;
	class C_SimpleUiLayer;
	class C_PostProcessLayer;
	class C_SystemLayer;
	class C_GameWorldLayer;
};

namespace Framework
{
	class C_DebugLayer;
};

namespace PhotoMode
{
	class C_AppPhotoCapture;
}

//! C_GameApp

class C_GameApp : public Framework::C_Application
{
public:

	C_GameApp();
	~C_GameApp();

	static C_GameApp* GetInstance();

	HRESULT OnInitialise();
	void OnShutdown();

	void OnUpdate();
	void OnRender();
	void RenderNow();

	Game::C_Game* GetGame();

	void CreatePostProcessLayer();
	void DestroyPostProcessLayer();
	
	void CreateGameWorldLayer();
	void DestroyGameWorldLayer();

	// inherited from C_GameAppBase
	void SetupUserControls();
	Game::C_PostProcessLayer* PostProcessLayer() { return m_pPostProcessLayer; }
	Game::C_GameWorldLayer* GetGameWorldLayer() { return m_pGameWorldLayer; }

	void CreateXtDatabase();
	void CloseXtDatabase();
	XtDb* GetXtDatabase();

	PhotoMode::C_AppPhotoCapture* PhotoCapture() { return m_PhotoCapture; }

private:
	Game::C_Game* m_pGame;
	Game::C_SimpleUiLayer* m_pSimpleUiLayer;
	Framework::C_DebugLayer* m_pDebugLayer;

	Game::C_PostProcessLayer* m_pPostProcessLayer;
	Game::C_SystemLayer* m_pSystemLayer;
	Game::C_GameWorldLayer* m_pGameWorldLayer;

	XtCallbackContext* m_pXtCC;
	XtDb* m_pXtDatabase;
	PhotoMode::C_AppPhotoCapture* m_PhotoCapture;

};

#endif // GAMEAPP_H
