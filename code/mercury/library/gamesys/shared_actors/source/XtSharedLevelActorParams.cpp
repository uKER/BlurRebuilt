#include "SharedActorsPch.h"
#include <shared_actors/XtSharedLevelActorParams.h>

#include <Xt/XtGame.h>
#include <core/xtstdx.h>

#include <actor_system/actor.h>
#include <actor_system/actorlist.h>
#include <shared_actors/XtActorBase.h>

#include <shared_actors/ActorCreation/MessageActorsCreate.h>

#include <shared_actors/Components/Interfaces/I_MessageComponents.h>

namespace Actors
{

// forward decs
class C_GameActorCreate;
struct C_CounterCreate;
struct C_CountdownTimerCreate;

// Actor Param

// An actor on the level
class C_XtParamLevelActor : public C_XtParamActor
{
public:
	virtual const Actor::C_ActorHandle	GetActorHandle(const Actor::C_Actor *pActorContext) const;

	Actors::C_GameActorCreate	*	m_Actor;
};

const Actor::C_ActorHandle	C_XtParamLevelActor::GetActorHandle(const Actor::C_Actor *pActorContext) const 
{
	if (m_Actor)
		return m_Actor->GetActorHandle();
	else 
		return Actor::C_ActorHandle();
}

// this actor
class C_XtParamThisActor : public C_XtParamActor
{
public:
	virtual const Actor::C_ActorHandle	GetActorHandle(const Actor::C_Actor *pActorContext) const;// {return pActorContext->GetHandle();}
private:
};

const Actor::C_ActorHandle	C_XtParamThisActor::GetActorHandle(const Actor::C_Actor *pActorContext) const 
{
	return pActorContext->GetHandle();
}

// Orientation Param

// An Actor's orientation
class C_XtParamOrientationActor : public C_XtParamOrientation
{
public:
	virtual const Maths::HMat43 &	GetOrientation(const Actor::C_Actor *pActorContext) const
	{
		assert(pActorContext);
		assert(m_pActorParam);

		const Actor::C_Actor *pActor = pActorContext->GetActorList()->GetActorPtr(m_pActorParam->GetActorHandle(pActorContext));

		if(pActor)
			return pActor->GetMatrix();
		else
		{
			// TODO: use const from maths
			static Maths::HMat43 mat; 

			return mat;
		}
	}
	C_XtParamActor *	m_pActorParam;
};


// Position Param

// An Actor's position
class C_XtParamPositionActor : public C_XtParamPosition
{
public:
	virtual const Maths::C_Vertex	GetPosition(const Actor::C_Actor *pActorContext) const
	{
		assert(pActorContext);
		assert(m_pActorParam);

		const Actor::C_Actor *pActor = pActorContext->GetActorList()->GetActorPtr(m_pActorParam->GetActorHandle(pActorContext));

		if(pActor)
			return pActor->GetPosition();
		else
			return Maths::C_Vertex();
	}
	C_XtParamActor *	m_pActorParam;
};

class C_XtParamPositionActorOffset : public C_XtParamPosition
{
public:
	virtual const Maths::C_Vertex	GetPosition(const Actor::C_Actor *pActorContext) const
	{
		assert(pActorContext);
		assert(m_pActorParam);

		const Actor::C_Actor *pActor = pActorContext->GetActorList()->GetActorPtr(m_pActorParam->GetActorHandle(pActorContext));

		if(pActor)
			return Maths::C_Vertex(Load(m_Offset)) * pActor->GetMatrix();
		else
			return Maths::C_Vertex();
	}

	C_XtParamActor *	m_pActorParam;
	XtVec3				m_Offset;
};

// Specific XYZ coords
class C_XtParamPositionXYZ : public C_XtParamPosition
{
public:
	virtual const Maths::C_Vertex	GetPosition(const Actor::C_Actor *pActorContext) const {return Maths::C_Vertex(Load(m_Position));}
public:
	XtVec3	m_Position;
};

// Vector Param
// To an Actor's position
class C_XtParamVectorToActor : public C_XtParamVector
{
public:
	virtual const Maths::C_Vector	GetVector(const Actor::C_Actor *pActorContext) const;

