#include "stdafx.h"
#include "Graphics/Wind.h"

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

#include <Dynamic/Particle/Manager.h>

Debug::Bool		g_debugWindRender							("Amax.Wind.DebugRender",					false);
Debug::Float	g_debugWindResponse							("Amax.Wind.Response",						0.4f,	Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugWindResponseLimit					("Amax.Wind.ResponseLimit",					0.01f,	Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugWindMaxBendAngle						("Amax.Wind.MaxBendAngle",					30.0f,	Debug::Range(0.0f, 90.0f));
Debug::Float	g_debugWindBranchExponent					("Amax.Wind.BranchExponent",				2.0f,	Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugWindLeafExponent						("Amax.Wind.LeafExponent",					2.0f,	Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugWindGustFrequency					("Amax.Wind.GustFrequency",					4.0f,	Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugWindGustDurationMin					("Amax.Wind.GustDurationMin",				2.0f,	Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugWindGustDurationMax					("Amax.Wind.GustDurationMax",				5.0f,	Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugWindBranchHorizontalLowWindAngle		("Amax.Wind.BranchHorizontalLowWindAngle",	0.0f,	Debug::Range(0.0f, 90.0f));
Debug::Float	g_debugWindBranchHorizontalHighWindAngle	("Amax.Wind.BranchHorizontalHighWindAngle",	22.5f,	Debug::Range(0.0f, 90.0f));
Debug::Float	g_debugWindBranchHorizontalLowWindSpeed		("Amax.Wind.BranchHorizontalLowWindSpeed",	0.0f,	Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugWindBranchHorizontalHighWindSpeed	("Amax.Wind.BranchHorizontalHighWindSpeed",	2.0f,	Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugWindBranchVerticalLowWindAngle		("Amax.Wind.BranchVerticalLowWindAngle",	22.5f,	Debug::Range(0.0f, 90.0f));
Debug::Float	g_debugWindBranchVerticalHighWindAngle		("Amax.Wind.BranchVerticalHighWindAngle",	8.0f,	Debug::Range(0.0f, 90.0f));
Debug::Float	g_debugWindBranchVerticalLowWindSpeed		("Amax.Wind.BranchVerticalLowWindSpeed",	0.0f,	Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugWindBranchVerticalHighWindSpeed		("Amax.Wind.BranchVerticalHighWindSpeed",	2.0f,	Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugWindLeafRockingLowWindAngle			("Amax.Wind.LeafRockingLowWindAngle",		5.0f,	Debug::Range(0.0f, 90.0f));
Debug::Float	g_debugWindLeafRockingHighWindAngle			("Amax.Wind.LeafRockingHighWindAngle",		84.0f,	Debug::Range(0.0f, 90.0f));
Debug::Float	g_debugWindLeafRockingLowWindSpeed			("Amax.Wind.LeafRockingLowWindSpeed",		0.0f,	Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugWindLeafRockingHighWindSpeed			("Amax.Wind.LeafRockingHighWindSpeed",		2.0f,	Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugWindLeafRustlingLowWindAngle			("Amax.Wind.LeafRustlingLowWindAngle",		8.0f,	Debug::Range(0.0f, 90.0f));
Debug::Float	g_debugWindLeafRustlingHighWindAngle		("Amax.Wind.LeafRustlingHighWindAngle",		88.0f,	Debug::Range(0.0f, 90.0f));
Debug::Float	g_debugWindLeafRustlingLowWindSpeed			("Amax.Wind.LeafRustlingLowWindSpeed",		0.0f,	Debug::Range(0.0f, 8.0f));
Debug::Float	g_debugWindLeafRustlingHighWindSpeed		("Amax.Wind.LeafRustlingHighWindSpeed",		3.0f,	Debug::Range(0.0f, 8.0f));

XT_TYPE(Game::WindSettings, "WindSettings")
	XT_FIELD(vWindDirection,	"windDirection")
	XT_FIELD(fWindStrength,		"windStrength")
	XT_FIELD(fGustStrength,		"gustStrength")
XT_END_TYPE

namespace Game
{

WindDefinition::WindController::WindController() :
	m_fWantedValue(0.0f),
	m_fCurrentValue(0.0f),
	m_fLastDelta(0.0f),
	m_fLastDeltaTime(0.0f)
{
}

void WindDefinition::WindController::Advance(F32 fDeltaTime, F32 fP, F32 fMaxA)
{
	const F32 c_fMaxControllableDeltaTime = 0.03f;

	if (fDeltaTime > c_fMaxControllableDeltaTime)
		fDeltaTime = c_fMaxControllableDeltaTime;

	F32 fDelta = fP * fDeltaTime * (m_fWantedValue - m_fCurrentValue);

	if (fMaxA != 0.0f)
	{
		if (fabs(fDelta) > fabs(m_fLastDelta) && fDeltaTime > 0.0f && m_fLastDeltaTime > 0.0f)
		{
			F32 fAccel = fDelta / fDeltaTime - m_fLastDelta / m_fLastDeltaTime;
			if (fAccel < -fMaxA)
				fAccel = -fMaxA;
			if (fAccel > fMaxA)
				fAccel = fMaxA;

			fDelta = m_fLastDelta + fAccel * fDeltaTime;
		}

		m_fLastDelta = fDelta;
		m_fLastDeltaTime = fDeltaTime;
	}

	m_fCurrentValue += fDelta;
}

void WindDefinition::WindController::Reset(F32 fValue)
{
	m_fCurrentValue = fValue;
	m_fWantedValue = fValue;
	m_fLastDelta = 0.0f;
	m_fLastDeltaTime = 0.0f;
}

WindDefinition::OscillationParams::OscillationParams(F32 fLowWindAngle, F32 fHighWindAngle, F32 fLowWindSpeed, F32 fHighWindSpeed) :
	m_fLowWindAngle(fLowWindAngle),
	m_fHighWindAngle(fHighWindAngle),
	m_fLowWindSpeed(fLowWindSpeed),
	m_fHighWindSpeed(fHighWindSpeed),
	m_fAdjustedTime(0.0f)
{
}

void WindDefinition::OscillationParams::Advance(F32 fDeltaTime, F32 fWindStrength)
{
	F32 fAngle = Maths::Lerp(m_fLowWindAngle, m_fHighWindAngle, fWindStrength);
	F32 fSpeed = Maths::Lerp(m_fLowWindSpeed, m_fHighWindSpeed, fWindStrength);

	m_fAdjustedTime += fDeltaTime * fSpeed;

	U32 uiSize = (U32) m_vOutputs.Size();
	for (U32 i = 0; i < uiSize; ++i)
		m_vOutputs[i] = fAngle * sinf(m_fAdjustedTime + F32(i));
}

WindDefinition::WindDefinition() :
	m_fMaxBendAngle(0.0f),
	m_fWindResponse(1.0f),
	m_fWindResponseLimit(0.0f),
	m_fGustingStrengthMin(0.0f),
	m_fGustingStrengthMax(0.0f),
	m_fGustingFrequency(0.0f),
	m_fGustingDurationMin(0.0f),
	m_fGustingDurationMax(0.0f),
	m_fBranchExponent(1.0f),
	m_fLeafExponent(1.0f),
	m_fLastTime(0.0f),
	m_fWindStrength(0.0f),
	m_fCurrentGustingStrength(0.0f),
	m_fGustStopTime(0.0f),
	m_fFinalBendAngle(0.0f)
{
	SetQuantities(4, 4);
}

WindDefinition::~WindDefinition( )
{
}

void WindDefinition::SetQuantities(S32 iNumWindMatrices, S32 iNumLeafAngles)
{
	if (iNumWindMatrices < 1)
		iNumWindMatrices = 1;

	if (iNumLeafAngles < 1)
		iNumLeafAngles = 1;

	m_vWindMatrices.Clear();
	m_vWindMatrices.Resize(iNumWindMatrices);

	m_sBranchHorizontal.m_vOutputs.Clear();
	m_sBranchVertical.m_vOutputs.Clear();
	m_sBranchHorizontal.m_vOutputs.Resize(iNumWindMatrices);
	m_sBranchVertical.m_vOutputs.Resize(iNumWindMatrices);

	m_sLeafRocking.m_vOutputs.Clear();
	m_sLeafRustling.m_vOutputs.Clear();
	m_vLeafAngleMatrices.Clear();
	m_sLeafRocking.m_vOutputs.Resize(iNumLeafAngles);
	m_sLeafRustling.m_vOutputs.Resize(iNumLeafAngles);
	m_vLeafAngleMatrices.Resize(iNumLeafAngles);
}

void WindDefinition::SetWindResponse(F32 fResponse, F32 fReponseLimit)
{
	m_fWindResponse = fResponse;
	m_fWindResponseLimit = fReponseLimit;
}

void WindDefinition::SetWindStrengthAndDirection(F32 fWindStrength, F32 fWindDirectionX, F32 fWindDirectionY, F32 fWindDirectionZ)
{
	m_fWindStrength = Maths::Clamp01(fWindStrength);

	F32 fSum = fWindDirectionX * fWindDirectionX + fWindDirectionY * fWindDirectionY + fWindDirectionZ * fWindDirectionZ;
	if (fSum != 0.0f)
	{
		fSum = sqrt(fSum);
		fWindDirectionX /= fSum;
		fWindDirectionY /= fSum;
		fWindDirectionZ /= fSum;
	}

	m_cFinalWindDirectionX.m_fWantedValue = fWindDirectionX;
	m_cFinalWindDirectionY.m_fWantedValue = fWindDirectionY;
	m_cFinalWindDirectionZ.m_fWantedValue = fWindDirectionZ;
}

void WindDefinition::SetMaxBendAngle(F32 fMaxBendAngle)
{
	m_fMaxBendAngle = fMaxBendAngle;
}

void WindDefinition::SetExponents(F32 fBranchExponent, F32 fLeafExponent)
{
	m_fBranchExponent = fBranchExponent;
	m_fLeafExponent = fLeafExponent;
}

void WindDefinition::SetGusting(F32 fGustStrengthMin, F32 fGustStrengthMax, F32 fGustFrequency, F32 fGustDurationMin, F32 fGustDurationMax)
{
	m_fGustingStrengthMin = fGustStrengthMin;
	m_fGustingStrengthMax = fGustStrengthMax;
	m_fGustingFrequency = fGustFrequency;
	m_fGustingDurationMin = fGustDurationMin;
	m_fGustingDurationMax = fGustDurationMax;
}

void WindDefinition::SetBranchHorizontal(F32 fLowWindAngle, F32 fHighWindAngle, F32 fLowWindSpeed, F32 fHighWindSpeed)
{
	m_sBranchHorizontal.m_fLowWindAngle = fLowWindAngle;
	m_sBranchHorizontal.m_fHighWindAngle = fHighWindAngle;
	m_sBranchHorizontal.m_fLowWindSpeed = fLowWindSpeed;
	m_sBranchHorizontal.m_fHighWindSpeed = fHighWindSpeed;
}

void WindDefinition::SetBranchVertical(F32 fLowWindAngle, F32 fHighWindAngle, F32 fLowWindSpeed, F32 fHighWindSpeed)
{
	m_sBranchVertical.m_fLowWindAngle = fLowWindAngle;
	m_sBranchVertical.m_fHighWindAngle = fHighWindAngle;
	m_sBranchVertical.m_fLowWindSpeed = fLowWindSpeed;
	m_sBranchVertical.m_fHighWindSpeed = fHighWindSpeed;
}

void WindDefinition::SetLeafRocking(F32 fLowWindAngle, F32 fHighWindAngle, F32 fLowWindSpeed, F32 fHighWindSpeed)
{
	m_sLeafRocking.m_fLowWindAngle = fLowWindAngle;
	m_sLeafRocking.m_fHighWindAngle = fHighWindAngle;
	m_sLeafRocking.m_fLowWindSpeed = fLowWindSpeed;
	m_sLeafRocking.m_fHighWindSpeed = fHighWindSpeed;
}

void WindDefinition::SetLeafRustling(F32 fLowWindAngle, F32 fHighWindAngle, F32 fLowWindSpeed, F32 fHighWindSpeed)
{
	m_sLeafRustling.m_fLowWindAngle = fLowWindAngle;
	m_sLeafRustling.m_fHighWindAngle = fHighWindAngle;
	m_sLeafRustling.m_fLowWindSpeed = fLowWindSpeed;
	m_sLeafRustling.m_fHighWindSpeed = fHighWindSpeed;
}

F32 WindDefinition::GetTime() const
{
	return m_fLastTime;
}

void WindDefinition::Advance(F32 fCurrentTime, bool bUpdateMatrices, bool bUpdateLeafAngleMatrices)
{
	F32 fDeltaTime = fCurrentTime - m_fLastTime;
	m_fLastTime = fCurrentTime;

	// stop current gust if needed
	if (fCurrentTime > m_fGustStopTime)
	{
		m_fCurrentGustingStrength = 0.0f;

		// make new gusts
		if (m_fGustingFrequency * fDeltaTime > F32(rand()) / F32(RAND_MAX))
		{
			m_fCurrentGustingStrength = F32(rand()) * (m_fGustingStrengthMax - m_fGustingStrengthMin) / F32(RAND_MAX) + m_fGustingStrengthMin;
			m_fGustStopTime = fCurrentTime + F32(rand()) * (m_fGustingDurationMax - m_fGustingDurationMin) / F32(RAND_MAX) + m_fGustingDurationMin;
		}
	}

	// compute the actual wind strength due to gusting
	m_cFinalWindStrength.m_fWantedValue = m_fWindStrength + m_fCurrentGustingStrength;
	m_cFinalWindStrength.m_fWantedValue = Maths::Clamp01(m_cFinalWindStrength.m_fWantedValue);
	m_cFinalWindStrength.Advance(fDeltaTime, m_fWindResponse, m_fWindResponseLimit);
	m_cFinalWindStrength.m_fCurrentValue = Maths::Clamp01(m_cFinalWindStrength.m_fCurrentValue);

	// smooth out the direction
	m_cFinalWindDirectionX.Advance(fDeltaTime, m_fWindResponse, m_fWindResponseLimit);
	m_cFinalWindDirectionY.Advance(fDeltaTime, m_fWindResponse, m_fWindResponseLimit);
	m_cFinalWindDirectionZ.Advance(fDeltaTime, m_fWindResponse, m_fWindResponseLimit);

	// compute the branch and leaf wind strengths due to exponents
	F32 fBranchStrength = powf(m_cFinalWindStrength.m_fCurrentValue, m_fBranchExponent);
	F32 fLeafStrength = powf(m_cFinalWindStrength.m_fCurrentValue, m_fLeafExponent);

	// update the main tree bend angle (and compensate for unnormalized, over the pole movement)
	m_fFinalBendAngle = fBranchStrength * m_fMaxBendAngle;

	// update the oscillating parts
	m_sBranchHorizontal.Advance(fDeltaTime, fBranchStrength);
	m_sBranchVertical.Advance(fDeltaTime, fBranchStrength);
	m_sLeafRocking.Advance(fDeltaTime, fLeafStrength);
	m_sLeafRustling.Advance(fDeltaTime, fLeafStrength);

	Maths::HVec3 vecTemp(m_cFinalWindDirectionX.m_fCurrentValue, m_cFinalWindDirectionY.m_fCurrentValue, m_cFinalWindDirectionZ.m_fCurrentValue);
	vecTemp.Normalise();

	F32 fOverThePoleScalar = 1.0f - fabs(vecTemp.Y());

	if (bUpdateMatrices)
	{
		// build wind matrices

		// vectors
		F32 fWindAngle = static_cast<F32>(atan2(m_cFinalWindDirectionX.m_fCurrentValue, m_cFinalWindDirectionZ.m_fCurrentValue)) - Maths::PiBy2;

		F32 fXRot = -cosf(fWindAngle);
		F32 fYRot = -sinf(fWindAngle);

		U32 uiSize = (U32)m_vWindMatrices.Size();
		for (U32 i = 0; i < uiSize; ++i)
		{
			Maths::Mat33 mRotate0 = Maths::MakeMat33_AxisAngle(Maths::MakeVector3(fYRot, 0.0f, fXRot), Maths::MakeVF32(fOverThePoleScalar * (m_fFinalBendAngle + m_sBranchVertical.m_vOutputs[i])));
			Maths::Mat33 mRotate1 = Maths::MakeMat33_AxisAngle(Maths::MakeVector3(-fXRot, 0.0f, fYRot), Maths::MakeVF32(fOverThePoleScalar * m_sBranchHorizontal.m_vOutputs[i]));
			Maths::Mat33 mRotate = mRotate0 * mRotate1;
		
			// final matrix
			Maths::Store(Maths::MakeMat44(mRotate), &m_vWindMatrices[i]);
		}
	}

	if (bUpdateLeafAngleMatrices)
	{
		U32 uiSize = (U32) m_vLeafAngleMatrices.Size();
		for (U32 i = 0; i < uiSize; ++i)
		{
			Maths::Mat33 mRotate = Maths::Const<Maths::Mat33>::Identity();
			mRotate = mRotate * Maths::MakeMat33_RotY(Maths::MakeVF32(-m_sLeafRustling.m_vOutputs[i]));
			mRotate = mRotate * Maths::MakeMat33_RotX(Maths::MakeVF32(m_sLeafRocking.m_vOutputs[i]));

			Maths::Store(Maths::MakeMat44(mRotate), &m_vLeafAngleMatrices[i]);
		}
	}
}

F32 WindDefinition::GetFinalStrength()
{
	return m_cFinalWindStrength.m_fCurrentValue;
}

U32 WindDefinition::GetNumWindMatrices() const
{
	return (U32) m_vWindMatrices.Size();
}

const F32* WindDefinition::GetWindMatrix(U32 uiIndex) const
{
	F32* pReturn = NULL;

	if (uiIndex < (U32)m_vWindMatrices.Size())
		pReturn = ((F32*)&m_vWindMatrices[uiIndex]);

	return pReturn;
}

U32 WindDefinition::GetNumLeafAngleMatrices() const
{
	return (U32) m_vLeafAngleMatrices.Size();
}

const F32* WindDefinition::GetLeafAngleMatrix(U32 uiIndex) const
{
	F32* pReturn = NULL;

	if (uiIndex < (U32)m_vWindMatrices.Size())
		pReturn = ((F32*)&m_vLeafAngleMatrices[uiIndex]);

	return pReturn;
}

void WindDefinition::Reset()
{
	U32 i = 0;
	U32 uiSize = (U32) m_vWindMatrices.Size();
	for (i = 0; i < uiSize; ++i)
	{
		m_vWindMatrices[i]._11 = 1.0f;
		m_vWindMatrices[i]._12 = 0.0f;
		m_vWindMatrices[i]._13 = 0.0f;
		m_vWindMatrices[i]._14 = 0.0f;
		m_vWindMatrices[i]._21 = 0.0f;
		m_vWindMatrices[i]._22 = 1.0f;
		m_vWindMatrices[i]._23 = 0.0f;
		m_vWindMatrices[i]._24 = 0.0f;
		m_vWindMatrices[i]._31 = 0.0f;
		m_vWindMatrices[i]._32 = 0.0f;
		m_vWindMatrices[i]._33 = 1.0f;
		m_vWindMatrices[i]._34 = 0.0f;
		m_vWindMatrices[i]._41 = 0.0f;
		m_vWindMatrices[i]._42 = 0.0f;
		m_vWindMatrices[i]._43 = 0.0f;
		m_vWindMatrices[i]._44 = 1.0f;
	}

	uiSize = (U32) m_vLeafAngleMatrices.Size();
	for (i = 0; i < uiSize; ++i)
	{
		m_vLeafAngleMatrices[i]._11 = 1.0f;
		m_vLeafAngleMatrices[i]._12 = 0.0f;
		m_vLeafAngleMatrices[i]._13 = 0.0f;
		m_vLeafAngleMatrices[i]._14 = 0.0f;
		m_vLeafAngleMatrices[i]._21 = 0.0f;
		m_vLeafAngleMatrices[i]._22 = 1.0f;
		m_vLeafAngleMatrices[i]._23 = 0.0f;
		m_vLeafAngleMatrices[i]._24 = 0.0f;
		m_vLeafAngleMatrices[i]._31 = 0.0f;
		m_vLeafAngleMatrices[i]._32 = 0.0f;
		m_vLeafAngleMatrices[i]._33 = 1.0f;
		m_vLeafAngleMatrices[i]._34 = 0.0f;
		m_vLeafAngleMatrices[i]._41 = 0.0f;
		m_vLeafAngleMatrices[i]._42 = 0.0f;
		m_vLeafAngleMatrices[i]._43 = 0.0f;
		m_vLeafAngleMatrices[i]._44 = 1.0f;
	}

	m_cFinalWindStrength.Reset(0.0f);
	m_cFinalWindDirectionX.Reset(0.0f);
	m_cFinalWindDirectionY.Reset(0.0f);
	m_cFinalWindDirectionZ.Reset(0.0f);

	m_fCurrentGustingStrength = 0.0f;
	m_fGustStopTime = 0.0f;
	m_fLastTime = 0.0f;

	m_sBranchHorizontal.Reset();
	m_sBranchVertical.Reset();
	m_sLeafRocking.Reset();
	m_sLeafRustling.Reset();
}

void WindDefinition::GetQuantities(S32& iNumWindMatrices, S32& iNumLeafAngles) const
{
	iNumWindMatrices = (S32)m_vWindMatrices.Size();
	iNumLeafAngles = (S32)m_vLeafAngleMatrices.Size();
}

void WindDefinition::GetWindResponse(F32& fResponse, F32& fReponseLimit) const
{
	fResponse = m_fWindResponse;
	fReponseLimit = m_fWindResponseLimit;
}

void WindDefinition::GetWindStrengthAndDirection(F32& fWindStrength, F32& fWindDirectionX, F32& fWindDirectionY, F32& fWindDirectionZ) const
{
	fWindStrength = m_fWindStrength;
	fWindDirectionX = m_cFinalWindDirectionX.m_fWantedValue;
	fWindDirectionY = m_cFinalWindDirectionY.m_fWantedValue;
	fWindDirectionZ = m_cFinalWindDirectionZ.m_fWantedValue;
}

F32 WindDefinition::GetMaxBendAngle() const
{
	return m_fMaxBendAngle;
}

void WindDefinition::GetExponents(F32& fBranchExponent, F32& fLeafExponent) const
{
	fBranchExponent = m_fBranchExponent;
	fLeafExponent = m_fLeafExponent;
}

void WindDefinition::GetGusting(F32& fGustStrengthMin, F32& fGustStrengthMax, F32& fGustFrequency, F32& fGustDurationMin, F32& fGustDurationMax) const
{
	fGustStrengthMin = m_fGustingStrengthMin;
	fGustStrengthMax = m_fGustingStrengthMax;
	fGustFrequency = m_fGustingFrequency;
	fGustDurationMin = m_fGustingDurationMin;
	fGustDurationMax = m_fGustingDurationMax;
}

void WindDefinition::GetBranchHorizontal(F32& fLowWindAngle, F32& fHighWindAngle, F32& fLowWindSpeed, F32& fHighWindSpeed) const
{
	fLowWindAngle = m_sBranchHorizontal.m_fLowWindAngle;
	fHighWindAngle = m_sBranchHorizontal.m_fHighWindAngle;
	fLowWindSpeed = m_sBranchHorizontal.m_fLowWindSpeed;
	fHighWindSpeed = m_sBranchHorizontal.m_fHighWindSpeed;
}

void WindDefinition::GetBranchVertical(F32& fLowWindAngle, F32& fHighWindAngle, F32& fLowWindSpeed, F32& fHighWindSpeed) const
{
	fLowWindAngle = m_sBranchVertical.m_fLowWindAngle;
	fHighWindAngle = m_sBranchVertical.m_fHighWindAngle;
	fLowWindSpeed = m_sBranchVertical.m_fLowWindSpeed;
	fHighWindSpeed = m_sBranchVertical.m_fHighWindSpeed;
}

void WindDefinition::GetLeafRocking(F32& fLowWindAngle, F32& fHighWindAngle, F32& fLowWindSpeed, F32& fHighWindSpeed) const
{
	fLowWindAngle = m_sLeafRocking.m_fLowWindAngle;
	fHighWindAngle = m_sLeafRocking.m_fHighWindAngle;
	fLowWindSpeed = m_sLeafRocking.m_fLowWindSpeed;
	fHighWindSpeed = m_sLeafRocking.m_fHighWindSpeed;
}

void WindDefinition::GetLeafRustling(F32& fLowWindAngle, F32& fHighWindAngle, F32& fLowWindSpeed, F32& fHighWindSpeed) const
{
	fLowWindAngle = m_sLeafRustling.m_fLowWindAngle;
	fHighWindAngle = m_sLeafRustling.m_fHighWindAngle;
	fLowWindSpeed = m_sLeafRustling.m_fLowWindSpeed;
	fHighWindSpeed = m_sLeafRustling.m_fHighWindSpeed;
}

bool WindDefinition::GetRustleAngles(F32* pAngles) const
{
	bool bSuccess = false;

	S32 nNumLeaves = S32(m_sLeafRustling.m_vOutputs.Size());

	BIZ_ASSERT(pAngles);
	memcpy(pAngles, &m_sLeafRustling.m_vOutputs[0], nNumLeaves * sizeof(F32));
	bSuccess = true;

	return bSuccess;
}

bool WindDefinition::GetRockAngles(F32* pAngles) const
{
	bool bSuccess = false;

	S32 nNumLeaves = S32(m_sLeafRocking.m_vOutputs.Size());

	BIZ_ASSERT(pAngles);
	memcpy(pAngles, &m_sLeafRocking.m_vOutputs[0], nNumLeaves * sizeof(F32));
	bSuccess = true;

	return bSuccess;
}

WindEnvSettings::WindEnvSettings(WindSettings* pData, C_Wind* pWind)
	: m_fWindDirectionX(pData->vWindDirection.vx)
	, m_fWindDirectionY(pData->vWindDirection.vy)
	, m_fWindDirectionZ(pData->vWindDirection.vz)
	, m_fWindStrength(pData->fWindStrength)
	, m_fGustStrength(pData->fGustStrength)
	, m_pWind(pWind)
{
}

/// construct by lerping two other PostProcessEnvSettings
WindEnvSettings::WindEnvSettings(const WindEnvSettings *pA, const WindEnvSettings *pB, float f)
	: m_fWindDirectionX(pA->m_fWindDirectionX, pB->m_fWindDirectionX, f)
	, m_fWindDirectionY(pA->m_fWindDirectionY, pB->m_fWindDirectionY, f)
	, m_fWindDirectionZ(pA->m_fWindDirectionZ, pB->m_fWindDirectionZ, f)
	, m_fWindStrength(pA->m_fWindStrength, pB->m_fWindStrength, f)
	, m_fGustStrength(pA->m_fGustStrength, pB->m_fGustStrength, f)
	, m_pWind(pA->m_pWind)
{
}

void WindEnvSettings::Set()
{
	if(!m_pWind)
		return;

	m_pWind->SetWindDirection(m_fWindDirectionX, m_fWindDirectionY, m_fWindDirectionZ);
	m_pWind->SetWindStrength(m_fWindStrength);
	m_pWind->SetGustStrength(m_fGustStrength);
}

FX::I_EnvironmentSettings* WindEnvSettings::Lerp(const FX::I_EnvironmentSettings* p, float f) const
{
	assert(Name() == p->Name());
	return new WindEnvSettings(this, (WindEnvSettings*)p, f);
}

void WindEnvSettings::AssociateDebugVariables(const Util::C_String& sName)
{
	m_fWindDirectionX.AssociateDebugVariables(sName + ".Wind.DirectionX");
	m_fWindDirectionY.AssociateDebugVariables(sName + ".Wind.DirectionY");
	m_fWindDirectionZ.AssociateDebugVariables(sName + ".Wind.DirectionZ");
	m_fWindStrength.AssociateDebugVariables(sName + ".Wind.WindStrength");
	m_fGustStrength.AssociateDebugVariables(sName + ".Wind.GustStrength");
}

C_Wind::C_Wind(DynamicEffect::Particle::C_ParticleManager* pParticleManager)
	: m_pParticleManager(pParticleManager)
	, m_fWindDirectionX(1.0f)
	, m_fWindDirectionY(0.0f)
	, m_fWindDirectionZ(0.0f)
	, m_fWindStrength(0.0f)
	, m_fGustStrength(0.0f)
{
	memset(m_WindArray, 0, sizeof(F32) * 64);
	memset(m_LeafArray, 0, sizeof(F32) * 64);
}

C_Wind::~C_Wind()
{
}

void C_Wind::SetWindDirection(const F32 fWindDirectionX, const F32 fWindDirectionY, const F32 fWindDirectionZ)
{
	m_fWindDirectionX = fWindDirectionX;
	m_fWindDirectionY = fWindDirectionY;
	m_fWindDirectionZ = fWindDirectionZ;
}

void C_Wind::SetWindStrength(const F32 fWindStrength)
{
	m_fWindStrength = fWindStrength;
}

void C_Wind::SetGustStrength(const F32 fGustStrength)
{
	m_fGustStrength = fGustStrength;;
}

void C_Wind::Update(const F32 fTime, const F32 fTimeStep)
{
	F32 fWindDirectionLength = sqrtf(m_fWindDirectionX*m_fWindDirectionX + m_fWindDirectionY*m_fWindDirectionY + m_fWindDirectionZ*m_fWindDirectionZ);
	if(fWindDirectionLength < 0.001f)
		fWindDirectionLength = 0.001f;
	m_WindDef.SetWindResponse(g_debugWindResponse, g_debugWindResponseLimit);
	m_WindDef.SetWindStrengthAndDirection(m_fWindStrength, m_fWindDirectionX / fWindDirectionLength, m_fWindDirectionY / fWindDirectionLength, m_fWindDirectionZ / fWindDirectionLength);
	m_WindDef.SetMaxBendAngle(g_debugWindMaxBendAngle * Maths::Pi / 180.0f);
	m_WindDef.SetExponents(g_debugWindBranchExponent, g_debugWindLeafExponent);
	m_WindDef.SetGusting(m_fWindStrength, m_fGustStrength, g_debugWindGustFrequency, g_debugWindGustDurationMin, g_debugWindGustDurationMax);
	m_WindDef.SetBranchHorizontal(g_debugWindBranchHorizontalLowWindAngle * Maths::Pi / 180.0f, g_debugWindBranchHorizontalHighWindAngle * Maths::Pi / 180.0f, g_debugWindBranchHorizontalLowWindSpeed, g_debugWindBranchHorizontalHighWindSpeed);
	m_WindDef.SetBranchVertical(g_debugWindBranchVerticalLowWindAngle * Maths::Pi / 180.0f, g_debugWindBranchVerticalHighWindAngle * Maths::Pi / 180.0f, g_debugWindBranchVerticalLowWindSpeed, g_debugWindBranchVerticalHighWindSpeed);
	m_WindDef.SetLeafRocking(g_debugWindLeafRockingLowWindAngle * Maths::Pi / 180.0f, g_debugWindLeafRockingHighWindAngle * Maths::Pi / 180.0f, g_debugWindLeafRockingLowWindSpeed, g_debugWindLeafRockingHighWindSpeed);
	m_WindDef.SetLeafRustling(g_debugWindLeafRustlingLowWindAngle * Maths::Pi / 180.0f, g_debugWindLeafRustlingHighWindAngle * Maths::Pi / 180.0f, g_debugWindLeafRustlingLowWindSpeed, g_debugWindLeafRustlingHighWindSpeed);

	m_WindDef.Advance(fTime, true, true);

	memcpy(&m_WindArray[0], m_WindDef.GetWindMatrix(0), sizeof(F32) * 16);
	memcpy(&m_WindArray[16], m_WindDef.GetWindMatrix(1), sizeof(F32) * 16);
	memcpy(&m_WindArray[32], m_WindDef.GetWindMatrix(2), sizeof(F32) * 16);
	memcpy(&m_WindArray[48], m_WindDef.GetWindMatrix(3), sizeof(F32) * 16);

	memcpy(&m_LeafArray[0], m_WindDef.GetLeafAngleMatrix(0), sizeof(F32) * 16);
	memcpy(&m_LeafArray[16], m_WindDef.GetLeafAngleMatrix(1), sizeof(F32) * 16);
	memcpy(&m_LeafArray[32], m_WindDef.GetLeafAngleMatrix(2), sizeof(F32) * 16);
	memcpy(&m_LeafArray[48], m_WindDef.GetLeafAngleMatrix(3), sizeof(F32) * 16);

	m_vCurrentWind = Maths::HVec3(m_fWindDirectionX * m_WindDef.GetFinalStrength(), m_fWindDirectionY * m_WindDef.GetFinalStrength(), m_fWindDirectionZ * m_WindDef.GetFinalStrength());

	if(m_pParticleManager)
		m_pParticleManager->SetGlobalForce(m_vCurrentWind);
}

void C_Wind::DebugRender()
{
	if(g_debugWindRender)
	{
		const Display::C_View* pView = Display::Device()->GetView();
		Maths::HVec3 vPosition = pView->GetViewSettings3D().GetCameraPosition();
		const Display::C_Matrix& viewMatrix = pView->GetViewSettings3D().GetViewMatrix();
		Maths::HVec3 vForward = Maths::HVec3(viewMatrix._13, viewMatrix._23, viewMatrix._33);

		Maths::HVec3 vArrowPosition = vPosition + vForward * 10.0f;

		Render::Debug::Deferred()->Draw3dArrow(vArrowPosition, m_vCurrentWind, 1.0f, 1.0f, Maths::RGBA(255, 255, 255, 255));
	}
}

}
