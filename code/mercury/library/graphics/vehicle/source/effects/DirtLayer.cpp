/////////////////////////////////////////////////////////////
// Dirt
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Effects/DirtLayer.h"

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

//#pragma optimize("", off)

namespace Vehicle
{

/* 
===============================================================
					class C_DirtLayer

NOTES: 
===============================================================
*/

/*
===============
NAME: C_DirtLayer

NOTE: Constructor
===============
*/
C_DirtLayer::C_DirtLayer()
{
	Reset();
}

/*
===============
NAME: Initialise

NOTE: Initialises the dirt
===============
*/
void C_DirtLayer::Initialise()
{
	Reset();
}

Debug::Colour g_debugDirtBaseColour("Vehicle.Effects.DirtLayer.Base.Colour", 1.0f, 0.85f, 0.65f, 1.0f);
Debug::Float g_debugDirtBaseStartTime("Vehicle.Effects.DirtLayer.Base.StartTime", 0.0f, Debug::Range(0.0f, 360.0f), Debug::Step(0.001f));
Debug::Float g_debugDirtBaseDuration("Vehicle.Effects.DirtLayer.Base.Duration", 120.0f, Debug::Range(0.0f, 360.0f), Debug::Step(0.001f));
Debug::Float g_debugDirtBaseMinSpeed("Vehicle.Effects.DirtLayer.Base.MinSpeed", 7.5f, Debug::Range(0.0f, 60.0f), Debug::Step(0.001f));
Debug::Bool g_debugDirtBaseOverride("Vehicle.Effects.DirtLayer.Base.Override", false);
Debug::Float g_debugDirtBaseScale("Vehicle.Effects.DirtLayer.Base.Scale", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Colour g_debugDirtDetailColour("Vehicle.Effects.DirtLayer.Detail.Colour", 0.425f, 0.325f, 0.0f, 1.0f);
Debug::Float g_debugDirtDetailStartTime("Vehicle.Effects.DirtLayer.Detail.StartTime", 0.0f, Debug::Range(0.0f, 360.0f), Debug::Step(0.001f));
Debug::Float g_debugDirtDetailDuration("Vehicle.Effects.DirtLayer.Detail.Duration", 60.0f, Debug::Range(0.0f, 360.0f), Debug::Step(0.001f));
Debug::Float g_debugDirtDetailMinSpeed("Vehicle.Effects.DirtLayer.Detail.MinSpeed", 15.0f, Debug::Range(0.0f, 60.0f), Debug::Step(0.001f));
Debug::Bool g_debugDirtDetailOverride("Vehicle.Effects.DirtLayer.Detail.Override", false);
Debug::Float g_debugDirtDetailScale("Vehicle.Effects.DirtLayer.Detail.Scale", 0.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: Initialise

NOTE: Updates the dirt
===============
*/
void C_DirtLayer::Update(float p_fTime, float p_fSpeed)
{
	PERF_EVENT("C_DirtLayer.Update");

	if (!m_bEnabled)
		return;

	m_vBaseLayerColour.w	= m_fBaseScale;
	m_vDetailLayerColour.w	= m_fDetailScale;
	
	if (p_fSpeed >= m_fMinSpeedForBase)
	{
		m_fBaseTime				+= p_fTime;
		float fTime				= m_fBaseTime - m_fBaseStartTime;
		m_fBaseScale			= Maths::Clamp(fTime / m_fBaseDuration, 0.0f, 1.0f);
		m_vBaseLayerColour.w	= m_fBaseScale;
	}

	if (g_debugDirtBaseOverride)
	{
		m_fBaseScale			= g_debugDirtBaseScale;
		m_vBaseLayerColour.w	= m_fBaseScale;
	}

	if (p_fSpeed >= m_fMinSpeedForDetail)
	{
		m_fDetailTime			+= p_fTime;
		float fTime				= m_fDetailTime - m_fDetailStartTime;
		m_fDetailScale			= Maths::Clamp(fTime / m_fDetailDuration, 0.0f, 1.0f);		
		m_vDetailLayerColour.w	= m_fDetailScale;
	}

	if (g_debugDirtDetailOverride)
	{
		m_fDetailScale			= g_debugDirtDetailScale;
		m_vDetailLayerColour.w	= m_fDetailScale;
	}
}

/*
===============
NAME: Reset

NOTE: Reset
===============
*/
void C_DirtLayer::Reset()
{
	m_vBaseLayerColour		= MathsStorage::Float4(0.0f, 0.0f, 0.0f, 0.0f);
	m_fBaseStartTime		= 0.0f;
	m_fBaseDuration			= 0.0f;
	m_fMinSpeedForBase		= 0.0f;
	m_fBaseScale			= 0.0f;
	m_fBaseTime				= 0.0f;

	m_vDetailLayerColour	= MathsStorage::Float4(0.0f, 0.0f, 0.0f, 0.0f);
	m_fDetailStartTime		= 0.0f;
	m_fDetailDuration		= 0.0f;
	m_fMinSpeedForDetail	= 0.0f;
	m_fDetailScale			= 0.0f;
	m_fDetailTime			= 0.0f;

	m_bEnabled				= false;
}

} //namespace Vehicle
