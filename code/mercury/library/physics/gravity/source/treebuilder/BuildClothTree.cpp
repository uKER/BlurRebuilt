#include "StdAfx.h"
#include "gravity/treebuilder/BuildClothTree.h"
#include "gravity/treebuilder/AABBTree.h"
#include "gravity/sharedphysics/safestack.h"
#include "gravity/narrowphase/AxisBox.inl"

namespace Baking
{
	//Add a single triangle!

	void C_BuildSoftBodyTree::BuildTree()
	{
		RemoveDuplicateVertices();
		RemoveDuplicateEdges();

		if(m_pSoftBodyVertexRemapArray)
			delete [] m_pSoftBodyVertexRemapArray;
		m_pSoftBodyVertexRemapArray = new U32[m_SoftBodyVertices.m_numElements];
		for(S32 a = 0; a < m_SoftBodyVertices.m_numElements; ++a)
		{
			m_pSoftBodyVertexRemapArray[a] = a;
		}

		m_pRootNode = Subdivide( 0, m_SoftBodyTriangles.m_numElements, 0, m_SoftBodyVertices.m_numElements );
		RemapTriangles();
		GenerateSupportEdges();
		CalculateParticleMasses();
	}

	void C_BuildSoftBodyTree::CalculateParticleMasses()
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		Maths::HF32 totalMass(Maths::Constants<Maths::HF32>::Zero());
		for(S32 a = 0; a < m_SoftBodyVertices.m_numElements; ++a)
		{
			Collision::C_GravityParticle& par = m_SoftBodyVertices[a];
			if(par.m_Mass > zero)
			{
				totalMass += par.m_Mass;
				par.m_Mass = -one;
			}
		}

		const F32 edgeScale = (F32)(m_SoftBodyEdges.m_numElements != 0 ? m_SoftBodyEdges.m_numElements : 1);
		const Maths::HF32 edgeWeight(totalMass/edgeScale);

		for(S32 a = 0; a < m_SoftBodyEdges.m_numElements; ++a)
		{
			Collision::C_SoftBodyEdge& edge = m_SoftBodyEdges[a];
			Collision::C_GravityParticle& parA = m_SoftBodyVertices[edge.particleA];
			Collision::C_GravityParticle& parB = m_SoftBodyVertices[edge.particleB];

			if(parA.m_Mass != zero)
			{
				if(parA.m_Mass < zero)
					parA.m_Mass = zero;
				parA.m_Mass += edgeWeight;
			}
			if(parB.m_Mass != zero)
			{
				if(parB.m_Mass < zero)
					parB.m_Mass = zero;
				parB.m_Mass += edgeWeight;
			}
		}

