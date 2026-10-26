#include "StdAfx.h"
#include "gravity/narrowphase/ConvexHull.h"
#include "gravity/sharedphysics/BitField.h"

namespace Collision
{
	Maths::HVec3 C_SpuConvexHull::SpuHillClimbingSupport(const Maths::HVec3 PCREF direction) const
	{
#if 0
		using namespace Maths;
		const Maths::HVec3 localDir(m_Orientation * direction);
		//MathsStorage::Float3* f = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices, sizeof(MathsStorage::Float3));
		//Maths::HVec3 point(Maths::Load<Maths::HVec3>(f));

		Maths::HVec3 point = GetLocalVertex(0);
		Maths::HF32 bestDir(point | localDir);
	
		for(U32 a = 1; a < m_NumVertices; ++a)
		{
			//f = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices + a, sizeof(MathsStorage::Float3));
			//const Maths::HVec3 vertex(Maths::Load<Maths::HVec3>(f));
			const HVec3 vertex = GetLocalVertex(a);
			const Maths::HF32 depth(vertex | localDir);
			const Maths::Scalar_Bool bLarger(depth.GT(bestDir));
			bestDir = Select(bLarger, depth, bestDir);
			point = Select(Maths::Vector3_Bool(bLarger), vertex, point);
		}
		return (point * m_Orientation) + m_Position;
#else
		Spu::MemBaseMarker marker;
		 
		BIZ_ASSERT(m_CurrentIndex < m_NumVertices);
		//DebugBreak();
		using namespace Maths;
		const Maths::HVec3 localDir(m_Orientation * direction);

		Maths::HVec3 point(GetLocalVertex(m_CurrentIndex));
		Maths::HF32 bestDir(point | localDir);
		U32 index = m_CurrentIndex;
		S32 myIndex = m_CurrentIndex;
		const U32 numVerts = (m_NumVertices >> 5) + 1;
#ifndef PLATFORM_SPU
		U32 *vertexIndices = (U32*)alloca(sizeof(U32) * numVerts);
#else
		U32 *vertexIndices = (U32*)Spu::Alloc((sizeof(U32) * numVerts + 15)&0xfffffff0);
#endif

		SharedPhysics::C_BitField bitField(vertexIndices, numVerts);
		
		bitField.ClearAllBits();
		bitField.SetBit(index);

		//U32 visitCount = 0;
		do
		{
			myIndex=index;
			BIZ_ASSERT(index < m_NumVertices);
			C_HillClimbingNode& node = m_HillClimbingNodes[index];
			C_HillClimbingNode& nextNode = m_HillClimbingNodes[index+1];
			
			BIZ_ASSERT(node.m_StartIndex <= m_NumNeighbours);
			BIZ_ASSERT((nextNode.m_StartIndex) <= m_NumNeighbours);
			for(U32 i=node.m_StartIndex; i< nextNode.m_StartIndex; ++i)
			{
				U32 neigh = m_pNeighbours[i];
				BIZ_ASSERT(neigh < m_NumVertices);
				const HVec3 vertex(GetLocalVertex(neigh));
				const Maths::HF32 depth(vertex | localDir);
				if(depth >= bestDir)
				{
					if(!bitField.TestAndSetBit(neigh))
					{
						bestDir = depth;
						point = vertex;
						index = neigh;
						//break;
					}
					
				}
			}
		}while(myIndex!=index);

		m_CurrentIndex = index;

		//return (point* m_Orientation) + m_Position;
		return (point * m_Orientation ) + m_Centre;
#endif
	}


	Maths::HVec3 C_SpuConvexHull::SpuHillClimbingMarginSupport(const Maths::HVec3 PCREF direction, const Maths::HF32 PCREF margin) const
	{
		Spu::MemBaseMarker marker;
		BIZ_ASSERT(m_CurrentIndex < m_NumVertices);
		//DebugBreak();
		using namespace Maths;
		const Maths::HVec3 localDir(m_Orientation * direction);

		Maths::HVec3 point(GetLocalVertex(m_CurrentIndex));
		Maths::HF32 bestDir(point | localDir);
		U32 index = m_CurrentIndex;
		S32 myIndex = m_CurrentIndex;
		const U32 numVerts = (m_NumVertices >> 5) + 1;
#ifndef PLATFORM_SPU
		U32 *vertexIndices = (U32*)alloca(sizeof(U32) * numVerts);
#else
		U32 *vertexIndices = (U32*)Spu::Alloc((sizeof(U32) * numVerts + 15)&0xfffffff0);
#endif

		SharedPhysics::C_BitField bitField(vertexIndices, numVerts);
		
		bitField.ClearAllBits();
		bitField.SetBit(index);

		//U32 visitCount = 0;
		do
		{
			myIndex=index;
			BIZ_ASSERT(index < m_NumVertices);
			C_HillClimbingNode& node = m_HillClimbingNodes[index];
			C_HillClimbingNode& nextNode = m_HillClimbingNodes[index+1];
			
			BIZ_ASSERT(node.m_StartIndex <= m_NumNeighbours);
			BIZ_ASSERT((nextNode.m_StartIndex) <= m_NumNeighbours);
			for(U32 i=node.m_StartIndex; i< nextNode.m_StartIndex; ++i)
			{
				U32 neigh = m_pNeighbours[i];
				BIZ_ASSERT(neigh < m_NumVertices);
				const HVec3 vertex(GetLocalVertex(neigh));
				const Maths::HF32 depth(vertex | localDir);
				if(depth >= bestDir)
				{
					if(!bitField.TestAndSetBit(neigh))
					{
						bestDir = depth;
						point = vertex;
						index = neigh;
						//break;
					}
					
				}
			}
		}while(myIndex!=index);

		m_CurrentIndex = index;

		//Get the margin version of point
		const HF32 b(point|localDir);
		const HF32 a(localDir.Length() * margin);
		const HF32 t(Constants<HF32>::One() - a/b);
		const HVec3 x(t*point);
		return (x*m_Orientation) + m_Centre;
	}

