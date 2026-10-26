#include "stdafx.h"

#include "Dynamic/Renderer.h"
#include "Dynamic/TypeIndex.h"
#include "Dynamic/Particle/EffectInterface.h"
#include "Dynamic/Particle/Manager.h"

#include "Dynamic/Particle/Shape/All.h"

#include "SceneGraph/View.h"

// Horrible, but less evil than the alternative. (Blame charlie :) )

#include "../../../../../../amax/game/Game/GameWorld/ViewType.h"

namespace DynamicEffect
{
namespace Particle
{

I_ParticleEffectBase::I_ParticleEffectBase(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex, const U32 iTypeIndex) : I_DynamicEffect(pRenderer, pManager, iIndex, iTypeIndex), m_pManager(pManager)
{
#ifndef CFG_SPEED
	m_Allocated = 0;
#endif
	m_EffectPosition = -1;
	m_ActivePosition = -1;
	m_SpawnSettings.blocks.Resize(16);
	m_SpawnSettings.blocks.Clear();
}

void I_ParticleEffectBase::Destroy()
{
	if(m_RenderSettings.pDiffuse)
		m_RenderSettings.pDiffuse->Release();

	if(m_RenderSettings.pNormalMap)
		m_RenderSettings.pNormalMap->Release();
}

void I_ParticleEffectBase::BeginFrame()
{
	for(S32 b = 0; b < m_SpawnSettings.blocks.Size(); ++b)
	{
		if(ParticleBlock* pBlock = m_SpawnSettings.blocks[b])
		{
			assert(pBlock->m_pEffect == this);
		}
	}
}

S32 I_ParticleEffectBase::GetNumParticles() const
{
	S32 count = 0;
	for(S32 b = 0; b < m_SpawnSettings.blocks.Size(); ++b)
	{
		count += m_SpawnSettings.blocks[b]->m_Element.Size();
	}

	return count;
}

void I_ParticleEffectBase::OnGather(const SceneGraph::C_View& view) const
{
	if (view.GetType() == Game::E_ViewType_LightTrails)
	{
		if (!m_RenderSettings.lightTrail)
			return;
	}
	else
	{
		if (m_RenderSettings.lightTrail)
			return;
	}

	S32 b;
	S32 size = m_SpawnSettings.blocks.Size();
	for(b = 0; b < size - 1; ++b)
	{
		if(ParticleBlock* pPrefetch = m_SpawnSettings.blocks[b+1])
		{
			DCBT(0, pPrefetch);
			DCBT(0x80, pPrefetch);
		}
		if(ParticleBlock* pBlock = m_SpawnSettings.blocks[b])
			pBlock->Gather(view);
	}
	if(ParticleBlock* pBlock = m_SpawnSettings.blocks[b])
		pBlock->Gather(view);
}

void I_ParticleEffectBase::Create(const UpdateSettings& updateSettings, const RenderStates& renderStates, const RenderSettings& renderSettings, const PhysicsSettings& physicsSettings)
{
	m_UpdateSettings = updateSettings;
	m_RenderStates = renderStates;
	m_RenderSettings = renderSettings;
	m_PhysicsSettings = physicsSettings;

	switch(m_iTypeIndex)
	{
		case E_EffectTypeIndex_Shape2DCollision:
		case E_EffectTypeIndex_Shape2DCollisionSuper:
		case E_EffectTypeIndex_Shape2DSimple:
		case E_EffectTypeIndex_Shape2DSimpleSuper:
		case E_EffectTypeIndex_Shape2DSpline:
			m_RenderSettings.vertexBuildSettings.eVertexBuild = E_VertexBuild_Shape2D;
			break;
		case E_EffectTypeIndex_Shape3DCollision:
		case E_EffectTypeIndex_Shape3DCollisionSuper:
		case E_EffectTypeIndex_Shape3DDecal:
		case E_EffectTypeIndex_Shape3DSimple:
		case E_EffectTypeIndex_Shape3DSimpleSuper:
			m_RenderSettings.vertexBuildSettings.eVertexBuild = E_VertexBuild_Shape3D;
			break;
		case E_EffectTypeIndex_SpriteRotating:
		case E_EffectTypeIndex_SpriteRotatingCollision:
		case E_EffectTypeIndex_SpriteRotatingSpline:
		case E_EffectTypeIndex_SpriteRotatingSuper:
			m_RenderSettings.vertexBuildSettings.eVertexBuild = E_VertexBuild_SpriteRotating;
			break;
	}
	m_RenderSettings.vertexShader = ShapePolicy::All::GetVertexShader(this);
	m_RenderSettings.primitiveType = ShapePolicy::All::GetPrimitiveType(this);
	m_RenderSettings.pVertexDeclaration = ShapePolicy::All::GetVertexDeclaration(this);
	m_RenderSettings.uvScale = ShapePolicy::All::GetUvScale(this);
	m_RenderSettings.uvScale.z = m_RenderSettings.additive ? 0.0f : 1.0f;
	m_RenderSettings.vertices = ShapePolicy::All::GetNumPrimitives(this) * ShapePolicy::All::GetNumVertices(this);
	m_RenderSettings.vertexSize = ShapePolicy::All::GetVertexSize(this);

	m_PhysicsSettings.vertexSize = m_RenderSettings.vertices * m_RenderSettings.vertexSize;

	m_RenderSettings.pixelShader = m_pManager->GetPixelShaderFromRenderSettings(m_RenderSettings);
	m_RenderSettings.vertexBuildSettings.iElementSize = m_pParticleFormat->GetElementSize();
	
	m_SpawnSettings.lodFade = m_RenderSettings.lodFade;
	m_SpawnSettings.cullDistance = m_RenderSettings.cullDistance;
	m_SpawnSettings.vertexSize = m_RenderSettings.vertexSize;
	m_SpawnSettings.atlasSize = m_RenderSettings.vertexBuildSettings.iAtlasSize;
	m_SpawnSettings.atlasOffset = m_RenderSettings.vertexBuildSettings.iAtlasOffset;
	m_SpawnSettings.particleShape = m_RenderSettings.vertexBuildSettings.eParticleShape;

	if(m_RenderSettings.vertexBuildSettings.iParticleLibraryIndex != -1)
	{
		const C_UserShape3DVariants& variants = C_UserParticleLibrary::Instance().GetShape3DVariants(m_RenderSettings.vertexBuildSettings.iParticleLibraryIndex);
		m_SpawnSettings.meshCount = variants.GetShape3DArray().Size();
		m_SpawnSettings.meshScale = variants.GetScale();
	}
	else
	{
		m_SpawnSettings.meshCount = 0;
		m_SpawnSettings.meshScale = 0.0f;
	}
	
	m_SpawnSettings.nodeType = m_pRenderer->GetNodeType();
#ifndef CFG_SPEED
	m_SpawnSettings.pAllocated = &m_Allocated;
#endif
	m_SpawnSettings.pEffect = this;
	m_SpawnSettings.pBlocks = &m_nBlocks;
	m_SpawnSettings.pBlocksLocal = &m_nBlocksLocal;
	
	// if lit is true then we want to have BeginFrame called on us to gather lights
	//if(m_RenderSettings.bLightEmit == true)
		m_bBegin = true;
}

void I_ParticleEffectBase::Clean(const Display::C_Texture2D* pTexture)
{
	if(m_RenderSettings.pDiffuse == pTexture)
		m_RenderSettings.pDiffuse = NULL;

	if(m_RenderSettings.pNormalMap == pTexture)
		m_RenderSettings.pNormalMap = NULL;
}



void I_ParticleEffectBase::Free()
{
	for(S32 b = 0; b < m_SpawnSettings.blocks.Size(); ++b)
	{
		if(ParticleBlock* pBlock = m_SpawnSettings.blocks[b])
		{
			assert(pBlock->m_pEffect == this);
		}
	}

	// give blocks back to block manager
	for(S32 b = 0; b < m_SpawnSettings.blocks.Size(); )
	{
		if(ParticleBlock* pBlock = m_SpawnSettings.blocks[b])
			DeleteBlock(pBlock);
		else
			++b;
	}
	m_SpawnSettings.blocks.Clear();
}

void I_ParticleEffectBase::Clean(const C_Node* pNode)
{
	for(S32 i = 0; i < m_SpawnSettings.blocks.Size(); ++i)
	{
		if(m_SpawnSettings.blocks[i]->m_pNode == pNode)
			DeleteBlockDeferred(m_SpawnSettings.blocks[i]);
	}
}

void I_ParticleEffectBase::DeleteBlock(ParticleBlock* pDelete)
{
	assert(m_nBlocks == m_SpawnSettings.blocks.Size());
#ifndef CFG_SPEED
	assert(m_nBlocks == m_Allocated);
	assert(m_Allocated <= m_SpawnSettings.blocks.Capacity());
#endif

	for(S32 i = 0; i < m_nBlocks; ++i)
	{
		assert(m_SpawnSettings.blocks[i]->m_pEffect == this);
	}
	assert(pDelete->m_pEffect == this);

	if(pDelete->m_pNode)
		--m_nBlocksLocal;
	
	// erase from vector
	ParticleBlock* pEnd = m_SpawnSettings.blocks[m_SpawnSettings.blocks.Size() - 1];
	assert(pEnd);
	m_SpawnSettings.blocks[pDelete->m_VectorPosition] = pEnd;
	pEnd->m_VectorPosition = pDelete->m_VectorPosition;

	assert(pEnd->m_pEffect == this);

	// set null and give back to storage
	pDelete->m_pEffect = NULL;
	pDelete->m_Element.Clear();
	++pDelete->m_iSalt; // increment salt
	m_SpawnSettings.pBlockManager->FreeBlock(pDelete);
#ifndef CFG_SPEED
	--m_Allocated;
#endif
	m_SpawnSettings.blocks.Size(--m_nBlocks);
#ifndef CFG_SPEED
	assert(m_Allocated <= m_nBlocks);
#endif
}


} // Particle
} // DynamicEffect
