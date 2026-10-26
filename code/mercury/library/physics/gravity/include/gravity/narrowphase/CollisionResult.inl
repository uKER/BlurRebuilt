#ifndef	COLLISIONRESULT_INL
#define COLLISIONRESULT_INL

#include "gravity/narrowphase/CollisionResult.h"
#include "gravity/sharedphysics/Collider.h"
#include "gravity/sharedphysics/ContactStream.h"
#include "gravity/sharedphysics/ContactStream.inl"
#include "Core/Utility/MemCpy.h"
#include "gravity/cloths/cloth.h"

namespace Collision
{
using namespace Maths;

//Debug::Float g_dbPatchNormalThreshold("Physics.ContactReduction.PatchNormalThreshold", 0.01f, Debug::Range(0.f, 1.f));
//Debug::Float g_dbPointThreshold("Physics.ContactReduction.ContactPointThreshold", 0.01f, Debug::Range(0.f, 1.f));
//Debug::Int g_dbKMeansMaxIterations("Physics.ContactReduction.KMeansMaxIterations", 5, Debug::Range(0, 100));
const static F32 g_dbPatchNormalThreshold(0.01f);
const static F32 g_dbPointThreshold(0.01f);

//template <int MaxPatches, int MaxContacts>
//void C_CollisionResult<MaxPatches, MaxContacts>::AddPatch(C_ContactPatch<MaxContacts>& patch)
//{
//	const Maths::HF32 closeNormal = SharedPhysics::Constants::CloseNormal();
//	const Maths::HF32 closeContact = SharedPhysics::Constants::CloseContact();
//	U32 incr = 0;
//	C_ContactPatch<MaxContacts>* patchResult(NULL);
//	if(m_NumContactPatches < MaxPatches)
//	{
//		patchResult = &m_ContactPatches[m_NumContactPatches];
//		patchResult->m_NumContactPoints = 0;
//		patchResult->m_Normal = patch.m_Normal;
//		incr = 1;
//	}
//	for(U32 p = 0; p < m_NumContactPatches; ++p)
//	{
//		const Maths::HF32 dot(m_ContactPatches[p].m_Normal | patch.m_Normal);
//		if(dot > closeNormal)
//		{
//			patchResult = &m_ContactPatches[p];
//			incr = 0;
//			break;
//		}
//	}
//	if(!patchResult)
//		return;
//	m_NumContactPatches += incr;
//	for(U32 b=0; b<patch.m_NumContactPoints; ++b)
//	{
//		if(patchResult->m_NumContactPoints >= MaxContacts)
//			break;
//		for(U32 c = 0; c < patchResult->m_NumContactPoints; ++c)
//		{
//			const Maths::HVec3 dif(patchResult->m_ContactPoints[c].m_ContactPoint - patch.m_ContactPoints[b].m_ContactPoint);
//			const Maths::HF32 d(dif | dif);
//			if(d < closeContact)
//			{
//				break;
//			}
//		}
//		patchResult->m_ContactPoints[patchResult->m_NumContactPoints++]=patch.m_ContactPoints[b];
//	}
//}

template <U32 MaxContacts>
void RemoveDuplicateVertices(Collision::C_ContactPatch<MaxContacts>& patch)
{
	const Maths::HF32 closeContact(SharedPhysics::Constants::CloseContact());
	S32 numContacts((S32)patch.m_NumContactPoints);
	S32 numContactsMinus1(numContacts-1);
	for(S32 b = 0; b < numContactsMinus1; ++b)
	{
		const Maths::HVec3 pointA(patch.m_ContactPoints[b].m_ContactPoint);
		for(S32 c = b+1; c < numContacts; ++c)
		{
			const Maths::HVec3 pointB(patch.m_ContactPoints[c].m_ContactPoint);
			const Maths::HVec3 dif(pointA - pointB);
			const Maths::HF32 dis(dif | dif);
			if(dis < closeContact)
			{
				//Remove this contact...
				--numContacts;
				--numContactsMinus1;
				for(S32 d = c; d < numContacts; ++d)
				{
					patch.m_ContactPoints[d] = patch.m_ContactPoints[d+1];
				}
				--c;
			}
		}
	}
}

template <int MaxContacts>
Maths::HVec3 PickDeepestPenetration(C_ContactPatch<MaxContacts>& patch)
{
	Maths::HVec3 bestPoint(patch.m_ContactPoints[0].m_ContactPoint);
	Maths::HF32 bestPenetrationDepth(Maths::Load<Maths::HF32>(&patch.m_ContactPoints[0].m_PenetrationDepth));

	for(U32 a = 1; a < patch.m_NumContactPoints; ++a)
	{
		const Maths::HF32 penDepth(Maths::Load<Maths::HF32>(&patch.m_ContactPoints[a].m_PenetrationDepth));
		const Maths::Scalar_Bool b(bestPenetrationDepth.LT(penDepth));
		bestPoint = Maths::Select(b, patch.m_ContactPoints[a].m_ContactPoint, bestPoint);
		bestPenetrationDepth = Maths::Select(b, penDepth, bestPenetrationDepth);
	}
	return bestPoint;
}

template <int MaxContacts>
Maths::HVec3 PickSupportPoint(const C_Cluster<C_ContactPoint*, MaxContacts> * const __restrict clusters, 
													C_ContactPatch<MaxContacts>& patch, const U32 numExistingClusters)
{
	Maths::HVec3 bestPoint(Maths::UNDEFINED);
	//Maths::HF32 smallDist(Maths::Constants<Maths::HF32>::Max());
	//Maths::HF32 bigDist(Maths::Constants<Maths::HF32>::Zero());
	const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
	Maths::HF32 bestDist(Maths::Constants<Maths::HF32>::Zero());	

	for(U32 a = 0; a < patch.m_NumContactPoints; ++a)
	{
		Maths::HF32 newSmallDist(Maths::Constants<Maths::HF32>::Max());
		Maths::HF32 newBigDist(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HVec3 point(patch.m_ContactPoints[a].m_ContactPoint);
		
		for(U32 b = 0; b < numExistingClusters; ++b)
		{
			const Maths::HVec3 dif(point - clusters[b].m_Centroid);
			const Maths::HF32 dis(dif | dif);
			newSmallDist = Maths::Min(newSmallDist, dis);
			newBigDist = Maths::Max(newBigDist, dis);
		}
		const Maths::HF32 newDist((two * newSmallDist) + newBigDist);
		const Scalar_Bool bb(newDist.GT(bestDist));
		bestPoint = Select(bb, point, bestPoint);
		bestDist = Select(bb, newDist, bestDist);
	}
	return bestPoint;
}

template <int MaxPatches, int MaxContacts>
Maths::HVec3 PickBestNormal(C_CollisionResult<MaxPatches, MaxContacts>& results)
{
	C_Normal bestNormal(results.m_ContactPatches[0].m_Normal);
	const Maths::HVec3 positionA(results.m_pObjectA->m_pShape->Position());
	Maths::HF32 bestProjection((results.m_ContactPatches[0].m_ContactPoints[0].m_ContactPoint - positionA) | results.m_ContactPatches[0].m_Normal);
	for(U32 a = 1; a < results.m_ContactPatches[0].m_NumContactPoints; ++a)
	{
		bestProjection = Maths::Max((results.m_ContactPatches[0].m_ContactPoints[a].m_ContactPoint - positionA) | results.m_ContactPatches[0].m_Normal, bestProjection);
	}

	for(U32 a = 1; a < results.m_NumContactPatches; ++a)
	{
		Maths::HF32 newBestProj(-Maths::Constants<Maths::HF32>::Max());
		const C_ContactPatch<MaxContacts>& patch(results.m_ContactPatches[a]);
		for(U32 b = 0; b < patch.m_NumContactPoints; ++b)
		{
			const C_ContactPoint& point(patch.m_ContactPoints[b]);
			newBestProj = Maths::Max(newBestProj, (point.m_ContactPoint - positionA) | patch.m_Normal);
		}

		const Maths::Scalar_Bool bl(newBestProj.GT(bestProjection));
		bestNormal = Select(bl, patch.m_Normal, bestNormal);
		bestProjection = Select(bl, newBestProj, bestProjection);
	}
	return bestNormal;
}

template <int MaxPatches, int MaxContacts>
Maths::HVec3 PickSupportNormal(const C_Cluster<C_ContactPatch<MaxContacts>*, MaxPatches>* const __restrict clusters, 
													   C_CollisionResult<MaxPatches, MaxContacts>& results, const U32 numExistingClusters)
{
	//Maths::HF32 smallDist(Maths::Constants<Maths::HF32>::Max());
	//Maths::HF32 bigDist(-Maths::Constants<Maths::HF32>::Max());
	const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
	Maths::HF32 bestDist(Maths::Constants<Maths::HF32>::Max());	
	//Loop through all normals and find the normal with the next-best projection onto
	C_Normal bestNormal(Maths::UNDEFINED);
	const Maths::HVec3 positionA(results.m_pObjectA->m_pShape->Position());
	Maths::HF32 bestProjection(-Maths::Constants<Maths::HF32>::Max());
	for(U32 a = 0; a < results.m_NumContactPatches; ++a)
	{
		Maths::HF32 newSmallDist(Maths::Constants<Maths::HF32>::Max());
		Maths::HF32 newBigDist(-Maths::Constants<Maths::HF32>::Max());
		const C_ContactPatch<MaxContacts>& patch(results.m_ContactPatches[a]);

		for(U32 b = 0; b < numExistingClusters; ++b)
		{
			const Maths::HF32 dif(Maths::Abs(patch.m_Normal | clusters[b].m_Centroid));
			newSmallDist = Maths::Min(dif, newSmallDist);
			newBigDist = Maths::Max(dif, newBigDist);
		}

		const Maths::HF32 score((two * newBigDist) + newSmallDist);
		const Maths::Scalar_Bool bl(score.LT(bestDist));
		bestDist = Select(bl, score, bestDist);
		bestNormal = Select(bl, patch.m_Normal, bestNormal);
	}
	return bestNormal;
}

/*template <int MaxPatches, int MaxContacts>
NOALIAS FORCE_INLINE void EncodeContacts(const C_CollisionResult<MaxPatches, MaxContacts>& result, SharedPhysics::C_ContactDatabase& database)
{
	using namespace SharedPhysics;
	//Loop through and put all this stuff in the contact database...
	C_CollisionResultDB* pResult(database.m_SleepyManager.GetFreeCollisionResult());
	pResult->m_pNodeA = result.m_pObjectA->m_pNodePtr;
	pResult->m_pNodeB = result.m_pObjectB->m_pNodePtr;
	pResult->m_NumPatches = result.m_NumContactPatches;
	for(U32 a = 0; a < result.m_NumContactPatches; ++a)
	{
		C_ContactPatchDB* pPatch(database.m_SleepyManager.GetFreeCollisionPatch());
		const Collision::C_ContactPatch<MaxContacts>& patch(result.m_ContactPatches[a]);
		pPatch->m_NumContactPoints = patch.m_NumContactPoints;
		pPatch->m_Normal = patch.m_Normal;
		for(U32 b = 0; b < patch.m_NumContactPoints; ++b)
		{
			const Collision::C_ContactPoint& point(patch.m_ContactPoints[b]);
			C_ContactPointDB* pPoint(database.m_SleepyManager.GetFreeCollisionPoint());
			pPoint->m_ContactPoint = point.m_ContactPoint;
			//Warm start forces should be 0 - this code is only reached when an object is inserted in a sleeping state!
			pPoint->m_PenetrationDepth = point.m_PenetrationDepth;
			pPoint->m_HashValue = point.m_HashValue;
			pPoint->m_MaterialIndices = point.m_MaterialIndices;
			pPatch->AddContactPoint(pPoint);
		}
		pResult->AddContactPatch(pPatch);
	}
	database.AddSleepyContact(pResult);
}*/

//void EncodeContact(const C_ClothContact* pContacts, const U32 numContacts, SharedPhysics::C_ContactStream& stream, 
//				   SharedPhysics::C_CollisionResultDB* pCollider)
//{
//	const U32 maxContacts(32);
//	Spu::MemBaseMarker marker;
//	const U32 size = sizeof(C_ClothContactStorage) * (maxContacts + 1);
//#ifndef PLATFORM_SPU
//	ALIGN16 U8 tempData[size];
//#else
//	U8* tempData = (U8*)Spu::Alloc(size);
//#endif
//
//	U8* dataStream(tempData);
//	const U32 numContacts(Maths::Max(numContacts, MaxContacts));
//	const U32 dataSize = numContacts * sizeof(C_ClothContactStorage);
//	U8* writeData = stream.LockMemory(dataSize);
//	for(S32 a = 0; a < numContacts; ++a)
//	{
//		const C_ClothContact& contact(pContacts[a]);
//		SharedPhysics::C_ClothContactStorage* pResult((SharedPhysics::C_ClothContactStorage*)dataStream);
//		pResult->m_ValidData = E_ValidData;
//		pResult->m_pClothPtr = contact.m_pClothPtr;
//		pResult->m_pNodePtr = contact.m_pNodePtr;
//		Maths::Store(contact.m_Contact, &pResult->m_Contact);
//		Maths::Store(contact.m_Penetration, &pResult->m_Penetration);
//		Maths::Store(contact.m_Stiffness, &pResult->m_Stiffness);
//		Maths::Store(contact.m_EscapeVector, &pResult->m_EscapeVector);
//		pResult->m_Type = contact.m_Type;
//		pResult->m_Index = contact.m_Index;
//	}
//
//	U8* writeData = stream.LockMemory(dataSize);
//	Spu::Store(writeData, tempData, pResult->m_OffsetToEnd);
//	if(pCollider)
//	{
//		SharedPhysics::C_Collider* coll = (SharedPhysics::C_Collider*)Spu::FetchAlloc(pCollider, sizeof(SharedPhysics::C_Collider));
//		coll->m_StreamPtr = SharedPhysics::C_ConstContactStreamPtr(stream.m_pPPUPtr,
//			writeData);
//#ifdef PLATFORM_SPU
//		Spu::Store(pCollider, coll, sizeof(SharedPhysics::C_Collider));
//#endif
//	
//}

//pCollider in SPU memory already!
template <int MaxPatches, int MaxContacts>
void EncodeContacts(const C_CollisionResult<MaxPatches, MaxContacts>& result, SharedPhysics::C_ContactStream& stream,
		SharedPhysics::C_Collider* pCollider)
{
	Spu::MemBaseMarker marker;
	const U32 size = 2048;
#ifndef PLAFORMS_SPU
	ALIGN16 U8 tempData[size];
#else
	U8* tempData = (U8*)Spu::Alloc(size);
#endif
	U8* dataStream(tempData);//stream.m_pCurrentWritePoint);
	SharedPhysics::C_ContactStreamResult* pResult((SharedPhysics::C_ContactStreamResult*)dataStream);
	dataStream += sizeof(SharedPhysics::C_ContactStreamResult);
	//assert(dataStream < stream.m_pStreamEnd);
	pResult->m_NumElements = result.m_NumContactPatches;
	pResult->m_pNodeA = result.m_pObjectA->m_pNodePtr;
	pResult->m_pNodeB = result.m_pObjectB->m_pNodePtr;
	pResult->m_ValidData = SharedPhysics::E_ValidData;
	pResult->m_ObjectAMassScale = 1.f;
	pResult->m_ObjectBMassScale = 1.f;
	pResult->m_ObjectAInertiaScale = 1.f;
	pResult->m_ObjectBInertiaScale = 1.f;
	pResult->m_BaumgarteOverride = 0.f;

	pResult->m_pCollider = pCollider != NULL? pCollider->m_pPpuPtr : NULL;
	U32 collisionCount = 0;
	for(U32 a = 0; a < result.m_NumContactPatches; ++a)
	{
		const C_ContactPatch<MaxContacts> &patch(result.m_ContactPatches[a]);
		const U8* patchStart(dataStream);
		SharedPhysics::C_ContactStreamPatch* pPatch((SharedPhysics::C_ContactStreamPatch*)dataStream);
		dataStream += sizeof(SharedPhysics::C_ContactStreamPatch);
		//assert(dataStream < stream.m_pStreamEnd);
		Maths::Store<Maths::HVec3>(patch.m_Normal.m_V, &pPatch->m_Normal);
		pPatch->m_NumElements = patch.m_NumContactPoints;
		pPatch->m_ValidData = SharedPhysics::E_ValidData;
		for(U32 b = 0; b < patch.m_NumContactPoints; ++b)
		{
			const C_ContactPoint& point(patch.m_ContactPoints[b]);
			SharedPhysics::C_ContactStreamPoint* pPoint((SharedPhysics::C_ContactStreamPoint*)dataStream);
			dataStream += sizeof(SharedPhysics::C_ContactStreamPoint);
			//assert(dataStream < stream.m_pStreamEnd);
			pPoint->m_HashValue = point.m_HashValue;
			pPoint->m_PhysicsMaterialIndices = point.m_PhysicsMaterialIndices;
			pPoint->m_EffectMaterialIndices = point.m_EffectMaterialIndices;
			pPoint->m_ValidData = SharedPhysics::E_ValidData;
			Maths::Store<Maths::HVec3>(point.m_ContactPoint, &pPoint->m_ContactPoint);
			//Maths::Store<Maths::HF32>(point.m_PenetrationDepth, &pPoint->m_PenetrationDepth);
			pPoint->m_PenetrationDepth = point.m_PenetrationDepth;
			++collisionCount;
			//pPoint->m_WarmstartValues.x = pPoint->m_pConstraint->m_pConstraint->m_WarmstartValues.y = pPoint->m_WarmstartValues.z = 0.f;
		}
		pPatch->m_OffsetToEnd = dataStream - patchStart;
	}

	pResult->m_TotalContacts = (U8)collisionCount;

	//Final bit - write the data to the stream!!!!
	INTPTR addy((INTPTR)(dataStream + 15));	// [Blur2PC][x64] align the full address
	dataStream = (U8*)(addy & ~(INTPTR)0xf);
	pResult->m_OffsetToEnd = dataStream - &tempData[0];

#if 0 //def PLATFORM_SPU
	DebugBreak();
#endif
	U8* writeData = stream.LockMemory(pResult->m_OffsetToEnd);
	if(writeData)
	{
		Spu::Store(writeData, tempData, pResult->m_OffsetToEnd);
		if(pCollider)
		{
			pCollider->m_StreamPtr = SharedPhysics::C_ConstContactStreamPtr(&stream,
				writeData);
		}

	}
}


template <int MaxPatches, int MaxContacts>
void EncodeContactsSpu(const C_CollisionResult<MaxPatches, MaxContacts>& result, SharedPhysics::C_ContactStream& stream,
		SharedPhysics::C_Collider* pCollider)
{
	Spu::MemBaseMarker marker;
	const U32 size = 2048;
#ifndef PLAFORMS_SPU
	ALIGN16 U8 tempData[size];
#else
	U8* tempData = (U8*)Spu::Alloc(size);
#endif
	U8* dataStream(tempData);//stream.m_pCurrentWritePoint);
	SharedPhysics::C_ContactStreamResult* pResult((SharedPhysics::C_ContactStreamResult*)dataStream);
	dataStream += sizeof(SharedPhysics::C_ContactStreamResult);
	//assert(dataStream < stream.m_pStreamEnd);
	pResult->m_NumElements = result.m_NumContactPatches;
	pResult->m_pNodeA = result.m_pObjectA->m_pNodePtr;
	pResult->m_pNodeB = result.m_pObjectB->m_pNodePtr;
	pResult->m_ValidData = SharedPhysics::E_ValidData;
	pResult->m_ObjectAMassScale = 1.f;
	pResult->m_ObjectBMassScale = 1.f;
	pResult->m_ObjectAInertiaScale = 1.f;
	pResult->m_ObjectBInertiaScale = 1.f;
	pResult->m_BaumgarteOverride = 0.f;

	pResult->m_pCollider = pCollider != NULL? pCollider->m_pPpuPtr : NULL;
	U32 collisionCount = 0;
	for(U32 a = 0; a < result.m_NumContactPatches; ++a)
	{
		const C_ContactPatch<MaxContacts> &patch(result.m_ContactPatches[a]);
		const U8* patchStart(dataStream);
		SharedPhysics::C_ContactStreamPatch* pPatch((SharedPhysics::C_ContactStreamPatch*)dataStream);
		dataStream += sizeof(SharedPhysics::C_ContactStreamPatch);
		//assert(dataStream < stream.m_pStreamEnd);
		Maths::Store<Maths::HVec3>(patch.m_Normal.m_V, &pPatch->m_Normal);
		pPatch->m_NumElements = patch.m_NumContactPoints;
		pPatch->m_ValidData = SharedPhysics::E_ValidData;
		for(U32 b = 0; b < patch.m_NumContactPoints; ++b)
		{
			const C_ContactPoint& point(patch.m_ContactPoints[b]);
			SharedPhysics::C_ContactStreamPoint* pPoint((SharedPhysics::C_ContactStreamPoint*)dataStream);
			dataStream += sizeof(SharedPhysics::C_ContactStreamPoint);
			//assert(dataStream < stream.m_pStreamEnd);
			pPoint->m_HashValue = point.m_HashValue;
			pPoint->m_PhysicsMaterialIndices = point.m_PhysicsMaterialIndices;
			pPoint->m_EffectMaterialIndices = point.m_EffectMaterialIndices;
			pPoint->m_ValidData = SharedPhysics::E_ValidData;
			Maths::Store<Maths::HVec3>(point.m_ContactPoint, &pPoint->m_ContactPoint);
			//Maths::Store<Maths::HF32>(point.m_PenetrationDepth, &pPoint->m_PenetrationDepth);
			pPoint->m_PenetrationDepth = point.m_PenetrationDepth;
			++collisionCount;
			//pPoint->m_WarmstartValues.x = pPoint->m_pConstraint->m_pConstraint->m_WarmstartValues.y = pPoint->m_WarmstartValues.z = 0.f;
		}
		pPatch->m_OffsetToEnd = dataStream - patchStart;
	}

	pResult->m_TotalContacts = (U8)collisionCount;

	//Final bit - write the data to the stream!!!!
	INTPTR addy((INTPTR)(dataStream + 15));	// [Blur2PC][x64] align the full address
	dataStream = (U8*)(addy & ~(INTPTR)0xf);
	pResult->m_OffsetToEnd = dataStream - &tempData[0];

	U8* writeData = stream.LockMemorySpu(pResult->m_OffsetToEnd);
	if(writeData)
	{
		memcpy(writeData, tempData, pResult->m_OffsetToEnd);
		if(pCollider)
		{
			pCollider->m_StreamPtr = SharedPhysics::C_ConstContactStreamPtr(&stream,
				writeData);
		}

	}
}


template<U32 MaxContacts, U32 DesiredContacts>
void ReducePatch(C_ContactPatch<MaxContacts>& patch)
{
	//const U32 maxIterations(5);
	const HVec3 zero(Maths::Constants<HVec3>::Zero());
	const Maths::HF32 one(Maths::Constants<HF32>::One());
	const Maths::HF32 sZero(Maths::Constants<HF32>::Zero());
	const Maths::HF32 closeContact(SharedPhysics::Constants::CloseContact());
	S32 numContacts = patch.m_NumContactPoints;
	if(patch.m_NumContactPoints > DesiredContacts)
	{
		Spu::MemBaseMarker marker;
#ifndef PLATFORM_SPU
		C_Cluster<C_ContactPoint*, MaxContacts>* pointClusters = (C_Cluster<C_ContactPoint*, MaxContacts>*)alloca(sizeof(C_Cluster<C_ContactPoint*, MaxContacts>) * DesiredContacts);
#else
		C_Cluster<C_ContactPoint*, MaxContacts>* pointClusters = (C_Cluster<C_ContactPoint*, MaxContacts>*)Spu::Alloc(sizeof(C_Cluster<C_ContactPoint*, MaxContacts>) * DesiredContacts);
#endif
		//Reduce the contact data...
		pointClusters[0].m_Centroid = PickDeepestPenetration(patch);
		for(U32 b = 1; b < DesiredContacts; ++b)
		{
			pointClusters[b].m_Centroid = PickSupportPoint(pointClusters, patch, b);
		}

		//for(U32 b = 0; b < 1/*maxIterations*/; ++b)
		{
			//Blank the centroids and re-calculate...
			for(U32 c = 0; c < DesiredContacts; ++c)
			{
				pointClusters[c].m_ClusterSize = 0;
				pointClusters[c].m_FClusterSize = sZero;
			}
			for(U32 c = 0; c < patch.m_NumContactPoints; ++c)
			{
				C_ContactPoint& point(patch.m_ContactPoints[c]);
				const HVec3 dd(point.m_ContactPoint - pointClusters[0].m_Centroid);
				HF32 minDis(dd | dd);
				U32 index = 0;
				for(U32 d = 1; d < DesiredContacts; ++d)
				{
					C_Cluster<C_ContactPoint*, MaxContacts>& cluster(pointClusters[d]);
					const HVec3 dif(point.m_ContactPoint - cluster.m_Centroid);
					const HF32 dis(dif | dif);
					//TODO - remove branch from this code
					
					if(dis < minDis)
					{
						minDis = dis;
						index = d;
					}
				}
				//Now we've id'd which cluster this point is in, add it to the cluster...
				C_Cluster<C_ContactPoint*, MaxContacts>& cluster(pointClusters[index]);
				cluster.m_Cluster[cluster.m_ClusterSize++] = &point;
				cluster.m_FClusterSize += one;
			}
			//Update centroids...
			/*HF32 diff(Constants<HF32>::Zero());
			for(U32 c = 0; c < DesiredContacts; ++c)
			{
				C_Cluster<C_ContactPoint*, MaxContacts>& cluster(clusters[c]);
				cluster.m_OldCentroid = cluster.m_Centroid;
				cluster.m_Centroid = zero;
				const F32 sz((F32)cluster.m_ClusterSize);
				const HF32 size(Load<HF32>(&sz));
				for(U32 d = 0; d < cluster.m_ClusterSize; ++d)
				{
					const C_ContactPoint* pPoint = cluster.m_Cluster[d];
					cluster.m_Centroid += pPoint->m_ContactPoint;
				}
				cluster.m_Centroid /= size;
				const HVec3 dv(cluster.m_OldCentroid - cluster.m_Centroid);
				diff += (dv | dv);
			}
			if(diff < pointThreshold)
				break;*/
		}

		//Copy across contact points now...
		U32 index = 0;
		C_ContactPoint points[DesiredContacts];
		for(U32 b = 0; b < DesiredContacts; ++b)
		{
			C_Cluster<C_ContactPoint*, MaxContacts>& cluster(pointClusters[b]);
			if(cluster.m_ClusterSize > 0)
			{
				//HF32 penetrationDepth(Maths::Constants<HF32>::Zero());
				F32 penetrationDepth(0.f);
				U32 hashValue(0);
				//const F32 sz((F32)cluster.m_ClusterSize);
				//const HF32 size(Load<HF32>(&sz));
				for(U32 c = 0; c < cluster.m_ClusterSize; ++c)
				{
					C_ContactPoint* pPoint(cluster.m_Cluster[c]);
					//penetrationDepth += pPoint->m_PenetrationDepth;
					penetrationDepth = Maths::Max(penetrationDepth, pPoint->m_PenetrationDepth);
					hashValue += pPoint->m_HashValue;
				}
				//penetrationDepth /= cluster.m_FClusterSize;
				points[b].m_ContactPoint = cluster.m_Centroid;
				points[b].m_PenetrationDepth = penetrationDepth;
				points[b].m_HashValue = hashValue;
				points[b].m_PhysicsMaterialIndices = cluster.m_Cluster[0]->m_PhysicsMaterialIndices;
				points[b].m_EffectMaterialIndices = cluster.m_Cluster[0]->m_EffectMaterialIndices;

			}
		}
		//Copy points across over the contact points...
		numContacts = 0;
		for(U32 b = 0; b < DesiredContacts; ++b)
		{
			if(pointClusters[b].m_ClusterSize > 0)
				patch.m_ContactPoints[numContacts++] = points[b];
		}
		//patch.m_NumContactPoints = numContacts;
	}
	//Final bit - loop through and remove duplicates...
	//for(S32 b = 0; b < numContacts-1; ++b)
	//{
	//	const Maths::HVec3 pointA(patch.m_ContactPoints[b].m_ContactPoint);
	//	for(S32 c = b+1; c < numContacts; ++c)
	//	{
	//		const Maths::HVec3 pointB(patch.m_ContactPoints[c].m_ContactPoint);
	//		const Maths::HVec3 dif(pointA - pointB);
	//		const Maths::HF32 dis(dif | dif);
	//		if(dis < closeContact)
	//		{
	//			//Remove this contact...
	//			--numContacts;
	//			for(S32 d = c; d < numContacts; ++d)
	//			{
	//				patch.m_ContactPoints[d] = patch.m_ContactPoints[d+1];
	//			}
	//			--c;
	//		}
	//	}
	//}
	patch.m_NumContactPoints = numContacts;
}

template <int MaxPatches, int MaxContacts, int DesiredPatches, int DesiredContacts>
void ReduceContacts(C_CollisionResult<MaxPatches, MaxContacts>& result)
{
	const HVec3 zero(Maths::Constants<HVec3>::Zero());
	const Maths::HF32 one(Maths::Constants<HF32>::One());
	const Maths::HF32 sZero(Maths::Constants<HF32>::Zero());
	const Maths::HF32 closeContact(SharedPhysics::Constants::CloseContact());
	//const Maths::HF32 closeContact(0.01f);
	const Maths::HF32 closeNormal(SharedPhysics::Constants::CloseNormal());

	for(U32 a = 0; a < result.m_NumContactPatches; ++a)
	{
		RemoveDuplicateVertices<MaxContacts>(result.m_ContactPatches[a]);
	}

	
	if(result.m_NumContactPatches > DesiredPatches)
	{
		Spu::MemBaseMarker marker;
#ifndef PLATFORM_SPU
		//C_Cluster<C_ContactPatch<MaxContacts>*, MaxPatches> patchClusters[DesiredPatches];
		C_Cluster<C_ContactPatch<MaxContacts>*, MaxPatches>* patchClusters = (C_Cluster<C_ContactPatch<MaxContacts>*, MaxPatches>*)alloca(sizeof(C_Cluster<C_ContactPatch<MaxContacts>*, MaxPatches>) * DesiredPatches);
#else
		C_Cluster<C_ContactPatch<MaxContacts>*, MaxPatches>* patchClusters = (C_Cluster<C_ContactPatch<MaxContacts>*, MaxPatches>*)Spu::Alloc(sizeof(C_Cluster<C_ContactPatch<MaxContacts>*, MaxPatches>) * DesiredPatches);
#endif
		//const HF32 patchNormalThreshold(Load<HF32>(&g_dbPatchNormalThreshold));
		const F32 np((F32)result.m_NumContactPatches);
		//Need to reduce the number of patches here...
		//Pick arbitrary centroids to be the initial starting points. This 
		patchClusters[0].m_Centroid = PickBestNormal(result);
		for(U32 a = 1; a < DesiredPatches; ++a)
		{
			patchClusters[a].m_Centroid = PickSupportNormal(patchClusters, result, a);
		}
		
		{
			//Blank the centroids and re-calculate...
			for(U32 b = 0; b < DesiredPatches; ++b)
			{
				patchClusters[b].m_ClusterSize = 0;
			}
			for(U32 b = 0; b < result.m_NumContactPatches; ++b)
			{
				C_ContactPatch<MaxContacts>& cp(result.m_ContactPatches[b]);
				HVec3 norm(cp.m_Normal.m_V);
				HF32 maxDist(Maths::Abs(patchClusters[0].m_Centroid | norm));
				U32 closest(0);
				for(U32 c = 1; c < DesiredPatches; ++c)
				{
					HF32 dis(Maths::Abs(patchClusters[c].m_Centroid | norm));
					if(dis > maxDist)
					{
						maxDist = dis;
						closest = c;
					}
				}
				C_Cluster<C_ContactPatch<MaxContacts>*, MaxPatches>& patch = patchClusters[closest];
				patch.m_Cluster[patch.m_ClusterSize++] = &result.m_ContactPatches[b];
				result.m_ContactPatches[b].Cluster(&patch);
			}
		}

		bool processed[DesiredContacts];
		for(U32 a = 0; a < DesiredPatches; ++a)
		{
			processed[a] = false;
			C_ContactPatch<MaxContacts>& patch(result.m_ContactPatches[a]);
			//Collision::C_Cluster<Collision::C_ContactPatch<MaxContacts>*, MaxPatches>* cluster = patch.Cluster<Collision::C_Cluster<Collision::C_ContactPatch<MaxContacts>*, MaxPatches> >();
			Collision::C_Cluster<Collision::C_ContactPatch<MaxContacts>*, MaxPatches>* cluster = (Collision::C_Cluster<Collision::C_ContactPatch<MaxContacts>*, MaxPatches>*)patch.m_pCluster;
			patch.m_Normal = Maths::C_Normal(cluster->m_Centroid, Maths::C_Normal::UNIT_LENGTH_VECTOR);
			if(cluster->processed == false)
			{
				processed[a] = true;
				cluster->processed = true;
				//Add this patch into the system...
				for(U32 b = a+1; b < cluster->m_ClusterSize; ++b)
				{
					//Add the remaining points from this cluster into the patch...
					C_ContactPatch<MaxContacts>& otherPatch(result.m_ContactPatches[b]);
					for(U32 c = 0; c < otherPatch.m_NumContactPoints; ++c)
					{
						
						bool bFound = false;
						for(U32 k = 0; k < patch.m_NumContactPoints; ++k)
						{
							const Maths::HVec3 dif(patch.m_ContactPoints[k].m_ContactPoint - otherPatch.m_ContactPoints[c].m_ContactPoint);
							const Maths::HF32 d(dif | dif);
							if(d < closeContact)
							{
								patch.m_ContactPoints[k].m_PenetrationDepth = Maths::Max(patch.m_ContactPoints[k].m_PenetrationDepth, otherPatch.m_ContactPoints[c].m_PenetrationDepth);
								bFound = true;
								break;
							}
						}
						if(patch.m_NumContactPoints < MaxContacts && !bFound)
							patch.m_ContactPoints[patch.m_NumContactPoints++] = otherPatch.m_ContactPoints[c];
					}
				}
			}
		}	
		//Most of the clusters should have been processed. Finish off the remaining one's now...
		U32 index = 0;
		for(U32 a = 0; a < DesiredPatches; ++a)
		{
			C_Cluster<C_ContactPatch<MaxContacts>*, MaxPatches>& cluster(patchClusters[a]);
			if(cluster.processed == false)
			{
				cluster.processed = true;
				while(processed[index] && index < DesiredContacts)
				{
					BIZ_ASSERT(index < DesiredPatches);
					++index;
				}
				if(index < DesiredPatches)
				{
					processed[index] = true;
					C_ContactPatch<MaxContacts>& patch = result.m_ContactPatches[index];
					patch.m_Normal = C_Normal(cluster.m_Centroid, C_Normal::UNIT_LENGTH_VECTOR);
					patch.m_NumContactPoints = 0;
					//Now insert new cluster into the contact patch...
					for(U32 b = 0; b < cluster.m_ClusterSize; ++b)
					{
						C_ContactPatch<MaxContacts>* pPatch = cluster.m_Cluster[b];
						for(U32 c = 0; c < pPatch->m_NumContactPoints; ++c)
						{
							bool bFound = false;
							for(U32 k = 0; k < patch.m_NumContactPoints; ++k)
							{
								const Maths::HVec3 dif(patch.m_ContactPoints[k].m_ContactPoint - pPatch->m_ContactPoints[c].m_ContactPoint);
								const Maths::HF32 d(dif | dif);
								if(d < closeContact)
								{
									patch.m_ContactPoints[k].m_PenetrationDepth = Maths::Max(patch.m_ContactPoints[k].m_PenetrationDepth, pPatch->m_ContactPoints[c].m_PenetrationDepth);
									bFound = true;
									break;
								}
							}
							if(patch.m_NumContactPoints < MaxContacts && !bFound)
								patch.m_ContactPoints[patch.m_NumContactPoints++] = pPatch->m_ContactPoints[c];
						}
					}
				}
			}
		}
		result.m_NumContactPatches = DesiredPatches;
	}
	//Patches reduced. Now we need to reduce the number of contacts within the patches
	for(U32 a = 0; a < result.m_NumContactPatches; ++a)
	{
		ReducePatch<MaxContacts, DesiredContacts>(result.m_ContactPatches[a]);
	}
}

/*template <int MaxPatches, int MaxContacts, int DesiredPatches, int DesiredContacts>
void ReduceContactsAndPackToDatabase(C_CollisionResult<MaxPatches, MaxContacts>& result, SharedPhysics::C_ContactDatabase& database)
{
	ReduceContacts<MaxPatches, MaxContacts, DesiredPatches, DesiredContacts>(result);
	EncodeContacts(result, database);
}*/

template <int MaxPatches, int MaxContacts, int DesiredPatches, int DesiredContacts>
void ReduceContactsAndPackToStream(C_CollisionResult<MaxPatches, MaxContacts>& result, SharedPhysics::C_ContactStream& stream,
		SharedPhysics::C_Collider* pCollider)
{
#if 0 //def PLATFORM_SPU
	DebugBreak();
#endif
	//assert(!result.m_pObjectA->m_pNodePtr->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject));
	//assert(result.m_pObjectA->m_pShape->m_pNodePtr->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active));
	ReduceContacts<MaxPatches, MaxContacts, DesiredPatches, DesiredContacts>(result);
	EncodeContacts(result, stream, pCollider);
}

template <int MaxPatches, int MaxContacts, int DesiredPatches, int DesiredContacts>
void ReduceContactsAndPackToStreamSpu(C_CollisionResult<MaxPatches, MaxContacts>& result, SharedPhysics::C_ContactStream& stream,
		SharedPhysics::C_Collider* pCollider)
{
#if 0 //def PLATFORM_SPU
	DebugBreak();
#endif
	ReduceContacts<MaxPatches, MaxContacts, DesiredPatches, DesiredContacts>(result);
	EncodeContactsSpu(result, stream, pCollider);
}

};

#endif

