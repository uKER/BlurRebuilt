#include "stdafx.h"

// header
#include "Sprite2D.h"

using namespace DynamicEffect;

Util::C_Random g_Random;

// initialise system with correct values, then define custom creation and updating functions
C_Sprite2D::C_Sprite2D(C_Renderer* pRenderer, const U32 iIndex) : I_ParticleEffect2D<C_Particle2D, ShapePolicy::Sprite2D, BuildPolicy::BeginVertices2D, Sprite2DSimpleCreate>(pRenderer, iIndex, 10101)
{
	m_UpdateSettings.fAirResistance = 0.0f;

	m_RenderStates.eBlendMode = DynamicEffect2::E_BlendMode_Solid;
	m_RenderSettings.pDiffuse = NULL;
	m_RenderSettings.emissiveColour = MathsStorage::Float4A16(0.0f, 0.0f, 0.0f, 0.0f);
	m_RenderSettings.nAtlasSize = 1;
	m_RenderSettings.nAtlasSizeSq = 1;

	m_RenderSettings.colourTable.table.PushBack(MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f));
	m_RenderSettings.colourTable.table.PushBack(MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f));
	m_RenderSettings.colourTable.vSize = MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f);

	m_RenderSettings.sizeTable.table.PushBack(MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f));
	m_RenderSettings.sizeTable.table.PushBack(MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f));
	m_RenderSettings.sizeTable.vSize = MathsStorage::Float4A16(1.0f, 1.0f, 1.0f, 1.0f);
}

C_Sprite2D::~C_Sprite2D()
{
}

void C_Sprite2D::Update()
{
	
	{
		PERF_EVENT("Sprite2D Spawn");
		TestSpawn();
	}
	
	
	PERF_EVENT("Sprite2D");
#ifdef RENDER_DYNAMICEFFECT2_USE_XMVECTOR
	XMVECTOR vTime = XMVectorSplatX(XMLoadFloat4A16((XMFLOAT4A16*)&m_pRenderer->GetTiming()));
	XMVECTOR vAcceleration = XMVectorMultiply(vTime, XMLoadFloat2((XMFLOAT2*)&m_UpdateSettings.vAcceleration));
	XMVECTOR vResistance = XMVectorSubtract(XMVectorSplatOne(), XMVectorMultiply(vTime, XMVectorReplicate(m_UpdateSettings.fAirResistance)));
#else
	Maths::HF32 vTime = m_pRenderer->GetFrameTime();
	Maths::HVec2 vAcceleration = Maths::HVec2(Maths::Load<Maths::V128>(&m_UpdateSettings.vAcceleration)) * vTime;
	Maths::HF32 vResistance = 1.0f - vTime * m_UpdateSettings.fAirResistance;
#endif

	// go through every block this system currently owns updating particles
	for(IteratorBegin(); !IteratorEnd(); )
	{
		ParticleBlock* pBlock = (ParticleBlock*)&(*m_pIterator);

		ParticleUpdate2D<Particle>	working;
	#ifdef PLATFORM_XENON1
		BlockList::Iterator pIteratorNext = m_pIterator;
		++pIteratorNext;

		if(pIteratorNext != m_Blocks.End())
		{
			ParticleBlock* pNextBlock = (ParticleBlock*)&(*pIteratorNext);
			__dcbt(0, &pNextBlock->m_Element[0]);
		}
	#endif
				
		for(U32 a = 0; a < pBlock->m_nCount; )
		{
			// prefetch data into cache lines
#ifdef PLATFORM_XENON
			__dcbt(128, &pBlock->m_Element[a]);
#endif

			Particle* pParticle = (Particle*)(&pBlock->m_Element[a]);
			
			working.SetParticle(pParticle);
			working.Load(vTime);
			//working.UpdateVelocity(vResistance, vAcceleration);
			//working.UpdatePosition();
			working.UpdateLife();
		
			// if particle is dead, copy particle from last block into here
			if(working.IsDead())
				FreeParticle(a);
			else
			{
				working.Store();
				a++;
			}
		}

		// update the blocks bounds and move to next block
		working.StoreBounds(&pBlock->m_vMin);
		IteratorNext();
	}
}

void C_Sprite2D::TestSpawn()
{
	if(m_nElements < 1000)
	{
		if(BeginCreate(10, NULL))
		{
			// set visible in both viewports
			m_pCurrent->m_iFlags = 2;

			MathsStorage::Float2 vOffset(g_Random.FRand(0.8f), g_Random.FRand(0.8f));
			
			U32 nSpawn = g_Random.Rand(10);
			for(U32 i = 0; i < nSpawn; ++i)
			{
				if(Particle* pParticle = NewParticle())
				{
					pParticle->vPosition = MathsStorage::Float2(vOffset.x + g_Random.FRand(0.2f), vOffset.y + g_Random.FRand(0.2f));
					pParticle->vVelocity = MathsStorage::Float2(g_Random.FRand(-0.1f, 0.1f), g_Random.FRand(-0.1f));
					pParticle->vLifeSizeLife = MathsStorage::Float3(1.0f, g_Random.FRand(0.01f), g_Random.FRand(0.8f, 1.2f));				// x = life, y = size, z = life modifier
					pParticle->iVariant = 0;
					pParticle->iFlags = 0;
					pParticle->iGameData0 = 0;	
					pParticle->iGameData1 = 0;	
					pParticle->vColour = MathsStorage::Float4(g_Random.FRand(1.0f), g_Random.FRand(1.0f), g_Random.FRand(1.0f), 1.0f);
				}
			}
		}
	}
}

















