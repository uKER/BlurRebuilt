// MainFrm.cpp : implementation of the MainFrame class
//

#include <nexus_app/stdafx.h>
#include <nexus_app/Nexus.h>

#include "../../resource.h"
#include <nexus_app/DockingPanelRight.h>
#include <mfc_extras/ChatRichEd.h>
#include <nexus_app/CommandEdit.h>
#include <nexus_app/CommandView.h>
#include <nexus_app/DockingPanelBottom.h>
#include <nexus_app/MainFrm.h>

#include <nexus_app/NexusSystem.h>
#include <nexus_app/NexusDoc.h>
#include <nexus_app/DisplayManager.h>
#include <nexus_app/ViewportParent.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Nexus::MainFrame* Nexus::g_pMainFrame = 0;

using namespace Nexus;

IMPLEMENT_DYNCREATE(MainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(MainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND_EX(IDR_TOOLBARX, OnBarCheck)
	ON_UPDATE_COMMAND_UI(IDR_TOOLBARX, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_OUTPUT_TOOLBAR, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUT_TOOLBAR, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_RIGHT_TOOLBAR, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RIGHT_TOOLBAR, OnUpdateControlBarMenu)
	ON_WM_INITMENU()
	ON_WM_ACTIVATE()
	ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
	ON_UPDATE_COMMAND_UI(ID_SHADING_SMOOTHSHADEALL, OnUpdateShadingSmoothshadeall)
	ON_UPDATE_COMMAND_UI(ID_SHADING_WIREFRAMEALL, OnUpdateShadingWireframeall)
	ON_COMMAND(ID_SHADING_POINTSALL, OnShadingPointsall)
	ON_UPDATE_COMMAND_UI(ID_SHADING_BACKFACECULLING, OnUpdateShadingBackfaceculling)
	ON_COMMAND(ID_SHADING_TEXTURING, OnShadingTexturing)
	ON_UPDATE_COMMAND_UI(ID_SHADING_TEXTURING, OnUpdateShadingTexturing)
	ON_COMMAND(ID_SHADING_WIREFRAMEALL, OnShadingWireframeall)
	ON_COMMAND(ID_SHADING_SMOOTHSHADEALL, OnShadingSmoothshadeall)
	ON_UPDATE_COMMAND_UI(ID_SHADING_POINTSALL, OnUpdateShadingPointsall)
	ON_COMMAND(ID_SHADING_BACKFACECULLING, OnShadingBackfaceculling)
	ON_UPDATE_COMMAND_UI(ID_RENDERING_VERTEXNORMALS, OnUpdateRenderingVertexnormals)
	ON_COMMAND(ID_RENDERING_VERTEXNORMALS, OnRenderingVertexnormals)
	ON_MESSAGE(UPDATE_RENDER_TIME, UpdateTime)
	ON_COMMAND(ID_RENDERING_AXES, OnRenderingAxes)
	ON_UPDATE_COMMAND_UI(ID_RENDERING_AXES, OnUpdateRenderingAxes)
	ON_COMMAND(ID_LIGHTING_DEFAULTLIGHTING, OnLightingDefaultlighting)
	ON_UPDATE_COMMAND_UI(ID_LIGHTING_DEFAULTLIGHTING, OnUpdateLightingDefaultlighting)
	ON_COMMAND(ID_LIGHTMAPTYPE_FULLCOLOUR, OnLightmaptypeFullcolour)
	ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_FULLCOLOUR, OnUpdateLightmaptypeFullcolour)
	ON_COMMAND(ID_LIGHTMAPTYPE_MULTIPLIED, OnLightmaptypeMultiplied)
	ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_MULTIPLIED, OnUpdateLightmaptypeMultiplied)
	ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_3, OnUpdateLightmaptype3)
	ON_COMMAND(ID_LIGHTMAPTYPE_3, OnLightmaptype3)
	ON_COMMAND(ID_LIGHTMAPTYPE_2, OnLightmaptype2)
	ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_2, OnUpdateLightmaptype2)
	ON_COMMAND(ID_SHOW_NULLS, OnShowNulls)
	ON_UPDATE_COMMAND_UI(ID_SHOW_NULLS, OnUpdateShowNulls)
	ON_COMMAND(ID_SHOW_MESHES, OnShowMeshes)
	ON_UPDATE_COMMAND_UI(ID_SHOW_MESHES, OnUpdateShowMeshes)
	ON_COMMAND(ID_SHOW_SPLINES, OnShowSplines)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SPLINES, OnUpdateShowSplines)
	ON_COMMAND(ID_SHOW_INSTANCES, OnShowInstances)
	ON_UPDATE_COMMAND_UI(ID_SHOW_INSTANCES, OnUpdateShowInstances)
	ON_COMMAND(ID_SHOW_POINTLIGHTS, OnShowPointlights)
	ON_UPDATE_COMMAND_UI(ID_SHOW_POINTLIGHTS, OnUpdateShowPointlights)
	ON_COMMAND(ID_SHOW_SPOTLIGHTS, OnShowSpotlights)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SPOTLIGHTS, OnUpdateShowSpotlights)
	ON_COMMAND(ID_SHOW_DIRECTIONALLIGHTS, OnShowDirectionallights)
	ON_UPDATE_COMMAND_UI(ID_SHOW_DIRECTIONALLIGHTS, OnUpdateShowDirectionallights)
	ON_COMMAND(ID_SHOW_JOINTS, OnShowJoints)
	ON_UPDATE_COMMAND_UI(ID_SHOW_JOINTS, OnUpdateShowJoints)
	ON_COMMAND(ID_VIEW_SHOWGRID, OnViewShowgrid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWGRID, OnUpdateViewShowgrid)
	ON_COMMAND(ID_SHADING_SHOWWEIGHTING, OnShadingShowweighting)
	ON_UPDATE_COMMAND_UI(ID_SHADING_SHOWWEIGHTING, OnUpdateShadingShowweighting)
	ON_COMMAND(ID_SHADING_FIXEDFUNCTIONSKINNING, OnShadingFixedfunctionskinning)
	ON_UPDATE_COMMAND_UI(ID_SHADING_FIXEDFUNCTIONSKINNING, OnUpdateShadingFixedfunctionskinning)
	ON_COMMAND(ID_SHADING_SHOWUV0, OnShadingShowuv0)
	ON_UPDATE_COMMAND_UI(ID_SHADING_SHOWUV0, OnUpdateShadingShowuv0)
	ON_COMMAND(ID_SHADING_SHOWUV1, OnShadingShowuv1)
	ON_UPDATE_COMMAND_UI(ID_SHADING_SHOWUV1, OnUpdateShadingShowuv1)
	ON_COMMAND(ID_VIEW_SCENEBOUNDINGBOX, OnViewSceneboundingbox)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SCENEBOUNDINGBOX, OnUpdateViewSceneboundingbox)
	ON_COMMAND(ID_RENDERING_VERTEXTANGENTSPACE, OnRenderingVertextangentspace)
	ON_UPDATE_COMMAND_UI(ID_RENDERING_VERTEXTANGENTSPACE, OnUpdateRenderingVertextangentspace)
	ON_COMMAND(ID_TASKS_OUTPUTSCENEBOUNDS, OnTasksOutputscenebounds)
	ON_COMMAND(ID_TASKSREPAIRSCENE, OnTasksrepairscene)
	ON_COMMAND(ID_TASKS_CALCULATEVERTEXTANGENTSPACEFORSCENE, OnTasksCalculatevertextangentspaceforscene)
	ON_COMMAND(ID_TASKS_WELDMESHCOMPONENTSFORSCENE, OnTasksWeldmeshcomponentsforscene)
	ON_COMMAND(ID_TASKS_WELDMESHPOLYGONVERTICESFORSCENE, OnTasksWeldmeshpolygonverticesforscene)
	ON_UPDATE_COMMAND_UI(ID_TASKS_WELDMESHPOLYGONVERTICESFORSCENE, OnUpdateTasksWeldmeshpolygonverticesforscene)
	ON_UPDATE_COMMAND_UI(ID_TASKS_WELDMESHCOMPONENTSFORSCENE, OnUpdateTasksWeldmeshcomponentsforscene)
	ON_UPDATE_COMMAND_UI(ID_TASKS_CALCULATEVERTEXTANGENTSPACEFORSCENE, OnUpdateTasksCalculatevertextangentspaceforscene)
	ON_UPDATE_COMMAND_UI(ID_TASKSREPAIRSCENE, OnUpdateTasksrepairscene)
	ON_UPDATE_COMMAND_UI(ID_TASKS_OUTPUTSCENEBOUNDS, OnUpdateTasksOutputscenebounds)
