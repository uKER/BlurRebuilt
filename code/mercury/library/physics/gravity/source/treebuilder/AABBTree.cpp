#include <stdafx.h>
#include "gravity/treebuilder/AABBTree.h"
#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/PrimitiveTest.inl"
#include "gravity/sharedphysics/SortedArray.inl"
#include "gravity/sharedphysics/ContactDatabase.inl"
#include <system/debuglog.h>
#include "gravity/sharedphysics/safeStack.h"

/*#include <Render/Mesh/MeshSaver.h>
#include <Render/Mesh/MeshBuilder.h>
#include <Render/Shaders/World/psWorld.h>
#include <Render/Shaders/World/vsWorld.h>
#include <Render/Debug/DebugRender.h>
#include <Render/Debug/ImmediateDebugRender.h>*/




//using namespace Maths;

namespace Baking
{
#if CFG_CONVERTER
Debug::Logger &g_Log = Debug::GetLogger("gravity.exporter");
#endif

void AABBTree::ValidateTree(AABBTreeNode* pNode)
{
	//Write the bounding box...
#if 0
	Collision::C_AxisBoxMinMax& aabb = pNode->aabb;
	Collision::C_QuantizedAxisBoxMinMax quantAABB(aabb, this->m_pRoot->aabb.m_min, m_pRoot->aabb.m_max);
	
	Collision::C_AxisBoxMinMax box;
	quantAABB.LoadBox(box, m_pRoot->aabb.m_min, m_pRoot->aabb.m_max);

	//assert(aabb.Inside(box));

	//Now write number of indices...
	if(pNode->m_leftNode)
	{
		ValidateTree(pNode->m_leftNode);
		ValidateTree(pNode->m_rightNode);
		BIZ_ASSERT(pNode->m_leftNode->aabb.Inside(box));
		BIZ_ASSERT(pNode->m_rightNode->aabb.Inside(box));
	}
	else
	{
		//Walk through all triangles and ensure they are inside this node!!!!!
		U32 offset = pNode->m_startIndex;
		Collision::C_TriangleHeader* header = m_Stream.GetHeader(offset);
		U32 headerSize = header->m_NumTriangles;
		offset += sizeof(Collision::C_TriangleHeader);

		for(U32 a = 0; a < pNode->m_sizeIndice; a += headerSize)
		{
			headerSize = header->m_NumTriangles;
			for(U32 b = 0; b < header->m_NumTriangles; ++b)
			{
				//U32 index = pNode->m_startIndex + a;
				Collision::AABBTreeTriangle* tri = m_Stream.GetTriangles(offset);
				offset += sizeof(Collision::AABBTreeTriangle);
				//Baking::AABBBuildTreeTriangle& tri(m_indice[index]);
				
				const Maths::HVec3 vA(m_vertice[tri->m_IndexA]);
				const Maths::HVec3 vB(m_vertice[tri->m_IndexB]);
				const Maths::HVec3 vC(m_vertice[tri->m_IndexC]);
				BIZ_ASSERT(box.Contains(vA));
				BIZ_ASSERT(box.Contains(vB));
				BIZ_ASSERT(box.Contains(vC));
			}
			header = m_Stream.GetHeader(offset);
			offset += sizeof(Collision::C_TriangleHeader);
		}


	}
#endif
}


U32 AABBTree::GetSplitStatus(const U32 index, AABBBuildTreeTriangle& tri, const Collision::C_Plane& p, const Maths::HF32 PCREF threshold)
{
	using namespace Maths;

	const Maths::HVec3 zero(-Maths::Constants<HF32>::Epsilon2());
	const HF32 fzero(Maths::Constants<HF32>::Zero());
	//AABBBuildTreeTriangle& tri = m_indice[index];
	const Maths::HVec3 a(m_vertice[tri.m_IndexA]);
	const Maths::HVec3 b(m_vertice[tri.m_IndexB]);
	const Maths::HVec3 c(m_vertice[tri.m_IndexC]);

	
	HVec3 dist(Maths::UNDEFINED);
	dist.X((p.m_n|a)-p.m_d);
	dist.Y((p.m_n|b)-p.m_d);
	dist.Z((p.m_n|c)-p.m_d);

	if(dist.All_GE(zero))
	{
		return POINT_IN_FRONT_PLANE;
	}
	if(dist.All_LT(-zero))
	{
		return POINT_IN_BACK_PLANE;
	}

	const HF32 sum(dist.X() + dist.Y() + dist.Z());
	const HF32 sign = sum.Sign();
	const HF32 min(Min(sign* dist.X(), Min(sign * dist.Y(),sign* dist.Z())));
//	const Maths::HF32 min(Maths::Max(Maths::Abs(dist.X()), Maths::Max(Maths::Abs(dist.Y()), Maths::Abs(dist.Z()))));
	//dd is the absolute value of the largest projection on the opposite side to the centre of the triangle
	//Test for straddle over threshold...
	if(Maths::Abs(min) > threshold)
		return POINT_STRADDLE_PLANE;
	//else
	

	if(sum >= fzero)
	{
		return POINT_IN_FRONT_PLANE;
	}
	return POINT_IN_BACK_PLANE;
}

/*F32 AABBTree::GetScore(const Collision::Plane& p, const U32 startIndex, const U32 endIndex, const F32 k)
{
	HF32 threshold(Maths::Constants<HF32>::Epsilon2());

	U32 numInFront=0; U32 numInBack=0; U32 numStraddle=0;

	for(U32 j=startIndex; j<endIndex; ++j)
	{
		//Maths::HVec3 dist=this->GetEachVertexSignedDistance(j, p);
		switch(GetSplitStatus(j, p, threshold))
		{
		case POINT_IN_FRONT_PLANE:
			numInFront++;
			break;
		case POINT_IN_BACK_PLANE:
			numInBack++;
			break;
		default:
			numStraddle++;
		}
	}

	U32 abs=Maths::Abs( (S32)numInFront - (S32)numInBack );

	return (k*((F32)numStraddle) + (1.0f - k)*((F32)abs));//Abs(numInFront-numInBack);
}*/

F32 AABBTree::GetScore(const Collision::C_Plane& p, const U32 startIndex, const U32 endIndex, const F32 k, const Maths::HF32 PCREF recipParentVolume, const Maths::HF32 PCREF threshold)
{
	using namespace Maths;
	const HVec3 epsilon(Maths::Constants<HVec3>::Epsilon2());

	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HF32 one(Maths::Constants<HF32>::One());
	const HF32 two(Maths::Constants<HF32>::Two());
	const HF32 one_point_five(one + Maths::Constants<HF32>::Half());
	const HF32 vk(Maths::Load<HF32>(&k));
	HF32 numInFront(zero); HF32 numInBack(zero); HF32 numStraddle(zero);
	

	HVec3 minFront(Maths::Constants<HVec3>::Max());
	HVec3 maxFront(-Maths::Constants<HVec3>::Max());
	HVec3 minBack(Maths::Constants<HVec3>::Max());
	HVec3 maxBack(-Maths::Constants<HVec3>::Max());
	SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter(_m_indice, startIndex);
	for(U32 j=startIndex; j<endIndex; ++j)
	{
		AABBBuildTreeTriangle& tri = *iter.Next();
		const HVec3 minTri(Min(m_vertice[tri.m_IndexA], Min(m_vertice[tri.m_IndexB], m_vertice[tri.m_IndexC])));
		const HVec3 maxTri(Max(m_vertice[tri.m_IndexA], Max(m_vertice[tri.m_IndexB], m_vertice[tri.m_IndexC])));
		//Maths::HVec3 dist=this->GetEachVertexSignedDistance(j, p);
		switch(GetSplitStatus(j, tri, p, threshold))
		{
		case POINT_IN_FRONT_PLANE:
			numInFront+=one;
			minFront = Min(minFront, minTri);
			maxFront = Max(maxFront, maxTri);
			break;
		case POINT_IN_BACK_PLANE:
			numInBack+=one;
			minBack = Min(minBack, minTri);
			maxBack = Max(maxBack, maxTri);
			break;
		default:
			numStraddle+=one;
			/*minFront = Min(minFront, minTri);
			maxFront = Max(maxFront, maxTri);
			minBack = Min(minBack, minTri);
			maxBack = Max(maxBack, maxTri);*/
		}
	}


	/*const HF32 origScore((one - vk) * (numInFront - numInBack).Abs() + k * numStraddle);
	const HVec3 eps(Constants<HVec3>::Epsilon2());
	const HVec3 dFront((maxFront-minFront) + eps);
	const HF32 volumeFront(dFront.X()*dFront.Y()*dFront.Z());
	const HVec3 dBack(maxBack-minBack);
	const HF32 volumeBack(dBack.X()*dBack.Y()*dBack.Z());

	const HF32 numerator((volumeFront * (numInFront + numStraddle)) + (volumeBack * (numInBack + numStraddle)));
	const HF32 score(numerator*recipParentVolume + origScore);
	F32 fScore;
	Store(score, &fScore);
	return fScore;
	*/

	const HVec3 dFront((maxFront-minFront) + epsilon);
	const HF32 volumeFront(dFront.X()*dFront.Y()*dFront.Z());
	const HVec3 dBack((maxBack-minBack) + epsilon);
	const HF32 volumeBack(dBack.X()*dBack.Y()*dBack.Z());

	const HF32 frontScore(volumeFront*recipParentVolume);
	const HF32 backScore(volumeBack*recipParentVolume);
	const HF32 totalScore(frontScore+backScore);
	F32 fScore;
	//Store(totalScore, &fScore);

	if(frontScore.GT(zero)& frontScore.LT(one)& backScore.GT(zero)& backScore.LT(one) & totalScore.LT(one_point_five))
	{
		const HF32 score((frontScore * (numInFront + numStraddle)) + (backScore * (numInBack + numStraddle)));
		//const HF32 score(numerator*recipParentVolume);
		Store(score, &fScore);
	}
	else
	{
		Store(Maths::Constants<HF32>::Max(), &fScore);
	}

	return fScore;
}

Collision::C_Plane AABBTree::PickASeperatedEdgePlane(const U32 startIndex, const U32 size, F32& score, const Maths::HF32 PCREF recipVolume, const Maths::HF32 PCREF threshold)
{
	using namespace Maths;

	U32 endIndex=startIndex+size;

	Collision::C_Plane bestEdgePlane;
	F32 bestEdgeScore(Maths::Constants<F32>::Max());
	const F32 k=0.7f;

	//pick the surface planes
	for(U32 i=startIndex; i<endIndex; ++i)
	{
		AABBBuildTreeTriangle& tri = _m_indice[i];
		const Maths::HVec3 a(m_vertice[tri.m_IndexA]);
		const Maths::HVec3 b(m_vertice[tri.m_IndexB]);
		const Maths::HVec3 c(m_vertice[tri.m_IndexC]);	

		const Maths::HVec3 ab(b-a);
		const Maths::HVec3 ac(c-a);
		const Maths::HVec3 normal(ab^ac);
	

		Maths::C_Normal g0(normal^ab);
		Maths::C_Normal g1(normal^ac);
		Maths::C_Normal g2(normal^(c-b));
	/*	g0.Normalise();
		g1.Normalise();
		g2.Normalise();*/

		const Maths::HF32 d0(g0|a);
		const Maths::HF32 d1(g1|a);
		const Maths::HF32 d2(g2|b);

		const Collision::C_Plane p0(g0, d0);
		const Collision::C_Plane p1(g1, d1);
		const Collision::C_Plane p2(g2, d2);
		
		
		F32 score0=GetScore(p0, startIndex, endIndex, k, recipVolume, threshold);
		F32 score1=GetScore(p1, startIndex, endIndex, k, recipVolume, threshold);
		F32 score2=GetScore(p2, startIndex, endIndex, k, recipVolume, threshold);
		if(score0 < score1 && score0<score2)
		{
			if(score0<bestEdgeScore)
			{
				bestEdgePlane=p0;
				bestEdgeScore=score0;
			}
		}
		else if(score1 < score0 && score1<score2)
		{
			if(score1<bestEdgeScore)
			{
				bestEdgePlane=p1;
				bestEdgeScore=score1;
			}
		}
		else
		{
			if(score2<bestEdgeScore)
			{
				bestEdgePlane=p2;
				bestEdgeScore=score2;
			}
		}
		
	}
	score=bestEdgeScore;
	return bestEdgePlane;
}



Collision::C_Plane AABBTree::PickASeperatedSurfacePlane(const U32 startIndex, const U32 size, F32& score, const Maths::HF32 PCREF recipVolume, const Maths::HF32 PCREF threshold)
{
	using namespace Maths;

	U32 endIndex=startIndex+size;

	Collision::C_Plane bestSurfacePlane;
	F32 bestSurfaceScore(Maths::Constants<F32>::Max());
	const F32 k=0.6f;
	//pick the surface planes
	for(U32 i=startIndex; i<endIndex; i++)
	{
		AABBBuildTreeTriangle& tri = _m_indice[i];
		const Maths::HVec3 a(m_vertice[tri.m_IndexA]);
		const Maths::HVec3 b(m_vertice[tri.m_IndexB]);
		const Maths::HVec3 c(m_vertice[tri.m_IndexC]);	


		const Maths::HVec3 ab(b-a);
		const Maths::HVec3 ac(c-a);
		Maths::C_Normal n(ab^ac);
		//n.Normalise();
		const Maths::HF32 d(n|a);

		const Collision::C_Plane p(n, d);

		F32 score=GetScore(p, startIndex, endIndex, k, recipVolume, threshold);
		if(score<bestSurfaceScore)
		{
			bestSurfacePlane=p;
			bestSurfaceScore=score;
		}
	}
	score=bestSurfaceScore;
	return bestSurfacePlane;
}

void AABBTree::ExtractEdgeEvents(SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter, SharedPhysics::SortedArray<C_EdgeEvent>& edgeEvents, U32 axis, U32 startIndex, U32 endIndex)
{
	edgeEvents.m_numElements = 0;
	//SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter(m_indice, startIndex);
	for(U32 a = startIndex; a < endIndex; ++a)
	{
		AABBBuildTreeTriangle& tri = *iter.Next();
		const Maths::HVec3 vertA = m_vertice[tri.m_IndexA];
		const Maths::HVec3 vertB = m_vertice[tri.m_IndexB];
		const Maths::HVec3 vertC = m_vertice[tri.m_IndexC];

		const Maths::HVec3 _max(Maths::Max(vertA, Maths::Max(vertB, vertC)) + Maths::Constants<Maths::HVec3>::Epsilon());
		const Maths::HVec3 _min(Maths::Min(vertA, Maths::Min(vertB, vertC)) - Maths::Constants<Maths::HVec3>::Epsilon());

		if(axis == 0) // X_AXIS
		{
			F32 range = _max.X() - _min.X();
			C_EdgeEvent ev1, ev2;
			ev1.value = _max.X();
			ev1.range = range;
			ev1.bStart = false;
			ev2.value = _min.X();
			ev2.bStart = true;
			ev2.range = range;
			ev1.m_Min = _min;
			ev1.m_Max = _max;
			ev2.m_Min = _min;
			ev2.m_Max = _max;
			edgeEvents.AddToEnd(&ev1);
			edgeEvents.AddToEnd(&ev2);
		}
		
		else if(axis == 1) // Y_AXIS
		{
			F32 range = _max.Y() - _min.Y();
			C_EdgeEvent ev1, ev2;
			ev1.value = _max.Y();
			ev1.bStart = false;
			ev1.range = range;
			ev2.value = _min.Y();
			ev2.bStart = true;
			ev2.range = range;
			ev1.m_Min = _min;
			ev1.m_Max = _max;
			ev2.m_Min = _min;
			ev2.m_Max = _max;
			edgeEvents.AddToEnd(&ev1);
			edgeEvents.AddToEnd(&ev2);
		}
		
		else //Z_AXIS
		{
			F32 range = _max.Z() - _min.Z();
			C_EdgeEvent ev1, ev2;
			ev1.value = _max.Z();
			ev1.range = range;
			ev1.bStart = false;
			ev2.value = _min.Z();
			ev2.bStart = true;
			ev2.range = range;
			ev1.m_Min = _min;
			ev1.m_Max = _max;
			ev2.m_Min = _min;
			ev2.m_Max = _max;
			edgeEvents.AddToEnd(&ev1);
			edgeEvents.AddToEnd(&ev2);
		}
	}
	SharedPhysics::C_S32SafeStack stack(16092);
	edgeEvents.QuickSort(stack, 10);
}

F32 AABBTree::PickFatSubdivision(SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter, const U32 startIndex, const U32 size, 
								 F32& scoreFat, const U32 axis)
{
	//0 = x-axis, 1 = y-axis, 2 = z-axis
	//Returns float value of the plane on this axis!

	if(size == 0)
		return 0.f;

	
	//Calculate the axis bounding box!
	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
	SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter1(iter);
	const U32 endIndex = startIndex + size;

	Maths::HVec3 vMin(Maths::Constants<Maths::HVec3>::Max()), vMax(-Maths::Constants<Maths::HVec3>::Max());

	for(U32 a = startIndex; a < endIndex; ++a)
	{
		AABBBuildTreeTriangle* tri = iter1.Next();
		const Maths::HVec3 vertA(m_vertice[tri->m_IndexA]);
		const Maths::HVec3 vertB(m_vertice[tri->m_IndexB]);
		const Maths::HVec3 vertC(m_vertice[tri->m_IndexC]);

		vMin = Maths::Min(vMin, Maths::Min(vertA, Maths::Min(vertB, vertC)));
		vMax = Maths::Max(vMax, Maths::Max(vertA, Maths::Max(vertB, vertC)));
	}

	const Maths::HVec3 centre((vMax + vMin) * half);
	U32 numLeft = 0, numRight = 0, numStraddle = 0;

	Maths::HF32 splitVal(Maths::UNDEFINED);
	if(axis == 0)
	{
		splitVal = centre.X();
		for(U32 a = startIndex; a < endIndex; ++a)
		{
			AABBBuildTreeTriangle* tri = iter.Next();
			const Maths::HVec3 vertA(m_vertice[tri->m_IndexA]);
			const Maths::HVec3 vertB(m_vertice[tri->m_IndexB]);
			const Maths::HVec3 vertC(m_vertice[tri->m_IndexC]);

			const Maths::HF32 valA(vertA.X());
			const Maths::HF32 valB(vertB.X());
			const Maths::HF32 valC(vertC.X());

			if(valA.LE(splitVal) & valB.LE(splitVal) & valC.LE(splitVal))
			{
				numLeft++;
			}
			else if (valA.GE(splitVal) & valB.GE(splitVal) & valC.GE(splitVal))
			{
				numRight++;
			}
			else
			{
				numStraddle++;
			}
		}
	}
	else if (axis == 1)
	{
		splitVal  = centre.Y();
		for(U32 a = startIndex; a < endIndex; ++a)
		{
			AABBBuildTreeTriangle* tri = iter.Next();
			const Maths::HVec3 vertA(m_vertice[tri->m_IndexA]);
			const Maths::HVec3 vertB(m_vertice[tri->m_IndexB]);
			const Maths::HVec3 vertC(m_vertice[tri->m_IndexC]);

			const Maths::HF32 valA(vertA.Y());
			const Maths::HF32 valB(vertB.Y());
			const Maths::HF32 valC(vertC.Y());

			if(valA.LE(splitVal) & valB.LE(splitVal) & valC.LE(splitVal))
			{
				numLeft++;
			}
			else if (valA.GE(splitVal) & valB.GE(splitVal) & valC.GE(splitVal))
			{
				numRight++;
			}
			else
			{
				numStraddle++;
			}
		}
	}
	else
	{
		splitVal = centre.Z();	
		for(U32 a = startIndex; a < endIndex; ++a)
		{
			AABBBuildTreeTriangle* tri = iter.Next();
			const Maths::HVec3 vertA(m_vertice[tri->m_IndexA]);
			const Maths::HVec3 vertB(m_vertice[tri->m_IndexB]);
			const Maths::HVec3 vertC(m_vertice[tri->m_IndexC]);

			const Maths::HF32 valA(vertA.Z());
			const Maths::HF32 valB(vertB.Z());
			const Maths::HF32 valC(vertC.Z());

			if(valA.LE(splitVal) & valB.LE(splitVal) & valC.LE(splitVal))
			{
				numLeft++;
			}
			else if (valA.GE(splitVal) & valB.GE(splitVal) & valC.GE(splitVal))
			{
				numRight++;
			}
			else
			{
				numStraddle++;
			}
		}
	}
	
	scoreFat = Score(numLeft, numRight, numStraddle, 0, 0, 0, 0);
	return (F32)splitVal;
}


F32 AABBTree::PickBestEdgePoint(SharedPhysics::SortedArray<C_EdgeEvent>& edgeEvents, F32& score)
{
	if(edgeEvents.m_numElements == 0)
		return 0.f;

	S32 activeCount = 0;
	S32 leftCount = 0;
	S32 rightCount = edgeEvents.m_numElements/2;
	F32 bestScore = Maths::Constants<F32>::Max();	F32 bestValue = 0.f;

	//Score algorithm = Maths::Abs(leftCount - rightCount) + 2 * activeCount
	F32 midLow = edgeEvents[0].value;
	F32 midHigh = edgeEvents[0].value;
	F32 low = edgeEvents[0].value;
	F32 high = edgeEvents[edgeEvents.m_numElements - 1].value;
	S32 midStartIndex = 0;

	U32 centreLow = 0;


	for(S32 a = 0; a < edgeEvents.m_numElements; ++a)
	{
		C_EdgeEvent& ev = edgeEvents[a];
		if(ev.bStart)
		{
			midHigh = ev.value + ev.range;
			activeCount++;
			rightCount--;
		}
		else
		{
			leftCount++;
			activeCount--;

			for(S32 b = midStartIndex; b < a; ++b)
			{
				//Find next start point and adjust mid-low!
				if(edgeEvents[b].bStart && 
					(edgeEvents[b].value + edgeEvents[b].range) > ev.value)
				{
					midLow = edgeEvents[b].value;
					midStartIndex = b;
					break;
				}
			}

		}
		F32 newScore = Score(leftCount, rightCount, activeCount, midLow, midHigh, low, high);
		//F32 newScore = Score(leftCount, rightCount, activeCount, midLow, midHigh, low, high, edgeEvents, 0, a);
		if(newScore < bestScore)
		{
			bestScore = newScore;
			bestValue = ev.value;
		}
	}
	score = bestScore;
	return bestValue;
}


Collision::C_Plane AABBTree::PickASeperatedAlignAxisPlane(SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter,
	const U32 startIndex, const U32 size, const Collision::C_AxisBoxMinMax& aabb, F32& score, 
	const Maths::HF32 PCREF recipVolume, Maths::HF32& threshold)
{
	using namespace Maths;

	const Maths::HF32 half(Maths::Constants<HF32>::Half());

	const Maths::HF32 zero(Maths::Constants<HF32>::Zero());
	const Maths::C_Normal xNormal(HVec3(1.f, 0.f, 0.f), Maths::C_Normal::UNIT_LENGTH_VECTOR);
	const Maths::C_Normal yNormal(HVec3(0.f, 1.f, 0.f), Maths::C_Normal::UNIT_LENGTH_VECTOR);
	const Maths::C_Normal zNormal(HVec3(0.f, 0.f, 1.f), Maths::C_Normal::UNIT_LENGTH_VECTOR);

	//Now pick average point...

	U32 endIndex = startIndex + size;

	F32 score1, score2, score3;
	F32 value1, value2, value3;
	SharedPhysics::SortedArray<C_EdgeEvent> edgeEvents(2 * size);
	ExtractEdgeEvents(iter, edgeEvents, 0, startIndex, endIndex);
	value1 = PickBestEdgePoint(edgeEvents, score1);
	ExtractEdgeEvents(iter, edgeEvents, 1, startIndex, endIndex);
	value2 = PickBestEdgePoint(edgeEvents, score2);
	ExtractEdgeEvents(iter, edgeEvents, 2, startIndex, endIndex);
	value3 = PickBestEdgePoint(edgeEvents, score3);

	F32 scoreFat1, scoreFat2, scoreFat3;
	//F32 fatValue1, fatValue2, fatValue3;

	////Pick an axis-aligned fat subdivision plane...
	/*fatValue1 = PickFatSubdivision(iter, startIndex, size, scoreFat1, 0);
	fatValue1 = PickFatSubdivision(iter, startIndex, size, scoreFat1, 1);
	fatValue1 = PickFatSubdivision(iter, startIndex, size, scoreFat1, 2);*/


	if(!m_bSplitX)
	{
		score1 = Maths::Constants<F32>::Max();
		scoreFat1 = Maths::Constants<F32>::Max();
	}
	if(!m_bSplitY)
	{
		score2 = Maths::Constants<F32>::Max();
		scoreFat2 = Maths::Constants<F32>::Max();
	}
	if(!m_bSplitZ)
	{
		score3 = Maths::Constants<F32>::Max();
		scoreFat3 = Maths::Constants<F32>::Max();
	}

	/*if(scoreFat1 < scoreFat2 && scoreFat1 < scoreFat3 && 
		scoreFat1 < score1 && scoreFat1 < score2 && scoreFat1 < score3)
	{
		threshold = m_splitRatio * (aabb.m_max.X() - aabb.m_min.X());
		return Collision::C_Plane(xNormal, Maths::HF32(fatValue1));
	}
	else if (scoreFat2 < scoreFat3 && scoreFat2 < score1 && scoreFat2 < score2 && scoreFat2 < score3)
	{
		threshold = m_splitRatio * (aabb.m_max.Y() - aabb.m_min.Y());
		return Collision::C_Plane(yNormal, Maths::HF32(fatValue2));
	}
	else if (scoreFat3 < score1 && scoreFat3 < score2 && scoreFat3 < score3)
	{
		threshold = m_splitRatio * (aabb.m_max.Z() - aabb.m_min.Z());
		return Collision::C_Plane(zNormal, Maths::HF32(fatValue3));
	}
	else*/ if(score1 < score2 && 
		score1 < score3)
	{
		threshold = m_splitRatio * (aabb.m_max.X() - aabb.m_min.X());
		return Collision::C_Plane(xNormal, Maths::HF32(value1));
	}
	else if (score2 < score3)
	{
		threshold = m_splitRatio * (aabb.m_max.Y() - aabb.m_min.Y());
		return Collision::C_Plane(yNormal, Maths::HF32(value2));
	}
	threshold = m_splitRatio * (aabb.m_max.Z() - aabb.m_min.Z());
	return Collision::C_Plane(zNormal, Maths::HF32(value3));
}


//Collision::C_Plane AABBTree::PickASeperatedAlignAxisPlane(const U32 startIndex, const U32 size, const C_AxisBoxMinMax& aabb, F32& score, const Maths::HF32 PCREF recipVolume, const Maths::HF32 PCREF threshold)
//{
//	using namespace Maths;
//	const F32 _size(F32(size*3));
//	U32 endIndex=startIndex+size;
//	Collision::C_Plane bestPlane;
//	F32 bestScore(Maths::Constants<F32>::Max());
//	const Maths::HF32 half(Maths::Constants<HF32>::Half());
//
//	const Maths::HVec3 center((aabb.m_min + aabb.m_max)*half);
//
//	const F32 k=0.7f;
//	const Maths::HF32 zero(Maths::Constants<HF32>::Zero());
//	const Maths::HVec3 xNormal(1.f, 0.f, 0.f);
//	const Maths::HVec3 yNormal(0.f, 1.f, 0.f);
//	const Maths::HVec3 zNormal(0.f, 0.f, 1.f);
//
//	//Now pick average point...
//	HVec3 sumPoints(Maths::Constants<HVec3>::Zero());
//	const HF32 sizeRecip = Load<HF32>(&_size).Reciprocal(); 
//	for(U32 a = startIndex; a < endIndex; ++a)
//	{
//		AABBBuildTreeTriangle& tri = m_indice[a];
//		sumPoints += m_vertice[tri.m_IndexA];
//		sumPoints += m_vertice[tri.m_IndexB];
//		sumPoints += m_vertice[tri.m_IndexC];
//	}
//	sumPoints *= sizeRecip;
//
//	const Collision::C_Plane xp(xNormal, center.X());
//	const Collision::C_Plane yp(yNormal, center.Y());
//	const Collision::C_Plane zp(zNormal, center.Z());
//	const Collision::C_Plane xp2(xNormal, sumPoints.X());
//	const Collision::C_Plane yp2(yNormal, sumPoints.Y());
//	const Collision::C_Plane zp2(zNormal, sumPoints.Z());
//
//	F32 xScore=GetScore(xp, startIndex, endIndex, k, recipVolume, threshold);
//	F32 yScore=GetScore(yp, startIndex, endIndex, k, recipVolume, threshold);
//	F32 zScore=GetScore(zp, startIndex, endIndex, k, recipVolume, threshold);
//
//	F32 xScore2=GetScore(xp2, startIndex, endIndex, k, recipVolume, threshold);
//	F32 yScore2=GetScore(yp2, startIndex, endIndex, k, recipVolume, threshold);
//	F32 zScore2=GetScore(zp2, startIndex, endIndex, k, recipVolume, threshold);
//
//	bestScore = xScore;
//	bestPlane = xp;
//	if(yScore < bestScore)
//	{
//		bestPlane=yp;
//		bestScore=yScore;
//	}
//	if(zScore < bestScore)
//	{
//		bestPlane=zp;
//		bestScore=zScore;
//	}
//	if (xScore2 < bestScore)
//	{
//		bestPlane = xp2;
//		bestScore = xScore2;
//	}
//	if (yScore2 < bestScore)
//	{
//		bestPlane = yp2;
//		bestScore = yScore2;
//	}
//	if (zScore2 < bestScore)
//	{
//		bestPlane = zp2;
//		bestScore = zScore2;
//	}
//
//
//	score=bestScore;
//	return bestPlane;
//}

AABBTreeNode* AABBTree::CreateAABBNode(const U32 startIndex, const U32 size, SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter)
{
	using namespace Maths;
	AABBTreeNode* node=new AABBTreeNode(startIndex, size);
	Maths::HVec3 min_a(Maths::Constants<HVec3>::Max());
	Maths::HVec3 max_a(-Maths::Constants<HVec3>::Max());
	const Maths::HVec3 epsilon(Maths::Constants<Maths::HF32>::Epsilon2());

	U32 endIndex=startIndex+size;
	//SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter(m_indice, startIndex);
	for(U32 i=startIndex; i<endIndex; ++i)
	{
		AABBBuildTreeTriangle* tri = iter.Next();
		//assert(tri->m_MaterialIndex < 64);
		const Maths::HVec3 temp(m_vertice[tri->m_IndexA]);
		const Maths::HVec3 temp1(m_vertice[tri->m_IndexB]);
		const Maths::HVec3 temp2(m_vertice[tri->m_IndexC]);
		Maths::HVec3 min_t12 = Min(temp1, temp2);
		Maths::HVec3 max_t12 = Max(temp1, temp2);
		Maths::HVec3 min_t_t12 = Min(temp, min_t12);
		Maths::HVec3 max_t_t12 = Max(temp, max_t12);
		min_a = Min(min_t_t12, min_a);
		max_a = Max(max_t_t12, max_a);
	}
	node->aabb.m_min=(min_a - epsilon);
	node->aabb.m_max=(max_a + epsilon);
	return node;
}


Debug::Bool g_DrawSplitTrianglesAABB("Test.DrawAABBSplitTriangles", false);

//OK. Conditions - we need to ID which edges our new edges are on. If tey
U8 CalculateActiveEdgeFlag(U32 indexA, U32 indexB, U32 indexC, U32 newIndA, U32 newIndB, U32 newIndC, U8 activeEdgeFlags)
{
	U8 res = 0;
	//Edge 0
	if((indexA == newIndA || indexB == newIndB))
	{
		res |= activeEdgeFlags & 1;
	}
	else if (indexB == newIndA || indexC == newIndB)
	{
		res |= (activeEdgeFlags & 2) >> 1;
	}
	else if (indexC == newIndA || indexA == newIndB)
	{
		res |= (activeEdgeFlags & 4) >> 2;
	}

	//Edge1
	if((indexA == newIndB || indexB == newIndC))
	{
		res |= (activeEdgeFlags & 1) << 1;
	}
	else if (indexB == newIndB || indexC == newIndC)
	{
		res |= (activeEdgeFlags & 2);
	}
	else if (indexC == newIndB || indexA == newIndC)
	{
		res |= (activeEdgeFlags & 4) >> 1;
	}

	//Edge2
	if((indexA == newIndC || indexB == newIndA))
	{
		res |= (activeEdgeFlags & 1) << 2;
	}
	else if (indexB == newIndC || indexC == newIndA)
	{
		res |= (activeEdgeFlags & 2) << 1;
	}
	else if (indexC == newIndC || indexA == newIndA)
	{
		res |= (activeEdgeFlags & 4);
	}

	return res;
}


//#pragma optimize("", off)
//U32 AABBTree::SplitTriangle(U32& index, AABBBuildTreeTriangle& tri, AABBBuildTreeTriangle* leftTri, AABBBuildTreeTriangle* rightTri, const Collision::C_Plane& p, const U32 startIndex, const U32 endIndex,
//							SharedPhysics::C_DoubleListAlloc<AABBBuildTreeTriangle>& m_indice)
//{
//	using namespace Maths;
//	//AABBBuildTreeTriangle& tri(m_indice[index]);
//	const U32 indexA = tri.m_IndexA;
//	const U32 indexB = tri.m_IndexB;
//	const U32 indexC = tri.m_IndexC;
//
//	U32 temp0=tri.m_IndexA;
//	U32 temp1=tri.m_IndexB;
//	U32 temp2=tri.m_IndexC;
//	U32 physicsMaterial = tri.m_PhysicsMaterialIndex;
//	U32 effectMaterial = tri.m_EffectMaterialIndex;
//	U32 userDataFlag = tri.m_UserDataFlags;
//	U32 flags = tri.m_CollisionFlags;
//	U8 edgeFlags = tri.m_ActiveEdgeFlags;
//
//	const Maths::HF32 epi(Maths::Constants<HF32>::Epsilon());
//	const Maths::HF32 zero(Maths::Constants<HF32>::Zero());
//
//	const Maths::HVec3 a(m_vertice[temp0]);
//	const Maths::HVec3 b(m_vertice[temp1]);
//	const Maths::HVec3 c(m_vertice[temp2]);
//
//	const HF32 sa((p.m_n|a)-p.m_d);
//	const HF32 sb((p.m_n|b)-p.m_d);
//	const HF32 sc((p.m_n|c)-p.m_d);
//
//
//	Maths::HVec3 t0(a);
//	Maths::HVec3 t1(b);
//	Maths::HVec3 t2(c);
//	Maths::HF32  st0(sa);
//	Maths::HF32	 st1(sb);
//	Maths::HF32  st2(sc);
//
//	if((sa*sb).GE(zero))
//	{
//		//point c is the point in different side
//		t0=c;
//		t2=a;
//		U32 temp=temp0;
//		temp0=temp2;
//		temp2=temp;
//
//		HF32 st(st0);
//		st0=st2;
//		st2=st;
//	}
//	else if((sa*sc).GE(zero))
//	{
//		//point b is the point in different side
//		t0=b;
//		t1=a;
//		U32 temp=temp0;
//		temp0=temp1;
//		temp1=temp;
//
//		HF32 st(st0);
//		st0=st1;
//		st1=st;
//	}
//
//	const Maths::HVec3 t0t1(t1-t0);
//	const Maths::HVec3 t0t2(t2-t0);
//
//	const Maths::HVec3 ab(b-a);
//	const Maths::HVec3 ac(c-a);
//	const Maths::HVec3 tp(t0t1^t0t2);
//	const Maths::HVec3 op(ab^ac);
//	const Maths::HF32 res(tp|op);
//
//	AABBBuildTreeTriangle front;
//	AABBBuildTreeTriangle mid;
//	AABBBuildTreeTriangle back;
//
//	AABBBuildTreeTriangle* pFront=&front;
//	AABBBuildTreeTriangle* pBack=&back;
//	AABBBuildTreeTriangle* pMid=NULL;
//
//	bool midInFront=true;
//
//	if(st1.Abs().LT(epi))
//	{
//		//t1 is one of the split point
//		Maths::HVec3 vmid(Maths::UNDEFINED);
//		//assert(IntersectSegmentPlane(t0, t2, p, vmid);
//		/*if(!IntersectSegmentPlane(t0, t2, p, vmid))
//			return 0;*/
//		bool inter = IntersectSegmentPlane(t0, t2, p, vmid);
//		BIZ_ASSERT(inter);
//		m_vertice.AddToEnd(&vmid);
//		U32 temp3=m_vertice.m_numElements-1;
//
//		//if tp and op in the same direction, res is 1, if tp and op oppisite direction, res is -1
//		if(res.GT(zero))
//		{
//			//the wiring order is the same as the original triangle
//			front.m_IndexA=temp3; front.m_IndexB=temp1; front.m_IndexC=temp2; 
//			front.m_PhysicsMaterialIndex = physicsMaterial; front.m_CollisionFlags = flags;//triangle mid_t1_t2
//			front.m_EffectMaterialIndex = effectMaterial;
//			front.m_UserDataFlags = userDataFlag;
//			//Calculate which edges are still existing edges and which edges are new (therefore not valid)
//			front.m_ActiveEdgeFlags = CalculateActiveEdgeFlag(indexA, indexB, indexC, front.m_IndexA, front.m_IndexB, front.m_IndexC, edgeFlags);
//			back.m_IndexA=temp3; back.m_IndexB=temp0; back.m_IndexC=temp1;//triangle mid_t0_t1
//			back.m_PhysicsMaterialIndex = physicsMaterial; back.m_CollisionFlags = flags;
//			back.m_EffectMaterialIndex = effectMaterial;
//			back.m_UserDataFlags = userDataFlag;
//			front.m_ActiveEdgeFlags = CalculateActiveEdgeFlag(indexA, indexB, indexC, back.m_IndexA, back.m_IndexB, back.m_IndexC, edgeFlags);
//		}
//		else
//		{
//			//the wiring order is not the same as the original triangle
//			front.m_IndexA=temp3; front.m_IndexB=temp1; front.m_IndexC=temp0;//triangle mid_t1_t0
//			front.m_PhysicsMaterialIndex = physicsMaterial; front.m_CollisionFlags = flags;
//			front.m_EffectMaterialIndex = effectMaterial;
//			front.m_UserDataFlags = userDataFlag;
//			front.m_ActiveEdgeFlags = CalculateActiveEdgeFlag(indexA, indexB, indexC, front.m_IndexA, front.m_IndexB, front.m_IndexC, edgeFlags);
//			back.m_IndexA=temp3; back.m_IndexB=temp2; back.m_IndexC=temp1;	//triangle mid_t2_t1
//			back.m_PhysicsMaterialIndex = physicsMaterial; back.m_CollisionFlags = flags;
//			back.m_EffectMaterialIndex = effectMaterial;
//			back.m_UserDataFlags = userDataFlag;
//			front.m_ActiveEdgeFlags = CalculateActiveEdgeFlag(indexA, indexB, indexC, back.m_IndexA, back.m_IndexB, back.m_IndexC, edgeFlags);
//		}
//
//	}
//	else if(st2.Abs().LT(epi))
//	{
//		//t2 is one of the split point
//		Maths::HVec3 vmid(Maths::UNDEFINED);
//		//assert(IntersectSegmentPlane(t0, t1, p, vmid));
//		bool inter = IntersectSegmentPlane(t0, t1, p, vmid);
//		BIZ_ASSERT(inter);
//		m_vertice.AddToEnd(&vmid);
//		U32 temp3=m_vertice.m_numElements-1;
//
//		//if tp and op in the same direction, res is 1, if tp and op oppisite direction, res is -1
//		if(res.GT(zero))
//		{
//			//the wiring order is the same as the original
//			front.m_IndexA=temp3; front.m_IndexB=temp1; front.m_IndexC=temp2; //triangle mid_t1_t2
//			front.m_PhysicsMaterialIndex = physicsMaterial; front.m_CollisionFlags = flags;
//			front.m_EffectMaterialIndex = effectMaterial;
//			front.m_UserDataFlags = userDataFlag;
//			front.m_ActiveEdgeFlags = CalculateActiveEdgeFlag(indexA, indexB, indexC, front.m_IndexA, front.m_IndexB, front.m_IndexC, edgeFlags);
//
//			back.m_IndexA=temp3; back.m_IndexB=temp2; back.m_IndexC=temp0; //triangle mid_t2_t0
//			back.m_PhysicsMaterialIndex = physicsMaterial; back.m_CollisionFlags = flags;
//			back.m_EffectMaterialIndex = effectMaterial;
//			back.m_UserDataFlags = userDataFlag;
//			front.m_ActiveEdgeFlags = CalculateActiveEdgeFlag(indexA, indexB, indexC, back.m_IndexA, back.m_IndexB, back.m_IndexC, edgeFlags);
//		}
//		else
//		{
//			//the wiring order is not the same as the original triangle
//			front.m_IndexA=temp3; front.m_IndexB=temp2; front.m_IndexC=temp1; //triangle mid_t2_t1
//			front.m_CollisionFlags = flags; front.m_PhysicsMaterialIndex = physicsMaterial;
//			front.m_EffectMaterialIndex = effectMaterial;
//			front.m_UserDataFlags = userDataFlag;
//			front.m_ActiveEdgeFlags = CalculateActiveEdgeFlag(indexA, indexB, indexC, front.m_IndexA, front.m_IndexB, front.m_IndexC, edgeFlags);
//
//			back.m_IndexA=temp3;back.m_IndexB=temp0;back.m_IndexC=temp2; //triangle mid_t0_t2
//			back.m_CollisionFlags = flags; back.m_PhysicsMaterialIndex = physicsMaterial;
//			back.m_EffectMaterialIndex = effectMaterial;
//			back.m_UserDataFlags = userDataFlag;
//			front.m_ActiveEdgeFlags = CalculateActiveEdgeFlag(indexA, indexB, indexC, back.m_IndexA, back.m_IndexB, back.m_IndexC, edgeFlags);
//		}
//	}
//	else
//	{
//	
//		Maths::HVec3 mid1(Maths::UNDEFINED);
//		Maths::HVec3 mid2(Maths::UNDEFINED);
//
//		//assert(IntersectSegmentPlane(t0, t1, p, mid1));
//		//assert(IntersectSegmentPlane(t0, t2, p, mid2));
//
//		bool int1 = IntersectSegmentPlane(t0, t1, p, mid1);
//		bool int2 = IntersectSegmentPlane(t0, t2, p, mid2);
//
//		BIZ_ASSERT(int1 && int2);
//
//		m_vertice.AddToEnd(&mid1);
//		m_vertice.AddToEnd(&mid2);
//
//		U32 temp3=m_vertice.m_numElements-2;
//		U32 temp4=m_vertice.m_numElements-1;
//
//		
//		pMid=&mid;
//		
//		//if tp and op in the same direction, res is 1, if tp and op oppisite direction, res is -1
//		//Assume t0 is behind the plane
//		if(res.GT(zero))
//		{
//			//the wiring order is the same as the original
//
//			front.m_IndexA=temp4; front.m_IndexB=temp1; front.m_IndexC=temp2;	//trinagle mid2_t1_t2
//			front.m_CollisionFlags = flags; front.m_PhysicsMaterialIndex = physicsMaterial;
//			front.m_EffectMaterialIndex = effectMaterial;
//			front.m_UserDataFlags = userDataFlag;
//			front.m_ActiveEdgeFlags = 0;//CalculateActiveEdgeFlag(indexA, indexB, indexC, front.m_IndexA, front.m_IndexB, front.m_IndexC, edgeFlags);
//
//			mid.m_IndexA=temp4;	mid.m_IndexB=temp3;	mid.m_IndexC=temp1;	 //triangle mid2_mid1_t1
//			mid.m_CollisionFlags = flags; mid.m_PhysicsMaterialIndex = physicsMaterial;
//			mid.m_EffectMaterialIndex = effectMaterial;
//			mid.m_UserDataFlags = userDataFlag;
//			front.m_ActiveEdgeFlags = 0;//CalculateActiveEdgeFlag(indexA, indexB, indexC, mid.m_IndexA, mid.m_IndexB, mid.m_IndexC, edgeFlags);
//
//			back.m_IndexA=temp4; back.m_IndexB=temp0; back.m_IndexC=temp3;	//triangle mid2_t0_mid1
//			back.m_CollisionFlags = flags; back.m_PhysicsMaterialIndex = physicsMaterial;
//			back.m_EffectMaterialIndex = effectMaterial;
//			back.m_UserDataFlags = userDataFlag;
//			front.m_ActiveEdgeFlags = 0;//CalculateActiveEdgeFlag(indexA, indexB, indexC, back.m_IndexA, back.m_IndexB, back.m_IndexC, edgeFlags);
//
//		}
//		else
//		{
//			//the wiring order is not the same as the original triangle
//
//			front.m_IndexA=temp4; front.m_IndexB=temp2; front.m_IndexC=temp1;	//trinagle mid2_t2_t1
//			front.m_CollisionFlags = flags; front.m_PhysicsMaterialIndex = physicsMaterial;
//			front.m_EffectMaterialIndex = effectMaterial;
//			front.m_UserDataFlags = userDataFlag;
//			front.m_ActiveEdgeFlags = 0;//CalculateActiveEdgeFlag(indexA, indexB, indexC, front.m_IndexA, front.m_IndexB, front.m_IndexC, edgeFlags);
//
//			mid.m_IndexA=temp4;	mid.m_IndexB=temp1;	mid.m_IndexC=temp3;	//triangle mid2_t1_mid1
//			mid.m_CollisionFlags = flags; mid.m_PhysicsMaterialIndex = physicsMaterial;
//			mid.m_EffectMaterialIndex = effectMaterial;
//			mid.m_UserDataFlags = userDataFlag;
//			front.m_ActiveEdgeFlags = 0;//CalculateActiveEdgeFlag(indexA, indexB, indexC, mid.m_IndexA, mid.m_IndexB, mid.m_IndexC, edgeFlags);
//
//			back.m_IndexA=temp4; back.m_IndexB=temp3;back.m_IndexC=temp0;	//triangle mid2_mid1_t0
//			back.m_CollisionFlags = flags; back.m_PhysicsMaterialIndex = physicsMaterial;
//			back.m_EffectMaterialIndex = effectMaterial;
//			back.m_UserDataFlags = userDataFlag;
//			front.m_ActiveEdgeFlags = 0;//CalculateActiveEdgeFlag(indexA, indexB, indexC, back.m_IndexA, back.m_IndexB, back.m_IndexC, edgeFlags);
//		}
//	}
//
// // if t0 is in front of the plane, swap the front and back pointer
//	if(st0.GT(zero)) 
//	{
//		pFront=&back;
//		pBack=&front;
//		midInFront=false;
//		
//	}
//
//	const RGBA black(0, 0, 0, 255);
//	const RGBA blue(0, 64, 255, 255);
//
//	//m_indice.Remove(index, 1);
//
//	//Insert the front triangle in the front of m_indice
//	m_indice.InsertAfter(leftTri->m_pPrev, pFront);
//	/*m_indice.Insert(&pFront[1], 1);
//	m_indice.Insert(&pFront[2], 2);*/
//
//	index+=1;
//
//	//if the triangle is split into three
//	U32 newInds = 0;
//	if(pMid)
//	{
//		newInds += 1;
//		if(midInFront)
//		{
//			//m_indice.Insert(pMid, startIndex);
//			m_indice.InsertAfter(leftTri->m_pPrev, pMid);
//			/*m_indice.Insert(&pMid[1], 4);
//			m_indice.Insert(&pMid[2], 5);*/
//			index+=1;
//		}
//		else
//		{
//			m_indice.InsertAfter(rightTri, pMid);
//			/*m_indice.AddToEnd(&pMid[1]);
//			m_indice.AddToEnd(&pMid[2]);*/
//		}
//	}
//
//	m_indice.InsertAfter(rightTri, pBack);
//
//	BIZ_ASSERT(m_indice.GetTail()->m_pNext == NULL);
//
//	m_indice.Remove(&tri);
//	BIZ_ASSERT(m_indice.GetTail()->m_pNext == NULL);
//	return newInds + 1;
//}
//#pragma optimize("", on)

U32 AABBTree::SplitTriangle(U32& index, AABBBuildTreeTriangle& tri, AABBBuildTreeTriangle* leftTri, AABBBuildTreeTriangle* rightTri, const Collision::C_Plane& p, const U32 startIndex, const U32 endIndex,
							SharedPhysics::C_DoubleListAlloc<AABBBuildTreeTriangle>& m_indice, const Maths::HF32 PCREF planeThickness)
{
	using namespace Maths;
	//AABBBuildTreeTriangle& tri(m_indice[index]);
	const U32 indexA = tri.m_IndexA;
	const U32 indexB = tri.m_IndexB;
	const U32 indexC = tri.m_IndexC;

	U32 temp0=tri.m_IndexA;
	U32 temp1=tri.m_IndexB;
	U32 temp2=tri.m_IndexC;
	U32 physicsMaterial = tri.m_PhysicsMaterialIndex;
	U32 effectMaterial = tri.m_EffectMaterialIndex;
	U32 userDataFlag = tri.m_UserDataFlags;
	U32 flags = tri.m_CollisionFlags;
	U8 edgeFlags = tri.m_ActiveEdgeFlags;

	const Maths::HF32 epi(Maths::Constants<HF32>::Epsilon());
	const Maths::HF32 zero(Maths::Constants<HF32>::Zero());

	const Maths::HVec3 a(m_vertice[temp0]);
	const Maths::HVec3 b(m_vertice[temp1]);
	const Maths::HVec3 c(m_vertice[temp2]);
	
	const Maths::HVec3 ab(b - a);
	const Maths::HVec3 bc(c - b);
	const Maths::HVec3 ca(a - c);

	const HF32 sa((p.m_n|a)-p.m_d);
	const HF32 sb((p.m_n|b)-p.m_d);
	const HF32 sc((p.m_n|c)-p.m_d);

	const HF32 absSa(Maths::Abs(sa));
	const HF32 absSb(Abs(sb));
	const HF32 absSc(Abs(sc));

	//Test to see if edge0 (a - b) crosses the plane...
	bool bInterSegA = false;
	bool bInterSegB = false;
	bool bInterSegC = false;

	U32 newPoints[3];
	U32 numNewPoints = 0;

	//Two conditions to consider...
	//(1)	One point on or near plane, one on positive side, one on negative side (in which case equal split 
	//		to 2 triangles
	//(2)	Two points on one side, one on the other, in which case, we're splitting to 3 triangles...


	if(absSa.LE(planeThickness))
	{
		//Point a is on the plane, so we need to intersect for points b and c
		if(absSb.LE(planeThickness) | absSc.LE(planeThickness))
			return 0; //didn't get a hit!!!

		if((sb * sc) >= zero)
			return 0;	//other edge doesn't cross plane !!!!

		bInterSegB = true;
		//Get intersection t-value...
		const Maths::HF32 distanceSegment(sc - sb);
		Maths::HVec3 intersectPoint(b - (bc) * (sb/distanceSegment));
		//Add a new point...
		newPoints[numNewPoints++] = m_vertice.m_numElements;
		m_vertice.AddToEnd(&intersectPoint);
		
	}
	else if (absSb.LE(planeThickness))
	{
		if(absSc.LE(planeThickness))
			return 0; //didn't get a hit!!!
		if((sa * sc) >= zero)
			return 0;	//other edge doesn't cross plane !!!!
		bInterSegC = true;
		//Get intersection t-value...
		const Maths::HF32 distanceSegment(sa - sc);
		Maths::HVec3 intersectPoint(c - (ca) * (sc/distanceSegment));
		//Add a new point...
		newPoints[numNewPoints++] = m_vertice.m_numElements;
		m_vertice.AddToEnd(&intersectPoint);

	}
	else if (absSc.LE(planeThickness))
	{
		if((sa * sb) >= zero)
			return 0;	//other edge doesn't cross plane !!!!
		bInterSegA = true;
		//Get intersection t-value...
		const Maths::HF32 distanceSegment(sb - sa);
		Maths::HVec3 intersectPoint(a - (ab) * (sa/distanceSegment));
		//Add a new point...
		newPoints[numNewPoints++] = m_vertice.m_numElements;
		m_vertice.AddToEnd(&intersectPoint);
	}
	else 
	{
		if((sa * sb) < zero)
		{
			bInterSegA = true;
			//Get intersection t-value...
			const Maths::HF32 distanceSegment(sb - sa);
			Maths::HVec3 intersectPoint(a - (ab) * (sa/distanceSegment));
			//Add a new point...
			newPoints[numNewPoints++] = m_vertice.m_numElements;
			m_vertice.AddToEnd(&intersectPoint);
		}
		if((sb * sc) < zero)
		{
			bInterSegB = true;
			//Get intersection t-value...
			const Maths::HF32 distanceSegment(sc - sb);
			Maths::HVec3 intersectPoint(b - (bc) * (sb/distanceSegment));
			//Add a new point...
			newPoints[numNewPoints++] = m_vertice.m_numElements;
			m_vertice.AddToEnd(&intersectPoint);
		}

		if((sc * sa) < zero)
		{
			bInterSegC = true;
			//Get intersection t-value...
			const Maths::HF32 distanceSegment(sa - sc);
			Maths::HVec3 intersectPoint(c - (ca) * (sc/distanceSegment));
			//Add a new point...
			newPoints[numNewPoints++] = m_vertice.m_numElements;
			m_vertice.AddToEnd(&intersectPoint);
		}
	}

	AABBBuildTreeTriangle front, back, mid;
	if(numNewPoints == 1)
	{
		//Only need to remap the two triangles we had passed in...
		if(bInterSegA)
		{
			front.m_IndexA=indexA; front.m_IndexB=newPoints[0]; front.m_IndexC=indexC; //triangle mid_t2_t1
			front.m_CollisionFlags = flags; front.m_PhysicsMaterialIndex = physicsMaterial;
			front.m_EffectMaterialIndex = effectMaterial;
			front.m_UserDataFlags = userDataFlag;
			front.m_ActiveEdgeFlags = edgeFlags & (~2);

			back.m_IndexA=newPoints[0]; back.m_IndexB=indexB; back.m_IndexC=indexC; //triangle mid_t0_t2
			back.m_CollisionFlags = flags; back.m_PhysicsMaterialIndex = physicsMaterial;
			back.m_EffectMaterialIndex = effectMaterial;
			back.m_UserDataFlags = userDataFlag;
			back.m_ActiveEdgeFlags = edgeFlags & (~4);
			if(sa >= zero)
			{
				m_indice.InsertAfter(leftTri->m_pPrev, &front);
				m_indice.InsertAfter(rightTri, &back);
			}
			else
			{
				m_indice.InsertAfter(leftTri->m_pPrev, &back);
				m_indice.InsertAfter(rightTri, &front);
			}
		}
		else if (bInterSegB)
		{
			front.m_IndexA=indexA; front.m_IndexB=indexB; front.m_IndexC=newPoints[0]; //triangle mid_t2_t1
			front.m_CollisionFlags = flags; front.m_PhysicsMaterialIndex = physicsMaterial;
			front.m_EffectMaterialIndex = effectMaterial;
			front.m_UserDataFlags = userDataFlag;
			front.m_ActiveEdgeFlags = edgeFlags & (~4);

			back.m_IndexA=indexA; back.m_IndexB=newPoints[0]; back.m_IndexC=indexC; //triangle mid_t0_t2
			back.m_CollisionFlags = flags; back.m_PhysicsMaterialIndex = physicsMaterial;
			back.m_EffectMaterialIndex = effectMaterial;
			back.m_UserDataFlags = userDataFlag;
			back.m_ActiveEdgeFlags = edgeFlags & (~1);
			if(sb >= zero)
			{
				m_indice.InsertAfter(leftTri->m_pPrev, &front);
				m_indice.InsertAfter(rightTri, &back);
			}
			else
			{
				m_indice.InsertAfter(leftTri->m_pPrev, &back);
				m_indice.InsertAfter(rightTri, &front);
			}	
		}
		else if (bInterSegC)
		{
			front.m_IndexA=indexA; front.m_IndexB=indexB; front.m_IndexC=newPoints[0]; //triangle mid_t2_t1
			front.m_CollisionFlags = flags; front.m_PhysicsMaterialIndex = physicsMaterial;
			front.m_EffectMaterialIndex = effectMaterial;
			front.m_UserDataFlags = userDataFlag;
			front.m_ActiveEdgeFlags = edgeFlags & (~2);

			back.m_IndexA=newPoints[0]; back.m_IndexB=indexB; back.m_IndexC=indexC; //triangle mid_t0_t2
			back.m_CollisionFlags = flags; back.m_PhysicsMaterialIndex = physicsMaterial;
			back.m_EffectMaterialIndex = effectMaterial;
			back.m_UserDataFlags = userDataFlag;
			back.m_ActiveEdgeFlags = edgeFlags & (~1);
			if(sa >= zero)
			{
				m_indice.InsertAfter(leftTri->m_pPrev, &front);
				m_indice.InsertAfter(rightTri, &back);
			}
			else
			{
				m_indice.InsertAfter(leftTri->m_pPrev, &back);
				m_indice.InsertAfter(rightTri, &front);
			}
		}
		
	}
	else if (numNewPoints == 2)
	{
		if(bInterSegA && bInterSegB)
		{
			//b is the odd-one out...
			front.m_IndexA=newPoints[0]; front.m_IndexB=indexB; front.m_IndexC=newPoints[1]; //triangle mid_t2_t1
			front.m_CollisionFlags = flags; front.m_PhysicsMaterialIndex = physicsMaterial;
			front.m_EffectMaterialIndex = effectMaterial;
			front.m_UserDataFlags = userDataFlag;
			front.m_ActiveEdgeFlags = edgeFlags & (~4);

			mid.m_IndexA=indexA; mid.m_IndexB=newPoints[0]; mid.m_IndexC=indexC; //triangle mid_t2_t1
			mid.m_CollisionFlags = flags; mid.m_PhysicsMaterialIndex = physicsMaterial;
			mid.m_EffectMaterialIndex = effectMaterial;
			mid.m_UserDataFlags = userDataFlag;
			mid.m_ActiveEdgeFlags = edgeFlags & 4;

			back.m_IndexA=newPoints[0]; back.m_IndexB=newPoints[1]; back.m_IndexC=indexC; //triangle mid_t0_t2
			back.m_CollisionFlags = flags; back.m_PhysicsMaterialIndex = physicsMaterial;
			back.m_EffectMaterialIndex = effectMaterial;
			back.m_UserDataFlags = userDataFlag;
			back.m_ActiveEdgeFlags = edgeFlags & 2;

			if(sb >= zero)
			{
				m_indice.InsertAfter(leftTri->m_pPrev, &front);
				m_indice.InsertAfter(rightTri, &mid);
				m_indice.InsertAfter(rightTri, &back);
			}
			else
			{
				m_indice.InsertAfter(rightTri, &front);
				m_indice.InsertAfter(leftTri->m_pPrev, &mid);
				m_indice.InsertAfter(leftTri->m_pPrev, &back);
			}

		}
		else if (bInterSegA && bInterSegC)
		{
			//a is the odd one out
			front.m_IndexA=indexA; front.m_IndexB=newPoints[0]; front.m_IndexC=newPoints[1]; //triangle mid_t2_t1
			front.m_CollisionFlags = flags; front.m_PhysicsMaterialIndex = physicsMaterial;
			front.m_EffectMaterialIndex = effectMaterial;
			front.m_UserDataFlags = userDataFlag;
			front.m_ActiveEdgeFlags = edgeFlags & (~2);

			mid.m_IndexA=newPoints[0]; mid.m_IndexB=indexB; mid.m_IndexC=indexC; //triangle mid_t2_t1
			mid.m_CollisionFlags = flags; mid.m_PhysicsMaterialIndex = physicsMaterial;
			mid.m_EffectMaterialIndex = effectMaterial;
			mid.m_UserDataFlags = userDataFlag;
			mid.m_ActiveEdgeFlags = edgeFlags & (~4);

			back.m_IndexA=newPoints[1]; back.m_IndexB=newPoints[0]; back.m_IndexC=indexC; //triangle mid_t0_t2
			back.m_CollisionFlags = flags; back.m_PhysicsMaterialIndex = physicsMaterial;
			back.m_EffectMaterialIndex = effectMaterial;
			back.m_UserDataFlags = userDataFlag;
			back.m_ActiveEdgeFlags = edgeFlags & 4;
			
			if(sa >= zero)
			{
				m_indice.InsertAfter(leftTri->m_pPrev, &front);
				m_indice.InsertAfter(rightTri, &mid);
				m_indice.InsertAfter(rightTri, &back);
			}
			else
			{
				m_indice.InsertAfter(rightTri, &front);
				m_indice.InsertAfter(leftTri->m_pPrev, &mid);
				m_indice.InsertAfter(leftTri->m_pPrev, &back);
			}
		}
		else if (bInterSegB && bInterSegC)
		{
			//c is the odd one out
			front.m_IndexA=newPoints[1]; front.m_IndexB=newPoints[0]; front.m_IndexC=indexC; //triangle mid_t2_t1
			front.m_CollisionFlags = flags; front.m_PhysicsMaterialIndex = physicsMaterial;
			front.m_EffectMaterialIndex = effectMaterial;
			front.m_UserDataFlags = userDataFlag;
			front.m_ActiveEdgeFlags = edgeFlags & (~1);

			mid.m_IndexA=indexA; mid.m_IndexB=indexB; mid.m_IndexC=newPoints[0]; //triangle mid_t2_t1
			mid.m_CollisionFlags = flags; mid.m_PhysicsMaterialIndex = physicsMaterial;
			mid.m_EffectMaterialIndex = effectMaterial;
			mid.m_UserDataFlags = userDataFlag;
			mid.m_ActiveEdgeFlags = edgeFlags & (~4);

			back.m_IndexA=indexA; back.m_IndexB=newPoints[0]; back.m_IndexC=newPoints[1]; //triangle mid_t0_t2
			back.m_CollisionFlags = flags; back.m_PhysicsMaterialIndex = physicsMaterial;
			back.m_EffectMaterialIndex = effectMaterial;
			back.m_UserDataFlags = userDataFlag;
			back.m_ActiveEdgeFlags = edgeFlags & 4;

			if(sc >= zero)
			{
				m_indice.InsertAfter(leftTri->m_pPrev, &front);
				m_indice.InsertAfter(rightTri, &mid);
				m_indice.InsertAfter(rightTri, &back);
			}
			else
			{
				m_indice.InsertAfter(rightTri, &front);
				m_indice.InsertAfter(leftTri->m_pPrev, &mid);
				m_indice.InsertAfter(leftTri->m_pPrev, &back);
			}
		}
	}
	else 
	{
		//If we get here, we've hit some weird numerical instability issue which I can't imagine would happen
		//unless we've been given a negative plane thickness
		return 0;
	}

	BIZ_ASSERT(bInterSegA || bInterSegB || bInterSegC);

	m_indice.Remove(&tri);

	return numNewPoints;
}


void AABBTree::RemoveDuplicateTriangles()
{
	//Loop through the list of triangles and remove duplicates!
	for(U32 a = 0; a < _m_indice.Size(); ++a)
	{
		for(U32 b = a + 1; b < _m_indice.Size(); ++b)
		{
			if(_m_indice[a].SameIndices(_m_indice[b]))
			{
				_m_indice[a].MergeTriangle(_m_indice[b]);
				_m_indice.Remove(b, 1);
				b--;
			}
		}
	}
}


void AABBTree::BuildAABBTree(const Maths::HF32 PCREF normalThreshold, const F32 triangleIncreaseThreshold)
{
#if 1 //CFG_CONVERTER

	if(m_vertice.Size() == 0 || _m_indice.Size() == 0)
	{
		//LOGDEBUG(g_Log, "AABBTree::No vertice inside the triangle mesh!!!");
		return;
	}
//	COUT("Removing Duplicate vertices...Original vert count = " << m_vertice.m_numElements);
	//LOGDEBUG(g_Log, "Removing Duplicate vertices...Original vert count = %i\n", m_vertice.Size());
	RemoveDuplicatedVertices();
	//LOGDEBUG(g_Log, "Removed Duplicate vertices...New vert count = %i\n", m_vertice.Size());
	//LOGDEBUG(g_Log, "Removing degenerate triangles...original triangle count = %i\n", _m_indice.Size());
	RemoveDegeneratedTriangle();
	if(_m_indice.Size() == 0)
	{
		return;
	}
	/*COUT("Removing duplicate triangles - original triangle count " << m_indice.m_numElements);

	
	COUT("Finished removing duplicate triangles - new triangle count " << m_indice.m_numElements);*/
	//LOGDEBUG(g_Log, "Building AABB Tree... - original triangle count %i\n", _m_indice.Size());
	//RemoveDuplicateTriangles();

	CalculateFaceNormals();

	CalculateActiveEdgeFlags(normalThreshold);

	S32 maxNewTriangles = ((S32)(_m_indice.Size() * triangleIncreaseThreshold)) + 1;
	m_pRoot = BuildAABBTree(0, _m_indice.Size(), 0, maxNewTriangles, _m_indice);

	//Remove any vertices which have been duped when splitting polygons...
	RemoveDuplicatedVertices();
	//LOGDEBUG(g_Log, "Finished build - triangle count now %i\n", _m_indice.Size());
	//LOGDEBUG(g_Log, "Calculating face normals...\n");
	CalculateFaceNormals();
	//LOGDEBUG(g_Log, "Calculating active edge flags...\n");

	CopyTrianglesToIndexArray();

	RemapVerticesToLeafNodes();

	////LOGDEBUG(g_Log, "Transfer data to stream...\n");
	TransferDataToStream();
	////LOGDEBUG(g_Log, "Finished build!\n");
	ValidateTree(m_pRoot);
#endif
}


void AABBTree::CompressAABBTreeData()
{
	RemapVerticesToLeafNodes();
	//LOGDEBUG(g_Log, "Transfer data to stream...\n");
	TransferDataToStream();
	//LOGDEBUG(g_Log, "Finished build!\n");
	ValidateTree(m_pRoot);
}

F32 Power(F32 value, S32 power)
{
	F32 result = value;
	for(S32 a = 0; a < power - 1; ++a)
	{
		result = result * value;
	}
	return result;
}

void AABBTree::RemoveDegeneratedTriangle()
{
	using namespace Maths;

	//SharedPhysics::SortedArray<AABBBuildTreeTriangle> newIndice;
	const HF32 epi(0.000005f);
	SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter(_m_indice);
	while(iter.HasNext())
	{
		AABBBuildTreeTriangle& tri = *iter.Next();
		HVec3& a=m_vertice[tri.m_IndexA];
		HVec3& b=m_vertice[tri.m_IndexB];
		HVec3& c=m_vertice[tri.m_IndexC];

		HVec3 ab(b - a);
		ab.Normalise();
		Maths::HVec3 ac(c-a);
		ac.Normalise();
		const HVec3 cr(ab^ac);
		const HF32 res(cr | cr);
		

		//The triangle isn't degenerated, remove it from the mesh
		//if(res < epi)
		if(tri.m_IndexA == tri.m_IndexB || 
			tri.m_IndexA == tri.m_IndexC || 
			tri.m_IndexB == tri.m_IndexC || 
			res <= epi)
		{
			//newIndice.AddToEnd(&tri);
			iter.Remove();
			//newIndice.AddToEnd(&m_indice[i+1]);
			//newIndice.AddToEnd(&m_indice[i+2]);
		}
	}

	/*delete [] m_indice.m_pElems;
	m_indice.m_pElems=newIndice.m_pElems;
	m_indice.m_numElements = newIndice.m_numElements;
	m_indice.m_size = newIndice.m_size;
	newIndice.m_pElems = NULL;*/
}

void AABBTree::CalculateFaceNormals()
{
	using namespace SharedPhysics;
	//const Maths::HF32 closeNormal(Maths::Constants<Maths::HF32>::One());
	const Maths::HF32 closeNormal(1.f);
	m_Normals.m_numElements = 0;

	SharedPhysics::SortedArray<C_TreeVector> sortedNorms(_m_indice.Size());
	U32 *remappedNormArray = new U32[_m_indice.Size()];

	//Loop through all triangles and calculate face normal.
	{
		SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter(_m_indice);
		while(iter.HasNext())
		{
			AABBBuildTreeTriangle& tri = *iter.Next();
			const Maths::HVec3 ab(m_vertice[tri.m_IndexB] - m_vertice[tri.m_IndexA]);
			const Maths::HVec3 ac(m_vertice[tri.m_IndexC] - m_vertice[tri.m_IndexA]);
			Maths::C_Normal norm(ab ^ ac);
			/*bool found = false;
			for(S32 b = 0; b < m_Normals.m_numElements; ++b)
			{
				const Maths::HF32 dif(norm | m_Normals[b]);
				if(dif >= closeNormal)
				{
					tri.m_NormalIndex = b;
					found = true;
					break;
				}
			}*/
			//if(!found)
			{
				tri.m_NormalIndex = sortedNorms.m_numElements;
				C_TreeVector vector;
				vector.m_Index = sortedNorms.m_numElements;
				vector.m_Vector = norm.m_V;
				sortedNorms.AddToEnd(&vector);
				//m_Normals.AddToEnd(&norm.m_V);
				//m_Normals.AddToEnd(&norm.m_V);
				//tri.m_NormalIndex = a++;
			}
		}
	}

	//New algorithm - 
	//(1) Calculate all face normals and put in an array.
	//(2) Sort a remap array of the normals using a fast sort (e.g. quicksort)
	//(3) Remove duplicates using a linear algorithm, then generate normal indices


	C_S32SafeStack stack(16092);
	sortedNorms.QuickSort(stack, 10);

	m_Normals.m_numElements = 0;
	
	//m_vertice.AddToEnd(&sortedVerts[0].m_Vertex);
	for(S32 a = 0; a < sortedNorms.m_numElements; ++a)
	{
		C_TreeVector& v0 = sortedNorms[a];
		v0.m_RemappedIndex = m_Normals.m_numElements;
		remappedNormArray[v0.m_Index] = m_Normals.m_numElements;
		m_Normals.AddToEnd(&v0.m_Vector);
		S32 b = a + 1;
		for(; b < sortedNorms.m_numElements; ++b)
		{
			C_TreeVector& v1 = sortedNorms[b];
			if(v0 == v1)
			{
				v1.m_RemappedIndex = v0.m_RemappedIndex;
				remappedNormArray[v1.m_Index] = v0.m_RemappedIndex;
			}
			else
				break;
		}
		a = b - 1;
	}

	C_DoubleAllocIterator<Baking::AABBBuildTreeTriangle> iter(_m_indice);

	while(iter.HasNext())
	{
		Baking::AABBBuildTreeTriangle& tri = *iter.Next();
		tri.m_NormalIndex = remappedNormArray[tri.m_NormalIndex];
	}

	delete [] remappedNormArray;


}


U32 AABBTree::CountVertices(AABBTreeNode* pNode, SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter)
{
	//For now, just return 3 * num triangles!!! Later, do more clever stuff here...
	U32 indices[257];
	U32 numIndices = 0;
	U32 endIndex = pNode->m_TriangleSizeIndice + pNode->m_TriangleStartIndex; 
	for(U32 a = pNode->m_TriangleStartIndex; a < endIndex; ++a)
	{
		AABBBuildTreeTriangle* tri = iter.Next();
		bool bFoundA = false, bFoundB = false, bFoundC = false;
		U32 indA = tri->m_IndexA;
		U32 indB = tri->m_IndexB;
		U32 indC = tri->m_IndexC;
		for(U32 b = 0; b < numIndices; ++b)
		{
			if(indices[b] == indA)
			{
				bFoundA = true;
			}
			else if(indices[b] == indB)
			{
				bFoundB = true;
			}
			else if(indices[b] == indC)
			{
				bFoundC = true;
			}
		}
		if(!bFoundA)
		{
			indices[numIndices++] = indA;
			if(numIndices == 257)
				break;
		}
		if(!bFoundB)
		{
			indices[numIndices++] = indB;
			if(numIndices == 257)
				break;
		}
		if(!bFoundC)
		{
			indices[numIndices++] = indC;
			if(numIndices == 257)
				break;
		}
	}
	//return pNode->m_TriangleSizeIndice * 3;
	return numIndices;
	
}

AABBTreeNode* AABBTree::BuildAABBTree(const U32 startIndex, const U32 _size, const U32 depth, S32& maxNewTriangles,
									  SharedPhysics::C_DoubleListAlloc<AABBBuildTreeTriangle>& m_indice)
{
	BIZ_ASSERT(m_indice.m_List.m_pTail->m_pNext == NULL);
	/*static F32 percentComplete = 0.f;
	static U32 highestPercent = 0;
	if(depth == 0)
	{
		percentComplete = 0.f;
		highestPercent = 0;
	}*/
	using namespace Maths;
	U32 size = _size;

	/*if(U32(percentComplete*100.f) > highestPercent)
	{
		highestPercent = (U32)(percentComplete * 100.f);
		LOGINFO(g_Log, "Percent complete = %i\n", highestPercent);
	}*/

	//no more Triangle
	//if(size==0)return NULL;

	int numTriangle=size;

	SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter(m_indice);

	//Test to see if met termination criteria!
	AABBTreeNode* node=CreateAABBNode(startIndex, size, iter);
	if((depth >=m_maxDepth && CountVertices(node, iter) <= 256) || numTriangle <=MIN_LEAF_SIZE)
	{
		//assert(numTriangle < 32);
		//percentComplete += 1.f/Power(2.f, (S32)depth);
		return node;
	}

	

	const HVec3 epsilon(Maths::Constants<HVec3>::Epsilon());
	const HVec3 dd((node->aabb.m_max - node->aabb.m_min) + epsilon);
	const HF32 volume(dd.X() * dd.Y() * dd.Z());
	const HF32 recipVolume(volume.Reciprocal());

	//const Maths::HVec3 zero(Maths::Constants<HVec3>::Zero());
	const HF32 half(Maths::Constants<HF32>::Half());

	/*const Maths::HVec3 c((node->aabb.m_max + node->aabb.m_min)*half);
	const Maths::HVec3 t((node->aabb.m_max - c) * m_splitRatio);*/
	Maths::HF32 threshold(Maths::UNDEFINED);

	F32 score1;

	Collision::C_Plane splitPlane = PickASeperatedAlignAxisPlane(iter, startIndex, size, node->aabb, score1, recipVolume, threshold);

	U32 endIndex=startIndex+size;
	
	U32 i=startIndex;
	U32 j=endIndex-1;
	U32 splitIndex=startIndex;
	U32 numFront = 0;
	U32 numBack = 0;
	U32 numSplit = 0;

	//while(i<j)
	{
		for(U32 a = startIndex; a < endIndex; ++a)
		{
			AABBBuildTreeTriangle& tri = *iter.Next();
			switch(GetSplitStatus(a, tri, splitPlane,threshold ))
			{
			case POINT_IN_FRONT_PLANE:
				{
					++numFront;
					break;
				}
				case POINT_IN_BACK_PLANE:
				{	
					++numBack;
					break;
				}
				default: //point straddle the plane
				{
					++numSplit;
					break;
				}
			}
		}
	}

	//Analyse how the split lies!!!
	if((2 * numSplit + numFront) > _size || (2 * numSplit + numBack) > _size)
	{
		//We're potentially going to create a child with more polys than this node!!!
		if(numTriangle < 16)
			return node;
	}


	i=startIndex;
	j=endIndex-1;

	AABBBuildTreeTriangle* triI = m_indice.Get(i - startIndex);
	BIZ_ASSERT(triI != NULL);
	AABBBuildTreeTriangle* triJ = m_indice.Get(j - startIndex);
	BIZ_ASSERT(triJ != NULL);

	while(i<j)
	{
		bool inBackPlane=false;
		while(!inBackPlane && i<j)
		{
		
			switch(GetSplitStatus(i, *triI, splitPlane,threshold ))
			{
			case POINT_IN_FRONT_PLANE:
				{
					i++;
					triI = triI->m_pNext;
					splitIndex=i;
					break;
				}
			case POINT_IN_BACK_PLANE:
				{	
					inBackPlane=true;
					break;
				}
			default: //point straddle the plane
				{
					////Split the triangle and update i
					//triI = triI->m_pNext;
					AABBBuildTreeTriangle* tri(triI);
					
					if(maxNewTriangles > 0)
					{
						U32 newInds = SplitTriangle(i, *tri, tri, triJ,splitPlane, startIndex, endIndex,
							m_indice, threshold);
						maxNewTriangles -= newInds;
						j+= newInds;
						endIndex += newInds;
						size += newInds;
						node->m_TriangleSizeIndice += newInds;
						splitIndex=i;
						i++;
						triI = m_indice.Get(i - startIndex);
					}
					else
					{
						i++;
						triI = triI->m_pNext;
					}
					
					break;
				}
			}
		}
		bool inFrontPlane=false;
		while(!inFrontPlane && i < j)
		{
			switch(GetSplitStatus(j, *triJ, splitPlane,threshold))
			{
			case POINT_IN_FRONT_PLANE:
				{
					inFrontPlane=true;
					break;
				}
			case POINT_IN_BACK_PLANE:
				{	
					splitIndex=j;
					j--;
					triJ = triJ->m_pPrev;
					break;
				}
			default: //point straddle the plane
				{
					//split the triangle and update j
					//AABBBuildTreeTriangle* tri(triJ);
					//triJ = triJ;
					//assert(triJ != NULL);
					if(maxNewTriangles > 0)
					{
						
						U32 newInds = SplitTriangle(j, *triJ, triI, triJ, splitPlane, startIndex, endIndex,
							m_indice, threshold);
						maxNewTriangles -= newInds;
						endIndex += newInds;
						//i += (newInds - 1);
						splitIndex=j;
						node->m_TriangleSizeIndice += newInds;
						j--;
						triJ = m_indice.Get(j - startIndex);
					}
					else
					{
						j--;
						triJ = triJ->m_pPrev;
					}
					break;
				}
			}
		}
		/*U32 t0=m_indice[i];
		U32 t1=m_indice[i+1];
		U32 t2=m_indice[i+2];*/
		if(i < j)
		{
			/*AABBBuildTreeTriangle tri = m_indice[i];
			m_indice[i] = m_indice[j];
			m_indice[j] = tri;*/
			m_indice.Swap(triI, triJ);
			BIZ_ASSERT(m_indice.GetTail()->m_pNext == NULL);
			AABBBuildTreeTriangle* temp = triI;
			triI = triJ;
			triJ = temp;
		}
		//i+=3;
		//j-=3;
		
	}


	//Error - can't split any further!
	if(splitIndex >= endIndex || splitIndex == startIndex)
	{
		//assert(numTriangle < 32);
		return node;
	}

	SharedPhysics::C_DoubleListAlloc<AABBBuildTreeTriangle> listA(m_indice.m_pAllocator);
	SharedPhysics::C_DoubleListAlloc<AABBBuildTreeTriangle> listB(m_indice.m_pAllocator);

	const U32 mid = splitIndex - startIndex;

	/*{
		const U32 count = m_indice.Size();
		m_indice.SplitLists(listA, listB, mid - 1);
		assert(listA.CountElements() == listA.Size());
		assert(listB.CountElements() == listB.Size());
		m_indice.MergeLists(listA, listB);
		assert(count == m_indice.Size());
		assert(m_indice.CountElements() == count);
	}*/
	BIZ_ASSERT(m_indice.GetTail()->m_pNext == NULL);
	m_indice.SplitLists(listA, listB, mid - 1);
	
	
	const U32 right = endIndex - splitIndex;
	node->m_leftNode=BuildAABBTree(startIndex, mid, depth+1, maxNewTriangles, listA);
	node->m_TriangleSizeIndice = node->m_leftNode->m_TriangleSizeIndice + right;
	const U32 rightStart = node->m_TriangleStartIndex + (node->m_TriangleSizeIndice - right);
	node->m_rightNode=BuildAABBTree(rightStart, right, depth+1, maxNewTriangles, listB);
	node->m_TriangleSizeIndice = node->m_leftNode->m_TriangleSizeIndice + node->m_rightNode->m_TriangleSizeIndice;

	m_indice.MergeLists(listA, listB);
	/*const U32 count = m_indice.CountElements();
	assert(count == m_indice.Size());*/

	//assert(node->m_leftNode->aabb.Inside(node->aabb));
	//assert(node->m_rightNode->aabb.Inside(node->aabb));
	return node;
}

//AABBTreeNode* AABBTree::BuildAABBTree(const U32 startIndex, const U32 _size, const U32 depth)
//{
//	using namespace Maths;
//	U32 size = _size;
//	//no more Triangle
//	//if(size==0)return NULL;
//
//	int numTriangle=size;
//
//	AABBTreeNode* node=CreateAABBNode(startIndex, size);
//	if(depth >=m_maxDepth || numTriangle <=MIN_LEAF_SIZE)
//	{
//	//	assert(numTriangle < 256);
//		return node;
//	}
//
//	
//
//	const HVec3 epsilon(Maths::Constants<HVec3>::Epsilon());
//	const HVec3 dd((node->aabb.m_max - node->aabb.m_min) + epsilon);
//	const HF32 volume(dd.X() * dd.Y() * dd.Z());
//	const HF32 recipVolume(volume.Reciprocal());
//
//	//const Maths::HVec3 zero(Maths::Constants<HVec3>::Zero());
//	const HF32 half(Maths::Constants<HF32>::Half());
//
//	const Maths::HVec3 c((node->aabb.m_max + node->aabb.m_min)*half);
//	const Maths::HVec3 t((node->aabb.m_max - c) * m_splitRatio);
//	const Maths::HF32 threshold(t|t);
//
//	//F32 score0;
//	F32 score1;
//	//F32 score2;
//
//	//Something wrong on this method
//	/*Plane splitPlane= PickASeperatedSurfacePlane(startIndex, size, score0, recipVolume, threshold);
//
//	Plane p1 = PickASeperatedAlignAxisPlane(startIndex, size, node->aabb, score1, recipVolume, threshold);
//
//	//The problem is causing by this method
//	Plane p2 = PickASeperatedEdgePlane(startIndex, size, score2, recipVolume, threshold);
//
//	if(score1 < score0 && score1< score2)
//	{
//		splitPlane=p1;
//	}
//	else if(score2 < score0)
//	{
//		splitPlane=p2;
//	}*/
//
//	Collision::C_Plane splitPlane = PickASeperatedAlignAxisPlane(startIndex, size, node->aabb, score1, recipVolume, threshold);
//	
//	
//
//	U32 endIndex=startIndex+size;
//	
//	U32 i=startIndex;
//	U32 j=endIndex-1;
//	U32 splitIndex=0;
//	U32 numFront = 0;
//	U32 numBack = 0;
//	U32 numSplit = 0;
//
//	//while(i<j)
//	for(U32 a = startIndex; a < endIndex; ++a)
//	{
//		switch(GetSplitStatus(a, splitPlane,threshold ))
//		{
//		case POINT_IN_FRONT_PLANE:
//			{
//				++numFront;
//				break;
//			}
//			case POINT_IN_BACK_PLANE:
//			{	
//				++numBack;
//				break;
//			}
//			default: //point straddle the plane
//			{
//				++numSplit;
//				break;
//			}
//		}
//	}
//
//	//Analyse how the split lies!!!
//	if((2 * numSplit + numFront) > _size || (2 * numSplit + numBack) > _size)
//	{
//		//We're going to create a child with more polys than this node!!!
//		if(numTriangle < 256)
//			return node;
//	}
//
//
//	i=startIndex;
//	j=endIndex-1;
//
//	while(i<j)
//	{
//		bool inBackPlane=false;
//		while(!inBackPlane && i<=j)
//		{
//		
//			switch(GetSplitStatus(i, splitPlane,threshold ))
//			{
//			case POINT_IN_FRONT_PLANE:
//				{
//					i++;
//					splitIndex=i;
//					break;
//				}
//			case POINT_IN_BACK_PLANE:
//				{	
//					inBackPlane=true;
//					break;
//				}
//			default: //point straddle the plane
//				{
//					////Split the triangle and update i
//					U32 newInds = SplitTriangle(i, splitPlane, startIndex, endIndex);
//					j+= newInds;
//					endIndex += newInds;
//					size += newInds;
//					node->m_sizeIndice += newInds;
//					splitIndex=i;
//					i++;
//					break;
//				}
//			}
//		}
//		bool inFrontPlane=false;
//		while(!inFrontPlane && i < j)
//		{
//			switch(GetSplitStatus(j, splitPlane,threshold))
//			{
//			case POINT_IN_FRONT_PLANE:
//				{
//					inFrontPlane=true;
//					break;
//				}
//			case POINT_IN_BACK_PLANE:
//				{	
//					splitIndex=j;
//					j--;
//					break;
//				}
//			default: //point straddle the plane
//				{
//					//split the triangle and update j
//					U32 newInds = SplitTriangle(j, splitPlane, startIndex, endIndex);
//					endIndex += newInds;
//					//i += (newInds - 1);
//					splitIndex=j;
//					node->m_sizeIndice += newInds;
//					j--;
//					break;
//				}
//			}
//		}
//		/*U32 t0=m_indice[i];
//		U32 t1=m_indice[i+1];
//		U32 t2=m_indice[i+2];*/
//		if(i < j)
//		{
//			AABBBuildTreeTriangle tri = m_indice[i];
//			m_indice[i] = m_indice[j];
//			m_indice[j] = tri;
//		}
//		//i+=3;
//		//j-=3;
//		
//	}
//
//
//	if(splitIndex >= endIndex || splitIndex == startIndex)
//	{
//		//assert(numTriangle < 256);
//		return node;
//	}
//
//	const U32 mid = splitIndex - startIndex;
//	const U32 right = endIndex - splitIndex;
//	node->m_leftNode=BuildAABBTree(startIndex, mid, depth+1);
//	node->m_sizeIndice = node->m_leftNode->m_sizeIndice + right;
//	const U32 rightStart = node->m_startIndex + (node->m_sizeIndice - right);
//	node->m_rightNode=BuildAABBTree(rightStart, right, depth+1);
//	node->m_sizeIndice = node->m_leftNode->m_sizeIndice + node->m_rightNode->m_sizeIndice;
//
//	//assert(node->m_leftNode->aabb.Inside(node->aabb));
//	//assert(node->m_rightNode->aabb.Inside(node->aabb));
//	return node;
//}

U32 CalculateNumDescendants(AABBTreeNode* node)
{
	if(node->m_leftNode == NULL)
	{
		node->m_NumDescendants = 0;
	}
	else
	{
		node->m_NumDescendants = CalculateNumDescendants(node->m_leftNode) + CalculateNumDescendants(node->m_rightNode);
	}
	return node->m_NumDescendants + 1;
}

void AABBTree::CaclulateNumChildrenNodes()
{
	AABBTreeNode* node = this->m_pRoot;
	if(node->m_leftNode == NULL)
		node->m_NumDescendants = 0;
	else
		node->m_NumDescendants = CalculateNumDescendants(node->m_leftNode) + CalculateNumDescendants(node->m_rightNode);
}

U32 AABBTree::CountNumNodes(AABBTreeNode* pNode)
{
	if(!pNode)
		return 0;
	if(pNode->m_leftNode == NULL)
		return 1;
	return 1 + CountNumNodes(pNode->m_leftNode) + CountNumNodes(pNode->m_rightNode);
}

class C_TriangleEdge
{
public:
	U32 m_IndexA, m_IndexB;
	S32 m_TriangleIndex[2]; //Should only be used in 2 triangles...
	U32 m_NumTriangles;
	S32 m_ParentEdgeIndex;
	bool bDeleted;

