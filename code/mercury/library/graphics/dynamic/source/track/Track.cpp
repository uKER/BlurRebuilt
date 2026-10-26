#include "stdafx.h"

#include "Dynamic/Track/Track.h"
#include "Dynamic/TypeIndex.h"
#include "Dynamic/Shaders/Constants.h"
#include "Render/State/RenderState.h"

#include "SceneGraph/View.h"
#include "Display/Device/Device.h"	// [Blur2PC] T174: the track strip draws through the device

#include "../../../../../../amax/game/Game/GameWorld/ViewType.h"

EXTERN_SHADER(vsTrack);
EXTERN_SHADER(psTrackAlpha);
EXTERN_SHADER(psTrackAlphaHDR);
EXTERN_SHADER(psTrackAlphaHDRU32);
EXTERN_SHADER(psTrackMultiply);

namespace DynamicEffect
{
namespace Track
{

C_TrackRenderer::C_TrackRenderer(C_Renderer* pRenderer) : I_DynamicEffect(pRenderer, NULL, DynamicEffect::E_EffectTypeIndex_Tracks, DynamicEffect::E_EffectTypeIndex_Tracks)
{
	m_bAlwaysUpdate = true;
	m_MaterialIndex = 0;
	m_nEmitters = 0;
	m_nElements = 0;

	m_fAlphaScale = 1.0f;

	m_NodeType = pRenderer->GetNodeType();
	m_RenderPassType = pRenderer->GetRenderPassMapping(DynamicEffect::E_RenderPassIndex_Decal);
	pRenderer->AddBindPixelConstantsEffect(this);
}

C_TrackRenderer::~C_TrackRenderer()
{
	m_pRenderer->RemoveBindPixelConstantsEffect(this);
	m_Emitters.Free();
}

bool C_TrackRenderer::Initialise()
{
	return true;
}

bool C_TrackRenderer::Create()
{
	if (!Display::CreateVertexShaderFromFunc("vsTrack", 0, &m_VertexShader, REF_SHADER(vsTrack)))
		return false;
	
	if(Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		// choose which set of shaders we need to use - we only have depth fog for shader tonemap at the moment
		if(eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("psTrackAlphaHDRU32", 0, &m_PixelShaderAlpha, REF_SHADER(psTrackAlphaHDRU32)))
				return false;
		}
		else if(eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psTrackAlphaHDR", 0, &m_PixelShaderAlpha, REF_SHADER(psTrackAlphaHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psTrackAlpha", 0, &m_PixelShaderAlpha, REF_SHADER(psTrackAlpha)))
			return false;
	}

	if (!Display::CreatePixelShaderFromFunc("psTrackMultiply", 0, &m_PixelShaderMultiply, REF_SHADER(psTrackMultiply)))
		return false;

	m_LuminanceParam = m_PixelShaderAlpha.GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE, "g_vLuminance");

	m_SamplerState.SetDefaultStates();
	m_SamplerState.SetTextureFilter(Display2::E_TextureFilter_Linear);
	m_SamplerState.SetTextureAddressU(Display2::E_TextureAddress_Clamp);
	m_SamplerState.SetTextureAddressV(Display2::E_TextureAddress_Wrap);
	m_SamplerState.SetTextureAddressW(Display2::E_TextureAddress_Clamp);

	return true;
}

void C_TrackRenderer::Destroy()
{
	if(m_VertexShader.IsCreated())
	{
		Display::Device()->Destroy(&m_VertexShader);
	}
	if(m_PixelShaderAlpha.IsCreated())
		Display::Device()->Destroy(&m_PixelShaderAlpha);
	if(m_PixelShaderMultiply.IsCreated())
		Display::Device()->Destroy(&m_PixelShaderMultiply);
}

