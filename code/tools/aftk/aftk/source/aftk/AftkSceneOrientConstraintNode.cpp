#include <stdafx.h>
#include <Aftk/AftkEuler.h>

#define NUM_STATIC_PLUGS 3
#define STATIC_PLUG_ROTATE_OFFSET 0
#define STATIC_PLUG_REST_ROTATE 1
#define STATIC_PLUG_CONSTRAINT_ROTATE 2

using namespace Aftk;

C_SceneOrientConstraintNode::C_Target::C_Target(C_SceneOrientConstraintNode* pParent) : 
	targetRotatePlug(OnInputPlugChanged),
	targetWeightPlug(E_GenericAttributeType_F32, OnInputPlugChanged),
	targetParentPlug(OnInputPlugChanged)
{
	targetParentPlug.SetLogicalIndex(0);
	targetParentPlug.SetParentPtr(pParent);
	targetParentPlug.SetName("targetParent");
	targetParentPlug.SetChildrenLocked(true);

	targetRotatePlug.SetLogicalIndex(1);
	targetRotatePlug.SetParentPtr(pParent);

	{
		targetRotatePlug.SetName("targetRotate");
		targetRotatePlug.m_children.Reserve(3);
		
		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		targetRotatePlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		pPlugX->SetValue(0.f, false);
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		targetRotatePlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		pPlugY->SetValue(0.f, false);
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		targetRotatePlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
		pPlugZ->SetValue(0.f, false);
	}
	targetRotatePlug.SetChildrenLocked(true);

	targetWeightPlug.SetLogicalIndex(2);
	targetWeightPlug.SetParentPtr(pParent);
	targetWeightPlug.SetName("targetWeight");
	targetWeightPlug.SetChildrenLocked(true);

	targetWeightPlug.SetValue(1.f, false);
}

C_SceneOrientConstraintNode::C_SceneOrientConstraintNode(C_Scene* pScene) : I_SceneConstraintNode(pScene), C_LimitMixin(), 
m_rotateOffsetPlug(OnInputPlugChanged),
m_restRotatePlug(OnInputPlugChanged),
m_constraintRotatePlug(0)
{
	OneTimeInit();
}

C_Node* C_SceneOrientConstraintNode::Clone(C_Scene* pScene) const
{
	C_SceneOrientConstraintNode* p = new C_SceneOrientConstraintNode(pScene);
	if (!p)
		return 0;

	*p = *this;
	return p;
}

C_SceneOrientConstraintNode::~C_SceneOrientConstraintNode()
{
	ClearThis(true);
}

C_SceneOrientConstraintNode::C_SceneOrientConstraintNode(const C_SceneOrientConstraintNode& rhs) 
 : I_SceneConstraintNode(0),
m_rotateOffsetPlug(OnInputPlugChanged),
m_restRotatePlug(OnInputPlugChanged),
m_constraintRotatePlug(0)
{
	OneTimeInit();
	*this = rhs;
}

const C_SceneOrientConstraintNode& C_SceneOrientConstraintNode::operator=(const C_SceneOrientConstraintNode& rhs)
{
	C_SceneNode::operator=(rhs);
	C_LimitMixin::operator=(rhs);

	SetNumInputs(rhs.GetNumInputs());
	for (U32 i = 0; i < (U32)rhs.m_inputs.Size(); i++)
	{
		m_inputs[i]->targetParentPlug = rhs.m_inputs[i]->targetParentPlug;
		m_inputs[i]->targetRotatePlug = rhs.m_inputs[i]->targetRotatePlug;
		m_inputs[i]->targetWeightPlug = rhs.m_inputs[i]->targetWeightPlug;
	}

	m_constraintRotatePlug = rhs.m_constraintRotatePlug;
	m_rotateOffsetPlug = rhs.m_rotateOffsetPlug;
	m_restRotatePlug = rhs.m_restRotatePlug;
	m_constraintRotatePlug = rhs.m_constraintRotatePlug;


	return *this;
}

U32 C_SceneOrientConstraintNode::GetHash(E_ObjType classToAbortTest) const
{
	return I_SceneConstraintNode::GetHash(classToAbortTest);
}

E_ObjType C_SceneOrientConstraintNode::GetType() const
{
	return Aftk::E_ObjType_SceneOrientConstraint;
}

