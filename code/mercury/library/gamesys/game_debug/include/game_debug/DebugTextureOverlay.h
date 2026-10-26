#ifndef GAMESYS_GAME_DEBUG_DEBUG_TEXTURE_OVERLAY_H
#define GAMESYS_GAME_DEBUG_DEBUG_TEXTURE_OVERLAY_H

#ifdef _MSC_VER
#pragma once
#endif

namespace GameDebug
{

class C_DebugTextureOverlay // : Display::I_Renderer
{
public:
	C_DebugTextureOverlay();
	~C_DebugTextureOverlay();

	void Init();
	void Update();
	void Render2d();
	void Close();

private:
	void LoadTexture( const Util::C_String& filename );
	void FreeTexture();

private:
	Util::C_String m_TextureFilename;
	Display::C_Texture2D m_Texture2D;
};


}; // GameDebug

#endif // GAMESYS_GAME_DEBUG_DEBUG_TEXTURE_OVERLAY_H
