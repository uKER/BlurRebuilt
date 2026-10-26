#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/AftkHelpers.h>
#include <converter_shared/helpers/AftkProgress.h>

#include <data/DataTypes.h>
#include <data/PvsData.h>
#include <data/AftkSceneData.h>

#include <data/ConversionOptions.h>

#include <process/ReadPvsData.h>

#include <process/CarPVSHacks.h>

bool C_ProcessReadPvsData::Process( Progress::I_ProgressBar* pProgress )
{
	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	C_DataPvs* pDataPVS = (C_DataPvs*)GetOutput( E_GraphData_PvsData );
	assert(pDataPVS);
	
	//Fill pvs structure in DataPVS
	pDataPVS->Read(pInputConversionOptions->GetPVSDataFilename(), pInputConversionOptions->GetVisOutputFilename());

	if (pInputConversionOptions->GetModelType().Lower() == "vehicle")
	{
		// hack the PVS data slightly to account for PVS problems on damaged cars
		const C_DataAftkScene* pInputLitAftk = (C_DataAftkScene*)GetInput( E_GraphData_LitAftk );
		if (!pInputLitAftk)
			pInputLitAftk = (C_DataAftkScene*)GetInput( E_GraphData_PreprocessedAftk );
		ApplyCarPVSHacks(&pInputLitAftk->GetAftkScene(), pDataPVS );
	}

	bool success = true;
	return success;
}

const Util::C_String C_ProcessReadPvsData::GetName() const 
{
	return Util::C_String("C_ProcessReadPvsData");
}
