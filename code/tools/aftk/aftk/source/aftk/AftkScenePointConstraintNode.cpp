#include <stdafx.h>

#define NUM_STATIC_PLUGS 4
#define STATIC_PLUG_TRANSLATE_OFFSET 0
#define STATIC_PLUG_POLARITY 1
#define STATIC_PLUG_REST_TRANSLATE 2
#define STATIC_PLUG_CONSTRAINT_TRANSLATE 3

using namespace Aftk;

C_ScenePointConstraintNode::C_Target::C_Target(C_ScenePointConstraintNode* pParent) : 
	targetTranslatePlug(OnInputPlugChanged),
	targetWeightPlug(E_GenericAttributeType_F32, OnInputPlugChanged),
	targetParentPlug(OnInputPlugChanged)
{
	targetParentPlug.SetLogicalIndex(0);
	targetParentPlug.SetParentPtr(pParent);
	targetParentPlug.SetName("targetParent");
	targetParentPlug.SetChildrenLocked(true);

	{
		targetTranslatePlug.SetLogicalIndex(1);
		targetTranslatePlug.SetParentPtr(pParent);
		targetTranslatePlug.SetName("targetTranslate");
		targetTranslatePlug.m_children.Reserve(3);
		
		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		targetTranslatePlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		pPlugX->SetValue(0.f, false);
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		targetTranslatePlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		pPlugY->SetValue(0.f, false);
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		targetTranslatePlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
		pPlugZ->SetValue(0.f, false);

		targetTranslatePlug.SetChildrenLocked(true);
	}

	targetWeightPlug.SetLogicalIndex(2);
	targetWeightPlug.SetParentPtr(pParent);
	targetWeightPlug.SetName("targetWeight");
	targetWeightPlug.SetValue(1.f, false);
	targetWeightPlug.SetChildrenLocked(true);
}

C_ScenePointConstraintNode::C_ScenePointConstraintNode(C_Scene* pScene) : 
I_SceneConstraintNode(pScene), C_LimitMixin(), 
m_translateOffsetPlug(OnInputPlugChanged),
m_offsetPolarityPlug(E_GenericAttributeType_F32, OnInputPlugChanged),
m_restTranslatePlug(OnInputPlugChanged),
m_constraintTranslatePlug(0)
{
	OneTimeInit();
}

C_Node* C_ScenePointConstraintNode::Clone(C_Scene* pScene) const
{
	C_ScenePointConstraintNode* p = new C_ScenePointConstraintNode(pScene);
	if (!p)
		return 0;

	*p = *this;
	return p;
}

C_ScenePointConstraintNode::~C_ScenePointConstraintNode()
{
	ClearThis(true);
}

C_ScenePointConstraintNode::C_ScenePointConstraintNode(const C_ScenePointConstraintNode& rhs) 
 : I_SceneConstraintNode(0),
m_translateOffsetPlug(OnInputPlugChanged),
m_offsetPolarityPlug(E_GenericAttributeType_F32, OnInputPlugChanged),
m_restTranslatePlug(OnInputPlugChanged),
m_constraintTranslatePlug(0)
{
	OneTimeInit();
	*this = rhs;
}

const C_ScenePointConstraintNode& C_ScenePointConstraintNode::operator=(const C_ScenePointConstraintNode& rhs)
{
	C_SceneNode::operator=(rhs);
	C_LimitMixin::operator=(rhs);

	SetNumInputs(rhs.GetNumInputs());
	for (U32 i = 0; i < (U32)rhs.m_inputs.Size(); i++)
	{
		m_inputs[i]->targetTranslatePlug = rhs.m_inputs[i]->targetTranslatePlug;
		m_inputs[i]->targetWeightPlug = rhs.m_inputs[i]->targetWeightPlug;
		m_inputs[i]->targetParentPlug = rhs.m_inputs[i]->targetParentPlug;
	}

	m_constraintTranslatePlug = rhs.m_constraintTranslatePlug;
	m_translateOffsetPlug = rhs.m_translateOffsetPlug;
	m_offsetPolarityPlug = rhs.m_offsetPolarityPlug;
	m_restTranslatePlug = rhs.m_restTranslatePlug;
	m_constraintTranslatePlug = rhs.m_constraintTranslatePlug;

	return *this;
}

U32 C_ScenePointConstraintNode::GetHash(E_ObjType classToAbortTest) const
{
	return I_SceneConstraintNode::GetHash(classToAbortTest);
}

