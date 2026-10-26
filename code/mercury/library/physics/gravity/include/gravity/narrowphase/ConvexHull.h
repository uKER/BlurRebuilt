#ifndef CONVEX_HULL_H
#define CONVEX_HULL_H

#include "gravity/narrowphase/AxisBox.h"
#include "gravity/sharedphysics/constants.h"
#include "gravity/sharedphysics/sortedarray.h"
#include <system/scheduler/jobqueue.h>

namespace Collision
{

#define USE_HILLCLIMBING 1

	class C_ConvexHullProxy;
	class C_SpuConvexHull;

	enum E_CONVEX_TYPE
	{
		E_CONVEX_SPHERE,
		E_CONVEX_OBB,
		E_CONVEX_CAPSULE,
		E_CONVEX_CYLINDER,
		E_CONVEX_CONVEXHULL,
		E_CONVEX_TRIANGLE,
		E_CONVEX_AABB,
		E_CONVEX_CONE,
		E_CONVEX_SPUCONVEXHULL,
		E_CONVEX_SPUCONVEXHULL_LOCAL
	};

	class C_Convex
	{
		
	public:

		C_Convex(E_CONVEX_TYPE type, const Maths::HVec3 PCREF centre = Maths::Constants<Maths::HVec3>::Zero()) : m_Type(type), m_Centre(centre), m_wStart(centre), m_Margin(Maths::Constants<Maths::HF32>::Zero()), m_Epsilon(Maths::Constants<Maths::HF32>::Epsilon2())
		{
		}

		/*C_Convex() : m_Type(E_CONVEX_CONVEXHULL), wStart(Maths::UNDEFINED), margin(Maths::UNDEFINED), epsilon(Maths::Constants<Maths::HF32>::Epsilon2())
		{
		}*/

		virtual ~C_Convex()
		{
		}
		NOALIAS inline Maths::HF32 Epsilon() const
		{
			return m_Epsilon;
		}

		NOALIAS inline void Epsilon(const Maths::HF32 PCREF eps)
		{
			m_Epsilon = eps;
		}

		//NOALIAS inline Maths::HVec3 Centre() const
		//{
		//	return Maths::Constants<Maths::HVec3>::Zero();
		//}

		NOALIAS inline bool QuadraticShape() const
		{
			return false;
		}

		NOALIAS inline Maths::HF32 Margin() const
		{
			return m_Margin;
		}

		NOALIAS inline void Margin(const Maths::HF32 PCREF _margin)
		{
			m_Margin=_margin;
		}

		NOALIAS inline Maths::HVec3 WarmStart() const
		{
			return m_wStart;
		}

		NOALIAS inline void WarmStart(const Maths::HVec3& warmStart)
		{
			m_wStart=warmStart;
		}

		NOALIAS inline Maths::HVec3 Centre() const
		{
			return m_Centre;
		}

		NOALIAS inline void Centre(const Maths::HVec3 PCREF  centre)
		{
			m_Centre=centre;
		}

		//NB - all C_Convex should supply 2 support functions - support Margin/Support. Support Margin is used 
		//in GJK Margin, which returns the support point expanded by the margin...
		Maths::HVec3 m_wStart;
		Maths::HVec3 m_Centre;
		Maths::HF32 m_Margin;//which is used by the ISA_GJK to get the penetration depth of the enlarged objects
		Maths::HF32 m_Epsilon;
		E_CONVEX_TYPE m_Type;
	};

	struct C_ConvexEdge
	{
		U16 m_VertexA;
		U16 m_VertexB;
	};

	struct C_ConvexTriangle
	{
		MathsStorage::HenDN3 m_Normal;
		U8 m_VertexA;						//VertexA
		U8 m_VertexB;						//VertexB
		U8 m_VertexC;						//VertexC
	};

	class C_Triangle : public C_Convex
	{
	public:
		Maths::HVec3 m_VertexA, m_VertexB, m_VertexC;
		Maths::HVec3 m_Normal;
		//Maths::HVec3 m_Extruded;
		Maths::HF32 m_Extrusion;
		C_Triangle() : m_VertexA(Maths::UNDEFINED), m_VertexB(Maths::UNDEFINED), m_VertexC(Maths::UNDEFINED), m_Normal(Maths::UNDEFINED),
			/*m_Extruded(Maths::UNDEFINED), */m_Extrusion(Maths::UNDEFINED), C_Convex(E_CONVEX_TRIANGLE)
		{
			Margin(Maths::Constants<Maths::HF32>::Zero());
		}

		virtual ~C_Triangle()
		{
		}

		NOALIAS inline Maths::HVec3 Centre() const
		{
			return (m_VertexA + m_VertexB + m_VertexC) * SharedPhysics::Constants::Third();
		}

		NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF direction) const
		{
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			//const Maths::HVec3 extrusion(Maths::Select((direction | m_Normal).LT(zero), Maths::HVec3(zero), m_Normal * m_Extrusion));
			const Maths::HF32 dA(m_VertexA | direction);
			const Maths::HF32 dB(m_VertexB | direction);
			const Maths::HF32 dC(m_VertexC | direction);
			//const Maths::HF32 dX(m_Extruded | direction);
			//return extrusion + Select(dA.GT(dB), Select(dA.GT(dC), m_VertexA, m_VertexC), Select(dB.GT(dC), m_VertexB, m_VertexC));
			return Select(dA.GT(dB), Select(dA.GT(dC), m_VertexA, m_VertexC), Select(dB.GT(dC), m_VertexB, m_VertexC));
			//return Select(dA.GT(dB), Select(dA.GT(dC), Select(dA.GT(dX), m_VertexA, m_Extruded), Select(dC.GT(dX), m_VertexC, m_Extruded)), 
			//	Select(dB.GT(dC), Select(dB.GT(dX), m_VertexB, m_Extruded), Select(dC.GT(dX), m_VertexC, m_Extruded)));
		}

		NOALIAS inline Maths::HVec3 SupportMargin(const Maths::HVec3 PCREF direction, const Maths::HF32 PCREF margin) const
		{
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			//const Maths::HVec3 extrusion(Maths::Select((direction | m_Normal).LT(zero), Maths::HVec3(zero), m_Normal * m_Extrusion));
			const Maths::HF32 dA(m_VertexA | direction);
			const Maths::HF32 dB(m_VertexB | direction);
			const Maths::HF32 dC(m_VertexC | direction);
			//const Maths::HF32 dX(m_Extruded | direction);
			//return extrusion + Select(dA.GT(dB), Select(dA.GT(dC), m_VertexA, m_VertexC), Select(dB.GT(dC), m_VertexB, m_VertexC));
			return Select(dA.GT(dB), Select(dA.GT(dC), m_VertexA, m_VertexC), Select(dB.GT(dC), m_VertexB, m_VertexC));
			//return Select(dA.GT(dB), Select(dA.GT(dC), Select(dA.GT(dX), m_VertexA, m_Extruded), Select(dC.GT(dX), m_VertexC, m_Extruded)), 
			//	Select(dB.GT(dC), Select(dB.GT(dX), m_VertexB, m_Extruded), Select(dC.GT(dX), m_VertexC, m_Extruded)));
		}

