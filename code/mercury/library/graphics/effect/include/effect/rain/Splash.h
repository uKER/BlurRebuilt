#ifndef EFFECT_RAIN_SPLASH_H
#define EFFECT_RAIN_SPLASH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Parameters.h"
#include <Render/View/ViewManager.h>
#include <System/Misc/Random.h>
#include <core\containers\bitvector.h>
#include <core\utility\singleton.h>

namespace Effect
{

namespace Rain
{

const U32					g_ParticlesPerBlock = 256;
extern Util::C_Random		g_Random;
extern ::Debug::Bool		g_debugSplashFreezeOffscreen;

#define USE_INT_LIFE

struct ALIGN16 SplashParticle
{
#ifdef USE_INT_LIFE
	MathsStorage::Float3				vPosition;
	F32									fLife;
	MathsStorage::Float4A16				vColour;
	MathsStorage::Float2				vForceSizeLife;
	S32									iLife;
	S32									iLifeReduce;
#else
	MathsStorage::Float3				vPosition;
	F32									fLife;
	MathsStorage::Float4A16				vColour;
	MathsStorage::Float4A16				vForceSizeLife;
#endif
};

struct ALIGN16 SplashBlock : public Ut::ListElementDouble<SplashBlock>
{
	MathsStorage::Float4A16				m_vMin;
	MathsStorage::Float4A16				m_vMax;			
	MathsStorage::Float4A16				m_vCentre;
	MathsStorage::Float4A16				m_vSize;
	F32									m_fDistanceSpawnScale;
	F32									m_fSpeedSpawnScale;
	F32									m_fCycle;
	bool								m_bVisible;
	S32									m_iX;			// the x gridsquare this block represents
	S32									m_iZ;			// the y gridsquare this block represents
	S32									m_iIndex;
	Ut::Vector<SplashParticle>			m_Element;
	void*								m_pCollisionData;
};

typedef Ut::UtVector<Ut::StackVector<Ut::BitVector, 4> >		BitVectors;

struct SplashSettings
{
	F32		fBlockSize;
	F32		fDistanceFade;
	F32		fDistanceFadeSq;
	F32		fDistanceFadeRcp;
};

class C_SplashRendererBase : public Display::I_Renderer
{
public:

	C_SplashRendererBase();
	~C_SplashRendererBase();

	bool OnCreate();
	void OnDestroy();

	void SetLighting(const MathsStorage::Float4& vLighting) { m_vLighting = vLighting; }
	
	void SetParameters(const SplashParameters& parameters);
	const SplashParameters& GetParameters() const { return m_Parameters; }

	void Update(const F32 fTime, const bool bPaused);
	
	void PreBuild();
	void Display() const;

protected:
	typedef Ut::UtMap<U32, bool>															ActiveBlocks;
	typedef Ut::DynamicPool<SplashBlock, Ut::ArrayAllocator<U8>, Ut::SingleThreaded>	BlockPool;
	typedef	Ut::UtList<SplashBlock, Ut::DoubleEndedList, Ut::MaintainedCounter>			BlockList;
	
	bool OnInitialise();
	void OnShutdown();
	
	void		DisplayBlocks(const Ut::Vector<const SplashBlock*> &blocks, const S32 vertexCount) const;
	S32			GetSpawnAmount();
	void		SetupBlock(SplashBlock& block);
	S32			PackCoords(S16 x, S16 y) const;

	Display::C_Texture2D*				m_pTexture;
	Display::C_VertexShader				m_VertexShader;
	Display::C_PixelShader				m_PixelShader;

	MathsStorage::Float4				m_vLighting;

	Ut::Vector<MathsStorage::Short2>	m_GridPositions;
	Ut::Vector<MathsStorage::Short2>	m_CameraPositions;

	SplashParameters					m_Parameters;
	SplashSettings						m_Settings;

	bool								m_bActive;
	S32									m_nBlocks;
	S32									m_nParticles;

	F32									m_fTime;
	F32									m_fFrameTime;
	F32									m_fFrameTimeRcp;
	F32									m_fCycle;
	BlockList							m_Blocks;
	BlockPool							m_BlockPool;
	ActiveBlocks						m_ActiveBlocks;
	BitVectors							m_VisBits;

