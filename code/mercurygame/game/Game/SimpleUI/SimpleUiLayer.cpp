#include "stdafx.h"

#include <Render/debug/DebugRender.h>

#include <Game/Game.h>
#include <Game/SimpleUi/SimpleUiLayer.h>

#include <framework/application/ApplicationBase.h>
#include <game/gamecontrols/GameControls.h>

#include <Game/Game.h>

#include <Game/GameFlow/GameflowMessages.h>
#include <Game/GameWorld/GameWorldLayer.h>
#include <Game/GameWorld/PostProcessLayer.h>
#include <GameApp.h>
#include <Framework/application/applicationbase.h>
#include <Render/debug/ImmediateDebugRender.h>

namespace Game
{

Debug::Bool g_CacheLevelResource("Game.Frontend.CacheLevelResource", false);
Debug::Bool g_CacheModelResource("Game.Frontend.CacheModelResource", false);

//////////////////////////////////////////////////////////////////////////
//	FileDialogController
//////////////////////////////////////////////////////////////////////////
class C_FileDialogController : public DebugSystems::I_FileDialogController
{
public:
	C_FileDialogController(Input::C_GameInput* pGameInput) : m_pGameInput(pGameInput) {};
	
	virtual const bool MoveUp( )	{ return m_pGameInput->GetButtonAutoRepeat( Framework::E_Button_MenuUp, 16, 4 ); }
	virtual const bool MoveDown( )	{ return m_pGameInput->GetButtonAutoRepeat( Framework::E_Button_MenuDown, 16, 4 ); }
	
