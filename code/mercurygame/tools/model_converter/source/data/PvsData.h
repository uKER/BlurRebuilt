#ifndef PVS_DATA_H
#define PVS_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/pvs/pvs.h>

class C_DataPvs : public Tools::ProcessGraph::C_Data
{
public:
	
	C_DataPvs();
	~C_DataPvs();

	// inherited from C_Data
	const Util::C_String GetName() const;

	bool Export(Sys::I_File* pFile, const Util::E_Platform platform ) const;

	//Read
	void Read(Util::C_String pvsDataFile, Util::C_String dataFromPvsTool );

	//Get
	const Pvs::Build::PvsData& GetPVSData() const  { return m_pvsData;};
	

private:
	Pvs::Build::PvsData m_pvsData;
};

#endif