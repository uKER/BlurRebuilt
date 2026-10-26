#include "stdafx.h"
#include "../Aftk/Ftk.h"

#include "NativeDefinitions.h"
#include "NativeUtility.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeFtk.h"
#include "NativeIObject.h"
#include "NativeNode.h"
#include "NativeSceneNode.h"
#include "NativeSceneMeshRemappableNode.h"
#include "NativeSceneMeshNode.h"
#include "NativePolygon.h"
#include "NativePolygonVertex.h"
#include "NativeComponentCluster.h"
#include "NativePolygonCluster.h"
#include <aftk/AftkUtility.h>

using namespace Aftk;

C_Status C_NativeSceneMeshNode::Serialise(
	C_NativeDiskWriter* pDiskWriter, 
	C_NativeFtk* pNativeFtk,
	const C_SceneMeshNode* pSceneMeshNode,
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders)
		pDiskWriter->BeginBlock(NATIVE_BLOCK_ID_MESH_NODE);

	//Write out the fragment of this node that comes from C_SceneMeshRemappableNode
	{
		//Setting false means that we don't use unnecessary block padding.
		//This is done simply to save disk space.
		status = C_NativeSceneMeshRemappableNode::Serialise(
			pDiskWriter, 
			pNativeFtk,
			pSceneMeshNode, 
			true);
		if (!status) return status;
		
		//Write out vertex positions
		{
			U32 n = (U32)pSceneMeshNode->m_vertexPositions.Size();
			*pDiskWriter << n;
			if (n > 0)
				pDiskWriter->WriteBytes(
					&pSceneMeshNode->m_vertexPositions[0], 
					sizeof(Position3)*n);
		}

		//Write out vertex normals
		{
			U32 n = (U32)pSceneMeshNode->m_vertexNormals.Size();
			*pDiskWriter << n;
			if (n > 0)
				pDiskWriter->WriteBytes(
					&pSceneMeshNode->m_vertexNormals[0], 
					sizeof(Normal3)*n);
		}

		//Write out texture coordinates
		//Number of texture coordinate sets
		*pDiskWriter << (U32)pSceneMeshNode->m_textureCoordinateSets.Size();
		//Loop through texture coordinate sets
		for (U32 set = 0; set < (U32)pSceneMeshNode->m_textureCoordinateSets.Size(); set++)
		{
			const Aftk::C_TextureCoordinateSet& uvSet = pSceneMeshNode->GetTextureCoordinateSet(set);
			//Write out the name of this texture coordinate set.
			*pDiskWriter << uvSet.GetName();

			//Write out texture coordinates
			{
				U32 n = (U32)uvSet.GetTextureCoordinateArray().Size();
				*pDiskWriter << n;
				if (n > 0)
				pDiskWriter->WriteBytes(
					&uvSet.GetTextureCoordinateArray()[0], 
					sizeof(TextureCoordinate4)*n);
			}

			//Write out vertex tangents
			{
				U32 n = (U32)uvSet.GetTangentArray().Size();
				*pDiskWriter << n;
				if (n > 0)
					pDiskWriter->WriteBytes(
						&uvSet.GetTangentArray()[0], 
						sizeof(Vector3F32)*n);
			}

			//Write out vertex binormal count
			{
				U32 n = (U32)uvSet.GetBinormalArray().Size();
				*pDiskWriter << n;
				if (n > 0)
					pDiskWriter->WriteBytes(
						&uvSet.GetBinormalArray()[0], 
						sizeof(Vector3F32)*n);
			}

		}

		//Write out vertex channel information
		//Number of vertex data channels
		*pDiskWriter << (U32)pSceneMeshNode->m_vertexDataChannelInfo.Size();
		//Loop through vertex channels
		for (U32 i = 0; i < (U32)pSceneMeshNode->m_vertexDataChannelInfo.Size(); i++)
		{
			//Write out the name of this texture coordinate set.
			*pDiskWriter << pSceneMeshNode->m_vertexDataChannelInfo[i].GetName();
			*pDiskWriter << (U32)pSceneMeshNode->m_vertexDataChannelInfo[i].GetDataType();
		}

		//Write out polygon vertices
		{
			*pDiskWriter << (U32)pSceneMeshNode->GetNumPolygonVertices();
			//Loop thru poly verts
			for (U32 iPolyVert = 0; iPolyVert < (U32)pSceneMeshNode->GetNumPolygonVertices();iPolyVert++)
			{
				const C_PolygonVertex* pPolygonVertex = 
					&pSceneMeshNode->GetPolygonVertex(iPolyVert);

				status = C_NativePolygonVertex::Serialise(
					pDiskWriter, 
					pNativeFtk, 
					pPolygonVertex, 
					pSceneMeshNode,
					false);
				if (!status) return status;
			}
		}

		{
			//Write out the number of lod levels.
			*pDiskWriter << pSceneMeshNode->GetNumLodLevels();
			for (U32 iLod = 0; iLod < pSceneMeshNode->GetNumLodLevels(); iLod++)
			{
				{
					//Write out lod level info
					const C_MeshLodLevelInfo* pInfo = &pSceneMeshNode->GetLodLevelInfo(iLod);
					*pDiskWriter << pInfo->GetName();
					*pDiskWriter << pInfo->GetThreshold();
				}
				//Write out the number of polygons in this set.
				*pDiskWriter << pSceneMeshNode->GetNumPolygons(iLod);
				for (U32 jPoly = 0; jPoly < pSceneMeshNode->GetNumPolygons(iLod); jPoly++)
				{
					const C_Polygon* pPolygon = &pSceneMeshNode->GetPolygon(iLod, jPoly);

					//Serialise polygons
					status = C_NativePolygon::Serialise(
						pDiskWriter, 
						pNativeFtk,
						pPolygon, 
						pSceneMeshNode,
						false);
					if (!status) return status;
				}
			}
		}

		//Write out our poly cluster information
		{
			//Write out polygon clusters
			*pDiskWriter << (U32)pSceneMeshNode->m_polygonClusters.Size();
			for (U32 i = 0; i < (U32)pSceneMeshNode->m_polygonClusters.Size(); i++)
			{
				const C_PolygonCluster* pPolygonCluster = pSceneMeshNode->m_polygonClusters[i];

				status = C_NativePolygonCluster::Serialise(
					pDiskWriter, 
					pNativeFtk,
					pPolygonCluster, 
					false);
				if (!status) return status;
			}
		}

		//Write out UV linking info
		{
			// Write the map size to disk
			Ut::UtMap<const C_MaterialResource*, C_SceneMeshRemappableNode::UVLinkageList>::ConstIterator iter;
			*pDiskWriter << (U32)pSceneMeshNode->m_uvLinkInfo.Size();
			
			for (iter  = pSceneMeshNode->m_uvLinkInfo.Begin();
				 iter != pSceneMeshNode->m_uvLinkInfo.End(); 
				 iter ++) {

			    // write the material's unique index to disk
				const C_MaterialResource* pMaterialRes = iter->GetKey();
				*pDiskWriter << (S32)pMaterialRes->GetIndex();

				// Get the material's UVLinkageList and write its size to disk
				C_SceneMeshRemappableNode::UVLinkageList currentUVLinkageList = iter->GetValue();
				*pDiskWriter << (U32)currentUVLinkageList.Size();

				// Loop through the UVLinkageList and write out its contents to disk
				for (U32 i = 0; i < (U32)currentUVLinkageList.Size(); i++) {

					*pDiskWriter << (S32)(currentUVLinkageList[i].pImageNode)->GetNodeIndex();
					*pDiskWriter << (unsigned int)currentUVLinkageList[i].uvIndex;

				}
			}
		}
	}
	
	if (useOuterBlockHeaders)
		pDiskWriter->EndBlock();
	
	return C_Status::E_Status_Success;
}

