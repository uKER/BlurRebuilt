/////////////////////////////////////////////////////////////
// Setup
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/CodeProcessor.h"
#include "Vehicle/PartCodes.h"

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

DEBUG_ENUM(Vehicle::E_PartCode)
DE_OPTION(Vehicle::E_PartCode_None,					"None")
DE_OPTION(Vehicle::E_PartCode_Wheel,				"Wheel")
DE_OPTION(Vehicle::E_PartCode_Tyre,					"Tyre")
DE_OPTION(Vehicle::E_PartCode_Alloy,				"Alloy")
DE_OPTION(Vehicle::E_PartCode_AlloyBlur,			"AlloyBlur")
DE_OPTION(Vehicle::E_PartCode_BrakeDisc,			"BrakeDisc")
DE_OPTION(Vehicle::E_PartCode_BrakeCaliper,			"BrakeCaliper")
DE_OPTION(Vehicle::E_PartCode_WheelLod,				"WheelLod")
DE_OPTION(Vehicle::E_PartCode_Bonnet,				"Bonnet")
DE_OPTION(Vehicle::E_PartCode_Boot,					"Boot")
DE_OPTION(Vehicle::E_PartCode_Door,					"Door")
DE_OPTION(Vehicle::E_PartCode_Wing,					"Wing")
DE_OPTION(Vehicle::E_PartCode_Bumper,				"Bumper")
DE_OPTION(Vehicle::E_PartCode_Mirror,				"Mirror")
DE_OPTION(Vehicle::E_PartCode_Exhaust,				"Exhaust")
DE_OPTION(Vehicle::E_PartCode_LicensePlate,			"LicensePlate")
DE_OPTION(Vehicle::E_PartCode_SteeringWheel,		"SteeringWheel")
DE_OPTION(Vehicle::E_PartCode_HandBrake,			"HandBrake")
DE_OPTION(Vehicle::E_PartCode_GearStick,			"GearStick")
DE_OPTION(Vehicle::E_PartCode_PaddleShift,			"PaddleShift")
DE_OPTION(Vehicle::E_PartCode_SignalStalk,			"SignalStalk")
DE_OPTION(Vehicle::E_PartCode_Window,				"Window")
DE_OPTION(Vehicle::E_PartCode_WindowInterior,		"WindowInterior")
DE_OPTION(Vehicle::E_PartCode_Wiper,				"Wiper")
DE_OPTION(Vehicle::E_PartCode_HeadLight,			"HeadLight")
DE_OPTION(Vehicle::E_PartCode_RearLight,			"RearLight")
DE_OPTION(Vehicle::E_PartCode_FogLight,				"FogLight")
DE_OPTION(Vehicle::E_PartCode_BrakeLight,			"BrakeLight")
DE_OPTION(Vehicle::E_PartCode_ReverseLight,			"ReverseLight")
DE_OPTION(Vehicle::E_PartCode_RunningLight,			"RunningLight")
DE_OPTION(Vehicle::E_PartCode_SignalLight,			"SignalLight")
DE_OPTION(Vehicle::E_PartCode_RearAndBrakeLight,	"RearAndBrakeLight")
DE_OPTION(Vehicle::E_PartCode_HeadlightCover,		"HeadLightCover")
DE_OPTION(Vehicle::E_PartCode_FogLightCover,		"FogLightCover")
DE_OPTION(Vehicle::E_PartCode_RearLightCover,		"RearLightCover")
END_DEBUG_ENUM

