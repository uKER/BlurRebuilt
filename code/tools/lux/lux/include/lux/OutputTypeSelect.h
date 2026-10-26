#ifndef __LUX_OUTPUTTYPESELECT_H__
#define __LUX_OUTPUTTYPESELECT_H__

class JobRenderer;
class ImageFilter;
class ITexture;

namespace LUX
{

class SurfaceIntegrator;


namespace DataTypes
{

/// a list of the different rendering modes (e.g. gotham, the club, per-vertex sh light transfer)
enum RenderMode
{
	E_RenderMode_Default,
	E_RenderMode_GothamNight,
	E_RenderMode_Basis3,
	E_RenderMode_LightTransfer,
	E_RenderMode_RGBDirection,
	E_Num_RenderModes
};

/// a list of the different iterators that exist
enum IteratorType
{
	E_TexelIterator,
	E_VertexIterator,
	E_SamplePointIterator,
	E_Num_Iterators
};

/// define a table which list the different datatypes used to render each mode
extern DataTypes g_ModeDataTypes[E_Num_RenderModes][E_Num_Iterators][2];

/// a function to retrieve a particular iterator given a mode, iterator type and bounce lighting switch
I_JobRenderer *CreateJobRenderer(RenderMode mode, IteratorType iterator, bool bBouncePass, BaseObject *pJob);

/// create a FlexibleTexture for the given rendermode and bounce pass
ITexture *CreateSpecializedTexture(RenderMode mode, bool bBouncePass, int cX, int cY, SurfaceIntegrator *pFilter);

}

}

#endif