//	ON_MESSAGE(WM_DROPFILES,OnDropFiles)// Message Handler for Drang and Drop
	ON_COMMAND(ID_TASKS_WELDMESH, OnTasksWeldmesh)
	ON_UPDATE_COMMAND_UI(ID_TASKS_WELDMESH, OnUpdateTasksWeldmesh)
	ON_COMMAND(ID_TANGENTSPACE_UVSET0, OnTangentspaceUvset0)
	ON_UPDATE_COMMAND_UI(ID_TANGENTSPACE_UVSET0, OnUpdateTangentspaceUvset0)
	ON_COMMAND(ID_TANGENTSPACE_UVSET1, OnTangentspaceUvset1)
	ON_UPDATE_COMMAND_UI(ID_TANGENTSPACE_UVSET1, OnUpdateTangentspaceUvset1)
	ON_COMMAND(ID_TANGENTSPACE_UVSET2, OnTangentspaceUvset2)
	ON_UPDATE_COMMAND_UI(ID_TANGENTSPACE_UVSET2, OnUpdateTangentspaceUvset2)
	ON_COMMAND(ID_TANGENTSPACE_UVSET3, OnTangentspaceUvset3)
	ON_UPDATE_COMMAND_UI(ID_TANGENTSPACE_UVSET3, OnUpdateTangentspaceUvset3)
	ON_COMMAND(ID_TANGENTSPACE_UVSET4, OnTangentspaceUvset4)
	ON_UPDATE_COMMAND_UI(ID_TANGENTSPACE_UVSET4, OnUpdateTangentspaceUvset4)
	ON_COMMAND(ID_VIEW_ANIMATIONDETAILS, OnViewAnimationdetails)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ANIMATIONDETAILS, OnUpdateViewAnimationdetails)
	ON_COMMAND(ID_VIEW_POLYCOUNT, OnViewPolycount)
	ON_UPDATE_COMMAND_UI(ID_VIEW_POLYCOUNT, OnUpdateViewPolycount)
	ON_COMMAND(ID_SHADING_CROPSHEET, OnShadingCropsheet)
	ON_UPDATE_COMMAND_UI(ID_SHADING_CROPSHEET, OnUpdateShadingCropsheet)
	ON_COMMAND(ID_TASKS_OUTPUTSELECTIONSTATS, OnTasksOutputselectionstats)
	ON_COMMAND(ID_HEADSUPDISPLAY_CAMERADETAILS, OnHeadsupdisplayCameradetails)
	ON_COMMAND(ID_TASKS_REMOVEDUPLICATEMESHES, OnTasksRemoveduplicatemeshes)
	ON_COMMAND(ID_TASKS_CREATEINSTANCES, OnTasksCreateinstances)
	ON_COMMAND(ID_SHOW_AREALIGHTS, OnShowArealights)
	ON_UPDATE_COMMAND_UI(ID_SHOW_AREALIGHTS, OnUpdateShowArealights)
	ON_UPDATE_COMMAND_UI(ID_HEADSUPDISPLAY_CAMERADETAILS, OnUpdateHeadsupdisplayCameradetails)
	ON_COMMAND(ID_SHOW_LIGHTS, OnShowLights)
	ON_UPDATE_COMMAND_UI(ID_SHOW_LIGHTS, OnUpdateShowLights)
	ON_COMMAND(ID_SELECTIONMODE_OBJECT, OnSelectionModeObject)
	ON_UPDATE_COMMAND_UI(ID_SELECTIONMODE_OBJECT, OnUpdateSelectionModeObject)
	ON_COMMAND(ID_SELECTIONMODE_POLYGON, OnSelectionModeFace)
	ON_UPDATE_COMMAND_UI(ID_SELECTIONMODE_POLYGON, OnUpdateSelectionModeFace)
	ON_COMMAND(ID_SELECTIONMODE_POINT, OnSelectionModePoint)
	ON_UPDATE_COMMAND_UI(ID_SELECTIONMODE_POINT, OnUpdateSelectionModePoint)
	ON_UPDATE_COMMAND_UI(ID_SHADING_SMOOTHSHADEALL, OnUpdateShadingSmoothshadeall)
	ON_UPDATE_COMMAND_UI(ID_SHADING_WIREFRAMEALL, OnUpdateShadingWireframeall)
	ON_COMMAND(ID_SHADING_POINTSALL, OnShadingPointsall)
	ON_UPDATE_COMMAND_UI(ID_SHADING_BACKFACECULLING, OnUpdateShadingBackfaceculling)
	ON_COMMAND(ID_SHADING_TEXTURING, OnShadingTexturing)
	ON_UPDATE_COMMAND_UI(ID_SHADING_TEXTURING, OnUpdateShadingTexturing)
	ON_COMMAND(ID_SHADING_WIREFRAMEALL, OnShadingWireframeall)
	ON_COMMAND(ID_SHADING_SMOOTHSHADEALL, OnShadingSmoothshadeall)
	ON_UPDATE_COMMAND_UI(ID_SHADING_POINTSALL, OnUpdateShadingPointsall)
	ON_COMMAND(ID_SHADING_BACKFACECULLING, OnShadingBackfaceculling)
	ON_UPDATE_COMMAND_UI(ID_RENDERING_VERTEXNORMALS, OnUpdateRenderingVertexnormals)
	ON_COMMAND(ID_RENDERING_VERTEXNORMALS, OnRenderingVertexnormals)
	ON_MESSAGE(UPDATE_RENDER_TIME, UpdateTime)
	ON_COMMAND(ID_RENDERING_AXES, OnRenderingAxes)
	ON_UPDATE_COMMAND_UI(ID_RENDERING_AXES, OnUpdateRenderingAxes)
	ON_COMMAND(ID_LIGHTING_DEFAULTLIGHTING, OnLightingDefaultlighting)
	ON_UPDATE_COMMAND_UI(ID_LIGHTING_DEFAULTLIGHTING, OnUpdateLightingDefaultlighting)
	ON_COMMAND(ID_LIGHTMAPTYPE_FULLCOLOUR, OnLightmaptypeFullcolour)
	ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_FULLCOLOUR, OnUpdateLightmaptypeFullcolour)
	ON_COMMAND(ID_LIGHTMAPTYPE_MULTIPLIED, OnLightmaptypeMultiplied)
	ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_MULTIPLIED, OnUpdateLightmaptypeMultiplied)
	ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_3, OnUpdateLightmaptype3)
	ON_COMMAND(ID_LIGHTMAPTYPE_3, OnLightmaptype3)
	ON_COMMAND(ID_LIGHTMAPTYPE_2, OnLightmaptype2)
	ON_UPDATE_COMMAND_UI(ID_LIGHTMAPTYPE_2, OnUpdateLightmaptype2)
	ON_COMMAND(ID_SHOW_NULLS, OnShowNulls)
	ON_UPDATE_COMMAND_UI(ID_SHOW_NULLS, OnUpdateShowNulls)
	ON_COMMAND(ID_SHOW_MESHES, OnShowMeshes)
	ON_UPDATE_COMMAND_UI(ID_SHOW_MESHES, OnUpdateShowMeshes)
	ON_COMMAND(ID_SHOW_SPLINES, OnShowSplines)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SPLINES, OnUpdateShowSplines)
	ON_COMMAND(ID_SHOW_INSTANCES, OnShowInstances)
	ON_UPDATE_COMMAND_UI(ID_SHOW_INSTANCES, OnUpdateShowInstances)
	ON_COMMAND(ID_SHOW_POINTLIGHTS, OnShowPointlights)
	ON_UPDATE_COMMAND_UI(ID_SHOW_POINTLIGHTS, OnUpdateShowPointlights)
	ON_COMMAND(ID_SHOW_SPOTLIGHTS, OnShowSpotlights)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SPOTLIGHTS, OnUpdateShowSpotlights)
	ON_COMMAND(ID_SHOW_DIRECTIONALLIGHTS, OnShowDirectionallights)
	ON_UPDATE_COMMAND_UI(ID_SHOW_DIRECTIONALLIGHTS, OnUpdateShowDirectionallights)
	ON_COMMAND(ID_SHOW_JOINTS, OnShowJoints)
	ON_UPDATE_COMMAND_UI(ID_SHOW_JOINTS, OnUpdateShowJoints)
	ON_COMMAND(ID_VIEW_SHOWGRID, OnViewShowgrid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWGRID, OnUpdateViewShowgrid)
	ON_COMMAND(ID_SHADING_SHOWWEIGHTING, OnShadingShowweighting)
	ON_UPDATE_COMMAND_UI(ID_SHADING_SHOWWEIGHTING, OnUpdateShadingShowweighting)
	ON_COMMAND(ID_SHADING_FIXEDFUNCTIONSKINNING, OnShadingFixedfunctionskinning)
	ON_UPDATE_COMMAND_UI(ID_SHADING_FIXEDFUNCTIONSKINNING, OnUpdateShadingFixedfunctionskinning)
	ON_COMMAND(ID_SHADING_SHOWUV0, OnShadingShowuv0)
	ON_UPDATE_COMMAND_UI(ID_SHADING_SHOWUV0, OnUpdateShadingShowuv0)
	ON_COMMAND(ID_SHADING_SHOWUV1, OnShadingShowuv1)
	ON_UPDATE_COMMAND_UI(ID_SHADING_SHOWUV1, OnUpdateShadingShowuv1)
	ON_COMMAND(ID_VIEW_SCENEBOUNDINGBOX, OnViewSceneboundingbox)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SCENEBOUNDINGBOX, OnUpdateViewSceneboundingbox)
	ON_COMMAND(ID_RENDERING_VERTEXTANGENTSPACE, OnRenderingVertextangentspace)
	ON_UPDATE_COMMAND_UI(ID_RENDERING_VERTEXTANGENTSPACE, OnUpdateRenderingVertextangentspace)
	ON_COMMAND(ID_TASKS_OUTPUTSCENEBOUNDS, OnTasksOutputscenebounds)
	ON_COMMAND(ID_TASKSREPAIRSCENE, OnTasksrepairscene)
	ON_COMMAND(ID_TASKS_CALCULATEVERTEXTANGENTSPACEFORSCENE, OnTasksCalculatevertextangentspaceforscene)
	ON_COMMAND(ID_TASKS_WELDMESHCOMPONENTSFORSCENE, OnTasksWeldmeshcomponentsforscene)
	ON_COMMAND(ID_TASKS_WELDMESHPOLYGONVERTICESFORSCENE, OnTasksWeldmeshpolygonverticesforscene)
	ON_UPDATE_COMMAND_UI(ID_TASKS_WELDMESHPOLYGONVERTICESFORSCENE, OnUpdateTasksWeldmeshpolygonverticesforscene)
	ON_UPDATE_COMMAND_UI(ID_TASKS_WELDMESHCOMPONENTSFORSCENE, OnUpdateTasksWeldmeshcomponentsforscene)
	ON_UPDATE_COMMAND_UI(ID_TASKS_CALCULATEVERTEXTANGENTSPACEFORSCENE, OnUpdateTasksCalculatevertextangentspaceforscene)
	ON_UPDATE_COMMAND_UI(ID_TASKSREPAIRSCENE, OnUpdateTasksrepairscene)
	ON_UPDATE_COMMAND_UI(ID_TASKS_OUTPUTSCENEBOUNDS, OnUpdateTasksOutputscenebounds)
	ON_COMMAND(ID_SELECTIONMODE_OBJECT, OnSelectionModeObject)
	ON_UPDATE_COMMAND_UI(ID_SELECTIONMODE_OBJECT, OnUpdateSelectionModeObject)
	ON_COMMAND(ID_SELECTIONMODE_POLYGON, OnSelectionModeFace)
	ON_UPDATE_COMMAND_UI(ID_SELECTIONMODE_POLYGON, OnUpdateSelectionModeFace)
	ON_COMMAND(ID_SELECTIONMODE_POINT, OnSelectionModePoint)
	ON_UPDATE_COMMAND_UI(ID_SELECTIONMODE_POINT, OnUpdateSelectionModePoint)
	ON_UPDATE_COMMAND_UI(ID_SELECTIONINFO_SHOWFACEINFOONSELECTION, OnUpdateShowFaceInfoOnSelection)
	ON_COMMAND(ID_SELECTIONINFO_SHOWFACEINFOONSELECTION, OnShowFaceInfoOnSelection)
	ON_UPDATE_COMMAND_UI(ID_SELECTIONINFO_SHOWNODEINFOONSELECTION, OnUpdateShowObjectInfoOnSelection)
	ON_COMMAND(ID_SELECTIONINFO_SHOWNODEINFOONSELECTION, OnShowObjectInfoOnSelection)
