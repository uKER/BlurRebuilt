//=============================================================================
// CameraNoiseSpecialFX.cpp
// 02/11/09
//=============================================================================
#include "stdafx.h"

#include "game/AmaxCam/CameraNoiseSpecialFX.h"
#include "misc_shared/Curve/HermiteCurve.h"
#include "misc_shared/Noise/Perlin.h"
#include <framework/application/ApplicationBase.h>

XT_TYPE(AmaxCam::C_CameraNoiseSpecialFX::NoiseParams,"CameraNoiseParams")
	XT_FIELD(m_XScale,"XScale")	
	XT_FIELD(m_YScale,"YScale")	
	XT_FIELD(m_ZScale,"ZScale")	
	XT_FIELD(m_XIntensityCurve,"XData")	
	XT_FIELD(m_YIntensityCurve,"YData")	
	XT_FIELD(m_ZIntensityCurve,"ZData")	
XT_END_TYPE

XT_TYPE(AmaxCam::C_CameraNoiseSpecialFX, "CameraNoiseSpecialFX")
	XT_BASE(SpecialFX::C_BaseSpecialFXComponent)
	XT_FIELD(m_Time,				"Time")
	XT_FIELD(m_Frequency,			"Frequency")
	XT_FIELD(m_TranslationParams,	"TranslationParams")
	XT_FIELD(m_RotationParams,		"RotationParams")
XT_END_TYPE

XT_LIBRARY(AmaxCameraNoiseSpecialFX)
	XT_DEPENDS_ON(SpecialFX)
	XT_REGISTER_TYPE(AmaxCam::C_CameraNoiseSpecialFX)
	XT_REGISTER_TYPE(AmaxCam::C_CameraNoiseSpecialFX::NoiseParams)
XT_END_LIBRARY

namespace AmaxCam
{

	static C_CameraNoiseSpecialFX::ActiveNoiseList				s_activeNoiseList;

	class C_CameraNoiseSpecialFXInstance : public SpecialFX::I_SpecialFXComponentInstance
	{
		F32										m_Intensity;
		F32										m_TotalTime;
		F32										m_Frequency;
		
		class C_NoiseInput
		{
		public:
			F32 m_XScale;
			F32 m_YScale;
			F32 m_ZScale;
			Curve::C_HermiteCurve				m_XIntensityCurve;
			Curve::C_HermiteCurve				m_YIntensityCurve;
			Curve::C_HermiteCurve				m_ZIntensityCurve;
		};

		class C_NoiseData
		{
		public:
				C_NoiseData();
		
			S32									m_ShakePos;
		};
			

		C_NoiseInput							m_TranslationInput;
		C_NoiseInput							m_RotationInput;

		C_NoiseData								m_TranslationData;
		C_NoiseData								m_RotationData;

		C_CameraNoiseSpecialFX::C_NoiseOutput	m_Output;

		friend class C_CameraNoiseSpecialFX;