void C_TrackRenderer::Reset()
{
	for (EmitterList::Iterator pEmitter = m_Emitters.m_Active.Begin(); pEmitter != m_Emitters.m_Active.End(); ++pEmitter)
	{
		// make sure all the emitters clear their current track pointer!
		pEmitter->Destroy();
	}

	// go through track list
	while(!m_Tracks.Empty())
	{
		TrackList::Iterator pTrackIterator = m_Tracks.Begin();

		pTrackIterator->Clear();
		
		// remove track from list and give back to pool
		C_Track* pDelete = &(*pTrackIterator);
		m_Tracks.PopFront();
		FreeTrack(pDelete);
	}
}

void C_TrackRenderer::Shutdown()
{
	
}

void C_TrackRenderer::ReserveEmitters(const U32 nEmitters)
{
	if(nEmitters > m_nEmitters)
	{
		for(U32 i = m_nEmitters; i < nEmitters; ++i)
		{
			m_Emitters.Cache(new C_Emitter(this));
		}
		m_nEmitters = nEmitters;
	}
}

void C_TrackRenderer::Update()
{
	PERF_EVENT("TrackRenderer::Update");

	for (EmitterList::Iterator pEmitter = m_Emitters.m_Active.Begin(); pEmitter != m_Emitters.m_Active.End(); ++pEmitter)
	{
		pEmitter->Update();
	}

	if(m_nElements >= g_nParticlesMinusFade)
	{
		U32 nElementsFade = m_nElements - g_nParticlesMinusFade;
		U8 iFade = nElementsFade;
		F32 fFade = 1.0f - ((F32)nElementsFade / (F32)g_nParticlesFade);
		F32 fFadeDelta = 1.0f / (F32)g_nParticlesFade;

		for(TrackList::ReverseIterator pTrackIterator = m_Tracks.ReverseBegin(); pTrackIterator != m_Tracks.ReverseEnd() && iFade > 0; ++pTrackIterator)
		{
			C_Track* pTrack = &(*pTrackIterator);

			for(ParticleList::ReverseIterator pParticleIterator = pTrack->m_Particles.ReverseBegin(); pParticleIterator != pTrack->m_Particles.ReverseEnd() && iFade > 0; ++pParticleIterator)
			{
				pParticleIterator->fFadeAlpha = fFade;
				fFade += fFadeDelta;
				--iFade;
			}
		}
	}
}

void C_TrackRenderer::CleanMaterial(const C_Material* pMaterial)
{
	for(TrackList::Iterator pTrackIterator = m_Tracks.Begin(); pTrackIterator != m_Tracks.End(); )
	{
		C_Track* pTrack = &(*pTrackIterator);

		if(pTrack->m_pMaterial == pMaterial)
		{
			pTrack->Clear();
			pTrackIterator = m_Tracks.Erase(pTrackIterator);
			FreeTrack(pTrack);
		}
		else
			++pTrackIterator;
	}

	// free up any active emitters which are referencing this material
	for (EmitterList::Iterator pIterator = m_Emitters.m_Active.Begin(); pIterator != m_Emitters.m_Active.End(); )
	{
		C_Emitter* pEmitter(&(*pIterator));
		++pIterator;
		if(pEmitter->m_pMaterial == pMaterial)
			pEmitter->Free();
	}
}

void C_TrackRenderer::OnGather(const SceneGraph::C_View& view) const
{
	PERF_EVENT("DynamicEffect.GatherTracks");
	
	if (view.GetType() == Game::E_ViewType_LightTrails)
		return;

	// cull all tracks to this view
	for(TrackList::ConstIterator pTrack = m_Tracks.Begin(); pTrack != m_Tracks.End(); ++pTrack)
	{
		if (pTrack->m_VisibilityFlags & view.GetTypeFlag())
		{
			Maths::HVec4 vCentreOne(Maths::Load<Maths::HVec4>(&pTrack->m_vCentre));
			Maths::HVec4 vSizeCull(Maths::Load<Maths::HVec4>(&pTrack->m_vSize));

			S32 visible = view.GetViewFrustum().CullAxisBox(vCentreOne, vSizeCull);

			if(visible)
			{
				// add these to the decal render pass
				
				view.AddVisibleNode(&(*pTrack), m_RenderPassType);
			}
		}
	}
}