	static const U32 m_nNumPrimitives = 2;
	static const U32 m_nNumVertices = 3;
	static const Display::E_PrimitiveType	m_ePrimitiveType = Display::E_PrimitiveType_TriangleList;

private:
	void	SetDefaults();
};

INLINE S32 C_SplashRendererBase::PackCoords(S16 x, S16 z) const
{
	return (x << 16) + (((S32)z) & 0xFFFF);
}

INLINE void C_SplashRendererBase::SetParameters(const SplashParameters& parameters)
{
#ifndef CFG_GOLD
	bool bRecreateAssets = false;

	if(m_Parameters.texture != parameters.texture)
		bRecreateAssets = true;
#endif

	m_Parameters = parameters;

	// clamp emitrate to a sensible range
	m_Parameters.fEmitRate = Maths::Clamped(m_Parameters.fEmitRate, 0.0f, 10000.0f);
	m_Parameters.fEmitRateRcp = (m_Parameters.fEmitRate > 0.f) ? 1.0f / m_Parameters.fEmitRate : 0.f;

	m_bActive = m_Parameters.fEmitRate > 0.0f;

#ifndef CFG_GOLD
	if(bRecreateAssets)
		Reinitialise();
#endif
}

// game needs to provide an implementation of this class, probably using game's physics
class DefaultGamePolicy
{
public:
	void	BeginSpawn()
	{
	}

	void	SpawnBlock(SplashBlock* pBlock, const SplashSettings& settings, const SplashParameters& parameters, const F32 fFrameTime, S32 nNumEmit)
	{
		// make sure at least one particle gets spawned into block for culling etc
		if(nNumEmit == 0 && pBlock->m_Element.Size() == 0)
		{
			Maths::HVec3 vMin(pBlock->m_iX * settings.fBlockSize, parameters.fHeightOffset, pBlock->m_iZ * settings.fBlockSize );
			Maths::HVec3 vBlock(settings.fBlockSize, 0.0f, settings.fBlockSize);
			Maths::HVec3 vHalfBlock = vBlock * Maths::Constants<Maths::HF32>::Half();
			Maths::HVec3 vMax = vMin + vBlock;

			Maths::HVec3 vCentre = vMin + vHalfBlock;
							
			Maths::Store((Maths::V128)vMin, &pBlock->m_vMin);
			Maths::Store((Maths::V128)vMax, &pBlock->m_vMax);
			Maths::Store((Maths::V128)vCentre, &pBlock->m_vCentre);
			Maths::Store((Maths::V128)vHalfBlock, &pBlock->m_vSize);

			pBlock->m_vCentre.w = 1.0f;
			pBlock->m_vSize.w = settings.fDistanceFade;
		}
		else if(nNumEmit != 0)
		{
			Maths::HVec3 vMax(Maths::Load<Maths::HVec3>(&pBlock->m_vMax));
			Maths::HVec3 vMin(Maths::Load<Maths::HVec3>(&pBlock->m_vMin));
			
			while(nNumEmit > 0 && (U32)pBlock->m_Element.Size() < g_ParticlesPerBlock)
			{		
				pBlock->m_Element.PushBack();
				SplashParticle* pParticle = &pBlock->m_Element.Back();

				F32 fColour = Maths::Clamped(parameters.fColourScale + g_Random.FRand(-parameters.fColourScale, parameters.fColourScale), 0.0f, 1.0f);
				
	
				Maths::HVec4 vPositionLife;
				vPositionLife = Maths::HVec4(pBlock->m_iX * settings.fBlockSize + g_Random.FRand(settings.fBlockSize),
											parameters.fHeightOffset,
											pBlock->m_iZ * settings.fBlockSize + g_Random.FRand(settings.fBlockSize),
											1.0f);
				Maths::HVec3 vPosition(vPositionLife);
				vMax = vMax.Max(vPosition);
				vMin = vMin.Min(vPosition);
				Maths::Store(vPositionLife, (MathsStorage::Float4A16*)&pParticle->vPosition);
				
				pParticle->vColour = MathsStorage::Float4A16(fColour, fColour, fColour, fColour);
				pParticle->vForceSizeLife.x = 1.0f + g_Random.FRand(-parameters.fLifeVariation, parameters.fLifeVariation);
				pParticle->vForceSizeLife.y = 1.0f / (parameters.fLife + g_Random.FRand(-parameters.fLifeVariation, parameters.fLifeVariation));
				
				#ifdef USE_INT_LIFE
				pParticle->iLife = (1 << 30);
				pParticle->iLifeReduce = (S32)(fFrameTime * pParticle->vForceSizeLife.y * (F32)(1 << 30));
				#endif

				--nNumEmit;
			}

			Maths::HVec3 vCentre = (vMin + vMax) * 0.5f;
			Maths::HVec3 vSize = vMax - vCentre;

			Maths::Store((Maths::V128)vMin, &pBlock->m_vMin);
			Maths::Store((Maths::V128)vMax, &pBlock->m_vMax);
			Maths::Store((Maths::V128)vCentre, &pBlock->m_vCentre);
			Maths::Store((Maths::V128)vSize, &pBlock->m_vSize);

			pBlock->m_vCentre.w = 1.0f;
			pBlock->m_vSize.w = settings.fDistanceFade;
		}
	}

