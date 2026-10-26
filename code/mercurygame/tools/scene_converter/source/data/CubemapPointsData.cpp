#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/CubemapPointsData.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>


C_DataCubemapPoints::C_DataCubemapPoints()
{
}

C_DataCubemapPoints::~C_DataCubemapPoints()
{
}

/// set localised cubemap path
void C_DataCubemapPoints::SetLocalisedCubemapPath(const char *psz)
{
	m_sLocalisedCubemapPath = psz;
}

/// set default cubemap filename
void C_DataCubemapPoints::SetDefaultCubemapFilename(const char *psz)
{
	m_sDefaultCubemapFilename = psz;
}

/// add cubemap zone
void C_DataCubemapPoints::AddCubemapZone(const Maths::HMat44 mtx, int iCubemap)
{
	CubemapZone item;
	item.mtx = (Maths::HMat43)mtx.Inverse();
	item.iLocation = iCubemap;
	m_Zones.PushBack(item);
}

/// add cubemap location
int C_DataCubemapPoints::AddCubemapLocation(const Maths::HVec3 &vPos, const char *pszName)
{
	CubemapLocation item;
	item.vPos = vPos;
	item.sName = pszName;
	m_Locations.PushBack(item);
	return m_Locations.Size()-1;
}

/// export
bool C_DataCubemapPoints::Export(Sys::I_File* pFile, const Util::E_Platform platform ) const
{
	Sys::C_FileBlock cubemapPointsBlk(pFile, "CubePt2");

	Sys::BinarySerialiserAutoEndian ar( cubemapPointsBlk );

	ar << m_Locations;
	ar << m_Zones;
	ar << m_sLocalisedCubemapPath << m_sDefaultCubemapFilename;

	return true;
}

/// inherited from C_Data
const Util::C_String C_DataCubemapPoints::GetName() const
{
	return "CubemapPoints";
}

/// generate full cubemap pathname from a node name
Util::C_String C_DataCubemapPoints::GenerateCubemapFilename(const char *psz) const
{
	return m_sLocalisedCubemapPath + "/" + psz;
}

/// returns the filename for the cubemap for the specified bounding box
Util::C_String C_DataCubemapPoints::GetCubemapFilename(const Maths::C_AxisBox &bbox) const
{
	int results[4];
	// intersect against ABB intead of centre
	int nResults = m_Zones.Intersect(bbox.Centre(), results, 4);
	if (!nResults)
		return m_sDefaultCubemapFilename;

	// return closest cubemap
	return GenerateCubemapFilename(m_Locations[m_Zones[results[0]].iLocation].sName + ".dds");
}
