#include <stdafx.h>
#include <Aftk/AftkEuler.h>

using namespace Aftk;

#define AFTK_SCENE_LOOK_AT_CONSTRAINT_NODE_NUM_STATIC_PLUGS 2
#define AFTK_SCENE_LOOK_AT_CONSTRAINT_NODE_STATIC_PLUG_ROLL 0
#define AFTK_SCENE_LOOK_AT_CONSTRAINT_NODE_STATIC_PLUG_DISTANCE_BETWEEN 1

C_SceneLookAtConstraintNode::C_SceneLookAtConstraintNode(C_Scene* pScene) : C_SceneAimConstraintNode(pScene),
m_rollPlug(E_GenericAttributeType_F32, OnInputPlugChanged), //Input
m_distanceBetweenPlug(E_GenericAttributeType_F32, 0) //Output
{
	OneTimeInit();
}

C_Node* C_SceneLookAtConstraintNode::Clone(C_Scene* pScene) const
{
	C_SceneLookAtConstraintNode* p = new C_SceneLookAtConstraintNode(pScene);
	if (!p)
		return 0;

	*p = *this;
	return p;
}

C_SceneLookAtConstraintNode::~C_SceneLookAtConstraintNode()
{
}

C_SceneLookAtConstraintNode::C_SceneLookAtConstraintNode(const C_SceneLookAtConstraintNode& rhs) 
: C_SceneAimConstraintNode(0),
m_rollPlug(E_GenericAttributeType_F32, OnInputPlugChanged), //Input
m_distanceBetweenPlug(E_GenericAttributeType_F32, 0) //Output
{
	OneTimeInit();
	*this = rhs;
}

const C_SceneLookAtConstraintNode& C_SceneLookAtConstraintNode::operator=(const C_SceneLookAtConstraintNode& rhs)
{
	C_SceneAimConstraintNode::operator=(rhs);

	m_rollPlug = rhs.m_rollPlug;
	m_distanceBetweenPlug = rhs.m_distanceBetweenPlug;

	return *this;
}

U32 C_SceneLookAtConstraintNode::GetHash(E_ObjType classToAbortTest) const
{
	return I_SceneConstraintNode::GetHash(classToAbortTest);
}

E_ObjType C_SceneLookAtConstraintNode::GetType() const
{
	return Aftk::E_ObjType_SceneLookAtConstraint;
}

bool C_SceneLookAtConstraintNode::HasType(E_ObjType type) const
{
	if (C_SceneAimConstraintNode::HasType(type))
		return true;
	if (type == E_ObjType_SceneLookAtConstraint) 
		return true;

	return false;
}

bool C_SceneLookAtConstraintNode::IsUsed() const 
{
	return C_SceneAimConstraintNode::IsUsed();
}

bool C_SceneLookAtConstraintNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

bool C_SceneLookAtConstraintNode::IsEqualToThis(const I_Object &rhs, U32 flags) const
{
	return true;
}

C_StaticPlug<F32>& C_SceneLookAtConstraintNode::GetRollPlug()
{
	return m_rollPlug;
}

const C_StaticPlug<F32>& C_SceneLookAtConstraintNode::GetRollPlug() const
{
	return m_rollPlug;
}

C_StaticPlug<F32>& C_SceneLookAtConstraintNode::GetDistanceBetweenPlug()
{
	return m_distanceBetweenPlug;
}

const C_StaticPlug<F32>& C_SceneLookAtConstraintNode::GetDistanceBetweenPlug() const
{
	return m_distanceBetweenPlug;
}

void C_SceneLookAtConstraintNode::Clear()
{
	ClearThis(true);
	C_SceneAimConstraintNode::Clear();
}

void C_SceneLookAtConstraintNode::ClearThis(bool evaluateDependants)
{
	m_rollPlug.SetValue(0.f, evaluateDependants);
	m_distanceBetweenPlug.SetValue(0.f, evaluateDependants);
}

