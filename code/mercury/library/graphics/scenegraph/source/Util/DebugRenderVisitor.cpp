#include "stdafx.h"
#include "SceneGraph/Util/DebugRenderVisitor.h"

#include "SceneGraph/GatherNode.h"

#include "SceneGraph/RenderingData/StaticScene.h"
#include "SceneGraph/RenderingData/StaticRenderListNode.h"
#include "SceneGraph/RenderingData/CachedRenderListNode.h"

#include "SceneGraph/Light/Manager.h"

#include "System/Debug/I_DebugRender.h"

namespace SceneGraph
{

extern Debug::Int g_RenderListShowIndex;
extern Debug::Bool g_StaticUseCache;

C_DebugRenderVisitor::C_DebugRenderVisitor() : I_Visitor(1), m_RenderListIndex(-1)
{
}

C_DebugRenderVisitor::~C_DebugRenderVisitor()
{
}

void C_DebugRenderVisitor::Visit(const I_GatherNode& node)
{
}

void C_DebugRenderVisitor::Visit(const C_LevelInstance& node)
{
	m_RenderListIndex = -1;
}

void C_DebugRenderVisitor::Visit(const C_CachedStaticScene& node)
{
	m_RenderListIndex = -1;
}

Debug::Bool	g_DebugRenderRenderListNode	("Graphics.SceneGraph.DebugRender.RenderListNode", true);

void C_DebugRenderVisitor::Visit(const C_CachedRenderListNode& node)
{
	if(g_DebugRenderRenderListNode && g_StaticUseCache)
	{
		m_RenderListIndex++;
		if(g_RenderListShowIndex != -1 && g_RenderListShowIndex != m_RenderListIndex)
			return;

		Maths::HVec3 vCentre(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&node.centre));
		Maths::HVec3 vSize(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&node.size));
		Maths::HVec3 vMinimum = vCentre - vSize;
		Maths::HVec3 vMaximum = vCentre + vSize;
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(vMinimum, vMaximum, Maths::HMat43(), Maths::RGBA(255, 0, 0, 128));

		Util::C_String nodes;
		nodes += "Primitives: ";

		S32 primitives = 0;
		for(S32 i = 0; i < node.GetNumPrimitiveCachedData(); ++i)
		{
			Util::C_String primitives;
			primitives.Printf("%d ", node.GetPrimitiveCachedData(i).indices.Size());

			nodes += primitives;
		}

		nodes += "Instances: ";

		S32 instances = 0;
		for(S32 i = 0; i < node.GetNumInstanceCachedData(); ++i)
		{
			Util::C_String instances;
			instances.Printf("%d ", node.GetInstanceCachedData(i).indices.Size());
			nodes += instances;
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(vCentre, nodes);
	}
}

void C_DebugRenderVisitor::Visit(const C_StaticRenderListNode& node)
{
	if(!g_StaticUseCache)
	{
		m_RenderListIndex++;
		if(g_RenderListShowIndex != -1 && g_RenderListShowIndex != m_RenderListIndex)
			return;

		Maths::HVec3 vMinimum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&node.GetRenderListNode()->GetBounds().aabbMin));
		Maths::HVec3 vMaximum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&node.GetRenderListNode()->GetBounds().aabbMax));
		Maths::HVec3 vCentre = (vMinimum + vMaximum) * Maths::Constants<Maths::HF32>::Half();
		Maths::HVec3 vSize = vMaximum - vCentre;
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(vMinimum, vMaximum, Maths::HMat43(), Maths::RGBA(255, 0, 0, 128));

		Util::C_String nodes;
		nodes += "Primitives: ";
		
		for(S32 i = 0; i < RenderingData::RENDER_PASS_MAX; ++i)
		{
			const RenderingData::RenderListNode::RenderPassInfo& renderPassInfo = node.GetRenderListNode()->GetRenderPassInfo((RenderingData::RenderPass)i);
			
			Util::C_String primitives;
			primitives.Printf("%d ", renderPassInfo.nPrimitives);
			nodes += primitives;
		}

		nodes += "Instances: ";

		for(S32 i = 0; i < RenderingData::RENDER_PASS_MAX; ++i)
		{
			const RenderingData::RenderListNode::RenderPassInfo& renderPassInfo = node.GetRenderListNode()->GetRenderPassInfo((RenderingData::RenderPass)i);
			
			Util::C_String instances;
			instances.Printf("%d ", renderPassInfo.nInstances);
			nodes += instances;
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(vCentre, nodes);
	}
}

Debug::Bool	g_DebugRenderLightManager	("Graphics.SceneGraph.DebugRender.LightManager", true);

void C_DebugRenderVisitor::Visit(const C_LightManager& node)
{
	if(g_DebugRenderLightManager)
	{
		for(PointLights::ConstIterator it = node.m_ActivePointLights.Begin(); it != node.m_ActivePointLights.End(); ++it)
		{
			const Maths::HVec4 vSphere(Maths::Load<Maths::HVec4>(&it->GetBoundingSphere()));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(Maths::HVec3(vSphere), vSphere.W(), Maths::RGBA(0xFF0000FF));
		}
		for(SpotLights::ConstIterator it = node.m_ActiveSpotLights.Begin(); it != node.m_ActiveSpotLights.End(); ++it)
		{
			const Maths::HVec4 vSphere(Maths::Load<Maths::HVec4>(&it->GetBoundingSphere()));
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(Maths::HVec3(vSphere), vSphere.W(), Maths::RGBA(0xFF00FF00));
		}
	}
}

} // namespace SceneGraph

