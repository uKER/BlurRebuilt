#ifndef EXAMPLE_EXPORT_DATA_H
#define EXAMPLE_EXPORT_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

class C_ExampleExportData
{
public:
	INLINE C_ExampleExportData();

	INLINE bool Serialise(Sys::I_File& file) const;

	INLINE bool Deserialise(Sys::I_File& file);

private:
	static const U32 m_currentFileVersion = 100;
	const char* m_mainBlockID;
};



C_ExampleExportData::C_ExampleExportData()
{
	m_mainBlockID = "MainBlk";
}

bool C_ExampleExportData::Serialise(Sys::I_File& file) const
{
	//Block header for file.
	Sys::C_FileBlock packageBlock(&file, m_mainBlockID);
	Sys::BinarySerialiserAutoEndian ar(packageBlock);

	//Serialise main block contents.
	{
		ar << m_currentFileVersion;
		//Write out other things here.
	}

	return true;
}

bool C_ExampleExportData::Deserialise(Sys::I_File& file)
{
	//Block header for file.
	Sys::C_FileBlock mainBlock(&file);
	if (strcmp(mainBlock.GetBlockHeader().m_Id, m_mainBlockID))
		//Wrong block ID.
		return false;

	//Deserialise main block contents.
	{
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(mainBlock);
		U32 currentFileVersion;
		ar >> currentFileVersion;
		file.SetVersionNumber(currentFileVersion);
		//Unsupported version.
		if (currentFileVersion > m_currentFileVersion)
			return false;

		//Read in other things here.
	}

	return true;
}

#endif
