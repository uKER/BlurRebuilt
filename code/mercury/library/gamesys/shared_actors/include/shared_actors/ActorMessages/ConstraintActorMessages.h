
#ifndef CONSTRAINTACTORMESSAGES_H
#define CONSTRAINTACTORMESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/messaging/MessageBase.h>
#include <misc_shared/uid/FourCC.h>

namespace Gravity { class C_Body; }


namespace ActorMsg
{

// Constraint actor messages.
enum E_ConstraintMsg
{
	E_ConstraintMsg_HiBits = FOURCC_HI20('CONM'),

	E_ConstraintMsg_Destroy,
	E_ConstraintMsg_ChangeBody,

	E_ConstraintMsg_End, 
	E_ConstraintMsg_Begin = E_ConstraintMsg_HiBits + 1, 
	E_ConstraintMsg_Count = E_ConstraintMsg_End - E_ConstraintMsg_Begin
};


class C_ConstraintChangeBodyMsg : public Messaging::C_MessageDerived<C_ConstraintChangeBodyMsg, E_ConstraintMsg_ChangeBody>
{
public:
	C_ConstraintChangeBodyMsg(const Gravity::C_Body* oldBody, const Gravity::C_Body* newBody) : m_OldBody(oldBody), m_NewBody(newBody) {}

public:
	const Gravity::C_Body* m_OldBody;
	const Gravity::C_Body* m_NewBody;
};


} // namespace ActorMsg


#endif
