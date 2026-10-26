#ifndef APP_LIGHT_AFTK_H
#define APP_LIGHT_AFTK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>

namespace LuxInterface
{
	class C_LightingModel;
}

class C_ProcessLightAftk : public Tools::ProcessGraph::C_Process
{
	// inherited from C_Process
	bool Process( Progress::I_ProgressBar* pProgress );

	// inherited from C_Process
	const Util::C_String GetName() const;

	bool SetupLightingModel( LuxInterface::C_LightingModel* pLightingModel );

	bool RunLux( Progress::I_ProgressBar* pProgress );
};

#endif
