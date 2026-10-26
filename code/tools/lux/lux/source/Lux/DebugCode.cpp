#include "stdafx.h"
#include "Lux/LightmapPacking/PackingRectangle.h"
#include "Lux/LightmapPacking/LightmapPacking.h"
#include "Lux/Parameterization/SuperGroups.h"
#include "Lux/Parameterization/LMPolyAttributes.h"
#include "Lux/DebugCode.h"

void ColourMeshByAtlas(TChartList *pCharts)
{
	int iAtlasRGBs[400];

	for (int i=0; i<400; i++)
	{
		int iRGB = (rand()&255) + ((rand()&255)<<8) + ((rand()&255)<<16);

		iAtlasRGBs[i] = iRGB;
	}


	for (int iChart=0; iChart<pCharts->Size(); iChart++)
	{
		CTextureChart *pChart = pCharts->At(iChart);
		if (!pChart)
			break;

		Aftk::C_SceneMeshNode *m_pMesh = pChart->m_pMesh;

		int iRGB = (rand()&255) + ((rand()&255)<<8) + ((rand()&255)<<16);

		iRGB = iAtlasRGBs[pChart->m_iAtlas];

		// iterate all polygons in this chart
		int iFace = pChart->m_iFace;

		while (iFace != -1)
		{
			Aftk::C_Polygon *pPoly = &m_pMesh->GetPolygon(iFace);

			// colour this face
			SetPolygonColour(pPoly, iRGB);

			iFace = POLY_DATA(iFace)->iChartLink;
		}
	}

}

void ColourMeshBySuperGroup(TChartList *pCharts)
{
	int iAtlasRGBs[400];

	for (int i=0; i<400; i++)
	{
		int iRGB = (rand()&255) + ((rand()&255)<<8) + ((rand()&255)<<16);

		iAtlasRGBs[i] = iRGB;
	}

	for (int iChart=0; iChart<pCharts->Size(); iChart++)
	{
		CTextureChart *pChart = pCharts->At(iChart);
		if (!pChart)
			break;

		Aftk::C_SceneMeshNode *m_pMesh = pChart->m_pMesh;

		int iRGB = (rand()&255) + ((rand()&255)<<8) + ((rand()&255)<<16);

		iRGB = iAtlasRGBs[pChart->m_iSuperGroup];

		// iterate all polygons in this chart
		int iFace = pChart->m_iFace;

		while (iFace != -1)
		{
			Aftk::C_Polygon *pPoly = &m_pMesh->GetPolygon(iFace);

			// colour this face
			SetPolygonColour(pPoly, iRGB);

			iFace = POLY_DATA(iFace)->iChartLink;
		}
	}

}

void ColourChart(CTextureChart *pChart, int iRGB)
{
	Aftk::C_SceneMeshNode *m_pMesh = pChart->m_pMesh;

	// iterate all polygons in this chart
	int iFace = pChart->m_iFace;

	while (iFace != -1)
	{
		Aftk::C_Polygon *pPoly = &m_pMesh->GetPolygon(iFace);

		// colour this face
		SetPolygonColour(pPoly, iRGB);

		iFace = POLY_DATA(iFace)->iChartLink;
	}

}

void SetPolygonColour(Aftk::C_Polygon *pPoly, int iColour)
{
	Aftk::C_SceneMeshNode *pNode = pPoly->GetMeshNodePtr();

	int iVtxColChan = -1;

	for (int i=0; i<pNode->GetNumVertexDataChannels(); i++)
	{
		Aftk::C_VertexDataChannelInfo *pInfo = &pNode->GetVertexDataChannelInfo(i);
		if (pInfo->GetName() == "vertexColour")
		{
			if (pInfo->GetDataType() == Aftk::E_GenericAttributeType_U32)
			{
				iVtxColChan = i;
				break;
			}
			else
			{
				pInfo->SetName("oldVertexColour");
			}
		}
	}

	if (iVtxColChan == -1)
	{
		iVtxColChan = pNode->GetNumVertexDataChannels();

		Aftk::C_VertexDataChannelInfo avdci;
		avdci.SetName("vertexColour");
		avdci.SetDataType(Aftk::E_GenericAttributeType_U32);
		pNode->SetVertexDataChannelInfo(iVtxColChan, avdci);

		U32 iWhite = 0xffffffff;

		// set all vertex colours to white by default
		for (int i=0; i<pNode->GetNumPolygonVertices(); i++)
		{
			pNode->GetPolygonVertex(i).SetData(iVtxColChan, iWhite);
		}
	}

	// set vertex colour here depending on visibility
	for (int iVtx=0; iVtx<3; iVtx++)
	{
		Aftk::C_PolygonVertex *pVtx = &pPoly->GetPolygonVertex(iVtx);

		pVtx->SetData(iVtxColChan, iColour);
	}
}
