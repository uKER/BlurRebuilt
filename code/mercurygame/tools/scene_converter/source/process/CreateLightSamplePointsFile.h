#ifndef APP_CREATE_LIGHT_SAMPLE_POINTS_FILE_H
#define APP_CREATE_LIGHT_SAMPLE_POINTS_FILE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>

namespace LuxSampleList
{
	class C_SampleList; 
}

class C_ProcessCreateLightSamplePointsFile : public Tools::ProcessGraph::C_Process
{
public:
	// inherited from C_Process
	bool Process( Progress::I_ProgressBar* pProgress );

	// inherited from C_Process
	const Util::C_String GetName() const;

private:
	bool AddSamplePoints( LuxSampleList::C_SampleList* pSampleList );

};

#endif




