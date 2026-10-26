#ifndef GAME_GAMEDATA_H
#define GAME_GAMEDATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtGame.h>

namespace Game
{

class C_GameData
{
public:
	C_GameData();
	~C_GameData();
	void Init(XtDb* db, const Util::C_String& filename);
	void Update();
	void Close();
	XtPack *GetMainPack();

	static C_GameData* GetInstance();

private:

	Util::C_String m_Filename;
	XtPack*	m_pXtMainPack;
	XtDb *m_pXtDb;
	static C_GameData *m_pGameData;
};

} // namespace Game

#endif // GAME_GAMEDATA_H
