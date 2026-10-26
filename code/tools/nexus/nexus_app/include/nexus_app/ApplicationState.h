#ifndef NEXUS_APPLICATION_STATE_H
#define NEXUS_APPLICATION_STATE_H

namespace Nexus
{
	struct C_ApplicationState
	{
		C_ApplicationState()
		{
			showAnimationDetails = true;
			showPolyCount = true;
			showCameraDetails = true;
			selectionMode = E_SelectionMode_Object;
			showFaceInfoOnSelection = true;
			showObjectInfoOnSelection = true;
		}

		bool showAnimationDetails;
		bool showPolyCount;
		bool showCameraDetails;
		bool showFaceInfoOnSelection;
		bool showObjectInfoOnSelection;

		E_SelectionMode selectionMode;
	};
};

#endif
