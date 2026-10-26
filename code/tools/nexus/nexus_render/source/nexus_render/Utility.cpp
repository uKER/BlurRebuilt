#include <nexus_render/stdafx.h>
#include <nexus_render/Utility.h>

using namespace NexusRender;

RenderNode* Utility::GetIndirectSourcePtr(RenderNode* pRenderNode)
{
	if (pRenderNode->GetType() == RenderNode::RENDER_NODE_TYPE_INSTANCE)
	{
		RenderInstanceNode* pInst = (RenderInstanceNode*)pRenderNode;
		RenderNode* pSource = pInst->GetSourcePtr();
		if (pSource->GetType() == RenderNode::RENDER_NODE_TYPE_INSTANCE)
			return GetIndirectSourcePtr(pSource);

        return pSource;
	}

	return pRenderNode;
}
