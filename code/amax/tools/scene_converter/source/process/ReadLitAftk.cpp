#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/AftkHelpers.h>
#include <converter_shared/helpers/AftkProgress.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/AftkSceneData.h>

#include <process/ReadLitAftk.h>

bool C_ProcessReadLitAftk::Process( Progress::I_ProgressBar* pProgress )
{
	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	const Tools::ProcessGraph::C_Data* pLitAftkFile = GetInput( E_GraphData_LitAftkFile );
	const Tools::ProcessGraph::C_Data* pPreprocessedAftkFile = GetInput( E_GraphData_PreprocessedAftkFile );

	C_DataAftkScene* pOutputLitAftk = (C_DataAftkScene*)GetOutput( E_GraphData_LitAftk );
	assert(pOutputLitAftk);

	bool success = true;

	Util::C_String aftkFilename;
	if( pLitAftkFile )
	{
		aftkFilename = pInputConversionOptions->GetLitAftkFilename();
	}
	else
	if( pPreprocessedAftkFile )
	{
		aftkFilename = pInputConversionOptions->GetPreprocessedAftkFilename();
	}

	success = AftkHelpers::ReadAftkScene( pOutputLitAftk->GetAftkScene(), aftkFilename , pProgress );

	return success;
}

const Util::C_String C_ProcessReadLitAftk::GetName() const 
{
	return Util::C_String("C_ProcessReadLitAftk");
}
