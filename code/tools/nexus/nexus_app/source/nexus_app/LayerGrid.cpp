#include <nexus_app/stdafx.h>
#include <nexus_app/Nexus.h>

#include <nexus_app/MainFrm.h>

#include "../../resource.h"
#include <nexus_app/NexusSystem.h>
#include <nexus_app/NexusDoc.h>
#include <nexus_app/LayerGrid.h>
#include <nexus_app/DisplayManager.h>
#include <nexus_app/ViewportParent.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Nexus;

BEGIN_MESSAGE_MAP(LayerGrid, CExtGridWnd)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

#define NEXUS_APP_LAYER_GRID_NUM_COLUMNS 2
#define NEXUS_APP_LAYER_GRID_VISIBILITY_COLUMN 0
#define NEXUS_APP_LAYER_GRID_NAME_COLUMN 1

bool LayerGrid::Create(CWnd* pParentWnd)
{
	if (!CExtGridWnd::Create(
		pParentWnd,
		CRect(0,0,0,0),
		AFX_IDW_PANE_FIRST
		))
		return false;

	static LPCTSTR g_arrHdrNames[] = {_T("Vis"),_T("Layer Name")};
	static INT g_arrHdrExtents[] = { 30, 250 };
//	static INT g_arrHdrIconIndexes[] = { __DEMO_ICON_YELLOW_ARROW, __DEMO_ICON_BLUE_CUBE, __DEMO_ICON_NOTE, __DEMO_ICON_YELLOW_PAPER, __DEMO_ICON_NOTE };

	InitTable(g_arrHdrNames, g_arrHdrExtents, 0, NEXUS_APP_LAYER_GRID_NUM_COLUMNS);
	return true;
}