END_MESSAGE_MAP()


static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// MainFrame construction/destruction

MainFrame::MainFrame()
{
	memset(&m_dataFrameWP, 0, sizeof(WINDOWPLACEMENT));
	m_dataFrameWP.length = sizeof(WINDOWPLACEMENT);
	m_dataFrameWP.showCmd = SW_HIDE;
	g_pMainFrame = this;
	m_pDockingPanelRight = 0;
	m_pDockingPanelBottom = 0;
}

MainFrame::~MainFrame()
{
	if (m_pDockingPanelRight)
	{
		m_pDockingPanelRight->Destroy();
		delete m_pDockingPanelRight;
	}
	if (m_pDockingPanelBottom)
	{
		m_pDockingPanelBottom->Destroy();
		delete m_pDockingPanelBottom;
	}
}


int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	g_CmdManager->ProfileSetup( "Nexus", GetSafeHwnd());

	DragAcceptFiles();

	if (g_pViewportParent->GetDisplayManagerPtr())
	{
		NexusRender::SceneRendererSettings& settings =
			g_pViewportParent->GetDisplayManagerPtr()->GetSettings();

		CWinApp* pApp = ::AfxGetApp();

#pragma warning(push)
#pragma warning(disable:4800)
		S32 temp;

		temp = pApp->GetProfileInt(
			"shading", 
			"usebackfaceculling",		
			settings.backFaceCulling);
		settings.backFaceCulling = (bool)temp;

		temp = pApp->GetProfileInt(
			"lighting",
			"userealtimelighting", 
			settings.useRealTimeLighting);
		settings.useRealTimeLighting = (bool)temp;

		temp = pApp->GetProfileInt(
			"shading",
			"usetextures", 
			settings.useTextures);
		settings.useTextures = (bool)temp;
		
		temp = pApp->GetProfileInt(
			"shading",
			"componentMode", 
			settings.componentMode);
		settings.componentMode = (U32)temp;

		temp = pApp->GetProfileInt(
			"view", 
			"userealtimelighting",		
			settings.showGrid);
		settings.showGrid = (bool)temp;

		temp = pApp->GetProfileInt(
			"view", 
			"rendernormals",		
			settings.renderNormals);
		settings.renderNormals = (bool)temp;

		temp = pApp->GetProfileInt(
			"view", 
			"tangentspacemode",		
			settings.tangentSpaceMode);
		settings.tangentSpaceMode = temp;

		temp = pApp->GetProfileInt(
			"view", 
			"renderaxes",		
			settings.renderAxes);
		settings.renderAxes = (bool)temp;

		temp = pApp->GetProfileInt(
			"view", 
			"rendersplines",		
			settings.renderSplines);
		settings.renderSplines = (bool)temp;
		
		temp = pApp->GetProfileInt(
			"view", 
			"renderjoints",		
			settings.renderJoints);
		settings.renderJoints = (bool)temp;
		
		temp = pApp->GetProfileInt(
			"view", 
			"renderpointlights",		
			settings.renderPointLights);
		settings.renderPointLights = (bool)temp;
		
		temp = pApp->GetProfileInt(
			"view", 
			"renderdirectionallights",		
			settings.renderDirectionalLights);
		settings.renderDirectionalLights = (bool)temp;
		
		temp = pApp->GetProfileInt(
			"view", 
			"renderspotlights",		
			settings.renderSpotLights);
		settings.renderSpotLights = (bool)temp;

		temp = pApp->GetProfileInt(
			"view", 
			"renderarealights",		
			settings.renderAreaLights);
		settings.renderAreaLights = (bool)temp;
		
		temp = pApp->GetProfileInt(
			"view", 
			"renderlights",		
			settings.renderLights);
		settings.renderLights = (bool)temp;

		temp = pApp->GetProfileInt(
			"view", 
			"renderinstances",		
			settings.renderInstances);
		settings.renderInstances = (bool)temp;

		temp = pApp->GetProfileInt(
			"view", 
			"rendernulls",		
			settings.renderNulls);
		settings.renderNulls = (bool)temp;
		
		temp = pApp->GetProfileInt(
			"view", 
			"rendermeshes",		
			settings.renderMeshes);
		settings.renderMeshes = (bool)temp;
		
		temp = pApp->GetProfileInt(
			"view", 
			"->lightmapMode",		
			settings.lightmapMode);
		settings.lightmapMode = (U32)temp;
		
		temp = pApp->GetProfileInt(
			"view", 
			"skinningmode",		
			settings.skinningMode);
		settings.skinningMode = (U32)temp;
		
		temp = pApp->GetProfileInt(
			"view", 
			"boundingboxmode",		
			settings.boundingBoxMode);
		settings.boundingBoxMode = (U32)temp;
		
		temp = pApp->GetProfileInt(
			"view", 
			"animationdetails",		
			Nexus::g_pNexusSystem->GetState().showAnimationDetails);
		Nexus::g_pNexusSystem->GetState().showAnimationDetails = (bool)temp;
		
		temp = pApp->GetProfileInt(
			"view", 
			"polycount",		
			Nexus::g_pNexusSystem->GetState().showPolyCount);
		Nexus::g_pNexusSystem->GetState().showPolyCount = (bool)temp;

		temp = pApp->GetProfileInt(
			"view", 
			"cameradetails",		
			Nexus::g_pNexusSystem->GetState().showCameraDetails);
		Nexus::g_pNexusSystem->GetState().showCameraDetails = (bool)temp;

		temp = pApp->GetProfileInt(
			"view", 
			"showfaceinfoonselection",		
			Nexus::g_pNexusSystem->GetState().showFaceInfoOnSelection);
		Nexus::g_pNexusSystem->GetState().showFaceInfoOnSelection = (bool)temp;

		temp = pApp->GetProfileInt(
			"view", 
			"showobjectinfoonselection",		
			Nexus::g_pNexusSystem->GetState().showObjectInfoOnSelection);
		Nexus::g_pNexusSystem->GetState().showObjectInfoOnSelection = (bool)temp;
		
		temp = pApp->GetProfileInt(
			"rendering", 
			"tangentSpace",		
			settings.tangentSpaceMode);
		settings.tangentSpaceMode = temp;
#pragma warning(pop)
	}
	
	if (!m_wndMenuBar.Create( "Menu", this, IDR_MAINFRAME ))
	{
		TRACE0("Failed to create menubar\n");
		return -1; 
	}

	if( !m_wndToolBar.Create(
			_T( "Toolbar" ),
			this,
			IDR_TOOLBARX
		)
		|| !m_wndToolBar.LoadToolBar( IDR_TOOLBARX )
		)
	{
		TRACE0( "Failed to create toolbar" );
		return -1;
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_pDockingPanelRight = new DockingPanelRight;
	if (!m_pDockingPanelRight)
		return -1;
	m_pDockingPanelBottom = new DockingPanelBottom;
	if (!m_pDockingPanelBottom)
		return -1;

	if (!m_pDockingPanelRight->Create(this))
		return -1;
	if (!m_pDockingPanelBottom->Create(this))
		return -1;

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.FrameEnableDocking(this);

	//Initialise right tool bar
	m_pDockingPanelRight->EnableDocking(CBRS_ALIGN_ANY);
	m_pDockingPanelRight->FrameEnableDocking(this);
	m_pDockingPanelRight->DockControlBar( AFX_IDW_DOCKBAR_RIGHT, 1, this, false );
	DockControlBar(m_pDockingPanelRight);

	//Initialise bottom tool bar
	m_pDockingPanelBottom->EnableDocking(CBRS_ALIGN_ANY);
	m_pDockingPanelBottom->FrameEnableDocking(this);
	m_pDockingPanelBottom->DockControlBar( AFX_IDW_DOCKBAR_BOTTOM, 1, this, false );
	DockControlBar(m_pDockingPanelBottom);

	// this disables the hiding of rarely used commands
	CExtPopupColorMenuWnd::g_bMenuExpanding = false;
	CExtPopupColorMenuWnd::g_bMenuHighlightRarely = false;

	// load recent file list etc.
	g_CmdManager->SerializeState( "Nexus", "Bizarre Creations", "Nexus", false);

	// load toolbar positions
	if (!CExtControlBar::ProfileBarStateLoad(
		this,
		g_theApp.m_pszRegistryKey,
		g_theApp.m_pszProfileName,
		g_theApp.m_pszProfileName,
		&m_dataFrameWP
		))
	{
	}

	return 0;
}

