#ifndef BUILDPORTALDATA_H
#define BUILDPORTALDATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Portal/Builder.h>
#include <converter_shared/ProcessGraph.h>
#include <data/SceneDescription.h>
#include <data/PortalData.h>

class C_ProcessBuildPortalData : public Tools::ProcessGraph::C_Process
{
public:
	C_ProcessBuildPortalData();

	/// Inherited from C_Process
	const Util::C_String GetName() const;

	/// Process method for building the portal data.
	bool Process(Progress::I_ProgressBar* pProgress);

private:
	Portal::C_BuilderRoom*		m_pWorldRoom;				// The world room.

	/// Build rooms and portals from Aftk nodes.
	bool Build(Progress::I_ProgressBar* pProgress, const C_AftkNodeList& rkNodeList, C_DataPortalData* pOutputPortalData);
	/// Reduce unrequired tesselation in room and portal polygons.
	bool ReportCleanUpMeshes(Progress::I_ProgressBar* pProgress, const C_AftkNodeList& rkNodeList, C_DataPortalData* pOutputPortalData);
	/// Flatten polygons in rooms and portals as best as possible.
	bool FlattenPortalPolygons(Progress::I_ProgressBar* pProgress, C_DataPortalData* pOutputPortalData);
	/// Setup portals in rooms.
	bool SetupPortals(Progress::I_ProgressBar* pProgress, C_DataPortalData* pOutputPortalData);
	/// Compute containment data for rooms.
	bool BuildRoomContainmentData(Progress::I_ProgressBar* pProgress, C_DataPortalData* pOutputPortalData);
	/// Compute the room grid.
	bool BuildRoomGrid(Progress::I_ProgressBar* pProgress, C_DataPortalData* pOutputPortalData);
	/// Flatten portal polygons.
	const bool FlattenPortalPolygonWorker(C_DataPortalData* pOutputPortalData);
};

#endif // BUILDPORTALDATA_H
