#include "stdafx.h"
#include <system/xml/XmlDom.h>

#include <system/debug/log/DebugLog.h>
#include <system/debug/log/subscribers/logfilesubscriber.h>
#include <system/debug/log/ExecuterFeedback.h>
#include <tools_shared/executer/ExecuterHelpers.h>
#include <tools_shared/shim/Shim.h>
#include <tools_shared/ToolsApplication.h>
#include <tools_shared/VersionStrings.inl>

#include "Application.h"
#include "TextureConverter.h"

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

	Sys::C_XmlDom dom;
	//Build options struct
	{
		//Parse our command-line options.
		Util::C_String xmlFilePath;
		success = conversionOptions.ParseCommandLine(xmlFilePath, argc, argv);
		if (!success) goto exit;

		if (!dom.Read(xmlFilePath.Chars()))
			return false;

		if (!conversionOptions.ParseXml(xmlFilePath, dom))
			return false;
	}

	//Perform export.
	{
		Tools::Shim::C_FileCopyXmlExport::ClearXmlFileList(
			conversionOptions.GetInputTempFilePath(),
			conversionOptions.GetExportName());

		Tools::FileCopy::C_FileCopyList uberCopyList;

		success = C_TextureConverter::Process(
			uberCopyList,
			conversionOptions);

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
