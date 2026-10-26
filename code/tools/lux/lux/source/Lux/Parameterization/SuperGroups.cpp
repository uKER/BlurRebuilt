//-----------------------------------------------------------------------------
// File: SuperGroups.cpp
//
// Desc: Generate groups of charts that share the same material
//       and that are spatially connected
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

#include "Lux/Parameterization/TextureAtlas.h"
#include "Lux/Parameterization/SuperGroups.h"
#include "Lux/Parameterization/LMPolyAttributes.h"

// supergroup itself

SuperGroup::SuperGroup(int iID, int iMtl) :
	m_iID(iID), m_fArea(0), m_iAtlas(-1), m_iMaterial(iMtl)
{
}

void SuperGroup::AddChart(TChartList &charts, int iChart)
{
	CTextureChart *pChart = charts[iChart];
	m_chartlist.PushBack(iChart);

	m_bbox = box3f(m_bbox, pChart->m_bbox);

	pChart->m_iSuperGroup = m_iID;
	m_fArea += pChart->m_iTexWidth * pChart->m_iTexHeight;
}

void SuperGroup::Print()
{
	S_DEBUG( LUX::g_Log, "supergroup no " << m_iID << " mtl " << m_iMaterial << " area " << m_fArea << " num charts " << m_chartlist.Size() << "\n");
}

// group builder

SuperGroupBuilder::SuperGroupBuilder(Aftk::C_Scene *pScene, TChartList &charts, float fMaxGroupRadius, float fMaxArea) :
	m_pScene(pScene), m_Charts(charts), m_fMaxGroupRadius(fMaxGroupRadius), m_fMaxArea(fMaxArea), m_nIndividualTextures(0)
{
	GenerateSuperGroups();
}

SuperGroupBuilder::~SuperGroupBuilder()
{
	for (int i=0; i<m_supergroups.Size(); i++)
		delete m_supergroups[i];
	m_supergroups.Destroy();
}

// get number of supergroups
int	SuperGroupBuilder::GetNumGroups()
{
	return m_supergroups.Size();
}

// get direct access to the supergroup list
Ut::Vector<SuperGroup*> &SuperGroupBuilder::GetGroupList()
{
	return m_supergroups;
}

// get pointer to a particular supergroup
SuperGroup *SuperGroupBuilder::GetGroup(int i)
{
	return m_supergroups[i];
}

// return a unique ID for each texture
int	SuperGroupBuilder::GetIdFromMtl(const Aftk::C_MaterialResource *pRes)
{
	std::string name = "<none>";
	if (pRes)
	{
		if (pRes->GetContent().GetNumShaderImageNodes())
		{
			const Aftk::C_ShaderImageNode *pSIN = pRes->GetContent().GetShaderImageNodePtr(0);
			if (pSIN)
			{
				pSIN->GetFilePlug().GetValue(name);
			}
		}
	}		

	std::map<std::string, int>::iterator iter = m_namemap.find(name);

	// found it
	if (iter != m_namemap.end())
		return m_namemap[name];

	return m_namemap[name] = m_nIndividualTextures++;	
}

// returns the index of the material of the first face of the given chart
int SuperGroupBuilder::GetChartMtlIdx(CTextureChart *pChart)
{
	// retrieve the material index of the first triangle in the chart
	const Aftk::C_Polygon *pPoly = &pChart->m_pMesh->GetPolygon(pChart->m_iFace);

	return GetIdFromMtl( GetMaterialResourcePtr(pPoly) );
}

// find best matching group for this chart (if any)
int SuperGroupBuilder::FindBestMatchingGroup(Ut::Vector<SuperGroup*> &groups, CTextureChart *pChart)
{
	box3f chartbbox = pChart->m_bbox;

	int iBest = -1;
	float fBestDistance = 0;
	for (int i=0; i<groups.Size(); i++)
	{
		// is this group full?
		if (groups[i]->m_fArea > m_fMaxArea)
			continue;

		// is the distance too large?
		box3f newbbox(groups[i]->m_bbox, chartbbox);
		float3 vLength = newbbox.GetSize();
		float fDist = vLength.x;
		if (vLength.y > fDist) fDist = vLength.y;
		if (vLength.z > fDist) fDist = vLength.z;

		if (fDist > m_fMaxGroupRadius)
			continue;

		// is it the best so far?
		if (iBest == -1 || fBestDistance > fDist)
		{
			// this is our new best match
			iBest = i;
			fBestDistance = fDist;
		}

	}

	return iBest;
}

// create a new supergroup
SuperGroup *SuperGroupBuilder::CreateNewSuperGroup(int iMtl)
{
	SuperGroup *pSG = new SuperGroup( m_supergroups.Size(), iMtl );

	m_supergroups.PushBack( pSG );

	return pSG;
}

//
void SuperGroupBuilder::ClusterChartsByMtl(int iMtl, int iFirstChart)
{
	Ut::Vector<SuperGroup*> groups;

	for (int iChart=iFirstChart; iChart<m_Charts.Size(); iChart++)
	{
		int iChartMtl = m_Charts[iChart]->m_iMaterial;
		if (iChartMtl != iMtl) continue;

		int iBest = FindBestMatchingGroup(groups, m_Charts[iChart]);

		if (iBest == -1)
		{
			// have to make a new group
			iBest = groups.Size();
			groups.PushBack( CreateNewSuperGroup(iMtl) );
		}

		// add the chart to this group
		groups[iBest]->AddChart( m_Charts, iChart );
	}
}


// every time we encounter a new material on a chart, cluster charts with those materials together
void SuperGroupBuilder::GenerateSuperGroups()
{
	Ut::Vector<bool> bMtlDone;
	int nMtls = m_pScene->GetNumMaterialNodes();
	bMtlDone.Resize( nMtls+1 );
	memset(&bMtlDone[0], 0, sizeof(bool) * nMtls+1);

	// calculate a material ID for each chart
	for (int iChart = 0; iChart < m_Charts.Size(); iChart++)
	{
		if (!m_Charts[iChart]->IsInstance())
			m_Charts[iChart]->m_iMaterial = GetChartMtlIdx( m_Charts[iChart] );
		else
			m_Charts[iChart]->m_iMaterial = -1;		 // skip instances - do them later
	}

	// 
	for (int iChart = 0; iChart < m_Charts.Size(); iChart++)
	{
		int iMtl = m_Charts[iChart]->m_iMaterial;

		if (iMtl == -1 || bMtlDone[iMtl])
			continue;

        // find other charts of this material and cluster them into supergroups
		ClusterChartsByMtl(iMtl, iChart);
		bMtlDone[iMtl] = true;
	}

	// give each instance their own supergroup
	for (int iChart = 0; iChart < m_Charts.Size(); iChart++)
	{
		// skip anything that's not an instance
		int iMtl = m_Charts[iChart]->m_iMaterial;
		if (iMtl != -1)
			continue;

		// create a supergroup for each instance
		CreateNewSuperGroup(-1)->AddChart( m_Charts, iChart );
	}

}

