//-----------------------------------------------------------------------------
// File: Clustering.cpp
//
// Desc: Group charts into atlases
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "Lux/LightmapPacking/PackingRectangle.h"
#include "Lux/LightmapPacking/LightmapPacking.h"
#include "Lux/Parameterization/SuperGroups.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include <spatial_hierarchy/SpatialDividerHeaders.h>
#include <spatial_hierarchy/Kd-TreeBuilder.h>
#include "Lux/LightmapPacking/AABBBuilder.h"
#include "Lux/DebugCode.h"
#include "Lux/LightmapPacking/Clustering.h"
#include "Lux/Parameterization/SuperGroups.h"
#include <map>

#define NOT_CLUSTERED -1

// tree search touches this chart
void GroupPrim::Intersect(void *p)
{
	GroupTree *pTree = (GroupTree*)p;

	// only add groups that haven't already been clustered
	if (pTree->m_pGroupList->At(m_iGroup)->m_iAtlas != NOT_CLUSTERED)
		return;

	GroupResult result;
	result.m_iGroup = m_iGroup;

	pTree->m_Result.PushBack(result);
}

// compare group costs
bool GroupResult::operator<(const GroupResult &e) const
{
	return m_iCost < e.m_iCost;
}

//

#define NOT_CLUSTERED -1

// use clustering to build atlases
GroupTree::GroupTree(TGroupList &groups)
{
	Ut::Vector<Box3> boxlist;

	m_pGroupList = &groups;

	int nGroups = groups.Size();

	// build list of bounding boxes for groups
	boxlist.Reserve(nGroups);
	boxlist.Resize(nGroups);
	for (int i=0; i<nGroups; i++)
	{
		boxlist[i] = groups[i]->m_bbox;
		groups[i]->m_iAtlas = NOT_CLUSTERED;
	}

	// build the tree
	KdBuilder *pBuilder = new KdBuilder( boxlist,
		1,				// intersect cost
		1,				// traversal cost
		-5,				// empty bonus
		8,				// max groups per leafnode
		-1				// max depth
		);

	// free the bbox list
	boxlist.Clear();
	boxlist.Reserve(0);

	// create primitive list
	GroupPrim *pPrims = new GroupPrim[nGroups];
	for (int i=0; i<nGroups; i++)
	{
		pPrims[i].m_iMailBox = -1;
		pPrims[i].m_iGroup = i;
	}

	// copy contents of tree builder into our tree class
	CopyTreeBuilderContents(pBuilder, pPrims);
}

// trace a bbox through the tree, remember all Groups that touch the bbox
void GroupTree::Trace(box3f &bbox)
{
	m_Result.Resize(0);

	KdTreeTemplate<GroupPrim>::Trace(bbox);
}

// keep track of favourite materials to cluster together
class FavouriteMaterialList
{
public:
					FavouriteMaterialList();

	Ut::Vector<int>	m_list;

	int				m_iCurFavMtl;

	bool			ContainsFavouriteMaterial(SuperGroup *pGroup);
	void			AddMtlsToFavouriteList(SuperGroup *pGroup);
	int				GetMtlId(Aftk::C_Polygon *p);
	bool			MtlIdExists(int iID);
	bool			NextFavouriteMtl();
};

// get a unique material ID from a polygon
int	FavouriteMaterialList::GetMtlId(Aftk::C_Polygon *p)
{
	const Aftk::C_MaterialResource *pMR = GetMaterialResourcePtr( p );
	if (!pMR)
		return -1;
	else
		return pMR->GetIndex();
}

// check whether a particular material already exists in our favourites list
bool FavouriteMaterialList::MtlIdExists(int iID)
{
	for (int i=0; i<m_list.Size(); i++)
	{
		if (m_list[i] == iID)
			return true;
	}
	return false;
}

// check whether this group contains our favourite material
bool FavouriteMaterialList::ContainsFavouriteMaterial(SuperGroup *pGroup)
{
	// ran out of favourite list
	if (m_iCurFavMtl >= m_list.Size())
		return true;

	return (m_list[m_iCurFavMtl] == pGroup->m_iMaterial);
}

// advance to the next material, returns true if that was the last one
bool FavouriteMaterialList::NextFavouriteMtl()
{
	if (m_iCurFavMtl > m_list.Size())
		return false;

	m_iCurFavMtl++;

	return true;
}