	C_XtParamActor *	m_pActorParam;
};


const Maths::C_Vector	C_XtParamVectorToActor::GetVector(const Actor::C_Actor *pActorContext) const
{
	assert(pActorContext);
	assert(m_pActorParam);

	const Actor::C_Actor *pActor = pActorContext->GetActorList()->GetActorPtr(m_pActorParam->GetActorHandle(pActorContext));

	if(pActor)
		return pActor->GetPosition() - pActorContext->GetPosition();
	else
		return Maths::C_Vector();

}

// Specific XYZ coords
class C_XtParamVectorXYZ : public C_XtParamVector
{
public:
	virtual const Maths::C_Vector	GetVector(const Actor::C_Actor *pActorContext) const {return Maths::C_Vector(Load(m_Vector));}
public:
	XtVec3	m_Vector;
};

// Unit vector in the X Axis
class C_XtParamVectorXAxis : public C_XtParamVector
{
public:
	virtual const Maths::C_Vector	GetVector(const Actor::C_Actor *pActorContext) const {return Maths::C_Vector(1.0f,0.0f,0.0f);}
};

// Unit vector in the Y Axis
class C_XtParamVectorYAxis : public C_XtParamVector
{
public:
	virtual const Maths::C_Vector	GetVector(const Actor::C_Actor *pActorContext) const {return Maths::C_Vector(0.0f,1.0f,0.0f);}
};
// Unit vector in the Z Axis
class C_XtParamVectorZAxis : public C_XtParamVector
{
public:
	virtual const Maths::C_Vector	GetVector(const Actor::C_Actor *pActorContext) const {return Maths::C_Vector(0.0f,0.0f,1.0f);}
};


// Float Param

// Basic Value
class C_XtParamFloatVal : public C_XtParamFloat
{
public:
	virtual F32		GetFloat(const Actor::C_Actor *pActorContext) const { return m_Val;}
public:
	F32				m_Val;
};

// Countdown Timer Value
class C_XtParamFloatCountdownTimerValue : public C_XtParamFloat
{
public:
	virtual F32		GetFloat(const Actor::C_Actor *pActorContext) const
	{ 
		const Actor::C_Actor *pActor = pActorContext->GetActorList()->GetActorPtr(m_pTimerCreate->GetActorHandle());

		if(pActor)
		{
			const Components::I_Timer *pTimerComp = (const Components::I_Timer *)pActor->QueryInterface(Components::E_IF_Timer);

			if(pTimerComp)
				return pTimerComp->GetValue();
		}

		return 0;
	}
public:
	C_CountdownTimerCreate	*m_pTimerCreate;

};

// Distance between 2 positions
class C_XtParamFloatDistance : public C_XtParamFloat
{
public:
	virtual F32		GetFloat(const Actor::C_Actor *pActorContext) const
	{ 
		if(m_A && m_B)
		{
			return m_A->GetPosition(pActorContext).Distance(m_B->GetPosition(pActorContext));
		}

		return 0;
	}
public:
	C_XtParamPosition	*m_A;
	C_XtParamPosition	*m_B;
};

// Integer Param
// Basic Value
class C_XtParamIntegerVal : public C_XtParamInteger
{
public:
	virtual S32		GetInteger(const Actor::C_Actor *pActorContext) const { return m_Val;}
public:
	S32				m_Val;
};

// Counter Value
class C_XtParamIntegerCounterValue : public C_XtParamInteger
{
public:
	virtual S32		GetInteger(const Actor::C_Actor *pActorContext) const;
public:
	C_CounterCreate	*m_pCounterCreate;

};

S32	C_XtParamIntegerCounterValue::GetInteger(const Actor::C_Actor *pActorContext) const 
{ 
	const Actor::C_Actor *pActor = pActorContext->GetActorList()->GetActorPtr(m_pCounterCreate->GetActorHandle());

	if(pActor)
	{
		const Components::I_Counter *pCounterComp = (const Components::I_Counter *)pActor->QueryInterface(Components::E_IF_Counter);

		if(pCounterComp)
			return pCounterComp->GetValue();
	}

	return 0;
}

// Bool Param
// Basic Value 
class C_XtParamBoolVal : public C_XtParamBool
{
public:
	virtual bool	GetBool(const Actor::C_Actor *pActorContext) const {return m_Val;}
public:
	bool			m_Val;
};

// [MarkC] I've removed these because they weren't used (according to Cakey-scan) and we have a better alternative
#if 0

// Logical Operations
// AND
class C_XtParamBoolAND : public C_XtParamBool
{
public:
	virtual bool	GetBool(const Actor::C_Actor *pActorContext) const 
	{
		if(m_ValA && m_ValB)
		{
			return (m_ValA->GetBool(pActorContext) && m_ValB->GetBool(pActorContext));
		}
		return false;
	}
public:
	C_XtParamBool *	m_ValA;
	C_XtParamBool *	m_ValB;
};

// OR
class C_XtParamBoolOR : public C_XtParamBool
{
public:
	virtual bool	GetBool(const Actor::C_Actor *pActorContext) const 
	{
		if(m_ValA && m_ValB)
		{
			return (m_ValA->GetBool(pActorContext) || m_ValB->GetBool(pActorContext));
		}
		return false;
	}
public:
	C_XtParamBool *	m_ValA;
	C_XtParamBool *	m_ValB;
};

// NOT
class C_XtParamBoolNOT : public C_XtParamBool
{
public:
	virtual bool	GetBool(const Actor::C_Actor *pActorContext) const 
	{
		if(m_Val)
		{
			return !m_Val->GetBool(pActorContext);
		}
		return false;
	}
public:
	C_XtParamBool *	m_Val;
};

// Float comparisons

// Greater than
class C_XtParamBoolFloatGT : public C_XtParamBool
{
public:
	virtual bool	GetBool(const Actor::C_Actor *pActorContext) const 
	{
		if(m_FValA && m_FValB)
		{
			return m_FValA->GetFloat(pActorContext) > m_FValB->GetFloat(pActorContext);
		}
		return false;
	}
public:
	C_XtParamFloat *	m_FValA;
	C_XtParamFloat *	m_FValB;
};

// Less than
class C_XtParamBoolFloatLT : public C_XtParamBool
{
public:
	virtual bool	GetBool(const Actor::C_Actor *pActorContext) const 
	{
		if(m_FValA && m_FValB)
		{
			return m_FValA->GetFloat(pActorContext) < m_FValB->GetFloat(pActorContext);
		}
		return false;
	}
public:
	C_XtParamFloat *	m_FValA;
	C_XtParamFloat *	m_FValB;
};


// Integer Comparisons

// Greater than
class C_XtParamBoolIntegerGT : public C_XtParamBool
{
public:
	virtual bool	GetBool(const Actor::C_Actor *pActorContext) const 
	{
		if(m_IValA && m_IValB)
		{
			return m_IValA->GetInteger(pActorContext) > m_IValB->GetInteger(pActorContext);
		}
		return false;
	}
public:
	C_XtParamInteger *	m_IValA;
	C_XtParamInteger *	m_IValB;
};

// Less than
class C_XtParamBoolIntegerLT : public C_XtParamBool
{
public:
	virtual bool	GetBool(const Actor::C_Actor *pActorContext) const 
	{
		if(m_IValA && m_IValB)
		{
			return m_IValA->GetInteger(pActorContext) < m_IValB->GetInteger(pActorContext);
		}
		return false;
	}
public:
	C_XtParamInteger *	m_IValA;
	C_XtParamInteger *	m_IValB;
};

// Equal
class C_XtParamBoolIntegerEQ : public C_XtParamBool
{
public:
	virtual bool	GetBool(const Actor::C_Actor *pActorContext) const 
	{
		if(m_IValA && m_IValB)
		{
			return m_IValA->GetInteger(pActorContext) == m_IValB->GetInteger(pActorContext);
		}
		return false;
	}
public:
	C_XtParamInteger *	m_IValA;
	C_XtParamInteger *	m_IValB;
};
#endif
}//namespace Actors

