#ifndef ACHIEVEMENTS_MANAGER_H
#define ACHIEVEMENTS_MANAGER_H

#include <misc_shared/Messaging/Messaging.h>
#include <misc_shared\Messaging\MessageMultiHandler.h>
#include <Game\Achievements\AchievementEnums.h>
#include <GameLogic/GameData/AchievementData.h>

#include <GameLogic\Layers\RaceLogicLayer.h>

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{
	class C_Sticker;

	struct Achievement
	{
		XtLocId mName;
		BLUR_ACHIEVEMENT mType;
		BLUR_ACHIEVEMENT_CATEGORY mCategory;
		U8 mTextureIndex;
		bool mSinglePlayer;
		bool mMultiplayer;
		bool mSplitScreen;
		U8 mStickerBookLocation;
		Btl::Vector<C_Sticker*> mStickers;
	};

	class C_AchievementsManager : public Messaging::C_MessageMultiHandler
	{
	public:
		C_AchievementsManager()
		{
		};

		void							Init();
		void							Update();
		void							LoadFromSorceryData();

		int								GetControllerIndex(int idx) const;
		bool							IsSignedIn() const;
		Achievement*					GetAchievement(BLUR_ACHIEVEMENT type);
		Achievement*					GetAchievementContainingSticker(BLUR_STICKER type);
		bool							IsAchievementComplete(BLUR_ACHIEVEMENT achievement);
		bool							IsAchievementComplete(Achievement* achievement);

		void							GiveAchievement(int user_idx, BLUR_ACHIEVEMENT achievement);

		Btl::Vector<Achievement*>		GetFilteredStickerLocations(bool singlePlayer);
		C_Sticker*						GetStickerInQuadrant(Achievement* a, int quadrant);
		U32								GetNumAchievements() const { return mAchievements.Size(); }

		void							StickerComplete(BLUR_STICKER sticker);
		void							StickerProgressed(Achievement* a, C_Sticker* sticker);

		static C_AchievementsManager& Instance(){return s_instance;};

		// For game messages
		bool MessageHandler(const Messaging::C_MessageBase* pMessage);

		// for any given location in the sticker book, we may index into 
		// this array and find the achievement that should live there
		Btl::Vector<Achievement*>	mStickerBookLocations;

	private:
		static C_AchievementsManager s_instance;

		Btl::Vector<Achievement>	mAchievements;

		typedef Btl::BitVectorTemplate<Btl::BitArrayBase<Btl::Vector<U8, Btl::VectorParameters_Array<32> > > > BitArray256;
	};

} // namespace Game

#endif // ACHIEVEMENTS_MANAGER_H