#include "stdafx.h"

#include <GameApp.h>
#include <Game/Game.h>
#include <Gamelogic\Twitter\TwitterManager.h>

#include <GameLogic/Profile/GameProfileManager.h>
#include <GameLogic/Profile/GameProfile.h>

#include <Game\Achievements\AchievementSystem.h>

#include <RaceBook/RaceBookManager.h>

#include <Network/Core.h>
#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>

#include <ui/DashboardInterface.h>

#include <ui\Game\UILocaliser.h>

#include <Racebook\RaceBookManager.h>

#ifdef PLATFORM_PS3
#include <cell/l10n.h>
#endif

Debug::Logger &gTwitterMessageLogger = Debug::GetLogger("Amax.Twitter");

Debug::Bool g_debugTwitterInjectGeneralError("Twitter.InjectError", false);
Debug::Int g_debugTwitterInjectGeneralErrorEnum("Twitter.InjectErrorEnum", 0);

Debug::Int g_debugTwitterTimeOutOveride("Twitter.TimeOutOveride", 0);

namespace Game
{

C_TwitterManager C_TwitterManager::s_instance;

void C_TwitterManager::Init()
{
	mCurrentTaskType=TWITTER_TASK_TYPE_NOTHING;
	mAuthenticated=false;
	
	//mHasPublisherFile=false;
	mCheckingForPublisherFile=false;

	mTimeOutTimer=0.f;
	mTimeOut=0;
}

void C_TwitterManager::SetUsername(WCHAR *user)
{
	C_GameProfileManager::Instance().GetPrimaryProfile()->mTwitterUsername=user;
}

void C_TwitterManager::SetPassword(WCHAR *pass)
{
	C_GameProfileManager::Instance().GetPrimaryProfile()->mTwitterPassword=pass;
}

const WCHAR *C_TwitterManager::GetUsername()
{
	return C_GameProfileManager::Instance().GetPrimaryProfile()->mTwitterUsername.Chars();
}

const WCHAR *C_TwitterManager::GetPassword()
{
	return C_GameProfileManager::Instance().GetPrimaryProfile()->mTwitterPassword.Chars();
}

bool C_TwitterManager::IsAuthenticated()
{
	return mAuthenticated;
}

void C_TwitterManager::Authenticate()
{
	bdTwitter *twitter = RacerNet::C_NetServices::Instance().Router().GetTwitter();

	KillCurrentTask();

	if(twitter)
	{
		memset(mUtfUsername,0,sizeof(mUtfUsername));
		memset(mUtfPassword,0,sizeof(mUtfPassword));

#ifdef PLATFORM_PS3
		L10nResult res;
		size_t len  = MAX_UTF_STRING_SIZE;

		size_t wlen = wcslen( C_GameProfileManager::Instance().GetPrimaryProfile()->mTwitterUsername.Chars() );
		res = UCS2stoUTF8s( (uint16_t*)C_GameProfileManager::Instance().GetPrimaryProfile()->mTwitterUsername.Chars(), &wlen, (uint8_t*)mUtfUsername, &len );

		len  = MAX_UTF_STRING_SIZE;
		wlen = wcslen( C_GameProfileManager::Instance().GetPrimaryProfile()->mTwitterPassword.Chars() );
		res = UCS2stoUTF8s( (uint16_t*)C_GameProfileManager::Instance().GetPrimaryProfile()->mTwitterPassword.Chars(), &wlen, (uint8_t*)mUtfPassword, &len );
#else
		WideCharToMultiByte(CP_UTF8, 0, C_GameProfileManager::Instance().GetPrimaryProfile()->mTwitterUsername.Chars(), wcslen(C_GameProfileManager::Instance().GetPrimaryProfile()->mTwitterUsername.Chars()), mUtfUsername, MAX_UTF_STRING_SIZE, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, C_GameProfileManager::Instance().GetPrimaryProfile()->mTwitterPassword.Chars(), wcslen(C_GameProfileManager::Instance().GetPrimaryProfile()->mTwitterPassword.Chars()), mUtfPassword, MAX_UTF_STRING_SIZE, NULL, NULL);
#endif
		mAsyncTask = twitter->registerAccount(mUtfUsername, mUtfPassword);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gTwitterMessageLogger,"Twitter - Authentication Started\n");

			mCurrentTaskType=TWITTER_TASK_TYPE_REGISTER;
		}
	}
}

bool C_TwitterManager::PumpCurrentTask(int *error)
{
	bdTwitter *twitter = RacerNet::C_NetServices::Instance().Router().GetTwitter();

	if(twitter)
	{
		*error=BD_NO_ERROR;

		switch (NetPlay::DemonwareRemoteTask::CheckState(mAsyncTask))
		{
		case NetPlay::DemonwareRemoteTask::ePending:
			{
				return true;
			}
			break;

		case NetPlay::DemonwareRemoteTask::eFailed:
			{
				int e=mAsyncTask ? mAsyncTask->getErrorCode() : BD_HANDLE_TASK_FAILED;
				LOGINFO( gTwitterMessageLogger,"Twitter - Task Failed (%d)\n",mAsyncTask ? mAsyncTask->getErrorCode() : 0);
				*error = e;
				return false;
			}
			break;

		case NetPlay::DemonwareRemoteTask::eDone:
			if (mAsyncTask->getErrorCode() == BD_NO_ERROR)
			{
				// successful
				if(mCurrentTaskType==TWITTER_TASK_TYPE_REGISTER)
				{
					mAuthenticated=true;
				}
				else if(mCurrentTaskType==TWITTER_TASK_TYPE_POST)
				{
					mTimeOutTimer=mTimeOut;
				}

				LOGINFO( gTwitterMessageLogger,"Twitter - Task Completed and Successful\n");
				*error = BD_NO_ERROR;

#ifndef CFG_GOLD
				if(g_debugTwitterInjectGeneralError)
				{
					g_debugTwitterInjectGeneralError=false;
					*error=g_debugTwitterInjectGeneralErrorEnum;
				}
#endif
			}
			else
			{
				// failed
				int e=mAsyncTask ? mAsyncTask->getErrorCode() : BD_HANDLE_TASK_FAILED;

				LOGINFO( gTwitterMessageLogger,"Twitter - Task Completed but Failed\n");
				*error = e;
				return false;
			}
			break;

		default:
			*error = BD_HANDLE_TASK_FAILED;
			break;
		}
	}
	else
	{
		*error = BD_HANDLE_TASK_FAILED;
	}
	
	// close async task to release demonware resources.
	KillCurrentTask();

	return false;
}

