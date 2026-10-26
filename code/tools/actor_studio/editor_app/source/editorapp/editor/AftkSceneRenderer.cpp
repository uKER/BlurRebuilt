#include "stdafx.h"

#include <Display/Shaders/Constants.h>

#include <EditorLib/Dg/IDgNode.h>

#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/AftkSceneRenderer.h>
#include <EditorApp/Editor/DgNodeSelection.h>

using namespace Editor;

static const F32 Solid[4]		= {0.5f, 0.5f, 0.5f, 1.f};
static const F32 Wireframe[4]	= {0.f, 0.f, 0.375f, 1.f};
static const F32 Highlight[4]	= {0.f, 1.f, 0.5f, 1.f};

struct VSLookupIndex
{
	union
	{
		struct
		{
			U8 colour	: 1;
			U8 uv		: 1;
			U8 tangents	: 1;
		};
		U8 value;
	} index;

	VSLookupIndex()
	{
		index.value = 0;
	}

	VSLookupIndex(const Display::C_VertexDeclaration* pDecl)
	{
		index.value = 0;
		const S32 elementCount = pDecl->GetElementCount();
		if (elementCount > 0)
		{
			Display::C_VertexElement* pElements = new Display::C_VertexElement[elementCount+1];
			if (pDecl->GetElements(elementCount, pElements))
			{
				for (S32 i=0; i<elementCount; ++i)
				{
					switch (pElements[i].GetUsage())
					{
					//case Display::E_VertexElementUsage_Colour:
					//	index.colour = true; break;
					case Display::E_VertexElementUsage_TexCoord:
						index.uv = true; break;
					case Display::E_VertexElementUsage_Tangent:
					case Display::E_VertexElementUsage_Binormal:
						index.tangents = true;
					};
				}
			}
			SAFE_DELETE_ARY(pElements);
		}
	}
};

static const char* const VertexShaderBasicLookup[8] = 
{
	"shaders/vsBasicNormal.shader",
	"shaders/vsBasicNormalColour.shader",
	"shaders/vsBasicNormalUV.shader",
	"shaders/vsBasicNormalColourUV.shader",
	"shaders/vsBasicNormalTangents.shader",
	"shaders/vsBasicNormalColourTangents.shader",
	"shaders/vsBasicNormalUVTangents.shader",
	"shaders/vsBasicNormalColourUVTangents.shader"
};

static const char* const VertexShaderBillboardLookup[8] = 
{
	"shaders/vsBillboard.shader",
	"shaders/vsBillboardColour.shader",
	"shaders/vsBillboard.shader",
	"shaders/vsBillboardColour.shader",
	"shaders/vsBillboardTangents.shader",
	"shaders/vsBillboardColourTangents.shader",
	"shaders/vsBillboardTangents.shader",
	"shaders/vsBillboardColourTangents.shader"
};

static const char* const VertexShaderSkinnedLookup[8] = 
{
	"shaders/vsSkinned.shader",
	"shaders/vsSkinnedColour.shader",
	"shaders/vsSkinnedUV.shader",
	"shaders/vsSkinnedColourUV.shader",
	"shaders/vsSkinnedTangents.shader",
	"shaders/vsSkinnedColourTangents.shader",
	"shaders/vsSkinnedUVTangents.shader",
	"shaders/vsSkinnedColourUVTangents.shader"
};

static const char* const PixelShaderLookup[8] = 
{
	"shaders/psBasicColour.shader",
	"shaders/psBasicXrayColour.shader",
	"shaders/psBasicTexture.shader",
	"shaders/psBasicXrayTexture.shader",
	"shaders/psBasicNdotLColour.shader",
	"shaders/psBasicXrayNdotLColour.shader",
	"shaders/psBasicNdotLTexture.shader",
	"shaders/psBasicXrayNdotLTexture.shader"
};

C_AftkSceneRenderer::C_AftkSceneRenderer()
: AftkRender::C_SceneRenderer()
, m_showWireframeOverlay(false)
{
}

C_AftkSceneRenderer::~C_AftkSceneRenderer()
{
}

void C_AftkSceneRenderer::Update(const Maths::C_TimeS& timeStep, AftkRender::E_RendererPass pass)
{
	m_OverlayRenderContextArray.Clear();
	AftkRender::C_SceneRenderer::Update(timeStep, pass);
}

