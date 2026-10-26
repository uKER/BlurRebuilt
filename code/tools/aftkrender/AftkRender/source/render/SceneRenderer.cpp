#include "AftkRenderPch.h"

#include "common/TextureManager.h"
#include "common/TextureState.h"
#include "common/ShaderManager.h"
#include "common/Material.h"
#include "common/ShaderConstant.h"
#include "common/RenderContext.h"
#include "common/Renderer.h"
#include "common/Scene.h"
#include "common/SceneGraphNode.h"
#include "common/GeometryNode.h"

#include "mesh/MeshSceneBuilder.h"

#include "scene/AftkSceneBuilder.h"
#include "scene/MaterialBuilder.h"

#include "render/SceneRenderer.h"
#include "Utility.h"

#include <algorithm>

namespace AftkRender
{
	extern Debug::Logger& g_Log;

	Ut::Vector<C_SceneRenderer*> g_SceneRendererStack;

	C_SceneRenderer* GetSceneRenderer()
	{
		if (g_SceneRendererStack.Size())
			return g_SceneRendererStack.Back();
		else
			return 0;
	}

	void PushSceneRenderer(C_SceneRenderer *pRenderer)
	{
		g_SceneRendererStack.PushBack(pRenderer);
	}

	C_SceneRenderer* PopSceneRenderer()
	{
		assert(g_SceneRendererStack.Size() > 0);
		C_SceneRenderer *pRenderer = g_SceneRendererStack.Back();
		g_SceneRendererStack.PopBack();
		return pRenderer;
	}
}

using namespace AftkRender;

EXTERN_SHADER(vsBasic);
EXTERN_SHADER(vsBasicNormal);
EXTERN_SHADER(vsBasicNormalColour);
EXTERN_SHADER(vsBasicNormalUV);
EXTERN_SHADER(vsBasicNormalTangents);
EXTERN_SHADER(vsBasicNormalColourUV);
EXTERN_SHADER(vsBasicNormalColourTangents);
EXTERN_SHADER(vsBasicNormalUVTangents);
EXTERN_SHADER(vsBasicNormalColourUVTangents);

EXTERN_SHADER(vsBillboard);
EXTERN_SHADER(vsBillboardColour);
EXTERN_SHADER(vsBillboardTangents);
EXTERN_SHADER(vsBillboardColourTangents);

EXTERN_SHADER(vsSkinned);
EXTERN_SHADER(vsSkinnedColour);
EXTERN_SHADER(vsSkinnedUV);
EXTERN_SHADER(vsSkinnedTangents);
EXTERN_SHADER(vsSkinnedColourUV);
EXTERN_SHADER(vsSkinnedColourTangents);
EXTERN_SHADER(vsSkinnedUVTangents);
EXTERN_SHADER(vsSkinnedColourUVTangents);

EXTERN_SHADER(psBasic);
EXTERN_SHADER(psBasicSHL);
EXTERN_SHADER(psBasicColour);
EXTERN_SHADER(psBasicVertexColour);
EXTERN_SHADER(psBasicVertexColourLighting);
EXTERN_SHADER(psBasicTexture);
EXTERN_SHADER(psBasicTextureColour);
EXTERN_SHADER(psBasicTextureVertexColour);
EXTERN_SHADER(psBasicTextureVertexColourLighting);
EXTERN_SHADER(psBasicNdotL);
EXTERN_SHADER(psBasicNdotLColour);
EXTERN_SHADER(psBasicNdotLVertexColour);
EXTERN_SHADER(psBasicNdotLVertexColourLighting);
EXTERN_SHADER(psBasicNdotLTexture);
EXTERN_SHADER(psBasicNdotLTextureColour);
EXTERN_SHADER(psBasicNdotLTextureVertexColour);
EXTERN_SHADER(psBasicNdotLTextureVertexColourLighting);
EXTERN_SHADER(psBasicXray);
EXTERN_SHADER(psBasicXrayColour);
EXTERN_SHADER(psBasicXrayVertexColour);
EXTERN_SHADER(psBasicXrayVertexColourLighting);
EXTERN_SHADER(psBasicXrayTexture);
EXTERN_SHADER(psBasicXrayTextureColour);
EXTERN_SHADER(psBasicXrayTextureVertexColour);
EXTERN_SHADER(psBasicXrayTextureVertexColourLighting);
EXTERN_SHADER(psBasicXrayNdotL);
EXTERN_SHADER(psBasicXrayNdotLColour);
EXTERN_SHADER(psBasicXrayNdotLVertexColour);
EXTERN_SHADER(psBasicXrayNdotLVertexColourLighting);
EXTERN_SHADER(psBasicXrayNdotLTexture);
EXTERN_SHADER(psBasicXrayNdotLTextureColour);
EXTERN_SHADER(psBasicXrayNdotLTextureVertexColour);
EXTERN_SHADER(psBasicXrayNdotLTextureVertexColourLighting);

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
					case Display::E_VertexElementUsage_Colour:
						index.colour = true; break;
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

