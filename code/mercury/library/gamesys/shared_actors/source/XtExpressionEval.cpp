#include "SharedActorsPch.h"
#include <actor_system/Actor.h>
#include <shared_actors/XtSharedLevelActorParams.h>
#include <shared_actors/XtExpressionEval.h>


// This is the code for evaluating bool expressions

namespace Actors
{

struct C_ExpressionContext
{
	const Actor::C_Actor* m_ContextActor;
};

class C_ParamExpr
{
public:
	virtual bool EvaluateBool(C_ExpressionContext& ctx) { return false; }
	virtual S32 EvaluateInt(C_ExpressionContext& ctx) { return 0; }
	virtual F32 EvaluateFloat(C_ExpressionContext& ctx) { return 0.0f; }
	virtual Actor::C_ActorHandle EvaluateActor(C_ExpressionContext& ctx) { return Actor::C_ActorHandle(); }
};

// Base Types
class C_ParamExprBool : public C_ParamExpr { };
class C_ParamExprInt : public C_ParamExpr { };
class C_ParamExprFloat : public C_ParamExpr { };
class C_ParamExprActor : public C_ParamExpr { };

// Boolean Expressions
// -------------------

// AND
class C_ParamExprBool_And : public C_ParamExprBool
{
public:
	virtual bool EvaluateBool(C_ExpressionContext& ctx)
	{
		return (m_A ? m_A->EvaluateBool(ctx): false) && (m_B ? m_B->EvaluateBool(ctx) : false);
	}

public:
	C_ParamExprBool* m_A;
	C_ParamExprBool* m_B;
};

// OR
class C_ParamExprBool_Or : public C_ParamExprBool
{
public:
	virtual bool EvaluateBool(C_ExpressionContext& ctx)
	{
		return (m_A ? m_A->EvaluateBool(ctx) : false) || (m_B ? m_B->EvaluateBool(ctx) : false);
	}

public:
	C_ParamExprBool* m_A;
	C_ParamExprBool* m_B;
};

// NOT
class C_ParamExprBool_Not : public C_ParamExprBool
{
public:
	virtual bool EvaluateBool(C_ExpressionContext& ctx)
	{
		return m_A ? !m_A->EvaluateBool(ctx) : false;
	}

public:
	C_ParamExprBool* m_A;
};

// NAND
class C_ParamExprBool_Nand : public C_ParamExprBool
{
public:
	virtual bool EvaluateBool(C_ExpressionContext& ctx)
	{
		return !((m_A ? m_A->EvaluateBool(ctx): false) && (m_B ? m_B->EvaluateBool(ctx) : false));
	}

public:
	C_ParamExprBool* m_A;
	C_ParamExprBool* m_B;
};

// NOR
class C_ParamExprBool_Nor : public C_ParamExprBool
{
public:
	virtual bool EvaluateBool(C_ExpressionContext& ctx)
	{
		return !((m_A ? m_A->EvaluateBool(ctx) : false) || (m_B ? m_B->EvaluateBool(ctx) : false));
	}

public:
	C_ParamExprBool* m_A;
	C_ParamExprBool* m_B;
};

// XOR
class C_ParamExprBool_Xor : public C_ParamExprBool
{
public:
	virtual bool EvaluateBool(C_ExpressionContext& ctx)
	{
		return ((m_A ? m_A->EvaluateBool(ctx) : false) ^ (m_B ? m_B->EvaluateBool(ctx) : false));
	}

public:
	C_ParamExprBool* m_A;
	C_ParamExprBool* m_B;
};

// Integer Equal
class C_ParamExprBool_EqInt : public C_ParamExprBool
{
public:
	virtual bool EvaluateBool(C_ExpressionContext& ctx)
	{
		return (m_A ? m_A->EvaluateInt(ctx) : 0) == (m_B ? m_B->EvaluateInt(ctx) : 0);
	}

public:
	C_ParamExprInt* m_A;
	C_ParamExprInt* m_B;
};

// Integer Greater Than
class C_ParamExprBool_GtInt: public C_ParamExprBool
{
public:
	virtual bool EvaluateBool(C_ExpressionContext& ctx)
	{
		return (m_A ? m_A->EvaluateInt(ctx) : 0) > (m_B ? m_B->EvaluateInt(ctx) : 0);
	}

public:
	C_ParamExprInt* m_A;
	C_ParamExprInt* m_B;
};

// Integer Less Than
class C_ParamExprBool_LtInt: public C_ParamExprBool
{
public:
	virtual bool EvaluateBool(C_ExpressionContext& ctx)
	{
		return (m_A ? m_A->EvaluateInt(ctx) : 0) < (m_B ? m_B->EvaluateInt(ctx) : 0);
	}

public:
	C_ParamExprInt* m_A;
	C_ParamExprInt* m_B;
};

// Float Greater Than
class C_ParamExprBool_GtFloat: public C_ParamExprBool
{
public:
	virtual bool EvaluateBool(C_ExpressionContext& ctx)
	{
		return (m_A ? m_A->EvaluateFloat(ctx) : 0.0f) > (m_B ? m_B->EvaluateFloat(ctx) : 0.0f);
	}

public:
	C_ParamExprFloat* m_A;
	C_ParamExprFloat* m_B;
};

// Float Less Than
class C_ParamExprBool_LtFloat: public C_ParamExprBool
{
public:
	virtual bool EvaluateBool(C_ExpressionContext& ctx)
	{
		return (m_A ? m_A->EvaluateFloat(ctx): 0.0f) < (m_B ? m_B->EvaluateFloat(ctx): 0.0f);
	}

public:
	C_ParamExprFloat* m_A;
	C_ParamExprFloat* m_B;
};

// Actor Equal
class C_ParamExprBool_EqActor: public C_ParamExprBool
{
public:
	virtual bool EvaluateBool(C_ExpressionContext& ctx)
	{
		if(m_A && m_B)
			return m_A->EvaluateActor(ctx) == m_B->EvaluateActor(ctx);

		return false;
	}

public:
	C_ParamExprActor* m_A;
	C_ParamExprActor* m_B;
};

// Integer Expressions
// -------------------

class C_ParamExprInt_Arithmetic : public C_ParamExprInt
{
public:
	virtual S32 EvaluateInt(C_ExpressionContext& ctx)
	{
		return Calculate((m_A ? m_A->EvaluateInt(ctx) : 0), (m_B ? m_B->EvaluateInt(ctx) : 0));
	}
protected:
	virtual S32 Calculate(S32 a, S32 b) const = 0;
public:
	C_ParamExprInt* m_A;
	C_ParamExprInt* m_B;
};
// Add
class C_ParamExprInt_Add : public C_ParamExprInt_Arithmetic
{
protected:
	virtual S32 Calculate(S32 a, S32 b) const { return a + b; }
};

// Subtract
class C_ParamExprInt_Sub : public C_ParamExprInt_Arithmetic
{
protected:
	virtual S32 Calculate(S32 a, S32 b) const { return a - b; }
};

// Multiply
class C_ParamExprInt_Mul : public C_ParamExprInt_Arithmetic
{
protected:
	virtual S32 Calculate(S32 a, S32 b) const  { return a * b; }
};

// Divide
class C_ParamExprInt_Div : public C_ParamExprInt_Arithmetic
{
protected:
	virtual S32 Calculate(S32 a, S32 b) const { return (b != 0) ? (a / b) : 0; }
};

// Float Expressions
// -----------------

class C_ParamExprFloat_Arithmetic : public C_ParamExprFloat
{
public:
	virtual F32 EvaluateFloat(C_ExpressionContext& ctx)
	{
		return Calculate((m_A ? m_A->EvaluateFloat(ctx) : 0), (m_B ? m_B->EvaluateFloat(ctx) : 0));
	}
protected:
	virtual F32 Calculate(F32 a, F32 b) = 0;
public:
	C_ParamExprFloat* m_A;
	C_ParamExprFloat* m_B;
};

// Add
class C_ParamExprFloat_Add : public C_ParamExprFloat_Arithmetic
{
protected:
	virtual F32 Calculate(F32 a, F32 b){return a + b;}
};

// Subtract
class C_ParamExprFloat_Sub : public C_ParamExprFloat_Arithmetic
{
protected:
	virtual F32 Calculate(F32 a, F32 b){return a - b;}
};

// Multiply
class C_ParamExprFloat_Mul : public C_ParamExprFloat_Arithmetic
{
protected:
	virtual F32 Calculate(F32 a, F32 b){return a * b;}
};

// Divide
class C_ParamExprFloat_Div : public C_ParamExprFloat_Arithmetic
{
protected:
	virtual F32 Calculate(F32 a, F32 b){return (b != 0) ? (a / b) : 0;}
};

// Constants
// ---------

// Bool constant
class C_ParamExprBool_Const : public C_ParamExprBool
{
public:
	virtual bool EvaluateBool(C_ExpressionContext& ctx)
	{
		return m_Value ? m_Value->GetBool(ctx.m_ContextActor) : false;
	}

public:
	C_XtParamBool* m_Value;
};

// Integer constant
class C_ParamExprInt_Const : public C_ParamExprInt
{
public:
	virtual S32 EvaluateInt(C_ExpressionContext& ctx)
	{
		return m_Value ? m_Value->GetInteger(ctx.m_ContextActor) : 0;
	}

public:
	C_XtParamInteger* m_Value;
};

// Float constant
class C_ParamExprFloat_Const : public C_ParamExprFloat
{
public:
	virtual F32 EvaluateFloat(C_ExpressionContext& ctx)
	{
		return m_Value ? m_Value->GetFloat(ctx.m_ContextActor) : 0.0f;
	}

public:
	C_XtParamFloat* m_Value;
};

// Actor constant
class C_ParamExprActor_Const : public C_ParamExprActor
{
public:
	virtual Actor::C_ActorHandle EvaluateActor(C_ExpressionContext& ctx)
	{
		return m_Value ? m_Value->GetActorHandle(ctx.m_ContextActor) : Actor::C_ActorHandle();
	}

public:
	C_XtParamActor* m_Value;
};


// Boolean Expression 

// Result
class C_ParamExprResult_Bool
{
public:
	C_ParamExprBool* GetResult() const { return m_Result; }

