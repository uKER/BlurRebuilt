#ifndef APP_LIGHT_SAMPLE_POINTS_DATA_H
#define APP_LIGHT_SAMPLE_POINTS_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/lux_interface/LuxSampleList.h>

class C_DataLightSamplePoints : public Tools::ProcessGraph::C_Data
{
public:
	
	C_DataLightSamplePoints();
	~C_DataLightSamplePoints();

	// export
	bool Export(Sys::I_File* pFile, const Tools::E_Platform platform ) const;

	// inherited from C_Data
	const Util::C_String GetName() const;

	bool Read(const Util::C_String lightSamplePointsFile, const Util::C_String lightSampleDataFile);

private:
	LuxSampleList::C_SampleList m_LightSampleList;

	LuxSampleList::C_BSPPointTree m_BSPPointTree;
};

#endif