// add all the materials of a chart to our favourites list (checks for duplicates)
void FavouriteMaterialList::AddMtlsToFavouriteList(SuperGroup *pGroup)
{
	// get material for group
	int iMatId = pGroup->m_iMaterial;

	if (MtlIdExists(iMatId))
		return;

	m_list.PushBack(iMatId);
}


FavouriteMaterialList::FavouriteMaterialList()
{
	m_iCurFavMtl = 0;
}

// add charts to an image list until it doesn't fit in an atlas anymore
bool CLightmapPacking::AddGroupsUntilFull(TGroupList &groups, Ut::Vector<GroupResult> &result, Ut::Vector<SuperGroup*> &outlist)
{
	FavouriteMaterialList favmtl;

	for (int i=0; i<groups.Size(); i++)
		groups[i]->m_iFlag = 0;

	int iArea = 0;
	int iCheck = 0;
	int iMaxCost = 100;
	int iCheckArea = 0;
	while (1)
	{
		// keep adding charts to the cell until we've reached the max area
		int iMaxArea = m_iTexturePackRes * m_iTexturePackRes;
		bool bFull = false;
		int iResult;
		for (iResult=0; iResult<result.Size(); iResult++)
		{
			int iGroup = result[iResult].m_iGroup;
			SuperGroup *pGroup = groups[iGroup];

			// stop when we've reached maximum cost
			if (result[iResult].m_iCost > iMaxCost)
				break; // expand radius and continue

			// check whether we've already added this one
			if (pGroup->m_iFlag == 1)
				continue;

			// if we have a favourite material, only add groups with that material on
			if (!favmtl.ContainsFavouriteMaterial(pGroup))
				continue;

			// if this group contains new materials, add it to our favourite list
			favmtl.AddMtlsToFavouriteList(pGroup);

			// add all charts contained in the current group to our image list
			for (int iGChart=0; iGChart<pGroup->m_chartlist.Size(); iGChart++)
			{
				int iChart = pGroup->m_chartlist[iGChart];
				iArea += m_pCharts->At(iChart)->m_iUsedSpace;
			}

			pGroup->m_iFlag = 1;
			outlist.PushBack(pGroup);

			// pack our list of lightmaps. stop if it doesn't fit
			if (iArea > iMaxArea)
			{
				return false;
			}

			// once we get to a high area, check whether it still fits every so often. stop if it doesn't.
			// we check at 50%, 60%, 70%, 80%, 90% and after that once every 32 charts
			if ((iArea > iMaxArea*0.90f && iCheck<=0) ||
				(iArea > iMaxArea*0.50f && (iArea-iCheckArea)>iMaxArea*0.1f))
			{
				iCheck = 32;
				iCheckArea = iArea;
				if (!TestPackGroupList(outlist, true, NULL))
				{
					return false;	// here when this lightmap is full
				}
			}
		}

		if (!favmtl.NextFavouriteMtl())
		{
			iMaxCost += 100;
			favmtl.m_iCurFavMtl = 0;

			if (iMaxCost > m_fTreeRadius + 50)
			{
				S_DEBUG( LUX::g_Log, "Atlas " << m_iCurrentAtlas << " managed to pack everything!\n");
				break; // stop and return
			}
		}
	}

	return true; // managed to pack everything
}

void CheckImageList(TpMyImageList *pTmpList, TChartList &charts)
{
	int nInstancesOfZero = 0;

	for (TpMyImageList::List_Iterator i=pTmpList->begin(); i!=pTmpList->end(); ++i)
	{
		int xi = i->data->iChart;
		CTextureChart *pChart = charts[ xi ];

		if (xi == 0)
			nInstancesOfZero++;

		if (pChart->m_iAtlas != NOT_CLUSTERED)
		{
			printf("INTERNAL ERROR 2\n");
		}
	}

	printf("%d\n",nInstancesOfZero);
}