	C_ParamExprBool* m_Result;
};

bool C_XtParamBoolExpression::GetBool(const Actor::C_Actor *pActorContext) const
{
	C_ExpressionContext ctx;
	ctx.m_ContextActor = pActorContext;
	return m_Result ? m_Result->GetResult()->EvaluateBool(ctx) : false;
}

// Integer Expression

// Result
class C_ParamExprResult_Integer
{
public:
	C_ParamExprInt* GetResult() const { return m_Result; }

	C_ParamExprInt* m_Result;
};

S32	C_XtParamIntegerExpression::GetInteger(const Actor::C_Actor *pActorContext) const
{
	C_ExpressionContext ctx;
	ctx.m_ContextActor = pActorContext;
	return m_Result ? m_Result->GetResult()->EvaluateInt(ctx) : false;
}

// Float Expression

// Result
class C_ParamExprResult_Float
{
public:
	C_ParamExprFloat* GetResult() const { return m_Result; }

	C_ParamExprFloat* m_Result;
};


F32	C_XtParamFloatExpression::GetFloat(const Actor::C_Actor *pActorContext) const
{
	C_ExpressionContext ctx;
	ctx.m_ContextActor = pActorContext;
	return m_Result ? m_Result->GetResult()->EvaluateFloat(ctx) : false;
}

}// namespace Actors

