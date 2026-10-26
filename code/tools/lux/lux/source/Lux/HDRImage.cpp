#include "stdafx.h"

#include "Lux/HdrImage.h"
#include "Lux/Sampler.h"

#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <algorithm>

/*
 * HDR Image class - contains a HDR image used for image-based lighting
 */
// returns the width of this image
int	HDRImage::GetWidth()
{
	return m_iWidth;
}

// returns the height of this image
int	HDRImage::GetHeight()
{
	return m_iHeight;
}

// lookup a texel from the image from a given x and y coordinate
float3 &HDRImage::Lookup(int x, int y)
{
	if (x < 0)			x += m_iWidth;
	if (x >= m_iWidth)	x -= m_iWidth;
	if (y < 0)			y += m_iHeight;
	if (y >= m_iHeight)	y -= m_iHeight;

	int i = x + y*m_iWidth;

	return *(((float3*)m_pCols)+i);
}

// returns a colour value for the texture for a given u and v coordinate
float3 HDRImage::Lookup(float u, float v)
{
	float dx,fx,dy,fy;
	float3 vt[4],f1,f2;
	int x,y;

	// get texel position
	fx = u * m_iWidth;
	x = (int)fx;
	if (x<0) x=0; else if (x>=m_iWidth) x=m_iWidth-1;
	fy = v * m_iHeight;
	y = (int)fy;
	if (y<0) y=0; else if (y>=m_iHeight) y=m_iHeight-1;

	// get fractions
	dx = fx - ceil(fx) + 1.0f;
	dy = fy - ceil(fy) + 1.0f;

	// get edge colours
	vt[0] = Lookup(x  ,y  );	vt[1] = Lookup(x+1,y  );
	vt[2] = Lookup(x  ,y+1);	vt[3] = Lookup(x+1,y+1);

	// calculate filtered output colour
	f1 = vt[0] + (vt[1] - vt[0]) * dx;
	f2 = vt[2] + (vt[3] - vt[2]) * dx;
	return f1 + (f2 - f1) * dy;
}

// apply a gamma curve to the image
void HDRImage::ApplyGamma(float fGamma)
{
	for (int i=0; i<m_iWidth*m_iHeight*3; i++)
	{
		m_pCols[i]=powf(m_pCols[i], fGamma);
	}
}

// apply a scale to the image
void HDRImage::ApplyScale(float3 fScale)
{
	float3 *pv = (float3*)m_pCols;
	for (int i=0; i<m_iWidth*m_iHeight; i++)
	{
		pv[i] *= fScale;
	}
}

HDRImage::HDRImage()
{
	m_pCols = NULL;
}

// delete hdr image
HDRImage::~HDRImage()
{
	if (m_pCols)
		delete m_pCols;
}


/*
 * HDR Loading class. exports a HDRImage class
 */
bool HDRLoader::Load(const char *fileName, HDRImage &res)
{
	int i;
	char str[200];
	FILE *file;

	file = fopen(fileName, "rb");
	if (!file)
		return false;

	fread(str, 10, 1, file);
	if (memcmp(str, "#?RADIANCE", 10)) {
		fclose(file);
		return false;
	}

	fseek(file, 1, SEEK_CUR);

	char cmd[200];
	i = 0;
	char c = 0, oldc;
	while(true) {
		oldc = c;
		c = fgetc(file);
		if (c == 0xa && oldc == 0xa)
			break;
		cmd[i++] = c;
	}

	char reso[200];
	i = 0;
	while(true) {
		c = fgetc(file);
		reso[i++] = c;
		if (c == 0xa)
			break;
	}

	int w, h;
	if (!sscanf(reso, "-Y %ld +X %ld", &h, &w))
	{
		fclose(file);
		return false;
	}

	res.m_iWidth = w;
	res.m_iHeight = h;

	float *cols = new float[w * h * 3];
	res.m_pCols = cols;

	RGBE *scanline = new RGBE[w];
	if (!scanline) {
		fclose(file);
		return false;
	}

	// convert image 
	for (int y = h - 1; y >= 0; y--)
	{
		if (Decrunch(scanline, w, file) == false)
			break;
		WorkOnRGBE(scanline, w, cols);
		cols += w * 3;
	}

	delete [] scanline;
	fclose(file);

	return true;
}

float HDRLoader::ConvertComponent(int expo, int val)
{
	float v = val / 256.0f;
	float d = (float) pow(2.0f, expo);
	return v * d;
}

void HDRLoader::WorkOnRGBE(RGBE *scan, int len, float *cols)
{
	while (len-- > 0) {
		int expo = scan[0][E] - 128;
		cols[0] = ConvertComponent(expo, scan[0][R]);
		cols[1] = ConvertComponent(expo, scan[0][G]);
		cols[2] = ConvertComponent(expo, scan[0][B]);
		cols += 3;
		scan++;
	}
}

bool HDRLoader::Decrunch(RGBE *scanline, int len, FILE *file)
{
	int  i, j;
					
	if (len < MINELEN || len > MAXELEN)
		return OldDecrunch(scanline, len, file);

	i = fgetc(file);
	if (i != 2)
	{
		fseek(file, -1, SEEK_CUR);
		return OldDecrunch(scanline, len, file);
	}

	scanline[0][G] = fgetc(file);
	scanline[0][B] = fgetc(file);
	i = fgetc(file);

	if (scanline[0][G] != 2 || scanline[0][B] & 128)
	{
		scanline[0][R] = 2;
		scanline[0][E] = i;
		return OldDecrunch(scanline + 1, len - 1, file);
	}

	// read each component
	for (i = 0; i < 4; i++)
	{
	    for (j = 0; j < len; )
		{
			unsigned char code = fgetc(file);
			if (code > 128)
			{ // run
			    code &= 127;
			    unsigned char val = fgetc(file);
			    while (code--)
					scanline[j++][i] = val;
			}
			else 
			{	// non-run
			    while(code--)
					scanline[j++][i] = fgetc(file);
			}
		}
    }

	return feof(file) ? false : true;
}

bool HDRLoader::OldDecrunch(RGBE *scanline, int len, FILE *file)
{
	int i;
	int rshift = 0;
	
	while (len > 0) {
		scanline[0][R] = fgetc(file);
		scanline[0][G] = fgetc(file);
		scanline[0][B] = fgetc(file);
		scanline[0][E] = fgetc(file);
		if (feof(file))
			return false;

		if (scanline[0][R] == 1 &&
			scanline[0][G] == 1 &&
			scanline[0][B] == 1) {
			for (i = scanline[0][E] << rshift; i > 0; i--) {
				memcpy(&scanline[0][0], &scanline[-1][0], 4);
				scanline++;
				len--;
			}
			rshift += 8;
		}
		else {
			scanline++;
			len--;
			rshift = 0;
		}
	}
	return true;
}

