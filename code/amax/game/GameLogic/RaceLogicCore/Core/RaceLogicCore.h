#ifndef RACELOGICCORE_H
#define RACELOGICCORE_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined(USER_CASPINALL) || defined(USER_PKERBY) || defined(USER_WBROWN) || defined(USER_PSNAPE) || defined(USER_DGILL) || defined(USER_TSTOCKTON) || defined(USER_JSANCHEZ) || defined(USER_SBILTON) || defined(USER_DWOOD)
#ifdef _MSC_VER
	#define RLC_DEBUG
#endif
#endif

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <GameApp.h>


//TODO Get rid of this horribleness using components...
#include "../Data/RLDataStructures.h"
#include "../Components/IRLComponent.h"

#include "../../Structures/VehicleRaceStatistics.h"
#include "../../Structures/RaceStatistics.h"


//FORWARD DECLARATIONS
class RLDatabase;
class C_VehicleCollisionCallback;
struct RLSystem;
struct RLPickupType;
enum E_PICKUP_TYPE;
struct AmaxVehicle;

namespace RaceBook { class C_RaceBookVehicle; }

namespace Game
{
class C_GameWorld;

namespace Actors
{

///////////////////////////////////////////////////////////////////////////////
//RLC ACTOR CREATE
///////////////////////////////////////////////////////////////////////////////
class RaceLogicCoreActorCreate : public ::Actors::C_GameActorCreate
{
public:

	static Actor::C_ActorTypeId					ClassType() { return E_ActorTypeId_RaceLogicCore; }
	const Actor::C_ActorTypeId					GetType() const { return E_ActorTypeId_RaceLogicCore; }

	C_GameWorld									*m_pGameWorld;
};
///////////////////////////////////////////////////////////////////////////////
//RLC ACTOR CREATE
///////////////////////////////////////////////////////////////////////////////

class RaceLogicCore : public C_GameActor
{
//Friends
	friend class								RLDatabaseController;
	friend class								RLDebugLineController;

//Functions
public:
	RaceLogicCore();
	~RaceLogicCore();

	static const Actor::C_ActorTypeId			TypeId() { return E_ActorTypeId_RaceLogicCore; }
	static const Btl::String32					TypeName() { return "RaceLogicCore"; }

	const RaceLogicCoreActorCreate*				GetCreationData() const;

	C_GameWorld									*GetGameWorld() const;

	void										RenderDebugLines();
	void										Reset();

	const float									GetFrameTime() const;
	const float									GetRaceTime() const;

	void										SwitchUIFocusCar();
	void										ShowLockedCarSFX();
	void										HideLockedCarSFX();
	bool										ShouldLoadVehicle(AmaxVehicle* p_pVehicle);
	void										StartVehicleLoad( RaceBook::C_RaceBookVehicle* pRaceBookVehicle );
	void										StartAmaxVehicleLoad( AmaxVehicle* pAmaxVehicle, bool bMultiplayer, U32 uTeamColour = 0 );
	void										SetVehicleLoadingHandle(Actor::C_ActorHandle p_Handle);
	void										HandleVehicleLoadedUI(bool p_Status);
	void										LoadNewVehicle(S32 p_Slot);

	const RLPickupType							*GetPickupType(const enum E_PICKUP_TYPE ePickupType) const;
	XtHandle<RLPickupType>						GetPickupHandle(const enum E_PICKUP_TYPE ePickupType);

	const U32									GetNumVehicles() const;

	const S8									GetPlayerVehicleIndex(const int controllerIndex = 0) const; //Use this to work out which is the player's vehicle for the control pad
	const S8									GetVehicleIndexFromActorHandle(const Actor::C_ActorHandle& actorHandle) const;
	const S8									GetVehicleIndexFromActorHandle(const Actor::C_ActorHandle* pActorHandle) const;
	const Actor::C_ActorHandle					GetActorHandleFromVehicleIndex(const U8 vehicleIndex) const;
	C_VehicleActor								*GetVehicleActor(const U8 vehicleIndex) const;

	const Vehicle::C_ModelInput					*GetVehicleModelInput(const U8 vehicleIndex) const;

	const U32									GetVehicleRevs(const U8 vehicleIndex) const;
	const U32									GetVehicleRedLine(const U8 vehicleIndex) const;

	const S8									GetVehicleGear(const U8 vehicleIndex) const;

	const PickupPositionArray&					GetPickupPositions() const;

	const S8									GetViewportFocusVehicleIndex(const U8 viewportIndex) const;
	void										SetViewportFocusVehicleIndex(const U8 viewportIndex, const U8 vehicleIndex);
	const S8									GetViewportControllerIndex(const U8 viewportIndex) const;
	const S8									GetViewportIndex(const U8 vehicleIndex) const;

	const bool									IsVehicleRaceStarted(const U8 vehicleIndex) const;		//Defines if the vehicle's crossed the start line
	const bool									IsVehicleRaceFinished(const U8 vehicleIndex) const;		//Defines if the vehicle's crossed the finish line
	const bool									IsVehicleLoaded(const U8 vehicleIndex) const;			//Is this vehicle ready to race

