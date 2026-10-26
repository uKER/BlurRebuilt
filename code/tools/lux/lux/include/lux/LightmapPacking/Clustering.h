#pragma once

#ifndef __CLUSTERING_H__
#define __CLUSTERING_H__

class GroupPrim
{
public:
	int		m_iMailBox;
	int		m_iGroup;

	void	Intersect(void *pTree);
};

class GroupResult
{
public:
	int		m_iGroup;
	int		m_iCost;

	bool operator<(const GroupResult &e) const;
};

class GroupTree :  public KdTreeTemplate<GroupPrim>
{
public:
	GroupTree(TGroupList &groups);

	// trace a bbox through the tree, charts that touch the bbox end up in result list
	void Trace(box3f &bbox);

	Ut::Vector<GroupResult>	m_Result;

	TGroupList				*m_pGroupList;
};


#endif
