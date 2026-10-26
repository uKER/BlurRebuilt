/////////////////////////////////////////////////////////////
// VehicleWheel
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_WHEEL_H__
#define __VEHICLE_WHEEL_H__

namespace RacingPhysics { class CarWheel; }
namespace WorldRender { class C_LightSampler; }

#include <Graphics/Effects/EffectTypes.h>
#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Particle/EventInstanceHandle.h>
#include <Dynamic/Track/Emitter.h>
#include <Misc_Shared/Curve/HermiteCurve.h>
#include "btl/utility/autopointer.h"
#include "Xt/XtGame.h"
#include <Scenegraph/light/Light.h>

namespace Vehicle
{
	class C_WheelDamage;
}

namespace SplashCamera
{
	class C_SplashCamera;
}

namespace DynamicEffect
{
	namespace Particle { class C_ParticleManager; }
	namespace Track { class C_TrackRenderer; }
}

namespace SceneGraph
{
	class C_SceneGraph;
}

struct AmaxWheelSurfaceParticleEvent;
struct AmaxWheelSurfaceTrackEvent;

namespace Game
{

namespace Effects
{

/* 
===============================================================
					class C_VehicleWheel

NOTES: 
===============================================================
*/
class C_VehicleWheel
{
public:
	
	C_VehicleWheel();
	~C_VehicleWheel();

	void Create(U32 p_Index, const RacingPhysics::CarWheel* p_pWheel,
				DynamicEffect::Particle::C_ParticleManager* p_pParticleManager,
				SceneGraph::C_SceneGraph* p_pSceneGraph,
				DynamicEffect::Track::C_TrackRenderer* p_pTrackRenderer, float p_HalfWidth);

	void Destroy();

	void Update(Maths::HMat43* p_pParentMatrix, Maths::HMat33* p_pYawMatrix, F32 p_fTimeStep, const Maths::C_RGBA_F32& p_Lighting, const Vehicle::C_WheelDamage* p_pWheelDamage, const bool p_Enable, float p_fEmitScale, float p_fLifeScale, S32 splashViewport, SplashCamera::C_SplashCamera* pSplashCamera, U32 visibilityFlags, Maths::HVec3 p_Velocity);
	void UpdateCommon(Maths::HMat43* p_pParentMatrix, Maths::HMat33* p_pYawMatrix, F32 p_fTimeStep, const Vehicle::C_WheelDamage* p_pWheelDamage, const bool p_Enable, S32 splashViewport, SplashCamera::C_SplashCamera* pSplashCamera, U32 visibilityFlags);
	void UpdateParticles(Maths::HMat43* p_pParentMatrix, F32 p_fTimeStep, const Maths::C_RGBA_F32& p_Lighting, float p_fEmitScale, float p_fLifeScale, U32 visibilityFlags, Maths::HVec3 p_Velocity);
	void UpdateTracks(const Maths::C_RGBA_F32& p_Lighting, const Vehicle::C_WheelDamage* p_pWheelDamage, U32 visibilityFlags);
	void Clear();
	void ClearDeferred();

private:

	void	ClearParticles();
	void	ClearTracks();
	void	ClearMisc();
	void	UpdateWheelDirection();

	DynamicEffect::Particle::C_ParticleManager*		m_pParticleManager;
	DynamicEffect::Track::C_TrackRenderer*			m_pTrackRenderer;
	SceneGraph::C_SceneGraph*						m_pSceneGraph;

	DynamicEffect::Particle::C_EventInstanceHandle	m_SparksEvent;

	struct SurfaceParticle
	{
		DynamicEffect::Particle::C_EventInstanceHandle	particleEvent;
		AmaxWheelSurfaceParticleEvent*					pEvent;
		F32												intensityCurrent;
		F32												alphaCurrent;
		XtHandle<AmaxWheelSurfaceParticleEvent>			event;
	};

	typedef Btl::Vector<SurfaceParticle, Btl::VectorParameters_Static<5> >	SurfaceParticles;
	SurfaceParticles							m_SurfaceParticles;

	struct SurfaceTrack
	{
		DynamicEffect::Track::C_EmitterHandle	track;
		AmaxWheelSurfaceTrackEvent*				pEvent;
		F32										alphaCurrent;
		U32										materialID;
		XtHandle<AmaxWheelSurfaceTrackEvent>	event;
	};

	SurfaceTrack								m_SurfaceTrack;

	DynamicEffect::C_Node						m_LocalSpace;
	DynamicEffect::C_Node						m_LocalSpaceSpin;	
	Maths::C_Quaternion							m_vForwardDirectionQuat;
	Maths::C_Quaternion							m_vReverseDirectionQuat;

	S32											m_CurrentSurfaceType;
	S32											m_CurrentModifier;
	F32											m_CurrentSurfaceStateTime;

	S32											m_UniqueIndex;

	F32											m_HalfWidth;

	const RacingPhysics::CarWheel*				m_pWheel;

	//Light
	SceneGraph::C_PointLightHandle				m_RimPointLightHandle;
	U32											m_RimPointLightFlickerPosition;
	F32											m_RimPointLightFlickerRest;

	bool										m_Update;
	bool										m_Refresh;
	bool										m_OnGround;
	bool										m_RimEvent;

	float										m_Evaluate[4];

	F32											m_Speed;
	F32											m_TrackWidth;
	U32											m_SurfaceModifier;
	S32											m_EffectModifierMapping;
	S32											m_TrackModifierMapping;
	S32											m_EffectMapping;
	S32											m_TrackMapping;
	U32											m_SurfaceType;
	
	Maths::HVec3								m_GroundPosition;	
	Maths::HVec3								m_Position;	
	Maths::HVec3								m_Velocity;	
	Maths::HVec3								m_Normal;

	//MathsStorage::Float3						m_GroundPosition;	
	//MathsStorage::Float3						m_Position;	
	//MathsStorage::Float3						m_Velocity;	
	//MathsStorage::Float3						m_Normal;	

	bool										m_ClearDeferred;
};

} //namespace Effects

} //namespace Game

#endif
