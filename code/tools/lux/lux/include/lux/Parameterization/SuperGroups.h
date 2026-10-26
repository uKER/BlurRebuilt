#pragma once

class SuperGroup;

typedef Ut::Vector<SuperGroup*> TGroupList;

class SuperGroup
{
public:
	SuperGroup(int iID, int iMtl);

	box3f	m_bbox;		// bounding box of all charts in the group
	float	m_fArea;	// total area of all charts in the group
	int		m_iID;		// index of the group
	int		m_iAtlas;	// atlas that this group has been packed into
	int		m_iMaterial;// material ID of the charts on this supergroup
	int		m_iFlag;
	Ut::Vector<int>	m_chartlist;	// list of indices of all charts in the group

	void AddChart(TChartList &m_Charts, int iChart);
	void Print();
};

class Aftk::C_Scene;

class SuperGroupBuilder
{
public:
	SuperGroupBuilder(Aftk::C_Scene *pScene, TChartList &m_Charts, float fMaxGroupRadius, float fMaxArea);
	~SuperGroupBuilder();

	void	GenerateSuperGroups();
	int		GetNumGroups();
	SuperGroup *GetGroup(int i);
	Ut::Vector<SuperGroup*> &GetGroupList();

private:
	int		GetChartMtlIdx(CTextureChart *pChart);
	void	ClusterChartsByMtl(int iMtl, int iFirstChart);
	int		FindBestMatchingGroup(Ut::Vector<SuperGroup*> &groups, CTextureChart *pChart);
	SuperGroup *SuperGroupBuilder::CreateNewSuperGroup(int iMtl);
	int		GetIdFromMtl(const Aftk::C_MaterialResource *pMatRes);

	Aftk::C_Scene	*m_pScene;
	TChartList	&m_Charts;
	Ut::Vector<SuperGroup*>	m_supergroups;

	std::map<std::string, int>	m_namemap;
	int							m_nIndividualTextures;

	float		m_fMaxGroupRadius;
	float		m_fMaxArea;
};