C_SceneRenderer::C_SceneRenderer()
: C_Renderer()
{
}

C_SceneRenderer::~C_SceneRenderer()
{
	Clear();
}

void C_SceneRenderer::Clear()
{
	m_SceneGraphNodeArray.Clear();
}

void C_SceneRenderer::Update(const Maths::C_TimeS& timeStep, E_RendererPass pass)
{
	m_RenderContextArray.Clear();

	Ut::UtVector<Ut::DynamicVector<C_SceneGraphNode*> >::Iterator it = m_SceneGraphNodeArray.Begin();
	while (it != m_SceneGraphNodeArray.End())
	{
		(*it)->AttachToRenderer(0, pass);
		it++;
	}
	SortNodes();
}

void C_SceneRenderer::Render()
{
	//Display::Device()->SetAlphaTestEnable(Display::E_Bool_True);
	//Display::Device()->SetAlphaTestFunc(Display::E_CmpFunc_Greater);
	//Display::Device()->SetAlphaTestRef(0x80);
	//Display::Device()->SetBlendEnable(Display::E_Bool_False);

	////AA alpha testing
	//if (Display::Device()->m_pd3dObject->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, (D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A')) == S_OK)
	//	Display::Device()->m_pd3dDevice->SetRenderState(D3DRS_ADAPTIVETESS_Y, (D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A'));

	Display::Device()->SetFillMode((Display::E_FillMode)m_Flags.fillmode);
	Display::Device()->SetCullMode((Display::E_CullMode) m_Flags.cullmode);

	for (S32 i=0; i<m_RenderContextArray.Size(); ++i)
	{
		const C_SceneGraphNode *pNode = m_RenderContextArray[i].first;

		C_RenderContext& rc = m_RenderContextArray[i].second;
		const C_GeometryCluster *pCluster = rc.GetGeometryCluster();
		U32 batch = rc.GetBatch();

		// view frustrum culling
		const Display::C_ViewFrustum& frustum = Display::Device()->GetViewFrustum();
		S32 cullResult = frustum.CullAxisBox(pNode->GetWorldAABBox());
		if (cullResult > 0)
		{
			S16 materialIndex = rc.GetRemappedMaterialIndex();
			if (materialIndex != -1)
			{
				const C_Scene *pScene = pNode->GetScene();
				if (pScene)
				{
					S32 materialIndexMapping = pScene->GetMaterialIndexMapping(materialIndex);
					const C_Material *pMaterial = pScene->GetMaterial(materialIndexMapping);
					assert(pMaterial && "Invalid material index");

					if (m_Flags.textured)
						pMaterial->Enable();

					if (pMaterial->GetCullMode() == Display::E_CullMode_Default &&
						m_Flags.cullmode != Display::E_CullMode_Default)
						Display::Device()->SetCullMode((Display::E_CullMode) m_Flags.cullmode);

					if (!pMaterial->TexturesLoaded() || !pMaterial->GetNumTextures() || !m_Flags.textured)
						Display::Device()->SetTexture(0, m_TextureManager.GetDefault2DTexture());

					//enable alpha blending if not rendering solids (in case line AA is set)
					if (m_Flags.fillmode != Display::E_FillMode_Solid && pMaterial->GetBlendMode() == Display::E_BlendMode_Default)
						Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);
				}
			}
			else
				Display::Device()->SetTexture(0, m_TextureManager.GetDefault2DTexture());

			if (m_Flags.xray)
			{
				//potentially undoes the state set by the material...
				Display::Device()->SetAlphaTestEnable(Display::E_Bool_True);
				Display::Device()->SetAlphaTestFunc(Display::E_CmpFunc_Greater);
				Display::Device()->SetAlphaTestRef(0x00);
				Display::Device()->SetBlendEnable(Display::E_Bool_True);
				Display::Device()->SetBlendSrc(Display::E_Blend_SrcAlpha);
				Display::Device()->SetBlendDst(Display::E_Blend_InvSrcAlpha);
				Display::Device()->SetBlendOp(Display::E_BlendOp_Add);
			}

			rc.Apply();
			pCluster->RenderGeometry(batch);
		}
	}

	Display::Device()->SetFillMode(Display::E_FillMode_Default);
	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetBlendMode(Display::E_BlendMode_Default);
}


