#include "stdafx.h"

#include <Game\Achievements\AchievementSystem.h>
#include <GameLogic\AchievementsManager.h>
#include <Ui/Game/Profiles/ProfileInterface.h>
#include <Game\Messaging\GameflowMessages.h>

#include <np.h>
#include <cell/l10n.h>

//#include <sysutil/sysutil_msgdialog.h>
#include <sys/timer.h> // for sys_timer_usleep

namespace Game
{

Debug::Logger &g_ALogger( Debug::GetLogger("Amax.AchievementSystemPS3") );

C_AchievementSystem C_AchievementSystem::s_instance;

int trophyCb(SceNpTrophyContext context, SceNpTrophyStatus status, int completed, int total, void *arg)
{
//	int ret;
//	uint32_t delta = 0;
//	static float progress = 0.0f;
//	static float step = 0.0f;
//	float curr = 0.0f;
//	float curve = 0.0f;
//
//	(void)arg;
#ifndef CFG_GOLD
	printf("entered trophy cb context=%d, status=%d, completed=%d/%d\n", context, status, completed, total);
#endif
//
//	switch (status) 
//	{
//		case SCE_NP_TROPHY_STATUS_NOT_INSTALLED:
//			ret = cellMsgDialogProgressBarSetMsg( CELL_MSGDIALOG_PROGRESSBAR_INDEX_SINGLE, "Installing Trophies" );
//			break;
//		case SCE_NP_TROPHY_STATUS_DATA_CORRUPT:
//			ret = cellMsgDialogProgressBarSetMsg( CELL_MSGDIALOG_PROGRESSBAR_INDEX_SINGLE, "Re-installing Trophies" );
//			break;
//		case SCE_NP_TROPHY_STATUS_INSTALLED:
//			ret = cellMsgDialogProgressBarSetMsg( CELL_MSGDIALOG_PROGRESSBAR_INDEX_SINGLE, "Setting up Trophies" );
//			break;
//		case SCE_NP_TROPHY_STATUS_REQUIRES_UPDATE:
//			ret = cellMsgDialogProgressBarSetMsg( CELL_MSGDIALOG_PROGRESSBAR_INDEX_SINGLE, "Updating Trophies" );
//			break;
//
//		case SCE_NP_TROPHY_STATUS_PROCESSING_SETUP:
//			curve = 10.0f;
//			break;
//		case SCE_NP_TROPHY_STATUS_PROCESSING_PROGRESS:
//			curve = 80.0f;
//			break;
//		case SCE_NP_TROPHY_STATUS_PROCESSING_FINALIZE:
//			curve = 10.0f;
//			break;
//		case SCE_NP_TROPHY_STATUS_PROCESSING_COMPLETE:
//			delta = 100;
//			ret = cellMsgDialogClose(0.1 * 1000.0f);
//			break;
//
//		case SCE_NP_TROPHY_STATUS_UNKNOWN:
//		default:
//#ifndef CFG_GOLD
//			printf("not supposed to be here! (%d)\n", status);
//#endif
//			break;
//	}
//
//	if (curve) 
//	{
//		curr = (((float)completed / (float)total) * curve) + step;
//		if (completed == total) 
//		{
//			step = curr;
//		}
//		if (curr > progress) 
//		{
//			delta = (uint32_t)(curr - progress);
//		}
//		if (delta) 
//		{
//			progress = curr;
//		}
//	}
//#ifndef CFG_GOLD
//	printf("progress=%f, curr=%f, step=%f, delta=%d\n", progress, curr, step, delta);
//#endif
//	if (delta) 
//	{
//		ret = cellMsgDialogProgressBarInc(CELL_MSGDIALOG_PROGRESSBAR_INDEX_SINGLE, delta);
//		if (ret < 0) 
//		{
//#ifndef CFG_GOLD
//			printf("cellMsgDialogProgressBarInc() failed %x\n", ret);
//#endif
//		}
//	}
//
	return 0;
}

static const SceNpCommunicationSignature npcommsign = {
    {
        0xb9,0xdd,0xe1,0x3b,0x01,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x3e,0x08,0x3a,0xfe,
        0xb1,0x12,0x23,0x7b,0x5c,0xe7,0x5b,0x21,
        0xac,0xc4,0x37,0x9a,0x0d,0x47,0xc0,0xad,
        0x76,0xe3,0x32,0x12,0x08,0xd7,0x59,0x17,
        0x31,0xfe,0x04,0x63,0x05,0x89,0xf6,0x33,
        0x57,0x1b,0x42,0x32,0x88,0xd6,0x5c,0x7b,
        0xda,0xe0,0xb8,0x0b,0x1c,0xe1,0xfd,0xc8,
        0x11,0x07,0x02,0xca,0x41,0x6e,0x5d,0x8f,
        0xf6,0xc5,0x52,0xb7,0xc8,0xf3,0xf2,0x79,
        0xc1,0x56,0x65,0x73,0x71,0x99,0x42,0x7c,
        0xaf,0xda,0x35,0x64,0x16,0x4a,0x05,0x80,
        0x59,0xc6,0xa9,0x45,0xdd,0x20,0xe4,0xc9,
        0xb3,0xf5,0x79,0xa0,0x07,0x83,0xa7,0x2a,
        0x3d,0x6e,0xb3,0xd4,0xec,0xe8,0x1f,0x4a,
        0x6a,0xba,0x6c,0xc8,0xf2,0xc2,0x46,0xe9,
        0x92,0x44,0x5a,0x62,0x17,0xdf,0x9a,0xa9,
        0xbb,0x53,0x43,0x0d,0x34,0xfe,0xd7,0xba,
        0xb8,0x54,0xf2,0x60,0xde,0x69,0x29,0xbe,
        0x9f,0xca,0x17,0x75,0xd5,0xb8,0x53,0x8d
    }
};

static const SceNpCommunicationId amax_trophy_network_platform_communication_id = 
	{
		{'N','P','W','R','0','0','5','6','7'},
		'\0',
		0,
		0
	};


void C_AchievementSystem::Init()
{
	int ret;
	
	if (mInitialised) return;

	ret = sceNpTrophyInit(NULL,0,SYS_MEMORY_CONTAINER_ID_INVALID,0);
	if (ret < 0) 
	{
		// Error handling
		return;
	}

	ret = sceNpTrophyCreateContext(&mTrophyContext,&amax_trophy_network_platform_communication_id,&npcommsign,0);
	if (ret < 0) 
	{
		//Error handling
		return;
	}


	ret = sceNpTrophyCreateHandle(&mTrophyHandle);
	if (ret < 0) 
	{
		//Error handling
		return;
	}

	ret = sceNpTrophyGetRequiredDiskSpace( mTrophyContext, mTrophyHandle, &mRequiredDiscSpace, 0 );
#ifndef CFG_GOLD
	printf("sceNpTrophyGetRequiredDiskSpace ret=(%d[%x]) size = %d\n",ret,ret,(int)mRequiredDiscSpace);
#endif

	//not the prettiest bit of code, but it gets a job done
	mAchievementMapping[BLUR_ACHIEVEMENT_SWING_OF_THINGS]		=	1;	
	mAchievementMapping[BLUR_ACHIEVEMENT_GETTING_STARTED]		=	2;	
	mAchievementMapping[BLUR_ACHIEVEMENT_CHALLENGE_MASTER]		=	3;	
	mAchievementMapping[BLUR_ACHIEVEMENT_CONE_GATES]			=	4;	
	mAchievementMapping[BLUR_ACHIEVEMENT_SKILLS]				=	5;
	mAchievementMapping[BLUR_ACHIEVEMENT_FIRST_TEST]			=	6;
	mAchievementMapping[BLUR_ACHIEVEMENT_LEARNING_THE_ROPES]	=	7;
	mAchievementMapping[BLUR_ACHIEVEMENT_BREAK_YOU]				=	8;
	mAchievementMapping[BLUR_ACHIEVEMENT_BETTER_ENEMIES]		=	9;
	mAchievementMapping[BLUR_ACHIEVEMENT_NOT_SO_PRETTY_BOY]		=	10;
	mAchievementMapping[BLUR_ACHIEVEMENT_BOLT_MASTER]			=	11;
	mAchievementMapping[BLUR_ACHIEVEMENT_FAN_BOY]				=	12;
	mAchievementMapping[BLUR_ACHIEVEMENT_BARGE_MASTER]			=	13;
	mAchievementMapping[BLUR_ACHIEVEMENT_NITRO_MASTER]			=	14;
	mAchievementMapping[BLUR_ACHIEVEMENT_TOURIST]				=	15;
	mAchievementMapping[BLUR_ACHIEVEMENT_SYBERIAN_EXPRESS]		=	16;
	mAchievementMapping[BLUR_ACHIEVEMENT_WRECKING_MACHINE] 		=	17;
	mAchievementMapping[BLUR_ACHIEVEMENT_CHECKPOINT_MASTER]		=	18;
	mAchievementMapping[BLUR_ACHIEVEMENT_THORNY_ISSUE]			=	19;
	mAchievementMapping[BLUR_ACHIEVEMENT_SHOCK_MASTER]			=	20;
	mAchievementMapping[BLUR_ACHIEVEMENT_TALONTED]				=	21;
	mAchievementMapping[BLUR_ACHIEVEMENT_MINE_MASTER]			=	22;
	mAchievementMapping[BLUR_ACHIEVEMENT_REPAIR_MASTER]			=	23;
	mAchievementMapping[BLUR_ACHIEVEMENT_RACE_MASTER]			=	24;
	mAchievementMapping[BLUR_ACHIEVEMENT_GLOBETROTTER]			=	25;
	mAchievementMapping[BLUR_ACHIEVEMENT_FAN_DEMAND_MASTER]		=	26;
	mAchievementMapping[BLUR_ACHIEVEMENT_TWIN_TROUBLE]			=	27;
	mAchievementMapping[BLUR_ACHIEVEMENT_SHIELD_MASTER]			=	28;
	mAchievementMapping[BLUR_ACHIEVEMENT_FOR_THE_FANS]			=	29;
	mAchievementMapping[BLUR_ACHIEVEMENT_GOING_GREY]			=	30;
	mAchievementMapping[BLUR_ACHIEVEMENT_LIGHTS_OUT_MEATBALL]	=	31;
	mAchievementMapping[BLUR_ACHIEVEMENT_MASTER_OF_THE_MACHINE]	=	32;
	mAchievementMapping[BLUR_ACHIEVEMENT_SHUNT_MASTER]			=	33;
	mAchievementMapping[BLUR_ACHIEVEMENT_SHOW_WHOS_BOSS]		=	34;
	mAchievementMapping[BLUR_ACHIEVEMENT_THE_REAL_HUSTLE]		=	35;
	mAchievementMapping[BLUR_ACHIEVEMENT_SPEED_DEMON]			=	36;
	mAchievementMapping[BLUR_ACHIEVEMENT_COMPLETIST]			=	37;
	mAchievementMapping[BLUR_ACHIEVEMENT_MEET_THE_MITCHELLS]	=	38;
	mAchievementMapping[BLUR_ACHIEVEMENT_GIANT_KILLER]			=	39;
	mAchievementMapping[BLUR_ACHIEVEMENT_CHALLENGER]			=	40;
	mAchievementMapping[BLUR_ACHIEVEMENT_ALL_ROUNDER]			=	41;
	mAchievementMapping[BLUR_ACHIEVEMENT_PAINT_JOB]				=	42;
	mAchievementMapping[BLUR_ACHIEVEMENT_RANK_FIFTY]			=	43;
	mAchievementMapping[BLUR_ACHIEVEMENT_DOMINATOR]				=	44;
	mAchievementMapping[BLUR_ACHIEVEMENT_MODDER]				=	45;
	mAchievementMapping[BLUR_ACHIEVEMENT_PHOTO_FINISH]			=	46;
	mAchievementMapping[BLUR_ACHIEVEMENT_MOTOR_MAN]				=	47;
	mAchievementMapping[BLUR_ACHIEVEMENT_BLUR_PARTY]			=	48;
																	
	mAchievementMapping[BLUR_ACHIEVEMENT_SOCIALISER]			=	-1;	//	This doesn't exist in the PS3 version of the game.
	mAchievementMapping[BLUR_ACHIEVEMENT_BEAT_BIZARRE]			=	-1;	//	This doesn't exist in the PS3 version of the game.
																	
	mTrophyStore.Clear();
	mThread=NULL;

	mInitialised = true;;

}

void C_AchievementSystem::InitialiseSubscription( Messaging::C_MessageSubscriberSystem* pSubscriberSystem )
{
	SetSubscriberSystem( pSubscriberSystem );

	//AddSubscriber( E_GameflowMessage_MusicLoadingComplete );
}

void C_AchievementSystem::Install()
{
	if ((!mInstalled) && (!mInstalling))
	{
		LOGINFO(g_ALogger, "C_AchievementSystem::Install - Create");

		mInstallThread = new C_AchievementSystemInstallThread();
		mInstallThread->mTrophyHandle=mTrophyHandle;
		mInstallThread->mTrophyContext=mTrophyContext; 
		mInstallThread->mpTrophyDetails				= &mTrophyDetails;
		mInstallThread->mpTrophyData				= &mTrophyData;
		mInstallThread->mpIndividualTrophyDetails	= mIndividualTrophyDetails;
		mInstallThread->mpIndividualTrophyData		= mIndividualTrophyData;
		mInstallThread->mpAchievementSystem			= this;

		LOGINFO(g_ALogger, "C_AchievementSystem::Install - PreStart(%x)", mInstallThread);
		
		mInstallThread->Start(0, -50, "Trophy Installing");
		
		LOGINFO(g_ALogger, "C_AchievementSystem::Install - PostStart");

		mInstalling = true;
	}
}

int C_AchievementSystem::GetRequiredDiskSpace()
{
	if (mRequiredDiscSpace == -1)
	{
		Init();
	}
	return mRequiredDiscSpace;
}

WCHAR *C_AchievementSystem::GetAchievementName(BLUR_ACHIEVEMENT achievement)
{
	TrophyName& trophyName = mTrophyNames[mAchievementMapping[achievement]];
	if(trophyName.name[0] == 0){

		char t[ SCE_NP_TROPHY_NAME_MAX_SIZE];
		memset( t, 0, SCE_NP_TROPHY_NAME_MAX_SIZE);
		strcpy_s( t, SCE_NP_TROPHY_NAME_MAX_SIZE, mIndividualTrophyDetails[mAchievementMapping[achievement]].name);

		size_t len = strlen( t );
		size_t wlen = SCE_NP_TROPHY_NAME_MAX_SIZE;

		L10nResult res = UTF8stoUCS2s((uint8_t*)t, &len, (uint16_t*)trophyName.name, &wlen);
#ifndef CFG_GOLD
		if(res != ConversionOK)
		{
			wcscpy(trophyName.name, L"Error");
		}
#endif // CFG_GOLD
	}

	return trophyName.name;
}

U32 C_AchievementSystem::GetAchievementScore(BLUR_ACHIEVEMENT achievement)
{
	return mIndividualTrophyDetails[mAchievementMapping[achievement]].trophyGrade;
}

void C_AchievementSystem::GetAchievementDetails(int user_index)
{
	Install();
}

void C_AchievementSystem::Update()
{
	if(mInstallThread)
	{
		static bool bFirstTime = true;

		if (bFirstTime)
		{
			LOGINFO(g_ALogger, "C_AchievementSystem::Update - First - mInstallThread(0x%x) mInstalling(%d) mInstalled(%d) Running(%d)", mInstallThread, mInstalling, mInstalled, mInstallThread->IsRunning());
			bFirstTime = false;
		}

		cellSysutilCheckCallback();

		//if(!mInstallThread->IsRunning())
		if (mInstalled)
		{
			LOGINFO(g_ALogger, "C_AchievementSystem::Update - Destroy Thread(%x) mInstalling(%d) mInstalled(%d)", mInstallThread, mInstalling, mInstalled);

			mInstallThread->Join();

			delete(mInstallThread);
			mInstallThread=NULL;

			PreCacheTrophyNames();
			
			//RemoveSubscriber( E_GameflowMessage_MusicLoadingComplete );
		}
	}
	else
	{
		//run the thread and give the trophy
		if(mThread)
		{
			if(!mThread->IsRunning())
			{
				mThread->Join();

				delete(mThread);
				mThread=NULL;
			}
		}
		else
		{
			//have we got anything to give, then give it
			if(mTrophyStore.Size())
			{
				if(!mIndividualTrophyData[mTrophyStore[0]].unlocked)
				{
					BIZ_ASSERT_MSG(mThread == NULL, "Trying to unlock a Trophy, when one is still un-locking");

					//set the value myself, nothing else uses this apart from me and it gets re-done at boot by the system
					mIndividualTrophyData[mTrophyStore[0]].unlocked=true;

					mThread = new C_AchievementSystemThread();
					
					mThread->mAchievement=mTrophyStore[0];
					mThread->mTrophyHandle=mTrophyHandle;
					mThread->mTrophyContext=mTrophyContext; 

					mThread->Start(0, 0, "Trophy Unlocking");
				}

				mTrophyStore.EraseByIndex(0);
			}
		}
	}
}

bool C_AchievementSystem::HasAchievement(int user_index,BLUR_ACHIEVEMENT achievement)
{
	return mIndividualTrophyData[mAchievementMapping[achievement]].unlocked;
}

void C_AchievementSystem::GiveAchievement(int user_index,BLUR_ACHIEVEMENT achievement)
{
	if(SysProfile::C_Manager::Instance().GetPrimaryPadNo() != user_index)
		return;

	mTrophyStore.PushBack(mAchievementMapping[achievement]);
}

void C_AchievementSystem::PreCacheTrophyNames()
{
	//	Pre-cache the trophy names as it's a bit slow to grab them the first time we need to display them in the UI
	for(int i = 0; i < BLUR_ACHIEVEMENT_MAX; i++)
	{
		if(mAchievementMapping[i] != -1)
			GetAchievementName((BLUR_ACHIEVEMENT)i);
	}
}

bool C_AchievementSystem::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	PERF_EVENT("C_AchievementSystem::MessageHandler");

