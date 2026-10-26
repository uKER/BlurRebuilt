#include "stdafx.h"
#include <lux_common/LpvsUtility.h>

using namespace LPVS;

Position3 LPVS::ApplyMatrix(const Matrix44& matrix, const Position3& src)
{
	Position3 dst;

	dst[0] = ( (matrix.At<0>().X() * src.X()) + (matrix.At<1>().X() * src.Y()) + (matrix.At<2>().X() * src.Z()) + matrix.At<3>().X() );
	dst[1] = ( (matrix.At<0>().Y() * src.X()) + (matrix.At<1>().Y() * src.Y()) + (matrix.At<2>().Y() * src.Z()) + matrix.At<3>().Y() );
	dst[2] = ( (matrix.At<0>().Z() * src.X()) + (matrix.At<1>().Z() * src.Y()) + (matrix.At<2>().Z() * src.Z()) + matrix.At<3>().Z() );

	return dst;
}

Normal3 LPVS::ApplyMatrix(const Matrix44& normalTransform, const Normal3 src)
{
	Normal3 dst;
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

HMat44 LPVS::ConvertMatrix(const Matrix44 &matrix)
{
	return HMat44(
		float4(matrix.At<0>().X(), matrix.At<0>().Y(), matrix.At<0>().Z(),0),
		float4(matrix.At<1>().X(), matrix.At<1>().Y(), matrix.At<1>().Z(),0), 
		float4(matrix.At<2>().X(), matrix.At<2>().Y(), matrix.At<2>().Z(),0), 
		float4(matrix.At<3>().X(), matrix.At<3>().Y(), matrix.At<3>().Z(),1));
}

double LPVS::DegToRad(double deg)
{
	return deg * PI/180;
}

double LPVS::RadToDeg(double rad)
{
	return rad * 180/PI;
}

void SetMaterialResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_MaterialResource* pRef)
{
	Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();

	if (pRef)
	{
		S16 materialLocalIndex = pMesh->AddPolygonMaterialMapping(pRef);
		assert(materialLocalIndex != -1);
		pPoly->SetMaterialMappingIndex(materialLocalIndex);
	}
	else
	{
		pPoly->SetMaterialMappingIndex(-1);
	}
}

void SetBlindDataResourcePtr(Aftk::C_Polygon* pPoly, const Aftk::C_BlindDataResource* pRef)
{
	Aftk::C_SceneMeshNode* pMesh = pPoly->GetMeshNodePtr();

	if (pRef)
	{
		S16 bdLocalIndex = pMesh->AddPolygonBlindDataMapping(pRef);
		assert(bdLocalIndex != -1);
		pPoly->SetBlindDataMappingIndex(bdLocalIndex);
	}
	else
	{
		pPoly->SetBlindDataMappingIndex(-1);
	}
}

bool DoesMatrixHaveNegativeScaling(Aftk::Matrix44 &mtx)
{
	float3 v1 = float3(mtx.At<0>().X(), mtx.At<0>().Y(), mtx.At<0>().Z());
	float3 v2 = float3(mtx.At<1>().X(), mtx.At<1>().Y(), mtx.At<1>().Z());
	float3 v3 = float3(mtx.At<2>().X(), mtx.At<2>().Y(), mtx.At<2>().Z());

	// check and warn about negative scaling on matrices
	float3 vec = CrossProduct(v1,v2);
	float f = DotProduct(vec, v3);

    return ( f < 0.0 );
}

float4 PreScaleSHVector(const float4 &sh)
{
static const float fSqrtPI = ((float)sqrtf(M_PI));
static const float fC0 = 1.0f/(2.0f*fSqrtPI);
static const float fC1 = (float)sqrt(3.0f)/(3.0f*fSqrtPI);

	return float4(fC0 * sh[0], -fC1 * sh[3], -fC1 * sh[1], fC1 * sh[2]);
	//dot = fC0 * sh[0] - fC1 * sh[1] * normal.y + fC1 * sh[2] * normal.z - fC1 * sh[3] * normal.x;
}

void PreScaleSHVector(float *pf)
{
	float4 &v = *(float4*)pf;

	v = PreScaleSHVector(v);
}

// this function is not provided by linux
void StrToLower(char *psz)
{
	while (*psz)
	{
		*psz = tolower(*psz);
		psz++;
	}
}