bool C_SceneLookAtConstraintNode::IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return C_SceneAimConstraintNode::IsEqualToAll(rhs, classToAbortTest, flags);
}

void C_SceneLookAtConstraintNode::OneTimeInit()
{
	U32 staticOffset = C_SceneAimConstraintNode::GetNumStaticPlugs();

	m_rollPlug.SetLogicalIndex(staticOffset + AFTK_SCENE_LOOK_AT_CONSTRAINT_NODE_STATIC_PLUG_ROLL);
	m_rollPlug.SetParentPtr(this);
	m_rollPlug.SetName("roll");

	//Output
	m_distanceBetweenPlug.SetLogicalIndex(staticOffset + AFTK_SCENE_LOOK_AT_CONSTRAINT_NODE_STATIC_PLUG_DISTANCE_BETWEEN);
	m_distanceBetweenPlug.SetParentPtr(this);
	m_distanceBetweenPlug.SetName("distanceBetween");

	ClearThis(false);
}

const I_Plug* C_SceneLookAtConstraintNode::GetStaticPlugPtr(U32 instance) const
{
	U32 staticOffset = C_SceneAimConstraintNode::GetNumStaticPlugs();

	if (instance < staticOffset)
		return C_SceneAimConstraintNode::GetStaticPlugPtr(instance);

	instance -= staticOffset;
	switch (instance)
	{
		case AFTK_SCENE_LOOK_AT_CONSTRAINT_NODE_STATIC_PLUG_ROLL:
			return &m_rollPlug;
		case AFTK_SCENE_LOOK_AT_CONSTRAINT_NODE_STATIC_PLUG_DISTANCE_BETWEEN:
			return &m_distanceBetweenPlug;
		default:
			break;
	};

	return 0;
}

U32	C_SceneLookAtConstraintNode::GetNumStaticPlugs() const
{
	return C_SceneAimConstraintNode::GetNumStaticPlugs() + 
		AFTK_SCENE_LOOK_AT_CONSTRAINT_NODE_NUM_STATIC_PLUGS;
}

void C_SceneLookAtConstraintNode::OnInputPlugChanged(void* _pPlug)
{
	I_Plug* pPlug = (I_Plug*)_pPlug;
	C_SceneLookAtConstraintNode* pNode;
	if (pPlug->IsPlugChild())
		pNode = (C_SceneLookAtConstraintNode*)((I_Plug*)pPlug->GetParentPtr())->GetParentPtr();
	else
		pNode = (C_SceneLookAtConstraintNode*)pPlug->GetTopLevelParentPtr();

	AFTK_ASSERT(pNode && pNode->HasType(E_ObjType_SceneLookAtConstraint));

	//Bail if this plug isn't connected to an input.
	const Ut::UtVector<Ut::DynamicVector<I_Plug*> >& connectedPlugs = 
		pPlug->GetConnectedPlugs();

	//We should only have at most a single target.
	AFTK_ASSERT(connectedPlugs.Size() <= 1);

	//Calculate new output state from current inputs.
	pNode->Evaluate();
}

