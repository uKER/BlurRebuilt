#include "stdafx.h"
#include "Render/Stats/StatsGame.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

Debug::Float	g_debugGameStatsDisplayPosX		("Graphics.Render.Stats.Display.PosX",		50.0f,		Debug::Range(0.0f, 1024.0f));
Debug::Float	g_debugGameStatsDisplayPosY		("Graphics.Render.Stats.Display.PosY",		50.0f,		Debug::Range(0.0f, 768.0f));
Debug::Float	g_debugGameStatsDisplayDimX		("Graphics.Render.Stats.Display.DimX",		20.0f,		Debug::Range(0.0f, 100.0f));
Debug::Float	g_debugGameStatsDisplayDimY		("Graphics.Render.Stats.Display.DimY",		20.0f,		Debug::Range(0.0f, 100.0f));
Debug::Int		g_debugGameStatsDisplayColR		("Graphics.Render.Stats.Display.ColR",		255,		Debug::Range(0, 255));
Debug::Int		g_debugGameStatsDisplayColG		("Graphics.Render.Stats.Display.ColG",		255,		Debug::Range(0, 255));
Debug::Int		g_debugGameStatsDisplayColB		("Graphics.Render.Stats.Display.ColB",		255,		Debug::Range(0, 255));
Debug::Int		g_debugGameStatsDisplayColA		("Graphics.Render.Stats.Display.ColA",		255,		Debug::Range(0, 255));
Debug::Int		g_debugGameStatsDisplayAlpha	("Graphics.Render.Stats.Display.Alpha",		0,			Debug::Range(0, 255));

namespace Render
{

C_StatGame::C_StatGame()
{
}

C_StatGame::~C_StatGame()
{
}

void C_StatGame::Reset()
{
}

void C_StatGame::DrawBody(Display::C_Float2& pos)
{
}

void C_StatGame::DumpBody()
{
}

Maths::RGBA C_StatGame::GetBackgroundCol()
{
	return Maths::RGBA(0, 0, 0, (S32)g_debugGameStatsDisplayAlpha);
}

Display::C_Float2 C_StatGame::GetFontPos()
{
	return Display::C_Float2((F32)g_debugGameStatsDisplayPosX, (F32)g_debugGameStatsDisplayPosY);
}

Display::C_Float2 C_StatGame::GetFontDim()
{
	return Display::C_Float2((F32)g_debugGameStatsDisplayDimX, (F32)g_debugGameStatsDisplayDimY);
}

Maths::RGBA C_StatGame::GetFontCol()
{
	return Maths::RGBA((S32)g_debugGameStatsDisplayColR, (S32)g_debugGameStatsDisplayColG, (S32)g_debugGameStatsDisplayColB, (S32)g_debugGameStatsDisplayColA);
}

void C_StatGame::DrawHead(Display::C_Float2& pos, const char* pText)
{
	const Maths::RGBA col1 = GetBackgroundCol(); 
	Display::BasicRender()->DrawColoured2DRect(0.0f, 0.0f, 1024.f, 768.f, col1);

	const Maths::RGBA col2 = GetFontCol();
	char buffer[512];
	sprintf_s(buffer, sizeof(buffer), ":: %s Stats ::", pText);
	Display::FontRender()->Draw(buffer, pos.x, pos.y, 25.0f, 25.0f, col2);
	pos.y += 35.0f;
}

void C_StatGame::DrawItem(Display::C_Float2& pos, const char* pText, ...)
{
	char buffer[512];
	va_list arglist;
	va_start(arglist, pText);
	vsprintf_s(buffer, sizeof(buffer), pText, arglist);
	va_end(arglist);

	const Display::C_Float2& dim = GetFontDim();
	const Maths::RGBA col = GetFontCol();
	Display::FontRender()->Draw(buffer, pos.x, pos.y, dim.x, dim.y, col);
	pos.y += dim.y + 5.0f;
}

C_StatGame* C_StatsGame::m_pStat = NULL;

C_StatsGame::C_StatsGame()
{
}

C_StatsGame::~C_StatsGame()
{
}

}; // namespace Render
