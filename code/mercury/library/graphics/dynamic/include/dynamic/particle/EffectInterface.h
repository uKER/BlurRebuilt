#ifndef RENDER_DYNAMICEFFECT_PARTICLE_EFFECTINTERFACE_H
#define RENDER_DYNAMICEFFECT_PARTICLE_EFFECTINTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core\containers\bitvector.h>
#include "Dynamic/Particle/VertexBuildSettings.h"

#include "Dynamic/Common.h"
#include "Dynamic/Interface.h"
#include "Dynamic/Particle/Manager.h"
#include "Dynamic/Particle/Block.h"
#include "Dynamic/Particle/PhysicsSettings.h"
#include "Dynamic/Particle/SpawnSettings.h"

namespace DynamicEffect
{
class C_Renderer;

namespace Particle
{

class C_ParticleFormat;
class C_ParticleManager;
class C_Emitter;
struct EmitterDefinition;
struct UpdateData;

struct UpdateSettings					// shared settings between all types of particle systems
{
	UpdateSettings() : iFreezeAmount(0), bTrackCollisions(false), bTrackDeaths(false), iGameIndex(0) {}
	~UpdateSettings() {}

	U32			iFreezeAmount;				// number of particles which won't get updated
	// tracking
	bool		bTrackCollisions;			// track collisions for external objects to query
	bool		bTrackDeaths;				// track deaths for external object to query
	U8			iGameIndex;					// used by the application, is stored on each particle created by this effect
};

// render settings provide a superset of all rendering methods used by any effect
// as such any render settings could be used for any effect though not all parameters are necessarily used
struct RenderSettings
{
	RenderSettings() : pDiffuse(NULL), pNormalMap(NULL), bLightEmit(false), bLightReceive(false), fLightRadius(0.f), fLightIntensity(0.f), iLightPriority(0), lightTrail(false) {};

	Display::C_RefTexture*			pDiffuse;
	Display::C_RefTexture*			pNormalMap;
	E_VertexShader					vertexShader;
	E_PixelShader					pixelShader;
	E_BlendMode						blendMode;
	Display::E_PrimitiveType		primitiveType;
	Display::C_VertexDeclaration*	pVertexDeclaration;
	MathsStorage::Float4			uvScale;
	U32								vertices;
	U32								vertexSize;

	VertexBuildSettings		vertexBuildSettings;
	
	MathsStorage::Float2	lodFade;
	F32						cullDistance;

	// lights
	bool					bLightEmit;					// whether particles emit light
	bool					bLightReceive;				// whether particle receives light
	F32						fLightRadius;				// whether particles emit light
	F32						fLightIntensity;			// whether particles emit light
	U32						iLightPriority;			
	
	bool					soft;						// soft particles which use z-buffer
	bool					additive;					// some type of additive blendmode
	bool					blendModeTransparent;		// dependent on blendmode

	bool					lightTrail;
};

class I_ParticleEffectBase : public DynamicEffect::I_DynamicEffect
{
public:
	I_ParticleEffectBase(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex, const U32 iTypeIndex);
	virtual ~I_ParticleEffectBase() { Free(); };

	// virtual methods for particle effects
	virtual void				AddChild(E_EffectChild type, const EmitterDefinition& definition, const MathsStorage::Float4A16& settings) {};
	virtual void				Destroy();

	virtual void				BeginFrame();
	virtual void				OnGather(const SceneGraph::C_View& view) const;

	// implemented virtual methods from I_DynamicEffect
	virtual void				Clean(const C_Node* pNode);
	virtual void				Clean(const Display::C_Texture2D* pTexture);
	virtual void				Reset()
	{
		Free();
	}

	// standard methods
	void						Create(const UpdateSettings& updateSettings, const RenderStates& renderStates, const RenderSettings& renderSettings, const PhysicsSettings& physicsSettings);

	const PhysicsSettings&		GetPhysicsSettings() const { return m_PhysicsSettings; }
	const RenderSettings&		GetRenderSettings() const { return m_RenderSettings; }
	const UpdateSettings&		GetUpdateSettings() const { return m_UpdateSettings; }
	E_Collision					GetCollisionType() const { return m_PhysicsSettings.eCollision; }

	Scheduler::jqModule*		GetUpdateModule() { return m_pUpdateModule; }
	Scheduler::jqModule*		GetSpawnModule() { return m_pSpawnModule; }
	EffectSpawnFunctor			GetSpawnFunctor() { return m_pSpawnFunctor; }

	const C_ParticleFormat*		GetParticleFormat() const { return m_pParticleFormat; }
	S32							GetNumParticles() const;

	virtual void				SpawnCustom(C_Emitter* pEmitter, C_Random& random, SpawnSettings* pSpawnSettings, const EmitterUpdateConstants* pUpdateConstants) {};
	const C_ParticleManager*	GetManager() const { return m_pManager; }

	friend class 				C_Emitter;
	friend class				C_ParticleManager;

	SpawnSettings						m_SpawnSettings;

protected:
	
	void				Free();

	// remove block from list and return to store
	void				DeleteBlock(ParticleBlock* pBlock);

	FORCE_INLINE void	DeleteBlockDeferred(ParticleBlock* pDelete)
	{
		pDelete->m_Element.Clear();
	}

	RenderSettings						m_RenderSettings;
	UpdateSettings						m_UpdateSettings;
	PhysicsSettings						m_PhysicsSettings;

	const C_ParticleFormat*				m_pParticleFormat;
	C_ParticleManager*					m_pManager;
	S32									m_EffectPosition;
	S32									m_ActivePosition;

	Scheduler::jqModule*				m_pUpdateModule;
	Scheduler::jqModule*				m_pSpawnModule;
	EffectSpawnFunctor					m_pSpawnFunctor;

#ifndef CFG_SPEED
	S32									m_Allocated;
#endif
};

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_EFFECTINTERFACE_H
