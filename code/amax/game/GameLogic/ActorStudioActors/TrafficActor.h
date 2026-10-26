#ifndef TRAFFICACTOR_H
#define TRAFFICACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>

#include <GameLogic/ActorStudioActors/ResourceTypes.h>

#include <Shared_Actors/ActorImpl/SplineActor.h>
#include <Shared_Actors/TypeWrappers/SplineVec3Wrapper.h>

#include <world_render/Scene.h>
#include <world_render/EnvironmentSampler.h>
#include <SceneGraph/SceneGraph.h>



namespace Actors
{
	class C_SplineActor;
}


//=========================================================================
//						TRAFFIC ACTOR CLASSES
//=========================================================================



namespace Actors
{

typedef struct TrafficItem						// State of each traffic item (vehicle)
{
	TrafficItem() : m_pModelResource(NULL), m_ModelInstance(NULL), m_Loaded(false)
	{
	}

	XtHandle<ResourceUtils::C_GameModel>	m_ModelType;			// Which model to use
	bool									m_Loaded;				// Has it been loaded
	F32										m_SpacingOffset;		// Offset from standard spline position (spacing variation)

	GameResource::C_ModelHandle				m_ModelHandle;
	GameResource::C_ModelResource*			m_pModelResource;
	WorldRender::C_ModelInstance*			m_ModelInstance;		// Render model instance

	Maths::HMat43							m_WorldMatrix;
}
TrafficItem;


//=========================================================================

	
class TrafficActorCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_TrafficActor;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_TrafficActor;}

	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);

	const ::Actors::C_SplineActorCreate*	m_Spline;
	XtHandle<Game::DynamicObjectDefGroup>	m_TrafficGroup;
	F32										m_Speed;
	F32										m_Spacing;
	F32										m_SpacingVariation;
	bool									m_UpdateLighting;
};


//=========================================================================


class TrafficActor : public Game::Actors::C_GameActor, public Resource::C_Listener
{
public:	
	TrafficActor();
	virtual ~TrafficActor();

	void								ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData );
	void								CreateModelInstance(TrafficItem& ti);						// Create and load model into render scene
	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);


	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_TrafficActor; }
	static const Btl::String32			TypeName()		{ return "Traffic Actor"; }
	const TrafficActorCreationData*		GetCreationData() const {return static_cast<const TrafficActorCreationData*>(C_GameActor::GetCreationData().GetData());}
	
protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();

private:
	bool								m_Initialised;
	bool								m_FirstUpdate;


	const ::Actors::C_SplineActor*		m_Spline;
	F32									m_SplineLength;

	Btl::Vector<TrafficItem>			m_TrafficItems;

	Util::C_Random						m_Random;
	
	F32									m_offset;

	WorldRender::C_EnvironmentSampler*	m_pEnvSampler;

};

}// End namespace Actors


#endif // TRAFFICACTOR_H
