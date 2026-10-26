#ifndef APP_CREATE_PVS_SAMPLE_POINTS_FILE_H
#define APP_CREATE_PVS_SAMPLE_POINTS_FILE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/pvs/pvs.h>

class C_ProcessCreatePvsSamplePointsFile : public Tools::ProcessGraph::C_Process
{
public:
	// inherited from C_Process
	bool Process( Progress::I_ProgressBar* pProgress );

	// inherited from C_Process
	const Util::C_String GetName() const;

private:
	void AddPVSZones(const Aftk::C_SceneNode* pAftkNode);
	Pvs::Build::PvsData m_pvsData;
};

#endif




