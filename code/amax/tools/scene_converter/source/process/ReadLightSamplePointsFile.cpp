#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/lux_interface/LuxSampleList.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/AftkSceneData.h>

#include <process/ReadLightSamplePointsFile.h>
#include <data/LightSamplePointsData.h>

bool C_ProcessReadLightSamplePointsFile::Process( Progress::I_ProgressBar* pProgress )
{
	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);
	
	C_DataLightSamplePoints* pInputLightSamplePoints = (C_DataLightSamplePoints*)GetOutput( E_GraphData_LightSamples );
	assert(pInputConversionOptions);
	
	return pInputLightSamplePoints->Read(pInputConversionOptions->GetLightSamplePointsFilename(), pInputConversionOptions->GetLightSamplesFilename());
}

const Util::C_String C_ProcessReadLightSamplePointsFile::GetName() const 
{
	return Util::C_String("C_ProcessReadLightSamplePointsFile");
}
