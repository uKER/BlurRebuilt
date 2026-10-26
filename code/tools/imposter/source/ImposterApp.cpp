#include "ImposterPch.h"

#include <cmdline.h>
#include <tools_shared/ToolsApplication.h>

#include "imposter/ImposterProcess.h"

#include <system/misc/BtlStringUtil.h>
#include <system/Serialise.h>
#include <tools_shared/memorydbg.h>

class C_ImposterCreateApp : public Tools::C_ToolsApplication
{
public:
	C_ImposterCreateApp()
		: Tools::C_ToolsApplication(
			"ImposterCreate",
			"Bizarre Creations Imposter LOD Generator",
			"tools.imposter",
			"")
	{
	}

	int Main(int argc, char **argv, Progress::I_ProgressBar *pProgress);
};


int C_ImposterCreateApp::Main(int argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	//_CrtSetBreakAlloc(15492);

	std::string a3dInPath, a3dOutPath, a3dBasePath;
	bool useTextureAtlas = false;
	try
	{
		TCLAP::CmdLine cmd("imposter_create", ' ', "0.1");
		TCLAP::UnlabeledValueArg<std::string> a3dInPathArg(
			"path", "Path to a3d file", "", "Valid file path");

		TCLAP::ValueArg<std::string> a3dOutPathArg(
			"o", "output", "Path to a3d output file", false, "", "Valid file path");

		TCLAP::ValueArg<std::string> a3dBasePathArg(
			"b", "base", "Path to (lit) a3d base mesh file", false, "", "Valid file path");

		TCLAP::SwitchArg atlasArg("a", "atlas", "Pack textures into atlas(es)", false);

		cmd.add(a3dInPathArg);		
		cmd.add(a3dOutPathArg);
		cmd.add(a3dBasePathArg);
		cmd.add(atlasArg);

		cmd.parse(argc, argv);
		if (a3dInPathArg.getValue().length() != 0)
			a3dInPath = a3dInPathArg.getValue();

		if (a3dOutPathArg.getValue().length() != 0)
			a3dOutPath = a3dOutPathArg.getValue();

		if (a3dBasePathArg.getValue().length() != 0)
			a3dBasePath = a3dBasePathArg.getValue();

		useTextureAtlas = atlasArg.getValue();
	}
	catch (TCLAP::ArgException& e)
	{
		LOGFATAL(GetLogger(), "Failed to parse command line arguments\n %s", e.error().c_str());
		return 1;
	}

	bool success = true;
	Aftk::C_Global::Create();
	{
		Aftk::C_Scene scene, baseScene;
		if (scene.Read(a3dInPath))
		{
			Btl::String workingFolder = a3dOutPath.size()
										? Misc::ExtractDirectory(a3dOutPath.c_str())
										: Misc::ExtractDirectory(a3dInPath.c_str());

			Sys::C_FileSystem::Instance().CreateDirectory(workingFolder.Chars());

			Aftk::C_Scene *pBaseScene = 0;
			if (a3dBasePath.length() && baseScene.Read(a3dBasePath))
				pBaseScene = &baseScene;

			success = Imposter::ProcessAftk(scene, workingFolder, useTextureAtlas, pBaseScene);
			if (success)
				success = scene.Write(a3dOutPath.size() ? a3dOutPath : a3dInPath);
		}
		else
			success = false;
	}

	Aftk::C_Global::Destroy();

	return success ? 0 : 1;
}


int main(int argc, char **argv)
{
	C_ImposterCreateApp app;
	return app.Run(argc,argv);
}