void C_SceneLookAtConstraintNode::Evaluate()
{
	//Calculate the value of constrainedRotatePlug's children [output] based upon the following
	//inputs:
	//m_constraintTranslatePlug
	//m_targetTranslatePlug
	//m_rollPlug
	//m_aimVectorPlug
	//m_upVectorPlug

	//outputs:
	//m_rotateConstraintPlug
	//m_distanceBetweenPlug

	U32 rotateOrder;
	m_constraintRotateOrderPlug.GetValue(rotateOrder);

	//INPUT
	Vector3F32 targetPos;
	m_targetTranslatePlug.GetCompoundValue(targetPos);
	{
		Matrix44 parent;
		const I_Object* pConNode = m_targetParentPlug.GetSourceConnectionPtr();
		if (pConNode && pConNode->HasType(Aftk::E_ObjType_SceneNode) && ((Aftk::C_SceneNode*)pConNode)->GetParentPtr())
		{
			parent = ((Aftk::C_SceneNode*)pConNode)->GetParentPtr()->GetWorldMatrix();
		}
		else
		{
			UtMaths::SetIdentity(&parent);
		}
		targetPos = ApplyMatrix(parent, targetPos);
	}

	//INPUT
	Matrix44 constrainedParentInverseMatrix;
	Vector3F32 constainedObjectPos;
	m_constraintTranslatePlug.GetCompoundValue(constainedObjectPos);
	{
		Matrix44 parent;
		const I_Object* pConNode = m_constraintParentPlug.GetSourceConnectionPtr();
		if (pConNode && pConNode->HasType(Aftk::E_ObjType_SceneNode) && ((Aftk::C_SceneNode*)pConNode)->GetParentPtr())
		{
			parent = ((Aftk::C_SceneNode*)pConNode)->GetParentPtr()->GetWorldMatrix();
			constrainedParentInverseMatrix = 
				((Aftk::C_SceneNode*)pConNode)->GetParentPtr()->GetWorldMatrixInverse();
		}
		else
		{
			UtMaths::SetIdentity(&parent);
			UtMaths::SetIdentity(&constrainedParentInverseMatrix);
		}

		constainedObjectPos = ApplyMatrix(parent, constainedObjectPos);
	}

	//Angle of rotation about our aim vector in radians.
	F32 rollAngle;
	m_rollPlug.GetValue(rollAngle);

	//Create matrix for roll about aim vector (z-axis).
	Matrix44 roll;
	EulerToMatrix(roll, Vector3F32(0, 0, rollAngle), (E_RotationOrder)rotateOrder);

	//Generate rotation matrix
	Matrix44 r = roll * GetAimMatrix() * constrainedParentInverseMatrix;

	//Extract euler angles from our lookat matrix.
	Vector3F32 euler;
	MatrixToEuler(euler, r, (E_RotationOrder)rotateOrder);

	//Calculate distance betweeen target and constrained node in the constrained node's 
	//local space. Store in m_distanceBetweenPlug
	Vector3F32 relativeVector = constainedObjectPos - targetPos;
	m_distanceBetweenPlug.SetValue(UtMaths::Magnitude(relativeVector));

	if (IsLimited(E_LimitType_RotateMinX) && euler[0] < GetLimit(E_LimitType_RotateMinX))
		euler[0] = GetLimit(E_LimitType_RotateMinX);
	if (IsLimited(E_LimitType_RotateMinY) && euler[1] < GetLimit(E_LimitType_RotateMinY))
		euler[1] = GetLimit(E_LimitType_RotateMinY);
	if (IsLimited(E_LimitType_RotateMinZ) && euler[2] < GetLimit(E_LimitType_RotateMinZ))
		euler[2] = GetLimit(E_LimitType_RotateMinZ);

	if (IsLimited(E_LimitType_RotateMaxX) && euler[0] > GetLimit(E_LimitType_RotateMaxX))
		euler[0] = GetLimit(E_LimitType_RotateMaxX);
	if (IsLimited(E_LimitType_RotateMaxY) && euler[1] > GetLimit(E_LimitType_RotateMaxY))
		euler[1] = GetLimit(E_LimitType_RotateMaxY);
	if (IsLimited(E_LimitType_RotateMaxZ) && euler[2] > GetLimit(E_LimitType_RotateMaxZ))
		euler[2] = GetLimit(E_LimitType_RotateMaxZ);

	//Update rotation constraints.
	((C_StaticPlug<F32>*)m_constraintRotatePlug.GetChildPtr(0))->SetValue(euler[0]);
	((C_StaticPlug<F32>*)m_constraintRotatePlug.GetChildPtr(1))->SetValue(euler[1]);
	((C_StaticPlug<F32>*)m_constraintRotatePlug.GetChildPtr(2))->SetValue(euler[2]);
}

