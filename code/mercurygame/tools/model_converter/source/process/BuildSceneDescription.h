#ifndef APP_BUILD_SCENE_DESCRIPTION_H
#define APP_BUILD_SCENE_DESCRIPTION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>

enum E_BuildSceneDescNodeType
{
	E_BuildSceneDescNodeType_Ignore = 0,
	E_BuildSceneDescNodeType_Mesh,
	E_BuildSceneDescNodeType_Element,
};

class C_ProcessBuildSceneDescription : public Tools::ProcessGraph::C_Process
{
public:
	C_ProcessBuildSceneDescription();
	~C_ProcessBuildSceneDescription();

	// inherited from C_Process
	bool Process( Progress::I_ProgressBar* pProgress );

	// inherited from C_Process
	const Util::C_String GetName() const;

private:

	//bool CreateDefaultModel( ModelData::C_ModelData& modelData );
	//ModelData::C_Model* GetDefaultModel();

	//ModelData::C_ModelElement* CreateDefaultElement( ModelData::C_Model* pModel );

	bool BuildSceneDescription( ModelData::C_ModelData* pModelData, ModelData::C_Model* pCurrModel, ModelData::C_ModelElement* pCurrElement, const Aftk::C_SceneNode* pAftkNode, const C_DataConversionOptions* pOptions );

private:
	//ModelData::C_Model* m_pDefaultModel;
};

#endif