	void CreateBlock(SplashBlock* pBlock, F32 fBlockSize)
	{
	}

	void RemoveBlock(Effect::Rain::SplashBlock* pBlock)
	{
	}
};
			

template <class GamePolicy = DefaultGamePolicy>
class C_SplashRenderer : public C_SplashRendererBase, public Ut::Singleton<C_SplashRenderer<GamePolicy>, Ut::CreateExternal>
{
public:
	void Update(const F32 fTime, const bool bPaused);
	void Shutdown();
private:
	void Spawn();
	void UpdateBlocks();

	GamePolicy	m_GamePolicy;
};

template <class GamePolicy>
void C_SplashRenderer<GamePolicy>::Update(const F32 fTime, const bool bPaused)
{
	C_SplashRendererBase::Update(fTime, bPaused);
	if(!bPaused && m_bActive)
	{
		UpdateBlocks();
		Spawn();
	}
}

template <class GamePolicy>
void C_SplashRenderer<GamePolicy>::Shutdown()
{
	C_SplashRendererBase::Shutdown();

	for(BlockList::Iterator pBlock = m_Blocks.Begin(); pBlock != m_Blocks.End(); )
	{
		SplashBlock* pDelete = &(*pBlock);
		m_GamePolicy.RemoveBlock(pDelete);
		pDelete->m_Element.Clear();
		pBlock = m_Blocks.Erase(pBlock);
		m_BlockPool.Free(pDelete);
	}
}

template <class GamePolicy>
void C_SplashRenderer<GamePolicy>::Spawn()
{
	S32 nTotalEmit = GetSpawnAmount();

	static Util::C_String128 pixName;
	PERF_EVENT(pixName.Printf("RainSplashes::SpawnParticles (%d)", m_Blocks.Size() * nTotalEmit));
	
	m_GamePolicy.BeginSpawn();

	for(BlockList::Iterator pBlock = m_Blocks.Begin(); pBlock != m_Blocks.End(); ++pBlock)
	{
		if(pBlock->m_bVisible || !g_debugSplashFreezeOffscreen)
		{
			pBlock->m_fCycle += m_fFrameTime * pBlock->m_fDistanceSpawnScale * pBlock->m_fSpeedSpawnScale;
			S32 nNumEmit = (S32)(pBlock->m_fCycle * m_Parameters.fEmitRate);

			m_GamePolicy.SpawnBlock(&(*pBlock), m_Settings, m_Parameters, m_fFrameTime, nNumEmit);

			if(nNumEmit != 0)
				pBlock->m_fCycle -= nNumEmit * m_Parameters.fEmitRateRcp;
		}
	}
};

template <class GamePolicy>
void C_SplashRenderer<GamePolicy>::UpdateBlocks()
{
	PERF_EVENT("RainSplashes::UpdateBlocks");

	Ut::Vector<MathsStorage::Short2> cameraPositions;
	// work out current grid positions of cameras
	for(S32 iView = 0; iView < Render::C_ViewManager::Instance().GetViewCount(); ++iView)
	{
		const Display::C_View* pView = &Render::C_ViewManager::Instance().GetView(iView)->m_View;
		if(pView->GetUserIndex() != -1)
		{
			const Display::C_ViewSettings3D& viewSettings3D = pView->GetViewSettings3D();
			MathsStorage::Float4 vCameraPosition = viewSettings3D.GetViewPosition();
			MathsStorage::Short2 vCameraGrid;

			vCameraGrid.x = (S32)(vCameraPosition.x / m_Settings.fBlockSize);
			vCameraGrid.y = (S32)(vCameraPosition.z / m_Settings.fBlockSize);

			if(vCameraPosition.x < 0.0f)
				--vCameraGrid.x;
			if(vCameraPosition.z < 0.0f)
				--vCameraGrid.y;

			cameraPositions.PushBack(vCameraGrid);
		}
	}

	// only recalculate block positions if cameras have changed grid squares
	m_CameraPositions.Resize(cameraPositions.Size());
	bool bDifferent = false;
	for(S32 i = 0; i < cameraPositions.Size(); ++i)
	{
		if(cameraPositions[i].x != m_CameraPositions[i].x || cameraPositions[i].y != m_CameraPositions[i].y)
			bDifferent = true;
	}
	
	if(bDifferent)
	{
		PERF_EVENT("RainSplashes::RecreateBlocks");

		m_CameraPositions = cameraPositions;
		m_ActiveBlocks.Clear();

		// reinsert all active blocks
		for(S32 i = 0; i < cameraPositions.Size(); ++i)
		{
			for(S32 iGrid = 0; iGrid < m_GridPositions.Size(); ++iGrid)
			{
				m_ActiveBlocks.Insert(ActiveBlocks::DataType(PackCoords(cameraPositions[i].x +  m_GridPositions[iGrid].x, cameraPositions[i].y +  m_GridPositions[iGrid].y), false));
			}
		}

		// go through active blocks either removing them, or telling the active blocks map that we are created
		for(BlockList::Iterator pBlock = m_Blocks.Begin(); pBlock != m_Blocks.End(); )
		{
			ActiveBlocks::Iterator it = m_ActiveBlocks.Find(pBlock->m_iIndex);
			if(it != m_ActiveBlocks.End())
			{
				it->GetValue() = true;
				++pBlock;
			}
			else
			{
				SplashBlock* pDelete = &(*pBlock);
				m_GamePolicy.RemoveBlock(pDelete);
				pDelete->m_Element.Clear();
				pBlock = m_Blocks.Erase(pBlock);
				m_BlockPool.Free(pDelete);
			}
		}

		// go through active blocks map and create the ones we need to
		for(ActiveBlocks::Iterator pIterator = m_ActiveBlocks.Begin(); pIterator != m_ActiveBlocks.End(); pIterator++)
		{
			bool& bCreated = pIterator->GetValue();
			if(!bCreated)
			{
				SplashBlock* pBlock = m_BlockPool.Allocate();
				pBlock->m_Element.Reserve(g_ParticlesPerBlock);
				m_Blocks.PushBack(*pBlock);
				S16 x = (S16)(pIterator->GetKey() >> 16);
				S16 z = (S16)pIterator->GetKey();
				pBlock->m_fCycle = 0.0f;
				pBlock->m_fDistanceSpawnScale = 0.0f;
				pBlock->m_fSpeedSpawnScale = 0.0f; 
				pBlock->m_iX = x;
				pBlock->m_iZ = z;
				pBlock->m_iIndex = PackCoords(x, z);
				pBlock->m_vMin = MathsStorage::Float4A16(10000.0f, 10000.0f, 10000.0f, 1.0f);
				pBlock->m_vMax = MathsStorage::Float4A16(-10000.0f, -10000.0f, -10000.0f, 1.0f);
				// set centre of box and large size so block doesn't get culled before it has any particles in it
				pBlock->m_vCentre = MathsStorage::Float4A16(pBlock->m_iX * m_Settings.fBlockSize + m_Settings.fBlockSize * 0.5f, 0.0f, pBlock->m_iZ * m_Settings.fBlockSize + m_Settings.fBlockSize * 0.5f, 1.0f);
				pBlock->m_vSize = MathsStorage::Float4A16(10000.0f, 10000.0f, 10000.0f, 10000.0f);
				// make visible so at least a couple of particles get spawned into it allowing it to be culled
				pBlock->m_bVisible = true;

				m_GamePolicy.CreateBlock(pBlock, m_Settings.fBlockSize);

				bCreated = true;
			}
		}
	}
}

} // namespace Rain
} // namespace Effect

#endif // EFFECT_RAIN_SPLASH_H
