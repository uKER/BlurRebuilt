#ifndef __DRIFTER_ABILITY_H__
#define __DRIFTER_ABILITY_H__


#include <misc_shared/Messaging/MessageHandler.h>
#include <Game/GameActors/Components/VehicleLogic/Scoring/VLC_NewScoring.h>	// [Blur2PC]

namespace Components
{
	class C_VehicleComponent_NewScoring;
}
struct MultiplayerConfig;
// [Blur2PC] forward declarations for the pointer members below
namespace Game { class C_MultiplayerAbilities; class C_LevelStateMultiplayerVehicleAbilities; namespace Actors { class C_VehicleLogicActor; } }

namespace Game
{

class C_LevelStateMultiplayerVehicleAbilities;
class C_MultiplayerAbilities;


class C_DrifterAbility : public Messaging::C_MessageHandler
{
	public:
		C_DrifterAbility();
		void										Init(U8 vehicleIndex, const Game::C_MultiplayerAbilities* multiplayerAbilities, MultiplayerConfig* multiplayerConfig,Game::Actors::C_VehicleLogicActor* vehLogicActor, Components::C_VehicleComponent_NewScoring* scoringComponent, Game::C_LevelStateMultiplayerVehicleAbilities* vehicleAbilities );
		bool										MessageHandler(const Messaging::C_MessageBase* pMessage);
		


	private:
	

		static const float 							sDriftIntensitySmoothingUpwards;
		static const float 							sDriftIntensitySmoothingDownwards;
		static const float 							sDriftPercentageSmoothingUpwards;
		static const float 							sDriftPercentageSmoothingDownwards;

		void										Update(float timeStep);

		F32											CalculateDriftPercentage( F32 driftDistance ); 
		
		Game::C_LevelStateMultiplayerVehicleAbilities* mpMultiplayerVehicleAbilities;
		const Game::C_MultiplayerAbilities*			mpMultiplayerAbilities;
		MultiplayerConfig*							mpMpConfig;
		Game::Actors::C_VehicleLogicActor*			mpVehicleLogicActor;
		Components::C_VehicleComponent_NewScoring*	mpScoringComponent;
		U8											mVehicleIndex;
		int											mDrifterSpecialFX;
		F32											mDriftPercentageVel;
		F32											mDriftPercentage;
		F32											mDriftIntensity;
		F32											mDriftIntensityVel;
};

}// end namespace Game

#endif // __DRIFTER_ABILITY_H__