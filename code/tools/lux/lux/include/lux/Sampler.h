#pragma once

float	UniformConePdf(float f);
float3	UniformSampleCone(const float2 &vSample, float f);
void	UniformSampleTriangle(float u1, float u2, float *u, float *v);
void	ComputeStep1dCDF(float *f, int nSteps, float *c, float *cdf);
float	SampleStep1d(float *f, float *cdf, float c,int nSteps, float u, float *pdf);
void	Shuffle(float *samp, int count, int dims);

void ConcentricSampleDisk(float u1, float u2, float *dx, float *dy);
float3 CosineSampleHemisphere(float u1, float u2);

// random number generator
void init_genrand(unsigned long seed);
unsigned long genrand_int32(void);
float genrand_real1(void);
float genrand_real2(void);
inline unsigned long RandomUInt() { return genrand_int32(); }
inline float		RandomFloat() { return genrand_real1(); }

// Sampling Inline Functions
inline float VanDerCorput(unsigned int n, unsigned int scramble)
{
	n = (n << 16) | (n >> 16);
	n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
	n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
	n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
	n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
	n ^= scramble;
	return ((float)n / (float)0x100000000LL);
}
inline float Sobol2(unsigned int n, unsigned int scramble)
{
	for (unsigned int v = 1 << 31; n != 0; n >>= 1, v ^= v >> 1)
		if (n & 0x1) scramble ^= v;
	return ((float)scramble / (float)0x100000000LL);
}

inline void Sample02(unsigned int n, unsigned int scramble[2], float sample[2])
{
	sample[0] = VanDerCorput(n, scramble[0]);
	sample[1] = Sobol2(n, scramble[1]);
}

inline void LDShuffleScrambled1D(int nSamples, int nPixel, float *samples)
{
	unsigned int scramble = RandomUInt();
	for (unsigned int i = 0; i < nSamples * nPixel; ++i)
		samples[i] = VanDerCorput(i, scramble);
	for (unsigned int i = 0; i < nPixel; ++i)
		Shuffle(samples + i * nSamples, nSamples, 1);
	Shuffle(samples, nPixel, nSamples);
}

inline void LDShuffleScrambled2D(int nSamples,int nPixel, float *samples)
{
	unsigned int scramble[2] = { RandomUInt(), RandomUInt() };
	for (int i = 0; i < nSamples * nPixel; ++i)
		Sample02(i, scramble, &samples[2*i]);
	for (int i = 0; i < nPixel; ++i)
		Shuffle(samples + 2 * i * nSamples, nSamples, 2);
	Shuffle(samples, nPixel, 2 * nSamples);
}

inline float SmoothStep(float min, float max, float value) {
	float v = Clamp((value - min) / (max - min), 0.f, 1.f);
	return v * v * (-2.f * v  + 3.f);
}

inline float3 UniformSampleSphere(float u1, float u2) {
	float z = 1.f - 2.f * u1;
	float r = sqrtf(max(0.f, 1.f - z*z));
	float phi = 2.f * PI * u2;
	float x = r * cosf(phi);
	float y = r * sinf(phi);
	return float3(x, y, z);
}

inline void CoordinateSystem(const float3 &v1, float3 *v2, float3 *v3)
{
	float3 _v2;

	if (fabsf(v1.x) > fabsf(v1.y))
	{
		float invLen = 1.f / sqrtf(v1.x*v1.x + v1.z*v1.z);
		_v2 = float3(-v1.z * invLen, 0.f, v1.x * invLen);
	}
	else
	{
		float invLen = 1.f / sqrtf(v1.y*v1.y + v1.z*v1.z);
		_v2 = float3(0.f, v1.z * invLen, -v1.y * invLen);
	}
	*v3 = CrossProduct(v1, _v2);
	*v2 = _v2;
}

// ---

struct AASampleInfo
{
	float2	vSamplePos;

	// TODO: add more float2's for sampling directions etc.
};

class Sampler
{
public:
	AASampleInfo	*m_pImageSamples;
	int				m_nSamplesPerTexel;
	const static int m_iMaxSamples;

			Sampler(int nSamplerPerTexel);
			~Sampler();

	float2	GetImageSample(int i);
	void	GenerateSamples();
};


