#include <stdafx.h>
#include <Aftk/AftkEuler.h>

#define AFTK_SCENE_AIM_CONSTRAINT_NODE_NUM_STATIC_PLUGS 12
#define AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_TARGET_TRANSLATE 0
#define AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_AIM_VECTOR 1
#define AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_UP_VECTOR 2
#define AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_ROTATIONAL_OFFSET 3
#define AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_WORLD_UP_VECTOR_TYPE 4
#define AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_WORLD_UP_VECTOR 5
#define AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_CONSTRAINT_TRANSLATE 6
#define AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_CONSTRAINT_ROTATE_ORDER 7
#define AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_CONSTRAINT_ROTATE  8
#define AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_WORLD_UP_MATRIX 9
#define AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_CONSTRAINT_PARENT 10
#define AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_TARGET_PARENT 11

using namespace Aftk;


C_SceneAimConstraintNode::C_SceneAimConstraintNode(C_Scene* pScene) : I_SceneConstraintNode(pScene), C_LimitMixin(), 
m_targetTranslatePlug(OnInputPlugChanged), //Input
m_constraintRotateOrderPlug(E_GenericAttributeType_U32, OnInputPlugChanged), //Input
m_aimVectorPlug(OnInputPlugChanged), //Input
m_upVectorPlug(OnInputPlugChanged), //Input
m_rotationalOffsetPlug(OnInputPlugChanged), //Input
m_worldUpVectorTypePlug(E_GenericAttributeType_U32, OnInputPlugChanged), //Input
m_worldUpVectorPlug(OnInputPlugChanged), //Input
m_constraintTranslatePlug(OnInputPlugChanged), //Input
m_constraintRotatePlug(0),
m_worldUpMatrixPlug(E_GenericAttributeType_Matrix44F32, OnInputPlugChanged),
m_constraintParentPlug(OnInputPlugChanged),
m_targetParentPlug(OnInputPlugChanged)
{
	OneTimeInit();
}

C_Node* C_SceneAimConstraintNode::Clone(C_Scene* pScene) const
{
	C_SceneAimConstraintNode* p = new C_SceneAimConstraintNode(pScene);
	if (!p)
		return 0;

	*p = *this;
	return p;
}

C_SceneAimConstraintNode::~C_SceneAimConstraintNode()
{
}

C_SceneAimConstraintNode::C_SceneAimConstraintNode(const C_SceneAimConstraintNode& rhs) 
 : I_SceneConstraintNode(0),
m_targetTranslatePlug(OnInputPlugChanged), //Input
m_constraintRotateOrderPlug(E_GenericAttributeType_U32, OnInputPlugChanged), //Input
m_aimVectorPlug(OnInputPlugChanged), //Input
m_upVectorPlug(OnInputPlugChanged), //Input
m_rotationalOffsetPlug(OnInputPlugChanged), //Input
m_worldUpVectorTypePlug(E_GenericAttributeType_U32, OnInputPlugChanged), //Input
m_worldUpVectorPlug(OnInputPlugChanged), //Input
m_constraintTranslatePlug(OnInputPlugChanged), //Input
m_constraintRotatePlug(0),
m_worldUpMatrixPlug(E_GenericAttributeType_Matrix44F32, OnInputPlugChanged),
m_constraintParentPlug(OnInputPlugChanged),
m_targetParentPlug(OnInputPlugChanged)
{
	OneTimeInit();
	*this = rhs;
}

