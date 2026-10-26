#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/pvs/pvs.h>

#include <data/DataTypes.h>
#include <data/CrowdData.h>

F32 GetDistanceSqrFromAABBToAABB(const Maths::C_AxisBox& a, const Maths::C_AxisBox& b)
{
	F32 distSqr = 0.f;

	if (b.m_Max.X() < a.m_Min.X())
    {
        distSqr += (a.m_Min.X() - b.m_Max.X()) * (a.m_Min.X() - b.m_Max.X());
    }
    else if (a.m_Max.X() < b.m_Min.X())
    {
        distSqr += (b.m_Min.X() - a.m_Max.X()) * (b.m_Min.X() - a.m_Max.X());
    };

    if (b.m_Max.Y() < a.m_Min.Y())
    {
        distSqr += (a.m_Min.Y() - b.m_Max.Y()) * (a.m_Min.Y() - b.m_Max.Y());
    }
    else if (a.m_Max.Y() < b.m_Min.Y())
    {
        distSqr += (b.m_Min.Y() - a.m_Max.Y()) * (b.m_Min.Y() - a.m_Max.Y());
    };

    if (b.m_Max.Z() < a.m_Min.Z())
    {
        distSqr += (a.m_Min.Z() - b.m_Max.Z()) * (a.m_Min.Z() - b.m_Max.Z());
    }
    else if (a.m_Max.Z() < b.m_Min.Z())
    {
        distSqr += (b.m_Min.Z() - a.m_Max.Z()) * (b.m_Min.Z() - a.m_Max.Z());
    };

    return distSqr;
}

F32 GetDistanceSqrFromAABBToPoint(const Maths::C_AxisBox& axisBox, const Maths::HVec3& point)
{
	F32 distSqr = 0.f;

	for (U32 i = 0; i < 3; ++i)
	{
		F32 v = point[i];
		
		if (v < axisBox.Min()[i]) distSqr += (axisBox.Min()[i] - v) * (axisBox.Min()[i] - v);
		if (v > axisBox.Max()[i]) distSqr += (v - axisBox.Max()[i]) * (v - axisBox.Max()[i]);
	}

	return distSqr;
}

Maths::HVec3 GetNearestPointOnAABBToPoint(const Maths::C_AxisBox& axisBox, const Maths::HVec3& point)
{
	Maths::HVec3 pointOnAABB;

	for (U32 i = 0; i < 3; ++i)
	{
		F32 v = point[i];
		if (v < axisBox.Min()[i]) v = axisBox.Min()[i];
		if (v > axisBox.Max()[i]) v = axisBox.Max()[i];
		pointOnAABB[i] = v;
	}

	return pointOnAABB;
}

void GetNearestPVSCells(const Maths::C_AxisBox& axisBox, const Pvs::Build::PvsData& PVSData,
						Btl::Vector<Btl::Pair<U32, F32>, Btl::VectorParameters_Static<16> >& nearestPVSCells)
{
	nearestPVSCells.Clear();
	
	for (U32 i = 0, count = static_cast<U32>(PVSData.GetNumPvsCells()); i < count; ++i)
	{
		const MathsStorage::Matrix44& cellMatrix = PVSData.GetCellMatrix(i);

		Maths::HMat43 cell(	Maths::HVec3(cellMatrix.rx.x, cellMatrix.rx.y, cellMatrix.rx.z),
							Maths::HVec3(cellMatrix.ry.x, cellMatrix.ry.y, cellMatrix.ry.z),
							Maths::HVec3(cellMatrix.rz.x, cellMatrix.rz.y, cellMatrix.rz.z),
							Maths::HVec3(cellMatrix.rw.x, cellMatrix.rw.y, cellMatrix.rw.z));

		Maths::C_AxisBox cellAxisBox(Maths::HVec3(-1.f, -1.f, -1.f) * cell, Maths::HVec3(1.f, 1.f, 1.f) * cell);

		F32 fDistSqr = GetDistanceSqrFromAABBToAABB(axisBox, cellAxisBox);

		if (nearestPVSCells.Size() < 16)
		{
			nearestPVSCells.PushBack(Btl::Pair<U32, F32>(i, fDistSqr));
		}
		else
		{
			// Find furthest away cell
			U32 index = 0;
			for (U32 j = 1; j < static_cast<U32>(nearestPVSCells.Size()); ++j)
			{
				if (nearestPVSCells[j].second < nearestPVSCells[index].second)
				{
					index = j;
				}
			}

			if (fDistSqr < nearestPVSCells[index].second)
			{
				nearestPVSCells[index].first = i;
				nearestPVSCells[index].second = fDistSqr;
			}
		}
	}
}

