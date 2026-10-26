/******************************************************************************/
// Name: AFTKSceneNonAmbientLightNode.h
//
// Author: Ken MacLeod
// 
// Description: see header file
//
// History: created 13rd October 2004
//
// Copyright: 2004 Bizarre Creations
//
// Notes: N/A
//
/******************************************************************************/

#include <stdafx.h>

using namespace Aftk;

C_SceneNonAmbientLightNode::C_SceneNonAmbientLightNode(C_Scene* pScene) 
: 
C_SceneLightNode(pScene)
{
	OneTimeInit();
}

C_SceneNonAmbientLightNode::C_SceneNonAmbientLightNode(
	const C_SceneNonAmbientLightNode& rhs)
: 
C_SceneLightNode(0)
{
	OneTimeInit();
	*this = rhs;
}

const C_SceneNonAmbientLightNode& C_SceneNonAmbientLightNode::operator=(const C_SceneNonAmbientLightNode& rhs)
{
	C_SceneLightNode::operator=(rhs);

	ClearThis();

	m_decayType = rhs.m_decayType;

	return *this;
}

C_SceneNonAmbientLightNode::~C_SceneNonAmbientLightNode()
{
	ClearThis();
}

void C_SceneNonAmbientLightNode::SetDecayType(E_LightDecay decayType)
{
	m_decayType = decayType;
}

E_LightDecay C_SceneNonAmbientLightNode::GetDecayType() const
{
	return m_decayType;
}

bool C_SceneNonAmbientLightNode::operator== (const C_SceneNonAmbientLightNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_SceneNonAmbientLightNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_SceneNonAmbientLightNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (classToAbort == E_ObjType_SceneNonAmbientLight)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_SceneLightNode::IsEqualToAll(rhs, classToAbort, flags));
}

bool C_SceneNonAmbientLightNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_SceneNonAmbientLight))
	{
		return false;
	}

	C_SceneNonAmbientLightNode& rhs = (C_SceneNonAmbientLightNode&) _rhs;

	if (m_decayType != rhs.m_decayType)
		return false;

	return true;
}

void C_SceneNonAmbientLightNode::Clear()
{
	ClearThis();
	C_SceneLightNode::Clear();
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneNonAmbientLightNode::OneTimeInit()
{
	m_decayType = E_LightDecay_None;
}

void C_SceneNonAmbientLightNode::ClearThis()
{
	m_decayType = E_LightDecay_None;
}

#endif
