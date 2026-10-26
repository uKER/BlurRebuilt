#include "stdafx.h"

#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/RenderViewport.h>

#include <EditorApp/Editor/Primitives.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/AftkSceneRenderer.h>
#include <EditorApp/Editor/SimpleRenderer.h>
#include <EditorApp/Editor/ComplexRenderer.h>
#include <EditorApp/Editor/SpriteRenderer.h>
#include <EditorApp/Editor/VertexRenderer.h>
#include <EditorApp/Editor/Collision/CollisionRenderer.h>
#include <EditorApp/Editor/HudElements.h>

#include <EditorApp/Editor/manipulators/Manipulator.h>
#include <EditorApp/Editor/ActorScene.h>
#include <EditorApp/Editor/AftkRefScene.h>
#include <EditorApp/Editor/DgNodeSelection.h>

#include <EditorApp/Editor/Manipulators/SelectManipulator.h>

#include <wx/msw/registry.h>
#include <EditorApp/Misc/Registry.h>

#include <EditorApp/Tasks/UITasks.h>

#include <Editorlib/Core.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/SceneXtActorNode.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Aftk/AftkRefManager.h>

#define RENDERPIPELINE_REGISTRY_VERSION 1

namespace Editor
{

C_RenderPipeline::C_RenderPipeline()
: m_pSceneRenderer(0)
, m_pSimpleRenderer(0)
, m_pComplexRenderer(0)
, m_pVertexRenderer(0)
, m_pSpriteRenderer(0)
, m_pBasicRenderer(0)
, m_pFontRenderer(0)
, m_pCollisionRenderer(0)
, m_pDebugRenderSys(0)
{
	// Load default render state
	m_globalRenderState.gridSize = Tasks::GetGridSize();
	m_globalRenderState.gridSpacing = Tasks::GetGridSpacing();
	m_globalRenderState.hudSelectionCount = Tasks::GetShowHUDSelectedActors();

}

C_RenderPipeline::~C_RenderPipeline()
{
}

bool C_RenderPipeline::CreateRenderers()
{
	// create debug renderer & set as default
	m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;
	DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);

	m_pSimpleRenderer = new C_SimpleRenderer();
	m_pSimpleRenderer->Create();
	m_pSimpleRenderer->CreateVolatile();

	m_pComplexRenderer = new C_ComplexRenderer();
	m_pComplexRenderer->Create();
	m_pComplexRenderer->CreateVolatile();

	m_pVertexRenderer = new C_VertexRenderer(1048576);
	m_pVertexRenderer->Create();
	m_pVertexRenderer->CreateVolatile();

	m_pSpriteRenderer = new C_SpriteRenderer();
	m_pSpriteRenderer->Create();
	m_pSpriteRenderer->CreateVolatile();

	m_pSceneRenderer = new C_AftkSceneRenderer();
	m_pSceneRenderer->LoadDefaultShaders();
	m_pSceneRenderer->Create();
	m_pSceneRenderer->CreateVolatile();
	AftkRender::PushSceneRenderer(m_pSceneRenderer);

	m_pBasicRenderer = new Display::C_BasicRender();
	m_pBasicRenderer->Create();
	m_pBasicRenderer->CreateVolatile();

	m_pFontRenderer = new AftkRender::C_FontRenderer();
	m_pFontRenderer->Create();
	m_pFontRenderer->CreateVolatile();
	m_pFontRenderer->LoadFont("Arial", 8);
	m_pFontRenderer->LoadFont("Arial", 10);

	m_pCollisionRenderer = new C_CollisionRenderer();
	m_pCollisionRenderer->Create();
	m_pCollisionRenderer->CreateVolatile();

	//create stencil stipple texture
	{
		Display::C_RefTexture2D *pTexture = new Display::C_RefTexture2D();

		if (Display::Device()->CreateTexture2D(2,2,1,Display::E_Usage_Managed,Display::E_Format_A8R8G8B8,pTexture))
		{
			Display::C_LockedRect rect = pTexture->Lock(0);
			if (rect.pBits)
			{
				const U32 pattern[4] = {0xffffffff, 0x00ffffff, 0x00ffffff, 0xffffffff};
				memcpy(rect.pBits, pattern, sizeof(U32)*4);
			}
			pTexture->Unlock(0);
			m_pStippleTexture = Editor::I_EditorRenderer::GetTextureManager().AddTexture2D("stencil_stipple", pTexture);
		}
		else
		{
			delete pTexture;
		}
	}