BOOL MainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}

void MainFrame::Update()
{
	m_pDockingPanelRight->Update();
	m_pDockingPanelBottom->Update();
}

DockingPanelRight* MainFrame::GetDockingPanelRightPtr()
{
	return m_pDockingPanelRight;
}

DockingPanelBottom* MainFrame::GetDockingPanelBottomPtr()
{
	return m_pDockingPanelBottom;
}

const DockingPanelRight* MainFrame::GetDockingPanelRightPtr() const
{
	return m_pDockingPanelRight;
}

const DockingPanelBottom* MainFrame::GetDockingPanelBottomPtr() const
{
	return m_pDockingPanelBottom;
}

// MainFrame diagnostics

#ifdef _DEBUG
void MainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void MainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG

void MainFrame::OnCriticalError(Tu::Status status)
{
	MessageBox(status.ErrorString().c_str(), "Nexus Critical Error.", MB_OK);
	assert(false);
	PostMessage(WM_CLOSE, 0, 0);
}

void MainFrame::OnError(Tu::Status status)
{
	MessageBox(status.ErrorString().c_str(), "Nexus Error", MB_OK);
}

void MainFrame::OnMessage(Tu::Status status)
{
	MessageBox(status.ErrorString().c_str(), "Nexus Message", MB_OK);
}

