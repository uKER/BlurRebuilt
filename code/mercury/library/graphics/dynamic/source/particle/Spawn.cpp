#include "stdafx.h"

#include "Dynamic/Particle/Spawn.h"
#include "Dynamic/BlockManager.h"

#ifdef PLATFORM_SPU
#include "../source/maths/hvec/Matrix.cpp"
namespace Maths
{ 
	const vector float Const< vector float >::m_UnitW = (vector float)( 0.0f, 0.0f, 0.0f, 1.0f );
	const vector unsigned int Const< vector unsigned int >::m_SignMask = (vector unsigned int)( 0x80000000, 0x80000000, 0x80000000, 0x80000000 );
	const vector unsigned int Const< vector unsigned int >::m_AbsMask  = (vector unsigned int)( 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF );
	const F32 Pi     = (F32)(3.1415926535897932384626433832795);
	const F32 Pi2    = (F32)(6.283185307179586476925286766558);
	ALIGN16 const F32 g_Const_PI[8] =
	{
		3.1415926535897932384626433832795f,  // PI
		6.283185307179586476925286766559f,   // 2.0 * PI
		0.31830988618379067153776752674503f, // 1.0 / PI
		0.15915494309189533576888376337251f, // 2.0 / PI
		1.5707963267948966192313216916398f,  // PI / 2.0
		0.78539816339744830961566084581988f, // PI / 4.0
		0.70710678118654752440084436210485f, // sin( PI / 4.0f )
		0.0f
	};
}
#endif

namespace DynamicEffect
{
namespace Particle
{

// spawning functions

ParticleBlock* BeginCreateExisting(U32& nNumEmit, U32& nTotalEmit, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, const EmitterUpdateConstants* pUpdateConstants)
{
	// check to see if we can fit creation inside any current blocks
	if(ParticleBlock* pBlockLS = pEmitter->GetBlock())
	{
		assert(pBlockLS->m_pEffect);
		assert(pBlockLS->m_Element.Remaining() > 0);
		// check local position since block's centre is in local space
		Maths::HVec3 vBlock(Maths::Load<Maths::HVec3>(&pEmitter->GetPosition()));
		Maths::HVec3 vCentre(Maths::Load<Maths::HVec3>(&pBlockLS->m_vCentre));
		vCentre -= vBlock;
		// only return current block if its within 5 metres of centre of block
		if(vCentre.Length2() < 25.f)
		{
			U32 remaining = (U32) pBlockLS->m_Element.Remaining();
			if(nTotalEmit < remaining)
			{
				// theres more than enough space to fit this emission
				nNumEmit = nTotalEmit;
				nTotalEmit = 0;
			}
			else
			{
				// emission is equal or greater than remaining space - set emitter block to NULL
				nTotalEmit -= remaining;
				nNumEmit = remaining;
			}
			return pBlockLS;
		}
	}

	return NULL;
}

ParticleBlock* BeginCreateNew(U32& nNumEmit, U32& nTotalEmit, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, const EmitterUpdateConstants* pUpdateConstants)
{
#ifndef CFG_SPEED
	//assert(*pSpawnSettings->pAllocated <= pSpawnSettings->blocks.Capacity());
#endif
	S32 queuePosition = MT::AtomicIncrement(pSpawnSettings->pBlocks);

	if(queuePosition == 0)
	{
		S32 activedPosition = MT::AtomicIncrement(pUpdateConstants->pActivedEffectsCount);
		Spu::Set((INTPTR*)&pUpdateConstants->ppActivedEffects[activedPosition], (INTPTR)(I_ParticleEffect*)pSpawnSettings->pEffect);	// [Blur2PC][x64] pointer-wide
	}

	if(queuePosition < pSpawnSettings->blocks.Capacity())
	{
		if(ParticleBlock* pBlockEA = pSpawnSettings->pBlockManager->NewBlock())
		{
		#ifndef CFG_SPEED
			S32 currentAllocated = MT::AtomicIncrement(pSpawnSettings->pAllocated);
			assert(currentAllocated < pSpawnSettings->blocks.Capacity());
		#endif
		#ifdef PLATFORM_SPU
			Spu::Fetch(pEmitter->m_pBlockLS, pBlockEA, sizeof(ParticleBlock));
			ParticleBlock* pBlockLS = pEmitter->m_pBlockLS;
		#else
			ParticleBlock* pBlockLS = pBlockEA;
		#endif

			pBlockLS->Clear();
			pBlockLS->SetNodeType(pSpawnSettings->nodeType);
			pBlockLS->m_fTime = pUpdateConstants->time;
			pBlockLS->m_fExpiryTime = pEmitter->GetDefinition()->fExpiryTime;
			pBlockLS->m_pEffect = (I_DynamicEffect*)pSpawnSettings->pEffect;
			pBlockLS->m_iFlags = 0;
			pBlockLS->m_vLodFade = pEmitter->GetDefinition()->lodFade.x != 0.0f ? pEmitter->GetDefinition()->lodFade : pSpawnSettings->lodFade;
			pBlockLS->m_VectorPosition = queuePosition;
			pBlockLS->SetVisibilityFlags(pEmitter->GetVisibilityFlags());

			Spu::Set((INTPTR*)&pSpawnSettings->blocks[queuePosition], (INTPTR)pBlockEA);	// [Blur2PC][x64] pointer-wide

			if(pEmitter->GetDefinition()->eSpace == E_EmitterSpace_Local)
			{
				if((pBlockLS->m_pNode = pEmitter->GetParentTransformEA()))
					MT::AtomicIncrement(pSpawnSettings->pBlocksLocal);
			}
			else
				pBlockLS->m_pNode = NULL;

			pEmitter->SetBlock(pBlockLS, pBlockEA);

			// work out whether we emit a full blocks worth or not
			if(nTotalEmit < pSpawnSettings->pBlockManager->GetBlockSize())
			{
				nNumEmit = nTotalEmit;
				nTotalEmit = 0;
			}
			else
			{
				nNumEmit = pSpawnSettings->pBlockManager->GetBlockSize();
				nTotalEmit -= nNumEmit;
			}
			return pBlockLS;
		}
		else
		{
			Spu::Set((INTPTR*)&pSpawnSettings->blocks[queuePosition], (INTPTR)0);	// [Blur2PC][x64] pointer-wide
		}
	}

	nNumEmit = 0;
	nTotalEmit = 0;
	return NULL;
}

} // Particle
} // DynamicEffect
