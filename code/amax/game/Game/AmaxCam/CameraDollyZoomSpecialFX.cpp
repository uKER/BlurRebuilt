//=============================================================================
// CameraDollyZoomSpecialFX.cpp
// 29/01/09
//=============================================================================
#include "stdafx.h"

#include "game/AmaxCam/CameraDollyZoomSpecialFX.h"
#include "misc_shared/Curve/HermiteCurve.h"

XT_TYPE(AmaxCam::C_CameraDollyZoomSpecialFX, "CameraDollyZoomSpecialFX")
	XT_BASE(SpecialFX::C_BaseSpecialFXComponent)
	XT_FIELD(m_Time,			"Time")
	XT_FIELD(m_FovGraph,		"OffsetFov")
XT_END_TYPE

XT_LIBRARY(AmaxCameraDollyZoomSpecialFX)
	XT_DEPENDS_ON(SpecialFX)
	XT_REGISTER_TYPE(AmaxCam::C_CameraDollyZoomSpecialFX)
XT_END_LIBRARY


namespace AmaxCam
{

	static C_CameraDollyZoomSpecialFX::ActiveDollyZoomList				s_activeDollyZoomList;

	class C_CameraDollyZoomSpecialFXInstance : public SpecialFX::I_SpecialFXComponentInstance
	{
		F32												m_Intensity;
		F32												m_TotalTime;
		Curve::C_HermiteCurve							m_FovGraph;	

		C_CameraDollyZoomSpecialFX::C_DollyZoomOutput	m_Output;

		friend class C_CameraDollyZoomSpecialFX;

	public:
		C_CameraDollyZoomSpecialFXInstance() { m_Intensity = 1.f;};	

		virtual void Start();
		virtual void UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal) {}
		virtual void Stop();
		virtual bool Update(F32 time);
		virtual F32	 GetDuration() { return m_TotalTime; } ;
		virtual void SetParameter(U32 paramID, void* pData);
		virtual void UpdateIntensity(F32 intensity);
	};

	void C_CameraDollyZoomSpecialFXInstance::Start()
	{	
		s_activeDollyZoomList.PushBack(m_Output);
	}

	void C_CameraDollyZoomSpecialFXInstance::Stop()
	{	
		s_activeDollyZoomList.Erase(C_CameraDollyZoomSpecialFX::ActiveDollyZoomList::Iterator(&m_Output));
	}

	bool C_CameraDollyZoomSpecialFXInstance::Update(F32 time)
	{
		
		m_Output.m_Fov = m_FovGraph.Evaluate(time) * m_Intensity;
		

		return true;
	}

	void C_CameraDollyZoomSpecialFXInstance::SetParameter( U32 paramID, void* pData )
	{
		if (paramID == SpecialFX::E_ParamID_SetCameraViewport)
		{
			m_Output.m_ViewportID = (S32)(INTPTR) pData;	// [Blur2PC][x64] viewport id carried in the pointer
		}
	}

	void C_CameraDollyZoomSpecialFXInstance::UpdateIntensity( F32 intensity )
	{
		m_Intensity = intensity;
	}

	SpecialFX::I_SpecialFXComponentInstance* C_CameraDollyZoomSpecialFX::CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const
	{
		C_CameraDollyZoomSpecialFXInstance* pCameraDollyZoomSpecialFXInstance = new C_CameraDollyZoomSpecialFXInstance;
		
		pCameraDollyZoomSpecialFXInstance->m_TotalTime = m_Time;

		//Build Effect graph
		SpecialFX::C_HermiteKeyFrame keyFrame;

		//Build the effect curve
		for(int j = 0; j < m_FovGraph.count; j++)
		{
			keyFrame = m_FovGraph[j];

			pCameraDollyZoomSpecialFXInstance->m_FovGraph.AddKeyFrame(keyFrame.mTime, 
				keyFrame.mValue, 
				keyFrame.mInTangent, 
				keyFrame.mOutTangent);
		}
		
		return pCameraDollyZoomSpecialFXInstance;
	}

	C_CameraDollyZoomSpecialFX::C_DollyZoomOutput::C_DollyZoomOutput() 
		:	m_Fov(0.0f),
			m_ViewportID(-1)
	{
	}

	const C_CameraDollyZoomSpecialFX::ActiveDollyZoomList& 
		C_CameraDollyZoomSpecialFX::GetActiveDollyZoomList() 
	{
		return s_activeDollyZoomList;
	}
} // End of namespace
