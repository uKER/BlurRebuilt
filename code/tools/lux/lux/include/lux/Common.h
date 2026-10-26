#ifndef LUX_COMMON_H
#define LUX_COMMON_H

#define LIGHTMAP_JOB		0
#define SAMPLEPOINT_JOB		(1<<30)
#define SHLIGHTING_JOB		(2<<30)
#define VTXLIGHTING_JOB		(3<<30)
#define JOB_INDEX_MASK		0x3fffffff

#include <lux_common/SerialiseUtVectorToOst.h>

namespace LUX
{
	class LightingModelProperties;

	Vector3F32 Lerp(
		const Vector3F32& minimum, 
		const Vector3F32& maximum, 
		F32 factor);

	Position3 Lerp(
		const Position3& minimum, 
		const Position3& maximum, 
		F32 factor);

	bool DoesNodeNeedProcessing(Aftk::C_SceneNode *pNode);
	bool IsNodeASpeedTreeInstance(Aftk::C_SceneNode *pNode);
	bool DoesBaseInstanceNeedExporting(Aftk::C_SceneNode *pNode);

	inline ost::Engine& operator << (ost::Engine& ar, const Ut::BitArray128 &vec)
	{
		const U8 *in = vec.Data(); 
		ar << in[0] << in[1] << in[2] << in[3];

		return ar;
	}

	inline ost::Engine& operator >> (ost::Engine& ar, Ut::BitArray128 &vec)
	{
		U8 *in = vec.Data(); 
		ar >> in[0] >> in[1] >> in[2] >> in[3];

		return ar;
	}
}


typedef float3 Spectrum;

struct Visibility
{
	SD::Ray ray;

	void SetSegment(const float3 &vStart, const float3 &vEnd);
	void SetRay(const float3 &vStart, const float3 &vDir);
	bool Unoccluded(const SD::SpatialHierarchy *pScene);
};

inline float Lerp(float t, float v1, float v2)
{
	return (1.f - t) * v1 + t * v2;
}

inline float Clamp(float val, float low, float high)
{
	if (val < low)
		return low;
	else if (val > high)
		return high;

	return val;
}

inline float SphericalTheta(const float3 &v)
{
	return acosf(Clamp(v.z, -1.f, 1.f));
}
inline float SphericalPhi(const float3 &v)
{
	float p = atan2f(v.y, v.x);
	return (p < 0.f) ? p + 2.f*(float)M_PI : p;
}
inline float AbsDot(const float3 &v1, const float3 &v2)
{
	return fabsf(DotProduct(v1, v2));
}

enum NodeLightingMode
{
	LMode_NotSet,
	LMode_VtxColour,
	LMode_SH,
	LMode_Lightmap,
	LMode_DoNotLight,
};


// utility function for calculating decay - may be used by other lights
float GetDecay(Aftk::E_LightDecay type, float vDistSq);
float GetMaxRadius(const float3 &vIntensity);
float GetMaxRadius(const float3 &vIntensity, float fMin);

// expand polys flagged as double-sided into two separate polygons

// debugging utility function
float3 PosToColour(float3 v);

// serializing for float3 vectors
#include <old_nightmare_maths/serializevecmat.h>

void SortVectorsBySigns(float3 *pVectorList, int nVectors);

int	GetUVSetIndexFromMeshNode(Aftk::I_SceneMeshNode *pMesh);
int GetNumProcessors();
void SetDAZ();

bool DoesNodeHaveVertexLightingActorProperties(Aftk::C_SceneNode *pNode);
bool DoesNodeHaveSHLightingFlagSet(Aftk::C_SceneNode *pNode);
void ConvertNormalInstancesToRemappedInstances(Aftk::C_Scene *pScene, SD::SpatialHierarchy *pTree);
NodeLightingMode GetNodeLightingMode(const LUX::LightingModelProperties *pProp, Aftk::C_SceneNode *pNode);
void AddBlindData(Aftk::C_SceneNode *pNode, Aftk::C_GenericAttribute &attr);
bool CheckModelIsHighestLod(Aftk::C_SceneNode *pNode);
bool CalculateAftkNodeBoundingBox(Aftk::C_SceneNode *pNode, box3f &bbox, bool bWorldSpace = true);


// returns the unused colour value as an int. endian-independant
inline U32 GetUnusedColourValue()
{
	int iColour;
	U8 *pColour = (U8*)&iColour;

	pColour[0] = 0xde;
	pColour[1] = 0xad;
	pColour[2] = 0xff;
	pColour[3] = 0;

	return iColour;
}

// returns the unused colour value+1 as an int. endian-independant
inline U32 GetUnusedColourValuePlusOne()
{
	int iColour;
	U8 *pColour = (U8*)&iColour;

	pColour[0] = 0xde;
	pColour[1] = 0xad;
	pColour[2] = 0xfe;
	pColour[3] = 0;

	return iColour;
}

bool CheckBuggeredFloat(float f);
bool NodeIsLocatorWithSHData(Aftk::C_SceneNode *pNode);

Aftk::C_SceneMeshNode *ConvertRemappedInstanceToMesh(Aftk::C_SceneMeshRemappedInstanceNode *pInstance);
void ConvertRemappedInstancesToMesh(Aftk::C_Scene *pScene);
void SetLuxCustomDataToNode(Aftk::C_SceneNode *pNode, U8 *pData, int nc);

#endif