void LayerGrid::OnLButtonDown(UINT nFlags, CPoint point)
{
	CExtGridWnd::OnLButtonDown(nFlags, point);
	CExtGridHitTestInfo hitInfo;
	hitInfo.m_ptClient = point;
	HitTest(
		hitInfo,
		true, 
		true, 
		true);

	if ((hitInfo.m_nRowNo == -1) || (hitInfo.m_nColNo == -1))
		return;

	CExtGridCell* pCell = GridCellGet(
		hitInfo.m_nColNo, 
		hitInfo.m_nRowNo,
		0, 
		0);
	
	switch(hitInfo.m_nColNo)
	{
		case NEXUS_APP_LAYER_GRID_VISIBILITY_COLUMN:
			{
				NexusRender::SceneRenderer* pSceneRenderer = 
					(NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();

				NexusRender::SceneRenderer::LayerInfo* pLayerInfo = 
					pSceneRenderer->GetLayerInfoPtr(hitInfo.m_nRowNo);

				if ((pCell->GetStyle()&__EGCS_CHECKED) != 0)
				{
					//Show this layer.
					pLayerInfo->isVisible = true;
				}
				else
				{
					//Hide this layer.
					pLayerInfo->isVisible = false;
				}
				g_pDocument->UpdateAllViews(0);
			}
			break;

		default:
			break;
	};
}

void LayerGrid::InitTable(LPCTSTR *arrHdrNames, INT *arrHdrExtents, INT *arrHdrIconIndexes, int nColCount)
{
	ASSERT_VALID( this );
	ASSERT( GetSafeHwnd() != NULL && ::IsWindow(GetSafeHwnd()) );

	SiwModifyStyle(
		__ESIS_STH_PIXEL|__ESIS_STV_ITEM
			|__EGBS_SFM_FULL_ROWS|__EGBS_SF_SELECT_OUTER_ROWS
			|__EGBS_RESIZING_CELLS_OUTER
			|__EGBS_DYNAMIC_RESIZING_H
			|__EGBS_MULTI_AREA_SELECTION|__EGBS_NO_HIDE_SELECTION
			|__EGBS_GRIDLINES_V|__EGBS_LBEXT_SELECTION|__EGBS_SUBTRACT_SEL_AREAS
		,
		0,
		false
		);
	SiwModifyStyleEx(
		__EGBS_EX_HVI_EVENT_CELLS
		|__EGBS_EX_HVO_EVENT_CELLS
			|__EGBS_EX_HVI_HIGHLIGHT_ROWS
			|__EGBS_EX_HVO_HIGHLIGHT_ROWS
			|__EGBS_EX_SI_HIGHLIGHT_ROWS
			|__EGBS_EX_SO_HIGHLIGHT_ROWS
		|__EGBS_EX_CELL_TOOLTIPS_OUTER|__EGBS_EX_CELL_EXPANDING_INNER
			|__EGBS_EX_CORNER_AREAS_3D|__EGBS_EX_CORNER_AREAS_CURVE
		,
		0,
		false
		);
	BseModifyStyle(
		__EGWS_BSE_SORT_MULTIPLE_COLUMNS,
		__EGWS_BSE_DEFAULT,
		false
		); 
	BseModifyStyleEx(
		__EGBS_BSE_EX_DRAG_COLUMNS
			|__EGBS_BSE_EX_HIGHLIGHT_PRESSING_COLUMNS_OUTER
			|__EGBS_BSE_EX_HIGHLIGHT_PRESSING_STAY_COLUMNS,
		__EGWS_BSE_EX_DEFAULT,
		false
		); 
	RowRemoveAll( false );
	ColumnRemoveAll( false );
	OuterRowCountTopSet( 1L, false );
	OuterColumnCountLeftSet( 1L, false );
	OuterColumnCountRightSet( 1L, false );
	OuterColumnWidthSet( true, 0L, 30 );
	OuterColumnWidthSet( false, 0L, 14 );
	ColumnAdd( nColCount, false );
	
	// add column headers
	for( LONG nColNo = 0L; nColNo < nColCount; nColNo++ )
	{
		CExtGridCellHeader * pCell = (CExtGridCellHeader*) GridCellGetOuterAtTop( nColNo, 0L, RUNTIME_CLASS(CExtGridCellHeader) );
		pCell->ExtentSet( arrHdrExtents[nColNo] );
		pCell->TextSet( arrHdrNames[nColNo] );
//		pCell->IconIndexSet( arrHdrIconIndexes[nColNo] );
	}

	OnSwUpdateScrollBars();
	OnSwDoRedraw();
}


void LayerGrid::Destroy()
{
}

void LayerGrid::Update()
{
	RowRemoveAll(false);
	const Aftk::C_Scene* pAftkScene = Nexus::g_pNexusSystem->GetCorePtr()->GetAftkScenePtr();
	if (pAftkScene->GetNumLayers() == 0)
		return;

	RowAdd(pAftkScene->GetNumLayers(), false );

	// add rows of data
	for (U32 iRow = 0; iRow < pAftkScene->GetNumLayers(); iRow++)
	{
		const Aftk::C_LayerNode* pLayer = pAftkScene->GetLayerPtr(iRow);

		CExtGridCellHeader *pCellHeaderAtLeft, *pCellHeaderAtRight;
		CExtGridCellVariant *pCellVisibility;
		CExtGridCellStringDM *pCellString;

		// insert left and right headers
		pCellHeaderAtLeft = (CExtGridCellHeader*) GridCellGetOuterAtLeft(
			0L, 
			iRow, 
			RUNTIME_CLASS(CExtGridCellHeader));
		pCellHeaderAtLeft->ModifyStyle(
			__EGCS_HDR_FOCUS_ARROW_RESERVE_SPACE|
			__EGCS_HDR_FOCUS_ARROW_DISPLAY|
			__EGCS_HDR_ROW_COLUMN_NUMBER|
			__EGCS_TA_HORZ_RIGHT );

		pCellHeaderAtRight = (CExtGridCellHeader*) GridCellGetOuterAtRight( 
			0L, 
			iRow,
			RUNTIME_CLASS(CExtGridCellHeader));
		pCellHeaderAtRight->ModifyStyle(
			__EGCS_HDR_FOCUS_ARROW_RESERVE_SPACE|
			__EGCS_HDR_FOCUS_ARROW_DISPLAY );

		// insert allocation size
		pCellVisibility = (CExtGridCellVariant*)GridCellGet(
			0L, 
			iRow, 
			0, 
			0, 
			RUNTIME_CLASS(CExtGridCellVariant));
		pCellVisibility->ModifyStyle(
			__EGCS_NO_INPLACE_CONTROL|
			__EGCS_CHK_CHECK|
			__EGCS_ICA_HORZ_CENTER );
		pCellVisibility->ModifyStyle(__EGCS_CHECKED);

		// insert allocation address
		pCellString = (CExtGridCellStringDM*)GridCellGet(
			1L, 
			iRow,
			0, 
			0, 
			RUNTIME_CLASS(CExtGridCellStringDM));
		pCellString->TextSet(pLayer->GetName().c_str());
	}

	OnSwUpdateScrollBars();
	OnSwDoRedraw();
}

void LayerGrid::Clear()
{
	RowRemoveAll(true);
}


