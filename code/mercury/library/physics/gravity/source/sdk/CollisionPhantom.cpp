#include "StdAfx.h"
#include "gravity/sdk/CollisionPhantom.h"
#include "gravity/broadphase/Object.inl"
#include "gravity/sharedphysics/GraphNode.h"

namespace Gravity
{

	C_CollisionPhantom::C_CollisionPhantom(Collision::C_Shape* pShape) : 
	 m_Object(m_GraphNode.m_Index, pShape, &m_GraphNode), m_pShape(pShape), m_pUserData(NULL)
	{
		m_OffsetToCentre = pShape->m_ElementOffset;
	}

	void C_CollisionPhantom::Transform(const Maths::HMat43 PCREF transform)
	{
		this->m_Object.Transform(transform);
	}

	C_Object* C_CollisionPhantom::GetObject()
	{
		return &m_Object;
	}

	SharedPhysics::C_GraphNode* C_CollisionPhantom::GetGraphNode()
	{
		return &m_GraphNode;
	}

	Collision::C_Shape* C_CollisionPhantom::GetShape()
	{
		return m_pShape;
	}

	void C_CollisionPhantom::SetUserData(void* pData)
	{
		m_pUserData = pData;
	}

	void* C_CollisionPhantom::GetUserData()
	{
		return m_pUserData;
	}

	void C_CollisionPhantom::DebugRender(DbUtil::I_DebugRender* pRender)
	{
#ifndef PLATFORM_SPU
		m_pShape->DebugRender(pRender);
#endif
	}

}