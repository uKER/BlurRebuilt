#include "stdafx.h"
#include "TreeDefinition.h"
#include "TreeRenderer.h"
#include "cmdline.h"
#include "tools_shared/VersionStrings.inl"

#include <string>
#include <iostream>

/*
#if defined(AFTK_MAYA_IO) && !defined(_DEBUG)
#include <maya/MLibrary.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>
#endif
*/

Debug::Logger& SpeedtreeBillboard::g_Log = Debug::GetLogger("tools.speedtreebillboard");

int main(int argc, char **argv)
{
	SPTBillboardApp app;
	return app.Run(argc,argv);
}


int SPTBillboardApp::Main(int argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	std::string scenePath;
	std::string outputPath;
	std::string xmlConfigPath;

	try
	{
		TCLAP::CmdLine cmd("speedtree_billboard", ' ', "0.1");
		//TCLAP::UnlabeledValueArg<std::string> scene("scene", "The scene containing trees", "", "string");
		TCLAP::ValueArg<std::string> scene(
			"","scene","scene file",true,"","aftk scene file");
		cmd.add(scene);

		TCLAP::ValueArg<std::string> xmlconfig(
			"","xml","xml file",true,"","xml config file");
		cmd.add(xmlconfig);

		cmd.parse(argc, argv);
		scenePath = scene.getValue();
		xmlConfigPath = xmlconfig.getValue();
	}
	catch (TCLAP::ArgException& e)
	{
		LOGFATAL(SpeedtreeBillboard::g_Log, "Failed to parse command line arguments\n%s", e.what());
	}

	SpeedtreeBillboard::C_TreeRendererApp treeRenderer;

	// Initialise WINSOCK
	#ifndef _DEBUG
	if (WSAStartup(WINSOCK_VERSION, &WSADATA()) != 0)
	{
		std::cout << "Winsock Error" << std::endl;
		WSACleanup();
		return 1;
	}
	#endif
/*
	#if defined(AFTK_MAYA_IO) && !defined(_DEBUG)
		MStatus mayaStatus = MLibrary::initialize("Speedtree Billboard Rendering", true);
		if (!mayaStatus)
		{
			return 0;
		}
		MString mVersion = MGlobal::mayaVersion();
		std::cout << "Initialising Maya [Version " << mVersion.asChar() << "]" << std::endl;
	#endif
*/
	Debug::LogFileSubscriber *pLogFile;
	{
		std::string logFile;
		std::size_t extIndex = scenePath.find_last_of('.');
		if (extIndex == std::string::npos)
			logFile = scenePath + ".log";
		else
			logFile = scenePath.substr(0, extIndex) + ".log";

		pLogFile = new Debug::LogFileSubscriber(logFile.c_str());
		SpeedtreeBillboard::g_Log.AddSubscriber(pLogFile);
	}

	Aftk::C_Global::Create();

	SpeedtreeBillboard::C_TreeRendererSettings settings;
	if (!SpeedtreeBillboard::C_TreeRendererSettings::ReadSettingsFromXml(settings, xmlConfigPath.c_str()))
	{
		LOGFATAL(SpeedtreeBillboard::g_Log, "Failed to read xml config settings\n");
		WSACleanup();
		SpeedtreeBillboard::g_Log.RemoveSubscriber(pLogFile);
		delete pLogFile;
		return 1;
	}

	Aftk::C_Scene *pScene = new Aftk::C_Scene();
	Aftk::C_Status status = pScene->Read(scenePath);
	LOGINFO(SpeedtreeBillboard::g_Log, "Reading scene %s\n", scenePath.c_str());
	if (!status)
	{
		LOGFATAL(SpeedtreeBillboard::g_Log, "%s\n", status.GetErrorString().c_str());
		WSACleanup();
		delete pScene;
		Aftk::C_Global::Destroy();
		SpeedtreeBillboard::g_Log.RemoveSubscriber(pLogFile);
		delete pLogFile;
		return 1;
	}
	else
		LOGINFO(SpeedtreeBillboard::g_Log, "Scene successfully read ... beginning billboard generation\n", scenePath.c_str());

	treeRenderer.m_DeviceParams.m_Width			= 1;
	treeRenderer.m_DeviceParams.m_Height		= 1;
	treeRenderer.m_DeviceParams.m_pName			= "test_aftk_mercury_render_pc";
	treeRenderer.m_DeviceParams.m_hInstance		= 0;
	treeRenderer.m_DeviceParams.m_CmdShow		= 0;

#ifdef USE_REF_DEVICE
	LOGINFO(SpeedtreeBillboard::g_Log, "Using D3D Reference Device");
	treeRenderer.m_DeviceParams.m_Multisample	= Display::E_Multisample_None;
	treeRenderer.m_DeviceParams.m_UseREF		= true;
#else
	LOGINFO(SpeedtreeBillboard::g_Log, "Using D3D HAL Device");
	treeRenderer.m_DeviceParams.m_Multisample	= Display::E_Multisample_4;
	treeRenderer.m_DeviceParams.m_UseREF		= false;
#endif
	treeRenderer.m_DeviceParams.m_hWnd			= GetConsoleWindow();

	if (!treeRenderer.Create(&settings))
	{
		LOGFATAL(SpeedtreeBillboard::g_Log, "Failed to initialize renderer\n");
		WSACleanup();
		delete pScene;
		Aftk::C_Global::Destroy();
		SpeedtreeBillboard::g_Log.RemoveSubscriber(pLogFile);
		delete pLogFile;
		return 1;
	}
	treeRenderer.Run(pScene);
	treeRenderer.Destroy();

	WSACleanup();
	delete pScene;
	Aftk::C_Global::Destroy();
	SpeedtreeBillboard::g_Log.RemoveSubscriber(pLogFile);
	delete pLogFile;

	return 0;
}