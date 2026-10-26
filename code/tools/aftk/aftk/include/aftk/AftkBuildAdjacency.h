#ifdef _MSC_VER_
#pragma once
#endif

#ifndef AFTK_BUILD_ADJACENCY
#define AFTK_BUILD_ADJACENCY

namespace Aftk
{
	typedef Ut::UtVector< Ut::DynamicVector<U32> > intlist;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS

	class C_BuildAdjacency
	{
	public:
		///\brief Constructor.
		///
		C_BuildAdjacency();

		///\brief Destructor.
		///
		virtual ~C_BuildAdjacency();

		U32						m_nLodLevels;
		intlist**				m_ppVertexUsageList;

		const C_SceneMeshNode*	m_pMesh;
		bool					m_bValidMesh;

		bool				GenerateAdjacency(const C_SceneMeshNode *pMesh);
		void				FindAdjacentPolys(U32 lodLevel, U32 polyIndex);
		void				GenerateUsageList();
		int					GetAdjacentPolyIndex(U32 lodLevel, U32 polyIndex, U32 iVtx1, U32 iVtx2) const;

		friend class C_SceneMeshNode;
		friend class C_Polygon;
	};
	#endif
}
#endif
