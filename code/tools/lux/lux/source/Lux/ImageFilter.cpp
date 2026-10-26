#include "stdafx.h"

#include "Lux/ImageFilter.h"
#include "Lux/Sampler.h"

// precompute filter values and store in a table
void ImageFilter::InitializeTable()
{
	float *ftp = pfFilterTable;
	for (int y = 0; y < FILTER_TABLE_SIZE; ++y)
	{
		float fy = ((float)y + .5f) * yWidth / FILTER_TABLE_SIZE;
		for (int x = 0; x < FILTER_TABLE_SIZE; ++x)
		{
			float fx = ((float)x + .5f) * xWidth / FILTER_TABLE_SIZE;
			*ftp++ = Evaluate(fx, fy);
		}
	}
}

// precalculate filter response for each of the possible image sample points from the sampler
void ImageFilter::PrecalcFilterResponse(Sampler *pSampler)
{
	m_pSampler = pSampler;

	// interate sample positions
	for (int iSample=0; iSample<pSampler->m_nSamplesPerTexel; iSample++)
	{
		FilterResponse r;
		float2 vSubPos = pSampler->GetImageSample(iSample) - float2(0.7f,0.7f);
		float *pfOutRow = r.fResponse;

		float dImageX = vSubPos.x - 0.5f;
		float dImageY = vSubPos.y - 0.5f;
		int dx = Ceil2Int (dImageX - xWidth);
		int ex = Floor2Int(dImageX + xWidth) + 1;
		int dy = Ceil2Int (dImageY - yWidth);
		int ey = Floor2Int(dImageY + yWidth) + 1;
		r.dx = dx;
		r.dy = dy;
		r.cx = ex - dx;
		r.cy = ey - dy;

		assert(r.cx<=4);
		assert(r.cy<=4);

		// calculate filter response for a 4x4 grid surrounding the main sample position
		for (int y=dy; y<ey; y++, pfOutRow+=4)
		{
			float *pfOut = pfOutRow;
			for (int x=dx; x<ex; x++, pfOut++)
				*pfOut = Evaluate((float)x - dImageX, (float)y - dImageY);
		}

		m_response.PushBack(r);
	}
}

// retrieve precalculated filter response
FilterResponse *ImageFilter::GetResponse(int i)
{
	return &m_response[i];
}


// Box Filter Method Definitions
float BoxFilter::Evaluate(float x, float y) const
{
	return 1.;
}

// Gaussian Filter Method Definitions
float GaussianFilter::Evaluate(float x, float y) const
{
	return Gaussian(x, expX) * Gaussian(y, expY);
}