		NOALIAS inline bool QuadraticShape() const
		{
			return false;
		}
	};

	class C_BaseHillClimbingNode
	{
	public:
		SharedPhysics::SortedArray<U32> neighbors;

		C_BaseHillClimbingNode() : neighbors(10)
		{
		}

	};

	class C_HillClimbingNode
	{
	public:
		U16 m_StartIndex;
		//U16 m_NumNeighbours; //Can be determined from next element!

		C_HillClimbingNode() : m_StartIndex(0)
		{
		}

	};

	

	class C_ConvexHull : public C_Convex
	{
	public:

		C_ConvexHull() : m_pVertices(NULL), m_pTriangles(NULL),m_Orientation(Maths::UNDEFINED), m_NumVertices(0), 
			m_NumTriangles(0), C_Convex(E_CONVEX_CONVEXHULL),m_HillClimbingNodes(NULL), m_CurrentIndex(0), m_ExternallyOwned(0), 
			m_pNeighbours(NULL), m_NumNeighbours(0),m_BaseHillClimbingNodes(NULL), m_Extents(Maths::UNDEFINED), 
			m_Low(Maths::UNDEFINED), m_High(Maths::UNDEFINED), m_Range(Maths::UNDEFINED)
		{
		}

		C_ConvexHull(const U32 numVertices) : m_pTriangles(NULL), m_NumVertices(numVertices), m_NumTriangles(0),C_Convex(E_CONVEX_CONVEXHULL),
			m_CurrentIndex(0), m_ExternallyOwned(0), m_pNeighbours(NULL), m_NumNeighbours(0), m_Extents(Maths::UNDEFINED), 
			m_Low(Maths::UNDEFINED), m_High(Maths::UNDEFINED), m_Range(Maths::UNDEFINED)
		{
			//m_pVertices = new MathsStorage::Float3[numVertices + 2];
			m_pVertices = new MathsStorage::Fixed3_S16<14>[numVertices + 3]; // JASOND: Padded to prevent unaligned vector loading crash!
			m_HillClimbingNodes = new C_HillClimbingNode[numVertices+7]; // JASOND: Why is this padded?!?
			m_BaseHillClimbingNodes = new C_BaseHillClimbingNode[numVertices + 2]; // JASOND: Why is this padded?!?
		}

		C_ConvexHull(const U32 numVertices, const U32 numTriangles) : m_NumVertices(numVertices), m_NumTriangles(0),C_Convex(E_CONVEX_CONVEXHULL),
			m_CurrentIndex(0), m_ExternallyOwned(0), m_pNeighbours(NULL), m_NumNeighbours(0), m_Extents(Maths::UNDEFINED), 
			m_Low(Maths::UNDEFINED), m_High(Maths::UNDEFINED), m_Range(Maths::UNDEFINED)
		{
			//m_pVertices = new MathsStorage::Float3[numVertices + 2];
			m_pVertices = new MathsStorage::Fixed3_S16<14>[numVertices + 3]; // JASOND: Padded to prevent unaligned vector loading crash!
			m_pTriangles = new C_ConvexTriangle[numTriangles + 2]; // JASOND: Why is this padded?!?
			m_HillClimbingNodes = new C_HillClimbingNode[numVertices+7]; // JASOND: Why is this padded?!?
			m_BaseHillClimbingNodes = new C_BaseHillClimbingNode[numVertices + 2]; // JASOND: Why is this padded?!?
		}

		virtual ~C_ConvexHull()
		{
			if(!m_ExternallyOwned)
			{
				if(m_pVertices)
					delete [] m_pVertices;
				if(m_pTriangles)
					delete [] m_pTriangles;
				if(m_HillClimbingNodes)
					delete[] m_HillClimbingNodes;
				if(m_BaseHillClimbingNodes)
					delete [] m_BaseHillClimbingNodes;
				if(m_pNeighbours)
					delete [] m_pNeighbours;
			}
			m_pVertices = NULL;
			m_pTriangles = NULL;
			m_HillClimbingNodes = NULL;
			m_BaseHillClimbingNodes = NULL;
			m_pNeighbours = NULL;
			m_NumVertices = m_NumTriangles = m_CurrentIndex = m_NumNeighbours = 0;
		}

		NOALIAS inline const Maths::HVec3 GetVertex(const U32 index) const
		{
			Spu::MemBaseMarker marker;
			MathsStorage::Fixed3_S16<14>* pF = (MathsStorage::Fixed3_S16<14>*)Spu::FetchAllocUnaligned(m_pVertices + index, sizeof(MathsStorage::Fixed3_S16<14>));
			const Maths::HVec3 localV(pF);
			return (localV * m_Range + m_Low);

		}

		NOALIAS inline MathsStorage::Fixed3_S16<14>* GetVertices()
		{
			return m_pVertices;
		}

		NOALIAS inline const void SetVertex(const Maths::HVec3 PCREF vert, const U32 index)
		{
			m_pVertices[index] = (MathsStorage::Fixed3_S16<14>)vert;
		}

		NOALIAS inline MathsStorage::Fixed3_S16<14>* GetVertices() const
		{
			return m_pVertices;
		}

		NOALIAS inline C_HillClimbingNode* GetHillClimbingNodes() const
		{
			return m_HillClimbingNodes;
		}

		NOALIAS inline C_BaseHillClimbingNode* GetBaseHillClimbingNodes() const
		{
			return m_BaseHillClimbingNodes;
		}

		NOALIAS inline C_ConvexTriangle* GetTriangles() const
		{
			return m_pTriangles;
		}

		NOALIAS inline C_ConvexTriangle& GetTriangle(const U32 index) const
		{
			return m_pTriangles[index];
		}

		NOALIAS inline U8* GetNeighbours() const
		{
			return m_pNeighbours;
		}

		NOALIAS inline U32 GetNumNeighbours() const
		{
			return m_NumNeighbours;
		}

		NOALIAS inline U32 GetNumTriangles() const
		{
			return m_NumTriangles;
		}

		NOALIAS inline U32 GetNumVertices() const
		{
			return m_NumVertices;
		}


		NOALIAS inline void CalculateExtents()
		{
			//const Maths::HVec3 range(m_High - m_Low);
			m_Extents = Maths::Constants<Maths::HVec3>::Zero();
			for(U32 a =0; a < m_NumVertices; ++a)
			{
				const Maths::HVec3 localVert(&m_pVertices[a]);
				const Maths::HVec3 vert((localVert*m_Range) + m_Low);
				m_Extents = Maths::Max(m_Extents, Maths::Abs(vert));
			}
		}

	/*	NOALIAS inline Maths::HVec3 Centre() const
		{
			return m_Position;
		}

		NOALIAS inline void Centre(const Maths::HVec3 PCREF centre)
		{
			m_Position = centre;
			WarmStart(centre);
		}*/

		NOALIAS inline void Orientation(const Maths::HMat33&  orientation)
		{
			m_Orientation = orientation;
		}

		NOALIAS inline void Low(const Maths::HVec3 PCREF low)
		{
			m_Low = low;
		}

		NOALIAS inline void High(const Maths::HVec3 PCREF high)
		{
			m_High = high;
		}

		NOALIAS inline void Range(const Maths::HVec3 PCREF range)
		{
			m_Range = range;
		}

		NOALIAS inline Maths::HMat33 Orientation() const
		{
			return m_Orientation;
		}

		NOALIAS inline void SetNumberVertices(const U32 numVertices)
		{
			if(m_pVertices)
				delete [] m_pVertices;
			//m_pVertices = new MathsStorage::Float3[numVertices + 2];
			m_pVertices = new MathsStorage::Fixed3_S16<14>[numVertices + 3];
			m_NumVertices = numVertices;

			if(m_HillClimbingNodes)
				delete[] m_HillClimbingNodes;
			m_HillClimbingNodes = new C_HillClimbingNode[numVertices + 7];

			if(m_BaseHillClimbingNodes)
				delete[] m_BaseHillClimbingNodes;
			m_BaseHillClimbingNodes = new C_BaseHillClimbingNode[numVertices + 2];
		}

		NOALIAS inline void SetNumberTriangles(const U32 numTris)
		{
			if(m_pTriangles)
				delete [] m_pTriangles;
			m_pTriangles = new C_ConvexTriangle[numTris + 2]; // JASOND: Why is this padded?!?
			m_NumTriangles = 0;
		}

		/*NOALIAS inline void SetNumberVertices(const U32 numVertices, MathsStorage::Float3* vertices, 
			C_HillClimbingNode* pNodes)
		{

			if(m_pVertices && !m_ExternallyOwned)
				delete [] m_pVertices;
			m_pVertices = vertices;
			m_NumVertices = numVertices;

			if(m_HillClimbingNodes && !m_ExternallyOwned)
				delete[] m_HillClimbingNodes;
			m_HillClimbingNodes = pNodes;
		}*/

		NOALIAS inline void SetNumberVertices(const U32 numVertices, MathsStorage::Fixed3_S16<14>* vertices, 
			C_HillClimbingNode* pNodes, U8* pNeighbours, U32 numNeighbours)
		{

			if(m_pVertices && !m_ExternallyOwned)
				delete [] m_pVertices;
			m_pVertices = vertices;
			m_NumVertices = numVertices;

			if(m_HillClimbingNodes && !m_ExternallyOwned)
				delete[] m_HillClimbingNodes;
			m_HillClimbingNodes = pNodes;

			if(m_pNeighbours && !m_ExternallyOwned)
				delete [] m_pNeighbours;
			m_pNeighbours = pNeighbours;
			m_NumNeighbours = numNeighbours;
		}

		NOALIAS inline void SetNumberTriangles(const U32 numTris, C_ConvexTriangle* pTris)
		{
			if(m_pTriangles && !m_ExternallyOwned)
				delete [] m_pTriangles;
			m_pTriangles = pTris;
			m_NumTriangles = numTris;
			m_CurrentIndex = m_pTriangles[0].m_VertexA;
		}

		void SetExternallyOwned(bool externallyOwned)
		{
			m_ExternallyOwned = (U32)externallyOwned;
		}

	/*	NOALIAS inline void SetTriangles(U32 indexA, U32 indexB, U32 indexC)
		{
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			assert(indexA < m_NumVertices);
			assert(indexB < m_NumVertices);
			assert(indexC < m_NumVertices);
			

			const Maths::HVec3 vertA(Maths::Load<Maths::HVec3>(&m_pVertices[indexA]));
			const Maths::HVec3 vertB(Maths::Load<Maths::HVec3>(&m_pVertices[indexB]));
			const Maths::HVec3 vertC(Maths::Load<Maths::HVec3>(&m_pVertices[indexC]));

			const Maths::HVec3 vAB(vertA - vertB);
			const Maths::HVec3 vAC(vertA - vertC);
			const Maths::C_Normal norm(vAB ^ vAC);
			if((norm | norm).GT(zero))
			{
				m_pTriangles[m_NumTriangles].m_VertexA = indexA;
				m_pTriangles[m_NumTriangles].m_VertexB = indexB;
				m_pTriangles[m_NumTriangles].m_VertexC=  indexC;
				
				
				m_BaseHillClimbingNodes[indexA].neighbors.InsertOverwrite(&indexB);
				m_BaseHillClimbingNodes[indexA].neighbors.InsertOverwrite(&indexC);
				
				m_BaseHillClimbingNodes[indexB].neighbors.InsertOverwrite(&indexA);
				m_BaseHillClimbingNodes[indexB].neighbors.InsertOverwrite(&indexC);

				m_BaseHillClimbingNodes[indexC].neighbors.InsertOverwrite(&indexA);
				m_BaseHillClimbingNodes[indexC].neighbors.InsertOverwrite(&indexB);

				Maths::Store<Maths::HVec3>(norm.m_V, &m_pTriangles[m_NumTriangles].m_Normal);
				const Maths::HF32 dis(vertA | norm);
				Maths::Store<Maths::HF32>(dis, &m_pTriangles[m_NumTriangles++].m_Dist);
			}
		}*/

		NOALIAS inline void SetTriangles(U8 indexA, U8 indexB, U8 indexC)
		{
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			BIZ_ASSERT(indexA < m_NumVertices);
			BIZ_ASSERT(indexB < m_NumVertices);
			BIZ_ASSERT(indexC < m_NumVertices);

			U32 indA = indexA;
			U32 indB = indexB;
			U32 indC = indexC;
			

			const Maths::HVec3 tempVertA(&m_pVertices[indexA]);
			const Maths::HVec3 tempVertB(&m_pVertices[indexB]);
			const Maths::HVec3 tempVertC(&m_pVertices[indexC]);

			const Maths::HVec3 range(m_High - m_Low);
			const Maths::HVec3 vertA(tempVertA*range + m_Low);
			const Maths::HVec3 vertB(tempVertB*range + m_Low);
			const Maths::HVec3 vertC(tempVertC*range + m_Low);

			const Maths::HVec3 vAB(vertA - vertB);
			const Maths::HVec3 vAC(vertA - vertC);
			const Maths::C_Normal norm(vAB ^ vAC);
			if((norm | norm).GT(zero))
			{
				m_pTriangles[m_NumTriangles].m_VertexA = (U8)indexA;
				m_pTriangles[m_NumTriangles].m_VertexB = (U8)indexB;
				m_pTriangles[m_NumTriangles].m_VertexC=  (U8)indexC;
				
				
				m_BaseHillClimbingNodes[indexA].neighbors.InsertOverwrite(&indB);
				m_BaseHillClimbingNodes[indexA].neighbors.InsertOverwrite(&indC);
				
				m_BaseHillClimbingNodes[indexB].neighbors.InsertOverwrite(&indA);
				m_BaseHillClimbingNodes[indexB].neighbors.InsertOverwrite(&indC);

				m_BaseHillClimbingNodes[indexC].neighbors.InsertOverwrite(&indA);
				m_BaseHillClimbingNodes[indexC].neighbors.InsertOverwrite(&indB);

				Maths::Store<Maths::HVec3>(norm.m_V, &m_pTriangles[m_NumTriangles++].m_Normal);
				//const Maths::HF32 dis(vertA | norm);
				//Maths::Store<Maths::HF32>(dis, &m_pTriangles[m_NumTriangles++].m_Dist);
			}
		}

		void FinaliseHull()
		{
			//Count number of neighbours...
			U32 numNeighbours = 0;
			for(U32 a = 0; a < m_NumVertices; ++a)
			{
				numNeighbours += m_BaseHillClimbingNodes[a].neighbors.m_numElements;
			}

			//Allocate structure big enough for this many neighbors...
			m_pNeighbours = new U8[numNeighbours + 17]; //Add 16 bytes to the end for DMA alignment!
			m_NumNeighbours = numNeighbours;

			U32 index = 0;
			//DebugBreak();
			for(U32 a = 0; a < m_NumVertices; ++a)
			{
				C_BaseHillClimbingNode* node = &m_BaseHillClimbingNodes[a];
				C_HillClimbingNode* hillNode = &m_HillClimbingNodes[a];
				const U32 neighs = node->neighbors.m_numElements;
				hillNode->m_StartIndex = (U16)index;
				//hillNode->m_NumNeighbours = neighs;
				BIZ_ASSERT(hillNode->m_StartIndex < m_NumNeighbours);
				for(U32 b = 0; b < neighs; ++b)
				{
					BIZ_ASSERT(node->neighbors[b] < m_NumVertices);
					m_pNeighbours[index++] = (U8)node->neighbors[b];
				}
			}

			m_CurrentIndex = this->m_pTriangles[0].m_VertexA;

			m_HillClimbingNodes[m_NumVertices].m_StartIndex = (U16)m_NumNeighbours;
			delete [] m_BaseHillClimbingNodes;
			m_BaseHillClimbingNodes = NULL;
		}

	

		NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF direction) const
		{
#if !USE_HILLCLIMBING //def PLATFORM_SPU
			using namespace Maths;
			Spu::MemBaseMarker marker;
			const Maths::HVec3 localDir(m_Orientation * direction);
			//MathsStorage::Float3* f = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices, sizeof(MathsStorage::Float3));
			//Maths::HVec3 point(Maths::Load<Maths::HVec3>(f));


			Maths::HVec3 point = GetVertex(0);
			Maths::HF32 bestDir(point | localDir);
		
			for(U32 a = 1; a < m_NumVertices; ++a)
			{
				marker.Restore();

				//f = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices + a, sizeof(MathsStorage::Float3));
				//const Maths::HVec3 vertex(Maths::Load<Maths::HVec3>(f));
				const HVec3 vertex = GetVertex(a);
				const Maths::HF32 depth(vertex | localDir);
				const Maths::Scalar_Bool bLarger(depth.GT(bestDir));
				bestDir = Select(bLarger, depth, bestDir);
				point = Select(Maths::Vector3_Bool(bLarger), vertex, point);
			}
			//return (point * m_Orientation) + m_Position;
			return (point * m_Orientation) + m_Centre;
#else
			return HillClimbingSupport(direction);
#endif
		}

		Maths::HVec3 HillClimbingSupport(const Maths::HVec3 PCREF direction) const;


		NOALIAS inline Maths::HVec3 SupportMargin(const Maths::HVec3 PCREF direction, const Maths::HF32 PCREF margin) const
		{
			using namespace Maths;
#if !USE_HILLCLIMBING //def PLATFORM_SPU
			Spu::MemBaseMarker marker;
			const Maths::HVec3 localDir(m_Orientation * direction);
			//MathsStorage::Float3* f = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices, sizeof(MathsStorage::Float3));
			//Maths::HVec3 point(Maths::Load<Maths::HVec3>(f));
			HVec3 point = GetVertex(0);
			Maths::HF32 bestDir(point | localDir);
		
			for(U32 a = 1; a < m_NumVertices; ++a)
			{
				marker.Restore();
			//	f = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices + a, sizeof(MathsStorage::Float3));
			//	const Maths::HVec3 vertex(Maths::Load<Maths::HVec3>(f));
				HVec3 vertex = GetVertex(a);
				const Maths::HF32 depth(vertex | localDir);
				const Maths::Scalar_Bool bLarger(depth.GT(bestDir));
				bestDir = Select(bLarger, depth, bestDir);
				point = Select(Maths::Vector3_Bool(bLarger), vertex, point);
			}

			return (point * m_Orientation) + m_Centre;
#else
			return HillClimbingSupport(direction);
#endif
		}

		NOALIAS inline Maths::HVec3 GetExtents()
		{
			Maths::HVec3 extents(Maths::Constants<Maths::HVec3>::Zero());

			Maths::HVec3 range(m_High - m_Low);
			for(U32 a = 0; a < m_NumVertices; ++a)
			{
				const Maths::HVec3 localV(&m_pVertices[a]);
				const Maths::HVec3 v(localV*range + m_Low);
				extents = Maths::Max(extents, Maths::Abs(v)); 
			}
			return extents;
		}


		const Maths::HVec3 Range() const
		{
			return m_Range;
		}

		const Maths::HVec3 Low() const
		{
			return m_Low;
		}

		const Maths::HVec3 High() const
		{
			return m_High;
		}



		/*NOALIAS __forceinline bool TraceHull(const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, Maths::HVec3 & intersectPoint) const
		{
			//Put start and end in local space...
			const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
			const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
			const Maths::HVec3 localStart(m_Orientation * (start - m_Position));
			Maths::HVec3 localEnd(m_Orientation * (end - m_Position));
			Maths::HVec3 d(localEnd - localStart);



			Maths::Scalar_Bool bHit(false);


			for(U32 a = 0; a < m_NumTriangles; ++a)
			{
				C_ConvexTriangle& tri(m_pTriangles[a]);
				const Maths::HVec3 norm(Maths::Load<Maths::HVec3>(&tri.m_Normal));
				const Maths::HF32 dist(Maths::Load<Maths::HF32>(&tri.m_Dist));

				//Test to see if the line crosses the plane...
				const Maths::HF32 dS((norm | localStart) - dist);
				const Maths::HF32 dE((norm | localEnd) - dist);

				const Maths::HF32 side(dS * dE);
				if(side <= zero)
				{
					//The ray crosses the line, so find the point of contact and check to make sure it's 
					//inside the triangle

					const Maths::HVec3 vA(Maths::Load<Maths::HVec3>(m_pVertices + tri.m_VertexA));
					const Maths::HVec3 vB(Maths::Load<Maths::HVec3>(m_pVertices + tri.m_VertexB));
					const Maths::HVec3 vC(Maths::Load<Maths::HVec3>(m_pVertices + tri.m_VertexC));

					const Maths::HF32 t(dS /(dS + dE));

					const Maths::HVec3 contactPoint(localStart + d * t);

					const Maths::HVec3 v0(vC - vA);
					const Maths::HVec3 v1(vB - vA);
					const Maths::HVec3 v2(contactPoint - vA);

					const Maths::HF32 dot00(v0 | v0);
					const Maths::HF32 dot01(v0 | v1);
					const Maths::HF32 dot02(v0 | v2);
					const Maths::HF32 dot11(v1 | v1);
					const Maths::HF32 dot12(v1 | v2);

					const Maths::HF32 invDenom((dot00*dot11 - dot01 * dot01).Reciprocal());;
					const Maths::HF32 u((dot11 * dot02 - dot01 * dot12) * invDenom);
					const Maths::HF32 v((dot00 * dot12 - dot01 * dot02) * invDenom);

					Maths::Scalar_Bool b((u.GT(zero) & v.GT(zero) & (u + v).LT(one)));
					bHit = bHit | b;

					localEnd = Select(Maths::Vector3_Bool(b), localEnd, contactPoint);
					d = localEnd - localStart;
				}
			}
			intersectPoint = (m_Orientation * localEnd) + m_Position;
			return bHit;
		}*/

		friend class C_ConvexHullProxy;
		friend class C_SpuConvexHull;
		friend class C_LocalSpaceSpuConvexHull;