const C_SceneAimConstraintNode& C_SceneAimConstraintNode::operator=(const C_SceneAimConstraintNode& rhs)
{
	C_SceneNode::operator=(rhs);
	C_LimitMixin::operator=(rhs);

	m_constraintTranslatePlug = rhs.m_constraintTranslatePlug;
	m_targetTranslatePlug = rhs.m_targetTranslatePlug;
	m_aimVectorPlug = rhs.m_aimVectorPlug;
	m_upVectorPlug = rhs.m_upVectorPlug;
	m_rotationalOffsetPlug = rhs.m_rotationalOffsetPlug;

	m_worldUpVectorTypePlug = rhs.m_worldUpVectorTypePlug;
	m_worldUpVectorPlug = rhs.m_worldUpVectorPlug;

	m_constraintRotateOrderPlug = rhs.m_constraintRotateOrderPlug;

	m_constraintRotatePlug = rhs.m_constraintRotatePlug;

	m_worldUpVectorPlug = rhs.m_worldUpVectorPlug;

	m_worldUpMatrixPlug = rhs.m_worldUpMatrixPlug;

	m_constraintParentPlug = rhs.m_constraintParentPlug;
	m_targetParentPlug = rhs.m_targetParentPlug;

	return *this;
}

U32 C_SceneAimConstraintNode::GetHash(E_ObjType classToAbortTest) const
{
	return I_SceneConstraintNode::GetHash(classToAbortTest);
}

E_ObjType C_SceneAimConstraintNode::GetType() const
{
	return Aftk::E_ObjType_SceneAimConstraint;
}

bool C_SceneAimConstraintNode::HasType(E_ObjType type) const
{
	if (I_SceneConstraintNode::HasType(type))
		return true;
	if (type == E_ObjType_SceneAimConstraint) 
		return true;

	return false;
}

bool C_SceneAimConstraintNode::IsUsed() const 
{
	return C_SceneNode::IsUsed();
}

bool C_SceneAimConstraintNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbortTest, flags);
}

bool C_SceneAimConstraintNode::IsEqualToThis(const I_Object &rhs, U32 flags) const
{
	return true;
}

C_NullPlug& C_SceneAimConstraintNode::GetConstraintTranslatePlug() 
{
	return m_constraintTranslatePlug;
}

const C_NullPlug& C_SceneAimConstraintNode::GetConstraintTranslatePlug() const
{
	return m_constraintTranslatePlug;
}

C_NullPlug& C_SceneAimConstraintNode::GetTargetTranslatePlug() 
{
	return m_targetTranslatePlug;
}

const C_NullPlug& C_SceneAimConstraintNode::GetTargetTranslatePlug() const
{
	return m_targetTranslatePlug;
}

const C_NullPlug& C_SceneAimConstraintNode::GetConstraintParentPlug() const
{
	return m_constraintParentPlug;
}

C_NullPlug& C_SceneAimConstraintNode::GetConstraintParentPlug()
{
	return m_constraintParentPlug;
}

const C_NullPlug& C_SceneAimConstraintNode::GetTargetParentPlug() const
{
	return m_targetParentPlug;
}

C_NullPlug& C_SceneAimConstraintNode::GetTargetParentPlug()
{
	return m_targetParentPlug;
}


C_NullPlug& C_SceneAimConstraintNode::GetAimVectorPlug() 
{
	return m_aimVectorPlug;
}

const C_NullPlug& C_SceneAimConstraintNode::GetAimVectorPlug() const
{
	return m_aimVectorPlug;
}

C_NullPlug& C_SceneAimConstraintNode::GetUpVectorPlug() 
{
	return m_upVectorPlug;
}

const C_NullPlug& C_SceneAimConstraintNode::GetUpVectorPlug() const
{
	return m_upVectorPlug;
}

C_NullPlug& C_SceneAimConstraintNode::GetRotationalOffsetPlug() 
{
	return m_rotationalOffsetPlug;
}

const C_NullPlug& C_SceneAimConstraintNode::GetRotationalOffsetPlug() const
{
	return m_rotationalOffsetPlug;
}

C_StaticPlug<U32>& C_SceneAimConstraintNode::GetWorldUpVectorTypePlug() 
{
	return m_worldUpVectorTypePlug;
}

const C_StaticPlug<U32>& C_SceneAimConstraintNode::GetWorldUpVectorTypePlug() const
{
	return m_worldUpVectorTypePlug;
}

C_NullPlug& C_SceneAimConstraintNode::GetWorldUpVectorPlug() 
{
	return m_worldUpVectorPlug;
}

