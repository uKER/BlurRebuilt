#include "CrowdSystemPch.h"

#include <crowd_system/effects/CrowdCameraFlashes.h>
#include <crowd_system/CrowdVisibleBlockData.h>
#include <crowd_system/CrowdRandom.h>

#include <crowd_system/CrowdBlockRef.h>
#include <game_resource/CrowdData.h>

#include <crowd_system/CrowdBlockRef.h>
#include <crowd_system/CrowdInstance.h>
#include <crowd_system/CrowdCharacterRef.h>

#include <Dynamic/Particle/Event.h>
#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Particle/EventInstanceHandle.h>

#include <system/Perf.h>

namespace AnimatedCrowd
{

Debug::Int		g_CameraFlashMinBlockPriority("GameSys.CrowdSystem.Effects.CameraFlashes.MinBlockPriority", 10, Debug::Range(0, (CROWD_BLOCK_CACHE_BUCKET_COUNT - 1)));
Debug::Int		g_CameraFlashMaxBlockPriority("GameSys.CrowdSystem.Effects.CameraFlashes.MaxBlockPriority", 25, Debug::Range(0, CROWD_BLOCK_CACHE_BUCKET_COUNT));
Debug::Float	g_CameraFlashBlockFalloffLerp("GameSys.CrowdSystem.Effects.CameraFlashes.BlockFalloffLerp", 0.f, Debug::Range(0.f, 1.f));
Debug::Int		g_CameraFlashBlockFalloffPower("GameSys.CrowdSystem.Effects.CameraFlashes.BlockFalloffPower", 10, Debug::Range(1, 12));
Debug::Int		g_CameraFlashBlockLowerCount("GameSys.CrowdSystem.Effects.CameraFlashes.BlockLowerCount", 2, Debug::Range(1, 10));
Debug::Int		g_CameraFlashBlockUpperCount("GameSys.CrowdSystem.Effects.CameraFlashes.BlockUpperCount", 40, Debug::Range(10, 60));
Debug::Float	g_CameraFlashXZOffset("GameSys.CrowdSystem.Effects.CameraFlashes.XZOffset", 0.4f, Debug::Range(-2.f, 2.f), Debug::Step(0.1f));
Debug::Float	g_CameraFlashYOffset("GameSys.CrowdSystem.Effects.CameraFlashes.YOffset", 1.5f, Debug::Range(-2.5f, 2.5f), Debug::Step(0.01f));

C_CrowdCameraFlashes::C_CrowdCameraFlashes(DynamicEffect::Particle::C_Event* pEvent)
	: m_pEvent(pEvent),
	m_FirstFree(0),
	m_SpawnRate(0),
	m_WaitTime(0.f),
	m_SpawnTime(0.f)
{
	m_InstanceBits.Resize(CROWD_MAX_CAMERA_FLASHES);
	m_InstanceBits.ClearBits();
	m_EnabledBehaviours.ClearBits();
}

C_CrowdCameraFlashes::~C_CrowdCameraFlashes()
{
	m_pEvent = NULL;
}

void C_CrowdCameraFlashes::Enable(const U32 spawnRate, const F32 effectWaitTime)
{
	m_SpawnRate = spawnRate;
	m_WaitTime = effectWaitTime;
}

void C_CrowdCameraFlashes::Disable()
{
	m_pEvent = NULL;
	m_SpawnTime = 0.f;
}

void C_CrowdCameraFlashes::EnableBehaviour(const S32 behaviourIndex)
{
	if (behaviourIndex >= 0)
	{
		BIZ_ASSERT(behaviourIndex < 128); // can be increased if needed

		m_EnabledBehaviours.SetBit(behaviourIndex);
	}
}

void C_CrowdCameraFlashes::Update(const F32 deltaTime, const C_CrowdVisibleBlockData& visibleBlockData, const GameResource::C_CrowdData* pCrowdData)
{
	if (!m_pEvent || !pCrowdData || !m_EnabledBehaviours.AnyBitsSet())
		return;

	const U32 numVisibleBlocks = visibleBlockData.GetNumVisibleBlocks();
	if (!numVisibleBlocks)
		return;

	S32 blockCountDelta = g_CameraFlashBlockUpperCount - g_CameraFlashBlockLowerCount;
	if (blockCountDelta <= 0)
		return;

	F32 blockCountFactor = Maths::Clamp<S32>(Maths::Min<S32>(static_cast<S32>(numVisibleBlocks), g_CameraFlashBlockUpperCount.Value()) - g_CameraFlashBlockLowerCount, 0, blockCountDelta) / (F32)blockCountDelta;

	m_SpawnTime += deltaTime;

	U32 numFlashes = (U32)::floorf(((F32)m_SpawnRate * blockCountFactor * m_SpawnTime) + 0.5f);
	if (!numFlashes)
		return;

	m_SpawnTime = 0.f;

	Maths::HVec3 verticalOffset(0.f, g_CameraFlashYOffset.Value(), 0.f);

	DynamicEffect::C_Node node;

	U32 groupSize = 1;
	if (numVisibleBlocks > numFlashes)
		groupSize = numVisibleBlocks / numFlashes;

	for (U32 iGroup = 0, groupCount = numFlashes; iGroup < groupCount; ++iGroup)
	{
		if (m_Nodes.Size() == m_Nodes.Capacity())
			break;

		if (m_FirstFree >= static_cast<U32>(m_InstanceBits.Size()))
			break;

		const U32 lowerIndex = iGroup * groupSize;
		const U32 upperIndex = Maths::Min<U32>(lowerIndex + groupSize, numVisibleBlocks);// - 1;
		if ((lowerIndex < numVisibleBlocks) && (lowerIndex < upperIndex))
		{
			U32 randomBlock = C_Random::Instance().RandomU32(lowerIndex, upperIndex);

			BIZ_ASSERT(randomBlock < visibleBlockData.GetNumVisibleBlocks());

			const C_CrowdVisibleBlock& visibleBlock = visibleBlockData.GetVisibleBlock(randomBlock);

			if (visibleBlock.GetPriority() >= static_cast<U32>(g_CameraFlashMinBlockPriority.Value()) &&
				(visibleBlock.GetPriority() < Maths::Max<U32>(g_CameraFlashMinBlockPriority.Value(), g_CameraFlashMaxBlockPriority.Value())))
			{
				const C_CrowdBlockRef& blockRef = pCrowdData->GetCrowdBlockRef(visibleBlock.GetBlockRefIndex());

				if (blockRef.GetNumInstances())
				{
					F32 retryPriority = visibleBlock.GetPriority() / (F32)CROWD_BLOCK_CACHE_BUCKET_COUNT;
					retryPriority = ::powf(Maths::Lerp(1.f - retryPriority, retryPriority, Maths::Clamp<F32>(g_CameraFlashBlockFalloffLerp, 0.f, 1.f)), (F32)g_CameraFlashBlockFalloffPower);

					if (AnimatedCrowd::C_Random::Instance().RandomF32() >= retryPriority)
					{
						U32 randomInstance = blockRef.GetInstanceIndex(C_Random::Instance().RandomU32(blockRef.GetNumInstances()));

						Btl::OpenHashSet<U32>::ConstIterator iter = m_InstanceHashSet.Find(randomInstance);

						if (iter == m_InstanceHashSet.End())
						{
							const C_CrowdInstance& instance = pCrowdData->GetCrowdInstance(randomInstance);
							U8 behaviourIndex = pCrowdData->GetCrowdCharacterRef(instance.GetCharacterIndex());

							if (m_EnabledBehaviours[behaviourIndex])
							{
								node.SetPosition(instance.GetPosition() + (instance.GetDirection() * g_CameraFlashXZOffset) + verticalOffset);
								m_Nodes.PushBack(node);

								m_InstanceHashSet.Insert(randomInstance);
								m_Instances[m_FirstFree].Set(randomInstance, m_WaitTime);
								m_InstanceBits.SetBit(m_FirstFree);
								UpdateFirstFree();
							}
						}
					}
				}
			}
		}
	}

	for (U32 i = 0, count = static_cast<U32>(m_Instances.Size()); i < count; ++i)
	{
		m_Instances[i].Update(deltaTime);
		if (m_Instances[i].Finished() && m_InstanceBits.GetBitValue(i))
		{
			m_InstanceBits.ClearBit(i);
			m_InstanceHashSet.Erase(m_Instances[i].GetInstanceIndex());
			m_FirstFree = i;
		}
	}

	for (U32 i = 0, count = m_Nodes.Size(); i < count; ++i)
		m_pEvent->Create(m_Nodes[i]);

	m_Nodes.Clear();
}

const CrowdEffectSystemType C_CrowdCameraFlashes::GetType()
{
	return E_CROWD_EFFECT_SYSTEM_CAMERA_FLASHES;
}

void C_CrowdCameraFlashes::UpdateFirstFree()
{
	m_FirstFree = -1;
	for (S32 i = 0, count = m_InstanceBits.Size(); i < count; ++i)
	{
		if (!m_InstanceBits.GetBitValue(i))
		{
			m_FirstFree = i;
			break;
		}
	}
}

}; // namespace AnimatedCrowd