// Xt Declarations
XT_TYPE(Actors::C_ParamExpr, "ParamExpr")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprBool, "ParamExprBool")
XT_BASE(Actors::C_ParamExpr)
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprInt, "ParamExprInt")
XT_BASE(Actors::C_ParamExpr)
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprFloat, "ParamExprFloat")
XT_BASE(Actors::C_ParamExpr)
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprActor, "ParamExprActor")
XT_BASE(Actors::C_ParamExpr)
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprBool_And, "ParamExprBool_And")
XT_BASE(Actors::C_ParamExprBool)
XT_FIELD(m_A, "a")
XT_FIELD(m_B, "b")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprBool_Or, "ParamExprBool_Or")
XT_BASE(Actors::C_ParamExprBool)
XT_FIELD(m_A, "a")
XT_FIELD(m_B, "b")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprBool_Nand, "ParamExprBool_Nand")
XT_BASE(Actors::C_ParamExprBool)
XT_FIELD(m_A, "a")
XT_FIELD(m_B, "b")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprBool_Nor, "ParamExprBool_Nor")
XT_BASE(Actors::C_ParamExprBool)
XT_FIELD(m_A, "a")
XT_FIELD(m_B, "b")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprBool_Xor, "ParamExprBool_Xor")
XT_BASE(Actors::C_ParamExprBool)
XT_FIELD(m_A, "a")
XT_FIELD(m_B, "b")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprBool_Not, "ParamExprBool_Not")
XT_BASE(Actors::C_ParamExprBool)
XT_FIELD(m_A, "a")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprBool_EqInt, "ParamExprBool_EqInt")
XT_BASE(Actors::C_ParamExprBool)
XT_FIELD(m_A, "a")
XT_FIELD(m_B, "b")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprBool_GtInt, "ParamExprBool_GtInt")
XT_BASE(Actors::C_ParamExprBool)
XT_FIELD(m_A, "a")
XT_FIELD(m_B, "b")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprBool_LtInt, "ParamExprBool_LtInt")
XT_BASE(Actors::C_ParamExprBool)
XT_FIELD(m_A, "a")
XT_FIELD(m_B, "b")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprBool_GtFloat, "ParamExprBool_GtFloat")
XT_BASE(Actors::C_ParamExprBool)
XT_FIELD(m_A, "a")
XT_FIELD(m_B, "b")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprBool_LtFloat, "ParamExprBool_LtFloat")
XT_BASE(Actors::C_ParamExprBool)
XT_FIELD(m_A, "a")
XT_FIELD(m_B, "b")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprBool_EqActor, "ParamExprBool_EqActor")
XT_BASE(Actors::C_ParamExprBool)
XT_FIELD(m_A, "a")
XT_FIELD(m_B, "b")
XT_END_TYPE

