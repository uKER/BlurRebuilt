#ifndef __FIX_CHART_SEAMS_H__
#define __FIX_CHART_SEAMS_H__

void FixChartSeam(	ITexture *pTex1, const float2 &uv1a, const float2 &uv1b, 
					ITexture *pTex2, const float2 &uv2a, const float2 &uv2b,
					int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

#endif