	public:
		C_CameraNoiseSpecialFXInstance();
				void PopulateHermiteCurve(Curve::C_HermiteCurve& Output, XtArray<SpecialFX::C_HermiteKeyFrame> Input );
		virtual void Start();
		virtual void UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal) {}
		virtual void Stop();
		virtual bool Update(F32 time);
		virtual F32	 GetDuration() { return m_TotalTime; } ;
		virtual void SetParameter(U32 paramID, void* pData);
		virtual void UpdateIntensity(F32 intensity);


	};

	C_CameraNoiseSpecialFXInstance::C_CameraNoiseSpecialFXInstance()
	{
		m_Intensity = 1.f;
	}

	void C_CameraNoiseSpecialFXInstance::Start()
	{	
		s_activeNoiseList.PushBack(m_Output);
	}

	void C_CameraNoiseSpecialFXInstance::Stop()
	{	
		s_activeNoiseList.Erase(C_CameraNoiseSpecialFX::ActiveNoiseList::Iterator(&m_Output));
	}

	bool C_CameraNoiseSpecialFXInstance::Update(F32 time)
	{
		if (time > m_TotalTime)
			return false;

		C_PerlinNoiseDefault noise;

		// How else can I get the frame time?	
		F32 elapsedTime = Framework::C_ApplicationBase::GetInstance()->GetTimer().GetFrameTime();

		F32 xTrans = noise.SoftNoise1D(m_TranslationData.m_ShakePos)			* m_TranslationInput.m_XIntensityCurve.Evaluate(time/m_TotalTime)	* m_TranslationInput.m_XScale;
		F32 yTrans = noise.SoftNoise1D(m_TranslationData.m_ShakePos + 0x234567) * m_TranslationInput.m_YIntensityCurve.Evaluate(time/m_TotalTime)	* m_TranslationInput.m_YScale;
		F32 zTrans = noise.SoftNoise1D(m_TranslationData.m_ShakePos + 0x566757) * m_TranslationInput.m_ZIntensityCurve.Evaluate(time/m_TotalTime)	* m_TranslationInput.m_ZScale;
		F32 pitch  = noise.SoftNoise1D(m_RotationData.m_ShakePos + 0x786452)	* m_RotationInput.m_XIntensityCurve.Evaluate(time/m_TotalTime)		* m_RotationInput.m_XScale;
		F32 yaw	   = noise.SoftNoise1D(m_RotationData.m_ShakePos + 0x918256)	* m_RotationInput.m_YIntensityCurve.Evaluate(time/m_TotalTime)		* m_RotationInput.m_YScale;
		F32 roll   = noise.SoftNoise1D(m_RotationData.m_ShakePos + 0x184399)	* m_RotationInput.m_ZIntensityCurve.Evaluate(time/m_TotalTime)		* m_RotationInput.m_ZScale;

		m_Output.m_Position = Maths::HVec3(xTrans,yTrans,zTrans);
		m_Output.m_Orientation = Maths::HVec3(pitch,yaw,roll);
		//m_Output.m_Orientation = Maths::C_Quaternion(sin(pitch), 0.f, 0.f, cos(pitch));
		//m_Output.m_Orientation = m_Output.m_Orientation * Maths::C_Quaternion(0.f, sin(yaw), 0.f, cos(yaw));
		//m_Output.m_Orientation = m_Output.m_Orientation * Maths::C_Quaternion(0.f, 0.f, sin(roll), cos(roll));

		m_TranslationData.m_ShakePos += (S32)(elapsedTime * 65536.f * m_Frequency);
		m_RotationData.m_ShakePos += (S32)(elapsedTime * 65536.f * m_Frequency);
		
		return true;
	}

	void C_CameraNoiseSpecialFXInstance::SetParameter( U32 paramID, void* pData )
	{
		if (paramID == SpecialFX::E_ParamID_SetCameraViewport)
		{
			m_Output.m_ViewportID = (S32)(INTPTR) pData;	// [Blur2PC][x64] viewport id carried in the pointer
		}
	}

	void C_CameraNoiseSpecialFXInstance::UpdateIntensity( F32 intensity )
	{
		m_Intensity = intensity;
	}	
	
	void C_CameraNoiseSpecialFXInstance::PopulateHermiteCurve(
		Curve::C_HermiteCurve& Output, 
		XtArray<SpecialFX::C_HermiteKeyFrame> Input 
		)
	{
		SpecialFX::C_HermiteKeyFrame keyFrame;

		for(int j = 0; j < Input.count; j++)
		{
			keyFrame = Input[j];

			Output.AddKeyFrame(	keyFrame.mTime, 
				keyFrame.mValue, 
				keyFrame.mInTangent, 
				keyFrame.mOutTangent);
		}
	}

	C_CameraNoiseSpecialFXInstance::C_NoiseData::C_NoiseData()
	{
		m_ShakePos = 0;
	}

	SpecialFX::I_SpecialFXComponentInstance* C_CameraNoiseSpecialFX::CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const
	{
		C_CameraNoiseSpecialFXInstance* pCameraNoiseSpecialFXInstance = new C_CameraNoiseSpecialFXInstance;
		
		pCameraNoiseSpecialFXInstance->m_TotalTime = m_Time;
		pCameraNoiseSpecialFXInstance->m_Frequency = m_Frequency;
		
		pCameraNoiseSpecialFXInstance->m_TranslationInput.m_XScale = m_TranslationParams.m_XScale;
		pCameraNoiseSpecialFXInstance->m_TranslationInput.m_YScale = m_TranslationParams.m_YScale; 
		pCameraNoiseSpecialFXInstance->m_TranslationInput.m_ZScale = m_TranslationParams.m_ZScale;

		pCameraNoiseSpecialFXInstance->m_RotationInput.m_XScale = m_RotationParams.m_XScale;
		pCameraNoiseSpecialFXInstance->m_RotationInput.m_YScale = m_RotationParams.m_YScale; 
		pCameraNoiseSpecialFXInstance->m_RotationInput.m_ZScale = m_RotationParams.m_ZScale;

		pCameraNoiseSpecialFXInstance->PopulateHermiteCurve(pCameraNoiseSpecialFXInstance->m_TranslationInput.m_XIntensityCurve, m_TranslationParams.m_XIntensityCurve);
		pCameraNoiseSpecialFXInstance->PopulateHermiteCurve(pCameraNoiseSpecialFXInstance->m_TranslationInput.m_YIntensityCurve, m_TranslationParams.m_YIntensityCurve);
		pCameraNoiseSpecialFXInstance->PopulateHermiteCurve(pCameraNoiseSpecialFXInstance->m_TranslationInput.m_ZIntensityCurve, m_TranslationParams.m_ZIntensityCurve);
		pCameraNoiseSpecialFXInstance->PopulateHermiteCurve(pCameraNoiseSpecialFXInstance->m_RotationInput.m_XIntensityCurve, m_RotationParams.m_XIntensityCurve);
		pCameraNoiseSpecialFXInstance->PopulateHermiteCurve(pCameraNoiseSpecialFXInstance->m_RotationInput.m_YIntensityCurve, m_RotationParams.m_YIntensityCurve);
		pCameraNoiseSpecialFXInstance->PopulateHermiteCurve(pCameraNoiseSpecialFXInstance->m_RotationInput.m_ZIntensityCurve, m_RotationParams.m_ZIntensityCurve);

		return pCameraNoiseSpecialFXInstance;
	}



	C_CameraNoiseSpecialFX::C_NoiseOutput::C_NoiseOutput() 
		:	m_ViewportID(-1)
	{
		//m_Orientation = Maths::Const<Maths::Quaternion>::Identity();
		m_Orientation = Maths::Constants<Maths::HVec3>::Zero();
		m_Position = Maths::Constants<Maths::HVec3>::Zero();
	}

	const C_CameraNoiseSpecialFX::ActiveNoiseList& 
		C_CameraNoiseSpecialFX::GetActiveNoiseList() 
	{
		return s_activeNoiseList;
	}
} // End of namespace
