
#include <stdlib.h>
#include <crtdbg.h>

#pragma warning (push,3)
#pragma warning (disable:4311)
#pragma warning (disable:4312)
#include <aftk/Aftk.h>
#include <core/containers.h>
#include <system/serialise.h>
#include <system/serialise/I_FileSystem.h>
#pragma warning (pop)

#include <camera2/export/AftkToCameraScene.h>
#include <camera2/tv/CameraScene.h>
#include <Camera2/tv/CameraSceneSerialise.h>

// CameraExportTool.exe
// This is a command-line line tool for exporting camera data from an .A3D file into a Camera
// scene file (.cse)
// It uses the Camera library (tool version), and takes an A3D scene class and turns it into a block of data

void dprintf(char *format, ...)
{
	va_list val=NULL;
	va_start(val,format);

	vfprintf(stdout,format,val);
	fflush(stdout);

	va_end(val);
}

// Search the AFTK Scene for a node which is in a Cameras_... layer
static void FindCameras(Aftk::C_Scene& scene, Cam2::C_CameraScene& outScene)
{
	Aftk::C_SceneNodeIterator it(&scene);

	for ( ; it; it++)
	{
		Aftk::C_SceneNode *node = it;
		Aftk::C_LayerNode *layer = node->GetLayerPtr(0);
		
		if (layer == 0) continue;
		if (layer->GetName().substr(0, 8) == "Cameras_")
		{
			// Convert the node to a C_CameraScene
			Cam2::AftkToCameraScene(&scene, node, outScene);
			break;
		}
	}
}

int main(int argc,char **argv)
{
	char *sceneName="Z:\\MercuryGame\\assets\\Locations\\System\\TestWorld\\TestWorld_Cameras.a3d";
	char *outName="cams.cse";
	Sys::I_BinaryStream *file=0;
	
	if (argc>=2) sceneName=argv[1];
	if (argc>=3) outName=argv[2];
 
	Aftk::C_Global::Create();
	{
		Aftk::C_Scene scene;

		scene.Read(sceneName);

		file=Sys::C_FileSystem::Instance().Open(outName,Sys::E_FileAccess_Write);

		Cam2::C_CameraScene cameraScene;
		FindCameras(scene, cameraScene);

		if (file)
		{
			// Write camera scene to disk:
			Sys::BinarySerialiser<Ut::NativeToBigEndian> archive(*file);

			S32 fileVersion = cameraScene.Version, pad = 0;
			archive << fileVersion << pad;
			archive << cameraScene;
		}

		if (file) { delete file; file=0; }
	}
	Aftk::C_Global::Destroy();

	// Dump memory leaks:
	_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF );
	
	return 0;
}
