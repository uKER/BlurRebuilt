#ifndef FACEBOOK_MANAGER_H
#define FACEBOOK_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <bdLobby\bdFacebook\bdFacebook.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdContentStreaming/bdContentStreaming.h>

#include <Gamelogic\SocialNetworking\SocialNetworkManager.h>

namespace Game
{
	const int FACEBOOK_SERVER_TIMEOUT=15;
	const int MAX_FB_UTF_STRING_SIZE=256;
	const int MAX_FB_UTF_DESCRIPTION_SIZE=512;

	enum FACEBOOK_TASK_TYPE
	{
		FACEBOOK_TASK_TYPE_NOTHING,

		FACEBOOK_TASK_TYPE_REGISTER,
		FACEBOOK_TASK_TYPE_UNREGISTER,

		FACEBOOK_TASK_TYPE_POST,
		FACEBOOK_TASK_TYPE_UPLOAD_PHOTO,
	};

	typedef struct AmaxImageURL
	{
		Util::C_String32 mKeyword;
		Util::C_String128 mURL;
	}AmaxImageURL;

	class C_FacebookManager
	{
	public:

		C_FacebookManager()
		{
			mHasPublisherFile=false;
			mImageURLs=NULL;
		};

		void Init();
		void Update();

		void SetUsername(const WCHAR *user);
		void SetPassword(const WCHAR *pass);

		const WCHAR *GetUsername();
		const WCHAR *GetPassword();

		bool IsAuthenticated();

		void SetAuthenticated(bool auth){mAuthenticated=auth;};
		void Authenticate();
		void Unauthenticate(){mAuthenticated=false;};
		
		bool Publish();
		bool UploadPhoto(U64 id, const WCHAR *caption);

		int GetImageURL(const char *name);

		bool PumpCurrentTask(int *error);
		void KillCurrentTask();

		static C_FacebookManager& Instance(){return s_instance;};
		
		float GetTimeout(){return mTimeOutTimer;};

		void CheckPublisherFile();
		bool HasPublisherFile(){return mHasPublisherFile;};
		bool HasPublisherFileCheckFinished(){return mAllChecksDone;};

		void CheckImageURLFile();
		void DownloadImageURLFile();

		void ParseImageURLFile();

		bool HasRegistered(){return mHasRegistered.m_isRegistered;};
		void UnRegister();

	private:
		static C_FacebookManager s_instance;

		bdRemoteTaskRef mAsyncTask;

		bdRemoteTaskRef mPublisherAsyncTask;
		bool mHasPublisherFile;
		bool mCheckingForPublisherFile;
		bdFileMetaData mPublisherFileDataResults;

		FACEBOOK_TASK_TYPE mCurrentTaskType;
		
		float mDeltaTime;

		bool mAuthenticated;

		int mTimeOut;
		float mTimeOutTimer;

		U8 *mImageURLs;
		bool mCheckingForImageURLFile;
		bdFileMetaData mImageURLFileDataResults;
		bdFileMetaData mImageURLFileDownload;
		bool mDownloadingImageURL;

		bool mAllChecksDone;

		float mServerTimeOut;

		char mUtfUsername[MAX_FB_UTF_STRING_SIZE];
		char mUtfPassword[MAX_FB_UTF_STRING_SIZE];

		char mUtfMessage[MAX_FB_UTF_STRING_SIZE];
		char mUtfName[MAX_FB_UTF_STRING_SIZE];
		char mUtfCaption[MAX_FB_UTF_STRING_SIZE];
		char mUtfDescription[MAX_FB_UTF_DESCRIPTION_SIZE];
		char mUtfImageURL[MAX_FB_UTF_STRING_SIZE];
		char mUtfImageHRef[MAX_FB_UTF_STRING_SIZE];

		Btl::Vector<AmaxImageURL> mImageURLStore;

		// checks for is registered
		void CheckIsRegistered();
		bdRemoteTaskRef mRegisteredAsyncTask;
		bdFacebookIsRegistered mHasRegistered;
		bool mCheckingHasRegistered;
	};

} // namespace Game

#endif // FACEBOOK_MANAGER_H