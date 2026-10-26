#pragma once

class ITexture;
class RGBTexture;
struct FilterResponse;
#include "Lux\SurfaceIterator.h"

typedef Ut::Vector<RGBTexture*> TRGBTextureList;

#define TEXEL_UNUSED_COLOUR_VALUE					GetUnusedColourValue()			//0x00ffdead
#define TEXEL_UNUSED_COLOUR_VALUE_PLUS_ONE			GetUnusedColourValuePlusOne()	//0x00fedead

// container for output RGB data
// the lightmap data is converted from the original format to this format before being packed and saved out
class RGBTexture : public BaseObject
{
	DECLARE_PERSISTENCE(RGBTexture);

	void write( ost::Engine& ar );
	void read( ost::Engine& ar );

public:
				RGBTexture() : m_bInitialized(false), m_bFoundQNAN(false), m_pRGBA32Data(NULL) {};
				RGBTexture(ITexture *pOrg, float fGamma, float fBrightness);
				RGBTexture(RGBTexture *pOrg);
				~RGBTexture() { Free(); };

	int			m_cX, m_cY;
	int			m_iAssociatedChart;

	// 2 channels of pixel touching / usage checking
	bool		CheckPixelUsed(int x, int y);
	bool		CheckImageBlockUsed(int x, int y, int w, int h);

	// get number of texture layers
	int			GetNumLayers() { return m_nLayers; }

	// delete our RGBA data
	void		FlushRGBAData();

	// clear the contents of the texture
	void		Clear();

	// retrieve raw RGBA bits
	U32			*GetRGBAData(int iChannel);

	// optimize chart by scaling it down if it's flat-coloured
	bool		Optimize();

	// retrieve custom data vector
	Ut::Vector<U8> &GetCustomData();

	// returns true if a QNAN was found in the output data
	bool		FoundQNAN();

private:
	U32		   **m_pRGBA32Data;

	void		Free();

	void		FillForDXT(U32 *pData);
	void		CreateRGBATexture(ITexture *pOriginal, float fGamma, float fBrightness);

	bool		m_bInitialized;
	int			m_nLayers;
	bool		m_bFoundQNAN;

	Ut::Vector<U8>	m_CustomData;
};

