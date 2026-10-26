#ifndef RENDER_DYNAMICEFFECT_PARTICLE_EFFECT2D_H
#define RENDER_DYNAMICEFFECT_PARTICLE_EFFECT2D_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/EffectInterface.h"
#include "Dynamic/Particle/Emitter.h"

#include "Dynamic/Defines.h"
#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
#include "Dynamic/Particle/UpdateXM.h"
#else
#include "Dynamic/Particle/UpdateHV.h"
#endif

namespace DynamicEffect
{
namespace Particle
{

template<class ParticleType, template <class Type> class ShapePolicy, template <typename Type, template <class Type> class Render> class BuildPolicy, template <class Type> class CreationPolicy>
class I_ParticleEffect2D : public I_ParticleEffectBase
{
public:
	// use this typedef to improve readability
	typedef I_ParticleEffectBase	Base;

	I_ParticleEffect2D(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex, const U32 iTypeIndex) : Base(pRenderer, pManager, iIndex, iTypeIndex, sizeof(ParticleType)) { }	
	virtual ~I_ParticleEffect2D() { };

	virtual C_RenderNode*	Spawn(const C_Emitter* pEmitter);
	
protected:

	C_RenderNode*			BeginCreate(U32 nNum, BlockBase* pBlock);
};

/*
template<class ParticleType, template <class Type> class ShapePolicy, template <typename Type, template <class Type> class Render> class BuildPolicy, template <class Type> class CreationPolicy>
void I_ParticleEffect2D<ParticleType, ShapePolicy, BuildPolicy, CreationPolicy>::OnGatherBlocks(BlockArrays& blockArrays, const U32 iViewIndex) const
{
	// opaque and decals can be sorted by material
	for(typename Base::BlockList::ConstIterator pBlock = Base::m_Blocks.Begin(); pBlock != Base::m_Blocks.End(); ++pBlock)
	{
		C_RenderNode	block;
		block.pBlock = &(*pBlock);
		block.pMatrix = NULL;
		block.pInstance = NULL;
		block.sortKey = Base::m_iIndex;	// sort by effect

		blockArrays.twoDimensional.PushBack(block);
	}
}
*/

template<class ParticleType, template <class Type> class ShapePolicy, template <typename Type, template <class Type> class Render> class BuildPolicy, template <class Type> class CreationPolicy>
C_RenderNode* I_ParticleEffect2D<ParticleType, ShapePolicy, BuildPolicy, CreationPolicy>::Spawn(const C_Emitter* pEmitter)
{
	return NULL;
}


template<class ParticleType, template <class Type> class ShapePolicy, template <typename Type, template <class Type> class Render> class BuildPolicy, template <class Type> class CreationPolicy>
FORCE_INLINE C_RenderNode* I_ParticleEffect2D<ParticleType, ShapePolicy, BuildPolicy, CreationPolicy>::BeginCreate(U32 nNum, C_RenderNode* pBlock = NULL)
{
	if(pBlock)
	{
		// see if block has potentially died
		if(pBlock->m_pEffect != this)
			return NULL;
		else if(nNum <= (GetBlockSize(Base::m_pManager, Base::m_iElementSize) - pBlock->Size()))
		{
			Base::m_pCurrent = (typename ParticleBlock*)pBlock;
			// move the block to the front of the block list
			typename BlockList::Iterator pMoveToFront(Base::m_pCurrent);
			pMoveToFront = Base::m_Blocks.Erase(pMoveToFront);
			Base::m_Blocks.PushFront(*Base::m_pCurrent);
			return pBlock;
		}
	}
	
	if((Base::m_pCurrent = NewBlock(Base::m_pManager, Base::m_iElementSize)))
	{
		++Base::m_nBlocks;
		++Base::m_nVisBits;
		Base::m_pCurrent->m_fTime = Base::m_pRenderer->GetTime();
		Base::m_pCurrent->m_pEffect = this;
		Base::m_Blocks.PushFront(*Base::m_pCurrent);
		
		return Base::m_pCurrent;
	}

	return NULL;
}

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_EFFECT2D_H
