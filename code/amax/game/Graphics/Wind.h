#ifndef AMAX_GRAPHICS_WIND_H
#define AMAX_GRAPHICS_WIND_H

#include <FX/EnvironmentSettings.h>
#include <FX/EnvironmentFloat.h>
#include <Xt/XtGame.h>

namespace DynamicEffect
{
namespace Particle
{
class C_ParticleManager;
}
}

namespace Game
{

class WindDefinition
{
public:

	WindDefinition();
	~WindDefinition();

	// parameter setting
	void				Reset();
	void				SetQuantities(S32 nNumWindMatrices, S32 nNumLeafAngles);
	void				SetWindResponse(F32 fResponse, F32 fReponseLimit);
	void				SetWindStrengthAndDirection(F32 fWindStrength, F32 fWindDirectionX, F32 fWindDirectionY, F32 fWindDirectionZ);
	void				SetMaxBendAngle(F32 fMaxBendAngle);
	void				SetExponents(F32 fBranchExponent, F32 fLeafExponent);
	void				SetGusting(F32 fGustStrengthMin, F32 fGustStrengthMax, F32 fGustFrequency, F32 fGustDurationMin, F32 fGustDurationMax);
	void				SetBranchHorizontal(F32 fLowWindAngle, F32 fHighWindAngle, F32 fLowWindSpeed, F32 fHighWindSpeed);
	void				SetBranchVertical(F32 fLowWindAngle, F32 fHighWindAngle, F32 fLowWindSpeed, F32 fHighWindSpeed);
	void				SetLeafRocking(F32 fLowWindAngle, F32 fHighWindAngle, F32 fLowWindSpeed, F32 fHighWindSpeed);
	void				SetLeafRustling(F32 fLowWindAngle, F32 fHighWindAngle, F32 fLowWindSpeed, F32 fHighWindSpeed);

	// parameter getting
	void				GetQuantities(S32& iNumWindMatrices, S32& iNumLeafAngles) const;
	void				GetWindResponse(F32& fResponse, F32& fReponseLimit) const;
	void				GetWindStrengthAndDirection(F32& fWindStrength, F32& fWindDirectionX, F32& fWindDirectionY, F32& fWindDirectionZ) const;
	F32					GetMaxBendAngle() const;
	void				GetExponents(F32& fBranchExponent, F32& fLeafExponent) const;
	void				GetGusting(F32& fGustStrengthMin, F32& fGustStrengthMax, F32& fGustFrequency, F32& fGustDurationMin, F32& fGustDurationMax) const;
	void				GetBranchHorizontal(F32& fLowWindAngle, F32& fHighWindAngle, F32& fLowWindSpeed, F32& fHighWindSpeed) const;
	void				GetBranchVertical(F32& fLowWindAngle, F32& fHighWindAngle, F32& fLowWindSpeed, F32& fHighWindSpeed) const;
	void				GetLeafRocking(F32& fLowWindAngle, F32& fHighWindAngle, F32& fLowWindSpeed, F32& fHighWindSpeed) const;
	void				GetLeafRustling(F32& fLowWindAngle, F32& fHighWindAngle, F32& fLowWindSpeed, F32& fHighWindSpeed) const;
	F32					GetTime() const;

	// render interface
	void				Advance(F32 fCurrentTime, bool bUpdateMatrices = true, bool bUpdateLeafAngleMatrices = true);
	F32					GetFinalStrength();
	U32					GetNumWindMatrices() const;
	const F32*			GetWindMatrix(U32 uiIndex) const;
	U32					GetNumLeafAngleMatrices() const;
	const F32*			GetLeafAngleMatrix(U32 uiIndex) const;
	bool				GetRustleAngles(F32* pAngles) const;
	bool				GetRockAngles(F32* pAngles) const;

private:

	class OscillationParams
	{
	public:

		OscillationParams(F32 fLowWindAngle = 0.0f, F32 fHighWindAngle = 0.0f, F32 fLowWindSpeed = 0.0f, F32 fHighWindSpeed = 0.0f);

