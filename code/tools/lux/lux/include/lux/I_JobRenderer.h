#ifndef __LUX_SURFACE_RENDERER_H__
#define __LUX_SURFACE_RENDERER_H__

namespace LUX
{

class SurfaceIntegrator;

}

// surface renderers are classes that can render a lighting job e.g. lightmapjob, vertexlightingjob, etc.
class I_JobRenderer
{
public:
	// virtual detructor
	virtual ~I_JobRenderer() {};

	/// return the object containing the resulting data, to be sent back to the server
	virtual BaseObject *Process(LUX::SurfaceIntegrator *pIntegrator) = 0;
};


#endif
