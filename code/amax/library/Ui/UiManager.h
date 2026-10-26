#ifndef UIMANAGER_H
#define UIMANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Input/GameInput.h>

#include <core\containers\String.h>

#include <Ui\UIDefines.h>
#include <Ui\UiInterface.h>

#include <ui/Game/Gadgets/UIFriendsList.h>

#include <misc_shared/Messaging/MessageMultiHandler.h>
#include <Network/Router/NetRouterDefines.h>

#include <GameLogic/GameData/GameVehicleData.h>

namespace Cam2
{
	class C_Camera;
	class I_Feed;
}

namespace Game
{
class C_GameWorld;
}

namespace Advertising
{
	class MessageController;
}

namespace UI
{

class C_MovieAlternateTexture;

enum E_ManagerMsg
{
	E_ManagerMsg_None = 0,
	E_ManagerMsg_Start_MainGameSection,
	E_ManagerMsg_Start_TestUiSection,
	E_ManagerMsg_RestartSection,
	E_ManagerMsg_Count
};

enum E_ManagerSection
{
	E_ManagerSection_None = 0,
	E_ManagerSection_TestUi,
	E_ManagerSection_MainGame,
};

enum E_GameSpritePanel
{
	E_GameSpritePanel_2D = 0,				// 0 : 
	E_GameSpritePanel_2DAA,					// 1 : 
	E_GameSpritePanel_3DAA_WORLD,			// 2 : 
	E_GameSpritePanel_3DAA_LIGHT,			// 3 : 
	E_GameSpritePanel_2DBG,					// 4 : 
	E_GameSpritePanel_3DAA_0,				// 5 : 
	E_GameSpritePanel_3DAA_1,				// 6 : Async load / save camera
	E_GameSpritePanel_3D,					// 7 : 
	E_GameSpritePanel_3D_WORLD,				// 8 : 
	E_GameSpritePanel_VP0,					// 9 :
	E_GameSpritePanel_VP1,					// 10 :
	E_GameSpritePanel_VP2,					// 11 :
	E_GameSpritePanel_VP3,					// 12 :
	E_GameSpritePanel_2DIG_A,				// 13 : Render ingame AA before world / cars / skywrap
	E_GameSpritePanel_2DBGAA,				// 14 : 
	E_GameSpritePanel_DUMMY1,				// 15 : for saving
	E_GameSpritePanel_2DIG_B,				// 16 : Render ingame AA after the world / cars / skywrap
	E_GameSpritePanel_RT0,					// 17 : VP0 Map
	E_GameSpritePanel_RT1,					// 18 : VP1 Map
	E_GameSpritePanel_RT2,					// 19 : VP2 Map
	E_GameSpritePanel_RT3,					// 20 : VP3 Map
#if PLATFORM_PC
	E_GameSpritePanel_VP0_CAR,				// 21 : [Blur2PC] VP0 sprites attached to the player car (perks); built per update, moved to the car's render pose per frame
#endif

	E_GameSpritePanel_Count,
};

enum 
{
	E_GameSpriteType_LightPaint = E_SpriteType_Count,
	E_GameSpriteType_Count
};

class C_UiInterface : public I_UiInterface
{
public:
	C_UiInterface( )												{  }
	virtual ~C_UiInterface()										{  }

	const bool			IsWideScreen();
	void				PlaySound( const S32 sIndex, const F32 fValue1 = 0.0f, const F32 fValue2 = 0.0f );
	Sys::I_FileSystem*	GetFileSys();
	void				RegisterLibs( lua_State *pState );
	const bool			GetMousePos( F32& x, F32& y )				{ return false; }
	const bool			GetLeftMouseDown()							{ return false; }
	const bool			GetMouseScrollUp()							{ return false; }
	const bool			GetMouseScrollDown()						{ return false; }

	const bool			UseMouseInput( )							{ return false; }
	void				UseMouseInput( const bool bUseMouseInput )	{  }

	const S32			GetSoundIndex( const E_BaseSoundEffect eEffect );
	const bool			ExternalRender( const C_Sprite* sprite );
	const U32			PushMessageLuaParams(lua_State* pState, const Messaging::C_MessageBase *pMessage );

	Display::ResourceBundler::I_ResourceBundle*		CreateResourceBundle();

	void				RenameMovieFor50Hz( const char* pOriginalFileName, char* pNewFileName, const S32 sBufferSize )			{  }
	void				RenameMovieForSmallNTSC( const char* pOriginalFileName, char* pNewFileName, const S32 sBufferSize )		{  }

