#pragma once

#include <misc_shared/Messaging/Messaging.h>
#include <framework/application/ApplicationTypes.h>

namespace Game
{

enum E_XTPACK
{
	E_XTPACK_NONE = -1,

	E_XTPACK_GAMEDATA, 
	E_XTPACK_RACELOGICDATABASE, 

	E_XTPACK_AIR,								//Racing AI data

	E_XTPACK_VEHICLE,
	E_XTPACK_DRIVER,							// Deprecated - will be removed soon
	E_XTPACK_DRIVERANIMATION_ANIMTREES,
	E_XTPACK_DRIVERANIMATION_CYCLES,
	E_XTPACK_DRIVERANIMATION_CYCLEGROUPS,

	E_XTPACK_PICKUPS,

	E_XTPACK_CROWD,
	E_XTPACK_SPLITSCREEN,

	E_XTPACK_SCORING,
	E_XTPACK_ACHIEVEMENTS,
	E_XTPACK_SOCIAL_NETWORKING,
	E_XTPACK_DLC_OFFER_E,

	E_XTPACK_SINGLEPLAYER,
	E_XTPACK_GLOBAL_ACTOR_LIST,
	E_XTPACK_RACEBOOK, 

	//DON'T USE THIS
	E_XTPACK_MAX
	//DON'T USE THIS
};

struct GameXtPack
{
	void Initialise()
	{
		meType = E_XTPACK_NONE;
		mXtAmaxPack = 0;
		mXtPackFilePath = "";
		mFileTime = 0;
		mLoaded=false;
	}

	E_XTPACK									meType;
	XtPack*										mXtAmaxPack;
	Util::C_String								mXtPackFilePath;
	Time::C_Time								mFileTime;
	bool										mLoaded;
};

class GameXtLoader : public Messaging::C_MessageHandler
{
public:

	void Init();
	void Update();

	// For game messages
	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	static GameXtLoader& Instance() {return s_instance;}

	bool										AddPack(E_XTPACK type,Util::C_String pack_name);
	GameXtPack*									GetPack(E_XTPACK type);

	HRESULT										LoadXt();
	HRESULT										ReLoadXt();

private:

	static GameXtLoader s_instance;

	HRESULT										LoadXtPackFile(GameXtPack* pGamePack);
	HRESULT										ReLoadXtPackFile(GameXtPack* pGamePack, int& noXtPacksReloaded);

	Btl::Vector<GameXtPack, Btl::VectorParameters_Static<E_XTPACK_MAX> > mGameXtPacks;
};

} // namespace Game