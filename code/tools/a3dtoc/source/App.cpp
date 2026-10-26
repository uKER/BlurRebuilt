#include "stdafx.h"
#include "aftk/aftk.h"

int main(int argc, char* argv[])
{
	if (argc < 3) {
		std::cerr << "Not enough arguments" << std::endl << "Expected: <input.a3d> <output.c>";
		return 1;
	}

	//Initialise the Aftk. This must be called before any Aftk features are used as it starts the
	//Aftk's memory manager.
	Aftk::C_Global::Create();
	{
		//This will store the results of our read and write operations.
		Aftk::C_Status status;
		
		//This is our scene instance.
		Aftk::C_Scene aftkScene;

		//Read in an A3D file from disk.
		status = aftkScene.Read(argv[1]);
		//Check whether we succeed or not.
		if (!status)
		{
			std::cout << status.GetErrorString() << std::endl;
			return -1;
		}
		else
			std::cout << "File read okay." << std::endl;

		std::ofstream output(argv[2]);

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

			/*if (pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				Aftk::C_SceneMeshNode* pMesh = (Aftk::C_SceneMeshNode*)pSceneNode;

				int vertexCount = pMesh->GetNumVertexPositions();
				
				Aftk::C_MeshTriangulate::Triangulate(pMesh);

				pMesh->Evaluate();
				
				pMesh->ApplyMatrix(pMesh->GetLocalMatrix());

				output << "U32 Mesh" << pSceneNode->GetName() << "Vertex_Count = " << pMesh->GetNumVertexPositions() << ";" << std::endl;
				output << "U32 Mesh" << pSceneNode->GetName() << "Index_Count = " << pMesh->GetNumPolygons(0)*3 << ";" << std::endl << std::endl;

				float minX = 10000000;
				float maxX = -10000000;
				float minY = 10000000;
				float maxY = -10000000;
				float minZ = 10000000;
				float maxZ = -10000000;

				output << "F32 Mesh" << pSceneNode->GetName() << "VertexPosition[] = {";
				for (U32 i=0; i<pMesh->GetNumVertexPositions(); i++)
				{
					Aftk::Position3 position = pMesh->GetVertexPosition(i);
					output << position.X() << "," << position.Y() << "," << -position.Z() << "," << std::endl;

					if (position.X() < minX)
						minX = position.X();

					if (position.X() > maxX)
						maxX = position.X();

					if (position.Y() < minY)
						minY = position.Y();

					if (position.Y() > maxY)
						maxY = position.Y();

					if (position.Z() < minZ)
						minZ = position.Z();

					if (position.Z() > maxZ)
						maxZ = position.Z();
				}
				output << "};" << std::endl << std::endl;

				output << "F32 Mesh" << pSceneNode->GetName() << "Extents_MinX = " << minX << ";" << std::endl;
				output << "F32 Mesh" << pSceneNode->GetName() << "Extents_MaxX = " << maxX << ";" << std::endl;
				output << "F32 Mesh" << pSceneNode->GetName() << "Extents_MinY = " << minY << ";" << std::endl;
				output << "F32 Mesh" << pSceneNode->GetName() << "Extents_MaxY = " << maxY << ";" << std::endl;
				output << "F32 Mesh" << pSceneNode->GetName() << "Extents_MinZ = " << minZ << ";" << std::endl;
				output << "F32 Mesh" << pSceneNode->GetName() << "Extents_MaxZ = " << maxZ << ";" << std::endl;

				output << "F32 Mesh" << pSceneNode->GetName() << "VertexNormal[] = {";
				for (U32 i=0; i<pMesh->GetNumVertexPositions(); i++)
				{
					int normalIndex = pMesh->GetPolygonVertex(i).GetNormalIndex();
					Aftk::Normal3 normal = pMesh->GetVertexNormal(normalIndex);
					output << normal.X() << "," << normal.Y() << "," << normal.Z() << "," << std::endl;
				}
				output << "};" << std::endl << std::endl;

				output << "U32 Mesh" << pSceneNode->GetName() << "Index[] = {";
				for (U32 i=0; i<pMesh->GetNumPolygons(0); i++)
				{
					const Aftk::C_Polygon& poly = pMesh->GetPolygon(i);
					output << poly.GetPolygonVertex(0).GetPositionIndex() << ","
						   << poly.GetPolygonVertex(1).GetPositionIndex() << ","
						   << poly.GetPolygonVertex(2).GetPositionIndex() << ","
						   << std::endl;
				}
				output << "};" << std::endl;
			}*/

			if (pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				Aftk::C_SceneMeshNode* pMesh = (Aftk::C_SceneMeshNode*)pSceneNode;

				int vertexCount = pMesh->GetNumVertexPositions();
				
				Aftk::C_MeshTriangulate::Triangulate(pMesh);

				pMesh->Evaluate();
				
				pMesh->ApplyMatrix(pMesh->GetLocalMatrix());

				output << "U32 Mesh" << pSceneNode->GetName() << "Vertex_Count = " << pMesh->GetNumPolygons(0)*3 << ";" << std::endl;
				output << "U32 Mesh" << pSceneNode->GetName() << "Index_Count = " << pMesh->GetNumPolygons(0)*3 << ";" << std::endl << std::endl;

				float minX = 10000000;
				float maxX = -10000000;
				float minY = 10000000;
				float maxY = -10000000;
				float minZ = 10000000;
				float maxZ = -10000000;

				output << "F32 Mesh" << pSceneNode->GetName() << "VertexPosition[] = {";
				for (U32 i=0; i<pMesh->GetNumPolygons(0); i++)
				{
					Aftk::C_Polygon poly = pMesh->GetPolygon(0,i);
					for (int j=0; j<3; j++) {
						Aftk::Position3 position = pMesh->GetVertexPosition(poly.GetPolygonVertex(j).GetPositionIndex());
						output << position.X() << "," << position.Y() << "," << -position.Z() << "," << std::endl;

						if (position.X() < minX)
							minX = position.X();

						if (position.X() > maxX)
							maxX = position.X();

						if (position.Y() < minY)
							minY = position.Y();

						if (position.Y() > maxY)
							maxY = position.Y();

						if (position.Z() < minZ)
							minZ = position.Z();

						if (position.Z() > maxZ)
							maxZ = position.Z();
					}
				}
				output << "};" << std::endl << std::endl;

				output << "F32 Mesh" << pSceneNode->GetName() << "Extents_MinX = " << minX << ";" << std::endl;
				output << "F32 Mesh" << pSceneNode->GetName() << "Extents_MaxX = " << maxX << ";" << std::endl;
				output << "F32 Mesh" << pSceneNode->GetName() << "Extents_MinY = " << minY << ";" << std::endl;
				output << "F32 Mesh" << pSceneNode->GetName() << "Extents_MaxY = " << maxY << ";" << std::endl;
				output << "F32 Mesh" << pSceneNode->GetName() << "Extents_MinZ = " << minZ << ";" << std::endl;
				output << "F32 Mesh" << pSceneNode->GetName() << "Extents_MaxZ = " << maxZ << ";" << std::endl;

				output << "F32 Mesh" << pSceneNode->GetName() << "VertexNormal[] = {";
				for (U32 i=0; i<pMesh->GetNumPolygons(0); i++)
				{
					Aftk::C_Polygon poly = pMesh->GetPolygon(0,i);
					for (int j=0; j<3; j++) {
						int normalIndex = poly.GetPolygonVertex(j).GetNormalIndex();
						Aftk::Normal3 normal = pMesh->GetVertexNormal(normalIndex);
						output << normal.X() << "," << normal.Y() << "," << normal.Z() << "," << std::endl;
					}
				}
				output << "};" << std::endl << std::endl;

				output << "U32 Mesh" << pSceneNode->GetName() << "Index[] = {";
				for (U32 i=0; i<pMesh->GetNumPolygons(0); i++)
				{
					const Aftk::C_Polygon& poly = pMesh->GetPolygon(i);
					output << (i*3) << ","
						   << (i*3)+1 << ","
						   << (i*3)+2 << ","
						   << std::endl;
				}
				output << "};" << std::endl;
			}
		}

		output.close();
	}

	Aftk::C_Global::Destroy();

	return 0;
}

