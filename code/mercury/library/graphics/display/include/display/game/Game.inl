#ifndef DISPLAY_GAME_GAME_INL
#define DISPLAY_GAME_GAME_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

#if 0

INLINE const S32 C_Game::GetViewCount() const
{
	return m_ViewList.Size();
}

INLINE C_GameView* C_Game::GetView(const S32 viewIndex)
{
	assert(viewIndex >= 0 && viewIndex < m_ViewList.Size());
	return m_ViewList[viewIndex];
}

INLINE const U32 C_Game::GetMemorySize() const
{
	return m_MemSize;
}

INLINE U8* C_Game::GetMemoryAddress()
{
	return m_MemPtr;
}

INLINE const U32 C_Game::GetResolutionWidth() const
{
	return m_ResWidth;
}

INLINE const U32 C_Game::GetResolutionHeight() const
{
	return m_ResHeight;
}

INLINE const E_Multisample C_Game::GetResolutionMultisample() const
{
	return m_ResMultisample;
}

#endif

} // namespace Display

#endif // DISPLAY_GAME_GAME_H
