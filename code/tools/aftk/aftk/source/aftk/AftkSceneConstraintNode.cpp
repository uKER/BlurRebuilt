#include <stdafx.h>

using namespace Aftk;


C_SceneConstraintNode::C_SceneConstraintNode(C_Scene* pScene) : I_SceneConstraintNode(pScene)
{
}

C_Node* C_SceneConstraintNode::Clone(C_Scene* pScene) const
{
	C_SceneConstraintNode* p = new C_SceneConstraintNode(pScene);
	if (!p)
		return 0;

	*p = *this;
	return p;
}

C_SceneConstraintNode::~C_SceneConstraintNode()
{
}

C_SceneConstraintNode::C_SceneConstraintNode(const C_SceneConstraintNode& rhs) 
 : I_SceneConstraintNode(0)
{
	*this = rhs;
}

const C_SceneConstraintNode& C_SceneConstraintNode::operator=(const C_SceneConstraintNode& rhs)
{
	C_SceneNode::operator=(rhs);
	return *this;
}

E_ObjType C_SceneConstraintNode::GetType() const
{
	return Aftk::E_ObjType_SceneConstraint;
}

bool C_SceneConstraintNode::HasType(E_ObjType type) const
{
	if (I_SceneConstraintNode::HasType(type))
		return true;
	if (type == E_ObjType_SceneConstraint) 
		return true;

	return false;
}
