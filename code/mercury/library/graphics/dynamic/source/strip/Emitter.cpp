#include "stdafx.h"

#include "Dynamic/Strip/Emitter.h"
#include "Dynamic/Strip/EffectInterface.h"
#include "Dynamic/Strip/Manager.h"

using namespace DynamicEffect;
using namespace DynamicEffect::Strip;

//lint -e613

namespace DynamicEffect
{
	extern Debug::Logger& g_Log;

}

Maths::HF32		C_Emitter::m_fInterpolate(0.0f);
Maths::HF32		C_Emitter::m_fInterpolateDelta(0.0f);
Maths::HVec3	C_Emitter::m_vStart;
Maths::HVec3	C_Emitter::m_vEnd;

C_Emitter::C_Emitter(C_Manager* pRenderer)
{
	m_pDefinition = NULL;
	m_pManager = pRenderer;
	m_pManager->AddEmitter(this);

	m_iSalt = 0;
	m_pParentTransformEA = NULL;
	m_pParentTransformLS = NULL;
}


C_Emitter::~C_Emitter()
{
	assert(m_pManager);
	m_pManager->RemoveEmitter(this);
}

void C_Emitter::Free()
{
	m_pManager->FreeEmitter(this);
}

void C_Emitter::Reset()
{
	m_Transform = C_Node();
	m_Target = C_Node();
	m_pDefinition = NULL;
	m_pBlockEA = NULL;
	m_pParentTransformEA = NULL;
	m_pParentTransformLS = NULL;

	m_fTexCoord = 0.0f;
}

void C_Emitter::Update(const F32 fFrameTime, const F32 fFrameRate)
{
	Maths::HVec3 vPosition(Maths::Load<Maths::HVec3>(&m_Transform.GetPosition()));
	if(m_pParentTransformEA)
	{
		Maths::C_QuatTrans transform(*GetParentTransform());
		vPosition = vPosition * transform.Matrix();
	}
	Maths::Store(vPosition, &m_vPosition);
	
	m_nSegments = 0;

	if(m_pDefinition->eType == E_EmitterType_Trail)
	{
		// segments are emitted as a trail as this emitter moves around
		Maths::HVec3 vPositionPrevious(Maths::Load<Maths::HVec3>(&m_vPositionPreviousEmit));
		Maths::HF32 fSegmentLength = m_pDefinition->fSegmentLength;
		Maths::HF32 fSegmentLengthSq = fSegmentLength * fSegmentLength;

		Maths::HVec3 vDifference = vPosition - vPositionPrevious;
		Maths::HF32 fLengthSq = Maths::Length2(vDifference);

		if(fLengthSq > fSegmentLengthSq)
		{
			m_vStart = vPositionPrevious;
			m_vEnd = vPosition;

			m_nSegments = (U32)(fLengthSq.Sqrt()/fSegmentLength);
			m_fInterpolateDelta = 1.0f/(F32)m_nSegments;
			m_fInterpolate = 0.0f;

			m_pBlockEA = m_pDefinition->pEffect->Spawn(this);

			// store previous emission position
			Maths::Store(vPosition, &m_vPositionPreviousEmit);
		}
	}
	else
	{
		// segments are emitted between 2 fixed positions
		m_vStart = Maths::Load<Maths::HVec3>(&m_Transform.GetPosition());
		m_vEnd = Maths::Load<Maths::HVec3>(&m_Target.GetPosition());
		
		Maths::HF32 fSegmentLength(m_pDefinition->fSegmentLength);
		Maths::HF32 fStripLength = Maths::Length(m_vEnd - m_vStart);

		if(fStripLength > fSegmentLength)
		{
			m_nSegments = (U32)(fStripLength / fSegmentLength);
			m_fInterpolateDelta = 1.0f/(F32)m_nSegments;
			m_fInterpolate = 0.0f;

			m_pBlockEA = m_pDefinition->pEffect->Spawn(this);
		}

		Free();
	}
}

Maths::HVec3 C_Emitter::GetSpawnPosition() const
{
	Maths::HVec3 vPosition = Maths::Lerp(m_vStart, m_vEnd, m_fInterpolate);
	m_fInterpolate += m_fInterpolateDelta;

	return vPosition;
}

F32 C_Emitter::GetSpawnTexCoord() const
{
	F32 fTexCoord = m_fTexCoord;
	m_fTexCoord += m_pDefinition->fTextureRepeatDelta;

	return fTexCoord;
}


// Emitter Definition

void EmitterDefinition::Validate()
{
	fTextureRepeatDelta = fSegmentLength / fTextureRepeatLength;
}