DEBUG_ENUM(Vehicle::E_PartSubCode)
DE_OPTION(Vehicle::E_PartSubCode_None,				"None")
DE_OPTION(Vehicle::E_PartSubCode_Left,				"Left")
DE_OPTION(Vehicle::E_PartSubCode_Right,				"Right")
DE_OPTION(Vehicle::E_PartSubCode_Centre,			"Centre")
DE_OPTION(Vehicle::E_PartSubCode_Front,				"Front")
DE_OPTION(Vehicle::E_PartSubCode_FrontLeft,			"FrontLeft")
DE_OPTION(Vehicle::E_PartSubCode_FrontRight,		"FrontRight")
DE_OPTION(Vehicle::E_PartSubCode_Back,				"Back")
DE_OPTION(Vehicle::E_PartSubCode_BackLeft,			"BackLeft")
DE_OPTION(Vehicle::E_PartSubCode_BackRight,			"BackRight")
DE_OPTION(Vehicle::E_PartSubCode_Top,				"Top")
DE_OPTION(Vehicle::E_PartSubCode_TopLeft,			"TopLeft")
DE_OPTION(Vehicle::E_PartSubCode_TopRight,			"TopRight")
DE_OPTION(Vehicle::E_PartSubCode_Bottom,			"Bottom")
DE_OPTION(Vehicle::E_PartSubCode_BottomLeft,		"BottomLeft")
DE_OPTION(Vehicle::E_PartSubCode_BottomRight,		"BottomRight")
END_DEBUG_ENUM

