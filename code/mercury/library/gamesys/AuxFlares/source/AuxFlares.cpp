#include "AuxFlaresPch.h"
#include "AuxFlares/AuxFlares.h"
#include "display/shaders/Constants.h"
#include <SceneGraph/View.h>

#define SCENEGRAPH_ALLOW_INCLUDE // only for AuxFlares.cpp
#include <SceneGraph/SceneGraphImpl.h>

#ifdef PLATFORM_PS3
#include <cell/cgb.h>
#endif

EXTERN_SHADER(vsAuxFlares);
EXTERN_SHADER(psAuxFlares);

namespace AuxFlares
{
	Debug::Logger &g_AuxFlaresLog( Debug::GetLogger("gamesys.auxFlares") );

	typedef Display::Vertex_XYZ_HDR_UV_N Vertex;

	C_AuxFlares::C_AuxFlares(SceneGraph::C_SceneGraph* pSceneGraph, const char* flareTextureName) : Display::I_Renderer("AuxFlares"), SceneGraph::I_Renderer(pSceneGraph)
	{
		m_FlareTextureName = flareTextureName;
		m_BufferIndex = 0;
		m_bCreated = false;
		m_resourcesPending = true;
		m_SamplerState.SetDefaultStates();
		m_SamplerState.SetTextureFilter(Display2::E_TextureFilter_Linear);
		m_SamplerState.SetTextureAddress(Display2::E_TextureAddress_Clamp);
#if PLATFORM_PS3
		m_SamplerState.SetTextureGamma(1);
#endif
	}

	C_AuxFlares::~C_AuxFlares()
	{
		DestroyRenderer();
	}

	C_AuxFlareInstance* C_AuxFlares::AllocAuxFlareInstance()
	{
		m_AuxFlareList.PushBack();
		C_AuxFlareInstance* pAuxFlare = &m_AuxFlareList.Back();
		pAuxFlare->m_Type = m_NodeType;
		return pAuxFlare;
	}

	bool C_AuxFlares::OnCreate()
	{
		if (m_bCreated)
			return true;

		RESOURCE_MEMORY_ID("AuxFlares");

		if (!Display::CreateVertexShaderFromFunc("Shader/vsAuxFlares", 0, &m_FlareVertexShader, REF_SHADER(vsAuxFlares)))
			return false;

		m_FlareSizeParameter = m_FlareVertexShader.GetParameter(200, "g_FlareSize");
		m_FlareColourParameter = m_FlareVertexShader.GetParameter(201, "g_FlareColour");

		if (!Display::CreatePixelShaderFromFunc("Shader/psAuxFlares", 0, &m_FlarePixelShader, REF_SHADER(psAuxFlares)))
			return false;

		const Util::C_String fileName(m_FlareTextureName);
		if(fileName.ISearch(".texture") != -1 && Sys::GetDefaultFileSystem()->FileExists(fileName))
		{
			m_FlareTextureHandle.Load(Util::C_String(fileName), this);
		}
		else
		{
			LOGWARN(g_AuxFlaresLog, "Flare texture %s not found", fileName.Chars());	
		}

		m_bCreated = true;

		return true;
	}

	void C_AuxFlares::OnDestroy()
	{
		Display::Device()->Destroy(&m_FlareVertexShader);
		Display::Device()->Destroy(&m_FlarePixelShader);

		m_bCreated = false;
	}

