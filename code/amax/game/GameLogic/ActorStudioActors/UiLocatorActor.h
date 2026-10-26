#ifndef UiLocatorActor_H
#define UiLocatorActor_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <Shared_Actors/ActorImpl/SplineActor.h>

namespace Actors
{
	
class UiLocatorCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
	const Actor::C_ActorTypeId			GetType()		const { return Game::E_ActorTypeId_UiLocator; }

	const ::Actors::C_SplineActorCreate*	m_Spline;
	XtSz8									m_LocationName;
};


//=========================================================================


class UiLocatorActor : public Game::Actors::C_GameActor
{
public:
	typedef Ut::UtSet<UiLocatorActor*>	List;

	UiLocatorActor();
	virtual ~UiLocatorActor();

	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_UiLocator; }
	const UiLocatorCreationData*		GetCreationData() const {return static_cast<const UiLocatorCreationData*>(C_GameActor::GetCreationData().GetData());}
	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);
	virtual const Maths::HMat43&		GetMatrix(void) const {return m_WorldMatrix;}

	static	UiLocatorActor *			FindByName( const char* name );

protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();
	virtual void						KillImpl();

protected:
	Maths::HMat43						m_WorldMatrix;

private:
	static List							m_Store;
};

}// End namespace Actors


#endif // UiLocatorActor_H