	const bool									IsVehicleAIControlled(const U8 vehicleIndex) const;
	const bool									IsVehiclePlayerControlled(const U8 vehicleIndex) const;
	const bool									ArePickupsEnabled() const;

	const bool									IsAIDataAnalysed() const;								//Has the AI data been received and analysed
	const bool									IsSplitDataAnalysed() const;							//Has the split data been received and analysed

	//Database
	::RLDatabase								*GetDatabase() const;
	const RLSystem								*GetRLSystem() const;

	//Misc
	const bool									IsPaused() const;
	void										SetPaused(const bool paused);

	void										SkipFrames(S32 p_NumFrames);
	//Misc


	//Messaging *KEEP THIS AT THE BOTTOM*	
	bool										OnMessage(const Messaging::C_MessageBase *pMessage);
	//Messaging *KEEP THIS AT THE BOTTOM*

	inline const bool							VehicleValid(int vehicleIndex) const
	{
		if(vehicleIndex >= 0 && vehicleIndex < mVehicles.Size())
			return true;
		else
			return false;
	}


	RLTrackData									*GetTrackData() const;

	const RLStartPosition						GetStartPosition(const int startNumber, const enum E_STARTPOSITION_TYPE eStartPositionType = E_STARTPOSITION_TYPE_STANDARD);
	const RLStartPosition&						GetRandomStartPosition(const enum E_STARTPOSITION_TYPE eStartPositionType = E_STARTPOSITION_TYPE_STANDARD) const;
	const RLStartPosition&						GetBestStartPosition( const int vehicleIndex, const enum E_STARTPOSITION_TYPE eStartPositionType = E_STARTPOSITION_TYPE_STANDARD) const;
	const int									GetNumberOfStartPositionsOfType(const enum E_STARTPOSITION_TYPE eStartPositionType) const;

	void										SpawnVehicleActors(bool bDelayPositioning = false);

//Functions
protected:
	bool										InitialiseImpl();
	bool										CreateImpl();


//Functions
private:

	void										ReloadSorceryData();

	void										CreateAIDriver(	Game::Actors::C_VehicleActor *pActor, 
																Util::C_String &AIname, 
																Util::C_String &Vehiclename, 
																S32 index, 
																S32 steeringIndex);

	void										ReLoadSorceryDataAIDrivers();
	void										ReLoadSorceryDataAIDriver(Game::Actors::C_VehicleActor *pActor, Util::C_String &AIname, Util::C_String &Vehiclename,  U32 index, U32 steeringIndex, int num);


	inline const bool							ViewportValid(int viewportIndex) const
	{
		if(viewportIndex >= 0 && viewportIndex < mViewportDetails.Size())
			return true;
		else 
			return false;
	}

	void										Initialise();
	void										InitialiseDataStructures();
	void										InitialiseControllers();
	void										InitialiseViewportDetails();

	void										CheckInitialisation();

	void										SetState(const enum E_RLC_STATE eNewState);
	const enum E_RLC_STATE						GetState() const;

	void										Delete();
	void										DeleteDataStructures();
	void										DeleteControllers();

	//
	void										ProcessUpdate();
	void										PausedUpdate();
	void										CheckDebugKeyCombos();
	void										ProcessDebugUpdate();
	void										FocusOnPreviousVehicle();
	void										FocusOnNextVehicle();

	void										FrameSkipUpdate();
	void										PreUpdate();
	void										Update();
	void										PostUpdate();

	void										Render();

	void										SendControls();

	void										PlaceVehicles();

	void										NotifyCameraSystemAboutViewports();
	const int									DetermineBestSpawnPointIndex(const int numberOfSpawnPoints, const int spawningVehicleIndex) const;
	const U32									GenerateRandomSpawnPointIndex(const int numberOfStartPositions) const;

	//Data structures
	
	RLTimeData									*GetTimeData() const;
	


//Variables
public:
	F32											m_RaceLoadTime;				// Exact time stamp when everything was loaded


//Variables
private:
	C_GameWorld									*m_pGameWorld;	
	
	//Data 
	RLTrackData									*m_pTrackData;
	RLTimeData									*m_pTimeData;

	//Component systems...like controllers and controller/processors from Amax Prototype
	Btl::Vector<IRLComponent*, Btl::VectorParameters_Static<E_RLCOMPONENTTYPE_MAX> > mComponents;

	Btl::Vector<RLVehicleData, Btl::VectorParameters_Static<MAX_VEHICLES> > mVehicles;
	Btl::Vector<RLVehicleData, Btl::VectorParameters_Static<MAX_VEHICLES> > mLastVehicles;
	Actor::C_ActorHandle						m_LoadingVehicleHandle;
	
	Btl::Vector<RLViewportDetails, Btl::VectorParameters_Static<MAX_VIEWPORTS> > mViewportDetails;

	enum E_RLC_STATE							meState;

	S32											m_UnPauseForFrames;

	SpecialFX::I_SpecialFXInstance*				m_pGarageLockedCarSFX;
	Maths::HMat43								m_GarageSFXMatrix;
	float										m_fGarageSFXTime;
};

}
}

#endif //RACELOGICCORE_H;