void C_TrackRenderer::BindPixelConstants(const SceneGraph::C_View& view, Display::C_Context* pDisplay) const
{
	Render::C_RenderState* pRenderState = &Render::C_RenderState::Instance();
	MathsStorage::Float4 hdrParam = pRenderState->ShaderHDR() ? pRenderState->HDR()->GetShaderParameters(view.GetDisplayView()->GetUserIndex()) : pRenderState->GetGainShaderParameters();
		
	if(pDisplay)
		pDisplay->SetPixelShaderConstantF(CP_RENDER_POSTPROCESS_LUMINANCE, (F32*)&hdrParam, 1);
	else
		Display::Device()->SetPixelShaderParameterF(m_LuminanceParam, (F32*)&hdrParam);
}

// [Blur2PC] T174: retail PC (Blur.exe 1.2, C_TrackRenderer::OnRender 0x005d8860) draws only while this
// debug value is set (registered at 0x00e17840, default on; value byte 0x011453b8); the snapshot has no
// such value.
Debug::Bool g_TyreTrackEnable("Vehicle.Effects.Wheel.TyreTrackEnable", true);

void C_TrackRenderer::OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& batch, const U32 states) const
{
	PERF_EVENT("TrackRenderer.Render");

	if(!g_TyreTrackEnable)	// [Blur2PC] T174
		return;

	U32 vertexCount = 0;

	for(S32 i = 0; i < batch.blocks.Size(); ++i)
	{
		C_Track* pTrack = (C_Track*)batch.blocks[i];
		vertexCount += pTrack->m_Particles.Size() * 2 + 2;
	}

	if(vertexCount > 0)
		vertexCount -= 2;

	// bind effect wide states
	if(states && E_States_BindEffect)
	{
	#ifndef PLATFORM_PS3
		BindPixelConstants(*view.GetView(), pDisplay);
	#endif
				
		pDisplay->SetVertexShader(&m_VertexShader);
		pDisplay->SetVertexDeclaration(Vertex::m_pDeclaration);
		pDisplay->SetSamplerState(0, &m_SamplerState);
	}
	
	// bind per render states
	if(states && E_States_BindEffectVolatile)
	{
		const C_Track* pTrack = (const C_Track*)batch.blocks[0];
		const C_Material* pMaterial = pTrack->m_pMaterial;
		
		assert(pMaterial);

		if(pMaterial->m_Settings.blendMode == Display::E_BlendMode_Multiply)
		{
			pDisplay->SetBlendHighPrecisionEnable(Display::E_Bool_False);
			pDisplay->SetPixelShader(&m_PixelShaderMultiply);
		}
		else
		{
			pDisplay->SetBlendHighPrecisionEnable(Display::E_Bool_True);
			pDisplay->SetPixelShader(&m_PixelShaderAlpha);
		}

		pDisplay->SetBlendMode(pMaterial->m_Settings.blendMode);
		pDisplay->SetTexture(0, pMaterial->m_Settings.pDiffuse->GetGPUTexture());
	}

	const C_Particle* pParticle;
	const C_Particle* pPrev = NULL;
	// [Blur2PC] T174: retail PC writes the strip through the device (0x005d8860: Device BeginVertices
	// 0x005abb80 with checkUsage false, EndVertices 0x005abbe0). The PC context's BeginVertices is a stub
	// that returns NULL (Context_D3D9.cpp), so through the context no track was ever drawn on PC.
	Vertex* pVertices = (Vertex*)Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleStrip, vertexCount, sizeof(Vertex), Vertex::m_pDeclaration, false);

	if(pVertices)
	{
		for(S32 i = 0; i < batch.blocks.Size(); ++i)
		{
			const C_Track* pTrack = (const C_Track*)batch.blocks[i];

			// add degenerate triangles to link individual tracks
			if(pPrev)
			{
				ParticleList::ConstIterator pParticleIterator = pTrack->m_Particles.Begin();
				pParticle = &(*pParticleIterator);

				pVertices->x = pPrev->vPosition2.x;
				pVertices->y = pPrev->vPosition2.y;
				pVertices->z = pPrev->vPosition2.z;
				pVertices->r = 0.0f;
				pVertices->g = 0.0f;
				pVertices->b = 0.0f;
				pVertices->a = 0.0f;
				pVertices->u = 0.0f;
				pVertices->v = 0.0f;
				++pVertices;

				pVertices->x = pParticle->vPosition1.x;
				pVertices->y = pParticle->vPosition1.y;
				pVertices->z = pParticle->vPosition1.z;
				pVertices->r = 0.0f;
				pVertices->g = 0.0f;
				pVertices->b = 0.0f;
				pVertices->a = 0.0f;
				pVertices->u = 0.0f;
				pVertices->v = 0.0f;
				++pVertices;
			}

			for(ParticleList::ConstIterator pParticleIterator = pTrack->m_Particles.Begin(); pParticleIterator != pTrack->m_Particles.End(); ++pParticleIterator)
			{
				pParticle = &(*pParticleIterator);
				
				float alpha = pParticle->vColour.w * pParticle->fFadeAlpha * m_fAlphaScale;
				if (pParticleIterator == pTrack->m_Particles.Begin())
					alpha = 0.f;

				pVertices->x = pParticle->vPosition1.x;
				pVertices->y = pParticle->vPosition1.y;
				pVertices->z = pParticle->vPosition1.z;
				pVertices->r = pParticle->vColour.x;
				pVertices->g = pParticle->vColour.y;
				pVertices->b = pParticle->vColour.z;
				pVertices->a = alpha;
				pVertices->u = 1.0f;
				pVertices->v = pParticle->fV;
				++pVertices;

				pVertices->x = pParticle->vPosition2.x;
				pVertices->y = pParticle->vPosition2.y;
				pVertices->z = pParticle->vPosition2.z;
				pVertices->r = pParticle->vColour.x;
				pVertices->g = pParticle->vColour.y;
				pVertices->b = pParticle->vColour.z;
				pVertices->a = alpha;
				pVertices->u = 0.0f;
				pVertices->v = pParticle->fV;
				++pVertices;

				pPrev = pParticle;
			}	
		}
			
		Display::Device()->EndVertices();	// [Blur2PC] T174
	}
}

