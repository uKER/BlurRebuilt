/////////////////////////////////////////////////////////////
// PartCodes
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_PART_CODES_H__
#define __VEHICLE_PART_CODES_H__

namespace Vehicle
{

class C_RenderLayer;

/* 
===============================================================
					enum E_PartCode

NOTES: Codes for all the parts on the vehicle
===============================================================
*/
enum E_PartCode
{
	E_PartCode_None,

	E_PartCode_Wheel,
	E_PartCode_Tyre,
	E_PartCode_AlloyRim,
	E_PartCode_Alloy,
	E_PartCode_AlloyBlur,
	E_PartCode_BrakeDisc,
	E_PartCode_BrakeCaliper,
	E_PartCode_WheelLod,

	E_PartCode_Bonnet,
	E_PartCode_Boot,
	E_PartCode_Door,
	E_PartCode_Wing,
	E_PartCode_Bumper,
	E_PartCode_Mirror,
	E_PartCode_Exhaust,
	E_PartCode_LicensePlate,

	E_PartCode_SteeringWheel,
	E_PartCode_HandBrake,
	E_PartCode_GearStick,
	E_PartCode_PaddleShift,
	E_PartCode_SignalStalk,
	
	E_PartCode_Window,
	E_PartCode_WindowInterior,
	E_PartCode_Wiper,

	E_PartCode_LightCluster,
	E_PartCode_HeadLight,
	E_PartCode_RearLight,
	E_PartCode_FogLight,
	E_PartCode_BrakeLight,
	E_PartCode_ReverseLight,
	E_PartCode_RunningLight,
	E_PartCode_SignalLight,
	E_PartCode_RearAndBrakeLight,
	E_PartCode_HeadlightCover,
	E_PartCode_FogLightCover,
	E_PartCode_RearLightCover,

	E_PartCode_Max,
};

/* 
===============================================================
					enum E_PartSubCode

NOTES: Positional codes for all the parts on the vehicle
===============================================================
*/
enum E_PartSubCode
{
	E_PartSubCode_None,

	//Ordered like this as is the order of the wheels in the physics
	E_PartSubCode_FrontLeft,
	E_PartSubCode_FrontRight,
	E_PartSubCode_BackLeft,
	E_PartSubCode_BackRight,

	E_PartSubCode_Left,
	E_PartSubCode_Right,
	E_PartSubCode_Centre,

	E_PartSubCode_Front,
	//E_PartSubCode_FrontLeft,
	//E_PartSubCode_FrontRight,

	E_PartSubCode_Back,
	//E_PartSubCode_BackLeft,
	//E_PartSubCode_BackRight,

	E_PartSubCode_Top,
	E_PartSubCode_TopLeft,
	E_PartSubCode_TopRight,

	E_PartSubCode_Bottom,
	E_PartSubCode_BottomLeft,
	E_PartSubCode_BottomRight,

	E_PartSubCode_Max,
};

struct PartNamesMap
{
	Util::C_String		m_Name;
	E_PartCode			m_Code;
};

extern const PartNamesMap s_PartNames[E_PartCode_Max - 1];

struct LocationNamesMap
{
	Util::C_String		m_Name;
	E_PartSubCode		m_Code;
};

const static int extraSubCodesForErrorCheck = 4;
#define NUM_PARTSUBCODE_STRINGS (E_PartSubCode_Max - 1 + extraSubCodesForErrorCheck)

extern const LocationNamesMap s_LocationNames[NUM_PARTSUBCODE_STRINGS];

} //namespace Vehicle

#endif
