/////////////////////////////////////////////////////////////
// CodeProcessor
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_CODE_PROCESSOR_H__
#define __VEHICLE_CODE_PROCESSOR_H__

#include <btl/containers/map.h>

#include <Vehicle/PartCodes.h>

namespace Vehicle
{

/* 
===============================================================
					class C_CodeProcessor

NOTES: 
===============================================================
*/
class C_CodeProcessor
{
public:

	static C_CodeProcessor& Instance() { return s_Instance; }

	C_CodeProcessor();
	~C_CodeProcessor();

	//! Builds the maps for part/location codes and names
	void BuildNameMaps();

	//! Clear
	void Clear();

	//! Get part and location codes from the model/element name
	void GetCodes(Util::C_String p_Name, E_PartCode& p_PartCode, E_PartSubCode& p_LocCode);

public:

	static C_CodeProcessor						s_Instance;

	Btl::Map<Util::C_String, E_PartCode>		m_PartNamesMap;
	Btl::Map<Util::C_String, E_PartSubCode>	m_LocationNamesMap;
};

} //namespace Vehicle

#endif
