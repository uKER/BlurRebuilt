#include "stdafx.h"

#include <EditorApp/editor/viewportpanel/ViewportShadingMenu.h>
#include <EditorApp/editor/viewportpanel/ViewportMenuBar.h>
#include <EditorApp/editor/viewportpanel/ViewportPanel.h>
#include <EditorApp/editor/RenderCore.h>
#include <EditorApp/editor/RenderPipeline.h>
#include <EditorApp/editor/RenderViewport.h>
											
#include <EditorApp/misc/EventProxy.h>
#include <EditorApp/misc/MenuBuilder.h>

#include <wx/msw/registry.h>
#include <EditorApp/Misc/Registry.h>

#include <wx/tokenzr.h>

using namespace Editor;


#define RENDERPIPELINE_REGISTRY_VERSION 1

C_ViewportShadingMenu::C_ViewportShadingMenu(C_ViewportMenuBar *pMenuBar)
: m_pMenuBar(pMenuBar)
{
}

C_ViewportShadingMenu::~C_ViewportShadingMenu()
{
}

bool C_ViewportShadingMenu::InitialiseUI()
{
	C_MenuBuilderBase *pMenuBuilder = m_pMenuBar->GetMenuBuilder();

	S32 id = -1;

	{
		id = pMenuBuilder->AddMenuEntry("Shading|Points", 
			m_pMenuBar->GetViewportPanel(), this, 
			&C_ViewportShadingMenu::OnSetRenderMode, &C_ViewportShadingMenu::OnUpdateRenderMode,
			E_MenuItem_Radio);

		MenuItem item = {id, MenuItem::E_ItemId_Fillmode, (U32) Display::E_FillMode_Point};
		m_menuItems.PushBack(item);
	}

	{
		id = pMenuBuilder->AddMenuEntry("Shading|Wireframe", 
			m_pMenuBar->GetViewportPanel(), this, 
			&C_ViewportShadingMenu::OnSetRenderMode, &C_ViewportShadingMenu::OnUpdateRenderMode,
			E_MenuItem_Radio);

		MenuItem item = {id, MenuItem::E_ItemId_Fillmode, (U32) Display::E_FillMode_Wireframe};
		m_menuItems.PushBack(item);
	}

	{
		id = pMenuBuilder->AddMenuEntry("Shading|Smooth Shade All", 
			m_pMenuBar->GetViewportPanel(), this, 
			&C_ViewportShadingMenu::OnSetRenderMode, &C_ViewportShadingMenu::OnUpdateRenderMode,
			E_MenuItem_Radio);

		{
			MenuItem item = {id, MenuItem::E_ItemId_Fillmode, (U32) Display::E_FillMode_Default};
			m_menuItems.PushBack(item);
		}
		{
			MenuItem item = {id, MenuItem::E_ItemId_Fillmode, (U32) Display::E_FillMode_Solid};
			m_menuItems.PushBack(item);
		}
	}

	pMenuBuilder->AddMenuEntry("Shading|Separator0", E_MenuItem_Separator);

	{
		id = pMenuBuilder->AddMenuEntry("Shading|Back Face Culling",
			m_pMenuBar->GetViewportPanel(), this, 
			&C_ViewportShadingMenu::OnSetRenderMode, &C_ViewportShadingMenu::OnUpdateRenderMode,
			E_MenuItem_Radio);

		{
			MenuItem item = {id, MenuItem::E_ItemId_Cullmode, (U32) Display::E_CullMode_Default};
			m_menuItems.PushBack(item);
		}
		{
			MenuItem item = {id, MenuItem::E_ItemId_Cullmode, (U32) Display::E_CullMode_Ccw};
			m_menuItems.PushBack(item);
		}
	}

	{
		id = pMenuBuilder->AddMenuEntry("Shading|Front Face Culling",
			m_pMenuBar->GetViewportPanel(), this, 
			&C_ViewportShadingMenu::OnSetRenderMode, &C_ViewportShadingMenu::OnUpdateRenderMode,
			E_MenuItem_Radio);

		MenuItem item = {id, MenuItem::E_ItemId_Cullmode, (U32) Display::E_CullMode_Cw};
		m_menuItems.PushBack(item);
	}

	{
		id = pMenuBuilder->AddMenuEntry("Shading|No Face Culling",
			m_pMenuBar->GetViewportPanel(), this, 
			&C_ViewportShadingMenu::OnSetRenderMode, &C_ViewportShadingMenu::OnUpdateRenderMode,
			E_MenuItem_Radio);

		MenuItem item = {id, MenuItem::E_ItemId_Cullmode, (U32) Display::E_CullMode_None};
		m_menuItems.PushBack(item);
	}

	pMenuBuilder->AddMenuEntry("Shading|Separator1", E_MenuItem_Separator);

	{
		id = pMenuBuilder->AddMenuEntry("Shading|Wireframe On Shaded", 
			m_pMenuBar->GetViewportPanel(), this, 
			&C_ViewportShadingMenu::OnSetRenderMode, &C_ViewportShadingMenu::OnUpdateRenderMode,
			E_MenuItem_Check);

		MenuItem item = {id, MenuItem::E_ItemId_WireframeOverlay, 0};
		m_menuItems.PushBack(item);
	}

	{
		id = pMenuBuilder->AddMenuEntry("Shading|X ray", 
			m_pMenuBar->GetViewportPanel(), this, 
			&C_ViewportShadingMenu::OnSetRenderMode, &C_ViewportShadingMenu::OnUpdateRenderMode,
			E_MenuItem_Check);

		MenuItem item = {id, MenuItem::E_ItemId_XRay, 0};
		m_menuItems.PushBack(item);
	}

	{
		id = pMenuBuilder->AddMenuEntry("Shading|Texturing", 
			m_pMenuBar->GetViewportPanel(), this, 
			&C_ViewportShadingMenu::OnSetRenderMode, &C_ViewportShadingMenu::OnUpdateRenderMode,
			E_MenuItem_Check);

		MenuItem item = {id, MenuItem::E_ItemId_Texturing, 0};
		m_menuItems.PushBack(item);
	}

	pMenuBuilder->AddMenuEntry("Shading|Separator2", E_MenuItem_Separator);

	{
		id = pMenuBuilder->AddMenuEntry("Shading|Default Lighting", 
			m_pMenuBar->GetViewportPanel(), this, 
			&C_ViewportShadingMenu::OnSetRenderMode, &C_ViewportShadingMenu::OnUpdateRenderMode,
			E_MenuItem_Check);

		MenuItem item = {id, MenuItem::E_ItemId_Lighting, 0};
		m_menuItems.PushBack(item);
	}
	return true;
}

