#pragma once

#include "CServ/ClusterServHeaders.h"
#include "Lux/RasterizedTriIterator.h"

class RasterizedTriIterator;
class ITexture;

namespace LUX
{

class SurfaceIntegrator;
class SamplePointJob;
class SHLightingJob;
class LightmapJob;
class VtxLightingJob;

// class that processes a job
class ClientJobRenderer : public ClientInterface
{
public:
	DECLARE_PERSISTENCE(ClientJobRenderer);
	ClientJobRenderer() {};
	virtual ~ClientJobRenderer();

	void write(ost::Engine &ar);
	void read(ost::Engine &ar);

	void Initialize();
	BaseObject *Process(BaseObject *pJob);
	BaseObject *ProcessSamplePointJob(SamplePointJob *pJob);
	BaseObject *ProcessSHLightingJob(SHLightingJob *pJob);
	BaseObject *ProcessLightmapJob(LightmapJob *pJob);
	BaseObject *ProcessVtxLightingJob(VtxLightingJob *pJob);
	void CleanUp();

	// timing variables
	U64						m_timer;

	// persistent variables (to be serialized)
	SurfaceIntegrator		*m_pIntegrator;
	int						m_iBiggestArea;
	float					m_fDensity;
};

}
