#include "stdafx.h"
#include "vis_core/sampler.h"
#include "vis_core/util.h"
#include <algorithm>
#include <AftkUt/AftkUtNodeFlagParser.h>

using std::swap;

bool gbCarMode = false;
bool gbCrowdPvsLayerOnly = false;

float UniformConePdf(float f)
{
	return 1.f / (2.f * PI * (1.f - f));
}

float3 UniformSampleCone(const float2 &vSample, float f)
{
	float costheta = Lerp(vSample.x, f, 1.f);
	float sintheta = sqrtf(1.f - costheta*costheta);
	float phi = vSample.y * 2.f * PI;
	return float3(cosf(phi) * sintheta,
	              sinf(phi) * sintheta,
		          costheta);

}

void UniformSampleTriangle(float u1, float u2, float *u, float *v)
{
	float su1 = sqrtf(u1);
	*u = 1.f - su1;
	*v = u2 * su1;
}

void ComputeStep1dCDF(float *f, int nSteps, float *c, float *cdf)
{
	// Compute integral of step function at $x_i$
	int i;
	cdf[0] = 0.;
	for (i = 1; i < nSteps+1; ++i)
		cdf[i] = cdf[i-1] + f[i-1] / nSteps;
	// Transform step function integral into cdf
	*c = cdf[nSteps];
	for (i = 1; i < nSteps+1; ++i)
		cdf[i] /= *c;
}

float SampleStep1d(float *f, float *cdf, float c,int nSteps, float u, float *pdf)
{
	// Find surrounding cdf segments
	float *ptr = std::lower_bound(cdf, cdf+nSteps+1, u);
	int offset = (int) (ptr-cdf-1);
	// Return offset along current cdf segment
	u = (u - cdf[offset]) / (cdf[offset+1] - cdf[offset]);
	*pdf = f[offset] / c;
	return (float)(offset + u) / (float)nSteps;
}


float GetFloatRand()
{
	return (float)rand()/(float)RAND_MAX;
}

void Shuffle(float *samp, int count, int dims)
{
	for (int i = 0; i < count; ++i)
	{
		int other = RandomUInt() % count;
		for (int j = 0; j < dims; ++j)
			swap(samp[dims*i + j], samp[dims*other + j]);
	}
}

void ConcentricSampleDisk(float u1, float u2, float *dx, float *dy)
{
	float r, theta;
	// Map uniform random numbers to $[-1,1]^2$
	float sx = 2 * u1 - 1;
	float sy = 2 * u2 - 1;
	// Map square to $(r,\theta)$
	// Handle degeneracy at the origin
	if (sx == 0.0 && sy == 0.0) {
		*dx = 0.0;
		*dy = 0.0;
		return;
	}
	if (sx >= -sy) {
		if (sx > sy) {
			// Handle first region of disk
			r = sx;
			if (sy > 0.0)
				theta = sy/r;
			else
				theta = 8.0f + sy/r;
		}
		else {
			// Handle second region of disk
			r = sy;
			theta = 2.0f - sx/r;
		}
	}
	else {
		if (sx <= sy) {
			// Handle third region of disk
			r = -sx;
			theta = 4.0f - sy/r;
		}
		else {
			// Handle fourth region of disk
			r = -sy;
			theta = 6.0f + sx/r;
		}
	}
	theta *= PI / 4.f;
	*dx = r*cosf(theta);
	*dy = r*sinf(theta);
}

float3 CosineSampleHemisphere(float u1, float u2)
{
	float3 ret;
	ConcentricSampleDisk(u1, u2, &ret.x, &ret.y);
	ret.z = sqrtf(max(0.f, 1.f - ret.x*ret.x - ret.y*ret.y));
	return ret;
}

// ---

// Sampler class - generates and stores all the samples for a texel
Sampler::Sampler(int nSamplesPerTexel)
{
	m_nSamplesPerTexel = nSamplesPerTexel;

	// allocate space to store the samples
	m_pImageSamples = new AASampleInfo[nSamplesPerTexel];

	GenerateSamples();
}

