#include "stdafx.h"

//#include <Render/debug/DebugRender.h>
//
//#include <Game/Game.h>
//#include <Game/SimpleUi/SimpleUiLayer.h>
//
//#include <framework/application/ApplicationBase.h>
//#include <game/gamecontrols/GameControls.h>
//
//#include <Game/Game.h>
//
//#include <Game/Messaging/GameflowMessages.h>
//#include <Game/GameWorld/GameWorldLayer.h>
//#include <Game/GameWorld/PostProcessLayer.h>
//#include <GameApp.h>
//#include <Framework/application/applicationbase.h>
//#include <Render/debug/ImmediateDebugRender.h>
//
//namespace Game
//{
//
//Debug::Bool g_CacheLevelResource("MercuryGame.Frontend.CacheLevelResource", false);
//
////////////////////////////////////////////////////////////////////////////
////	FileDialogController
////////////////////////////////////////////////////////////////////////////
//class C_FileDialogController : public DebugSystems::I_FileDialogController
//{
//public:
//	C_FileDialogController(Input::C_GameInput* pGameInput) : m_pGameInput(pGameInput) {};
//	
//	virtual const bool MoveUp( )	{ return m_pGameInput->GetButtonAutoRepeat( Framework::E_Button_MenuUp, 16, 4 ); }
//	virtual const bool MoveDown( )	{ return m_pGameInput->GetButtonAutoRepeat( Framework::E_Button_MenuDown, 16, 4 ); }
//	
//	virtual const bool Confirm( )	{ return m_pGameInput->GetButtonPressedDebounced( Framework::E_Button_MenuConfirm ); }
//	virtual const bool Cancel( )	{ return m_pGameInput->GetButtonPressedDebounced( Framework::E_Button_MenuCancel ); }
//
//private:
//	Input::C_GameInput*		m_pGameInput;
//};
//
//
//C_UiStyle::C_UiStyle()
//	: m_pOverlayFont( NULL )
//	, m_pTitleFont( NULL )
//	, m_pItemFont( NULL )
//	, m_pItemSelectedFont( NULL )
//
//{
//}
//
//C_UiStyle::~C_UiStyle()
//{
//	SAFE_DELETE_PTR( m_pOverlayFont );
//	SAFE_DELETE_PTR( m_pTitleFont );
//	SAFE_DELETE_PTR( m_pItemFont );
//	SAFE_DELETE_PTR( m_pItemSelectedFont );
//
//	Display::Device()->DestroyTexture2D(&m_BackgoundTexture);
//}
//
//void C_UiStyle::Create()
//{
//	Maths::RGBA white(0xff, 0xff, 0xff, 0xff);
//	Maths::RGBA black(0x00, 0x00, 0x00, 0xff);
//	Maths::RGBA bizarrePurple1(228, 224, 232, 0xff);
//	Maths::RGBA bizarrePurple2(203, 194, 210, 0xff);
//	Maths::RGBA bizarrePurple3(178, 167, 191, 0xff);
//	Maths::RGBA bizarrePurple4(153, 140, 173, 0xff);
//	Maths::RGBA bizarrePurple5(131, 117, 157, 0xff);
//	Maths::RGBA bizarrePurple6(108, 95, 143, 0xff);
//	Maths::RGBA bizarrePurple7(87, 74, 129, 0xff);
//	Maths::RGBA bizarrePurple8(66, 56, 117, 0xff);
//	Maths::RGBA bizarrePurple9(47, 39, 106, 0xff);
//	Maths::RGBA bizarrePurple10(27, 24, 96, 0xff);
//
//	Maths::RGBA paperWhite(0xa0, 0xa0, 0xa0, 0xff);
//
//	Maths::RGBA shadowColour = bizarrePurple10;
//
//	m_BackgroundColourA = bizarrePurple10;
//	m_BackgroundColourB = paperWhite;
//
//	m_SelectedColourA = Maths::RGBA(0xff, 0x80, 0x00, 0xff);
//	m_SelectedColourB = bizarrePurple2;
//
//	if (m_pOverlayFont == NULL)
//	{
//		//Create the font renderer
//		m_pOverlayFont = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
//		m_pOverlayFont->SetScaleX(32);
//		m_pOverlayFont->SetScaleY(64);
//		m_pOverlayFont->SetFeather( 2.0f );
//		m_pOverlayFont->SetShadowEnable( true );
//		m_pOverlayFont->SetShadowOffset( 0.05f );
//	}
//
//	if (m_pTitleFont == NULL)
//	{
//		//Create the font renderer
//		m_pTitleFont = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
//		m_pTitleFont->SetSpacingH( -0.05f );
//		m_pTitleFont->SetScaleX( 48 );
//		m_pTitleFont->SetScaleY( 96 );
//		m_pTitleFont->SetFeather( 2.0f );
//		m_pTitleFont->SetOutlineSize( 0.2f );
//		m_pTitleFont->SetColourBottom( bizarrePurple1 );
//		m_pTitleFont->SetColourTop( bizarrePurple2 );
//		m_pTitleFont->SetColourOutline( bizarrePurple6 );
//		m_pTitleFont->SetShadowEnable( true );
//		m_pTitleFont->SetShadowOffset( 0.05f );
//	}
//
//	if (m_pItemFont == NULL)
//	{
//		//Create the font renderer
//		m_pItemFont = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
//		m_pItemFont->SetSpacingH( -0.05f );
//		m_pItemFont->SetScaleX(24);
//		m_pItemFont->SetScaleY(32);
//		m_pItemFont->SetFeather( 2.0f );
//		m_pItemFont->SetColourBottom( bizarrePurple2 );
//		m_pItemFont->SetColourTop( bizarrePurple3 );
//		m_pItemFont->SetColourOutline( bizarrePurple6 );
//		m_pItemFont->SetShadowEnable( true );
//		m_pItemFont->SetShadowOffset( 0.1f );
//		m_pItemFont->SetOutlineSize( 0.3f );
//	}
//	
//	if (m_pItemSelectedFont == NULL)
//	{
//		//Create the font renderer
//		m_pItemSelectedFont = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
//		m_pItemSelectedFont->SetSpacingH( -0.05f );
//		m_pItemSelectedFont->SetScaleX(24);
//		m_pItemSelectedFont->SetScaleY(32);
//		m_pItemSelectedFont->SetFeather( 1.0f );
//		m_pItemSelectedFont->SetColourBottom( bizarrePurple2 );
//		m_pItemSelectedFont->SetColourTop( bizarrePurple3 );
//		m_pItemSelectedFont->SetColourOutline( bizarrePurple6 );
//		m_pItemSelectedFont->SetShadowEnable( true );
//		m_pItemSelectedFont->SetShadowOffset( 0.1f );
//		m_pItemSelectedFont->SetOutlineSize( 0.3f );
//	}
//
//	//Load frame texture
//	CreateTexture2DFromFile("dontship/textures/background.dds", &m_BackgoundTexture);
//}
//
//void C_UiStyle::Update()
//{
//	F32 pulseSpeed = 8.0f;
//	F32 blend = (1.0f + sinf( pulseSpeed * C_GameApp::GetInstance()->GetTimer().GetElapsedTime())) * 0.5f;
//	Maths::C_RGBA_F32 colourDiff = (Maths::C_RGBA_F32)m_SelectedColourB - (Maths::C_RGBA_F32)m_SelectedColourA;
//	Maths::RGBA colour = (Maths::C_RGBA_F32)m_SelectedColourA + colourDiff * blend;
//	m_pItemSelectedFont->SetColourTop( colour );
//	m_pItemSelectedFont->SetColourBottom( colour );
//}
//
//Debug::String g_SimpleUiLevelString("MercuryGame.Frontend.LevelName", ".\\levels\\testlevels\\testworld\\mylevel.level");
//
//enum E_FrontendItem
//{
//	E_FrontendItem_LevelFilename = 0,
//	E_FrontendItem_Start,
//	E_FrontendItem_Quit,
//	E_FrontendItem_Count,
//};
//
//C_FrontendMenu::C_FrontendMenu( C_SimpleUiLayer* pSimpleUiLayer, C_UiStyle* pStyle )
//	: m_pSimpleUiLayer( pSimpleUiLayer )
//	, m_SelectedItem( E_FrontendItem_Start )
//	, m_pStyle( pStyle )
//	, m_pFileDialog( NULL)
//	, m_pFileDialogController( NULL )
//{	
//}
//
//C_FrontendMenu::~C_FrontendMenu()
//{
//	SAFE_DELETE_PTR(m_pFileDialog);
//	SAFE_DELETE_PTR(m_pFileDialogController);
//}
//
//void C_FrontendMenu::Update()
//{
//	if (m_pFileDialog && m_pFileDialog->Active())
//	{
//		//Update
//		m_pFileDialog->Update(Framework::C_ApplicationBase::GetInstance()->GetTimer().GetFrameTime());
//
//		//Check if new file is selected
//		Util::C_String selectedLevel;
//		if (m_pFileDialog->CheckState(selectedLevel))
//		{
//			g_SimpleUiLevelString = selectedLevel;
//		}
//	}
//	else
//	{
//
//		if( Input::GetGameInput(0)->GetButtonAutoRepeat( Framework::E_Button_MenuUp, 16, 4 ) )
//		{
//			m_SelectedItem--;
//			if( m_SelectedItem < 0 )
//				m_SelectedItem = 0;
//		}
//
//		if( Input::GetGameInput(0)->GetButtonAutoRepeat( Framework::E_Button_MenuDown, 16, 4 ) )
//		{
//			m_SelectedItem++;
//			if( m_SelectedItem >= E_FrontendItem_Count )
//				m_SelectedItem = E_FrontendItem_Count - 1;
//		}
//
//		if( Input::GetGameInput(0)->GetButtonPressedDebounced( Framework::E_Button_MenuConfirm ) )
//		{
//			switch(m_SelectedItem)
//			{
//				case E_FrontendItem_LevelFilename:
//					{
//						if (m_pFileDialog == NULL)
//						{
//							//Create file dialog
//							m_pFileDialogController = new C_FileDialogController(Input::GetGameInputManager()->GetGameInput(0));
//
//							DebugSystems::C_FileDialogSkin fileDialogSkin;
//							m_pFileDialog = new DebugSystems::C_FileDialog("Select level", "level",".\\levels\\",m_pFileDialogController, fileDialogSkin);
//						}
//						else
//							m_pFileDialog->Active(true);
//					}
//				break;
//
//				case E_FrontendItem_Start:
//					//C_Game::GetInstance()->GetLevelSettings().m_LevelFilename = (const char*)g_SimpleUiLevelString;
//					m_pSimpleUiLayer->GetSubscriberSystem().BroadcastMessage( E_GameflowMessage_StartGame );
//				break;
//
//				case E_FrontendItem_Quit:
//					C_GameApp::GetInstance()->Quit();
//				break;
//			}
//		}
//	}
//}
//
//void C_FrontendMenu::Render()
//{
//	Render::Debug::C_ImmediateDebugRender* pImmediateDebugRender = Render::Debug::C_ImmediateDebugRender::GetInstance();
//	pImmediateDebugRender->Draw2DTextureBar(Maths::HVec2( 0.0f, 0.0f ), Maths::HVec2( 1024.0f, 768.0f ), Maths::RGBA(1.f,1.f,1.f,1.f), m_pStyle->m_BackgoundTexture);
//
//	//Render::Debug::Immediate()->Draw2dBar( Maths::HVec2( 0.0f, 0.0f ), Maths::HVec2( 1024.0f, 768.0f ), m_pStyle->m_BackgroundColourA);
//	//Render::Debug::Immediate()->Draw2dBar( Maths::HVec2( 0.0f, 768.0f - 192.0f ), Maths::HVec2( 1024.0f, 192.0f ), m_pStyle->m_BackgroundColourB);
//
//	m_pStyle->m_pTitleFont->Print( Maths::HVec2(512.0f, 64.0f), "Amax '09", Render::Font::E_Justification_Centre );
//
//	F32 yPos = 256.0f;
//
//	for(S32 itemNo=0; itemNo<E_FrontendItem_Count; itemNo++)
//	{
//		Util::C_String itemText;
//		switch(itemNo)
//		{
//			case E_FrontendItem_LevelFilename:
//				itemText.Printf( "Level Filename: %s", (const char*)g_SimpleUiLevelString );
//			break;
//
//			case E_FrontendItem_Start:
//				itemText.Printf( "Start" );
//			break;
//
//			case E_FrontendItem_Quit:
//				itemText.Printf( "Quit" );
//			break;
//		}
//
//		Render::Font::C_FontRenderer* pFont = m_pStyle->m_pItemFont;
//
//		if(itemNo == m_SelectedItem)
//		{
//			pFont = m_pStyle->m_pItemSelectedFont;
//		}
//		
//		pFont->Print( Maths::HVec2(48.0f, yPos), itemText );		
//
//		yPos += pFont->GetDimensions(0, itemText).Y();
//	}
//
//	if (m_pFileDialog && m_pFileDialog->Active())
//		m_pFileDialog->Render();
//}
//
//
//C_SimpleUiLayer::C_SimpleUiLayer( Messaging::C_MessageSubscriberSystem* pSubscriberSystem )
//	: m_State( E_UiState_Frontend )
//	, m_FrontendMenu( this, &m_Style )
//{
//	SetSubscriberSystem( pSubscriberSystem );
//	
//	AddSubscriber( E_GameflowMessage_StartGame );
//	AddSubscriber( E_GameflowMessage_QuitLevel );
//}
//
//C_SimpleUiLayer::~C_SimpleUiLayer()
//{
//}
//
//HRESULT C_SimpleUiLayer::ProcessMessage(const Framework::E_LayerMessage message)
//{
//	switch( message )
//	{
//		case Framework::E_LayerMessage_Create:
//			return Create();
//		break;
//
//		case Framework::E_LayerMessage_Destroy:
//			return Destroy();
//		break;
//
//		case Framework::E_LayerMessage_Update:
//			return Update();
//		break;
//
//		case Framework::E_LayerMessage_RenderFullscreen:
//			return RenderFullscreen();
//		break;
//	}
//
//	return S_OK;
//}
//
//HRESULT C_SimpleUiLayer::Create()
//{
//	m_Style.Create();
//	return S_OK;
//}
//
//HRESULT C_SimpleUiLayer::Destroy()
//{	
//	return S_OK;
//}
//
//void C_SimpleUiLayer::UpdateCacheLevelFile()
//{
//	if(!g_CacheLevelResource)
//		return;
//
//	bool loadLevel = false;
//
//	Util::C_String levelFilename(g_SimpleUiLevelString);
//
//	GameResource::C_LevelResource* pLevelResource;
//
//	pLevelResource = m_LevelHandle;
//
//	if(pLevelResource == NULL)
//	{
//		loadLevel = true;
//	}
//	else
//	{
//		if(pLevelResource->GetResourceFilename() != levelFilename)
//		{
//			loadLevel = true;
//		}
//	}
//
//	if(loadLevel)
//	{
//		m_LevelHandle.Load(levelFilename);
//	}
//}
//
//HRESULT C_SimpleUiLayer::Update()
//{
//	m_Style.Update();
//
//	switch( m_State )
//	{
//		case E_UiState_Frontend:
//		{
//			m_FrontendMenu.Update();
//		}
//		break;
//
//		case E_UiState_Ingame:
//		{
//		}
//		break;
//	}
//
//	UpdateCacheLevelFile();
//
//	return S_OK;
//}
//
//bool C_SimpleUiLayer::MessageHandler(const  Messaging::C_MessageBase* pMessage)
//{
//	bool handledMessage = true;
//
//	switch( pMessage->Type() )
//	{
//		case E_GameflowMessage_StartGame:
//		{
//			m_State = E_UiState_Ingame;
//
//			C_GameApp::GetInstance()->CreatePostProcessLayer();
//
//			C_GameApp::GetInstance()->InsertLayer( new C_GameWorldLayer( &GetSubscriberSystem() ) );
//		}
//		break;
//
//		case E_GameflowMessage_QuitLevel:
//			m_State = E_UiState_Frontend;
//		break;
//
//		default:
//			handledMessage = false;
//		break;
//	}
//
//	return handledMessage;
//}
//
//HRESULT C_SimpleUiLayer::RenderFullscreen()
//{	
//	switch( m_State )
//	{
//		case E_UiState_Frontend:
//		{
//			// const Maths::HVec2 &vPos, const char *text, E_Justification justification = E_Justification_Left, U32 nWrapWidth = 0
//			m_FrontendMenu.Render();
//		}
//		break;
//
//		case E_UiState_Ingame:
//		{
//			// const Maths::HVec2 &vPos, const char *text, E_Justification justification = E_Justification_Left, U32 nWrapWidth = 0
//			m_Style.m_pOverlayFont->Print( Maths::HVec2(64.0f, 64.0f), "Ingame" );
//		}
//		break;
//	}
//
//
//	return S_OK;
//}
//
//
//} // namespace Game
