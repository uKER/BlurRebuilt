#include "stdafx.h"
#include "Dynamic/RenderNode.h"
#include "Dynamic/Block.h"
#include "Dynamic/Node.h"

#include <system\Debug\I_DebugRender.h>

namespace DynamicEffect
{

Debug::Bool	g_BoundsDynamicEffect("Graphics.SceneGraph.Bounds.Nodes.DynamicEffect", true);

extern Debug::Logger &g_Log;

void C_CompositeRenderNode::DebugRender(const SceneGraph::C_VisibleNode& node) const
{
	if(g_BoundsDynamicEffect)
	{
		/*Maths::RGBA colours[7];
		colours[0] = Maths::RGBA(178, 34, 34);
		colours[1] = Maths::RGBA(30, 144, 255);
		colours[2] = Maths::RGBA(218, 65, 32);
		colours[3] = Maths::RGBA(240, 255, 255);
		colours[4] = Maths::RGBA(0, 0, 255);
		colours[5] = Maths::RGBA(173, 255, 47);
		colours[6] = Maths::RGBA(255, 255, 255);*/

		Maths::HVec3 vCompositeMin(10000.0f);
		Maths::HVec3 vCompositeMax(-10000.0f);

		for(S32 i = 0; i < blocks.Size(); ++i)
		{	
			const C_RenderNode* pBlock = blocks[i];
			
			Maths::RGBA vColour(255, 255, 255, 255);
			Maths::HMat43 mTransform;
			if(pBlock->m_pNode)
			{
				Maths::C_QuatTrans transform = *pBlock->m_pNode;
				mTransform = transform.Matrix();
				vColour = Maths::RGBA(255, 128, 128, 255);
				Maths::HVec3 vOrigin(0.0f, 0.0f, 0.0f);
				Maths::HVec3 vBlockCentre(Maths::HVec3((F32*)&pBlock->m_vCentre));

				vOrigin = vOrigin * mTransform;
				vBlockCentre = vBlockCentre * mTransform;

				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(vOrigin, vBlockCentre, Maths::RGBA(160, 0, 160));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(mTransform);
			}

			Maths::HVec3 vCentre(Maths::Load<Maths::HVec3>(&pBlock->m_vCentre));
			Maths::HVec3 vSize(Maths::Load<Maths::HVec3>(&pBlock->m_vSize));

			Maths::HVec3 vMin = vCentre - vSize;
			Maths::HVec3 vMax = vCentre + vSize;
			
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(vMin, vMax, mTransform, vColour);

			vMin = vMin * mTransform;
			vMax = vMax * mTransform;

			vCompositeMin = Maths::Min(vCompositeMin, vMin);
			vCompositeMax = Maths::Max(vCompositeMax, vMax);
		}

		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(vCompositeMin, vCompositeMax, Maths::HMat43(), Maths::RGBA(255, 64, 64, 255));
	}
}

} // namespace DynamicEffect
