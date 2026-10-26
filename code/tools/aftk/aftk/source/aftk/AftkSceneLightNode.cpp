#include <stdafx.h>

using namespace Aftk;

C_SceneLightNode::C_SceneLightNode(C_Scene* pScene) : C_SceneNode(pScene)
{
	OneTimeInit();
}

C_SceneLightNode::C_SceneLightNode(const C_SceneLightNode& rhs) : C_SceneNode(0)
{
	OneTimeInit();

	*this = rhs;
}

const C_SceneLightNode& C_SceneLightNode::operator=(const C_SceneLightNode& rhs)
{
	C_SceneNode::operator=(rhs);

	ClearThis();

	m_intensity = rhs.m_intensity;
	m_colour = rhs.m_colour;

	return *this;
}

C_SceneLightNode::~C_SceneLightNode()
{
	ClearThis();
}

void C_SceneLightNode::SetIntensity(F32 intensity)
{
	m_intensity = intensity;
}

void C_SceneLightNode::SetColour(const Vector4F32& colour)
{
	m_colour = colour;
}

F32 C_SceneLightNode::GetIntensity() const
{
	return m_intensity;
}

const Vector4F32& C_SceneLightNode::GetColour() const
{
	return m_colour;
}

Vector4F32& C_SceneLightNode::GetColour()
{
	return m_colour;
}

bool C_SceneLightNode::operator== (const C_SceneLightNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_SceneLightNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_SceneLightNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (classToAbort == E_ObjType_SceneLight)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_SceneNode::IsEqualToAll(rhs, classToAbort, flags));
}

bool C_SceneLightNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_SceneLight))
	{
		return false;
	}

	C_SceneLightNode& rhs = (C_SceneLightNode&) _rhs;

	if (!(EqualityTestFScalar(m_intensity, rhs.m_intensity)) ||
		!(EqualityTestFVector(m_colour, rhs.m_colour)))
		return false;

	return true;
}

void C_SceneLightNode::Clear()
{
	ClearThis();
	C_SceneNode::Clear();
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneLightNode::OneTimeInit()
{
	m_intensity = 0.0f;
	m_colour = Vector4F32(0,0,0,0);
}

void C_SceneLightNode::ClearThis()
{
	m_intensity = 0.0f;
	m_colour = Vector4F32(0,0,0,0);
}

#endif