Maths::HVec3 FindNearestSamplePoint(const Maths::C_AxisBox& axisBox, const Pvs::Build::PvsData& PVSData, Maths::HVec3& outVector)
{
	Maths::HVec3 nearestSamplePointPosition(0.f, 0.f, 0.f);

	F32 distToCellSqr = FLT_MAX;
	
	Btl::Vector<Btl::Pair<U32, F32>, Btl::VectorParameters_Static<16> > nearestPVSCells;

	GetNearestPVSCells(axisBox, PVSData, nearestPVSCells);

	F32 distToSamplePointSqr = FLT_MAX;

	for (U32 iCell = 0; iCell < static_cast<U32>(nearestPVSCells.Size()); ++iCell)
	{
		const MathsStorage::Matrix44& cellMatrix = PVSData.GetCellMatrix(nearestPVSCells[iCell].first);

		Maths::HMat43 cell(	Maths::HVec3(cellMatrix.rx.x, cellMatrix.rx.y, cellMatrix.rx.z),
							Maths::HVec3(cellMatrix.ry.x, cellMatrix.ry.y, cellMatrix.ry.z),
							Maths::HVec3(cellMatrix.rz.x, cellMatrix.rz.y, cellMatrix.rz.z),
							Maths::HVec3(cellMatrix.rw.x, cellMatrix.rw.y, cellMatrix.rw.z));

		Maths::C_AxisBox cellAxisBox(Maths::HVec3(-1.f, -1.f, -1.f) * cell, Maths::HVec3(1.f, 1.f, 1.f) * cell);

		if (nearestPVSCells[iCell].second <= AnimatedCrowd::CROWD_BLOCK_HIGHDETAIL_DISTSQR)
		{
			for (U32 i = 0, count = PVSData.GetNumSamplePoints(nearestPVSCells[iCell].first); i < count; ++i)
			{
				if (PVSData.IsSamplePointDirectional(nearestPVSCells[iCell].first, i) || PVSData.IsSamplePointOptional(nearestPVSCells[iCell].first, i))
					continue;

				Maths::HVec3 samplePointPosition = PVSData.GetSamplePointPosition(nearestPVSCells[iCell].first, i);

				F32 fDistToSamplePointSqr = GetDistanceSqrFromAABBToPoint(axisBox, samplePointPosition);

				if (fDistToSamplePointSqr < distToSamplePointSqr)
				{
					// Dear Diary, Jackpot!

					Maths::HVec3 nearestPointOnAABBToPoint = GetNearestPointOnAABBToPoint(axisBox, samplePointPosition);

					outVector = (samplePointPosition - nearestPointOnAABBToPoint);

					distToSamplePointSqr = fDistToSamplePointSqr;

					nearestSamplePointPosition = samplePointPosition;
				}
			}
		}
	}

	return nearestSamplePointPosition;
}

C_DataCrowd::C_DataCrowd()
	: m_RandomSeed(0)
{
}

C_DataCrowd::~C_DataCrowd()
{
}

const Util::C_String C_DataCrowd::GetName() const
{
	return Util::C_String("C_DataCrowd");
}

const U32 C_DataCrowd::GetCrowdBehavioursSize() const
{
	return static_cast<U32>(m_BehaviourNameHashes.Size());
}

const U32 C_DataCrowd::GetCrowdCharactersSize() const
{
	return static_cast<U32>(m_Characters.Size());
}

const U32 C_DataCrowd::GetCrowdInstancesSize() const
{
	return static_cast<U32>(m_Instances.Size());
}

void C_DataCrowd::AddCrowdBehaviour(const U32 behaviourNameHash)
{
	m_BehaviourNameHashes.PushBack(behaviourNameHash);
}

void C_DataCrowd::AddCrowdCharacter(const AnimatedCrowd::C_CrowdCharacterRef& character)
{
	m_Characters.PushBack(character);
}

void C_DataCrowd::AddCrowdInstance(const AnimatedCrowd::C_CrowdInstance &instance)
{
	m_Instances.PushBack(instance);
}

//void C_DataCrowd::AddCrowdInstanceSHNode(const U32 instanceIndex, const LuxSamplePoint& samplePoint)
void C_DataCrowd::AddCrowdInstanceSHNode(const AnimatedCrowd::C_CrowdSHNodeData& shNodeData)
{
	m_InstanceSHNodes.PushBack(shNodeData);
}

