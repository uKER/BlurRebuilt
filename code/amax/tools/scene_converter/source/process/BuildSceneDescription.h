#ifndef APP_BUILD_SCENE_DESCRIPTION_H
#define APP_BUILD_SCENE_DESCRIPTION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>

class C_ProcessBuildSceneDescription : public Tools::ProcessGraph::C_Process
{
public:
	// inherited from C_Process
	bool Process( Progress::I_ProgressBar* pProgress );

	// inherited from C_Process
	const Util::C_String GetName() const;

	bool CreateDefaultModel( ModelData::C_ModelData& modelData );

	ModelData::C_Model* GetDefaultModel();

private:
	bool BuildSceneDescription( C_DataSceneDescription* pSceneDescription, ModelData::C_ModelData* pModelData, ModelData::C_Model* pCurrModel, ModelData::C_ModelElement* pCurrElement, const Aftk::C_SceneNode* pAftkNode, const C_DataConversionOptions* pOptions );

	ModelData::C_Model* m_pDefaultModel;
};

#endif




