#ifndef LASERSIGHT_ABILITY_H
#define LASERSIGHT_ABILITY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessageHandler.h>

struct MiscModSettings;

namespace Gravity
{
	class C_Scene;
}

namespace Game
{

class C_LevelStateMultiplayerVehicleAbilities;

class C_LasersightAbility : public Messaging::C_MessageHandler
{
	public:
		C_LasersightAbility();
		void										Init(U8 vehicleIndex, C_LevelStateMultiplayerVehicleAbilities* multiplayerAbilities, Gravity::C_Scene* physicsScene, MiscModSettings* modSettings, S8 viewportIndex);
		bool										MessageHandler(const Messaging::C_MessageBase* pMessage);
		void										Activate(bool activate);

	private:
		void										Update(float timeStep);
		
		C_LevelStateMultiplayerVehicleAbilities*	mpMultiplayerAbilities;
		Gravity::C_Scene*							mPhysicsScene;
		MiscModSettings*							mSettings;

		U8											mVehicleIndex;
		S8											mViewportIndex;
		bool										mActive;
};

}// end namespace Game

#endif //#ifndef LASERSIGHT_ABILITY_H