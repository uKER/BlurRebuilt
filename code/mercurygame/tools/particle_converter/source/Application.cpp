#include "stdafx.h"
#include <system/xml/XmlDom.h>
#include <system/debug/log/debuglog.h>
#include <system/debug/log/subscribers/logfilesubscriber.h>
#include <system/debug/log/ExecuterFeedback.h>

#include <dynamic/particle/tools_shared/UserShape3DData.h>
#include <dynamic/particle/tools_shared/UserShape3DVariants.h>
#include <dynamic/particle/tools_shared/UserParticleContainer.h>

#include <tools_shared/shim/Shim.h>
#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.inl>

#include "Options.h"
#include "Application.h"
#include "ParticleConverter.h"
#include "PreProcScene.h"

C_Application::C_Application(
	const char *szName,
	const char *szSubtitle,
	const char *szLogger,
	const char *szDmpPath,
	bool bExecuter)
	:
	Tools::C_ToolsApplication(
		szName,
		szSubtitle,
		szLogger,
		szDmpPath,
		bExecuter)
{
}

C_Application::~C_Application()
{
}

S32 C_Application::Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	C_Options conversionOptions;
	bool success = true;

	//Build options struct
	{
		Util::C_String xmlFilePath;
		//Parse our command-line options.
		success = conversionOptions.ParseCommandLine(xmlFilePath, argc, argv);
		if (!success) goto exit;

		LOGINFO(Tools::g_pMyApplication->GetLogger(), "Parsing: %s\n", xmlFilePath.Chars());
		
		Sys::C_XmlDom dom;
		success = dom.Read(xmlFilePath.Chars());
		if (!success) goto exit;

		success = conversionOptions.ParseXml(xmlFilePath, dom);
		if (!success) goto exit;
	}

	//Perform expot.
	{
		Tools::Shim::C_FileCopyXmlExport::ClearXmlFileList(
			conversionOptions.GetInputTempFilePath(), 
			conversionOptions.GetExportName());

		Tools::FileCopy::C_FileCopyList uberCopyList;
		success = ExportData(uberCopyList, conversionOptions);
		if (!success) goto exit;

		success = Tools::Shim::C_FileCopyXmlExport::ExportXmlFileList(
			conversionOptions.GetRootTempPath(),
			uberCopyList, 
			conversionOptions.GetInputTempFilePath(),
			conversionOptions.GetCopyLocations(),
			conversionOptions.GetLaunchComputerName(),
			conversionOptions.GetExportName());
		if (!success) goto exit;
	}

exit:
	if (success)
	{
		EXECUTER_INFO("Processing completed succesfully!\n");
	}
	else
	{
		EXECUTER_INFO("Processing failed!\n");
	}

	if (success)
		return 0;
	else
		return 1;
}

bool C_Application::ExportData(Tools::FileCopy::C_FileCopyList& uberCopyList, const C_Options& options)
{
	bool success;
	//Our exported data container.
	DynamicEffect::Particle::C_UserParticleContainer particleExportData;

	success = Aftk::C_Global::Create();
	if (!success) goto exit;
	{
		Aftk::C_Scene aftkScene;

		//Read in input data.
		success = aftkScene.Read(options.GetInputTempFilePath().Chars());
		if (!success) goto exit;

		success = C_PreProcScene::PreProcScene(aftkScene);
		if (!success) goto exit;

#ifdef BUILD_CONFIG_DEBUG
		aftkScene.Write("preproc.a3d");
#endif

		//Convert our particle data into our in-memory format.
		success = C_ParticleConverter::Convert(
			particleExportData, 
			aftkScene,
			options);
		if (!success) goto exit;
	}

	//Write out data for our different skus.
	for(S32 platformNo=0; platformNo < Util::E_Platform_Count; platformNo++)
    {
		bool endianSwap = Tools::ShouldEndianSwap(Util::E_Platform(platformNo));

		Util::C_String relativeOutputFile = options.GetInputTempFilePath().IReplace(options.GetRootTempPath(), "");
		relativeOutputFile = Util::RemoveFileExtension(relativeOutputFile);
		Util::C_String outputFilePath = 
			options.GetCopyLocations()[platformNo].workingPath + relativeOutputFile + ".particleMesh";

		//Create output folder.
		success = Sys::C_FileSystem::Instance().CreateDirectory(Util::ExtractDirectory(outputFilePath));
		if (!success) goto exit;
		
		uberCopyList.AddFileCopyItem(
			(Util::E_Platform)platformNo,
			Tools::FileCopy::C_FileCopyItem((const char*)outputFilePath.Chars(), ""));
		Sys::I_File* m_pFile = Sys::C_FileSystem::Instance().Open(
			outputFilePath.Chars(), 
			Sys::E_FileAccess_Write);
		if (!m_pFile)
		{
			success = false;
			goto exit;
		}
		m_pFile->SetEndianSwapFlag(endianSwap);

		success = particleExportData.Serialise(*m_pFile);
		if (!success) goto exit;

		delete m_pFile;
		m_pFile = 0;
	}

exit:
	Aftk::C_Global::Destroy();
	return success;
}