// XT Annontations
// ---------------

// Actor Params
// ------------
XT_ABSTRACT_TYPE(Actors::C_XtParamActor, "ParamActor")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamLevelActor, "ParamActorLevelActor")
XT_BASE(Actors::C_XtParamActor)
XT_FIELD(m_Actor, "Actor")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamThisActor, "ParamActorThis")
XT_BASE(Actors::C_XtParamActor)
XT_END_TYPE

// Orientation Params
// ---------------
XT_ABSTRACT_TYPE(Actors::C_XtParamOrientation, "ParamOrientation")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamOrientationActor, "ParamOrientationActor")
XT_BASE(Actors::C_XtParamOrientation)
XT_FIELD(m_pActorParam, "Actor")
XT_END_TYPE

// Position Params
// ---------------
XT_ABSTRACT_TYPE(Actors::C_XtParamPosition, "ParamPosition")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamPositionActor, "ParamPositionActor")
XT_BASE(Actors::C_XtParamPosition)
XT_FIELD(m_pActorParam, "Actor")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamPositionActorOffset, "ParamPositionActorOffset")
XT_BASE(Actors::C_XtParamPosition)
XT_FIELD(m_pActorParam, "Actor")
XT_FIELD(m_Offset, "Offset")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamPositionXYZ, "ParamPositionXYZ")
XT_BASE(Actors::C_XtParamPosition)
XT_FIELD(m_Position, "Position")
XT_END_TYPE

