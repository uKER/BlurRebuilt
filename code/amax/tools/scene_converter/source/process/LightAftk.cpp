#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <converter_shared/lux_interface/LuxInterface.h>
#include <converter_shared/helpers/AftkHelpers.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/AftkSceneData.h>

#include <process/LightAftk.h>

#include <polyflags.h>
 

bool C_ProcessLightAftk::Process( Progress::I_ProgressBar* pProgress )
{
	bool success = true;
	Progress::UpdateSimple progressSimple(pProgress, GetName());
		
	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	Util::C_String defaultLightingModelFilename = pInputConversionOptions->GetToolConfig().GetLuxBaseConfigFilename();
	Util::C_String specificLightingModelFilename = pInputConversionOptions->GetLightingModelOptions().GetLuxSpecificConfigFilename();
	Util::C_String lightingModelFilename = pInputConversionOptions->GetLightingModelFilename();

	LuxInterface::C_LightingModel lightingModel;
	success = lightingModel.ReadXml( defaultLightingModelFilename );
	if(!success) return false;

	if(specificLightingModelFilename.Length() > 0)
	{
		success = lightingModel.ReadXml( specificLightingModelFilename );
		if(!success) return false;
	}

	SetupLightingModel( &lightingModel );

	if (LightUpToDate(pInputConversionOptions, lightingModel) && !pInputConversionOptions->ForceFullExport())
	{
		LOGINFO( Tools::g_pMyApplication->GetLogger(), "ProcessLightAftk, light not preprocessed, no change detected\n");
		return true;
	}

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

	const C_DataAftkScene* pPreprocessedAftk = (C_DataAftkScene*)GetInput( E_GraphData_PreprocessedAftk );
	assert(pPreprocessedAftk);
	const Aftk::C_Scene& aftkScene = pPreprocessedAftk->GetAftkScene();
	
	const C_LightingOptions& lightingOptions = pInputConversionOptions->GetLightingOptions();

	F32 texelDensity = 0.3f;
	F32 alternateTexelDensityRatio = 128.0f;

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

	U32 noSunSamples = 8;
	U32 noSkySamples = 256;
	F32 sunPenumbra = 0.999961f;

	// TODO : m_pOptions->m_LightmapQuality
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

	// Setup lighting model settings
	pLightingModel->m_LightingModelProperties.m_TexelDensity = texelDensity;
	pLightingModel->m_LightingModelProperties.m_AlternateTexelDensityRatio = alternateTexelDensityRatio;
	//pLightingModel->m_LightingModelProperties.m_InputGamma = 2.2f;
	//pLightingModel->m_LightingModelProperties.m_OutputGamma = 2.2f;
	//pLightingModel->m_LightingModelProperties.m_AlphaScaling = LuxInterface::E_On;
	//pLightingModel->m_LightingModelProperties.m_AntiLightLeaking = LuxInterface::E_On;
	pLightingModel->m_LightingModelProperties.m_InstanceMode = LuxInterface::E_InstanceMode_Instance;
	//pLightingModel->m_LightingModelProperties.m_SaveExtraSamplePointData = LuxInterface::E_On;
	pLightingModel->m_LightingModelProperties.m_InstancesDefaultToSHLighting = LuxInterface::E_Off;
	pLightingModel->m_LightingModelProperties.m_IgnoreDirectionalLightsFromMaya = LuxInterface::E_On;

	// Add extra lights

	// TODO : Set this so that it equals maya rotation values
	Maths::HVec3 sunDirection = AftkHelpers::CalculateSunDirection( lightingOptions.GetSunHeading(), lightingOptions.GetSunElevation() );

	bool addSkyLight = lightingOptions.GetAddSkyLight();
	S32 bouncePasses = lightingOptions.GetBouncePasses();
	F32 bounceScale = lightingOptions.GetBounceScale();
	F32 skyIntensity = lightingOptions.GetSkyIntensity();
	if(addSkyLight)
	{
		if(bouncePasses > 1)
		{
			LuxInterface::C_LightBounceGather* pLightBounceGather = new LuxInterface::C_LightBounceGather();
			pLightingModel->m_LightList.PushBack( pLightBounceGather );

			pLightBounceGather->m_Colour = Maths::C_RGBA_F32(1.0f, 1.0f, 1.0f, 1.0f);
			pLightBounceGather->m_Intensity = skyIntensity;
			pLightBounceGather->m_Samples = noSkySamples;
			pLightBounceGather->m_BounceScale = bounceScale;
			pLightBounceGather->m_BouncePasses = bouncePasses;
		}
		else
		{
			LuxInterface::C_LightHemisphere* pLightHemisphere = new LuxInterface::C_LightHemisphere();
			pLightingModel->m_LightList.PushBack( pLightHemisphere );

			pLightHemisphere->m_Colour = Maths::C_RGBA_F32(1.0f, 1.0f, 1.0f, 1.0f);
			pLightHemisphere->m_Intensity = skyIntensity;
			pLightHemisphere->m_Samples = noSkySamples;
		}
	}


	// add sun light
	bool addSunLight = lightingOptions.GetAddSunLight();
	F32 sunIntensity = 1.0f;
	if(addSunLight)
	{
		{
			LuxInterface::C_LightDirectional* pLightDirectional = new LuxInterface::C_LightDirectional();
			pLightingModel->m_LightList.PushBack( pLightDirectional );

			pLightDirectional->m_Colour = Maths::C_RGBA_F32(1.0f, 1.0f, 1.0f, 1.0f);
			pLightDirectional->m_Intensity = sunIntensity;
			pLightDirectional->m_Lambert = LuxInterface::E_Off;
			pLightDirectional->m_Direction = sunDirection;
			pLightDirectional->m_Penumbra = sunPenumbra;
			pLightDirectional->m_Samples = noSunSamples;
		}
	}

	// Setup light sample properties

	pLightingModel->m_SamplePointProperties.m_Type = LuxInterface::E_SamplePointsType_SH;
	pLightingModel->m_SamplePointProperties.m_InputFilename = pInputConversionOptions->GetLightSamplePointsFilename();
	pLightingModel->m_SamplePointProperties.m_OutputFilename = pInputConversionOptions->GetLightSamplesFilename();
	pLightingModel->m_SamplePointProperties.m_Output = LuxInterface::E_SamplePointsOutputFormat_Binary;

	// Setup Passes

	LuxInterface::C_Pass newPass;
	newPass.m_SetList.PushBack( LuxInterface::C_Set( LuxInterface::E_SetAttribute_Light, "All" ) );
	newPass.m_SetList.PushBack( LuxInterface::C_Set( LuxInterface::E_SetAttribute_CastShadow, "All" ) );

	AddSpecialLayersToPass( &newPass, &aftkScene );
		
	pLightingModel->m_PassList.PushBack(newPass);

	return true;
}

