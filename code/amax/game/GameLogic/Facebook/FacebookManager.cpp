#include "stdafx.h"

#include <Game/Game.h>
#include <Gamelogic\Facebook\FacebookManager.h>

#include <GameLogic/Profile/GameProfileManager.h>
#include <GameLogic/Profile/GameProfile.h>

#include <Game\Achievements\AchievementSystem.h>

#include <Network/Core.h>
#include <Network/Core/NetServices.h>
#include <Network/Router/NetRouterBase.h>

#include <ui/DashboardInterface.h>

#include <ui\Game\UILocaliser.h>

#include <GameLogic\Twitter\TwitterManager.h>
#include <RaceBook/RaceBookManager.h>
#include <GameApp.h>

#ifdef PLATFORM_PS3
#include <cell/l10n.h>
#endif

Debug::Logger &gFacebookMessageLogger = Debug::GetLogger("Amax.Facebook");

Debug::Bool g_debugFacebookInjectGeneralError("Facebook.InjectError", false);
Debug::Int g_debugFacebookInjectGeneralErrorEnum("Facebook.InjectErrorEnum", 0);

namespace Game
{

C_FacebookManager C_FacebookManager::s_instance;

void C_FacebookManager::Init()
{
	mCurrentTaskType=FACEBOOK_TASK_TYPE_NOTHING;
	mAuthenticated=false;
	
	//mHasPublisherFile=false;
	mCheckingForPublisherFile=false;

	mTimeOutTimer=0.f;
	mTimeOut=0;

	mCheckingForImageURLFile=false;
	mDownloadingImageURL=false;

	mAllChecksDone=false;

	mHasRegistered.m_isRegistered=false;
	mCheckingHasRegistered=false;
}

void C_FacebookManager::ParseImageURLFile()
{
	mImageURLStore.Clear();

	bool doit=true;
	char *temp_url_file=(char *)mImageURLs;
	
	while(doit)
	{
		AmaxImageURL url;

		int num;
		
		int ret=sscanf(temp_url_file, "# %s", const_cast<char*>(url.mKeyword.Chars()));

		if(ret == 1)
		{
			//find next line
			temp_url_file++;
			while(*temp_url_file != 10) temp_url_file++;
			temp_url_file++;
			
			ret=sscanf(temp_url_file, "%d %s", &num, const_cast<char*>(url.mURL.Chars()));

			if(ret == 2)
			{
				//find next line
				temp_url_file++;
				while(*temp_url_file != 10) temp_url_file++;
				temp_url_file++;

				mImageURLStore.PushBack(url);
			}
			else
			{
				doit=false;
			}
		}
		else
		{
			doit=false;
		}
	}

	delete[] mImageURLs;
	mImageURLs=NULL;
}

void C_FacebookManager::CheckIsRegistered()
{
	bdFacebook *facebook = RacerNet::C_NetServices::Instance().Router().GetFacebook();

	NetPlay::DemonwareRemoteTask::Close(mRegisteredAsyncTask);

	if(facebook)
	{
		mRegisteredAsyncTask = facebook->isRegistered(&mHasRegistered);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mRegisteredAsyncTask))
		{
			LOGINFO( gFacebookMessageLogger,"Facebook - CheckIsRegistered Started\n");

			mCheckingHasRegistered=true;
		}
	}
}

