#include "stdafx.h"

#include "Lux/RenderChannel.h"
#include "Lux/SurfaceIterator.h"
#include "Lux/GlobalIllumination/Integrator.h"
#include "Lux/SamplePoints.h"
#include "Lux/I_JobRenderer.h"
#include "Lux/SamplePointIterator.h"
#include "Lux/IIntegrator.h"
#include "Lux/Common.h"
#include "Lux/DataTypes.h"

using namespace LUX;
using namespace XERCES_CPP_NAMESPACE;

IMPLEMENT_PERSISTENCE(SamplePointResult, "SPointResult");
IMPLEMENT_PERSISTENCE(SamplePointJob, "SPointJob");

