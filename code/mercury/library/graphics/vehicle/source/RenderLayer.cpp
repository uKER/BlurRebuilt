/////////////////////////////////////////////////////////////
// RenderLayer
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/RenderLayer.h"

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

/* 
===============================================================
					class C_RenderLayer

NOTES: Completely self contained render layer, doesn't know anything else exists, and doesn't give a flying fudge. 
===============================================================
*/

/*
===============
NAME: C_RenderLayer

NOTE: Constructor
===============
*/
C_RenderLayer::C_RenderLayer()
{
	m_pScene		= NULL;

	m_bCreated		= false;
}

/*
===============
NAME: ~C_RenderLayer

NOTE: Constructor
===============
*/
C_RenderLayer::~C_RenderLayer()
{
	m_bCreated		= false;
}

/*
===============
NAME: Render

NOTE: Display using the vehicle renderer
===============
*/
void C_RenderLayer::Render()
{

}

/*
===============
NAME: Create

NOTE: Creates layer
===============
*/
bool C_RenderLayer::Create()
{
	return true;
}

/*
===============
NAME: Destroy

NOTE: Destroys layer
===============
*/
bool C_RenderLayer::Destroy()
{
	m_bCreated = false;

	return true;
}

/*
===============
NAME: Update

NOTE: Updates layer
===============
*/
void C_RenderLayer::Update()
{
	
}

/*
===============
NAME: SetPartVisible

NOTE: Sets visibility of a part
===============
*/
void C_RenderLayer::SetPartVisible(U8 p_ePartCode, U8 p_ePositionCode, bool p_bVisible)
{
	return;
}

} //namespace Vehicle