void C_FacebookManager::Update()
{
	if(mCheckingHasRegistered)
	{
		switch (NetPlay::DemonwareRemoteTask::CheckState(mRegisteredAsyncTask))
		{
			case NetPlay::DemonwareRemoteTask::ePending:
				break;

			case NetPlay::DemonwareRemoteTask::eFailed:
				{
					mCheckingHasRegistered=false;
					mHasRegistered.m_isRegistered=false;
				}
				break;

			case NetPlay::DemonwareRemoteTask::eDone:
				{
					if (mRegisteredAsyncTask->getErrorCode() == BD_NO_ERROR)
					{
						mCheckingHasRegistered=false;
						//has registered is valid, so leave alone
					}
					else
					{
						mCheckingHasRegistered=false;
						mHasRegistered.m_isRegistered=false;
					}
				}
				break;

			default:
				{
					mCheckingHasRegistered=false;
					mHasRegistered.m_isRegistered=false;
				}
				break;
		}
	}
	
	if(mCheckingForPublisherFile || mCheckingForImageURLFile || mDownloadingImageURL)
	{
		mServerTimeOut += C_GameApp::GetInstance()->GetTimeStep();

		if(mServerTimeOut >= FACEBOOK_SERVER_TIMEOUT)
		{
			NetPlay::DemonwareRemoteTask::Close(mPublisherAsyncTask);

			mCheckingForPublisherFile=false;
			mCheckingForImageURLFile=false;
			mDownloadingImageURL=false;
			
			//mHasPublisherFile=false;

			mAllChecksDone=true;
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
						if(mCheckingForPublisherFile)
						{
							mCheckingForPublisherFile=false;
							//mHasPublisherFile=false;
						}

						if(mCheckingForImageURLFile)
						{
							mCheckingForImageURLFile=false;
							//mHasPublisherFile=false;
						}

						if(mDownloadingImageURL)
						{
							mDownloadingImageURL=false;
							//mHasPublisherFile=false;
						}

						mAllChecksDone=true;

						return;
					}
					break;
				case NetPlay::DemonwareRemoteTask::eDone:
					{
						if(mCheckingForPublisherFile)
						{
							mCheckingForPublisherFile=false;
							//mHasPublisherFile=false;

							if (mPublisherAsyncTask->getErrorCode() == BD_NO_ERROR)
							{
								if(mPublisherAsyncTask->getNumResults() == 1)
								{
									if(!strncmp(mPublisherFileDataResults.m_fileName,"Facebook.",9))
									{
										//mHasPublisherFile=true;

										mTimeOut=0;
										sscanf(mPublisherFileDataResults.m_fileName,"Facebook.%d",&mTimeOut);

										CheckImageURLFile();
										return;
									}
								}
							}

							//error
							mAllChecksDone=true;
							return;
						}
						else if(mCheckingForImageURLFile)
						{
							mCheckingForImageURLFile=false;
							//mHasPublisherFile=false;

							if (mPublisherAsyncTask->getErrorCode() == BD_NO_ERROR)
							{
								if(mPublisherAsyncTask->getNumResults() == 1)
								{
									if(!strcmp(mImageURLFileDataResults.m_fileName,"image_urls"))
									{
										//mHasPublisherFile=true;
										
										DownloadImageURLFile();
										return;
									}
								}
							}

							//error
							mAllChecksDone=true;
							return;
						}
						else if(mDownloadingImageURL)
						{
							mDownloadingImageURL=false;
							//mHasPublisherFile=false;

							if (mPublisherAsyncTask->getErrorCode() == BD_NO_ERROR)
							{
								mHasPublisherFile=true;
								ParseImageURLFile();

								CheckIsRegistered();
							}

							mAllChecksDone=true;
							return;
						}
					}
					break;
				default:
					{
						if(mCheckingForPublisherFile)
						{
							mCheckingForPublisherFile=false;
							//mHasPublisherFile=false;
						}

						if(mCheckingForImageURLFile)
						{
							mCheckingForImageURLFile=false;
							//mHasPublisherFile=false;
						}

						if(mDownloadingImageURL)
						{
							mDownloadingImageURL=false;
							//mHasPublisherFile=false;
						}

						mAllChecksDone=true;

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

void C_FacebookManager::SetUsername(const WCHAR *user)
{
	C_GameProfileManager::Instance().GetPrimaryProfile()->mFacebookUsername=user;
}

void C_FacebookManager::SetPassword(const WCHAR *pass)
{
	C_GameProfileManager::Instance().GetPrimaryProfile()->mFacebookPassword=pass;
}

const WCHAR *C_FacebookManager::GetUsername()
{
	return C_GameProfileManager::Instance().GetPrimaryProfile()->mFacebookUsername.Chars();
}

const WCHAR *C_FacebookManager::GetPassword()
{
	return C_GameProfileManager::Instance().GetPrimaryProfile()->mFacebookPassword.Chars();
}

bool C_FacebookManager::IsAuthenticated()
{
	return mAuthenticated;
}

void C_FacebookManager::Authenticate()
{
	bdFacebook *facebook = RacerNet::C_NetServices::Instance().Router().GetFacebook();

	KillCurrentTask();

	if(facebook)
	{
		memset(mUtfUsername,0,sizeof(mUtfUsername));
		memset(mUtfPassword,0,sizeof(mUtfPassword));

#ifdef PLATFORM_PS3
		L10nResult res;
		size_t len  = MAX_UTF_STRING_SIZE;

		size_t wlen = wcslen( C_GameProfileManager::Instance().GetPrimaryProfile()->mFacebookUsername.Chars() );
		res = UCS2stoUTF8s( (uint16_t*)C_GameProfileManager::Instance().GetPrimaryProfile()->mFacebookUsername.Chars(), &wlen, (uint8_t*)mUtfUsername, &len );

		len  = MAX_UTF_STRING_SIZE;
		wlen = wcslen( C_GameProfileManager::Instance().GetPrimaryProfile()->mFacebookPassword.Chars() );
		res = UCS2stoUTF8s( (uint16_t*)C_GameProfileManager::Instance().GetPrimaryProfile()->mFacebookPassword.Chars(), &wlen, (uint8_t*)mUtfPassword, &len );
#else
		WideCharToMultiByte(CP_UTF8, 0, C_GameProfileManager::Instance().GetPrimaryProfile()->mFacebookUsername.Chars(), wcslen(C_GameProfileManager::Instance().GetPrimaryProfile()->mFacebookUsername.Chars()), mUtfUsername, MAX_UTF_STRING_SIZE, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, C_GameProfileManager::Instance().GetPrimaryProfile()->mFacebookPassword.Chars(), wcslen(C_GameProfileManager::Instance().GetPrimaryProfile()->mFacebookPassword.Chars()), mUtfPassword, MAX_UTF_STRING_SIZE, NULL, NULL);
#endif

		mAsyncTask = facebook->registerAccount(mUtfUsername,mUtfPassword);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gFacebookMessageLogger,"Facebook - Authentication Started\n");

			mCurrentTaskType=FACEBOOK_TASK_TYPE_REGISTER;
		}
	}
}

void C_FacebookManager::UnRegister()
{
	bdFacebook *facebook = RacerNet::C_NetServices::Instance().Router().GetFacebook();

	KillCurrentTask();

	if(facebook)
	{
		mAsyncTask = facebook->unregisterAccount();

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gFacebookMessageLogger,"Facebook - unregisterAccount() Started\n");

			mCurrentTaskType=FACEBOOK_TASK_TYPE_UNREGISTER;
		}
	}
}

