#ifndef TWITTER_MANAGER_H
#define TWITTER_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <bdLobby\bdTwitter\bdTwitter.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdContentStreaming/bdContentStreaming.h>

namespace Game
{
	const int TWITTER_SERVER_TIMEOUT=15;
	const int MAX_UTF_STRING_SIZE=256;

	enum TWITTER_TASK_TYPE
	{
		TWITTER_TASK_TYPE_NOTHING,

		TWITTER_TASK_TYPE_REGISTER,
		TWITTER_TASK_TYPE_UNRESISTER,

		TWITTER_TASK_TYPE_POST,
	};

	class C_TwitterManager
	{
	public:

		C_TwitterManager()
		{
			mHasPublisherFile=false;
		};

		void Init();
		void Update();

		void SetUsername(WCHAR *user);
		void SetPassword(WCHAR *pass);

		const WCHAR *GetUsername();
		const WCHAR *GetPassword();

		bool IsAuthenticated();

		void SetAuthenticated(bool auth){mAuthenticated=auth;};
		void Authenticate();
		void Unauthenticate(){mAuthenticated=false;};
		
		bool Tweet(const WCHAR *message);

		bool PumpCurrentTask(int *error);
		void KillCurrentTask();

		static C_TwitterManager& Instance(){return s_instance;};

		void CheckPublisherFile();
		bool HasPublisherFile(){return mHasPublisherFile;};
		bool HasPublisherFileCheckFinished(){return !mCheckingForPublisherFile;};

		float GetTimeout(){return mTimeOutTimer;};

	private:
		static C_TwitterManager s_instance;

		bdRemoteTaskRef mAsyncTask;

		bdRemoteTaskRef mPublisherAsyncTask;
		bool mHasPublisherFile;
		bool mCheckingForPublisherFile;
		bdFileMetaData mPublisherFileDataResults;

		TWITTER_TASK_TYPE mCurrentTaskType;
		
		float mDeltaTime;

		bool mAuthenticated;

		int mTimeOut;
		float mTimeOutTimer;

		float mServerTimeOut;

		char mUtfTranslation[MAX_UTF_STRING_SIZE];

		char mUtfUsername[MAX_UTF_STRING_SIZE];
		char mUtfPassword[MAX_UTF_STRING_SIZE];

		//need to add time to reset of API call count

	};

} // namespace Game

#endif // TWITTER_MANAGER_H