	// scenegraph I_GatherNode implementation
	void C_AuxFlares::OnGather(const SceneGraph::C_View& view) const
	{
		if (view.RenderNodeEnabled(m_NodeFlag))
		{
			for (Btl::List<C_AuxFlareInstance>::ConstIterator it = m_AuxFlareList.Begin(); it != m_AuxFlareList.End(); ++it)
			{
				const C_AuxFlareInstance& flare = *it;

				// see if flare is visible in this view
				if(flare.m_ViewFlags & view.GetTypeFlag())
				{
					//Frustum check
					Maths::HF32 radius = Maths::Max(m_ViewSettings[view.GetType()].sizeX, m_ViewSettings[view.GetType()].sizeY);
					Maths::HVec4 vCentreOne(flare.m_Position || Maths::HVecW(1.f));
					Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&view.GetViewData().forwardZ));
					Maths::HF32 depthScale(view.GetViewData().depthScale);
					Maths::HF32 depth = (vCentreOne | forward);

					if (depth < (m_ViewSettings[view.GetType()].farDistanceFade + radius))
					{
						S32 visible = view.GetViewFrustum().CullSphere(vCentreOne, radius);
						
						if (visible)
						{
							// see if we need to calculate depth for sorting
							const SceneGraph::C_RenderPass* pRenderPass = SceneGraph::I_Renderer::m_pSceneGraph->GetRenderPass(m_RenderPass);
							SceneGraph::VolatileNodeInfo volatileNodeInfo;

								// calculate depth if we need it
							if(pRenderPass->GetVolatileInfoFlags() & SceneGraph::E_VolatileNodeInfo_Distance)
							{
								depth *= depthScale;
								volatileNodeInfo.distance = depth.F2I();
							}

							view.AddVisibleNode(&flare, m_RenderPass, 0, volatileNodeInfo.data);
						}
					}
				}
			}
		}
	}

	void C_AuxFlares::AddFlare(volatile Vertex* pVertices,  const C_AuxFlareInstance& flare, const F32 fadeFactor) const
	{
		Maths::HVec4 colour(flare.m_Colour.R() * flare.m_Intensity * fadeFactor, flare.m_Colour.G() * flare.m_Intensity * fadeFactor, flare.m_Colour.B() * flare.m_Intensity * fadeFactor, flare.m_Size);
	
		// when writing to write-combine memory writes *must* be in order or we get massive penalties of up to x50 slower write performance
		Maths::Store(flare.m_Position, (MathsStorage::Float3*)&pVertices[0].x);
		Maths::Store(colour, (MathsStorage::Float4*)&pVertices[0].r);
		pVertices[0].u = -1.f;
		pVertices[0].v = 1.f;
		Maths::Store(flare.m_Direction, (MathsStorage::Float3*)&pVertices[0].nx);

		Maths::Store(flare.m_Position, (MathsStorage::Float3*)&pVertices[1].x);
		Maths::Store(colour, (MathsStorage::Float4*)&pVertices[1].r);
		pVertices[1].u = 1.f;
		pVertices[1].v = 1.f;
		Maths::Store(flare.m_Direction, (MathsStorage::Float3*)&pVertices[1].nx);

		Maths::Store(flare.m_Position, (MathsStorage::Float3*)&pVertices[2].x);
		Maths::Store(colour, (MathsStorage::Float4*)&pVertices[2].r);
		pVertices[2].u = 1.f;
		pVertices[2].v = -1.f;
		Maths::Store(flare.m_Direction, (MathsStorage::Float3*)&pVertices[2].nx);

		Maths::Store(flare.m_Position, (MathsStorage::Float3*)&pVertices[3].x);
		Maths::Store(colour, (MathsStorage::Float4*)&pVertices[3].r);
		pVertices[3].u = -1.f;
		pVertices[3].v = -1.f;
		Maths::Store(flare.m_Direction, (MathsStorage::Float3*)&pVertices[3].nx);
	}
	
	void C_AuxFlares::Render(const SceneGraph::C_View& view, SceneGraph::C_Context& context, const SceneGraph::C_VisibleNodes& visibleNodes, const SceneGraph::C_VisibleNodeBatch& batch) const
	{
		PERF_EVENT("AuxFlares.Render");

		if (!m_resourcesPending)
		{
			Display::C_Context* pDisplay = Display::GetContext(&context);

			if(context.GetContextFlags() != pDisplay->GetFlags())
			{
		#if PLATFORM_PS3
				pDisplay->SetVertexShaderConstantF(4, (F32*)&view.GetViewSettings3D().GetViewProjMatrix(), 4);
		#endif
			}
			context.SetContextFlags(pDisplay->GetFlags());

			//Setup states
			pDisplay->SetBlendMode(Display::E_BlendMode_AlphaPreMultiply);
			pDisplay->SetDepthEnable(Display::E_Bool_True);
			pDisplay->SetDepthWriteEnable(Display::E_Bool_False);
			pDisplay->SetCullMode(Display::E_CullMode_None);
			
			//Set Params
			Display::C_Float4 flareSizeParam(m_ViewSettings[view.GetType()].sizeX, m_ViewSettings[view.GetType()].sizeY, 0.f, 0.f);
			pDisplay->SetVertexShaderConstantF(m_FlareSizeParameter.m_RegisterIndex, (F32*)&flareSizeParam, 1);

			Display::C_Float4 flareColourParam(m_ViewSettings[view.GetType()].colour.R(), m_ViewSettings[view.GetType()].colour.G(), m_ViewSettings[view.GetType()].colour.B(), 1.f);
			pDisplay->SetVertexShaderConstantF(m_FlareColourParameter.m_RegisterIndex, (F32*)&flareColourParam, 1);

			const Display::C_Texture2D* pFlareTexture = NULL;
			TextureResource::C_TextureResource* pTextureResource = m_FlareTextureHandle;
			if (pTextureResource && pTextureResource->IsDataValid())
			{
				pFlareTexture = (const Display::C_Texture2D*) pTextureResource->GetDisplayTexture();
			}
			pDisplay->SetTexture(0, pFlareTexture ? pFlareTexture->GetGPUTexture() : NULL);
			pDisplay->SetSamplerState(0, &m_SamplerState);

			Display::C_SamplerUsageFlags flags;
			flags.SetEnabled(0, 0);
			pDisplay->SetSamplerUsageFlags(&flags);

			//Set vertex and pixel shaders
			pDisplay->SetVertexDeclaration(Vertex::m_pDeclaration);
#ifdef PLATFORM_PS3
			pDisplay->SetVtxAttrUsageFlags(UINT_MAX);
#endif
			pDisplay->SetVertexShader(&m_FlareVertexShader);
			pDisplay->SetPixelShader(&m_FlarePixelShader);

#ifdef PLATFORM_PS3
			uint32_t new_mask = 0;
			cellCgbGetVertexAttributeOutputMask( m_FlareVertexShader.GetCgbProgram(), &new_mask );
			// Enable User Clip 0 attribute
			new_mask |= CELL_GCM_ATTRIB_OUTPUT_MASK_UC0; 
			pDisplay->SetVertexAttribOutputMask( new_mask );
#endif

			S32 vertexCount = batch.m_NodeCount * 4;

			// knock out until we have a begin vertices method for PC
		#ifndef PLATFORM_PC
			Vertex* pVertices = (Vertex*)pDisplay->BeginVertices(Display::E_PrimitiveType_QuadList, vertexCount, sizeof(Vertex), Vertex::m_pDeclaration);

			if(pVertices)
			{
				for(S32 i = batch.m_StartIndex; i <= batch.m_EndIndex; ++i)
				{
					const C_AuxFlareInstance* pInstance = visibleNodes[i].GetNode<C_AuxFlareInstance>();
					
					//Calculate fade
					F32 farPlusRadius = m_ViewSettings[view.GetType()].farDistanceFade + Maths::Max(m_ViewSettings[view.GetType()].sizeX, m_ViewSettings[view.GetType()].sizeY);
					Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&view.GetViewData().forwardZ));
					Maths::HVec4 position(pInstance->m_Position);
					position.W(Maths::Constants<Maths::HF32>::One());
					Maths::HF32 depth = (position | forward);
					F32 fadeFactor = Maths::Clamp((F32(depth) - farPlusRadius) / (m_ViewSettings[view.GetType()].nearDistanceFade - farPlusRadius), 0.f, 1.f);

					AddFlare(pVertices, *pInstance, fadeFactor);
					pVertices += 4;
				}
				pDisplay->EndVertices();
			}
		#endif
		}
	}

	void C_AuxFlares::ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
	{
		switch(result)
		{
			case Resource::E_Notify_DataReady:
			{				
				m_resourcesPending = false;
			}
			break;
			case Resource::E_Notify_DataError:
			case Resource::E_Notify_NoData:
			break;
		}
	}
}