	void				PollPlatformCallbacks();
};


// -----------
// C_UiManager
// -----------
class C_MirrorAlternateTexture;
class C_PhotoModeAlternateTexture;
class C_AlternateTexture;
class C_MirrorGlassAlternateTexture;
#ifdef PLATFORM_PC
class C_ControllerGlyphTexture; // [Blur2PC] T256
#endif

class C_UiManager : public Messaging::C_MessageMultiHandler, public C_UIFriendsInterface
{
public:
	typedef enum E_Context
	{
		E_Context_Main = 0,
		E_Context_Popup,
		E_Context_LoadSave,
		E_Context_Friends,
		E_Context_Blurb,
		E_Context_CarouselApp,
		E_Context_Subscreen0,
		E_Context_Subscreen1,
		E_Context_Subscreen2,

		E_Context_Count
	};

	typedef enum E_RwCarouselBranch
	{
		E_RwCarouselBranch_Races = 0,
		E_RwCarouselBranch_Garage ,
		E_RwCarouselBranch_Quit,
		E_RwCarouselBranch_Help,
		E_RwCarouselBranch_Photos,
		E_RwCarouselBranch_Blurb,
		E_RwCarouselBranch_Options,
		E_RwCarouselBranch_Stickers,
		E_RwCarouselBranch_Leaderboards,
		E_RwCarouselBranch_Count,
	};

	typedef enum E_RwMpCarouselBranch
	{
		E_RwMpCarouselBranch_Online = 0,
		E_RwMpCarouselBranch_Splitscreen,
		E_RwMpCarouselBranch_Options,
		E_RwMpCarouselBranch_Lan,
		E_RwMpCarouselBranch_RaceHistory,
		E_RwMpCarouselBranch_Help,
		E_RwMpCarouselBranch_Stickers,
		E_RwMpCarouselBranch_Quit,
	};

	typedef enum E_RwGameCarouselBranch
	{
		E_RwGameCarouselBranch_Paused,
		E_RwGameCarouselBranch_Retry,
		E_RwGameCarouselBranch_Options,
		E_RwGameCarouselBranch_Help,
		E_RwGameCarouselBranch_Stats,
		E_RwGameCarouselBranch_Goals,
		E_RwGameCarouselBranch_PhotoMode,
		E_RwGameCarouselBranch_Quit,
		E_RwGameCarouselBranch_ReturnToLobby,
	};

	enum E_LaunchMode
	{
		E_LaunchMode_Master,		// Master version
		E_LaunchMode_Final,			// Final version (master debug)
		E_LaunchMode_Debug,			// DevMode - Go straight to debug sreen
		E_LaunchMode_DebugRaceBook,	// DevMode - Don't go to first race but run RaceBook
		E_LaunchMode_Demo,			// LIVE + PSN + Coverdisk demos (TBC)
		E_LaunchMode_Automation,	// Lloyds GPU hang debug test thing - leave alone
	};

public:
	C_UiManager( const U32 initParam );
	~C_UiManager( );

	void								PostXtInit();

	void								Process( const Maths::C_TimeS& time );
	void								RenderNonAA();
	void								PreRender();
	void								RenderAA();
	void								RenderRT( const U32 viewportIndex );
	void								Render2DBG();
	void								RenderIngameAA( const bool postWorldRender );

	void								LoadFrontendTextureBundle( const bool initialLoad = false );
	void								LoadGameTextureBundle();
	void								DeleteTextureBundle( const bool all );

	void								LoadingTextureDestroy( const U32 index );
							
	void								ProfileNetStatusChanged()								{ m_ProfilesNetStatusChanged = true; }
									
	void								ActOnProfileChanged( bool onOff )						{ m_ActOnProfilesChanged = onOff; }
	void								ProfilesChanged( const S32 sPadNo )						{ m_ProfilesChanged = true; m_sProfileChangedPadNo = sPadNo; }
	void								AllowProfileChange(const bool allowChange)				{ m_AllowProfileChange = allowChange; }

	void								SetGameWorld( Game::C_GameWorld* gw );
	void								SetRenderBuffer( Display::C_Texture2D* texture );
	void								SetMiniMapRenderTarget( Display::C_Texture2D* texture, const U32 localPlayerIndex );
	void								SetIgRenderTarget( Display::C_Texture2D* texture );

	void								LoadingTextureClone( Display::C_Texture* src, const U32 index );

	// friends list interface
	bool								CanInvite( const U32 uIndex );
	bool								CanJoinSessionInProgress( const U32 uIndex );

	void 								SendInvite( const U32 uIndex );
	void 								JoinSessionInProgress( const U32 uIndex );

	void								BuildPresenceString( const U32 uIndex, C_WideString64& string );
									