XT_ABSTRACT_TYPE(Actors::C_ParamExprInt_Arithmetic,"ParamExprInt_Arithmetic")
XT_BASE(Actors::C_ParamExprInt)
XT_FIELD(m_A, "a")
XT_FIELD(m_B, "b")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprInt_Add,"ParamExprInt_Add")
XT_BASE(Actors::C_ParamExprInt_Arithmetic)
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprInt_Sub,"ParamExprInt_Sub")
XT_BASE(Actors::C_ParamExprInt_Arithmetic)
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprInt_Mul,"ParamExprInt_Mul")
XT_BASE(Actors::C_ParamExprInt_Arithmetic)
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprInt_Div,"ParamExprInt_Div")
XT_BASE(Actors::C_ParamExprInt_Arithmetic)
XT_END_TYPE


XT_ABSTRACT_TYPE(Actors::C_ParamExprFloat_Arithmetic,"ParamExprFloat_Arithmetic")
XT_BASE(Actors::C_ParamExprFloat)
XT_FIELD(m_A, "a")
XT_FIELD(m_B, "b")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprFloat_Add,"ParamExprFloat_Add")
XT_BASE(Actors::C_ParamExprFloat_Arithmetic)
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprFloat_Sub,"ParamExprFloat_Sub")
XT_BASE(Actors::C_ParamExprFloat_Arithmetic)
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprFloat_Mul,"ParamExprFloat_Mul")
XT_BASE(Actors::C_ParamExprFloat_Arithmetic)
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprFloat_Div,"ParamExprFloat_Div")
XT_BASE(Actors::C_ParamExprFloat_Arithmetic)
XT_END_TYPE


XT_TYPE(Actors::C_ParamExprBool_Const, "ParamExprBool_Const")
XT_BASE(Actors::C_ParamExprBool)
XT_FIELD(m_Value, "value")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprInt_Const, "ParamExprInt_Const")
XT_BASE(Actors::C_ParamExprInt)
XT_FIELD(m_Value, "value")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprFloat_Const, "ParamExprFloat_Const")
XT_BASE(Actors::C_ParamExprFloat)
XT_FIELD(m_Value, "value")
XT_END_TYPE

XT_TYPE(Actors::C_ParamExprActor_Const, "ParamExprActor_Const")
XT_BASE(Actors::C_ParamExprActor)
XT_FIELD(m_Value, "value")
XT_END_TYPE