bool C_SceneOrientConstraintNode::HasType(E_ObjType type) const
{
	if (I_SceneConstraintNode::HasType(type))
		return true;
	if (type == E_ObjType_SceneOrientConstraint) 
		return true;

	return false;
}

bool C_SceneOrientConstraintNode::IsUsed() const 
{
	return C_SceneNode::IsUsed();
}

bool C_SceneOrientConstraintNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

bool C_SceneOrientConstraintNode::IsEqualToThis(const I_Object &rhs, U32 flags) const
{
	return true;
}

C_NullPlug& C_SceneOrientConstraintNode::GetRotateOffsetPlug()
{
	return m_rotateOffsetPlug;
}

const C_NullPlug& C_SceneOrientConstraintNode::GetRotateOffsetPlug() const
{
	return m_rotateOffsetPlug;
}

C_NullPlug& C_SceneOrientConstraintNode::GetRestRotatePlug()
{
	return m_restRotatePlug;
}

const C_NullPlug& C_SceneOrientConstraintNode::GetRestRotatePlug() const
{
	return m_restRotatePlug;
}

C_NullPlug& C_SceneOrientConstraintNode::GetConstraintRotatePlug()
{
	return m_constraintRotatePlug;
}

const C_NullPlug& C_SceneOrientConstraintNode::GetConstraintRotatePlug() const
{
	return m_constraintRotatePlug;
}

bool C_SceneOrientConstraintNode::RestRotateEnabled() const
{
	return m_enableRestRotate;
}

void C_SceneOrientConstraintNode::EnableRestRotate(bool enable)
{
	m_enableRestRotate = enable;
}

C_NullPlug& C_SceneOrientConstraintNode::GetTargetRotatePlug(U32 target)
{
	return m_inputs[target]->targetRotatePlug;
}

const C_NullPlug& C_SceneOrientConstraintNode::GetTargetRotatePlug(U32 target) const
{
	return m_inputs[target]->targetRotatePlug;
}

C_StaticPlug<F32>& C_SceneOrientConstraintNode::GetTargetWeightPlug(U32 target)
{
	return m_inputs[target]->targetWeightPlug;
}

const C_StaticPlug<F32>& C_SceneOrientConstraintNode::GetTargetWeightPlug(U32 target) const
{
	return m_inputs[target]->targetWeightPlug;
}

C_NullPlug& C_SceneOrientConstraintNode::GetTargetParentPlug(U32 target)
{
	return m_inputs[target]->targetParentPlug;
}

const C_NullPlug& C_SceneOrientConstraintNode::GetTargetParentPlug(U32 target) const
{
	return m_inputs[target]->targetParentPlug;
}

const I_Plug* C_SceneOrientConstraintNode::GetStaticPlugPtr(U32 instance) const
{
	U32 staticOffset = C_SceneNode::GetNumStaticPlugs();

	if (instance < staticOffset)
		return C_SceneNode::GetStaticPlugPtr(instance);

	instance -= staticOffset;
	switch (instance)
	{
		case STATIC_PLUG_ROTATE_OFFSET:
			return &m_rotateOffsetPlug;
		case STATIC_PLUG_REST_ROTATE:
			return &m_restRotatePlug;
		case STATIC_PLUG_CONSTRAINT_ROTATE:
			return &m_constraintRotatePlug;

		default:
			break;
	};

	return 0;
}

U32	C_SceneOrientConstraintNode::GetNumStaticPlugs() const
{
	return C_SceneNode::GetNumStaticPlugs() + NUM_STATIC_PLUGS;
}

void C_SceneOrientConstraintNode::Clear()
{
	ClearThis(true);
	C_SceneNode::Clear();
}

void C_SceneOrientConstraintNode::ClearThis(bool evaluateDependants)
{
	for (U32 i = 0; i < (U32)m_inputs.Size(); i++)
		delete m_inputs[i];
	m_inputs.Clear();

	((C_StaticPlug<F32>*)m_rotateOffsetPlug.GetChildPtr(0))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_rotateOffsetPlug.GetChildPtr(1))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_rotateOffsetPlug.GetChildPtr(2))->SetValue(0.f, evaluateDependants);

	((C_StaticPlug<F32>*)m_restRotatePlug.GetChildPtr(0))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_restRotatePlug.GetChildPtr(1))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_restRotatePlug.GetChildPtr(2))->SetValue(0.f, evaluateDependants);

	((C_StaticPlug<F32>*)m_constraintRotatePlug.GetChildPtr(0))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_constraintRotatePlug.GetChildPtr(1))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_constraintRotatePlug.GetChildPtr(2))->SetValue(0.f, evaluateDependants);

	m_enableRestRotate = false;
}