// Vector Params
// -------------

XT_ABSTRACT_TYPE(Actors::C_XtParamVector, "ParamVector")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamVectorToActor, "ParamVectorToActor")
XT_BASE(Actors::C_XtParamVector)
XT_FIELD(m_pActorParam, "Actor")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamVectorXYZ, "ParamVectorXYZ")
XT_BASE(Actors::C_XtParamVector)
XT_FIELD(m_Vector, "Direction")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamVectorXAxis, "ParamVectorXAxis")
XT_BASE(Actors::C_XtParamVector)
XT_END_TYPE

XT_TYPE(Actors::C_XtParamVectorYAxis, "ParamVectorYAxis")
XT_BASE(Actors::C_XtParamVector)
XT_END_TYPE

XT_TYPE(Actors::C_XtParamVectorZAxis, "ParamVectorZAxis")
XT_BASE(Actors::C_XtParamVector)
XT_END_TYPE

// Float Params
// ------------
XT_ABSTRACT_TYPE(Actors::C_XtParamFloat, "ParamFloat")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamFloatVal, "ParamFloatVal")
XT_BASE(Actors::C_XtParamFloat)
XT_FIELD(m_Val, "Value")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamFloatDistance, "ParamFloatDistance")
XT_BASE(Actors::C_XtParamFloat)
XT_FIELD(m_A, "from")
XT_FIELD(m_B, "to")
XT_END_TYPE


XT_TYPE(Actors::C_XtParamFloatCountdownTimerValue, "ParamFloatCountdownTimerValue")
XT_BASE(Actors::C_XtParamFloat)
XT_FIELD(m_pTimerCreate, "TimerActor")
XT_END_TYPE

// Integer Params
// ------------
XT_ABSTRACT_TYPE(Actors::C_XtParamInteger, "ParamInteger")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamIntegerVal, "ParamIntegerVal")
XT_BASE(Actors::C_XtParamInteger)
XT_FIELD(m_Val, "Value")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamIntegerCounterValue, "ParamIntegerCounterValue")
XT_BASE(Actors::C_XtParamInteger)
XT_FIELD(m_pCounterCreate, "CounterActor")
XT_END_TYPE

// Bool Params
// -----------
XT_ABSTRACT_TYPE(Actors::C_XtParamBool, "ParamBool")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamBoolVal, "ParamBoolVal")
XT_BASE(Actors::C_XtParamBool)
XT_FIELD(m_Val, "value")
XT_END_TYPE

