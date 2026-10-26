#ifndef ACHIEVEMENT_SYSTEM_H
#define ACHIEVEMENT_SYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Game\Achievements\AchievementEnums.h>
#include <Core/Threads/Thread.h>
#include <SchedulerConfig.h>
#include <misc_shared/Messaging/MessageHandler.h>

//platform specific things
#ifdef PLATFORM_PS3
#include <np.h>
#include <cell/l10n.h>
#endif

namespace Game
{

class C_AchievementSystem;

class C_AchievementSystemThread : public MT::Thread
{
public:
	void ThreadMain();

#ifdef PLATFORM_PS3
	int mAchievement;

	SceNpTrophyHandle mTrophyHandle;
	SceNpTrophyContext mTrophyContext;
#endif

};

class C_AchievementSystemInstallThread : public MT::Thread
{
public:
	void ThreadMain();

#ifdef PLATFORM_PS3
	SceNpTrophyHandle mTrophyHandle;
	SceNpTrophyContext mTrophyContext;

	SceNpTrophyGameDetails*	mpTrophyDetails;
	SceNpTrophyGameData*	mpTrophyData;

	SceNpTrophyDetails*		mpIndividualTrophyDetails;
	SceNpTrophyData*		mpIndividualTrophyData;
	
	C_AchievementSystem*	mpAchievementSystem;
#endif
};


//these will get mapped to the relevant platform
class C_AchievementSystem : public Messaging::C_MessageHandler
{
public:

	C_AchievementSystem()
	{
#ifdef PLATFORM_PS3
		mRequiredDiscSpace = -1;
		mInstalled = false;
		mInitialised = false;
		mInstalling = false;
#endif
	};

	~C_AchievementSystem()
	{
	};

	void Init();
	void InitialiseSubscription( Messaging::C_MessageSubscriberSystem* pSubscriberSystem );
#ifdef PLATFORM_PS3
	void Install();
#endif
	void Update();
	void GiveAchievement(int user_index,BLUR_ACHIEVEMENT achievement);
	bool HasAchievement(int user_index,BLUR_ACHIEVEMENT achievement);	//	returns TRUE if the player already has this achievement

	void GetAchievementDetails(int user_index);

	WCHAR *GetAchievementName(BLUR_ACHIEVEMENT achievement);
	U32 GetAchievementScore(BLUR_ACHIEVEMENT achievement);

	static C_AchievementSystem& Instance(){return s_instance;};

	virtual bool MessageHandler(const  Messaging::C_MessageBase* pMessage);

private:
	static C_AchievementSystem s_instance;

	int mAchievementMapping[BLUR_ACHIEVEMENT_MAX];

	//platform specific things
#ifdef PLATFORM_PS3
	struct TrophyName
	{
		WCHAR name[SCE_NP_TROPHY_NAME_MAX_SIZE];
		TrophyName() { name[0] = 0; }
	};

	SceNpTrophyGameDetails mTrophyDetails;
	SceNpTrophyGameData mTrophyData;
	SceNpTrophyHandle mTrophyHandle;
	SceNpTrophyContext mTrophyContext;

	SceNpTrophyDetails mIndividualTrophyDetails[BLUR_ACHIEVEMENT_MAX+1];		//+1 for the platinum
	SceNpTrophyData mIndividualTrophyData[BLUR_ACHIEVEMENT_MAX+1];			//+1 for the platinum

	SceNpTrophyId mPlatinumId;

	Btl::Vector<TrophyName, Btl::VectorParameters_Array<BLUR_ACHIEVEMENT_MAX+1> > mTrophyNames;
	void PreCacheTrophyNames();
	C_AchievementSystemThread *mThread;
	C_AchievementSystemInstallThread *mInstallThread;

	Btl::Vector<int> mTrophyStore;

	uint64_t mRequiredDiscSpace;
#ifdef PLATFORM_PS3
	bool	 mInstalled;
	bool	 mInitialised;
	bool	 mInstalling;
#endif
public:
	int GetRequiredDiskSpace();
#ifdef PLATFORM_PS3
	bool GetInstalled() { return mInstalled; }
	void SetInstalled( bool set ) { mInstalled = set; }
	void SetInstalling( bool set ) { mInstalling = set; }
#endif
private:
#endif
};


} // namespace Game

#endif // ACHIEVEMENT_SYSTEM_H