	m_pSimpleRenderer->SetObjTypePrimitive(
		EditorLib::C_SceneXtActorNode::GetStaticTypeName(),
		Primitive::Locator(1.f, 1.f, 1.f), 
		Maths::RGBA(255,255,0,255));

	return true;
}

void C_RenderPipeline::DestroyRenderers()
{
	if (m_pSimpleRenderer)
	{
		m_pSimpleRenderer->Destroy();
		m_pSimpleRenderer->DestroyVolatile();
		SAFE_DELETE_PTR(m_pSimpleRenderer);
	}

	if (m_pComplexRenderer)
	{
		m_pComplexRenderer->Destroy();
		m_pComplexRenderer->DestroyVolatile();
		SAFE_DELETE_PTR(m_pComplexRenderer);
	}

	if (m_pVertexRenderer)
	{
		m_pVertexRenderer->Destroy();
		m_pVertexRenderer->DestroyVolatile();
		SAFE_DELETE_PTR(m_pVertexRenderer);
	}

	if (m_pSpriteRenderer)
	{
		m_pSpriteRenderer->DestroyVolatile();
		m_pSpriteRenderer->Destroy();
		SAFE_DELETE_PTR(m_pSpriteRenderer);
	}

	if (m_pSceneRenderer)
	{
		AftkRender::C_SceneRenderer* pRenderer = AftkRender::PopSceneRenderer();
		assert(pRenderer == m_pSceneRenderer);
		m_pSceneRenderer->Destroy();
		m_pSceneRenderer->DestroyVolatile();
		SAFE_DELETE_PTR(m_pSceneRenderer);
	}

	if (m_pBasicRenderer)
	{
		m_pBasicRenderer->Destroy();
		m_pBasicRenderer->DestroyVolatile();
		SAFE_DELETE_PTR(m_pBasicRenderer);
	}

	if (m_pFontRenderer)
	{
		m_pFontRenderer->Destroy();
		m_pFontRenderer->DestroyVolatile();
		SAFE_DELETE_PTR(m_pFontRenderer);
	}

	if (m_pCollisionRenderer)
	{
		m_pCollisionRenderer->Destroy();
		m_pCollisionRenderer->DestroyVolatile();
		SAFE_DELETE_PTR(m_pCollisionRenderer);
	}

	if (Display::DeviceExists())
	{
		Display::Device()->DestroyRenderers();
		Display::Device()->DestroyRenderersVolatile();

		SAFE_DELETE_PTR(m_pDebugRenderSys);
	}

	Registry::Write(kRegKeyPath, "Viewport_Version", RENDERPIPELINE_REGISTRY_VERSION);

	//save viewport render state (ASSUMPTION: viewports haven't been destroyed yet)
	ViewportRenderStateMap::Iterator it = m_viewportRenderStateMap.Begin();
	while (it != m_viewportRenderStateMap.End())
	{
		const C_RenderViewport *pViewport = it->GetKey();
		Btl::String key;

		const ViewportRenderState& state = it->GetValue();

		key.Printf("Viewport_%s_fillmode", pViewport->GetName());
		Registry::Write(kRegKeyPath, key.Chars(), state.fillmode);
		
		key.Printf("Viewport_%s_cullmode", pViewport->GetName());
		Registry::Write(kRegKeyPath, key.Chars(), state.cullmode);

		key.Printf("Viewport_%s_overlay", pViewport->GetName());
		Registry::Write(kRegKeyPath, key.Chars(), state.overlay);
		
		key.Printf("Viewport_%s_xray", pViewport->GetName());
		Registry::Write(kRegKeyPath, key.Chars(), state.xray);

		key.Printf("Viewport_%s_textured", pViewport->GetName());
		Registry::Write(kRegKeyPath, key.Chars(), state.textured);

		key.Printf("Viewport_%s_lit", pViewport->GetName());
		Registry::Write(kRegKeyPath, key.Chars(), state.lit);

		key.Printf("Viewport_%s_lineAA", pViewport->GetName());
		Registry::Write(kRegKeyPath, key.Chars(), state.lineAA);

		key.Printf("Viewport_%s_grid", pViewport->GetName());
		Registry::Write(kRegKeyPath, key.Chars(), state.grid);

		++it;
	}
}
void C_RenderPipeline::UpdateActorRenderers(AftkRender::E_RendererPass pass)
{
	m_pSimpleRenderer->Clear();
	m_pComplexRenderer->Clear();	
	m_pSpriteRenderer->Clear();

	// Attach all scene nodes to the relevant renderers
	C_RenderCore::Instance().GetRenderScene().AttachToRenderers(pass);

	m_pSimpleRenderer->Update(pass);
	m_pComplexRenderer->Update(pass);
	m_pSpriteRenderer->Update(pass);
}

