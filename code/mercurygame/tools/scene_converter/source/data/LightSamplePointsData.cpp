#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/LightSamplePointsData.h>
#include <misc_shared\Types\Float16.h>

C_DataLightSamplePoints::C_DataLightSamplePoints() : m_LightSampleList(16)
{

}

C_DataLightSamplePoints::~C_DataLightSamplePoints()
{
}

const Util::C_String C_DataLightSamplePoints::GetName() const
{
	return Util::C_String("C_DataLightSamplePoints");
}

enum E_LightSamplePointsFileVersion
{
	E_LightSamplePointsFileVersion_Unknown = -1,
	E_LightSamplePointsFileVersion_Initial = 0,
	E_LightSamplePointsFileVersion_Current = 1
};

enum E_LightSampleDataType
{
	E_LightSampleDataType_RGBSH_SunSkyFactor = 0,
	E_LightSampleDataType_SunSkySH = 1,
	E_LightSampleDataType_RGBSH = 2
};

bool C_DataLightSamplePoints::Read(const Util::C_String lightSamplePointsFile, const Util::C_String lightSampleDataFile, const Util::C_String lightModelName)
{
	if (!m_LightSampleList.ReadSamplePointsFile(lightSamplePointsFile))
		return false;
	if (!m_LightSampleList.ReadSampleDataFile(lightSampleDataFile, lightModelName))
		return false;

	//Compact SampleData
	m_LightSampleList.CompressSampleData();

	//Build Tree
	//Fill position array
	Btl::Vector<Maths::HVec3> positionVector;
	U32 numPositions = (U32) m_LightSampleList.m_SampleList.Size();

	if (numPositions > 0)
	{
		positionVector.Resize(numPositions);
		for (U32 i = 0; i < numPositions; i++)
			positionVector[i] = m_LightSampleList.m_SampleList[i].m_Position;

		//Build
		m_BSPPointTree.Build(32, &positionVector[0], numPositions);
	}

	m_LightModelName = lightModelName;

	return true;
}

bool C_DataLightSamplePoints::Export( Sys::I_File* pFile, const Util::E_Platform /*platform*/) const
{
	Sys::C_FileBlock rootBlock(pFile, "LS");

	{
		//Header
		Sys::C_FileBlock headerBlock(&rootBlock, "Header");
		Sys::BinarySerialiserAutoEndian ar( headerBlock );
		ar << S32(E_LightSamplePointsFileVersion_Current);
		
		if (m_LightModelName == "RGBSH_SunSkyFactor")
		{
			ar << S32(E_LightSampleDataType_RGBSH_SunSkyFactor);
		}
		else if (m_LightModelName == "RGBSH")
		{
			ar << S32(E_LightSampleDataType_RGBSH);
		}
		else
		{
			LOGERROR(Tools::g_log,"LightPointSampleData format unknown, update tools/data/scene_converter/config");
			ar << S32(E_LightSamplePointsFileVersion_Unknown);
			return true; 
		}
	}

	{
		Sys::C_FileBlock lightSamplePointsBlock(&rootBlock, "LSPoint");
		Sys::BinarySerialiserAutoEndian ar( lightSamplePointsBlock );
		//Light Sample Points
		ar << m_LightSampleList.m_SampleList.Size();
		for (S32 i = 0; i < m_LightSampleList.m_SampleList.Size(); i++)
		{
			//Convert to F16 //ar << m_LightSampleList.m_SampleList[i].m_Position;
			ar << Half::F16(m_LightSampleList.m_SampleList[i].m_Position.X()).m_U16;
			ar << Half::F16(m_LightSampleList.m_SampleList[i].m_Position.Y()).m_U16;
			ar << Half::F16(m_LightSampleList.m_SampleList[i].m_Position.Z()).m_U16;
			
			ar << m_LightSampleList.m_SampleList[i].m_SampleDataIndex;
		}
	}
	
	{
		Sys::C_FileBlock lightSampleDataBlock(&rootBlock, "LSData");
		Sys::BinarySerialiserAutoEndian ar( lightSampleDataBlock );

		//Light Sample Data
		ar << m_LightSampleList.m_SampleDataList.Size();

		for (S32 i = 0; i < m_LightSampleList.m_SampleDataList.Size(); i++)
		{
			if (m_LightModelName == "RGBSH_SunSkyFactor")
			{
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[0]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[1]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[2]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[3]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[4]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[5]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[6]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[7]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[8]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[9]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[10]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[11]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[12]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[13]).m_U16;
			}
			else if (m_LightModelName == "RGBSH")
			{
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[0]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[1]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[2]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[3]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[4]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[5]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[6]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[7]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[8]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[9]).m_U16;
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[10]).m_U16;	
				ar << Half::F16(m_LightSampleList.m_SampleDataList[i].m_Data[11]).m_U16;	
			}	
		}
	}
	
	{
		Sys::C_FileBlock lightSampleTreeBlock(&rootBlock, "LSTree");
		
		m_BSPPointTree.Export(&lightSampleTreeBlock);
	}
	

	

	return true;
}