	C_TriangleEdge() : m_IndexA(0), m_IndexB(0), m_NumTriangles(0), bDeleted(false), m_ParentEdgeIndex(-1)
	{
		m_TriangleIndex[0] = m_TriangleIndex[1] = -1;
	}

	C_TriangleEdge(U32 indexA, U32 indexB, U32 triangleIndex) : m_ParentEdgeIndex(-1)
	{
		m_IndexA = Maths::Min(indexA, indexB);
		m_IndexB = Maths::Max(indexA, indexB);
		m_TriangleIndex[0] = triangleIndex;
		m_TriangleIndex[1] = -1;
		m_NumTriangles = 1;
		bDeleted = false;
	}

	C_TriangleEdge(const C_TriangleEdge& other)
	{
		m_IndexA = other.m_IndexA;
		m_IndexB = other.m_IndexB;
		m_TriangleIndex[0] = other.m_TriangleIndex[0];
		m_TriangleIndex[1] = other.m_TriangleIndex[1];
		m_NumTriangles = other.m_NumTriangles;
		m_ParentEdgeIndex = other.m_ParentEdgeIndex;
		bDeleted = other.bDeleted;
	}

	C_TriangleEdge& operator = (const C_TriangleEdge& other)
	{
		m_IndexA = other.m_IndexA;
		m_IndexB = other.m_IndexB;
		m_TriangleIndex[0] = other.m_TriangleIndex[0];
		m_TriangleIndex[1] = other.m_TriangleIndex[1];
		m_NumTriangles = other.m_NumTriangles;
		m_ParentEdgeIndex = other.m_ParentEdgeIndex;
		bDeleted = other.bDeleted;
		return *this;
	}

