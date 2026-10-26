#include "stdafx.h"
#include "renderingdata/scene.h"
#include "btl/containers/algorithm.h"

//#pragma optimize("", off)

namespace RenderingData
{

bool GetEndianSwap(PlatformType platformType)
{
	switch (platformType)
	{
		case RenderingData::PLATFORM_TYPE_360: return Ut::NativeToBigEndian::swap;
		case RenderingData::PLATFORM_TYPE_PS3: return Ut::NativeToBigEndian::swap;
		case RenderingData::PLATFORM_TYPE_PC: return Ut::NativeToLittleEndian::swap;
		default: assert(0); return false;
	}
}

Scene::Scene()// : m_rootNode(this)
{
	m_rootNode.SetScenePtr(this);
}

Scene::~Scene()
{
	assert(m_activeNodes.Size() == 1);
}

const Btl::String& Scene::GetName() const
{
	return m_name;
}

void Scene::RegisterNode(Node* node)
{
	m_activeNodes.PushBack();
	m_activeNodes.Back().pNode = node;
	m_activeNodes.Back().cachedType = node->GetNodeType();
}

void Scene::DeregisterNode(Node* node)
{
	S32 n = m_activeNodes.Size();
	Btl::RemoveNonOrdered(m_activeNodes, node);
	assert(m_activeNodes.Size() == n - 1);
}

bool Scene::IsActive(const Node* node) const
{
	return Btl::Find(m_activeNodes, node) != m_activeNodes.End();
}

};
