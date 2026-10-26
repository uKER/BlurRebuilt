#include "stdafx.h"


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <Game/GameDef/UserData.h>
#include <shared_actors/XtActorBase.h>

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>

#include <GameLogic/Layers/AmaxWorldLayer.h>

#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

#include <Game/GameWorld/GameWorld.h>
#include <Game/Messaging/gamemessages.h>
#include <misc_shared/Messaging/Messaging.h>
#include <misc_shared/Messaging/GameMessaging.h>
#include <actor_system/HandleManager.h>

#include <actor_system/ActorList.h>

#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>
#include <gravity/narrowphase/Ray.h>

#include <Game/GameWorld/GameWorld.h>
#include <Game/GameDef/PhysicsConstants.h>
#include <gravity/sdk/BodyHelper.h>
#include <shared_actors/Components/Impl/PhysicsBodyComponent.h>
#include <Game/GameDef/Polyflags.h>

#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <game_resource/LevelResource.h>

#include <shared_actors/SharedActorUtils.h>

#include <game/AmaxCam/CameraSystem.h>
#include <game/AmaxCam/PhysicsChecks.h>
#include <game/AmaxCam/AmaxChaseCamera.h>

#include <GameLogic/ActorStudioActors/PhysicalObjectActor.h>

#include <GameLogic/ActorStudioActors/ResourceTypes.h>
#include <resource_utils/GameModel.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>

#include <shared_actors/actordef/BasePhysicalObjectDef.h>

#include "system/automation/Automation.h"
#include "actor_system/ActorFactory.h"

//========================== Debug stuff ==================================

/*
void DropDynamicObjectEvent(void*)							// Called when DebugView button is pressed
{
	Actors::C_ActorUtils::Instance().DropDynamicObject();
}

Debug::String	g_DropObjectID("DynamicObjects.Objects.DynamicObjectDefID","00F0F46E");
Debug::Function	g_DropObject("DynamicObjects.Objects.DropObject", DropDynamicObjectEvent, 0);
*/
//=========================================================================


extern Debug::Colour g_debugBackLightColour;
extern Debug::Vector g_debugBackLightDirection;


//#pragma optimize("", off)

namespace Actors
{

Debug::Bool g_debugDisplayModelUsage("DynamicObjects.DisplayModelUsage", false);


//=========================================================================
//			Actor callback stuff for getting all actor pointers
//=========================================================================

/*
struct C_ActorListVisitor_GetPtr : public Actor::C_ActorList::I_ActorListVisitor
{
	virtual bool VisitHandleItem(Actor::C_Actor* pItem)
	{
		C_ActorUtils::Instance().AddtoActorList(pItem);
		return true;
	}
};
*/

//=========================================================================
//					Actor Utility Routines
//=========================================================================


C_ActorUtils	C_ActorUtils::s_Instance;				// Our single instance


//=========================================================================

C_ActorUtils::C_ActorUtils()
{
	gameworld = NULL;
	SanityCheck_Initialise();
	m_NumDeletedtriggers = 0;
}

//=========================================================================

C_ActorUtils::~C_ActorUtils()
{
}


//=========================================================================
//						DEBUG RENDER
//=========================================================================


void C_ActorUtils::RenderDynamicObjectDebugInfo()
{
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(Maths::HVec3(0.0f, 0.0f, 0.0f), 0.25f, "The Origin");

	gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_DynamicObjMsg_RenderDebug );
}


//=========================================================================
//					ACTOR ALIGNMENT
//=========================================================================


