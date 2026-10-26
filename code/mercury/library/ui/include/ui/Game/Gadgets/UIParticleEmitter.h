#ifndef UIPARTICLEEMITTER_H
#define UIPARTICLEEMITTER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <System/Misc/Random.h>

#include "ui/Game/Gadgets/Buttons.h"
#include "ui/Graphics/UIParticle.h"

namespace UI
{
	
// The gadget
class UIParticleEmitter : public C_GadgetBase
{
public:
	~UIParticleEmitter();

	DECLARE_GADGET( E_UIButtonType_ParticleEmitter, "PARTICLE_EMITTER" )

	void Init();
	void FrameUpdate();
	void Render();

	void EmitParticle();

public:
	enum E_CycleType
	{	
		E_CycleType_Constant = 0,
		E_CycleType_Duration,
	};
	E_CycleType						m_CycleType;
	F32								m_Duration;
	bool							m_Running;

	F32								m_Time;
	F32								m_EmitRate;

	XtDispersionF32					m_AngleInc;
	XtDispersionF32					m_ScaleInc;

	// Spawn attributes
	XtDispersionF32					m_SpawnLife;
	XtDispersionF32					m_SpawnScale;
	XtDispersionF32					m_SpawnAngle;

	C_VectorBase					m_SpawnSize;
	C_VectorBase					m_SpawnMinVel;
	C_VectorBase					m_SpawnMaxVel;
	C_VectorBase					m_SpawnMinAcc;
	C_VectorBase					m_SpawnMaxAcc;

	Util::C_Random					m_Rand;

private:
	TPoolId							m_PoolId;
};

}; // namespace UI

#endif // UIPARTICLEEMITTER_H
