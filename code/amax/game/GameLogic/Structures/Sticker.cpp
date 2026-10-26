#include "stdafx.h"
#include <GameLogic/SinglePlayer/SP_BossDemands.h>	// [Blur2PC]

#include "Sticker.h"
#include <GameLogic\AchievementsManager.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <Ui\UiLayer.h>


namespace Game
{
	void C_Sticker::Serialise( Sys::BinarySerialiserAutoEndian& stream )
	{
		stream << mProgress;
	}

	void C_Sticker::Deserialise( Sys::BinaryDeserialiserAutoEndian& stream )
	{
		stream >> mProgress;
	}

	void C_Sticker::AddProgress(F32 progressToAdd)
	{
		if(mOneShot)
			SetProgress(mGoal);
		else
			SetProgress(mProgress + progressToAdd);
	}

	void C_Sticker::SetProgress(F32 progress)
	{
		if(!CanAddStickerProgress())
			return;

		float oldProgress = mProgress;

		mProgress = progress;

		CheckProgress(oldProgress);
	}

	void C_Sticker::CheckProgress(float oldProgress)
	{
		if(mProgress + 0.0001f >= mGoal)
		{
			mProgress = mGoal;

			// inform the achievements manager we're complete
			C_AchievementsManager::Instance().StickerComplete(mType);
		}
		else
		{

			if (SP_BossDemands::Instance().ShouldSendHUDMessage((S32)oldProgress, (S32)mProgress, (S32)mGoal))
				C_AchievementsManager::Instance().StickerProgressed(m_pAchievement, this);

/*
			bool notify = false;

			if (mOneShot || (mGoal <= 10.0f))
			{
				// Always notify when less than 10 notches

				notify = true;
			}
			else
			{
				float delta = mGoal - mProgress;

				if ((mProgress < 4.0f) || (delta < 4.0f))
				{
					// Notify first and last three notches

					notify = true;
				}
				else
				{
					// TODO: Rather than notifying every 20%, calculate a rounded number near that 20% and notifying using that instead.

					int iOld = (int)((oldProgress * 5) / mGoal);
					int iNew = (int)((mProgress * 5) / mGoal);

					notify = iOld != iNew;
				}
			}

			// TODO: logic about when to display progress towards stickers (e.g. in 1/5ths)
			if (notify)
				C_AchievementsManager::Instance().StickerProgressed(m_pAchievement, this);
*/
		}
	}

	// Make sure we check (in order of probability!) all these things before we just blindly add progress to a sticker
	bool C_Sticker::CanAddStickerProgress()
	{
		// Can't progress beyond 100%
		if(IsComplete())
			return false;

		// Can't give progress if there's no pad to give it to
		if(SysProfile::C_Manager::Instance().GetPrimaryPadNo() == -1)
		{
			LOGWARN(g_Log, "C_Sticker::CanAddStickerProgress >>>> Couldn't add progress to Sticker since primary pad == -1" );
			return false;
		}

		// Can't give progress in Split Screen ( BLUR_PARTY sticker needs exception case here )
		if( C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() == Game::E_GameMode_SplitScreen)
		{
			if(!m_pAchievement->mSplitScreen)
				return false;
		}
		else if( (C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameMode() != Game::E_GameMode_SinglePlayer) &&
			(!C_GameApp::GetInstance()->GetRaceBookLayer()->GetGameModePtr()->IsRanked()) &&
			(!C_GameApp::GetInstance()->GetUiLayer()->IsFrontend()))
		{
			//	If we're not in ranked online multiplayer or single player then NO Sticker progress can be awarded (with the exception of the single split screen one above).
			return false;
		}

		// Can't stand in the way of progress...
		return true;
	}
};