#include "stdafx.h"

#include <memory.h>
#include <math.h>
#include <string.h>

//#include "texture_export/algebra.h"
#include "ngl/system.h"
#include "texture_export/Rescale.h"
#include "texture_export/Texture.h"
#include "texture_export/Surface.h"

#include <core/maths/mathsutil.h>

#if defined( PI )
#undef PI
#endif 

#define PI  3.14159265358979323846264338327950288419716939937510

#define tlt_max  Maths::Max
#define tlt_min  Maths::Min
#define TLT_ASSERT assert

#if defined( Epsilon )
#undef Epsilon
#endif 
#define Epsilon  1.0e-12

using namespace ActTools;

static char theFilterNames[][16] = {
	CSTR_UNDEFINED,
	CSTR_POINT,
	CSTR_BOX,
	CSTR_TRIANGLE,
	CSTR_HERMITE,
	CSTR_HANNING,
	CSTR_HAMMING,
	CSTR_BLACKMAN,
	CSTR_GAUSSIAN,
	CSTR_QUADRATIC,
	CSTR_CUBIC,
	CSTR_CATROM,
	CSTR_MITCHELL,
	CSTR_LANCZOS,
	CSTR_BESSEL,
	CSTR_SINC,
};

inline bool IsValidFilterType( int aFilter )
{
	return ( FT_FIRST <= aFilter && aFilter <= FT_LAST );
}

enum FilterType ActTools::GetFilterType( const char* aFilterName )
{
	size_t i;
	for ( i = elementsof(theFilterNames); --i && _stricmp( aFilterName, theFilterNames[i]); )
		{}
	return static_cast<FilterType>(i);
}

const char* ActTools::GetFilterName( int FilterType )
{
	return IsValidFilterType( FilterType ) ? theFilterNames[FilterType] : "INVALID";
}
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   B e s s e l O r d e r O n e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method BesselOrderOne computes the Bessel function of x of the first kind
%  of order 0:
%
%    Reduce x to |x| since j1(x)= -j1(-x), and for x in (0,8]
%
%       j1(x) = x*j1(x);
%
%    For x in (8,inf)
%
%       j1(x) = sqrt(2/(pi*x))*(p1(x)*cos(x1)-q1(x)*sin(x1))
%
%    where x1 = x-3*pi/4. Compute sin(x1) and cos(x1) as follow:
%
%       cos(x1) =  cos(x)cos(3pi/4)+sin(x)sin(3pi/4)
%               =  1/sqrt(2) * (sin(x) - cos(x))
%       sin(x1) =  sin(x)cos(3pi/4)-cos(x)sin(3pi/4)
%               = -1/sqrt(2) * (sin(x) + cos(x))
%
%  The format of the BesselOrderOne method is:
%
%      double BesselOrderOne(double x)
%
%  A description of each parameter follows:
%
%    o value: Method BesselOrderOne returns the Bessel function of x of the
%      first kind of orders 1.
%
%    o x: double value.
%
%
*/

static double J1( const double x )
{
	double p, q;

	static const double Pone[] =
	{
		 0.581199354001606143928050809e+21,
		-0.6672106568924916298020941484e+20,
		 0.2316433580634002297931815435e+19,
		-0.3588817569910106050743641413e+17,
		 0.2908795263834775409737601689e+15,
		-0.1322983480332126453125473247e+13,
		 0.3413234182301700539091292655e+10,
		-0.4695753530642995859767162166e+7,
		 0.270112271089232341485679099e+4
	};
	static const double Qone[] =
	{
		0.11623987080032122878585294e+22,
		0.1185770712190320999837113348e+20,
		0.6092061398917521746105196863e+17,
		0.2081661221307607351240184229e+15,
		0.5243710262167649715406728642e+12,
		0.1013863514358673989967045588e+10,
		0.1501793594998585505921097578e+7,
		0.1606931573481487801970916749e+4,
		0.1e+1
	};

	p = Pone[8];
	q = Qone[8];
	for ( register int i = 7; 0 <= i; i-- )
	{
		p = p * x * x + Pone[i];
		q = q * x * x + Qone[i];
	}
	return ( p / q );
}

static double P1( const double x )
{
  double p, q;

	static const double Pone[] =
	{
		0.352246649133679798341724373e+5,
		0.62758845247161281269005675e+5,
		0.313539631109159574238669888e+5,
		0.49854832060594338434500455e+4,
		0.2111529182853962382105718e+3,
		0.12571716929145341558495e+1
	};
	static const double Qone[] =
	{
		0.352246649133679798068390431e+5,
		0.626943469593560511888833731e+5,
		0.312404063819041039923015703e+5,
		0.4930396490181088979386097e+4,
		0.2030775189134759322293574e+3,
		0.1e+1
	};

	p = Pone[5];
	q = Qone[5];
	for ( register int i = 4; 0 <= i; i-- )
	{
		p = p * ( 8.0 / x ) * ( 8.0 / x ) + Pone[i];
		q = q * ( 8.0 / x ) * ( 8.0 / x ) + Qone[i];
	}
	return ( p / q );
}