	void SetEdge(U32 indexA, U32 indexB, U32 triangleIndex)
	{
		m_IndexA = Maths::Min(indexA, indexB);
		m_IndexB = Maths::Max(indexA, indexB);
		m_TriangleIndex[0] = triangleIndex;
		m_TriangleIndex[1] = -1;
		m_NumTriangles = 1;
	}

	void AddTriangle(U32 triangleIndex)
	{
		//assert(m_NumTriangles < 2);
		if(m_NumTriangles < 2)
		{
			m_TriangleIndex[m_NumTriangles] = triangleIndex;
		}

		++m_NumTriangles;
	}

	NOALIAS __forceinline bool operator < (C_TriangleEdge& other)
	{
		return m_IndexA < other.m_IndexA || (m_IndexA == other.m_IndexA && m_IndexB < other.m_IndexB);
	}

	NOALIAS __forceinline bool operator <= (C_TriangleEdge& other)
	{
		return m_IndexA < other.m_IndexA || (m_IndexA == other.m_IndexA && m_IndexB <= other.m_IndexB);
	}

	NOALIAS __forceinline bool operator > (C_TriangleEdge& other)
	{
		return m_IndexA > other.m_IndexA || (m_IndexA == other.m_IndexA && m_IndexB > other.m_IndexB);
	}

