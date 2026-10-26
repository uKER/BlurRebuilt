
#include "stdafx.h"

#include "game/AmaxCam/CameraSpecialFX.h"
#include "misc_shared/Curve/HermiteCurve.h"
#include "CameraSystem.h"
#include "VehicleFeed.h"

XT_TYPE(AmaxCam::C_CameraSpecialFX, "CameraSpecialFX")
	XT_BASE(SpecialFX::C_BaseSpecialFXComponent)
	XT_FIELD(m_Time,				"Time")
	XT_FIELD(m_OffsetXGraph,		"OffsetX")
	XT_FIELD(m_OffsetYGraph,		"OffsetY")
	XT_FIELD(m_OffsetZGraph,		"OffsetZ")
	XT_FIELD(m_OffsetPitchGraph,	"Pitch")
	XT_FIELD(m_OffsetYawGraph,		"Yaw")
	XT_FIELD(m_OffsetRollGraph,		"Roll")
	XT_FIELD(m_OffsetFovGraph,		"OffsetFov")
XT_END_TYPE

XT_LIBRARY(AmaxCameraSpecialFX)
	XT_DEPENDS_ON(SpecialFX)
	XT_REGISTER_TYPE(AmaxCam::C_CameraSpecialFX)
XT_END_LIBRARY

namespace AmaxCam
{

using namespace Maths;

class C_CameraSpecialFXInstance : public SpecialFX::I_SpecialFXComponentInstance
{
	F32									m_TotalTime;
	F32									m_Intensity;

	Curve::C_HermiteCurve				m_OffsetXCurve;	
	Curve::C_HermiteCurve				m_OffsetYCurve;	
	Curve::C_HermiteCurve				m_OffsetZCurve;	
	Curve::C_HermiteCurve				m_OffsetPitchCurve;	
	Curve::C_HermiteCurve				m_OffsetYawCurve;	
	Curve::C_HermiteCurve				m_OffsetRollCurve;	
	Curve::C_HermiteCurve				m_OffsetFovCurve;	

	S32									m_ViewportID;
	Maths::HMat33						m_Transform;
	bool								m_EnableTransform;
	friend class C_CameraSpecialFX;

public:
	C_CameraSpecialFXInstance() { m_ViewportID = 0; m_Intensity = 1.f;};	

	virtual void Start() { m_EnableTransform = false; }
	virtual void UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal) { m_Transform = Maths::HMat33(worldMatrix); }
	virtual void Stop();
	virtual bool Update(F32 time);
	virtual F32	 GetDuration() { return m_TotalTime; } 
	virtual void SetParameter(U32 paramID, void* pData);
	virtual void UpdateIntensity(F32 intensity);
};

void C_CameraSpecialFXInstance::Stop()
{
	AmaxCam::CameraSystem().VehicleFeed()->AddOffset(HVec3(), HVec3(), 0.f);
}

bool C_CameraSpecialFXInstance::Update(F32 time)
{
	if (time > m_TotalTime)
		return false;
		
	F32 offsetX = m_OffsetXCurve.Evaluate(time / m_TotalTime) * m_Intensity;
	F32 offsetY = m_OffsetYCurve.Evaluate(time / m_TotalTime) * m_Intensity;
	F32 offsetZ = m_OffsetZCurve.Evaluate(time / m_TotalTime) * m_Intensity;
	F32 offsetPitch = m_OffsetPitchCurve.Evaluate(time / m_TotalTime) * m_Intensity;
	F32 offsetYaw = m_OffsetYawCurve.Evaluate(time / m_TotalTime) * m_Intensity;
	F32 offsetRoll = m_OffsetRollCurve.Evaluate(time / m_TotalTime) * m_Intensity;
	F32 offsetFov = m_OffsetFovCurve.Evaluate(time / m_TotalTime) * m_Intensity;
	
	HVec3 offset(offsetX, offsetY, offsetZ);
	HVec3 offsetrot(offsetPitch, offsetYaw, offsetRoll);

	if(m_EnableTransform)
	{
		offset = offset * m_Transform;
		offsetrot = offsetrot * m_Transform;
	}

	if (m_ViewportID != -1)
		AmaxCam::CameraSystem().VehicleFeed(m_ViewportID)->AddOffset(offset, offsetrot, offsetFov);
	return true;
}