	bool								MessageHandler( const Messaging::C_MessageBase* pMessage );

	void								SetSchedulerGroup( int iGroup );
	void								DeferredUpdate();

	//! [Blur2PC] retail Blur PC (T146): UISystem.SystemQuitGame asked to close the game (C_GameApp::OnUpdate).
	bool								Blur2PCQuitGameRequested() const						{ return m_Blur2PCQuitGame; }
	void								LoadFrontendShapes();
	void								FreeFrontendShapes();
	void								FreeCommonShapes();

private:
	void								ProcessController();
	void								CreateGameLuaEnums( lua_State *pState );
	void								CreateGameGlobals( lua_State *pState );

#ifdef PLATFORM_PS3
	void								HandleControllerDisconnects( F32 fFrameTime );
#endif

private:
	void								RefreshLocalNetPlayerName();

private:
	E_ManagerSection					m_Section;
	E_LaunchMode						m_InitParam;
	C_UiInterface						m_UiInterface;
	const char*							m_pCurrentRouteName;
	bool								m_FrontendShapesLoaded;
	bool								m_UnlockAll;
	bool								m_AllowProfileChange;
	bool								m_bAllowAllPadInput;
	bool								m_ActOnProfilesChanged;
	bool								m_ProfilesChanged;
	S32									m_sProfileChangedPadNo;
	bool								m_ProfilesNetStatusChanged;
	bool								m_AcceptedGameInvite;
	S32									m_sInvitePadNo;
	Util::C_String64					m_UiColourStyle;	
	C_MirrorAlternateTexture*			m_pMirrorAlternateTexture;
	C_PhotoModeAlternateTexture*		m_pPhotoModeAlternateTexture;
	C_AlternateTexture*					m_pRenderBufferAlternateTexture;
	C_AlternateTexture*					m_pIgRenderBufferAlternateTexture;
	C_MirrorGlassAlternateTexture*		m_pMirrorGlassAlternateTexture;
#ifdef PLATFORM_PC
	C_ControllerGlyphTexture* m_pControllerGlyphTexture; // [Blur2PC] T256
#endif
	Display::C_Texture					m_MiniMapTexture;

	Advertising::MessageController*		m_pAdvertisingMessageController;

	static const U32					m_NumLoadingTextures = 2;
	static const U32					m_MaxLocalPlayers = 4;
	static const U32					m_MovieSlots = 10;

	Btl::Vector< C_MovieAlternateTexture*,	Btl::VectorParameters_Array< m_MovieSlots > >			m_pMovieAlternateTexture;
	Btl::Vector< C_AlternateTexture*,		Btl::VectorParameters_Array< m_MaxLocalPlayers > >		m_pMiniMapRenderTargetAlternateTexture;
	Btl::Vector< C_AlternateTexture*,		Btl::VectorParameters_Array< m_NumLoadingTextures > >	m_pLoadingAlternateTexture;
	Btl::Vector< Display::C_Texture,		Btl::VectorParameters_Array< m_NumLoadingTextures > >	m_LoadingTexture;

	// Depth of field settings
	Cam2::C_Camera*						m_WorldCamera;
	Btl::Vector< U8, Btl::VectorParameters_Array< 4	> > m_MaskXTime;
	Btl::Vector< U8, Btl::VectorParameters_Array< 4	> > m_MaskYTime;

	C_TextureManager::C_ResourceIndex	m_Ps3InstallBundle;
	C_TextureManager::C_ResourceIndex	m_SwitchBundle;
	int									m_SchedulerGroup;
	bool								m_Blur2PCQuitGame;	// [Blur2PC] retail Blur PC: C_UiManager + 0x124 (T146)

	Btl::Vector< F32, Btl::VectorParameters_Array< 8 > >	m_AudioEq;

	bool								m_bNormalNextButtonMapping;

	F32									m_fInviteShowTime;
	F32									m_fInviteTimer;

	S32									m_sActiveControllerDisconnected;
	F32									m_fDisconnectShowTime;
	F32									m_fDisconnectTimer;

	Maths::HMat44						m_TransformRT0;
	bool								m_PlatformConnectionState; // last known state
	AmaxCarClass						m_eCarClassSP;

#ifdef PLATFORM_PS3
	void RenderBlackRectangle();
	bool m_renderBlackRectangle;
	bool m_systemMenuState;
	bool m_systemOverlayState;
#endif

};

} // namespace UI

#ifndef CFG_GOLD
extern Debug::Enum<UI::C_UiManager::E_LaunchMode>	g_UiDebugLaunchMode;
#endif

#endif
