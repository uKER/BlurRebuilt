#ifndef APP_PROGRESS_LIGHT_LIST_AFTK_H
#define APP_PROGRESS_LIGHT_LIST_AFTK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>

class C_ProcessLightList : public Tools::ProcessGraph::C_Process
{
public:
	// inherited from C_Process
	bool Process( Progress::I_ProgressBar* pProgress );

	// inherited from C_Process
	const Util::C_String GetName() const;

};

#endif