		for(S32 a = 0; a < m_SoftBodyVertices.m_numElements; ++a)
		{
			Collision::C_GravityParticle& par = m_SoftBodyVertices[a];
			par.m_MassInverse = par.m_Mass.Reciprocal();
		}


	}

	void C_BuildSoftBodyTree::RemapTriangles()
	{

		Collision::C_GravityParticle* pParticles = this->m_SoftBodyVertices.m_pElems;
		m_SoftBodyVertices.m_pElems = new Collision::C_GravityParticle[m_SoftBodyVertices.m_size];
		U32* remapArray = new U32[m_SoftBodyVertices.m_numElements];

		for(S32 a = 0; a < m_SoftBodyVertices.m_numElements; ++a)
		{
			//Remapping particle from old position to new position...
			Collision::C_GravityParticle& par = m_SoftBodyVertices[a];
			par = pParticles[m_pSoftBodyVertexRemapArray[a]];

			//Look-up table from old position to new position...
			remapArray[m_pSoftBodyVertexRemapArray[a]] = a;
		}

		for(S32 a = 0; a < m_SoftBodyTriangles.m_numElements; ++a)
		{
			Collision::C_SoftBodyTriangle& tri = m_SoftBodyTriangles[a];
			/*const Maths::HVec3 vertA(pParticles[tri.m_VertexIndexA].m_CurPosition);
			const Maths::HVec3 vertB(pParticles[tri.m_VertexIndexB].m_CurPosition);
			const Maths::HVec3 vertC(pParticles[tri.m_VertexIndexC].m_CurPosition);
			tri.m_VertexIndexA = m_pClothVertexRemapArray[tri.m_VertexIndexA];
			tri.m_VertexIndexB = m_pClothVertexRemapArray[tri.m_VertexIndexB];
			tri.m_VertexIndexC = m_pClothVertexRemapArray[tri.m_VertexIndexC];

			const Maths::HVec3 newVertA(m_ClothVertices[tri.m_VertexIndexA].m_CurPosition);
			const Maths::HVec3 newVertB(m_ClothVertices[tri.m_VertexIndexB].m_CurPosition);
			const Maths::HVec3 newVertC(m_ClothVertices[tri.m_VertexIndexC].m_CurPosition);

			assert(newVertA.All_EQ(vertA));
			assert(newVertB.All_EQ(vertB));
			assert(newVertC.All_EQ(vertC));*/

			tri.m_VertexIndexA = remapArray[tri.m_VertexIndexA];
			tri.m_VertexIndexB = remapArray[tri.m_VertexIndexB];
			tri.m_VertexIndexC = remapArray[tri.m_VertexIndexC];

		}

		for(S32 a = 0; a < m_SoftBodyEdges.m_numElements; ++a)
		{
			Collision::C_SoftBodyEdge& edge = m_SoftBodyEdges[a];

			edge.particleA = remapArray[edge.particleA];
			edge.particleB = remapArray[edge.particleB];
			
		}

		//Now re-sort vertices...
		
		//OK...Remapped!!!!
		delete [] pParticles;
	}

	void C_BuildSoftBodyTree::GenerateSupportEdges()
	{
		//Generate support edges to support 2 adjacent triangles!

		return;

		const S32 numEdges = this->m_SoftBodyEdges.m_numElements;

		for(S32 a = 0; a < m_SoftBodyEdges.m_numElements; ++a)
		{
			Collision::C_SoftBodyEdge& edge = m_SoftBodyEdges[a];
			edge.m_TriangleIndexA = -1;
			edge.m_TriangleIndexB = -1;
		}

		for(S32 a = 0; a < m_SoftBodyTriangles.m_numElements; ++a)
		{
			Collision::C_SoftBodyTriangle& tri = m_SoftBodyTriangles[a];
			Collision::C_SoftBodyEdge& edgeA = m_SoftBodyEdges[tri.m_EdgeIndexA];
			Collision::C_SoftBodyEdge& edgeB = m_SoftBodyEdges[tri.m_EdgeIndexB];
			Collision::C_SoftBodyEdge& edgeC = m_SoftBodyEdges[tri.m_EdgeIndexC];

			if(edgeA.m_TriangleIndexA == -1)
				edgeA.m_TriangleIndexA = a;
			else
				edgeA.m_TriangleIndexB = a;

			if(edgeB.m_TriangleIndexA == -1)
				edgeB.m_TriangleIndexA = a;
			else
				edgeB.m_TriangleIndexB = a;

			if(edgeC.m_TriangleIndexA == -1)
				edgeC.m_TriangleIndexA = a;
			else
				edgeC.m_TriangleIndexB = a;
		}

		for(S32 a = 0; a < numEdges; ++a)
		{
			//Get edge...
			Collision::C_SoftBodyEdge& edge = m_SoftBodyEdges[a];

			if(edge.m_TriangleIndexB != -1)
			{

				Collision::C_SoftBodyTriangle& triA = m_SoftBodyTriangles[edge.m_TriangleIndexA];
				Collision::C_SoftBodyTriangle& triB = m_SoftBodyTriangles[edge.m_TriangleIndexB];

				//Find the odd-vertices-out!

				U32 oddVertexA(triA.m_VertexIndexA);
				if(triA.m_VertexIndexB != edge.particleA && triA.m_VertexIndexB != edge.particleB)
					oddVertexA = triA.m_VertexIndexB;
				else if (triA.m_VertexIndexC != edge.particleA && triA.m_VertexIndexC != edge.particleB)
					oddVertexA = triA.m_VertexIndexC;

				U32 oddVertexB(triB.m_VertexIndexA);
				if(triB.m_VertexIndexB != edge.particleA && triB.m_VertexIndexB != edge.particleB)
					oddVertexB = triB.m_VertexIndexB;
				else if (triB.m_VertexIndexC != edge.particleA && triB.m_VertexIndexC != edge.particleB)
					oddVertexB = triB.m_VertexIndexC;

				//Introduce an edge connecting these two points!

				Collision::C_SoftBodyEdge newEdge;
				newEdge.particleA = oddVertexA;
				newEdge.particleB = oddVertexB;
				newEdge.m_TriangleIndexA = edge.m_TriangleIndexA;
				newEdge.m_TriangleIndexB = edge.m_TriangleIndexB;

				Collision::C_GravityParticle& parA(m_SoftBodyVertices[oddVertexA]);
				Collision::C_GravityParticle& parB(m_SoftBodyVertices[oddVertexB]);

				const Maths::HVec3 dif(parA.m_CurPosition - parB.m_CurPosition);
				const Maths::HF32 dis(dif.Length());
				newEdge.restLength = dis;

				m_SoftBodyEdges.AddToEnd(&newEdge);

			}
		}
	}

	Maths::HVec3 C_BuildSoftBodyTree::AveragePoint(U32 triangleIndex)
	{
		Collision::C_SoftBodyTriangle& tri(m_SoftBodyTriangles[triangleIndex]);
		const Maths::HVec3 vertA(m_SoftBodyVertices[tri.m_VertexIndexA].m_CurPosition);
		const Maths::HVec3 vertB(m_SoftBodyVertices[tri.m_VertexIndexB].m_CurPosition);
		const Maths::HVec3 vertC(m_SoftBodyVertices[tri.m_VertexIndexC].m_CurPosition);
		return (vertA + vertB + vertC)*SharedPhysics::Constants::Third();
	}


	C_BuildSoftBodyNode* C_BuildSoftBodyTree::BuildNode( U32 startIndex, U32 numIndices, U32 vertexStartIndex, U32 numVertexIndices )
	{
		C_BuildSoftBodyNode* pNode = new C_BuildSoftBodyNode;
		pNode->m_StartIndex = startIndex;
		pNode->m_NumTriangles = numIndices;
		pNode->m_BoundingBox.SetInvalidExtents();
		pNode->m_StartVertexIndex = vertexStartIndex;
		pNode->m_NumVertices = numVertexIndices;
		for(U32 a = 0; a < numIndices; ++a)
		{
			Collision::C_SoftBodyTriangle& tri = m_SoftBodyTriangles[startIndex + a];
			const Maths::HVec3 vertA(m_SoftBodyVertices[tri.m_VertexIndexA].m_CurPosition);
			const Maths::HVec3 vertB(m_SoftBodyVertices[tri.m_VertexIndexB].m_CurPosition);
			const Maths::HVec3 vertC(m_SoftBodyVertices[tri.m_VertexIndexC].m_CurPosition);
			pNode->m_BoundingBox.m_min = Maths::Min(pNode->m_BoundingBox.m_min, Maths::Min(vertA, Maths::Min(vertB, vertC) ) );
			pNode->m_BoundingBox.m_max = Maths::Max(pNode->m_BoundingBox.m_max, Maths::Max(vertA, Maths::Max(vertB, vertC) ) );
		}
		pNode->m_ParentIndex = -1;
		return pNode;
	}

	void C_BuildSoftBodyTree::ExtractEdgeEvents(SharedPhysics::SortedArray<C_EdgeEvent>& edgeEvents, U32 axis, U32 startIndex, U32 endIndex)
	{
		edgeEvents.m_numElements = 0;
		for(U32 a = startIndex; a < endIndex; ++a)
		{
			Collision::C_SoftBodyTriangle& tri = m_SoftBodyTriangles[a];
			const Maths::HVec3 vertA = m_SoftBodyVertices[tri.m_VertexIndexA].m_CurPosition;
			const Maths::HVec3 vertB = m_SoftBodyVertices[tri.m_VertexIndexB].m_CurPosition;
			const Maths::HVec3 vertC = m_SoftBodyVertices[tri.m_VertexIndexC].m_CurPosition;

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
				edgeEvents.AddToEnd(&ev1);
				edgeEvents.AddToEnd(&ev2);
			}
		}
		SharedPhysics::C_S32SafeStack stack(16092);
		edgeEvents.QuickSort(stack, 10);
	}

	F32 C_BuildSoftBodyTree::PickBestEdgePoint(SharedPhysics::SortedArray<C_EdgeEvent>& edgeEvents, F32& score)
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
			if(newScore < bestScore)
			{
				bestScore = newScore;
				bestValue = ev.value;
			}
		}
		score = bestScore;
		return bestValue;
	}


	Collision::C_Plane C_BuildSoftBodyTree::PickSplitAxis( C_BuildSoftBodyNode* pNode )
	{
		using namespace Maths;

		const Maths::HF32 half(Maths::Constants<HF32>::Half());

		const Maths::HF32 zero(Maths::Constants<HF32>::Zero());
		const Maths::C_Normal xNormal(HVec3(1.f, 0.f, 0.f), Maths::C_Normal::UNIT_LENGTH_VECTOR);
		const Maths::C_Normal yNormal(HVec3(0.f, 1.f, 0.f), Maths::C_Normal::UNIT_LENGTH_VECTOR);
		const Maths::C_Normal zNormal(HVec3(0.f, 0.f, 1.f), Maths::C_Normal::UNIT_LENGTH_VECTOR);

		//Now pick average point...

		U32 endIndex = pNode->m_StartIndex + pNode->m_NumTriangles;

		F32 score1(0.f), score2(0.f), score3(0.f);
		F32 value1, value2, value3;
		SharedPhysics::SortedArray<C_EdgeEvent> edgeEvents(2 * pNode->m_NumTriangles);
		ExtractEdgeEvents(edgeEvents, 0, pNode->m_StartIndex, endIndex);
		value1 = PickBestEdgePoint(edgeEvents, score1);
		ExtractEdgeEvents(edgeEvents, 1, pNode->m_StartIndex, endIndex);
		value2 = PickBestEdgePoint(edgeEvents, score2);
		ExtractEdgeEvents(edgeEvents, 2, pNode->m_StartIndex, endIndex);
		value3 = PickBestEdgePoint(edgeEvents, score3);

		if(score1 < score2 && 
			score1 < score3)
		{
			return Collision::C_Plane(xNormal, Maths::HF32(value1));
		}
		else if (score2 < score3)
		{
			return Collision::C_Plane(yNormal, Maths::HF32(value2));
		}
		return Collision::C_Plane(zNormal, Maths::HF32(value3));
	}

	C_BuildSoftBodyNode* C_BuildSoftBodyTree::Subdivide( U32 startIndex, U32 numIndices, U32 vertexStartIndex, U32 numVertexIndices )
	{
		//We've subdivided enough!
		C_BuildSoftBodyNode* pNode = BuildNode(startIndex, numIndices, vertexStartIndex, numVertexIndices);
		if(numIndices <= this->m_MinTriangles)
			return pNode;

		U32 endIndex = startIndex + numIndices;

		//Otherwise, we need to split this node!
		Collision::C_Plane splitPlane = PickSplitAxis(pNode);

		//Re-sort the triangles such that all those on the negative side of the plane are at the beginning of 
		//the list, and all those on the positive side are on the right side of the list!

		U32 i=startIndex;
		U32 j=endIndex -1;
		U32 splitIndex = i;
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());

		while(i<j)
		{
			bool inBackPlane=false;
			while(!inBackPlane && i<j)
			{
				const Maths::HVec3 average(AveragePoint(i));
				const Maths::HF32 distance(splitPlane.Distance(average));
				if(distance <= zero)
				{
					splitIndex = i;
					i++;
				}
				else
				{
					inBackPlane = true;
				}

			}
			bool inFrontPlane=false;
			while(!inFrontPlane && i < j)
			{
				const Maths::HVec3 average(AveragePoint(j));
				const Maths::HF32 distance(splitPlane.Distance(average));
				if(distance < zero)
				{
					inFrontPlane = true;
				}
				else
				{
					j--;
					splitIndex = j;
				}

			}
			if(i < j)
			{
				//Swap
				Collision::C_SoftBodyTriangle temp = m_SoftBodyTriangles[i];
				m_SoftBodyTriangles[i] = m_SoftBodyTriangles[j];
				m_SoftBodyTriangles[j] = temp;
			}
		}

		if(startIndex == splitIndex || endIndex == splitIndex)
			return pNode;


		U32 endVertexIndex = vertexStartIndex + numVertexIndices;
		U32 i2=vertexStartIndex;
		U32 j2=endVertexIndex -1;
		U32 vertexSplitIndex = i2;

		while(i2<j2)
		{
			bool inBackPlane=false;
			while(!inBackPlane && i2<j2)
			{
				//const Maths::HVec3 average(AveragePoint(i));
				const Maths::HVec3 point(this->m_SoftBodyVertices[m_pSoftBodyVertexRemapArray[i2]].m_CurPosition);
				const Maths::HF32 distance(splitPlane.Distance(point));
				if(distance <= zero)
				{
					
					vertexSplitIndex = i2;
					i2++;
					BIZ_ASSERT(vertexSplitIndex >= vertexStartIndex && vertexSplitIndex < endVertexIndex);
				}
				else
				{
					inBackPlane = true;
				}

			}
			bool inFrontPlane=false;
			while(!inFrontPlane && i2 < j2)
			{
				const Maths::HVec3 point(this->m_SoftBodyVertices[m_pSoftBodyVertexRemapArray[j2]].m_CurPosition);
				const Maths::HF32 distance(splitPlane.Distance(point));
				if(distance < zero)
				{
					inFrontPlane = true;
				}
				else
				{
					j2--;
					vertexSplitIndex = j2;
					BIZ_ASSERT(vertexSplitIndex >= vertexStartIndex && vertexSplitIndex < endVertexIndex);
				}

			}
			if(i2 < j2)
			{
				//Swap
				U32 temp = m_pSoftBodyVertexRemapArray[i2];
				m_pSoftBodyVertexRemapArray[i2] = m_pSoftBodyVertexRemapArray[j2];
				m_pSoftBodyVertexRemapArray[j2] = temp;
			}
		}



		//We know what the split index is now!
		pNode->m_pLeftNode = Subdivide(startIndex, splitIndex - startIndex, vertexStartIndex, vertexSplitIndex - vertexStartIndex);
		pNode->m_pRightNode = Subdivide(splitIndex,  endIndex - splitIndex, vertexSplitIndex, endVertexIndex - vertexSplitIndex);
		return pNode;
	}

	void C_BuildSoftBodyTree::AddTriangle(Maths::HVec3 PCREF vertA, Maths::HVec3 PCREF vertB, Maths::HVec3 PCREF vertC,
		Maths::HF32 PCREF massA, Maths::HF32 PCREF massB, Maths::HF32 PCREF massC)
	{
		U32 indexA = m_SoftBodyVertices.m_numElements;
		U32 indexB = indexA + 1;
		U32 indexC = indexA + 2;

		U32 edgeAIndex = m_SoftBodyEdges.m_numElements;
		U32 edgeBIndex = m_SoftBodyEdges.m_numElements+1;
		U32 edgeCIndex = m_SoftBodyEdges.m_numElements+2;

		Collision::C_GravityParticle parA;
		parA.m_CurPosition = vertA;
		parA.m_Mass = massA;
		parA.m_MassInverse = Maths::Reciprocal(massA);

		Collision::C_GravityParticle parB;
		parB.m_CurPosition = vertB;
		parB.m_Mass = massB;
		parB.m_MassInverse = Maths::Reciprocal(massB);

		Collision::C_GravityParticle parC;
		parC.m_CurPosition = vertC;
		parC.m_Mass = massC;
		parC.m_MassInverse = Maths::Reciprocal(massC);

		m_SoftBodyVertices.AddToEnd(&parA);
		m_SoftBodyVertices.AddToEnd(&parB);
		m_SoftBodyVertices.AddToEnd(&parC);

		Collision::C_SoftBodyEdge edgeA;
		edgeA.particleA = indexA;
		edgeA.particleB = indexB;
		edgeA.restLength = (vertB - vertA).Length();

		Collision::C_SoftBodyEdge edgeB;
		edgeB.particleA = indexB;
		edgeB.particleB = indexC;
		edgeB.restLength = (vertC - vertB).Length();

		Collision::C_SoftBodyEdge edgeC;
		edgeC.particleA = indexC;
		edgeC.particleB = indexA;
		edgeC.restLength = (vertA - vertC).Length();

		m_SoftBodyEdges.AddToEnd(&edgeA);
		m_SoftBodyEdges.AddToEnd(&edgeB);
		m_SoftBodyEdges.AddToEnd(&edgeC);

		Collision::C_SoftBodyTriangle triangle;
		triangle.m_EdgeIndexA = edgeAIndex;
		triangle.m_EdgeIndexB = edgeBIndex;
		triangle.m_EdgeIndexC = edgeCIndex;
		triangle.m_VertexIndexA = indexA;
		triangle.m_VertexIndexB = indexB;
		triangle.m_VertexIndexC = indexC;

		m_SoftBodyTriangles.AddToEnd(&triangle);
	}

	void C_BuildSoftBodyTree::RemoveDuplicateVertices()
	{
		if(m_SoftBodyVertices.m_numElements == 0)
			return;

		SharedPhysics::SortedArray<C_SoftBodyTreeVertex> sortedVerts(m_SoftBodyVertices.m_numElements);
		S32 *remappedVertexArray = new S32[m_SoftBodyVertices.m_numElements];

		for(S32 a = 0; a < m_SoftBodyVertices.m_numElements; ++a)
		{
			C_SoftBodyTreeVertex vertex;
			vertex.m_Index = a;
			vertex.m_RemappedIndex = a;
			vertex.m_Vertex = m_SoftBodyVertices[a];
			sortedVerts.AddToEnd(&vertex);
			remappedVertexArray[a] = -1;
		}

		SharedPhysics::C_S32SafeStack stack(16092);
		sortedVerts.QuickSort(stack, 10);

		m_SoftBodyVertices.m_numElements = 0;
		
		//m_vertice.AddToEnd(&sortedVerts[0].m_Vertex);
		for(S32 a = 0; a < sortedVerts.m_numElements; ++a)
		{
			C_SoftBodyTreeVertex& v0 = sortedVerts[a];
			v0.m_RemappedIndex = m_SoftBodyVertices.m_numElements;
			remappedVertexArray[v0.m_Index] = m_SoftBodyVertices.m_numElements;
			m_SoftBodyVertices.AddToEnd(&v0.m_Vertex);
			S32 b = a + 1;
			for(; b < sortedVerts.m_numElements; ++b)
			{
				C_SoftBodyTreeVertex& v1 = sortedVerts[b];
				if(v0 == v1)
				{
					v1.m_RemappedIndex = v0.m_RemappedIndex;
					remappedVertexArray[v1.m_Index] = v0.m_RemappedIndex;
				}
				else
					break;
			}
			a = b - 1;
		}
		for(S32 a = 0; a < m_SoftBodyEdges.m_numElements; ++a)
		{
			Collision::C_SoftBodyEdge& edge = m_SoftBodyEdges[a];
			edge.particleA = remappedVertexArray[edge.particleA];
			edge.particleB = remappedVertexArray[edge.particleB];
		}
		for(S32 a = 0; a < m_SoftBodyTriangles.m_numElements; ++a)
		{
			Collision::C_SoftBodyTriangle& tri = m_SoftBodyTriangles[a];
			tri.m_VertexIndexA = remappedVertexArray[tri.m_VertexIndexA];
			tri.m_VertexIndexB = remappedVertexArray[tri.m_VertexIndexB];
			tri.m_VertexIndexC = remappedVertexArray[tri.m_VertexIndexC];
		}

		delete [] remappedVertexArray;
	}

	void C_BuildSoftBodyTree::RemoveDuplicateEdges()
	{
		if(m_SoftBodyEdges.m_numElements == 0)
			return;

		SharedPhysics::SortedArray<C_RemapSoftBodyEdge> sortedEdges(m_SoftBodyEdges.m_numElements);
		U32 *remappedVertexArray = new U32[m_SoftBodyEdges.m_numElements];

		for(S32 a = 0; a < m_SoftBodyEdges.m_numElements; ++a)
		{
			C_RemapSoftBodyEdge edge;
			edge.m_Index = a;
			//edge.m_Edge = m_ClothEdges[a];
			Collision::C_SoftBodyEdge& cEdge = m_SoftBodyEdges[a];
			edge.m_Edge.particleA = Maths::Min(cEdge.particleA, cEdge.particleB);
			edge.m_Edge.particleB = Maths::Max(cEdge.particleA, cEdge.particleB);
			edge.m_Edge.m_TriangleIndexA = -1;
			edge.m_Edge.m_TriangleIndexB = -1;
			edge.m_Edge.restLength = cEdge.restLength;
			sortedEdges.AddToEnd(&edge);
		}

		SharedPhysics::C_S32SafeStack stack(16092);
		sortedEdges.QuickSort(stack, 10);

		m_SoftBodyEdges.m_numElements = 0;
		
		//m_vertice.AddToEnd(&sortedVerts[0].m_Vertex);
		for(S32 a = 0; a < sortedEdges.m_numElements; ++a)
		{
			C_RemapSoftBodyEdge& v0 = sortedEdges[a];
			v0.m_RemappedIndex = m_SoftBodyEdges.m_numElements;
			remappedVertexArray[v0.m_Index] = m_SoftBodyEdges.m_numElements;
			m_SoftBodyEdges.AddToEnd(&v0.m_Edge);
			S32 b = a + 1;
			for(; b < sortedEdges.m_numElements; ++b)
			{
				C_RemapSoftBodyEdge& v1 = sortedEdges[b];
				if(v0 == v1)
				{
					v1.m_RemappedIndex = v0.m_RemappedIndex;
					remappedVertexArray[v1.m_Index] = v0.m_RemappedIndex;
				}
				else
					break;
			}
			a = b - 1;
		}
		for(S32 a = 0; a < m_SoftBodyTriangles.m_numElements; ++a)
		{
			Collision::C_SoftBodyTriangle& tri = m_SoftBodyTriangles[a];
			tri.m_EdgeIndexA = remappedVertexArray[tri.m_EdgeIndexA];
			tri.m_EdgeIndexB = remappedVertexArray[tri.m_EdgeIndexB];
			tri.m_EdgeIndexC = remappedVertexArray[tri.m_EdgeIndexC];
			Collision::C_SoftBodyEdge& edgeA = m_SoftBodyEdges[tri.m_EdgeIndexA];
			if(edgeA.m_TriangleIndexA == -1)
				edgeA.m_TriangleIndexA = a;
			else
				edgeA.m_TriangleIndexB = a;

			Collision::C_SoftBodyEdge& edgeB = m_SoftBodyEdges[tri.m_EdgeIndexB];
			if(edgeB.m_TriangleIndexA == -1)
				edgeB.m_TriangleIndexA = a;
			else
				edgeB.m_TriangleIndexB = a;

			Collision::C_SoftBodyEdge& edgeC = m_SoftBodyEdges[tri.m_EdgeIndexC];
			if(edgeC.m_TriangleIndexA == -1)
				edgeC.m_TriangleIndexA = a;
			else
				edgeC.m_TriangleIndexB = a;
		}

		delete [] remappedVertexArray;
	}

	U32 C_BuildSoftBodyNode::CalculateNumDescendants(U32 index)
	{
		if(this->m_pLeftNode == NULL)
		{
			this->m_NumDescendants = 0;
		}
		else
		{
			m_pLeftNode->m_ParentIndex = index;
			m_pRightNode->m_ParentIndex = index;
			m_NumDescendants = m_pLeftNode->CalculateNumDescendants(index+1);
			m_NumDescendants += m_pRightNode->CalculateNumDescendants(index + m_NumDescendants + 1);
		}
		return m_NumDescendants + 1;
	}

	void C_BuildSoftBodyTree::CalculateNumDescendants()
	{
		if(this->m_pRootNode)
		{
			U32 index = 0;
			m_pRootNode->CalculateNumDescendants(index);
		}
	}
}