Sampler::~Sampler()
{
	delete m_pImageSamples;
}

// generate a set of samples for this texel
void Sampler::GenerateSamples()
{
	// if anti-aliasing is turned off, generate just one sample in the middle of the texel
	if (m_nSamplesPerTexel == 1)
	{
		m_pImageSamples[0].vSamplePos = float2(0.5,0.5);
		return;
	}

	float2 *pfBuffer = new float2[m_nSamplesPerTexel];

	// generate samples for AA image positions
	LDShuffleScrambled2D(1,m_nSamplesPerTexel, (float*)pfBuffer);
	// copy them into the array
	for (int i=0; i<m_nSamplesPerTexel; i++)
		m_pImageSamples[i].vSamplePos = pfBuffer[i];

	// TODO: generate more samples for sampling direction etc.
	delete pfBuffer;
}

// get uv image position AA sample
float2 Sampler::GetImageSample(int i)
{
	assert(i>=0 && i<m_nSamplesPerTexel);
	return m_pImageSamples[i].vSamplePos;
}

// Random Number State
// Makoto Matsumoto and Takuji Nishimura,
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */
// Random Number Functions
static void init_genrand(unsigned long seed) {
	mt[0]= seed & 0xffffffffUL;
	for (mti=1; mti<N; mti++) {
		mt[mti] =
		(1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
		/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
		/* In the previous versions, MSBs of the seed affect   */
		/* only MSBs of the array mt[].                        */
		/* 2002/01/09 modified by Makoto Matsumoto             */
		mt[mti] &= 0xffffffffUL;
		/* for >32 bit machines */
	}
}
unsigned long genrand_int32(void)
{
	unsigned long y;
	static unsigned long mag01[2]={0x0UL, MATRIX_A};
	/* mag01[x] = x * MATRIX_A  for x=0,1 */

	if (mti >= N) { /* generate N words at one time */
		int kk;

		if (mti == N+1)   /* if init_genrand() has not been called, */
			init_genrand(5489UL); /* default initial seed */

		for (kk=0;kk<N-M;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		for (;kk<N-1;kk++) {
			y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
			mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
		mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

		mti = 0;
	}

	y = mt[mti++];

	/* Tempering */
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);

	return y;
}
/* generates a random number on [0,1]-real-interval */
float genrand_real1(void)
{
	return genrand_int32()*((float)1.0/(float)4294967295.0);
	/* divided by 2^32-1 */
}
/* generates a random number on [0,1)-real-interval */
float genrand_real2(void)
{
	return genrand_int32()*((float)1.0/(float)4294967296.0);
	/* divided by 2^32 */
}

// returns true if the node is highest lod or if it has no lods
bool CheckModelIsHighestLod(Aftk::C_SceneNode *pNode)
{
	// check for LODs
	Aftk::C_SceneNode *pParent = pNode->GetParentPtr();
	if (pParent && pParent->GetType() == Aftk::E_ObjType_SceneLodGroup)
	{
		Aftk::C_SceneLodGroupNode *pLG = (Aftk::C_SceneLodGroupNode*)pParent;
		if (pLG->GetNumLodLevels() == 0)
			return true;	// lod group without any lods at all - shouldn't happen technically

		// if it's part of a lod group, but it's not the highest lod, ignore it
		Aftk::C_GroupLodLevelInfo &info = pLG->GetLodLevelInfo(0);
		if (info.GetSceneNodePtr() != pNode)
			return false;
	}
	return true;
}

bool IsThisARouteNode(Aftk::C_SceneNode *pNode);

// check whether a node needs processing by the lightmapper
extern char gpszOnlyLightObject[]; // this is in globalilluminationmodel.cpp
bool DoesNodeNeedProcessing(Aftk::C_SceneNode *pNode, bool bAcceptRoutes)
{
	if (gbCarMode)
	{
		if (pNode->GetType() == Aftk::E_ObjType_SceneMesh)
		{
			// skip stuff with 'glass' in the material name
			Aftk::C_SceneMeshNode *pMesh = (Aftk::C_SceneMeshNode*)pNode;
			if (pMesh->GetNumPolygons(0)==0)
				return false;

			typedef const Aftk::C_SharedResource< Aftk::C_MaterialNode > AftkMatResource;

			AftkMatResource *pMatRes = GetMaterialResourcePtr(&((Aftk::C_SceneMeshNode*)pNode)->GetPolygon(0));
			if (pMatRes)
			{
				const char *pszMatName = pMatRes->GetContent().GetName().c_str();
				if (strstr(pszMatName,"Glass"))
					return false;
			}
		}
	}

	{
		// get actor properties string from blind data
		AftkUt::C_NodeFlagParser parser(*pNode);
		{
			// don't export tree instances which have been made into billboards
			if (parser.GetFlagGroupCategory() == "Effect" && parser.GetFlagGroupName() == "TreeBillboard")
				return false;

			bool pvsExclude = false;
			if (parser.GetParameterValue("DoNotPVS", pvsExclude) && pvsExclude)
				return false;
		}
	}
/*
	// don't light damage parts for cars for Chad
	if (strstr(pNode->GetName().c_str(), "damage") ||
		strstr(pNode->GetName().c_str(), "moving") ||
		strstr(pNode->GetName().c_str(), "scratch") ||
		strstr(pNode->GetName().c_str(), "Damage") ||
		strstr(pNode->GetName().c_str(), "Moving") ||
		strstr(pNode->GetName().c_str(), "Scratch"))
		return false;
*/
	// skip things in layers called 'sky'
	Aftk::C_LayerNode *pLayer = pNode->GetLayerPtr(0);
	if (pLayer)
	{
		if (!stricmp(pLayer->GetName().c_str(), "Sky"))
			return false;

		if (!stricmp(pLayer->GetName().c_str(), "Occluders"))
			return false;

		if (!stricmp(pLayer->GetName().c_str(), "NavMesh"))
			return false;

		if (!stricmp(pLayer->GetName().c_str(), "Noexport"))
			return false;

		if (strstr(pLayer->GetName().c_str(), "rotoscope"))
			return false;

		if (strstr(pLayer->GetName().c_str(), "Rotoscope"))
			return false;

		// route layers are now enabled/disabled from the XML file
		if (!bAcceptRoutes)
		{
			if (IsThisARouteNode(pNode))
				return false;
		}
	}

	// treat low-lods as route items
	if (!bAcceptRoutes)
	{
		if (!CheckModelIsHighestLod(pNode))
			return false;
	}

	{
		// ignore camera nodes
		AftkUt::C_NodeFlagParser parser(*pNode);
		std::string strClass, strName;
		if (parser.GetFlagGroupCategory() == "RouteItem" && parser.GetFlagGroupName() == "TVCameraZone")
			return false;
	}
	// some handy code to make it only process one particular object - useful for debugging
	// has to be called "polySurface4" or we'll skip it
//	if (gpszOnlyLightObject[0]!=0 && stricmp(pNode->GetName().c_str(), gpszOnlyLightObject))
//		return false;

	return true;
}

void SetPolygonColour(Aftk::C_Polygon *pPoly, int iColour)
{
	Aftk::C_SceneMeshNode *pNode = pPoly->GetMeshNodePtr();

	if (pNode->GetNumVertexDataChannels() == 0)
	{
		Aftk::C_VertexDataChannelInfo avdci;
		avdci.SetName("vertexColour");
		avdci.SetDataType(Aftk::E_GenericAttributeType_U32);
		pNode->SetVertexDataChannelInfo(0, avdci);
	}

	// set vertex colour here depending on visibility
	for (int iVtx=0; iVtx<3; iVtx++)
	{
		Aftk::C_PolygonVertex *pVtx = &pPoly->GetPolygonVertex(iVtx);

		pVtx->SetData(0, iColour);
	}
}
