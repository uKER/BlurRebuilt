/////////////////////////////////////////////////////////////
// Wheels
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Effects/Wheels.h"
#include "Vehicle/Input.h"
#include "Vehicle/Component.h"
#include "Vehicle/Material/Material.h"

#include <RenderingData/RenderEnums.h>
#include <world_render/ModelInstance.h>	// [Blur2PC] full WorldRender::C_ModelInstance

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

/* 
===============================================================
					class C_WheelBlur

NOTES: 
===============================================================
*/

/*
===============
NAME: C_WheelBlur

NOTE: Constructor
===============
*/
C_WheelBlur::C_WheelBlur()
{
	m_pInput			= NULL;

	m_pUnblurredSpokes	= NULL;
	m_pBlurredSpokes	= NULL;

	m_fScale			= 0.0f;

	m_bUpdateMe			= false;

	m_SolidPass			= 0;
	m_AlphaPass			= 0;
}

/*
===============
NAME: Initialise

NOTE: Initialises the blur
===============
*/
void C_WheelBlur::Initialise(C_Part* p_pUnblurredSpokes, C_Part* p_pBlurredSpokes, C_WheelInput* p_pInput)
{
	m_pInput			= p_pInput;

	m_pUnblurredSpokes	= p_pUnblurredSpokes;
	m_pBlurredSpokes	= p_pBlurredSpokes;

	m_fScale			= 0.0f;

	m_iCurrentState		= -1;

	m_bUpdateMe			= false;
}