MathsStorage::Int2 C_TrackRenderer::OnDisplayDebug(MathsStorage::Int2 position) const
{
	Util::C_String str;
	str.Printf("Track System");
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.x += 25;
	position.y += 25;
	
	str.Printf("Emitters: %i", m_Emitters.m_Active.Size());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	str.Printf("Tracks: %i", m_Tracks.Size());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	position.x -= 25;

	return position;
}


C_EmitterHandle C_TrackRenderer::NewEmitter(const C_Material* pMaterial)
{
	if(C_Emitter* pEmitter = m_Emitters.Allocate())
	{
		pEmitter->m_pMaterial = pMaterial;
		C_EmitterHandle handle(pEmitter);
		return handle;
	}
	else
		return C_EmitterHandle();
}

void C_TrackRenderer::FreeEmitter(C_Emitter* pEmitter)
{
	pEmitter->m_iSalt += 1;
	m_Emitters.Free(pEmitter);
}

C_Particle* C_TrackRenderer::NewParticle()
{
	while(m_ParticlePool.Empty())
		Deallocate();

	++m_nElements;
	return m_ParticlePool.Allocate();
}

void C_TrackRenderer::Deallocate()
{
	TrackList::ReverseIterator pIterator = m_Tracks.ReverseBegin();
		
	// start at end
	if(pIterator != m_Tracks.ReverseEnd())
	{
		C_Track* pTrack = &(*pIterator);
		// make sure this is an inactive track
		assert(!pTrack->m_Particles.Empty());
		// remove the last particle of the track
		C_Particle* pParticle = &pTrack->m_Particles.Back();
		pTrack->m_Particles.PopBack();
		FreeParticle(pParticle);

		// either remove block or calculate new extents
		if(pTrack->m_Particles.Size() <= 2)
		{
			pTrack->Clear();
			
			// if track is active then make sure pointer to it is made null
			if(pTrack->m_bActive)
				pTrack->m_pEmitter->Destroy();

			pIterator = m_Tracks.Erase(pIterator);
			FreeTrack(pTrack);
		}
		else
			pTrack->CalculateExtents();

	}
}

