#ifndef __GRAVITY_CONTACT_STREAM_H__
#define __GRAVITY_CONTACT_STREAM_H__

#include <Core/Utility/MemCpy.h>
#include "gravity/sharedphysics/ContactStream.h"


namespace SharedPhysics
{

inline U8* C_ContactStream::LockMemory(const U32 size)
{
	//Need to do some atomic increment stuff
	U8* pStreamEnd = (U8*)Spu::Get((INTPTR*)&m_pStreamEnd);	// [Blur2PC][x64] pointer-wide
	U8* res = (U8*)MT::AtomicAdd((INTPTR*)(&m_pCurrentWritePoint), (INTPTR)size);	// [Blur2PC][x64] pointer-wide
	//assert(m_pCurrentWritePoint <= m_pStreamEnd);
	bool bOverflow = (res + size) > pStreamEnd;
	BIZ_ASSERT(!bOverflow);
	return SharedPhysics::Fast_Select<U8>(bOverflow, NULL, res);
}

inline U8* C_ContactStream::LockMemorySpu(const U32 size)
{
	U8* pStreamEnd = m_pStreamEnd;
	U8* res = m_pCurrentWritePoint;
	bool bOverflow = (res + size) > pStreamEnd;
	m_pCurrentWritePoint = SharedPhysics::Fast_Select<U8>(bOverflow, m_pCurrentWritePoint, m_pCurrentWritePoint + size);
	BIZ_ASSERT(!bOverflow);
	return SharedPhysics::Fast_Select<U8>(bOverflow, NULL, res);
}



NOALIAS inline void EncodeContacts(SharedPhysics::C_Collider* result, SharedPhysics::C_ContactStream& stream)
{
	const SharedPhysics::C_ConstContactStreamPtr& ptr = result->m_StreamPtr;
	U8* dataStream(stream.m_pCurrentWritePoint);
	//C_ContactStreamResult* pResult((C_ContactStreamResult*)dataStream);
	const U8* oldStream(ptr.ReadPoint());
	const C_ContactStreamResult* oldResult((C_ContactStreamResult*)ptr.ReadPoint());
	::FastMemCpy(dataStream, (void*)oldStream, oldResult->m_OffsetToEnd);
	dataStream += oldResult->m_OffsetToEnd;
	stream.m_pCurrentWritePoint = dataStream;
	C_ContactStreamResult* pResultScrap((C_ContactStreamResult*)dataStream);
	pResultScrap->m_ValidData = E_NullData;
}

NOALIAS inline void EncodeContacts(C_CollisionResultDB& result, C_ContactStream& stream)
{
	//U8* dataStream(stream.m_pCurrentWritePoint);
	//C_ContactStreamResult* pResult((C_ContactStreamResult*)dataStream);
	//dataStream += sizeof(C_ContactStreamResult);
	//BIZ_ASSERT(dataStream < stream.m_pStreamEnd);
	//pResult->m_NumElements = result.m_NumPatches;
	//pResult->m_pNodeA = result.m_pNodeA;
	//pResult->m_pNodeB = result.m_pNodeB;
	//pResult->m_ValidData = E_ValidData;
	//C_ContactPatchDB* patch(result.m_pContactPatches);
	//for(U32 a = 0; a < result.m_NumPatches; ++a)
	//{
	//	const U8* patchStart(dataStream);
	//	C_ContactStreamPatch* pPatch((C_ContactStreamPatch*)dataStream);
	//	dataStream += sizeof(C_ContactStreamPatch);
	//	BIZ_ASSERT(dataStream < stream.m_pStreamEnd);
	//	//Maths::Store<Maths::HVec3>(patch->m_Normal, &pPatch->m_Normal);
	//	pPatch->m_Normal = patch->m_Normal;
	//	pPatch->m_NumElements = patch->m_NumContactPoints;
	//	pPatch->m_ValidData = E_ValidData;
	//	C_ContactPointDB* point(patch->m_pContactPoints);
	//	for(U32 b = 0; b < patch->m_NumContactPoints; ++b)
	//	{
	//		C_ContactStreamPoint* pPoint((C_ContactStreamPoint*)dataStream);
	//		dataStream += sizeof(C_ContactStreamPoint);
	//		BIZ_ASSERT(dataStream < stream.m_pStreamEnd);
	//		//Maths::Store<Maths::HVec3>(point->m_ContactPoint, &pPoint->m_ContactPoint);
	//		pPoint->m_ContactPoint = point->m_ContactPoint;
	//		pPoint->m_PenetrationDepth = point->m_PenetrationDepth;
	//		//Maths::Store<Maths::HVec3>(point->m_WarmstartValues, &pPoint->m_WarmstartValues);
	//		pPoint->m_pConstraint->m_pConstraint->StoreWarmstartValues(&point->m_WarmstartForces.v[0]);
	//		pPoint->m_HashValue = point->m_HashValue;
	//		pPoint->m_PhysicsMaterialIndices = point->m_PhysicsMaterialIndices;
	//		pPoint->m_EffectMaterialIndices = point->m_EffectMaterialIndices;
	//		pPoint->m_ValidData = E_ValidData;
	//		point = point->m_pNext;
	//	}
	//	pPatch->m_OffsetToEnd = dataStream - patchStart;
	//	patch = patch->m_pNext;
	//}
	//U32 addy((U32)(dataStream + 15));
	//dataStream = (U8*)(addy & 0xfffffff0);
	//pResult->m_OffsetToEnd = dataStream - stream.m_pCurrentWritePoint;
	//stream.m_pCurrentWritePoint = dataStream;
	//C_ContactStreamResult* pResultScrap((C_ContactStreamResult*)dataStream);
	//pResultScrap->m_ValidData = E_NullData;
}

};

#endif