bool C_FacebookManager::Publish()
{
	if(mTimeOutTimer)
		return false;

	bdFacebook *facebook = RacerNet::C_NetServices::Instance().Router().GetFacebook();

	KillCurrentTask();

	if(facebook)
	{
		const AmaxSocialNetworkingItem *sni=C_SocialNetworkManager::Instance().GetCurrentItem();

		memset(mUtfMessage,0,sizeof(mUtfMessage));
		memset(mUtfName,0,sizeof(mUtfName));
		memset(mUtfCaption,0,sizeof(mUtfCaption));
		memset(mUtfDescription,0,sizeof(mUtfDescription));
		memset(mUtfImageURL,0,sizeof(mUtfImageURL));
		memset(mUtfImageHRef,0,sizeof(mUtfImageHRef));

#ifdef PLATFORM_PS3
		L10nResult res;
		size_t wlen = 0;
		size_t len  = MAX_UTF_STRING_SIZE;

		wlen = wcslen(Game::C_SocialNetworkManager::Instance().mCurrentMessage.Chars());
		res = UCS2stoUTF8s( (uint16_t*)Game::C_SocialNetworkManager::Instance().mCurrentMessage.Chars(), &wlen, (uint8_t*)mUtfMessage, &len );

		len  = MAX_UTF_STRING_SIZE;
		int url=GetImageURL(sni->facebook_story.image);
		if(url != -1)
		{
			UI::C_WideString256 image_url_str(mImageURLStore[url].mURL.Chars());

			wlen = wcslen(image_url_str.Chars());
			res = UCS2stoUTF8s( (uint16_t*)image_url_str.Chars(), &wlen, (uint8_t*)mUtfImageURL, &len );
		}
		else
		{
			mUtfImageURL[0]=0;
		}

		len  = MAX_UTF_STRING_SIZE;
		wlen = wcslen(C_SocialNetworkManager::Instance().GetCurrentItemText());
		res = UCS2stoUTF8s( (uint16_t*)C_SocialNetworkManager::Instance().GetCurrentItemText(), &wlen, (uint8_t*)mUtfName, &len );

		len  = MAX_UTF_STRING_SIZE;
		wlen = wcslen(UI::C_TextStringID(sni->facebook_story.caption.id).String());
		res = UCS2stoUTF8s( (uint16_t*)UI::C_TextStringID(sni->facebook_story.caption.id).String(), &wlen, (uint8_t*)mUtfCaption, &len );
		
		len  = MAX_FB_UTF_DESCRIPTION_SIZE;
		wlen = wcslen(UI::C_TextStringID(sni->facebook_story.description.id).String());
		res = UCS2stoUTF8s( (uint16_t*)UI::C_TextStringID(sni->facebook_story.description.id).String(), &wlen, (uint8_t*)mUtfDescription, &len );
		
		len  = MAX_UTF_STRING_SIZE;
		wlen = wcslen(UI::C_TextStringID(sni->facebook_story.image_href.id).String());
		res = UCS2stoUTF8s( (uint16_t*)UI::C_TextStringID(sni->facebook_story.image_href.id).String(), &wlen, (uint8_t*)mUtfImageHRef, &len );

#else
		WideCharToMultiByte(CP_UTF8, 0, Game::C_SocialNetworkManager::Instance().mCurrentMessage.Chars(), wcslen(Game::C_SocialNetworkManager::Instance().mCurrentMessage.Chars()), mUtfMessage, MAX_UTF_STRING_SIZE, NULL, NULL);

		int url=GetImageURL(sni->facebook_story.image);
		if(url != -1)
		{
			UI::C_WideString256 image_url_str(mImageURLStore[url].mURL.Chars());
			WideCharToMultiByte(CP_UTF8, 0, image_url_str.Chars(), wcslen(image_url_str.Chars()), mUtfImageURL, MAX_UTF_STRING_SIZE, NULL, NULL);
		}
		else
		{
			mUtfImageURL[0]=0;
		}
		
		WideCharToMultiByte(CP_UTF8, 0, C_SocialNetworkManager::Instance().GetCurrentItemText(), wcslen(C_SocialNetworkManager::Instance().GetCurrentItemText()), mUtfName, MAX_FB_UTF_STRING_SIZE, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, UI::C_TextStringID(sni->facebook_story.caption.id).String(),	wcslen(UI::C_TextStringID(sni->facebook_story.caption.id).String()), mUtfCaption, MAX_UTF_STRING_SIZE, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, UI::C_TextStringID(sni->facebook_story.description.id).String(),wcslen(UI::C_TextStringID(sni->facebook_story.description.id).String()), mUtfDescription, MAX_FB_UTF_DESCRIPTION_SIZE, NULL, NULL);
		WideCharToMultiByte(CP_UTF8, 0, UI::C_TextStringID(sni->facebook_story.image_href.id).String(), wcslen(UI::C_TextStringID(sni->facebook_story.image_href.id).String()), mUtfImageHRef, MAX_UTF_STRING_SIZE, NULL, NULL);
#endif
		mAsyncTask = facebook->post(mUtfMessage, mUtfName, mUtfCaption, mUtfDescription, mUtfImageURL, mUtfImageHRef);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gFacebookMessageLogger,"Facebook - Posting Started\n");

			mCurrentTaskType=FACEBOOK_TASK_TYPE_POST;

			return true;
		}
	}

	return false;
}

