#ifndef RENDER_DYNAMICEFFECT_STRIP_EFFECT_H
#define RENDER_DYNAMICEFFECT_STRIP_EFFECT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Strip/Manager.h"
#include "Dynamic/Strip/EffectInterface.h"

#include "Dynamic/Common.h"
#include "Dynamic/Renderer.h"
#include "Dynamic/Interface.h"
#include "Dynamic/Block.h"
#include "Render/View/ViewManager.h"

#include "Dynamic/Strip/Strip.h"

namespace SceneGraph
{
	class C_View;
}

namespace DynamicEffect
{
namespace Strip
{

extern Debug::Bool g_debugStripEnable;

class C_Manager;

// TODO add policies for vertex building
// - screen aligned strip, arbitrary number of elements between start and end point (lasers, electricity, beams, simple rope etc)
// - world aligned strip with quaternion (animated world aligned stuff, eg wake on water)
// - world aligned strip with 2x positions (static stuff - tyre tracks, scratch marks, other decals)
// - world aligned X strip (or more than 2 intersecting strips) with quaternion (smoke trails, other volumetric strip effects)

// strip effect contains a number of strips which contain blocks of Elements of a certain type
// it is assumed strips can only have elements removed from front and back and not middle
template<class T_Element, class T_Strip, int T_NumStrips>
class I_StripEffect : public I_StripEffectBase
{
public:
	I_StripEffect(C_Renderer* pRenderer, C_Manager* pManager, const U32 iIndex, const U32 iTypeIndex, const U32 iElementSize) : I_StripEffectBase(pRenderer, pManager, iIndex, iTypeIndex, iElementSize) {}
	virtual ~I_StripEffect() {}

	typedef T_Element			ElementType;
	typedef T_Strip				StripType;

protected:

	typedef C_Strip<ElementType, StripType>										Strip;
	typedef Ut::UtList<Strip, Ut::DoubleEndedList, Ut::MaintainedCounter>		StripList;
	typedef Ut::StaticPool<Strip, T_NumStrips>									StripPool;

	void				OnBuild(const C_View& view, C_CompositeRenderNode& node) const;
	virtual void		OnGather(const SceneGraph::C_View& view) const;

	void				Reset();

	Strip*				NewStrip();
	void				FreeStrip(typename StripList::Iterator& pIterator);

	StripList			m_Strips;				// list of active strips in the system
	StripPool			m_StripPool;			// pool of strips
};

template<class T_Element, class T_Strip, int T_NumStrips>
void I_StripEffect<T_Element, T_Strip, T_NumStrips>::OnBuild(const C_View& view, C_CompositeRenderNode& node) const
{
	node.iVertexBuffer = 0xFFFFFFFF;
}

template<class T_Element, class T_Strip, int T_NumStrips>
void I_StripEffect<T_Element, T_Strip, T_NumStrips>::OnGather(const SceneGraph::C_View& view) const
{

}

/*
template<class T_Element, class T_Strip, int T_NumStrips>
void I_StripEffect<T_Element, T_Strip, T_NumStrips>::OnGatherBlocks(BlockArrays& blockArrays, const U32 iViewIndex) const
{
	Maths::HVec4 vViewDirection(Maths::Load<Maths::HVec4>(&m_pRenderer->GetViewDirection()));
	Maths::HVec4 vViewPosition(Maths::Load<Maths::HVec4>(&m_pRenderer->GetViewPosition()));

	Maths::HF32 vDepthScale(vViewPosition.W());
	Maths::HF32 vMaxInt = (F32)0xFFFFFFFF;
	Maths::HF32 vDepthBias(Maths::Load<Maths::HF32>(&m_RenderStates.fDepthBias));

	for(typename StripList::ConstIterator pIterator = m_Strips.Begin(); pIterator != m_Strips.End(); ++pIterator)
	{
		// skip this block if it only has 1 segment since it won't be drawable
		if(pIterator->m_Blocks.Size() == 1)
		{
			if(pIterator->m_Blocks.Begin()->Size() - pIterator->m_iOffset == 1)
				continue;
		}

		assert(pIterator->GetElementCount() > 1);

		Maths::HVec4 vCentreOne(Maths::Load<Maths::HVec4>(&pIterator->m_vCentre));
		Maths::HVec4 vSizeCull(Maths::Load<Maths::HVec4>(&pIterator->m_vSize));
		Maths::HVec3 vCentre3(vCentreOne);
		Maths::HVec3 vSize3(vSizeCull);

		if(pIterator->m_pNode)
		{
			Maths::HMat43 mTransform(Maths::C_QuatTrans(*pIterator->m_pNode).Matrix());
			vCentre3 = vCentre3 * mTransform;
			vSize3 = vSize3 * mTransform.Rotation();
		}
		Maths::HVec4 vCentre(vCentre3);
		Maths::HVec4 vSize(vSize3);

		vCentre.W(vCentreOne.W());
		vSize.W(vSizeCull.W());

		S32 visible;

		if(pIterator->m_pNode)
			visible = Render::C_ViewManager::Instance().GetView(iViewIndex)->m_View.GetViewFrustum().CullSphere(vCentre, Maths::Length(vSize3), vSizeCull.W());
		else
			visible = Render::C_ViewManager::Instance().GetView(iViewIndex)->m_View.GetViewFrustum().CullAxisBox(vCentre, vSize);
		
		if(visible)
		{
			assert(pIterator->GetElementCount() > 1);

			Maths::HF32 vDepth = (vCentre | vViewDirection) + vDepthBias;
			vDepth = vMaxInt - vDepth * vDepthScale;
			
			C_RenderNode	block;
			block.pBlock = &(*pIterator);
			block.pMatrix = NULL;
			block.pInstance = NULL;
			block.sortKey = (U32)vDepth;

			blockArrays.transparent.PushBack(block);
		}
	}
}
*/

template<class T_Element, class T_Strip, int T_NumStrips>
typename I_StripEffect<T_Element, T_Strip, T_NumStrips>::Strip* I_StripEffect<T_Element, T_Strip, T_NumStrips>::NewStrip()
{
	Strip* pStrip = NULL;
	
	if((pStrip = m_StripPool.Allocate()))
	{
		pStrip->SetElementSize(m_iElementSize);
		pStrip->m_pManager = m_pManager;
		pStrip->m_pEffect = this;
		++m_nBlocks;
		assert(pStrip->m_Blocks.Size() == 0);

		// get block from manager
		if(typename Strip::ElementBlockType* pBlock = NewBlock(m_pManager, m_iElementSize))
			pStrip->m_Blocks.PushBack(*pBlock);
		else
		{
			m_StripPool.Free(pStrip);
			pStrip = NULL;
		}
	}

	return pStrip;
}

template<class T_Element, class T_Strip, int T_NumStrips>
void I_StripEffect<T_Element, T_Strip, T_NumStrips>::Reset()
{
	for(typename StripList::Iterator pIterator = m_Strips.Begin(); pIterator != m_Strips.End(); )
	{
		FreeStrip(pIterator);
	}
}

template<class T_Element, class T_Strip, int T_NumStrips>
void I_StripEffect<T_Element, T_Strip, T_NumStrips>::FreeStrip(typename StripList::Iterator& pIterator)
{
	// remove all storage that this strip is using
	pIterator->Free();

	 Strip* pDelete = &(*pIterator);

	pDelete->Reset();

	// remove strip from our list of strips
	pIterator = m_Strips.Erase(pIterator);
	// return strip to pool to be reused
	m_StripPool.Free(pDelete);
}

} // Strip
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_STRIP_EFFECT_H
