#include "stdafx.h"

#include <EditorApp/editor/viewportpanel/ViewportRendererMenu.h>
#include <EditorApp/editor/viewportpanel/ViewportMenuBar.h>
#include <EditorApp/editor/viewportpanel/ViewportPanel.h>
#include <EditorApp/editor/RenderCore.h>
#include <EditorApp/editor/RenderPipeline.h>

#include <EditorApp/misc/EventProxy.h>
#include <EditorApp/misc/MenuBuilder.h>

#include <wx/tokenzr.h>

using namespace Editor;

C_ViewportRendererMenu::C_ViewportRendererMenu(C_ViewportMenuBar *pMenuBar)
: m_pMenuBar(pMenuBar)
{
	m_LabelModeId[0] = m_LabelModeId[1] = m_LabelModeId[2] = m_LabelModeId[3] = 
		Ut::Pair<S32, U8>(-1, 0);
}

C_ViewportRendererMenu::~C_ViewportRendererMenu()
{
}

bool C_ViewportRendererMenu::InitialiseUI()
{
	C_MenuBuilderBase *pMenuBuilder = m_pMenuBar->GetMenuBuilder();
	S32 id = -1;

	id = pMenuBuilder->AddMenuEntry("Renderer|Show Grid",
		m_pMenuBar->GetViewportPanel(), this, 
		&C_ViewportRendererMenu::OnSetGridState, &C_ViewportRendererMenu::OnUpdateGridState,
		E_MenuItem_Check);

	id = pMenuBuilder->AddMenuEntry("Renderer|Antialias Lines",
		m_pMenuBar->GetViewportPanel(), this, 
		&C_ViewportRendererMenu::OnSetLineMode, &C_ViewportRendererMenu::OnUpdateLineMode,
		E_MenuItem_Check);

	id = pMenuBuilder->AddMenuEntry("Renderer|Labels|Mesh", 
		m_pMenuBar->GetViewportPanel(), this, 
		&C_ViewportRendererMenu::OnSetLabelMode, &C_ViewportRendererMenu::OnUpdateLabelMode,
		E_MenuItem_Check);
	m_LabelModeId[0] = Ut::Pair<S32, U8>(id, 0x01);

	id = pMenuBuilder->AddMenuEntry("Renderer|Labels|Locator",
		m_pMenuBar->GetViewportPanel(), this, 
		&C_ViewportRendererMenu::OnSetLabelMode, &C_ViewportRendererMenu::OnUpdateLabelMode,
		E_MenuItem_Check);
	m_LabelModeId[1] = Ut::Pair<S32, U8>(id, 0x02);

	id = pMenuBuilder->AddMenuEntry("Renderer|Labels|Properties",
		m_pMenuBar->GetViewportPanel(), this, 
		&C_ViewportRendererMenu::OnSetLabelMode, &C_ViewportRendererMenu::OnUpdateLabelMode,
		E_MenuItem_Check);
	m_LabelModeId[2] = Ut::Pair<S32, U8>(id, 0x04);

	pMenuBuilder->AddMenuEntry("Renderer|Labels|Separator0", E_MenuItem_Separator);

	id = pMenuBuilder->AddMenuEntry("Renderer|Labels|Selection",
		m_pMenuBar->GetViewportPanel(), this, 
		&C_ViewportRendererMenu::OnSetLabelMode, &C_ViewportRendererMenu::OnUpdateLabelMode,
		E_MenuItem_Check);
	m_LabelModeId[3] = Ut::Pair<S32, U8>(id, 0x08);

	return true;
}

void C_ViewportRendererMenu::OnSetGridState(wxCommandEvent& e)
{
	const C_RenderViewport *pViewport = m_pMenuBar->GetViewportPanel()->GetViewport();
	C_RenderPipeline::ViewportRenderState& state = 
		C_RenderCore::Instance().GetRenderPipeline().GetViewportRenderState(pViewport);

	state.grid = !state.grid;
	C_RenderCore::Instance().RefreshAllViewports();
}

void C_ViewportRendererMenu::OnUpdateGridState(wxUpdateUIEvent& e)
{
	const C_RenderViewport *pViewport = m_pMenuBar->GetViewportPanel()->GetViewport();
	const C_RenderPipeline::ViewportRenderState& state = 
		C_RenderCore::Instance().GetRenderPipeline().GetViewportRenderState(pViewport);
	e.Check(state.grid);
}

void C_ViewportRendererMenu::OnSetLineMode(wxCommandEvent& e)
{
	const C_RenderViewport *pViewport = m_pMenuBar->GetViewportPanel()->GetViewport();
	C_RenderPipeline::ViewportRenderState& state = 
		C_RenderCore::Instance().GetRenderPipeline().GetViewportRenderState(pViewport);

	state.lineAA = !state.lineAA;
	C_RenderCore::Instance().RefreshAllViewports();
}

void C_ViewportRendererMenu::OnUpdateLineMode(wxUpdateUIEvent& e)
{
	const C_RenderViewport *pViewport = m_pMenuBar->GetViewportPanel()->GetViewport();
	const C_RenderPipeline::ViewportRenderState& state = 
		C_RenderCore::Instance().GetRenderPipeline().GetViewportRenderState(pViewport);

	e.Check(state.lineAA);
}

void C_ViewportRendererMenu::OnSetLabelMode(wxCommandEvent& e)
{
	const C_RenderViewport *pViewport = m_pMenuBar->GetViewportPanel()->GetViewport();
	C_RenderPipeline::ViewportRenderState& state = 
		C_RenderCore::Instance().GetRenderPipeline().GetViewportRenderState(pViewport);

	for (U32 i=0; i<4; ++i)
	{
		if (e.GetId() == m_LabelModeId[i].first)
		{
			switch (m_LabelModeId[i].second)
			{
			case 0x01:
				state.labelDgSelection = false;
				state.labelAftkRef = !state.labelAftkRef;
				break;
			case 0x02:
				state.labelDgSelection = false;
				state.labelDgSceneNodes = !state.labelDgSceneNodes;
				break;

			case 0x04:
				state.labelDgSelection = false;
				state.labelDgPropNodes = !state.labelDgPropNodes;
				break;
			case 0x08:
				state.labelAftkRef = state.labelDgSceneNodes = state.labelDgPropNodes = false;
				state.labelDgSelection = !state.labelDgSelection;
				break;
			}
		}
	}
	C_RenderCore::Instance().RefreshAllViewports();
}

void C_ViewportRendererMenu::OnUpdateLabelMode(wxUpdateUIEvent& e)
{
	const C_RenderViewport *pViewport = m_pMenuBar->GetViewportPanel()->GetViewport();
	const C_RenderPipeline::ViewportRenderState& state = 
		C_RenderCore::Instance().GetRenderPipeline().GetViewportRenderState(pViewport);

	for (U32 i=0; i<4; ++i)
	{
		if (e.GetId() == m_LabelModeId[i].first)
		{
			switch (m_LabelModeId[i].second)
			{
			case 0x01:
				e.Check(state.labelAftkRef); break;
			case 0x02:
				e.Check(state.labelDgSceneNodes); break;
			case 0x04:
				e.Check(state.labelDgPropNodes); break;
			case 0x08:
				e.Check(state.labelDgSelection); break;
			}
		}
	}
}