bool C_FacebookManager::UploadPhoto(U64 id, const WCHAR *caption)
{
	if(mTimeOutTimer)
		return false;

	bdFacebook *facebook = RacerNet::C_NetServices::Instance().Router().GetFacebook();

	KillCurrentTask();

	if(facebook)
	{
		memset(mUtfCaption,0,sizeof(mUtfCaption));

#ifdef PLATFORM_PS3
		size_t len  = MAX_UTF_STRING_SIZE;

		size_t wlen = wcslen( caption );
		L10nResult res = UCS2stoUTF8s( (uint16_t*)caption, &wlen, (uint8_t*)mUtfCaption, &len );
#else
		WideCharToMultiByte(CP_UTF8, 0, caption, wcslen(caption), mUtfCaption, MAX_UTF_STRING_SIZE, NULL, NULL);
#endif

		mAsyncTask = facebook->uploadPhoto(id, mUtfCaption);

		// did the task start
		if (NetPlay::DemonwareRemoteTask::IsBusy(mAsyncTask))
		{
			LOGINFO( gFacebookMessageLogger,"Facebook - Posting Started\n");

			mCurrentTaskType=FACEBOOK_TASK_TYPE_UPLOAD_PHOTO;

			return true;
		}
	}

	return false;
}

int C_FacebookManager::GetImageURL(const char *name)
{
	for(int a=0;a<mImageURLStore.Size();a++)
	{
		if(!strcmp(name,mImageURLStore[a].mKeyword))
		{
			return a;
		}
	}
	
	return -1;
}

