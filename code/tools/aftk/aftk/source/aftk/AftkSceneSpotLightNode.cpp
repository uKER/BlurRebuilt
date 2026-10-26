#include <stdafx.h>

using namespace Aftk;

C_SceneSpotLightNode::C_SceneSpotLightNode(C_Scene* pScene) 
: 
C_SceneNonAmbientLightNode(pScene)
{
	OneTimeInit();
}

C_Node* C_SceneSpotLightNode::Clone(C_Scene* pScene) const
{
	C_SceneSpotLightNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneSpotLightNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneSpotLightNode(pScene);
		(C_SceneSpotLightNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_SceneSpotLightNode::C_SceneSpotLightNode(const C_SceneSpotLightNode& rhs)
: 
C_SceneNonAmbientLightNode(0)
{
	OneTimeInit();

	*this = rhs;
}

const C_SceneSpotLightNode& C_SceneSpotLightNode::operator=(const C_SceneSpotLightNode& rhs)
{
	C_SceneNonAmbientLightNode::operator=(rhs);

	ClearThis();

	m_coneAngle = rhs.m_coneAngle;
	m_penumbraAngle = rhs.m_penumbraAngle;
	m_dropOff = rhs.m_dropOff;
	m_cutOffRadius = rhs.m_cutOffRadius;

	return *this;
}

C_SceneSpotLightNode::~C_SceneSpotLightNode()
{
	ClearThis();
}

E_ObjType C_SceneSpotLightNode::GetType() const
{
	return E_ObjType_SceneSpotLight;
}

bool C_SceneSpotLightNode::HasType(E_ObjType type) const
{
	if (C_SceneNonAmbientLightNode::HasType(type))
		return true;
	if (type == C_SceneSpotLightNode::GetType()) 
		return true;

	return false;
}

void C_SceneSpotLightNode::SetConeAngle(F32 coneAngle)
{
	m_coneAngle = coneAngle;
}

void C_SceneSpotLightNode::SetPenumbraAngle(F32 penumbraAngle)
{
	m_penumbraAngle = penumbraAngle;
}

void C_SceneSpotLightNode::SetDropOff(F32 dropOff)
{
	m_dropOff = dropOff;
}

void C_SceneSpotLightNode::SetCutOffRadius(F32 cutOffRadius)
{
	m_cutOffRadius = cutOffRadius;
}

F32 C_SceneSpotLightNode::GetConeAngle() const
{
	return m_coneAngle;
}

F32 C_SceneSpotLightNode::GetPenumbraAngle() const
{
	return m_penumbraAngle;
}

F32 C_SceneSpotLightNode::GetDropOff() const
{
	return m_dropOff;
}

F32 C_SceneSpotLightNode::GetCutOffRadius() const
{
	return m_cutOffRadius;
}

bool C_SceneSpotLightNode::operator== (const C_SceneSpotLightNode &rhs) const
{
	return C_SceneSpotLightNode::IsEqualTo(rhs);
}

bool C_SceneSpotLightNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_SceneSpotLightNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (classToAbort == E_ObjType_SceneSpotLight)
		return C_SceneSpotLightNode::IsEqualToThis(rhs, flags);
	return (C_SceneSpotLightNode::IsEqualToThis(rhs, flags) && C_SceneNonAmbientLightNode::IsEqualToAll(rhs, classToAbort, flags));
}

bool C_SceneSpotLightNode::IsEqualToThis(const I_Object& _rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_SceneSpotLight))
	{
		return false;
	}

	C_SceneSpotLightNode& rhs = (C_SceneSpotLightNode&) _rhs;

	if ((!EqualityTestFScalar(m_coneAngle, rhs.m_coneAngle)) ||
		(!EqualityTestFScalar(m_penumbraAngle, rhs.m_penumbraAngle)) ||
		(!EqualityTestFScalar(m_dropOff, rhs.m_dropOff)) ||
		(!EqualityTestFScalar(m_cutOffRadius, rhs.m_cutOffRadius)))
		return false;

	return true;
}

void C_SceneSpotLightNode::Clear()
{
	ClearThis();
	C_SceneNonAmbientLightNode::Clear();
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneSpotLightNode::OneTimeInit()
{
	m_coneAngle = 0;
	m_penumbraAngle = 0;
	m_dropOff = 0;
	m_cutOffRadius = 0;
}

void C_SceneSpotLightNode::ClearThis()
{
	m_coneAngle = 0;
	m_penumbraAngle = 0;
	m_dropOff = 0;
	m_cutOffRadius = 0;
}

/*
C_SceneSpotLightNode::C_SceneSpotLightNode()
{
	m_coneAngle = 0;
	m_penumbraAngle = 0;
	m_dropOff = 0;
	m_cutOffRadius = 0;
}

C_SceneSpotLightNode::C_SceneSpotLightNode(
	const C_SceneSpotLightNode& rhs)
{
	Reset();
	*this = rhs;
}

const C_SceneSpotLightNode& C_SceneSpotLightNode::operator =(
	const C_SceneSpotLightNode& rhs)
{
	Clear();
	C_SceneNonAmbientLightNode::operator=(rhs);

	m_coneAngle = rhs.m_coneAngle;
	m_penumbraAngle = rhs.m_penumbraAngle;
	m_dropOff = rhs.m_dropOff;

	return *this;
}
*/

#endif
