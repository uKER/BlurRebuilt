#ifndef GLOBALILLUMINATION_MODEL
#define GLOBALILLUMINATION_MODEL

#include "Lux\Rasterize.h"
#include <spatial_hierarchy\Light-Tree.h>

class JobServer;
class RGBTexture;

namespace LUX
{
class DirectionalLight;
class SurfaceIntegrator;
class GlobalIlluminationModel;
class SceneUnwrapper;
class SHLighting;
class VtxLighting;

// global illuminatioon model lighting class
class GlobalIlluminationModel : public LightingModel
{
public:
	GlobalIlluminationModel();
	GlobalIlluminationModel(const GlobalIlluminationModel& rhs);
	virtual ~GlobalIlluminationModel();
	GlobalIlluminationModel& operator=(const GlobalIlluminationModel& rhs);
	LuxModelTypeEnum GetType() const;

	// main lighting function
	bool Light(
		LUX::SpatialHierarchy* pHierachy, 
		const LightingModelProperties* pProperties,
		Lux* pLux,
		Aftk::C_Scene* pAftkScene,
		Progress::I_ProgressBar *pProgress
	);

	// light kd-tree
	LightTree		*m_pLightTree;
	// global light list
	LightPtrVector	m_GlobalLightList;
	// global directional light
	DirectionalLight*m_pDirectionalLight;

	LUX::SpatialHierarchy			*GetSpatialHierarchy() { return m_pHierarchy; }
	
	GlobalIlluminationProperties	*m_pGIProperties;
	SurfaceIntegrator				*m_pIntegrator;
	SceneUnwrapper					*m_pUnwrapper;
	SHLighting						*m_pSHLighting;
	VtxLighting						*m_pVtxLighting;
	bool							m_bBouncePass;
	Progress::I_ProgressBar			*m_pUnwrapProgress;
	Ut::Vector<Progress::I_ProgressBar*> m_RenderProgress;
	Progress::I_ProgressBar			*m_pPackingProgress;
	Progress::I_ProgressBar			*m_pValidateProgress;

	void	Reset();
	void	ForceGothamChannels();
	void	ForceShooterChannels();
	bool	Render();
	void	InitializeLights();
	void	FreeLights();
	void	RenderVertexAndSHLighting();
	static BaseObject *GenerateJobStatic(void *pContext, int iChart);
	BaseObject *GenerateJob(int iChart);

	LUX::SpatialHierarchy		*m_pHierarchy;
};

}

#endif