void C_ProcessLightAftk::AddSpecialLayersToPass(LuxInterface::C_Pass* pPass, const Aftk::C_Scene* pAftkScene )
{
	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	const U32 noLayers = pAftkScene->GetNumLayers();
    for( U32 layerNo=0; layerNo<noLayers; layerNo++ )
    {
        const Aftk::C_LayerNode* pCurrLayer = pAftkScene->GetLayerPtr(layerNo);

        if(pCurrLayer)
        {
			Objectflags::C_Objectflags objectFlags( pInputConversionOptions->GetObjectflagConfig(), pCurrLayer );

			if( objectFlags.GetValue( E_Objectflag_DontLight ) )
			{
				pPass->m_SetList.PushBack( LuxInterface::C_Set( LuxInterface::E_SetAttribute_DontLight, pCurrLayer->GetName().c_str() ) );
			}

			if( objectFlags.GetValue( E_Objectflag_DontCastShadows ) )
			{
				pPass->m_SetList.PushBack( LuxInterface::C_Set( LuxInterface::E_SetAttribute_DontCastShadow, pCurrLayer->GetName().c_str() ) );
			}
		}
	}
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

bool CompareFiles(const Util::C_String& file1, const Util::C_String& file2)
{
	//Open the files
	Sys::I_File* pLastLightModelFile = Sys::C_FileSystem::Instance().Open(file1);
	if (pLastLightModelFile == NULL)
		return false;
	Sys::I_File* pNextLightModelFile = Sys::C_FileSystem::Instance().Open(file2);
	if (pNextLightModelFile == NULL)
		return false;
	
	Sys::C_StreamSize lastSize = pLastLightModelFile->Length();
	Sys::C_StreamSize nextSize = pNextLightModelFile->Length();

	if (lastSize != nextSize)
		return false;

	//Compare files byte by byte
	for (U32 i = 0; i < lastSize; i++)
	{
		U8 lastData, nextData;
		//Read a byte of each
		pLastLightModelFile->Read((void*)&lastData, 1);
		pNextLightModelFile->Read((void*)&nextData, 1);
		if (lastData != nextData)
			return false;
	}

	return true;
}

bool C_ProcessLightAftk::LightUpToDate(const C_DataConversionOptions* pConversionOptions, const LuxInterface::C_LightingModel& setupLightModel) const
{
	Sys::C_Fileinfo sourceAftkFileDate;
	Sys::C_Fileinfo litAftkFileDate;
	Sys::C_Fileinfo luxFileDate;
	Sys::C_Fileinfo sceneConverterFileDate;
	
	if (!Sys::C_FileSystem::Instance().FileInfo(pConversionOptions->GetSourceAftkFilename(), &sourceAftkFileDate))
		return false;

	if (!Sys::C_FileSystem::Instance().FileInfo(pConversionOptions->GetLitAftkFilename(), &litAftkFileDate))
		return false;

	if (!Sys::C_FileSystem::Instance().FileInfo(pConversionOptions->GetToolConfig().GetLuxFilenameWin32(), &luxFileDate))
		return false;

	if (!Sys::C_FileSystem::Instance().FileInfo(pConversionOptions->GetExecutableFileNameWin32(), &sceneConverterFileDate))
		return false;

	if (sourceAftkFileDate.m_FileTime > litAftkFileDate.m_FileTime)
		return false;

	if (litAftkFileDate.m_FileTime < luxFileDate.m_FileTime)
		return false;

#ifndef NOCHECK_SCENECONVERTER_FILE_TIME
	if (litAftkFileDate.m_FileTime < sceneConverterFileDate.m_FileTime)
		return false;
#endif
	//Write the lightModel to a temp file and compare with pConversionOptions->GetLightingModelFilename()
	setupLightModel.WriteXml(pConversionOptions->GetLightingModelFilename() + ".temp");
	
	if (!CompareFiles(pConversionOptions->GetLightingModelFilename(), pConversionOptions->GetLightingModelFilename() + ".temp"))
		return false;
	

	return true;
}