bool C_FacebookManager::PumpCurrentTask(int *error)
{
	bdFacebook *facebook = RacerNet::C_NetServices::Instance().Router().GetFacebook();

	if(facebook)
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
				LOGINFO( gFacebookMessageLogger,"Facebook - Task Failed (%d)\n",mAsyncTask ? mAsyncTask->getErrorCode() : 0);
				*error = e;
				return false;
			}
			break;

		case NetPlay::DemonwareRemoteTask::eDone:
			if (mAsyncTask->getErrorCode() == BD_NO_ERROR)
			{
				// successful
				if(mCurrentTaskType==FACEBOOK_TASK_TYPE_REGISTER)
				{
					mHasRegistered.m_isRegistered=true;
					mAuthenticated=true;
				}
				else if(mCurrentTaskType==FACEBOOK_TASK_TYPE_UNREGISTER)
				{
					mHasRegistered.m_isRegistered=false;
					mAuthenticated=false;
				}
				else if(mCurrentTaskType==FACEBOOK_TASK_TYPE_POST || mCurrentTaskType==FACEBOOK_TASK_TYPE_UPLOAD_PHOTO)
				{
					mTimeOutTimer=mTimeOut;
				}

				LOGINFO( gFacebookMessageLogger,"Facebook - Task Completed and Successful\n");
				*error = BD_NO_ERROR;

#ifndef CFG_GOLD
				if(g_debugFacebookInjectGeneralError)
				{
					g_debugFacebookInjectGeneralError=false;
					*error=g_debugFacebookInjectGeneralErrorEnum;
				}
#endif

			}
			else
			{
				// failed
				int e=mAsyncTask ? mAsyncTask->getErrorCode() : BD_HANDLE_TASK_FAILED;

				LOGINFO( gFacebookMessageLogger,"Facebook - Task Completed but Failed\n");
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

void C_FacebookManager::KillCurrentTask()
{
	LOGINFO( gFacebookMessageLogger,"Facebook - Killing Any Current Task\n");

	mCurrentTaskType=FACEBOOK_TASK_TYPE_NOTHING;
	NetPlay::DemonwareRemoteTask::Close(mAsyncTask);
}

void C_FacebookManager::CheckPublisherFile()
{
	NetPlay::DemonwareRemoteTask::Close(mPublisherAsyncTask);

	mAllChecksDone=false;
	//mHasPublisherFile=false;

	if (RacerNet::C_NetServices::Instance().IsInitialised())
	{
		RacerNet::C_NetServices::Instance().Router().EnableContentStreaming(true);
	}

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();

	if(streaming)
	{
		mPublisherAsyncTask = streaming->listAllPublisherFiles(0,2,&mPublisherFileDataResults,1);

		if (NetPlay::DemonwareRemoteTask::IsBusy(mPublisherAsyncTask))
		{
			mServerTimeOut=0.f;
			mCheckingForPublisherFile=true;
			return;
		}
	}

	mAllChecksDone=true;
}

void C_FacebookManager::CheckImageURLFile()
{
	NetPlay::DemonwareRemoteTask::Close(mPublisherAsyncTask);

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();

	if(streaming)
	{
		mPublisherAsyncTask = streaming->listAllPublisherFiles(0,3,&mImageURLFileDataResults,1);

		if (NetPlay::DemonwareRemoteTask::IsBusy(mPublisherAsyncTask))
		{
			mCheckingForImageURLFile=true;
			return;
		}
	}

	mAllChecksDone=true;
}

void C_FacebookManager::DownloadImageURLFile()
{
	NetPlay::DemonwareRemoteTask::Close(mPublisherAsyncTask);

	bdContentStreaming *streaming = RacerNet::C_NetServices::Instance().GetContentStreamingServer();

	if(streaming)
	{
		if(mImageURLs)
		{
			delete[] mImageURLs;
		}

		mImageURLs = new U8[mImageURLFileDataResults.m_fileSize];

		mPublisherAsyncTask = streaming->downloadPublisherFile(mImageURLFileDataResults.m_fileID,mImageURLs,mImageURLFileDataResults.m_fileSize,&mImageURLFileDownload);
	
		if (NetPlay::DemonwareRemoteTask::IsBusy(mPublisherAsyncTask))
		{
			mDownloadingImageURL=true;
			return;
		}
	}

	mAllChecksDone=true;
}

}; // namespace Game
