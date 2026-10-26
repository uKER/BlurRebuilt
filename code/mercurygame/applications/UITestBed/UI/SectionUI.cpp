#include "StdAfx.h"
#include "SectionUI.h"

#include "UiManager.h"

#include <SystemProfile/SystemProfile.h>
#include <SystemProfile/SystemProfileManager.h>

#include <framework/application/ApplicationBase.h>
#include <Framework/Application/UserControls.h>

#ifdef PLATFORM_PS3
#include <cell/sysmodule.h>
#include <np.h>
#endif

#include <Network/Core.h>

#if defined(PLATFORM_XENON) || defined(PLATFORM_PC)
#include <input\Pad360.h>
#elif defined(PLATFORM_PS3)
#include <input\PadPS3.h>
#else
#error( "***UNKNOWN BUILD" )
#endif

Debug::Bool g_UiDebugRenderAsAA( "UI.RenderAsAA", false, Debug::Persist() );

C_SectionUI::C_SectionUI( const Util::C_String& name ) 
	: Framework::C_TestSection( name )
{
#ifdef PLATFORM_PS3
	cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
	cellSysmoduleLoadModule(CELL_SYSMODULE_NETCTL);
	cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL);
	cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP2);
	cellSysmoduleLoadModule(CELL_SYSMODULE_L10N);
#endif

#ifdef CFG_GOLD
	NetPlay::C_Core::Instance().Initialise(true, false);
#else
	NetPlay::C_Core::Instance().Initialise(true, true);
#endif

	if ( !NetPlay::C_Core::Instance().IsInitialised() )
		NetPlay::C_Core::Instance().Initialise( true, true );

	if ( NetPlay::C_Core::Instance().IsInitialised() )
		SysProfile::C_Manager::Instance().Initialise();

	m_pUiManager = new UI::C_UiManager( 0 );
	assert( m_pUiManager );

	m_pUiManager->CreateTextureBundle( false );
}

C_SectionUI::~C_SectionUI()
{
	SysProfile::C_Manager::Instance().Shutdown();

	m_pUiManager->DeleteTextureBundle( true );

	SAFE_DELETE_PTR( m_pUiManager );
}

void C_SectionUI::OnUpdate()
{
	if ( Framework::C_ApplicationBase::GetInstance()->IsPaused( Framework::E_LayerPause_DebugMenu ) )
		return;

	// ***
	// this would be in the game somewhere done by someone else
	SysProfile::Notifications notifications;
	SysProfile::GameInvite invite;

	SysProfile::C_Manager::Instance().Poll( invite, notifications );

	// TODO - profile/net/invite changes UI messages etc ...
	// ***

	Maths::C_TimeS frameTime = Framework::C_ApplicationBase::GetInstance()->GetTimer().GetFrameTime();

	m_pUiManager->Process( frameTime );
}

void C_SectionUI::OnPreRender()
{
	m_pUiManager->PreRender();
}

void C_SectionUI::OnRender3DViewWithAA( const S32 viewIndex )
{
	// note this is only here because nothing clears the tiles otherwise, if the game is to be rendered then remove me.
	Display::Device()->Clear( Display::E_Clear_Colour | Display::E_Clear_Depth | Display::E_Clear_Stencil );

	if ( g_UiDebugRenderAsAA )
		m_pUiManager->Render();
}

void C_SectionUI::OnRender2DView()
{
	if ( !g_UiDebugRenderAsAA )
		m_pUiManager->Render();
}

void C_SectionUI::OnSetupUserControls( const S32 viewIndex )
{
	U32 userCount = 4;
#ifdef PLATFORM_PC
	userCount = 1;
#endif
	Framework::C_ApplicationBase::GetInstance()->SetInputUserCount( userCount );	
	Framework::C_ApplicationBase::GetInstance()->SetInputButtonCount( E_UiButton_Count );
	Framework::C_ApplicationBase::GetInstance()->SetInputAxisCount( E_UiAxis_Count );

	Input::GetGameInputManager()->Resize( userCount, E_UiButton_Count, E_UiAxis_Count );
	
	for ( U32 i = 0; i < userCount; i++ )
	{
		Framework::SetupApplicationUserControls( 
			Input::GetGameInput( i ), 
			Framework::C_ApplicationBase::GetInstance()->GetKeyboard( i ), 
			Framework::C_ApplicationBase::GetInstance()->GetMouse( i ), 
			Framework::C_ApplicationBase::GetInstance()->GetPad( i )
		);

		SetupUiControls( Input::GetGameInput( i ), Framework::C_ApplicationBase::GetInstance()->GetPad( i ) );
	}
}