bool C_SceneOrientConstraintNode::IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return C_SceneNode::IsEqualToAll(rhs, classToAbortTest, flags);
}

void C_SceneOrientConstraintNode::OneTimeInit()
{
	U32 staticOffset = C_SceneNode::GetNumStaticPlugs();

	{
		m_rotateOffsetPlug.SetLogicalIndex(staticOffset+STATIC_PLUG_ROTATE_OFFSET);
		m_rotateOffsetPlug.SetParentPtr(this);
		m_rotateOffsetPlug.SetName("rotateOffset");
		m_rotateOffsetPlug.m_children.Reserve(3);
		
		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_rotateOffsetPlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_rotateOffsetPlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_rotateOffsetPlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}

	{
		m_restRotatePlug.SetLogicalIndex(staticOffset+STATIC_PLUG_REST_ROTATE);
		m_restRotatePlug.SetParentPtr(this);
		m_restRotatePlug.SetName("restRotate");
		m_restRotatePlug.m_children.Reserve(3);
		
		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_restRotatePlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_restRotatePlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_restRotatePlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}

	{
		m_constraintRotatePlug.SetLogicalIndex(staticOffset+STATIC_PLUG_CONSTRAINT_ROTATE);
		m_constraintRotatePlug.SetParentPtr(this);
		m_constraintRotatePlug.SetName("constraintRotate");
		m_constraintRotatePlug.m_children.Reserve(3);
		
		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, 0);
		m_constraintRotatePlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, 0);
		m_constraintRotatePlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, 0);
		m_constraintRotatePlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}

	ClearThis(false);
}

void C_SceneOrientConstraintNode::OnInputPlugChanged(void* _pPlug)
{
	I_Plug* pPlug = (I_Plug*)_pPlug;
	C_SceneOrientConstraintNode* pNode;
	if (pPlug->IsPlugChild())
		pNode = (C_SceneOrientConstraintNode*)((I_Plug*)pPlug->GetParentPtr())->GetParentPtr();
	else
		pNode = (C_SceneOrientConstraintNode*)pPlug->GetTopLevelParentPtr();

	AFTK_ASSERT(pNode && pNode->HasType(E_ObjType_SceneOrientConstraint));

	//Bail if this plug isn't connected to an input.
	const Ut::UtVector<Ut::DynamicVector<I_Plug*> >& connectedPlugs = 
		pPlug->GetConnectedPlugs();

	//We should only have at most a single target.
	AFTK_ASSERT(connectedPlugs.Size() <= 1);

	//Calculate new output state from current inputs.
	pNode->Evaluate();
}

