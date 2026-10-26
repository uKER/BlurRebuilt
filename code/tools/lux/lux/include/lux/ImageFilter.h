#pragma once

#define FILTER_TABLE_SIZE 16
class Sampler;

struct FilterResponse
{
	int dx,dy;
	int cx,cy;
	float fResponse[5*5];
};

typedef Ut::UtVector< Ut::DynamicVector<FilterResponse> > FilterResponseVector;

class ImageFilter
{
public:
	// Filter Interface
	virtual ~		ImageFilter()	{ }
					ImageFilter(float xw, float yw) : xWidth(xw), yWidth(yw), invXWidth(1.f/xw), invYWidth(1.f/yw) { }
	virtual float	Evaluate(float x, float y) const = 0;

	// precalculating functions
	void			InitializeTable();
	float			*GetTable()		{ return pfFilterTable; }
	void			PrecalcFilterResponse(Sampler *pSampler);
	FilterResponse *GetResponse(int i);

	// precalculated filter table
	float		pfFilterTable[FILTER_TABLE_SIZE * FILTER_TABLE_SIZE];
	// precalculated filter response table
	FilterResponseVector m_response;

	Sampler *m_pSampler; // for debugging only

	// Filter Public Data
	const float xWidth, yWidth;
	const float invXWidth, invYWidth;
};

typedef Ut::UtVector< Ut::DynamicVector<ImageFilter*> > FilterVector;

// Box Filter Declarations
class BoxFilter : public ImageFilter
{
public:
	BoxFilter(float xw, float yw) : ImageFilter(xw, yw) { }
	float Evaluate(float x, float y) const;
};

// Gaussian Filter Declarations
class GaussianFilter : public ImageFilter
{
public:
	// GaussianFilter Public Methods
	GaussianFilter(float xw, float yw, float a)
		: ImageFilter(xw, yw) {
		alpha = a;
		expX = expf(-alpha * xWidth * xWidth);
		expY = expf(-alpha * yWidth * yWidth);
	}
	float Evaluate(float x, float y) const;
private:
	// GaussianFilter Private Data
	float alpha;
	float expX, expY;
	// GaussianFilter Utility Functions
	float Gaussian(float d, float expv) const {
		return max(0.f, float(expf(-alpha * d * d) - expv));
	}
};

