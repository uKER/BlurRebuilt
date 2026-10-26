#include <stdafx.h>

using namespace Aftk;

C_SceneGeometryNode::C_SceneGeometryNode(C_Scene* pScene) : C_SceneNode(pScene)
{
	OneTimeInit();
}

C_SceneGeometryNode::~C_SceneGeometryNode()
{
	ClearThis();
}

C_SceneGeometryNode::C_SceneGeometryNode(const C_SceneGeometryNode& rhs) 
: C_SceneNode(0)
{
	OneTimeInit();
	*this = rhs;
}

const C_SceneGeometryNode& C_SceneGeometryNode::operator =(const C_SceneGeometryNode& rhs)
{
	// base-class asignment operator
	C_SceneNode::operator=(rhs);

	ClearThis();

	return *this;
}

bool C_SceneGeometryNode::operator== (const C_SceneGeometryNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_SceneGeometryNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

void C_SceneGeometryNode::Clear()
{
	ClearThis();
	C_SceneNode::Clear();
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneGeometryNode::OneTimeInit()
{
}

void C_SceneGeometryNode::ClearThis()
{
}

bool C_SceneGeometryNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (classToAbort == E_ObjType_SceneGeometry)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_SceneNode::IsEqualToAll(rhs, classToAbort, flags));
}

bool C_SceneGeometryNode::IsEqualToThis(const I_Object &rhs, U32 flags) const
{
	if (!rhs.HasType(E_ObjType_SceneGeometry))
		return false;
	return true;
}


U32 C_SceneGeometryNode::GetHashOfThis() const
{
	return 1;
}

U32 C_SceneGeometryNode::GetHash(E_ObjType classToAbort) const
{
	if (classToAbort)
		return 0;

	return C_SceneGeometryNode::GetHashOfThis()*C_SceneNode::GetHash();
}

#endif