Debug::Float g_debugWheelBlurMinSpin("Vehicle.Wheels.Blur.MinSpinVelocity", 15.0f, Debug::Range(0.0f, 100.0f), Debug::Step(0.001f));
Debug::Float g_debugWheelBlurMaxSpin("Vehicle.Wheels.Blur.MaxSpinVelocity", 38.0f, Debug::Range(0.0f, 100.0f), Debug::Step(0.001f));
Debug::Bool g_debugWheelBlurOverrideScale("Vehicle.Wheels.Blur.Override", false);
Debug::Float g_debugWheelBlurScale("Vehicle.Wheels.Blur.Scale", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: Initialise

NOTE: Updates the blur
===============
*/
void C_WheelBlur::Update()
{
	PERF_EVENT("C_WheelBlur.Update");

	if (!m_pUnblurredSpokes || !m_pUnblurredSpokes->m_pMaterialParameters || !m_pUnblurredSpokes->m_pModelInstance ||
		!m_pBlurredSpokes || !m_pBlurredSpokes->m_pMaterialParameters || !m_pBlurredSpokes->m_pModelInstance ||
		!m_pInput)
		return;

	if (m_pUnblurredSpokes->Hidden() || m_pBlurredSpokes->Hidden())
	{
		m_bUpdateMe = true;
		return;
	}

	float fSpinSpeed = m_pInput->m_vAngularVelocity.Length();

	float fScale = 0.0f;

	float fRange = (float)g_debugWheelBlurMaxSpin - (float)g_debugWheelBlurMinSpin;

	if (fRange > 0.0f)
		fScale = Maths::Clamp((fSpinSpeed - (float)g_debugWheelBlurMinSpin) / fRange, 0.0f, 1.0f);

	if (g_debugWheelBlurOverrideScale)
		fScale = g_debugWheelBlurScale;
		
	//if (fScale == m_fScale)
	//	return;
	
	m_fScale = fScale;

	if (m_fScale == 0.0f)
	{
		//Blur = off
		if (m_iCurrentState	!= 1 || m_bUpdateMe)
		{
			m_pUnblurredSpokes->SetVisible(true);
			m_pBlurredSpokes->SetVisible(false);
			m_bUpdateMe = true;
			m_iCurrentState	= 1;
// 			m_pUnblurredSpokes->m_pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_BACK_FRONT, m_SolidPass);
// 			m_pBlurredSpokes->m_pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_BACK_FRONT, m_SolidPass);
 			m_pUnblurredSpokes->m_pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, m_SolidPass);
 			//m_pBlurredSpokes->m_pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, m_SolidPass);
			m_pUnblurredSpokes->m_pModelInstance->Build();
			//m_pBlurredSpokes->m_pModelInstance->Build();
		}
	}
	else if (m_fScale == 1.0f)
	{
		//Blur = on
		if (m_iCurrentState	!= 2 || m_bUpdateMe)
		{
			m_pUnblurredSpokes->SetVisible(false);
			m_pBlurredSpokes->SetVisible(true);	
			m_bUpdateMe = true;	
			m_iCurrentState	= 2;
		}
	}
	else
	{
		if (m_iCurrentState	!= 3 || m_bUpdateMe)
		{
			m_pUnblurredSpokes->SetVisible(true);
			m_pBlurredSpokes->SetVisible(true);	
			m_bUpdateMe = true;
			m_iCurrentState	= 3;
			//m_pUnblurredSpokes->m_pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_BACK_FRONT, m_AlphaPass);
			//m_pBlurredSpokes->m_pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_BACK_FRONT, m_AlphaPass);
			m_pUnblurredSpokes->m_pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, m_AlphaPass);
			//m_pBlurredSpokes->m_pModelInstance->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_OPAQUE, m_AlphaPass);
			m_pUnblurredSpokes->m_pModelInstance->Build();
			//m_pBlurredSpokes->m_pModelInstance->Build();
		}
	}

	if (m_iCurrentState == 3 || m_bUpdateMe)
	{
		//Blur = transition
		if (m_pUnblurredSpokes->m_pMaterialParameters->m_Type == E_MaterialType_Alloy)
		{
			C_MaterialAlloy* pMat = (C_MaterialAlloy*)m_pUnblurredSpokes->m_pMaterialParameters;
			pMat->m_Colour.w = 1.0f - m_fScale;
			m_pUnblurredSpokes->m_MaterialParametersUpdate = true;
		}
		if (m_pBlurredSpokes->m_pMaterialParameters->m_Type == E_MaterialType_Alloy)
		{
			C_MaterialAlloy* pMat = (C_MaterialAlloy*)m_pBlurredSpokes->m_pMaterialParameters;
			pMat->m_Colour.w = m_fScale;
			m_pBlurredSpokes->m_MaterialParametersUpdate = true;
		}
	}
	else
	{
		m_pUnblurredSpokes->m_MaterialParametersUpdate = false;
		m_pBlurredSpokes->m_MaterialParametersUpdate = false;
	}

	m_bUpdateMe = false;
}

/*
===============
NAME: Shutdown

NOTE: Shutdown
===============
*/
void C_WheelBlur::Shutdown()
{
	m_pInput			= NULL;

	m_pUnblurredSpokes	= NULL;
	m_pBlurredSpokes	= NULL;

	m_fScale			= 0.0f;
}

/*
===============
NAME: Reset

NOTE: Reset
===============
*/
void C_WheelBlur::Reset()
{
	if (!m_pUnblurredSpokes || !m_pUnblurredSpokes->m_pMaterialParameters ||
		!m_pBlurredSpokes || !m_pBlurredSpokes->m_pMaterialParameters || 
		!m_pInput)
		return;
	
	m_pUnblurredSpokes->SetVisible(true);
	m_pBlurredSpokes->SetVisible(false);
	m_iCurrentState	= 1;

	C_MaterialAlloy* pMat = (C_MaterialAlloy*)m_pUnblurredSpokes->m_pMaterialParameters;
	pMat->m_Colour.w = 1.0f;	
	m_pUnblurredSpokes->m_MaterialParametersUpdate = true;
	
	pMat = (C_MaterialAlloy*)m_pBlurredSpokes->m_pMaterialParameters;
	pMat->m_Colour.w = 0.0f;
	m_pBlurredSpokes->m_MaterialParametersUpdate = true;

	m_bUpdateMe = true;
}

} //namespace Vehicle