C_Status C_NativeSceneMeshNode::Deserialise(
	C_NativeDiskReader* pDiskReader, 
	C_NativeFtk* pNativeFtk, 
	C_SceneMeshNode* pSceneMeshNode,
	bool useOuterBlockHeaders)
{
	C_Status status;

	if (useOuterBlockHeaders)
	{
		if (!pDiskReader->FindBlock(NATIVE_BLOCK_ID_MESH_NODE))
			return C_Status(C_Status::E_Status_Failure, "Failed to find mesh node block.");
		pDiskReader->EnterBlock();
	}
	{
		//Prior to V143, the mesh class was derived from C_SceneNode, not
		//C_SceneMeshRemappableNode.
		if (pDiskReader->GetVersion() < 143)
		{
			//Read in the fragment of this node that comes from C_SceneNode
			C_SceneNode* pSceneNode = (C_SceneNode*)pSceneMeshNode;
			//Setting false means that we don't use unnecessary block padding.
			//This is done simply to save disk space.
			status = C_NativeSceneNode::Deserialise(
				pDiskReader, 
				pNativeFtk, 
				pSceneNode, 
				true);
			if (!status) return status;
		}
		else
		{
			status = C_NativeSceneMeshRemappableNode::Deserialise(
				pDiskReader,
				pNativeFtk,
				pSceneMeshNode,
				true);
			if (!status) return status;
		}

		//Read in vertex positions
		{
			U32 n;
			*pDiskReader >> n;
			if (n > 0)
			{
				pSceneMeshNode->m_vertexPositions.Resize(n);
				pDiskReader->ReadBytes(&pSceneMeshNode->m_vertexPositions[0], sizeof(Position3)*n);
			}
		}
		//Read in vertex normals
		{
			U32 n;
			*pDiskReader >> n;
			if (n > 0)
			{
				pSceneMeshNode->m_vertexNormals.Resize(n);
				pDiskReader->ReadBytes(&pSceneMeshNode->m_vertexNormals[0], sizeof(Normal3)*n);
			}
		}
		//Read in texture coordinate sets
		{
			//Number of texture coordinate sets
			U32 numSets;
			if (pDiskReader->GetVersion() >= 205)
			{
				*pDiskReader >> numSets;
			}
			else
			{
				U8 tmp;
				*pDiskReader >> tmp;
				numSets = tmp;
			}
			pSceneMeshNode->SetTextureCoordinateSetSize(numSets);
			//Loop through texture coordinate sets
			for (U32 set = 0; set < numSets; set++)
			{
				Aftk::C_TextureCoordinateSet& uvSet = pSceneMeshNode->GetTextureCoordinateSet(set);

				//Read in the name of this texture coordinate set
				std::string setName;
				*pDiskReader >> setName;
				uvSet.SetName(setName);

				//Read uvs
				{
					U32 n;
					*pDiskReader >> n;
					if (n > 0)
					{
						uvSet.GetTextureCoordinateArray().Resize(n);
						pDiskReader->ReadBytes(&uvSet.GetTextureCoordinateArray()[0], sizeof(TextureCoordinate4)*n);
					}
				}

				if (pDiskReader->GetVersion() > 145)
				{
					//Read tangents
					{
						U32 n;
						*pDiskReader >> n;
						if (n > 0)
						{
							uvSet.GetTangentArray().Resize(n);
							pDiskReader->ReadBytes(&uvSet.GetTangentArray()[0], sizeof(Vector3F32)*n);
						}
					}

					//Read binormals
					{
						U32 n;
						*pDiskReader >> n;
						if (n > 0)
						{
							uvSet.GetBinormalArray().Resize(n);
							pDiskReader->ReadBytes(&uvSet.GetBinormalArray()[0], sizeof(Vector3F32)*n);
						}
					}
				}
			}
		}
		

		//In v1.32 support for arbitrary vertex data channels was added.
		if (pDiskReader->GetVersion() >= 133)
		{
			U32 numVertexDataChannels;
			if (pDiskReader->GetVersion() >= 205)
			{
				*pDiskReader >> numVertexDataChannels;
			}
			else
			{
				U8 tmp;
				*pDiskReader >> tmp;
				numVertexDataChannels = tmp;
			}
			if (numVertexDataChannels > 0)
			{
				for (U32 i = 0; i < numVertexDataChannels; i++)
				{
					C_VertexDataChannelInfo info;

					std::string name;
					*pDiskReader >> name;
					info.SetName(name);

					if (pDiskReader->GetVersion() >= 150)
					{
						U32 dataType;
						*pDiskReader >> dataType;
						info.SetDataType((E_GenericAttributeType)dataType);
					}
					else
					{
						U32 bits;
						*pDiskReader >> bits;
						info.SetDataType(E_GenericAttributeType_Vector4F32);
					}

					pSceneMeshNode->SetVertexDataChannelInfo(i, info);
				}
			}
		}

		//IMPORTANT:
		//At version v142 shared polygon vertices were reintroduced!
		//Therefore between v131 and 141 we need to convert unshared poly verts on the polys to shared
		//polygon verts on the mesh. This is taken care of in the native polygon class.

		//We're using shared polygon vertices again as of v142.
		if (pDiskReader->GetVersion() >= 142)
		{
			if (pDiskReader->GetVersion() < 205)
				pDiskReader->EnterBlock();

			//Read in polygon vertices
			U32 numPolygonVertices;
			*pDiskReader >> numPolygonVertices;

			for (U32 i = 0; i < numPolygonVertices; i++)
			{
				pSceneMeshNode->AddPolygonVertex();
				C_PolygonVertex* pPv = 
					&pSceneMeshNode->GetPolygonVertex(i);
					
				//Deserialise polygon vertices
				status = C_NativePolygonVertex::Deserialise(
					pDiskReader,
					pNativeFtk, 
					pPv, 
					pSceneMeshNode,
					false);
				if (!status) return status;
			}
			if (pDiskReader->GetVersion() < 205)
				pDiskReader->ExitBlock();
		}

		if (pDiskReader->GetVersion() < 205)
			pDiskReader->EnterBlock();

		//Read in polygons
		if (pDiskReader->GetVersion() < 150)
		{
			U32 numPolys;
			*pDiskReader >> numPolys;
			pSceneMeshNode->SetPolygonInitialCapacity(0, numPolys);
			for (U32 jPoly = 0; jPoly < numPolys; jPoly++)
			{
				C_Polygon* pPoly;
				if (pDiskReader->GetVersion() >= 136)
				{
					//The function call below avoids adding polygons to the default cluster.
					//We set the correct cluster later during the mesh loading.
					pSceneMeshNode->AddPolygonNoCluster(0, C_Polygon());
					pPoly = &pSceneMeshNode->GetPolygon(0, jPoly);
				}
				else
				{
					//Old file format. Add to default cluster.
					pSceneMeshNode->AddPolygon(0,0, C_Polygon());
					pPoly = &pSceneMeshNode->GetPolygon(0, jPoly);
				}
				//Deserialise polygons
				status = C_NativePolygon::Deserialise(
					pDiskReader, 
					pNativeFtk, 
					pPoly,
					pSceneMeshNode,
					false);
				if (!status) return status;
			}
		}
		else
		{
			U32 numLodLevels;
			*pDiskReader >> numLodLevels;

			for (U32 iLod = 0; iLod < numLodLevels; iLod++)
			{
				{
					//Read lod level info
					C_MeshLodLevelInfo info;
					std::string name;
					*pDiskReader >> name;
					info.SetName(name);

					F32 threshold;
					*pDiskReader >> threshold;
					info.SetThreshold(threshold);
					pSceneMeshNode->SetLodLevelInfo(iLod, info);
				}

				U32 numPolys;
				*pDiskReader >> numPolys;
				pSceneMeshNode->SetPolygonInitialCapacity(iLod, numPolys);
				for (U32 jPoly = 0; jPoly < numPolys; jPoly++)
				{
					C_Polygon* pPoly;
					//The function call below avoids adding polygons to the default cluster.
					//We set the correct cluster later during the mesh loading.
					pSceneMeshNode->AddPolygonNoCluster(iLod, C_Polygon());
					pPoly = &pSceneMeshNode->GetPolygon(iLod, jPoly);
					//Deserialise polygons
					status = C_NativePolygon::Deserialise(
						pDiskReader, 
						pNativeFtk, 
						pPoly,
						pSceneMeshNode,
						false);
					if (!status) return status;
				}
			}
		}
		if (pDiskReader->GetVersion() < 205)
			pDiskReader->ExitBlock();

		if (pDiskReader->GetVersion() < 133)
		{
			//Prior to v1.32 we didn't have any vertex channel information. Therefore we need to spoof it.
            //Prior to v1.32 it was all vertex colour information.

			U32 numVertexChannels = 0;
			for (U32 i = 0; i < (U32)pSceneMeshNode->m_polygonVertices.Size(); i++)
			{
				const C_PolygonVertex* pPolyVert = &pSceneMeshNode->GetPolygonVertex(i);
				if (pPolyVert->GetNumData() > numVertexChannels)
				{
					numVertexChannels = pPolyVert->GetNumData();
				}
			}

			if (numVertexChannels > 0)
			{
				//Okay, now name all of our vertex channels as AFTK_DATA_CHANNEL_USAGE_VERTEX_COLOUR
				pSceneMeshNode->m_vertexDataChannelInfo.Reserve(numVertexChannels);
				for (U32 i = 0; i < numVertexChannels; i++)
				{
					pSceneMeshNode->m_vertexDataChannelInfo.PushBack();
					C_VertexDataChannelInfo* pChannelInfo = &pSceneMeshNode->m_vertexDataChannelInfo[i];
					pChannelInfo->SetName(AFTK_DATA_CHANNEL_USAGE_VERTEX_COLOUR);
					pChannelInfo->SetDataType(E_GenericAttributeType_Vector4F32);
				}
			}
		}

		if (pDiskReader->GetVersion() >= 136)
		{
			if (pDiskReader->GetVersion() < 205)
				pDiskReader->EnterBlock();
			//Read in our poly cluster information
			
			U32 numPolyClusters;
			*pDiskReader >> numPolyClusters;
			//Preallocate storage for polygon clusters.
			pSceneMeshNode->SetPolygonClusterInitialCapacity(numPolyClusters);
			for (U32 i = 0; i < (U32)numPolyClusters; i++)
			{
				if (i > 0)
				{
					//Add new polygon cluster.
					pSceneMeshNode->AddPolygonCluster();
				}
				//Get a pointer to our new polygon cluster.
				C_PolygonCluster* pPolygonCluster = 
					pSceneMeshNode->GetPolygonClusterPtr(i);
				//Read in poly cluster.
				status = C_NativePolygonCluster::Deserialise(
					pDiskReader, 
					pNativeFtk,
					pPolygonCluster,
					pSceneMeshNode,
					false);
				if (!status) return status;
			}

			if (pDiskReader->GetVersion() < 205)
				pDiskReader->ExitBlock();
		}

		//Read UV linking info
		if (pDiskReader->GetVersion() >= 170)
		{
			// Read the map size from disk
			U32 mapSize;
			*pDiskReader >> mapSize;

			for (U32 i = 0; i < mapSize; i++)
			{
			    // Get the material's unique index from disk
				S32 materialIndex;
				*pDiskReader >> materialIndex;

				// Get the material from the index
				const C_MaterialResource* pMatRes = 
					pSceneMeshNode->GetScenePtr()->GetMaterialResourcePtr(materialIndex);
				const C_MaterialNode* pMaterial = &pMatRes->GetContent();

				// Get the material's UVLinkageList size from disk
				U32 uvLinkageListSize;
				*pDiskReader >> uvLinkageListSize;

				if (uvLinkageListSize > 0) 
				{
					// Create a new UVLinkageList
					C_SceneMeshRemappableNode::UVLinkageList uvLinkageList;

					// Add each C_UvLink object back to the Mesh's UVLinkageList
					for (U32 j = 0; j < uvLinkageListSize; j++) {

						// Get the instance of the C_ShaderImageNode from disk
						S32 imageNodeIndex;
						*pDiskReader >> imageNodeIndex;

						// Get the imageNode from the index
						const C_ShaderImageNode* pImage = (const C_ShaderImageNode*)pMaterial->GetShaderNodePtr(imageNodeIndex);

						// Get the uvIndex from disk
						unsigned int uvIndex;
						*pDiskReader >> uvIndex;

						// Create a new C_UvLink object
						C_SceneMeshRemappableNode::C_UvLink newUVLink;
						newUVLink.pImageNode = pImage;
						newUVLink.uvIndex = uvIndex;

						// Add the new C_UvLink to the UVLinkageList
						uvLinkageList.PushBack(newUVLink);

					}

					// Add the material to the map with its UVLinkageList
					pSceneMeshNode->m_uvLinkInfo[pMatRes] = uvLinkageList;
				}
			}
		}
	}
	if (useOuterBlockHeaders)
		pDiskReader->ExitBlock();
	return C_Status::E_Status_Success;
}