bool C_RenderPipeline::Render(const C_RenderViewport* pViewport)
{
	//initialise stencil pattern
	{
		Display::BasicRender()->BindTextured2D(m_pStippleTexture);
		struct Vertex
		{
			F32 x, y;
			U32 c;
			F32 u, v;
		};

		Vertex *pVertex = (Vertex*) Display::Device()->BeginVertices(
			Display::E_PrimitiveType_TriangleFan,
			4,
			sizeof(Vertex),
			Display::BasicRender()->m_pVertexDeclaration[Display::E_BasicRenderVertexShader_Textured2D]);

		if (pVertex)
		{
			Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
			Display::Device()->SetBlendMode(Display::E_BlendMode_SolidAlphaTest);
			Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
			Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Wrap);

			Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_None);
			Display::Device()->SetStencilEnable(Display::E_Bool_True);
			Display::Device()->SetStencilFunc(Display::E_CmpFunc_Always);
			Display::Device()->SetStencilPass(Display::E_StencilOp_Replace);
			Display::Device()->SetStencilRef(1);

			wxSize s = pViewport->GetSize();
			pVertex[0].x = 0.f; pVertex[0].y = 0.f; pVertex[0].c = 0xffffffff; pVertex[0].u = 0.25f; pVertex[0].v = 0.25f;
			pVertex[1].x = s.x; pVertex[1].y = 0.f; pVertex[1].c = 0xffffffff; pVertex[1].u = (F32)s.x/2.f; pVertex[1].v = 0.25f;
			pVertex[2].x = s.x; pVertex[2].y = s.y; pVertex[2].c = 0xffffffff; pVertex[2].u = (F32)s.x/2.f; pVertex[2].v = (F32)s.y/2.f;
			pVertex[3].x = 0.f; pVertex[3].y = s.y; pVertex[3].c = 0xffffffff; pVertex[3].u = 0.25f; pVertex[3].v = (F32)s.y/2.f;

			pVertex += 4;
			Display::Device()->EndVertices(pVertex);

			Display::Device()->SetBlendMode(Display::E_BlendMode_Default);
			Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
			Display::Device()->SetStencilFail(Display::E_StencilOp_Keep);
			Display::Device()->SetStencilFunc(Display::E_CmpFunc_Equal);
			Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
			Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);

			Display::Device()->SetStencilEnable(Display::E_Bool_False);
		}
	}


	const ViewportRenderState* pState = &GetViewportRenderState(pViewport);
	{
		AftkRender::C_SceneRenderer::Flags sceneflags;
		sceneflags.fillmode = pState->fillmode;
		sceneflags.cullmode = pState->cullmode;
		sceneflags.xray = pState->xray;
		sceneflags.textured = pState->textured;
		sceneflags.lit = pState->lit;
		m_pSceneRenderer->SetFlags(sceneflags);
		m_pSceneRenderer->ShowWireframeOverlay(pState->overlay);

		C_SimpleRenderer::RenderFlags actorflags;
		actorflags.fillmode = pState->fillmode;
		actorflags.cullmode = pState->cullmode;
		actorflags.textured = pState->textured;
		actorflags.lit = pState->lit;
		m_pSimpleRenderer->SetRenderFlags(actorflags);

		Display::Device()->GetD3DDevice()->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, pState->lineAA ? TRUE : FALSE);
	}


	if (pState->grid)
	{
		if (pState->xray || pState->fillmode==Display::E_FillMode_Wireframe)
			Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);

		const AftkRender::C_Camera& camera = pViewport->GetViewContext().GetView().camera;
		if (camera.GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
		{
			AftkRender::C_GridRenderer::Render(Maths::HVec2(-m_globalRenderState.gridSize,-m_globalRenderState.gridSize), Maths::HVec2(m_globalRenderState.gridSize, m_globalRenderState.gridSize), 
				Maths::RGBA(140,140,140,255), m_globalRenderState.gridSpacing, 0.f, AftkRender::C_GridRenderer::E_GridPlane_XZ);
		}
		else
		{
			Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
			Maths::HVec3 viewDir = camera.GetFocusPosition()-camera.GetPosition();
			viewDir.Normalise();

			F32 extent = camera.GetHeightExtent()/2;
			F32 aspectRatio = camera.GetAspectRatio();

			if (Maths::Abs(viewDir[0]) > 0.98f)
			{
				//render grid on YZ plane
				Maths::HVec2 min, max;
				if (Maths::Abs(camera.GetUpVector()[1]) > Maths::Abs(camera.GetUpVector()[2]))
				{
					//y-up
					min = Maths::HVec2(-extent-m_globalRenderState.gridSpacing, -aspectRatio*extent-m_globalRenderState.gridSpacing);
					max = Maths::HVec2(extent+m_globalRenderState.gridSpacing, aspectRatio*extent+m_globalRenderState.gridSpacing);
				}
				else
				{
					//z-up
					min = Maths::HVec2(-aspectRatio*extent-m_globalRenderState.gridSpacing, -extent-m_globalRenderState.gridSpacing);
					max = Maths::HVec2(aspectRatio*extent+m_globalRenderState.gridSpacing, extent+m_globalRenderState.gridSpacing);
				}

				min[0] += camera.GetPosition()[1]; min[1] += camera.GetPosition()[2];
				max[0] += camera.GetPosition()[1]; max[1] += camera.GetPosition()[2];

				F32 offset = 0;
				if (viewDir[0] > 0)
					offset = camera.GetPosition()[0] + camera.GetNearClipDistance() + 1.f;
				else
					offset = camera.GetPosition()[0] - camera.GetNearClipDistance() - 1.f;

				AftkRender::C_GridRenderer::Render(min, max, 
					Maths::RGBA(140,140,140,255), m_globalRenderState.gridSpacing, offset, AftkRender::C_GridRenderer::E_GridPlane_YZ);
			}
			else if (Maths::Abs(viewDir[1]) > 0.98f)
			{
				//render grid on XZ plane
				Maths::HVec2 min, max;
				if (Maths::Abs(camera.GetUpVector()[0]) > Maths::Abs(camera.GetUpVector()[2]))
				{
					//x-up
					min = Maths::HVec2(-extent-m_globalRenderState.gridSpacing, -aspectRatio*extent-m_globalRenderState.gridSpacing);
					max = Maths::HVec2(extent+m_globalRenderState.gridSpacing, aspectRatio*extent+m_globalRenderState.gridSpacing);
				}
				else
				{
					//z-up
					min = Maths::HVec2(-aspectRatio*extent-m_globalRenderState.gridSpacing, -extent-m_globalRenderState.gridSpacing);
					max = Maths::HVec2(aspectRatio*extent+m_globalRenderState.gridSpacing, extent+m_globalRenderState.gridSpacing);
				}

				min[0] += camera.GetPosition()[0]; min[1] += camera.GetPosition()[2];
				max[0] += camera.GetPosition()[0]; max[1] += camera.GetPosition()[2];

				F32 offset = 0;
				if (viewDir[1] > 0)
					offset = camera.GetPosition()[1] + camera.GetNearClipDistance() + 1.f;
				else
					offset = camera.GetPosition()[1] - camera.GetNearClipDistance() - 1.f;

				AftkRender::C_GridRenderer::Render(min, max, 
					Maths::RGBA(140,140,140,255), m_globalRenderState.gridSpacing, offset, AftkRender::C_GridRenderer::E_GridPlane_XZ);

			}
			else if (Maths::Abs(viewDir[2]) > 0.98f)
			{
				//render grid on XY plane
				Maths::HVec2 min, max;
				if (Maths::Abs(camera.GetUpVector()[0]) > Maths::Abs(camera.GetUpVector()[1]))
				{
					//x-up
					min = Maths::HVec2(-extent-m_globalRenderState.gridSpacing, -aspectRatio*extent-m_globalRenderState.gridSpacing);
					max = Maths::HVec2(extent+m_globalRenderState.gridSpacing, aspectRatio*extent+m_globalRenderState.gridSpacing);
				}
				else
				{
					//y-up
					min = Maths::HVec2(-aspectRatio*extent-m_globalRenderState.gridSpacing, -extent-m_globalRenderState.gridSpacing);
					max = Maths::HVec2(aspectRatio*extent+m_globalRenderState.gridSpacing, extent+m_globalRenderState.gridSpacing);
				}

				min[0] += camera.GetPosition()[0]; min[1] += camera.GetPosition()[1];
				max[0] += camera.GetPosition()[0]; max[1] += camera.GetPosition()[1];

				F32 offset = 0;
				if (viewDir[2] > 0)
					offset = camera.GetPosition()[2] + camera.GetNearClipDistance() + 1.f;
				else
					offset = camera.GetPosition()[2] - camera.GetNearClipDistance() - 1.f;

				AftkRender::C_GridRenderer::Render(min, max, 
					Maths::RGBA(140,140,140,255), m_globalRenderState.gridSpacing, offset, AftkRender::C_GridRenderer::E_GridPlane_XY);
			}
		}
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	}

	UpdateActorRenderers(AftkRender::E_RendererPass_Normal);
	m_pSceneRenderer->Update(0);
	
	//Render actor primitives
	m_pSimpleRenderer->Render();
	m_pComplexRenderer->Render();
	m_pVertexRenderer->Render();

	//render the aftk reference scenes
	m_pSceneRenderer->Render();

	m_pSpriteRenderer->Render();
	m_pCollisionRenderer->Render();

	//render the active manipulator
	I_Manipulator *pManipulator = C_RenderCore::Instance().GetManipulatorManager().GetActiveManipulator();
	if (pManipulator)
		pManipulator->Render(pViewport);