E_ObjType C_ScenePointConstraintNode::GetType() const
{
	return Aftk::E_ObjType_ScenePointConstraint;
}

bool C_ScenePointConstraintNode::HasType(E_ObjType type) const
{
	if (I_SceneConstraintNode::HasType(type))
		return true;
	if (type == E_ObjType_ScenePointConstraint) 
		return true;

	return false;
}

bool C_ScenePointConstraintNode::IsUsed() const 
{
	return C_SceneNode::IsUsed();
}

bool C_ScenePointConstraintNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

bool C_ScenePointConstraintNode::IsEqualToThis(const I_Object &rhs, U32 flags) const
{
	return true;
}

C_NullPlug& C_ScenePointConstraintNode::GetTranslateOffsetPlug()
{
	return m_translateOffsetPlug;
}

const C_NullPlug& C_ScenePointConstraintNode::GetTranslateOffsetPlug() const
{
	return m_translateOffsetPlug;
}

C_StaticPlug<F32>& C_ScenePointConstraintNode::GetOffsetPolarityPlug()
{
	return m_offsetPolarityPlug;
}

const C_StaticPlug<F32>& C_ScenePointConstraintNode::GetOffsetPolarityPlug() const
{
	return m_offsetPolarityPlug;
}

C_NullPlug& C_ScenePointConstraintNode::GetRestTranslatePlug()
{
	return m_restTranslatePlug;
}

const C_NullPlug& C_ScenePointConstraintNode::GetRestTranslatePlug() const
{
	return m_restTranslatePlug;
}

C_NullPlug& C_ScenePointConstraintNode::GetConstraintTranslatePlug()
{
	return m_constraintTranslatePlug;
}

const C_NullPlug& C_ScenePointConstraintNode::GetConstraintTranslatePlug() const
{
	return m_constraintTranslatePlug;
}

bool C_ScenePointConstraintNode::RestTranslateEnabled() const
{
	return m_enableRestTranslate;
}

void C_ScenePointConstraintNode::EnableRestTranslate(bool enable)
{
	m_enableRestTranslate = enable;
}

C_NullPlug& C_ScenePointConstraintNode::GetTargetTranslatePlug(U32 target)
{
	return m_inputs[target]->targetTranslatePlug;
}

const C_NullPlug& C_ScenePointConstraintNode::GetTargetTranslatePlug(U32 target) const
{
	return m_inputs[target]->targetTranslatePlug;
}

C_StaticPlug<F32>& C_ScenePointConstraintNode::GetTargetWeightPlug(U32 target)
{
	return m_inputs[target]->targetWeightPlug;
}

const C_StaticPlug<F32>& C_ScenePointConstraintNode::GetTargetWeightPlug(U32 target) const
{
	return m_inputs[target]->targetWeightPlug;
}

C_NullPlug& C_ScenePointConstraintNode::GetTargetParentPlug(U32 target)
{
	return m_inputs[target]->targetParentPlug;
}

const C_NullPlug& C_ScenePointConstraintNode::GetTargetParentPlug(U32 target) const
{
	return m_inputs[target]->targetParentPlug;
}

const I_Plug* C_ScenePointConstraintNode::GetStaticPlugPtr(U32 instance) const
{
	U32 staticOffset = C_SceneNode::GetNumStaticPlugs();

	if (instance < staticOffset)
		return C_SceneNode::GetStaticPlugPtr(instance);

	instance -= staticOffset;
	switch (instance)
	{
		case STATIC_PLUG_TRANSLATE_OFFSET:
			return &m_translateOffsetPlug;
		case STATIC_PLUG_POLARITY:
			return &m_offsetPolarityPlug;
		case STATIC_PLUG_REST_TRANSLATE:
			return &m_restTranslatePlug;
		case STATIC_PLUG_CONSTRAINT_TRANSLATE:
			return &m_constraintTranslatePlug;

		default:
			break;
	};

	return 0;
}

U32	C_ScenePointConstraintNode::GetNumStaticPlugs() const
{
	return C_SceneNode::GetNumStaticPlugs() + NUM_STATIC_PLUGS;
}

void C_ScenePointConstraintNode::Clear()
{
	ClearThis(true);
	C_SceneNode::Clear();
}

