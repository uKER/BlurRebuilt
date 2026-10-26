#include "stdafx.h"

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
	bool success = AftkHelpers::WriteAftkScene( pInputPreprocessedAftk->GetAftkScene(), preprocessedAftkFilename , pProgress );

	return success;
}

const Util::C_String C_ProcessWritePreprocessedAftk::GetName() const 
{
	return Util::C_String("C_ProcessWritePreprocessedAftk");
}