	NOALIAS __forceinline bool operator >= (C_TriangleEdge& other)
	{
		return m_IndexA > other.m_IndexA || (m_IndexA == other.m_IndexA && m_IndexB >= other.m_IndexB);
	}

	NOALIAS __forceinline bool operator == (C_TriangleEdge& other)
	{
		return m_IndexA == other.m_IndexA && m_IndexB == other.m_IndexB;
	}

	NOALIAS __forceinline bool operator != (C_TriangleEdge& other)
	{
		return m_IndexA != other.m_IndexA || m_IndexB != other.m_IndexB;
	}
};


bool AABBTree::MatchNormal(AABBBuildTreeTriangle& tri)
{
	const Maths::HVec3 v0(m_vertice[tri.m_IndexA]);
	const Maths::HVec3 v1(m_vertice[tri.m_IndexB]);
	const Maths::HVec3 v2(m_vertice[tri.m_IndexC]);

	const Maths::HVec3 ab(v1 - v0);
	const Maths::HVec3 ac(v2 - v0);

	const Maths::C_Normal norm(ab ^ ac);
	return((norm | m_Normals[tri.m_NormalIndex]) >= 0.99f) != 0;
}

bool AABBTree::MatchNormal(Collision::AABBTreeTriangle& tri)
{
	const Maths::HVec3 v0(m_vertice[tri.m_IndexA]);
	const Maths::HVec3 v1(m_vertice[tri.m_IndexB]);
	const Maths::HVec3 v2(m_vertice[tri.m_IndexC]);

	const Maths::HVec3 ab(v1 - v0);
	const Maths::HVec3 ac(v2 - v0);

	const Maths::C_Normal norm(ab ^ ac);
	return((norm | m_Normals[tri.m_SurfaceNormal.v]) >= 0.99f) != 0;
}


int FindVerticeIndex(SharedPhysics::SortedArray<Maths::HVec3>& remapVertices, const U32& startIndex, const Maths::HVec3& vertex)
{
	const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon());
	for(S32 i=startIndex; i<remapVertices.m_numElements; ++i)
	{
		//const Maths::HVec3 temp = remapVertices[i];
		const Maths::HVec3 v(vertex - remapVertices[i]);
		//if(vertex.All_EQ(temp))
		if((v|v).LE(epsilon))
		{
			return i;
		}
	}
	return -1;
}

