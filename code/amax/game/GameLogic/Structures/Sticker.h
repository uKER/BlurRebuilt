#ifndef STICKER_H
#define STICKER_H

#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <Game\Achievements\AchievementEnums.h>
#include <GameLogic/GameData/AchievementData.h>

namespace Game
{
	struct Achievement;

	class C_Sticker
	{
	public:
		C_Sticker()
		:	mDescription( 0 ),
			mShortDescription( 0 ),
			mProgress( 0 ),
			mGoal( 0 ),
			mType( BLUR_STICKER_INVALID ),
			mOneShot( false ),
			mQuadTopLeft( false ), mQuadTopRight( false ),
			mQuadBottomLeft( false ), mQuadBottomRight( false ),
			m_pAchievement( NULL )
		{
		}

		C_Sticker( StickerData& data, Achievement* a )
		:	mDescription( data.mDescription ),
			mShortDescription( data.mShortDescription ),
			mProgress( 0 ),
			mGoal( data.mGoal ),
			mType( data.mType ),
			mOneShot( data.mOneShot ),
			mQuadTopLeft( data.mQuadTopLeft ), mQuadTopRight( data.mQuadTopRight ),
			mQuadBottomLeft( data.mQuadBottomLeft ), mQuadBottomRight( data.mQuadBottomRight ),
			m_pAchievement( a )
		{
			//for the UI
			mUiDesc.Format( UI::C_TextStringID(mDescription.id).String(), mGoal );
		}

		void 								Serialise( Sys::BinarySerialiserAutoEndian& stream );
		void 								Deserialise( Sys::BinaryDeserialiserAutoEndian& stream );

		XtLocId Description() const { return mDescription; }
		XtLocId ShortDescription() const { return mShortDescription; }
		const UI::C_WideString128* GetUiDescription() const { return &mUiDesc; }
		F32 Progress() const { return mProgress; } //progress towards completion

		F32 Goal() const { return mGoal; } //goal for completion
		void Goal(F32 g) { mGoal = g; }

		BLUR_STICKER Type() const { return mType; } //sticker enum
		void Type(BLUR_STICKER t) { mType = t; }

		bool								InTopLeftQuad() const { return mQuadTopLeft; }
		bool								InTopRightQuad() const { return mQuadTopRight; }
		bool								InBottomRightQuad() const { return mQuadBottomRight; }
		bool								InBottomLeftQuad() const { return mQuadBottomLeft; }

		void								AddProgress(F32 progressToAdd = 1.f);
		void								SetProgress(F32 progress);
		bool								IsComplete() const { return mProgress >= mGoal; }
		bool								IsOneShot() const { return mOneShot; }
		void								Reset() { mProgress = 0; }

		Achievement* m_pAchievement; // pointer to the achievement containing us

	private:
		void								CheckProgress(float oldProgress);
		bool								CanAddStickerProgress(); // various things that may prevent us from giving progress

		XtLocId mDescription;
		XtLocId mShortDescription;
		UI::C_WideString128 mUiDesc;
		F32 mProgress;
		F32 mGoal;
		BLUR_STICKER mType;
		bool mOneShot; // if true, this sticker is either done or it isn't, no "halfway done" middle ground!
		bool mQuadTopLeft, mQuadTopRight, mQuadBottomLeft, mQuadBottomRight;

		//both of these may be set to true
		bool mCanCompleteInSinglePlayer;
		bool mCanCompleteInMultiplayer;
	};
};

#endif //STICKER_H