	C_SpuConvexHull::C_SpuConvexHull(const C_ConvexHull& convexHull, bool fetchTriangles): C_Convex(E_CONVEX_SPUCONVEXHULL),
		m_Low(convexHull.m_Low), m_High(convexHull.m_High), m_Range(convexHull.m_Range), m_Orientation(convexHull.m_Orientation)
	{
		WarmStart(convexHull.WarmStart());
		Epsilon(convexHull.Epsilon());
		Margin(convexHull.Margin());
		Centre(convexHull.Centre());
		m_NumVertices = convexHull.m_NumVertices;
		m_NumNeighbours = convexHull.m_NumNeighbours;
		m_NumTriangles = convexHull.m_NumTriangles;
		m_CurrentIndex = convexHull.m_CurrentIndex;
		using namespace Maths;

#ifdef PLATFORM_SPU
		m_pNeighbours = (U8*)Spu::FetchAllocUnalignedAsync(&convexHull.m_pNeighbours[0], sizeof(U8)*m_NumNeighbours, 1);
		m_pVertices = (MathsStorage::Fixed3_S16<14>*)Spu::FetchAllocUnalignedAsync(&convexHull.m_pVertices[0], sizeof(MathsStorage::Fixed3_S16<14>)*m_NumVertices, 1);
		m_HillClimbingNodes = (C_HillClimbingNode*)Spu::FetchAllocUnalignedAsync(&convexHull.m_HillClimbingNodes[0], sizeof(C_HillClimbingNode) * (m_NumVertices + 1), 1);
		if(fetchTriangles)
		{
			m_pTriangles = (C_ConvexTriangle*)Spu::FetchAllocUnalignedAsync(&convexHull.m_pTriangles[0], sizeof(C_ConvexTriangle)*m_NumTriangles, 1);
		}
		Spu::Wait(1);

		for(U32 a = 0; a < m_NumVertices; ++a)
		{
			BIZ_ASSERT(m_HillClimbingNodes[a].m_StartIndex < m_NumNeighbours);
		}

#else
		m_pVertices = convexHull.m_pVertices;
		m_pNeighbours = convexHull.m_pNeighbours;
		m_HillClimbingNodes = convexHull.m_HillClimbingNodes;
		m_pTriangles = convexHull.m_pTriangles;
#endif
	}



	Maths::HVec3 C_LocalSpaceSpuConvexHull::SpuHillClimbingSupport(const Maths::HVec3 PCREF direction) const
	{
		Spu::MemBaseMarker marker;
		BIZ_ASSERT(m_CurrentIndex < m_NumVertices);
		//DebugBreak();
		using namespace Maths;

		//Already in this object's space, so no need for the transform!
		const Maths::HVec3 localDir(direction);

		Maths::HVec3 point(GetLocalVertex(m_CurrentIndex));
		Maths::HF32 bestDir(point | localDir);
		U32 index = m_CurrentIndex;
		S32 myIndex = m_CurrentIndex;
		const U32 numVerts = (m_NumVertices >> 5) + 1;
#ifndef PLATFORM_SPU
		U32 *vertexIndices = (U32*)alloca(sizeof(U32) * numVerts);
#else
		U32 *vertexIndices = (U32*)Spu::Alloc((sizeof(U32) * numVerts + 15)&0xfffffff0);
#endif

		SharedPhysics::C_BitField bitField(vertexIndices, numVerts);
		
		bitField.ClearAllBits();
		bitField.SetBit(index);

		//U32 visitCount = 0;
		do
		{
			myIndex=index;
			BIZ_ASSERT(index < m_NumVertices);
			C_HillClimbingNode& node = m_HillClimbingNodes[index];
			C_HillClimbingNode& nextNode = m_HillClimbingNodes[index+1];
			
			BIZ_ASSERT(node.m_StartIndex <= m_NumNeighbours);
			BIZ_ASSERT((nextNode.m_StartIndex) <= m_NumNeighbours);
			for(U32 i=node.m_StartIndex; i< nextNode.m_StartIndex; ++i)
			{
				U32 neigh = m_pNeighbours[i];
				BIZ_ASSERT(neigh < m_NumVertices);
				const HVec3 vertex(GetLocalVertex(neigh));
				const Maths::HF32 depth(vertex | localDir);
				if(depth >= bestDir)
				{
					if(!bitField.TestAndSetBit(neigh))
					{
						bestDir = depth;
						point = vertex;
						index = neigh;
						//break;
					}
					
				}
			}
		}while(myIndex!=index);

		m_CurrentIndex = index;

		//return (point* m_Orientation) + m_Position;
		return point;
	}

