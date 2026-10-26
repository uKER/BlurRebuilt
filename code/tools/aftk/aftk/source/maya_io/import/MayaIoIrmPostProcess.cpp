#include "stdafx.h"

#include "MayaIoIrmPostProcess.h"


using namespace MayaIo;

Aftk::C_Status C_IrmPostProcess::PostProcessScene(Aftk::C_Scene& scene)
{
	//This code ensures that children of objects instanced via the IRM system inherit the layer
	//properties of their parent objects.
	for (Aftk::C_SceneNodeIterator it(&scene); it; ++it)
	{
		Aftk::C_SceneNode* pSceneNode = it;
		Aftk::I_Plug* pRefChildPlug = pSceneNode->GetPlugPtr("instanceReferenceChild");
		if (pRefChildPlug)
		{
			//We don't need this plug anymore, let's save some memory.
			pSceneNode->DeletePlug(pRefChildPlug->GetLogicalIndex());

			//Add to our parent layer.
			Aftk::C_SceneNode* pParent = pSceneNode->GetParentPtr();
			while(pParent)
			{
				if (pParent->GetNumLayers() != 0)
				{
					//Remove ourselves from any previous layers.
					while(pSceneNode->GetNumLayers() != 0)
					{
						pSceneNode->GetLayerPtr(pSceneNode->GetNumLayers()-1)->
							DeleteMember(pSceneNode);
					}

					//Add ourself to our parents layers.
					for (U32 i = 0; i < pParent->GetNumLayers(); i++)
					{
						//Add to our parent's layer.
						pParent->GetLayerPtr(i)->AddMember(pSceneNode);
					}
					break;
				}

				//Fly upward looking for a parent with layers.
				pParent = pParent->GetParentPtr();
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}
