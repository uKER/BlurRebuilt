#ifndef APP_CUBEMAP_POINTS_DATA_H
#define APP_CUBEMAP_POINTS_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared\KdTree\KdTree.h>
#include <converter_shared\renderdataconversion\CubemapInform.h>

struct CubemapZone
{
	Maths::HMat43 mtx;
	int iLocation;

	bool Intersects(const Maths::HVec3 PCREF vPos) const
	{
		Maths::HVec3 v = vPos * mtx;
		return (v.X() > -1.f && v.X() < 1.f &&
				v.Y() > -1.f && v.Y() < 1.f &&
				v.Z() > -1.f && v.Z() < 1.f);			
	}

	operator const Maths::C_AxisBox () const
	{
		Maths::C_AxisBox box;
		Maths::HMat44 _mtx2(mtx);
		Maths::HMat44 _mtx(_mtx2.Inverse());
		for (int i=0; i<8; i++)
			box.UpdateExtents(Maths::HVec3((i&1)?-1.f:1.f,(i&2)?-1.f:1.f,(i&4)?-1.f:1.f) * _mtx);
		return box;
	}
};

template<typename T>
void Serialise(T& ar, const CubemapZone& src)
{
	ar << src.mtx << src.iLocation;
}

template<typename T>
void Deserialise(T& ar, CubemapZone& src)
{
	ar >> src.mtx >> src.iLocation;
}

struct CubemapLocation
{
	Maths::HVec3 vPos;
	Util::C_String sName;
};

template<typename T>
void Serialise(T& ar, const CubemapLocation& src)
{
	ar << src.vPos << src.sName;
}

template<typename T>
void Deserialise(T& ar, CubemapLocation& src)
{
	ar >> src.vPos >> src.sName;
}

class C_DataCubemapPoints : public Tools::ProcessGraph::C_Data, public RenderDataConversion::I_CubemapInform
{
public:
	
	C_DataCubemapPoints();
	~C_DataCubemapPoints();

	/// add cubemap location
	int AddCubemapLocation(const Maths::HVec3 &vPos, const char *pszName);

	/// add cubemap zone
	void AddCubemapZone(const Maths::HMat44, int iCubemap);

	// export
	bool Export(Sys::I_File* pFile, const Util::E_Platform platform ) const;

	// inherited from C_Data
	const Util::C_String GetName() const;

	// returns the filename for the cubemap for the specified bounding box
	Util::C_String GetCubemapFilename(const Maths::C_AxisBox &bbox) const;

	/// set localised cubemap path
	void SetLocalisedCubemapPath(const char *psz);

	/// set default cubemap filename
	void SetDefaultCubemapFilename(const char *psz);

private:
	/// generate full cubemap pathname from a node name
	Util::C_String GenerateCubemapFilename(const char *psz) const;

	Btl::Vector<CubemapLocation> m_Locations;
	Util::KdTree<CubemapZone> m_Zones;

	Util::C_String m_sLocalisedCubemapPath, m_sDefaultCubemapFilename;
};

#endif