namespace Vehicle
{

extern const PartNamesMap s_PartNames[E_PartCode_Max - 1] =
{
	{"Wheel",			E_PartCode_Wheel},
	{"Tyre",			E_PartCode_Tyre},
	{"AlloyRim",		E_PartCode_AlloyRim},	
	{"Alloy",			E_PartCode_Alloy},
	{"AlloyBlur",		E_PartCode_AlloyBlur},
	{"BrakeDisc",		E_PartCode_BrakeDisc},
	{"BrakeCaliper",	E_PartCode_BrakeCaliper},
	{"WheelLod",		E_PartCode_WheelLod},	

	{"Bonnet",			E_PartCode_Bonnet},
	{"Boot",			E_PartCode_Boot},
	{"Door",			E_PartCode_Door},
	{"Wing",			E_PartCode_Wing},
	{"Bumper",			E_PartCode_Bumper},
	{"Mirror",			E_PartCode_Mirror},
	{"Exhaust",			E_PartCode_Exhaust},
	{"LicensePlate",	E_PartCode_LicensePlate},

	{"SteeringWheel",	E_PartCode_SteeringWheel},
	{"HandBrake",		E_PartCode_HandBrake},
	{"GearStick",		E_PartCode_GearStick},
	{"PaddleShift",		E_PartCode_PaddleShift},
	{"SignalStalk",		E_PartCode_SignalStalk},
	
	{"Window",			E_PartCode_Window},
	{"WindowInterior",	E_PartCode_WindowInterior},
	{"Wiper",			E_PartCode_Wiper},
	
	{"LightCluster",	E_PartCode_LightCluster},
	{"HeadLight",		E_PartCode_HeadLight},
	{"RearLight",		E_PartCode_RearLight},
	{"FogLight",		E_PartCode_FogLight},
	{"BrakeLight",		E_PartCode_BrakeLight},
	{"ReverseLight",	E_PartCode_ReverseLight},
	{"RunningLight",	E_PartCode_RunningLight},
	{"SignalLight",		E_PartCode_SignalLight},
	{"RearAndBrakeLight",		E_PartCode_RearAndBrakeLight},
	{"HeadlightCover",	E_PartCode_HeadlightCover},
	{"FoglightCover",	E_PartCode_FogLightCover},
	{"RearlightCover",	E_PartCode_RearLightCover},
};

extern const LocationNamesMap s_LocationNames[NUM_PARTSUBCODE_STRINGS] =
{
	{"Left",			E_PartSubCode_Left},
	{"Right",			E_PartSubCode_Right},
	{"Centre",			E_PartSubCode_Centre},
	{"Center",			E_PartSubCode_Centre},

	{"Front",			E_PartSubCode_Front},
	{"FrontLeft",		E_PartSubCode_FrontLeft},
	{"FrontRight",		E_PartSubCode_FrontRight},

	{"Back",			E_PartSubCode_Back},
	{"BackLeft",		E_PartSubCode_BackLeft},
	{"BackRight",		E_PartSubCode_BackRight},
	{"Rear",			E_PartSubCode_Back},
	{"RearLeft",		E_PartSubCode_BackLeft},
	{"RearRight",		E_PartSubCode_BackRight},

	{"Top",				E_PartSubCode_Top},
	{"TopLeft",			E_PartSubCode_TopLeft},
	{"TopRight",		E_PartSubCode_TopRight},

	{"Bottom",			E_PartSubCode_Bottom},
	{"BottomLeft",		E_PartSubCode_BottomLeft},
	{"BottomRight",		E_PartSubCode_BottomRight},
};

C_CodeProcessor C_CodeProcessor::s_Instance;

/* 
===============================================================
					class C_CodeProcessor

NOTES: 
===============================================================
*/

/*
===============
NAME: C_CodeProcessor

NOTE: Constructor
===============
*/
C_CodeProcessor::C_CodeProcessor()
{

}

/*
===============
NAME: ~C_CodeProcessor

NOTE: Destructor
===============
*/
C_CodeProcessor::~C_CodeProcessor()
{
	m_PartNamesMap.Clear();
	m_LocationNamesMap.Clear();
}

/*
===============
NAME: BuildNameMaps

NOTE: Builds the maps for part/location codes and names
===============
*/
void C_CodeProcessor::BuildNameMaps()
{
	m_PartNamesMap.Clear();
	for (int iPartCode = 0; iPartCode < E_PartCode_Max-1; iPartCode++)
		m_PartNamesMap.Insert(s_PartNames[iPartCode].m_Name.Lower(), s_PartNames[iPartCode].m_Code);

	m_LocationNamesMap.Clear();
	for (int iLocationCode = 0; iLocationCode < NUM_PARTSUBCODE_STRINGS; iLocationCode++)
		m_LocationNamesMap.Insert(s_LocationNames[iLocationCode].m_Name.Lower(), s_LocationNames[iLocationCode].m_Code);
}

/*
===============
NAME: Clear

NOTE: Clear
===============
*/
void C_CodeProcessor::Clear()
{
	m_PartNamesMap.Clear();
	m_LocationNamesMap.Clear();
}

/*
===============
NAME: GetCodes

NOTE: Get part and location codes from the model/element name
===============
*/
void C_CodeProcessor::GetCodes(Util::C_String p_Name, 
							   E_PartCode& p_PartCode, 
							   E_PartSubCode& p_LocCode)
{
	p_Name.ConvertToLower();

	char name[MAX_CHARS];
	_snprintf(name, lengthof(name), p_Name.Chars());

	S32 length = p_Name.Length();

	if (length > 2)
	{
		if (name[0] == '_')
		{
			_snprintf(name, length, &name[1]);
			length -= 1;
			p_Name = Util::C_String(name);
		}
		else if (name[1] == '_')
		{
			_snprintf(name, length - 1, &name[2]);
			length -= 2;
			p_Name = Util::C_String(name);
		}
	}

	if (length == 0)
		return;

	S32 uPos = p_Name.Search("_");

	if (uPos != -1 && (uPos+1) < length)
	{
		char locationName[MAX_CHARS];
		_snprintf(locationName, lengthof(locationName), &name[uPos+1]);
		Util::C_String locationString(locationName);

		for (	Btl::Map<Util::C_String, E_PartSubCode>::ConstIterator it = m_LocationNamesMap.Begin();
				it != m_LocationNamesMap.End();
				++it)
		{
			if (it->GetKey() == locationString)
				p_LocCode = it->GetValue();
		}

		length -= length - uPos;
	}

	char partName[MAX_CHARS];
	_snprintf(partName, length+1, name);
	partName[length] = 0;
	Util::C_String partString(partName);

	for (	Btl::Map<Util::C_String, E_PartCode>::ConstIterator it = m_PartNamesMap.Begin();
			it != m_PartNamesMap.End();
			++it)
	{
		if (it->GetKey() == partString)
			p_PartCode = it->GetValue();
	}
}

} //namespace Vehicle

