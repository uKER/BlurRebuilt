#ifndef RENDER_DYNAMICEFFECT_TRACK_H
#define RENDER_DYNAMICEFFECT_TRACK_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Emitter.h"
#include "Dynamic/Renderer.h"
#include "Dynamic/Interface.h"
#include "Dynamic/Block.h"
#include "Dynamic/Handle.h"
#include "Display/Buffer/VertexDeclarations.h"

namespace DynamicEffect
{
namespace Track
{

// how much storage do we want to keep tyre marks around?
const U32 g_nParticles = 2500;
const U32 g_nParticlesFade = 255;
const U32 g_nParticlesMinusFade = g_nParticles - g_nParticlesFade; // fade out last 255 particles
// make num tracks equal num particles / 4 as minimum track size requires 2 particles
const U32 g_nTracks	= g_nParticles / 4;

typedef Display::Vertex_XYZ_HDR_UV Vertex;

class C_TrackRenderer;

class C_Particle : public Ut::ListElementDouble<C_Particle>
{
public:
	void					Destroy() { return; }
	MathsStorage::Float3	vPosition1;
	U32						iFrameCreated;
	MathsStorage::Float3	vPosition2;
	F32						fV;
	F32						fFadeAlpha;
	MathsStorage::Float4	vColour;
};

typedef Ut::UtList<C_Particle, Ut::DoubleEndedList, Ut::MaintainedCounter> ParticleList;

class C_TrackRenderer;
class C_Emitter;
class C_Material;

class ALIGN16 C_Track : public DynamicEffect::C_RenderNode, public Ut::ListElementDouble<C_Track>
{
public:
	C_Track() : m_VisibilityFlags(0xFFFFFFFF) {};
	void							Destroy() { return; }
	void							CalculateExtents();
	void							UpdateExtents(const C_Particle* pParticle);
	void							SetVisibilityFlags(U32 visibilityFlags) {m_VisibilityFlags = visibilityFlags;}

	void							Clear();
	ParticleList					m_Particles;
	C_Emitter*						m_pEmitter;
	const C_Material*				m_pMaterial;
	U32								m_iStartFrame;
	U32								m_iEndFrame;
	bool							m_bActive;
	U32								m_VisibilityFlags;
};

typedef Ut::UtList<C_Track, Ut::DoubleEndedList, Ut::MaintainedCounter> TrackList;
typedef Ut::ListPool<C_Emitter>						EmitterList;

typedef Ut::StaticPool<C_Particle, g_nParticles>	ParticlePool;
typedef Ut::StaticPool<C_Track, g_nTracks>			TrackPool;

typedef Ut::UtSet<C_Material*>						Materials;


class C_TrackRenderer : public DynamicEffect::I_DynamicEffect
{
public:
	C_TrackRenderer(C_Renderer* pRenderer);
	~C_TrackRenderer();
	
	void							FlushTracks(U32 iFrame); // remove all tracks past a given frame
	void							CleanMaterial(const C_Material* pMaterial);

	C_EmitterHandle					NewEmitter(const C_Material* pMaterial = NULL);
	void							FreeEmitter(C_Emitter* pEmitter);
	void							ReserveEmitters(const U32 nEmitters);

	void							SetAlphaScale(const F32 fAlpha);

	bool							Create();
	void							Destroy();

	

	friend class					C_Emitter;
	friend class					C_Material;
	friend class					C_Track;

protected:
	void							Deallocate();
	
	C_Track*						NewTrack();
	void							EraseTrack(C_Track* pTrack);
	void							FreeTrack(C_Track* pTrack);
	C_Particle*						NewParticle();
	void							FreeParticle(C_Particle* pParticle);

	void							OnBuild(const C_View& view, C_CompositeRenderNode& node) const;
	void							OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& node, const U32 states) const;
	MathsStorage::Int2				OnDisplayDebug(MathsStorage::Int2 position) const;
	void							OnGather(const SceneGraph::C_View& view) const;
	void							OnBindPixelConstants(const SceneGraph::C_View& view, Display::C_Context* pDisplay) const { BindPixelConstants(view, pDisplay); }

	void							AddMaterial(C_Material* pMaterial);
	void							RemoveMaterial(C_Material* pMaterial);

private:
	bool							Initialise();
	void							Reset();
	void							Shutdown();
	void							Update();

	void							BindPixelConstants(const SceneGraph::C_View& view, Display::C_Context* pDisplay) const;
	
	TrackList						m_Tracks;
	ParticlePool					m_ParticlePool;			// pool of particles
	TrackPool						m_TrackPool;			// pool of tracks
	Materials						m_Materials;
	U32								m_MaterialIndex;
	S32								m_NodeType;
	S32								m_RenderPassType;
	
	EmitterList						m_Emitters;
	U32								m_nEmitters;
	U32								m_nVertexNum;				// primitives to render
	U32								m_nCurrentBuffer;
	U32								m_nTrackIndex;
	U32								m_nElements;

	Display::C_VertexShader			m_VertexShader;
	Display::C_PixelShader			m_PixelShaderAlpha;
	Display::C_PixelShader			m_PixelShaderMultiply;
	Display::C_PixelShaderParameter	m_LuminanceParam;
	Display::C_SamplerState			m_SamplerState;

	F32								m_fAlphaScale;
};

INLINE void C_TrackRenderer::SetAlphaScale(const F32 fAlphaScale)
{
	m_fAlphaScale = fAlphaScale;
}

struct MaterialSettings
{
	Display::C_RefTexture*	pDiffuse;
	Display::C_RefTexture*	pNormal;
	Display::E_BlendMode	blendMode;
};

class C_Material
{
public:
	C_Material(C_TrackRenderer* pTrackRenderer, const MaterialSettings& settings);
	~C_Material();

	U32 GetIndex() const { return m_iIndex; }

	friend class C_TrackRenderer;

private:
	C_TrackRenderer*				m_pTrackRenderer;
	MaterialSettings				m_Settings;
	U32								m_iIndex;
};

} // Track
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_TRACK_EMITTER_H


