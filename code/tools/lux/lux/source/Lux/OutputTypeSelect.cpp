#include "stdafx.h"

#include "Lux/LightSample.h"
#include "Lux/DataTypes.h"
#include "Lux/I_JobRenderer.h"
#include "Lux/GlobalIllumination/Integrator.h"
#include "Lux/OutputTypeSelect.h"
#include "Lux/VtxLighting.h"
#include "Lux/I_JobRenderer.h"
#include "Lux/VertexIterator2.h"
#include "Lux/SamplePointIterator.h"
#include "Lux/TextureTypes/DefaultTexture.h"
#include "Lux/TextureTypes/RGBTexture.h"
#include "Lux/Rasterize.h"
#include "Lux/LightmapRenderer.h"
#include "Lux/ImageFilter.h"
#include "Lux/Sampler.h"
#include "Lux/RenderChannel.h"
#include "Lux/TextureTypes/FlexibleTexture.h"

namespace LUX
{

namespace DataTypes
{

DataTypes g_ModeDataTypes[E_Num_RenderModes][E_Num_Iterators][2] =
{
	// default
	{
	{ E_DataType_Colour,			E_DataType_Colour },			// texel iterator
	{ E_DataType_Colour,			E_DataType_Colour },			// vertex iterator
	{ E_DataType_None,				E_DataType_GothamSamplePoint },	// samplepoint iterator
	},

	// Alpha Scaling (Gotham Night)
	{
	{ E_DataType_Colour,			E_DataType_GothamNight },		// texel iterator
	{ E_DataType_Colour,			E_DataType_Colour },			// vertex iterator
	{ E_DataType_None,				E_DataType_GothamSamplePoint },	// samplepoint iterator
	},

	// basis-3 lightmaps (The Club)
	{
	{ E_DataType_ColourAndSky,		E_DataType_Basis3TheClub },		// texel iterator
	{ E_DataType_Colour,			E_DataType_Basis3TheClub },		// vertex iterator
	{ E_DataType_None,				E_DataType_SamplePoint },		// sample point iterator
	},

	// light transfer
	{
	{ E_DataType_SHVector,			E_DataType_SHVector01 },		// texel iterator
	{ E_DataType_SHVector,			E_DataType_SHVector01 },		// vertex iterator
	{ E_DataType_None,				E_DataType_TransferMatrix },	// samplepoint iterator
	},

	// RGB direction
	{
	{ E_DataType_ColourAndSky,		E_DataType_RGBDirection },		// texel iterator
	{ E_DataType_Colour,			E_DataType_RGBDirection },		// vertex iterator
	{ E_DataType_None,				E_DataType_SamplePoint },		// samplepoint iterator
	},
};

// instantiate an iterator with the specified datatype
template<typename T>
I_JobRenderer *CreateJobRenderer(IteratorType type, BaseObject *pJob)
{
	switch (type)
	{
	case E_TexelIterator:
		return new LightmapRenderer((LightmapJob*)pJob);
	case E_VertexIterator:
		return new VertexIterator2<T>((VtxLightingJob*)pJob);
	case E_SamplePointIterator:
		return new SamplePointIterator<T>((SamplePointJob*)pJob);
	default:
		assert(0);
		break;
	}
	return NULL;
}

/// a function to retrieve a particular iterator given a mode, iterator type and bounce lighting switch
I_JobRenderer *CreateJobRenderer(RenderMode mode, IteratorType iterator, bool bBouncePass, BaseObject *pJob)
{
	// get the datatype
	DataTypes type = g_ModeDataTypes[mode][iterator][bBouncePass?0:1];
	switch (type)
	{
	case E_DataType_Scalar:					return CreateJobRenderer<Scalar>(iterator, pJob);					break;
	case E_DataType_Colour:					return CreateJobRenderer<Colour>(iterator, pJob);					break;
	case E_DataType_ColourAlphaScale:		return CreateJobRenderer<ColourAlphaScale>(iterator, pJob);			break;
	case E_DataType_SHVector:				return CreateJobRenderer<SHVector<2> >(iterator, pJob);				break;
	case E_DataType_SHVectorRGB:			return CreateJobRenderer<SHVectorRGB<2> >(iterator, pJob);			break;
	case E_DataType_SamplePoint:			return CreateJobRenderer<SamplePoint>(iterator, pJob);				break;
	case E_DataType_SHVector01:				return CreateJobRenderer<SHVector01<2> >(iterator, pJob);			break;
	case E_DataType_TransferMatrix:			return CreateJobRenderer<TransferMatrix<2> >(iterator, pJob);		break;
	case E_DataType_Basis3:					return CreateJobRenderer<Basis3>(iterator, pJob);					break;
	case E_DataType_GothamSamplePoint:		return CreateJobRenderer<GothamSamplePoint>(iterator, pJob);		break;
	case E_DataType_GothamNight:			return CreateJobRenderer<GothamNight>(iterator, pJob);				break;
	case E_DataType_RGBDirection:			return CreateJobRenderer<RGBDirection>(iterator, pJob);				break;
	case E_DataType_Basis3TheClub:			return CreateJobRenderer<Basis3TheClub>(iterator, pJob);			break;

	default: assert(0); break;
	}

	return NULL;
}

/// create a FlexibleTexture for the given rendermode and bounce pass
ITexture *CreateSpecializedTexture(RenderMode mode, bool bBouncePass, int cX, int cY, SurfaceIntegrator *pI)
{
	DataTypes type = g_ModeDataTypes[mode][E_TexelIterator][bBouncePass?0:1];
	switch (type)
	{
	case E_DataType_Scalar:				return new FlexibleTexture<Scalar>(cX, cY, pI);				break;
	case E_DataType_Colour:				return new FlexibleTexture<Colour>(cX, cY, pI);				break;
	case E_DataType_ColourAlphaScale:	return new FlexibleTexture<ColourAlphaScale>(cX, cY, pI);	break;
	case E_DataType_SHVector:			return new FlexibleTexture<SHVector<2> >(cX, cY, pI);		break;
	case E_DataType_SHVectorRGB:		return new FlexibleTexture<SHVectorRGB<2> >(cX, cY, pI);	break;
	case E_DataType_SamplePoint:		return new FlexibleTexture<SamplePoint>(cX, cY, pI);		break;
	case E_DataType_SHVector01:			return new FlexibleTexture<SHVector01<2> >(cX, cY, pI);		break;
	case E_DataType_TransferMatrix:		return new FlexibleTexture<TransferMatrix<2> >(cX, cY, pI);	break;
	case E_DataType_Basis3:				return new FlexibleTexture<Basis3>(cX, cY, pI);				break;
	case E_DataType_Basis3TheClub:		return new FlexibleTexture<Basis3TheClub>(cX, cY, pI);		break;
	case E_DataType_ColourAndSky:		return new FlexibleTexture<ColourAndSky>(cX, cY, pI);		break;
	case E_DataType_GothamNight:		return new FlexibleTexture<GothamNight>(cX, cY, pI);		break;
	case E_DataType_RGBDirection:		return new FlexibleTexture<RGBDirection>(cX, cY, pI);		break;
	default: assert(0); break;
	}

	return NULL;
}


};

};
