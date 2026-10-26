/////////////////////////////////////////////////////////////
// Dirt
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_DIRT_H__
#define __VEHICLE_DIRT_H__

namespace Vehicle
{

/* 
===============================================================
					class C_DirtLayer

NOTES: 
===============================================================
*/
class C_DirtLayer
{
public:

	C_DirtLayer();

	//! Initialises dirt
	void Initialise();

	//! Updates the dirt
	void Update(float p_fTime, float p_fSpeed);

	//! Reset
	void Reset();

	//! Enables dirt layer
	void Enable() { m_bEnabled = true; }

public:

	MathsStorage::Float4	m_vBaseLayerColour;			//!< Colour of dirt base layer
	MathsStorage::Float4	m_vDetailLayerColour;		//!< Colour of dirt detail layer
	MathsStorage::Float4	m_vDirtSettings;			//!< Reflection, etc, values

	float					m_fBaseTime;				//!< Current time for base layer
	float					m_fBaseStartTime;			//!< When to start accumulating dirt
	float					m_fBaseDuration;			//!< Time to maximum dirt layerage
	float					m_fMinSpeedForBase;			//!< Minimum speed the vehicle has to be travelling to accumulate base dirt
	float					m_fBaseScale;				//!< Current base dirt level

	float					m_fDetailTime;				//!< Current time for detail layer
	float					m_fDetailStartTime;			//!< When to start accumulating dirt
	float					m_fDetailDuration;			//!< Time to maximum dirt layerage
	float					m_fMinSpeedForDetail;		//!< Minimum speed the vehicle has to be travelling to accumulate detail dirt
	float					m_fDetailScale;				//!< Current detail dirt level

	bool					m_bEnabled;
};

} //namespace Vehicle

#endif