// cluster charts together to form atlases
void CLightmapPacking::PackLightmapsUsingClustering(TChartList &charts, SuperGroupBuilder *pSuperGroups, bool bSilent)
{
	GroupTree tree(pSuperGroups->GetGroupList());

	Ut::Vector<SuperGroup*> &groups(pSuperGroups->GetGroupList());

	if (groups.Size() == 0)
		return;

	m_iCurrentAtlas = 0;

	TpMyImageList *pTmpList;

	bool bNextAtlas = true;
	bool bNoMoreGroups = false;

	Ut::Vector<SuperGroup*> groupptrlist;

	while (1)
	{
		// pick a seed group
		int iSeedGroup=0;
		while (groups[iSeedGroup]->m_iAtlas != NOT_CLUSTERED)
		{
			iSeedGroup++;
			if (iSeedGroup>=groups.Size())
				break;
		}
		if (iSeedGroup>=groups.Size())
		{
			bNoMoreGroups = true;
		}
		else
			bNoMoreGroups = false;

		// all groups are packed and no groups left in groupptr list?
		if (bNoMoreGroups && groupptrlist.Size()==0)
		{
			// we're done
			delete tree.m_pTris;
			printf("\n");
			return;
		}

		if (!bSilent)
			printf("\rPacking atlas %2d",m_iCurrentAtlas);

		if (!bNoMoreGroups)
		{
			SuperGroup *pGroup = groups[iSeedGroup];

			// get all groups that are within 250 metres of this group's centre point
			float3 vCentre = pGroup->m_bbox.GetCenter();
			float fRadius = m_fTreeRadius * m_fMeterSize;
			float3 vRadius(fRadius,fRadius,fRadius);
			box3f tracebox = box3f(vCentre - vRadius, vCentre + vRadius);

			tree.Trace(tracebox);
			assert(tree.m_Result.Size() > 0);
			if (tree.m_Result.Size() == 0)
				return;

			// calculate a cost heuristic for each chart
			int i;
			for (i=0; i<tree.m_Result.Size(); i++)
			{
				GroupResult *pResult = &tree.m_Result[i];
				SuperGroup *pOtherGroup = groups[pResult->m_iGroup];

				// cost = distance
				pResult->m_iCost = (int)(Length(vCentre - pOtherGroup->m_bbox.GetCenter()) / m_fMeterSize);
			}

			// sort by cost
			sort(&tree.m_Result[0], &tree.m_Result[0] + tree.m_Result.Size());

			bool bPackedEverything = AddGroupsUntilFull(groups, tree.m_Result, groupptrlist);

			if (bPackedEverything && !bNoMoreGroups)
			{
				// mark groups as packed so we don't get the same groups again
				for (i=0; i<groupptrlist.Size(); i++)
				{
					groupptrlist[i]->m_iAtlas = 0;
				}
				// grab more charts to pack
				continue;
			}
		}

		if (groupptrlist.Size())
		{
			// unmark our groups, in case they don't get packed
			for (int i=0; i<groupptrlist.Size(); i++)
			{
				groupptrlist[i]->m_iAtlas = NOT_CLUSTERED;
			}

			// now try to pack it - if it doesn't fit, remove the last chart and try again
			int iRollBack = 0;
			int iArea;
			while (1)
			{
				if (TestPackGroupList(groupptrlist, true, NULL))
					break;

				// remove the last group and try again
				groupptrlist.PopBack();

				iRollBack++;
			}

			// pack the atlas for real
			TestPackGroupList(groupptrlist, false, &iArea);

			S_DEBUG( LUX::g_Log, "packed atlas " << m_iCurrentAtlas << " area " << iArea << " rollback " << iRollBack << "\n");

			// done with this cell - start on next
			m_iCurrentAtlas++;
			groupptrlist.Resize(0);
		}
	}
}

// generate an image list from a group list and try to pack it
bool CLightmapPacking::TestPackGroupList(Ut::Vector<SuperGroup*> &grouplist, bool bTest, int *piArea)
{
	assert(grouplist.Size() != 0);

	int iArea = 0;
	TpMyImageList *pTmpList = new TpMyImageList;

	for (int iG=0; iG<grouplist.Size(); iG++)
	{
		SuperGroup *pGroup = grouplist[iG];
		if (!bTest)
			pGroup->m_iAtlas = m_iCurrentAtlas;

		// add all charts contained in the current group to our image list
		for (int iGChart=0; iGChart<pGroup->m_chartlist.Size(); iGChart++)
		{
			int iChart = pGroup->m_chartlist[iGChart];
			CMyImage *pImage = &m_pImageArray[iChart];
			pTmpList->push(pImage, pImage->w*256 + pImage->h);

			CTextureChart *pChart = m_pCharts->At(iChart);
			if (!bTest)
			{
				pChart->m_iAtlas = m_iCurrentAtlas;
			}
			iArea += pChart->m_iUsedSpace;
		}
	}

	bool bResult = TestPackImageList(pTmpList, bTest);

	delete pTmpList;

	if (piArea)
		*piArea = iArea;

	return bResult;
}
