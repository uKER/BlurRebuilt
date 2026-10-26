#pragma once

typedef Ut::UtVector< Ut::DynamicVector<U32> > intlist;

class CGenerateAdjacency
{
public:
	CGenerateAdjacency() : m_pMesh(NULL) {};
					~CGenerateAdjacency();
	
	intlist				*m_pVertexUsageList;

	Aftk::C_SceneMeshNode	*m_pMesh;
	bool				m_bValidMesh;

	bool				GenerateAdjacency(Aftk::C_SceneMeshNode *pMesh);
	void				FindAdjacentPolys(U32 i);
	void				GenerateUsageList();
	int					GetAdjacentPolyIndex(U32 iThisPoly, U32 iVtx1, U32 iVtx2);
};