const AnimatedCrowd::C_CrowdCharacterRef* C_DataCrowd::GetCrowdCharacter(const U32 index) const
{
	if (index < static_cast<U32>(m_Characters.Size()))
		return &m_Characters[index];
	return NULL;
}

void C_DataCrowd::BuildBlocks(const Pvs::Build::PvsData* pPVSData)
{
	if (m_Instances.Size() < 1)
		return;

	Btl::Vector<S32> instances;

	for (S32 i = 0; i < m_Instances.Size(); ++i)
	{
		instances.PushBack(i);
	}

	Btl::Vector<S32>::Iterator iter = instances.Begin();

	while (instances.Size() > 0)
	{
		S32 iInstance = *iter;

		S32 route = m_Instances[iInstance].GetRoute();

		if (m_Blocks.Size() < 1)
		{
			AnimatedCrowd::C_CrowdBlockRef block;

			block.Create(m_Blocks.Size(), route, iInstance, &m_Characters, &m_Instances);

			m_Blocks.PushBack(block);

			instances.Erase(iter);
		}
		else
		{
			S32 nearestBlock = -1;
			
			F32 distanceToNearestBlockSqr = FLT_MAX;

			for (S32 iBlock = 0; iBlock < m_Blocks.Size(); ++iBlock)
			{
				F32 distanceToBlockSqr = GetDistanceSqrFromAABBToAABB(m_Blocks[iBlock].GetAxisBox(),
					m_Characters[m_Instances[iInstance].GetCharacterIndex()].GetAxisBox(m_Instances[iInstance].GetPosition()));

				if (distanceToBlockSqr < distanceToNearestBlockSqr && m_Blocks[iBlock].GetRoute() == route)
				{
					nearestBlock = iBlock;
					distanceToNearestBlockSqr = distanceToBlockSqr;
				}
			}

			if (nearestBlock >= 0 && distanceToNearestBlockSqr <= AnimatedCrowd::CROWD_MAX_BLOCK_GROWTH_DISTANCE_SQR
				&& m_Blocks[nearestBlock].CheckBounds(m_Characters[m_Instances[iInstance].GetCharacterIndex()].GetAxisBox(m_Instances[iInstance].GetPosition())))
			{
				U32 outIndex = 0;

				AnimatedCrowd::C_CrowdBlockRef::InstanceInsertResult insertResult = m_Blocks[nearestBlock].AddInstanceIndex(iInstance, distanceToNearestBlockSqr, outIndex);
				
				if (insertResult == AnimatedCrowd::C_CrowdBlockRef::E_RESULT_ADDED_NO_REMOVE)
				{
					instances.Erase(iter);
				}
				else
				{
					if (insertResult == AnimatedCrowd::C_CrowdBlockRef::E_RESULT_ADDED_REMOVE)
					{
						instances.PushBack(outIndex);

						instances.Erase(iter);
					}
					else
					{
						AnimatedCrowd::C_CrowdBlockRef block;

						block.Create(m_Blocks.Size(), route, iInstance, &m_Characters, &m_Instances);

						m_Blocks.PushBack(block);

						instances.Erase(iter);
					}
				}
			}
			else
			{
				AnimatedCrowd::C_CrowdBlockRef block;

				block.Create(m_Blocks.Size(), route, iInstance, &m_Characters, &m_Instances);

				m_Blocks.PushBack(block);

				instances.Erase(iter);
			}
		}
	}

	if (pPVSData)
	{
		for (S32 i = 0, count = m_Blocks.Size(); i < count; ++i)
		{
			Maths::HVec3 v(FLT_MAX, FLT_MAX, FLT_MAX);

			Maths::HVec3 nearestSamplePoint = FindNearestSamplePoint(m_Blocks[i].GetAxisBox(), *pPVSData, v);

			m_Blocks[i].SetBlockType(AnimatedCrowd::E_BLOCK_TYPE_LOW_DETAIL);
			
			if (::fabsf(v.Y()) <= AnimatedCrowd::CROWD_BLOCK_HIGHDETAIL_Y_DIST)
			{
				F32 xz = ::sqrtf((v.X() * v.X()) + (v.Z() * v.Z()));

				if (xz <= AnimatedCrowd::CROWD_BLOCK_HIGHDETAIL_XZ_DIST)
				{
					m_Blocks[i].SetBlockType(AnimatedCrowd::E_BLOCK_TYPE_HIGH_DETAIL);
				}
			}
		}
	}
}

