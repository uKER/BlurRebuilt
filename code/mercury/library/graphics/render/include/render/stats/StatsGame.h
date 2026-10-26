#ifndef RENDER_STATS_STATS_GAME_H
#define RENDER_STATS_STATS_GAME_H

#ifdef _MSC_VER
#pragma once
#endif

#ifndef CFG_GOLD
#define TRACK_GAME_STATS
#endif

#ifdef TRACK_GAME_STATS
#define TRACK_VISIBLE_STATS
#endif

namespace Render
{

//! Game stat.

class C_StatGame
{
public:

	C_StatGame();
	~C_StatGame();

	void Reset();

protected:

	void DrawBody(Display::C_Float2& pos);
	void DumpBody();

	static Maths::RGBA GetBackgroundCol();
	static Display::C_Float2 GetFontPos();
	static Display::C_Float2 GetFontDim();
	static Maths::RGBA GetFontCol();

	static void DrawHead(Display::C_Float2& pos, const char* pText);
	static void DrawItem(Display::C_Float2& pos, const char* pText, ...);
};

//! Game stats.

class C_StatsGame
{
public:

	C_StatsGame();
	~C_StatsGame();

	static C_StatGame* GetStat();
	static void SetStat(C_StatGame* pStat);

private:

	static C_StatGame* m_pStat;
};

INLINE C_StatGame* C_StatsGame::GetStat()
{
	return m_pStat;
}

INLINE void C_StatsGame::SetStat(C_StatGame* pStat)
{
	m_pStat = pStat;
}

}; // namespace Render

#endif // RENDER_STATS_STATS_GAME_H