void C_ScenePointConstraintNode::ClearThis(bool evaluateDependants)
{
	((C_StaticPlug<F32>*)m_translateOffsetPlug.GetChildPtr(0))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_translateOffsetPlug.GetChildPtr(1))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_translateOffsetPlug.GetChildPtr(2))->SetValue(0.f, evaluateDependants);

	m_offsetPolarityPlug.SetValue(0.f);

	((C_StaticPlug<F32>*)m_restTranslatePlug.GetChildPtr(0))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_restTranslatePlug.GetChildPtr(1))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_restTranslatePlug.GetChildPtr(2))->SetValue(0.f, evaluateDependants);

	((C_StaticPlug<F32>*)m_constraintTranslatePlug.GetChildPtr(0))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_constraintTranslatePlug.GetChildPtr(1))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_constraintTranslatePlug.GetChildPtr(2))->SetValue(0.f, evaluateDependants);

	m_enableRestTranslate = false;

	for (U32 i = 0; i < (U32)m_inputs.Size(); i++)
		delete m_inputs[i];
	m_inputs.Clear();
}

bool C_ScenePointConstraintNode::IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return C_SceneNode::IsEqualToAll(rhs, classToAbortTest, flags);
}

void C_ScenePointConstraintNode::OneTimeInit()
{
	U32 staticOffset = C_SceneNode::GetNumStaticPlugs();

	{
		m_translateOffsetPlug.SetLogicalIndex(staticOffset+STATIC_PLUG_TRANSLATE_OFFSET);
		m_translateOffsetPlug.SetParentPtr(this);
		m_translateOffsetPlug.SetName("translateOffset");
		m_translateOffsetPlug.m_children.Reserve(3);
		
		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_translateOffsetPlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_translateOffsetPlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_translateOffsetPlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}

	m_offsetPolarityPlug.SetLogicalIndex(staticOffset+STATIC_PLUG_POLARITY);
	m_offsetPolarityPlug.SetParentPtr(this);
	m_offsetPolarityPlug.SetName("offsetPolarity");

	{
		m_restTranslatePlug.SetLogicalIndex(staticOffset+STATIC_PLUG_REST_TRANSLATE);
		m_restTranslatePlug.SetParentPtr(this);
		m_restTranslatePlug.SetName("restTranslate");
		m_restTranslatePlug.m_children.Reserve(3);
		
		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_restTranslatePlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_restTranslatePlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_restTranslatePlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}

	{
		m_constraintTranslatePlug.SetLogicalIndex(staticOffset+STATIC_PLUG_CONSTRAINT_TRANSLATE);
		m_constraintTranslatePlug.SetParentPtr(this);
		m_constraintTranslatePlug.SetName("constraintTranslate");
		m_constraintTranslatePlug.m_children.Reserve(3);
		
		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, 0);
		m_constraintTranslatePlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, 0);
		m_constraintTranslatePlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, 0);
		m_constraintTranslatePlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}

	ClearThis(false);
}

void C_ScenePointConstraintNode::OnInputPlugChanged(void* _pPlug)
{
	I_Plug* pPlug = (I_Plug*)_pPlug;
	C_ScenePointConstraintNode* pNode;
	if (pPlug->IsPlugChild())
		pNode = (C_ScenePointConstraintNode*)((I_Plug*)pPlug->GetParentPtr())->GetParentPtr();
	else
		pNode = (C_ScenePointConstraintNode*)pPlug->GetTopLevelParentPtr();

	AFTK_ASSERT(pNode && pNode->HasType(E_ObjType_ScenePointConstraint));

	//Bail if this plug isn't connected to an input.
	const Ut::UtVector<Ut::DynamicVector<I_Plug*> >& connectedPlugs = 
		pPlug->GetConnectedPlugs();

	//We should only have at most a single target.
	AFTK_ASSERT(connectedPlugs.Size() <= 1);

	//Calculate new output state from current inputs.
	pNode->Evaluate();
}

