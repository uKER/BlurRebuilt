#pragma once

#include <misc_shared/Messaging/Messaging.h>
#include <GameLogic\GameData\GameData.h>

struct AmaxIntroSection;

namespace Game
{

class RaceIntroOutro : public Messaging::C_MessageHandler
{
public:

	void Init();
	void Reset();
	void PostUpdate();

	// For game messages
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	static RaceIntroOutro& Instance() {return s_instance;}

	bool PlayingIntro(){return mPlayingIntro;};
	bool PlayingOutro(){return mPlayingOutro;};

	void DoIntroSection(const AmaxIntroSection *section);

	void StartIntro();
	void StartOutro();

	void OverideControls();

	void FinishIntro();
	void FinishOutro();

private:

	static RaceIntroOutro s_instance;

	bool mPlayingIntro;
	bool mStartIntro;

	int mIntroSection;
	float mIntroSectionTimer;

	bool mPlayingOutro;
	bool mStartOutro;

	int mOutroSection;
	float mOutroSectionTimer;
};

} // namespace Game