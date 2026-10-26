#ifndef APP_SCENE_DESCRIPTION_H
#define APP_SCENE_DESCRIPTION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/SceneDesc.h>
#include <misc_shared/DataStructure/StringTable.h>
#include <btl/containers/Vector.h>
#include <converter_shared/ModelData.h>

//! C_DataSceneDescription

class C_DataSceneDescription : public Tools::ProcessGraph::C_Data
{
public:

	C_DataSceneDescription();

	~C_DataSceneDescription();

	// inherited from C_Data
	const Util::C_String GetName() const;

	ModelData::C_ModelData& GetModelData();

	const ModelData::C_ModelData& GetModelData() const;

	const bool ExportModelData( Sys::I_File* pFile ) const;

private:
	ModelData::C_ModelData	m_ModelData;
};


#endif