		void				Advance(F32 fDeltaTime, F32 fWindStrendth);
		void				Reset() { m_fAdjustedTime = 0.0f; }

		F32					m_fLowWindAngle;
		F32					m_fHighWindAngle;
		F32					m_fLowWindSpeed;
		F32					m_fHighWindSpeed;
		F32					m_fAdjustedTime;
		Btl::Vector<F32>	m_vOutputs;
	};

	class WindController
	{
	public:

		WindController();

		void				Advance(F32 fDeltaTime, F32 fP, F32 fMaxA);
		void				Reset(F32 fValue);

		F32					m_fWantedValue;
		F32					m_fCurrentValue;
		F32					m_fLastDelta;
		F32					m_fLastDeltaTime;
	};

	OscillationParams			m_sBranchHorizontal;
	OscillationParams			m_sBranchVertical;
	OscillationParams			m_sLeafRocking;
	OscillationParams			m_sLeafRustling;
	F32							m_fMaxBendAngle;
	F32							m_fWindResponse;
	F32							m_fWindResponseLimit;
	F32							m_fGustingStrengthMin;
	F32							m_fGustingStrengthMax;
	F32							m_fGustingFrequency;
	F32							m_fGustingDurationMin;
	F32							m_fGustingDurationMax;
	F32							m_fBranchExponent;
	F32							m_fLeafExponent;

	F32							m_fLastTime;
	F32							m_fWindStrength;
	F32							m_fCurrentGustingStrength;
	F32							m_fGustStopTime;

	WindController				m_cFinalWindStrength;
	WindController				m_cFinalWindDirectionX;
	WindController				m_cFinalWindDirectionY;
	WindController				m_cFinalWindDirectionZ;
	F32							m_fFinalBendAngle;

	Btl::Vector<MathsStorage::MatrixRow44>	m_vWindMatrices;
	Btl::Vector<MathsStorage::MatrixRow44>	m_vLeafAngleMatrices;

};

class C_Wind;

struct WindSettings
{
	XtVec3	vWindDirection;
	XtF32	fWindStrength;
	XtF32	fGustStrength;
};

class WindEnvSettings : public FX::I_EnvironmentSettings
{
public:

	WindEnvSettings(WindSettings* pData, C_Wind* pWind);
	WindEnvSettings(const WindEnvSettings* pA, const WindEnvSettings* pB, float f);

	Util::C_String			Name() const { return "Wind"; }

	I_EnvironmentSettings*	Lerp(const I_EnvironmentSettings* p, float f) const;

	void					Set();
	void					AssociateDebugVariables(const Util::C_String& sName);

	static void				InitialiseParameters();

private:

	FX::EnvFloatParam	m_fWindDirectionX;
	FX::EnvFloatParam	m_fWindDirectionY;
	FX::EnvFloatParam	m_fWindDirectionZ;
	FX::EnvFloatParam	m_fWindStrength;
	FX::EnvFloatParam	m_fGustStrength;

	C_Wind*				m_pWind;

};



class C_Wind
{
public:

	C_Wind(DynamicEffect::Particle::C_ParticleManager* pParticleManager);
	~C_Wind();

	void SetWindDirection(const F32 fWindDirectionX, const F32 fWindDirectionY, const F32 fWindDirectionZ);
	void SetWindStrength(const F32 fWindStrength);
	void SetGustStrength(const F32 fGustStrength);

	void Update(const F32 fTime, const F32 fTimeStep);
	void DebugRender();

	const F32* const GetWindMatrices() const { return m_WindArray; }
	const F32* const GetLeafMatrices() const { return m_LeafArray; }

private:

	DynamicEffect::Particle::C_ParticleManager*		m_pParticleManager;

	Maths::HVec3	m_vCurrentWind;

	WindDefinition	m_WindDef;

	F32				m_fWindDirectionX;
	F32				m_fWindDirectionY;
	F32				m_fWindDirectionZ;
	F32				m_fWindStrength;
	F32				m_fGustStrength;

	F32				m_WindArray[64];
	F32				m_LeafArray[64];

};

}

#endif // AMAX_GRAPHICS_WIND_H