#if 0
XT_TYPE(Actors::C_XtParamBoolAND, "ParamBoolAND")
XT_BASE(Actors::C_XtParamBool)
XT_FIELD(m_ValA, "Value")
XT_FIELD(m_ValB, "AndValue")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamBoolOR, "ParamBoolOR")
XT_BASE(Actors::C_XtParamBool)
XT_FIELD(m_ValA, "Value")
XT_FIELD(m_ValB, "OrValue")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamBoolNOT, "ParamBoolNOT")
XT_BASE(Actors::C_XtParamBool)
XT_FIELD(m_Val, "NotValue")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamBoolFloatGT, "ParamBoolFloatGT")
XT_BASE(Actors::C_XtParamBool)
XT_FIELD(m_FValA, "Value")
XT_FIELD(m_FValB, "GreaterThan")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamBoolFloatLT, "ParamBoolFloatLT")
XT_BASE(Actors::C_XtParamBool)
XT_FIELD(m_FValA, "Value")
XT_FIELD(m_FValB, "LessThan")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamBoolIntegerGT, "ParamBoolIntegerGT")
XT_BASE(Actors::C_XtParamBool)
XT_FIELD(m_IValA, "Value")
XT_FIELD(m_IValB, "GreaterThan")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamBoolIntegerLT, "ParamBoolIntegerLT")
XT_BASE(Actors::C_XtParamBool)
XT_FIELD(m_IValA, "Value")
XT_FIELD(m_IValB, "LessThan")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamBoolIntegerEQ, "ParamBoolIntegerEQ")
XT_BASE(Actors::C_XtParamBool)
XT_FIELD(m_IValA, "Value")
XT_FIELD(m_IValB, "EqualTo")
XT_END_TYPE

#endif

XT_LIBRARY(XtSharedLevelActorParams)

XT_DEPENDS_ON(ExpressionEvaluator)

XT_REGISTER_TYPE(Actors::C_XtParamActor)
XT_REGISTER_TYPE(Actors::C_XtParamLevelActor)
XT_REGISTER_TYPE(Actors::C_XtParamThisActor)

XT_REGISTER_TYPE(Actors::C_XtParamOrientation)
XT_REGISTER_TYPE(Actors::C_XtParamOrientationActor)

XT_REGISTER_TYPE(Actors::C_XtParamPosition)
XT_REGISTER_TYPE(Actors::C_XtParamPositionActor)
XT_REGISTER_TYPE(Actors::C_XtParamPositionActorOffset)
XT_REGISTER_TYPE(Actors::C_XtParamPositionXYZ)

XT_REGISTER_TYPE(Actors::C_XtParamVector)
XT_REGISTER_TYPE(Actors::C_XtParamVectorToActor)
XT_REGISTER_TYPE(Actors::C_XtParamVectorXYZ)
XT_REGISTER_TYPE(Actors::C_XtParamVectorXAxis)
XT_REGISTER_TYPE(Actors::C_XtParamVectorYAxis)
XT_REGISTER_TYPE(Actors::C_XtParamVectorZAxis)

XT_REGISTER_TYPE(Actors::C_XtParamFloat)
XT_REGISTER_TYPE(Actors::C_XtParamFloatVal)
XT_REGISTER_TYPE(Actors::C_XtParamFloatDistance)
XT_REGISTER_TYPE(Actors::C_XtParamFloatCountdownTimerValue)

XT_REGISTER_TYPE(Actors::C_XtParamInteger)
XT_REGISTER_TYPE(Actors::C_XtParamIntegerVal)
XT_REGISTER_TYPE(Actors::C_XtParamIntegerCounterValue)

XT_REGISTER_TYPE(Actors::C_XtParamBool)
XT_REGISTER_TYPE(Actors::C_XtParamBoolVal)

#if 0
XT_REGISTER_TYPE(Actors::C_XtParamBoolAND)
XT_REGISTER_TYPE(Actors::C_XtParamBoolOR)
XT_REGISTER_TYPE(Actors::C_XtParamBoolNOT)
XT_REGISTER_TYPE(Actors::C_XtParamBoolFloatGT)
XT_REGISTER_TYPE(Actors::C_XtParamBoolFloatLT)
XT_REGISTER_TYPE(Actors::C_XtParamBoolIntegerGT)
XT_REGISTER_TYPE(Actors::C_XtParamBoolIntegerLT)
XT_REGISTER_TYPE(Actors::C_XtParamBoolIntegerEQ)
#endif

XT_END_LIBRARY

