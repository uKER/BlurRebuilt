#include "ImposterPch.h"

#include "Imposter/ImposterRenderer.h"
#include "Imposter/ImposterImageHandle.h"
#include "Imposter/ImposterData.h"
#include "Imposter/ImposterUtils.h"

namespace Imposter
{

extern Debug::Logger& g_Log;

AftkRender::C_Scene*		Renderer::m_pScene = NULL;
AftkRender::C_Application*	Renderer::m_pRenderApplication = NULL;


bool Renderer::DeviceInit(HWND hwnd)
{
	U32 x = 256;
	x = UnsignedRoundUpPow2(x);

	if (!m_pRenderApplication)
	{
		m_pRenderApplication = new AftkRender::C_Application();

		//create a 1x1 render surface.
		//we don't actually care about this as we'll be rendering into a separate swap chain anyway.
		m_pRenderApplication->m_DeviceParams.m_Width		= 1;
		m_pRenderApplication->m_DeviceParams.m_Height		= 1;
		m_pRenderApplication->m_DeviceParams.m_pName		= "tools.Imposter";
		m_pRenderApplication->m_DeviceParams.m_hInstance	= NULL;
		m_pRenderApplication->m_DeviceParams.m_CmdShow		= 0;

#ifdef USE_REF_DEVICE
		LOGDEBUG(g_Log, "Initialising D3D HAL Device");
		m_pRenderApplication->m_DeviceParams.m_UseREF		= false;
#else
		LOGDEBUG(g_Log, "Initialising D3D Reference Device");
		m_pRenderApplication->m_DeviceParams.m_UseREF		= true;
#endif
		//MSAA is not guaranteed to work on build machines, so we'll have to manually supersample
		m_pRenderApplication->m_DeviceParams.m_Multisample	= Display::E_Multisample_None;
		m_pRenderApplication->m_DeviceParams.m_hWnd			= hwnd ? hwnd : GetConsoleWindow();

		if (!m_pRenderApplication->Create(1, 1))
		{
			SAFE_DELETE_PTR(m_pRenderApplication)
			return false;
		}
	}
	return true;
}

void Renderer::DeviceDestroy()
{
	SAFE_DELETE_PTR(m_pScene);
	if (m_pRenderApplication)
		m_pRenderApplication->Destroy();
	SAFE_DELETE_PTR(m_pRenderApplication);
}

bool Renderer::IsInitialised()
{
	return m_pRenderApplication != NULL;
}

bool Renderer::BeginRendering()
{
	AftkRender::C_ViewContext *pContext = GetViewContext();
	if (pContext)
	{
		assert(m_pRenderApplication);
		return m_pRenderApplication->BeginRendering(pContext);

	}
	return false;
}

void Renderer::EndRendering()
{
	AftkRender::C_ViewContext *pContext = GetViewContext();
	if (pContext)
	{
		assert(m_pRenderApplication);
		m_pRenderApplication->EndRendering(pContext, false);
	}
}

AftkRender::C_Scene& Renderer::GetScene()
{
	if (!m_pScene)
		m_pScene = new AftkRender::C_Scene();
	
	assert(m_pScene);
	return *m_pScene;
}

AftkRender::C_ViewContext* Renderer::GetViewContext()
{
	if (m_pRenderApplication)
	{
		//we don't want the implicit swap chain created by the device.
		//create/retrieve the first swap chain context
		AftkRender::C_ViewContext *pContext = m_pRenderApplication->GetViewContext(1);
		if (!pContext)
		{
			Display::C_DeviceParams params;
			params.m_Width = 1024;
			params.m_Height = 1024;
			params.m_Multisample = Display::E_Multisample_4;
			params.m_hWnd = (HWND) m_pRenderApplication->m_DeviceParams.m_hWnd ? m_pRenderApplication->m_DeviceParams.m_hWnd : GetConsoleWindow();
			pContext = m_pRenderApplication->AddViewContext(params);
			
			//HACK: there seems to be a problem with the initial surfaces that are created (no D3DUSAGE_RENDERTARGET specified for some unknown reason)
			//NOTE: this also happens in actorstudio ;)
			pContext->Resize(1024,1024);
			return pContext;
		}

		return pContext;
	}

	return 0;
}



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


C_ImposterRenderer::C_ImposterRenderer()
: AftkRender::C_SceneRenderer()
{
	m_Flags.fillmode = Display::E_FillMode_Solid;
	m_Flags.cullmode = Display::E_CullMode_None;
	m_Flags.xray = false;
	m_Flags.textured = true;
	m_Flags.lit = false;
}

C_ImposterRenderer::~C_ImposterRenderer()
{
}

void C_ImposterRenderer::Render()
{
	Display::Device()->SetFillMode((Display::E_FillMode)m_Flags.fillmode);
	Display::Device()->SetCullMode((Display::E_CullMode) m_Flags.cullmode);

	for (S32 i=0; i<m_RenderContextArray.Size(); ++i)
	{
		const AftkRender::C_SceneGraphNode *pNode = m_RenderContextArray[i].first;

		AftkRender::C_RenderContext& rc = m_RenderContextArray[i].second;
		const AftkRender::C_GeometryCluster *pCluster = rc.GetGeometryCluster();
		U32 batch = rc.GetBatch();

		// view frustrum culling
		const Display::C_ViewFrustum& frustum = Display::Device()->GetViewFrustum();
		S32 cullResult = frustum.CullAxisBox(pNode->GetWorldAABBox());
		if (cullResult > 0)
		{
			S16 materialIndex = rc.GetRemappedMaterialIndex();
			if (materialIndex != -1)
			{
				const AftkRender::C_Scene *pScene = pNode->GetScene();
				if (pScene)
				{
					S32 materialIndexMapping = pScene->GetMaterialIndexMapping(materialIndex);
					const AftkRender::C_Material *pMaterial = pScene->GetMaterial(materialIndexMapping);
					assert(pMaterial && "Invalid material index");

					if (m_Flags.textured)
						pMaterial->Enable();

					if (pMaterial->GetCullMode() == Display::E_CullMode_Default &&
						m_Flags.cullmode != Display::E_CullMode_Default)
						Display::Device()->SetCullMode((Display::E_CullMode) m_Flags.cullmode);

					if (!pMaterial->TexturesLoaded() || !pMaterial->GetNumTextures() || !m_Flags.textured)
						Display::Device()->SetTexture(0, m_TextureManager.GetDefault2DTexture());
				}
			}
			else
				Display::Device()->SetTexture(0, m_TextureManager.GetDefault2DTexture());

			rc.Apply();

			Display::Device()->SetAlphaTestEnable(Display::E_Bool_True);
			Display::Device()->SetAlphaTestFunc(Display::E_CmpFunc_Greater);
			Display::Device()->SetAlphaTestRef(0x00);
			Display::Device()->SetBlendEnable(Display::E_Bool_True);
			Display::Device()->SetBlendSrc(Display::E_Blend_SrcAlpha);
			Display::Device()->SetBlendDst(Display::E_Blend_InvSrcAlpha);
			Display::Device()->SetBlendOp(Display::E_BlendOp_Add);
			Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
			pCluster->RenderGeometry(batch);

			Display::Device()->SetBlendEnable(Display::E_Bool_False);
			Display::Device()->SetAlphaTestRef(0x80);
			Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
			pCluster->RenderGeometry(batch);
		}
	}

	Display::Device()->SetFillMode(Display::E_FillMode_Default);
	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetBlendMode(Display::E_BlendMode_Default);
}

void C_ImposterRenderer::AddRenderContext(AftkRender::C_RenderContext& rc, const AftkRender::C_SceneGraphNode *pNode)
{
	const AftkRender::C_GeometryCluster *pCluster = rc.GetGeometryCluster();
	const AftkRender::C_PrimitiveBatch& batch = pCluster->GetPrimitiveBatch(rc.GetBatch());

	//ignore non-polygonal primitives
	if (batch.primitiveType == AftkRender::E_PrimitiveType_PointList ||
		batch.primitiveType == AftkRender::E_PrimitiveType_LineList  ||
		batch.primitiveType == AftkRender::E_PrimitiveType_LineStrip)
		return;

	//set shader for cluster
	{
		VSLookupIndex vsIndex(pCluster->GetVertexDeclaration());
		if (rc.GetFlags().isBillboard)
			rc.SetVertexShader(m_ShaderManager.GetVertexShader(VertexShaderBillboardLookup[vsIndex.index.value]));
		else if (rc.GetFlags().isSkinned)
			rc.SetVertexShader(m_ShaderManager.GetVertexShader(VertexShaderSkinnedLookup[vsIndex.index.value]));
		else
			rc.SetVertexShader(m_ShaderManager.GetVertexShader(VertexShaderBasicLookup[vsIndex.index.value]));

		if (vsIndex.index.colour)
		{
			//modulate with vertex colour
			if (m_Flags.lit)
				rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicNdotLTextureVertexColourLighting.shader"));
			else
				rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicTextureVertexColourLighting.shader"));
		}
		else
		{
			if (m_Flags.lit)
				rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicNdotLTexture.shader"));
			else
				rc.SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicTexture.shader"));
		}
	}

	m_RenderContextArray.PushBack(Ut::Pair<const AftkRender::C_SceneGraphNode*, AftkRender::C_RenderContext>(pNode, rc));
}


}