#ifndef APP_BUILD_NAVMESH_DATA_H
#define APP_BUILD_NAVMESH_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>

class C_ProcessBuildNavMeshData : public Tools::ProcessGraph::C_Process
{
public:
	C_ProcessBuildNavMeshData();

	// inherited from C_Process
	const Util::C_String GetName() const;

	bool Process( Progress::I_ProgressBar* pProgress );

private:
};

#endif