#if 0 //defined (_DEBUG) && defined(USER_OTAO)
	const Maths::C_AxisBox& aabb = C_RenderCore::Instance().GetRenderScene().GetSelectionBounds();

	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	Display::Device()->SetBlendMode(Display::E_BlendMode_Default);
	Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);
	Display::BasicRender()->DrawColoured3DBox(
		aabb.m_Min[0],aabb.m_Min[1],aabb.m_Min[2],
		aabb.m_Max[0],aabb.m_Max[1],aabb.m_Max[2],
		Maths::RGBA(255, 255, 0, 255));
	Display::Device()->SetFillMode(Display::E_FillMode_Default);
	Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);
	Display::BasicRender()->DrawColoured3DBox(
		aabb.m_Min[0],aabb.m_Min[1],aabb.m_Min[2],
		aabb.m_Max[0],aabb.m_Max[1],aabb.m_Max[2],
		Maths::RGBA(255, 255, 0, 64));
	Display::Device()->SetBlendMode(Display::E_BlendMode_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
#endif

	UpdateActorRenderers(AftkRender::E_RendererPass_Alpha);

	m_pFontRenderer->Begin();
	UpdateActorRenderers(AftkRender::E_RendererPass_2D);

	if (pState && (pState->labelAftkRef | pState->labelDgSceneNodes | pState->labelDgPropNodes | pState->labelDgSelection))
	{
		const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

		EditorLib::C_Selection selection;
		EditorLib::C_Core::Instance().GetSelection(selection);

		//render labels
		if (!pState->labelDgSelection)
		{
			EditorLib::C_DgNodeConstIterator it = EditorLib::C_Core::Instance().GetDgManager().GetDgIterator();
			while (it)
			{
				if (pState->labelDgSelection && selection.Find(it->GetId()))
				{
					it++;
					continue;
				}

				if ((pState->labelAftkRef && it->IsA(EditorLib::E_ObjType_SceneAftkNode)) ||
					(pState->labelDgSceneNodes && it->IsA(EditorLib::E_ObjType_SceneXtNodeBase)) ||
					(pState->labelDgPropNodes && it->IsA(EditorLib::E_ObjType_DgXtPropNodeBase)))
				{
					const AftkRender::C_SceneGraphNode *pSgNode = C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(it);
					if (pSgNode)
					{
						const Maths::HVec3 tmpPos = viewSettings3D.TransformWorldToWindow((const Maths::HVec3&)pSgNode->GetWorldMatrix()[3]);
						if (tmpPos.Z() > 0.f)
							m_pFontRenderer->Draw("Arial", 10, it->GetName().Chars(), tmpPos[0], tmpPos[1], Maths::RGBA(255,255,255,255));
					}
				}
				it++;
			}
		}

		if (pState->labelDgSelection)
		{
			const EditorLib::C_Selection::ItemArray& items = selection.GetItems();
			EditorLib::C_Selection::ItemArray::ConstIterator it = items.Begin();
			while (it != items.End())
			{
				const EditorLib::I_DgNode *pDgNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(it->id);
				if (pDgNode)
				{
					const AftkRender::C_SceneGraphNode *pSgNode = C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pDgNode);
					if (pSgNode)
					{
						const Maths::HVec3 tmpPos = viewSettings3D.TransformWorldToWindow((const Maths::HVec3&)pSgNode->GetWorldMatrix()[3]);
						if (tmpPos.Z() > 0.f)
							m_pFontRenderer->Draw("Arial", 10, pDgNode->GetName().Chars(), tmpPos[0], tmpPos[1], Maths::RGBA(255,255,255,255));
					}
				}
				it++;
			}
		}
	}

	D3DPRESENT_PARAMETERS viewportParams = pViewport->GetViewContext().PresentParams();
	
	{
		//centre viewport name text in viewport
		RECT rect = m_pFontRenderer->GetRect("Arial", 10, pViewport->GetLabel().c_str(), 0, 0);
		S32 xOffset = ((S32)viewportParams.BackBufferWidth - rect.right)/2;
		S32 yOffset = ((S32)viewportParams.BackBufferHeight - rect.bottom - 5);

		m_pFontRenderer->Draw("Arial", 10, pViewport->GetLabel().c_str(), xOffset, yOffset, Maths::RGBA(0,70,25,255));
		
		//const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
		//const Maths::HVec3 tmpPos = viewSettings3D.TransformWorldToWindow(Maths::HVec3(0.f, 0.f, 0.f));
		//if (tmpPos.Z() > 0.f)
		//	m_pFontRenderer->Draw("arial", "origin", tmpPos[0], tmpPos[1], Maths::RGBA(0,0,128,255));	
	}

	//HUD
	{
		if(m_globalRenderState.hudSelectionCount)
		{
			EditorLib::C_Selection selection;
			EditorLib::C_Core::Instance().GetSelection(selection);
			const EditorLib::C_Selection::ItemArray& items = selection.GetItems();
			
			Btl::String messageName = "Selected Nodes:";
			Btl::String messageValue = Format("%i", items.Size()); 
			
			RECT rectMessageName = m_pFontRenderer->GetRect("Arial", 10, messageName.Chars(), 0, 0);
			RECT rectMessageValue = m_pFontRenderer->GetRect("Arial", 10, messageValue.Chars(), 0, 0);

			S32 xOffset = ((S32)viewportParams.BackBufferWidth - rectMessageName.right - 50);
			S32 yOffset = 5;
			
			m_pFontRenderer->Draw("Arial", 10, messageName.Chars(), xOffset, yOffset, Maths::RGBA(0,70,25,255));

			xOffset = ((S32)viewportParams.BackBufferWidth - rectMessageValue.right - 5);
			yOffset = 5;
			
			m_pFontRenderer->Draw("Arial", 10, messageValue.Chars(), xOffset, yOffset, Maths::RGBA(255,255,255,255));
		}
	}

	m_pFontRenderer->End();

	HUDElements::RenderOrientationAxes(pViewport, wxPoint(5, pViewport->GetSize().y-30), 30);

	//turn of line antialiasing
	Display::Device()->GetD3DDevice()->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);

	return true;
}