void AABBTree::RemapVerticesToLeafNodes()
{
	using namespace Maths;
	SharedPhysics::C_SafeStack<true> stack(16000);
	U32 size = 1;
	stack.Push(m_pRoot, 0);
	SharedPhysics::SortedArray<Maths::HVec3> remapVertices(m_vertice.m_numElements * 2);
	int startIndex = 0;
	
	while(size--)
	{
		AABBTreeNode* pNode = stack.Pop<AABBTreeNode>(size);
		if(pNode->m_leftNode)
		{
			stack.Push(pNode->m_rightNode, size++);
			stack.Push(pNode->m_leftNode, size++);
			continue;
		}

		startIndex = remapVertices.m_numElements;
		pNode->m_VerticeStartIndex = startIndex;

		//If we get here, we're in a leaf node, we need to remap the vertices to the leaf node
	
		for(U32 i = 0; i < pNode->m_TriangleSizeIndice; ++i)
		{
			S32 index = pNode->m_TriangleStartIndex + i;
			AABBBuildTreeTriangle* tri(m_indicesArray[index]);
			HVec3 a(m_vertice[tri->m_IndexA]);
			HVec3 b(m_vertice[tri->m_IndexB]);
			HVec3 c(m_vertice[tri->m_IndexC]);

			int insertPosA = FindVerticeIndex(remapVertices, startIndex, a);
			int insertPosB = FindVerticeIndex(remapVertices, startIndex, b);
			int insertPosC = FindVerticeIndex(remapVertices, startIndex, c);

			if(insertPosA == -1)
			{
				tri->m_IndexA = remapVertices.m_numElements - startIndex;
				remapVertices.AddToEnd(&a);
			}
			else
				tri->m_IndexA = insertPosA - startIndex;

			if(insertPosB == -1)
			{
				tri->m_IndexB = remapVertices.m_numElements - startIndex;
				remapVertices.AddToEnd(&b);
			}
			else
				tri->m_IndexB = insertPosB - startIndex;

			if(insertPosC == -1)
			{
				tri->m_IndexC = remapVertices.m_numElements - startIndex;
				remapVertices.AddToEnd(&c);
			}
			else
				tri->m_IndexC = insertPosC - startIndex;
		}
		pNode->m_NumVertices = remapVertices.m_numElements - startIndex;
	}

	m_vertice.m_numElements = 0;
	//convert the remapvertice array to the m_vertice
	for(S32 i=0; i<remapVertices.m_numElements; ++i)
	{
		m_vertice.AddToEnd(&remapVertices[i]);
	}
}


