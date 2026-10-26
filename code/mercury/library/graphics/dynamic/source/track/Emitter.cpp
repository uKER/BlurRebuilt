#include "stdafx.h"

#include "Dynamic/Track/Emitter.h"
#include "Dynamic/Track/Track.h"

namespace DynamicEffect
{
namespace Track
{

Debug::Float g_TrackAngle("Graphics.Effects.Track.AngleThreshold", 0.75f, Debug::Range(0.00f, 1.0f), Debug::Step(0.01f));

// Emitter Class. Is owned by objects which produce tracks, eg wheels

C_Emitter::C_Emitter(C_TrackRenderer* pTrackRenderer)
{
	m_bActive = false;
	m_iFrame = 0;

	m_pTrack = NULL;
	m_pMaterial = NULL;
	m_pTrackRenderer = pTrackRenderer;
	m_bEmitting = false;
	m_VisibilityFlags = 0xFFFFFFFF;
}

void C_Emitter::Destroy()
{
	// if theres a current track which is being terminated
	// make sure its end frame is set correctly, and it also gets made inactive
	if(m_pTrack)
	{
		m_pTrack->m_bActive = false;

		// if particle is not empty
		if(!m_pTrack->m_Particles.Empty())
		{
			if(m_pTrack->m_Particles.Size() <= 2)
				m_pTrackRenderer->EraseTrack(m_pTrack);
			else
			{
				C_Particle* pParticle = &m_pTrack->m_Particles.Front();
				pParticle->vColour.w = 0.0f;	// set last particle to zero alpha so track fades out
				// set end frame to equal final particle
				m_pTrack->m_iEndFrame = pParticle->iFrameCreated;
			}
		}
	}

	m_pTrack = NULL;
}

void C_Emitter::Free()
{
	m_pTrackRenderer->FreeEmitter(this);
}

void C_Emitter::Update()
{
	// check to see if position is reasonable	
	F32 fTime = m_pTrackRenderer->GetRenderer()->GetFrameTime();
	C_Particle* pParticle;

	Maths::HVec3 vDirectionNormalised;
	Maths::HF32 fLenSq = Maths::Length2(m_vDirection);

	if(fLenSq < 0.001f)
		m_bEmitting = false;
	else
	{
		// get normalised direction
		Maths::HF32 fLen = Maths::Sqrt(fLenSq);
		vDirectionNormalised = m_vDirection / fLen;
				
		// start emitting new particles by fading in the track
		m_bEmitting = m_bActive && m_fIntensity != Maths::Constants<Maths::HF32>::Zero();

		// if the angle between current and previous tracks is too great then stop emitting
		// this should stop weirdness with the strips where 2 consecutive particles were at extreme angles
		if((vDirectionNormalised | m_vLastDirection) < g_TrackAngle && m_pTrack)
			m_bEmitting = false;
		else
		{
			m_vPosition1 = m_vPosition + vDirectionNormalised * m_fWidth;
			m_vPosition2 = m_vPosition - vDirectionNormalised * m_fWidth;
		}
	}

	// if we are emitting, increase the alpha component unless its already 1.0
	if(m_bEmitting)
	{
		bool resetTrack;
		// if track is bigger than a certain size then finish track and start again
		/*if(m_pTrack && m_pTrack->m_Particles.Size() > 32)
		{
			// move end particle to right position for this frame
			pParticle = &m_pTrack->m_Particles.Front();

			// set positions
			Maths::Store(m_vPosition1, (MathsStorage::Float3*)&pParticle->vPosition1);
			Maths::Store(m_vPosition2, (MathsStorage::Float3*)&pParticle->vPosition2);

			m_pTrack = NULL;
			resetTrack = false;
		}
		else*/
			resetTrack = true;

		// if there isnt a current track then create a new one
		if(!m_pTrack && m_fIntensity != Maths::Constants<Maths::HF32>::Zero())
		{
			m_pTrack = m_pTrackRenderer->NewTrack();
			if(m_pTrack)
			{
				// set up block
				m_pTrack->m_pMaterial = m_pMaterial;
				
				// use the flags to sort the tracks by material when batching
				// if no material choose highest 16 bit value
				if(m_pMaterial)
					m_pTrack->m_iFlags = m_pMaterial->GetIndex();
				else
					m_pTrack->m_iFlags = 0x0000FFFF;

				m_pTrack->m_bActive = true;
				m_pTrack->m_pEmitter = this;
				DynamicEffect::GetLodFade(m_pTrack->m_vLodFade, 100.0f);

				//Frame in the block was created
				m_pTrack->m_iStartFrame = m_iFrame;
				m_pTrack->m_iEndFrame = m_iFrame;

				Maths::C_RGBA_F32 vColour = m_vColour * m_vLighting;

				if(resetTrack)
				{
					m_fV = 0.f;
					vColour.A(Maths::Constants<Maths::HF32>::Zero());
				}
				else
					vColour.A(vColour.A() * m_fIntensity);
				
				for(int a = 0; a < 2; a++)
				{
					if((pParticle = m_pTrackRenderer->NewParticle()))
					{
						CreateParticle(pParticle, vDirectionNormalised, vColour);
					}
				}

				if(m_pTrack)
					m_pTrack->CalculateExtents();
			}
		}
	}
	else
	{
		// otherwise terminate the current track
		if(m_pTrack)
		{
			assert(!m_pTrack->m_Particles.Empty());
			// end this track
			Destroy();
		}
	}

	
	// if the particle system should emit new particles
	if(m_pTrack)
	{
		assert(!m_pTrack->m_Particles.Empty());
		assert(m_pTrack->m_bActive);

		Maths::HVec3 vTemp = m_vPosition - m_vLastPosition;
		Maths::HF32 fLen = Maths::Length2(vTemp);
		Maths::HF32 fStepSq = m_fStep * m_fStep;
		Maths::C_RGBA_F32 vColour = m_vColour * m_vLighting;

		vColour.A(vColour.A() * m_fIntensity);

		// only emit if we are far enough away from previous emission to warrant it
		// or if the current track is empty
		if(fLen > fStepSq)
		{
			// emit new particle if possible
			pParticle = m_pTrackRenderer->NewParticle();
			if(pParticle != NULL)
			{
				m_fV += 1.0f;
				CreateParticle(pParticle, vDirectionNormalised, vColour);
			}		
		}
		else
		{
			// move the position of the most recently emitted particle to follow wheel
			// so we get smooth production of tracks without visible popping
			pParticle = &m_pTrack->m_Particles.Front();

			// set positions
			Maths::Store(m_vPosition1, (MathsStorage::Float3*)&pParticle->vPosition1);
			Maths::Store(m_vPosition2, (MathsStorage::Float3*)&pParticle->vPosition2);
		}

		// update the extents of the bounds of the track
		if(m_pTrack)
		{
			m_pTrack->UpdateExtents(pParticle);
			m_pTrack->SetVisibilityFlags(m_VisibilityFlags);
		}

		
	}
}

void C_Emitter::CreateParticle(C_Particle* pParticle, const Maths::HVec3& vDirection, const Maths::C_RGBA_F32& vColour)
{
	if(!m_pTrack)
	{
		// the new particle request actually deallocated this track, so give particle back
		m_pTrackRenderer->FreeParticle(pParticle);
	}
	else
	{
		// add it to the front of the current list
		m_pTrack->m_Particles.PushFront(*pParticle);

		// set positions and alphas
		Maths::Store(m_vPosition1, (MathsStorage::Float3*)&pParticle->vPosition1);
		Maths::Store(m_vPosition2, (MathsStorage::Float3*)&pParticle->vPosition2);

		pParticle->fV = m_fV;
		//Frame in the replay system the particle was created
		pParticle->iFrameCreated = m_iFrame;

		pParticle->fFadeAlpha = 1.0f;
		Maths::Store(vColour, &pParticle->vColour);

			// set the last emitted position
		m_vLastPosition = m_vPosition;
		m_vLastDirection = m_vDirection;
	}
}

} // Track
} // DynamicEffect



