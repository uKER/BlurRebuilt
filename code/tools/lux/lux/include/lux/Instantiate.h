#pragma once	

namespace LUX
{

class Lux;

void PackInstancedMeshNodes(LUX::Lux *pLux, SceneUnwrapper *pUnwrapper, const LightingModelProperties *pProp, const Ut::Vector<LightingPass*> *pLightingPasses, bool bInstantiate=false);

#define PCFM_OK				0
#define PCFM_NO_CHARTS		1
#define PCFM_DOES_NOT_FIT	2

}
