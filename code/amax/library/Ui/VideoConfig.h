#ifndef UI_VIDEOCONFIG_H
#define UI_VIDEOCONFIG_H

struct GameModeVideo
{
	XtSz8	m_Video;
	bool	m_CanSkip;
	bool	m_Fullscreen;
};

struct GameModeVideoConfig
{
	GameModeVideo m_GlossyIntro;
	GameModeVideo m_RaceMode;
	GameModeVideo m_DestructionMode;
	GameModeVideo m_CheckpointMode;
	GameModeVideo m_FanMode;
	GameModeVideo m_OneOnOne;
	GameModeVideo m_PowerUps;
};

struct TutorialsVideoConfig
{
	GameModeVideo m_Tutorial1;
	GameModeVideo m_Tutorial2;
};

struct FanModeVideoConfig
{
	GameModeVideo m_FanPar;
	GameModeVideo m_FanRun;
	GameModeVideo m_FanDemand;
};

struct EndOfGameVideoConfig
{
	GameModeVideo m_EndCredits;
	GameModeVideo m_Phonecall;
};

struct VideoConfig
{
	GameModeVideoConfig				m_GameModes;
	TutorialsVideoConfig			m_Tutorials;	
	FanModeVideoConfig				m_FanModes;
	EndOfGameVideoConfig			m_EndOfGame;
	XtArray<GameModeVideo>			m_TierUnlock;
	XtArray<GameModeVideo>			m_BossUnlock;
	XtArray<GameModeVideo>			m_BossBeaten;
};

extern VideoConfig*	gUiVideoData;

#endif // UI_VIDEOCONFIG_H