void C_AftkSceneRenderer::AddRenderContext(AftkRender::C_RenderContext& rc, const AftkRender::C_SceneGraphNode* pNode)
{
#if 1 //ifdef NONTRIANGULATED_MESH
	const AftkRender::C_GeometryCluster *pCluster = rc.GetGeometryCluster();
	assert (pCluster);

	{
		VSLookupIndex vsIndex(pCluster->GetVertexDeclaration());

		if (rc.GetFlags().isBillboard)
			rc.SetVertexShader(m_ShaderManager.GetVertexShader(VertexShaderBillboardLookup[vsIndex.index.value]));
		else if (rc.GetFlags().isSkinned)
			rc.SetVertexShader(m_ShaderManager.GetVertexShader(VertexShaderSkinnedLookup[vsIndex.index.value]));
		else
			rc.SetVertexShader(m_ShaderManager.GetVertexShader(VertexShaderBasicLookup[vsIndex.index.value]));
	}

	bool isSolid = !m_Flags.fillmode || 
		(Display::E_FillMode) m_Flags.fillmode == Display::E_FillMode_Default || 
		(Display::E_FillMode) m_Flags.fillmode == Display::E_FillMode_Solid;

	const AftkRender::C_PrimitiveBatch& batch = pCluster->GetPrimitiveBatch(rc.GetBatch());

	if (isSolid && batch.primitiveType != Display::E_PrimitiveType_LineList)
	{
		U32 lookupIndex = m_Flags.value >> 6;
		assert(lookupIndex < 8);

		rc.SetPixelShader(m_ShaderManager.GetPixelShader(PixelShaderLookup[lookupIndex]));
		rc.AddShaderConstant(AftkRender::C_ShaderConstant(AftkRender::C_ShaderConstant::E_ShaderType_Pixel, CP_DISPLAY_COLOUR, Solid, 1));

		m_RenderContextArray.PushBack(Ut::Pair<const AftkRender::C_SceneGraphNode*, AftkRender::C_RenderContext>(pNode, rc));
	}
	else if (!isSolid && batch.primitiveType == Display::E_PrimitiveType_LineList)
	{
		rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicColour.shader"));
		if (m_Selection.Find(pNode) == m_Selection.End())
			rc.AddShaderConstant(AftkRender::C_ShaderConstant(AftkRender::C_ShaderConstant::E_ShaderType_Pixel, CP_DISPLAY_COLOUR, Wireframe, 1));
		else
			rc.AddShaderConstant(AftkRender::C_ShaderConstant(AftkRender::C_ShaderConstant::E_ShaderType_Pixel, CP_DISPLAY_COLOUR, Highlight, 1));

		m_RenderContextArray.PushBack(Ut::Pair<const AftkRender::C_SceneGraphNode*, AftkRender::C_RenderContext>(pNode, rc));
	}

	if (isSolid && batch.primitiveType == Display::E_PrimitiveType_LineList)
	{
		//as we're rendering solids, we need to add a wireframe overlay as this node is selected;
		if (m_Selection.Find(pNode) != m_Selection.End())
		{
			rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicColour.shader"));
			rc.AddShaderConstant(AftkRender::C_ShaderConstant(AftkRender::C_ShaderConstant::E_ShaderType_Pixel, CP_DISPLAY_COLOUR, Highlight, 1));
			m_OverlayRenderContextArray.PushBack(Ut::Pair<const AftkRender::C_SceneGraphNode*, AftkRender::C_RenderContext>(pNode, rc));
		}
		else if (m_showWireframeOverlay)
		{
			rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicColour.shader"));
			rc.AddShaderConstant(AftkRender::C_ShaderConstant(AftkRender::C_ShaderConstant::E_ShaderType_Pixel, CP_DISPLAY_COLOUR, Wireframe, 1));
			m_OverlayRenderContextArray.PushBack(Ut::Pair<const AftkRender::C_SceneGraphNode*, AftkRender::C_RenderContext>(pNode, rc));
		}
	}

#else

	const AftkRender::C_GeometryCluster *pCluster = rc.GetGeometryCluster();
	assert (pCluster);

	{
		VSLookupIndex vsIndex(pCluster->GetVertexDeclaration());

		if (rc.GetFlags().isBillboard)
			rc.SetVertexShader(m_ShaderManager.GetVertexShader(VertexShaderBillboardLookup[vsIndex.index.value]));
		else if (rc.GetFlags().isSkinned)
			rc.SetVertexShader(m_ShaderManager.GetVertexShader(VertexShaderSkinnedLookup[vsIndex.index.value]));
		else
			rc.SetVertexShader(m_ShaderManager.GetVertexShader(VertexShaderBasicLookup[vsIndex.index.value]));
	}

	bool isSolid = !m_Flags.fillmode || 
		(Display::E_FillMode) m_Flags.fillmode == Display::E_FillMode_Default || 
		(Display::E_FillMode) m_Flags.fillmode == Display::E_FillMode_Solid;

	if (isSolid)
	{
		U32 lookupIndex = m_Flags.value >> 6;
		assert(lookupIndex < 8);

		rc.SetPixelShader(m_ShaderManager.GetPixelShader(PixelShaderLookup[lookupIndex]));
		rc.AddShaderConstant(AftkRender::C_ShaderConstant(AftkRender::C_ShaderConstant::E_ShaderType_Pixel, CP_DISPLAY_COLOUR, Solid, 1));
	}
	else
	{
		rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicColour.shader"));
		if (m_Selection.Find(pNode) == m_Selection.End())
			rc.AddShaderConstant(AftkRender::C_ShaderConstant(AftkRender::C_ShaderConstant::E_ShaderType_Pixel, CP_DISPLAY_COLOUR, Wireframe, 1));
		else
			rc.AddShaderConstant(AftkRender::C_ShaderConstant(AftkRender::C_ShaderConstant::E_ShaderType_Pixel, CP_DISPLAY_COLOUR, Highlight, 1));
	}

	m_RenderContextArray.PushBack(Ut::Pair<const AftkRender::C_SceneGraphNode*, AftkRender::C_RenderContext>(pNode, rc));

	if (isSolid)
	{
		//as we're rendering solids, we need to add a wireframe overlay as this node is selected;
		if (m_Selection.Find(pNode) != m_Selection.End())
		{
			rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicColour.shader"));
			rc.AddShaderConstant(AftkRender::C_ShaderConstant(AftkRender::C_ShaderConstant::E_ShaderType_Pixel, CP_DISPLAY_COLOUR, Highlight, 1));
			m_OverlayRenderContextArray.PushBack(Ut::Pair<const AftkRender::C_SceneGraphNode*, AftkRender::C_RenderContext>(pNode, rc));
		}
		else if (m_showWireframeOverlay)
		{
			rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicColour.shader"));
			rc.AddShaderConstant(AftkRender::C_ShaderConstant(AftkRender::C_ShaderConstant::E_ShaderType_Pixel, CP_DISPLAY_COLOUR, Wireframe, 1));
			m_OverlayRenderContextArray.PushBack(Ut::Pair<const AftkRender::C_SceneGraphNode*, AftkRender::C_RenderContext>(pNode, rc));
		}
	}
#endif
}

