#include "stdafx.h"
#include <file_tag_db/FileTagDB.h>
#include <tools_shared/utility/Misc.h>
#include <tools_shared/utility/AssetMeta.h>

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/AftkHelpers.h>
#include <converter_shared/helpers/AftkProgress.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/AftkSceneData.h>

#include <process/WritePreprocessedAftk.h>

bool C_ProcessWritePreprocessedAftk::Process( Progress::I_ProgressBar* pProgress )
{
	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	const C_DataAftkScene* pInputPreprocessedAftk = (C_DataAftkScene*)GetInput( E_GraphData_PreprocessedAftk );
	assert(pInputPreprocessedAftk);

	Util::C_String preprocessedAftkFilename = pInputConversionOptions->GetPreprocessedAftkFilename();

	bool success = true;
	//Errors in scene?
	if (pInputPreprocessedAftk->Valid())
	{
		//Write scene.
		success = AftkHelpers::WriteAftkScene(
			pInputPreprocessedAftk->GetAftkScene(), preprocessedAftkFilename, pProgress );

		Ut::Vector<FileTag::C_MetaTag::C_Input> inputs;
		inputs.PushBack();
		FileTag::C_MetaTag::C_Input& input = inputs.Back();
		input.file = pInputConversionOptions->GetSourceAftkFilename();
		input.fileTime = Tools::GetFileTime(pInputConversionOptions->GetSourceAftkFilename().Chars());

		//Store meta-data.
		FileTag::C_MetaTag metaTag;
		if (FileTag::C_MetaManager::Instance().AddTag(metaTag, preprocessedAftkFilename))
		{
			metaTag.SetBuildTime(Tools::GetCurrentExeTime());
			metaTag.SetInputs(inputs);
		}

		//Flush databases to disk.
		FileTag::C_MetaManager::Instance().Write();
	}

	return success;
}

const Util::C_String C_ProcessWritePreprocessedAftk::GetName() const 
{
	return Util::C_String("C_ProcessWritePreprocessedAftk");
}