C_AftkSceneRenderer& C_RenderPipeline::GetAftkSceneRenderer()
{
	return *m_pSceneRenderer;
}

C_SimpleRenderer& C_RenderPipeline::GetSimpleRenderer()
{
	return *m_pSimpleRenderer;
}

C_SpriteRenderer& C_RenderPipeline::GetSpriteRenderer()
{
	return *m_pSpriteRenderer;
}

C_ComplexRenderer& C_RenderPipeline::GetComplexRenderer()
{
	return *m_pComplexRenderer;
}

C_VertexRenderer& C_RenderPipeline::GetVertexRenderer()
{
	return *m_pVertexRenderer;
}

AftkRender::C_FontRenderer& C_RenderPipeline::GetFontRenderer()
{
	return *m_pFontRenderer;
}

void C_RenderPipeline::LoadTextures()
{
	const Ut::Vector<C_AftkRefScene*>& aftkScenes = C_RenderCore::Instance().GetRenderScene().GetAftkRefScenes();
	for (S32 i=0; i<aftkScenes.Size(); ++i)
	{
		const EditorLib::C_AftkRef *pRef = 
			EditorLib::C_Core::Instance().GetAftkRefManager().GetRef(
				aftkScenes[i]->GetOriginalFilePath());

		if (pRef && !pRef->IsVisualisation())
			aftkScenes[i]->LoadTextures();
	}

	m_pSimpleRenderer->LoadTextures();
}