void AABBTree::TransferDataToStream()
{
	//Transfer triangle to data to streams. First, need to traverse through all leaf nodes are re-organise 
	//triangles into common sets of material indices and flagss
	m_Stream.Resize((sizeof(Collision::AABBTreeTriangle) + sizeof(Collision::C_TriangleHeader)) * this->_m_indice.Size());
	SharedPhysics::C_SafeStack<true> stack(16000);
	U32 size = 1;
	stack.Push(m_pRoot, 0);
	U32 offset = 0;
	while(size--)
	{
		AABBTreeNode* node = stack.Pop<AABBTreeNode>(size);
		if(node->m_leftNode)
		{
			stack.Push(node->m_rightNode, size++);
			stack.Push(node->m_leftNode, size++);
			continue;
		}
		//If we get here, we're in a leaf node. We need to find the triangles in this node and re-organise them
		//into common sets of materials and collision flags...
		ReSortTriangles(node);


		AABBBuildTreeTriangle &start(*m_indicesArray[node->m_TriangleStartIndex]);
		//Now store the triangles + headers into the triangle stream...
		
		//This is the start address to export with the node - takes into account all the 
		//headers as well as triangles!
		U32 startAddress = offset;

		Collision::C_TriangleHeader* header(m_Stream.GetHeader(offset));
		offset += sizeof(Collision::C_TriangleHeader);
		
		header->m_CollisionFlags = start.m_CollisionFlags;
		header->m_PhysicsMaterialIndex = start.m_PhysicsMaterialIndex;
		header->m_EffectMaterialIndex = start.m_EffectMaterialIndex;
		//assert(header->m_MaterialIndex < 64);
		header->m_UserDataFlags = start.m_UserDataFlags;
		header->m_NumTriangles = 0;

		

		U32 trianglesProcessed = 0;
		//SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter(m_indice, node->m_startIndex);
		for(U32 a = 0; a < node->m_TriangleSizeIndice; ++a)
		{
			//AABBBuildTreeTriangle& tri(m_indice[node->m_startIndex + a]);
			AABBBuildTreeTriangle& tri(*m_indicesArray[node->m_TriangleStartIndex + a]);
			if(tri.m_CollisionFlags != header->m_CollisionFlags || tri.m_PhysicsMaterialIndex != header->m_PhysicsMaterialIndex
				|| tri.m_EffectMaterialIndex != header->m_EffectMaterialIndex || tri.m_UserDataFlags != header->m_UserDataFlags || header->m_NumTriangles == 255)
			{
				header = m_Stream.GetHeader(offset);
				offset += sizeof(Collision::C_TriangleHeader);
				header->m_CollisionFlags = tri.m_CollisionFlags;
				header->m_PhysicsMaterialIndex = tri.m_PhysicsMaterialIndex;
				header->m_EffectMaterialIndex = tri.m_EffectMaterialIndex;
				header->m_UserDataFlags = tri.m_UserDataFlags;
				header->m_NumTriangles = 0;
			}
			Collision::AABBTreeTriangle* triangle(m_Stream.GetTriangles(offset));
			offset += sizeof(Collision::AABBTreeTriangle);
			triangle->m_IndexA = tri.m_IndexA;
			triangle->m_IndexB = tri.m_IndexB;
			triangle->m_IndexC = tri.m_IndexC;
			Maths::Store<Maths::HVec3, MathsStorage::HenDN3>(m_Normals[tri.m_NormalIndex], &triangle->m_SurfaceNormal);
			//triangle->m_ActiveEdgeFlags = tri.m_ActiveEdgeFlags;

			if(tri.m_ActiveEdgeFlags != 0)
			{
				//Add a triangle edge 
				Collision::C_ActiveEdgeNormals norm;
				norm.m_EdgeNormA = tri.m_EdgeNormA;
				norm.m_EdgeNormB = tri.m_EdgeNormB;
				norm.m_EdgeNormC = tri.m_EdgeNormC;
				triangle->m_ActiveEdgeDataIndex = (m_TriangleEdgeNormals.m_numElements << 3) | tri.m_ActiveEdgeFlags;
				m_TriangleEdgeNormals.AddToEnd(&norm);	
			}
			else
			{
				triangle->m_ActiveEdgeDataIndex = 0;
			}

			//triangle->m_EdgeNormA = tri.m_EdgeNormA;
			//triangle->m_EdgeNormB = tri.m_EdgeNormB;
			//triangle->m_EdgeNormC = tri.m_EdgeNormC;

			//Now validate the normals!
			//assert(MatchNormal(tri));

			header->m_NumTriangles++;
		}
		node->m_TriangleStartIndex = startAddress;
	}
	m_StreamSize = offset;
}