void C_TwitterManager::KillCurrentTask()
{
	LOGINFO( gTwitterMessageLogger,"Twitter - Killing Any Current Task\n");

	mCurrentTaskType=TWITTER_TASK_TYPE_NOTHING;
	NetPlay::DemonwareRemoteTask::Close(mAsyncTask);
}

bool C_TwitterManager::Tweet(const WCHAR *message)
{
	if(mTimeOutTimer)
		return false;

	bdTwitter *twitter = RacerNet::C_NetServices::Instance().Router().GetTwitter();

	KillCurrentTask();

	if(twitter)
	{
		memset(mUtfTranslation,0,sizeof(mUtfTranslation));

#ifdef PLATFORM_PS3
		size_t wlen = wcslen( message );
		size_t len  = MAX_UTF_STRING_SIZE;
		L10nResult res = UCS2stoUTF8s( (uint16_t*)message, &wlen, (uint8_t*)mUtfTranslation, &len );
#else
		WideCharToMultiByte(CP_UTF8, 0, message, wcslen(message), mUtfTranslation, MAX_UTF_STRING_SIZE, NULL, NULL);
#endif

		mAsyncTask = twitter->post(mUtfTranslation);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gTwitterMessageLogger,"Twitter - Posting Started\n");

			mCurrentTaskType=TWITTER_TASK_TYPE_POST;

			return true;
		}
	}

	return false;
}

void C_TwitterManager::Update()
{
	if(mCheckingForPublisherFile)
	{
		mServerTimeOut += C_GameApp::GetInstance()->GetTimeStep();

		if(mServerTimeOut >= TWITTER_SERVER_TIMEOUT)
		{
			NetPlay::DemonwareRemoteTask::Close(mPublisherAsyncTask);

			mCheckingForPublisherFile=false;
			//mHasPublisherFile=false;
			return;
		}

		RacerNet::C_NetServices& services = RacerNet::C_NetServices::Instance();

		bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();
		if (streaming)
		{
			switch (NetPlay::DemonwareRemoteTask::CheckState(mPublisherAsyncTask))
			{
				case NetPlay::DemonwareRemoteTask::ePending:
					{

					}
					break;
				case NetPlay::DemonwareRemoteTask::eFailed:
					{
						mCheckingForPublisherFile=false;
						//mHasPublisherFile=false;
						return;
					}
					break;
				case NetPlay::DemonwareRemoteTask::eDone:
					{
						mCheckingForPublisherFile=false;
						//mHasPublisherFile=false;

						if (mPublisherAsyncTask->getErrorCode() == BD_NO_ERROR)
						{
							if(mPublisherAsyncTask->getNumResults() == 1)
							{
								if(!strncmp(mPublisherFileDataResults.m_fileName,"Twitter.",8))
								{
									mHasPublisherFile=true;

									mTimeOut=0;
									sscanf(mPublisherFileDataResults.m_fileName,"Twitter.%d",&mTimeOut);

#ifndef CFG_GOLD
									if(g_debugTwitterTimeOutOveride)
									{
										mTimeOut=g_debugTwitterTimeOutOveride;
									}
#endif
								}
							}
							return;
						}
						else
						{
							//mHasPublisherFile=false;
							return;
						}
					}
					break;
				default:
					{
						mCheckingForPublisherFile=false;
						//mHasPublisherFile=false;
						return;
					}
					break;
			}
		}
	}

	if(mTimeOutTimer)
	{
		mTimeOutTimer -= C_GameApp::GetInstance()->GetTimeStep();

		if(mTimeOutTimer < 0.f)
		{
			mTimeOutTimer=0.f;
		}
	}
}

void C_TwitterManager::CheckPublisherFile()
{
	NetPlay::DemonwareRemoteTask::Close(mPublisherAsyncTask);

	//mHasPublisherFile=false;

	if (RacerNet::C_NetServices::Instance().IsInitialised())
	{
		RacerNet::C_NetServices::Instance().Router().EnableContentStreaming(true);
	}

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();

	if(streaming)
	{
		mPublisherAsyncTask = streaming->listAllPublisherFiles(0,1,&mPublisherFileDataResults,1);	//,0,"Twitter.Activate");

		if (NetPlay::DemonwareRemoteTask::IsBusy(mPublisherAsyncTask))
		{
			mServerTimeOut=0.f;
			mCheckingForPublisherFile=true;
		}
	}
}

}; // namespace Game