void C_SceneOrientConstraintNode::Evaluate()
{
	if (m_inputs.Size() == 0 || (U32)m_inputs.Size() > 2)
		return;

	Maths::HMat33 worldMatrix;

	Maths::HMat33 parentM1;
	I_Object* pConNode1 = m_inputs[0]->targetParentPlug.GetSourceConnectionPtr();
	if (pConNode1 && pConNode1->HasType(Aftk::E_ObjType_SceneNode) && ((Aftk::C_SceneNode*)pConNode1)->GetParentPtr())
		Aftk::TemplateToNightmareMatrix33(parentM1, ((Aftk::C_SceneNode*)pConNode1)->GetParentPtr()->GetWorldMatrix());
	else
		parentM1.SetIdentity();

	Maths::HMat33 localM1;
	Vector3F32 euler1;
	m_inputs[0]->targetRotatePlug.GetCompoundValue(euler1);
	EulerToMatrix(localM1, Aftk::TemplateToNightmare(euler1), m_rotationOrder);

	F32 weightSum(0);
	if (m_inputs.Size() == 1)
	{
		worldMatrix = localM1 * parentM1;
	}
	else
	{
		Maths::HMat33 parentM2;
		I_Object* pConNode2 = m_inputs[1]->targetParentPlug.GetSourceConnectionPtr();
		if (pConNode2 && pConNode2->HasType(Aftk::E_ObjType_SceneNode) && ((Aftk::C_SceneNode*)pConNode2)->GetParentPtr())
			Aftk::TemplateToNightmareMatrix33(parentM2, ((Aftk::C_SceneNode*)pConNode2)->GetParentPtr()->GetWorldMatrix());
		else
			parentM2.SetIdentity();

		Maths::HMat33 localM2;
		Vector3F32 euler2;
		m_inputs[1]->targetRotatePlug.GetCompoundValue(euler2);
		EulerToMatrix(localM2, Aftk::TemplateToNightmare(euler2), m_rotationOrder);

		Maths::C_Quaternion slerped;
		Maths::C_Quaternion q1(localM1 * parentM1);
		Maths::C_Quaternion q2(localM2 * parentM2);

		Vector2F32 weights;
		m_inputs[0]->targetWeightPlug.GetValue(weights[0]);
		m_inputs[1]->targetWeightPlug.GetValue(weights[1]);

		slerped = q1.ShortestSlerp(q2, (weights[0]-weights[1])/(weights[0]+weights[1]));
		slerped.Normalise();

		worldMatrix = (Maths::HMat33)slerped;
	}

	//Assumption about constrained object:
	Maths::HMat33 constrainedParentInverseMatrix;
	constrainedParentInverseMatrix.SetIdentity();
	if (m_constraintRotatePlug.GetChildPtr(0)->GetConnectedPlugs().Size() != 0)
	{
		I_Plug* pPlug = *m_constraintRotatePlug.GetChildPtr(0)->GetConnectedPlugs().Begin();
		assert(pPlug->GetTopLevelParentPtr());
		if (pPlug->GetTopLevelParentPtr()->HasType(Aftk::E_ObjType_SceneNode))
			TemplateToNightmareMatrix33(
				constrainedParentInverseMatrix,
				((Aftk::C_SceneNode*)pPlug->GetTopLevelParentPtr())->GetParentPtr()->GetWorldMatrixInverse());
	}

	Maths::HMat33 hResultM = worldMatrix * constrainedParentInverseMatrix;

	Maths::HVec3 hResult;
	MatrixToEuler(hResult, hResultM, m_rotationOrder);

	Vector3F32 result;
	result = NightmareToTemplate(hResult);

	if (IsLimited(E_LimitType_RotateMinX) && result[0] < GetLimit(E_LimitType_RotateMinX))
		result[0] = GetLimit(E_LimitType_RotateMinX);
	if (IsLimited(E_LimitType_RotateMinY) && result[1] < GetLimit(E_LimitType_RotateMinY))
		result[1] = GetLimit(E_LimitType_RotateMinY);
	if (IsLimited(E_LimitType_RotateMinZ) && result[2] < GetLimit(E_LimitType_RotateMinZ))
		result[2] = GetLimit(E_LimitType_RotateMinZ);

	if (IsLimited(E_LimitType_RotateMaxX) && result[0] > GetLimit(E_LimitType_RotateMaxX))
		result[0] = GetLimit(E_LimitType_RotateMaxX);
	if (IsLimited(E_LimitType_RotateMaxY) && result[1] > GetLimit(E_LimitType_RotateMaxY))
		result[1] = GetLimit(E_LimitType_RotateMaxY);
	if (IsLimited(E_LimitType_RotateMaxZ) && result[2] > GetLimit(E_LimitType_RotateMaxZ))
		result[2] = GetLimit(E_LimitType_RotateMaxZ);

	((C_StaticPlug<F32>*)m_constraintRotatePlug.GetChildPtr(0))->SetValue(result[0]);
	((C_StaticPlug<F32>*)m_constraintRotatePlug.GetChildPtr(1))->SetValue(result[1]);
	((C_StaticPlug<F32>*)m_constraintRotatePlug.GetChildPtr(2))->SetValue(result[2]);
}

void C_SceneOrientConstraintNode::SetNumInputs(U32 nInputs)
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

U32 C_SceneOrientConstraintNode::GetNumInputs() const
{
	return (U32)m_inputs.Size();
}