bool C_SceneRenderer::IsVisible(const RECT &region, const C_SceneGraphNode* pNode)
{
	// view frustrum culling
	const Display::C_ViewFrustum& frustum = Display::Device()->GetViewFrustum();
	S32 cullResult = frustum.CullAxisBox(pNode->GetWorldAABBox());
	if (cullResult > 0)
	{
		for (S32 i=0; i<m_RenderContextArray.Size(); ++i)
		{
			if (m_RenderContextArray[i].first == pNode)
			{
				C_RenderContext& rc = m_RenderContextArray[i].second;
				BeginVisibilityTest(region);
				rc.Apply();
				rc.GetGeometryCluster()->RenderGeometry(rc.GetBatch());
				if (EndVisibilityTest())
					return true;
			}
		}
	}
	return false;
}


bool C_SceneRenderer::LoadDefaultShaders()
{
	bool success = true;

	if (m_ShaderManager.LoadVertexShader("shaders/vsBasic.shader", REF_SHADER(vsBasic)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsBasicNormal.shader", REF_SHADER(vsBasicNormal)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsBasicNormalColour.shader", REF_SHADER(vsBasicNormalColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsBasicNormalUV.shader", REF_SHADER(vsBasicNormalUV)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsBasicNormalTangents.shader", REF_SHADER(vsBasicNormalTangents)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsBasicNormalColourUV.shader", REF_SHADER(vsBasicNormalColourUV)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsBasicNormalColourTangents.shader", REF_SHADER(vsBasicNormalColourTangents)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsBasicNormalUVTangents.shader", REF_SHADER(vsBasicNormalUVTangents)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsBasicNormalColourUVTangents.shader", REF_SHADER(vsBasicNormalColourUVTangents)) == 0)
		success = success && false;

	if (m_ShaderManager.LoadVertexShader("shaders/vsBillboard.shader", REF_SHADER(vsBillboard)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsBillboardColour.shader", REF_SHADER(vsBillboardColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsBillboardTangents.shader", REF_SHADER(vsBillboardTangents)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsBillboardColourTangents.shader", REF_SHADER(vsBillboardColourTangents)) == 0)
		success = success && false;

	if (m_ShaderManager.LoadVertexShader("shaders/vsSkinned.shader", REF_SHADER(vsSkinned)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsSkinnedColour.shader", REF_SHADER(vsSkinnedColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsSkinnedUV.shader", REF_SHADER(vsSkinnedUV)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsSkinnedTangents.shader", REF_SHADER(vsSkinnedTangents)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsSkinnedColourUV.shader", REF_SHADER(vsSkinnedColourUV)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsSkinnedColourTangents.shader", REF_SHADER(vsSkinnedColourTangents)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsSkinnedUVTangents.shader", REF_SHADER(vsSkinnedUVTangents)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadVertexShader("shaders/vsSkinnedColourUVTangents.shader", REF_SHADER(vsSkinnedColourUVTangents)) == 0)
		success = success && false;

	if (m_ShaderManager.LoadPixelShader("shaders/psBasic.shader", REF_SHADER(psBasic)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicSHL.shader", REF_SHADER(psBasicSHL)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicColour.shader", REF_SHADER(psBasicColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicVertexColour.shader", REF_SHADER(psBasicVertexColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicVertexColourLighting.shader", REF_SHADER(psBasicVertexColourLighting)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicTexture.shader", REF_SHADER(psBasicTexture)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicTextureColour.shader", REF_SHADER(psBasicTextureColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicTextureVertexColour.shader", REF_SHADER(psBasicTextureVertexColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicTextureVertexColourLighting.shader", REF_SHADER(psBasicTextureVertexColourLighting)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicNdotL.shader", REF_SHADER(psBasicNdotL)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicNdotLColour.shader", REF_SHADER(psBasicNdotLColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicNdotLVertexColour.shader", REF_SHADER(psBasicNdotLVertexColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicNdotLVertexColourLighting.shader", REF_SHADER(psBasicNdotLVertexColourLighting)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicNdotLTexture.shader", REF_SHADER(psBasicNdotLTexture)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicNdotLTextureColour.shader", REF_SHADER(psBasicNdotLTextureColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicNdotLTextureVertexColour.shader", REF_SHADER(psBasicNdotLTextureVertexColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicNdotLTextureVertexColourLighting.shader", REF_SHADER(psBasicNdotLTextureVertexColourLighting)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXray.shader", REF_SHADER(psBasicXray)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayColour.shader", REF_SHADER(psBasicXrayColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayVertexColour.shader", REF_SHADER(psBasicXrayVertexColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayVertexColourLighting.shader", REF_SHADER(psBasicXrayVertexColourLighting)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayTexture.shader", REF_SHADER(psBasicXrayTexture)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayTextureColour.shader", REF_SHADER(psBasicXrayTextureColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayTextureVertexColour.shader", REF_SHADER(psBasicXrayTextureVertexColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayTextureVertexColourLighting.shader", REF_SHADER(psBasicXrayTextureVertexColourLighting)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayNdotL.shader", REF_SHADER(psBasicXrayNdotL)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayNdotLColour.shader", REF_SHADER(psBasicXrayNdotLColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayNdotLVertexColour.shader", REF_SHADER(psBasicXrayNdotLVertexColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayNdotLVertexColourLighting.shader", REF_SHADER(psBasicXrayNdotLVertexColourLighting)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayNdotLTexture.shader", REF_SHADER(psBasicXrayNdotLTexture)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayNdotLTextureColour.shader", REF_SHADER(psBasicXrayNdotLTextureColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayNdotLTextureVertexColour.shader", REF_SHADER(psBasicXrayNdotLTextureVertexColour)) == 0)
		success = success && false;
	if (m_ShaderManager.LoadPixelShader("shaders/psBasicXrayNdotLTextureVertexColourLighting.shader", REF_SHADER(psBasicXrayNdotLTextureVertexColourLighting)) == 0)
		success = success && false;

	return success;
}

void C_SceneRenderer::AttachSceneGraphNode(C_SceneGraphNode* pSgNode)
{
	Ut::UtVector<Ut::DynamicVector<C_SceneGraphNode*> >::Iterator it = m_SceneGraphNodeArray.Begin();
	while (it != m_SceneGraphNodeArray.End())
	{
		if (*it == pSgNode)
			return;
		it++;
	}
	m_SceneGraphNodeArray.PushBack(pSgNode);
}

void C_SceneRenderer::DetachSceneGraphNode(const C_SceneGraphNode *pSgNode)
{
	Ut::UtVector<Ut::DynamicVector<C_SceneGraphNode*> >::Iterator it = m_SceneGraphNodeArray.Begin();
	while (it != m_SceneGraphNodeArray.End())
	{
		if (*it == pSgNode)
		{
			m_SceneGraphNodeArray.Erase(it);
			return;
		}
		it++;
	}
}

void C_SceneRenderer::AddRenderContext(C_RenderContext& rc, const C_SceneGraphNode* pNode)
{
	const C_GeometryCluster *pCluster = rc.GetGeometryCluster();
	assert (pCluster);
	
	//set shader for cluster
	{
		VSLookupIndex vsIndex(pCluster->GetVertexDeclaration());
		if (rc.GetFlags().isBillboard)
			rc.SetVertexShader(m_ShaderManager.GetVertexShader(VertexShaderBillboardLookup[vsIndex.index.value]));
		else if (rc.GetFlags().isSkinned)
			rc.SetVertexShader(m_ShaderManager.GetVertexShader(VertexShaderSkinnedLookup[vsIndex.index.value]));
		else
			rc.SetVertexShader(m_ShaderManager.GetVertexShader(VertexShaderBasicLookup[vsIndex.index.value]));

		if (m_Flags.xray)
		{
			if (m_Flags.lit)
				rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicXrayNdotL.shader"));
			else
				rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicXray.shader"));
		}
		else if (m_Flags.lit)
			rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicNdotL.shader"));
		else
			rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicTextured.shader"));
	}

	m_RenderContextArray.PushBack(Ut::Pair<const C_SceneGraphNode*, C_RenderContext>(pNode, rc));
}

void C_SceneRenderer::SortNodes()
{
	Maths::HVec3 cameraPosition = Display::Device()->GetView()->m_ViewSettings3D.GetCameraPosition();
	if (m_RenderContextArray.Size() > 1)
		std::sort(&m_RenderContextArray[0], &m_RenderContextArray[m_RenderContextArray.Size()-1], Compare);	
}

bool C_SceneRenderer::Compare(
	const Ut::Pair<const C_SceneGraphNode*, C_RenderContext>& l,
	const Ut::Pair<const C_SceneGraphNode*, C_RenderContext>& r)
{
	const C_SceneGraphNode *pLeft = l.first;
	const C_SceneGraphNode *pRight = r.first;

	//camera distance
	Maths::HVec3 camPos = Display::Device()->GetView()->m_ViewSettings3D.GetCameraPosition();
	const Maths::HVec4 lPos = pLeft->GetWorldMatrix().AW();
	const Maths::HVec4 rPos = pRight->GetWorldMatrix().AW();

	Maths::HVec3 vecL(camPos.X()-lPos.X(), camPos.Y()-lPos.Y(), camPos.Z()-lPos.Z());
	Maths::HVec3 vecR(camPos.X()-rPos.X(), camPos.Y()-rPos.Y(), camPos.Z()-rPos.Z());

	F32 distL = vecL.LengthSquared();
	F32 distR = vecR.LengthSquared();

	return (distL < distR);
}

const C_SceneRenderer::Flags& C_SceneRenderer::GetFlags() const
{
	return m_Flags;
}

void C_SceneRenderer::SetFlags(const C_SceneRenderer::Flags& flags)
{
	m_Flags = flags;
}
