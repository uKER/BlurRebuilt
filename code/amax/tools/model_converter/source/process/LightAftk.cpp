#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <converter_shared/lux_interface/LuxInterface.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>

#include <process/LightAftk.h>

bool C_ProcessLightAftk::Process( Progress::I_ProgressBar* pProgress )
{
	bool success = true;
	Progress::UpdateSimple progressSimple(pProgress, GetName());
		
	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);
	
	Util::C_String defaultLightingModelFilename = pInputConversionOptions->GetToolConfig().GetLuxBaseConfigFilename();
	Util::C_String lightingModelFilename = pInputConversionOptions->GetLightingModelFilename();

	LuxInterface::C_LightingModel lightingModel;
	success = lightingModel.ReadXml( defaultLightingModelFilename );
	if(!success) return false;

	SetupLightingModel( &lightingModel );

	success = lightingModel.WriteXml( lightingModelFilename );
	if(!success) return false;

	success = RunLux( pProgress );
	if(!success) return false;

	return true;
}

const Util::C_String C_ProcessLightAftk::GetName() const 
{
	return Util::C_String("C_ProcessLightAftk");
}

bool C_ProcessLightAftk::SetupLightingModel( LuxInterface::C_LightingModel* pLightingModel )
{
	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);
	
	const C_LightingOptions& lightingOptions = pInputConversionOptions->GetLightingOptions();

	F32 texelDensity = 0.3f;
	F32 alternateTexelDensityRatio = 128.0f;

	/*
	switch( lightingOptions.GetLightmapResolution() )
	{
		case E_LightmapResolution_Low:
			// TODO: read values from tools config xml file
			texelDensity = 0.3f;
			alternateTexelDensityRatio = 32;
		break;

		case E_LightmapResolution_Medium:
			// TODO: read values from tools config xml file
			texelDensity = 0.3f;
			alternateTexelDensityRatio = 64;
		break;

		case E_LightmapResolution_High:
			// TODO: read values from tools config xml file
			texelDensity = 0.3f;
			alternateTexelDensityRatio = 128;
		break;

		case E_LightmapResolution_VeryHigh:
			// TODO: read values from tools config xml file
			texelDensity = 0.6f;
			alternateTexelDensityRatio = 256;
		break;
	}
	*/

	U32 noSunSamples = 8;
	U32 noSkySamples = 256;
	F32 sunPenumbra = 0.999961f;

	/*
	switch( lightingOptions.GetLightmapQuality() )
	{
		case E_LightmapQuality_Low:
			// TODO: read values from tools config xml file
			noSunSamples = 1;
			noSkySamples = 64;
			sunPenumbra = 1.0f;
		break;

		case E_LightmapQuality_Medium:
			// TODO: read values from tools config xml file
			noSunSamples = 4;
			noSkySamples = 128;			
			sunPenumbra = 0.999961f;
		break;

		case E_LightmapQuality_High:
			// TODO: read values from tools config xml file
			noSunSamples = 8;
			noSkySamples = 256;	
			sunPenumbra = 0.999961f;
		break;

		case E_LightmapQuality_VeryHigh:
			// TODO: read values from tools config xml file
			noSunSamples = 16;
			noSkySamples = 512;	
			sunPenumbra = 0.999961f;
		break;
	}
	*/

	// Setup lighting model settings
	pLightingModel->m_LightingModelProperties.m_TexelDensity = texelDensity;
	pLightingModel->m_LightingModelProperties.m_AlternateTexelDensityRatio = alternateTexelDensityRatio;
	pLightingModel->m_LightingModelProperties.m_InputGamma = 2.2f;
	pLightingModel->m_LightingModelProperties.m_OutputGamma = 2.2f;
	pLightingModel->m_LightingModelProperties.m_AlphaScaling = LuxInterface::E_On;
	pLightingModel->m_LightingModelProperties.m_AntiLightLeaking = LuxInterface::E_On;
	pLightingModel->m_LightingModelProperties.m_InstanceMode = LuxInterface::E_InstanceMode_Instance;
	pLightingModel->m_LightingModelProperties.m_SaveExtraSamplePointData = LuxInterface::E_On;
	pLightingModel->m_LightingModelProperties.m_InstancesDefaultToSHLighting = LuxInterface::E_Off;
	pLightingModel->m_LightingModelProperties.m_IgnoreDirectionalLightsFromMaya = LuxInterface::E_On;

	// Add extra lights

	// Setup light sample properties

	pLightingModel->m_SamplePointProperties.m_Type = LuxInterface::E_SamplePointsType_None;

	// Setup Passes

	LuxInterface::C_Pass newPass;
	newPass.m_SetList.PushBack( LuxInterface::C_Set( LuxInterface::E_SetAttribute_Light, "All" ) );
	newPass.m_SetList.PushBack( LuxInterface::C_Set( LuxInterface::E_SetAttribute_CastShadow, "All" ) );
	pLightingModel->m_PassList.PushBack(newPass);

	return true;
}

bool C_ProcessLightAftk::RunLux( Progress::I_ProgressBar* pProgress )
{
	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	Util::C_String luxApp = pInputConversionOptions->GetToolConfig().GetLuxFilenameWin32();
    Util::C_String luxElf = pInputConversionOptions->GetToolConfig().GetLuxFilenameYdl();
	Util::C_String luxAppServer = pInputConversionOptions->GetToolConfig().GetLuxFilenameServer();
	Util::C_String luxInputFile = pInputConversionOptions->GetPreprocessedAftkFilename();
	Util::C_String luxOutputFile = pInputConversionOptions->GetLitAftkFilename();
	Util::C_String luxLightingScript = pInputConversionOptions->GetLightingModelFilename();
    Util::C_String luxLightmapOutputDir = pInputConversionOptions->GetLightingTempDirectory();

    Util::C_String luxCommandLine = "";
	
    if( pInputConversionOptions->GetLightingOptions().GetUseLightingScreensavers() )
    {
        luxCommandLine = luxCommandLine 
						+ " -g " + luxElf 
						+ " -w " + luxApp +  " ";
    }

	luxCommandLine = luxCommandLine + " " 
					+ luxInputFile + " " 
					+ luxOutputFile + " " 
					+ luxLightingScript + " " 
					+ luxLightmapOutputDir;
            
	LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "Executing command line :\n");
	LOGDEBUG( Tools::g_pMyApplication->GetLogger(), "%s\n",luxCommandLine.Chars()); 

	int result = Sys::ShellExec::ExecuteCommandAndWait(
		luxAppServer, luxCommandLine, "", Sys::ShellExec::E_LaunchState_Minimise,
		&Tools::g_pMyApplication->GetLogger(), pProgress, "Process Lighting");

    if(result != 0)
    {
        EXECUTER_FATAL("Light Preprocess failed\n");
        return false;
    }

	return true;
}
