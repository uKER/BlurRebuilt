#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/LightSamplePointsData.h>

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
	E_LightSamplePointsFileVersion_Initial = 0,
	E_LightSamplePointsFileVersion_Current = E_LightSamplePointsFileVersion_Initial
};

bool C_DataLightSamplePoints::Read(const Util::C_String lightSamplePointsFile, const Util::C_String lightSampleDataFile)
{
	if (!m_LightSampleList.ReadSamplePointsFile(lightSamplePointsFile))
		return false;
	if (!m_LightSampleList.ReadSampleDataFile(lightSampleDataFile))
		return false;

	//Compact SampleData (Not working)
	//m_LightSampleList.CompressSampleData();

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
	return true;
}

bool C_DataLightSamplePoints::Export( Sys::I_File* pFile, const Tools::E_Platform /*platform*/) const
{
	Sys::C_FileBlock rootBlock(pFile, "LS");

	{
		//Header
		Sys::C_FileBlock headerBlock(&rootBlock, "Header");
		Sys::BinarySerialiserAutoEndian ar( headerBlock );
		ar << S32(E_LightSamplePointsFileVersion_Current);
	}

	{
		Sys::C_FileBlock lightSamplePointsBlock(&rootBlock, "LSPoint");
		Sys::BinarySerialiserAutoEndian ar( lightSamplePointsBlock );
		//Light Sample Points
		ar << m_LightSampleList.m_SampleList.Size();
		for (S32 i = 0; i < m_LightSampleList.m_SampleList.Size(); i++)
		{
			ar << m_LightSampleList.m_SampleList[i].m_Position;
			ar << m_LightSampleList.m_SampleList[i].m_Index;
		}
	}
	
	{
		Sys::C_FileBlock lightSampleDataBlock(&rootBlock, "LSData");
		Sys::BinarySerialiserAutoEndian ar( lightSampleDataBlock );

		ar << m_LightSampleList.m_SampleDataList.Size();
		for (S32 i = 0; i < m_LightSampleList.m_SampleDataList.Size(); i++)
		{
			for (S32 j = 0; j < 16; j++)
				ar << m_LightSampleList.m_SampleDataList[i].m_Data[j];
		}
	}
	
	{
		Sys::C_FileBlock lightSampleTreeBlock(&rootBlock, "LSTree");
		
		m_BSPPointTree.Export(&lightSampleTreeBlock);
	}
	

	

	return true;
}