// Boolean Expression
XT_TYPE(Actors::C_ParamExprResult_Bool, "ParamExprResult_Bool")
XT_FIELD(m_Result, "result")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamBoolExpression, "ParamBoolExpression")
XT_BASE(Actors::C_XtParamBool)
XT_FIELD(m_Result, "result")
XT_FIELD(m_Exprs, "exprs")
XT_END_TYPE

// Integer Expression
XT_TYPE(Actors::C_ParamExprResult_Integer, "ParamExprResult_Integer")
XT_FIELD(m_Result, "result")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamIntegerExpression, "ParamIntegerExpression")
XT_BASE(Actors::C_XtParamInteger)
XT_FIELD(m_Result, "result")
XT_FIELD(m_Exprs, "exprs")
XT_END_TYPE

// Float Expression
XT_TYPE(Actors::C_ParamExprResult_Float, "ParamExprResult_Float")
XT_FIELD(m_Result, "result")
XT_END_TYPE

XT_TYPE(Actors::C_XtParamFloatExpression, "ParamFloatExpression")
XT_BASE(Actors::C_XtParamFloat)
XT_FIELD(m_Result, "result")
XT_FIELD(m_Exprs, "exprs")
XT_END_TYPE



// Register Xt Classes
XT_LIBRARY(ExpressionEvaluator)
XT_REGISTER_TYPE(Actors::C_ParamExpr)
XT_REGISTER_TYPE(Actors::C_ParamExprBool)
XT_REGISTER_TYPE(Actors::C_ParamExprInt)
XT_REGISTER_TYPE(Actors::C_ParamExprFloat)
XT_REGISTER_TYPE(Actors::C_ParamExprActor)

XT_REGISTER_TYPE(Actors::C_ParamExprBool_And)
XT_REGISTER_TYPE(Actors::C_ParamExprBool_Or)
XT_REGISTER_TYPE(Actors::C_ParamExprBool_Nand)
XT_REGISTER_TYPE(Actors::C_ParamExprBool_Nor)
XT_REGISTER_TYPE(Actors::C_ParamExprBool_Xor)
XT_REGISTER_TYPE(Actors::C_ParamExprBool_Not)

XT_REGISTER_TYPE(Actors::C_ParamExprBool_EqInt)
XT_REGISTER_TYPE(Actors::C_ParamExprBool_GtInt)
XT_REGISTER_TYPE(Actors::C_ParamExprBool_LtInt)

XT_REGISTER_TYPE(Actors::C_ParamExprBool_GtFloat)
XT_REGISTER_TYPE(Actors::C_ParamExprBool_LtFloat)

XT_REGISTER_TYPE(Actors::C_ParamExprBool_EqActor)

XT_REGISTER_TYPE(Actors::C_ParamExprInt_Arithmetic)
XT_REGISTER_TYPE(Actors::C_ParamExprInt_Add)
XT_REGISTER_TYPE(Actors::C_ParamExprInt_Sub)
XT_REGISTER_TYPE(Actors::C_ParamExprInt_Mul)
XT_REGISTER_TYPE(Actors::C_ParamExprInt_Div)

XT_REGISTER_TYPE(Actors::C_ParamExprFloat_Arithmetic)
XT_REGISTER_TYPE(Actors::C_ParamExprFloat_Add)
XT_REGISTER_TYPE(Actors::C_ParamExprFloat_Sub)
XT_REGISTER_TYPE(Actors::C_ParamExprFloat_Mul)
XT_REGISTER_TYPE(Actors::C_ParamExprFloat_Div)

XT_REGISTER_TYPE(Actors::C_ParamExprBool_Const)
XT_REGISTER_TYPE(Actors::C_ParamExprInt_Const)
XT_REGISTER_TYPE(Actors::C_ParamExprFloat_Const)
XT_REGISTER_TYPE(Actors::C_ParamExprActor_Const)

XT_REGISTER_TYPE(Actors::C_ParamExprResult_Bool)
XT_REGISTER_TYPE(Actors::C_XtParamBoolExpression)

XT_REGISTER_TYPE(Actors::C_ParamExprResult_Integer)
XT_REGISTER_TYPE(Actors::C_XtParamIntegerExpression)

XT_REGISTER_TYPE(Actors::C_ParamExprResult_Float)
XT_REGISTER_TYPE(Actors::C_XtParamFloatExpression)

XT_END_LIBRARY