void C_TrackRenderer::FreeParticle(C_Particle* pParticle)
{
	--m_nElements;
	m_ParticlePool.Free(pParticle);
}

C_Track* C_TrackRenderer::NewTrack()
{
	// allocates a new track and puts it on front of track list
	while(m_TrackPool.Empty())
		Deallocate();

	C_Track* pTrack = m_TrackPool.Allocate();
	pTrack->SetNodeType(m_NodeType);
	pTrack->m_pEffect = this;
	m_Tracks.PushFront(*pTrack);
	++m_nBlocks;

	return pTrack;
}

void C_TrackRenderer::FreeTrack(C_Track* pTrack)
{
	m_TrackPool.Free(pTrack);
	--m_nBlocks;
}

// go through all tracks and remove the current track back to storage
void C_TrackRenderer::EraseTrack(C_Track* pTrack)
{
	pTrack->Clear();
	TrackList::Iterator pIterator(pTrack);
	m_Tracks.Erase(pIterator);
	FreeTrack(pTrack);
}

void C_TrackRenderer::OnBuild(const C_View& view, C_CompositeRenderNode& node) const
{
	node.iVertexBuffer = 0xFFFFFFFF;
}


// clears all particle from track
void C_Track::Clear()
{
	C_TrackRenderer* pTrackRenderer = (C_TrackRenderer*)m_pEffect;

	while(!m_Particles.Empty())
	{
		C_Particle* pDeath = &m_Particles.Front();
		m_Particles.PopFront();
		pTrackRenderer->FreeParticle(pDeath);
	}
}

// update the bounds using this particle
void C_Track::UpdateExtents(const C_Particle* pParticle)
{
	Maths::HVec3 vCentre(Maths::Load<Maths::HVec3>(&m_vCentre));
	Maths::HVec3 vSize(Maths::Load<Maths::HVec3>(&m_vSize));

	Maths::HVec3 vMin = vCentre - vSize;
	Maths::HVec3 vMax = vCentre + vSize;

	Maths::HVec3 vPosition1(Maths::Load<Maths::HVec3>(&pParticle->vPosition1));
	Maths::HVec3 vPosition2(Maths::Load<Maths::HVec3>(&pParticle->vPosition2));
	
	vMax = Maths::Max(vMax, vPosition1);
	vMin = Maths::Min(vMin, vPosition1);

	vMax = Maths::Max(vMax, vPosition2);
	vMin = Maths::Min(vMin, vPosition2);

	vCentre = (vMax + vMin) * Maths::Constants<Maths::HF32>::Half();
	vSize = vMax - vCentre;

	Maths::HVec4 vCentre2(vCentre);
	Maths::HVec4 vSize2(vSize);

	vSize2.W(1000.0f);
	vCentre2.W(Maths::Constants<Maths::HF32>::One());
	
	Maths::Store(vCentre2, &m_vCentre);
	Maths::Store(vSize2, &m_vSize);
}