void AABBTree::ReSortTriangles(AABBTreeNode* pNode)
{
	//Small data set, so use bubble sort...
	bool bChanged = true;
	while(bChanged)
	{
		bChanged = false;
		S32 triSizeIndex = ((S32)pNode->m_TriangleSizeIndice) - 1;
		for(S32 a = 0; a < triSizeIndex; ++a)
		{
			S32 index = pNode->m_TriangleStartIndex + a;
			AABBBuildTreeTriangle* triA(m_indicesArray[index]);
			AABBBuildTreeTriangle& _triA(*triA);
			AABBBuildTreeTriangle* triB(m_indicesArray[index + 1]);
			AABBBuildTreeTriangle& _triB(*triB);

			if(_triA > _triB)
			{
				//Swap...
				bChanged = true;
				//AABBBuildTreeTriangle* tri(m_indicesArray[index]);
				m_indicesArray.Set(&triB, index);
				m_indicesArray.Set(&triA, index+1);
				//m_indice.Swap(m_indice.Get(index), m_indice.Get(index + 1));
			}
		}
	}
}

void AABBTree::RemoveDuplicatedVertices()
{
	/* 
	 * (1) Create an array of remapped vertex data
	 * (2) Sort them
	 * (3) Iterate over and remap/recreate list of vertices!
	 */

	using namespace SharedPhysics;
	if(m_vertice.m_numElements == 0)
		return;

	//Half-centimeter
	const Maths::HF32 closeScale(500.f);
	const Maths::HF32 closeEpsilon(0.005f*0.005f);
	//Be conservative - make sure this is far enough away to not cause a problem at all
	const Maths::HF32 tenCloseEpsilon(closeEpsilon);

	SharedPhysics::SortedArray<C_TreeVertex> sortedVerts(m_vertice.m_numElements);
	S32 *remappedVertexArray = new S32[m_vertice.m_numElements];

	for(S32 a = 0; a < this->m_vertice.m_numElements; ++a)
	{
		C_TreeVertex vertex(m_vertice[a], a, closeScale);
		sortedVerts.AddToEnd(&vertex);
		//Welded vertex
		m_vertice[a] = vertex.m_Vertex;
	}

	SharedPhysics::C_S32SafeStack stack(16092);
	sortedVerts.QuickSort(stack, 10);

	m_vertice.m_numElements = 0;
	
	//m_vertice.AddToEnd(&sortedVerts[0].m_Vertex);
	for(S32 a = 0; a < sortedVerts.m_numElements; ++a)
	{
		C_TreeVertex& v0 = sortedVerts[a];
		if(v0.m_RemappedIndex == v0.m_Index) //Not already found as a duplicate...
		{
			v0.m_RemappedIndex = m_vertice.m_numElements;
			remappedVertexArray[v0.m_Index] = m_vertice.m_numElements;
			m_vertice.AddToEnd(&v0.m_Vertex);
			for(S32 b = a+1; b < sortedVerts.m_numElements; ++b)
			{
				C_TreeVertex& v1 = sortedVerts[b];
				const Maths::HVec3 dif(v0.m_Vertex - v1.m_Vertex);
				const Maths::HF32 d(dif | dif);
				if(d.LE(closeEpsilon))
				{
					v1.m_RemappedIndex = v0.m_RemappedIndex;
					remappedVertexArray[v1.m_Index] = (S32)v0.m_RemappedIndex;
				}
				else if((v1.m_Projection - v0.m_Projection).GT(tenCloseEpsilon))
					break;
			}
		}
	}

	//Go through and ensure there are no vertices closer than closeEpsilon to each-other..
	/*for(S32 a = 0; a < m_vertice.m_numElements; ++a)
	{
		for(S32 b = a+1; b < m_vertice.m_numElements; ++b)
		{
			const Maths::HVec3 dif(m_vertice[a] - m_vertice[b]);
			const Maths::HF32 d(dif | dif);
			assert(d.GT(closeEpsilon));
		}
	}*/

	C_DoubleAllocIterator<Baking::AABBBuildTreeTriangle> iter(_m_indice);

	/*for(S32 a = 0; a < m_indice.Size(); ++a)
	{
		Baking::AABBBuildTreeTriangle& tri = m_indice[a];
		tri.m_IndexA = remappedVertexArray[tri.m_IndexA];
		tri.m_IndexB = remappedVertexArray[tri.m_IndexB];
		tri.m_IndexC = remappedVertexArray[tri.m_IndexC];
	}*/

	while(iter.HasNext())
	{
		Baking::AABBBuildTreeTriangle& tri = *iter.Next();
		BIZ_ASSERT(remappedVertexArray[tri.m_IndexA] >= 0 && remappedVertexArray[tri.m_IndexA] < m_vertice.m_numElements);
		BIZ_ASSERT(remappedVertexArray[tri.m_IndexB] >= 0 && remappedVertexArray[tri.m_IndexB] < m_vertice.m_numElements);
		BIZ_ASSERT(remappedVertexArray[tri.m_IndexC] >= 0 && remappedVertexArray[tri.m_IndexC] < m_vertice.m_numElements);
		tri.m_IndexA = remappedVertexArray[tri.m_IndexA];
		tri.m_IndexB = remappedVertexArray[tri.m_IndexB];
		tri.m_IndexC = remappedVertexArray[tri.m_IndexC];
	}

	delete [] remappedVertexArray;
}

