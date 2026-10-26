#include "GameDebugPch.h"
#include <game_debug/DebugSamplerStates.h>
#include <fx/SamplerStates.h>

enum E_DebugFilterMode
{
	E_DebugFilterMode_None,
	E_DebugFilterMode_Point,
	E_DebugFilterMode_Linear,
	E_DebugFilterMode_Anisotropic
};

DEBUG_ENUM(E_DebugFilterMode)
	DE_OPTION(E_DebugFilterMode_None,			"None")
	DE_OPTION(E_DebugFilterMode_Point,			"Point")
	DE_OPTION(E_DebugFilterMode_Linear,			"Linear")
	DE_OPTION(E_DebugFilterMode_Anisotropic,	"Anisotropic")
END_DEBUG_ENUM

namespace GameDebug
{

Debug::Bool		g_debugSSOverride						("Graphics.Debug.WorldSamplerStates.OverrideSamplerStates",		false);

Debug::Bool		g_debugSSOverrideAlbedo					("Graphics.Debug.WorldSamplerStates.OverrideAlbedo",			false);
Debug::Int		g_debugSSAlbedoLodBias					("Graphics.Debug.WorldSamplerStates.AlbedoLodBias",			0, Debug::Range(-127, 127) );
Debug::Enum<E_DebugFilterMode> g_debugSSAlbedoFilterMode("Graphics.Debug.WorldSamplerStates.AlbedoFilterMode",		E_DebugFilterMode_None);
Debug::Int		g_debugSSAlbedoMaxAniso					("Graphics.Debug.WorldSamplerStates.AlbedoMaxAniso",		0, Debug::Range(0, 16) );

Debug::Bool		g_debugSSOverrideNormal					("Graphics.Debug.WorldSamplerStates.OverrideTangentNormal",	false);
Debug::Int		g_debugSSTangentNormalLodBias			("Graphics.Debug.WorldSamplerStates.TangentNormalLodBias",	0, Debug::Range(-127, 127) );
Debug::Enum<E_DebugFilterMode> g_debugSSTangentNormalFilterMode("Graphics.Debug.WorldSamplerStates.TangentNormalFilterMode",		E_DebugFilterMode_None);
Debug::Int		g_debugSSTangentNormalMaxAniso					("Graphics.Debug.WorldSamplerStates.TangentNormalMaxAniso",			0, Debug::Range(0, 16) );

Debug::Bool		g_debugSSOverrideSpecular				("Graphics.Debug.WorldSamplerStates.OverrideSpecular",		false);
Debug::Int		g_debugSSSpecularLodBias				("Graphics.Debug.WorldSamplerStates.SpecularLodBias",		0, Debug::Range(-127, 127) );
Debug::Enum<E_DebugFilterMode> g_debugSSSpecularFilterMode("Graphics.Debug.WorldSamplerStates.SpecularFilterMode",		E_DebugFilterMode_None);
Debug::Int		g_debugSSSpecularMaxAniso					("Graphics.Debug.WorldSamplerStates.SpecularMaxAniso",			0, Debug::Range(0, 16) );

Debug::Bool		g_debugSSOverrideEmissive				("Graphics.Debug.WorldSamplerStates.OverrideEmissive",		false);
Debug::Int		g_debugSSEmissiveLodBias				("Graphics.Debug.WorldSamplerStates.EmissiveLodBias",		0, Debug::Range(-127, 127) );
Debug::Enum<E_DebugFilterMode> g_debugSSEmissiveFilterMode("Graphics.Debug.WorldSamplerStates.EmissiveFilterMode",		E_DebugFilterMode_None);
Debug::Int		g_debugSSEmissiveMaxAniso					("Graphics.Debug.WorldSamplerStates.EmissiveMaxAniso",			0, Debug::Range(0, 16) );

Debug::Bool		g_debugSSOverrideAllWithAlbedo			("Graphics.Debug.WorldSamplerStates.OverrideAllWithAlbedo",		false);

Display::E_TextureFilter GetTextureFilter( E_DebugFilterMode debugFilterMode )
{
	switch( debugFilterMode )
	{
		default:
		case E_DebugFilterMode_None: return Display::E_TextureFilter_None; break;
		case E_DebugFilterMode_Point: return Display::E_TextureFilter_Point; break;
		case E_DebugFilterMode_Linear: return Display::E_TextureFilter_Linear; break;
		case E_DebugFilterMode_Anisotropic: return Display::E_TextureFilter_Anisotropic; break;
	}

	return Display::E_TextureFilter_None;
}

C_DebugSamplerStates::C_DebugSamplerStates()
{
}

C_DebugSamplerStates::~C_DebugSamplerStates()
{	
}

void C_DebugSamplerStates::Update()
{
	if (!g_debugSSOverride)
	{
		FX::g_DebugSamplerStates.bEnable = false;
		FX::g_DebugSamplerStates.m_Sampler[0].bEnable = false;
		FX::g_DebugSamplerStates.m_Sampler[1].bEnable = false;
		FX::g_DebugSamplerStates.m_Sampler[2].bEnable = false;
		FX::g_DebugSamplerStates.m_Sampler[3].bEnable = false;
		return;
	}

	FX::g_DebugSamplerStates.bEnable = true;
	FX::g_DebugSamplerStates.bOverrideAllWithAlbedo = g_debugSSOverrideAllWithAlbedo;

	if( g_debugSSOverrideAlbedo )
	{
		FX::DebugSamplerStates::DebugSamplerState& debugSamplerState = FX::g_DebugSamplerStates.m_Sampler[0];
		debugSamplerState.bEnable = true;
		debugSamplerState.m_MipLodBias = g_debugSSAlbedoLodBias;
		debugSamplerState.m_FilterMode = GetTextureFilter( g_debugSSAlbedoFilterMode );
		debugSamplerState.m_MaxAniso = g_debugSSAlbedoMaxAniso;
	}

	if( g_debugSSOverrideNormal )
	{
		FX::DebugSamplerStates::DebugSamplerState& debugSamplerState = FX::g_DebugSamplerStates.m_Sampler[1];
		debugSamplerState.bEnable = true;
		debugSamplerState.m_MipLodBias = g_debugSSTangentNormalLodBias;
		debugSamplerState.m_FilterMode = GetTextureFilter( g_debugSSTangentNormalFilterMode );
		debugSamplerState.m_MaxAniso = g_debugSSTangentNormalMaxAniso;
	}

	if( g_debugSSOverrideSpecular )
	{
		FX::DebugSamplerStates::DebugSamplerState& debugSamplerState = FX::g_DebugSamplerStates.m_Sampler[2];
		debugSamplerState.bEnable = true;
		debugSamplerState.m_MipLodBias = g_debugSSSpecularLodBias;
		debugSamplerState.m_FilterMode = GetTextureFilter( g_debugSSSpecularFilterMode );
		debugSamplerState.m_MaxAniso = g_debugSSSpecularMaxAniso;
	}

	if( g_debugSSOverrideEmissive )
	{
		FX::DebugSamplerStates::DebugSamplerState& debugSamplerState = FX::g_DebugSamplerStates.m_Sampler[3];
		debugSamplerState.bEnable = true;
		debugSamplerState.m_MipLodBias = g_debugSSEmissiveLodBias;
		debugSamplerState.m_FilterMode = GetTextureFilter( g_debugSSEmissiveFilterMode );
		debugSamplerState.m_MaxAniso = g_debugSSEmissiveMaxAniso;
	}
}

}; // namespace GameDebug
