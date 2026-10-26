#include <stdafx.h>

#include <Btl/Containers/vector.h>

#include <Misc_Shared/NewCurve/Definitions.h>
#include <Misc_Shared/NewCurve/CurveHelpers.h>
#include <Misc_Shared/NewCurve/CurveData.h>
#include <Misc_Shared/NewCurve/Curve.h>
#include <Misc_Shared/NewCurve/HermitePolicies.h>
#include <Misc_Shared/NewCurve/MayaHermiteSpline.h>
#include <Misc_Shared/NewCurve/Polyline.h>

#define AFTK_ANIMATION_CURVE_NODE_NUM_STATIC_PLUGS 1
#define AFTK_ANIMATION_CURVE_NODE_STATIC_PLUG_INPUT 0

namespace Aftk
{

class C_HermiteCurveData : public Curve::C_CurveData<Curve::C_MayaHermiteSplineKnotU<F32> >
{
};

class C_HermiteCurveEval : public Curve::C_MayaHermiteSpline<F32, Curve::C_CurveData<Curve::C_MayaHermiteSplineKnotU<F32> >, 
		Curve::C_HermiteCtrlPt1DTanAngle<F32, Curve::C_CurveData<Curve::C_MayaHermiteSplineKnotU<F32> > > >
{
public:
	C_HermiteCurveEval(Curve::C_MayaHermiteSplineFlags flags = Curve::C_MayaHermiteSplineFlags()) : 
	Curve::C_MayaHermiteSpline<F32, Curve::C_CurveData<Curve::C_MayaHermiteSplineKnotU<F32> >, 
		Curve::C_HermiteCtrlPt1DTanAngle<F32, Curve::C_CurveData<Curve::C_MayaHermiteSplineKnotU<F32> > > >(flags) {}
};

class C_PolyLineCurveData : public Curve::C_CurveData<Curve::C_KnotU<F32> >
{
};

class C_PolyLineEval : public Curve::C_PolyLine<
	F32, Curve::C_CurveData<Curve::C_KnotU<F32> > >
{
public:
	C_PolyLineEval(Curve::C_PolyLineFlags flags = Curve::C_PolyLineFlags()) : 
		Curve::C_PolyLine<
			F32, Curve::C_CurveData<Curve::C_KnotU<F32> > >(flags) {}
};

C_AnimationCurveNode::C_AnimationCurveNode(C_Scene* pScene) 
: 
I_EvaluatableNode(pScene),
m_inputPlug(OnInputPlugChanged)
{
	OneTimeInit();
}

C_Node* C_AnimationCurveNode::Clone(C_Scene* pScene) const
{
	C_Node* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_AnimationCurveNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_AnimationCurveNode(pScene);
		(C_AnimationCurveNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_AnimationCurveNode::C_AnimationCurveNode(const C_AnimationCurveNode& rhs)
:
I_EvaluatableNode(0),
m_inputPlug(OnInputPlugChanged)
{
	OneTimeInit();
	*this = rhs;
}

C_AnimationCurveNode::~C_AnimationCurveNode()
{
	Clear();
}

const C_AnimationCurveNode& C_AnimationCurveNode::operator =(const C_AnimationCurveNode& rhs)
{
	if (&rhs == this) return *this;

	//Delete existing curve data.
	ClearThis();

	C_Node::operator =(rhs);

	m_isTimeInput = rhs.m_isTimeInput;
	m_inputPlug	= rhs.m_inputPlug;
	m_curveType	= rhs.m_curveType;

	DestroyCurve();
	CreateCurve(rhs.m_curveType);

	switch(m_curveType)
	{
	case E_AnimCurveType_Linear:
	{
		((C_PolyLineCurveData*)m_pCurveData)->GetKnots() = 
			((C_PolyLineCurveData*)rhs.m_pCurveData)->GetKnots();
		break;
	}
	case E_AnimCurveType_Hermite:
	{
		((C_HermiteCurveData*)m_pCurveData)->GetKnots() = 
			((C_HermiteCurveData*)rhs.m_pCurveData)->GetKnots();
		break;
	}
	default:
		break;
	}

	//Note that we don't copy plug connectivity information.
	return *this;
}

void C_AnimationCurveNode::CreateCurve(E_AnimCurveType type)
{
	DestroyCurve();
	
	m_curveType = type;
	switch (m_curveType)
	{
	case E_AnimCurveType_Linear:
		{
			m_pCurveData = new C_PolyLineCurveData();
			m_pCurveEval = new C_PolyLineEval();
			((C_PolyLineEval*)m_pCurveEval)->SetData((C_PolyLineCurveData*)m_pCurveData);
		}
		break;
	case E_AnimCurveType_Hermite:
		{
			m_pCurveData = new C_HermiteCurveData();
			m_pCurveEval = new C_HermiteCurveEval();
			((C_HermiteCurveEval*)m_pCurveEval)->SetData((C_HermiteCurveData*)m_pCurveData);
		}
		break;
	}
}

void C_AnimationCurveNode::DestroyCurve()
{
	switch (m_curveType)
	{
	case E_AnimCurveType_Linear:
		{
			delete (((C_PolyLineCurveData*)m_pCurveData));
			delete (((C_PolyLineEval*)m_pCurveEval));
			m_pCurveData = 0;
			m_pCurveEval = 0;
		}
		break;
	case E_AnimCurveType_Hermite:
		{
			delete(((C_HermiteCurveData*)m_pCurveData));
			delete(((C_HermiteCurveEval*)m_pCurveEval));
			m_pCurveData = 0;
			m_pCurveEval = 0;
		}
		break;
	default:
		break;
	}
}

S32 C_AnimationCurveNode::AddKey(const C_LinearKey& key)
{
	if (m_curveType != E_AnimCurveType_Linear && m_curveType != E_AnimCurveType_Unknown)
		//We cannot add a key to this curve as the given key is of the wrong type.
		return -1;
	if (!m_pCurveData)
		CreateCurve(E_AnimCurveType_Linear);

	Btl::Vector<C_PolyLineCurveData::KnotType>& knots = 
		((C_PolyLineCurveData*)m_pCurveData)->GetKnots();

	knots.PushBack();
	C_PolyLineCurveData::KnotType& knot = knots.Back();

	knot.SetU(key.GetX());
	knot.value = key.GetValue();
	return knots.Size()-1;
}

S32 C_AnimationCurveNode::AddKey(const C_HermiteKey& key)
{
	if (m_curveType != E_AnimCurveType_Hermite && m_curveType != E_AnimCurveType_Unknown)
		//We cannot add a key to this curve as the given key is of the wrong type.
		return -1;
	if (!m_pCurveData)
		CreateCurve(E_AnimCurveType_Hermite);

	Btl::Vector<C_HermiteCurveData::KnotType>& knots = 
		((C_HermiteCurveData*)m_pCurveData)->GetKnots();

	knots.PushBack();
	C_HermiteCurveData::KnotType& knot = knots.Back();

	knot.SetU(key.GetX());
	knot.value = key.GetValue();
	knot.tangentIn = tanf(key.GetInTangentAngle());
	knot.tangentOut = tanf(key.GetOutTangentAngle());
	knot.tangentInType = AftkTangentToCurveTangent(key.GetInTangentType());	
	knot.tangentOutType = AftkTangentToCurveTangent(key.GetOutTangentType());	
	return knots.Size()-1;
}


bool C_AnimationCurveNode::SetKey(U32 keyIndex, const C_LinearKey& key)
{
	if (m_curveType != E_AnimCurveType_Linear)
		return false;

	Btl::Vector<C_PolyLineCurveData::KnotType>& knots = 
		((C_PolyLineCurveData*)m_pCurveData)->GetKnots();
	if (knots.Size() == 0)
		return false;
	C_PolyLineCurveData::KnotType& knot = knots[keyIndex];
	
	knot.SetU(key.GetX());
	knot.value = key.GetValue();
	return true;
}

bool C_AnimationCurveNode::SetKey(U32 keyIndex, const C_HermiteKey& key)
{
	if (m_curveType != E_AnimCurveType_Hermite)
		return false;

	Btl::Vector<C_HermiteCurveData::KnotType>& knots = 
		((C_HermiteCurveData*)m_pCurveData)->GetKnots();
	if (knots.Size() == 0)
		return false;
	C_HermiteCurveData::KnotType& knot = knots[keyIndex];
	
	knot.SetU(key.GetX());
	knot.value = key.GetValue();
	knot.tangentIn = tanf(key.GetInTangentAngle());
	knot.tangentOut = tanf(key.GetOutTangentAngle());
	knot.tangentInType = AftkTangentToCurveTangent(key.GetInTangentType());	
	knot.tangentOutType = AftkTangentToCurveTangent(key.GetOutTangentType());	
	return true;
}

U32 C_AnimationCurveNode::GetNumKeys() const
{
	switch(m_curveType)
	{
	case E_AnimCurveType_Linear:
		return ((C_PolyLineCurveData*)m_pCurveData)->GetNumKnots();
	case E_AnimCurveType_Hermite:
		return ((C_HermiteCurveData*)m_pCurveData)->GetNumKnots();
	default:
		return 0;
	};
}

F32 C_AnimationCurveNode::GetStartX() const
{
	switch(m_curveType)
	{
	case E_AnimCurveType_Linear:
		return ((C_PolyLineEval*)m_pCurveEval)->GetMinU();
	case E_AnimCurveType_Hermite:
		return ((C_HermiteCurveEval*)m_pCurveEval)->GetMinU();
	default:
		return 0;
	};
}

F32 C_AnimationCurveNode::GetEndX() const
{
	switch(m_curveType)
	{
	case E_AnimCurveType_Linear:
		return ((C_PolyLineEval*)m_pCurveEval)->GetMaxU();
	case E_AnimCurveType_Hermite:
		return ((C_HermiteCurveEval*)m_pCurveEval)->GetMaxU();
	default:
		return 0;
	};
}

bool C_AnimationCurveNode::GetAdjacentKeyIndices(S32& keyIndex, S32& keyIndex2, F32 x) const
{
	switch(m_curveType)
	{
	case E_AnimCurveType_Linear:
		BinarySearchForKeyPair(
			keyIndex, 
			keyIndex2, 
			*(C_PolyLineCurveData*)m_pCurveData, 
			x);
		break;
	case E_AnimCurveType_Hermite:
		BinarySearchForKeyPair(
			keyIndex, 
			keyIndex2, 
			*(C_HermiteCurveData*)m_pCurveData, 
			x);
		break;
	};

	if ((keyIndex == -1) && (keyIndex2 == -1))
		return false;

	return true;
}

bool C_AnimationCurveNode::GetAdjacentKeys(C_BaseKey& key1, C_BaseKey& key2, F32 x) const
{
	S32 keyIndex1;
	S32 keyIndex2;
	GetAdjacentKeyIndices(keyIndex1, keyIndex2, x);

	if (keyIndex1 == -1)
		return false;

	if (keyIndex2 == -1)
		keyIndex2 = keyIndex1;

	switch(m_curveType)
	{
	case E_AnimCurveType_Linear:
	{
		if (!GetKey((C_LinearKey&)key1, keyIndex1))
			return false;
		if (!GetKey((C_LinearKey&)key2, keyIndex2))
			return false;
		return true;
	}
	case E_AnimCurveType_Hermite:
		if (!GetKey((C_HermiteKey&)key1, keyIndex1))
			return false;
		if (!GetKey((C_HermiteKey&)key2, keyIndex2))
			return false;
		return true;
	}
	return false;
}

bool C_AnimationCurveNode::GetAdjacentKeys(C_LinearKey& key1, C_LinearKey& key2, F32 x) const
{
	if (m_curveType != E_AnimCurveType_Linear)
		//Curve is of the incorrect type.
		return false;

	return GetAdjacentKeys((C_BaseKey&)key1, (C_BaseKey&)key2, x);
}

bool C_AnimationCurveNode::GetAdjacentKeys(C_HermiteKey& key1, C_HermiteKey& key2, F32 x) const
{
	if (m_curveType != E_AnimCurveType_Hermite)
		//Curve is of the incorrect type.
		return false;

	return GetAdjacentKeys((C_BaseKey&)key1, (C_BaseKey&)key2, x);
}

bool C_AnimationCurveNode::GetAdjacentKeys(C_BaseKey& key1, C_BaseKey& key2) const
{
	F32 x;
	if (m_isTimeInput)
		x = m_pScene->GetCurrentTime();
	else if (!m_inputPlug.GetValue(x))
		x = 0;

	return GetAdjacentKeys((C_BaseKey&)key1, (C_BaseKey&)key2, x);
}

bool C_AnimationCurveNode::GetAdjacentKeys(C_LinearKey& key1, C_LinearKey& key2) const
{
	if (m_curveType != E_AnimCurveType_Linear)
		//Curve is of the incorrect type.
		return false;

	return GetAdjacentKeys((C_BaseKey&)key1, (C_BaseKey&)key2);
}

bool C_AnimationCurveNode::GetAdjacentKeys(C_HermiteKey& key1, C_HermiteKey& key2) const
{
	if (m_curveType != E_AnimCurveType_Hermite)
		//Curve is of the incorrect type.
		return false;

	return GetAdjacentKeys((C_BaseKey&)key1, (C_BaseKey&)key2);
}

bool C_AnimationCurveNode::Evaluate(F32& value) const
{
	if (m_isTimeInput)
		return Evaluate(value, m_pScene->GetCurrentTime());
	
	F32 x;
	if (m_inputPlug.GetValue(x))
		return Evaluate(value, x);
	else
	{
		value = 0.f;
		return false;
	}
}

bool C_AnimationCurveNode::Evaluate(F64& value) const
{
	F32 fValue = 0.f;
	if (Evaluate(fValue))
	{
		// Rounding is important here.
		value = static_cast<F64>(fValue);
		return true;
	}
	value = 0.0;
	return false;
}

bool C_AnimationCurveNode::Evaluate(S32& value) const
{ 
	F32 fValue = 0.f;
	if (Evaluate(fValue))
	{
		// Rounding is important here.
		value = static_cast<S32>(fValue + 0.5f);
		return true;
	}
	value = 0;
	return false;
}

bool C_AnimationCurveNode::Evaluate(bool& value) const
{ 
	F32 fValue = 0.f;
	if (Evaluate(fValue))
	{
		value = fabsf(fValue) >= 0.5f;
		return true;
	}
	value = false;
	return false;
}

bool C_AnimationCurveNode::Evaluate(F32& value, F32 x) const
{
	switch(m_curveType)
	{
		case E_AnimCurveType_Linear:
			return EvaluateLinear(value, x);
		case E_AnimCurveType_Hermite:
			return EvaluateHermite(value, x);
	}

	return false;
}

bool C_AnimationCurveNode::EvaluateLinear(F32& value, F32 x) const
{
	switch(m_curveType)
	{
		case E_AnimCurveType_Linear:
			value = ((C_PolyLineEval*)m_pCurveEval)->Evaluate(x);
			return true;
		case E_AnimCurveType_Hermite:
			((C_HermiteCurveEval*)m_pCurveEval)->SetPreInfinity(AftkInfinityToCurveInfinity(m_preInfinity));
			((C_HermiteCurveEval*)m_pCurveEval)->SetPostInfinity(AftkInfinityToCurveInfinity(m_postInfinity));
			value = ((C_HermiteCurveEval*)m_pCurveEval)->EvaluateLinear(x);
			return true;
		default:
			break;
	}
	return false;
}

bool C_AnimationCurveNode::EvaluateHermite(F32& value, F32 x) const
{
	switch(m_curveType)
	{
		case E_AnimCurveType_Linear:
			return false;
		case E_AnimCurveType_Hermite:
			((C_HermiteCurveEval*)m_pCurveEval)->SetPreInfinity(AftkInfinityToCurveInfinity(m_preInfinity));
			((C_HermiteCurveEval*)m_pCurveEval)->SetPostInfinity(AftkInfinityToCurveInfinity(m_postInfinity));
			value = ((C_HermiteCurveEval*)m_pCurveEval)->Evaluate(x);
			return true;
		default:
			break;
	}
	return false;
}

C_NullPlug& C_AnimationCurveNode::GetInputPlug()
{
	return m_inputPlug;
}

const C_NullPlug& C_AnimationCurveNode::GetInputPlug() const
{
	return m_inputPlug;
}

void C_AnimationCurveNode::SetIsTimeInput(bool isTimeInput)
{
	m_isTimeInput = isTimeInput;
}

bool C_AnimationCurveNode::IsTimeInput() const
{
	return m_isTimeInput;
}

bool C_AnimationCurveNode::GetKey(C_LinearKey& key, U32 keyIndex) const
{
	if (!m_pCurveData)
		return false;

	if (m_curveType != E_AnimCurveType_Linear)
		return false;

	const Btl::Vector<C_PolyLineCurveData::KnotType>& knots = 
		((C_PolyLineCurveData*)m_pCurveData)->GetKnots();
	const C_PolyLineCurveData::KnotType& knot = knots[keyIndex];
	key.SetX(knot.GetU(keyIndex, knots.Size()));
	key.SetValue(knot.value);
	return true;
}

bool C_AnimationCurveNode::GetKey(C_HermiteKey& key, U32 keyIndex) const
{
	if (!m_pCurveData)
		return false;

	if (m_curveType != E_AnimCurveType_Hermite)
		return false;

	const Btl::Vector<C_HermiteCurveData::KnotType>& knots = 
		((C_HermiteCurveData*)m_pCurveData)->GetKnots();
	const C_HermiteCurveData::KnotType& knot = knots[keyIndex];
	key.SetX(knot.GetU(keyIndex, knots.Size()));
	key.SetValue(knot.value);
	key.SetInTangentAngle(atanf(knot.tangentIn));
	key.SetOutTangentAngle(atanf(knot.tangentOut));
	key.SetInTangentType(CurveTangentToAftkTangent(knot.tangentInType));
	key.SetOutTangentType(CurveTangentToAftkTangent(knot.tangentOutType));
	return true;
}

E_AnimCurveType C_AnimationCurveNode::GetCurveType() const
{
	return m_curveType;
}

E_ObjType C_AnimationCurveNode::GetType() const
{
	return E_ObjType_AnimationCurve;
}

bool C_AnimationCurveNode::HasType(E_ObjType type) const
{
	if (I_EvaluatableNode::HasType(type))
		return true;
	if (type == E_ObjType_AnimationCurve) 
		return true;

	return false;
}

U32 C_AnimationCurveNode::GetHash(E_ObjType classToAbort) const
{
	return I_EvaluatableNode::GetHash(classToAbort);
}

bool C_AnimationCurveNode::IsUsed() const
{
	return I_EvaluatableNode::IsUsed();
}

const I_Plug* C_AnimationCurveNode::GetStaticPlugPtr(U32 instance) const
{
	U32 staticOffset = I_EvaluatableNode::GetNumStaticPlugs();
	if (instance < staticOffset)
		return I_EvaluatableNode::GetStaticPlugPtr(instance);
	
	instance -= staticOffset;
	if (instance == 0)
		return &m_inputPlug;

	return 0;
}

U32	C_AnimationCurveNode::GetNumStaticPlugs() const
{
	return C_Node::GetNumStaticPlugs() + AFTK_ANIMATION_CURVE_NODE_NUM_STATIC_PLUGS;
}

void C_AnimationCurveNode::SetPreInfinity(E_CurveInfinityType type)
{
	m_preInfinity = type;
}

void C_AnimationCurveNode::SetPostInfinity(E_CurveInfinityType type)
{
	m_postInfinity = type;
}

E_CurveInfinityType C_AnimationCurveNode::GetPreInfinity() const
{
	return m_preInfinity;
}

E_CurveInfinityType C_AnimationCurveNode::GetPostInfinity() const
{
	return m_postInfinity;
}

void C_AnimationCurveNode::Clear()
{
	ClearThis();
	C_Node::Clear();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_AnimationCurveNode::OneTimeInit()
{
	m_inputPlug.SetName("input");
	m_inputPlug.SetLogicalIndex(0);
	m_inputPlug.SetParentPtr(this);
	m_pCurveData = 0;
	m_pCurveEval = 0;

	ClearThis();
}

void C_AnimationCurveNode::ClearThis()
{
	m_isTimeInput = true;

	DestroyCurve();

	m_curveType = E_AnimCurveType_Unknown;
	m_preInfinity = E_CurveInfinityType_Constant;
	m_postInfinity = E_CurveInfinityType_Constant;
}

void C_AnimationCurveNode::OnInputPlugChanged(void* _pPlug)
{
	I_Plug* pPlug = (I_Plug*)_pPlug;

	C_AnimationCurveNode* pCurve;
	if (pPlug->IsPlugChild())
		pCurve = (C_AnimationCurveNode*)((I_Plug*)pPlug->GetParentPtr())->GetParentPtr();
	else
		pCurve = (C_AnimationCurveNode*)pPlug->GetTopLevelParentPtr();

	AFTK_ASSERT(pCurve && pCurve->HasType(E_ObjType_AnimationCurve));

	//Force an update of nodes evaluating us.
	for (U32 i = 0; i < (U32)pCurve->m_connectedPlugs.Size(); i++)
	{
		pCurve->m_connectedPlugs[i]->OnChanged();
	}
}

E_CurveTangentType C_AnimationCurveNode::CurveTangentToAftkTangent(Curve::E_TangentType type) const
{
	switch(type)
	{
	case Curve::E_TangentType_Stepped:
		return E_CurveTangentType_Stepped;

	case Curve::E_TangentType_Clamped:
		return E_CurveTangentType_Clamped;

	case Curve::E_TangentType_SteppedNext:
		return E_CurveTangentType_SteppedNext;

	case Curve::E_TangentType_Unknown:
	default:
		break;
	};

	return E_CurveTangentType_Unknown;
}

Curve::E_TangentType C_AnimationCurveNode::AftkTangentToCurveTangent(E_CurveTangentType type) const
{
	switch(type)
	{
	case E_CurveTangentType_Stepped:
		return Curve::E_TangentType_Stepped;
	case E_CurveTangentType_Clamped:
		return Curve::E_TangentType_Clamped;
	case E_CurveTangentType_SteppedNext:
		return Curve::E_TangentType_SteppedNext;
	case Curve::E_TangentType_Unknown:
	default:
		break;
	};

	return Curve::E_TangentType_Unknown;
}

E_CurveInfinityType C_AnimationCurveNode::CurveInfinityToAftkInfinity(Curve::E_InfinityType type) const
{
	switch(type)
	{
	case Curve::E_InfinityType_Linear:
		return E_CurveInfinityType_Linear;
	case Curve::E_InfinityType_Cycle:
		return E_CurveInfinityType_Cycle;
	case Curve::E_InfinityType_CycleWithOffset:
		return E_CurveInfinityType_CycleWithOffset;
	case Curve::E_InfinityType_Oscillate:
		return E_CurveInfinityType_Oscillate;
	case Curve::E_InfinityType_Constant:
	default:
		break;
	};

	return E_CurveInfinityType_Constant;
}

Curve::E_InfinityType C_AnimationCurveNode::AftkInfinityToCurveInfinity(E_CurveInfinityType type) const
{
	switch(type)
	{
	case E_CurveInfinityType_Linear:
		return Curve::E_InfinityType_Linear;
	case E_CurveInfinityType_Cycle:
		return Curve::E_InfinityType_Cycle;
	case E_CurveInfinityType_CycleWithOffset:
		return Curve::E_InfinityType_CycleWithOffset;
	case E_CurveInfinityType_Oscillate:
		return Curve::E_InfinityType_Oscillate;
	case E_CurveInfinityType_Constant:
	default:
		break;
	};

	return Curve::E_InfinityType_Constant;
}

}

#endif