void AABBTree::CopyTrianglesToIndexArray()
{
	this->m_indicesArray.m_numElements = 0;
	m_indicesArray.Resize(_m_indice.Size());
	U32 numActiveEdges = 0;

	//for(S32 a = 0; a < m_indice.Size(); ++a)
	SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter(_m_indice);
	S32 a = 0;
	while(iter.HasNext())
	{
		AABBBuildTreeTriangle* tri(iter.Next());
		m_indicesArray.AddToEnd(&tri);
	}
}

void AABBTree::CalculateActiveEdgeFlags(const Maths::HF32 PCREF normThreshold)
{
	//TODO - loop over all triangles, working out which edges are shared by triangles. Then 
	//Work out if the edge is active or not...

	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	SharedPhysics::SortedArray<C_TriangleEdge> triangleEdges;
	//SharedPhysics::SortedArray<AABBBuildTreeTriangle*> triArray(m_indice.Size());
	this->m_indicesArray.m_numElements = 0;
	m_indicesArray.Resize(_m_indice.Size());
	U32 numActiveEdges = 0;

	//for(S32 a = 0; a < m_indice.Size(); ++a)
	SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter(_m_indice);
	S32 a = 0;
	while(iter.HasNext())
	{
		AABBBuildTreeTriangle* tri(iter.Next());
		tri->m_ActiveEdgeFlags = 0; //All 3 edges are active by default!!!
		//BIZ_ASSERT(tri->m_IndexA != 4 && tri->m_IndexB != 10);
		C_TriangleEdge edge0(tri->m_IndexA, tri->m_IndexB, a);
		C_TriangleEdge edge1(tri->m_IndexA, tri->m_IndexC, a);
		C_TriangleEdge edge2(tri->m_IndexB, tri->m_IndexC, a);
		triangleEdges.AddToEnd(&edge0);
		triangleEdges.AddToEnd(&edge1);
		triangleEdges.AddToEnd(&edge2);

		/*triangleEdges.Insert(&edge0);
		triangleEdges.Insert(&edge1);
		triangleEdges.Insert(&edge2);*/
		m_indicesArray.AddToEnd(&tri);
		++a;
	}

	SharedPhysics::C_S32SafeStack stack(16092);
	triangleEdges.QuickSort(stack, 10);
	S32 size = triangleEdges.Size();
	const U32 sizeMinus1(size-1);
	for(a = 0; a < size; )
	{
		U32 parentIndex = a;
		C_TriangleEdge* edge0 = &triangleEdges[a++];
		//assert(edge0->m_IndexA < edge0->m_IndexB);
		//assert(edge0->m_IndexA != 910 || edge0->m_IndexB != 922);
		if(edge0->m_ParentEdgeIndex == -1)
		{
			edge0->m_ParentEdgeIndex = parentIndex;
			if(a < size)
			{
				C_TriangleEdge* edge1 = &triangleEdges[a];
				
				while(((*edge0) == (*edge1)))
				{
					edge1->bDeleted = true;
					edge1->m_ParentEdgeIndex = parentIndex;
					edge0->AddTriangle(edge1->m_TriangleIndex[0]);
					if(a == (sizeMinus1))
						break;
					edge1 = &triangleEdges[++a];
					BIZ_ASSERT((*edge0) <= (*edge1));
				}
			}
		}
	}

	//Now loop through the edges and mark them as active IF
	//(a) There is only one triangle contributing to the edge
	//(b) The normals of the triangles sharing the edges are sufficiently different...

	for(a = 0; a < size;)
	{
		//C_TriangleEdge& tempEdge(triangleEdges[a++]);
		//C_TriangleEdge& edge = triangleEdges[tempEdge.m_ParentEdgeIndex];
		C_TriangleEdge& edge = triangleEdges[a];
		//Get triangle that this edge relates to
		//assert(edge.m_IndexA != 910 || edge.m_IndexB != 922);
		MathsStorage::HenDN3 edgeNormal;
		edgeNormal.v = 0; //0 normal to begin with...
		bool bActiveEdge = false;//edge.m_NumTriangles < 2;
		{
			if(edge.m_NumTriangles == 1)
			{
				bActiveEdge = true;
				//Calculate edge normal
				AABBBuildTreeTriangle& triA(*m_indicesArray[edge.m_TriangleIndex[0]]);
				Maths::HVec3 surfaceNormal(m_Normals[triA.m_NormalIndex]);
				//Calculate edge normal - cross product edge direction with face normal...
				const Maths::HVec3 edgeDir(m_vertice[edge.m_IndexA] - m_vertice[edge.m_IndexB]);
				const Maths::C_Normal edgeNor(edgeDir ^ surfaceNormal);
				//Pick sign....
				U32 spareIndexA = triA.GetSpareIndex(edge.m_IndexA, edge.m_IndexB);
				const Maths::HVec3 spareVertex(m_vertice[spareIndexA]);
				const Maths::HF32 side((spareVertex - m_vertice[edge.m_IndexA]) | edgeNor);
				const Maths::C_Normal norm(Maths::Select(side.LE(zero), edgeNor, -edgeNor));
				Maths::Store<Maths::HVec3, MathsStorage::HenDN3>(norm.m_V, &edgeNormal);
			}

			if(edge.m_NumTriangles > 1 && edge.m_NumTriangles < 3)
			{
				//This edge may be inactive - find out more...
				AABBBuildTreeTriangle& triA(*m_indicesArray[edge.m_TriangleIndex[0]]);
				AABBBuildTreeTriangle& triB(*m_indicesArray[edge.m_TriangleIndex[1]]);
				Maths::C_Normal normA(this->m_Normals[triA.m_NormalIndex]);
				Maths::C_Normal normB(this->m_Normals[triB.m_NormalIndex]);
		
				const Maths::HF32 cosAng(normA | normB);

				const Maths::HF32 disA(normA | this->m_vertice[triA.m_IndexA]);
				const Maths::HF32 disB(normB | this->m_vertice[triB.m_IndexA]);

				//Get spare vertex...
				U32 spareIndexA(triA.GetSpareIndex(edge.m_IndexA, edge.m_IndexB));
				U32 spareIndexB(triB.GetSpareIndex(edge.m_IndexA, edge.m_IndexB));
				const Maths::HVec3 vertA(m_vertice[spareIndexA]);
				const Maths::HVec3 vertB(m_vertice[spareIndexB]);

				const Maths::HF32 disSpareA((vertB | normA) - disA);
				const Maths::HF32 disSpareB((vertA | normB) - disB);

				

				
				if(disSpareA.LT(zero) & disSpareB.LT(zero) & cosAng.LE(normThreshold))
				{
					const Maths::C_Normal norm(m_Normals[triA.m_NormalIndex] + m_Normals[triB.m_NormalIndex]);
					Maths::Store<Maths::HVec3, MathsStorage::HenDN3>(norm.m_V, &edgeNormal);
					bActiveEdge = true;
				}
			}
			while(a < size)
			{
				C_TriangleEdge& otherEdge = triangleEdges[a];
				if(otherEdge != edge)
					break;
				++a;
				if(bActiveEdge)
				{
					++numActiveEdges;
					AABBBuildTreeTriangle& tri(*m_indicesArray[otherEdge.m_TriangleIndex[0]]);
					//Inactive edge, so mark it as so!
					//Match up which edge it is...
					if((tri.m_IndexA == edge.m_IndexA && tri.m_IndexB == edge.m_IndexB) || 
						(tri.m_IndexA == edge.m_IndexB && tri.m_IndexB == edge.m_IndexA))
					{
						//Edge0 is inactive...
						tri.m_ActiveEdgeFlags |= 1;
						tri.m_EdgeNormA = edgeNormal;
					}

					if((tri.m_IndexB == edge.m_IndexA && tri.m_IndexC == edge.m_IndexB) || 
						(tri.m_IndexB == edge.m_IndexB && tri.m_IndexC == edge.m_IndexA))
					{
						//Edge0 is inactive...
						tri.m_ActiveEdgeFlags |= 2;
						tri.m_EdgeNormB = edgeNormal;
					}

					if((tri.m_IndexA == edge.m_IndexA && tri.m_IndexC == edge.m_IndexB) || 
						(tri.m_IndexA == edge.m_IndexB && tri.m_IndexC == edge.m_IndexA))
					{
						//Edge0 is inactive...
						tri.m_ActiveEdgeFlags |= 4;
						tri.m_EdgeNormC = edgeNormal;
					}
				}
			}
		}

	}
}

//void AABBTree::CalculateActiveEdgeFlags(const Maths::HF32 PCREF normThreshold)
//{
//	//OK. Algorithm...
//	//Active edges are acute edges which need to put some edge normal goodness in them.
//	//Edges are active if they
//	//(a) Are a member of only one polygon!
//	//(b) Are a 
//}


};


