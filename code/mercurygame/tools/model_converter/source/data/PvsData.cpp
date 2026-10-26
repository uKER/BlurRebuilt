#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/PvsData.h>


C_DataPvs::C_DataPvs()
{

}

C_DataPvs::~C_DataPvs()
{
}

const Util::C_String C_DataPvs::GetName() const
{
	return Util::C_String("C_DataPvs");
}

void C_DataPvs::Read( Util::C_String pvsDataFile, Util::C_String dataFromPvsTool )
{
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(pvsDataFile, Sys::E_FileAccess_Read);
	if (pFile)
	{
		Arch::BinaryInputArchive ar(*pFile);
		ar >> m_pvsData;
		delete pFile;
	}

	m_pvsData.ReadDataFromPvsTool(dataFromPvsTool);
}

bool C_DataPvs::Export( Sys::I_File* pFile, const Util::E_Platform /*platform*/ ) const
{
	Sys::C_FileBlock pvsDataBlock(pFile, "PVSData");

	Arch::BinaryOutputArchive ar( pvsDataBlock );

	ar << m_pvsData;

	return true;
}