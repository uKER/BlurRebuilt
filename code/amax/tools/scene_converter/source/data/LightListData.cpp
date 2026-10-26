#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/LightListData.h>

C_DataLightList::C_DataLightList()
{
	m_HasSkyLight = false;
	m_HasSunLight = false;
}

C_DataLightList::~C_DataLightList()
{
}

const Util::C_String C_DataLightList::GetName() const
{
	return Util::C_String("C_DataLightList");
}

enum E_LightListFileVersion
{
	E_LightListFileVersion_Initial = 0,
	E_LightListFileVersion_Current = E_LightListFileVersion_Initial
};

bool C_DataLightList::Export( Sys::I_File* pFile, const Tools::E_Platform /*platform*/) const
{
	Sys::C_FileBlock lightListBlock(pFile, "Lights");

	Sys::BinarySerialiserAutoEndian ar( lightListBlock );

    ar << S32(E_LightListFileVersion_Current);

	if (m_HasSunLight)
	{
		ar << U32(1);
		ar << m_SunLight;
	}
	else
	{
		ar << U32(0);
	}

	if (m_HasSkyLight)
	{
		ar << U32(1);
		ar << m_SkyLight;
	}
	else
	{
		ar << U32(0);
	}

	U32 noLights = m_LightList.Size();
	ar << noLights;
	
	for (S32 i = 0; i < m_LightList.Size(); i++)
	{
		const C_Light& light = m_LightList[i];

		ar << light;
	}

	return true;
}