// calculate the bounds of the track from scratch
void C_Track::CalculateExtents()
{
	Maths::HVec3 vMax(Maths::HF32(-10000.0f));
	Maths::HVec3 vMin(Maths::HF32(10000.0f));

	for(ParticleList::Iterator pParticleIterator = m_Particles.Begin(); pParticleIterator != m_Particles.End(); ++pParticleIterator)
	{
		C_Particle* pParticle = &(*pParticleIterator);

		Maths::HVec3 vPosition1(Maths::Load<Maths::HVec3>(&pParticle->vPosition1));
		Maths::HVec3 vPosition2(Maths::Load<Maths::HVec3>(&pParticle->vPosition2));
		
		vMax = Maths::Max(vMax, vPosition1);
		vMin = Maths::Min(vMin, vPosition1);

		vMax = Maths::Max(vMax, vPosition2);
		vMin = Maths::Min(vMin, vPosition2);
	}

	Maths::HVec3 vCentre = (vMax + vMin) * Maths::Constants<Maths::HF32>::Half();
	Maths::HVec3 vSize = vMax - vCentre;

	Maths::HVec4 vCentre2(vCentre);
	Maths::HVec4 vSize2(vSize);

	// TODO: fix this
	vSize2.W(1000.0f);
	vCentre2.W(Maths::Constants<Maths::HF32>::One());
	
	Maths::Store(vCentre2, &m_vCentre);
	Maths::Store(vSize2, &m_vSize);
}

// Gets rid of all tracks created after a given frame

void C_TrackRenderer::FlushTracks(U32 iFrame)
{
	for(EmitterList::Iterator pEmitter = m_Emitters.m_Active.Begin(); pEmitter != m_Emitters.m_Active.End(); ++pEmitter)
	{
		// make sure all the emitters get ended where they are
		pEmitter->Destroy();
	}

	if(!m_Tracks.Empty())
	{
		// step backwards through list
		TrackList::Iterator pIterator;
		
		for(pIterator = m_Tracks.Begin(); pIterator != m_Tracks.End();)
		{
			C_Track* pTrack = &(*pIterator);

			assert(!pTrack->m_Particles.Empty());

			//Clear the whole track
			if (pTrack->m_iStartFrame > iFrame && pTrack->m_iEndFrame > iFrame)
			{
				//Free all particles
				pTrack->Clear();
			}
			//Only clear some particles of the track
			else if (pTrack->m_iEndFrame > iFrame || pTrack->m_iEndFrame == 0)
			{
				C_Particle* pParticle = &pTrack->m_Particles.Front();

				while (pParticle && pParticle->iFrameCreated > iFrame)
				{
					C_Particle* pDeath = &pTrack->m_Particles.Front();
					pTrack->m_Particles.PopFront();
					FreeParticle(pDeath);

					if (!pTrack->m_Particles.Empty())
						pParticle = &pTrack->m_Particles.Front();
					else
						pParticle = NULL;
				}
			}		

			if(pTrack->m_Particles.Size() <= 2)
			{
				pTrack->Clear();
				pIterator = m_Tracks.Erase(pIterator);
				FreeTrack(pTrack);
			}
			else
				++pIterator;
		}
	}
}


void C_TrackRenderer::AddMaterial(C_Material *pMaterial)
{
	pMaterial->m_iIndex = m_MaterialIndex++;
	m_Materials.Insert(pMaterial);
}

void C_TrackRenderer::RemoveMaterial(C_Material *pMaterial)
{
	Materials::Iterator it = m_Materials.Find(pMaterial);
	assert(it != m_Materials.End());
	if(it != m_Materials.End())
		m_Materials.Erase(it);
}

// Material

C_Material::C_Material(DynamicEffect::Track::C_TrackRenderer *pTrackRenderer, const MaterialSettings& settings) : m_pTrackRenderer(pTrackRenderer)
{
	m_Settings = settings;
	m_pTrackRenderer->AddMaterial(this);
}

C_Material::~C_Material()
{
	m_Settings.pDiffuse->Release();
	m_pTrackRenderer->RemoveMaterial(this);
}

} // Track
} // DynamicEffect