C_RenderPipeline::GlobalRenderState& C_RenderPipeline::GetGlobalRenderState()
{
	return m_globalRenderState;
}

const C_RenderPipeline::GlobalRenderState& C_RenderPipeline::GetGlobalRenderState() const
{
	return m_globalRenderState;
}


C_RenderPipeline::ViewportRenderState& C_RenderPipeline::GetViewportRenderState(const C_RenderViewport *pViewport)
{
	ViewportRenderStateMap::Iterator it = m_viewportRenderStateMap.Find(pViewport);
	if (it != m_viewportRenderStateMap.End())
		return it->GetValue();

	ViewportRenderState state;
	//load user prefs here...
	{
		if (Registry::Read(kRegKeyPath, "Viewport_Version", 0) == RENDERPIPELINE_REGISTRY_VERSION)
		{
			Btl::String key;

			key.Printf("Viewport_%s_fillmode", pViewport->GetName());
			state.fillmode = Registry::Read(kRegKeyPath, key.Chars(), Display::E_FillMode_Default);

			key.Printf("Viewport_%s_cullmode", pViewport->GetName());
			state.cullmode = Registry::Read(kRegKeyPath, key.Chars(), Display::E_CullMode_Default);
			
			key.Printf("Viewport_%s_overlay", pViewport->GetName());
			state.overlay = Registry::Read(kRegKeyPath, key.Chars(), false);

			key.Printf("Viewport_%s_xray", pViewport->GetName());
			state.xray = Registry::Read(kRegKeyPath, key.Chars(), false);

			key.Printf("Viewport_%s_textured", pViewport->GetName());
			state.textured = Registry::Read(kRegKeyPath, key.Chars(), false);

			key.Printf("Viewport_%s_lit", pViewport->GetName());
			state.lit = Registry::Read(kRegKeyPath, key.Chars(), true);

			key.Printf("Viewport_%s_lineAA", pViewport->GetName());
			state.lineAA = Registry::Read(kRegKeyPath, key.Chars(), false);

			key.Printf("Viewport_%s_grid", pViewport->GetName());
			state.grid = Registry::Read(kRegKeyPath, key.Chars(), true);
		}
	}

	ViewportRenderStateMap::InsertionReturnType r = 
		m_viewportRenderStateMap.Insert(ViewportRenderStateMap::DataType(pViewport, state));
	return r.first->GetValue();
}

