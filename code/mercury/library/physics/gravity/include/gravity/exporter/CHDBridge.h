#ifndef CHD_BRIDGE_H
#define	CHD_BRIDGE_H

namespace Gravity
{
	class C_Facet;

	class C_Pocket
	{
	public:
		C_Pocket(): m_PocketBoundary(10), m_Knots(10)
		{}

		C_Pocket& operator = (C_Pocket& other)
		{

			m_PocketBoundary.m_numElements = other.m_PocketBoundary.m_numElements;
			for(S32 i=0; i<other.m_PocketBoundary.m_numElements; ++i)
			{
				m_PocketBoundary[i] = other.m_PocketBoundary[i];
			}
		}

		SharedPhysics::SortedArray<C_GraphEdge> m_PocketBoundary;
		SharedPhysics::SortedArray<U32> m_Knots;
	};

	class C_Bridge
	{
	public:
		C_Bridge() : m_pPreFacets(&m_Facets1), m_pCurFacets(&m_Facets2), m_BridgeBoundary(10)
		{
			m_Facets1.m_numElements = 0;
			m_Facets2.m_numElements = 0;
		}

		C_Bridge& operator = (C_Bridge& other)
		{
			m_SupportPlaneNormal = other.m_SupportPlaneNormal;
			m_SupportPlaneDist = other.m_SupportPlaneDist;

			m_Facets1.m_numElements = other.m_Facets1.m_numElements;
			for(S32 i=0; i<other.m_Facets1.m_numElements; ++i)
			{
				m_Facets1[i] = other.m_Facets1[i];
			}

			m_Facets2.m_numElements = other.m_Facets2.m_numElements;
			for(S32 i=0; i<other.m_Facets2.m_numElements; ++i)
			{
				m_Facets2[i] = other.m_Facets2[i];
			}

			m_pPreFacets = (other.m_pPreFacets == &other.m_Facets1) ? &m_Facets1 : &m_Facets2;
			m_pCurFacets = (other.m_pCurFacets == &other.m_Facets1) ? &m_Facets1 : &m_Facets2;

			m_BridgeBoundary.m_numElements = other.m_BridgeBoundary.m_numElements;
			for(S32 i=0; i< other.m_BridgeBoundary.m_numElements; ++i)
			{
				m_BridgeBoundary[i]=other.m_BridgeBoundary[i];
			}
			m_Point = other.m_Point;
		//	m_Pocket = m_Pocket;
			return *this;
		}
		
		Maths::C_Normal m_SupportPlaneNormal;
		Maths::HF32 m_SupportPlaneDist;
		Maths::HVec3 m_Point;
		SharedPhysics::SortedArray<C_Facet*> m_Facets1;
		SharedPhysics::SortedArray<C_Facet*> m_Facets2;
		SharedPhysics::SortedArray<C_Facet*>* m_pPreFacets;
		SharedPhysics::SortedArray<C_Facet*>* m_pCurFacets;
		SharedPhysics::SortedArray<C_GraphEdge> m_BridgeBoundary;
		C_Pocket m_Pocket;
	};

}
#endif