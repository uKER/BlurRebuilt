#ifndef APP_BUILD_ADVERT_DATA
#define APP_BUILD_ADVERT_DATA

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>

class C_ProcessBuildAdvertisingData : public Tools::ProcessGraph::C_Process
{
public:
	C_ProcessBuildAdvertisingData();

	// inherited from C_Process
	const Util::C_String GetName() const;

	bool IsAdPlacementNode(const Aftk::C_SceneNode *pAftkSceneNode);
	S32 GetAdPlacementIndex(const Aftk::C_SceneNode* pNode);

	bool Process( Progress::I_ProgressBar* pProgress );
};

#endif