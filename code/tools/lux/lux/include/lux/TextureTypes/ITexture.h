#ifndef __ITEXTURE_H__
#define __ITEXTURE_H__

struct FilterResponse;
#include "Lux\SurfaceIterator.h"

class ImageFilter;

namespace LUX
{
	class RenderChannel;
}

struct STouched
{
	struct
	{
		unsigned int bTouched1 : 1;
		unsigned int bTouched2 : 1;
	};
};

class ITexture
{
public:
				ITexture() : m_pTouchedBuf(NULL) {};
				ITexture(int x, int y, ImageFilter *pFilter) : m_pFilter(pFilter) { Create(x,y); }
	virtual		~ITexture() { Free(); };

	int			m_cX, m_cY;
	int			m_iAssociatedChart;

	// 2 channels of pixel touching / usage checking
	void		TouchPixel(int x, int y);
	bool		CheckPixelUsed(int x, int y);
	void		TouchPixel2(int x, int y);
	bool		CheckPixelUsed2(int x, int y);

	// generate output format data
	virtual U32		   *GenerateRGBAData(int iLayer, float fGamma, float fBrightness, bool *pbFoundQNAN) = 0;
	// retrieve custom data
	virtual void		GetCustomData(Ut::Vector<U8> &data) = 0;

	// add a sample to the normal buffer
	virtual void		AddSample(float2 t, const LightSample &sample, const HMat33 &mtxTangent) = 0;
	// add a sample to the dicontinuity buffer
	virtual void		AddDiscontinuitySample(int iX, int iY, const LightSample &samples, const HMat33 &mtxTangent) = 0;
	// add multiple samples to the normal buffer
	virtual void		AddSample(float2 t, const Ut::Vector<LightSample> &samples, const HMat33 &mtxTangent) = 0;
	// add multiple samples to the discontinuity buffer
	virtual void		AddDiscontinuitySample(int iX, int iY, const Ut::Vector<LightSample> &samples, const HMat33 &mtxTangent) = 0;
	// convolve all added samples into the intermediate buffer
	virtual void		Resolve() = 0;
	// set a subwindow within the texture. rendering will be clamped to this window
	virtual void		SetWindow(int x, int y, int cx, int cy);
	// merge the values of two texels. this function is used by the anti chart seam code
	virtual void		MergeTexels(float2 t1, ITexture *pOtherTexture, float2 t2,
									int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) = 0;
	// return the number of texture layers (depends on T)
	virtual int			GetNumLayers() = 0;
	// resize dimensions of the texture
	virtual void		Resize(int x, int y);
	// clear the contents of the texture
	virtual void		Clear();
	// returns true if this texture needs tangent space to render
	virtual bool		NeedTangentSpace();

protected:
	STouched    *m_pTouchedBuf;
	bool	 	 m_bInitialized;
	int			 m_iPass;
	ImageFilter *m_pFilter;
	struct
	{
		int			x, y, cx, cy;
	} m_window;

	virtual void		Create(int x, int y);
	virtual void		Free();
	bool				CheckPixelUsed(int i);

	// utility function - precalculate sample offset table
	void PrecalcSampleOffsets(float2 t, ImageFilter *pFilter,
							  int &x0, int &y0, int &x1, int &y1, int *ifx, int *ify);
};

#endif
