#ifndef _HDRIMAGE_H_
#define _HDRIMAGE_H_

#define  MINELEN	8				// minimum scanline length for encoding
#define  MAXELEN	0x7fff			// maximum scanline length for encoding

typedef unsigned char RGBE[4];
typedef enum
{
	R = 0,
	G = 1,
	B = 2,
	E = 3
} HDRChannel;

class HDRImage
{
	friend class HDRLoader;

public:
			HDRImage();
			~HDRImage();
	// apply gamma correction to the image.
	// f = gamma amount (e.g. 2.2)
	void	ApplyGamma(float f);

	// apply a scale factor to the whole image
	void	ApplyScale(float3 fScale);

	// returns a colour value for the texture for a given u and v coordinate
	float3	Lookup(float s, float t);

	// lookup a texel from the image from a given x and y coordinate
	float3	&Lookup(int x, int y);

	// returns the width of this image
	int		GetWidth();

	// returns the height of this image
	int		GetHeight();

	// serialize/deserialize
	friend ost::Engine& operator << (ost::Engine& ar, HDRImage &t)
	{
		ar << t.m_iWidth << t.m_iHeight;
		ar.writeBinary(t.m_pCols, t.m_iWidth * t.m_iHeight * 3*sizeof(float));
		return ar;
	}
	friend ost::Engine& operator >> (ost::Engine& ar, HDRImage &t)
	{
		ar >> t.m_iWidth >> t.m_iHeight;
		t.m_pCols = new float[3*t.m_iWidth * t.m_iHeight];
		ar.readBinary(t.m_pCols, t.m_iWidth * t.m_iHeight * 3*sizeof(float));
		return ar;
	}

private:
	int m_iWidth, m_iHeight;
	// each pixel takes 3 float32, each component can be of any value...
	float *m_pCols;
};

// load a HDR image and store it in HDRLoaderResult class
class HDRLoader
{
public:
	static bool Load(const char *fileName, HDRImage &res);

private:
	static void WorkOnRGBE(RGBE *scan, int len, float *cols);
	static bool Decrunch(RGBE *scanline, int len, FILE *file);
	static bool OldDecrunch(RGBE *scanline, int len, FILE *file);
	static float ConvertComponent(int expo, int val);
};


#endif
