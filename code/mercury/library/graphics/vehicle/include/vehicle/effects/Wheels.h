/////////////////////////////////////////////////////////////
// Wheels
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_WHEELS_H__
#define __VEHICLE_WHEELS_H__

namespace Vehicle
{

class C_WheelInput;
class C_Part;

/* 
===============================================================
					class C_WheelBlur

NOTES: 
===============================================================
*/
class C_WheelBlur
{
public:

	C_WheelBlur();

	//! Initialises the blur
	void Initialise(C_Part* p_pUnblurredSpokes, C_Part* p_pBlurredSpokes, C_WheelInput* p_pInput);

	//! Updates the blur
	void Update();

	//! Shutdown
	void Shutdown();

	//! Reset
	void Reset();

public:

	C_WheelInput*		m_pInput;					//!< Pointer to physics/model interface

	C_Part*	m_pUnblurredSpokes;			//!< Pointer to unblurred spokes
	C_Part*	m_pBlurredSpokes;			//!< Pointer to blurred spokes

	float				m_fScale;
	int					m_iCurrentState;
	bool				m_bUpdateMe;

	U32					m_SolidPass;
	U32					m_AlphaPass;
};

} //namespace Vehicle

#endif