void C_ActorUtils::AlignToScene(Maths::HMat43 *mat, Game::E_ALIGNMENT_TYPE aligntype, F32 range, Gravity::C_Scene *physics_scene)						// Moves mat so it's positioned on the ground or a wall etc
{
	F32					itrange;
	Maths::HVec3		start,end;
	Collision::C_Ray	ray;
	Collision::C_CollisionFilter rayCollisionFilter(Game::PolyFlags::kSolidNoLiquid, Game::PolyFlags::kSolidNoLiquid);				// Probably need to check ONLY for static scene geometry


	if (aligntype == Game::E_ALIGNMENT_TYPE_NONE)						// Forget if no alignment
		return;

	for(itrange=1.0f;itrange<=range;itrange+=1.0f)					// Try small distances first so we don't find the highest collision point
	{
		switch(aligntype)
		{
			case Game::E_ALIGNMENT_TYPE_VERTICAL:
				start = mat->Translation() + Maths::HVec3( 0.0f, itrange, 0.0f );
				end = mat->Translation() + Maths::HVec3( 0.0f, -itrange, 0.0f );
				break;

			case Game::E_ALIGNMENT_TYPE_LEFTRIGHT:				// TODO - NEED TO ALIGN TO MODEL ORIENTATION
				start = mat->Translation() + (mat->AX() * itrange);
				end = mat->Translation() - (mat->AX() * itrange);
				break;

			case Game::E_ALIGNMENT_TYPE_FORWARDBACK:
				start = mat->Translation() + (mat->AZ() * itrange);
				end = mat->Translation() - (mat->AZ() * itrange);
				break;
		}
		ray.InitialiseRay( start, end,  &rayCollisionFilter);
		
		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, 1024);
		physics_scene->DoRayTraceMesh(ray, stack);

		Collision::C_RayTracingResult& result = ray.m_result;

		if (result.Hit())													// If we've got a new position then adjust the mat translation
		{
			mat->Translation(result.HitPoint());
			return;
		}
	}
}


//=========================================================================


bool C_ActorUtils::CheckCollision(Maths::HVec3 *start, Maths::HVec3 *end, Maths::HVec3 *collision, Maths::HVec3 *normal)
{
	Gravity::C_Scene *physics_scene = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPhysicsScene();

	Collision::C_Ray				ray;
	Collision::C_CollisionFilter	rayCollisionFilter(Game::PolyFlags::kSolidNoLiquid, Game::PolyFlags::kSolidNoLiquid);				// Probably need to check ONLY for static scene geometry
//	Collision::C_CollisionFilter	rayCollisionFilter(Game::PolyFlags::kCameraSolid, Game::PolyFlags::kCameraSolid);


	ray.InitialiseRay(*start, *end,  &rayCollisionFilter);
	
	U8 data[1024 * sizeof(void*)];
	SharedPhysics::C_SafeStack<true> stack(data, 1024);
	physics_scene->DoRayTraceMesh(ray, stack);
	Collision::C_RayTracingResult& result = ray.m_result;

	if (result.Hit())													// If we've got a new position then adjust the mat translation
	{
		*collision = result.HitPoint();
		if (normal)
			*normal = result.Normal();
		return(true);
	}
	else
	{
		*collision = *start;
		return(false);
	}
}



//=========================================================================
//					ACTOR SANITY CHECKS
//=========================================================================


void C_ActorUtils::SanityCheck_Initialise()
{
#ifndef CFG_GOLD
	m_NumActors							= 0;
	m_NumPhysicalObjectActors			= 0;
	m_NumDynamicObjectActors			= 0;
	m_OriginalNumPhysicalObjectActors	= 0;
	m_OriginalNumDynamicObjectActors	= 0;
#endif
}

//=========================================================================

