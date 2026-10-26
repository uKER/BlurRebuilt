#include "stdafx.h"
#include "AftkSpeedTreeApp.h"
#include "tools_shared/VersionStrings.inl"
#include "cmdline.h"


Debug::Logger& g_Log = Debug::GetLogger("tools.aftkspeedtree");

int main(int argc, char **argv)
{
	AftkSpeedTreeApp app;
	return app.Run(argc,argv);
}

void ApplySceneMatrixHack(Aftk::C_Scene* pScene, const Aftk::Matrix44& m)
{
	Aftk::C_SceneNode* pRoot = pScene->GetRootSceneNodePtr();
	if (pRoot->GetName() == "AftkRoot")
	{
		//Reset root!
		pScene->GetRootSceneNodePtr()->SetLocalMatrix(m);
	}
	else if (pRoot->GetName() == "AftkUtImportRoot")
	{
		for (U32 i = 0; i < pRoot->GetNumChildren(); i++)
		{
			Aftk::C_SceneNode* pChild = pRoot->GetChildPtr(i);
			if (pChild->GetName() == "AftkRoot")
			{
				//Reset root!
				pChild->SetLocalMatrix(m);
			}
		}
	}
}

int AftkSpeedTreeApp::Main(int argc, char **argv, Progress::I_ProgressBar *pProgress)
{

	std::string xmlConfig;
	std::string aftkInput;
	std::string aftkOutput;

	Debug::LogFileSubscriber *pLogFile;
	{
		std::string logFile("aftkspeedtreegen.log");

		pLogFile = new Debug::LogFileSubscriber(logFile.c_str());
		g_Log.AddSubscriber(pLogFile);
	}

	try
	{
		TCLAP::CmdLine cmd("aftkspeedtreegen", ' ', "0.1");
		TCLAP::ValueArg<std::string> config(
			"","config","xml config file",true,"","xml config file");

		TCLAP::ValueArg<std::string> in(
			"","in","aftk input scene",true,"","aftk input scene");

		TCLAP::ValueArg<std::string> out(
			"","out","aftk output scene",true,"","aftk output scene");

		cmd.add(config);
		cmd.add(in);
		cmd.add(out);
		cmd.parse(argc, argv);

		xmlConfig = config.getValue();
		aftkInput = in.getValue();
		aftkOutput = out.getValue();
	}
	catch (TCLAP::ArgException& e)
	{
		LOGFATAL(g_Log, "Failed to parse command line arguments\n%s", e.what());
		g_Log.RemoveSubscriber(pLogFile);
		delete pLogFile;
		return 1;
	}

	AftkSpeedTree::SpeedTree speedTree;
	if (!AftkSpeedTree::ReadSetupXmlFile(xmlConfig.c_str(), speedTree))
	{
		LOGFATAL(g_Log, "Failed to read speedtree setup configuration ... either missing or incomplete\n");
		g_Log.RemoveSubscriber(pLogFile);
		delete pLogFile;
		return 1;
	}

	Aftk::C_Global::Create();

	Aftk::C_Scene *pScene = new Aftk::C_Scene();

	if (!pScene->Read(aftkInput))
	{
		LOGFATAL(g_Log, "Failed to read aftk scene");
		g_Log.RemoveSubscriber(pLogFile);
		delete pLogFile;
		return 1;
	}

	Aftk::Matrix44 id;
	UtMaths::SetIdentity(&id);
	ApplySceneMatrixHack(pScene, id);

	speedTree.ProcessAftk(*pScene);

	Aftk::Matrix44 inverseZ;
	UtMaths::SetIdentity(&inverseZ);
	inverseZ[2][2] = -1.f;
	ApplySceneMatrixHack(pScene, inverseZ);

	Aftk::C_Status status = pScene->Write(aftkOutput);

	delete pScene;

	Aftk::C_Global::Destroy();

	g_Log.RemoveSubscriber(pLogFile);
	delete pLogFile;

	if (!status)
		return 1;
	else
		return 0;
}