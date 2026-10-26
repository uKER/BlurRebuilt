#ifndef LPVS_UTILITY_H
#define LPVS_UTILITY_H

#pragma once

inline const Aftk::C_BlindDataResource* GetBlindDataResourcePtr(const Aftk::C_Polygon* pPoly)
{
	const Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();

	S16 localBdIndex = pPoly->GetBlindDataMappingIndex();
	if (localBdIndex == -1)
	{
		return 0;
	}

	return pMesh->GetPolygonBlindDataMappingPtr(localBdIndex);
}

inline Aftk::Position3 ApplyMatrix(const Aftk::Matrix44& matrix, const Aftk::Position3& src)
{
	Aftk::Position3 dst;

	dst[0] = ( (matrix.At<0>().X() * src.X()) + (matrix.At<1>().X() * src.Y()) + (matrix.At<2>().X() * src.Z()) + matrix.At<3>().X() );
	dst[1] = ( (matrix.At<0>().Y() * src.X()) + (matrix.At<1>().Y() * src.Y()) + (matrix.At<2>().Y() * src.Z()) + matrix.At<3>().Y() );
	dst[2] = ( (matrix.At<0>().Z() * src.X()) + (matrix.At<1>().Z() * src.Y()) + (matrix.At<2>().Z() * src.Z()) + matrix.At<3>().Z() );

	return dst;
}

inline Aftk::Normal3 ApplyMatrix(const Aftk::Matrix44& normalTransform, const Aftk::Normal3 src)
{
	Aftk::Normal3 dst;
typedef UtMaths::MVector<UtMaths::VectorXYZ<F32> >					Vector3F32;
	Vector3F32 dstvec;

	dstvec[0] = ( (normalTransform.At<0>().X() * src.X()) + (normalTransform.At<1>().X() * src.Y()) + (normalTransform.At<2>().X() * src.Z()) );
	dstvec[1] = ( (normalTransform.At<0>().Y() * src.X()) + (normalTransform.At<1>().Y() * src.Y()) + (normalTransform.At<2>().Y() * src.Z()) );
	dstvec[2] = ( (normalTransform.At<0>().Z() * src.X()) + (normalTransform.At<1>().Z() * src.Y()) + (normalTransform.At<2>().Z() * src.Z()) );

	Normalise(&dstvec);

	dst[0] = dstvec[0];
	dst[1] = dstvec[1];
	dst[2] = dstvec[2];

	return dst;
}

inline const Aftk::C_MaterialResource* GetMaterialResourcePtr(const Aftk::C_Polygon* pPoly)
{
	const Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();

	S16 localMaterialIndex = pPoly->GetMaterialMappingIndex();
	if (localMaterialIndex == -1)
	{
		return 0;
	}

	return pMesh->GetPolygonMaterialMappingPtr(localMaterialIndex);
}

bool DoesNodeHaveVertexLightingActorProperties(Aftk::C_SceneNode *pNode);

int GetLayerIndexFromSceneNodePtr(Aftk::C_SceneNode *pNode, int iLayer);

#endif


