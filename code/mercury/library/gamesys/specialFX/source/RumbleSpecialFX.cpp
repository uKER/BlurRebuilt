#include "SpecialFXPch.h"

#include "SpecialFX/RumbleSpecialFX.h"
#include "misc_shared/Curve/HermiteCurve.h"
#include <framework/application/ApplicationBase.h>

XT_TYPE(SpecialFX::C_RumbleSpecialFX, "RumbleSpecialFX")
	XT_BASE(SpecialFX::C_BaseSpecialFXComponent)
	XT_FIELD(m_RumbleTypeID,		"RumbleTypeID")
	XT_FIELD(m_Time,				"Time")
	XT_FIELD(m_LowFreqGraph,		"LowFreqIntensity")
	XT_FIELD(m_HighFreqGraph,		"HighFreqIntensity")
	XT_FIELD(m_LowFreqPulseFreq,	"LowFreqPulseFreq")
	XT_FIELD(m_HighFreqPulseFreq,	"HighFreqPulseFreq")
	XT_FIELD(m_LowFreqPulseLength,	"LowFreqPulseLength")
	XT_FIELD(m_HighFreqPulseLength,	"HighFreqPulseLength")
XT_END_TYPE

namespace SpecialFX
{

	static C_RumbleSpecialFX::ActiveRumblesList				s_activeRumblesList;

	class C_RumbleSpecialFXInstance : public SpecialFX::I_SpecialFXComponentInstance
	{
		F32									m_TotalTime;
		F32									m_Intensity;

		class C_FrequencyParams
		{
		public:
			Curve::C_HermiteCurve			m_PulseFreq;
			Curve::C_HermiteCurve			m_PulseLength;
		};

		class C_FrequencyData
		{
		public:
			C_FrequencyData();

			void							Update(F32 elapsedTime, F32 time, const Curve::C_HermiteCurve& freqCurve, const Curve::C_HermiteCurve& lengthCurve );
			bool							m_Pulsing;
			F32								m_Period;
			F32								m_PulseTime;
			F32								m_Time;
			
		};

		C_FrequencyData						m_LowFrequencyData;
		C_FrequencyData						m_HighFrequencyData;

		C_FrequencyParams					m_LowFrequencyParams;
		C_FrequencyParams					m_HighFrequencyParams;

		Curve::C_HermiteCurve				m_LowFreqCurve;	
		Curve::C_HermiteCurve				m_HighFreqCurve;	
	

		friend class C_RumbleSpecialFX;

		C_RumbleSpecialFX::C_RumbleOutput	m_output;

	public:
		C_RumbleSpecialFXInstance() { m_Intensity = 1.f; }	

