
#include "stdafx.h"
#include "SceneSend.h"

#define NT_PLUGIN
#include <maya/MGlobal.h>
#pragma warning (push,3)
#include <aftk/Aftk.h>
#pragma warning (pop)
#include <camera2/export/AftkToCameraScene.h>
#include <system/serialise.h>
#include <system/serialise/C_MemoryStream.h>
#include <camera2/tv/CameraScene.h>
#include <camera2/tv/CameraSceneSerialise.h>
#include "AnimSend.h"
#include "CamCom.h"

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

static inline U32 Fcc(char a, char b, char c, char d)
{
	return (a<<24) | (b<<16) | (c<<8) | d;
}

S32 SceneSend()
{
	// Remember selection, select the cameras and build an AFTK scene from them:	
	Util::C_String cmd = "string $selection[]=`ls -sl`;\n";
	cmd += "string $layers[] = `ls -type displayLayer \"Cameras_*\"`;\n";
	cmd += "for ($layer in $layers)\n";
	cmd += "  select -add -hi `editDisplayLayerMembers -fullNames -query $layer`;\n";
	cmd += "select -add -hi InstanceReferences;\n";

	MGlobal::executeCommand(cmd.Chars());

	Aftk::C_Scene inScene;
	inScene.ReadExternalScene(Aftk::E_ExternalScene_Maya, "exportSelection=1");
	MGlobal::executeCommand("select -r $selection;"); // Restore the selection

	// Find the camera layer and convert it to a C_CameraScene
	Cam2::C_CameraScene outScene;
	FindCameras(inScene, outScene);

	dprintf("Serialising %d zones and %d cameras\n", outScene.m_zones.Size(), outScene.m_cams.Size());

	// Serialise:
	Sys::C_MemoryStream buffer(128);
	buffer.Write("CSCE", 4);

	Sys::BinarySerialiser<Ut::NativeToBigEndian> archive(buffer);
	S32 pad = 0;
	archive << Cam2::C_CameraScene::Version << pad;
	archive << outScene;

	S32 bsize = (S32)buffer.GetBufferSize();
	dprintf("Camera data size: %.1fk\n", (F32)bsize / 1024.f);

	if (bsize <= 0)
		return 1;

	S32 confirmsBefore = g_CamCom.Confirms();
	g_CamCom.Send(buffer.GetBufferPtr(), bsize);

	S32 start = (S32)Time::GetCurrentTime();
	for (;;)
	{
		g_CamCom.Update();
		S32 rel = g_CamCom.Confirms() - confirmsBefore;
		if (rel > 0)
		{
			dprintf("Sent ok!\n");
			break;
		}
		
		if ((S32)Time::GetCurrentTime() - start >= 3000000)
			break; // Time out
		MT::Thread::Sleep(1);
	}

	return 0;
}