static double Q1( const double x )
{
	double p, q;

	static const double Pone[] =
	{
		0.3511751914303552822533318e+3,
		0.7210391804904475039280863e+3,
		0.4259873011654442389886993e+3,
		0.831898957673850827325226e+2,
		0.45681716295512267064405e+1,
		0.3532840052740123642735e-1
    };
	static const double Qone[] =
	{
		0.74917374171809127714519505e+4,
		0.154141773392650970499848051e+5,
		0.91522317015169922705904727e+4,
		0.18111867005523513506724158e+4,
		0.1038187585462133728776636e+3,
		0.1e+1
	};

	p = Pone[5];
	q = Qone[5];
	for ( register int i = 4; 0 <= i; i-- )
	{
		p = p * ( 8.0 / x ) * ( 8.0 / x ) + Pone[i];
		q = q * ( 8.0 / x ) * ( 8.0 / x ) + Qone[i];
	}
	return ( p / q );
}

static double BesselOrderOne( double x )
{
  double p, q;

	if ( 0.0 == x )
		return ( 0.0 );

	p = x;

	if ( x < 0.0 )
		x = (-x);
	if ( x < 8.0 )
		return ( p * J1( x ) );

	q = sqrt( 2.0 / ( PI * x ) ) * 
	    ( P1( x ) * ( 1.0 / sqrt( 2.0 ) * ( sin( x ) - cos( x ) ) ) - 8.0 / x * Q1( x ) * ( -1.0 / sqrt( 2.0 ) * ( sin( x ) + cos( x ) ) ) );

	if ( p < 0.0 )
		q = ( -q );

	return(q);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s i z e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResizeImage() scales an image to the desired dimensions with one of these
%  filters:
%
%    Bessel   Blackman   Box
%    Catrom   Cubic      Gaussian
%    Hanning  Hermite    Lanczos
%    Mitchell Point      Quandratic
%    Sinc     Triangle
%
%  If a NULL image is returned, check exception to determine why the image
%  failed to scale.
%
%  The format of the ResizeImage method is:
%
%      Image *ResizeImage(Image *image,const unsigned long columns,
%        const unsigned long rows,const FilterType filter,const double blur,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
%    o columns: The number of columns in the scaled image.
%
%    o rows: The number of rows in the scaled image.
%
%    o filter: Image filter to use.
%
%    o blur: The blur factor where > 1 is blurry, < 1 is sharp.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
typedef struct tagContributionInfo
{
	double weight;
	long pixel;
} ContributionInfo;

typedef double (*FilterFunctionPtr)(double);
typedef struct tagFilterInfo
{
	double (*function)(double);
	double support;
} FilterInfo;


static double Box( double x )
{
	if ( ( -0.5 <= x ) && ( x < 0.5 ) )
		return ( 1.0 );
	return ( 0.0 );
}

static double Bessel( double x )
{
	if ( 0.0 == x )
		return( PI / 4.0 );
	return ( BesselOrderOne( PI * x ) / ( 2.0 * x ) );
}

static double Blackman( double x )
{
	return ( 0.42 + 0.50 * cos( PI * x ) + 0.08 * cos( 2.0 * PI * x ) );
}

static double Catrom( double x )
{
	if ( x < 0.0 )
		x = ( -x );
	if ( x < 1.0 )
		return ( 0.5 * ( 2.0 + x * x * ( -5.0 + x * 3.0 ) ) );
	if ( x < 2.0 )
		return ( 0.5 * ( 4.0 + x * ( -8.0 + x * ( 5.0 - x ) ) ) );
	return ( 0.0 );
}

static double Cubic( double x )
{
	if ( x < 0.0 )
		x = ( -x );
	if ( x < 1.0 )
		return ( ( 0.5 * x * x * x ) - x * x + ( 2.0 / 3.0 ) );
	if ( x < 2.0 )
	{
		x = 2.0 - x;
		return ( ( 1.0 / 6.0 ) * x * x * x );
	}
	return ( 0.0 );
}

static double Gaussian( double x )
{
	return ( exp( -2.0 * x * x ) * sqrt( 2.0 / PI ) );
}

static double Hanning( double x )
{
	return ( 0.5 + 0.5 * cos( PI * x ) );
}

static double Hamming( double x )
{
	return ( 0.54 + 0.46 * cos( PI * x ) );
}

static double Hermite( double x )
{
	if ( x < 0 )
		x = ( -x );
	if ( x < 1.0 )
		return ( ( 2.0 * x - 3.0 ) * x * x + 1.0 );
	return ( 0.0 );
}

static double Sinc( double x )
{
	x *= PI;
	if ( 0.0 != x )
		return( sin( x ) / x );
	return ( 1.0 );
}

static double Lanczos( double x )
{
	if ( x < 0 )
		x = ( -x );
	if ( x < 3.0 )
		return ( Sinc( x ) * Sinc( x / 3.0 ) );
	return ( 0.0 );
}

static double Mitchell( double x )
{
  double b, c;

	b = 1.0 / 3.0;
	c = 1.0 / 3.0;
	if ( x < 0.0 )
		x = (-x);
	if ( x < 1.0 )
	{
		x = ( ( 12.0 - 9.0 * b - 6.0 * c ) * ( x * x * x ) ) + ( ( -18.0 + 12.0 * b + 6.0 * c ) * x * x ) + ( 6.0 - 2.0 * b );
		return ( x / 6.0 );
	}
	if ( x < 2.0 )
	{
		x = ( ( -1.0 * b - 6.0 * c ) * ( x * x * x ) ) + 
		            ( ( 6.0 * b + 30.0 * c ) * x * x ) + 
		              ( ( -12.0 * b - 48.0 * c ) * x ) +
		                        ( 8.0 * b + 24.0 * c );
		return ( x / 6.0 );
	}
	return ( 0.0 );
}

static double Quadratic( double x )
{
	if ( x < 0 )
		x = ( -x );
	if ( x < 0.5 )
		return( 0.75 - x * x );
	if ( x < 1.5 )
	{
		x -= 1.5;
		return ( 0.5 * x * x );
	}
	return ( 0.0 );
}

static double Triangle( double x )
{
	if ( x < 0.0 )
		x =( -x );
	if ( x < 1.0 )
		return( 1.0 - x );
	return(0.0);
}
/*
static unsigned int HorizontalFilter(const Image *source,Image *destination,
  double x_factor,const FilterInfo *filter_info,const double blur,
  ContributionInfo *contribution,const size_t span,unsigned int *quantum,
  ExceptionInfo *exception)*/
static int HorizontalFilter( u8* pDstImage, int iDstWidth, int iDstHeight,
                             const u8* pSrcImage, int iSrcWidth, int iSrcHeight,
                             double x_factor, 
                             const FilterInfo* pColorFilterInfo, const double dColorBlur, ContributionInfo* pColorContribution,
                             const FilterInfo* pAlphaFilterInfo, const double dAlphaBlur, ContributionInfo* pAlphaContribution,
                             const EdgeFilter aFilterEdge,
                             const size_t ACTT_UNUSED(span) )
{
//	if ( ( iSrcWidth  == iDstWidth ) && ( iSrcHeight == iDstHeight ) )
//	{
//		// Equal width and height - just copy pixels.
//		PixelFormat::CopyPixels( *pSrcPF, pSrcImage, *pDstPF, pDstImage, iSrcWidth * iSrcHeight );
//		memcpy( pDstImage, pSrcImage, iSrcWidth * iSrcHeight * 4 );
//		return 0;
//	}

	// Apply filter to resize horizontally from source to destination.
	double ColorScale = dColorBlur * tlt_max( 1.0 / x_factor, 1.0 );
	double ColorSupport = tlt_max( ColorScale * pColorFilterInfo->support, 0.5 );
	if ( 0.5 < ColorSupport )
/*		SetImageType( destination, TrueColorType )*/;
	else
	{
		// Reduce to point sampling.
		ColorSupport = 0.5 + Epsilon;
		ColorScale = 1.0;
	}
	double AlphaScale = dAlphaBlur * tlt_max( 1.0 / x_factor, 1.0 );
	double AlphaSupport = tlt_max( AlphaScale * pAlphaFilterInfo->support, 0.5 );
	if ( 0.5 < AlphaSupport )
/*		SetImageType( destination, TrueColorType )*/;
	else
	{
		// Reduce to point sampling.
		AlphaSupport = 0.5 + Epsilon;
		AlphaScale = 1.0;
	}

	EdgeFilter FilterEdge(aFilterEdge);
	if ( iSrcWidth < ColorSupport )
		FilterEdge = EDGE_CLAMP; // tiling/miroring will not work for image size that is smaller than filter size
	int x;
	for ( x = 0; x < iDstWidth; x++ )
	{
		double center;
		int i, na, nc, ColorStart, ColorEnd, AlphaStart, AlphaEnd;

		switch ( FilterEdge )
		{
			case EDGE_CLAMP :
				center = (double)x / x_factor + ( 1.0 / ( 2.0 * x_factor ) );
				ColorStart = (int)tlt_max( center - ColorSupport + 0.5, 0. );
				ColorEnd   = (int)tlt_min( center + ColorSupport + 0.5, (double)iSrcWidth );
				AlphaStart = (int)tlt_max( center - AlphaSupport + 0.5, 0. );
				AlphaEnd   = (int)tlt_min( center + AlphaSupport + 0.5, (double)iSrcWidth );
				break;

			case EDGE_MIRROR :
//				break;

			case EDGE_TILE :
				if ( x == iDstWidth - 1 )
					center = (double)iDstWidth / x_factor - ( 1.0 / ( 2.0 * x_factor ) );
				else
					center = (double)x / x_factor + ( 1.0 / ( 2.0 * x_factor ) );
				ColorStart = (int)( center - ColorSupport + 0.5 );
				ColorEnd   = (int)( center + ColorSupport + 0.5 );
				AlphaStart = (int)( center - AlphaSupport + 0.5 );
				AlphaEnd   = (int)( center + AlphaSupport + 0.5 );
				break;

			default :
				assert( 0 );
				return -1;
		}
		double ColorDensity = 0.0;
		for ( nc = 0, i = ColorStart; i < ColorEnd; i++, nc++ )
		{
			pColorContribution[nc].pixel = i;
			pColorContribution[nc].weight = pColorFilterInfo->function( ( i - center + 0.5 ) / ColorScale );
			pColorContribution[nc].weight /= ColorScale;
			ColorDensity += pColorContribution[nc].weight;
		}
		ColorDensity = 0.0 == ColorDensity ? 1.0 : 1.0 / ColorDensity;
		// normalize
		if ( 1 == nc )
			pColorContribution[0].weight = 1;
		else
			for ( i = 0; i < nc; i++ )
				pColorContribution[i].weight *= ColorDensity;

		double AlphaDensity = 0.0;
		for ( na = 0, i = AlphaStart; i < AlphaEnd; i++, na++ )
		{
			pAlphaContribution[na].pixel = i;
			pAlphaContribution[na].weight = pAlphaFilterInfo->function( ( i - center + 0.5 ) / AlphaScale );
			pAlphaContribution[na].weight /= AlphaScale;
			AlphaDensity += pAlphaContribution[na].weight;
		}
		AlphaDensity = 0.0 == AlphaDensity ? 1.0 : 1.0 / AlphaDensity;
		// normalize
		if ( 1 == na )
			pAlphaContribution[0].weight = 1.0f;
		else
			for ( i = 0; i < na; i++ )
				pAlphaContribution[i].weight *= AlphaDensity;

//		p = AcquireImagePixels( source, contribution[0].pixel, 0,
//		                        contribution[n - 1].pixel - contribution[0].pixel + 1, source->rows, exception );
//		q = SetImagePixels( destination, x, 0, 1, destination->rows );
		u8* pDstPixel = pDstImage + x * 4;

		for ( int y = 0; y < iDstHeight; y++ )
		{
			double red = 0.0, green = 0.0, blue = 0.0, alpha = 0.0;
			switch ( FilterEdge )
			{
			case EDGE_CLAMP : 
				for ( i = 0; i < nc; i++ )
				{
					const u8* Texel = pSrcImage + ( y * iSrcWidth + pColorContribution[i].pixel ) * 4;
					red   += pColorContribution[i].weight * Texel[2];
					green += pColorContribution[i].weight * Texel[1];
					blue  += pColorContribution[i].weight * Texel[0];
				}
				for ( i = 0; i < na; i++ )
				{
					const u8* Texel = pSrcImage + ( y * iSrcWidth + pAlphaContribution[i].pixel ) * 4;
					alpha += pAlphaContribution[i].weight * Texel[3];
				}
				break;

			case EDGE_MIRROR :
				for ( i = 0; i < nc; i++ )
				{
					int x;
					if ( pColorContribution[i].pixel < 0 )
						x = abs( pColorContribution[i].pixel );
					else
					if ( iSrcWidth <= pColorContribution[i].pixel )
						x = ( iSrcWidth << 1 ) - pColorContribution[i].pixel - 1;
					else
						x = pColorContribution[i].pixel;
					const u8* Texel = pSrcImage + ( y * iSrcWidth + x ) * 4;
					red   += pColorContribution[i].weight * Texel[2];
					green += pColorContribution[i].weight * Texel[1];
					blue  += pColorContribution[i].weight * Texel[0];
				}
				for ( i = 0; i < na; i++ )
				{
					int x;
					if ( pAlphaContribution[i].pixel < 0 )
						x = abs( pAlphaContribution[i].pixel );
					else
					if ( iSrcWidth <= pAlphaContribution[i].pixel )
						x = ( iSrcWidth << 1 ) - pAlphaContribution[i].pixel - 1; 
					else
						x = pAlphaContribution[i].pixel;
					const u8* Texel = pSrcImage + ( y * iSrcWidth + x ) * 4;
					alpha += pAlphaContribution[i].weight * Texel[3];
				}
				break;

			case EDGE_TILE :
				for ( i = 0; i < nc; i++ )
				{
					const u8* Texel = pSrcImage + ( y * iSrcWidth + ( iSrcWidth + ( pColorContribution[i].pixel ) ) % iSrcWidth ) * 4;
					red   += pColorContribution[i].weight * Texel[2];
					green += pColorContribution[i].weight * Texel[1];
					blue  += pColorContribution[i].weight * Texel[0];
				}
				for ( i = 0; i < na; i++ )
				{
					const u8* Texel = pSrcImage + ( y * iSrcWidth + ( iSrcWidth + ( pAlphaContribution[i].pixel ) ) % iSrcWidth ) * 4;
					alpha += pAlphaContribution[i].weight * Texel[3];
				}
				break;

			default :
				assert( 0 );
			}
			pDstPixel[0] = (u8) ( (  blue < 0.0 ) ? 0 : ( 255.0 < blue  ) ? 255.0 : blue  + 0.5 );
			pDstPixel[1] = (u8) ( ( green < 0.0 ) ? 0 : ( 255.0 < green ) ? 255.0 : green + 0.5 );
			pDstPixel[2] = (u8) ( (   red < 0.0 ) ? 0 : ( 255.0 < red   ) ? 255.0 : red   + 0.5 );
			pDstPixel[3] = (u8) ( ( alpha < 0.0 ) ? 0 : ( 255.0 < alpha ) ? 255.0 : alpha + 0.5 );
			pDstPixel += iDstWidth * 4;
		}
	}

	return ( x == iDstWidth ) ? 0 : -1;
}

static int VerticalFilter( u8* pDstImage, int iDstWidth, int iDstHeight,
                           const u8* pSrcImage, int iSrcWidth, int iSrcHeight,
                           double y_factor,
                           const FilterInfo* pColorFilterInfo, const double dColorBlur, ContributionInfo* pColorContribution,
                           const FilterInfo* pAlphaFilterInfo, const double dAlphaBlur, ContributionInfo* pAlphaContribution,
                           const EdgeFilter aFilterEdge,
                           const size_t ACTT_UNUSED(span) )
{
//	if ( ( iSrcWidth  == iDstWidth ) && ( iSrcHeight == iDstHeight) )
//	{
//		// Equal width and height - just copy pixels.
//		PixelFormat::CopyPixels( *pSrcPF, pSrcImage, *pDstPF, pDstImage, iSrcWidth * iSrcHeight );
//		memcpy( pDstImage, pSrcImage, iSrcWidth * iSrcHeight * 4 );
//		return 0;
//	}

	// Apply filter to resize vertically from source to destination.
	double ColorScale = dColorBlur * tlt_max( 1.0 / y_factor, 1.0 );
	double ColorSupport = tlt_max( ColorScale * pColorFilterInfo->support, 0.5 );
	if ( 0.5 < ColorSupport )
		/*SetImageType( destination, TrueColorType )*/;
	else
	{
		// Reduce to point sampling.
		ColorSupport = 0.5 + Epsilon;
		ColorScale = 1.0;
	}
	double AlphaScale = dAlphaBlur * tlt_max( 1.0 / y_factor, 1.0 );
	double AlphaSupport = tlt_max( AlphaScale * pAlphaFilterInfo->support, 0.5 );
	if ( 0.5 < AlphaSupport )
		/*SetImageType( destination, TrueColorType )*/;
	else
	{
		// Reduce to point sampling.
		AlphaSupport = 0.5 + Epsilon;
		AlphaScale = 1.0;
	}

	EdgeFilter FilterEdge(aFilterEdge);
	if ( iSrcHeight < ColorSupport )
		FilterEdge = EDGE_CLAMP; // tiling/miroring will not work for image size that is smaller than filter size
	int y;
	for ( y = 0; y < iDstHeight; y++ )
	{
		double center;
		int i, nc, ColorStart, ColorEnd, na, AlphaStart, AlphaEnd;

		switch ( FilterEdge )
		{
			case EDGE_CLAMP :
				center = (double)y / y_factor + ( 1.0 / ( 2.0 * y_factor ) );
				ColorStart = (int)tlt_max( center - ColorSupport + 0.5, 0. );
				ColorEnd = (int)tlt_min( center + ColorSupport + 0.5, (double)iSrcHeight );
				AlphaStart = (int)tlt_max( center - AlphaSupport + 0.5, 0. );
				AlphaEnd = (int)tlt_min( center + AlphaSupport + 0.5, (double)iSrcHeight );
				break;

			case EDGE_MIRROR :
//				break;

			case EDGE_TILE :
				if ( y == iDstHeight - 1 )
					center = (double)iDstHeight / y_factor -  ( 1.0 / ( 2.0 * y_factor ) );
				else
					center = (double)y / y_factor + ( 1.0 / ( 2.0 * y_factor ) );
				ColorStart = (int)( center - ColorSupport + 0.5 );
				ColorEnd = (int)( center + ColorSupport + 0.5 );
				AlphaStart = (int)( center - AlphaSupport + 0.5 );
				AlphaEnd = (int)( center + AlphaSupport + 0.5 );
				break;

			default :
				assert( 0 );
				return -1;
		}
		double ColorDensity = 0.0;
		for ( nc = 0, i = ColorStart; i < ColorEnd; nc++, i++ )
		{
			pColorContribution[nc].pixel = i;
			pColorContribution[nc].weight = pColorFilterInfo->function( ( i - center + 0.5 ) / ColorScale );
			pColorContribution[nc].weight /= ColorScale;
			ColorDensity += pColorContribution[nc].weight;
		}
		ColorDensity = 0.0 == ColorDensity ? 1.0 : 1.0 / ColorDensity;
		// normalize
		if ( 1 == nc )
			pColorContribution[0].weight = 1;
		else
			for ( i = 0; i < nc; i++ )
				pColorContribution[i].weight *= ColorDensity;

		double AlphaDensity = 0.0;
		for ( na = 0, i = AlphaStart; i < AlphaEnd; na++, i++ )
		{
			pAlphaContribution[na].pixel = i;
			pAlphaContribution[na].weight = pAlphaFilterInfo->function( ( i - center + 0.5 ) / AlphaScale );
			pAlphaContribution[na].weight /= AlphaScale;
			AlphaDensity += pAlphaContribution[na].weight;
		}
		AlphaDensity = 0.0 == AlphaDensity ? 1.0 : 1.0 / AlphaDensity;
		// normalize
		if ( 1 == na )
			pAlphaContribution[0].weight = 1;
		else
			for ( i = 0; i < na; i++ )
				pAlphaContribution[i].weight *= AlphaDensity;

		u8* pDstPixel = pDstImage + ( 0 + y * iDstWidth ) * 4;
		for ( int x = 0; x < iDstWidth; x++ )
		{
			double red = 0.0, green = 0.0, blue = 0.0, alpha = 0.0;
			switch ( FilterEdge )
			{
				case EDGE_CLAMP :
					for ( i = 0; i < nc; i++ )
					{
						const u8* Texel = pSrcImage + ( pColorContribution[i].pixel * iSrcWidth + x ) * 4;
						red   += pColorContribution[i].weight * Texel[2];
						green += pColorContribution[i].weight * Texel[1];
						blue  += pColorContribution[i].weight * Texel[0];
					}
					for ( i = 0; i < na; i++ )
					{
						const u8* Texel = pSrcImage + ( pAlphaContribution[i].pixel * iSrcWidth + x ) * 4;
						alpha += pAlphaContribution[i].weight * Texel[3];
					}
					break;

				case EDGE_MIRROR :
					for ( i = 0; i < nc; i++ )
					{
						int y;
						if ( pColorContribution[i].pixel < 0 )
							y = abs( pColorContribution[i].pixel );
						else
						if ( iSrcHeight <= pColorContribution[i].pixel )
							y = ( iSrcHeight << 1 ) - pColorContribution[i].pixel - 1;
						else
							y = pColorContribution[i].pixel;
						const u8* Texel = pSrcImage + ( y * iSrcWidth + x ) * 4;
						red   += pColorContribution[i].weight * Texel[2];
						green += pColorContribution[i].weight * Texel[1];
						blue  += pColorContribution[i].weight * Texel[0];
					}
					for ( i = 0; i < na; i++ )
					{
						int y;
						if ( pAlphaContribution[i].pixel < 0 )
							y = abs( pAlphaContribution[i].pixel );
						else
						if ( iSrcHeight <= pAlphaContribution[i].pixel )
							y = ( iSrcHeight << 1 ) - pAlphaContribution[i].pixel - 1;
						else
							y = pAlphaContribution[i].pixel;
						const u8* Texel = pSrcImage + ( y * iSrcWidth + x ) * 4;
						alpha += pAlphaContribution[i].weight * Texel[3];
					}
					break;

				case EDGE_TILE :
					for ( i = 0; i < nc; i++ )
					{
						const u8* Texel = pSrcImage + ( ( ( iSrcHeight + pColorContribution[i].pixel ) % iSrcHeight ) * iSrcWidth + x ) * 4;
						red   += pColorContribution[i].weight * Texel[2];
						green += pColorContribution[i].weight * Texel[1];
						blue  += pColorContribution[i].weight * Texel[0];
					}
					for ( i = 0; i < na; i++ )
					{
						const u8* Texel = pSrcImage + ( ( ( iSrcHeight + pColorContribution[i].pixel ) % iSrcHeight ) * iSrcWidth + x ) * 4;
						alpha += pAlphaContribution[i].weight * Texel[3];
					}
					break;

			default :
				TLT_ASSERT( 0 );
			}
			pDstPixel[0] = (u8) ( (  blue < 0.0 ) ? 0 : ( 255.0 < blue  ) ? 255.0 : blue  + 0.5 );
			pDstPixel[1] = (u8) ( ( green < 0.0 ) ? 0 : ( 255.0 < green ) ? 255.0 : green + 0.5 );
			pDstPixel[2] = (u8) ( (   red < 0.0 ) ? 0 : ( 255.0 < red   ) ? 255.0 : red   + 0.5 );
			pDstPixel[3] = (u8) ( ( alpha < 0.0 ) ? 0 : ( 255.0 < alpha ) ? 255.0 : alpha + 0.5 );
			pDstPixel += 4;
		}
	}

	return( y == iDstHeight ? 0 : -1 );
}

ACTRESULT ActTools::RescaleA8R8G8B8( u8* pDstImage, int iDstWidth, int iDstHeight,
                               const u8* pSrcImage, int iSrcWidth, int iSrcHeight,
                               FilterType aColorFilter, double aColorBlur,
                               FilterType aAlphaFilter, double aAlphaBlur,
                               const EdgeFilter aFilterEdgeX, const EdgeFilter aFilterEdgeY )
{
	static const FilterInfo filters[FT_LAST + 1] =
	{
		{ Box, 0.0 },
		{ Box, 0.0 },
		{ Box, 0.5 },
		{ Triangle, 1.0 },
		{ Hermite, 1.0 },
		{ Hanning, 1.0 },
		{ Hamming, 1.0 },
		{ Blackman, 1.0 },
		{ Gaussian, 1.25 },
		{ Quadratic, 1.5 },
		{ Cubic, 2.0 },
		{ Catrom, 2.0 },
		{ Mitchell, 2.0 },
		{ Lanczos, 3.0 },
		{ Bessel, 3.2383 },
		{ Sinc, 4.0 }
	};

	size_t span;

//	Initialize resize image attributes.

	assert( pDstImage != NULL && pSrcImage != NULL );
//	assert(image->signature == MagickSignature);
//	assert(exception != (ExceptionInfo *) NULL);
//	assert(exception->signature == MagickSignature);
//	TLT_ASSERT((filter >= 0) && (filter <= SincFilter));
	if ( ( 0 == iDstWidth ) || ( 0 == iDstHeight ) )
	{
//		ThrowImageException( OptionWarning, "Unable to resize image", "image dimensions are zero" );
		return ACT_ERROR;
	}
	//if ( ( iDstWidth == iSrcWidth ) && ( iDstHeight == iSrcHeight ) )
	//{
	//	// just copy the image
	//	memcpy( pDstImage, pSrcImage, iSrcWidth * iSrcHeight * 4 );
	//	return ACT_OK;
	//}

//	resize_image=CloneImage(image,columns,rows,False,exception);
//	if ( resize_image == (Image *) NULL )
//		return((Image *) NULL);

//    Allocate filter contribution info.
	double xFactor = (double)iDstWidth / iSrcWidth;
	double yFactor = (double)iDstHeight / iSrcHeight;

	double xColorScale = aColorBlur * tlt_max( 1.0 / xFactor, 1.0 );
	double xColorSupport = tlt_max( xColorScale * filters[aColorFilter].support, 0.5 );
	double yColorScale = aColorBlur * tlt_max( 1.0 / yFactor, 1.0 );
	double yColorSupport = tlt_max( yColorScale * filters[aColorFilter].support, 0.5 );
	double ColorSupport = tlt_max( xColorSupport, yColorSupport );
	if ( ColorSupport < filters[aColorFilter].support )
		ColorSupport = filters[aColorFilter].support;
	ContributionInfo* pColorContribution = new ContributionInfo[(int)( ColorSupport * 2 + 3 )];
	if ( NULL == pColorContribution )
	{
		TLT_ASSERT( 0 );
		return ACT_ERROR;
	}

	double xAlphaScale = aAlphaBlur * tlt_max( 1.0 / xFactor, 1.0 );
	double xAlphaSupport = tlt_max( xAlphaScale * filters[aAlphaFilter].support, 0.5 );
	double yAlphaScale = aAlphaBlur * tlt_max( 1.0 / yFactor, 1.0 );
	double yAlphaSupport = tlt_max( yAlphaScale * filters[aAlphaFilter].support, 0.5 );
	double AlphaSupport = tlt_max( xAlphaSupport, yAlphaSupport );
	if ( AlphaSupport < filters[aAlphaFilter].support )
		AlphaSupport = filters[aAlphaFilter].support;
	ContributionInfo* pAlphaContribution = new ContributionInfo[(int)( AlphaSupport * 2 + 3 )];
	if ( NULL == pAlphaContribution )
	{
		TLT_ASSERT( 0 );
		return ACT_ERROR;
	}

//	Resize image.
	u8* pTmpImage = NULL;
//	const PixelFormat* pTmpPF = pSrcPF;
	if ( (size_t)( iDstWidth * ( iSrcHeight + iDstHeight ) ) < (size_t)( iDstHeight * ( iSrcWidth + iDstWidth ) ) )
	{
		int iTmpWidth  = iDstWidth;
		int iTmpHeight = iSrcHeight;
		pTmpImage = new u8[ iTmpWidth * iTmpHeight * 4 ];
		if ( NULL == pTmpImage )
		{
			delete pColorContribution;
			delete pAlphaContribution;
			return ACT_ERROR;
		}

		span = iTmpWidth + iDstHeight;
		if ( HorizontalFilter( pTmpImage, iTmpWidth, iTmpHeight, // dest
		                       pSrcImage, iSrcWidth, iSrcHeight, // source
		                       xFactor, 
		                       &filters[aColorFilter], aColorBlur, pColorContribution, 
		                       &filters[aAlphaFilter], aAlphaBlur, pAlphaContribution, 
		                       aFilterEdgeX,
		                       span ) ||
		     VerticalFilter( pDstImage, iDstWidth, iDstHeight, // dest
		                     pTmpImage, iTmpWidth, iTmpHeight, // source
		                     yFactor,
		                     &filters[aColorFilter], aColorBlur, pColorContribution,
		                     &filters[aAlphaFilter], aAlphaBlur, pAlphaContribution,
		                     aFilterEdgeY,
		                     span ) )
		{
			TLT_ASSERT( 0 );
		}
	}
	else
	{
		int iTmpWidth  = iSrcWidth;
		int iTmpHeight = iDstHeight;
		pTmpImage = new u8[ iTmpWidth * iTmpHeight * 4 ];
		if ( NULL == pTmpImage )
		{
			delete pColorContribution;
			delete pAlphaContribution;
			return ACT_ERROR;
		}

		span = iDstWidth + iTmpHeight;
		if ( VerticalFilter( pTmpImage, iTmpWidth, iTmpHeight, // dest
		                     pSrcImage, iSrcWidth, iSrcHeight, // source
		                     yFactor,
		                     &filters[aColorFilter], aColorBlur, pColorContribution,
		                     &filters[aAlphaFilter], aAlphaBlur, pAlphaContribution,
		                     aFilterEdgeY,
		                     span ) ||
		     HorizontalFilter( pDstImage, iDstWidth, iDstHeight, // dest
		                       pTmpImage, iTmpWidth, iTmpHeight, // source
		                       xFactor,
		                       &filters[aColorFilter], aColorBlur, pColorContribution,
		                       &filters[aAlphaFilter], aAlphaBlur, pAlphaContribution,
		                       aFilterEdgeX,
		                       span ) )
		{
			TLT_ASSERT( 0 );
		}
	}

	delete pTmpImage;
	delete pColorContribution;
	delete pAlphaContribution;

	return ACT_OK;
}

ACTRESULT ActTools::RescaleSurface( Surface* aDstSurface,
                              const Surface* aSrcSurface,
                              FilterType aColorFilter, double aColorBlur,
                              FilterType aAlphaFilter, double aAlphaBlur,
                              EdgeFilter aEdgeFilterX, EdgeFilter aEdgeFilterY )
{
	if ( _stricmp( aDstSurface->GetFormat()->GetId(), "A8R8G8B8" ) ||
	     _stricmp( aSrcSurface->GetFormat()->GetId(), "A8R8G8B8" ) )
		return ACT_ERROR;

	/*for (int i = 0; i < aDstSurface->GetWidth(); i++)
	{
		for (int j = 0; j < aDstSurface->GetHeight(); j++)
		{
			float x = ((float) i / aDstSurface->GetWidth());
			float y = ((float) j / aDstSurface->GetHeight());

			U32 pixel = *(((U32*)aSrcSurface->GetTexels()) + U32(x * aSrcSurface->GetWidth() + aSrcSurface->GetWidth() * aSrcSurface->GetHeight() * y));
			*((U32*)(aDstSurface->GetTexels()) + i + j * aDstSurface->GetWidth()) = pixel;

			//U32 g = 255 * ((float) i / aDstSurface->GetWidth());
			//*((U32*)(aDstSurface->GetTexels()) + i + j * aDstSurface->GetWidth()) = 0xff000000 + g + (g << 8) + (g << 16);
		}
	}
	return ACT_OK;*/

	if (aSrcSurface->GetWidth() == 1 && aSrcSurface->GetHeight() == 1
		&& aDstSurface->GetWidth() == 4 && aDstSurface->GetHeight() == 4)
		int breakhere = 0;

	return RescaleA8R8G8B8( const_cast<u8*>(aDstSurface->GetTexels()), (int)aDstSurface->GetWidth(), (int)aDstSurface->GetHeight(),
	                        aSrcSurface->GetTexels(), (int)aSrcSurface->GetWidth(), (int)aSrcSurface->GetHeight(),
	                        aColorFilter, aColorBlur,
	                        aAlphaFilter, aAlphaBlur,
	                        aEdgeFilterX, aEdgeFilterY );
}


