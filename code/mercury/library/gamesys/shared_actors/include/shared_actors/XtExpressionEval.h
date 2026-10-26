#ifndef XT_EXPRESSION_EVAL_H
#define XT_EXPRESSION_EVAL_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/XtSharedLevelActorParams.h>

namespace Actors
{
// forward decs
class C_ParamExpr;
class C_ParamExprResult_Bool;
class C_ParamExprResult_Integer;
class C_ParamExprResult_Float;

// Bool Expression Type
class C_XtParamBoolExpression : public C_XtParamBool
{
public:
	virtual bool	GetBool(const Actor::C_Actor *pActorContext) const;

public:
	C_ParamExprResult_Bool*		m_Result;
	XtArray<C_ParamExpr*>		m_Exprs;
};

// Integer Expression Type
class C_XtParamIntegerExpression : public C_XtParamInteger
{
public:
	virtual S32	GetInteger(const Actor::C_Actor *pActorContext) const;

public:
	C_ParamExprResult_Integer*	m_Result;
	XtArray<C_ParamExpr*>		m_Exprs;
};

// Float Expression Type
class C_XtParamFloatExpression : public C_XtParamFloat
{
public:
	virtual F32	GetFloat(const Actor::C_Actor *pActorContext) const;

public:
	C_ParamExprResult_Float*	m_Result;
	XtArray<C_ParamExpr*>		m_Exprs;
};

}//namespace Actors

#endif