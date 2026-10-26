#ifndef SUBTITLE_MANAGER_H
#define SUBTITLE_MANAGER_H

#include <GameLogic\GameData\GameEventData.h>

namespace Game
{

class C_SubtitleManager
{
public:

	C_SubtitleManager()
	{
		mPlaySubtitles=false;
		m_pAmaxEvent = NULL;
	};

	void StartSubtitles( const char* pEventName );
	void EndSubtitles();
	void Update();

	const WCHAR *GetSubtitle();

	static C_SubtitleManager& Instance(){return s_instance;};

private:

	static C_SubtitleManager s_instance;

	bool mPlaySubtitles;
	float mTime;
	
	int mSubtitleIndex;
	bool mShowSubtitleIndex;

	AmaxEvent* m_pAmaxEvent;
};

} // namespace Game

#endif // SUBTITLE_MANAGER_H