#ifndef LPVS_UTILITY_H
#define LPVS_UTILITY_H

#pragma once

#include <Aftk/Aftk.h>

namespace LPVS
{
	Position3 ApplyMatrix(const Matrix44& matrix, const Position3& src);
	Normal3 ApplyMatrix(const Matrix44& matrix, const Normal3 src);
	HMat44 ConvertMatrix(const Matrix44 &matrix);

	double DegToRad(double deg);
	double RadToDeg(double rad);
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

void PreScaleSHVector(float *pf);
float4 PreScaleSHVector(const float4 &v);

void SetMaterialResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_MaterialResource* pRef);

void SetBlindDataResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_BlindDataResource* pRef);

bool DoesMatrixHaveNegativeScaling(Aftk::Matrix44 &mtx);

void StrToLower(char *);

inline HVec3::HVec3(const UtMVectorXYZ& p)
{
	*this = HVec3(p[0], p[1], p[2]);
}

inline const HVec3& HVec3::operator =(const UtMVectorXYZ& p)    
{ 
	*this = HVec3(p[0], p[1], p[2]);
	return *this;
}

inline HVec3::HVec3(const UtMPointXYZ& p)
{
	*this = HVec3(p[0], p[1], p[2]);
}

inline const HVec3& HVec3::operator =(const UtMPointXYZ& p)    
{ 
	*this = HVec3(p[0], p[1], p[2]);
	return *this;
}

inline HVec3::HVec3(const UtMNormalXYZ& p)
{
	*this = HVec3(p[0], p[1], p[2]);
}

inline const HVec3& HVec3::operator =(const UtMNormalXYZ& p)    
{ 
	*this = HVec3(p[0], p[1], p[2]);
	return *this;
}

#endif
