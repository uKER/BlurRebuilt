#ifndef SIMPLE_UI_LAYER_H
#define SIMPLE_UI_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

//#include <Render/font/Font.h>
//#include <Framework/Layer/Layer.h>
//#include <misc_shared/Messaging/Messaging.h>
//#include <misc_shared/UID/FourCC.h>
//
//#include <game_resource/LevelResource.h>
//#include <DebugSystems/FileDialog.h>
//#include <display/texture/Texture2D.h>
//
//namespace Game
//{
//
//enum E_UiState
//{
//	E_UiState_Frontend,
//	E_UiState_Ingame
//};
//
//class C_SimpleUiLayer;
//class C_FileDialogController;
//
//class C_UiStyle
//{
//public:
//	C_UiStyle();
//	~C_UiStyle();
//	
//	void Create();
//	void Update();
//
//public:
//	Render::Font::C_FontRenderer* m_pOverlayFont;
//	Render::Font::C_FontRenderer* m_pTitleFont;
//	Render::Font::C_FontRenderer* m_pItemFont;
//	Render::Font::C_FontRenderer* m_pItemSelectedFont;
//
//	Maths::RGBA m_BackgroundColourA;
//	Maths::RGBA m_BackgroundColourB;
//
//	Maths::RGBA m_SelectedColourA;
//	Maths::RGBA m_SelectedColourB;
//
//	Display::C_Texture2D m_BackgoundTexture;
//};
//
//class C_FrontendMenu
//{
//friend class C_SimpleUiLayer;
//public:
//	C_FrontendMenu( C_SimpleUiLayer* pSimpleUiLayer, C_UiStyle* pStyle );
//	~C_FrontendMenu();
//
//	void Update();
//	void Render();
//
//private:		
//	C_UiStyle* m_pStyle;
//
//	C_SimpleUiLayer* m_pSimpleUiLayer;
//
//	S32 m_SelectedItem;
//
//	C_FileDialogController* m_pFileDialogController;
//	DebugSystems::C_FileDialog*	m_pFileDialog;
//};
//
//class C_SimpleUiLayer : public Framework::C_Layer, public Messaging::C_MessageHandler
//{
//public:
//
//	C_SimpleUiLayer( Messaging::C_MessageSubscriberSystem* pSubscriberSystem );
//	virtual ~C_SimpleUiLayer();
//
//	// For layer messages
//	HRESULT ProcessMessage(const Framework::E_LayerMessage message);
//
//	HRESULT Create();
//	HRESULT Destroy();
//	HRESULT Update();
//	HRESULT RenderFullscreen();
//
//	// For game messages
//	bool MessageHandler(const Messaging::C_MessageBase* pMessage);
//
//private:
//	void UpdateCacheLevelFile();
//
//private:
//	E_UiState m_State;
//
//	C_UiStyle m_Style;
//	C_FrontendMenu m_FrontendMenu;
//
//	GameResource::C_LevelHandle m_LevelHandle;
//};
//
//} // namespace Game

#endif // GAME_LAYER_H
