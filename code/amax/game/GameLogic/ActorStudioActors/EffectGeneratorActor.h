#ifndef EFFECTGENERATORACTOR_H
#define EFFECTGENERATORACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>

#include <Dynamic/Node.h>
#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Particle/Event.h>
#include <Dynamic/Particle/EventInstance.h>

#include <GameLogic/ActorStudioActors/ResourceTypes.h>

#include <SpecialFX/SpecialFX.h>

#include <shared_actors/Components/Creation/MovementComponentsCreate.h>


namespace Components
{
	class I_Movement;
}


//=========================================================================
//						SPAWNER ACTOR CLASSES
//=========================================================================


namespace Actors
{

class C_VisibleDetect;

enum E_EFFECT_LOOP_TYPE
{
	E_EFFECT_LOOP_TYPE_NONE,
	E_EFFECT_LOOP_TYPE_PLAYONCE,
	E_EFFECT_LOOP_TYPE_LOOPED,
	E_EFFECT_LOOP_TYPE_PINGPONG,
};


//=========================================================================



class EffectGeneratorCreationData : public C_PosActorCreate
{
public:
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_EffectGenerator;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_EffectGenerator;}


	Game::E_ALIGNMENT_TYPE				m_Alignment;
	bool								m_StartDeactivated;		// Might begin halfway through a level


	XtHandle<SpecialFX::C_SpecialFX>	m_SpecialEffect;
	F32									m_T1LoopStart;
	F32									m_T2LoopEnd;
	E_EFFECT_LOOP_TYPE					m_LoopType;

	F32									m_StartTime;



	// Components
	virtual void						DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);


	Components::C_MovementCreateInfo*				m_pSorceryMovementComponentCreate;
	Component::I_ComponentCreator::C_DataRef		m_pMovementCreator;
	const Components::C_MovementCreateInfo*			GetMovementCreator() const {return static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData());}
	Components::C_MovementCreateInfo*				GetMovementCreator() {return const_cast<Components::C_MovementCreateInfo*>(static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData()));}
};


//=========================================================================


class EffectGeneratorActor : public Game::Actors::C_GameActor
{
public:
	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_EffectGenerator; }
	static const Btl::String32			TypeName()		{ return "Effect Generator"; }

	virtual const Maths::C_Vertex		GetPosition(void) const{return m_WorldMatrix.AW();}			// Needed for movement
	virtual const Maths::HMat43&		GetMatrix(void) const {return m_WorldMatrix;}


	EffectGeneratorActor();
	~EffectGeneratorActor();


	const EffectGeneratorCreationData	*pCreationData;
	Maths::HMat43						m_WorldMatrix;

	SpecialFX::I_SpecialFXInstance*		m_pSFX;


	void								ActivateEffect();
	void								DeactivateEffect();

	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);
	void								Update(float frameTime);					// Update logic called each frame
	void								CheckLoopTimes();							// Because sometimes the effect is slightly shorter than expected

	void								DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;

	const EffectGeneratorCreationData	*GetCreationData() const {return static_cast<const EffectGeneratorCreationData*>(C_GameActor::GetCreationData().GetData());}

	C_VisibleDetect*					GetVisibleDetect() const { return m_pVisDetect; }

protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();

	C_VisibleDetect*					m_pVisDetect;

private:
	Btl::Vector<Actor::C_ActorHandle>	m_Children;
	const Components::I_Movement*		m_pMovementComponent;

	F32									m_T1LoopStart;				// Corrected versions of creation data
	F32									m_T2LoopEnd;
	E_EFFECT_LOOP_TYPE					m_LoopType;

	F32									m_FramePos;

	bool								m_Reverse;
	bool								m_Paused;
	bool								m_Selected;
	bool								m_Activated;
	bool								m_bVisible;

	float								m_PreviousTime;
};


}// End namespace Actors

#endif // EFFECTGENERATORACTOR_H
