#ifndef RENDER_POSTPROCESS_UTIL_H
#define RENDER_POSTPROCESS_UTIL_H

#ifdef _MSC_VER
#pragma once
#endif

namespace PostProcess
{

// some useful functions for post processing

F32 INLINE GaussianDistribution(F32 x, F32 y, F32 rho)
{
	F32 g = 1.0f / sqrtf(2.0f * Maths::Pi2 * rho * rho);
    g *= expf(-(x * x + y * y)/(2 * rho * rho));

    return g;
}

template<int N>
void GetGaussianSamples(U32 iWidth, U32 iHeight, MathsStorage::Float4* vWeights, F32 fDeviation = 2.0f, F32 fMultiplier = 1.0f)
{
	static const S32 N2 = (N + 1) / 2;
	BIZ_STATIC_ASSERT(N%2 == 1);
    S32 i = 0;
    F32 tu, tv;
	
	if(iWidth > 0)
		tu = 1.0f / (F32)iWidth;
	else
		tu = 0.0f;

	if(iHeight > 0)
		tv = 1.0f / (F32)iHeight;
	else
		tv = 0.0f;

    // Fill the center texel
	F32 fWeight = GaussianDistribution(0, 0, fDeviation);
	F32 fTotalWeight = fWeight;
	vWeights[0] = MathsStorage::Float4(0.0f, 0.0f, 0.0f, fWeight);
    
    // Fill the first half
    for(S32 i = 1; i < N2; ++i)
    {
        // Get the Gaussian intensity for this offset
        fWeight = GaussianDistribution( (F32)i, 0, fDeviation );
		fTotalWeight += fWeight * 2;
        vWeights[i] = MathsStorage::Float4(i * tu, i * tv, 0.0f, fWeight);
    }

    // Mirror to the second half
	for(S32 i = N2; i < N; ++i)
    {
        vWeights[i] = vWeights[i-N2+1];
		vWeights[i].x *= -1.0f;
		vWeights[i].y *= -1.0f;
    }

	F32 fWeightModifier = fMultiplier / fTotalWeight;
	
	for(S32 i = 0; i < N; ++i)
    {
		vWeights[i].w *= fWeightModifier;
	}
}

template<int N>
void GetBilinearGaussianSamples(U32 iWidth, U32 iHeight, MathsStorage::Float4* vWeights, F32 fDeviation = 2.0f, F32 fMultiplier = 1.0f)
{
	static const S32 N2 = N / 2;
	BIZ_STATIC_ASSERT(N%2 == 0);
	S32 i = 0;
	F32 tu, tv;

	if(iWidth > 0)
		tu = 1.0f / (F32)iWidth;
	else
		tu = 0.0f;

	if(iHeight > 0)
		tv = 1.0f / (F32)iHeight;
	else
		tv = 0.0f;

	// Fill the first half
	F32 fTotalWeight = 0.0f;
	for(S32 i = 0; i < N2; ++i)
	{
		// Get the Gaussian intensity for this offset
		F32 fWeight = GaussianDistribution(i + 0.5f, 0, fDeviation);
		fTotalWeight += fWeight * 2.0f;
		vWeights[i] = MathsStorage::Float4((i + 0.5f) * tu, (i + 0.5f) * tv, 0.0f, fWeight);
	}

	// Mirror to the second half
	for(S32 i = N2; i < N; ++i)
	{
		vWeights[i] = vWeights[i-N2+1];
		vWeights[i].x *= -1.0f;
		vWeights[i].y *= -1.0f;
	}

	F32 fWeightModifier = fMultiplier / fTotalWeight;

	for(S32 i = 0; i < N; ++i)
	{
		vWeights[i].w *= fWeightModifier;
	}
}

template<int N>
void GetHorizontalBilinearGaussianSamples(U32 iWidth, U32 iHeight, MathsStorage::Float4* vWeights, F32 fDeviation = 2.0f, F32 fMultiplier = 1.0f)
{
	F32 tu = 0.0f;
	if(iWidth > 0)
		tu = 1.0f / (F32)iWidth;

	F32 tv = 0.0f;
	if(iHeight > 0)
		tv = 1.0f / (F32)iHeight;

	F32 fTotalWeight = 0.0f;
	for(S32 i = 0; i < N; ++i)
	{
		F32 fSample0 = (F32)(i * 2 - N);
		F32 fSample1 = (F32)(i * 2 - N + 1);

		F32 fWeight0 = GaussianDistribution(abs(fSample0), 0, fDeviation);
		F32 fWeight1 = GaussianDistribution(abs(fSample1), 0, fDeviation);
		F32 fSummedWeight = fWeight0 + fWeight1;
		F32 fLerp = fWeight1 / fSummedWeight;
		F32 fX = fSample0 + (fSample1 - fSample0) * fLerp;
		
		fTotalWeight += fSummedWeight;

		vWeights[i] = MathsStorage::Float4((fX - 0.5f) * tu, 0.5f * tv, 0.0f, fSummedWeight);
	}

	F32 fWeightModifier = fMultiplier / fTotalWeight;
	for(S32 i = 0; i < N; ++i)
	{
		vWeights[i].w *= fWeightModifier;
	}
}

template<int N>
void GetVerticalBilinearGaussianSamples(U32 iWidth, U32 iHeight, MathsStorage::Float4* vWeights, F32 fDeviation = 2.0f, F32 fMultiplier = 1.0f)
{
	F32 tu = 0.0f;
	if(iWidth > 0)
		tu = 1.0f / (F32)iWidth;

	F32 tv = 0.0f;
	if(iHeight > 0)
		tv = 1.0f / (F32)iHeight;

	F32 fTotalWeight = 0.0f;
	for(U32 i = 0; i < N; ++i)
	{
		F32 fSample0 = (F32)(i * 2 - N);
		F32 fSample1 = (F32)(i * 2 - N + 1);

		F32 fWeight0 = GaussianDistribution(fSample0, 0, fDeviation);
		F32 fWeight1 = GaussianDistribution(fSample1, 0, fDeviation);
		F32 fSummedWeight = fWeight0 + fWeight1;
		F32 fLerp = fWeight1 / fSummedWeight;
		F32 fY = fSample0 + (fSample1 - fSample0) * fLerp;

		fTotalWeight += fSummedWeight;

		vWeights[i] = MathsStorage::Float4(0.5f * tu, (fY - 1.5f) * tv, 0.0f, fSummedWeight);
	}

	F32 fWeightModifier = fMultiplier / fTotalWeight;
	for(S32 i = 0; i < N; ++i)
	{
		vWeights[i].w *= fWeightModifier;
	}
}

template<int N1, int N2>
void GetGaussianSamples(U32 iWidth, U32 iHeight, MathsStorage::Float4* vWeights, F32 fMultiplier = 1.0f)
{
    FLOAT tu = 1.0f / (F32)iWidth;
    FLOAT tv = 1.0f / (F32)iHeight;

	static const S32 N3 = (N1 - 1) / 2;
	static const S32 N4 = (N2 - 1) / 2;
	
	F32 fWhite = 1.0f;
    
    F32 fTotalWeight = 0.0f;
    S32 nWeights = 0;
    for( S32 x = -N3; x <= N3; x++ )
    {
        for( S32 y = -N4; y <= N4; y++ )
        {
            // Exclude pixels with a block distance greater than 2. This will
            // create a kernel which approximates a 5x5 kernel using only 13
            // sample points instead of 25; this is necessary since 2.0 shaders
            // only support 16 texture grabs.
			if(Maths::Abs((F32)x) + Maths::Abs((F32)y) > (F32)N3 )
                continue;

            // Get the unscaled Gaussian intensity for this offset
            vWeights[nWeights].x = (F32)x * tu;
            vWeights[nWeights].y = (F32)y * tv;
            vWeights[nWeights].z = 0.0f;
            vWeights[nWeights].w = fWhite * GaussianDistribution( (F32)x, (F32)y, 1.0f );
            fTotalWeight += vWeights[nWeights].w;

            ++nWeights;
        }
    }

    // Divide the current weight by the total weight of all the samples; Gaussian
    // blur kernels add to 1.0f to ensure that the intensity of the image isn't
    // changed when the blur occurs. An optional multiplier variable is used to
    // add or remove image intensity during the blur.
    F32 fWeightModifier = fMultiplier / fTotalWeight;
	for(S32 i = 0; i < nWeights; ++i)
    {
        vWeights[i].w *= fWeightModifier;
    }
}

template<int N>
void GetBoxSamples(U32 iWidth, U32 iHeight, MathsStorage::Float4* vWeights, F32 fMultiplier = 1.0f)
{
	static const S32 N2 = (N + 1) / 2;
	BIZ_STATIC_ASSERT(N%2 == 1);
    S32 i = 0;
    F32 tu, tv;
	
	if(iWidth > 0)
		tu = 1.0f / (F32)iWidth;
	else
		tu = 0.0f;

	if(iHeight > 0)
		tv = 1.0f / (F32)iHeight;
	else
		tv = 0.0f;

    // Fill the center texel
	F32 fWeight = fMultiplier / (F32)N;
	vWeights[0] = MathsStorage::Float4(0.0f, 0.0f, 0.0f, fWeight);
    
    // Fill the first half
    for(S32 i = 1; i < N2; ++i)
    {
        // Get the Gaussian intensity for this offset
        vWeights[i] = MathsStorage::Float4(i * tu, i * tv, 0.0f, fWeight);
    }

    // Mirror to the second half
	for(S32 i = N2; i < N; ++i)
    {
        vWeights[i] = vWeights[i-N2+1];
		vWeights[i].x *= -1.0f;
		vWeights[i].y *= -1.0f;
    }
}

template<int N1, int N2>
void GetBoxSamples(U32 iWidth, U32 iHeight, MathsStorage::Float4* vWeights, F32 fMultiplier = 1.0f)
{
	FLOAT tu = 1.0f / (F32)iWidth;
    FLOAT tv = 1.0f / (F32)iHeight;

	static bool N1even = (N1%2 == 0);
	static bool N2even = (N2%2 == 0);

	static const S32 N3 = N1even ? N1 / 2 : (N1 - 1) / 2;
	static const S32 N4 = N2even ? N2 / 2 : (N2 - 1) / 2;
	
	F32 fWhite = 1.0f;
    
    F32 fWeight = fMultiplier / ((F32)N1 * (F32)N2);
    S32 nWeights = 0;
    for( S32 x = -N3; x <= N3; x++ )
    {
		if(!N1even || (N1even && x != 0))
		{	
			for( S32 y = -N4; y <= N4; y++ )
			{
				if(!N2even || (N2even && y != 0))
				{	
					// Get the unscaled Gaussian intensity for this offset
					vWeights[nWeights].x = (F32)x * tu;
					vWeights[nWeights].y = (F32)y * tv;
					vWeights[nWeights].z = 0.0f;
					vWeights[nWeights].w = fWeight;
					++nWeights;
				}
			}
		}
    }
}

template<int N>
void GetHalfGaussianSamples(F32* fWeights, F32 fDeviation = 2.0f, F32 fMultiplier = 1.0f)
{
    // Fill the center texel
	F32 fWeight = GaussianDistribution(0, 0, fDeviation);
	F32 fTotalWeight = fWeight;
	fWeights[0] = fWeight;
    
    // Fill the first half
    for(S32 i = 1; i < N; ++i)
    {
        // Get the Gaussian intensity for this offset
        fWeight = GaussianDistribution((F32)i, 0, fDeviation);
		fTotalWeight += fWeight;
        fWeights[i] = fWeight;
    }

	F32 fWeightModifier = fMultiplier / fTotalWeight;
	
	for(S32 i = 0; i < N; ++i)
	{
		fWeights[i] *= fWeightModifier;
	}
}

template<int N>
void GetHalfBilinearGaussianSamples(F32* fWeights, F32 fDeviation = 2.0f, F32 fMultiplier = 1.0f)
{
	// Fill the first half
	F32 fTotalWeight = 0.0f;
	for(S32 i = 0; i < N; ++i)
	{
		// Get the Gaussian intensity for this offset
		F32 fWeight = GaussianDistribution(i + 0.5f, 0, fDeviation);
		fTotalWeight += fWeight;
		fWeights[i] = fWeight;
	}

	F32 fWeightModifier = fMultiplier / fTotalWeight;

	for(S32 i = 0; i < N; ++i)
	{
		fWeights[i] *= fWeightModifier;
	}
}

// texture functions

INLINE void SetTextureExpanded(const U32 sampler, Display::C_Texture2D* pTexture)
{
	Display::Device()->SetTexture(sampler, pTexture);
}

INLINE void SetTextureUnexpanded(const U32 sampler, Display::C_Texture2D* pTexture)
{
	Display::Device()->SetTexture(sampler, pTexture);
}

INLINE void DownsampleResolve(Display::C_Texture2D* pTexture)
{
}

} // namespace PostProcess

#endif // RENDER_POSTPROCESS_H
