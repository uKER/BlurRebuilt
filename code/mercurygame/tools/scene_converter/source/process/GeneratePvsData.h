#ifndef APP_GENERATE_PVS_DATA_H
#define APP_GENERATE_PVS_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>

class C_ProcessGeneratePvsData : public Tools::ProcessGraph::C_Process
{
	// inherited from C_Process
	bool Process( Progress::I_ProgressBar* pProgress );

	// inherited from C_Process
	const Util::C_String GetName() const;
};

#endif
