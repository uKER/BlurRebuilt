#ifndef APP_LIGHT_AFTK_H
#define APP_LIGHT_AFTK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>

namespace Aftk
{
	class C_Scene;
};

namespace LuxInterface
{
	class C_LightingModel;
	class C_Pass;
}

class C_ProcessLightAftk : public Tools::ProcessGraph::C_Process
{
	// inherited from C_Process
	bool Process( Progress::I_ProgressBar* pProgress );

	// inherited from C_Process
	const Util::C_String GetName() const;

	bool SetupLightingModel( LuxInterface::C_LightingModel* pLightingModel );

	void AddSpecialLayersToPass(LuxInterface::C_Pass* pPass, const Aftk::C_Scene* pAftkScene );

	bool RunLux( Progress::I_ProgressBar* pProgress );

	bool LightUpToDate(const C_DataConversionOptions* pInputConversionOptions, const LuxInterface::C_LightingModel& setupLightModel) const;
};

#endif
