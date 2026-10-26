
#include "SharedActorsPch.h"

#include <shared_actors/actormessages/ConstraintActorMessages.h>
#include <shared_actors/XtActorMessages.h>


namespace ActorMsg
{

class C_XtConstraintDestroyMsg : public ActorMsg::C_XtGenericMessage
{
public:
	C_XtConstraintDestroyMsg() : C_XtGenericMessage(E_ConstraintMsg_Destroy) {}
};

} // namespace ActorMsg


XT_TYPE(ActorMsg::C_XtConstraintDestroyMsg, "ConstraintDestroyMsg")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE


XT_LIBRARY(ConstraintActorMessages)
	XT_REGISTER_TYPE(ActorMsg::C_XtConstraintDestroyMsg)
XT_END_LIBRARY

