#ifndef CUTSCENEPROXYACTOR_H
#define CUTSCENEPROXYACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>




//=========================================================================
//						CUTSCENE PROXY ACTOR CLASSES
//=========================================================================



namespace Actors
{
	
class CutSceneProxyCreationData : public C_GameActorCreate
{
public:
	// Standard stuff
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_CutSceneProxy;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_CutSceneProxy;}

	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);

//	XtArray<ActorMsg::C_XtSendMessage>		m_OnCutsceneSkip;
};


//=========================================================================


class CutSceneProxyActor : public Game::Actors::C_GameActor, public Resource::C_Listener
{
public:
	CutSceneProxyActor();
	virtual ~CutSceneProxyActor();

	void								ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData );
	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);


	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_CutSceneProxy; }
	static const Btl::String32			TypeName()		{ return "CutScene Proxy"; }
	const CutSceneProxyCreationData*	GetCreationData() const {return static_cast<const CutSceneProxyCreationData*>(C_GameActor::GetCreationData().GetData());}

	void								SetupLights();
	void								SetupLight(U32 index, F32 heading, F32 elevation, F32 wrap, F32 intensity, F32 red, F32 green, F32 blue);
	
protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();

private:
	XtHandle<Game::CutSceneLightDef>	m_CutSceneLights;
	S32									m_NumCameraCuts;
	bool								m_Active;
};

}// End namespace Actors


#endif // CUTSCENEPROXYACTOR_H
