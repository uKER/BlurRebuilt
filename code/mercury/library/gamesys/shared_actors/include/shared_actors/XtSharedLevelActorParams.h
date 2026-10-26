#ifndef SHARED_LEVEL_ACTOR_PARAMS_H
#define SHARED_LEVEL_ACTOR_PARAMS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>

namespace Actors	// should we use the Actors name space? or use a new one such as ActorParam ?
{

// Level Actor Params
// These are polymorphic values that can be extended 

// Actor Param
// -----------

// Base Type
class C_XtParamActor
{
public:
	virtual const Actor::C_ActorHandle	GetActorHandle(const Actor::C_Actor *pActorContext) const = 0;
private:
};

// Orientation Param
// -----------------

// Base Type
class C_XtParamOrientation
{
public:
	virtual const Maths::HMat43 &	GetOrientation(const Actor::C_Actor *pActorContext) const = 0;
private:

};


// Position Param
// --------------

// Base Type
class C_XtParamPosition
{
public:
	virtual const Maths::C_Vertex	GetPosition(const Actor::C_Actor *pActorContext) const = 0;
private:

};

// Vector Param
// --------------

// Base Type
class C_XtParamVector
{
public:
	virtual const Maths::C_Vector	GetVector(const Actor::C_Actor *pActorContext) const = 0;
private:

};

// Float Param
// -----------

// Base Type
class C_XtParamFloat
{
public:
	virtual F32		GetFloat(const Actor::C_Actor *pActorContext) const = 0;
private:
};

// Integer Param
// -----------

// Base Type
class C_XtParamInteger
{
public:
	virtual S32		GetInteger(const Actor::C_Actor *pActorContext) const = 0;
private:
};

// Bool Param
// ----------

// Base Type
class C_XtParamBool
{
public:
	virtual bool	GetBool(const Actor::C_Actor *pActorContext) const = 0;
private:
};

} // namespace Actors

#endif