const C_NullPlug& C_SceneAimConstraintNode::GetWorldUpVectorPlug() const
{
	return m_worldUpVectorPlug;
}

C_StaticPlug<U32>& C_SceneAimConstraintNode::GetConstraintRotateOrderPlug() 
{
	return m_constraintRotateOrderPlug;
}

const C_StaticPlug<U32>& C_SceneAimConstraintNode::GetConstraintRotateOrderPlug() const
{
	return m_constraintRotateOrderPlug;
}

C_NullPlug& C_SceneAimConstraintNode::GetConstraintRotatePlug() 
{
	return m_constraintRotatePlug;
}

const C_NullPlug& C_SceneAimConstraintNode::GetConstraintRotatePlug() const
{
	return m_constraintRotatePlug;
}

const I_Plug* C_SceneAimConstraintNode::GetStaticPlugPtr(U32 instance) const
{
	U32 staticOffset = C_SceneNode::GetNumStaticPlugs();

	if (instance < staticOffset)
		return C_SceneNode::GetStaticPlugPtr(instance);

	instance -= staticOffset;
	switch (instance)
	{
		case AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_TARGET_TRANSLATE:
			return &m_targetTranslatePlug;
		case AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_AIM_VECTOR:
			return &m_aimVectorPlug;
		case AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_UP_VECTOR:
			return &m_upVectorPlug;
		case AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_ROTATIONAL_OFFSET:
			return &m_rotationalOffsetPlug;
		case AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_WORLD_UP_VECTOR_TYPE:
			return &m_worldUpVectorTypePlug;
		case AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_WORLD_UP_VECTOR:
			return &m_worldUpVectorPlug;
		case AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_CONSTRAINT_ROTATE_ORDER:
			return &m_constraintRotateOrderPlug;
		case AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_CONSTRAINT_TRANSLATE:
			return &m_constraintTranslatePlug;
		case AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_CONSTRAINT_ROTATE:
			return &m_constraintRotatePlug;
		case AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_WORLD_UP_MATRIX:
			return &m_worldUpMatrixPlug;
		case AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_CONSTRAINT_PARENT:
			return &m_constraintParentPlug;
		case AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_TARGET_PARENT:
			return &m_targetParentPlug;

		default:
			break;
	};

	return 0;
}

U32	C_SceneAimConstraintNode::GetNumStaticPlugs() const
{
	return C_SceneNode::GetNumStaticPlugs() + AFTK_SCENE_AIM_CONSTRAINT_NODE_NUM_STATIC_PLUGS;
}

void C_SceneAimConstraintNode::Clear()
{
	ClearThis(true);
	C_SceneNode::Clear();
}

void C_SceneAimConstraintNode::ClearThis(bool evaluateDependants)
{
	((C_StaticPlug<F32>*)m_targetTranslatePlug.GetChildPtr(0))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_targetTranslatePlug.GetChildPtr(1))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_targetTranslatePlug.GetChildPtr(2))->SetValue(0.f, evaluateDependants);

	((C_StaticPlug<F32>*)m_aimVectorPlug.GetChildPtr(0))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_aimVectorPlug.GetChildPtr(1))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_aimVectorPlug.GetChildPtr(2))->SetValue(0.f, evaluateDependants);

	((C_StaticPlug<F32>*)m_upVectorPlug.GetChildPtr(0))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_upVectorPlug.GetChildPtr(1))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_upVectorPlug.GetChildPtr(2))->SetValue(0.f, evaluateDependants);

	((C_StaticPlug<F32>*)m_rotationalOffsetPlug.GetChildPtr(0))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_rotationalOffsetPlug.GetChildPtr(1))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_rotationalOffsetPlug.GetChildPtr(2))->SetValue(0.f, evaluateDependants);

	m_worldUpVectorTypePlug.SetValue((U32)E_UpVectorType_None);

	((C_StaticPlug<F32>*)m_worldUpVectorPlug.GetChildPtr(0))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_worldUpVectorPlug.GetChildPtr(1))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_worldUpVectorPlug.GetChildPtr(2))->SetValue(0.f, evaluateDependants);

	((C_StaticPlug<F32>*)m_constraintTranslatePlug.GetChildPtr(0))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_constraintTranslatePlug.GetChildPtr(1))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_constraintTranslatePlug.GetChildPtr(2))->SetValue(0.f, evaluateDependants);

	m_constraintRotateOrderPlug.SetValue((U32)m_rotationOrder, evaluateDependants);

	((C_StaticPlug<F32>*)m_constraintRotatePlug.GetChildPtr(0))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_constraintRotatePlug.GetChildPtr(1))->SetValue(0.f, evaluateDependants);
	((C_StaticPlug<F32>*)m_constraintRotatePlug.GetChildPtr(2))->SetValue(0.f, evaluateDependants);

	Matrix44 id;
	UtMaths::SetIdentity(&id);
	m_worldUpMatrixPlug.SetValue(id);
}

