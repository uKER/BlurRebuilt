#ifndef CONVERTER_SHARED_VIS_INTERFACE_INL
#define CONVERTER_SHARED_VIS_INTERFACE_INL

#include <converter_shared/vis_interface/VisInterface.h>

namespace VisInterface
{

// C_SamplePoint

template <typename Archive>
void C_SamplePoint::Serialise( Archive& ar ) const 
{
	//This should serialise in the correct format for the PVS tool
	ar << (S32)m_Type;
	ar << m_Pos;
	ar << m_Dir;
}

template <typename Archive>
void C_SamplePoint::Deserialise( Archive& ar ) 
{
	S32 iType;
	ar >> iType;
	m_Type = (E_SamplePointType)iType;

	ar >> m_Pos;
	ar >> m_Dir;
}

// C_Zone

template <typename Archive>
void C_Zone::Serialise( Archive& ar ) const 
{
	ar << m_Matrix;

	const S32 size = m_SamplePointList.Size();
	ar << size;

	for (S32 i=0; i< size; i++)
	{
		ar << m_SamplePointList[i];
	}
}

template <typename Archive>
void C_Zone::Deserialise( Archive& ar ) 
{
	ar << m_Matrix;

	S32 size;
	ar >> size;

	m_SamplePointList.Resize(size);

	for (S32 i=0; i< size; i++)
	{
		ar >> m_SamplePointList[i];
	}
}

// C_VisData

template <typename Archive>
void C_VisData::Serialise( Archive& ar ) const
{
}

template <typename Archive>
void C_VisData::Deserialise( Archive& ar )
{
}

} // VisInterface

#endif