#if defined(PLATFORM_XENON) || defined(PLATFORM_PC)
struct C_PadOrAxisSetup
{
	E_UiButton					mapping;
	Input::C_Pad360::E_Button	pad;
	Input::C_Pad360::E_Axis		axis;
	bool						invert;
};

static const C_PadOrAxisSetup kPadOrAxisSetup[] = 
{
	{	E_UiButton_PadOrAxis_Up,		Input::C_Pad360::E_Button_DPadUp,		Input::C_Pad360::E_Axis_LeftStick_Y,	false,	},
	{	E_UiButton_PadOrAxis_Down,		Input::C_Pad360::E_Button_DPadDown,		Input::C_Pad360::E_Axis_LeftStick_Y,	true,	},
	{	E_UiButton_PadOrAxis_Left,		Input::C_Pad360::E_Button_DPadLeft,		Input::C_Pad360::E_Axis_LeftStick_X,	true,	},
	{	E_UiButton_PadOrAxis_Right,		Input::C_Pad360::E_Button_DPadRight,	Input::C_Pad360::E_Axis_LeftStick_X,	false,	},
};

void SetupUiControls( Input::C_GameInput* pGameInput, Input::C_Device* pPad )
{
	// buttons
	pGameInput->SetButtonInput( E_UiButton_Pad_Up,			new Input::C_ButtonInputFromDevice( pPad,	Input::C_Pad360::E_Button_DPadUp ) );
	pGameInput->SetButtonInput( E_UiButton_Pad_Down,		new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_DPadDown ) );
	pGameInput->SetButtonInput( E_UiButton_Pad_Left,		new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_DPadLeft ) );
	pGameInput->SetButtonInput( E_UiButton_Pad_Right,		new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_DPadRight ) );
	pGameInput->SetButtonInput( E_UiButton_Confirm,			new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_A ) );
	pGameInput->SetButtonInput( E_UiButton_Cancel,			new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_B ) );
	pGameInput->SetButtonInput( E_UiButton_Option1,			new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_Y ) );
	pGameInput->SetButtonInput( E_UiButton_Option2,			new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_X ) );
	pGameInput->SetButtonInput( E_UiButton_LeftTrigger,		new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_LeftTrigger ) );
	pGameInput->SetButtonInput( E_UiButton_RightTrigger,	new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_RightTrigger ) );
	pGameInput->SetButtonInput( E_UiButton_LeftShoulder,	new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_LeftShoulder ) );
	pGameInput->SetButtonInput( E_UiButton_RightShoulder,	new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_RightShoulder ) );
	pGameInput->SetButtonInput( E_UiButton_LeftThumb,		new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_LeftThumb ) );
	pGameInput->SetButtonInput( E_UiButton_RightThumb,		new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_RightThumb ) );
	pGameInput->SetButtonInput( E_UiButton_Start,			new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_Start ) );
	pGameInput->SetButtonInput( E_UiButton_Back,			new Input::C_ButtonInputFromDevice( pPad, 	Input::C_Pad360::E_Button_Back ) );

	// axis
	pGameInput->SetAxisInput(	E_UiAxis_LeftStick_UD,		new Input::C_AxisInputFromDevice(	pPad,	Input::C_Pad360::E_Axis_LeftStick_Y ) );
	pGameInput->SetAxisInput(	E_UiAxis_LeftStick_LR,		new Input::C_AxisInputFromDevice(	pPad,	Input::C_Pad360::E_Axis_LeftStick_X ) );
	pGameInput->SetAxisInput(	E_UiAxis_RightStick_UD,		new Input::C_AxisInputFromDevice(	pPad,	Input::C_Pad360::E_Axis_RightStick_Y ) );
	pGameInput->SetAxisInput(	E_UiAxis_RightStick_LR,		new Input::C_AxisInputFromDevice(	pPad,	Input::C_Pad360::E_Axis_RightStick_X ) );
	// axis combiners

	for ( S32 i = 0; i < lengthof( kPadOrAxisSetup ); i++ )
	{
		const C_PadOrAxisSetup& setup = kPadOrAxisSetup[ i ];

		Input::C_ButtonInputCombiner* pButton = new Input::C_ButtonInputCombiner();
		pButton->AddButtonInput( new Input::C_ButtonInputFromDevice( pPad, setup.pad ) );
		pButton->AddButtonInput( new Input::C_ButtonInputFromAxis( new Input::C_AxisInputFromDevice( pPad, setup.axis ), setup.invert, 0.5f ) );

		pGameInput->SetButtonInput( setup.mapping, pButton );
	}
}
#endif