bool C_SceneAimConstraintNode::IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest, U32 flags) const
{
	return C_SceneNode::IsEqualToAll(rhs, classToAbortTest, flags);
}

void C_SceneAimConstraintNode::OneTimeInit()
{
	U32 staticOffset = C_SceneNode::GetNumStaticPlugs();
	//Initialise transform plugs.
	{
		//Translate
		m_targetTranslatePlug.SetLogicalIndex(staticOffset+AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_TARGET_TRANSLATE);
		m_targetTranslatePlug.SetParentPtr(this);
		m_targetTranslatePlug.SetName("targetTranslate");
		m_targetTranslatePlug.m_children.Reserve(3);
		
		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_targetTranslatePlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_targetTranslatePlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_targetTranslatePlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}

	{
		//Aim vector
		m_aimVectorPlug.SetLogicalIndex(staticOffset+AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_AIM_VECTOR);
		m_aimVectorPlug.SetParentPtr(this);
		m_aimVectorPlug.SetName("aimVector");
		m_aimVectorPlug.m_children.Reserve(3);

		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_aimVectorPlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_aimVectorPlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_aimVectorPlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}
	{
		//Up vector
		m_upVectorPlug.SetLogicalIndex(staticOffset+AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_UP_VECTOR);
		m_upVectorPlug.SetParentPtr(this);
		m_upVectorPlug.SetName("upVector");
		m_upVectorPlug.m_children.Reserve(3);

		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_upVectorPlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_upVectorPlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_upVectorPlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}

	{
		//Rotational offset
		m_rotationalOffsetPlug.SetLogicalIndex(staticOffset+AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_ROTATIONAL_OFFSET);
		m_rotationalOffsetPlug.SetParentPtr(this);
		m_rotationalOffsetPlug.SetName("rotationalOffset");
		m_rotationalOffsetPlug.m_children.Reserve(3);

		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_rotationalOffsetPlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_rotationalOffsetPlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_rotationalOffsetPlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}

	//World up vector type
	m_worldUpVectorTypePlug.SetLogicalIndex(staticOffset+AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_WORLD_UP_VECTOR_TYPE);
	m_worldUpVectorTypePlug.SetParentPtr(this);
	m_worldUpVectorTypePlug.SetName("worldUpVectorType");
	m_worldUpVectorTypePlug.m_children.Reserve(3);

	{
		//World up vector
		m_worldUpVectorPlug.SetLogicalIndex(staticOffset+AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_WORLD_UP_VECTOR);
		m_worldUpVectorPlug.SetParentPtr(this);
		m_worldUpVectorPlug.SetName("worldUpVector");

		C_StaticPlug<F32>* pPlugX = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_worldUpVectorPlug.AddChildPtr(pPlugX);
		pPlugX->SetName("x");
		
		C_StaticPlug<F32>* pPlugY = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_worldUpVectorPlug.AddChildPtr(pPlugY);
		pPlugY->SetName("y");
		
		C_StaticPlug<F32>* pPlugZ = 
			new C_StaticPlug<F32>(E_GenericAttributeType_F32, OnInputPlugChanged);
		m_worldUpVectorPlug.AddChildPtr(pPlugZ);
		pPlugZ->SetName("z");
	}
	
	{
		//Output translate constraint
		m_constraintTranslatePlug.SetLogicalIndex(staticOffset+AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_CONSTRAINT_TRANSLATE);
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

	m_constraintRotateOrderPlug.SetLogicalIndex(staticOffset+AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_CONSTRAINT_ROTATE_ORDER);
	m_constraintRotateOrderPlug.SetParentPtr(this);
	m_constraintRotateOrderPlug.SetName("rotateOrderConstraint");
	{
		//Output rotate constraint
		m_constraintRotatePlug.SetLogicalIndex(staticOffset+AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_CONSTRAINT_ROTATE);
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

	m_worldUpMatrixPlug.SetLogicalIndex(staticOffset+AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_WORLD_UP_MATRIX);
	m_worldUpMatrixPlug.SetParentPtr(this);
	m_worldUpMatrixPlug.SetName("worldUpMatrix");

	m_constraintParentPlug.SetLogicalIndex(staticOffset+AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_CONSTRAINT_PARENT);
	m_constraintParentPlug.SetParentPtr(this);
	m_constraintParentPlug.SetName("constraintParent");
	m_constraintParentPlug.SetChildrenLocked(true);

	m_targetParentPlug.SetLogicalIndex(staticOffset+AFTK_SCENE_AIM_CONSTRAINT_NODE_STATIC_PLUG_TARGET_PARENT);
	m_targetParentPlug.SetParentPtr(this);
	m_targetParentPlug.SetName("targettParent");
	m_targetParentPlug.SetChildrenLocked(true);
	
	ClearThis(false);
}

C_StaticPlug<Matrix44>& C_SceneAimConstraintNode::GetWorldUpMatrixPlug()
{
	return m_worldUpMatrixPlug;
}

const C_StaticPlug<Matrix44>& C_SceneAimConstraintNode::GetWorldUpMatrixPlug() const
{
	return m_worldUpMatrixPlug;
}

void C_SceneAimConstraintNode::OnInputPlugChanged(void* _pPlug)
{
	I_Plug* pPlug = (I_Plug*)_pPlug;
	C_SceneAimConstraintNode* pNode;
	if (pPlug->IsPlugChild())
		pNode = (C_SceneAimConstraintNode*)((I_Plug*)pPlug->GetParentPtr())->GetParentPtr();
	else
		pNode = (C_SceneAimConstraintNode*)pPlug->GetTopLevelParentPtr();

	AFTK_ASSERT(pNode && pNode->HasType(E_ObjType_SceneAimConstraint));

	//Bail if this plug isn't connected to an input.
	const Ut::UtVector<Ut::DynamicVector<I_Plug*> >& connectedPlugs = 
		pPlug->GetConnectedPlugs();

	//We should only have at most a single target.
	AFTK_ASSERT(connectedPlugs.Size() <= 1);

	//Calculate new output state from current inputs.
	pNode->Evaluate();
}

Matrix44 C_SceneAimConstraintNode::GetAimMatrix() const
{
	//INPUT
	Vector3F32 upVector;
	m_upVectorPlug.GetCompoundValue(upVector);

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
	Vector3F32 constainedObjectPos;
	m_constraintTranslatePlug.GetCompoundValue(constainedObjectPos);
	{
		Matrix44 parent;
		const I_Object* pConNode = m_constraintParentPlug.GetSourceConnectionPtr();
		if (pConNode && pConNode->HasType(Aftk::E_ObjType_SceneNode) && ((Aftk::C_SceneNode*)pConNode)->GetParentPtr())
		{
			parent = ((Aftk::C_SceneNode*)pConNode)->GetParentPtr()->GetWorldMatrix();
		}
		else
		{
			UtMaths::SetIdentity(&parent);
		}

		constainedObjectPos = ApplyMatrix(parent, constainedObjectPos);
	}

	//r is our look-at matrix.
	Matrix44 r;
	{
		//OUTPUT
		//Generate our coordinate system.
		Vector3F32 trueAimVector = constainedObjectPos - targetPos;
		Vector3F32 rightVector = trueAimVector ^ upVector;
		Vector3F32 orthoUpVector = trueAimVector ^ rightVector;

		//Normalise our axes
		UtMaths::SafeNormalise(&trueAimVector);
		UtMaths::SafeNormalise(&rightVector);
		UtMaths::SafeNormalise(&orthoUpVector);

		//Build a rotation matrix.
		r[0][0] = rightVector[0];	r[0][1] = rightVector[1];	r[0][2] = rightVector[2];	r[0][3] = 0.f;
		r[1][0] = orthoUpVector[0];	r[1][1] = orthoUpVector[1];	r[1][2] = orthoUpVector[2];	r[1][3] = 0.f;
		r[2][0] = trueAimVector[0];	r[2][1] = trueAimVector[1];	r[2][2] = trueAimVector[2];	r[2][3] = 0.f;
		r[3] = Vector4F32(0,0,0,1.f);
	}

	//AIM OFFSET
	Matrix44 rOffset;
	{
		//Generate our coordinate system for our aim offset orientation.
		Vector3F32 aimVector;
		m_aimVectorPlug.GetCompoundValue(aimVector);

		Vector3F32 rightVector = aimVector ^ upVector;
		Vector3F32 orthoUpVector = aimVector ^ rightVector;

		//Normalise our axes
		UtMaths::SafeNormalise(&aimVector);
		UtMaths::SafeNormalise(&rightVector);
		UtMaths::SafeNormalise(&orthoUpVector);

		//Build a rotation matrix.
		rOffset[0][0] = rightVector[0];		rOffset[0][1] = rightVector[1];		rOffset[0][2] = rightVector[2];		rOffset[0][3] = 0.f;
		rOffset[1][0] = orthoUpVector[0];	rOffset[1][1] = orthoUpVector[1];	rOffset[1][2] = orthoUpVector[2];	rOffset[1][3] = 0.f;
		rOffset[2][0] = aimVector[0];		rOffset[2][1] = aimVector[1];		rOffset[2][2] = aimVector[2];		rOffset[2][3] = 0.f;
		rOffset[3] = Vector4F32(0,0,0,1.f);
	}

	Matrix44 aimMatrix =  rOffset * r;

	//Stage 2: Apply rotational offset.
	{
		U32 rotateOrder;
		m_constraintRotateOrderPlug.GetValue(rotateOrder);

		Vector3F32 rotationalOffsetEuler;
		m_rotationalOffsetPlug.GetCompoundValue(rotationalOffsetEuler);

		Matrix44 rotationalOffsetM;
		EulerToMatrix(rotationalOffsetM, rotationalOffsetEuler, (E_RotationOrder)rotateOrder);
		UtMaths::Transpose(&rotationalOffsetM);

		aimMatrix = rotationalOffsetM * aimMatrix;
	}

	return aimMatrix;
}

void C_SceneAimConstraintNode::Evaluate()
{
	//INPUT
	Matrix44 constrainedParentInverseMatrix;
	{
		Matrix44 parent;
		const I_Object* pConNode = m_constraintParentPlug.GetSourceConnectionPtr();
		if (pConNode && pConNode->HasType(Aftk::E_ObjType_SceneNode) && ((Aftk::C_SceneNode*)pConNode)->GetParentPtr())
		{
			constrainedParentInverseMatrix = 
				((Aftk::C_SceneNode*)pConNode)->GetParentPtr()->GetWorldMatrixInverse();
		}
		else
		{
			UtMaths::SetIdentity(&constrainedParentInverseMatrix);
		}
	}

	U32 rotateOrder;
	m_constraintRotateOrderPlug.GetValue(rotateOrder);
	Vector3F32 result;
	MatrixToEuler(result, GetAimMatrix() * constrainedParentInverseMatrix, (E_RotationOrder)rotateOrder);

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
