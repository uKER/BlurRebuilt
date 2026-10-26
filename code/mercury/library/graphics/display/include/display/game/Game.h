#ifndef DISPLAY_GAME_GAME_H
#define DISPLAY_GAME_GAME_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Surface/Surface.h"
#include "Display/Texture/RenderTexture2D.h"

namespace Display
{

#if 0


//! Game view.

class C_GameView
{
public:

	C_GameView(const U32 width, const U32 height, const E_Multisample multisample);
	~C_GameView();

	bool Create(U32& curMemSize, U8*& curMemPtr);
	void Destroy();

public:

	struct C_Buffer
	{
		C_Surface					m_ColourSurface;
		C_Surface					m_DepthStencilSurface;
	};

	C_Buffer						m_WithAA;
	C_Buffer						m_WithoutAA;
	U32								m_Width;
	U32								m_Height;
	E_Multisample					m_Multisample;
	E_Format						m_ColourFormat;
	C_RenderTexture2D				m_ColourRenderTexture;
	E_Format						m_DepthStencilFormat;
	C_RenderTexture2D				m_DepthStencilRenderTexture;
};

//! Game.

class C_Game
{
public:

	C_Game();
	~C_Game();

	// View management
	void ClearViews();
	const S32 GetViewCount() const;
	C_GameView* GetView(const S32 viewIndex);
	S32 AddView(const U32 width, const U32 height, const E_Multisample multisample);
	bool CreateViews();
	void DestroyViews();

	// Memory management
	const U32 GetMemorySize() const;
	U8* GetMemoryAddress();
	bool CreateMemory();
	void DestroyMemory();

	// Resolution management
	const U32 GetResolutionWidth() const;
	const U32 GetResolutionHeight() const;
	const E_Multisample GetResolutionMultisample() const;
	bool CheckForResolutionChange(const bool disableMultisample = false);

private:

	// View management
	Ut::Vector<C_GameView*>		m_ViewList;

	// Memory management
	S32							m_MemID;
	U8*							m_MemPtr;
	U32							m_MemSize;
	U32							m_MemFlags;

	// Resolution management
	S32							m_ResMode;
	U32							m_ResWidth;
	U32							m_ResHeight;
	E_Multisample				m_ResMultisample;
};

extern C_Game* g_pGame;

void Game(C_Game* pGame);

INLINE C_Game* Game()
{
	assert(g_pGame);
	return g_pGame;
}

INLINE bool GameExists()
{
	return g_pGame != NULL;
}

#endif

} // namespace Display

#include "Display/Game/Game.inl"

#endif // DISPLAY_GAME_GAME_H