#ifdef PLATFORM_PS3
struct C_PadOrAxisSetup
{
	E_UiButton					mapping;
	Input::C_PadPS3::E_Button	pad;
	Input::C_PadPS3::E_Axis		axis;
	bool						invert;
};

static const C_PadOrAxisSetup kPadOrAxisSetup[] = 
{
	{	E_UiButton_PadOrAxis_Up,		Input::C_PadPS3::E_Button_DPadUp,		Input::C_PadPS3::E_Axis_LeftStick_Y,	false,	},
	{	E_UiButton_PadOrAxis_Down,		Input::C_PadPS3::E_Button_DPadDown,		Input::C_PadPS3::E_Axis_LeftStick_Y,	true,	},
	{	E_UiButton_PadOrAxis_Left,		Input::C_PadPS3::E_Button_DPadLeft,		Input::C_PadPS3::E_Axis_LeftStick_X,	true,	},
	{	E_UiButton_PadOrAxis_Right,		Input::C_PadPS3::E_Button_DPadRight,	Input::C_PadPS3::E_Axis_LeftStick_X,	false,	},
};

void SetupUiControls( Input::C_GameInput* pGameInput, Input::C_Device* pPad )
{
	// buttons
	pGameInput->SetButtonInput( E_UiButton_Pad_Up,			new Input::C_ButtonInputFromDevice( pPad,	Input::C_PadPS3::E_Button_DPadUp ) );
	pGameInput->SetButtonInput( E_UiButton_Pad_Down,		new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_DPadDown ) );
	pGameInput->SetButtonInput( E_UiButton_Pad_Left,		new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_DPadLeft ) );
	pGameInput->SetButtonInput( E_UiButton_Pad_Right,		new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_DPadRight ) );
	pGameInput->SetButtonInput( E_UiButton_Confirm,			new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_Cross ) );
	pGameInput->SetButtonInput( E_UiButton_Cancel,			new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_Circle ) );
	pGameInput->SetButtonInput( E_UiButton_Option1,			new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_Triangle ) );
	pGameInput->SetButtonInput( E_UiButton_Option2,			new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_Square ) );
	pGameInput->SetButtonInput( E_UiButton_LeftTrigger,		new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_L2 ) );
	pGameInput->SetButtonInput( E_UiButton_RightTrigger,	new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_R2 ) );
	pGameInput->SetButtonInput( E_UiButton_LeftShoulder,	new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_L1 ) );
	pGameInput->SetButtonInput( E_UiButton_RightShoulder,	new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_R1 ) );
	pGameInput->SetButtonInput( E_UiButton_LeftThumb,		new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_L3 ) );
	pGameInput->SetButtonInput( E_UiButton_RightThumb,		new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_R3 ) );
	pGameInput->SetButtonInput( E_UiButton_Start,			new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_Start ) );
	pGameInput->SetButtonInput( E_UiButton_Back,			new Input::C_ButtonInputFromDevice( pPad, 	Input::C_PadPS3::E_Button_Select ) );

	// axis
	pGameInput->SetAxisInput(	E_UiAxis_LeftStick_UD,		new Input::C_AxisInputFromDevice(	pPad,	Input::C_PadPS3::E_Axis_LeftStick_Y ) );
	pGameInput->SetAxisInput(	E_UiAxis_LeftStick_LR,		new Input::C_AxisInputFromDevice(	pPad,	Input::C_PadPS3::E_Axis_LeftStick_X ) );
	pGameInput->SetAxisInput(	E_UiAxis_RightStick_UD,		new Input::C_AxisInputFromDevice(	pPad,	Input::C_PadPS3::E_Axis_RightStick_Y ) );
	pGameInput->SetAxisInput(	E_UiAxis_RightStick_LR,		new Input::C_AxisInputFromDevice(	pPad,	Input::C_PadPS3::E_Axis_RightStick_X ) );
	// axis combiners

	for ( S32 i = 0; i < lengthof( kPadOrAxisSetup ); i++ )
	{
		const C_PadOrAxisSetup& setup = kPadOrAxisSetup[ i ];

		Input::C_ButtonInputCombiner* pButton = new Input::C_ButtonInputCombiner();
		pButton->AddButtonInput( new Input::C_ButtonInputFromDevice( pPad, setup.pad ) );
		pButton->AddButtonInput( new Input::C_ButtonInputFromAxis( new Input::C_AxisInputFromDevice( pPad, setup.axis ), setup.invert, 0.5f ) );

		pGameInput->SetButtonInput( setup.mapping, pButton );
	}
}
#endif