void C_ScenePointConstraintNode::Evaluate()
{
	F32 offsetPolarity;
	m_offsetPolarityPlug.GetValue(offsetPolarity);

	Vector3F32 weightedAverage(0,0,0);

	F32 weightSum(0);
	for (U32 i = 0; i < (U32)m_inputs.Size(); i++)
	{
		F32 targetWeight;
		m_inputs[i]->targetWeightPlug.GetValue(targetWeight);

		weightSum += targetWeight;
	}
	if (weightSum == 0)// && m_enableRestTranslate)
	{
		Vector3F32 rest;
		m_restTranslatePlug.GetCompoundValue(rest);

		weightedAverage = rest;
	}
	else
	{
		for (U32 i = 0; i < (U32)m_inputs.Size(); i++)
		{
			Vector3F32 trans;
			m_inputs[i]->targetTranslatePlug.GetCompoundValue(trans);
			F32 targetWeight;
			m_inputs[i]->targetWeightPlug.GetValue(targetWeight);

			Maths::HMat44 parentM;
			I_Object* pConNode = m_inputs[i]->targetParentPlug.GetSourceConnectionPtr();
			if (pConNode && pConNode->HasType(Aftk::E_ObjType_SceneNode) && ((Aftk::C_SceneNode*)pConNode)->GetParentPtr())
				Aftk::TemplateToNightmareMatrix44(parentM, ((Aftk::C_SceneNode*)pConNode)->GetParentPtr()->GetWorldMatrix());
			else
				parentM.SetIdentity();

			Matrix44 parent;
			NightmareToTemplateMatrix44(parent, parentM);
			trans = ApplyMatrix(parent, trans);

			trans = trans * offsetPolarity;

			weightedAverage = weightedAverage + targetWeight*trans;
		}
		weightedAverage = weightedAverage / weightSum;
	}

	if (IsLimited(E_LimitType_TranslateMinX) && weightedAverage[0] < GetLimit(E_LimitType_TranslateMinX))
		weightedAverage[0] = GetLimit(E_LimitType_TranslateMinX);
	if (IsLimited(E_LimitType_TranslateMinY) && weightedAverage[1] < GetLimit(E_LimitType_TranslateMinY))
		weightedAverage[1] = GetLimit(E_LimitType_TranslateMinY);
	if (IsLimited(E_LimitType_TranslateMinZ) && weightedAverage[2] < GetLimit(E_LimitType_TranslateMinZ))
		weightedAverage[2] = GetLimit(E_LimitType_TranslateMinZ);

	if (IsLimited(E_LimitType_TranslateMaxX) && weightedAverage[0] > GetLimit(E_LimitType_TranslateMaxX))
		weightedAverage[0] = GetLimit(E_LimitType_TranslateMaxX);
	if (IsLimited(E_LimitType_TranslateMaxY) && weightedAverage[1] > GetLimit(E_LimitType_TranslateMaxY))
		weightedAverage[1] = GetLimit(E_LimitType_TranslateMaxY);
	if (IsLimited(E_LimitType_TranslateMaxZ) && weightedAverage[2] > GetLimit(E_LimitType_TranslateMaxZ))
		weightedAverage[2] = GetLimit(E_LimitType_TranslateMaxZ);

	//Assumption about constrained object:
	Matrix44 constrainedParentInverseMatrix;
	UtMaths::SetIdentity(&constrainedParentInverseMatrix);
	if (m_constraintTranslatePlug.GetChildPtr(0)->GetConnectedPlugs().Size() != 0)
	{
		I_Plug* pPlug = *m_constraintTranslatePlug.GetChildPtr(0)->GetConnectedPlugs().Begin();
		assert(pPlug->GetTopLevelParentPtr());
		if (pPlug->GetTopLevelParentPtr()->HasType(Aftk::E_ObjType_SceneNode))
			constrainedParentInverseMatrix = 
				((Aftk::C_SceneNode*)pPlug->GetTopLevelParentPtr())->GetParentPtr()->GetWorldMatrixInverse();
	}
	weightedAverage = ApplyMatrix(constrainedParentInverseMatrix, weightedAverage);

	((C_StaticPlug<F32>*)m_constraintTranslatePlug.GetChildPtr(0))->SetValue(weightedAverage[0]);
	((C_StaticPlug<F32>*)m_constraintTranslatePlug.GetChildPtr(1))->SetValue(weightedAverage[1]);
	((C_StaticPlug<F32>*)m_constraintTranslatePlug.GetChildPtr(2))->SetValue(weightedAverage[2]);
}

void C_ScenePointConstraintNode::SetNumInputs(U32 nInputs)
{
	//Clip inputs
	for (U32 i = nInputs; i < (U32)m_inputs.Size(); i++)
	{
		delete m_inputs[i];
	}

	U32 oldSize = (U32)m_inputs.Size();
	m_inputs.Resize(nInputs);
	for (U32 i = oldSize; i < (U32)m_inputs.Size(); i++)
	{
		m_inputs[i] = new C_Target(this);
	}
}

U32 C_ScenePointConstraintNode::GetNumInputs() const
{
	return (U32)m_inputs.Size();
}