	switch( pMessage->Type() )
	{
		//case E_GameflowMessage_MusicLoadingComplete:
		//	Install();
		//	return true;

		default:
			break;
	};

	return false;
}

void C_AchievementSystemThread::ThreadMain()
{
	SceNpTrophyId platinumId = SCE_NP_TROPHY_INVALID_TROPHY_ID;

	int ret = sceNpTrophyUnlockTrophy(mTrophyContext,mTrophyHandle,mAchievement,&platinumId);
	
	if ( ret < 0 ) 
	{
		// Error handling
		// wait a bit before another unlock-thread can start 
		sys_timer_usleep( 100000 );
	}
	// Waited in the unlock for 5-10 seconds so no need for sleep
#ifdef COMPILER_VC
	Close();
#endif
}



void C_AchievementSystemInstallThread::ThreadMain()
{
	LOGINFO(g_ALogger, "C_AchievementSystemInstallThread::ThreadMain - In(0x%x)", this);

	// Sleep here to make sure that the boot load has finished HDD access, 
	// the aim here is to have as little as possible going on with the HDD
	// when this is called.
	//sys_timer_sleep(2);

	int ret = sceNpTrophyRegisterContext(mTrophyContext, mTrophyHandle, Game::trophyCb, NULL, 0);
	if ( ret < 0 ) 
	{
		// Error handling
	}

	memset(mpTrophyDetails, 0x00, sizeof(*mpTrophyDetails));
	memset(mpTrophyData,	0x00, sizeof(*mpTrophyData));

	ret = sceNpTrophyGetGameInfo(mTrophyContext, mTrophyHandle, mpTrophyDetails, mpTrophyData);
	if ( ret < 0 ) 
	{
		// Error handling
	}

	for(int a=0;a<mpTrophyDetails->numTrophies;a++)
	{
		//LOGINFO(g_ALogger, "C_AchievementSystemInstallThread::ThreadMain - %d/%d", a, mpTrophyDetails->numTrophies);

		memset(&mpIndividualTrophyDetails[a],	0x00, sizeof(mpIndividualTrophyDetails[a]));
		memset(&mpIndividualTrophyData[a],		0x00, sizeof(mpIndividualTrophyData[a]));

		int attempts = 0;

		do
		{
			ret = sceNpTrophyGetTrophyInfo(mTrophyContext, mTrophyHandle, a, &mpIndividualTrophyDetails[a], &mpIndividualTrophyData[a]);
			// Pause here to let the main thread have some breathing-space to
			// render etc.
			sys_timer_usleep(60);
			attempts++;

			//LOGINFO(g_ALogger, "C_AchievementSystemInstallThread::ThreadMain - %d - ret(%d) - attempts(%d)", a, ret, attempts);

			//	Hidden trophies return a non-zero error code so we don't want to re-attempt these.
			if(ret == 0x80022913)
				break;
		} while( ret < 0 && attempts < 4);
	}		
	mpAchievementSystem->SetInstalled( true );
	mpAchievementSystem->SetInstalling( false );

	LOGINFO(g_ALogger, "C_AchievementSystemInstallThread::ThreadMain - Out(0x%x)", this);
}

}; // namespace Game