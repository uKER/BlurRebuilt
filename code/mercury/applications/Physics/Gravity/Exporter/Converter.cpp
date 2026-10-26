#include <core/core.h>
#include <core/maths.h>
#include "Aftk/Aftk.h"
#include <tools_debug/DebugManager.h>

//This includes the Aftk itself.
#include "Aftk/Aftk.h"
#include "AftkUt/AftkUt.h"



#include "gravity/sharedphysics/sortedarray.h"
#include "gravity/sharedphysics/sortedarray.inl"

#include "gravity/treebuilder/aabbtree.h"
#include "gravity/treebuilder/aabbtree.inl"
#include "system/serialise/C_FileSystem.h"
#include "system/serialise/I_BinaryStream.h"
#include "gravity/sdk/shapedescriptors.h"
#include "gravity/sdk/shapedescriptors.inl"
#include "gravity/exporter/ExportPrimitivesList.h"
#include "gravity/importer/Importer.h"

#include "ConvertA3D.h"


namespace Gravity
{
	int C_Converter::ConvertA3DToGravity(const char* fileName, const char* resultFile)
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
			status = aftkScene.Read(fileName);
			//Check whether we succeed or not.
			if (!status)
			{
				std::cout << status.GetErrorString() << std::endl;
				return 0;
			}
			else
				std::cout << "File read okay." << std::endl;

			
			status = Aftk::C_SceneNodeVisit::Traverse(
				&aftkScene,                                           //Pointer to an Aftk::C_Scene object.
				Aftk::C_MeshTriangulate::C_TriangulateVisitor(),      //The operation that will be performed
				NULL);                                           //Optional pointer to Aftk::I_Progress derived progress bar class

			if(!status)
			{
				std::cout << "Failed to triangulate mesh!" <<std::endl;
				return 0;
			}
			Sys::I_BinaryStream *filePtr = Sys::C_FileSystem::Instance().Open( resultFile , Sys::E_FileAccess_Write);
			filePtr->SetEndianSwapFlag(true);
			//Sys::BinarySerialiser<Ut::NativeToBigEndian> archive(*filePtr);

			/*Baking::AABBTree tree(0.25, 20);
			Gravity::C_ExportPrimitives<Sys::BinaryDeserialiser<Ut::NativeToBigEndian>> buildPrim;*/



			Aftk::C_SceneNode* pRoot = aftkScene.GetRootSceneNodePtr();

			
			//Create a scene node iterator. By passing in a pointer to the scene we create an iterator
			//that will traverse the entire scene hierarchy. Alternatively, passing in a pointer to a specific
			//node will iterate over all siblings and children of that node.

			Aftk::C_SceneNodeIterator it(&aftkScene);
		
			C_AftkSceneNodeList aftkSceneStaticNodeList;

			C_AftkSceneNodeList aftkSceneConstraintNodeList;
		
			for(; it; it++)
			//for(S32 i=0; i<pRoot->GetNumChildren(); ++i)
			{
				//Get a pointer to the current scene node.
				Aftk::C_SceneNode* pSceneNode = it;
				//Display the name of this node.

				//Aftk::C_SceneNode* pSceneNode = pRoot->GetChildPtr(i);

				
				/*
					Take care of compound objects
				*/

				std::string name=pSceneNode->GetName();

				if(name.length() > 0 && ( (name[0] == 'M' && name[1] == '_') || 
					(name[0] == 'm' && name[1] == '_')))
				{	
					C_AftkSceneNodeList aftkSceneNodeList;
	
					U32 size=pSceneNode->GetNumChildren();

					aftkSceneNodeList.PushBack<Aftk::C_SceneNode*>(pSceneNode);	
		
					for(U32 i=0; i< size; ++i)
					{
						Aftk::C_SceneNode* pChild = (Aftk::C_SceneNode*)pSceneNode->GetChildPtr(i);
						aftkSceneNodeList.PushBack<Aftk::C_SceneNode*>(pChild);	
					}

					Maths::HMat43 worldMatrix;

					ConvertA3DMatrixToTransformationMatrix(pSceneNode->GetWorldMatrix(), worldMatrix);

					S32 id= AddDynamicShape( aftkSceneNodeList, worldMatrix );
					
					continue;
				} 

				/*
					Add constraint
				*/
				/*if(pSceneNode->GetType() == Aftk::E_ObjType_SceneNode)
				{
					bool find=false;
					for (U32 i = 0; i < pSceneNode->GetNumDynamicPlugs(); i++)
					{
						const Aftk::I_Plug& dynPlug = *pSceneNode->GetDynamicPlugPtr(i);
						if(dynPlug.GetName() == "constraintType")
						{
							S32 id=AddToConstraint(pSceneNode);
							find=true;
							break;
						}
					}
					if(find)
						continue;
				
				}*/

				if(pSceneNode->HasType(Aftk::E_ObjType_SceneMesh))
				{
					//might be fluid mesh
					/*if(name == "_water")
					{
						S32 id = AddToStaticWater(pSceneNode);
						continue;
					}*/
					//might be a cloth node
					bool find=false;
					for (U32 i = 0; i < pSceneNode->GetNumDynamicPlugs(); i++)
					{
						const Aftk::I_Plug& dynPlug = *pSceneNode->GetDynamicPlugPtr(i);
						if(dynPlug.GetName() == "Cloth")
						{
							S32 id=AddToCloth(pSceneNode);
							find=true;
							break;
						}
					}
					if(find)
						continue;
				}

				if(pSceneNode->HasType(Aftk::E_ObjType_SceneSpline))
				{
					aftkSceneStaticNodeList.PushBack(pSceneNode);
					continue;
				}


				if(pSceneNode->HasType(Aftk::E_ObjType_SceneMesh) | pSceneNode->HasType(Aftk::E_ObjType_SceneInstance))
				{
					if(AftkUt::Query::IsBaseInstance(pSceneNode))
					{
						continue;
					}

					Aftk::C_SceneNode* pParent = pSceneNode->GetParentPtr();
					
					std::string name=pParent->GetName();
					if(name.length() > 0 && ((name[0] =='_') || 
						(name[0] = 'M' && name[1] == '_' ) || 
						name[0] == 'm' && name[1] == '_'))
					{
						continue;
					}
					aftkSceneStaticNodeList.PushBack(pSceneNode);
					continue;
				}

				/*
					Add Constraint
				*/
				if(pSceneNode->GetType() == Aftk::E_ObjType_SceneConstraint)
				{
					aftkSceneConstraintNodeList.PushBack(pSceneNode);
				}
				
				
				std::cout << pSceneNode->GetName() << std::endl;

			}
			AddStaticNodes(aftkSceneStaticNodeList);

			U32 size = aftkSceneConstraintNodeList.Size();
			for(U32 i=0; i<size; ++i)
			{
				S32 id = AddToConstraint(aftkSceneConstraintNodeList[i]);	
			}

			Export(filePtr);

			delete filePtr;
			//m_Exporter.Export(fileName, resultFile);
		}
		//Shut down the Aftk.
		Aftk::C_Global::Destroy();
		return 1;
	}
};