void C_AftkSceneRenderer::Render()
{

	if (m_Flags.xray)
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);

	if (m_Flags.fillmode == Display::E_FillMode_Wireframe)
	{
#if 1 //ifdef NONTRIANGULATED_MESH
		if (Display::Device()->GetDepthFlip())
			Display::Device()->SetDepthFunc(Display::E_CmpFunc_Greater);
		else
			Display::Device()->SetDepthFunc(Display::E_CmpFunc_Less);
#else
		//artificially push objects back so that line/point primitives (where z-biasing is not possible) are more prominent.
		if (Display::Device()->GetDepthFlip())
			Display::Device()->SetDepthBias(-0.000002f);
		else
			Display::Device()->SetDepthBias(0.000002f);
#endif
	}
	AftkRender::C_SceneRenderer::Render();

	if (m_OverlayRenderContextArray.Size())
	{
#if 1 //ifdef NONTRIANGULATED_MESH
		if (Display::Device()->GetDepthFlip())
			Display::Device()->SetDepthFunc(Display::E_CmpFunc_Greater);
		else
			Display::Device()->SetDepthFunc(Display::E_CmpFunc_Less);
#else
		{
			F32 bias = (m_Flags.fillmode == Display::E_FillMode_Wireframe) ? 0.000001f : -0.000001f;
			if (Display::Device()->GetDepthFlip())
				Display::Device()->SetDepthBias(-bias);
			else
				Display::Device()->SetDepthBias(bias);
		}
#endif
		Display::Device()->SetCullMode((Display::E_CullMode) m_Flags.cullmode);

		//Display::Device()->SetSlopeScaledBias(-1.f);
		Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
		Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);

		Display::Device()->SetPixelShaderConstantF(CP_DISPLAY_COLOUR, Highlight, 1);

		for (S32 i=0; i<m_OverlayRenderContextArray.Size(); ++i)
		{
			const AftkRender::C_SceneGraphNode *pNode = m_OverlayRenderContextArray[i].first;

			AftkRender::C_RenderContext& rc = m_OverlayRenderContextArray[i].second;
			const AftkRender::C_GeometryCluster *pCluster = rc.GetGeometryCluster();
			U32 batch = rc.GetBatch();

			// view frustrum culling
			const Display::C_ViewFrustum& frustum = Display::Device()->GetViewFrustum();
			S32 cullResult = frustum.CullAxisBox(pNode->GetWorldAABBox());
			if (cullResult > 0)
			{
				rc.Apply();
				pCluster->RenderGeometry(batch);
			}
		}


		Display::Device()->SetFillMode(Display::E_FillMode_Default);
		Display::Device()->SetBlendMode(Display::E_BlendMode_Default);
		Display::Device()->SetDepthBias(0.f);
		Display::Device()->SetSlopeScaledBias(0.f);
		Display::Device()->SetDepthFunc(Display::E_CmpFunc_Default);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_Default);
		m_OverlayRenderContextArray.Clear();
	}
}

void C_AftkSceneRenderer::OnSelectionChanged()
{
	m_Selection.Clear();
	m_OverlayRenderContextArray.Clear();

	const C_DgNodeSelection& selection = C_RenderCore::Instance().GetRenderScene().GetSelection();
	const C_DgNodeSelection::ItemArray& items = selection.GetItems();
	C_DgNodeSelection::ItemArray::ConstIterator itemIt = items.Begin();

	while (itemIt != items.End())
	{
		if (itemIt->pDgNode->IsA(EditorLib::E_ObjType_SceneAftkNode))
		{
			AftkRender::C_SceneGraphNode *pSgNode = C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(itemIt->pDgNode);
			if (pSgNode)
				m_Selection.Insert(pSgNode);
		}
		++itemIt;
	}
}

void C_AftkSceneRenderer::ShowWireframeOverlay(bool state)
{
	m_showWireframeOverlay = state;
}