		virtual void Start();
		virtual void UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal) {};
		virtual void Stop();
		virtual bool Update(F32 time);
		virtual F32	 GetDuration() { return m_TotalTime; } ;
		virtual void SetParameter(U32 paramID, void* pData);
		virtual void UpdateIntensity(F32 intensity);
	};

	void C_RumbleSpecialFXInstance::Start()
	{
		s_activeRumblesList.PushBack(m_output);
	}

	void C_RumbleSpecialFXInstance::Stop()
	{
		s_activeRumblesList.Erase(C_RumbleSpecialFX::ActiveRumblesList::Iterator(&m_output));
	}

	bool C_RumbleSpecialFXInstance::Update(F32 time)
	{
		if (time > m_TotalTime)
			return false;
		// How else can I get the frame time?	
		F32 elapsedTime = Framework::C_ApplicationBase::GetInstance()->GetTimer().GetFrameTime();

		m_output.m_LowFreqSpeed = m_LowFreqCurve.Evaluate(time/m_TotalTime) * m_Intensity;
		m_output.m_HighFreqSpeed = m_HighFreqCurve.Evaluate(time/m_TotalTime) * m_Intensity;
		
		m_LowFrequencyData.Update(elapsedTime, time/m_TotalTime, m_LowFrequencyParams.m_PulseFreq, m_LowFrequencyParams.m_PulseLength);
		m_HighFrequencyData.Update(elapsedTime, time/m_TotalTime, m_HighFrequencyParams.m_PulseFreq, m_HighFrequencyParams.m_PulseLength);

		if( !m_LowFrequencyData.m_Pulsing )
		{
			m_output.m_LowFreqSpeed = 0.0f;
		}
		if( !m_HighFrequencyData.m_Pulsing )
		{
			m_output.m_HighFreqSpeed = 0.0f;
		}
		
		return true;
	}

	void C_RumbleSpecialFXInstance::SetParameter( U32 paramID, void* pData )
	{
		if (paramID == SpecialFX::E_ParamID_SetCameraViewport)
		{
			m_output.m_ViewportID = (S32)(INTPTR) pData;	// [Blur2PC][x64] integer carried in the pointer
		}
	}

	void C_RumbleSpecialFXInstance::UpdateIntensity( F32 intensity )
	{
		m_Intensity = intensity;
	}

	SpecialFX::I_SpecialFXComponentInstance* C_RumbleSpecialFX::CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const
	{
		C_RumbleSpecialFXInstance* pRumbleSpecialFXInstance = new C_RumbleSpecialFXInstance;
		
		pRumbleSpecialFXInstance->m_TotalTime = m_Time;
		pRumbleSpecialFXInstance->m_output.m_RumbleTypeID = m_RumbleTypeID;


		//Build Effect graph
		SpecialFX::C_HermiteKeyFrame keyFrame;

		//Build the effect curve
		for(int j = 0; j < m_LowFreqGraph.count; j++)
		{
			keyFrame = m_LowFreqGraph[j];

			pRumbleSpecialFXInstance->m_LowFreqCurve.AddKeyFrame(	keyFrame.mTime, 
																	keyFrame.mValue, 
																	keyFrame.mInTangent, 
																	keyFrame.mOutTangent);
		}
		
		for(int j = 0; j < m_HighFreqGraph.count; j++)
		{
			keyFrame = m_HighFreqGraph[j];

			pRumbleSpecialFXInstance->m_HighFreqCurve.AddKeyFrame(	keyFrame.mTime, 
																	keyFrame.mValue, 
																	keyFrame.mInTangent, 
																	keyFrame.mOutTangent);
		}

		for(int j = 0; j < m_LowFreqPulseFreq.count; j++)
		{
			keyFrame = m_LowFreqPulseFreq[j];

			pRumbleSpecialFXInstance->m_LowFrequencyParams.m_PulseFreq.AddKeyFrame(	keyFrame.mTime, 
																					keyFrame.mValue, 
																					keyFrame.mInTangent, 
																					keyFrame.mOutTangent);
		}

		for(int j = 0; j < m_HighFreqPulseFreq.count; j++)
		{
			keyFrame = m_HighFreqPulseFreq[j];

			pRumbleSpecialFXInstance->m_HighFrequencyParams.m_PulseFreq.AddKeyFrame(keyFrame.mTime, 
																					keyFrame.mValue, 
																					keyFrame.mInTangent, 
																					keyFrame.mOutTangent);
		}

		for(int j = 0; j < m_LowFreqPulseLength.count; j++)
		{
			keyFrame = m_LowFreqPulseLength[j];

			pRumbleSpecialFXInstance->m_LowFrequencyParams.m_PulseLength.AddKeyFrame(	keyFrame.mTime, 
				keyFrame.mValue, 
				keyFrame.mInTangent, 
				keyFrame.mOutTangent);
		}

		for(int j = 0; j < m_HighFreqPulseLength.count; j++)
		{
			keyFrame = m_HighFreqPulseLength[j];

			pRumbleSpecialFXInstance->m_HighFrequencyParams.m_PulseLength.AddKeyFrame(keyFrame.mTime, 
				keyFrame.mValue, 
				keyFrame.mInTangent, 
				keyFrame.mOutTangent);
		}

		return pRumbleSpecialFXInstance;
	}

	C_RumbleSpecialFXInstance::C_FrequencyData::C_FrequencyData()
		: m_Pulsing(true), m_Period(-1.0f), m_PulseTime(1.0f), m_Time(0.0f)
	{
		
	}

	void							
	C_RumbleSpecialFXInstance::C_FrequencyData::Update(
		F32 elapsedTime, 
		F32 time,
		const Curve::C_HermiteCurve& freqCurve, 
		const Curve::C_HermiteCurve& lengthCurve )
	{
		
		F32 frequency = freqCurve.Evaluate(time);
		if( frequency > 0.1f )
		{
			m_Time += elapsedTime;
			if(m_Time > m_PulseTime)
			{
				m_Pulsing = false;
			}
			if(m_Time > m_Period)
			{
				m_Period = 1.0f / frequency;
				m_PulseTime = lengthCurve.Evaluate(time);
				m_Time = 0.0f;
				m_Pulsing = true;
			}
		}
		else
		{
			m_Period = -1.0f;
			m_PulseTime = 1.0f;
			m_Pulsing = true;
			m_Time = 0.0f;
		}
	}

	C_RumbleSpecialFX::C_RumbleOutput::C_RumbleOutput() 
	:	m_LowFreqSpeed(0.0f), 
		m_HighFreqSpeed(0.0f),
		m_ViewportID(-1)
	{
	}

	const C_RumbleSpecialFX::ActiveRumblesList& 
	C_RumbleSpecialFX::GetActiveRumblesList() 
	{
		return s_activeRumblesList;
	}
} // End of namespace