void C_CameraSpecialFXInstance::SetParameter( U32 paramID, void* pData )
{
	if (paramID == SpecialFX::E_ParamID_SetCameraViewport)
	{
		m_ViewportID = (S32)(INTPTR) pData;	// [Blur2PC][x64] viewport id carried in the pointer
	}
	else if( paramID == SpecialFX::E_ParamID_EnableTransformUpdate )
	{
		m_EnableTransform =  ((int)(INTPTR)pData) == 1?true:false;	// [Blur2PC][x64]
	}
}

void C_CameraSpecialFXInstance::UpdateIntensity( F32 intensity )
{
	m_Intensity = intensity;
}

SpecialFX::I_SpecialFXComponentInstance* C_CameraSpecialFX::CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const
{
	C_CameraSpecialFXInstance* pCameraSpecialFXInstance = new C_CameraSpecialFXInstance;
	
	pCameraSpecialFXInstance->m_TotalTime = m_Time;
	pCameraSpecialFXInstance->m_Transform.SetIdentity();
	
	//Build Effect graph
	SpecialFX::C_HermiteKeyFrame keyFrame;

	//Build the effect curve
	for(int j = 0; j < m_OffsetXGraph.count; j++)
	{
		keyFrame = m_OffsetXGraph[j];

		pCameraSpecialFXInstance->m_OffsetXCurve.AddKeyFrame(keyFrame.mTime, 
			keyFrame.mValue, 
			keyFrame.mInTangent, 
			keyFrame.mOutTangent);
	}
	
	for(int j = 0; j < m_OffsetYGraph.count; j++)
	{
		keyFrame = m_OffsetYGraph[j];

		pCameraSpecialFXInstance->m_OffsetYCurve.AddKeyFrame(keyFrame.mTime, 
			keyFrame.mValue, 
			keyFrame.mInTangent, 
			keyFrame.mOutTangent);
	}
	
	for(int j = 0; j < m_OffsetZGraph.count; j++)
	{
		keyFrame = m_OffsetZGraph[j];

		pCameraSpecialFXInstance->m_OffsetZCurve.AddKeyFrame(keyFrame.mTime, 
			keyFrame.mValue, 
			keyFrame.mInTangent, 
			keyFrame.mOutTangent);
	}


	for(int j = 0; j < m_OffsetPitchGraph.count; j++)
	{
		keyFrame = m_OffsetPitchGraph[j];

		pCameraSpecialFXInstance->m_OffsetPitchCurve.AddKeyFrame(keyFrame.mTime, 
			keyFrame.mValue, 
			keyFrame.mInTangent, 
			keyFrame.mOutTangent);
	}
	
	for(int j = 0; j < m_OffsetYawGraph.count; j++)
	{
		keyFrame = m_OffsetYawGraph[j];

		pCameraSpecialFXInstance->m_OffsetYawCurve.AddKeyFrame(keyFrame.mTime, 
			keyFrame.mValue, 
			keyFrame.mInTangent, 
			keyFrame.mOutTangent);
	}
	
	for(int j = 0; j < m_OffsetRollGraph.count; j++)
	{
		keyFrame = m_OffsetRollGraph[j];

		pCameraSpecialFXInstance->m_OffsetRollCurve.AddKeyFrame(keyFrame.mTime, 
			keyFrame.mValue, 
			keyFrame.mInTangent, 
			keyFrame.mOutTangent);
	}

	
	for(int j = 0; j < m_OffsetFovGraph.count; j++)
	{
		keyFrame = m_OffsetFovGraph[j];

		pCameraSpecialFXInstance->m_OffsetFovCurve.AddKeyFrame(keyFrame.mTime, 
			keyFrame.mValue, 
			keyFrame.mInTangent, 
			keyFrame.mOutTangent);
	}
	
	return pCameraSpecialFXInstance;
}

} // End of namespace