// MainFrame message handlers
BOOL MainFrame::PreTranslateMessage(MSG* pMsg)
{
    if( m_wndMenuBar.TranslateMainFrameMessage(pMsg) )
	{
       return TRUE;
	}

	return CFrameWnd::PreTranslateMessage(pMsg);
}

void MainFrame::OnInitMenu(CMenu* pMenu)
{
	CFrameWnd::OnInitMenu(pMenu);
}

void MainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);
}

void MainFrame::OnFileImport()
{
	// TODO: Add your command handler code here
	
}

void MainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	CFrameWnd::OnUpdateFrameTitle(bAddToTitle);
}

BOOL MainFrame::DestroyWindow()
{
	CWinApp * pApp = ::AfxGetApp();

    CExtControlBar::ProfileBarStateSave(
        this,
        pApp->m_pszRegistryKey,
        pApp->m_pszProfileName,
        pApp->m_pszProfileName
        );

	if (g_pViewportParent->GetDisplayManagerPtr())
	{
		NexusRender::SceneRendererSettings& settings = g_pViewportParent->GetDisplayManagerPtr()->GetSettings();

		pApp->WriteProfileInt(
			"shading", 
			"usebackfaceculling",		
			settings.backFaceCulling);

		pApp->WriteProfileInt(
			"lighting", 
			"userealtimelighting",		
			settings.useRealTimeLighting);
		
		pApp->WriteProfileInt(
			"shading", 
			"usetextures",		
			settings.useTextures);
		pApp->WriteProfileInt(
			"shading", 
			"componentMode",		
			settings.componentMode);

		pApp->WriteProfileInt(
			"view", 
			"userealtimelighting",		
			settings.showGrid);
		pApp->WriteProfileInt(
			"view", 
			"rendernormals",		
			settings.renderNormals);
		pApp->WriteProfileInt(
			"view", 
			"tangentspacemode",		
			settings.tangentSpaceMode);
		pApp->WriteProfileInt(
			"view", 
			"renderaxes",		
			settings.renderAxes);
		pApp->WriteProfileInt(
			"view", 
			"rendersplines",		
			settings.renderSplines);
		pApp->WriteProfileInt(
			"view", 
			"renderjoints",		
			settings.renderJoints);
		pApp->WriteProfileInt(
			"view", 
			"renderpointlights",		
			settings.renderPointLights);
		pApp->WriteProfileInt(
			"view", 
			"renderdirectionallights",		
			settings.renderDirectionalLights);
		pApp->WriteProfileInt(
			"view", 
			"renderspotlights",		
			settings.renderSpotLights);
		pApp->WriteProfileInt(
			"view", 
			"renderarealights",		
			settings.renderAreaLights);
		pApp->WriteProfileInt(
			"view", 
			"renderlights",		
			settings.renderLights);
		pApp->WriteProfileInt(
			"view", 
			"renderinstances",		
			settings.renderInstances);
		pApp->WriteProfileInt(
			"view", 
			"rendernulls",		
			settings.renderNulls);
		pApp->WriteProfileInt(
			"view", 
			"rendermeshes",		
			settings.renderMeshes);
		pApp->WriteProfileInt(
			"view", 
			"->lightmapMode",		
			settings.lightmapMode);
		pApp->WriteProfileInt(
			"view", 
			"skinningmode",		
			settings.skinningMode);
		pApp->WriteProfileInt(
			"view", 
			"boundingboxmode",		
			settings.boundingBoxMode);
		pApp->WriteProfileInt(
			"view", 
			"animationdetails",		
			Nexus::g_pNexusSystem->GetState().showAnimationDetails);
		pApp->WriteProfileInt(
			"view", 
			"polycount",		
			Nexus::g_pNexusSystem->GetState().showPolyCount);
		pApp->WriteProfileInt(
			"view", 
			"cameradetails",		
			Nexus::g_pNexusSystem->GetState().showCameraDetails);
		pApp->WriteProfileInt(
			"view", 
			"showfaceinfoonselection",		
			Nexus::g_pNexusSystem->GetState().showFaceInfoOnSelection);
		pApp->WriteProfileInt(
			"view", 
			"showobjectinfoonselection",		
			Nexus::g_pNexusSystem->GetState().showObjectInfoOnSelection);
		pApp->WriteProfileInt(
			"rendering", 
			"tangentSpace",		
			settings.tangentSpaceMode);
	}

	g_CmdManager->SerializeState( "Nexus", "Bizarre Creations", "Nexus", true);

	g_CmdManager->ProfileWndRemove( GetSafeHwnd() );

	return CFrameWnd::DestroyWindow();
}

