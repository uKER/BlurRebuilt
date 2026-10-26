#ifndef CONVERTER_SHARED_VIS_INTERFACE_H
#define CONVERTER_SHARED_VIS_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>

namespace VisInterface
{

enum E_SamplePointType
{
	E_SamplePointType_Position = 1,
	E_SamplePointType_Direction = 2,
};

// C_SamplePoint
class C_SamplePoint
{
public:
	C_SamplePoint();
	~C_SamplePoint();

	template <typename Archive>
	void Serialise( Archive& ar ) const;

	template <typename Archive>
	void Deserialise( Archive& ar );

private:
	E_SamplePointType m_Type;
	Maths::HVec3 m_Pos;
	Maths::HVec3 m_Dir;
};

// C_Zone
class C_Zone
{
public:
	C_Zone();
	~C_Zone();


	template <typename Archive>
	void Serialise( Archive& ar ) const;

	template <typename Archive>
	void Deserialise( Archive& ar );

private:
	Btl::Vector<C_SamplePoint> m_SamplePointList;
};

// C_VisData
class C_VisData
{
public:
	C_VisData();
	~C_VisData();

	template <typename Archive>
	void Serialise( Archive& ar ) const;

	template <typename Archive>
	void Deserialise( Archive& ar );

private:
	Btl::Vector<C_Zone> m_ZoneList;
};

} // VisInterface

#endif