void C_ActorUtils::SanityCheck_Register(Game::Actors::C_GameActor* newactor)
{
#ifndef CFG_GOLD
	m_pObjectActors[m_NumActors++] = newactor;

	if (newactor)
	{
		if (newactor->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject)
			m_NumDynamicObjectActors++;	
		else if (newactor->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
			m_NumPhysicalObjectActors++;
	}
#endif
}

void C_ActorUtils::SanityCheck_Remove(Game::Actors::C_GameActor* newactor)
{
#ifndef CFG_GOLD
	if (newactor)
	{
		if (newactor->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject)
			m_NumDynamicObjectActors--;	
		else if (newactor->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
			m_NumPhysicalObjectActors--;
	}
#endif
}

#ifdef CFG_RELEASE
Debug::Bool g_debugSanityCheckObjects("Amax.Loading.SanityCheckObjects", true);
#else
Debug::Bool g_debugSanityCheckObjects("Amax.Loading.SanityCheckObjects", false);
#endif

//=========================================================================

void C_ActorUtils::SanityCheck_DynamicObjects()
{
#ifndef CFG_GOLD

	m_OriginalNumPhysicalObjectActors	= m_NumPhysicalObjectActors;
	m_OriginalNumDynamicObjectActors	= m_NumDynamicObjectActors;

	if (!g_debugSanityCheckObjects)
		return;
	S32										a,b;
	S32										errors = 0;
	Maths::C_Vertex							pos,pos2;
	bool									repeat;
//	const ResourceUtils::C_GameModel		*thismodel, *checkmodel;
	ResourceUtils::C_GameModel				*thismodel;

	LOGINFO( Resource::g_Log, "Total number of dynamic actors = %d", m_NumActors);
	
	for(a=0;a<m_NumActors;a++)
	{	
		pos = m_pObjectActors[a]->GetPosition();
		if ((pos.X() == 0.0f) && (pos.Y() == 0.0f) && (pos.Z() == 0.0f))
		{
			if (m_pObjectActors[a]->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject)
			{
				((DynamicObjectActor*)m_pObjectActors[a])->FlagError();
				LOGWARN( Resource::g_Log, "Dynamic Actor '%s' is at the origin", m_pObjectActors[a]->GetCreationData()->GetName());
				Automation::SendProblemFoundMsg("ActorStudio", "ActorAtOrigin");
			}
			else if (m_pObjectActors[a]->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
			{
				//m_pObjectActors[a]->FlagError();
				LOGWARN( Resource::g_Log, "Physical Object Actor '%s' is at the origin", m_pObjectActors[a]->GetCreationData()->GetName());
				Automation::SendProblemFoundMsg("ActorStudio", "ActorAtOrigin");
			}
			errors++;
		}
	}

	for(a=0;a<(m_NumActors-1);a++)
	{
		pos = m_pObjectActors[a]->GetPosition();
		for(b=a+1;b<m_NumActors;b++)
		{
			pos2 = m_pObjectActors[b]->GetPosition();
			if ((pos.X() == pos2.X()) && (pos.Y() == pos2.Y()) && (pos.Z() == pos2.Z()))
			{
				if (m_pObjectActors[a]->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject)
					((DynamicObjectActor*)m_pObjectActors[a])->FlagError();
				if (m_pObjectActors[b]->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject)
					((DynamicObjectActor*)m_pObjectActors[b])->FlagError();
				LOGWARN( Resource::g_Log, "Dynamic/PhysicalObject Actor '%s' & '%s' created at same position (%.f, %.f, %.f)", m_pObjectActors[a]->GetCreationData()->GetName(), m_pObjectActors[b]->GetCreationData()->GetName(), float(pos.X()), float(pos.Y()), float(pos.Z()));
				Automation::SendProblemFoundMsg("ActorStudio", "ActorsAtSamePosition");
				errors++;
			}
		}
	}

	if (errors > 0)
	{
		LOGWARN( Resource::g_Log, "%d possible errors", errors);
		Automation::SendProblemFoundMsg("ActorStudio", "PlacementErrorsFound");
	}
	else
		LOGDEBUG( Resource::g_Log, "No placement errors");


	if (g_debugDisplayModelUsage)
	{
		LOGINFO( Resource::g_Log, "Level Model Usage...");
		for(a=0;a<m_NumActors;a++)
		{
			thismodel = GetActorModel(m_pObjectActors[a]);
			repeat = false;
			for(b=0;b<a;b++)
			{
				if (GetActorModel(m_pObjectActors[b]) == thismodel)
				{
					repeat = true;
					break;
				}
			}

			if (!repeat)
				LOGINFO( Resource::g_Log, "    %s", GetActorModel(m_pObjectActors[a])->m_Filename.Text());
		}
	}
#endif
}

#ifndef CFG_SPEED
void C_ActorUtils::CheckForLeftOverPhysicsBodies(Gravity::C_Scene* pPhysicsScene, const Actor::C_ActorInfos& actorInfos, const Actor::C_ActorFactory& actorFactory)
{
	typedef Btl::Vector<Gravity::C_Body*> C_PhysicsBodies;
	
	C_PhysicsBodies gravityBodies;
	pPhysicsScene->GetLeftOverBodies(gravityBodies);

	U32 numActorBodies = 0;

	for (C_PhysicsBodies::ConstIterator i = gravityBodies.Begin(); i != gravityBodies.End(); ++i)
	{
		if ((*i)->GetUserData<void>() != NULL)
		{
			++numActorBodies;
		}
	}

	if (numActorBodies > 0)
	{
		LOGERROR(Game::g_Log, "%d physics bodies left over", numActorBodies);
		
		for (C_PhysicsBodies::ConstIterator i = gravityBodies.Begin(); i != gravityBodies.End(); ++i)
		{
			const Actor::C_Actor* pActor = NULL;
			Game::C_BodyUserDataBase* pUserData = (Game::C_BodyUserDataBase*)(*i)->GetUserData<void>();
			if (pUserData && pUserData->m_eType == Game::E_BodyUserDataType_Actor)
				pActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetActorPtr(((Game::C_BodyUserDataActor*)pUserData)->m_hActorHandle);

			if (pActor != NULL)
			{
				const Actor::C_ActorHandle handle = pActor->GetHandle();

				Actor::C_ActorInfos::ConstIterator j;
				
				for (j = actorInfos.Begin(); j != actorInfos.End(); ++j)
				{
					if (j->m_Handle == handle)
					{
						LOGINFO(Game::g_Log, "    %32s    %s", actorFactory.LookupName(j->m_TypeId).Chars(), j->m_Name);
						break;
					}
				}

				if (j == actorInfos.End())
				{
					LOGINFO(Game::g_Log, "    <UNKNOWN ACTOR> 0x%8x", static_cast<U32>(handle));
				}
			}
		}
	}
}
#endif

//=========================================================================

ResourceUtils::C_GameModel *C_ActorUtils::GetActorModel(Game::Actors::C_GameActor* actor)
{
	if (actor->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject)
		return((ResourceUtils::C_GameModel*)((DynamicObjectActor*)actor)->GetCreationData()->m_DynamicObjectDef->m_ModelType.Object());
	else if (actor->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
		return((ResourceUtils::C_GameModel*)((C_PhysicalObject*)actor)->GetElementRecord()->m_Model.Object());

	return NULL;
}

//=========================================================================

S32 C_ActorUtils::NumDynamicActors() const
{
	S32 count(0);
#ifndef CFG_GOLD
	for (S32 a=0; a < m_NumActors; a++)
	{	
		if (m_pObjectActors[a]->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject)
			count++;
	}
#endif
	return count;
}

//=========================================================================

S32 C_ActorUtils::NumPhysicalActors() const
{
	S32 count(0);
#ifndef CFG_GOLD
	for (S32 a=0; a < m_NumActors; a++)
	{	
		if (m_pObjectActors[a]->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
			count++;
	}
#endif
	return count;
}

//=========================================================================
//						ACTORS WITHIN A REGION
//=========================================================================


void C_ActorUtils::RegisterForceEffectWithActors(Maths::HMat43 *mat, U32 type)
{
	RLPickupForceEffects	*forces = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetDatabase()->mRLSystems[0]->mPickupForceEffects;
	RLForceEffect			*fe = NULL;
	S32						body;
	S32						bodyCount;
	Gravity::C_Scene		*pscene = gameworld->GetPhysicsScene();
	Actor::C_ActorHandle	handle;
	const Actor::C_Actor	*pActor;
	Maths::HVec3			actorpos;
	ActorMsg::C_DynamicObjectApplyImpulseMsg		msg;
	bool					valid;
	F32						dist;
	F32						force;
	Maths::HVec3			dir;
	F32						absx,absz;
	F32						duration;
	F32						weight;


	switch(type)			// Force Effect Type (Not force type which is area, forward etc)
	{
		case E_FORCE_EFFECT_LARGE_EXPLOSION:
			fe = &forces->m_LargeExplosion;
			break;

		case E_FORCE_EFFECT_SMALL_EXPLOSION:
			fe = &forces->m_SmallExplosion;
			break;

		case E_FORCE_EFFECT_SHOCK:
			fe = &forces->m_Shock;
			break;

		case E_FORCE_EFFECT_SHUNT_FIRE:
			fe = &forces->m_ShuntFire;
			break;

		case E_FORCE_EFFECT_SHUNT_EXPLODE:
			fe = &forces->m_ShuntExplode;
			break;

		case E_FORCE_EFFECT_BARGE:
			fe = &forces->m_Barge;
			break;

		case E_FORCE_EFFECT_MINE:
			fe = &forces->m_Mine;
			break;
	}


	Collision::C_Sphere sphere(mat->Translation(), fe->m_Radius);			// Get list of physics bodies within our sphere
	SharedPhysics::MemoryAllocator<Gravity::C_Body*> allocator;
	U8 stackData[1024 * sizeof(void*)];
	SharedPhysics::C_SafeStack<true> stack(stackData, 1024);
	bodyCount = pscene->GetIntersectingBodies(sphere, &Game::PhysicsConstants::kPerkCollisionFilter, allocator, stack);
	SharedPhysics::C_MemoryIterator<Gravity::C_Body*> iter(&allocator);
	for(body=0;body<bodyCount;body++)										// Loop through them all
	{
		Gravity::C_Body** pDoublePtr = iter.Get();
		Gravity::C_Body* pBody = *pDoublePtr;
		Game::C_BodyUserDataBase* pUserData = (Game::C_BodyUserDataBase*)pBody->GetUserData<void>();
		if (pUserData && pUserData->m_eType == Game::E_BodyUserDataType_Actor)
		{
			handle = ((Game::C_BodyUserDataActor*)pUserData)->m_hActorHandle;
			pActor = gameworld->GetActorList()->GetActorPtr(handle);
			if (pActor)
			{
				actorpos = pActor->GetPosition();
				weight = 99999;
				if (pActor->GetActorTypeId() == Game::E_ActorTypeId_DynamicObject)		// and if they're dynamic objects
					weight = ((DynamicObjectActor *)pActor)->GetCreationData()->m_DynamicObjectDef->m_Weight;
				else if (pActor->GetActorTypeId() == Game::E_ActorTypeId_PhysicalObject)
					weight = 50;		// ((C_PhysicalObject *)pActor)->;

				valid = false;
				switch(fe->m_ForceType)
				{
					case E_FORCE_EFFECT_TYPE_AREA:
					{
						dir = actorpos - mat->Translation();
						dist = dir.Length();							// How far from the centre
						dir.Normalise();
						dir.Y(dir.Y() + fe->m_UpwardForce);							// A bit of upward movement
						force = fe->m_MaxForce * (fe->m_Radius-dist/fe->m_Radius) ;
						msg.m_Force = dir * force / 100.0f;
						duration = (dist * fe->m_Time) / fe->m_Radius;
						valid = true;
						break;
					}

					case E_FORCE_EFFECT_TYPE_FORWARD:
					{
						Maths::HMat43	inverse_matrix = mat->InverseRotTrans();
						Maths::HVec3	effect_local_pos = actorpos * inverse_matrix;		// Where this object is local to th effect

						if (effect_local_pos.Z() > 0.0f)				// Only object ahead of effect (vehicle)
						{
							absx = Maths::Abs(effect_local_pos.X());
							if (effect_local_pos.Z() > (2.0f*absx))				// Only in a 30% cone
							{
								if (effect_local_pos.X() < 0.0f)				// If on the vehicles left
									dir = Maths::HVec3(-1.0f, fe->m_UpwardForce, 0.0f) * mat->Rotation();
								else
									dir = Maths::HVec3(1.0f, fe->m_UpwardForce, 0.0f) * mat->Rotation();
								dir.Normalise();								// Don't know why this is needed!
								msg.m_Force = dir * fe->m_MaxForce / 10.0f;

								duration = (effect_local_pos.Z() * fe->m_Time)/ fe->m_Radius;
								valid = true;
							}
						}
						break;
					}

					case E_FORCE_EFFECT_TYPE_SIDE:
					{
						Maths::HMat43	inverse_matrix = mat->InverseRotTrans();
						Maths::HVec3	effect_local_pos = actorpos * inverse_matrix;		// Where this object is local to th effect

						absx = Maths::Abs(effect_local_pos.X());
						absz = Maths::Abs(effect_local_pos.Z());
						if (absx > absz)								// Only in a 90% cone
						{
							if (effect_local_pos.X() < 0.0f)				// If on the vehicles left
								dir = Maths::HVec3(-1.0f, fe->m_UpwardForce, 0.0f) * mat->Rotation();
							else
								dir = Maths::HVec3(1.0f, fe->m_UpwardForce, 0.0f) * mat->Rotation();
							dir.Normalise();								// Don't know why this is needed!
							msg.m_Force = dir * fe->m_MaxForce / 10.0f;

							duration = (absx * fe->m_Time)/ fe->m_Radius;
							valid = true;
						}
						break;
					}
				}

				if (valid)
				{
					msg.m_DamageScale = fe->m_DamageScale;
					msg.m_ElectricalEffect = fe->m_ElectricalEffect;
					msg.m_ElectricalEffectColour = fe->m_ElectricalEffectColour;
					pActor->GetActorList()->DelayMessageToActorBySeconds(duration, handle, (const ActorMsg::C_DynamicObjectApplyImpulseMsg*)&msg);						// Apply force message
					if (fe->m_ElectricalEffect)
						pActor->GetActorList()->DelayMessageToActorBySeconds(duration + 1.0f + rand.FRand(), handle, ActorMsg::E_DynamicObjMsg_DynamicObjectDisableElectricalEffect);	// End electrical effect message
				}
			}
		}
	}
}



//=========================================================================
//						DYNAMIC ACTOR CREATION
//=========================================================================


//void C_ActorUtils::DropDynamicObject()
//{
//#ifndef CFG_GOLD
//	if (C_GameData::Instance().mDebugReferences.Number() < 1 || !C_GameData::Instance().mDebugReferences[0])
//		return;
//		
////	C_GameData::Instance().mDebugReferences[0]->m_DropObjectType;
//
//	const Framework::C_View& view = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0);
//	Maths::HMat43 cameraMatrix = view.DebugFeed().Mat43();
//	DynamicObjectActor *spawned = SpawnDynamicObject(&cameraMatrix, C_GameData::Instance().mDebugReferences[0]->m_DropObjectType);
//	if (spawned)
//	{
//		Maths::HVec3	speed(0.0f, 0.0f, 0.0f);
//		spawned->SetInitialSpeeds(&speed, &speed, true);
//	}
//#endif
//}


//=========================================================================


//DynamicObjectActor *C_ActorUtils::SpawnDynamicObject(Maths::HMat43 *mat, XtHandle<Game::DynamicObjectDef> objdef)
//{
//#ifndef CFG_GOLD
//	Actor::C_ActorHandle	handle;
//	DynamicObjectActor		*spawnactor;
//
//	Actor::C_ActorCreationDataFactoryHelper<DynamicObjectCreationData>		pDynObjCreationData;		// Initialise some creation data
//
//	pDynObjCreationData->m_WorldMatrix = Store(*mat);
//	pDynObjCreationData->m_DynamicObjectDef = objdef;
//
//	handle = gameworld->GetActorList()->Spawn(pDynObjCreationData.Get());
//	spawnactor = (DynamicObjectActor *)gameworld->GetActorList()->GetActorPtr(handle);	
//	return(spawnactor);
//#else
//	return NULL;
//#endif
//}



//=========================================================================
//					MISC ROUTINES
//=========================================================================



void C_ActorUtils::NormaliseMat(Maths::HMat43 *mat)
{
	mat->AX(mat->AY() ^ mat->AZ());
	mat->AZ(mat->AX() ^ mat->AY());
	mat->AY(mat->AZ() ^ mat->AX());
}

//=========================================================================

void C_ActorUtils::NormaliseMatXYZ(Maths::HMat43 *mat)
{
	Maths::HVec3 ax = mat->AY() ^ mat->AZ();
	ax.Normalise();
	Maths::HVec3 ay = mat->AZ() ^ ax;
	ay.Normalise();
	mat->AX(ax);
	mat->AY(ay);
}

//=========================================================================

void C_ActorUtils::NormaliseMatXYZ(Maths::HMat33 *mat)
{
	Maths::HVec3 ax = mat->AY() ^ mat->AZ();
	ax.Normalise();
	Maths::HVec3 ay = mat->AZ() ^ ax;
	ay.Normalise();
	mat->AX(ax);
	mat->AY(ay);
}

//=========================================================================
/*
void C_ActorUtils::NormaliseMatXYZ(Maths::HMat43 *mat)
{
	mat->AX(mat->AY() ^ mat->AZ());
	mat->AY(mat->AZ() ^ mat->AX());
	mat->AZ(mat->AX() ^ mat->AY());
}

//=========================================================================

void C_ActorUtils::NormaliseMatXYZ(Maths::HMat33 *mat)
{
	mat->AX(mat->AY() ^ mat->AZ());
	mat->AY(mat->AZ() ^ mat->AX());
	mat->AZ(mat->AX() ^ mat->AY());
}
*/


//=========================================================================
//					TRIGGER CREATION ROUTINES
//=========================================================================


Gravity::C_Trigger* C_ActorUtils::CreateTrigger(Actor::C_ActorHandle handle, Maths::HMat43 worldmat, XtBoundingBox bbox)
{
	Gravity::C_Trigger* triggerobject;

	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	if (!pRLC || !pRLC->GetGameWorld())
		return(NULL);

	Gravity::C_TriggerLayer* pTriggerLayer = pRLC->GetGameWorld()->GetTriggerLayer(Game::E_TriggerLayer_TriggerRegion);
	if (!pTriggerLayer)
		return(NULL);

	Actor::C_ActorList* pActorList	= pRLC->GetGameWorld()->GetActorList();

	triggerobject	= new Gravity::C_Trigger;
	if (!triggerobject)
		return(NULL);

	triggerobject->SetUserData( SharedActorUtils::HandleToUserData( handle ) );

	Maths::HMat43	tempmat = Load(bbox.matrix);
	Maths::HMat43	combinedmat = tempmat * worldmat;
	Maths::HVec3	extents = Maths::HVec3( bbox.size.vx, bbox.size.vy, bbox.size.vz );
	extents = extents * 0.5f;

	Collision::C_CollisionFilter filter;
	Gravity::C_BoxDesc desc;
	desc.m_HalfExtents = extents;
	triggerobject->m_pTriggerShape = desc.CreateShape(&filter);
	
	pTriggerLayer->AddTrigger(triggerobject);
	
	U8 data[1024 * sizeof(void*)];
	SharedPhysics::C_SafeStack<true> stack(data, 1024);
	Maths::HMat43 triggermat = Load(bbox.matrix);
	triggermat = triggermat * worldmat;
	triggermat.AW(extents * triggermat);
//	pTriggerLayer->SetTriggerMatrix(triggerobject, triggermat, stack);
	triggerobject->SetMatrix(triggermat, stack);

	return(triggerobject);
}

//=========================================================================


void C_ActorUtils::DestroyTrigger(Gravity::C_Trigger* triggerObject)
{
	S32		a;

	if (!triggerObject)
		return;

	for(a=0;a<m_NumDeletedtriggers;a++)				// Check for it already being marked for deletion
		if (m_pDeletedTriggers[a] == triggerObject)
			return;

	if (m_NumDeletedtriggers >= MAX_NUM_TRIGGERS)		// If we're deleting over 50 triggers in one frame we're likely to be closing down so we should be OK just deleting them now
		RemoveDestroyedTriggers();
	
	m_pDeletedTriggers[m_NumDeletedtriggers++] = triggerObject;

	triggerObject->m_pUserData = NULL;
}


//=========================================================================

void C_ActorUtils::RemoveDestroyedTriggers()
{
	S32		a;

	for(a=0;a<m_NumDeletedtriggers;a++)
	{	
		if (m_pDeletedTriggers[a]->m_pLayer)
			m_pDeletedTriggers[a]->m_pLayer->RemoveTrigger(m_pDeletedTriggers[a]);

		delete m_pDeletedTriggers[a];
	}
	m_NumDeletedtriggers = 0;
}


}// End namespace Actors


//=========================================================================
//						Might be useful one day
//=========================================================================


//#pragma optimize("", on)



