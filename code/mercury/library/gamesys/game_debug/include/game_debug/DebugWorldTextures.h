#ifndef GAMESYS_GAME_DEBUG_DEBUG_WORLD_TEXTURES_H
#define GAMESYS_GAME_DEBUG_DEBUG_WORLD_TEXTURES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace GameDebug
{

const U32 c_DebugTextureBufferSize = 3;

class C_DebugWorldTextures : Display::I_Renderer
{

public:

	C_DebugWorldTextures();
	~C_DebugWorldTextures();

	bool	OnCreate();
	bool	OnCreateVolatile();
	void	OnDestroy();
	void	OnDestroyVolatile();

	void	Update();

	void	SetAlbedoGamma(const bool srgb);
	void	SetNormalGamma(const bool srgb);
	void	SetSpecularGamma(const bool srgb);
	void	SetEmissiveGamma(const bool srgb);

private:

	Display::C_Texture2D*	GetAlbedoTexture();
	Display::C_Texture2D*	GetNormalTexture();
	Display::C_Texture2D*	GetSpecularTexture();
	Display::C_Texture2D*	GetEmissiveTexture();
	
	bool					UpdateSample(const U32 colour, Display::C_Texture2D* pTexture);
	bool					UpdateTexture(const Util::C_String& fileName, Display::C_Texture2D* pTexture, const bool srgb);

	void					SetSampleGamma();

private:

	bool					m_AlbedoGamma;
	bool					m_NormalGamma;
	bool					m_SpecularGamma;
	bool					m_EmissiveGamma;

	Display::C_Texture2D	m_AlbedoSample[c_DebugTextureBufferSize];
	Display::C_Texture2D	m_NormalSample[c_DebugTextureBufferSize];
	Display::C_Texture2D	m_SpecularSample[c_DebugTextureBufferSize];
	Display::C_Texture2D	m_EmissiveSample[c_DebugTextureBufferSize];

	U32						m_AlbedoIndex;
	U32						m_NormalIndex;
	U32						m_SpecularIndex;
	U32						m_EmissiveIndex;

	Display::C_Texture2D	m_AlbedoTexture;
	Display::C_Texture2D	m_NormalTexture;
	Display::C_Texture2D	m_SpecularTexture;
	Display::C_Texture2D	m_EmissiveTexture;

	Util::C_String			m_AlbedoName;
	Util::C_String			m_NormalName;
	Util::C_String			m_SpecularName;
	Util::C_String			m_EmissiveName;

};

}; // GameDebug

#endif // GAMESYS_GAME_DEBUG_DEBUG_WORLD_TEXTURES_H
