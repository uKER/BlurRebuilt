#ifndef DLC_CONTROLLER_H
#define DLC_CONTROLLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xt/xtgame.h>
#include <GameLogic\Profile\MultiplayerStatistics.h>
#include <GameLogic\RaceModes\RaceModeDefines.h>

namespace Game
{
	struct MultiplayerStatistics;
}

namespace UI
{
	class DLCController : public MT::Thread
	{
	public:
		void ThreadMain(void);

		void Enumerate();
		void LoadAndLicenseContent();
		void ClearDLC();

		bool HasErrors() { return ( mCorruptFiles.Size() != 0 ); }

		bool	IsGameDataObject(XtBase& base);
		bool	IdIsUnlocked(int id);
		float	GetFansMultiplier(Game::MultiplayerStatistics* pStatistics, Game::E_RaceMode raceMode);

		static DLCController& Instance(){return s_instance;};

	private:
		Btl::Set<U32>						mDLCPacksInstalled;
		Btl::Vector<XtPack*>				mPacksOpen;

		U8*									mpEnumeratedData;
		DWORD								mEnumeratedDataSizeRequired;
		HANDLE								mEnumerationHandle;

		Btl::Vector<Util::C_String128>		mFilesToLoad; // Massive, for safety.

		Btl::Vector<UI::C_WideString128>	mCorruptFiles;
		Btl::Vector<U32>					mCorruptLocIds;

		
		static DLCController s_instance;

	};
}

#endif // DLC_CONTROLLER_H