#ifndef ACTORUTILS_H
#define ACTORUTILS_H

// #pragma once


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>

#include <Game/GameWorld/GameWorld.h>
#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>
#include <btl/Containers/containers.h>

#include <actor_system/Actor.h>
#include <GameLogic/ActorStudioActors/DynamicObjectActor.h>

#include <resource_utils/GameModel.h>

#include <Gravity/SDK/ShapeDescriptors.h>
#include <Gravity/Triggers/TriggerSystem.h>

#include <game_resource/LevelSceneManager.h>

#include <system/Misc/Random.h>

namespace Actors
{

enum E_FORCE_EFFECT
{
	E_FORCE_EFFECT_LARGE_EXPLOSION,
	E_FORCE_EFFECT_SMALL_EXPLOSION,
	E_FORCE_EFFECT_SHOCK,
	E_FORCE_EFFECT_SHUNT_FIRE,
	E_FORCE_EFFECT_SHUNT_EXPLODE,
	E_FORCE_EFFECT_BARGE,
	E_FORCE_EFFECT_MINE,
};

//=========================================================================


class C_ActorUtils
{
//	friend class Actor::C_ActorList;

public:

	static C_ActorUtils& Instance() { return s_Instance; }
	
	C_ActorUtils();
	~C_ActorUtils();

	void				SetGameWorldPtr(Game::C_GameWorld* pGameWorld) { gameworld = pGameWorld; }
	void				SetLevelInstancePtr(GameResource::C_LevelInstance* plevelinstance) { levelinstance = plevelinstance; }
	
	void				RenderDynamicObjectDebugInfo();			// Displays dynamic object debug info if enabled

	void				AlignToScene(Maths::HMat43 *mat, Game::E_ALIGNMENT_TYPE aligntype, F32 range, Gravity::C_Scene *physics_scene);				// Moves mat so it's positioned on the ground or a wall etc
	bool				CheckCollision(Maths::HVec3 *start, Maths::HVec3 *end,  Maths::HVec3 *collision, Maths::HVec3 *normal = NULL);

	void				SanityCheck_Initialise();
	void				SanityCheck_Register(Game::Actors::C_GameActor* newactor);
	void				SanityCheck_Remove(Game::Actors::C_GameActor* newactor);
	void				SanityCheck_DynamicObjects();			// Makes sure we don't get dynamic objects in silly places or overlapping
#ifndef CFG_SPEED
	void				CheckForLeftOverPhysicsBodies(Gravity::C_Scene* pPhysicsScene, const Actor::C_ActorInfos& actorInfos, const Actor::C_ActorFactory& actorFactory);
#endif

	ResourceUtils::C_GameModel	*GetActorModel(Game::Actors::C_GameActor* actor);

	void				RegisterForceEffectWithActors(Maths::HMat43 *mat, U32 type);
	void				CallActorsWithinSphere(Maths::HVec3 *pos, F32 rad, void *data);
	
	F32					GetRandomNum() { return rand.FRand(); };

//	void				DropDynamicObject();
//	DynamicObjectActor	*SpawnDynamicObject(Maths::HMat43 *mat, XtHandle<Game::DynamicObjectDef> objdef);

	void				NormaliseMat(Maths::HMat43 *mat);
	void				NormaliseMatXYZ(Maths::HMat43 *mat);
	void				NormaliseMatXYZ(Maths::HMat33 *mat);

	S32					NumDynamicActors() const;
	S32					NumPhysicalActors() const;

//	Gravity::C_Trigger*	CreateTrigger(Actor::C_ActorHandle handle, Maths::HVec3 extents, Maths::HMat43 worldMat);
	Gravity::C_Trigger*	CreateTrigger(Actor::C_ActorHandle handle, Maths::HMat43 worldmat, XtBoundingBox bbox);
	void				DestroyTrigger(Gravity::C_Trigger* triggerobject);
	void				RemoveDestroyedTriggers();

	S32		m_NumDeletedtriggers;								// How many actors are in the current list
	Btl::Vector<Gravity::C_Trigger*,Btl::VectorParameters_Array<MAX_NUM_TRIGGERS> >	m_pDeletedTriggers;					// Contains pointers to all triggers that need deleting

	S32		m_NumActors;								// How many actors are in the current list
	Btl::Vector<Game::Actors::C_GameActor*,Btl::VectorParameters_Array<MAX_NUM_ACTORS> >	m_pObjectActors;			// Contains pointers to all actors (Note: does not monitor actor deaths etc)



	Game::C_GameWorld				*gameworld;
	GameResource::C_LevelInstance	*levelinstance;

#ifndef CFG_GOLD
	S32		m_NumPhysicalObjectActors;
	S32		m_NumDynamicObjectActors;
	S32		m_OriginalNumPhysicalObjectActors;
	S32		m_OriginalNumDynamicObjectActors;
#endif

private:

	static C_ActorUtils				s_Instance;					// Store single instance of this class

	Util::C_Random					rand;
};



}// End namespace Actors

#endif // ACTORUTILS_H
