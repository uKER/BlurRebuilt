#ifndef __SH_PROCESSING_H__
#define __SH_PROCESSING_H__

#define SH_MINORDER 2
#define SH_MAXORDER 6

#define PI M_PI

#define SH_SCALE_VALUE 2.9567933082580566f

inline void SHAdd(float *pOut, int iOrder, float *pA, float *pB)
{
	int n = iOrder*iOrder;
	for (int i=0; i<n; i++)
		pOut[i] = pA[i] + pB[i];
}

inline void SHScale(float *pOut, int iOrder, float *pIn, float fScale)
{
	int n = iOrder*iOrder;
	for (int i=0; i<n; i++)
		pOut[i] = pIn[i] * fScale;
}

inline void SHDirection(const float *pf, float *pOut)
{
	float x = pf[0];
	float y = pf[1];
	float z = pf[2];
	static const float a = 0.5f*sqrt(1.0f/(float)M_PI);
	static const float b = 0.5f*sqrt(3.0f/(float)M_PI);
	static const float c = 0.5f*sqrt(15.0f/(float)M_PI);
	static const float d = 0.25f*sqrt(5.0f/(float)M_PI);

	float f = 1.0f/(sqrt(4.f * (float)M_PI));

	pOut[0] = a;
	pOut[1] = -b*y;
	pOut[2] = b*z;
	pOut[3] = -b*x;
	pOut[4] = c*y*x;
	pOut[5] = -c*y*z;
	pOut[6] = d*(3.f*(z*z)-1.f);
	pOut[7] = -c*z*x;
	pOut[8] = 0.5f*c*(x*x-y*y);
}

inline bool SHEvalDirectionalLight(int iOrder, const float *pDir, float fR, float fG, float fB, float *pfROut, float *pfGOut, float *pfBOut)
{
	float coeff[9];

	SHDirection(pDir, coeff);
	int n = iOrder * iOrder;

	for (int i=0; i<n; i++)
	{
		pfROut[i] = coeff[i] * fR * SH_SCALE_VALUE;
		pfGOut[i] = coeff[i] * fG * SH_SCALE_VALUE;
		pfBOut[i] = coeff[i] * fB * SH_SCALE_VALUE;
	}

	return true;
}

#endif
