#pragma once

namespace Aftk
{
typedef const Aftk::C_SharedResource< Aftk::C_MaterialNode > C_MatResource;
}

inline Position3 ApplyMatrix(const Matrix44& matrix, const Position3& src)
{
	Position3 dst;

	dst[0] = ( (matrix.At<0>().X() * src.X()) + (matrix.At<1>().X() * src.Y()) + (matrix.At<2>().X() * src.Z()) + matrix.At<3>().X() );
	dst[1] = ( (matrix.At<0>().Y() * src.X()) + (matrix.At<1>().Y() * src.Y()) + (matrix.At<2>().Y() * src.Z()) + matrix.At<3>().Y() );
	dst[2] = ( (matrix.At<0>().Z() * src.X()) + (matrix.At<1>().Z() * src.Y()) + (matrix.At<2>().Z() * src.Z()) + matrix.At<3>().Z() );

	return dst;
}

inline const Aftk::C_MatResource* GetMaterialResourcePtr(const Aftk::C_Polygon* pPoly)
{
	const Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();

	S16 localMaterialIndex = pPoly->GetMaterialMappingIndex();
	if (localMaterialIndex == -1)
	{
		return 0;
	}

	return pMesh->GetPolygonMaterialMappingPtr(localMaterialIndex);
}

// get blind data associated with a node
inline bool GetNodeBlindData(const Aftk::C_SceneNode *pNode, std::string &text)
{
    // get a node's blind data
	if (pNode->GetBlindDataResourcePtr())
	{
		const Aftk::C_BlindDataNode *pBlindData = &pNode->GetBlindDataResourcePtr()->GetContent();

		if (pBlindData)
		{
			for (int i=0; i<pBlindData->GetNumAttributes(); i++)
			{
				if (pBlindData->GetAttribute(i).GetName() == "actorString")
				{
					U32 data;
					const Aftk::C_GenericAttribute *pAttr = &pBlindData->GetAttribute(i);
					if (pAttr)
					{
						pAttr->GetValue(text);
						return true;
					}
				}
			}
		}
	}
	return false;
}