const C_RenderPipeline::ViewportRenderState& C_RenderPipeline::GetViewportRenderState(const C_RenderViewport *pViewport) const
{
	return const_cast<C_RenderPipeline*>(this)->GetViewportRenderState(pViewport);
}

bool C_RenderPipeline::Select(const RECT& region, C_DgNodeSelection& selectionCandidates)
{
	//aftk ref scene selection...
	Ut::UtSet<AftkRender::C_SceneGraphNode*> meshSelection;

	//set node visibility according to selectability in preparation for occlusion tests
	C_RenderCore::Instance().GetRenderScene().HideUnselectableNodes();
	m_pSceneRenderer->Update(0, AftkRender::E_RendererPass_VisibilityTest);

	if (wxWindow::FindFocus() && wxWindow::FindFocus()->IsKindOf(CLASSINFO(C_RenderViewport)))
	{
		const ViewportRenderState& state = 
			GetViewportRenderState((const C_RenderViewport*)wxWindow::FindFocus());

		Display::Device()->SetFillMode((Display::E_FillMode)state.fillmode);
		Display::Device()->SetCullMode((Display::E_CullMode)state.cullmode);

		C_SimpleRenderer::RenderFlags actorflags;
		actorflags.fillmode = state.fillmode;
		actorflags.cullmode = state.cullmode;
		actorflags.textured = false;
		actorflags.lit = false;
		m_pSimpleRenderer->SetRenderFlags(actorflags);
	}

	if (C_SelectManipulator::GetSelectionMask().mesh)
	{
		const Ut::Vector<C_AftkRefScene*>& aftkRefScenes = C_RenderCore::Instance().GetRenderScene().GetAftkRefScenes();
		for (S32 i=0; i<aftkRefScenes.Size(); ++i)
			aftkRefScenes[i]->VisibilityTest(region, m_pSceneRenderer, meshSelection);
	}

	if (meshSelection.Size())
	{
		Ut::UtSet<AftkRender::C_SceneGraphNode*>::Iterator it = meshSelection.Begin();
		const Ut::Vector<C_AftkRefScene*>& aftkRefScenes = C_RenderCore::Instance().GetRenderScene().GetAftkRefScenes();

		while (it != meshSelection.End())
		{
			bool found = false;
			for (S32 i=0; i<aftkRefScenes.Size(); ++i)
			{
				EditorLib::I_DgNode* pDgNode = aftkRefScenes[i]->Find(it->GetValue());
				if (pDgNode)
				{
					//object selection for aftk ref nodes...
					selectionCandidates.Add(pDgNode, Ut::Vector<U32>());
					found = true;
					break;
				}
			}

			if (!found)
				LOGERROR(g_Log, "Mesh %s does not map to a DgNode", it->GetValue()->GetName().Chars());

			it++;
		}
	}

	//revert node visibility
	C_RenderCore::Instance().GetRenderScene().ShowUnselectableNodes();

	//actor scene selection...
	{
		UpdateActorRenderers(AftkRender::E_RendererPass_VisibilityTest);
		m_pSimpleRenderer->GetVisible(region, selectionCandidates);
		m_pComplexRenderer->GetVisible(region, selectionCandidates);
		m_pSpriteRenderer->GetVisible(region, selectionCandidates);
		C_RenderCore::Instance().GetRenderScene().GetActorScene().GetVisible(region, selectionCandidates);
	}

	#ifdef _DEBUG
	const C_DgNodeSelection::ItemArray& items = selectionCandidates.GetItems();
	C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin();
	while (it != items.End())
	{
		LOGDEBUG(g_Log, "Hit %s", it->pDgNode->GetName().Chars());
		it++;
	}
	#endif

	return true;
}

}
