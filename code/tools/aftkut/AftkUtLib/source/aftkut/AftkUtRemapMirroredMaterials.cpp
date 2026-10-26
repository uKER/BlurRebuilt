#include "stdafx.h"

using namespace AftkUt;

Aftk::C_Status C_RemapMirroredMaterials::Remap(
	Aftk::C_Scene *pScene,
	const std::string& namediscriminator,
	const std::string& nameconvention1,
	const std::string& nameconvention2,
	E_MirrorDirection mirrorDirection,
	S16 texCoordSet)
{

	Aftk::C_Status status;

	MaterialMapping materialMapping;
	BaseInstanceMapping instanceMapping;

	status = BuildMaterialRemappingTable(pScene, namediscriminator, nameconvention1, nameconvention2, materialMapping);
	if (!status)
		return status;

	for (MaterialMapping::Iterator matIt=materialMapping.Begin(); matIt!=materialMapping.End(); ++matIt)
	{
		LOGINFO(g_Log, "Remapping material %s to %s\n", matIt->GetKey()->GetContent().GetName().c_str(), matIt->GetValue()->GetContent().GetName().c_str());

		for (Aftk::C_SceneNodeConstIterator sceneIt(pScene); sceneIt; ++sceneIt)
		{
			if (!sceneIt->HasType(Aftk::E_ObjType_SceneMeshRemappable))
				continue;

			Aftk::C_SceneMeshRemappableNode& aftkRemappableNode = (Aftk::C_SceneMeshRemappableNode&)*sceneIt;

			S16 polyMappingIdx = aftkRemappableNode.GetPolygonMaterialMappingIndex(matIt->GetKey());
			if (polyMappingIdx != -1)
			{
				if (sceneIt->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
				{
					aftkRemappableNode.SetPolygonMaterialMapping(polyMappingIdx, matIt->GetValue());
					Aftk::C_SceneMeshRemappedInstanceNode& aftkRemappedInstanceNode = (Aftk::C_SceneMeshRemappedInstanceNode&) aftkRemappableNode;
					
					//Check if node is a directly pointing to the base instance
					if (aftkRemappedInstanceNode.GetSourceNodePtr()->GetType() == Aftk::E_ObjType_SceneMesh)
					{
						// See if we've already cloned the base instance mesh
						BaseInstanceMapping::Iterator instIt;
						for (instIt = instanceMapping.Begin(); instIt != instanceMapping.End(); ++instIt)
							if (instIt->GetKey() == matIt->GetKey())
								break;

						if (instIt != instanceMapping.End())
							// Set source to our cloned mesh
							aftkRemappedInstanceNode.SetSourceNodePtr(instIt->GetValue());
						else
						{
							// Clone the existing base instance mesh so that we can manipulate the texcoords later
							Aftk::C_SceneMeshNode *pNewMeshNode = (Aftk::C_SceneMeshNode*) aftkRemappedInstanceNode.GetSourceNodePtr()->Clone(pScene);
							std::string& name = pNewMeshNode->GetName();
							name +=  + "_mirrored";
							aftkRemappedInstanceNode.GetSourceNodePtr()->GetParentPtr()->AddChildPtr(pNewMeshNode);

							pNewMeshNode->SetPolygonMaterialMapping(polyMappingIdx, matIt->GetValue());
							aftkRemappedInstanceNode.SetSourceNodePtr(pNewMeshNode);
							instanceMapping[matIt->GetKey()] = pNewMeshNode;
						}
					}
				}
				else if (sceneIt->GetType() == Aftk::E_ObjType_SceneMesh)
				{
					instanceMapping[matIt->GetKey()] = (Aftk::C_SceneMeshNode*) &aftkRemappableNode;
				}
			}
		}
	}

	for (BaseInstanceMapping::Iterator instIt = instanceMapping.Begin(); instIt != instanceMapping.End(); ++instIt)
	{
		MirrorTextureCoords(instIt->GetValue(), materialMapping[instIt->GetKey()], mirrorDirection, texCoordSet);
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_RemapMirroredMaterials::BuildMaterialRemappingTable(
	Aftk::C_Scene *pScene,
	const std::string& namediscriminator,
	const std::string& nameconvention1,
	const std::string& nameconvention2,
	MaterialMapping& mapping)
{

	for (U32 iMat=0; iMat<pScene->GetNumMaterialNodes(); ++iMat)
	{
		const Aftk::C_MaterialResource* pMaterialResource = pScene->GetMaterialResourcePtr(iMat);
		{
			MaterialMapping::Iterator it;
			for (it  = mapping.Begin(); it!= mapping.End(); ++it)
				if (it->GetKey() == pMaterialResource)
					break;

			if (it != mapping.End())
				continue;
		}

		const Aftk::C_MaterialNode& materialNode = pMaterialResource->GetContent();

		std::string materialName = materialNode.GetName();
		std::transform(materialName.begin(), materialName.end(), materialName.begin(), std::tolower);

		if (materialName.find(namediscriminator) != std::string::npos)
		{
			std::size_t pos = materialName.find(nameconvention1);
			if (pos != std::string::npos)
			{
				std::string remapMaterialName =
					materialName.substr(0, pos) + nameconvention2 + materialName.substr(pos+nameconvention2.size()-1);

				const Aftk::C_MaterialResource* pRemapMaterialResource = pScene->GetMaterialResourcePtrByName(remapMaterialName);
				if (pRemapMaterialResource)
				{
					if (pRemapMaterialResource->GetReferenceCount() <= pMaterialResource->GetReferenceCount())
						mapping[pRemapMaterialResource] = pMaterialResource;
					else
						mapping[pMaterialResource] = pRemapMaterialResource;
				}
			}
			else
			{
				pos = materialName.find(nameconvention2);
				if (pos != std::string::npos)
				{
					std::string remapMaterialName =
						materialName.substr(0, pos) + nameconvention1 + materialName.substr(pos+nameconvention1.size()-1);

					const Aftk::C_MaterialResource* pRemapMaterialResource = pScene->GetMaterialResourcePtrByName(remapMaterialName);
					if (pRemapMaterialResource)
					{
						if (pRemapMaterialResource->GetReferenceCount() <= pMaterialResource->GetReferenceCount())
							mapping[pRemapMaterialResource] = pMaterialResource;
						else
							mapping[pMaterialResource] = pRemapMaterialResource;
					}
				}
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_RemapMirroredMaterials::MirrorTextureCoords(
	Aftk::C_SceneMeshNode* pMeshNode,
	const Aftk::C_MaterialResource* pMaterialRes,
	E_MirrorDirection mirrorDirection,
	S16 texCoordSet)
{

	S16 materialMappingIndex = pMeshNode->GetPolygonMaterialMappingIndex(pMaterialRes);
	if (materialMappingIndex == -1)
		return Aftk::C_Status::E_Status_Success;

	Ut::UtMap<S32, S32> newPVIndexMap;

	for (U32 iLod=0; iLod<pMeshNode->GetNumLodLevels(); ++iLod)
	{
		for (U32 jPoly=0; jPoly<pMeshNode->GetNumPolygons(iLod); ++jPoly)
		{
			Aftk::C_Polygon& polygon = pMeshNode->GetPolygon(iLod, jPoly);

			if (polygon.GetMaterialMappingIndex() == materialMappingIndex)
			{
				MirrorTextureCoords(pMeshNode, polygon, newPVIndexMap, mirrorDirection, texCoordSet);
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;

}

Aftk::C_Status C_RemapMirroredMaterials::MirrorTextureCoords(
	Aftk::C_SceneMeshNode *pMeshNode,
	Aftk::C_Polygon& polygon,
	Ut::UtMap<S32, S32>& newPVIndexMap,
	E_MirrorDirection mirrorDirection,
	S16 texCoordSet)
{
	for (U32 iVtx = 0; iVtx<polygon.GetNumPolygonVertices(); ++iVtx)
	{
		S32 oldPVIndex = polygon.GetPolygonVertexIndex(iVtx);
		S32 newPVIndex = -1;

		Ut::UtMap<S32, S32>::Iterator newPVIndexMapIt = newPVIndexMap.Find(oldPVIndex);
		if (newPVIndexMapIt == newPVIndexMap.End())
		{
			Aftk::C_PolygonVertex polyVtx = polygon.GetPolygonVertex(iVtx);

			for (U32 jTexSet = 0; jTexSet<polyVtx.GetNumTextureCoordinateIndices(); ++jTexSet)
			{
				if (texCoordSet == -1 || jTexSet == texCoordSet)
				{
					S32 coordIdx = polyVtx.GetTextureCoordinateIndex(jTexSet);
					Aftk::TextureCoordinate4 texCoord = pMeshNode->GetTextureCoordinate(jTexSet, coordIdx);
					(mirrorDirection == E_MirrorDirection_U) ? texCoord.x *= -1 : texCoord.y *= -1;
						
					S32 newCoordIdx = pMeshNode->AddTextureCoordinate(jTexSet, texCoord);
					polyVtx.SetTextureCoordinateIndex(jTexSet, newCoordIdx);
				}
			}

			newPVIndex = pMeshNode->AddPolygonVertex(polyVtx);
			newPVIndexMap[oldPVIndex] = newPVIndex;
		}
		else
			newPVIndex = newPVIndexMapIt->GetValue();
				
		polygon.SetPolygonVertexIndex(iVtx, newPVIndex);
	}

	return Aftk::C_Status::E_Status_Success;
}

