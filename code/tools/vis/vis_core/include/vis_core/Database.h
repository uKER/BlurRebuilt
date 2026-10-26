#pragma once

class MeshInfo;
class BitfieldDatabase;
class CD3DMesh;

#include "Vis_Core\PolyRef.h"

struct TriRef
{
	int iNode;
	int iTri;
};

// mesh 
class MeshInfo
{
public:
					MeshInfo(Aftk::C_Scene *pScene);
					~MeshInfo();
	void			AnalyzeData(int *pData, int c, int iCell, BitfieldDatabase *pDB);

	void			SetVisible(int iTri);

	BitfieldDatabase				*m_pDatabase;

	Ut::Vector<Aftk::C_SceneMeshNode*>	m_NodePtrs;
};

class Bitfield
{
public:
			Bitfield() {};
			Bitfield(int n);

			Bitfield(Bitfield &b) { m_iID = b.m_iID, m_pData = b.m_pData; }

	void	Set(int n);
	bool	Get(int n);

	bool	operator<(const Bitfield &e) const;

	int							m_iID;
	Ut::Vector<unsigned char>	*m_pData;
};


class BitfieldDatabase
{
public:
			BitfieldDatabase(Aftk::C_Scene *pScene, Ut::Vector<PolygonRef> *pPolyRefs, int nCells);
			BitfieldDatabase(Aftk::C_Scene *pScene, const std::string filename);
			~BitfieldDatabase();

	void	Set(int iID, int iBit);
	bool	Get(int iID, int iBit);
	void	DisplayVisibilityInfo(int iCell);
	void	DisplayUsageInfo();
	void	SetVisibilityInfoToVtxColours(Aftk::C_SceneMeshNode *pNode);

	Aftk::C_SceneMeshNode *GetSceneMeshNode(int iRef);
	Aftk::C_Polygon	*GetPolygon(int iRef);
	int		GetNumRefs();
	int		GetVisibilityCount(int i);

	bool	Save(const char *psz);
	void	FindCellsWithNothingVisible(int nCells);

	void	Combine();

	Ut::Vector<PolygonRef> *m_pPolyRefs;

	Ut::Vector<Bitfield> m_bitfields;

	Aftk::C_Scene			*m_pScene;
};