private:
		Maths::HMat33 m_Orientation;
		//Maths::HVec3 m_Position;
		Maths::HVec3 m_Extents;
		Maths::HVec3 m_Low;
		Maths::HVec3 m_High;
		Maths::HVec3 m_Range;
		MathsStorage::Fixed3_S16<14>* m_pVertices;
		C_HillClimbingNode* m_HillClimbingNodes;
		C_BaseHillClimbingNode* m_BaseHillClimbingNodes;
		C_ConvexTriangle* m_pTriangles;
		U8* m_pNeighbours;

		U32 m_NumVertices;		
		U32 m_NumTriangles;
		U32 m_ExternallyOwned;
		U32 m_NumNeighbours;
		mutable U32 m_CurrentIndex;
	};

	class C_SpuConvexHull : public C_Convex
	{
	public:

		C_SpuConvexHull(const C_ConvexHull& convexHull, bool fetchTriangles = false);

		const Maths::HVec3 GetLocalVertex(const U32 index) const
		{
			const Maths::HVec3 localV(&m_pVertices[index]);
			return (localV * m_Range + m_Low);
		}

		const Maths::HVec3 GetWorldVertex(const U32 index) const
		{
			using namespace Maths;
			const HVec3 compressedV(&m_pVertices[index]);
			const HVec3 unCompressedV(compressedV * m_Range + m_Low);
			//return m_Position + unCompressedV * m_Orientation;
			return m_Centre + unCompressedV * m_Orientation;
		}


		NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF direction) const
		{
#if !USE_HILLCLIMBING //def PLATFORM_SPU
			using namespace Maths;
			Spu::MemBaseMarker marker;
			const Maths::HVec3 localDir(m_Orientation * direction);
			//MathsStorage::Float3* f = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices, sizeof(MathsStorage::Float3));
			//Maths::HVec3 point(Maths::Load<Maths::HVec3>(f));


			Maths::HVec3 point = GetLocalVertex(0);
			Maths::HF32 bestDir(point | localDir);
		
			for(U32 a = 1; a < m_NumVertices; ++a)
			{
				marker.Restore();

				//f = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices + a, sizeof(MathsStorage::Float3));
				//const Maths::HVec3 vertex(Maths::Load<Maths::HVec3>(f));
				const HVec3 vertex = GetLocalVertex(a);
				const Maths::HF32 depth(vertex | localDir);
				const Maths::Scalar_Bool bLarger(depth.GT(bestDir));
				bestDir = Select(bLarger, depth, bestDir);
				point = Select(Maths::Vector3_Bool(bLarger), vertex, point);
			}
			//return (point * m_Orientation) + m_Position;
			return (point * m_Orientation) + m_Centre;
#else
			return SpuHillClimbingSupport(direction);
#endif
			
		}

		NOALIAS inline Maths::HVec3 SupportMargin(const Maths::HVec3 PCREF direction, const Maths::HF32 PCREF margin) const
		{
			using namespace Maths;
#if !USE_HILLCLIMBING//def PLATFORM_SPU
			Spu::MemBaseMarker marker;
			const Maths::HVec3 localDir(m_Orientation * direction);
			//MathsStorage::Float3* f = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices, sizeof(MathsStorage::Float3));
			//Maths::HVec3 point(Maths::Load<Maths::HVec3>(f));
			HVec3 point = GetLocalVertex(0);
			Maths::HF32 bestDir(point | localDir);
		
			for(U32 a = 1; a < m_NumVertices; ++a)
			{
				marker.Restore();
			//	f = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices + a, sizeof(MathsStorage::Float3));
			//	const Maths::HVec3 vertex(Maths::Load<Maths::HVec3>(f));
				HVec3 vertex = GetLocalVertex(a);
				const Maths::HF32 depth(vertex | localDir);
				const Maths::Scalar_Bool bLarger(depth.GT(bestDir));
				bestDir = Select(bLarger, depth, bestDir);
				point = Select(Maths::Vector3_Bool(bLarger), vertex, point);
			}

			//Get the margin version of point
			const HF32 b(point|localDir);
			const HF32 a(localDir.Length() * margin);
			const HF32 t(Constants<HF32>::One() - a/b);
			const HVec3 x(t*point);
			return (x*m_Orientation) + m_Centre;

			//return (point * m_Orientation) + m_Centre;
#else
			return SpuHillClimbingMarginSupport(direction, margin);
#endif
		}

		NOALIAS inline U32 GetNumTriangles() const
		{
			return m_NumTriangles;
		}

		NOALIAS inline U32 GetNumVertices() const
		{
			return m_NumVertices;
		}

		NOALIAS inline C_ConvexTriangle& GetTriangle(const U32 index) const
		{
			return m_pTriangles[index];
		}

		/*NOALIAS inline void Centre(const Maths::HVec3 PCREF centre)
		{
			m_Position = centre;
			WarmStart(centre);
		}*/

		NOALIAS inline void Orientation(const Maths::HMat33& orientation)
		{
			m_Orientation = orientation;
		}

		NOALIAS inline void Range(const Maths::HVec3 PCREF range)
		{
			m_Range = range;
		}

		NOALIAS inline void Low(const Maths::HVec3 PCREF low)
		{
			m_Low = low;
		}

		NOALIAS inline void High(const Maths::HVec3 PCREF high)
		{
			m_High = high;
		}

		NOALIAS inline Maths::HVec3 Range() const
		{
			return m_Range;
		}

		NOALIAS inline Maths::HVec3 Low() const
		{
			return m_Low;
		}

		NOALIAS inline Maths::HVec3 High() const
		{
			return m_High;
		}

		/*NOALIAS inline Maths::HVec3 Centre() const
		{
			return m_Position;
		}*/

		NOALIAS inline Maths::HMat33 Orientation() const
		{
			return m_Orientation;
		}

		Maths::HVec3 SpuHillClimbingSupport(const Maths::HVec3 PCREF direction) const;
		Maths::HVec3 SpuHillClimbingMarginSupport(const Maths::HVec3 PCREF direction, const Maths::HF32 PCREF margin) const;

	private:
		Maths::HMat33 m_Orientation;
		//Maths::HVec3 m_Position;
		Maths::HVec3 m_Low;
		Maths::HVec3 m_High;
		Maths::HVec3 m_Range;

		MathsStorage::Fixed3_S16<14>* m_pVertices;
		C_HillClimbingNode* m_HillClimbingNodes;
		U8* m_pNeighbours;
		C_ConvexTriangle* m_pTriangles;
		U32 m_NumNeighbours;
		U32 m_NumVertices;		
		U32 m_NumTriangles;
		mutable U32 m_CurrentIndex;

	};


	/*
	 * A local space spu convex hull. To be used when we want GJK to work in the space of the hull!
	 */

	class C_LocalSpaceSpuConvexHull : public C_Convex
	{
	public:

		C_LocalSpaceSpuConvexHull(const C_ConvexHull& convexHull, bool fetchTriangles = false);

		const Maths::HVec3 GetLocalVertex(const U32 index) const
		{
			const Maths::HVec3 localV(&m_pVertices[index]);
			return (localV * m_Range + m_Low);
		}

		//const Maths::HVec3 GetWorldVertex(const U32 index) const
		//{
		//	using namespace Maths;
		//	const HVec3 compressedV(&m_pVertices[index]);
		//	const HVec3 unCompressedV(compressedV * m_Range + m_Low);
		//	//return m_Position + unCompressedV * m_Orientation;
		//	return m_Centre + unCompressedV * m_Orientation;
		//}


		NOALIAS inline Maths::HVec3 Support(const Maths::HVec3 PCREF direction) const
		{
#if 0 //def PLATFORM_SPU
			using namespace Maths;
			Spu::MemBaseMarker marker;
			const Maths::HVec3 localDir(m_Orientation * direction);
			//MathsStorage::Float3* f = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices, sizeof(MathsStorage::Float3));
			//Maths::HVec3 point(Maths::Load<Maths::HVec3>(f));


			Maths::HVec3 point = GetLocalVertex(0);
			Maths::HF32 bestDir(point | localDir);
		
			for(U32 a = 1; a < m_NumVertices; ++a)
			{
				marker.Restore();

				//f = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices + a, sizeof(MathsStorage::Float3));
				//const Maths::HVec3 vertex(Maths::Load<Maths::HVec3>(f));
				const HVec3 vertex = GetLocalVertex(a);
				const Maths::HF32 depth(vertex | localDir);
				const Maths::Scalar_Bool bLarger(depth.GT(bestDir));
				bestDir = Select(bLarger, depth, bestDir);
				point = Select(Maths::Vector3_Bool(bLarger), vertex, point);
			}
			//return (point * m_Orientation) + m_Position;
			return (point * m_Orientation) + m_Centre;
#else
			return SpuHillClimbingSupport(direction);
#endif
			
		}

		NOALIAS inline Maths::HVec3 SupportMargin(const Maths::HVec3 PCREF direction, const Maths::HF32 PCREF margin) const
		{
						using namespace Maths;
#if 1 //def PLATFORM_SPU
			Spu::MemBaseMarker marker;
			const Maths::HVec3 localDir(m_Orientation * direction);
			//MathsStorage::Float3* f = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices, sizeof(MathsStorage::Float3));
			//Maths::HVec3 point(Maths::Load<Maths::HVec3>(f));
			HVec3 point = GetLocalVertex(0);
			Maths::HF32 bestDir(point | localDir);
		
			for(U32 a = 1; a < m_NumVertices; ++a)
			{
				marker.Restore();
			//	f = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices + a, sizeof(MathsStorage::Float3));
			//	const Maths::HVec3 vertex(Maths::Load<Maths::HVec3>(f));
				HVec3 vertex = GetLocalVertex(a);
				const Maths::HF32 depth(vertex | localDir);
				const Maths::Scalar_Bool bLarger(depth.GT(bestDir));
				bestDir = Select(bLarger, depth, bestDir);
				point = Select(Maths::Vector3_Bool(bLarger), vertex, point);
			}

			//return (point * m_Orientation) + m_Position;
			return (point * m_Orientation) + m_Centre;
#else
			return SpuHillClimbingSupport(direction);
#endif
		}

		NOALIAS inline U32 GetNumTriangles() const
		{
			return m_NumTriangles;
		}

		NOALIAS inline U32 GetNumVertices() const
		{
			return m_NumVertices;
		}

		NOALIAS inline C_ConvexTriangle& GetTriangle(const U32 index) const
		{
			return m_pTriangles[index];
		}

		/*NOALIAS inline void Centre(const Maths::HVec3 PCREF centre)
		{
			m_Position = centre;
			WarmStart(centre);
		}*/

		NOALIAS inline void Orientation(const Maths::HMat33& orientation)
		{
			m_Orientation = orientation;
		}

		NOALIAS inline void Range(const Maths::HVec3 PCREF range)
		{
			m_Range = range;
		}

		NOALIAS inline void Low(const Maths::HVec3 PCREF low)
		{
			m_Low = low;
		}

		NOALIAS inline void High(const Maths::HVec3 PCREF high)
		{
			m_High = high;
		}

		NOALIAS inline Maths::HVec3 Range() const
		{
			return m_Range;
		}

		NOALIAS inline Maths::HVec3 Low() const
		{
			return m_Low;
		}

		NOALIAS inline Maths::HVec3 High() const
		{
			return m_High;
		}

		/*NOALIAS inline Maths::HVec3 Centre() const
		{
			return m_Position;
		}*/

		NOALIAS inline Maths::HMat33 Orientation() const
		{
			return m_Orientation;
		}

		Maths::HVec3 SpuHillClimbingSupport(const Maths::HVec3 PCREF direction) const;

	private:
		Maths::HMat33 m_Orientation;
		//Maths::HVec3 m_Position;
		Maths::HVec3 m_Low;
		Maths::HVec3 m_High;
		Maths::HVec3 m_Range;

		MathsStorage::Fixed3_S16<14>* m_pVertices;
		C_HillClimbingNode* m_HillClimbingNodes;
		U8* m_pNeighbours;
		C_ConvexTriangle* m_pTriangles;
		U32 m_NumNeighbours;
		U32 m_NumVertices;		
		U32 m_NumTriangles;
		mutable U32 m_CurrentIndex;

	};


	
};


#endif
