#include "stdafx.h"

#include <GameLogic/SubtitleManager.h>
#include <GameLogic\GameData\GameEventData.h>
#include <GameApp.h>

Debug::Logger &g_SubtitleLogging( Debug::GetLogger("Amax.Subtitles") );

namespace Game
{

C_SubtitleManager C_SubtitleManager::s_instance;

void C_SubtitleManager::StartSubtitles( const char* pEventName )
{
/*
	mShowSubtitleIndex=false;
	mPlaySubtitles=true;
	mTime=0.f;
	mSubtitleIndex=0;

	// try and find the event
	m_pAmaxEvent = NULL;

	if ( pEventName && ( strlen( pEventName ) > 0 ) )
		m_pAmaxEvent = C_GameEventData::Instance().mEvents[ pEventName ];
*/
}

void C_SubtitleManager::EndSubtitles()
{
/*
	mShowSubtitleIndex=false;
	mPlaySubtitles=false;
	m_pAmaxEvent = NULL;
*/
}

const WCHAR *C_SubtitleManager::GetSubtitle()
{
/*
	if(!mPlaySubtitles)
	{
		return L"";
	}

	AmaxEvent *ev = m_pAmaxEvent;

	if(ev == NULL)
	{
		return L"";
	}

	if(!mShowSubtitleIndex)
		return L"";
	else
		return UI::C_TextStringID(ev->mSubtitles[mSubtitleIndex].subtitle.id).String();
*/
		return L"";
}

void C_SubtitleManager::Update()
{
/*
	if(!mPlaySubtitles)
	{
		return;
	}

	AmaxEvent *ev = m_pAmaxEvent;

	if(ev == NULL)
	{
		return;
	}

	if(ev->mSubtitles.count == 0)
	{
		return;
	}

	mShowSubtitleIndex=false;
	mTime += C_GameApp::GetInstance()->GetTimeStep();

	for(int a=0;a<ev->mSubtitles.count;a++)
	{
		if(ev->mSubtitles[a].life)
		{
			if(mTime >= ev->mSubtitles[a].starttime && mTime <= ev->mSubtitles[a].starttime+ev->mSubtitles[a].life)
			{
				mShowSubtitleIndex=true;
				mSubtitleIndex=a;
			}
		}
		else
		{
			if(a==ev->mSubtitles.count-1)
			{
				if(mTime >= ev->mSubtitles[a].starttime)
				{
					mShowSubtitleIndex=true;
					mSubtitleIndex=a;
				}
			}
			else if(mTime >= ev->mSubtitles[a].starttime && mTime <= ev->mSubtitles[a+1].starttime)
			{
				mShowSubtitleIndex=true;
				mSubtitleIndex=a;
			}
		}
	}
*/
}

}; // namespace Game
