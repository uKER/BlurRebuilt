#include "Stdafx.h"
#include <Game/GameWorld/GameWorld.h>	// [Blur2PC]

#include "UiPhotoMode.h"
#include "UiHelpers.h"

#include "Graphics/PostFxMatrix.h"
#include "Graphics/PostFxSettings.h"
#include "Game/AmaxCam/CameraSystem.h"
#include "Game/AmaxCam/PhotoCamera2.h"

#ifndef CFG_GOLD
Debug::Bool 	g_UiPhotoModeOverride( "UI.PhotoMode.Override", false );
Debug::Float 	g_UiPhotoModeBrightness( "UI.PhotoMode.Brightness", 0.0f, Debug::Range( -1.0f, 1.0f ) );
Debug::Float 	g_UiPhotoModeContrast( "UI.PhotoMode.Contrast", 0.0f, Debug::Range( -1.0f, 1.0f ) );
Debug::Float 	g_UiPhotoModeSaturation( "UI.PhotoMode.Saturation", 0.0f, Debug::Range( -1.0f, 1.0f ) );
Debug::Colour	g_UiPhotoModeHintColour( "UI.PhotoMode.HintColour", 0.0f, 0.0f, 0.0f, 0.0f );
Debug::Float	g_UiPhotoModeHintAmount( "UI.PhotoMode.HintAmount", 0.0f, Debug::Range( 0.0f, 1.0f ) );
#endif

namespace AmaxCam
{
	extern Debug::Float g_debugPhotoCamIdepth;
	extern Debug::Float g_debugPhotoCamAperture;
	extern Debug::Float g_debugPhotoCamShutterAngle;
}

namespace UI
{

// -------------
// C_UiPhotoMode
// -------------
C_UiPhotoMode C_UiPhotoMode::m_sInstance;

C_UiPhotoMode::C_UiPhotoMode()
	: m_fBrightness( 0.0f )
	, m_fContrast( 0.0f )
	, m_fSaturation( 0.0f )
	, m_HintColour()
	, m_fHintAmount( 0.0f )
	, m_fShutterAngle( 0.5f )
	, m_fIDepth( 0.1f )
	, m_fAperture( 0.0f )
{
	//
	m_PrevIdepth = m_PrevAperture = m_PrevShutterAngle = -1.f;
}

void C_UiPhotoMode::UpdateFilters()
{
	if ( !AmaxCam::CameraSystem().IsPhotoCamera() )
		return;

	Game::C_GameWorld* pGameWorld = GetGameWorld();

	if ( !pGameWorld )
		return;

	FX::I_EnvironmentSettings* pEnvironmentSettings = pGameWorld->GetEnvironmentSettings( 0 );	// no splitscreen, yeah?

	if ( !pEnvironmentSettings )
		return;

	Game::PostFxEnvSettings* pPostFXEnvSettings = (Game::PostFxEnvSettings*)pEnvironmentSettings->GetSettingsBlockByName( "PostFX" );

	if ( !pPostFXEnvSettings )
		return;

#ifndef CFG_GOLD
	if ( g_UiPhotoModeOverride )
	{
		m_fBrightness = g_UiPhotoModeBrightness;
		m_fContrast = g_UiPhotoModeContrast;
		m_fSaturation = g_UiPhotoModeSaturation;
		m_HintColour.Set01( 1.0f, g_UiPhotoModeHintColour->r, g_UiPhotoModeHintColour->g, g_UiPhotoModeHintColour->b );
		m_fHintAmount = g_UiPhotoModeHintAmount;
	}
#endif

	// build a colour matrix
	Maths::Mat44 colourMatrix = Maths::Const<Maths::Mat44>::Identity();

	// brightness
	Game::PostFxMatrixBrightness brightness;
	brightness.m_fBrightness = m_fBrightness;
	colourMatrix = colourMatrix * brightness.Matrix();

	// contrast
	Game::PostFxMatrixContrast contrast;
	contrast.m_fContrast = m_fContrast;
	colourMatrix = colourMatrix * contrast.Matrix();

	// saturation
	Game::PostFxMatrixSaturation saturation;
	saturation.m_fMaster = m_fSaturation;
	colourMatrix = colourMatrix * saturation.Matrix();

	// hint
	Game::PostFxMatrixHint hint;
	hint.m_fAmount = m_fHintAmount;
	hint.m_vColour.r = m_HintColour.R01();
	hint.m_vColour.g = m_HintColour.G01();
	hint.m_vColour.b = m_HintColour.B01();
	colourMatrix = colourMatrix * hint.Matrix();

	// apply the matrix
	pPostFXEnvSettings->m_ColourMatrix = pPostFXEnvSettings->m_ColourMatrix * colourMatrix;

	// set camera effects
	AmaxCam::C_PhotoCamera2* camera = AmaxCam::CameraSystem().PhotoCamera2();
	if (!camera)
		return;

	if (m_PrevAperture != m_fAperture)
		camera->SetAperture(m_fAperture);
	m_PrevAperture = m_fAperture;
	
	if (m_PrevIdepth != m_fIDepth)
		camera->SetIdepth(m_fIDepth);
	m_PrevIdepth = m_fIDepth;
	
	if (m_fShutterAngle != m_PrevShutterAngle)
		camera->SetShutterAngle(m_fShutterAngle);
	m_PrevShutterAngle = m_fShutterAngle;
}

}; // namespace UI