BOOL MainFrame::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle , const RECT& rect , CWnd* pParentWnd , LPCTSTR lpszMenuName , DWORD dwExStyle , CCreateContext* pContext)
{
	return CFrameWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, lpszMenuName, dwExStyle, pContext);
}

void MainFrame::OnRenderingAxes()
{
	NexusRender::SceneRenderer* pSceneRenderer = 
		(NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderAxes = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderAxes;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateRenderingAxes(CCmdUI *pCmdUI)
{
	NexusRender::SceneRenderer* pSceneRenderer = (NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderAxes)
		pCmdUI->SetCheck( true );
	else
		pCmdUI->SetCheck( false );
}

void MainFrame::OnLightingDefaultlighting()
{
	NexusRender::SceneRenderer* pSceneRenderer = (NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useRealTimeLighting = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useRealTimeLighting;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateLightingDefaultlighting(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useRealTimeLighting)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnLightmaptypeFullcolour()
{
	NexusRender::SceneRenderer* pSceneRenderer = (NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().lightmapMode = 0;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateLightmaptypeFullcolour(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().lightmapMode == 0)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnLightmaptypeMultiplied()
{
	NexusRender::SceneRenderer* pSceneRenderer = (NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().lightmapMode = 3;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateLightmaptypeMultiplied(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().lightmapMode == 3)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnUpdateLightmaptype3(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().lightmapMode == 2)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnLightmaptype3()
{
	NexusRender::SceneRenderer* pSceneRenderer = (NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().lightmapMode = 2;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnLightmaptype2()
{
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().lightmapMode = 1;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateLightmaptype2(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().lightmapMode == 1)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnShowNulls()
{
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderNulls = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderNulls;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShowNulls(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderNulls)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnShowMeshes()
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr())
	{
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderMeshes = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderMeshes;
		g_pDocument->UpdateAllViews(0);
	}
}

void MainFrame::OnUpdateShowMeshes(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderMeshes)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnShowSplines()
{
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderSplines = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderSplines;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShowSplines(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderSplines)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnShowInstances()
{
	NexusRender::SceneRenderer* pSceneRenderer = (NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderInstances = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderInstances;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShowInstances(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderInstances)
		pCmdUI->SetCheck( true );
	else
		pCmdUI->SetCheck( false );
}

void MainFrame::OnShowPointlights()
{
	NexusRender::SceneRenderer* pSceneRenderer = (NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderPointLights = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderPointLights;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShowPointlights(CCmdUI *pCmdUI)
{
	if (!g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderLights)
		pCmdUI->Enable(0);
	else
		pCmdUI->Enable(1);

	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderPointLights)
		pCmdUI->SetCheck( true );
	else
		pCmdUI->SetCheck( false );
}

void MainFrame::OnShowSpotlights()
{
	NexusRender::SceneRenderer* pSceneRenderer = (NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderSpotLights = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderSpotLights;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShowSpotlights(CCmdUI *pCmdUI)
{
	if (!g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderLights)
		pCmdUI->Enable(0);
	else
		pCmdUI->Enable(1);

	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderSpotLights)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnShowDirectionallights()
{
	NexusRender::SceneRenderer* pSceneRenderer = (NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderDirectionalLights = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderDirectionalLights;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShowDirectionallights(CCmdUI *pCmdUI)
{
	if (!g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderLights)
		pCmdUI->Enable(0);
	else
		pCmdUI->Enable(1);

	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderDirectionalLights)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnShowJoints()
{
	NexusRender::SceneRenderer* pSceneRenderer = (NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderJoints = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderJoints;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShowJoints(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderJoints)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnViewShowgrid()
{
	NexusRender::SceneRenderer* pSceneRenderer = (NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().showGrid = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().showGrid;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateViewShowgrid(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().showGrid)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnShadingShowweighting()
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().componentMode == 1)
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().componentMode = 0;
	else
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().componentMode = 1;

	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShadingShowweighting(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().componentMode == 1)
		pCmdUI->SetCheck( true );
	else
		pCmdUI->SetCheck( false );
}

void MainFrame::OnDebugOutputscenegraph()
{
	Nexus::g_pNexusSystem->GetCorePtr()->GetAftkScenePtr()->OutputSceneGraph();
}

void MainFrame::OnShadingFixedfunctionskinning()
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().skinningMode)
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().skinningMode = 0;
	else
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().skinningMode = 1;

	g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->OnSkinningModeChanged();
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShadingFixedfunctionskinning(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().skinningMode)
		pCmdUI->SetCheck( true );
	else
		pCmdUI->SetCheck( false );
}

void MainFrame::OnShadingShowuv0()
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().componentMode == 2)
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().componentMode = 0;
	else
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().componentMode = 2;

	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShadingShowuv0(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().componentMode == 2)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnShadingShowuv1()
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().componentMode == 3)
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().componentMode = 0;
	else
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().componentMode = 3;

	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShadingShowuv1(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().componentMode == 3)
		pCmdUI->SetCheck( true );
	else
		pCmdUI->SetCheck( false );
}

void MainFrame::OnViewSceneboundingbox()
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().boundingBoxMode == 1)
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().boundingBoxMode = 0;
	else
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().boundingBoxMode = 1;

	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateViewSceneboundingbox(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().boundingBoxMode == 1)
		pCmdUI->SetCheck( true );
	else
		pCmdUI->SetCheck( false );
}

void MainFrame::OnRenderingVertextangentspace()
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode == -1)
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode = 0;
	else
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode = -1;

	g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->OnGizmoRenderingChanged();
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateRenderingVertextangentspace(CCmdUI *pCmdUI)
{
	NEXUS_DISABLE_IF_NOT_LOADED;
}

void MainFrame::OnTasksOutputscenebounds()
{
	Nexus::AftkTasks::PrintSceneBounds();
}

void MainFrame::OnTasksrepairscene()
{
	Tu::Status status = Nexus::g_pNexusSystem->GetCorePtr()->ProgressCallback()->
		ExecuteTask(AftkTasks::RepairScene, Nexus::StringArray());
}

void MainFrame::OnTasksCalculatevertextangentspaceforscene()
{
	/*
	CalculateTangentsAndBinormalsDialog dialog;
	dialog.DoModal();
	//Abort if dialog cancelled.
	if (dialog.m_cancelled) return;

	//Weld meshes prior to calculation if desired.
	if (dialog.m_bWeldCheck)
	{
	    Nexus::StringArray args;

		std::stringstream ss; 
		ss << dialog.m_threshold;
		args.PushBack(ss.str());

		Nexus::g_pNexusSystem->GetCorePtr()->ProgressCallback()->
			ExecuteTask(AftkTasks::WeldMesh, args);
	}

    Nexus::StringArray args;
	
	//Copy magnitude scale.
	{
		std::stringstream ss; ss << dialog.m_mag;
		args.PushBack(ss.str());
	}
	//Copy angle.
	{
		std::stringstream ss; ss << dialog.m_angle;
		args.PushBack(ss.str());
	}
	//Copy threshold.
	{
		std::stringstream ss; ss << dialog.m_threshold;
		args.PushBack(ss.str());
	}	
	//Normalise tangent space vectors?
	{
		if (dialog.m_bNormaliseCheck)
			args.PushBack(std::string("true"));
		else
			args.PushBack(std::string("false"));
	}

	Tu::Status status = Nexus::g_pNexusSystem->GetCorePtr()->ProgressCallback()->
		ExecuteTask(AftkUtTasks::CalculateSmoothTangentsAndBinormals, args);
	NEXUS_VIEW_CHECK_ERROR;
	g_pDocument->UpdateAllViews(0);
	*/
}

void MainFrame::OnTasksWeldmeshcomponentsforscene()
{
	Tu::Status status = Nexus::g_pNexusSystem->GetCorePtr()->ProgressCallback()->
		ExecuteTask(AftkTasks::WeldMeshComponents, Nexus::StringArray());
	NEXUS_VIEW_CHECK_ERROR;
}

void MainFrame::OnTasksWeldmeshpolygonverticesforscene()
{
	Tu::Status status = Nexus::g_pNexusSystem->GetCorePtr()->ProgressCallback()->
		ExecuteTask(AftkTasks::WeldMeshPolygonVertices, Nexus::StringArray());
	NEXUS_VIEW_CHECK_ERROR;
}

void MainFrame::OnUpdateTasksWeldmeshpolygonverticesforscene(CCmdUI *pCmdUI)
{
	NEXUS_DISABLE_IF_NOT_LOADED;
}

void MainFrame::OnUpdateTasksWeldmeshcomponentsforscene(CCmdUI *pCmdUI)
{
	NEXUS_DISABLE_IF_NOT_LOADED;
}

void MainFrame::OnUpdateTasksCalculatevertextangentspaceforscene(CCmdUI *pCmdUI)
{
	NEXUS_DISABLE_IF_NOT_LOADED;
}

void MainFrame::OnUpdateTasksrepairscene(CCmdUI *pCmdUI)
{
	NEXUS_DISABLE_IF_NOT_LOADED;
}

void MainFrame::OnUpdateTasksOutputscenebounds(CCmdUI *pCmdUI)
{
	NEXUS_DISABLE_IF_NOT_LOADED;
}

void MainFrame::OnTasksWeldmesh()
{
	/*
	MeshWeldingOptions dialog;
	dialog.DoModal();
	//Abort if dialog cancelled.
	if (dialog.m_cancelled) return;

	//Weld meshes prior to calculation if desired.
    Nexus::StringArray args;

	std::stringstream ss; ss << dialog.m_weldDistance;
	args.PushBack(ss.str());

	Tu::Status status = Nexus::g_pNexusSystem->GetCorePtr()->ProgressCallback()->
		ExecuteTask(AftkTasks::WeldMesh, args);
	NEXUS_VIEW_CHECK_ERROR;
	g_pDocument->UpdateAllViews(0);
	*/
}

void MainFrame::OnUpdateTasksWeldmesh(CCmdUI *pCmdUI)
{
	NEXUS_DISABLE_IF_NOT_LOADED;
}

void MainFrame::OnTangentspaceUvset0()
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode == 0)
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode = -1;
	else
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode = 0;

	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateTangentspaceUvset0(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode != 0)
		pCmdUI->SetCheck(false);
	else
		pCmdUI->SetCheck(true);
}

void MainFrame::OnTangentspaceUvset1()
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode == 1)
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode = -1;
	else
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode = 1;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateTangentspaceUvset1(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode != 1)
		pCmdUI->SetCheck(false);
	else
		pCmdUI->SetCheck(true);
}

void MainFrame::OnTangentspaceUvset2()
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode == 2)
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode = -1;
	else
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode = 2;

	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateTangentspaceUvset2(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode != 2)
		pCmdUI->SetCheck( false );
	else
		pCmdUI->SetCheck( true );
}

void MainFrame::OnTangentspaceUvset3()
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode == 3)
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode = -1;
	else
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode = 3;

	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateTangentspaceUvset3(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode != 3)
		pCmdUI->SetCheck(false);
	else
		pCmdUI->SetCheck(true);
}

void MainFrame::OnTangentspaceUvset4()
{
	NexusRender::SceneRenderer* pSceneRenderer = (NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode == 4)
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode = -1;
	else
		g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode = 4;

	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateTangentspaceUvset4(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().tangentSpaceMode != 4)
		pCmdUI->SetCheck(false);
	else
		pCmdUI->SetCheck(true);
}


void MainFrame::OnViewAnimationdetails()
{
	Nexus::g_pNexusSystem->GetState().showAnimationDetails = 
		!Nexus::g_pNexusSystem->GetState().showAnimationDetails;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateViewAnimationdetails(CCmdUI *pCmdUI)
{
	if (Nexus::g_pNexusSystem->GetState().showAnimationDetails)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnViewPolycount()
{
	Nexus::g_pNexusSystem->GetState().showPolyCount =
		!Nexus::g_pNexusSystem->GetState().showPolyCount;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateViewPolycount(CCmdUI *pCmdUI)
{
	if (Nexus::g_pNexusSystem->GetState().showPolyCount)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnShadingCropsheet()
{
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useCropSheet = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useCropSheet;
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useCropSheet)
	{
		if (!g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->CropSheetTexturesLoaded())
		{
			g_pViewportParent->GetDisplayManagerPtr()->OnLoadCropSheetTextures();
		}
	}
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShadingCropsheet(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useCropSheet)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnTasksOutputselectionstats()
{
	AftkTasks::PrintSelectedNodeInfo();
}

void MainFrame::OnHeadsupdisplayCameradetails()
{
	Nexus::g_pNexusSystem->GetState().showCameraDetails = 
		!Nexus::g_pNexusSystem->GetState().showCameraDetails;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnTasksRemoveduplicatemeshes()
{
	Tu::Status status = Nexus::g_pNexusSystem->GetCorePtr()->ProgressCallback()->
		ExecuteTask(AftkUtTasks::RemoveDuplicateMeshes, Nexus::StringArray());
	NEXUS_VIEW_CHECK_ERROR;
}

void MainFrame::OnUpdateShadingSmoothshadeall(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().fillMode == NexusRender::E_FillMode_Solid)
	{
		pCmdUI->SetCheck( true );
	}
	else
	{
		pCmdUI->SetCheck( false);
	}
}

void MainFrame::OnUpdateShadingWireframeall(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().fillMode == NexusRender::E_FillMode_WireFrame)
	{
		pCmdUI->SetCheck( true );
	}
	else
	{
		pCmdUI->SetCheck( false );
	}
}

void MainFrame::OnShadingPointsall()
{
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().fillMode = NexusRender::E_FillMode_Point;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShadingBackfaceculling(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().backFaceCulling)
		pCmdUI->SetCheck( true );
	else
		pCmdUI->SetCheck( false );
}

void MainFrame::OnUpdateShadingTexturing(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useTextures)
	{
		pCmdUI->SetCheck( true );
	}
	else
	{
		pCmdUI->SetCheck( false );
	}
}

void MainFrame::OnShadingWireframeall()
{
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().fillMode = NexusRender::E_FillMode_WireFrame;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnShadingSmoothshadeall()
{
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().fillMode = NexusRender::E_FillMode_Solid;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnShadingTexturing()
{
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useTextures = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useTextures;
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().useTextures)
	{
		if (!g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->TexturesLoaded())
		{
			g_pViewportParent->GetDisplayManagerPtr()->OnLoadTextures();
		}
	}
		
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShadingPointsall(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().fillMode == NexusRender::E_FillMode_Point)
	{
		pCmdUI->SetCheck( true );
	}
	else
	{
		pCmdUI->SetCheck( false );
	}
}

void MainFrame::OnShadingBackfaceculling()
{
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().backFaceCulling = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().backFaceCulling;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateRenderingVertexnormals(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderNormals)
		pCmdUI->SetCheck( true );
	else
		pCmdUI->SetCheck( false );
}

void MainFrame::OnRenderingVertexnormals()
{
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderNormals = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderNormals;
	g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->OnGizmoRenderingChanged();
	g_pDocument->UpdateAllViews(0);
}


void MainFrame::OnTasksCreateinstances()
{
	Tu::Status status = Nexus::g_pNexusSystem->GetCorePtr()->ProgressCallback()->
		ExecuteTask(AftkUtTasks::CreateInstances, Nexus::StringArray());
	NEXUS_VIEW_CHECK_ERROR;
}

void MainFrame::OnShowArealights()
{
	NexusRender::SceneRenderer* pSceneRenderer = (NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderAreaLights = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderAreaLights;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShowArealights(CCmdUI *pCmdUI)
{
	if (!g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderLights)
		pCmdUI->Enable(0);
	else
		pCmdUI->Enable(1);
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderAreaLights)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnUpdateHeadsupdisplayCameradetails(CCmdUI *pCmdUI)
{
	if (Nexus::g_pNexusSystem->GetState().showCameraDetails)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnShowLights()
{
	NexusRender::SceneRenderer* pSceneRenderer = (NexusRender::SceneRenderer*)g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr();
	g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderLights = !g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderLights;
	g_pDocument->UpdateAllViews(0);
}

void MainFrame::OnUpdateShowLights(CCmdUI *pCmdUI)
{
	if (g_pViewportParent->GetDisplayManagerPtr()->GetSettings().renderLights)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnSelectionModeObject()
{
	Nexus::g_pNexusSystem->GetState().selectionMode = Nexus::E_SelectionMode_Object;

}

void MainFrame::OnUpdateSelectionModeObject(CCmdUI *pCmdUI)
{
	if (Nexus::g_pNexusSystem->GetState().selectionMode == Nexus::E_SelectionMode_Object)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnSelectionModeFace()
{
	Nexus::g_pNexusSystem->GetState().selectionMode = Nexus::E_SelectionMode_Face;
}

void MainFrame::OnUpdateSelectionModeFace(CCmdUI *pCmdUI)
{
	if (Nexus::g_pNexusSystem->GetState().selectionMode == Nexus::E_SelectionMode_Face)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnSelectionModePoint()
{
	Nexus::g_pNexusSystem->GetState().selectionMode = Nexus::E_SelectionMode_Point;
}

void MainFrame::OnUpdateSelectionModePoint(CCmdUI *pCmdUI)
{
	if (Nexus::g_pNexusSystem->GetState().selectionMode == Nexus::E_SelectionMode_Point)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnUpdateShowFaceInfoOnSelection(CCmdUI* pCmdUI)
{
	if (Nexus::g_pNexusSystem->GetState().showFaceInfoOnSelection)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnShowFaceInfoOnSelection()
{
	Nexus::g_pNexusSystem->GetState().showFaceInfoOnSelection = 
		!Nexus::g_pNexusSystem->GetState().showFaceInfoOnSelection;
}

void MainFrame::OnUpdateShowObjectInfoOnSelection(CCmdUI* pCmdUI)
{
	if (Nexus::g_pNexusSystem->GetState().showObjectInfoOnSelection)
		pCmdUI->SetCheck(true);
	else
		pCmdUI->SetCheck(false);
}

void MainFrame::OnShowObjectInfoOnSelection()
{
	Nexus::g_pNexusSystem->GetState().showObjectInfoOnSelection = 
		!Nexus::g_pNexusSystem->GetState().showObjectInfoOnSelection;
}

// Display next cell in animation sequence 
LRESULT MainFrame::UpdateTime(WPARAM wParam, LPARAM lParam)
{
	g_pDocument->UpdateAllViews(0);
    return 0;
}

