#include "stdafx.h"

#ifdef WIN32
#include <windows.h>
#endif

//This file provides definitions that are required by the Aftk. You could also provide these yourself of course
//as opposed to including this header.

//This includes the Aftk itself.
#include "../Aftk/include/Aftk/Aftk.h"

int main(int argc, char* argv[])
{
	//Initialise the Aftk. This must be called before any Aftk features are used as it starts the
	//Aftk's memory manager.
	Aftk::C_Global::Create();
	{
		//This will store the results of our read and write operations.
		Aftk::C_Status status;
		
		//This is our scene instance.
		Aftk::C_Scene aftkScene;

		//Read in an A3D file from disk.
		status = aftkScene.Read("Mikes FBX/MEND_M01_ChairThrow_L_01.a3d");
		//Check whether we succeed or not.
		if (!status)
		{
			std::cout << status.GetErrorString() << std::endl;
			return -1;
		}
		else
			std::cout << "File read okay." << std::endl;

		//Create a scene node iterator. By passing in a pointer to the scene we create an iterator
		//that will traverse the entire scene hierarchy. Alternatively, passing in a pointer to a specific
		//node will iterate over all siblings and children of that node.
		Aftk::C_SceneNodeIterator it(&aftkScene);
		for(; it; it++)
		{
			//Get a pointer to the current scene node.
			Aftk::C_SceneNode* pSceneNode = it;
			//Display the name of this node.
			std::cout << pSceneNode->GetName() << std::endl;
		}

		//Write our scene to disk.
		status = aftkScene.Write("output.a3d", Aftk::E_FileType_A3d);
		//Check whether we succeed or not.
		if (!status)
		{
			std::cout << status.GetErrorString() << std::endl;
			return -1;
		}
		else
			std::cout << "File written okay." << std::endl;
	}
	//Shut down the Aftk.
	Aftk::C_Global::Destroy();

	return 0;
}