	virtual const bool Confirm( )	{ return m_pGameInput->GetButtonPressedDebounced( Framework::E_Button_MenuConfirm ); }
	virtual const bool Cancel( )	{ return m_pGameInput->GetButtonPressedDebounced( Framework::E_Button_MenuCancel ); }

private:
	Input::C_GameInput*		m_pGameInput;
};


C_UiStyle::C_UiStyle()
	: m_pOverlayFont( NULL )
	, m_pTitleFont( NULL )
	, m_pItemFont( NULL )
	, m_pItemSelectedFont( NULL )

{
}

C_UiStyle::~C_UiStyle()
{
	SAFE_DELETE_PTR( m_pOverlayFont );
	SAFE_DELETE_PTR( m_pTitleFont );
	SAFE_DELETE_PTR( m_pItemFont );
	SAFE_DELETE_PTR( m_pItemSelectedFont );

	m_BackgroundTextureHandle = NULL;
}

void C_UiStyle::Create()
{
	Maths::RGBA white(0xff, 0xff, 0xff, 0xff);
	Maths::RGBA black(0x00, 0x00, 0x00, 0xff);
	Maths::RGBA bizarrePurple1(228, 224, 232, 0xff);
	Maths::RGBA bizarrePurple2(203, 194, 210, 0xff);
	Maths::RGBA bizarrePurple3(178, 167, 191, 0xff);
	Maths::RGBA bizarrePurple4(153, 140, 173, 0xff);
	Maths::RGBA bizarrePurple5(131, 117, 157, 0xff);
	Maths::RGBA bizarrePurple6(108, 95, 143, 0xff);
	Maths::RGBA bizarrePurple7(87, 74, 129, 0xff);
	Maths::RGBA bizarrePurple8(66, 56, 117, 0xff);
	Maths::RGBA bizarrePurple9(47, 39, 106, 0xff);
	Maths::RGBA bizarrePurple10(27, 24, 96, 0xff);

	Maths::RGBA paperWhite(0xa0, 0xa0, 0xa0, 0xff);

	Maths::RGBA shadowColour = bizarrePurple10;

	m_BackgroundColourA = bizarrePurple10;
	m_BackgroundColourB = paperWhite;

	m_SelectedColourA = Maths::RGBA(0xff, 0x80, 0x00, 0xff);
	m_SelectedColourB = bizarrePurple2;

	if (m_pOverlayFont == NULL)
	{
		//Create the font renderer
		m_pOverlayFont = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
		assert(m_pOverlayFont != NULL && "Failed to initialise FontRenderer");
		m_pOverlayFont->SetScaleX(32);
		m_pOverlayFont->SetScaleY(64);
		m_pOverlayFont->SetFeather( 2.0f );
		m_pOverlayFont->SetShadowEnable( true );
		m_pOverlayFont->SetShadowOffset( 0.05f );
	}

	if (m_pTitleFont == NULL)
	{
		//Create the font renderer
		m_pTitleFont = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
		m_pTitleFont->SetSpacingH( -0.05f );
		m_pTitleFont->SetScaleX( 48 );
		m_pTitleFont->SetScaleY( 96 );
		m_pTitleFont->SetFeather( 2.0f );
		m_pTitleFont->SetOutlineSize( 0.2f );
		m_pTitleFont->SetColourBottom( bizarrePurple1 );
		m_pTitleFont->SetColourTop( bizarrePurple2 );
		m_pTitleFont->SetColourOutline( bizarrePurple6 );
		m_pTitleFont->SetShadowEnable( true );
		m_pTitleFont->SetShadowOffset( 0.05f );
	}

	if (m_pItemFont == NULL)
	{
		//Create the font renderer
		m_pItemFont = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
		m_pItemFont->SetSpacingH( -0.05f );
		m_pItemFont->SetScaleX(24);
		m_pItemFont->SetScaleY(32);
		m_pItemFont->SetFeather( 2.0f );
		m_pItemFont->SetColourBottom( bizarrePurple2 );
		m_pItemFont->SetColourTop( bizarrePurple3 );
		m_pItemFont->SetColourOutline( bizarrePurple6 );
		m_pItemFont->SetShadowEnable( true );
		m_pItemFont->SetShadowOffset( 0.1f );
		m_pItemFont->SetOutlineSize( 0.3f );
	}
	
	if (m_pItemSelectedFont == NULL)
	{
		//Create the font renderer
		m_pItemSelectedFont = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
		m_pItemSelectedFont->SetSpacingH( -0.05f );
		m_pItemSelectedFont->SetScaleX(24);
		m_pItemSelectedFont->SetScaleY(32);
		m_pItemSelectedFont->SetFeather( 1.0f );
		m_pItemSelectedFont->SetColourBottom( bizarrePurple2 );
		m_pItemSelectedFont->SetColourTop( bizarrePurple3 );
		m_pItemSelectedFont->SetColourOutline( bizarrePurple6 );
		m_pItemSelectedFont->SetShadowEnable( true );
		m_pItemSelectedFont->SetShadowOffset( 0.1f );
		m_pItemSelectedFont->SetOutlineSize( 0.3f );
	}

	//Load frame texture
	m_BackgroundTextureHandle.Load( "dontship/textures/background.texture" );
	Resource::WaitForResourceData(m_BackgroundTextureHandle);
	TextureResource::C_TextureResource* frameTextureResource = m_BackgroundTextureHandle;
	m_pBackgoundTexture = (Display::C_Texture2D*)frameTextureResource->GetDisplayTexture();
}

void C_UiStyle::Update()
{
	F32 pulseSpeed = 8.0f;
	F32 blend = (1.0f + sinf( pulseSpeed * C_GameApp::GetInstance()->GetTimer().GetElapsedTime())) * 0.5f;
	Maths::C_RGBA_F32 colourDiff = (Maths::C_RGBA_F32)m_SelectedColourB - (Maths::C_RGBA_F32)m_SelectedColourA;
	Maths::RGBA colour = (Maths::C_RGBA_F32)m_SelectedColourA + colourDiff * blend;
	m_pItemSelectedFont->SetColourTop( colour );
	m_pItemSelectedFont->SetColourBottom( colour );
}


Debug::String g_SimpleUiLevelString("Game.Frontend.LevelName", ".\\levels\\testlevels\\testworld\\level.level", Debug::Persist());
Debug::String g_SimpleUiModelString("Game.Frontend.ModelName", ".\\models\\testmodels\\primitives\\box.model", Debug::Persist());
Debug::String g_SimpleUiActorsString("Game.Frontend.ActorsName", "", Debug::Persist());

C_FrontendFileDialog::C_FrontendFileDialog( const Util::C_String& name, const Util::C_String& extension, const Util::C_String& rootPath, Debug::String& debugString, const bool bClearOnCancel )
: m_pFileDialog( NULL )
, m_Name( name )
, m_Extension( extension )
, m_RootPath( rootPath )
, m_DebugString( debugString )
, m_bClearOnCancel( bClearOnCancel )
{
}

C_FrontendFileDialog::~C_FrontendFileDialog()
{
	SAFE_DELETE_PTR(m_pFileDialog);
}

void C_FrontendFileDialog::Activate( C_FileDialogController* pFileDialogController )
{
	if (m_pFileDialog == NULL)
	{
		//Create file dialog							
		DebugSystems::C_FileDialogSkin fileDialogSkin;
		m_pFileDialog = new DebugSystems::C_FileDialog(m_Name, m_Extension, m_RootPath, pFileDialogController, fileDialogSkin);
	}
	else
		m_pFileDialog->Active(true);

}

void C_FrontendFileDialog::Update()
{
	if (!IsActive())
		return;

	m_pFileDialog->Update(Framework::C_ApplicationBase::GetInstance()->GetTimer().GetFrameTime());

	//Check if new file is selected
	Util::C_String selectedLevel;
	if (m_pFileDialog->CheckState(selectedLevel))
	{
		m_DebugString = selectedLevel;
		m_pFileDialog->Active(false);
	}
	else if (!m_pFileDialog->Active() && m_bClearOnCancel)
	{
		m_DebugString = Util::C_String();
	}
}

bool C_FrontendFileDialog::IsActive()
{
	return (m_pFileDialog && m_pFileDialog->Active());
}

void C_FrontendFileDialog::Render()
{
	if (IsActive())
		m_pFileDialog->Render();
}

enum E_FrontendItem
{
	E_FrontendItem_LevelFilename = 0,
	E_FrontendItem_ModelFilename,
	E_FrontendItem_ActorsFilename,
	E_FrontendItem_Start,
	E_FrontendItem_Quit,
	E_FrontendItem_Count,
};

C_FrontendMenu::C_FrontendMenu( C_SimpleUiLayer* pSimpleUiLayer, C_UiStyle* pStyle )
	: m_pSimpleUiLayer( pSimpleUiLayer )
	, m_SelectedItem( E_FrontendItem_Start )
	, m_pStyle( pStyle )
	, m_pFileDialogController( NULL )
	, m_LevelFileDialog( "Select Level", "level", ".\\levels\\", g_SimpleUiLevelString )
	, m_ModelFileDialog( "Select Model", "model", ".\\models\\", g_SimpleUiModelString )
	, m_ActorsFileDialog( "Select Actors", "xt", ".\\image\\levels\\", g_SimpleUiActorsString, true )
{	
	m_pFileDialogController = new C_FileDialogController(Input::GetGameInputManager()->GetGameInput(0));
}

C_FrontendMenu::~C_FrontendMenu()
{
	SAFE_DELETE_PTR(m_pFileDialogController);
}

void C_FrontendMenu::Update()
{
	if(m_LevelFileDialog.IsActive())
	{
		m_LevelFileDialog.Update();
	}
	else
	if(m_ModelFileDialog.IsActive())
	{
		m_ModelFileDialog.Update();
	}
	else
	if(m_ActorsFileDialog.IsActive())
	{
		m_ActorsFileDialog.Update();
	}
	else
	{
		if( Input::GetGameInput(0)->GetButtonAutoRepeat( Framework::E_Button_MenuUp, 16, 4 ) )
		{
			m_SelectedItem--;
			if( m_SelectedItem < 0 )
				m_SelectedItem = 0;
		}

		if( Input::GetGameInput(0)->GetButtonAutoRepeat( Framework::E_Button_MenuDown, 16, 4 ) )
		{
			m_SelectedItem++;
			if( m_SelectedItem >= E_FrontendItem_Count )
				m_SelectedItem = E_FrontendItem_Count - 1;
		}

		if( Input::GetGameInput(0)->GetButtonPressedDebounced( Framework::E_Button_MenuConfirm ) )
		{
			switch(m_SelectedItem)
			{
				case E_FrontendItem_LevelFilename:
					{
						m_LevelFileDialog.Activate( m_pFileDialogController );
					}
				break;

				case E_FrontendItem_ModelFilename:
					{
						m_ModelFileDialog.Activate( m_pFileDialogController );
					}
				break;

				case E_FrontendItem_ActorsFilename:
					{
						m_ActorsFileDialog.Activate( m_pFileDialogController );
					}
				break;

				case E_FrontendItem_Start:
					{
						Game::C_LevelSettings& levelSettings = C_Game::GetInstance()->GetLevelSettings();
						levelSettings.m_LevelFilename = (const char*)g_SimpleUiLevelString;
						levelSettings.m_ActorsFilename = (const char*)g_SimpleUiActorsString;
					
						Messaging::C_MessageBase msg = Messaging::BuildSimpleMessage( E_GameflowMessage_StartGame );
						m_pSimpleUiLayer->GetSubscriberSystem().BroadcastMessage( &msg );
					}
				break;

				case E_FrontendItem_Quit:
					C_GameApp::GetInstance()->Quit();
				break;
			}
		}
	}
}

void C_FrontendMenu::Render()
{
	Render::Debug::C_ImmediateDebugRender* pImmediateDebugRender = Render::Debug::C_ImmediateDebugRender::GetInstance();
	pImmediateDebugRender->Draw2DTextureBar(Maths::HVec2( 0.0f, 0.0f ), Maths::HVec2( 1024.0f, 768.0f ), Maths::RGBA(1.f,1.f,1.f,1.f), *m_pStyle->m_pBackgoundTexture);

	//Render::Debug::Immediate()->Draw2dBar( Maths::HVec2( 0.0f, 0.0f ), Maths::HVec2( 1024.0f, 768.0f ), m_pStyle->m_BackgroundColourA);
	//Render::Debug::Immediate()->Draw2dBar( Maths::HVec2( 0.0f, 768.0f - 192.0f ), Maths::HVec2( 1024.0f, 192.0f ), m_pStyle->m_BackgroundColourB);

	m_pStyle->m_pTitleFont->Print( Maths::HVec2(512.0f, 64.0f), "MercuryGame", Render::Font::E_Justification_Centre );

	F32 yPos = 256.0f;

	for(S32 itemNo=0; itemNo<E_FrontendItem_Count; itemNo++)
	{
		Util::C_String itemText;
		switch(itemNo)
		{
			case E_FrontendItem_LevelFilename:
				itemText.Printf( "Level Filename: %s", (const char*)g_SimpleUiLevelString );
			break;

			case E_FrontendItem_ModelFilename:
				itemText.Printf( "Model Filename: %s", (const char*)g_SimpleUiModelString );
			break;

			case E_FrontendItem_ActorsFilename:
				itemText.Printf( "Actors Filename: %s", (const char*)g_SimpleUiActorsString );
			break;

			case E_FrontendItem_Start:
				itemText.Printf( "Start" );
			break;

			case E_FrontendItem_Quit:
				itemText.Printf( "Quit" );
			break;
		}

		Render::Font::C_FontRenderer* pFont = m_pStyle->m_pItemFont;

		if(itemNo == m_SelectedItem)
		{
			pFont = m_pStyle->m_pItemSelectedFont;
		}
		
		pFont->Print( Maths::HVec2(48.0f, yPos), itemText );		

		yPos += pFont->GetDimensions(0, itemText).Y();
	}

	m_LevelFileDialog.Render();
	m_ModelFileDialog.Render();
	m_ActorsFileDialog.Render();
}


C_SimpleUiLayer::C_SimpleUiLayer( Messaging::C_MessageSubscriberSystem* pSubscriberSystem )
	: m_State( E_UiState_Frontend )
	, m_FrontendMenu( this, &m_Style )
{
	SetSubscriberSystem( pSubscriberSystem );
	
	AddSubscriber( E_GameflowMessage_StartGame );
	AddSubscriber( E_GameflowMessage_QuitLevel );
}

C_SimpleUiLayer::~C_SimpleUiLayer()
{
}

HRESULT C_SimpleUiLayer::ProcessMessage(const Framework::E_LayerMessage message)
{
	switch( message )
	{
		case Framework::E_LayerMessage_Create:
			return Create();
		break;

		case Framework::E_LayerMessage_Destroy:
			return Destroy();
		break;

		case Framework::E_LayerMessage_Update:
			return Update();
		break;

		case Framework::E_LayerMessage_RenderFullscreen:
			return RenderFullscreen();
		break;
	}

	return S_OK;
}

HRESULT C_SimpleUiLayer::Create()
{
	m_Style.Create();
	return S_OK;
}

HRESULT C_SimpleUiLayer::Destroy()
{	
	return S_OK;
}

void C_SimpleUiLayer::UpdateCacheLevelFile()
{
	if(g_CacheLevelResource)
	{
		bool loadLevel = false;

		Util::C_String levelFilename(g_SimpleUiLevelString);

		GameResource::C_LevelResource* pLevelResource;

		pLevelResource = m_LevelHandle;

		if(pLevelResource == NULL)
		{
			loadLevel = true;
		}
		else
		{
			if(pLevelResource->GetResourceFilename() != levelFilename)
			{
				loadLevel = true;
			}
		}

		if(loadLevel)
		{
			m_LevelHandle.Load(levelFilename);
		}
	}

	if(g_CacheModelResource)
	{
		bool loadModel = false;

		Util::C_String modelFilename(g_SimpleUiModelString);

		GameResource::C_ModelResource* pModelResource;

		pModelResource = m_ModelHandle;

		if(pModelResource == NULL)
		{
			loadModel = true;
		}
		else
		{
			if(pModelResource->GetResourceFilename() != modelFilename)
			{
				loadModel = true;
			}
		}

		if(loadModel)
		{
			m_ModelHandle.Load(modelFilename);
		}
	}
}

HRESULT C_SimpleUiLayer::Update()
{
	if(C_GameApp::GetInstance()->IsPaused())
	{
		return S_OK;
	}

	m_Style.Update();

	switch( m_State )
	{
		case E_UiState_Frontend:
		{
			m_FrontendMenu.Update();
		}
		break;

		case E_UiState_Ingame:
		{
		}
		break;
	}

	UpdateCacheLevelFile();

	return S_OK;
}

bool C_SimpleUiLayer::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case E_GameflowMessage_StartGame:
		{
			m_State = E_UiState_Ingame;

			C_GameApp::GetInstance()->CreatePostProcessLayer();

			C_GameApp::GetInstance()->CreateGameWorldLayer();
		}
		break;

		case E_GameflowMessage_QuitLevel:
			m_State = E_UiState_Frontend;
		break;

		default:
			handledMessage = false;
		break;
	}

	return handledMessage;
}

HRESULT C_SimpleUiLayer::RenderFullscreen()
{	
	PERF_EVENT("UI.RenderFullscreen");
	switch( m_State )
	{
		case E_UiState_Frontend:
		{
			// const Maths::HVec2 &vPos, const char *text, E_Justification justification = E_Justification_Left, U32 nWrapWidth = 0
			m_FrontendMenu.Render();
		}
		break;

		case E_UiState_Ingame:
		{
			// const Maths::HVec2 &vPos, const char *text, E_Justification justification = E_Justification_Left, U32 nWrapWidth = 0
			m_Style.m_pOverlayFont->Print( Maths::HVec2(64.0f, 64.0f), "Ingame" );
		}
		break;
	}


	return S_OK;
}


} // namespace Game
