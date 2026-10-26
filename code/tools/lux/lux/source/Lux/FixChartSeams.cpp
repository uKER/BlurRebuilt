#include "Stdafx.h"

#include "Lux\TextureTypes\ITexture.h"
#include "Lux\TextureTypes\RGBTexture.h"
#include "Lux\Parameterization\ChartSeams.h"
#include "Lux\FixChartSeams.h"
#include "Lux\Lux.h"
#include "Lux\Core.h"
#include "Lux\ImageFilter.h"

// helper class that iterates texels touching an edge
class EdgeIterator
{
public:
	// setup
	EdgeIterator(const float2 &in_a, const float2 &in_b)
	{
		// find major axis
		float2 f(fabs(in_a.x - in_b.x), fabs(in_a.y - in_b.y));
		if (f.x > f.y)
			iMajorAxis = 0;
		else
			iMajorAxis = 1;
		iMinorAxis = iMajorAxis ^ 1;

		// check direction in major axis
		a = in_a;
		b = in_b;
		bPositive = (a[iMajorAxis] < b[iMajorAxis]);
		pos = a;

		delta.x = b.x - a.x;
		delta.y = b.y - a.y;
		dir = delta;
		m = delta[iMinorAxis] / delta[iMajorAxis];

		org = a;
		fMinorFrac = org[iMinorAxis] - floor(org[iMinorAxis]);

		bDeferMinorUpdate = false;

		// TODO: center start position
	}

	// check whether we've finished
	bool End() const
	{
		if (bPositive)
			return pos[iMajorAxis] > b[iMajorAxis];
		else
			return pos[iMajorAxis] < b[iMajorAxis];
	}

	float2 Pos() const
	{
		return pos;
	}

	float2 Dir() const
	{
		return dir;
	}

	float2 Org() const
	{
		return org;
	}

	float Frac() const
	{
		return (pos[iMajorAxis] - a[iMajorAxis]) / (b[iMajorAxis] - a[iMajorAxis]);
	}

	float Delta() const
	{
		return delta[iMajorAxis];
	}

	// move to the next texel
	void operator++()
	{
		if (bPositive)
		{
			pos[iMinorAxis]+=m;
			pos[iMajorAxis]++;
		}
		else
		{
			pos[iMinorAxis]-=m;
			pos[iMajorAxis]--;
		}

	}

private:
	float2 a,b, pos, delta, dir, org;
	float fMinorFrac;
	int iMajorAxis;
	int iMinorAxis;
	bool bPositive;
	bool bDeferMinorUpdate;
	float m;
};

inline Float2 operator *(const Float2 &a, const float b)
{
	Float2 rtn;
	rtn.x = a.x * b;
	rtn.y = a.y * b;
	return rtn;
}

// merge texels
void MergeTexels(	ITexture *pTex1, const EdgeIterator &iter1, 
					ITexture *pTex2, const EdgeIterator &iter2,
					int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
	float2 vTex1Dir = iter1.Dir();
	vTex1Dir.Normalize();
	float2 vTex1Nrm(-vTex1Dir.y, vTex1Dir.x);
	float2 vTex2Dir = iter2.Dir();
	vTex2Dir.Normalize();
	float2 vTex2Nrm(-vTex2Dir.y, vTex2Dir.x);

	for (float y=-0.5f; y<=1.f; y+=1.f)
	{
		for (float x=-0.5f; x<=1.f; x+=1.f)
		{
			float2 vOffset = iter1.Pos() + float2(x,y);
			vOffset.x = floor(vOffset.x) + 0.5f;
			vOffset.y = floor(vOffset.y) + 0.5f;
			vOffset = vOffset - iter1.Org();

			// convert offset from edge1 local space to edge2 local space
			float2 vOffsetWorld( DotProduct(vOffset, vTex1Dir), DotProduct(vOffset, vTex1Nrm));
			float2 vOffsetTex2 = vTex2Dir * vOffsetWorld.x + vTex2Nrm * vOffsetWorld.y;

			pTex1->MergeTexels(iter1.Org() + vOffset, pTex2, iter2.Org() + vOffsetTex2,
				x1, y1, w1, h1, x2, y2, w2, h2);
		}
	}

}

// blend together the pixels of two charts along their seam
void FixChartSeam(	ITexture *pTex1, const float2 &uv1a, const float2 &uv1b, 
					ITexture *pTex2, const float2 &uv2a, const float2 &uv2b,
					int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
	EdgeIterator iter1(uv1a,uv1b);
	EdgeIterator iter2(uv2a,uv2b);

	if (x1<0) x1=0;
	if (x1+w1>=pTex1->m_cX) w1=pTex1->m_cX-x1;
	if (y1<0) y1=0;
	if (y1+h1>=pTex1->m_cY) h1=pTex1->m_cY-y1;

	if (x2<0) x2=0;
	if (x2+w2>=pTex2->m_cX) w2=pTex2->m_cX-x2;
	if (y2<0) y2=0;
	if (y2+h2>=pTex2->m_cY) h2=pTex2->m_cY-y2;

	while(1)
	{
		MergeTexels(pTex1, iter1, pTex2, iter2, x1, y1, w1, h1, x2, y2, w2, h2);
		++iter1;
		if (iter1.End())
			break;
	}

	while(1)
	{
		MergeTexels(pTex2, iter2, pTex1, iter1, x1, y1, w1, h1, x2, y2, w2, h2); 
		++iter2;
		if (iter2.End())
			break;
	}
}