void C_ViewportShadingMenu::OnSetRenderMode(wxCommandEvent& e)
{
	const C_RenderViewport *pViewport = m_pMenuBar->GetViewportPanel()->GetViewport();
	C_RenderPipeline::ViewportRenderState& state = 
		C_RenderCore::Instance().GetRenderPipeline().GetViewportRenderState(pViewport);

	bool previouslyTextured = state.textured;

	for (S32 i=0; i<m_menuItems.Size(); ++i)
	{
		if (e.GetId() == m_menuItems[i].wxId)
		{
			switch (m_menuItems[i].id)
			{
			case MenuItem::E_ItemId_Fillmode:
				state.fillmode = m_menuItems[i].value; break;
			case MenuItem::E_ItemId_Cullmode:
				state.cullmode = m_menuItems[i].value; break;
			case MenuItem::E_ItemId_WireframeOverlay:
				state.overlay = !state.overlay; break;
			case MenuItem::E_ItemId_XRay:
				state.xray = !state.xray; break;
			case MenuItem::E_ItemId_Texturing:
				state.textured = !state.textured; break;
			case MenuItem::E_ItemId_Lighting:
				state.lit = !state.lit; break;
			};
		}
	}

	if (state.textured && state.textured != previouslyTextured)
		C_RenderCore::Instance().GetRenderPipeline().LoadTextures();

	C_RenderCore::Instance().RefreshAllViewports();
}

void C_ViewportShadingMenu::OnUpdateRenderMode(wxUpdateUIEvent& e)
{
	const C_RenderViewport *pViewport = m_pMenuBar->GetViewportPanel()->GetViewport();
	const C_RenderPipeline::ViewportRenderState& state = 
		C_RenderCore::Instance().GetRenderPipeline().GetViewportRenderState(pViewport);

	for (S32 i=0; i<m_menuItems.Size(); ++i)
	{
		if (e.GetId() == m_menuItems[i].wxId)
		{
			switch (m_menuItems[i].id)
			{
				case MenuItem::E_ItemId_Fillmode:
					//do not uncheck radio items
					if (state.fillmode == m_menuItems[i].value)
						e.Check(true);
					break;
				case MenuItem::E_ItemId_Cullmode:
					//do not uncheck radio items
					if (state.cullmode == m_menuItems[i].value)
						e.Check(true);
					break;
				case MenuItem::E_ItemId_WireframeOverlay:
					e.Check(state.overlay); break;
				case MenuItem::E_ItemId_XRay:
					e.Check(state.xray); break;
				case MenuItem::E_ItemId_Texturing:
					e.Check(state.textured); break;
				case MenuItem::E_ItemId_Lighting:
					e.Check(state.lit); break;
			};
		}
	}
}