	C_LocalSpaceSpuConvexHull::C_LocalSpaceSpuConvexHull(const C_ConvexHull& convexHull, bool fetchTriangles): C_Convex(E_CONVEX_SPUCONVEXHULL_LOCAL),
		m_Low(convexHull.m_Low),m_High(convexHull.m_High), m_Range(convexHull.m_Range), m_Orientation(convexHull.m_Orientation)
	{
		WarmStart(Maths::Constants<Maths::HVec3>::Zero());
		Centre(convexHull.Centre());
		Margin(convexHull.Margin());
		Epsilon(convexHull.Epsilon());
		m_NumVertices = convexHull.m_NumVertices;
		m_NumNeighbours = convexHull.m_NumNeighbours;
		m_NumTriangles = convexHull.m_NumTriangles;
		m_CurrentIndex = convexHull.m_CurrentIndex;
		using namespace Maths;

#ifdef PLATFORM_SPU
		m_pNeighbours = (U8*)Spu::FetchAllocUnalignedAsync(&convexHull.m_pNeighbours[0], sizeof(U8)*m_NumNeighbours, 1);
		m_pVertices = (MathsStorage::Fixed3_S16<14>*)Spu::FetchAllocUnalignedAsync(&convexHull.m_pVertices[0], sizeof(MathsStorage::Fixed3_S16<14>)*m_NumVertices, 1);
		m_HillClimbingNodes = (C_HillClimbingNode*)Spu::FetchAllocUnalignedAsync(&convexHull.m_HillClimbingNodes[0], sizeof(C_HillClimbingNode) * (m_NumVertices + 1), 1);
		if(fetchTriangles)
		{
			m_pTriangles = (C_ConvexTriangle*)Spu::FetchAllocUnalignedAsync(&convexHull.m_pTriangles[0], sizeof(C_ConvexTriangle)*m_NumTriangles, 1);
		}
		Spu::Wait(1);

		for(U32 a = 0; a < m_NumVertices; ++a)
		{
			BIZ_ASSERT(m_HillClimbingNodes[a].m_StartIndex < m_NumNeighbours);
		}

#else
		m_pVertices = convexHull.m_pVertices;
		m_pNeighbours = convexHull.m_pNeighbours;
		m_HillClimbingNodes = convexHull.m_HillClimbingNodes;
		m_pTriangles = convexHull.m_pTriangles;
#endif
	}




	Maths::HVec3 C_ConvexHull::HillClimbingSupport(const Maths::HVec3 PCREF direction) const
	{
		Spu::MemBaseMarker marker;
		BIZ_ASSERT(m_CurrentIndex < m_NumVertices);
		//DebugBreak();
		using namespace Maths;
		const Maths::HVec3 localDir(m_Orientation * direction);

		Maths::HVec3 point(GetVertex(m_CurrentIndex));
		Maths::HF32 bestDir(point | localDir);
		U32 index = m_CurrentIndex;
		S32 myIndex = m_CurrentIndex;
		const U32 numVerts = (m_NumVertices >> 5) + 1;
#ifndef PLATFORM_SPU
		U32 *vertexIndices = (U32*)alloca(sizeof(U32) * numVerts);
#else
		U32 *vertexIndices = (U32*)Spu::Alloc((sizeof(U32) * numVerts + 15)&0xfffffff0);
#endif

		SharedPhysics::C_BitField bitField(vertexIndices, numVerts);
		
		bitField.ClearAllBits();
		bitField.SetBit(index);

		//U32 visitCount = 0;
		do
		{
			myIndex=index;
			BIZ_ASSERT(index < m_NumVertices);
			C_HillClimbingNode& node = m_HillClimbingNodes[index];
			C_HillClimbingNode& nextNode = m_HillClimbingNodes[index+1];
			
			BIZ_ASSERT(node.m_StartIndex <= m_NumNeighbours);
			BIZ_ASSERT((nextNode.m_StartIndex) <= m_NumNeighbours);
			for(U32 i=node.m_StartIndex; i< nextNode.m_StartIndex; ++i)
			{
				U32 neigh = m_pNeighbours[i];
				BIZ_ASSERT(neigh < m_NumVertices);
				const HVec3 vertex(GetVertex(neigh));
				const Maths::HF32 depth(vertex | localDir);
				if(depth >= bestDir)
				{
					if(!bitField.TestAndSetBit(neigh))
					{
						bestDir = depth;
						point = vertex;
						index = neigh;
						//break;
					}
					
				}
			}
		}while(myIndex!=index);

		m_CurrentIndex = index;

		//return (point* m_Orientation) + m_Position;
		return (point*m_Orientation) + m_Centre;
	}
}