#ifndef APP_PREPROCESS_AFTK_H
#define APP_PREPROCESS_AFTK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>

class C_ProcessPreprocessAftk : public Tools::ProcessGraph::C_Process
{
public:
	// inherited from C_Process
	bool Process( Progress::I_ProgressBar* pProgress );

	// inherited from C_Process
	const Util::C_String GetName() const;

private:
	bool ProcessAftk( Aftk::C_Scene& aftkScene, Progress::I_ProgressBar* pProgress );
};

#endif