void C_DataCrowd::SortBlocks(const S32 routeCount)
{
	std::sort(m_Blocks.Begin(), m_Blocks.End());

	// As blocks are now in route order, the sum of how many blocks of each route there are
	// can be used are to offset into the block vector
	m_RouteBlockAndInstanceCounts.Resize(routeCount + 1);
	
	// zero out the counts
	for (S32 i = 0, count = m_RouteBlockAndInstanceCounts.Size(); i < count; ++i)
	{
		m_RouteBlockAndInstanceCounts[i].first = 0;
		m_RouteBlockAndInstanceCounts[i].second = 0;
	}

	// then start counting :)
	for (S32 i = 0, count = m_Blocks.Size(); i < count; ++i)
	{
		m_RouteBlockAndInstanceCounts[m_Blocks[i].GetRoute() + 1].first++;
		m_RouteBlockAndInstanceCounts[m_Blocks[i].GetRoute() + 1].second += m_Blocks[i].GetNumInstances();
	}
}

void C_DataCrowd::ConstructBlockGrid(const F32 cellSize, const F32 cellRange)
{
	Aftk::C_Bounds crowdBounds;

	if (m_Blocks.Size())
	{
		Maths::HVec3 boundsPadding(cellSize + (cellRange * 2));

		Maths::HVec3 worldMin = m_Blocks[0].GetAxisBox().Min();
		Maths::HVec3 worldMax = m_Blocks[0].GetAxisBox().Max();

		for (U32 i = 1, count = static_cast<U32>(m_Blocks.Size()); i < count; ++i)
		{
			const Maths::HVec3& blockMin = m_Blocks[i].GetAxisBox().Min();
			const Maths::HVec3& blockMax = m_Blocks[i].GetAxisBox().Max();

			worldMin = Maths::Min(worldMin, blockMin);
			worldMin = Maths::Min(worldMin, blockMax);

			worldMax = Maths::Max(worldMax, blockMin);
			worldMax = Maths::Max(worldMax, blockMax);
		}

		worldMin -= boundsPadding;
		worldMax += boundsPadding;

		crowdBounds.AddVertex(Aftk::Position3(worldMin.X(), worldMin.Y(), worldMin.Z()));
		crowdBounds.AddVertex(Aftk::Position3(worldMax.X(), worldMax.Y(), worldMax.Z()));
	}

	m_BlockGrid.Build(&m_Blocks, crowdBounds, cellSize, cellRange);
}

bool C_DataCrowd::Export( Sys::I_File* pFile, const Util::E_Platform platform ) const
{
	Sys::C_FileBlock crowdBlock(pFile, "Crowd");

	Sys::BinarySerialiserAutoEndian ar( crowdBlock );

	ar << (S32)AnimatedCrowd::E_CrowdFileVersion_Current;

	ar << m_RandomSeed;

	S32 behaviourListSize = m_BehaviourNameHashes.Size();
	ar << behaviourListSize;

	for (S32 i = 0; i < behaviourListSize; ++i)
	{
		ar << m_BehaviourNameHashes[i];
	}

	S32 characterListSize = m_Characters.Size();
	ar << characterListSize;

	for (S32 i = 0; i < characterListSize; ++i)
	{
		const AnimatedCrowd::C_CrowdCharacterRef& character = m_Characters[i];

		ar << character;
	}

	S32 instanceListSize = m_Instances.Size();
	ar << instanceListSize;

	for (S32 i = 0; i < instanceListSize; ++i)
	{
		const AnimatedCrowd::C_CrowdInstance& instance = m_Instances[i];

		ar << instance;
	}

	S32 blockListSize = m_Blocks.Size();
	ar << blockListSize;

	for (S32 i = 0; i < blockListSize; ++i)
	{
		const AnimatedCrowd::C_CrowdBlockRef& block = m_Blocks[i];

		ar << block;
	}

	S32 numRoutes = m_RouteBlockAndInstanceCounts.Size();
	ar << numRoutes;

	for (S32 i = 0; i < numRoutes; ++i)
	{
		ar << (S16)(m_RouteBlockAndInstanceCounts[i].first);
		ar << (S16)(m_RouteBlockAndInstanceCounts[i].second);
	}

	ar << m_BlockGrid;

	S32 instanceSHNodeListSize = m_InstanceSHNodes.Size();
	ar << instanceSHNodeListSize;

	for (S32 i = 0; i < instanceSHNodeListSize; ++i)
	{
		ar << m_InstanceSHNodes[i];
	}

	return true;
}

const bool C_DataCrowd::ContainsData() const
{
	// Check if data contains any instances
	if (m_Instances.Size())
		return true;

	return false;
}
