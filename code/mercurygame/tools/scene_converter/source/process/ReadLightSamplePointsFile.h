#ifndef APP_READ_LIGHT_SAMPLE_POINTS_FILE_H
#define APP_READ_LIGHT_SAMPLE_POINTS_FILE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>

class C_ProcessReadLightSamplePointsFile : public Tools::ProcessGraph::C_Process
{
public:
	// inherited from C_Process
	bool Process( Progress::I_ProgressBar* pProgress );

	// inherited from C_Process
	const Util::C_String GetName() const;

private:

};

#endif




