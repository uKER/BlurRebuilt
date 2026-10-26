#ifndef APP_BUILD_CUBEMAP_POINTS_DATA_H__
#define APP_BUILD_CUBEMAP_POINTS_DATA_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>

class C_ProcessBuildCubemapPoints : public Tools::ProcessGraph::C_Process
{
public:
	/// constructor
	C_ProcessBuildCubemapPoints();

	/// find localised cubemap render locations and cubemap zones and add them into the localised cubemap data
	bool Process( Progress::I_ProgressBar* pProgress );

	// inherited from C_Process
	const Util::C_String GetName() const;

private:
	/// for a given cubemap location, find all the zones associated with it
	/// (association is done by making the zone a child of the cubemap location in maya)
	void ProcessCubemapZones(C_DataCubemapPoints *pData, Aftk::C_SceneNode *pNode, int iCubemap);
};

#endif


